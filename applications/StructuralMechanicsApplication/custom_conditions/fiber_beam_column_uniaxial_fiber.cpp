// KRATOS  ___|  |                   |                   |
//       \___ \  __|  __| |   |  __| __| |   |  __| _` | |
//             | |   |    |   | (    |   |   | |   (   | |
//       _____/ \__|_|   \__,_|\___|\__|\__,_|_|  \__,_|_| MECHANICS
//
//  License:     BSD License
//  license:     structural_mechanics_application/license.txt
//
//  Main authors: Mahmoud Zidan
//

// System includes
#include<iostream>

// External includes

// Project includes
#include "custom_conditions/fiber_beam_column_uniaxial_fiber.hpp"

namespace Kratos {

FiberBeamColumnUniaxialFiber::FiberBeamColumnUniaxialFiber(IndexType NewId)
    : mId(NewId) {}

FiberBeamColumnUniaxialFiber::FiberBeamColumnUniaxialFiber(
    IndexType NewId,
    double Y,
    double Z,
    double Area,
    UniaxialFiberBeamColumnMaterialLaw::Pointer pMaterial
)
    : mId(NewId), mArea(Area), mpMaterial(pMaterial)
{
    mTransformationVector[0] =  -Y;
    mTransformationVector[1] =   Z;
    mTransformationVector[2] = 1.0;
}

void FiberBeamColumnUniaxialFiber::Initialize()
{
    KRATOS_TRY
    KRATOS_CATCH("")
}

Matrix FiberBeamColumnUniaxialFiber::CreateGlobalFiberStiffnessMatrix(){
    KRATOS_TRY
    double ea = mpMaterial->GetTangentModulus() * mArea;
    Matrix global_stiffness = ZeroMatrix(3, 3);
    noalias(global_stiffness) = ea * outer_prod(mTransformationVector, mTransformationVector);
    return global_stiffness;
    KRATOS_CATCH("")
}

void FiberBeamColumnUniaxialFiber::StateDetermination(const Vector& rSectionDeformationIncrements)
{
    KRATOS_TRY
    double strain_incr = inner_prod(mTransformationVector, rSectionDeformationIncrements);
    mStrain += strain_incr;
    mpMaterial->SetStrain(mStrain);
    mpMaterial->CalculateMaterialResponse();
    mStress = mpMaterial->GetStress();
    KRATOS_CATCH("")
}

Vector FiberBeamColumnUniaxialFiber::CreateGlobalFiberInternalForces()
{
    KRATOS_TRY
    return mTransformationVector * mArea * mStress;
    KRATOS_CATCH("")
}

void FiberBeamColumnUniaxialFiber::FinalizeSolutionStep()
{
    KRATOS_TRY
    mpMaterial->FinalizeMaterialResponse();
    KRATOS_CATCH("")
}

void FiberBeamColumnUniaxialFiber::PrintInfo(std::ostream& rOStream) const {
    rOStream << "FiberBeamColumnUniaxialFiber #" << Id();
}

void FiberBeamColumnUniaxialFiber::PrintData(std::ostream& rOStream) const {
    rOStream << "    Position : [" << -1.0 * mTransformationVector[0] << "," << mTransformationVector[1] << "]" << std::endl;
    rOStream << "    Area     : " << mArea << std::endl;
    rOStream << "    Material : " << *mpMaterial << std::endl;
}


void FiberBeamColumnUniaxialFiber::save(Serializer& rSerializer) const
{
    rSerializer.save("mId", mId);
    rSerializer.save("mTransformationVector", mTransformationVector);
    rSerializer.save("mArea", mArea);
    rSerializer.save("mpMaterial", mpMaterial);
    rSerializer.save("mStrain", mStrain);
    rSerializer.save("mStress", mStress);
}
void FiberBeamColumnUniaxialFiber::load(Serializer& rSerializer)
{
    rSerializer.load("mId", mId);
    rSerializer.load("mTransformationVector", mTransformationVector);
    rSerializer.load("mArea", mArea);
    rSerializer.load("mpMaterial", mpMaterial);
    rSerializer.load("mStrain", mStrain);
    rSerializer.load("mStress", mStress);
}

} // namespace Kratos.