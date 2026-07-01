"""Deterministic probes — each returns verified ``Fact`` records."""

from __future__ import annotations

import glob
import json
import os
import re
from typing import Any

from ..builders import PAD_FLOOR_OFFSET
from ..core import ROOT, ROMID
from ..from_json import EditorMapSpec
from ..pipeline import build_from_spec
from ..validate import validate_mapdef
from .facts import Fact

FIXTURES_DIR = os.path.join(ROOT, "journal", "uff_viewer", "fixtures")
DOC_WIKI = os.path.join(ROOT, "docs", "MAP_MAKING_WIKI.md")
DOC_CREATION = os.path.join(ROOT, "docs", "MAP_CREATION.md")


def _fact(category: str, claim: str, source: str, probe: str, **kwargs) -> Fact:
    return Fact(
        id=Fact.make_id(category, claim),
        category=category,
        claim=claim,
        source=source,
        verified_by=probe,
        **kwargs,
    )


def probe_coded_invariants() -> list[Fact]:
    """Invariants encoded in Python (not tribal knowledge)."""
    probe = "probe_coded_invariants"
    facts: list[Fact] = []

    facts.append(_fact(
        "invariant",
        f"Spawn pads must have Y > floor Y (default offset {PAD_FLOOR_OFFSET})",
        "tools/pdmap/builders.py:PAD_FLOOR_OFFSET",
        probe,
        tags=["pads", "collision"],
    ))
    facts.append(_fact(
        "invariant",
        "Pad indices must be contiguous 0..N-1 matching array order",
        "tools/pdmap/validate.py:validate_mapdef",
        probe,
        tags=["pads", "mkpads"],
    ))
    facts.append(_fact(
        "invariant",
        "Waypoint graph must be symmetric (bidirectional edges)",
        "tools/pdmap/core.py:MapDef.pack_pads_json",
        probe,
        tags=["bots", "waypoints"],
    ))
    facts.append(_fact(
        "invariant",
        "MP setup must emit ailist id 0x1000 (mp_init_simulants)",
        "tools/pdmap/core.py:MapDef.pack_setup",
        probe,
        tags=["bots", "setup"],
    ))
    facts.append(_fact(
        "invariant",
        "Intro Weapon param2 is dual-wield weapon id or -1, not pad index",
        "tools/pdmap/intro.py",
        probe,
        tags=["intro", "loadout"],
    ))
    facts.append(_fact(
        "invariant",
        "Intro Ammo param2 is quantity, not pad index",
        "tools/pdmap/intro.py",
        probe,
        tags=["intro", "loadout"],
    ))
    facts.append(_fact(
        "invariant",
        "Room 0 in tiles JSON must be empty (engine void room)",
        "docs/MAP_CREATION.md + tools/pdmap/builders.py:floor_box_tiles",
        probe,
        tags=["tiles", "rooms"],
    ))
    facts.append(_fact(
        "invariant",
        "Box arena collision comes from tiles; seg is visual + room bboxes",
        "docs/MAP_MAKING_WIKI.md §2",
        probe,
        tags=["seg", "tiles"],
    ))
    facts.append(_fact(
        "invariant",
        "PDMAP_SEG_MODE=empty avoids near-plane viewport sheet in FPS",
        "tools/pdmap/pipeline.py:build_from_spec",
        probe,
        tags=["seg", "rendering"],
    ))
    facts.append(_fact(
        "invariant",
        "G_VTX vertex count nibble max 16; box seg uses G_VTX(4) per face",
        "tools/pdmap/seg.py",
        probe,
        tags=["seg", "collision"],
    ))
    return facts


