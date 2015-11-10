/*
==============================================================================
Kratos
A General Purpose Software for Multi-Physics Finite Element Analysis
Version 1.0 (Released on march 05, 2007).

Copyright 2007
Pooyan Dadvand, Riccardo Rossi
pooyan@cimne.upc.edu
rrossi@cimne.upc.edu
CIMNE (International Center for Numerical Methods in Engineering),
Gran Capita' s/n, 08034 Barcelona, Spain

Permission is hereby granted, free  of charge, to any person obtaining
a  copy  of this  software  and  associated  documentation files  (the
"Software"), to  deal in  the Software without  restriction, including
without limitation  the rights to  use, copy, modify,  merge, publish,
distribute,  sublicense and/or  sell copies  of the  Software,  and to
permit persons to whom the Software  is furnished to do so, subject to
the following condition:

Distribution of this code for  any  commercial purpose  is permissible
ONLY BY DIRECT ARRANGEMENT WITH THE COPYRIGHT OWNER.

The  above  copyright  notice  and  this permission  notice  shall  be
included in all copies or substantial portions of the Software.

THE  SOFTWARE IS  PROVIDED  "AS  IS", WITHOUT  WARRANTY  OF ANY  KIND,
EXPRESS OR  IMPLIED, INCLUDING  BUT NOT LIMITED  TO THE  WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT  SHALL THE AUTHORS OR COPYRIGHT HOLDERS  BE LIABLE FOR ANY
CLAIM, DAMAGES OR  OTHER LIABILITY, WHETHER IN AN  ACTION OF CONTRACT,
TORT  OR OTHERWISE, ARISING  FROM, OUT  OF OR  IN CONNECTION  WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

==============================================================================
*/

//
//   Project Name:        Kratos
//   Last modified by:    $Author: mengmeng $
//   Date:                $Date: 2009-02-26 14:28:21 $
//   Revision:            $Revision: 1.13 $
//
//


// System includes

// External includes
#include <boost/python.hpp>


// Project includes
#include "includes/define.h"
#include "includes/ublas_interface.h"
#include "containers/data_value_container.h"
//#include "containers/hash_data_value_container.h"
#include "containers/variables_list_data_value_container.h"
#include "containers/fix_data_value_container.h"
#include "containers/vector_component_adaptor.h"
#include "containers/flags.h"
//#include "containers/all_variables_data_value_container.h"
// #include "includes/kratos_flags.h"
#include "includes/deprecated_variables.h"
#include "includes/constitutive_law.h"
#include "python/variable_indexing_python.h"
#include "python/vector_python_interface.h"
#include "python/vector_scalar_operator_python.h"
#include "python/vector_vector_operator_python.h"
#include "python/bounded_vector_python_interface.h"

#include "includes/convection_diffusion_settings.h"
#include "includes/radiation_settings.h"
#include "utilities/timer.h"



#ifdef KRATOS_REGISTER_IN_PYTHON_FLAG_IMPLEMENTATION
#undef KRATOS_REGISTER_IN_PYTHON_FLAG_IMPLEMENTATION
#endif
#define KRATOS_REGISTER_IN_PYTHON_FLAG_IMPLEMENTATION(flag) \
 scope().attr(#flag) = boost::ref(flag)      \

#ifdef KRATOS_REGISTER_IN_PYTHON_FLAG
#undef KRATOS_REGISTER_IN_PYTHON_FLAG
#endif
#define KRATOS_REGISTER_IN_PYTHON_FLAG(flag) \
    KRATOS_REGISTER_IN_PYTHON_FLAG_IMPLEMENTATION(flag);   \
    KRATOS_REGISTER_IN_PYTHON_FLAG_IMPLEMENTATION(NOT_##flag)

#ifdef KRATOS_REGISTER_IN_PYTHON_VARIABLE
#undef KRATOS_REGISTER_IN_PYTHON_VARIABLE
#endif
#define KRATOS_REGISTER_IN_PYTHON_VARIABLE(variable) \
    scope().attr(#variable) = boost::ref(variable);


#ifdef KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS
#undef KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS
#endif
#define KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS(name) \
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(name) \
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(name##_X) \
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(name##_Y) \
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(name##_Z)

namespace Kratos
{
//KRATOS_CREATE_FLAG(STRUCTURE,   63);

namespace Python
{
using namespace boost::python;

void  AddDeprecatedVariablesToPython()
{
          
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(  IS_INACTIVE )

    //for Level Set application:
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(  IS_DUPLICATED )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(  SPLIT_ELEMENT )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(  SPLIT_NODAL )

    KRATOS_REGISTER_IN_PYTHON_VARIABLE( IS_CONTACT_MASTER )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( IS_CONTACT_SLAVE )
    //for PFEM fluids application:
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( IS_JACK_LINK )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( IMPOSED_PRESSURE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( IMPOSED_VELOCITY_X )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( IMPOSED_VELOCITY_Y )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( IMPOSED_VELOCITY_Z )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( IMPOSED_ANGULAR_VELOCITY_X )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( IMPOSED_ANGULAR_VELOCITY_Y )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE( IMPOSED_ANGULAR_VELOCITY_Z )

    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   IS_INLET )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE (   IS_INTERFACE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE (   IS_VISITED )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE ( IS_EROSIONABLE )

    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   IS_STRUCTURE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   IS_POROUS )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   IS_WATER )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   IS_FLUID )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   IS_BOUNDARY )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   IS_FREE_SURFACE )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   IS_AIR_EXIT )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   IS_LAGRANGIAN_INLET )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   IS_WATER_ELEMENT )


    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   IS_BURN )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   IS_DRIPPING )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   IS_PERMANENT )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   IS_WALL )

    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   Ypr ) //var name does not follow standard
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   Yox )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   Yfuel )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   Hfuel )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   Hpr )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   Hpr1 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   Hox )

    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   RADIATIVE_INTENSITY )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   RADIATIVE_INTENSITY_1 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   RADIATIVE_INTENSITY_2 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   RADIATIVE_INTENSITY_3 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   RADIATIVE_INTENSITY_4 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   RADIATIVE_INTENSITY_5 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   RADIATIVE_INTENSITY_6 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   RADIATIVE_INTENSITY_7 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   RADIATIVE_INTENSITY_8 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   RADIATIVE_INTENSITY_9 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   RADIATIVE_INTENSITY_10 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   RADIATIVE_INTENSITY_11 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   RADIATIVE_INTENSITY_12 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   RADIATIVE_INTENSITY_13 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   RADIATIVE_INTENSITY_14 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   RADIATIVE_INTENSITY_15 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   RADIATIVE_INTENSITY_16 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   RADIATIVE_INTENSITY_17 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   RADIATIVE_INTENSITY_18 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   RADIATIVE_INTENSITY_19 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   RADIATIVE_INTENSITY_20 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   RADIATIVE_INTENSITY_21 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   RADIATIVE_INTENSITY_22 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   RADIATIVE_INTENSITY_23 )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   RADIATIVE_INTENSITY_24 )

    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   rhoD )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   xi )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   a )
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   b )


    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   IS_SLIP )

    //for Level Set application:
    KRATOS_REGISTER_IN_PYTHON_VARIABLE(   IS_DIVIDED )

    KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS( xi_c )

}
}  // namespace Python.
} // Namespace Kratos

#undef KRATOS_REGISTER_IN_PYTHON_FLAG
#undef KRATOS_REGISTER_IN_PYTHON_VARIABLE
#undef KRATOS_REGISTER_IN_PYTHON_3D_VARIABLE_WITH_COMPONENTS
