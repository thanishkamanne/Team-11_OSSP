#!/usr/bin/env python3
"""Smart Resource Allocator - OSSP (Operating Systems) project.

Real Linux features used:
  /proc          live CPU, memory, load and per-process data
  fork / exec    worker processes are created with subprocess (fork+exec) and fork() again to burn CPU
  pipes (IPC)    the manager sends allocation commands to each worker through its stdin pipe
  signals        SIGSTOP / SIGCONT / SIGKILL control a worker's whole process group
  nice           setpriority() changes scheduling priority
  Banker's       decides whether a request is safe before it is granted

Run:  python3 server.py      then open http://localhost:8000   (no packages needed)
"""
import atexit, json, os, signal, subprocess, sys, threading, time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

HERE = os.path.dirname(os.path.abspath(__file__))
CLK, PAGE = os.sysconf('SC_CLK_TCK'), os.sysconf('SC_PAGE_SIZE')
LOCK = threading.Lock()
RES = ['CPU cores', 'RAM (MB)']

WORKER = r'''
import os, sys
hold = []
for line in sys.stdin:                      # commands arrive through the pipe
    kind, n = line.split(); n = int(n)
    if kind == 'mem':
        hold.append(bytearray(b'x') * (n * 1048576))   # really touches n MB
    elif kind == 'cpu':
        for _ in range(n):
            if os.fork() == 0:              # child process that burns one core
                while True: pass
'''

# ---------------------------------------------------------------- /proc readers
def meminfo():
    d = {}
    for line in open('/proc/meminfo'):
        k, v = line.split(':')
        d[k] = int(v.split()[0]) // 1024
    return d

_prev_cpu = [None]
def cpu_percent():
    f = [int(x) for x in open('/proc/stat').readline().split()[1:]]
    idle, total = f[3] + f[4], sum(f)
    p, _prev_cpu[0] = _prev_cpu[0], (idle, total)
    if not p or total == p[1]: return 0.0
    return round(100 * (1 - (idle - p[0]) / (total - p[1])), 1)

_prev_proc = {}
def procs():
    global _prev_proc
    now, out, cur = time.time(), [], {}
    managed = set(W)
    for d in os.listdir('/proc'):
        if not d.isdigit(): continue
        try:
            s = open(f'/proc/{d}/stat').read()
            name = s[s.index('(') + 1:s.rindex(')')]
            r = s[s.rindex(')') + 2:].split()
            pid, ticks = int(d), int(r[11]) + int(r[12])
            cur[pid] = (ticks, now)
            o = _prev_proc.get(pid)
            cpu = (ticks - o[0]) / CLK / (now - o[1]) * 100 if o and now > o[1] else 0.0
            out.append(dict(pid=pid, ppid=int(r[1]), name=name, st=r[0], cpu=round(cpu, 1),
                            rss=round(int(r[21]) * PAGE / 1048576, 1), nice=int(r[16]), th=int(r[17]),
                            m=pid in managed or int(r[1]) in managed))
        except (OSError, ValueError, IndexError):
            continue
    _prev_proc = cur
    return out

def system():
    m = meminfo()
    ps = procs()
    states = {}
    for p in ps: states[p['st']] = states.get(p['st'], 0) + 1
    return dict(cpu=cpu_percent(), cores=os.cpu_count(), load=[round(x, 2) for x in os.getloadavg()],
                up=int(float(open('/proc/uptime').read().split()[0])), nproc=len(ps), states=states,
                mem=dict(total=m['MemTotal'], used=m['MemTotal'] - m['MemAvailable'], avail=m['MemAvailable'],
                         swap=m['SwapTotal'], swapused=m['SwapTotal'] - m['SwapFree']),
                procs=sorted(ps, key=lambda p: (-p['cpu'], -p['rss']))[:40])

# ---------------------------------------------------------------- Banker's algorithm
POOL = [os.cpu_count(), max(256, int(meminfo()['MemAvailable'] * 0.5))]   # resources the manager may hand out
W = {}      # pid -> worker record
LOG = []

