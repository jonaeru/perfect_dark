#!/usr/bin/env python3
"""Pass-11: Edit → Save → Play — one reliable path (Playwright + API).

Requires serve_editor.py on ports 8765–8775 serving journal/uff_viewer/ (bundled uff_map.html).

Flow: blank new map → edit mode → canvas spawn click → Save (PUT) → POST /api/test-map (build only).
"""

from __future__ import annotations

import json
import sys
import time
import urllib.error
import urllib.request
from pathlib import Path

from editor_port import discover_editor_base

HERE = Path(__file__).resolve().parent
EVIDENCE = HERE.parent / "uff_evidence"


def _base_url() -> str:
    base = discover_editor_base()
    if not base:
        raise RuntimeError(
            "serve_editor not reachable on ports 8765–8775 "
            "(start: python3 journal/uff_viewer/serve_editor.py)"
        )
    return base.rstrip("/")


def http_json(method: str, path: str, body: dict | None = None, timeout: int = 300) -> tuple[int, dict]:
    data = None
    headers = {"Accept": "application/json"}
    if body is not None:
        data = json.dumps(body).encode("utf-8")
        headers["Content-Type"] = "application/json"
    req = urllib.request.Request(_base_url() + path, data=data, headers=headers, method=method)
    try:
        with urllib.request.urlopen(req, timeout=timeout) as resp:
            raw = resp.read().decode()
            return resp.status, json.loads(raw) if raw else {}
    except urllib.error.HTTPError as exc:
        raw = exc.read().decode()
        try:
            payload = json.loads(raw) if raw else {}
        except json.JSONDecodeError:
            payload = {"error": raw}
        return exc.code, payload


