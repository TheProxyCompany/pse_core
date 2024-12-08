"""State machine walker module.

This module provides the base `Walker` and `StateMachine` classes for traversing state machines during parsing
and generation. Walkers track state transitions and maintain parsing history, while StateMachines manage the
constraints and transitions within the state machine.
"""

from __future__ import annotations

from abc import ABC, abstractmethod
from typing import Any, Self

from pse_core import Edge, State, StateGraph, VisitedEdge

class StateMachine(ABC):
    """
    A state machine that manages multiple walkers representing
    different valid states, enabling efficient traversal and minimizing backtracking.
    """

    def __init__(
        self,
        state_graph: StateGraph | None = None,
        start_state: State = 0,
        end_states: list[State] | None = None,
        is_optional: bool = False,
        is_case_sensitive: bool = True,
    ) -> None: ...
    @property
    def is_optional(self) -> bool:
        """Check if the state machine is optional."""
        ...

    @property
    def is_case_sensitive(self) -> bool:
        """Check if the state machine is case sensitive."""
        ...

    @abstractmethod
    def get_new_walker(self, state: State | None = None) -> Walker:
        """Get a new walker for this state machine."""
        ...

    @abstractmethod
    def get_walkers(self, state: State | None = None) -> list[Walker]:
        """Get walkers to traverse the state machine.

        Args:
            state: Optional starting state.

        Returns:
            A list of walker instances.
        """
        ...

    @abstractmethod
    def get_transitions(self, walker: Walker) -> list[tuple[Walker, State]]:
        """Get transitions from the given walker.

        Args:
            walker: Walker to get transitions from.

        Returns:
            A list of tuples containing a walker and a state.
        """
        ...

    @abstractmethod
    def get_edges(self, state: State) -> list[Edge]:
        """Get edges from the given state."""
        ...

    @abstractmethod
    def advance(self, walker: Walker, token: str) -> list[Walker]:
        """Advance the walker with the given input token.

        Args:
            walker: The walker to advance.
            token: The input token to process.

        Returns:
            A list of updated walkers after advancement.
        """
        ...

    @abstractmethod
    def branch_walker(self, walker: Walker, token: str | None = None) -> list[Walker]:
        """Branch the walker into multiple paths.

        Args:
            walker: Walker to branch.
            token: Optional token to consider.

        Returns:
            A list of branched walkers.
        """
        ...

    def __eq__(self, other: object) -> bool:
        """Check equality based on the state machine's state graph.

        Args:
            other: The object to compare with.

        Returns:
            True if both state machines are equal; False otherwise.
        """
        ...

    def __str__(self) -> str:
        """Return the string representation of the state machine."""
        ...

    def __repr__(self) -> str:
        """Return a detailed string representation of the state machine."""
        ...

    @property
    def start_state(self) -> State:
        """The start state of the state machine."""
        ...

    @start_state.setter
    def start_state(self, value: State) -> None: ...

    @property
    def end_states(self) -> list[State]:
        """The end states of the state machine."""
        ...

    @end_states.setter
    def end_states(self, value: list[State]) -> None: ...

    @property
    def state_graph(self) -> StateGraph:
        """The state transition graph."""
        ...

    @state_graph.setter
    def state_graph(self, value: StateGraph) -> None: ...

