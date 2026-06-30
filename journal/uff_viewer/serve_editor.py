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


def _resolve_bundle_dir() -> str:
    """Directory baked into .app Contents/Resources/editor/ at build time."""
    env_bundle = os.environ.get("PD_EDITOR_BUNDLE_DIR", "").strip()
    if env_bundle and os.path.isdir(env_bundle):
        return os.path.abspath(env_bundle)
    # serve_editor.py colocated with uff_map.html inside Resources/editor/
    if os.path.isfile(os.path.join(HERE, "uff_map.html")):
        return HERE
    return ""


BUNDLE_DIR = _resolve_bundle_dir()
# Static UI always from bundle when present; repo journal/uff_viewer is not required for HTML.
STATIC_DIR = BUNDLE_DIR or HERE


def _writable_state_dir() -> str:
    """Persist port/pid/last-test artifacts; honor PD_EDITOR_STATE_DIR when set."""
    env_state = os.environ.get("PD_EDITOR_STATE_DIR", "").strip()
    if env_state:
        os.makedirs(env_state, exist_ok=True)
        return env_state

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


def _resolve_maps_dir() -> str:
    """Persist maps under repo when writable; otherwise Application Support."""
    repo_maps = os.path.join(ROOT, "journal", "uff_viewer", "maps")
    try:
        os.makedirs(repo_maps, exist_ok=True)
        probe = os.path.join(repo_maps, ".pd_editor_maps_probe")
        with open(probe, "w", encoding="utf-8") as fp:
            fp.write("ok")
        os.remove(probe)
        return repo_maps
    except OSError:
        pass
    fallback = os.path.join(STATE_DIR, "maps")
    os.makedirs(fallback, exist_ok=True)
    return fallback


MAPS_DIR = _resolve_maps_dir()

if ROOT not in sys.path:
    sys.path.insert(0, ROOT)
if HERE not in sys.path:
    sys.path.insert(0, HERE)

from test_map import (  # noqa: E402
    DEFAULT_LOADOUT,
    TEST_MAP_SLOT,
    _detect_pd_binary,
    build_shell_script,
    level_is_box_arena,
    play_command,
)

LOG_FILE = os.path.expanduser("~/Library/Logs/PerfectDarkMapEditor.log")

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
    _cors_headers(handler)
    handler.send_header("Content-Type", "application/json; charset=utf-8")
    handler.send_header("Content-Length", str(len(body)))
    handler.end_headers()
    handler.wfile.write(body)


def _cors_headers(handler: BaseHTTPRequestHandler) -> None:
    origin = handler.headers.get("Origin")
    if _localhost_origin(origin):
        handler.send_header("Access-Control-Allow-Origin", origin or "http://127.0.0.1")
        handler.send_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS")
        handler.send_header("Access-Control-Allow-Headers", "Content-Type")
        handler.send_header("Vary", "Origin")


def _sanitize_map_name(raw: str) -> str | None:
    """Map filenames must match editor level-name rules (lowercase alnum + underscore)."""
    name = (raw or "").strip().lower()
    if not name or len(name) > 32:
        return None
    if not name[0].isalpha():
        return None
    for ch in name:
        if not (ch.isalnum() or ch == "_"):
            return None
    return name


def _map_file_path(name: str) -> str:
    return os.path.join(MAPS_DIR, f"{name}.json")


def _ensure_maps_dir() -> None:
    os.makedirs(MAPS_DIR, exist_ok=True)


def _starter_map(name: str) -> dict[str, Any]:
    """Default box + corner spawns + a few pickups for a new arena."""
    half = 2500.0
    y = 10.0
    d = 2000.0
    pads: list[dict[str, Any]] = [
        {"index": 0, "type": "spawn", "x": -d, "y": y, "z": -d, "room": 1},
        {"index": 1, "type": "spawn", "x": d, "y": y, "z": -d, "room": 1},
        {"index": 2, "type": "spawn", "x": -d, "y": y, "z": d, "room": 1},
        {"index": 3, "type": "spawn", "x": d, "y": y, "z": d, "room": 1},
        {"index": 4, "type": "weapon", "x": 0, "y": y, "z": -1500, "room": 1, "weapon": 0x11, "weaponName": "AR34"},
        {"index": 5, "type": "weapon", "x": 0, "y": y, "z": 1500, "room": 1, "weapon": 0x13, "weaponName": "Shotgun"},
        {"index": 6, "type": "ammo", "x": 0, "y": y, "z": 0, "room": 1, "ammoType": 0x04, "ammoName": "Shotgun", "quantity": 200},
        {"index": 7, "type": "scenario", "x": 0, "y": y, "z": -500, "room": 1, "scenario": "case", "team": 0},
    ]
    return {"name": name, "box_half": half, "box_height": 2000.0, "pads": pads}


