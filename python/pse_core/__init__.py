from importlib import metadata

from ._core import Edge, State, StateGraph, VisitedEdge  # type: ignore[attr-defined]

__version__ = metadata.version(__package__ or "")
del metadata

__all__ = ["Edge", "State", "StateGraph", "VisitedEdge"]
