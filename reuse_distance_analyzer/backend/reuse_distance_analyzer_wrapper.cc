#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "reuse_distance_analyzer.h"

namespace py = pybind11;

PYBIND11_MODULE(pybind_reuse_distance_analyzer, m) {
    m.doc() = "Python Bindings for the C++ implementation of the ReuseDistanceAnalyzer";

    py::class_<ReuseDistanceAnalyzer>(m, "ReuseDistanceAnalyzer")
        .def(py::init<uint32_t, uint32_t, uint32_t, u_int32_t>(), py::arg("sets"), py::arg("ways"), py::arg("cache_line_size"),
             py::arg("block_size"));

    /* py::class_<reuseAnalyzer, std::shared_ptr<reuseAnalyzer>>(m, "ReuseAnalyzer")
        .def(py::init<int, int, int, int>(), py::arg("sets"), py::arg("ways"), py::arg("cacheline_size"), py::arg("block_size"))
        .def("process_load", &reuseAnalyzer::processLoad)
        .def("process_store", &reuseAnalyzer::processStore)
        .def("get_reuse_distance_counts", &reuseAnalyzer::getReuseDistanceCounts)
        .def(py::init<std::shared_ptr<SetAssociativeCache>, int>(), py::arg("set_associative_cache"), py::arg("block_size") = 16)
        .def("analyze_instruction_generator", &reuseAnalyzer::analyzeInstructionGenerator, py::arg("instruction_generator"));
 */}
