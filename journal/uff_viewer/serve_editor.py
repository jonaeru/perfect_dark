#!/usr/bin/env python3
"""Local dev server for the uff map editor — static files + test-map API.

Browsers cannot spawn ``./build/pd.arm64`` directly; this helper accepts editor
JSON over HTTP and runs ``test_map.py`` on the host machine.

Usage (from repo root):
  python3 journal/uff_viewer/serve_editor.py
  python3 journal/uff_viewer/serve_editor.py --port 8765

Then open http://127.0.0.1:8765/ and use **Test / Play** in the editor panel.
"""

from __future__ import annotations

import argparse
import json
import os
import shlex
import socket
import subprocess
import sys
import threading
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from typing import Any
from urllib.parse import urlparse

HERE = os.path.dirname(os.path.abspath(__file__))
# Bundled .app copies set PD_REPO_ROOT; default infers repo from journal/uff_viewer layout.
_env_root = os.environ.get("PD_REPO_ROOT", "").strip()
ROOT = os.path.abspath(_env_root) if _env_root else os.path.dirname(os.path.dirname(HERE))
DEFAULT_PORT = 8765


def _writable_state_dir() -> str:
    """Persist port/pid/last-test artifacts in repo when possible, else Application Support."""
    repo_viewer = os.path.join(ROOT, "journal", "uff_viewer")
    if os.path.isdir(repo_viewer):
        try:
            probe = os.path.join(repo_viewer, ".pd_editor_write_probe")
            with open(probe, "w", encoding="utf-8") as fp:
                fp.write("ok")
            os.remove(probe)
            return repo_viewer
        except OSError:
            pass
    fallback = os.path.join(
        os.path.expanduser("~/Library/Application Support"),
        "PerfectDarkMapEditor",
    )
    os.makedirs(fallback, exist_ok=True)
    return fallback


STATE_DIR = _writable_state_dir()
PORT_FILE = os.path.join(STATE_DIR, ".editor_server.port")

if ROOT not in sys.path:
    sys.path.insert(0, ROOT)
if HERE not in sys.path:
    sys.path.insert(0, HERE)

from test_map import (  # noqa: E402
    TEST_MAP_SLOT,
    _detect_pd_binary,
    build_shell_script,
    play_command,
)

# Track detached game processes (pid -> popen) for optional /api/status polling.
_GAME_PROCS: dict[int, subprocess.Popen[Any]] = {}
_GAME_LOCK = threading.Lock()


def _localhost_origin(origin: str | None) -> bool:
    """Allow same-origin requests and localhost browser origins only."""
    if not origin:
        return True
    host = urlparse(origin).hostname
    return host in ("localhost", "127.0.0.1", "::1")


def _json_response(handler: BaseHTTPRequestHandler, status: int, payload: dict[str, Any]) -> None:
    body = json.dumps(payload, indent=2).encode("utf-8")
    handler.send_response(status)
    handler.send_header("Content-Type", "application/json; charset=utf-8")
    handler.send_header("Content-Length", str(len(body)))
    handler.end_headers()
    handler.wfile.write(body)


def _cors_headers(handler: BaseHTTPRequestHandler) -> None:
    origin = handler.headers.get("Origin")
    if _localhost_origin(origin):
        handler.send_header("Access-Control-Allow-Origin", origin or "http://127.0.0.1")
        handler.send_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
        handler.send_header("Access-Control-Allow-Headers", "Content-Type")
        handler.send_header("Vary", "Origin")


