// KRATOS  ___|  |                   |                   |
//       \___ \  __|  __| |   |  __| __| |   |  __| _` | |
//             | |   |    |   | (    |   |   | |   (   | |
//       _____/ \__|_|   \__,_|\___|\__|\__,_|_|  \__,_|_| MECHANICS
//
//  License:		 BSD License
//					 license: structural_mechanics_application/license.txt
//
//  Main authors:    Philipp Bucher
//                   Natalia Saiapova
//

// System includes

// External includes

// Project includes
#include "testing/testing.h"
#include "containers/model.h"
#include "spaces/ublas_space.h"
#include "geometries/quadrilateral_2d_4.h"
#include "utilities/variable_utils.h"
#include "processes/structured_mesh_generator_process.h"

#include "custom_utilities/general_convergence_criteria.h"


/*
Things we will test:
- For both residual- & solutionupdate-based => here we can think if it is really necessary for both, maybe for one of them just one test if they have a common baseclass
    - For relative and absolute convergence
        - For different types of variables:
            - No Input (e.g. what the Displacement and the Residual Criterion do atm)
            - One Array3D + Double Variable (what VelPrCriteria does)
            - Two Double Variables
            - Two Array3D Variables (What the criteria in StrucutralMechanics do atm)
            - ... ?
=> makes at least 2*2*(4+x) tests
*/

