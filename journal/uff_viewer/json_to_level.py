#!/usr/bin/env python3
"""Convert pass-2 editor JSON into a pdmap level module (src/levels/<name>.py).

Thin wrapper around ``tools.pdmap.from_json`` + ``tools.pdmap.level_codegen``.
Prefer ``pdmap from-json`` for build/play; use this module only when you need
a persistent ``src/levels/<name>.py`` on disk.

CLI:
  python3 json_to_level.py map.json > src/levels/foo.py
  python3 json_to_level.py map.json -o src/levels/foo.py
"""

from __future__ import annotations

import argparse
import json
import sys

# Allow running from journal/uff_viewer without installing pdmap.
import os

_HERE = os.path.dirname(os.path.abspath(__file__))
_ROOT = os.path.dirname(os.path.dirname(_HERE))
if _ROOT not in sys.path:
    sys.path.insert(0, _ROOT)

from tools.pdmap.from_json import EditorMapSpec  # noqa: E402
from tools.pdmap.level_codegen import render_level_module  # noqa: E402


def json_to_level_py(data: dict) -> str:
    """Render editor JSON as a complete src/levels/<name>.py module string."""
    spec = EditorMapSpec.from_json(data)
    return render_level_module(spec)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Convert editor JSON to a pdmap level .py module.")
    parser.add_argument("json_file", nargs="?", help="Editor JSON file (default: stdin)")
    parser.add_argument("-o", "--output", help="Write to file instead of stdout")
    args = parser.parse_args(argv)

    if args.json_file:
        with open(args.json_file, encoding="utf-8") as fp:
            data = json.load(fp)
    else:
        data = json.load(sys.stdin)

    py_src = json_to_level_py(data)

    if args.output:
        with open(args.output, "w", encoding="utf-8") as fp:
            fp.write(py_src)
    else:
        sys.stdout.write(py_src)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
