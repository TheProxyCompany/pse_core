from importlib import metadata
from typing import TypeAlias

from pse_core.state_machine import StateMachine

__version__ = metadata.version(__package__ or "")
del metadata

State: TypeAlias = int | str
Edge: TypeAlias = tuple[StateMachine, State]
VisitedEdge: TypeAlias = tuple[State, State | None, str | None]
StateGraph: TypeAlias = dict[State, list[Edge]]

__all__ = ["Edge", "State", "StateGraph", "VisitedEdge"]