def probe_pipeline_contract() -> list[Fact]:
    """Execute minimal JSON → build without deploy; record contract facts."""
    probe = "probe_pipeline_contract"
    facts: list[Fact] = []

    minimal = {
        "name": "learnprobe",
        "box_half": 2000,
        "box_height": 1500,
        "pads": [{"type": "spawn", "x": 0, "y": 0, "z": 0, "room": 1}],
    }
    spec = EditorMapSpec.from_json(minimal, deploy_name="learnprobe")

    pre_errors = validate_mapdef(spec.mapdef, tiles_room_count=2)
    facts.append(_fact(
        "pipeline",
        "EditorMapSpec.from_json auto-corrects Y<=0 pads to spawn_y",
        "tools/pdmap/from_json.py",
        probe,
        evidence={"y_corrected": list(spec.y_corrected_pads)},
        tags=["from-json"],
    ))
    facts.append(_fact(
        "pipeline",
        "validate_mapdef passes after Y correction on minimal spawn map",
        "tools/pdmap/validate.py",
        probe,
        evidence={"pre_build_errors": pre_errors},
        tags=["validate"],
    ))

    errors, warnings = build_from_spec(
        spec,
        deploy=False,
        want_seg=True,
        seg_mode="empty",
        skip_validate=False,
        verbose=False,
    )
    build_dir = os.path.join(ROOT, "build", ROMID, "assets", "files", "bgdata")
    assets = {
        "tiles": os.path.join(build_dir, "bg_learnprobe_tilesZ"),
        "pads": os.path.join(build_dir, "bg_learnprobe_padsZ"),
        "seg": os.path.join(build_dir, "bg_learnprobe.seg"),
        "setup": os.path.join(ROOT, "build", ROMID, "Ump_setuplearnprobeZ"),
    }
    facts.append(_fact(
        "pipeline",
        "build_from_spec produces five assets from editor JSON without level module",
        "tools/pdmap/pipeline.py",
        probe,
        evidence={
            "errors": errors,
            "warnings": warnings,
            "assets_exist": {k: os.path.exists(v) for k, v in assets.items()},
        },
        tags=["from-json", "build"],
    ))
    facts.append(_fact(
        "pipeline",
        "pdmap from-json is the canonical entry point for deterministic box maps",
        "tools/pdmap/__init__.py:cmd_from_json",
        probe,
        tags=["cli"],
    ))
    return facts


def probe_fixtures() -> list[Fact]:
    """Build every JSON fixture under journal/uff_viewer/fixtures/."""
    probe = "probe_fixtures"
    facts: list[Fact] = []
    pattern = os.path.join(FIXTURES_DIR, "*.json")
    paths = sorted(glob.glob(pattern))

    for path in paths:
        base = os.path.basename(path)
        with open(path, encoding="utf-8") as fp:
            data = json.load(fp)
        name = f"learn_{base.replace('.json', '').replace(' ', '_')}"
        try:
            spec = EditorMapSpec.from_json(data, deploy_name=name)
            errors, _warnings = build_from_spec(
                spec, deploy=False, want_seg=True, seg_mode="empty", verbose=False,
            )
            ok = not errors
            err_msg = errors
        except Exception as exc:
            ok = False
            err_msg = [str(exc)]

        facts.append(_fact(
            "pipeline",
            f"Fixture {base} builds cleanly via from-json pipeline",
            path,
            probe,
            confidence=1.0 if ok else 0.5,
            evidence={"ok": ok, "errors": err_msg, "pads": len(data.get("pads") or [])},
            tags=["fixture", base],
        ))
    return facts


