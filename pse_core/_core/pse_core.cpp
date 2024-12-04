#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/variant.h>
#include <nanobind/stl/pair.h>
#include <nanobind/stl/tuple.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/unordered_map.h>
#include "add.h"
#include "subtract.h"
#include "acceptor.h"
#include "acceptor_trampoline.h"

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
    )pbdoc";

    // Bind the Acceptor class with trampoline support
    nb::class_<Acceptor, PyAcceptor>(m, "Acceptor")
        .def(nb::init<std::shared_ptr<Acceptor::StateGraph>,
                     Acceptor::State,
                     std::vector<Acceptor::State>,
                     bool,
                     bool>(),
             "state_graph"_a = nullptr,
             "start_state"_a = 0,
             "end_states"_a = std::vector<Acceptor::State>{"$"},
             "is_optional"_a = false,
             "is_case_sensitive"_a = true)
        .def_prop_ro("is_optional", &Acceptor::is_optional)
        .def_prop_ro("is_case_sensitive", &Acceptor::is_case_sensitive)
        .def("walker_class", &Acceptor::walker_class)
        .def("get_walkers", &Acceptor::get_walkers, "state"_a = nb::none())
        .def("get_transitions", &Acceptor::get_transitions, "walker"_a)
        .def("advance", &Acceptor::advance, "walker"_a, "token"_a)
        .def("branch_walker", &Acceptor::branch_walker,
             "walker"_a, "token"_a = nb::none())
        .def("__eq__", &Acceptor::operator==)
        .def("__str__", &Acceptor::to_string)
        .def("__repr__", &Acceptor::repr);

    // Original bindings
    m.def("add", &add, "i"_a, "j"_a,
          "Add two numbers together.\n\n"
          "Args:\n"
          "    i: First integer to add\n"
          "    j: Second integer to add\n\n"
          "Returns:\n"
          "    Sum of the two input integers");

    m.def("subtract", &subtract, "i"_a, "j"_a,
          "Subtract two numbers together.\n\n"
          "Args:\n"
          "    i: First integer to subtract from\n"
          "    j: Second integer to subtract\n\n"
          "Returns:\n"
          "    Difference between the two input integers");
}