namespace Kratos
{
namespace Testing
{
typedef UblasSpace<double, CompressedMatrix, Vector> SparseSpaceType;
typedef UblasSpace<double, Matrix, Vector> LocalSpaceType;


typedef typename SparseSpaceType::DataType TDataType;

typedef typename SparseSpaceType::MatrixType TSystemMatrixType;
typedef typename SparseSpaceType::VectorType TSystemVectorType;
typedef typename SparseSpaceType::MatrixPointerType TSystemMatrixPointerType;
typedef typename SparseSpaceType::VectorPointerType TSystemVectorPointerType;

typedef typename LocalSpaceType::MatrixType TLocalMatrixType;
typedef typename LocalSpaceType::VectorType TLocalVectorType;
typedef typename LocalSpaceType::MatrixPointerType TLocalMatrixPointerType;
typedef typename LocalSpaceType::VectorPointerType TLocalVectorPointerType;


typedef ConvergenceCriteria<SparseSpaceType, LocalSpaceType> ConvergenceCriteriaType;
typedef Kratos::unique_ptr<ConvergenceCriteriaType> ConvergenceCriteriaPointerType;

typedef GeneralConvergenceCriteria<SparseSpaceType, LocalSpaceType> GenConvergenceCriteriaType;
typedef Kratos::unique_ptr<GenConvergenceCriteriaType> GenConvergenceCriteriaPointerType;

typedef ModelPart::DofsArrayType DofsArrayType;

typedef Node<3> NodeType;
typedef Kratos::unique_ptr<NodeType> NodeUniquePointerType;

// This function initializes the test depending on the passed configuration
// TODO for now the dofs are hard-coded, this should be made variable
void SetUpTest(ModelPart& rModelPart,
                DofsArrayType& rDofSet,
                TSystemVectorType& rSystemVec_Dx,
                TSystemVectorType& rSystemVec_b,
                const std::size_t NumDivisions)
{
    rModelPart.AddNodalSolutionStepVariable(ROTATION);
    rModelPart.AddNodalSolutionStepVariable(VELOCITY);
    rModelPart.AddNodalSolutionStepVariable(PRESSURE);

    Node<3>::Pointer p_point1(new Node<3>(1, 0.00, 0.00, 0.00));
    Node<3>::Pointer p_point2(new Node<3>(2, 0.00, 10.00, 0.00));
    Node<3>::Pointer p_point3(new Node<3>(3, 10.00, 10.00, 0.00));
    Node<3>::Pointer p_point4(new Node<3>(4, 10.00, 0.00, 0.00));

    Quadrilateral2D4<Node<3> > geometry(p_point1, p_point2, p_point3, p_point4);

    Parameters mesher_parameters(R"(
    {
        "number_of_divisions" : 0,
        "element_name"        : "Element2D3N",
        "create_skin_sub_model_part": false
    }  )");

    mesher_parameters["number_of_divisions"].SetInt(NumDivisions);

    StructuredMeshGeneratorProcess(geometry, rModelPart, mesher_parameters).Execute();

    const std::size_t system_size = rModelPart.NumberOfNodes();

    KRATOS_INFO("System Size") << system_size << std::endl;

    for (auto& r_node : rModelPart.Nodes())
    {
        r_node.AddDof(ROTATION_X);
        r_node.AddDof(ROTATION_Y);
        r_node.AddDof(ROTATION_Z);
        r_node.AddDof(VELOCITY_X);
        r_node.AddDof(VELOCITY_Z);
        r_node.AddDof(VELOCITY_Y);
        r_node.AddDof(PRESSURE);
    }

    VariableUtils var_utils();
    // var_utils.AddDof(ROTATION, rModelPart);
    // VariableUtils().AddDof(VELOCITY, rModelPart);
    // VariableUtils().AddDof(PRESSURE, rModelPart);

    const std::size_t num_dofs_per_node = 7; // TODO hard coded for now, to be made variable

    std::vector< Dof<double>::Pointer > DoF;
    DoF.reserve(system_size * num_dofs_per_node);
    for (auto& r_node : rModelPart.Nodes())
    {
        DoF.push_back(r_node.pGetDof(ROTATION_X));
        DoF.push_back(r_node.pGetDof(ROTATION_Y));
        DoF.push_back(r_node.pGetDof(ROTATION_Z));
        DoF.push_back(r_node.pGetDof(VELOCITY_X));
        DoF.push_back(r_node.pGetDof(VELOCITY_Y));
        DoF.push_back(r_node.pGetDof(VELOCITY_Z));
        DoF.push_back(r_node.pGetDof(PRESSURE));
    }

    rDofSet.clear();
    rDofSet.reserve(DoF.size());
    for (auto it= DoF.begin(); it!= DoF.end(); it++)
        rDofSet.push_back( it->get() );

    rDofSet.Sort();

    // Initializing the Solution Vector
    SparseSpaceType::Resize(rSystemVec_Dx, system_size*num_dofs_per_node);
    SparseSpaceType::SetToZero(rSystemVec_Dx);
    SparseSpaceType::Resize(rSystemVec_b, system_size*num_dofs_per_node);
    SparseSpaceType::SetToZero(rSystemVec_b);
}

double computeError(int stepIndex)
{
    int k = 3;
    double eps = 1e-9;
    int start = 1024 * 1024 * 1024;

    // On the k-th step error is equal to:
    // start * 2 ^ (-ki) * eps / (i + 2)
    return ( start >> ( stepIndex * k ) ) * eps / ( stepIndex + 2 );
}

// This function sets the solution on the dofs and the system vectors
// Usually this would be done by setting up and solving the system
void SetSolution(DofsArrayType &rDofSet,
                    TSystemVectorType &rSystemVec_Dx,
                    TSystemVectorType &rSystemVec_b,
                    int stepIndex)
{
    double err = computeError(stepIndex);
    double errPrevious = computeError(stepIndex - 1);

    for (auto& r_dof : rDofSet)
    {
        // The ideal solution would be 1
        // Solution on k-th step is 1 + err
        r_dof.GetSolutionStepValue() = 1 + err;
    }

    const std::size_t system_size = SparseSpaceType::Size(rSystemVec_Dx);
    KRATOS_ERROR_IF(system_size != SparseSpaceType::Size(rSystemVec_b))
        << "System Vector sizes are inconsistent!" << std::endl;

    // We compute the displacement like this.
    // however, we probably can use this value for residuals as well, snce its just a decreasing series of numbers.
    // dx = err - start * 2 ^ ( -k * (i + 1) ) / (i + 1)) * eps
    double dx = err - errPrevious;
    for (std::size_t i = 0; i < system_size; ++i)
    {
        rSystemVec_Dx[i] = dx;
        rSystemVec_b[i] = dx;
    }
}

KRATOS_TEST_CASE_IN_SUITE(ConvergenceCriteriaAbsoluteVelocityRotation, KratosStructuralMechanicsFastSuite)
{
    DofsArrayType dofs_array;
    const std::size_t num_divisions = 10;

    TSystemMatrixPointerType p_dummy_system_matrix = SparseSpaceType::CreateEmptyMatrixPointer();
    TSystemVectorPointerType p_system_vector_b = SparseSpaceType::CreateEmptyVectorPointer();
    TSystemVectorPointerType p_system_vector_Dx = SparseSpaceType::CreateEmptyVectorPointer();

    TSystemMatrixType& r_system_matrix = *p_dummy_system_matrix;
    TSystemVectorType& r_system_vector_b = *p_system_vector_b;
    TSystemVectorType& r_system_vector_Dx = *p_system_vector_Dx;

    Model current_model;
    ModelPart& dummy_model_part = current_model.CreateModelPart("dummy");
    SetUpTest(dummy_model_part, dofs_array, r_system_vector_Dx, r_system_vector_b, num_divisions);

    const TDataType NewRatioTolerance = 1e-7;
    const TDataType AlwaysConvergedNorm = 1e-5;

    Parameters default_params( R"({
        "basis_vector_type" : "solution_update",
        "variables_to_separate" : ["VELOCITY", "ROTATION"],
        "relative_convergence_tolerances" : [1e-20, 1e-20],
        "absolut_convergence_tolerances" : [1e-6, 1e-8]
    })" );