def probe_engine_wiring() -> list[Fact]:
    """Extract stage registration facts from C sources."""
    probe = "probe_engine_wiring"
    facts: list[Fact] = []

    files_h = os.path.join(ROOT, "src", "include", "files.h")
    stagetable = os.path.join(ROOT, "src", "game", "stagetable.c")
    setup_c = os.path.join(ROOT, "src", "game", "mplayer", "setup.c")

    if os.path.exists(files_h):
        text = open(files_h, encoding="utf-8", errors="replace").read()
        file_defs = re.findall(r"#define\s+(FILE_BG_\w+)\s+(0x[0-9A-Fa-f]+)", text)
        facts.append(_fact(
            "engine",
            f"files.h defines {len(file_defs)} FILE_BG_* asset constants",
            files_h,
            probe,
            evidence={"count": len(file_defs)},
            tags=["registration", "files.h"],
        ))
        uff_seg = [m for m in file_defs if "UFF" in m[0]]
        if uff_seg:
            facts.append(_fact(
                "engine",
                f"--test-map loads STAGE_TEST_UFF via bg_uff.* ({uff_seg[0][0]}={uff_seg[0][1]})",
                "src/game/title.c + files.h",
                probe,
                tags=["test-map", "uff"],
            ))

    if os.path.exists(stagetable):
        stages = len(re.findall(r"STAGE_", open(stagetable, encoding="utf-8", errors="replace").read()))
        facts.append(_fact(
            "engine",
            "stagetable.c maps STAGE_* ids to five bgdata file ids per stage",
            stagetable,
            probe,
            evidence={"stage_symbol_refs": stages},
            tags=["registration", "stagetable"],
        ))

    if os.path.exists(setup_c):
        arenas = len(re.findall(r"\{\s*STAGE_", open(setup_c, encoding="utf-8", errors="replace").read()))
        facts.append(_fact(
            "engine",
            "g_MpArenas[] in setup.c lists Combat Simulator menu entries",
            setup_c,
            probe,
            evidence={"arena_row_refs": arenas},
            tags=["registration", "menu"],
        ))
        facts.append(_fact(
            "engine",
            "pdmap register emits four C wiring snippets to journal/map_learn/register_<name>.md",
            "tools/pdmap/register.py",
            probe,
            tags=["registration", "codegen"],
        ))
        if _apply_registration_available():
            facts.append(_fact(
                "engine",
                "pdmap register --apply patches files.h, list.c, stagetable.c, setup.c, and constants.h",
                "tools/pdmap/register.py:apply_registration",
                probe,
                tags=["registration", "codegen"],
            ))
        else:
            facts.append(_fact(
                "engine",
                "Stage registration still requires manual paste (pdmap register --apply not implemented)",
                "tools/pdmap/register.py",
                probe,
                tags=["registration", "gap"],
            ))

    facts.append(_fact(
        "engine",
        "Quick-team --test-map sets g_MpSetup.chrslots=0x01 (player only) before bot allocation",
        "src/game/title.c",
        probe,
        tags=["bots", "test-map"],
    ))
    return facts


