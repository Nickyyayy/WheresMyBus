import http.server
import socketserver
import socket
import os
import sys
import json
import time
import subprocess

PORT = 8080
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
REQUEST_FILE = os.path.join(BASE_DIR, "command_request.json")
RESPONSE_FILE = os.path.join(BASE_DIR, "command_response.json")

def get_local_ip():
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        ip = s.getsockname()[0]
        s.close()
        return ip
    except Exception:
        return "127.0.0.1"

class CustomHandler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
        # Enable CORS and disable cache on APIs, dynamic JSON, and HTML pages
        self.send_header('Access-Control-Allow-Origin', '*')
        if self.path.startswith('/api/') or self.path.endswith('.json') or self.path.endswith('.html'):
            self.send_header('Cache-Control', 'no-cache, no-store, must-revalidate')
            self.send_header('Pragma', 'no-cache')
            self.send_header('Expires', '0')
        super().end_headers()

    def do_OPTIONS(self):
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()

    def do_GET(self):
        # Redirect root to /output/index.html
        if self.path == '/' or self.path == '':
            self.send_response(302)
            self.send_header('Location', '/output/index.html')
            self.end_headers()
            return

        # API endpoint to retrieve live system state
        if self.path == '/api/status' or self.path.startswith('/api/status?'):
            state_file = os.path.join(BASE_DIR, "output", "state.json")
            if os.path.exists(state_file):
                try:
                    with open(state_file, 'r', encoding='utf-8') as f:
                        data = f.read()
                    self.send_response(200)
                    self.send_header('Content-Type', 'application/json; charset=utf-8')
                    self.end_headers()
                    self.wfile.write(data.encode('utf-8'))
                    return
                except Exception:
                    pass
            self.send_response(200)
            self.send_header('Content-Type', 'application/json; charset=utf-8')
            self.end_headers()
            self.wfile.write(b'{"revision": 0, "status": "initializing"}')
            return

        super().do_GET()

    def do_POST(self):
        if self.path == '/api/action':
            try:
                content_len = int(self.headers.get('Content-Length', 0))
                body = self.rfile.read(content_len).decode('utf-8')
                payload = json.loads(body)
                bus_id = str(payload.get('busId', '')).strip()
                action = str(payload.get('action', '')).strip()

                if not bus_id or not action:
                    self.send_response(400)
                    self.send_header('Content-Type', 'application/json; charset=utf-8')
                    self.end_headers()
                    self.wfile.write(json.dumps({"ok": False, "message": "Missing busId or action parameter."}).encode('utf-8'))
                    return

                # Make sure backend is running
                check_and_launch_backend()

                # Clean prior response file
                if os.path.exists(RESPONSE_FILE):
                    try: os.remove(RESPONSE_FILE)
                    except: pass

                # Atomically write command_request.json
                req_tmp = REQUEST_FILE + ".tmp"
                with open(req_tmp, 'w', encoding='utf-8') as f:
                    json.dump({"busId": bus_id, "action": action}, f)
                os.replace(req_tmp, REQUEST_FILE)

                # Wait for backend engine (wheresmybus.exe) response
                start_time = time.time()
                timeout = 4.0  # seconds
                response_data = None

                while time.time() - start_time < timeout:
                    if os.path.exists(RESPONSE_FILE):
                        try:
                            with open(RESPONSE_FILE, 'r', encoding='utf-8') as f:
                                response_data = f.read()
                            os.remove(RESPONSE_FILE)
                            break
                        except (IOError, PermissionError):
                            pass
                    time.sleep(0.02)

                if response_data:
                    self.send_response(200)
                    self.send_header('Content-Type', 'application/json; charset=utf-8')
                    self.end_headers()
                    self.wfile.write(response_data.encode('utf-8'))
                else:
                    self.send_response(504)
                    self.send_header('Content-Type', 'application/json; charset=utf-8')
                    self.end_headers()
                    self.wfile.write(json.dumps({
                        "ok": False,
                        "message": "Backend engine did not respond in time. Please make sure wheresmybus.exe is running."
                    }).encode('utf-8'))

            except Exception as e:
                self.send_response(500)
                self.send_header('Content-Type', 'application/json; charset=utf-8')
                self.end_headers()
                self.wfile.write(json.dumps({"ok": False, "message": f"Server error: {str(e)}"}).encode('utf-8'))
            return

        self.send_response(404)
        self.end_headers()

def check_and_launch_backend():
    exe_name = "wheresmybus.exe"
    exe_path = os.path.join(BASE_DIR, exe_name)
    if not os.path.exists(exe_path):
        return
    is_running = False
    try:
        res = subprocess.run(
            ["powershell", "-NoProfile", "-Command", f"Get-Process -Name {exe_name[:-4]} -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Id"],
            capture_output=True, text=True, timeout=3
        )
        if res.stdout.strip():
            is_running = True
    except Exception:
        pass

    if not is_running:
        try:
            print(f"Starting backend ({exe_name})...")
            creation_flags = subprocess.CREATE_NEW_CONSOLE if sys.platform == 'win32' else 0
            subprocess.Popen([exe_path], cwd=BASE_DIR, creationflags=creation_flags)
            time.sleep(2.0)
        except Exception as e:
            print("Could not launch backend process:", e)

if __name__ == '__main__':
    # Ensure working directory is project root
    os.chdir(BASE_DIR)
    
    # Launch backend if not active
    check_and_launch_backend()

    local_ip = get_local_ip()
    print("=" * 60)
    print("Where's My Bus Server")
    print("=" * 60)
    print(f"Port:    {PORT}")
    print(f"Local:   http://localhost:{PORT}")
    print(f"Network: http://{local_ip}:{PORT}")
    print("=" * 60)
    print("Press Ctrl+C to stop.\n")

    socketserver.TCPServer.allow_reuse_address = True
    with socketserver.TCPServer(("", PORT), CustomHandler) as httpd:
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nWeb server stopped.")
