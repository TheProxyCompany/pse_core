#include "accepted_state.h"
#include "state_machine.h"
#include "state_machine_trampoline.h"
#include "walker.h"
#include "walker_trampoline.h"
#include <nanobind/nanobind.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/pair.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/tuple.h>
#include <nanobind/stl/unordered_map.h>
#include <nanobind/stl/variant.h>
#include <nanobind/stl/vector.h>

namespace nb = nanobind;
using namespace nb::literals;

NB_MODULE(_core, m)
{
     m.doc() = R"pbdoc(
        PSE Core C++ Bindings
        ---------------------

        .. currentmodule:: _core

        .. autosummary::
           :toctree: _generate

           StateMachine
           Walker
    )pbdoc";

     // Bind the StateMachine class with trampoline support
     nb::class_<StateMachine, PyStateMachine>(m, "StateMachine")
         .def(nb::init<StateMachine::StateGraph &&, StateMachine::State,
                       std::vector<StateMachine::State> &&, bool, bool>(),
              "state_graph"_a = StateMachine::StateGraph(), "start_state"_a = 0,
              "end_states"_a = std::vector<StateMachine::State>{"$"},
              "is_optional"_a = false, "is_case_sensitive"_a = true)
         .def_rw("state_graph", &StateMachine::state_graph_)
         .def_rw("start_state", &StateMachine::start_state_)
         .def_rw("end_states", &StateMachine::end_states_)
         .def_rw("is_optional", &StateMachine::is_optional_)
         .def_rw("is_case_sensitive", &StateMachine::is_case_sensitive_)
         .def("get_new_walker", &StateMachine::get_new_walker, "state"_a = nb::none())
         .def("get_walkers", &StateMachine::get_walkers, "state"_a = nb::none())
         .def("get_edges", &StateMachine::get_edges, "state"_a)
         .def("get_transitions", &StateMachine::get_transitions, "walker"_a, "state"_a = nb::none())
         .def("advance", &StateMachine::advance, "walker"_a, "token"_a)
         .def("branch_walker", &StateMachine::branch_walker, "walker"_a,
              "token"_a = nb::none())
         .def("__eq__", &StateMachine::operator==)
         .def("__str__", &StateMachine::to_string)
         .def("__repr__", &StateMachine::__repr__);

     // Bind the Walker class with trampoline support
     nb::class_<Walker, PyWalker>(m, "Walker")
         .def(
             nb::init<std::shared_ptr<StateMachine>, std::optional<StateMachine::State>>(),
             "state_machine"_a, "current_state"_a = nb::none())

         // Properties
         .def("get_current_value", &Walker::get_current_value)
         .def("get_raw_value", &Walker::get_raw_value)

         // Read/Write properties
         .def_rw("state_machine", &Walker::state_machine_)
         .def_rw("accepted_history", &Walker::accepted_history_)
         .def_rw("explored_edges", &Walker::explored_edges_)
         .def_rw("current_state", &Walker::current_state_)
         .def_rw("target_state", &Walker::target_state_)
         .def_rw("transition_walker", &Walker::transition_walker_)
         .def_rw("consumed_character_count", &Walker::consumed_character_count_)
         .def_rw("remaining_input", &Walker::remaining_input_, "remaining_input"_a = nb::none())
         .def_rw("_raw_value", &Walker::_raw_value_)
         .def_rw("_accepts_more_input", &Walker::_accepts_more_input_)

         // Pure virtual methods
         .def("clone", &Walker::clone)

         // Virtual methods with default implementations
         .def("consume_token", &Walker::consume_token)
         .def("can_accept_more_input", &Walker::can_accept_more_input)
         .def("is_within_value", &Walker::is_within_value)
         .def("should_start_transition", &Walker::should_start_transition)
         .def("should_complete_transition", &Walker::should_complete_transition)
         .def("accepts_any_token", &Walker::accepts_any_token)
         .def("get_valid_continuations", &Walker::get_valid_continuations,
              "depth"_a = 0)
         .def("has_reached_accept_state", &Walker::has_reached_accept_state)

         // Non-virtual methods
         .def("start_transition", &Walker::start_transition, "transition_walker"_a,
              "token"_a = nb::none(), "start_state"_a = nb::none(),
              "target_state"_a = nb::none())
         .def("complete_transition", &Walker::complete_transition)
         .def("branch", &Walker::branch, "token"_a = nb::none())

         // Special methods
         .def("__eq__", &Walker::operator==)
         .def("__str__", &Walker::to_string)
         .def("__repr__", &Walker::__repr__);

     nb::class_<AcceptedState, Walker>(m, "AcceptedState")
         .def(nb::init<std::shared_ptr<Walker>>())
         .def("clone", &AcceptedState::clone)
         .def("can_accept_more_input", &AcceptedState::can_accept_more_input)
         .def("has_reached_accept_state", &AcceptedState::has_reached_accept_state)
         .def("is_within_value", &AcceptedState::is_within_value)
         .def("should_start_transition", &AcceptedState::should_start_transition)
         .def("consume_token", &AcceptedState::consume_token)
         .def("__eq__", &AcceptedState::operator==)
         .def("__repr__", &AcceptedState::__repr__);
}
