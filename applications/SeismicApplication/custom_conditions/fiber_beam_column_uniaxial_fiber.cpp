//  KRATOS  ____       _               _
//         / ___|  ___(_)___ _ __ ___ (_) ___
//         \___ \ / _ \ / __| '_ ` _ \| |/ __|
//          ___) |  __/ \__ \ | | | | | | (__
//         |____/ \___|_|___/_| |_| |_|_|\___|
//
//  License:     BSD License
//  license:     structural_mechanics_application/license.txt
//
//  Main authors: Mahmoud Zidan
//    Co authors: Long Chen
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
    ConstitutiveLaw::Pointer pMaterial,
    PropertiesType::Pointer pProperties
)
    : mId(NewId), mArea(Area), mpMaterial(pMaterial), mpProperties(pProperties)
{
    mTransformationVector[0] =  -Y;
    mTransformationVector[1] =   Z;
    mTransformationVector[2] = 1.0;
}

void FiberBeamColumnUniaxialFiber::Initialize()
{
    KRATOS_TRY
    ProcessInfo temp_proc;
    GeometryType temp_geo;
    ConstitutiveLaw::Parameters params = ConstitutiveLaw::Parameters(temp_geo, *mpProperties, temp_proc);
    mpMaterial->InitializeMaterialResponse(params, ConstitutiveLaw::StressMeasure_PK2);
    KRATOS_CATCH("")
}

void FiberBeamColumnUniaxialFiber::CreateGlobalFiberStiffnessMatrix(Matrix& rGlobalStiffnessMatrix){
    KRATOS_TRY

    ProcessInfo temp_proc;
    GeometryType temp_geo;
    ConstitutiveLaw::Parameters params = ConstitutiveLaw::Parameters(temp_geo, *mpProperties, temp_proc);
    params.SetStrainVector(mStrain);
    Matrix constitutive_materix = ZeroMatrix(1);
    params.SetConstitutiveMatrix(constitutive_materix);
    params.Set(ConstitutiveLaw::USE_ELEMENT_PROVIDED_STRAIN);
    params.Set(ConstitutiveLaw::COMPUTE_CONSTITUTIVE_TENSOR);
    mpMaterial->CalculateMaterialResponse(params, ConstitutiveLaw::StressMeasure_PK2);

    double ea = constitutive_materix(0,0) * mArea;
    rGlobalStiffnessMatrix = ZeroMatrix(3, 3);
    noalias(rGlobalStiffnessMatrix) = ea * outer_prod(mTransformationVector, mTransformationVector);

    KRATOS_CATCH("")
}

void FiberBeamColumnUniaxialFiber::StateDetermination(const Vector& rSectionDeformationIncrements)
{
    KRATOS_TRY
    // increment strain
    double strain_incr = inner_prod(mTransformationVector, rSectionDeformationIncrements);
    mStrain[0] += strain_incr;

    // calculate stress and tangent modulus from the constitutive law
    ProcessInfo temp_proc;
    GeometryType temp_geo;
    ConstitutiveLaw::Parameters params = ConstitutiveLaw::Parameters(temp_geo, *mpProperties, temp_proc);
    params.SetStrainVector(mStrain);
    params.SetStressVector(mStress);
    Matrix constitutive_materix = ZeroMatrix(1);
    params.SetConstitutiveMatrix(constitutive_materix);
    params.Set(ConstitutiveLaw::USE_ELEMENT_PROVIDED_STRAIN);
    params.Set(ConstitutiveLaw::COMPUTE_CONSTITUTIVE_TENSOR);
    params.Set(ConstitutiveLaw::COMPUTE_STRESS);
    mpMaterial->CalculateMaterialResponse(params, ConstitutiveLaw::StressMeasure_PK2);
    KRATOS_CATCH("")
}

void FiberBeamColumnUniaxialFiber::CreateGlobalFiberInternalForces(Vector& rGlobalFiberInternalForces)
{
    KRATOS_TRY
    rGlobalFiberInternalForces = ZeroVector(3);
    rGlobalFiberInternalForces += mTransformationVector * mArea * mStress[0];
    KRATOS_CATCH("")
}

void FiberBeamColumnUniaxialFiber::FinalizeSolutionStep()
{
    KRATOS_TRY
    ProcessInfo temp_proc;
    GeometryType temp_geo;
    ConstitutiveLaw::Parameters params = ConstitutiveLaw::Parameters(temp_geo, *mpProperties, temp_proc);
    params.SetStrainVector(mStrain);
    params.SetStressVector(mStress);
    mpMaterial->FinalizeMaterialResponse(params, ConstitutiveLaw::StressMeasure_PK2);
    KRATOS_CATCH("")
}

void FiberBeamColumnUniaxialFiber::PrintInfo(std::ostream& rOStream) const {
    rOStream << "FiberBeamColumnUniaxialFiber #" << mId;
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
    rSerializer.save("mpProperties", mpProperties);
    rSerializer.save("mStrain", mStrain);
    rSerializer.save("mStress", mStress);
}
void FiberBeamColumnUniaxialFiber::load(Serializer& rSerializer)
{
    rSerializer.load("mId", mId);
    rSerializer.load("mTransformationVector", mTransformationVector);
    rSerializer.load("mArea", mArea);
    rSerializer.load("mpMaterial", mpMaterial);
    rSerializer.load("mpProperties", mpProperties);
    rSerializer.load("mStrain", mStrain);
    rSerializer.load("mStress", mStress);
}

} // namespace Kratos.