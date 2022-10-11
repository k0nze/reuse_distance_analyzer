#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "reusedist.h"
namespace py = pybind11;

//TODO comment why
py::object return_none() { return py::cast<py::none>(Py_None); }

PYBIND11_MODULE(reusedist, m){
    py::class_<reusedist,std::shared_ptr<reusedist>>(m,"reusedist").def(py::init<>());
}