    const std::string name_remaining_dofs = "PRESSURE";

    ConvergenceCriteriaPointerType p_conv_crit = Kratos::make_unique<GenConvergenceCriteriaType>(
        default_params);

    p_conv_crit->Initialize(dummy_model_part);

    bool is_converged = false;

    std::size_t counter = 0;
    std::size_t conv_iter = 10;
    std::size_t max_iter = 20;

    while( !is_converged && counter <= max_iter )
    {
        dummy_model_part.GetProcessInfo()[NL_ITERATION_NUMBER]++;

        SetSolution(dofs_array, r_system_vector_Dx, r_system_vector_b, counter);

        p_conv_crit->InitializeSolutionStep(dummy_model_part,
                                            dofs_array,
                                            r_system_matrix,
                                            r_system_vector_Dx,
                                            r_system_vector_b);

        is_converged = p_conv_crit->PostCriteria(dummy_model_part,
                                            dofs_array,
                                            r_system_matrix,
                                            r_system_vector_Dx,
                                            r_system_vector_b);

        counter += 1;
    }

    // check that convergence was achieved
    KRATOS_CHECK_EQUAL(counter, conv_iter);
}

KRATOS_TEST_CASE_IN_SUITE(ConvergenceCriteriaAbsolutePressure, KratosStructuralMechanicsFastSuite)
{
    DofsArrayType dofs_array;
    const std::size_t num_divisions = 10;

    TSystemMatrixPointerType p_dummy_system_matrix = SparseSpaceType::CreateEmptyMatrixPointer();
    TSystemVectorPointerType p_system_vector_b = SparseSpaceType::CreateEmptyVectorPointer();
    TSystemVectorPointerType p_system_vector_Dx = SparseSpaceType::CreateEmptyVectorPointer();

    TSystemMatrixType& r_system_matrix = *p_dummy_system_matrix;
    TSystemVectorType& r_system_vector_b = *p_system_vector_b;
    TSystemVectorType& r_system_vector_Dx = *p_system_vector_Dx;

    Model current_model;
    ModelPart& dummy_model_part = current_model.CreateModelPart("dummy");
    SetUpTest(dummy_model_part, dofs_array, r_system_vector_Dx, r_system_vector_b, num_divisions);

    const TDataType NewRatioTolerance = 1e-7;
    const TDataType AlwaysConvergedNorm = 1e-5;

    Parameters default_params( R"({
        "basis_vector_type" : "solution_update",
        "variables_to_separate" : ["PRESSURE"],
        "relative_convergence_tolerances" : [1e-20],
        "absolut_convergence_tolerances" : [1e-6]
    })" );

