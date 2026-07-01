#!/usr/bin/env python3
"""Pass-9 UI overhaul validation — Playwright at 1440×900."""

from __future__ import annotations

import json
import sys
import time
from pathlib import Path

from editor_port import discover_editor_base

HERE = Path(__file__).resolve().parent
EVIDENCE = HERE.parent / "uff_evidence"
FIXTURE = HERE / "fixtures" / "minimal_map.json"
SCREENSHOT = EVIDENCE / "ui_overhaul_verify.png"


def _base_url() -> str:
    base = discover_editor_base()
    if not base:
        raise RuntimeError(
            "serve_editor not reachable on ports 8765–8775 "
            "(start: python3 journal/uff_viewer/serve_editor.py)"
        )
    return base.rstrip("/")


def main() -> int:
    try:
        from playwright.sync_api import sync_playwright
    except ImportError:
        print("FAIL: pip install playwright && playwright install chromium")
        return 1

    try:
        base = _base_url()
    except RuntimeError as exc:
        print(f"FAIL: {exc}")
        return 1

    EVIDENCE.mkdir(parents=True, exist_ok=True)
    results: dict[str, str] = {}

    with sync_playwright() as p:
        browser = p.chromium.launch(headless=True)
        page = browser.new_page(viewport={"width": 1440, "height": 900})
        page.goto(base + "/?v=pass9", wait_until="networkidle", timeout=60000)
        page.on("dialog", lambda d: d.accept())
        page.wait_for_function("() => window.__editor && window.__editor.devServerOnline !== undefined")

        # 1. File toolbar visible and height
        maps_panel = page.locator("#mapsPanel")
        maps_panel.wait_for(state="visible")
        box = maps_panel.bounding_box()
        h = box["height"] if box else 999
        results["file_toolbar_visible"] = "PASS" if box else "FAIL"
        results["file_toolbar_height"] = "PASS" if h <= 80 else f"FAIL ({h:.0f}px)"

        # 2. Test/Play visible without scroll
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
        results["test_play_no_scroll"] = "PASS" if in_viewport else "FAIL"

        # 3. Open file flow with fixture
        page.evaluate(
            """async (jsonText) => {
              return window.__editor.loadMapFromJsonText(jsonText, 'minimal_map.json');
            }""",
            FIXTURE.read_text(),
        )
        title = page.locator("#currentMapTitle").inner_text()
        results["open_file"] = "PASS" if "testarena" in title.lower() else f"FAIL (title={title})"

        # 4. Save → reload → open from list
        test_name = "ui_overhaul_test_" + str(int(time.time()))
        saved = page.evaluate(
            """async (name) => {
              window.__editor.mapState.name = name;
              window.__editor.currentMapId = name;
              if (window.__editor.updateDocTitle) window.__editor.updateDocTitle();
              return window.__editor.saveCurrentMap();
            }""",
            test_name,
        )
        page.wait_for_timeout(500)
        page.evaluate("() => window.__editor.refreshMapsList()")
        page.wait_for_timeout(300)
        # Mark clean so open does not confirm
        page.evaluate("() => window.__editor.markClean()")
        opened = page.evaluate(
            """async (name) => window.__editor.loadSelectedMap(name)""",
            test_name,
        )
        results["save_reload_open"] = "PASS" if saved and opened else f"FAIL (saved={saved}, opened={opened})"
        page.evaluate(
            """async (name) => {
              window.__editor.currentMapId = name;
              return window.__editor.deleteSelectedMap();
            }""",
            test_name,
        )

        page.screenshot(path=str(SCREENSHOT), full_page=False)
        browser.close()

    print(json.dumps(results, indent=2))
    print(f"Screenshot: {SCREENSHOT}")
    return 0 if all(v == "PASS" for v in results.values()) else 1


if __name__ == "__main__":
    sys.exit(main())
