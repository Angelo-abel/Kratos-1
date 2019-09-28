// System includes

#ifdef KRATOS_USE_AMATRIX
#include "boost/numeric/ublas/matrix.hpp" // for the sparse space dense vector
#endif                                    // KRATOS_USE_AMATRIX

// External includes
#include "pybind11/pybind11.h"

// Project includes
#include "includes/model_part.h"
#include "linear_solvers/linear_solver.h"
#include "spaces/ublas_space.h"

#ifdef KRATOS_USING_MPI // mpi-parallel compilation
#include "Epetra_FEVector.h"
#include "trilinos_space.h"
#endif

#include "custom_python/add_custom_solving_processes_to_python.h"

// Application includes
#include "custom_processes/solving_strategies/k_epsilon_co_solving_process.h"
#include "custom_processes/solving_strategies/scalar_co_solving_process.h"

namespace Kratos
{
namespace Python
{
void AddCustomSolvingProcessesToPython(pybind11::module& m)
{
    namespace py = pybind11;

    using SparseSpaceType = UblasSpace<double, CompressedMatrix, Vector>;
    using LocalSpaceType = UblasSpace<double, Matrix, Vector>;
    using LinearSolverType = LinearSolver<SparseSpaceType, LocalSpaceType>;

    // Adding solving strategies
    using ScalarCoSolvingProcessType =
        ScalarCoSolvingProcess<SparseSpaceType, LocalSpaceType, LinearSolverType>;
    py::class_<ScalarCoSolvingProcessType, ScalarCoSolvingProcessType::Pointer, Process>(
        m, "ScalarCoSolvingProcess")
        .def(py::init<ModelPart&, Parameters&, Variable<double>&>())
        .def("AddStrategy", &ScalarCoSolvingProcessType::AddStrategy)
        .def("AddAuxiliaryProcess", &ScalarCoSolvingProcessType::AddAuxiliaryProcess);

    using KEpsilonCoSolvingProcessType =
        KEpsilonCoSolvingProcess<SparseSpaceType, LocalSpaceType, LinearSolverType>;
    py::class_<KEpsilonCoSolvingProcessType, KEpsilonCoSolvingProcessType::Pointer, ScalarCoSolvingProcessType, Process>(
        m, "KEpsilonCoSolvingProcess")
        .def(py::init<ModelPart&, Parameters&>());

#ifdef KRATOS_USING_MPI // mpi-parallel compilation
    using MPISparseSpaceType = TrilinosSpace<Epetra_FECrsMatrix, Epetra_FEVector>;
    using MPILinearSolverType = LinearSolver<MPISparseSpaceType, LocalSpaceType>;

    using MPIScalarCoSolvingProcessType =
        ScalarCoSolvingProcess<MPISparseSpaceType, LocalSpaceType, MPILinearSolverType>;
    py::class_<MPIScalarCoSolvingProcessType, MPIScalarCoSolvingProcessType::Pointer, Process>(
        m, "MPIScalarCoSolvingProcess")
        .def(py::init<ModelPart&, Parameters&, Variable<double>&>())
        .def("AddStrategy", &MPIScalarCoSolvingProcessType::AddStrategy)
        .def("AddAuxiliaryProcess", &MPIScalarCoSolvingProcessType::AddAuxiliaryProcess);

    using MPIKEpsilonCoSolvingProcessType =
        KEpsilonCoSolvingProcess<MPISparseSpaceType, LocalSpaceType, MPILinearSolverType>;
    py::class_<MPIKEpsilonCoSolvingProcessType, MPIKEpsilonCoSolvingProcessType::Pointer, MPIScalarCoSolvingProcessType, Process>(
        m, "MPIKEpsilonCoSolvingProcess")
        .def(py::init<ModelPart&, Parameters&>());
#endif
}

} // namespace Python.
} // Namespace Kratos
