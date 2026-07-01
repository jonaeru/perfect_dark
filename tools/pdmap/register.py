"""Stage registration planner — deterministic codegen for four C wiring points."""

from __future__ import annotations

import os
import re
from dataclasses import dataclass

from .core import ROOT, ROMID
from .from_json import validate_level_name

FILES_H = os.path.join(ROOT, "src", "include", "files.h")
CONSTANTS_H = os.path.join(ROOT, "src", "include", "constants.h")
STAGETABLE = os.path.join(ROOT, "src", "game", "stagetable.c")
SETUP_C = os.path.join(ROOT, "src", "game", "mplayer", "setup.c")
LIST_C = os.path.join(ROOT, "src", "assets", ROMID, "files", "list.c")


@dataclass
class RegistrationPlan:
    name: str
    upper: str
    stage_const: str
    file_ids: dict[str, int]
    already_registered: bool
    snippets: dict[str, str]

    def render_markdown(self) -> str:
        lines = [
            f"# Stage registration plan: {self.name}",
            "",
            f"Already registered: **{self.already_registered}**",
            "",
            "## 1. `src/include/files.h`",
            "",
            "```c",
            self.snippets["files_h"],
            "```",
            "",
            "## 2. `src/assets/ntsc-final/files/list.c`",
            "",
            "```c",
            self.snippets["list_c"],
            "```",
            "",
            "## 3. `src/game/stagetable.c`",
            "",
            "```c",
            self.snippets["stagetable"],
            "```",
            "",
            "## 4. `src/game/mplayer/setup.c` (`g_MpArenas[]`)",
            "",
            "```c",
            self.snippets["setup_c"],
            "```",
            "",
            "> After editing, run `make -j8` and `pdmap build {name} --deploy`.",
            "> For quick test without menu registration, use `--deploy-as uff` instead.",
            "",
        ]
        return "\n".join(lines)


def _read(path: str) -> str:
    with open(path, encoding="utf-8", errors="replace") as fp:
        return fp.read()


def _next_file_id() -> int:
    """Next unused FILE_* hex id (PC port block before NUM_FILES)."""
    text = _read(FILES_H)
    ids = [int(m.group(1), 16) for m in re.finditer(r"#define\s+FILE_\w+\s+(0x[0-9A-Fa-f]+)", text)]
    # Stay below PD Plus mod extension block; use high unused slot near custom maps.
    base = max(ids) + 1 if ids else 0x07E4
    return base


def _resolve_stage_const(name: str) -> tuple[str, int]:
    """Return (STAGE_FOO symbol, hex value) for constants.h / stagetable."""
    text = _read(CONSTANTS_H)
    existing = re.findall(r"#define\s+(STAGE_\w+)\s+(0x[0-9A-Fa-f]+)", text)
    target = f"STAGE_{name.upper()}"
    for const, val in existing:
        if const == target:
            return target, int(val, 16)
    ids = [int(val, 16) for _, val in existing]
    # Custom arenas: stay above stock test stages (UFF=0x4d); avoid title/menu ids.
    stage_val = max(0x80, max(ids) + 1 if ids else 0x80)
    return target, stage_val


def _next_stage_id(name: str) -> str:
    sym, val = _resolve_stage_const(name)
    if f"#define {sym}" in _read(CONSTANTS_H):
        return sym
    return f"{sym}  /* assign {val:#x} in constants.h */"


def _is_registered(name: str) -> bool:
    upper = name.upper()
    text = _read(FILES_H)
    return f"FILE_BG_{upper}_SEG" in text or f"FILE_BG_{upper}_TILES" in text