def need(x): return [x['mx'][j] - x['a'][j] for j in range(2)]
def avail(ws=None): return [POOL[j] - sum(x['a'][j] for x in (ws or W.values())) for j in range(2)]

def safe(ws, av):
    w, done, seq = list(av), [False] * len(ws), []
    while True:
        for i, x in enumerate(ws):
            if not done[i] and all(need(x)[j] <= w[j] for j in range(2)):
                seq.append(x['pid']); w = [w[j] + x['a'][j] for j in range(2)]; done[i] = True; break
        else:
            break
    return all(done), seq, [ws[i]['pid'] for i, d in enumerate(done) if not d], w

def fmt(v): return f'{v[0]} cores, {v[1]} MB'

def decide(x, req):
    nd, av = need(x), avail()
    st = [dict(ok=None, d='Not reached, an earlier check already failed.') for _ in range(3)]
    over = [j for j in range(2) if req[j] > nd[j]]
    if over:
        w = f"Asked for {', '.join(f'{req[j]} of {RES[j]}' for j in over)} but the remaining Need is only {fmt(nd)}."
        st[0] = dict(ok=False, d=w); return dict(k='claim', why=w, st=st)
    st[0] = dict(ok=True, d=f'Request is within the remaining Need ({fmt(nd)}).')
    short = [j for j in range(2) if req[j] > av[j]]
    if short:
        w = f"Only {fmt(av)} is available, so {' and '.join(f'{RES[j]} is short by {req[j] - av[j]}' for j in short)}. The request is deferred."
        st[1] = dict(ok=False, d=w); return dict(k='wait', why=w, st=st)
    st[1] = dict(ok=True, d=f'{fmt(av)} available, enough for the request.')
    trial = [dict(y, a=[y['a'][j] + (req[j] if y is x else 0) for j in range(2)]) for y in W.values()]
    ok, seq, stuck, w = safe(trial, [av[j] - req[j] for j in range(2)])
    if ok:
        t = 'A safe sequence exists after a temporary grant: ' + ' -> '.join(f'P{p}' for p in seq) + '. Allocation committed.'
        st[2] = dict(ok=True, d=t); return dict(k='ok', why=t, st=st)
    t = (f"If granted, {', '.join(f'P{p}' for p in stuck)} could not be guaranteed to finish: at most {fmt(w)} would ever be free. "
         "The temporary change is rolled back. Unsafe does not mean deadlock has happened, only that it can no longer be ruled out.")
    st[2] = dict(ok=False, d=t); return dict(k='unsafe', why=t, st=st)

def reap():
    for pid in [p for p, x in W.items() if x['proc'].poll() is not None]:
        LOG.append(dict(k='ok', h=f"P{pid} {W[pid]['name']} exited and released its resources", why='Resources returned to the pool.', t=time.time()))
        del W[pid]

def workers():
    reap()
    ws = list(W.values())
    ok, seq, stuck, w = safe(ws, avail())
    live = {p['pid']: p for p in procs()}
    return dict(pool=POOL, avail=avail(), res=RES, safe=dict(ok=ok, seq=seq, stuck=stuck), log=LOG[-40:][::-1],
                workers=[dict(pid=x['pid'], name=x['name'], a=x['a'], mx=x['mx'], nd=need(x),
                              st=live.get(x['pid'], {}).get('st', '?'), nice=live.get(x['pid'], {}).get('nice', 0),
                              kids=sum(1 for p in live.values() if p['ppid'] == x['pid'])) for x in ws])

