// KRATOS  ___|  |                   |                   |
//       \___ \  __|  __| |   |  __| __| |   |  __| _` | |
//             | |   |    |   | (    |   |   | |   (   | |
//       _____/ \__|_|   \__,_|\___|\__|\__,_|_|  \__,_|_| MECHANICS
//
//  License:         BSD License
//                     license:
// structural_mechanics_application/license.txt
//
//  Main authors:    Riccardo Rossi
//

#if !defined(KRATOS_STRUCTURAL_MECHANICS_APPLICATION_VARIABLES_H_INCLUDED)
#define KRATOS_STRUCTURAL_MECHANICS_APPLICATION_VARIABLES_H_INCLUDED

// System includes

// External includes

// Project includes
#include "includes/define.h"
#include "custom_utilities/shell_cross_section.hpp"
#include "includes/mat_variables.h"

namespace Kratos
{
///@name Kratos Globals
///@{

///@}
///@name Type Definitions
///@{

    typedef array_1d<double, 3> Vector3;

///@}
///@name  Enum's
///@{

    enum class SofteningType {Linear = 0, Exponential = 1};

///@}
///@name  Functions
///@{
    // General pourpose
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, int, INTEGRATION_ORDER); // The integration order considered on the element
    KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS(STRUCTURAL_MECHANICS_APPLICATION, LOCAL_MATERIAL_AXIS_1)
    KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS(STRUCTURAL_MECHANICS_APPLICATION, LOCAL_MATERIAL_AXIS_2)
    KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS(STRUCTURAL_MECHANICS_APPLICATION, LOCAL_MATERIAL_AXIS_3)
    KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS(STRUCTURAL_MECHANICS_APPLICATION, CENTER_OF_GRAVITY)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, double, MASS_MOMENT_OF_INERTIA )

    // Generalized eigenvalue problem
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, int, BUILD_LEVEL )
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, Vector, EIGENVALUE_VECTOR)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, Matrix , EIGENVECTOR_MATRIX )

    // Geometrical
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, double, AREA )
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, double, IT )
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, double, IY )
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, double, IZ )
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, double, CROSS_AREA )
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, double, MEAN_RADIUS )
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, int,    SECTION_SIDES )
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, Matrix , GEOMETRIC_STIFFNESS )
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, Matrix, LOCAL_ELEMENT_ORIENTATION)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION,double, MATERIAL_ORIENTATION_ANGLE)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION,bool, USE_CONSISTENT_MASS_MATRIX)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION,Vector, CONDENSED_DOF_LIST)

    // Truss generalized variables
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION,double, TRUSS_PRESTRESS_PK2)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION,double, HARDENING_MODULUS_1D)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION,double, TANGENT_MODULUS)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION,double, PLASTIC_ALPHA);

    // Beam generalized variables
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION,double, AREA_EFFECTIVE_Y)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION,double, AREA_EFFECTIVE_Z)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION,double, INERTIA_ROT_Y)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION,double, INERTIA_ROT_Z)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION,Vector, LOCAL_AXES_VECTOR)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION,double, TORSIONAL_INERTIA)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION,double, I22)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION,double, I33)

    // Shell generalized variables
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, bool, STENBERG_SHEAR_STABILIZATION_SUITABLE )
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, double, SHELL_OFFSET)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, Matrix, SHELL_STRAIN )
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, Matrix, SHELL_STRAIN_GLOBAL )
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, Matrix, SHELL_CURVATURE )
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, Matrix, SHELL_CURVATURE_GLOBAL )
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, Matrix, SHELL_FORCE )
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, Matrix, SHELL_FORCE_GLOBAL )
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, Matrix, SHELL_MOMENT )
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, Matrix, SHELL_MOMENT_GLOBAL )
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, Matrix, SHELL_STRESS_TOP_SURFACE)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, Matrix, SHELL_STRESS_TOP_SURFACE_GLOBAL)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, Matrix, SHELL_STRESS_MIDDLE_SURFACE)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, Matrix, SHELL_STRESS_MIDDLE_SURFACE_GLOBAL)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, Matrix, SHELL_STRESS_BOTTOM_SURFACE)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, Matrix, SHELL_STRESS_BOTTOM_SURFACE_GLOBAL)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, double, VON_MISES_STRESS_TOP_SURFACE)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, double, VON_MISES_STRESS_MIDDLE_SURFACE)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, double, VON_MISES_STRESS_BOTTOM_SURFACE)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, double, SHEAR_ANGLE)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, Matrix, SHELL_ORTHOTROPIC_STRESS_BOTTOM_SURFACE)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, Matrix, SHELL_ORTHOTROPIC_STRESS_TOP_SURFACE)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, Matrix, SHELL_ORTHOTROPIC_STRESS_BOTTOM_SURFACE_GLOBAL)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, Matrix, SHELL_ORTHOTROPIC_STRESS_TOP_SURFACE_GLOBAL)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, Matrix, SHELL_ORTHOTROPIC_4PLY_THROUGH_THICKNESS)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, double, TSAI_WU_RESERVE_FACTOR)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, Matrix, SHELL_ORTHOTROPIC_LAMINA_STRENGTHS)

    // Membrane generalized variables
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, Matrix, MEMBRANE_PRESTRESS)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, Vector, PRESTRESS_VECTOR)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, Vector, PRESTRESS_AXIS_1_GLOBAL)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, Vector, PRESTRESS_AXIS_2_GLOBAL)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, Matrix, PRESTRESS_AXIS_1)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, Matrix, PRESTRESS_AXIS_2)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, std::string, PROJECTION_TYPE_COMBO)

    //Formfinding
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, double, LAMBDA_MAX)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, bool, IS_FORMFINDING)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, Matrix, BASE_REF_1)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, Matrix, BASE_REF_2)
    // Shell energies
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, double, SHELL_ELEMENT_MEMBRANE_ENERGY)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, double, SHELL_ELEMENT_BENDING_ENERGY)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, double, SHELL_ELEMENT_SHEAR_ENERGY)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, double, SHELL_ELEMENT_MEMBRANE_ENERGY_FRACTION)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, double, SHELL_ELEMENT_BENDING_ENERGY_FRACTION)
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, double, SHELL_ELEMENT_SHEAR_ENERGY_FRACTION)

    // Cross section
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, ShellCrossSection::Pointer, SHELL_CROSS_SECTION )
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, int,          SHELL_CROSS_SECTION_OUTPUT_PLY_ID )
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, double,    SHELL_CROSS_SECTION_OUTPUT_PLY_LOCATION )
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, Matrix, SHELL_ORTHOTROPIC_LAYERS)

    // Nodal stiffness for the nodal concentrated element
    KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS(STRUCTURAL_MECHANICS_APPLICATION, NODAL_STIFFNESS )
    KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS(STRUCTURAL_MECHANICS_APPLICATION, NODAL_DAMPING_RATIO)

    // For explicit central difference scheme
    KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS(STRUCTURAL_MECHANICS_APPLICATION, MIDDLE_VELOCITY)
    KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS(STRUCTURAL_MECHANICS_APPLICATION, MIDDLE_ANGULAR_VELOCITY)
    KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS(STRUCTURAL_MECHANICS_APPLICATION, NODAL_INERTIA)

    // CONDITIONS
    /* Beam conditions */
    KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS(STRUCTURAL_MECHANICS_APPLICATION, POINT_MOMENT )

    // Adding the SPRISM EAS variables
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION,double, ALPHA_EAS);
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION,bool, CONSIDER_IMPLICIT_EAS_SPRISM_ELEMENT);
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION,bool, CONSIDER_TOTAL_LAGRANGIAN_SPRISM_ELEMENT);

    // Adding the SPRISM additional variables
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION,double, ANG_ROT); // TODO: Transform into a vector

    // Adding the SPRISM variable to deactivate the quadratic interpolation
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION,bool, CONSIDER_QUADRATIC_SPRISM_ELEMENT);

    // Additional strain measures
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, Vector, HENCKY_STRAIN_VECTOR);
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, Matrix, HENCKY_STRAIN_TENSOR);

    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, double, VON_MISES_STRESS )

    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, Matrix, REFERENCE_DEFORMATION_GRADIENT);
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, double, REFERENCE_DEFORMATION_GRADIENT_DETERMINANT);

    // Rayleigh variables
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, double, RAYLEIGH_ALPHA )
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, double, RAYLEIGH_BETA )

    // System damping
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, double, SYSTEM_DAMPING_RATIO )

    // Nodal load variables
    KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS( STRUCTURAL_MECHANICS_APPLICATION, POINT_LOAD )
    KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS( STRUCTURAL_MECHANICS_APPLICATION, LINE_LOAD )
    KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS( STRUCTURAL_MECHANICS_APPLICATION, SURFACE_LOAD )

    // Condition load variables
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, Vector, POINT_LOADS_VECTOR )
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, Vector, LINE_LOADS_VECTOR )
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, Vector, SURFACE_LOADS_VECTOR )
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, Vector, POSITIVE_FACE_PRESSURES_VECTOR )
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, Vector, NEGATIVE_FACE_PRESSURES_VECTOR )

    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, bool, INELASTIC_FLAG)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, double, INFINITY_YIELD_STRESS)

    // Response function variables
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, double, RESPONSE_VALUE)

    // Constitutive laws variables
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, double, YIELD_STRESS_TENSION)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, Vector, PLASTIC_STRAIN_VECTOR)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, Matrix, PLASTIC_DEFORMATION_GRADIENT)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, double, YIELD_STRESS_COMPRESSION)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, double, DILATANCY_ANGLE)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, int, SOFTENING_TYPE)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, int, HARDENING_CURVE)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, double, VISCOUS_PARAMETER)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, double, DELAY_TIME)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, double, MAXIMUM_STRESS)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, double, MAXIMUM_STRESS_POSITION)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, double, UNIAXIAL_STRESS)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, double, FRICTION_ANGLE)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, double, DAMAGE)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, double, THRESHOLD)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, Matrix, INTEGRATED_STRESS_TENSOR)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, Matrix, PLASTIC_STRAIN_TENSOR)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, double, DAMAGE_TENSION)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, double, DAMAGE_COMPRESSION)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, double, THRESHOLD_TENSION)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, double, THRESHOLD_COMPRESSION)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, double, UNIAXIAL_STRESS_TENSION)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, double, UNIAXIAL_STRESS_COMPRESSION)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, double,FRACTURE_ENERGY_COMPRESSION)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, Vector, CURVE_FITTING_PARAMETERS)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, Vector, PLASTIC_STRAIN_INDICATORS)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, double, EQUIVALENT_PLASTIC_STRAIN)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, Vector, KINEMATIC_PLASTICITY_PARAMETERS)
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, int, KINEMATIC_HARDENING_TYPE)

    // Adjoint variables
    KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS( STRUCTURAL_MECHANICS_APPLICATION, ADJOINT_DISPLACEMENT )
    KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS( STRUCTURAL_MECHANICS_APPLICATION, ADJOINT_ROTATION )
    KRATOS_DEFINE_APPLICATION_VARIABLE( STRUCTURAL_MECHANICS_APPLICATION, double, PERTURBATION_SIZE )

    // Variables for output of adjoint sensitivities
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, double, CROSS_AREA_SENSITIVITY );
    KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS( STRUCTURAL_MECHANICS_APPLICATION, POINT_LOAD_SENSITIVITY );
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, double, I22_SENSITIVITY );
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, double, I33_SENSITIVITY );
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, double, THICKNESS_SENSITIVITY );
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, double, YOUNG_MODULUS_SENSITIVITY );
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, double, AREA_EFFECTIVE_Y_SENSITIVITY );
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, double, AREA_EFFECTIVE_Z_SENSITIVITY );
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, bool, IS_ADJOINT );
    KRATOS_DEFINE_3D_APPLICATION_VARIABLE_WITH_COMPONENTS( STRUCTURAL_MECHANICS_APPLICATION, SHAPE );

	// Variables to for computing parts of sensitivity analysis
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, int, TRACED_STRESS_TYPE);
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, Matrix, STRESS_DISP_DERIV_ON_NODE);
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, Matrix, STRESS_DISP_DERIV_ON_GP);
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, Matrix, STRESS_DESIGN_DERIVATIVE_ON_NODE);
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, Matrix, STRESS_DESIGN_DERIVATIVE_ON_GP);
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, Vector, STRESS_ON_GP );
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, Vector, STRESS_ON_NODE );
    KRATOS_DEFINE_APPLICATION_VARIABLE(STRUCTURAL_MECHANICS_APPLICATION, std::string, DESIGN_VARIABLE_NAME);
}

#endif /* KRATOS_STRUCTURAL_MECHANICS_APPLICATION_VARIABLES_H_INCLUDED */
