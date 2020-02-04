//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:        BSD License
//                  Kratos default license: kratos/license.txt
//
//  Main authors:   Raul Bravo
//
//


// System includes

// External includes
#include <pybind11/pybind11.h>


// Project includes
#include "includes/define.h"
#include "custom_python/add_custom_utilities_to_python.h"
#include "custom_utilities/rom_residuals_utility.h"
#include "custom_utilities/residuals_utility.h"

#include "spaces/ublas_space.h"
#include "linear_solvers/linear_solver.h"


namespace Kratos {
namespace Python {

void AddCustomUtilitiesToPython(pybind11::module& m)
{
    namespace py = pybind11;

    typedef UblasSpace<double, CompressedMatrix, Vector> SparseSpaceType;
    typedef UblasSpace<double, Matrix, Vector> LocalSpaceType;
    typedef LinearSolver<SparseSpaceType, LocalSpaceType > LinearSolverType;


    py::class_<GetRomResiduals, typename GetRomResiduals::Pointer>(m, "GetRomResiduals")
    .def(py::init<ModelPart&, Parameters, BaseSchemeType::Pointer>()) // 
    .def("Execute",&GetRomResiduals::Calculate) //
    .def("GetVolumeVector",&GetRomResiduals::Volumes) //
    .def("ToNumpy",&GetRomResiduals::convert_to_numpy)
    ;  

    py::class_<GetResiduals, typename GetResiduals::Pointer>(m, "GetResiduals")
    .def(py::init<ModelPart&, BaseSchemeType::Pointer>()) // 
    .def("ElementalResidual",&GetResiduals::ElementalResidual) //
    .def("ConditionalResidual",&GetResiduals::ConditionalResidual) //  
    .def("ToNumpy2",&GetResiduals::convert_to_numpy)
    ;

}

} // namespace Python.
} // Namespace Kratos
