"""Orchestrate probes, merge knowledge, emit reports and deterministic spec."""

from __future__ import annotations

import json
import os
from dataclasses import dataclass, field
from datetime import datetime, timezone
from typing import Any

from ..core import ROOT
from .facts import Fact, load_knowledge, merge_facts, save_knowledge
from . import probes

LEARN_DIR = os.path.join(ROOT, "journal", "map_learn")
KNOWLEDGE_PATH = os.path.join(LEARN_DIR, "knowledge.json")
RUNS_DIR = os.path.join(LEARN_DIR, "runs")
SPEC_PATH = os.path.join(ROOT, "docs", "MAP_DETERMINISTIC_SPEC.md")
GAPS_PATH = os.path.join(LEARN_DIR, "gaps.md")


@dataclass
class LearnReport:
    """Result of one learning-engine iteration."""

    iteration: int
    run_id: str
    facts_total: int
    facts_new: int
    doc_coverage_ratio: float
    gaps: list[str] = field(default_factory=list)
    probe_errors: list[str] = field(default_factory=list)
    run_path: str = ""

    def summary(self) -> str:
        lines = [
            f"Map learn run {self.run_id} (iteration {self.iteration})",
            f"  Facts: {self.facts_total} total, {self.facts_new} new/updated this run",
            f"  Doc coverage: {self.doc_coverage_ratio:.1%}",
            f"  Gaps: {len(self.gaps)}",
        ]
        if self.probe_errors:
            lines.append(f"  Probe errors: {len(self.probe_errors)}")
        return "\n".join(lines)


