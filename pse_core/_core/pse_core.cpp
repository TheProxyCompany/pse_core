#include <nanobind/nanobind.h>
#include "add.h"
#include "subtract.h"

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
    )pbdoc";

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
