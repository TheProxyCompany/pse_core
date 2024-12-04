#include <pybind11/pybind11.h>
#include "include/add.h"
#include "include/subtract.h"

namespace py = pybind11;

PYBIND11_MODULE(_core, m) {
  m.doc() = R"pbdoc(
        PSE Core C++ Bindings
        ---------------------

        .. currentmodule:: _core

        .. autosummary::
           :toctree: _generate

           add
           subtract
    )pbdoc";

  m.def("add", &add, R"pbdoc(
        Add two numbers together.

        Args:
            i: First integer to add
            j: Second integer to add

        Returns:
            Sum of the two input integers
    )pbdoc",
        py::arg("i"), py::arg("j"));

  m.def("subtract", &subtract, R"pbdoc(
        Subtract two numbers together.

        Args:
            i: First integer to subtract from
            j: Second integer to subtract

        Returns:
            Difference between the two input integers
    )pbdoc",
        py::arg("i"), py::arg("j"));
}