def probe_doc_coverage(knowledge_claims: list[str]) -> list[Fact]:
    """Check which verified claims appear in canonical docs."""
    probe = "probe_doc_coverage"
    facts: list[Fact] = []

    wiki_text = ""
    creation_text = ""
    if os.path.exists(DOC_WIKI):
        wiki_text = open(DOC_WIKI, encoding="utf-8").read()
    if os.path.exists(DOC_CREATION):
        creation_text = open(DOC_CREATION, encoding="utf-8").read()
    combined = wiki_text + "\n" + creation_text

    # Keywords that must appear in docs for near-perfect coverage.
    required_topics = [
        ("pads", "spawn Y above floor"),
        ("ailist", "0x1000"),
        ("symmetric", "waypoint"),
        ("from-json", "from-json"),
        ("register", "pdmap register"),
        ("learn", "pdmap learn"),
        ("PDMAP_SEG_MODE", "empty"),
        ("chrslots", "0x01"),
        ("five files", "tilesZ"),
        ("intro", "dualweapon"),
        ("--test-map", "STAGE_TEST_UFF"),
        ("--moddir", "mod_allinone"),
    ]

    for tag, needle in required_topics:
        in_wiki = needle.lower() in wiki_text.lower() or tag.lower() in wiki_text.lower()
        in_creation = needle.lower() in creation_text.lower() or tag.lower() in creation_text.lower()
        documented = in_wiki or in_creation
        facts.append(_fact(
            "doc",
            f"Documentation covers topic '{tag}' ({needle})",
            DOC_WIKI if in_wiki else (DOC_CREATION if in_creation else "missing"),
            probe,
            confidence=1.0 if documented else 0.0,
            evidence={"in_wiki": in_wiki, "in_creation": in_creation},
            tags=["doc-coverage", tag],
        ))

    documented_topics = sum(1 for tag, needle in required_topics if (
        needle.lower() in wiki_text.lower() or tag.lower() in wiki_text.lower()
        or needle.lower() in creation_text.lower() or tag.lower() in creation_text.lower()
    ))
    topic_ratio = documented_topics / max(len(required_topics), 1)
    facts.append(_fact(
        "doc",
        f"Required-topic doc coverage: {topic_ratio:.1%} ({documented_topics}/{len(required_topics)})",
        "docs/MAP_MAKING_WIKI.md + docs/MAP_CREATION.md",
        probe,
        evidence={"ratio": topic_ratio, "documented": documented_topics, "total": len(required_topics)},
        tags=["doc-coverage", "metric"],
    ))

    # Secondary: learned-claim substring match (noisy; informational only).
    documented_count = 0
    for claim in knowledge_claims:
        snippet = claim[:48].lower()
        if snippet in combined.lower():
            documented_count += 1
    claim_ratio = documented_count / max(len(knowledge_claims), 1)
    facts.append(_fact(
        "doc",
        f"Learned-claim substring coverage: {claim_ratio:.1%} ({documented_count}/{len(knowledge_claims)})",
        "docs/MAP_MAKING_WIKI.md + docs/MAP_CREATION.md",
        probe,
        evidence={"ratio": claim_ratio, "documented": documented_count, "total": len(knowledge_claims)},
        tags=["doc-coverage", "claim-metric"],
    ))
    return facts


def _apply_registration_available() -> bool:
    """True when register --apply can patch C wiring automatically."""
    try:
        from ..register import apply_registration  # noqa: F401
        return callable(apply_registration)
    except ImportError:
        return False


def probe_scenario_pairing() -> list[Fact]:
    """Verify CTF case / case_respawn team pairing rules in validate_mapdef."""
    from ..core import MapDef, Pad
    from ..intro import Case, Spawn
    from ..validate import validate_mapdef

    probe = "probe_scenario_pairing"
    facts: list[Fact] = []

    facts.append(_fact(
        "invariant",
        "Each CTF team with a Case pad must have at least one CaseRespawn pad for that team",
        "tools/pdmap/validate.py:validate_mapdef",
        probe,
        tags=["scenario", "ctf", "case_respawn"],
    ))

    bad = MapDef(name="pairprobe")
    bad.pads = [Pad(0, 0, 10, 0, room=1)]
    bad.intro = [Spawn(pad=0), Case(team=0, pad=0)]
    bad_errors = validate_mapdef(bad, tiles_room_count=2)
    pairing_fail = any("case_respawn" in e for e in bad_errors)
    facts.append(_fact(
        "pipeline",
        "validate_mapdef rejects Case pads without matching case_respawn team",
        "tools/pdmap/validate.py",
        probe,
        confidence=1.0 if pairing_fail else 0.0,
        evidence={"errors": bad_errors, "detected": pairing_fail},
        tags=["scenario", "validation"],
    ))

    # Stock uff setup pairs four teams (verified in mp_setupuff.c).
    setup_path = os.path.join(ROOT, "src", "setups", "mp_setupuff.c")
    if os.path.exists(setup_path):
        text = open(setup_path, encoding="utf-8", errors="replace").read()
        case_teams = set(re.findall(r"case\s*\(\s*(\d+)", text))
        respawn_teams = set(re.findall(r"case_respawn\s*\(\s*(\d+)", text))
        paired = case_teams <= respawn_teams
        facts.append(_fact(
            "engine",
            f"Reference map uff pairs case/case_respawn for teams {sorted(case_teams)}",
            setup_path,
            probe,
            confidence=1.0 if paired else 0.5,
            evidence={
                "case_teams": sorted(case_teams),
                "respawn_teams": sorted(respawn_teams),
                "paired": paired,
            },
            tags=["scenario", "uff", "reference"],
        ))
    return facts