def _read_map_file(name: str) -> dict[str, Any] | None:
    path = _map_file_path(name)
    if not os.path.isfile(path):
        return None
    with open(path, encoding="utf-8") as fp:
        data = json.load(fp)
    if not isinstance(data, dict):
        return None
    data.setdefault("name", name)
    return data


def _write_map_file(name: str, data: dict[str, Any]) -> None:
    _ensure_maps_dir()
    payload = dict(data)
    payload["name"] = name
    path = _map_file_path(name)
    with open(path, "w", encoding="utf-8") as fp:
        json.dump(payload, fp, indent=2)
        fp.write("\n")


def _delete_map_file(name: str) -> bool:
    path = _map_file_path(name)
    if not os.path.isfile(path):
        return False
    os.remove(path)
    return True


def _list_maps() -> list[dict[str, Any]]:
    _ensure_maps_dir()
    entries: list[dict[str, Any]] = []
    for fname in sorted(os.listdir(MAPS_DIR)):
        if not fname.endswith(".json"):
            continue
        name = fname[:-5]
        path = os.path.join(MAPS_DIR, fname)
        try:
            st = os.stat(path)
            with open(path, encoding="utf-8") as fp:
                data = json.load(fp)
            pad_count = len(data.get("pads", [])) if isinstance(data, dict) else 0
        except (OSError, json.JSONDecodeError):
            pad_count = 0
            st = os.stat(path)
        entries.append(
            {
                "name": name,
                "updatedAt": int(st.st_mtime),
                "padCount": pad_count,
                "source": "file",
            }
        )
    return entries


def _maps_api_get(handler: BaseHTTPRequestHandler, path: str) -> bool:
    """Handle GET /api/maps and GET /api/maps/<name>. Return True if handled."""
    if path == "/api/maps":
        _json_response(handler, 200, {"ok": True, "maps": _list_maps(), "mapsDir": MAPS_DIR})
        return True
    if path.startswith("/api/maps/"):
        name = _sanitize_map_name(path[len("/api/maps/") :])
        if not name:
            _json_response(handler, 400, {"ok": False, "error": "invalid_map_name"})
            return True
        data = _read_map_file(name)
        if data is None:
            _json_response(handler, 404, {"ok": False, "error": "not_found"})
            return True
        _json_response(handler, 200, {"ok": True, "map": data})
        return True
    return False


def _maps_api_post(handler: BaseHTTPRequestHandler, payload: dict[str, Any]) -> None:
    """Create a new map JSON file (409 if it already exists)."""
    raw_name = payload.get("name")
    if not isinstance(raw_name, str):
        _json_response(handler, 400, {"ok": False, "error": "missing_name"})
        return
    name = _sanitize_map_name(raw_name)
    if not name:
        _json_response(handler, 400, {"ok": False, "error": "invalid_map_name"})
        return
    if os.path.isfile(_map_file_path(name)):
        _json_response(handler, 409, {"ok": False, "error": "already_exists", "name": name})
        return
    incoming = payload.get("map")
    if isinstance(incoming, dict) and isinstance(incoming.get("pads"), list):
        data = dict(incoming)
        data["name"] = name
    else:
        data = _starter_map(name)
    _write_map_file(name, data)
    _json_response(handler, 201, {"ok": True, "name": name, "map": data})


def _maps_api_put(handler: BaseHTTPRequestHandler, name: str, payload: dict[str, Any]) -> None:
    """Save (create or overwrite) a map JSON file."""
    incoming = payload.get("map")
    if not isinstance(incoming, dict) or not isinstance(incoming.get("pads"), list):
        _json_response(handler, 400, {"ok": False, "error": "invalid_map"})
        return
    data = dict(incoming)
    data["name"] = name
    _write_map_file(name, data)
    _json_response(handler, 200, {"ok": True, "name": name, "map": data})


def _maps_api_delete(handler: BaseHTTPRequestHandler, name: str) -> None:
    if not _delete_map_file(name):
        _json_response(handler, 404, {"ok": False, "error": "not_found"})
        return
    _json_response(handler, 200, {"ok": True, "name": name})


