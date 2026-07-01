#!/usr/bin/env python3
"""Ensure pd.ini [LLM] section matches LLM Play defaults.

The PC port reads keys under [LLM] (e.g. Enabled=1), not flat LLM.Enabled lines.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

# Values applied every LLM Play launch (PlayerMask=1 => human player 1).
LLM_DEFAULTS: dict[str, str] = {
    "Enabled": "1",
    "Port": "8780",
    "PlayerMask": "1",
    "BotMask": "15",
    "DecisionIntervalMs": "1000",
    "ActionTtlMs": "3000",
}

LEGACY_FLAT_KEY = re.compile(r"^LLM\.[A-Za-z0-9_]+=", re.ASCII)
SECTION_HEADER = re.compile(r"^\[([^\]]+)\]\s*$")


def _split_key_value(line: str) -> tuple[str, str] | None:
    stripped = line.strip()
    if not stripped or stripped.startswith("#") or stripped.startswith(";"):
        return None
    if SECTION_HEADER.match(stripped):
        return None
    if "=" not in stripped:
        return None
    key, value = stripped.split("=", 1)
    return key.strip(), value.strip()


def ensure_llm_section(text: str) -> tuple[str, list[str]]:
    """Return updated ini text and a short log of changes."""
    lines = text.splitlines(keepends=True)
    if not lines:
        lines = [""]

    # Drop mistaken flat keys appended by an older launcher (Video.LLM.* noise).
    filtered: list[str] = []
    changes: list[str] = []
    for line in lines:
        core = line.strip()
        if LEGACY_FLAT_KEY.match(core):
            changes.append(f"removed legacy line: {core}")
            continue
        filtered.append(line)
    lines = filtered

    out: list[str] = []
    in_llm = False
    llm_seen: set[str] = set()
    had_llm_section = False

    def append_missing_llm_keys() -> None:
        for key, value in LLM_DEFAULTS.items():
            if key not in llm_seen:
                out.append(f"{key}={value}\n")
                changes.append(f"added {key}={value}")

    idx = 0
    while idx < len(lines):
        line = lines[idx]
        stripped = line.strip()

        if stripped == "[LLM]":
            had_llm_section = True
            in_llm = True
            llm_seen.clear()
            out.append(line if line.endswith("\n") else line + "\n")
            idx += 1
            continue

        if in_llm and SECTION_HEADER.match(stripped):
            append_missing_llm_keys()
            in_llm = False
            llm_seen.clear()
            out.append(line if line.endswith("\n") else line + "\n")
            idx += 1
            continue

        if in_llm:
            parsed = _split_key_value(stripped)
            if parsed:
                key, old_val = parsed
                if key in LLM_DEFAULTS:
                    new_val = LLM_DEFAULTS[key]
                    if old_val != new_val:
                        changes.append(f"{key}: {old_val} -> {new_val}")
                    out.append(f"{key}={new_val}\n")
                    llm_seen.add(key)
                else:
                    out.append(line if line.endswith("\n") else line + "\n")
            else:
                out.append(line if line.endswith("\n") else line + "\n")
            idx += 1
            continue

        out.append(line if line.endswith("\n") else line + "\n")
        idx += 1

    if in_llm:
        append_missing_llm_keys()
        in_llm = False

    if not had_llm_section:
        block = ["\n", "[LLM]\n"]
        for key, value in LLM_DEFAULTS.items():
            block.append(f"{key}={value}\n")
            changes.append(f"created [LLM] {key}={value}")

        inserted = False
        rebuilt: list[str] = []
        for line in out:
            if not inserted and line.strip() == "[Game]":
                rebuilt.extend(block)
                inserted = True
            rebuilt.append(line)
        out = rebuilt if inserted else out + block

    result = "".join(out)
    if not result.endswith("\n"):
        result += "\n"
    return result, changes


def main() -> int:
    if len(sys.argv) != 2:
        print("usage: ensure-llm-pd-ini.py /path/to/pd.ini", file=sys.stderr)
        return 2

    ini_path = Path(sys.argv[1])
    if ini_path.exists():
        original = ini_path.read_text(encoding="utf-8")
    else:
        original = ""

    updated, changes = ensure_llm_section(original)
    if updated != original:
        ini_path.parent.mkdir(parents=True, exist_ok=True)
        ini_path.write_text(updated, encoding="utf-8")

    for line in changes:
        print(line)
    if not changes:
        print("unchanged")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
