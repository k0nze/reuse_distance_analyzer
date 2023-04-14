#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "acadl_instruction_generator.h"
#include "reuseAnalyzer.h"
#include "set_associative_cache.h"

namespace py = pybind11;

// TODO comment why
py::object return_none() { return py::cast<py::none>(Py_None); }

PYBIND11_MODULE(reusedist, m) {
    py::class_<reuseAnalyzer, std::shared_ptr<reuseAnalyzer>>(m, "ReuseAnalyzer")
        .def(py::init<int, int, int, int>(), py::arg("sets"), py::arg("ways"), py::arg("cacheline_size"), py::arg("block_size"))
        .def("process_load", &reuseAnalyzer::processLoad)
        .def("process_store", &reuseAnalyzer::processStore)
        .def("get_reuse_distance_counts", &reuseAnalyzer::getReuseDistanceCounts)
        .def(py::init<std::shared_ptr<SetAssociativeCache>, int>(), py::arg("set_associative_cache"), py::arg("block_size") = 16)
        .def("analyze_instruction_generator", &reuseAnalyzer::analyzeInstructionGenerator, py::arg("instruction_generator"));
}