def _play_params_from_opts(opts: dict[str, Any]) -> dict[str, Any]:
    """Extract --test-map launch knobs from editor options payload."""
    loadout = opts.get("loadout")
    if isinstance(loadout, list):
        loadout = [int(w) for w in loadout]
    else:
        loadout = None
    return {
        "num_sims": int(opts.get("numSims", 8)),
        "sim_difficulty": int(opts.get("simDifficulty", 2)),
        "loadout": loadout,
        "mp_options": int(opts.get("mpOptions", 0)),
    }

def _build_test_map_argv(json_path: str, opts: dict[str, Any], *, play: bool) -> list[str]:
    """Mirror ``buildTestCommandLine`` / ``test_map.py`` flags from editor options."""
    level = str(opts.get("level", "uff")).strip().lower()
    deploy_as = str(opts.get("deployAs", level)).strip().lower()
    # Box arenas must always rebuild seg; never honor Seg=off from the UI.
    force_seg = level_is_box_arena(deploy_as)
    build_seg = force_seg or bool(opts.get("seg", True))
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
    if build_seg:
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
    play_params = _play_params_from_opts(opts)
    cmd.extend(["--num-sims", str(play_params["num_sims"])])
    cmd.extend(["--sim-difficulty", str(play_params["sim_difficulty"])])
    if play_params["loadout"]:
        cmd.extend(["--loadout", ",".join(str(w) for w in play_params["loadout"])])
    if play_params["mp_options"]:
        cmd.extend(["--mp-options", str(play_params["mp_options"])])
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

    build_argv = _build_test_map_argv(json_path, opts, play=False)
    build_cmd_str = " ".join(build_argv)
    full_argv = _build_test_map_argv(json_path, opts, play=want_play)
    full_cmd_str = " ".join(full_argv)

    # Dry-run: return planned commands without shell artifacts or subprocess work.
    if opts.get("dryRun"):
        play_argv = play_command(
            mod_key=str(opts.get("mod", "mod_allinone")),
            scenario=int(opts.get("scenario", 0)),
            pd_binary=pd_binary,
            use_test_map=use_test_map and want_play,
            **_play_params_from_opts(opts),
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

    # Build shell script artifact (no-op for API, but keeps .last_test.sh in sync).
    import argparse as _argparse

    play_params = _play_params_from_opts(opts)
    loadout = play_params["loadout"] if play_params["loadout"] else list(DEFAULT_LOADOUT)
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
        num_sims=play_params["num_sims"],
        sim_difficulty=play_params["sim_difficulty"],
        loadout=loadout,
        mp_options=play_params["mp_options"],
    )
    sh_path = os.path.join(STATE_DIR, ".last_test.sh")
    try:
        with open(sh_path, "w", encoding="utf-8") as fp:
            fp.write(build_shell_script(ns, map_data))
        os.chmod(sh_path, 0o755)
    except Exception as exc:
        _log_traceback(f"build_shell_script failed: {exc}")

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
        **_play_params_from_opts(opts),
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
                "bundleDir": BUNDLE_DIR or None,
                "staticDir": STATIC_DIR,
                "scriptDir": HERE,
                "binaryFound": os.path.isfile(pd_binary),
                "binaryPath": pd_binary,
                "mapsDir": MAPS_DIR,
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

        if _maps_api_get(self, path):
            return

        # Static files from bundled editor/ (or journal/uff_viewer/ in dev).
        if path in ("/", "/index.html"):
            rel = "uff_map.html"
        else:
            rel = path.lstrip("/")
            if ".." in rel or rel.startswith("/"):
                self.send_error(403)
                return

        file_path = os.path.join(STATIC_DIR, rel)
        if not os.path.isfile(file_path):
            self.send_error(404, f"Not found: {rel}")
            return

        content_type = "application/octet-stream"
        cache_control = "public, max-age=3600"
        if rel.endswith(".html"):
            content_type = "text/html; charset=utf-8"
            # Bundled HTML is versioned at build time; avoid stale Electron cache.
            cache_control = "no-cache, no-store, must-revalidate"
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
        self.send_header("Cache-Control", cache_control)
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def do_POST(self) -> None:  # noqa: N802
        if self._check_cors_preflight():
            return
        if self._reject_origin():
            return

        path = urlparse(self.path).path
        if path == "/api/test-map":
            length = int(self.headers.get("Content-Length", "0"))
            raw = self.rfile.read(length) if length else b""
            try:
                payload = json.loads(raw.decode("utf-8") if raw else "{}")
            except json.JSONDecodeError as exc:
                _json_response(self, 400, {"ok": False, "error": f"invalid_json: {exc}"})
                return

            try:
                result = _run_test_map(payload)
            except Exception as exc:
                _log_traceback(f"POST /api/test-map failed: {exc}")
                _json_response(self, 500, {"ok": False, "error": str(exc)})
                return

            status = 200 if result.get("ok") else 500
            self.send_response(status)
            _cors_headers(self)
            self.send_header("Content-Type", "application/json; charset=utf-8")
            self.end_headers()
            self.wfile.write(json.dumps(result, indent=2).encode("utf-8"))
            return

        if path == "/api/maps":
            length = int(self.headers.get("Content-Length", "0"))
            raw = self.rfile.read(length) if length else b""
            try:
                payload = json.loads(raw.decode("utf-8") if raw else "{}")
            except json.JSONDecodeError as exc:
                _json_response(self, 400, {"ok": False, "error": f"invalid_json: {exc}"})
                return
            if not isinstance(payload, dict):
                _json_response(self, 400, {"ok": False, "error": "invalid_body"})
                return
            try:
                _maps_api_post(self, payload)
            except Exception as exc:
                _log_traceback(f"POST /api/maps failed: {exc}")
                _json_response(self, 500, {"ok": False, "error": str(exc)})
            return

        self.send_error(404)

    def do_PUT(self) -> None:  # noqa: N802
        if self._check_cors_preflight():
            return
        if self._reject_origin():
            return

        path = urlparse(self.path).path
        if not path.startswith("/api/maps/"):
            self.send_error(404)
            return
        name = _sanitize_map_name(path[len("/api/maps/") :])
        if not name:
            _json_response(self, 400, {"ok": False, "error": "invalid_map_name"})
            return

        length = int(self.headers.get("Content-Length", "0"))
        raw = self.rfile.read(length) if length else b""
        try:
            payload = json.loads(raw.decode("utf-8") if raw else "{}")
        except json.JSONDecodeError as exc:
            _json_response(self, 400, {"ok": False, "error": f"invalid_json: {exc}"})
            return
        if not isinstance(payload, dict):
            _json_response(self, 400, {"ok": False, "error": "invalid_body"})
            return
        try:
            _maps_api_put(self, name, payload)
        except Exception as exc:
            _log_traceback(f"PUT /api/maps/{name} failed: {exc}")
            _json_response(self, 500, {"ok": False, "error": str(exc)})

    def do_DELETE(self) -> None:  # noqa: N802
        if self._check_cors_preflight():
            return
        if self._reject_origin():
            return

        path = urlparse(self.path).path
        if not path.startswith("/api/maps/"):
            self.send_error(404)
            return
        name = _sanitize_map_name(path[len("/api/maps/") :])
        if not name:
            _json_response(self, 400, {"ok": False, "error": "invalid_map_name"})
            return
        try:
            _maps_api_delete(self, name)
        except Exception as exc:
            _log_traceback(f"DELETE /api/maps/{name} failed: {exc}")
            _json_response(self, 500, {"ok": False, "error": str(exc)})


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


def _log_traceback(message: str) -> None:
    """Append a traceback to the shared editor log for Electron/shell diagnostics."""
    import traceback
    from datetime import datetime

    stamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    body = traceback.format_exc()
    line = f"[{stamp}] [serve_editor] {message}\n{body}"
    print(line, file=sys.stderr, end="" if line.endswith("\n") else "\n")
    try:
        os.makedirs(os.path.dirname(LOG_FILE), exist_ok=True)
        with open(LOG_FILE, "a", encoding="utf-8") as fp:
            fp.write(line)
            if not line.endswith("\n"):
                fp.write("\n")
    except OSError:
        pass


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

    os.chdir(STATIC_DIR)
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
    print("  GET  /api/maps     — list saved maps (journal/uff_viewer/maps/)")
    print("  POST /api/maps     — create map (starter template if body.map omitted)")
    print("  PUT  /api/maps/<n> — save map JSON")
    print("  DELETE /api/maps/<n> — delete saved map")
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
    try:
        raise SystemExit(main())
    except SystemExit:
        raise
    except Exception as exc:
        _log_traceback(f"Fatal error in serve_editor.py: {exc}")
        raise SystemExit(1) from exc
