#!/usr/bin/env python3
"""Pass-10 native Mac UI validation — Playwright at 1440×900."""

from __future__ import annotations

import json
import subprocess
import sys
import time
import urllib.request
from pathlib import Path

HERE = Path(__file__).resolve().parent
EVIDENCE = HERE.parent / "uff_evidence"
FIXTURE = HERE / "fixtures" / "minimal_map.json"
BASE = "http://127.0.0.1:8765"


def curl_maps_ok() -> bool:
    try:
        with urllib.request.urlopen(BASE + "/api/maps", timeout=5) as resp:
            body = json.loads(resp.read().decode())
            return resp.status == 200 and isinstance(body.get("maps"), list)
    except Exception:
        return False


def main() -> int:
    try:
        from playwright.sync_api import sync_playwright
    except ImportError:
        print("FAIL: pip install playwright && playwright install chromium")
        return 1

    EVIDENCE.mkdir(parents=True, exist_ok=True)
    results: dict[str, str] = {}

    with sync_playwright() as p:
        browser = p.chromium.launch(headless=True)
        page = browser.new_page(viewport={"width": 1440, "height": 900})
        page.on("dialog", lambda d: d.accept())
        page.goto(BASE + "/?v=pass10", wait_until="load", timeout=120000)
        page.wait_for_function("() => window.__editor", timeout=120000)

        # 1. Old file toolbar must be gone
        maps_panel = page.locator("#mapsPanel")
        results["no_maps_panel"] = "PASS" if maps_panel.count() == 0 else "FAIL (still present)"

        # 2. Run bar visible and compact
        run_bar = page.locator("#runBar")
        run_bar.wait_for(state="visible")
        box = run_bar.bounding_box()
        h = box["height"] if box else 999
        results["run_bar_visible"] = "PASS" if box else "FAIL"
        results["run_bar_height"] = "PASS" if h <= 60 else f"FAIL ({h:.0f}px)"

        # 3. Status pill visible
        pill = page.locator("#statusPill")
        pill.wait_for(state="visible")
        results["status_pill_visible"] = "PASS" if pill.is_visible() else "FAIL"

        # 4. Primary chrome vertical budget (run bar + browser menu if any)
        chrome_h = page.evaluate(
            """() => {
              const run = document.getElementById('runBar');
              const menu = document.getElementById('browserMenuBar');
              let h = run ? run.getBoundingClientRect().height + 12 : 0;
              if (menu && getComputedStyle(menu).display !== 'none') {
                h += menu.getBoundingClientRect().height;
              }
              return h;
            }"""
        )
        results["chrome_vertical"] = "PASS" if chrome_h <= 120 else f"FAIL ({chrome_h:.0f}px)"

        # 5. Play button visible without scroll
        play_btn = page.locator("#testPlayBtn")
        play_btn.wait_for(state="visible")
        in_viewport = page.evaluate(
            """() => {
              const el = document.getElementById('testPlayBtn');
              if (!el) return false;
              const r = el.getBoundingClientRect();
              return r.top >= 0 && r.bottom <= window.innerHeight;
            }"""
        )
        results["play_no_scroll"] = "PASS" if in_viewport else "FAIL"

        # 6. Open file flow with fixture
        page.evaluate(
            """async (jsonText) => window.__editor.loadMapFromJsonText(jsonText, 'minimal_map.json')""",
            FIXTURE.read_text(),
        )
        title = page.locator("#currentMapTitle").inner_text()
        results["open_file"] = "PASS" if "testarena" in title.lower() else f"FAIL (title={title})"

        # 7. Save → reload → open from list
        test_name = "ui_v2_test_" + str(int(time.time()))
        saved = page.evaluate(
            """async (name) => {
              window.__editor.mapState.name = name;
              window.__editor.currentMapId = name;
              window.__editor.updateDocTitle();
              return window.__editor.saveCurrentMap();
            }""",
            test_name,
        )
        page.wait_for_timeout(500)
        page.evaluate("() => window.__editor.markClean()")
        opened = page.evaluate(
            """async (name) => window.__editor.loadSelectedMap(name)""",
            test_name,
        )
        results["save_reload_open"] = "PASS" if saved and opened else f"FAIL (saved={saved}, opened={opened})"
        page.evaluate(
            """async (name) => {
              const path = '/api/maps/' + encodeURIComponent(name);
              const res = await fetch(path, { method: 'DELETE' });
              return res.ok;
            }""",
            test_name,
        )

        # 8. Build settings modal opens from API
        page.evaluate("() => window.__editor.openBuildSettings()")
        modal_open = page.locator("#buildSettingsModal.open")
        results["build_settings_modal"] = "PASS" if modal_open.count() > 0 else "FAIL"
        page.evaluate("() => window.__editor.closeBuildSettings()")

        # 9. New map flow — POST starter template (avoids modal promise hang in headless)
        new_name = "test_new_xyz_" + str(int(time.time()))
        created = page.evaluate(
            """async (name) => {
              window.__editor.markClean();
              const res = await fetch('/api/maps', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ name }),
              });
              const payload = await res.json().catch(() => ({}));
              if (!res.ok || !payload.map) return false;
              window.__editor.loadMap(payload.map, { skipHistory: true });
              window.__editor.currentMapId = name;
              window.__editor.markClean();
              return window.__editor.mapState.pads.length === 8;
            }""",
            new_name,
        )
        map_file = HERE / "maps" / f"{new_name}.json"
        results["create_new_map"] = (
            "PASS" if created and map_file.is_file() else f"FAIL (created={created}, file={map_file.is_file()})"
        )
        if map_file.is_file():
            page.evaluate(
                """async (name) => {
                  const path = '/api/maps/' + encodeURIComponent(name);
                  await fetch(path, { method: 'DELETE' });
                }""",
                new_name,
            )
            if map_file.is_file():
                map_file.unlink()

        # Screenshots
        page.screenshot(path=str(EVIDENCE / "ui_v2_viewport.png"), full_page=False)
        page.evaluate("() => window.__editor.setEditing(true)")
        page.wait_for_timeout(200)
        page.screenshot(path=str(EVIDENCE / "ui_v2_edit_mode.png"), full_page=False)
        page.evaluate("() => window.__editor.setEditing(false)")

        browser.close()

    results["api_maps"] = "PASS" if curl_maps_ok() else "FAIL"

    print(json.dumps(results, indent=2))
    print(f"Screenshots: {EVIDENCE / 'ui_v2_viewport.png'}, {EVIDENCE / 'ui_v2_edit_mode.png'}")
    return 0 if all(v == "PASS" for v in results.values()) else 1


if __name__ == "__main__":
    sys.exit(main())
