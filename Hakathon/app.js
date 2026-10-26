const $ = (s, r = document) => r.querySelector(s), $$ = (s, r = document) => [...r.querySelectorAll(s)];
const api = (p, b) => fetch('/api/' + p, b ? {method: 'POST', headers: {'Content-Type': 'application/json'}, body: JSON.stringify(b)} : undefined).then(r => r.json());
const fmt = v => `${v[0]} cores, ${v[1]} MB`, vec = v => '[' + v.join(', ') + ']';
const BD = {ok: 'Approved', wait: 'Deferred', unsafe: 'Unsafe', claim: 'Over claim'}, LBL = {ok: 'Granted', wait: 'Deferred, must wait', unsafe: 'Rejected, unsafe state', claim: 'Rejected, exceeds maximum'};
const STN = {R: 'Running', S: 'Sleeping', D: 'Disk wait', T: 'Stopped', Z: 'Zombie', I: 'Idle'};
const T = {dash: 'System dashboard', proc: 'Processes', alloc: 'Smart allocator'};
const hist = []; let wkey = '';

const ring = (p, c) => `<svg viewBox="0 0 100 100" width="104" role="img" aria-label="${Math.round(p)} percent"><circle cx="50" cy="50" r="42" fill="none" stroke="var(--ln)" stroke-width="9"/><circle cx="50" cy="50" r="42" fill="none" style="stroke:${c}" stroke-width="9" stroke-linecap="round" stroke-dasharray="${2.64 * p} 264" transform="rotate(-90 50 50)"/><text x="50" y="56" text-anchor="middle" font-size="20" font-weight="600" style="fill:var(--ink)">${Math.round(p)}%</text></svg>`;
const up = s => `${Math.floor(s / 3600)}h ${Math.floor(s % 3600 / 60)}m`;

function procTable(list, n) {
  return '<tr><th>PID<th>PPID<th>Name<th>State<th>CPU %<th>RAM (MB)<th>Nice<th>Threads</tr>' +
    list.slice(0, n).map(p => `<tr class="${p.m ? 'mg' : ''}"><td>${p.pid}<td>${p.ppid}<td>${p.name}<td>${STN[p.st] || p.st}<td>${p.cpu}<td>${p.rss}<td>${p.nice}<td>${p.th}`).join('');
}

function renderSystem(s) {
  const m = s.mem, mp = m.used / m.total * 100, sp = m.swap ? m.swapused / m.swap * 100 : 0;
  $('#kpis').innerHTML = [['CPU usage', s.cpu + '%', s.cores + ' cores'], ['Memory', `${m.used} / ${m.total} MB`, `${m.avail} MB available`], ['Load average', s.load.join('  '), '1, 5 and 15 minutes'], ['Uptime', up(s.up), `${s.nproc} processes`]]
    .map(k => `<div class="card kpi"><small>${k[0]}</small><b style="font-size:${k[1].length > 11 ? 18 : 26}px">${k[1]}</b><span>${k[2]}</span></div>`).join('');
  $('#rings').innerHTML = [['CPU', s.cpu, s.cpu + '%'], ['Memory', mp, `${m.used} MB`], ['Swap', sp, `${m.swapused} MB`]]
    .map(r => `<div>${ring(r[1], r[1] > 85 ? 'var(--wa)' : 'var(--ac)')}<b>${r[0]}</b><small>${r[2]}</small></div>`).join('');
  hist.push(s.cpu); if (hist.length > 40) hist.shift();
  $('#spark').innerHTML = `<polyline points="${hist.map((v, i) => `${i / 39 * 200},${58 - v * .56}`).join(' ')}"/>`;
  const mx = Math.max(1, ...Object.values(s.states));
  $('#states').innerHTML = Object.entries(s.states).map(([k, v]) => `<div class="bar2"><span style="width:110px">${STN[k] || k}</span><div class="tr"><div style="width:${v / mx * 100}%"></div></div><b>${v}</b></div>`).join('');
  $('#top').innerHTML = procTable(s.procs, 8); $('#pt').innerHTML = procTable(s.procs, 40);
}

