#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "reuse_distance_analyzer.h"

namespace py = pybind11;

PYBIND11_MODULE(pybind_reuse_distance_analyzer, m) {
    m.doc() = "Python Bindings for the C++ implementation of the ReuseDistanceAnalyzer";

    py::class_<ReuseDistanceAnalyzer>(m, "ReuseDistanceAnalyzer")
        .def(py::init<uint32_t, uint32_t, uint32_t, u_int32_t>(), py::arg("sets"), py::arg("ways"), py::arg("cache_line_size"),
             py::arg("block_size"))
        .def("process_load", &ReuseDistanceAnalyzer::process_load)
        .def("process_store", &ReuseDistanceAnalyzer::process_store)
        .def("get_reuse_distance_counts", &ReuseDistanceAnalyzer::get_reuse_distance_counts);
}