def main() -> int:
    try:
        from playwright.sync_api import sync_playwright
    except ImportError:
        print("FAIL: pip install playwright && playwright install chromium")
        return 1

    EVIDENCE.mkdir(parents=True, exist_ok=True)
    results: dict[str, str] = {}

    try:
        base = _base_url()
    except RuntimeError as exc:
        print(f"FAIL: {exc}")
        return 1

    # Pre-flight: server + bundle hash
    try:
        with urllib.request.urlopen(base + "/api/health", timeout=5) as resp:
            health = json.loads(resp.read().decode())
        results["server_health"] = "PASS" if health.get("ok") else "FAIL"
        results["bundle_hash_present"] = "PASS" if health.get("bundleHash") else "FAIL (missing)"
    except Exception as exc:
        print(f"FAIL: serve_editor not reachable at {base} — {exc}")
        return 1

    test_name = "edit_play_" + str(int(time.time()))
    map_file = HERE / "maps" / f"{test_name}.json"

    with sync_playwright() as p:
        browser = p.chromium.launch(headless=True)
        page = browser.new_page(viewport={"width": 1440, "height": 900})
        page.on("dialog", lambda d: d.accept())
        page.goto(base + "/?v=pass11", wait_until="load", timeout=120000)
        page.wait_for_function("() => window.__editor", timeout=120000)

        # Bundle freshness: embedded hash should match /api/health
        embedded = page.evaluate("""() => {
          const m = document.documentElement.innerHTML.match(/const EDITOR_BUNDLE_HASH = '([^']+)'/);
          return m ? m[1] : '';
        }""")
        server_hash = health.get("bundleHash") or ""
        results["bundle_freshness"] = (
            "PASS"
            if embedded and server_hash and embedded == server_hash
            else f"FAIL (page={embedded!r}, server={server_hash!r})"
        )

        # Create blank map on server
        created = page.evaluate(
            """async (name) => {
              const blank = { name, box_half: 2500, box_height: 2000, pads: [] };
              const res = await fetch('/api/maps', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ name, map: blank }),
              });
              const payload = await res.json().catch(() => ({}));
              if (!res.ok || !payload.map) return false;
              window.__editor.loadMap(payload.map, { skipHistory: true });
              window.__editor.currentMapId = name;
              window.__editor.markClean();
              return window.__editor.mapState.pads.length === 0;
            }""",
            test_name,
        )
        results["blank_new_map"] = "PASS" if created else "FAIL"

        # Onboarding visible on empty map (session not dismissed yet in fresh context)
        onboarding_visible = page.locator("#onboardingOverlay.open").count() > 0
        results["onboarding_empty_map"] = "PASS" if onboarding_visible else "FAIL"
        page.evaluate("() => { document.getElementById('onboardingDismiss')?.click(); }")
        page.wait_for_timeout(100)

        # Enter edit + spawn tool
        page.evaluate("() => { window.__editor.setEditing(true); window.__editor.beginPlacementMode('spawn'); }")
        page.wait_for_timeout(200)
        results["edit_mode"] = (
            "PASS" if page.evaluate("() => document.body.classList.contains('editing')") else "FAIL"
        )

        # Real canvas click placement
        before_click = page.evaluate("() => window.__editor.mapState.pads.length")
        canvas = page.locator("#c")
        box = canvas.bounding_box()
        if box:
            page.mouse.click(box["x"] + box["width"] * 0.5, box["y"] + box["height"] * 0.5)
        page.wait_for_timeout(200)
        after = page.evaluate("""() => {
          const pads = window.__editor.mapState.pads;
          const last = pads[pads.length - 1];
          return { count: pads.length, y: last ? last.y : null, type: last ? last.type : null };
        }""")
        results["canvas_spawn"] = "PASS" if after["count"] > before_click else "FAIL"
        results["spawn_y"] = "PASS" if after.get("y") == 10 else f"FAIL (y={after.get('y')})"

        # Save via editor API (PUT)
        saved = page.evaluate(
            """async (name) => {
              window.__editor.mapState.name = name;
              window.__editor.currentMapId = name;
              return window.__editor.saveCurrentMap();
            }""",
            test_name,
        )
        page.wait_for_timeout(400)
        results["save_api"] = (
            "PASS" if saved and map_file.is_file() else f"FAIL (saved={saved}, file={map_file.is_file()})"
        )

        # Optional shortcut smoke tests (M/G/E)
        page.evaluate("""() => {
          window.__editor.setEditing(false);
          const s = document.getElementById('snapChk');
          if (s) s.checked = false;
        }""")
        page.wait_for_timeout(100)
        page.keyboard.press("KeyM")
        page.wait_for_timeout(150)
        m_edit = page.evaluate("() => document.body.classList.contains('editing')")
        page.evaluate("""() => {
          window.__editor.setEditing(false);
          const s = document.getElementById('snapChk');
          if (s) s.checked = false;
        }""")
        page.wait_for_timeout(100)
        page.keyboard.press("KeyG")
        page.wait_for_timeout(150)
        g_state = page.evaluate("""() => ({
          editing: document.body.classList.contains('editing'),
          snap: document.getElementById('snapChk')?.checked === true,
        })""")
        page.keyboard.press("KeyE")
        page.wait_for_timeout(100)
        e_off = page.evaluate("() => !document.body.classList.contains('editing')")
        results["shortcut_m_enters_edit"] = "PASS" if m_edit else "FAIL"
        results["shortcut_g_snap"] = "PASS" if g_state.get("editing") and g_state.get("snap") else "FAIL"
        results["shortcut_e_toggle"] = "PASS" if e_off else "FAIL"

        page.screenshot(path=str(EVIDENCE / "edit_play_flow.png"), full_page=False)
        results["screenshot"] = "PASS"

        browser.close()

    # Test Map build (no launch) — direct HTTP so client validation warnings do not block
    if map_file.is_file():
        with open(map_file, encoding="utf-8") as fp:
            map_data = json.load(fp)
        status, body = http_json(
            "POST",
            "/api/test-map",
            {
                "map": map_data,
                "options": {
                    "level": test_name,
                    "deployAs": "uff",
                    "play": False,
                    "seg": True,
                    "deploy": True,
                    "mod": "mod_allinone",
                    "scenario": 0,
                },
            },
            timeout=600,
        )
        if body.get("ok"):
            results["test_map_build"] = "PASS"
        else:
            err = body.get("error") or body.get("stderr") or body.get("stdout") or f"HTTP {status}"
            snippet = str(err).replace("\n", " ")[:240]
            results["test_map_build"] = f"FAIL ({snippet})"
    else:
        results["test_map_build"] = "FAIL (no saved map file)"

    # Cleanup test map file
    http_json("DELETE", f"/api/maps/{test_name}")
    if map_file.is_file():
        map_file.unlink()

    print(json.dumps(results, indent=2))
    print(f"Screenshot: {EVIDENCE / 'edit_play_flow.png'}")
    failed = [k for k, v in results.items() if v != "PASS"]
    if failed:
        print(f"Failed checks: {', '.join(failed)}", file=sys.stderr)
    return 0 if all(v == "PASS" for v in results.values()) else 1


if __name__ == "__main__":
    sys.exit(main())
