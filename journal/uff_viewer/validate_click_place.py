#!/usr/bin/env python3
"""Verify click-to-place map editing — spawn tool adds pad at Y=10."""

from __future__ import annotations

import json
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
EVIDENCE = HERE.parent / "uff_evidence"
BASE = "http://127.0.0.1:8765"


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
        page.goto(BASE + "/?v=clickplace", wait_until="load", timeout=120000)
        page.on("dialog", lambda d: d.accept())
        page.wait_for_function("() => window.__editor", timeout=120000)

        # Clear to empty map and enter placement mode
        page.evaluate("""() => {
          window.__editor.clearMap();
        }""")
        page.wait_for_timeout(300)

        before = page.evaluate("() => window.__editor.mapState.pads.length")
        editing = page.evaluate("() => document.body.classList.contains('editing')")
        tool = page.evaluate("() => window.__editor.mapState && window.__editor.setTool && true")

        results["edit_mode_active"] = "PASS" if editing else "FAIL"
        results["floating_toolbar"] = (
            "PASS" if page.locator("#placeToolbar").is_visible() else "FAIL"
        )

        # Place spawn via API raycast equivalent (center of viewport floor hit)
        placed = page.evaluate("""() => {
          const ed = window.__editor;
          ed.beginPlacementMode('spawn');
          const before = ed.mapState.pads.length;
          // Simulate floor click at map center (0, 0) — reliable without WebGL raycast in headless
          ed.addPad('spawn', { x: 500, z: 500 });
          const after = ed.mapState.pads.length;
          const last = ed.mapState.pads[after - 1];
          return {
            grew: after === before + 1,
            y: last ? last.y : null,
            type: last ? last.type : null,
          };
        }""")
        results["pad_added"] = "PASS" if placed.get("grew") else "FAIL"
        results["pad_y_floor"] = "PASS" if placed.get("y") == 10 else f"FAIL (y={placed.get('y')})"
        results["pad_type_spawn"] = "PASS" if placed.get("type") == "spawn" else f"FAIL ({placed.get('type')})"

        # Weapon variant placement
        weapon = page.evaluate("""() => {
          const ed = window.__editor;
          ed.setTool('weapon');
          const n = ed.mapState.pads.length;
          ed.addPad('weapon', { x: -500, z: 500 });
          const p = ed.mapState.pads[ed.mapState.pads.length - 1];
          return { grew: ed.mapState.pads.length === n + 1, weapon: p.weapon, y: p.y };
        }""")
        results["weapon_placed"] = "PASS" if weapon.get("grew") else "FAIL"
        results["weapon_y_floor"] = "PASS" if weapon.get("y") == 10 else f"FAIL (y={weapon.get('y')})"

        # Real canvas click placement (WebGL raycast on floor plane)
        page.evaluate("""() => {
          window.__editor.loadMap({ name: 'clicktest', box_half: 2500, box_height: 2000, pads: [] }, { skipHistory: true });
          window.__editor.beginPlacementMode('spawn');
        }""")
        page.wait_for_timeout(200)
        canvas = page.locator("#c")
        box = canvas.bounding_box()
        before_click = page.evaluate("() => window.__editor.mapState.pads.length")
        if box:
            page.mouse.click(box["x"] + box["width"] * 0.5, box["y"] + box["height"] * 0.5)
        page.wait_for_timeout(150)
        after_click = page.evaluate("""() => {
          const pads = window.__editor.mapState.pads;
          const last = pads[pads.length - 1];
          return { count: pads.length, y: last ? last.y : null, type: last ? last.type : null };
        }""")
        results["canvas_click_adds"] = "PASS" if after_click["count"] > before_click else "FAIL"
        results["canvas_click_y"] = "PASS" if after_click.get("y") == 10 else f"FAIL (y={after_click.get('y')})"

        page.evaluate("() => window.__editor.setEditing(true)")
        page.wait_for_timeout(150)
        page.screenshot(path=str(EVIDENCE / "click_place_edit.png"), full_page=False)
        results["screenshot"] = "PASS"

        browser.close()

    print(json.dumps(results, indent=2))
    print(f"Screenshot: {EVIDENCE / 'click_place_edit.png'}")
    return 0 if all(v == "PASS" for v in results.values()) else 1


if __name__ == "__main__":
    sys.exit(main())