def _build_test_map_argv(json_path: str, opts: dict[str, Any], *, play: bool) -> list[str]:
    """Mirror ``buildTestCommandLine`` / ``test_map.py`` flags from editor options."""
    level = str(opts.get("level", "uff")).strip().lower()
    deploy_as = str(opts.get("deployAs", level)).strip().lower()
    cmd = [
        sys.executable,
        os.path.join(HERE, "test_map.py"),
        json_path,
        "--level",
        level,
        "--mod",
        str(opts.get("mod", "mod_allinone")),
        "--scenario",
        str(int(opts.get("scenario", 0))),
        "--write-artifacts",
    ]
    if deploy_as != level:
        cmd.extend(["--deploy-as", deploy_as])
    if opts.get("seg", True):
        cmd.append("--seg")
    else:
        cmd.append("--no-seg")
    if opts.get("deploy", True):
        cmd.append("--deploy")
    else:
        cmd.append("--no-deploy")
    if opts.get("skipValidate"):
        cmd.append("--skip-validate")
    if opts.get("rebuildGame"):
        cmd.append("--rebuild-game")
    if opts.get("backup", True):
        cmd.append("--backup")
    else:
        cmd.append("--no-backup")
    if play:
        cmd.append("--play")
    return cmd


def _run_test_map(payload: dict[str, Any]) -> dict[str, Any]:
    """Execute build (+ optional detached play) and return API JSON."""
    map_data = payload.get("map")
    if not isinstance(map_data, dict):
        return {"ok": False, "error": "Missing or invalid 'map' object in request body."}

    opts = payload.get("options") if isinstance(payload.get("options"), dict) else {}
    level = str(opts.get("level") or map_data.get("name") or "uff").strip().lower()
    deploy_as = str(opts.get("deployAs") or level).strip().lower()
    want_play = bool(opts.get("play", True))

    map_data = dict(map_data)
    map_data["name"] = level
    # Box geometry from editor sliders (also embedded in serializeMap JSON).
    if "half" in opts and opts["half"] is not None:
        map_data["box_half"] = float(opts["half"])
    if "height" in opts and opts["height"] is not None:
        map_data["box_height"] = float(opts["height"])
    opts = dict(opts)
    opts["level"] = level
    opts["deployAs"] = deploy_as

    pd_binary = _detect_pd_binary()
    use_test_map = deploy_as == TEST_MAP_SLOT

    # Persist beside the viewer (or Application Support when repo viewer is evicted).
    json_path = os.path.join(STATE_DIR, ".last_test.json")
    with open(json_path, "w", encoding="utf-8") as fp:
        json.dump(map_data, fp, indent=2)

    # Build shell script artifact (no-op for API, but keeps .last_test.sh in sync).
    import argparse as _argparse

    ns = _argparse.Namespace(
        level=level,
        mod=str(opts.get("mod", "mod_allinone")),
        scenario=int(opts.get("scenario", 0)),
        seg=bool(opts.get("seg", True)),
        deploy=bool(opts.get("deploy", True)),
        skip_validate=bool(opts.get("skipValidate")),
        rebuild_game=bool(opts.get("rebuildGame")),
        play=want_play,
        verbose=False,
        backup=bool(opts.get("backup", True)),
        deploy_as=deploy_as,
    )
    sh_path = os.path.join(STATE_DIR, ".last_test.sh")
    with open(sh_path, "w", encoding="utf-8") as fp:
        fp.write(build_shell_script(ns, map_data))
    os.chmod(sh_path, 0o755)

    build_argv = _build_test_map_argv(json_path, opts, play=False)
    build_cmd_str = " ".join(build_argv)
    full_argv = _build_test_map_argv(json_path, opts, play=want_play)
    full_cmd_str = " ".join(full_argv)

    # Dry-run: return planned commands without writing level modules or building.
    if opts.get("dryRun"):
        play_argv = play_command(
            mod_key=str(opts.get("mod", "mod_allinone")),
            scenario=int(opts.get("scenario", 0)),
            pd_binary=pd_binary,
            use_test_map=use_test_map and want_play,
        )
        return {
            "ok": True,
            "dryRun": True,
            "command": build_cmd_str,
            "fullCommand": full_cmd_str,
            "playCommand": " ".join(play_argv),
            "options": opts,
            "pid": None,
        }

    if not os.path.isfile(pd_binary) and want_play:
        return {
            "ok": False,
            "command": build_cmd_str,
            "stdout": "",
            "stderr": f"Game binary not found at {pd_binary}. Build first: cmake --build build --target pd",
            "pid": None,
            "error": "missing_binary",
        }

    try:
        proc = subprocess.run(
            build_argv,
            cwd=ROOT,
            capture_output=True,
            text=True,
            check=False,
        )
    except OSError as exc:
        return {
            "ok": False,
            "command": build_cmd_str,
            "stdout": "",
            "stderr": str(exc),
            "pid": None,
            "error": "subprocess_failed",
        }

    result: dict[str, Any] = {
        "ok": proc.returncode == 0,
        "command": build_cmd_str,
        "fullCommand": full_cmd_str,
        "stdout": proc.stdout,
        "stderr": proc.stderr,
        "pid": None,
        "returncode": proc.returncode,
    }

    if proc.returncode != 0:
        result["error"] = "build_failed"
        return result

    if not want_play:
        return result

    play_argv = play_command(
        mod_key=str(opts.get("mod", "mod_allinone")),
        scenario=int(opts.get("scenario", 0)),
        pd_binary=pd_binary,
        use_test_map=use_test_map,
    )
    result["playCommand"] = " ".join(play_argv)

    # Replay shortcut for Play Last Test Map.app (launch only, no rebuild).
    play_sh = os.path.join(STATE_DIR, ".last_play.sh")
    with open(play_sh, "w", encoding="utf-8") as fp:
        fp.write("#!/bin/bash\n# Generated by serve_editor — replay last Test & Play launch.\n")
        fp.write("set -euo pipefail\n")
        fp.write(f'cd "{ROOT}"\n')
        fp.write("exec " + " ".join(shlex.quote(arg) for arg in play_argv) + "\n")
    os.chmod(play_sh, 0o755)

    try:
        game = subprocess.Popen(
            play_argv,
            cwd=ROOT,
            start_new_session=True,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
    except OSError as exc:
        result["ok"] = False
        result["error"] = "launch_failed"
        result["stderr"] = (result.get("stderr") or "") + f"\nLaunch failed: {exc}"
        return result

    with _GAME_LOCK:
        _GAME_PROCS[game.pid] = game

    result["pid"] = game.pid
    result["ok"] = True
    return result


class EditorHandler(BaseHTTPRequestHandler):
    """Serve ``uff_map.html`` and editor API routes."""

    def log_message(self, fmt: str, *args: Any) -> None:
        sys.stderr.write("[serve_editor] " + (fmt % args) + "\n")

    def _check_cors_preflight(self) -> bool:
        if self.command == "OPTIONS":
            self.send_response(204)
            _cors_headers(self)
            self.end_headers()
            return True
        return False

    def _reject_origin(self) -> bool:
        origin = self.headers.get("Origin")
        if origin and not _localhost_origin(origin):
            _json_response(self, 403, {"ok": False, "error": "origin_not_allowed"})
            return True
        return False

    def do_OPTIONS(self) -> None:  # noqa: N802
        if self._reject_origin():
            return
        self.send_response(204)
        _cors_headers(self)
        self.end_headers()

    def do_GET(self) -> None:  # noqa: N802
        if self._reject_origin():
            return

        path = urlparse(self.path).path

        if path == "/api/health":
            pd_binary = _detect_pd_binary()
            payload = {
                "ok": True,
                "service": "uff-editor",
                "port": self.server.server_port,  # type: ignore[attr-defined]
                "repoRoot": ROOT,
                "binaryFound": os.path.isfile(pd_binary),
                "binaryPath": pd_binary,
            }
            self.send_response(200)
            _cors_headers(self)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.end_headers()
            self.wfile.write(json.dumps(payload).encode("utf-8"))
            return

        if path == "/api/status":
            with _GAME_LOCK:
                alive = {
                    pid: proc.poll() is None
                    for pid, proc in list(_GAME_PROCS.items())
                }
                dead = [pid for pid, running in alive.items() if not running]
                for pid in dead:
                    _GAME_PROCS.pop(pid, None)
            payload = {"ok": True, "games": alive}
            self.send_response(200)
            _cors_headers(self)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.end_headers()
            self.wfile.write(json.dumps(payload).encode("utf-8"))
            return

        # Static files from journal/uff_viewer/
        if path in ("/", "/index.html"):
            rel = "uff_map.html"
        else:
            rel = path.lstrip("/")
            if ".." in rel or rel.startswith("/"):
                self.send_error(403)
                return

        file_path = os.path.join(HERE, rel)
        if not os.path.isfile(file_path):
            self.send_error(404, f"Not found: {rel}")
            return

        content_type = "application/octet-stream"
        if rel.endswith(".html"):
            content_type = "text/html; charset=utf-8"
        elif rel.endswith(".js"):
            content_type = "application/javascript; charset=utf-8"
        elif rel.endswith(".css"):
            content_type = "text/css; charset=utf-8"
        elif rel.endswith(".json"):
            content_type = "application/json; charset=utf-8"

        with open(file_path, "rb") as fp:
            data = fp.read()
        self.send_response(200)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def do_POST(self) -> None:  # noqa: N802
        if self._check_cors_preflight():
            return
        if self._reject_origin():
            return

        path = urlparse(self.path).path
        if path != "/api/test-map":
            self.send_error(404)
            return

        length = int(self.headers.get("Content-Length", "0"))
        raw = self.rfile.read(length) if length else b""
        try:
            payload = json.loads(raw.decode("utf-8") if raw else "{}")
        except json.JSONDecodeError as exc:
            _json_response(self, 400, {"ok": False, "error": f"invalid_json: {exc}"})
            return

        result = _run_test_map(payload)
        status = 200 if result.get("ok") else 500
        self.send_response(status)
        _cors_headers(self)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.end_headers()
        self.wfile.write(json.dumps(result, indent=2).encode("utf-8"))


def _port_bindable(host: str, port: int) -> bool:
    """Return True when ``host:port`` can be bound (port is free)."""
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        try:
            sock.bind((host, port))
            return True
        except OSError:
            return False


def _pick_port(host: str, start: int, *, max_tries: int = 20) -> int:
    """Choose the first bindable port in ``[start, start + max_tries)``."""
    for port in range(start, start + max_tries):
        if _port_bindable(host, port):
            return port
    raise OSError(f"No free port in range {start}-{start + max_tries - 1}")


def _write_port_file(port: int) -> None:
    with open(PORT_FILE, "w", encoding="utf-8") as fp:
        fp.write(str(port))


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Serve uff map editor + test-map API.")
    parser.add_argument("--host", default="127.0.0.1", help="Bind address (default: 127.0.0.1)")
    parser.add_argument("--port", type=int, default=DEFAULT_PORT, help=f"Port (default: {DEFAULT_PORT})")
    parser.add_argument(
        "--auto-port",
        action="store_true",
        help=f"If --port is busy, bind the next free port up to +19 (writes {PORT_FILE})",
    )
    args = parser.parse_args(argv)

    os.chdir(HERE)
    port = args.port
    if args.auto_port:
        port = _pick_port(args.host, args.port)
    elif not _port_bindable(args.host, port):
        print(
            f"Port {port} is already in use on {args.host}. "
            f"Stop the other process or re-run with --auto-port.",
            file=sys.stderr,
        )
        return 1

    try:
        server = ThreadingHTTPServer((args.host, port), EditorHandler)
    except OSError as exc:
        print(f"Failed to bind {args.host}:{port}: {exc}", file=sys.stderr)
        return 1

    _write_port_file(port)
    url = f"http://{args.host}:{port}/"
    print(f"uff editor server at {url}")
    print(f"PORT={port}")
    print("  GET  /api/health   — server + binary probe")
    print("  POST /api/test-map — build + launch via test_map.py")
    print("Press Ctrl+C to stop.")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nStopped.")
        return 0
    finally:
        try:
            os.remove(PORT_FILE)
        except OSError:
            pass
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
