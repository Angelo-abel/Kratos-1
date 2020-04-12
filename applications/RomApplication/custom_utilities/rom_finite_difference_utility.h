//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Altug Emiroglu, https://github.com/emiroglu
//

#if !defined(ROM_FINITE_DIFFERENCE_UTILITY_H_INCLUDED )
#define  ROM_FINITE_DIFFERENCE_UTILITY_H_INCLUDED

// System includes

// External includes

// Project includes
#include "includes/define.h"
#include "includes/element.h"

namespace Kratos
{

/** \brief RomFiniteDifferenceUtility
 *
 * This class calculates the derivatives of different element quantities (e.g. RHS, LHS, mass-matrix, ...)
 * with respect to a design variable (e.g. nodal-coordinate, property).
 */


class KRATOS_API(ROM_APPLICATION) RomFiniteDifferenceUtility
{
public:

    KRATOS_CLASS_POINTER_DEFINITION(RomFiniteDifferenceUtility);

    static void CalculateLeftHandSideDOFDerivative(Element& rElement,
                                                Dof<double>& rDof,
                                                const double& rPertubationSize,
                                                Matrix& rOutput,
                                                ProcessInfo& rCurrentProcessInfo);


}; // class RomFiniteDifferenceUtility.



}  // namespace Kratos.

#endif // ROM_FINITE_DIFFERENCE_UTILITY_H_INCLUDED  defined


