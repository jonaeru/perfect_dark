#!/usr/bin/env python3
"""Shared port discovery for the uff map editor server (8765–8775).

Used by serve_editor.py, validation scripts, and tooling that must not assume
the default port is always free.
"""

from __future__ import annotations

import json
import os
import socket
import urllib.error
import urllib.request

DEFAULT_HOST = "127.0.0.1"
DEFAULT_PORT = 8765
PORT_MIN = DEFAULT_PORT
PORT_MAX = 8775

HERE = os.path.dirname(os.path.abspath(__file__))


def _repo_root() -> str:
    env_root = os.environ.get("PD_REPO_ROOT", "").strip()
    if env_root:
        return os.path.abspath(env_root)
    return os.path.dirname(os.path.dirname(HERE))


def resolve_state_dir() -> str:
    """Match serve_editor.py state directory resolution."""
    env_state = os.environ.get("PD_EDITOR_STATE_DIR", "").strip()
    if env_state:
        os.makedirs(env_state, exist_ok=True)
        return env_state

    root = _repo_root()
    repo_viewer = os.path.join(root, "journal", "uff_viewer")
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


def port_file_path(state_dir: str | None = None) -> str:
    return os.path.join(state_dir or resolve_state_dir(), ".editor_server.port")


def read_port_file(state_dir: str | None = None) -> int | None:
    path = port_file_path(state_dir)
    try:
        with open(path, encoding="utf-8") as fp:
            raw = fp.read().strip()
        port = int(raw)
        return port if PORT_MIN <= port <= PORT_MAX else None
    except (OSError, ValueError):
        return None


def fetch_health(host: str, port: int, *, timeout: float = 2.0) -> dict | None:
    url = f"http://{host}:{port}/api/health"
    try:
        with urllib.request.urlopen(url, timeout=timeout) as resp:
            payload = json.loads(resp.read().decode("utf-8"))
        return payload if isinstance(payload, dict) else None
    except (urllib.error.URLError, urllib.error.HTTPError, TimeoutError, json.JSONDecodeError, OSError):
        return None


def is_editor_health(health: dict | None) -> bool:
    return bool(
        health
        and health.get("ok") is True
        and health.get("service") == "uff-editor"
    )


def bundle_dirs_match(
    health: dict,
    expected_bundle_dir: str,
    *,
    script_dir: str = HERE,
) -> bool:
    """True when an existing server serves the same bundle or dev tree."""
    if not is_editor_health(health):
        return False
    expected = (expected_bundle_dir or "").strip()
    if expected:
        return (health.get("bundleDir") or "") == expected
    health_script = health.get("scriptDir") or ""
    health_static = health.get("staticDir") or ""
    return health_script == script_dir or health_static == script_dir


def port_bindable(host: str, port: int) -> bool:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        try:
            sock.bind((host, port))
            return True
        except OSError:
            return False


def find_healthy_port(
    host: str = DEFAULT_HOST,
    min_port: int = PORT_MIN,
    max_port: int = PORT_MAX,
    *,
    expected_bundle_dir: str = "",
    script_dir: str = HERE,
) -> int | None:
    """First port in range with a healthy editor whose bundleDir matches."""
    for port in range(min_port, max_port + 1):
        health = fetch_health(host, port)
        if bundle_dirs_match(health or {}, expected_bundle_dir, script_dir=script_dir):
            return port
    return None


def discover_editor_port(
    host: str = DEFAULT_HOST,
    *,
    min_port: int = PORT_MIN,
    max_port: int = PORT_MAX,
    expected_bundle_dir: str = "",
    script_dir: str = HERE,
    state_dir: str | None = None,
) -> int | None:
    """Resolve a live editor port via PORT_FILE, then health scan."""
    from_file = read_port_file(state_dir)
    if from_file is not None:
        health = fetch_health(host, from_file)
        if is_editor_health(health):
            return from_file

    matched = find_healthy_port(
        host,
        min_port,
        max_port,
        expected_bundle_dir=expected_bundle_dir,
        script_dir=script_dir,
    )
    if matched is not None:
        return matched

    # Last resort: any healthy uff-editor in range (validation / dev).
    for port in range(min_port, max_port + 1):
        health = fetch_health(host, port)
        if is_editor_health(health):
            return port
    return None


def discover_editor_base(
    host: str = DEFAULT_HOST,
    *,
    min_port: int = PORT_MIN,
    max_port: int = PORT_MAX,
    expected_bundle_dir: str = "",
    script_dir: str = HERE,
    state_dir: str | None = None,
) -> str | None:
    port = discover_editor_port(
        host,
        min_port=min_port,
        max_port=max_port,
        expected_bundle_dir=expected_bundle_dir,
        script_dir=script_dir,
        state_dir=state_dir,
    )
    if port is None:
        return None
    return f"http://{host}:{port}"


def resolve_startup_port(
    host: str,
    requested: int,
    *,
    expected_bundle_dir: str = "",
    script_dir: str = HERE,
    strict: bool = False,
) -> tuple[int, str]:
    """Choose bind port or reuse an existing server.

    Returns ``(port, action)`` where *action* is ``"reuse"`` or ``"bind"``.
    Raises ``OSError`` when no port is available.
    """
    if strict:
        if port_bindable(host, requested):
            return requested, "bind"
        health = fetch_health(host, requested)
        if bundle_dirs_match(health or {}, expected_bundle_dir, script_dir=script_dir):
            return requested, "reuse"
        raise OSError(f"Port {requested} is already in use on {host}")

    reused = find_healthy_port(
        host,
        PORT_MIN,
        PORT_MAX,
        expected_bundle_dir=expected_bundle_dir,
        script_dir=script_dir,
    )
    if reused is not None:
        return reused, "reuse"

    if port_bindable(host, requested):
        return requested, "bind"

    for port in range(requested + 1, PORT_MAX + 1):
        if port_bindable(host, port):
            return port, "bind"
    for port in range(PORT_MIN, requested):
        if port_bindable(host, port):
            return port, "bind"

    raise OSError(f"No free port in range {PORT_MIN}-{PORT_MAX} on {host}")