    const std::string name_remaining_dofs = "VELOCITY_AND_ROTATION";

    ConvergenceCriteriaPointerType p_conv_crit = Kratos::make_unique<GenConvergenceCriteriaType>(
        default_params);

    p_conv_crit->Initialize(dummy_model_part);

    bool is_converged = false;

    std::size_t counter = 0;
    std::size_t conv_iter = 8;
    std::size_t max_iter = 20;

    while( !is_converged && counter <= max_iter )
    {
        dummy_model_part.GetProcessInfo()[NL_ITERATION_NUMBER]++;

        SetSolution(dofs_array, r_system_vector_Dx, r_system_vector_b, counter);

        p_conv_crit->InitializeSolutionStep(dummy_model_part,
                                            dofs_array,
                                            r_system_matrix,
                                            r_system_vector_Dx,
                                            r_system_vector_b);

        is_converged = p_conv_crit->PostCriteria(dummy_model_part,
                                            dofs_array,
                                            r_system_matrix,
                                            r_system_vector_Dx,
                                            r_system_vector_b);

        counter += 1;
    }

    // check that convergence was achieved
    KRATOS_CHECK_EQUAL(counter, conv_iter);
}

KRATOS_TEST_CASE_IN_SUITE(ConvergenceCriteriaAbsoluteNoSeparate, KratosStructuralMechanicsFastSuite)
{
    DofsArrayType dofs_array;
    const std::size_t num_divisions = 10;

    TSystemMatrixPointerType p_dummy_system_matrix = SparseSpaceType::CreateEmptyMatrixPointer();
    TSystemVectorPointerType p_system_vector_b = SparseSpaceType::CreateEmptyVectorPointer();
    TSystemVectorPointerType p_system_vector_Dx = SparseSpaceType::CreateEmptyVectorPointer();

    TSystemMatrixType& r_system_matrix = *p_dummy_system_matrix;
    TSystemVectorType& r_system_vector_b = *p_system_vector_b;
    TSystemVectorType& r_system_vector_Dx = *p_system_vector_Dx;

    Model current_model;
    ModelPart& dummy_model_part = current_model.CreateModelPart("dummy");
    SetUpTest(dummy_model_part, dofs_array, r_system_vector_Dx, r_system_vector_b, num_divisions);

    const TDataType NewRatioTolerance = 1e-7;
    const TDataType AlwaysConvergedNorm = 1e-5;

    Parameters default_params( R"({
        "basis_vector_type" : "solution_update",
        "variables_to_separate" : [],
        "relative_convergence_tolerances" : [],
        "absolut_convergence_tolerances" : []
    })" );

    ConvergenceCriteriaPointerType p_conv_crit = Kratos::make_unique<GenConvergenceCriteriaType>(
        default_params);

    p_conv_crit->Initialize(dummy_model_part);

    bool is_converged = false;

    std::size_t counter = 0;
    std::size_t conv_iter = 7;
    std::size_t max_iter = 20;

    while( !is_converged && counter <= max_iter )
    {
        dummy_model_part.GetProcessInfo()[NL_ITERATION_NUMBER]++;

        SetSolution(dofs_array, r_system_vector_Dx, r_system_vector_b, counter);
        p_conv_crit->InitializeSolutionStep(dummy_model_part,
                                            dofs_array,
                                            r_system_matrix,
                                            r_system_vector_Dx,
                                            r_system_vector_b);

        is_converged = p_conv_crit->PostCriteria(dummy_model_part,
                                            dofs_array,
                                            r_system_matrix,
                                            r_system_vector_Dx,
                                            r_system_vector_b);

        counter += 1;
    }

    KRATOS_CHECK_EQUAL(counter, conv_iter);
}

