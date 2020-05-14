//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                   Kratos default license: kratos/license.txt
//
//  Main authors:    Ignasi de Pouplana
//                   Guillermo Casas
//

// Project includes
#include "custom_utilities/multiaxial_control_module_generalized_2d_utilities.hpp"

namespace Kratos
{

// Before FEM and DEM solution
void MultiaxialControlModuleGeneralized2DUtilities::ExecuteInitialize() {
    KRATOS_TRY;

    // Iterate through all on plane actuators to set velocities to 0.0
    for(unsigned int map_index = 0; map_index < mOrderedMapKeys.size(); map_index++) {
        const std::string actuator_name = mOrderedMapKeys[map_index];
        std::vector<ModelPart*> SubModelPartList = mFEMBoundariesSubModelParts[actuator_name];
        if (actuator_name != "Radial" && actuator_name != "Z") {
            // Iterate through all FEMBoundaries
            for (unsigned int i = 0; i < SubModelPartList.size(); i++) {
                ModelPart& rSubModelPart = *(SubModelPartList[i]);
                // Iterate through nodes of Fem boundary
                const int NNodes = static_cast<int>(rSubModelPart.Nodes().size());
                ModelPart::NodesContainerType::iterator it_begin = rSubModelPart.NodesBegin();
                #pragma omp parallel for
                for(int j = 0; j<NNodes; j++) {
                    ModelPart::NodesContainerType::iterator it = it_begin + j;
                    array_1d<double,3>& r_velocity = it->FastGetSolutionStepValue(VELOCITY);
                    noalias(r_velocity) = ZeroVector(3);
                }
            }
        }
    }

    // Iterate through all actuators
    for(unsigned int map_index = 0; map_index < mOrderedMapKeys.size(); map_index++) {
        const std::string actuator_name = mOrderedMapKeys[map_index];
        std::vector<ModelPart*> SubModelPartList = mFEMBoundariesSubModelParts[actuator_name];
        if (actuator_name == "Radial") {
            // In axisymmetric cases we assume there is only 1 actuator in the FEM boundary
            ModelPart& rSubModelPart = *(SubModelPartList[0]);
            // Iterate through nodes of Fem boundary
            const int NNodes = static_cast<int>(rSubModelPart.Nodes().size());
            ModelPart::NodesContainerType::iterator it_begin = rSubModelPart.NodesBegin();
            #pragma omp parallel for
            for(int i = 0; i<NNodes; i++) {
                ModelPart::NodesContainerType::iterator it = it_begin + i;
                const double external_radius = std::sqrt(it->X()*it->X() + it->Y()*it->Y());
                const double cos_theta = it->X()/external_radius;
                const double sin_theta = it->Y()/external_radius;
                array_1d<double,3>& r_displacement = it->FastGetSolutionStepValue(DISPLACEMENT);
                array_1d<double,3>& r_delta_displacement = it->FastGetSolutionStepValue(DELTA_DISPLACEMENT);
                array_1d<double,3>& r_velocity = it->FastGetSolutionStepValue(VELOCITY);
                noalias(r_displacement) = ZeroVector(3);
                noalias(r_delta_displacement) = ZeroVector(3);
                r_velocity[0] = mVelocity[map_index] * cos_theta;
                r_velocity[1] = mVelocity[map_index] * sin_theta;
                r_velocity[2] = 0.0;
            }
        } else if (actuator_name == "Z") {
            mrDemModelPart.GetProcessInfo()[IMPOSED_Z_STRAIN_VALUE] = 0.0;
        } else {
            // Iterate through all FEMBoundaries
            for (unsigned int i = 0; i < SubModelPartList.size(); i++) {
                ModelPart& rSubModelPart = *(SubModelPartList[i]);
                // Iterate through nodes of Fem boundary
                const int NNodes = static_cast<int>(rSubModelPart.Nodes().size());
                ModelPart::NodesContainerType::iterator it_begin = rSubModelPart.NodesBegin();
                #pragma omp parallel for
                for(int j = 0; j<NNodes; j++) {
                    ModelPart::NodesContainerType::iterator it = it_begin + j;
                    array_1d<double,3>& r_displacement = it->FastGetSolutionStepValue(DISPLACEMENT);
                    array_1d<double,3>& r_delta_displacement = it->FastGetSolutionStepValue(DELTA_DISPLACEMENT);
                    array_1d<double,3>& r_velocity = it->FastGetSolutionStepValue(VELOCITY);
                    noalias(r_displacement) = ZeroVector(3);
                    noalias(r_delta_displacement) = ZeroVector(3);
                    noalias(r_velocity) += mVelocity[map_index] * mFEMOuterNormals[actuator_name][i];
                    r_velocity[2] = 0.0;
                }
            }
        }
    }

    KRATOS_CATCH("");
}

//***************************************************************************************************************

// Before FEM and DEM solution
void MultiaxialControlModuleGeneralized2DUtilities::ExecuteInitializeSolutionStep() {
    KRATOS_TRY;

    const double current_time = mrDemModelPart.GetProcessInfo()[TIME];

    // Update velocities
    if (mCMTime < current_time) {
        
        const double next_cm_time = mCMTime + mCMDeltaTime;

        const unsigned int number_of_actuators = mFEMBoundariesSubModelParts.size();
        Vector next_target_stress(number_of_actuators);
        noalias(next_target_stress) = ZeroVector(number_of_actuators);

        // Iterate through all actuators
        for(unsigned int map_index = 0; map_index < mOrderedMapKeys.size(); map_index++) {
            const std::string actuator_name = mOrderedMapKeys[map_index];
            std::vector<ModelPart*> FEMSubModelPartList = mFEMBoundariesSubModelParts[actuator_name];
            std::vector<ModelPart*> DEMSubModelPartList = mDEMBoundariesSubModelParts[actuator_name];
            unsigned int target_stress_table_id = mTargetStressTableIds[actuator_name];
            if (actuator_name == "Z") {
                TableType::Pointer pDEMTargetStressTable = (*(DEMSubModelPartList[0])).pGetTable(target_stress_table_id);
                next_target_stress[map_index] = pDEMTargetStressTable->GetValue(next_cm_time);
            } else {
                TableType::Pointer pFEMTargetStressTable = (*(FEMSubModelPartList[0])).pGetTable(target_stress_table_id);
                next_target_stress[map_index] = pFEMTargetStressTable->GetValue(next_cm_time);
            }
        }

        Vector target_stress_perturbation(number_of_actuators);
        noalias(target_stress_perturbation) = GetPerturbations(next_target_stress,next_cm_time);
        noalias(next_target_stress) += target_stress_perturbation;
        Matrix k_inverse(number_of_actuators,number_of_actuators);
        double k_det = 0.0;
        MathUtils<double>::InvertMatrix(mStiffness, k_inverse, k_det, -1.0);
        // TODO: check tolerance in CheckConditionNumber
        const bool is_k_invertible = MathUtils<double>::CheckConditionNumber(mStiffness, 
                                                        k_inverse, std::numeric_limits<double>::epsilon(),
                                                        false);
        const double k_condition_number = GetConditionNumber(mStiffness,k_inverse);

            // KRATOS_WATCH("Begin Updating velocity.........")
            // KRATOS_WATCH(current_time)
            // KRATOS_WATCH(mStiffness)
            // KRATOS_WATCH(k_condition_number)
            // KRATOS_WATCH(mVelocity)

        Vector delta_velocity(number_of_actuators);
        Vector delta_target_stress(number_of_actuators);
        noalias(delta_target_stress) = next_target_stress-mReactionStress;
        if (is_k_invertible == false || std::isnan(k_condition_number)) {
            noalias(delta_velocity) = ZeroVector(number_of_actuators);
            std::cout << "Stiffness matrix is not invertible. Keeping loading velocity constant" << std::endl;            
        } else {
            noalias(delta_velocity) = prod(k_inverse,delta_target_stress)/mCMDeltaTime - mVelocity;
        }
        noalias(mVelocity) += mVelocityFactor * delta_velocity;

            // KRATOS_WATCH("Updating velocity.........")
            // KRATOS_WATCH(mVelocity)

        for (unsigned int i = 0; i < mVelocity.size(); i++) {
            if (std::abs(mVelocity[i]) > std::abs(mLimitVelocities[i])) {
                if (mVelocity[i] > 0.0) {
                    mVelocity[i] = std::abs(mLimitVelocities[i]);
                } else {
                    mVelocity[i] = - std::abs(mLimitVelocities[i]);
                }
            }
        }
            // KRATOS_WATCH("End Updating velocity.........")
            // KRATOS_WATCH(mVelocity)
    }

    // Move Actuators
    const double delta_time = mrDemModelPart.GetProcessInfo()[DELTA_TIME];

    // Iterate through all actuators
    for(unsigned int map_index = 0; map_index < mOrderedMapKeys.size(); map_index++) {
        const std::string actuator_name = mOrderedMapKeys[map_index];
        std::vector<ModelPart*> SubModelPartList = mFEMBoundariesSubModelParts[actuator_name];
        if (actuator_name == "Radial") {
            // In axisymmetric cases we assume there is only 1 actuator in the FEM boundary
            ModelPart& rSubModelPart = *(SubModelPartList[0]);
            // Iterate through nodes of Fem boundary
            const int NNodes = static_cast<int>(rSubModelPart.Nodes().size());
            ModelPart::NodesContainerType::iterator it_begin = rSubModelPart.NodesBegin();
            #pragma omp parallel for
            for(int i = 0; i<NNodes; i++) {
                ModelPart::NodesContainerType::iterator it = it_begin + i;
                const double external_radius = std::sqrt(it->X()*it->X() + it->Y()*it->Y());
                const double cos_theta = it->X()/external_radius;
                const double sin_theta = it->Y()/external_radius;
                array_1d<double,3>& r_displacement = it->FastGetSolutionStepValue(DISPLACEMENT);
                array_1d<double,3>& r_delta_displacement = it->FastGetSolutionStepValue(DELTA_DISPLACEMENT);
                array_1d<double,3>& r_velocity = it->FastGetSolutionStepValue(VELOCITY);
                r_velocity[0] = mVelocity[map_index] * cos_theta;
                r_velocity[1] = mVelocity[map_index] * sin_theta;
                r_velocity[2] = 0.0;
                noalias(r_delta_displacement) = r_velocity * delta_time;
                noalias(r_displacement) += r_delta_displacement;
                noalias(it->Coordinates()) = it->GetInitialPosition().Coordinates() + r_displacement;
            }
        } else if (actuator_name == "Z") {
            mrDemModelPart.GetProcessInfo()[IMPOSED_Z_STRAIN_VALUE] += mVelocity[map_index]*delta_time/1.0;
        } else {
            // Iterate through all FEMBoundaries
            for (unsigned int i = 0; i < SubModelPartList.size(); i++) {
                ModelPart& rSubModelPart = *(SubModelPartList[i]);
                // Iterate through nodes of Fem boundary
                const int NNodes = static_cast<int>(rSubModelPart.Nodes().size());
                ModelPart::NodesContainerType::iterator it_begin = rSubModelPart.NodesBegin();
                #pragma omp parallel for
                for(int j = 0; j<NNodes; j++) {
                    ModelPart::NodesContainerType::iterator it = it_begin + j;
                    array_1d<double,3>& r_displacement = it->FastGetSolutionStepValue(DISPLACEMENT);
                    array_1d<double,3>& r_delta_displacement = it->FastGetSolutionStepValue(DELTA_DISPLACEMENT);
                    array_1d<double,3>& r_velocity = it->FastGetSolutionStepValue(VELOCITY);
                    noalias(r_velocity) = mVelocity[map_index] * mFEMOuterNormals[actuator_name][i];
                    r_velocity[2] = 0.0;
                    noalias(r_delta_displacement) = r_velocity * delta_time;
                    noalias(r_displacement) += r_delta_displacement;
                    noalias(it->Coordinates()) = it->GetInitialPosition().Coordinates() + r_displacement;
                }
            }
        }
    }

    KRATOS_CATCH("");
}

//***************************************************************************************************************

// After FEM and DEM solution
void MultiaxialControlModuleGeneralized2DUtilities::ExecuteFinalizeSolutionStep() {
    const double current_time = mrDemModelPart.GetProcessInfo()[TIME];
    const unsigned int number_of_actuators = mFEMBoundariesSubModelParts.size();

    // Update ReactionStresses and Stiffness matrix
    if (mCMTime < current_time) {

        // Advance CM time
        mCMTime += mCMDeltaTime;
        mCMStep += 1;
        
        Vector delta_reaction_stress(number_of_actuators);
        noalias(delta_reaction_stress) = MeasureReactionStress() - mReactionStress;
        noalias(mReactionStress) += delta_reaction_stress;
        // noalias(mReactionStress) += mReactionAlpha * delta_reaction_stress;

        for (unsigned int i = 0; i < number_of_actuators; i++) {
            mDeltaDisplacement(i,mActuatorCounter) = mVelocity[i]*mCMDeltaTime;
            mDeltaReactionStress(i,mActuatorCounter) = delta_reaction_stress[i];
        }

        if (mCMStep > 2) {
            // Update K if DeltaDisplacement is invertible
            Matrix delta_displacement_inverse(number_of_actuators,number_of_actuators);
            double delta_displacement_det = 0.0;
            MathUtils<double>::InvertMatrix(mDeltaDisplacement, delta_displacement_inverse, delta_displacement_det,-1.0);
            // TODO: check tolerance in CheckConditionNumber
            const bool is_delta_displacement_invertible = MathUtils<double>::CheckConditionNumber(mDeltaDisplacement, 
                                                            delta_displacement_inverse, 1.0e-10, 
                                                            false);
            const double delta_displacement_condition_number = GetConditionNumber(mDeltaDisplacement,delta_displacement_inverse);

            // KRATOS_WATCH("Begin Updating K.........")
            // KRATOS_WATCH(mDeltaDisplacement)
            // KRATOS_WATCH(delta_displacement_condition_number)
            // KRATOS_WATCH(mStiffness)

            Matrix k_estimated(number_of_actuators,number_of_actuators);
            if (is_delta_displacement_invertible == false || std::isnan(delta_displacement_condition_number) ) {
                noalias(k_estimated) = mStiffness;
                std::cout << "Delta displacement matrix is not invertible. Keeping stiffness matrix constant" << std::endl;
            } else {
                noalias(k_estimated) = prod(mDeltaReactionStress,delta_displacement_inverse);
            }
            noalias(mStiffness) = mStiffnessAlpha * k_estimated + (1.0 - mStiffnessAlpha) * mStiffness;

            // KRATOS_WATCH("End Updating K........")
            // KRATOS_WATCH(mStiffness)      
        }

        if (mActuatorCounter == number_of_actuators-1) {
            mActuatorCounter = 0;
        } else{
            mActuatorCounter++;
        }
    }

    // Print results

    // Iterate through all on plane actuators to set variables to 0.0
    for(unsigned int map_index = 0; map_index < mOrderedMapKeys.size(); map_index++) {
        const std::string actuator_name = mOrderedMapKeys[map_index];
        std::vector<ModelPart*> SubModelPartList = mFEMBoundariesSubModelParts[actuator_name];
        if (actuator_name != "Radial" && actuator_name != "Z") {
            // Iterate through all FEMBoundaries
            for (unsigned int i = 0; i < SubModelPartList.size(); i++) {
                ModelPart& rSubModelPart = *(SubModelPartList[i]);
                // Iterate through nodes of Fem boundary
                const int NNodes = static_cast<int>(rSubModelPart.Nodes().size());
                ModelPart::NodesContainerType::iterator it_begin = rSubModelPart.NodesBegin();
                #pragma omp parallel for
                for(int j = 0; j<NNodes; j++) {
                    ModelPart::NodesContainerType::iterator it = it_begin + j;
                    array_1d<double,3>& r_target_stress = it->FastGetSolutionStepValue(TARGET_STRESS);
                    array_1d<double,3>& r_reaction_stress = it->FastGetSolutionStepValue(REACTION_STRESS);
                    array_1d<double,3>& r_loading_velocity = it->FastGetSolutionStepValue(LOADING_VELOCITY);
                    noalias(r_target_stress) = ZeroVector(3);
                    noalias(r_reaction_stress) = ZeroVector(3);
                    noalias(r_loading_velocity) = ZeroVector(3);
                }
            }
        }
    }

    Vector reaction_stress_to_print(number_of_actuators);
    noalias(reaction_stress_to_print) = MeasureReactionStress();

    // Iterate through all actuators
    for(unsigned int map_index = 0; map_index < mOrderedMapKeys.size(); map_index++) {
        const std::string actuator_name = mOrderedMapKeys[map_index];
        std::vector<ModelPart*> FEMSubModelPartList = mFEMBoundariesSubModelParts[actuator_name];
        std::vector<ModelPart*> DEMSubModelPartList = mDEMBoundariesSubModelParts[actuator_name];
        unsigned int target_stress_table_id = mTargetStressTableIds[actuator_name];
        if (actuator_name == "Radial") {
            // In axisymmetric cases we assume there is only 1 actuator in the FEM boundary
            ModelPart& rSubModelPart = *(FEMSubModelPartList[0]);
            // Iterate through nodes of Fem boundary
            const int NNodes = static_cast<int>(rSubModelPart.Nodes().size());
            ModelPart::NodesContainerType::iterator it_begin = rSubModelPart.NodesBegin();
            TableType::Pointer TargetStressTable = rSubModelPart.pGetTable(target_stress_table_id);
            double current_target_stress = TargetStressTable->GetValue(current_time);
            #pragma omp parallel for
            for(int i = 0; i<NNodes; i++) {
                ModelPart::NodesContainerType::iterator it = it_begin + i;
                const double external_radius = std::sqrt(it->X()*it->X() + it->Y()*it->Y());
                const double cos_theta = it->X()/external_radius;
                const double sin_theta = it->Y()/external_radius;
                it->FastGetSolutionStepValue(TARGET_STRESS_X) = current_target_stress * cos_theta;
                it->FastGetSolutionStepValue(TARGET_STRESS_Y) = current_target_stress * sin_theta;
                it->FastGetSolutionStepValue(REACTION_STRESS_X) = reaction_stress_to_print[map_index] * cos_theta;
                it->FastGetSolutionStepValue(REACTION_STRESS_Y) = reaction_stress_to_print[map_index] * sin_theta;
                it->FastGetSolutionStepValue(LOADING_VELOCITY_X) = mVelocity[map_index] * cos_theta;
                it->FastGetSolutionStepValue(LOADING_VELOCITY_Y) = mVelocity[map_index] * sin_theta;
            }
        } else if (actuator_name == "Z") {
            // Iterate through all DEMBoundaries
            for (unsigned int i = 0; i < DEMSubModelPartList.size(); i++) {
                ModelPart& rSubModelPart = *(DEMSubModelPartList[i]);
                // Iterate through nodes of DEM boundary
                const int NNodes = static_cast<int>(rSubModelPart.Nodes().size());
                ModelPart::NodesContainerType::iterator it_begin = rSubModelPart.NodesBegin();
                TableType::Pointer TargetStressTable = rSubModelPart.pGetTable(target_stress_table_id);
                double current_target_stress = TargetStressTable->GetValue(current_time);
                #pragma omp parallel for
                for(int j = 0; j<NNodes; j++) {
                    ModelPart::NodesContainerType::iterator it = it_begin + j;
                    it->FastGetSolutionStepValue(TARGET_STRESS_Z) = current_target_stress;
                    it->FastGetSolutionStepValue(REACTION_STRESS_Z) = reaction_stress_to_print[map_index];
                    it->FastGetSolutionStepValue(LOADING_VELOCITY_Z) = mVelocity[map_index];
                }
                mrDemModelPart.GetProcessInfo()[TARGET_STRESS_Z] = std::abs(current_target_stress);
            }
        } else {
            // Iterate through all FEMBoundaries
            for (unsigned int i = 0; i < FEMSubModelPartList.size(); i++) {
                ModelPart& rSubModelPart = *(FEMSubModelPartList[i]);
                // Iterate through nodes of Fem boundary
                const int NNodes = static_cast<int>(rSubModelPart.Nodes().size());
                ModelPart::NodesContainerType::iterator it_begin = rSubModelPart.NodesBegin();
                TableType::Pointer TargetStressTable = rSubModelPart.pGetTable(target_stress_table_id);
                double current_target_stress = TargetStressTable->GetValue(current_time);
                #pragma omp parallel for
                for(int j = 0; j<NNodes; j++) {
                    ModelPart::NodesContainerType::iterator it = it_begin + j;
                    array_1d<double,3>& r_target_stress = it->FastGetSolutionStepValue(TARGET_STRESS);
                    array_1d<double,3>& r_reaction_stress = it->FastGetSolutionStepValue(REACTION_STRESS);
                    array_1d<double,3>& r_loading_velocity = it->FastGetSolutionStepValue(LOADING_VELOCITY);
                    noalias(r_target_stress) += current_target_stress * mFEMOuterNormals[actuator_name][i];
                    noalias(r_reaction_stress) += reaction_stress_to_print[map_index] * mFEMOuterNormals[actuator_name][i];
                    noalias(r_loading_velocity) += mVelocity[map_index] * mFEMOuterNormals[actuator_name][i];
                }
            }
        }
    }
}

//***************************************************************************************************************

Vector MultiaxialControlModuleGeneralized2DUtilities::GetPerturbations(const Vector& rTargetStress, const double& rTime) {

    const unsigned int number_of_actuators = rTargetStress.size();
    Vector stress_perturbation(number_of_actuators);
    noalias(stress_perturbation) = ZeroVector(number_of_actuators);

    // Iterate through all actuators
    for(unsigned int map_index = 0; map_index < mOrderedMapKeys.size(); map_index++) {
        const std::string actuator_name = mOrderedMapKeys[map_index];
        if (actuator_name == "Z") {
            stress_perturbation[map_index] = 0.0;
        } else {
            double amplitude = rTargetStress[map_index] * mStressTolerance;
            double omega = 2.0 * Globals::Pi / (mPerturbationPeriod * mCMDeltaTime);
            double phi = map_index * 2.0 * Globals::Pi / number_of_actuators;
            stress_perturbation[map_index] = amplitude * std::sin(omega * rTime + phi);
        }
    }
    return stress_perturbation;
}

//***************************************************************************************************************

Vector MultiaxialControlModuleGeneralized2DUtilities::MeasureReactionStress() {

    const unsigned int number_of_actuators = mFEMBoundariesSubModelParts.size();
    Vector reaction_stress(number_of_actuators);
    noalias(reaction_stress) = ZeroVector(number_of_actuators);

    // Iterate through all actuators
    for(unsigned int map_index = 0; map_index < mOrderedMapKeys.size(); map_index++) {
        const std::string actuator_name = mOrderedMapKeys[map_index];
        std::vector<ModelPart*> FEMSubModelPartList = mFEMBoundariesSubModelParts[actuator_name];
        std::vector<ModelPart*> DEMSubModelPartList = mDEMBoundariesSubModelParts[actuator_name];
        double face_area = 0.0;
        double face_reaction = 0.0;
        if (actuator_name == "Z") {
            // Calculate face_area
            // Iterate through all DEMBoundaries
            for (unsigned int i = 0; i < DEMSubModelPartList.size(); i++) {
                ModelPart& rSubModelPart = *(DEMSubModelPartList[i]);
                // Iterate through elements of DEM boundary
                ModelPart::ElementsContainerType& rElements = rSubModelPart.GetCommunicator().LocalMesh().Elements();
                #pragma omp parallel for reduction(+:face_area)
                for (int j = 0; j < (int)rElements.size(); j++) {
                    ModelPart::ElementsContainerType::ptr_iterator ptr_itElem = rElements.ptr_begin() + j;
                    Element* p_element = ptr_itElem->get();
                    SphericContinuumParticle* pDemElem = dynamic_cast<SphericContinuumParticle*>(p_element);
                    const double radius = pDemElem->GetRadius();
                    face_area += Globals::Pi*radius*radius;
                }
            }
            // Calculate face_reaction
            // Iterate through all DEMBoundaries
            for (unsigned int i = 0; i < DEMSubModelPartList.size(); i++) {
                ModelPart& rSubModelPart = *(DEMSubModelPartList[i]);
                // Iterate through elements of DEM boundary
                ModelPart::ElementsContainerType& rElements = rSubModelPart.GetCommunicator().LocalMesh().Elements();
                #pragma omp parallel for reduction(+:face_reaction)
                for (int j = 0; j < (int)rElements.size(); j++) {
                    ModelPart::ElementsContainerType::ptr_iterator ptr_itElem = rElements.ptr_begin() + j;
                    Element* p_element = ptr_itElem->get();
                    SphericContinuumParticle* pDemElem = dynamic_cast<SphericContinuumParticle*>(p_element);
                    BoundedMatrix<double, 3, 3> stress_tensor = ZeroMatrix(3,3);
                    noalias(stress_tensor) = (*(pDemElem->mSymmStressTensor));
                    const double radius = pDemElem->GetRadius();
                    face_reaction += stress_tensor(2,2) * Globals::Pi*radius*radius;
                }
            }
            if (std::abs(face_area) > 1.0e-12) {
                reaction_stress[map_index] = face_reaction/face_area;
            } else {
                reaction_stress[map_index] = 0.0;
            }
        } else {
            // Calculate face_area
            // Iterate through all FEMBoundaries
            for (unsigned int i = 0; i < FEMSubModelPartList.size(); i++) {
                ModelPart& rSubModelPart = *(FEMSubModelPartList[i]);
                // Iterate through conditions of FEM boundary
                const int NCons = static_cast<int>(rSubModelPart.Conditions().size());
                ModelPart::ConditionsContainerType::iterator con_begin = rSubModelPart.ConditionsBegin();
                #pragma omp parallel for reduction(+:face_area)
                for(int j = 0; j < NCons; j++) {
                    ModelPart::ConditionsContainerType::iterator itCond = con_begin + j;
                    face_area += itCond->GetGeometry().Area();
                }
            }
            // Calculate face_reaction
            // Iterate through all FEMBoundaries
            for (unsigned int i = 0; i < FEMSubModelPartList.size(); i++) {
                ModelPart& rSubModelPart = *(FEMSubModelPartList[i]);
                // Iterate through nodes of Fem boundary
                const int NNodes = static_cast<int>(rSubModelPart.Nodes().size());
                ModelPart::NodesContainerType::iterator it_begin = rSubModelPart.NodesBegin();
                #pragma omp parallel for reduction(+:face_reaction)
                for(int j = 0; j<NNodes; j++) {
                    ModelPart::NodesContainerType::iterator it = it_begin + j;
                    array_1d<double,3>& r_force = it->FastGetSolutionStepValue(CONTACT_FORCES);
                    face_reaction -= inner_prod(r_force,mFEMOuterNormals[actuator_name][i]);
                }
            }
            if (std::abs(face_area) > 1.0e-12) {
                reaction_stress[map_index] = face_reaction/face_area;
            } else {
                reaction_stress[map_index] = 0.0;
            }
        }
    }

    return reaction_stress;
}

double MultiaxialControlModuleGeneralized2DUtilities::GetConditionNumber(const Matrix& rInputMatrix, const Matrix& rInvertedMatrix) {

    // Find the condition number to define is inverse is OK
    const double input_matrix_norm = norm_frobenius(rInputMatrix);
    const double inverted_matrix_norm = norm_frobenius(rInvertedMatrix);

    // Now the condition number is the product of both norms
    const double cond_number = input_matrix_norm * inverted_matrix_norm ;

    return cond_number;
}

}  // namespace Kratos
