# Smart Resource Allocator (OSSP - Operating Systems)

A web dashboard backed by a Python program that uses real Linux features.
Needs only Python 3 (already on Ubuntu). No packages to install.

## Run
    cd ossp-linux
    python3 server.py
Open http://localhost:8000 in Firefox.

## OS concepts and where they are in the code (server.py)
- /proc filesystem   meminfo(), cpu_percent(), procs(), system()
- fork + exec        spawn() starts each worker; the worker fork()s children that burn CPU (WORKER string)
- pipes (IPC)        request() writes "cpu N" / "mem N" commands to the worker's stdin pipe
- signals            signal_worker() sends SIGSTOP, SIGCONT, SIGKILL to the worker's process group
- priority           renice() calls os.setpriority()
- deadlock avoidance safe() and decide() implement Banker's algorithm over (CPU cores, RAM MB)

## Safety
Only workers started by this app can be stopped, killed or reniced. It binds to 127.0.0.1 only.
Workers are killed when the server exits. The pool is half of the free RAM at start-up.

## Files
server.py (backend)  index.html, style.css, app.js (frontend)