function renderWorkers(w) {
  const name = p => { const x = w.workers.find(y => y.pid == p); return x ? `P${p} ${x.name}` : 'P' + p };
  $('#pool').innerHTML = [['Pool (cores, MB)', fmt(w.pool)], ['Allocated', fmt(w.pool.map((t, j) => t - w.avail[j]))], ['Available', fmt(w.avail)], ['State', w.safe.ok ? 'Safe' : 'Unsafe']]
    .map(k => `<div class="card kpi"><small>${k[0]}</small><b style="font-size:18px">${k[1]}</b></div>`).join('');
  $('#wt').innerHTML = '<tr><th>PID<th>Name<th>Allocation<th>Maximum<th>Need<th>State<th>Nice<th>Children<th>Actions</tr>' + (w.workers.map(x => `<tr><td>${x.pid}<td>${x.name}<td>${vec(x.a)}<td>${vec(x.mx)}<td>${vec(x.nd)}<td>${STN[x.st] || x.st}<td>${x.nice}<td>${x.kids}<td class="act"><button class="btn" data-a="stop" data-p="${x.pid}">Stop</button><button class="btn" data-a="cont" data-p="${x.pid}">Continue</button><button class="btn" data-a="nice" data-p="${x.pid}">Nice +5</button><button class="btn" data-a="kill" data-p="${x.pid}">Finish</button>`).join('') || '<tr><td colspan="9" class="mu">No workers yet. Create one or press "Create 3 demo workers".</td></tr>');
  $('#chain').innerHTML = w.safe.seq.map(p => `<div class="node">P${p}<small>${name(p).slice(name(p).indexOf(' ') + 1)}</small></div>`).concat(w.safe.stuck.map(p => `<div class="node bad">P${p}<small>stuck</small></div>`)).join('<span class="mu">→</span>') || '<span class="mu">No workers.</span>';
  const key = w.workers.map(x => x.pid).join(',');
  if (key != wkey) { const v = $('#rp').value; $('#rp').innerHTML = w.workers.map(x => `<option value="${x.pid}">P${x.pid} ${x.name}</option>`).join(''); if (v) $('#rp').value = v; wkey = key }
  $('#log').innerHTML = w.log.map(e => `<li><span class="bd ${e.k}">${BD[e.k]}</span> <b>${e.h}</b><p>${e.why}</p></li>`).join('') || '<li class="mu">No decisions yet.</li>';
}

async function tick() {
  try { const [s, w] = await Promise.all([api('system'), api('workers')]); renderSystem(s); renderWorkers(w); $('#pill').className = 'pill ok'; $('#pill').textContent = 'Live' }
  catch (e) { $('#pill').className = 'pill no'; $('#pill').textContent = 'Server offline' }
}

function show() { const v = T[location.hash.slice(1)] ? location.hash.slice(1) : 'dash'; $$('.view').forEach(s => s.hidden = s.id != 'v-' + v); $$('nav a').forEach(a => a.removeAttribute('aria-current')); $('nav a[data-v=' + v + ']').setAttribute('aria-current', 'page'); $('#title').textContent = T[v] }
addEventListener('hashchange', show);

$('#wt').onclick = async e => {
  const a = e.target.dataset.a, p = +e.target.dataset.p; if (!a) return;
  const r = a == 'nice' ? await api('nice', {pid: p, nice: 5}) : await api('signal', {pid: p, sig: a});
  if (r.error) alert(r.error); tick();
};
$('#spawn').onclick = async () => { const r = await api('spawn', {name: $('#wn').value, mx: [+$('#wc').value, +$('#wm').value]}); if (r.error) alert(r.error); $('#wn').value = ''; tick() };
$('#demo').onclick = async () => {
  const w = await api('workers'), [c, m] = w.pool;
  for (const [n, fc, fm] of [['Web Server', .25, .15], ['Database', .34, .35], ['AI Processing', .5, .5]])
    await api('spawn', {name: n, mx: [Math.max(1, Math.round(c * fc)), Math.round(m * fm)]});
  tick();
};
$('#sub').onclick = async () => {
  const o = $('#res'), r = await api('request', {pid: +$('#rp').value, req: [+$('#r0').value, +$('#r1').value]});
  if (r.error) { o.innerHTML = `<p class="no">${r.error}</p>`; return }
  const N = ['Request ≤ Need', 'Request ≤ Available', 'Safety algorithm after temporary grant'];
  o.innerHTML = `<div class="verdict ${r.k}">${LBL[r.k]}</div><div class="steps">` + r.st.map((s, k) => `<div class="step ${s.ok === true ? 'ok' : s.ok === false ? 'fail' : 'skip'}"><i>${s.ok === true ? '✓' : s.ok === false ? '✗' : k + 1}</i><div><b>${N[k]}</b><p>${s.d}</p></div></div>`).join('') + '</div>';
  tick();
};
$('#th').onchange = e => { document.documentElement.dataset.theme = e.target.value };
document.documentElement.dataset.theme = 'neon'; show(); tick(); setInterval(tick, 1500);
