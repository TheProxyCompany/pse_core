#include "acceptor.h"
#include "acceptor_trampoline.h"
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

NB_MODULE(_core, m) {
  m.doc() = R"pbdoc(
        PSE Core C++ Bindings
        ---------------------

        .. currentmodule:: _core

        .. autosummary::
           :toctree: _generate

           add
           subtract
           Acceptor
           Walker
    )pbdoc";

  // Bind the Acceptor class with trampoline support
  nb::class_<Acceptor, PyAcceptor>(m, "Acceptor")
      .def(nb::init<std::shared_ptr<Acceptor::StateGraph>, Acceptor::State,
                    std::vector<Acceptor::State>, bool, bool>(),
           "state_graph"_a = nullptr, "start_state"_a = 0,
           "end_states"_a = std::vector<Acceptor::State>{"$"},
           "is_optional"_a = false, "is_case_sensitive"_a = true)
      .def_prop_ro("is_optional", &Acceptor::is_optional)
      .def_prop_ro("is_case_sensitive", &Acceptor::is_case_sensitive)
      .def("walker_class", &Acceptor::walker_class)
      .def("get_walkers", &Acceptor::get_walkers, "state"_a = nb::none())
      .def("get_transitions", &Acceptor::get_transitions, "walker"_a)
      .def("advance", &Acceptor::advance, "walker"_a, "token"_a)
      .def("branch_walker", &Acceptor::branch_walker, "walker"_a,
           "token"_a = nb::none())
      .def("__eq__", &Acceptor::operator==)
      .def("__str__", &Acceptor::to_string)
      .def("__repr__", &Acceptor::repr);

  // Bind the Walker class with trampoline support
  nb::class_<Walker, PyWalker>(m, "Walker")
      .def(
          nb::init<std::shared_ptr<Acceptor>, std::optional<Acceptor::State>>(),
          "acceptor"_a, "current_state"_a = nb::none())

      // Properties
      .def_prop_ro("current_value", &Walker::current_value)
      .def_prop_ro("raw_value", &Walker::raw_value)
      .def_prop_ro("current_edge", &Walker::current_edge)

      // Read-only member variables
      // Read-only properties
      .def_prop_ro("acceptor", &Walker::get_acceptor)
      .def_prop_ro("accepted_history", &Walker::get_accepted_history)
      .def_prop_ro("explored_edges", &Walker::get_explored_edges)
      .def_prop_ro("current_state", &Walker::get_current_state)
      .def_prop_ro("target_state", &Walker::get_target_state)
      .def_prop_ro("transition_walker", &Walker::get_transition_walker)
      .def_prop_ro("consumed_character_count",
                   &Walker::get_consumed_character_count)
      .def_prop_ro("remaining_input", &Walker::get_remaining_input)

      // Pure virtual methods
      .def("consume_token", &Walker::consume_token)
      .def("can_accept_more_input", &Walker::can_accept_more_input)
      .def("is_within_value", &Walker::is_within_value)
      .def("clone", &Walker::clone)

      // Virtual methods with default implementations
      .def("should_start_transition", &Walker::should_start_transition)
      .def("should_complete_transition", &Walker::should_complete_transition)
      .def("accepts_any_token", &Walker::accepts_any_token)
      .def("get_valid_continuations", &Walker::get_valid_continuations,
           "depth"_a = 0)
      .def("has_reached_accept_state", &Walker::has_reached_accept_state)
      .def("find_valid_prefixes", &Walker::find_valid_prefixes)

      // Non-virtual methods
      .def("start_transition", &Walker::start_transition, "transition_walker"_a,
           "token"_a = nb::none(), "start_state"_a = nb::none(),
           "target_state"_a = nb::none())
      .def("complete_transition", &Walker::complete_transition)
      .def("branch", &Walker::branch, "token"_a = nb::none())

      // Special methods
      .def("__eq__", &Walker::operator==)
      .def("__str__", &Walker::to_string)
      .def("__repr__", &Walker::repr);
}
