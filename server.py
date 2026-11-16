"""
ResQroute bridge: web front end  <->  C++ backend (resqroute --json)

Run:   python server.py        then open http://localhost:8000
Needs: Python 3 only (no pip packages). Build the C++ program first with `make`.
"""
import json
import os
import re
import subprocess
import sys
import threading
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

ROOT = os.path.dirname(os.path.abspath(__file__))
EXE = os.path.join(ROOT, "resqroute.exe" if os.name == "nt" else "resqroute")
PAGE = os.path.join(ROOT, "frontend", "index.html")
PORT = 8000


class Engine:
    """Starts the C++ program once and talks to it line by line."""

    def __init__(self):
        if not os.path.exists(EXE):
            sys.exit("Build the C++ backend first: run `make` in this folder.")
        self.proc = subprocess.Popen(
            [EXE, "--json"], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
            text=True, bufsize=1)
        self.lock = threading.Lock()
        self.proc.stdout.readline()            # "ready" line
        self.next_patient_id = 1

    def send(self, command):
        with self.lock:                        # one command at a time
            self.proc.stdin.write(command + "\n")
            self.proc.stdin.flush()
            return json.loads(self.proc.stdout.readline())


engine = None


def text(value, limit=40):
    """Strip control characters so a name can never inject a second command."""
    value = re.sub(r"[\x00-\x1f\x7f]", " ", str(value)).strip()[:limit]
    if not value:
        raise ValueError("Name cannot be empty.")
    return value


def build_command(name, d):
    """Turn an API call into one line of the C++ text protocol."""
    if name == "state":
        return "STATE"
    if name == "demo":
        return "DEMO"
    if name == "reset":
        engine.next_patient_id = 1
        return "RESET"
    if name == "dispatch":
        return "DISPATCH"
    if name == "complete":
        return f"COMPLETE {int(d['ambulanceId'])}"
    if name == "road":
        return f"ROAD {int(d['from'])} {int(d['to'])} {int(d['distance'])}"
    if name == "ambulance":
        return (f"AMBULANCE {int(d['id'])} {int(d['location'])} "
                f"{1 if d.get('available', True) else 0} {text(d['driver'])}")
    if name == "hospital":
        return f"HOSPITAL {int(d['id'])} {int(d['location'])} {text(d['name'])}"
    if name == "patient":
        pid = engine.next_patient_id
        engine.next_patient_id += 1
        return (f"PATIENT {pid} {int(d['location'])} {int(d['priority'])} "
                f"{int(d.get('hospitalId', 0))} {text(d['name'])}")
    raise KeyError(name)


KNOWN = {"state", "demo", "reset", "dispatch", "complete", "road",
         "ambulance", "hospital", "patient"}


class Handler(BaseHTTPRequestHandler):
    def _json(self, code, obj):
        body = json.dumps(obj).encode()
        self.send_response(code)
        self.send_header("Content-Type", "application/json")
        self.send_header("Access-Control-Allow-Origin", "*")   # lets a React/other front end call it
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_OPTIONS(self):
        self.send_response(204)
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")
        self.send_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
        self.end_headers()

    def do_GET(self):
        if self.path in ("/", "/index.html"):
            with open(PAGE, "rb") as f:
                body = f.read()
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
        elif self.path == "/api/state":
            self._json(200, engine.send("STATE"))
        else:
            self._json(404, {"ok": False, "message": "Not found"})

    def do_POST(self):
        if not self.path.startswith("/api/"):
            return self._json(404, {"ok": False, "message": "Not found"})
        try:
            length = int(self.headers.get("Content-Length", 0))
            data = json.loads(self.rfile.read(length) or b"{}")
            name = self.path[5:]
            if name not in KNOWN:
                return self._json(404, {"ok": False, "message": "Unknown API call"})
            command = build_command(name, data)
            self._json(200, engine.send(command))
        except (KeyError, ValueError, TypeError, json.JSONDecodeError):
            self._json(400, {"ok": False, "message": "Invalid or missing fields."})

    def log_message(self, *args):
        pass


if __name__ == "__main__":
    engine = Engine()
    print(f"ResQroute running at http://localhost:{PORT}  (Ctrl+C to stop)")
    try:
        ThreadingHTTPServer(("127.0.0.1", PORT), Handler).serve_forever()
    except KeyboardInterrupt:
        engine.proc.terminate()