def plan_registration(name: str) -> RegistrationPlan:
    name = validate_level_name(name)
    upper = name.upper()
    already = _is_registered(name)

    seg_id = _next_file_id()
    tiles_id = seg_id + 1
    pads_id = seg_id + 2
    usetup_id = seg_id + 3
    ump_id = seg_id + 4

    stage_const = _next_stage_id(name)

    files_h = (
        f"#define FILE_BG_{upper}_SEG       {seg_id:#06x}\n"
        f"#define FILE_BG_{upper}_TILES     {tiles_id:#06x}\n"
        f"#define FILE_BG_{upper}_PADS      {pads_id:#06x}\n"
        f"#define FILE_USETUP{upper}         {usetup_id:#06x}\n"
        f"#define FILE_UMP_SETUP{upper}      {ump_id:#06x}"
    )

    list_c = (
        f"/*{seg_id:#06x}*/ \"bgdata/bg_{name}.seg\",\n"
        f"/*{tiles_id:#06x}*/ \"bgdata/bg_{name}_tilesZ\",\n"
        f"/*{pads_id:#06x}*/ \"bgdata/bg_{name}_padsZ\",\n"
        f"/*{usetup_id:#06x}*/ \"Usetup{name}Z\",\n"
        f"/*{ump_id:#06x}*/ \"Ump_setup{name}Z\","
    )

    stagetable = (
        f"/*{seg_id:#06x}*/ {stage_const.split()[0]}, 2, 255, 100, 100, 0, "
        f"FILE_BG_{upper}_SEG, FILE_BG_{upper}_TILES, FILE_BG_{upper}_PADS, "
        f"FILE_USETUP{upper}, FILE_UMP_SETUP{upper}, "
        f"1, 1, 100, 0, 0, -1, 255, 0x3e19999a, -1, 400, 0, 1, SFX_ALARM_DEFAULT, 0,"
    )

    setup_c = f"{{ {stage_const.split()[0]}, 0, 0x7FFF }}, // {name} custom arena"

    return RegistrationPlan(
        name=name,
        upper=upper,
        stage_const=stage_const,
        file_ids={
            "seg": seg_id,
            "tiles": tiles_id,
            "pads": pads_id,
            "usetup": usetup_id,
            "ump_setup": ump_id,
        },
        already_registered=already,
        snippets={
            "files_h": files_h,
            "list_c": list_c,
            "stagetable": stagetable,
            "setup_c": setup_c,
        },
    )


def write_plan(name: str, *, out_dir: str | None = None) -> str:
    plan = plan_registration(name)
    out_dir = out_dir or os.path.join(ROOT, "journal", "map_learn")
    os.makedirs(out_dir, exist_ok=True)
    path = os.path.join(out_dir, f"register_{name}.md")
    with open(path, "w", encoding="utf-8") as fp:
        fp.write(plan.render_markdown())
    return path


def _patch_insert_before(text: str, anchor: str, insertion: str, *, marker: str) -> str:
    """Insert block before anchor; no-op when marker already present (idempotent)."""
    if marker in text:
        return text
    pos = text.find(anchor)
    if pos < 0:
        raise ValueError(f"Registration patch anchor not found: {anchor!r}")
    block = insertion if insertion.endswith("\n") else insertion + "\n"
    return text[:pos] + block + text[pos:]


def apply_registration(name: str) -> list[str]:
    """Patch C sources for stage registration (idempotent). Returns changed file paths."""
    plan = plan_registration(name)
    if plan.already_registered:
        raise ValueError(f"{name}: already registered in files.h")

    stage_sym, stage_val = _resolve_stage_const(name)
    changed: list[str] = []

    const_line = f"#define {stage_sym:<20} {stage_val:#x}"
    const_text = _read(CONSTANTS_H)
    new_const = _patch_insert_before(
        const_text, "#define STAGE_TITLE", const_line + "\n", marker=const_line,
    )
    if new_const != const_text:
        with open(CONSTANTS_H, "w", encoding="utf-8") as fp:
            fp.write(new_const)
        changed.append(CONSTANTS_H)

    files_block = plan.snippets["files_h"] + "\n\n"
    files_text = _read(FILES_H)
    new_files = _patch_insert_before(
        files_text, "\n\n// PD Plus Mod", files_block, marker=f"FILE_BG_{plan.upper}_SEG",
    )
    if new_files != files_text:
        with open(FILES_H, "w", encoding="utf-8") as fp:
            fp.write(new_files)
        changed.append(FILES_H)

    list_lines = plan.snippets["list_c"].replace(",\n", ",\n\t").split("\n")
    list_block = "\n\t".join(list_lines) + "\n"
    list_text = _read(LIST_C)
    new_list = _patch_insert_before(list_text, "\n};", list_block, marker=f"bg_{name}.seg")
    if new_list != list_text:
        with open(LIST_C, "w", encoding="utf-8") as fp:
            fp.write(new_list)
        changed.append(LIST_C)

    stage_row = "\t" + plan.snippets["stagetable"] + ",\n"
    stage_text = _read(STAGETABLE)
    new_stage = _patch_insert_before(
        stage_text, "\t#endif\n};", stage_row, marker=f"FILE_BG_{plan.upper}_SEG",
    )
    if new_stage != stage_text:
        with open(STAGETABLE, "w", encoding="utf-8") as fp:
            fp.write(new_stage)
        changed.append(STAGETABLE)

    setup_row = "\t" + plan.snippets["setup_c"] + "\n"
    setup_text = _read(SETUP_C)
    new_setup = _patch_insert_before(
        setup_text, "\t// Random", setup_row, marker=stage_sym,
    )
    if new_setup != setup_text:
        with open(SETUP_C, "w", encoding="utf-8") as fp:
            fp.write(new_setup)
        changed.append(SETUP_C)

    return changed
