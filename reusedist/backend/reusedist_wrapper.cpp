#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "reuseAnalyzer.h"

namespace py = pybind11;

// TODO comment why
py::object return_none() { return py::cast<py::none>(Py_None); }

PYBIND11_MODULE(reusedist, m) {
  py::class_<reuseAnalyzer, std::shared_ptr<reuseAnalyzer>>(m, "ReuseAnalyzer")
      .def(py::init<int, int, int, int>(), py::arg("sets"), py::arg("ways"),
           py::arg("cacheline_size"), py::arg("block_size"))
      .def("process_load", &reuseAnalyzer::processLoad)
      .def("process_store", &reuseAnalyzer::processStore)
      .def("get_reuse_distance_counts", &reuseAnalyzer::getReuseDistanceCounts);
}