# ---------------------------------------------------------------- actions
def spawn(b):
    mx = [int(b['mx'][0]), int(b['mx'][1])]
    if len(W) >= 8: return dict(error='At most 8 managed workers.')
    if mx[0] < 0 or mx[1] < 0 or mx[0] > POOL[0] or mx[1] > POOL[1]:
        return dict(error=f'Maximum claim must fit inside the pool ({fmt(POOL)}).')
    p = subprocess.Popen([sys.executable, '-c', WORKER], stdin=subprocess.PIPE, stdout=subprocess.DEVNULL,
                         stderr=subprocess.DEVNULL, start_new_session=True)     # fork + exec, own process group
    W[p.pid] = dict(pid=p.pid, name=str(b.get('name') or 'Worker')[:24], a=[0, 0], mx=mx, proc=p)
    return dict(ok=True, pid=p.pid)

def request(b):
    x = W.get(int(b['pid']))
    if not x: return dict(error='Unknown worker.')
    req = [max(0, int(b['req'][0])), max(0, int(b['req'][1]))]
    if not any(req): return dict(error='Enter a request above zero.')
    d = decide(x, req)
    if d['k'] == 'ok':
        x['a'] = [x['a'][j] + req[j] for j in range(2)]
        try:                                                                  # IPC: tell the worker through its pipe
            if req[0]: x['proc'].stdin.write(f'cpu {req[0]}\n'.encode())
            if req[1]: x['proc'].stdin.write(f'mem {req[1]}\n'.encode())
            x['proc'].stdin.flush()
        except OSError: pass
    LOG.append(dict(k=d['k'], h=f"P{x['pid']} {x['name']} requested {fmt(req)}", why=d['why'], t=time.time()))
    return d

def signal_worker(b):
    x = W.get(int(b['pid']))
    if not x: return dict(error='Only managed workers can be controlled.')
    s = dict(stop=signal.SIGSTOP, cont=signal.SIGCONT, kill=signal.SIGKILL)[b['sig']]
    os.killpg(x['pid'], s)
    if b['sig'] == 'kill':
        try: x['proc'].wait(timeout=2)
        except Exception: pass
    return dict(ok=True)

def renice(b):
    x = W.get(int(b['pid']))
    if not x: return dict(error='Only managed workers can be reniced.')
    try: os.setpriority(os.PRIO_PGRP, x['pid'], max(-20, min(19, int(b['nice']))))
    except PermissionError: return dict(error='Lowering niceness needs root. Raising it is allowed.')
    return dict(ok=True)

@atexit.register
def cleanup():
    for x in list(W.values()):
        try: os.killpg(x['pid'], signal.SIGKILL)
        except OSError: pass

# ---------------------------------------------------------------- HTTP
TYPES = {'.html': 'text/html', '.css': 'text/css', '.js': 'text/javascript'}
class H(BaseHTTPRequestHandler):
    def log_message(self, *a): pass
    def out(self, body, ctype='application/json', code=200):
        data = body if isinstance(body, bytes) else json.dumps(body).encode()
        self.send_response(code); self.send_header('Content-Type', ctype); self.send_header('Content-Length', str(len(data)))
        self.end_headers(); self.wfile.write(data)
    def do_GET(self):
        p = self.path.split('?')[0]
        with LOCK:
            if p == '/api/system': return self.out(system())
            if p == '/api/workers': return self.out(workers())
        name = 'index.html' if p == '/' else p.lstrip('/')
        f = os.path.join(HERE, os.path.basename(name))
        if os.path.splitext(f)[1] in TYPES and os.path.isfile(f):
            return self.out(open(f, 'rb').read(), TYPES[os.path.splitext(f)[1]])
        self.out({'error': 'not found'}, code=404)
    def do_POST(self):
        try:
            b = json.loads(self.rfile.read(int(self.headers.get('Content-Length', 0))) or b'{}')
            fn = dict(spawn=spawn, request=request, signal=signal_worker, nice=renice)[self.path.split('/')[-1]]
            with LOCK: self.out(fn(b))
        except Exception as e:
            self.out(dict(error=str(e)), code=400)

if __name__ == '__main__':
    print(f'Pool for the allocator: {fmt(POOL)}')
    print('Open http://localhost:8000  (Ctrl+C to stop)')
    ThreadingHTTPServer(('127.0.0.1', 8000), H).serve_forever()
