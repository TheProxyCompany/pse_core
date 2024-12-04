#include <pybind11/pybind11.h>

namespace py = pybind11;

int add(int i, int j) { return i + j; }

PYBIND11_MODULE(pse_core, m) {
  m.doc() = R"pbdoc(
        PSE Core C++ Bindings
        ---------------------

        .. currentmodule:: pse_core

        .. autosummary::
           :toctree: _generate

           add
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
}