KRATOS_TEST_CASE_IN_SUITE(ConvergenceCriteriaRelativeVelocityRotation, KratosStructuralMechanicsFastSuite)
{
    DofsArrayType dofs_array;
    const std::size_t num_divisions = 10;

    TSystemMatrixPointerType p_dummy_system_matrix = SparseSpaceType::CreateEmptyMatrixPointer();
    TSystemVectorPointerType p_system_vector_b = SparseSpaceType::CreateEmptyVectorPointer();
    TSystemVectorPointerType p_system_vector_Dx = SparseSpaceType::CreateEmptyVectorPointer();

    TSystemMatrixType& r_system_matrix = *p_dummy_system_matrix;
    TSystemVectorType& r_system_vector_b = *p_system_vector_b;
    TSystemVectorType& r_system_vector_Dx = *p_system_vector_Dx;

    Model current_model;
    ModelPart& dummy_model_part = current_model.CreateModelPart("dummy");
    SetUpTest(dummy_model_part, dofs_array, r_system_vector_Dx, r_system_vector_b, num_divisions);

    const TDataType NewRatioTolerance = 1e-7;
    const TDataType AlwaysConvergedNorm = 1e-5;

    Parameters default_params( R"({
        "basis_vector_type" : "solution_update",
        "variables_to_separate" : ["VELOCITY", "ROTATION"],
        "relative_convergence_tolerances" : [1e-6, 1e-8],
        "absolut_convergence_tolerances" : [1e-20, 1e-20]
    })" );

    const std::string name_remaining_dofs = "PRESSURE";

    ConvergenceCriteriaPointerType p_conv_crit = Kratos::make_unique<GenConvergenceCriteriaType>(
        default_params);

    p_conv_crit->Initialize(dummy_model_part);

    bool is_converged = false;

    std::size_t counter = 0;
    std::size_t conv_iter = 10;
    std::size_t max_iter = 20;

    while( !is_converged && counter <= max_iter )
    {
        dummy_model_part.GetProcessInfo()[NL_ITERATION_NUMBER]++;

        SetSolution(dofs_array, r_system_vector_Dx, r_system_vector_b, counter);
        p_conv_crit->InitializeSolutionStep(dummy_model_part,
                                            dofs_array,
                                            r_system_matrix,
                                            r_system_vector_Dx,
                                            r_system_vector_b);

        is_converged = p_conv_crit->PostCriteria(dummy_model_part,
                                            dofs_array,
                                            r_system_matrix,
                                            r_system_vector_Dx,
                                            r_system_vector_b);

        counter += 1;
    }
    KRATOS_CHECK_EQUAL(counter, conv_iter);
}