class Walker(ABC):
    """
    Base class for state machine walkers.

    A `Walker` represents a position in a state machine graph and manages transitions
    between states as input is consumed. It tracks the current state, transition
    history, and accumulated values during parsing or generation.
    """

    def __init__(
        self,
        state_machine: StateMachine,
        current_state: State | None = None,
    ) -> None: ...

    @abstractmethod
    def clone(self) -> Self:
        """Create a clone of the walker.

        Returns:
            A new instance of the walker with the same state.
        """
        ...

    def consume_token(self, token: str) -> list[Walker]:
        """Advance the walker with the given input token.

        Args:
            token: The token to process.

        Returns:
            A list of updated walker instances after advancement.
        """
        ...

    def can_accept_more_input(self) -> bool:
        """Indicate whether the walker can accept more input for the current state.

        Returns:
            True if the walker can accept more input; False otherwise.
        """
        ...

    def is_within_value(self) -> bool:
        """Determine if the walker is currently within a value.

        Returns:
            True if in a value; False otherwise.
        """
        ...

    def should_start_transition(self, token: str) -> bool:
        """Determine if a transition should start with the given input token.

        Args:
            token: The token to process.

        Returns:
            True if the transition should start; False otherwise.
        """
        ...

    def should_complete_transition(self) -> bool:
        """Determine if the transition should complete.

        Returns:
            True if the transition should complete; False otherwise.
        """
        ...

    def accepts_any_token(self) -> bool:
        """Check if the state machine accepts any token (i.e., free text).

        Returns:
            True if all tokens are accepted; False otherwise.
        """
        ...

    def get_valid_continuations(self, depth: int = 0) -> list[str]:
        """Return the set of strings that allow valid continuation from current state.

        Args:
            depth: The current depth in the state machine traversal.

        Returns:
            A list of strings that represent valid continuations from current state.
        """
        ...

    def has_reached_accept_state(self) -> bool:
        """Check if the walker has reached an accepted (final) state.

        Returns:
            True if in an accepted state; False otherwise.
        """
        ...

    def parse_value(self, value: str | None) -> Any:
        """Parse the accumulated value into an appropriate type.

        Args:
            value: The value to parse.

        Returns:
            The parsed value.
        """
        ...

    def start_transition(
        self,
        transition_walker: Walker,
        token: str | None = None,
        start_state: State | None = None,
        target_state: State | None = None,
    ) -> Walker | None:
        """Start a new transition with the given token.

        Args:
            transition_walker: The walker handling the current transition.
            token: Optional token to consider.
            start_state: Optional starting state.
            target_state: Optional target state.

        Returns:
            A new walker instance after starting the transition or None if not possible.
        """
        ...

    def complete_transition(
        self,
        transition_walker: Walker,
    ) -> tuple[Walker | None, bool]:
        """Complete the current transition.

        Args:
            transition_walker: The walker handling the current transition.

        Returns:
            A tuple containing a new walker instance and a boolean indicating success.
        """
        ...

    def branch(self, token: str | None = None) -> list[Walker]:
        """Branch the current walker into multiple paths.

        Args:
            token: Optional token to consider.

        Returns:
            A list of branched walker instances.
        """
        ...



    @property
    def current_value(self) -> Any:
        """Retrieve the accumulated walker value.

        Returns:
            The current value from transition or history, parsed into appropriate type.
            Returns None if no value is accumulated.
        """
        ...

    @property
    def raw_value(self) -> str | None:
        """Retrieve the raw accumulated value as a string.

        Returns:
            The concatenated raw values from history and transitions.
        """
        ...

    @property
    def current_edge(self) -> VisitedEdge:
        """Return the current edge as a tuple.

        Returns:
            A tuple representing the current edge.
        """
        ...

    @property
    def state_machine(self) -> StateMachine:
        """The state machine associated with this walker."""
        ...

    @state_machine.setter
    def state_machine(self, value: StateMachine) -> None: ...

    @property
    def accepted_history(self) -> list[Walker]:
        """The history of accepted walkers."""
        ...

    @accepted_history.setter
    def accepted_history(self, value: list[Walker]) -> None: ...

    @property
    def explored_edges(self) -> set[VisitedEdge]:
        """The set of explored edges."""
        ...

    @explored_edges.setter
    def explored_edges(self, value: set[VisitedEdge]) -> None: ...

    @property
    def current_state(self) -> State:
        """The current state."""
        ...

    @current_state.setter
    def current_state(self, value: State) -> None: ...

    @property
    def target_state(self) -> State | None:
        """The target state."""
        ...

    @target_state.setter
    def target_state(self, value: State | None) -> None: ...

    @property
    def transition_walker(self) -> Walker | None:
        """The transition walker."""
        ...

    @transition_walker.setter
    def transition_walker(self, value: Walker | None) -> None: ...

    @property
    def consumed_character_count(self) -> int:
        """The number of consumed characters."""
        ...

    @consumed_character_count.setter
    def consumed_character_count(self, value: int) -> None: ...

    @property
    def remaining_input(self) -> str | None:
        """The remaining input string."""
        ...

    @remaining_input.setter
    def remaining_input(self, value: str | None) -> None: ...

    @property
    def _accepts_more_input(self) -> bool:
        """Check if the walker can accept more input."""
        ...

    @_accepts_more_input.setter
    def _accepts_more_input(self, value: bool) -> None: ...

    @property
    def _raw_value(self) -> str | None:
        """The raw accumulated value as a string."""
        ...

    @_raw_value.setter
    def _raw_value(self, value: str | None) -> None: ...

    def __eq__(self, other: object) -> bool:
        """Check equality based on the walker's state and accumulated value.

        Args:
            other: The object to compare with.

        Returns:
            True if both walkers are equal; False otherwise.
        """
        ...

    def __str__(self) -> str:
        """Return the string representation of the walker."""
        ...

    def __repr__(self) -> str:
        """Return a detailed string representation of the walker."""
        ...
