"""Deterministic map-creation learning engine.

Probes the repo (code + build + fixtures) to accumulate verified facts and
measure documentation coverage. See ``pdmap learn run`` and ``journal/map_learn/``.
"""

from .engine import LearnEngine, LearnReport
from .facts import Fact

__all__ = ["LearnEngine", "LearnReport", "Fact"]