KRATOS_TEST_CASE_IN_SUITE(ConvergenceCriteriaRelativeNoSeparate, KratosStructuralMechanicsFastSuite)
{
    DofsArrayType dofs_array;
    const std::size_t num_divisions = 10;

    TSystemMatrixPointerType p_dummy_system_matrix = SparseSpaceType::CreateEmptyMatrixPointer();
    TSystemVectorPointerType p_system_vector_b = SparseSpaceType::CreateEmptyVectorPointer();
    TSystemVectorPointerType p_system_vector_Dx = SparseSpaceType::CreateEmptyVectorPointer();

    TSystemMatrixType& r_system_matrix = *p_dummy_system_matrix;
    TSystemVectorType& r_system_vector_b = *p_system_vector_b;
    TSystemVectorType& r_system_vector_Dx = *p_system_vector_Dx;

    Model current_model;
    ModelPart& dummy_model_part = current_model.CreateModelPart("dummy");
    SetUpTest(dummy_model_part, dofs_array, r_system_vector_Dx, r_system_vector_b, num_divisions);

    const TDataType NewRatioTolerance = 1e-5;
    const TDataType AlwaysConvergedNorm = 1e-7;

    Parameters default_params( R"({
        "basis_vector_type" : "solution_update",
        "variables_to_separate" : [],
        "relative_convergence_tolerances" : [],
        "absolut_convergence_tolerances" : []
    })" );


    ConvergenceCriteriaPointerType p_conv_crit = Kratos::make_unique<GenConvergenceCriteriaType>(
        default_params);

    p_conv_crit->Initialize(dummy_model_part);

    bool is_converged = false;

    std::size_t counter = 0;
    std::size_t conv_iter = 7;
    std::size_t max_iter = 20;

    while( !is_converged && counter <= max_iter )
    {
        dummy_model_part.GetProcessInfo()[NL_ITERATION_NUMBER]++;

        SetSolution(dofs_array, r_system_vector_Dx, r_system_vector_b, counter);
        p_conv_crit->InitializeSolutionStep(dummy_model_part,
                                            dofs_array,
                                            r_system_matrix,
                                            r_system_vector_Dx,
                                            r_system_vector_b);

        is_converged = p_conv_crit->PostCriteria(dummy_model_part,
                                            dofs_array,
                                            r_system_matrix,
                                            r_system_vector_Dx,
                                            r_system_vector_b);

        counter += 1;
    }
    KRATOS_CHECK_EQUAL(counter, conv_iter);
}
KRATOS_TEST_CASE_IN_SUITE(ConvergenceCriteriaRelativePressure, KratosStructuralMechanicsFastSuite)
{
    DofsArrayType dofs_array;
    const std::size_t num_divisions = 10;

    TSystemMatrixPointerType p_dummy_system_matrix = SparseSpaceType::CreateEmptyMatrixPointer();
    TSystemVectorPointerType p_system_vector_b = SparseSpaceType::CreateEmptyVectorPointer();
    TSystemVectorPointerType p_system_vector_Dx = SparseSpaceType::CreateEmptyVectorPointer();

    TSystemMatrixType& r_system_matrix = *p_dummy_system_matrix;
    TSystemVectorType& r_system_vector_b = *p_system_vector_b;
    TSystemVectorType& r_system_vector_Dx = *p_system_vector_Dx;

    Model current_model;
    ModelPart& dummy_model_part = current_model.CreateModelPart("dummy");
    SetUpTest(dummy_model_part, dofs_array, r_system_vector_Dx, r_system_vector_b, num_divisions);

    const TDataType NewRatioTolerance = 1e-7;
    const TDataType AlwaysConvergedNorm = 1e-5;

    Parameters default_params( R"({
        "basis_vector_type" : "solution_update",
        "variables_to_separate" : ["PRESSURE"],
        "relative_convergence_tolerances" : [1e-6],
        "absolut_convergence_tolerances" : [1e-20]
    })" );

    const std::string name_remaining_dofs = "VELOCITY_AND_ROTATION";


    ConvergenceCriteriaPointerType p_conv_crit = Kratos::make_unique<GenConvergenceCriteriaType>(
        default_params);

    p_conv_crit->Initialize(dummy_model_part);

    bool is_converged = false;

    std::size_t counter = 0;
    std::size_t conv_iter = 8;
    std::size_t max_iter = 20;

    while( !is_converged && counter <= max_iter )
    {
        dummy_model_part.GetProcessInfo()[NL_ITERATION_NUMBER]++;

        SetSolution(dofs_array, r_system_vector_Dx, r_system_vector_b, counter);

        p_conv_crit->InitializeSolutionStep(dummy_model_part,
                                            dofs_array,
                                            r_system_matrix,
                                            r_system_vector_Dx,
                                            r_system_vector_b);

        is_converged = p_conv_crit->PostCriteria(dummy_model_part,
                                            dofs_array,
                                            r_system_matrix,
                                            r_system_vector_Dx,
                                            r_system_vector_b);

        counter += 1;
    }

    // check that convergence was achieved
    KRATOS_CHECK_EQUAL(counter, conv_iter);
}

} // namespace Testing
}  // namespace Kratos.