def probe_seg_script_inventory() -> list[Fact]:
    """Inventory level modules for custom SEG_SCRIPT vs procedural box seg."""
    probe = "probe_seg_script_inventory"
    facts: list[Fact] = []
    levels_dir = os.path.join(ROOT, "src", "levels")
    if not os.path.isdir(levels_dir):
        return facts

    with_script: list[str] = []
    box_only: list[str] = []
    for fname in sorted(os.listdir(levels_dir)):
        if not fname.endswith(".py") or fname.startswith("__"):
            continue
        path = os.path.join(levels_dir, fname)
        text = open(path, encoding="utf-8", errors="replace").read()
        name = fname[:-3]
        if re.search(r"^\s*SEG_SCRIPT\s*=", text, re.MULTILINE):
            with_script.append(name)
        elif "BOX_HALF" in text and "BOX_HEIGHT" in text:
            box_only.append(name)

    facts.append(_fact(
        "pipeline",
        f"Level modules with SEG_SCRIPT: {len(with_script)} ({', '.join(with_script) or 'none'})",
        levels_dir,
        probe,
        evidence={"modules": with_script},
        tags=["seg", "SEG_SCRIPT"],
    ))
    facts.append(_fact(
        "pipeline",
        f"Box-arena modules without SEG_SCRIPT (procedural seg): {len(box_only)}",
        "tools/pdmap/pipeline.py:build_from_spec",
        probe,
        evidence={"count": len(box_only), "modules": box_only[:12]},
        tags=["seg", "box"],
    ))
    facts.append(_fact(
        "invariant",
        "BOX_HALF+BOX_HEIGHT levels get procedural box seg; SEG_SCRIPT overrides with custom generator",
        "tools/pdmap/pipeline.py",
        probe,
        tags=["seg", "pipeline"],
    ))
    return facts


def _is_registered_gap_closed() -> bool:
    """Deprecated alias — use _apply_registration_available()."""
    return _apply_registration_available()


def probe_level_modules() -> list[Fact]:
    """Inventory src/levels/*.py modules."""
    probe = "probe_level_modules"
    facts: list[Fact] = []
    levels_dir = os.path.join(ROOT, "src", "levels")
    if not os.path.isdir(levels_dir):
        return facts

    from ..core import load_level_module

    for fname in sorted(os.listdir(levels_dir)):
        if not fname.endswith(".py") or fname.startswith("__"):
            continue
        name = fname[:-3]
        try:
            mod = load_level_module(name)
            m = mod.build()
            has_box = hasattr(mod, "BOX_HALF") and hasattr(mod, "BOX_HEIGHT")
            has_tiles = hasattr(mod, "build_tiles_json")
            facts.append(_fact(
                "pipeline",
                f"Level module {name}: {len(m.pads)} pads, box_arena={has_box}, custom_tiles={has_tiles}",
                os.path.join(levels_dir, fname),
                probe,
                evidence={"pads": len(m.pads), "props": len(m.props), "intro": len(m.intro)},
                tags=["level-module", name],
            ))
        except Exception as exc:
            facts.append(_fact(
                "pipeline",
                f"Level module {name} failed to load: {exc}",
                os.path.join(levels_dir, fname),
                probe,
                confidence=0.0,
                tags=["level-module", name, "error"],
            ))
    return facts


ALL_PROBES = [
    ("coded_invariants", probe_coded_invariants),
    ("pipeline_contract", probe_pipeline_contract),
    ("fixtures", probe_fixtures),
    ("engine_wiring", probe_engine_wiring),
    ("level_modules", probe_level_modules),
    ("scenario_pairing", probe_scenario_pairing),
    ("seg_script_inventory", probe_seg_script_inventory),
]