class LearnEngine:
    """Deterministic learning engine for map-creation knowledge."""

    def __init__(self, *, knowledge_path: str = KNOWLEDGE_PATH) -> None:
        self.knowledge_path = knowledge_path
        self._facts = load_knowledge(knowledge_path)

    @property
    def facts(self) -> dict[str, Fact]:
        return self._facts

    def run(self, *, iteration: int = 1) -> LearnReport:
        """Execute all probes, merge facts, measure doc coverage, write run artifact."""
        run_id = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
        os.makedirs(RUNS_DIR, exist_ok=True)

        before_ids = set(self._facts)
        # Gap facts are re-derived each run; drop stale entries from prior iterations.
        self._facts = {k: v for k, v in self._facts.items() if "gap" not in v.tags}
        all_new: list[Fact] = []
        probe_errors: list[str] = []

        for probe_name, probe_fn in probes.ALL_PROBES:
            try:
                if probe_name == "doc_coverage":
                    claims = [f.claim for f in self._facts.values()]
                    batch = probes.probe_doc_coverage(claims)
                else:
                    batch = probe_fn()
                all_new.extend(batch)
            except Exception as exc:
                probe_errors.append(f"{probe_name}: {exc}")

        self._facts = merge_facts(self._facts, all_new)

        # Re-run doc coverage after merge for accurate ratio.
        try:
            doc_facts = probes.probe_doc_coverage([f.claim for f in self._facts.values()])
            self._facts = merge_facts(self._facts, doc_facts)
        except Exception as exc:
            probe_errors.append(f"doc_coverage_final: {exc}")

        doc_ratio = 0.0
        for f in self._facts.values():
            if f.category == "doc" and "metric" in f.tags:
                doc_ratio = max(doc_ratio, float(f.evidence.get("ratio", 0.0)))

        gaps = self._compute_gaps()
        meta = {
            "last_run": run_id,
            "iteration": iteration,
            "doc_coverage_ratio": doc_ratio,
            "gap_count": len(gaps),
        }
        save_knowledge(self.knowledge_path, self._facts, meta=meta)

        run_payload = {
            "run_id": run_id,
            "iteration": iteration,
            "facts_added": len(set(f.id for f in all_new) - before_ids),
            "facts_total": len(self._facts),
            "doc_coverage_ratio": doc_ratio,
            "gaps": gaps,
            "probe_errors": probe_errors,
            "probes": [name for name, _ in probes.ALL_PROBES],
        }
        run_path = os.path.join(RUNS_DIR, f"{run_id}.json")
        with open(run_path, "w", encoding="utf-8") as fp:
            json.dump(run_payload, fp, indent=2)
            fp.write("\n")

        self._write_gaps_report(gaps, doc_ratio, run_id)
        self.emit_spec()

        return LearnReport(
            iteration=iteration,
            run_id=run_id,
            facts_total=len(self._facts),
            facts_new=len(set(f.id for f in all_new) - before_ids),
            doc_coverage_ratio=doc_ratio,
            gaps=gaps,
            probe_errors=probe_errors,
            run_path=run_path,
        )

    def _compute_gaps(self) -> list[str]:
        """Facts with confidence=0 or doc facts not documented."""
        gaps: list[str] = []
        for fact in self._facts.values():
            if fact.category == "doc" and fact.confidence < 1.0:
                gaps.append(f"[doc missing] {fact.claim}")
            if "gap" in fact.tags:
                gaps.append(f"[engine gap] {fact.claim}")
            if fact.category == "pipeline" and fact.confidence < 1.0:
                gaps.append(f"[pipeline fail] {fact.claim}")
                if fact.evidence.get("errors"):
                    gaps.append(f"  errors: {fact.evidence['errors']}")
        return gaps

    def _write_gaps_report(self, gaps: list[str], ratio: float, run_id: str) -> None:
        os.makedirs(LEARN_DIR, exist_ok=True)
        lines = [
            f"# Map learn gaps ({run_id})",
            "",
            f"Doc coverage: **{ratio:.1%}**",
            "",
            "## Action items",
            "",
        ]
        if not gaps:
            lines.append("_No gaps detected this run._")
        else:
            for g in gaps:
                lines.append(f"- {g}")
        lines.extend([
            "",
            "## Next probe targets",
            "",
            "- Runtime headless smoke (--test-map log parse)",
            "- Custom seg generators beyond procedural box (SEG_SCRIPT modules)",
            "- env.c / lang.c wiring for new registered stages",
            "- End-to-end register --apply + make smoke on a scratch map name",
            "",
        ])
        with open(GAPS_PATH, "w", encoding="utf-8") as fp:
            fp.write("\n".join(lines))
            fp.write("\n")

    def emit_spec(self, path: str = SPEC_PATH) -> str:
        """Write machine-verified deterministic spec from high-confidence facts."""
        by_category: dict[str, list[Fact]] = {}
        for fact in self._facts.values():
            if fact.confidence < 0.9:
                continue
            if fact.category == "doc":
                continue
            by_category.setdefault(fact.category, []).append(fact)

        lines = [
            "# Map creation — deterministic spec (auto-generated)",
            "",
            "_Generated by `pdmap learn run`. Only includes facts verified by code execution",
            "or direct source inspection. Do not edit by hand — re-run the learn engine._",
            "",
        ]

        order = ["invariant", "pipeline", "engine", "editor", "binary"]
        for cat in order:
            items = by_category.get(cat, [])
            if not items:
                continue
            lines.append(f"## {cat.title()}")
            lines.append("")
            for fact in sorted(items, key=lambda f: f.claim):
                lines.append(f"- **{fact.claim}**")
                lines.append(f"  - Source: `{fact.source}`")
                lines.append(f"  - Verified by: `{fact.verified_by}`")
                if fact.tags:
                    lines.append(f"  - Tags: {', '.join(fact.tags)}")
                lines.append("")

        lines.extend([
            "## Canonical commands",
            "",
            "```bash",
            "# Deterministic box map from editor JSON",
            "python3 tools/pdmap.py from-json map.json --deploy-as uff --deploy --play",
            "",
            "# Run learning engine",
            "python3 tools/pdmap.py learn run",
            "python3 tools/pdmap.py learn report",
            "```",
            "",
        ])

        os.makedirs(os.path.dirname(path), exist_ok=True)
        with open(path, "w", encoding="utf-8") as fp:
            fp.write("\n".join(lines))
            fp.write("\n")
        return path

    def report_text(self) -> str:
        """Human-readable status."""
        meta: dict[str, Any] = {}
        if os.path.exists(self.knowledge_path):
            with open(self.knowledge_path, encoding="utf-8") as fp:
                meta = json.load(fp).get("meta", {})

        ratio = float(meta.get("doc_coverage_ratio", 0.0))
        # Prefer live metric facts if meta stale.
        for f in self._facts.values():
            if f.category == "doc" and "metric" in f.tags:
                ratio = max(ratio, float(f.evidence.get("ratio", 0.0)))
        lines = [
            "=== Map Learn Engine ===",
            f"Knowledge: {self.knowledge_path}",
            f"Facts: {len(self._facts)}",
            f"Doc coverage: {ratio:.1%}",
            f"Last run: {meta.get('last_run', 'never')}",
            f"Gaps file: {GAPS_PATH}",
            f"Spec: {SPEC_PATH}",
            "",
        ]

        by_cat: dict[str, int] = {}
        for f in self._facts.values():
            by_cat[f.category] = by_cat.get(f.category, 0) + 1
        lines.append("Facts by category:")
        for cat, count in sorted(by_cat.items()):
            lines.append(f"  {cat}: {count}")

        gaps = self._compute_gaps()
        if gaps:
            lines.append("")
            lines.append(f"Open gaps ({len(gaps)}):")
            for g in gaps[:15]:
                lines.append(f"  - {g}")
            if len(gaps) > 15:
                lines.append(f"  ... and {len(gaps) - 15} more (see {GAPS_PATH})")

        return "\n".join(lines)
