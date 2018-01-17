// Created by: Salva Latorre, latorre@cimne.upc.edu

// System includes
#include <string>
#include <iostream>
#include <stdlib.h>

// Project includes
#include "rigid_body_element.h"
#include "custom_utilities/GeometryFunctions.h"
#include "custom_utilities/AuxiliaryFunctions.h"
#include "DEM_application_variables.h"
#include "includes/kratos_flags.h"
#include "includes/variables.h"
#include "geometries/point_3d.h"

namespace Kratos {

    RigidBodyElement3D::RigidBodyElement3D() : Element() {}
            
    RigidBodyElement3D::RigidBodyElement3D(IndexType NewId, GeometryType::Pointer pGeometry)
    : Element(NewId, pGeometry) {
        mpTranslationalIntegrationScheme = NULL;
        mpRotationalIntegrationScheme = NULL;
    }
      
    RigidBodyElement3D::RigidBodyElement3D(IndexType NewId, GeometryType::Pointer pGeometry, PropertiesType::Pointer pProperties)
    : Element(NewId, pGeometry, pProperties) {
        mpTranslationalIntegrationScheme = NULL;
        mpRotationalIntegrationScheme = NULL;
    }
      
    RigidBodyElement3D::RigidBodyElement3D(IndexType NewId, NodesArrayType const& ThisNodes)
    : Element(NewId, ThisNodes) {
        mpTranslationalIntegrationScheme = NULL;
        mpRotationalIntegrationScheme = NULL;
    }
    
    Element::Pointer RigidBodyElement3D::Create(IndexType NewId, NodesArrayType const& ThisNodes, PropertiesType::Pointer pProperties) const {
        return Element::Pointer(new RigidBodyElement3D(NewId, GetGeometry().Create(ThisNodes), pProperties));
    }      
    
    // Destructor
    RigidBodyElement3D::~RigidBodyElement3D() {

        mListOfCoordinates.clear();

        if (mpTranslationalIntegrationScheme!=NULL) {
            delete mpTranslationalIntegrationScheme;
        }
        
        if (mpRotationalIntegrationScheme!=NULL) {
            delete mpRotationalIntegrationScheme;
        }
    }

    void RigidBodyElement3D::Initialize(ProcessInfo& r_process_info) {
        
        if (GetGeometry()[0].GetDof(VELOCITY_X).IsFixed())          GetGeometry()[0].Set(DEMFlags::FIXED_VEL_X, true);
        else                                                        GetGeometry()[0].Set(DEMFlags::FIXED_VEL_X, false);
        if (GetGeometry()[0].GetDof(VELOCITY_Y).IsFixed())          GetGeometry()[0].Set(DEMFlags::FIXED_VEL_Y, true);
        else                                                        GetGeometry()[0].Set(DEMFlags::FIXED_VEL_Y, false);
        if (GetGeometry()[0].GetDof(VELOCITY_Z).IsFixed())          GetGeometry()[0].Set(DEMFlags::FIXED_VEL_Z, true);
        else                                                        GetGeometry()[0].Set(DEMFlags::FIXED_VEL_Z, false);
        if (GetGeometry()[0].GetDof(ANGULAR_VELOCITY_X).IsFixed())  GetGeometry()[0].Set(DEMFlags::FIXED_ANG_VEL_X, true);
        else                                                        GetGeometry()[0].Set(DEMFlags::FIXED_ANG_VEL_X, false);
        if (GetGeometry()[0].GetDof(ANGULAR_VELOCITY_Y).IsFixed())  GetGeometry()[0].Set(DEMFlags::FIXED_ANG_VEL_Y, true);
        else                                                        GetGeometry()[0].Set(DEMFlags::FIXED_ANG_VEL_Y, false);
        if (GetGeometry()[0].GetDof(ANGULAR_VELOCITY_Z).IsFixed())  GetGeometry()[0].Set(DEMFlags::FIXED_ANG_VEL_Z, true);
        else                                                        GetGeometry()[0].Set(DEMFlags::FIXED_ANG_VEL_Z, false);
        
        DEMIntegrationScheme::Pointer& translational_integration_scheme = GetProperties()[DEM_TRANSLATIONAL_INTEGRATION_SCHEME_POINTER];
        DEMIntegrationScheme::Pointer& rotational_integration_scheme = GetProperties()[DEM_ROTATIONAL_INTEGRATION_SCHEME_POINTER];
        SetIntegrationScheme(translational_integration_scheme, rotational_integration_scheme);
    }

    void RigidBodyElement3D::SetIntegrationScheme(DEMIntegrationScheme::Pointer& translational_integration_scheme, DEMIntegrationScheme::Pointer& rotational_integration_scheme){
        mpTranslationalIntegrationScheme = translational_integration_scheme->CloneRaw();
        mpRotationalIntegrationScheme = rotational_integration_scheme->CloneRaw();
    }
    
    void RigidBodyElement3D::CustomInitialize(ModelPart& rigid_body_element_sub_model_part) {
        
        mInertias = ZeroVector(3);

        mInertias[0] = rigid_body_element_sub_model_part[RIGID_BODY_INERTIAS][0];
        mInertias[1] = rigid_body_element_sub_model_part[RIGID_BODY_INERTIAS][1];
        mInertias[2] = rigid_body_element_sub_model_part[RIGID_BODY_INERTIAS][2];
        mMass = rigid_body_element_sub_model_part[RIGID_BODY_MASS];

        GetGeometry()[0].FastGetSolutionStepValue(ORIENTATION) = Quaternion<double>::Identity();
        GetGeometry()[0].FastGetSolutionStepValue(NODAL_MASS) = mMass;
        GetGeometry()[0].FastGetSolutionStepValue(EXTERNAL_APPLIED_FORCE)[0] = rigid_body_element_sub_model_part[EXTERNAL_APPLIED_FORCE][0];
        GetGeometry()[0].FastGetSolutionStepValue(EXTERNAL_APPLIED_FORCE)[1] = rigid_body_element_sub_model_part[EXTERNAL_APPLIED_FORCE][1];
        GetGeometry()[0].FastGetSolutionStepValue(EXTERNAL_APPLIED_FORCE)[2] = rigid_body_element_sub_model_part[EXTERNAL_APPLIED_FORCE][2];
        GetGeometry()[0].FastGetSolutionStepValue(EXTERNAL_APPLIED_MOMENT)[0] = rigid_body_element_sub_model_part[EXTERNAL_APPLIED_MOMENT][0];
        GetGeometry()[0].FastGetSolutionStepValue(EXTERNAL_APPLIED_MOMENT)[1] = rigid_body_element_sub_model_part[EXTERNAL_APPLIED_MOMENT][1];
        GetGeometry()[0].FastGetSolutionStepValue(EXTERNAL_APPLIED_MOMENT)[2] = rigid_body_element_sub_model_part[EXTERNAL_APPLIED_MOMENT][2];

        const array_1d<double,3>& reference_inertias = mInertias;                                
        GetGeometry()[0].FastGetSolutionStepValue(NODAL_MASS) = mMass;
        GetGeometry()[0].FastGetSolutionStepValue(PRINCIPAL_MOMENTS_OF_INERTIA)[0] = reference_inertias[0];
        GetGeometry()[0].FastGetSolutionStepValue(PRINCIPAL_MOMENTS_OF_INERTIA)[1] = reference_inertias[1];
        GetGeometry()[0].FastGetSolutionStepValue(PRINCIPAL_MOMENTS_OF_INERTIA)[2] = reference_inertias[2];
        
        array_1d<double, 3> base_principal_moments_of_inertia = GetGeometry()[0].FastGetSolutionStepValue(PRINCIPAL_MOMENTS_OF_INERTIA);  

        Quaternion<double>& Orientation = GetGeometry()[0].FastGetSolutionStepValue(ORIENTATION);
        Orientation.normalize();

        array_1d<double, 3> angular_velocity = GetGeometry()[0].FastGetSolutionStepValue(ANGULAR_VELOCITY);       
        array_1d<double, 3> angular_momentum;
        double LocalTensor[3][3];
        double GlobalTensor[3][3];
        GeometryFunctions::ConstructLocalTensor(base_principal_moments_of_inertia, LocalTensor);
        GeometryFunctions::QuaternionTensorLocal2Global(Orientation, LocalTensor, GlobalTensor);                   
        GeometryFunctions::ProductMatrix3X3Vector3X1(GlobalTensor, angular_velocity, angular_momentum);
        noalias(this->GetGeometry()[0].FastGetSolutionStepValue(ANGULAR_MOMENTUM)) = angular_momentum;

        array_1d<double, 3> local_angular_velocity;
        GeometryFunctions::QuaternionVectorGlobal2Local(Orientation, angular_velocity, local_angular_velocity);
        noalias(this->GetGeometry()[0].FastGetSolutionStepValue(LOCAL_ANGULAR_VELOCITY)) = local_angular_velocity;
    }
    
    void RigidBodyElement3D::SetOrientation(const Quaternion<double> Orientation) {
        this->GetGeometry()[0].FastGetSolutionStepValue(ORIENTATION) = Orientation;
    }

    void RigidBodyElement3D::Calculate(const Variable<double>& rVariable, double& Output, const ProcessInfo& r_process_info) {}

    void RigidBodyElement3D::UpdateLinearDisplacementAndVelocityOfNodes() {
        
        Node<3>& central_node = GetGeometry()[0]; //CENTRAL NODE OF THE RBE
        array_1d<double, 3>& rigid_body_velocity = central_node.FastGetSolutionStepValue(VELOCITY);
        array_1d<double, 3> global_relative_coordinates;
        Quaternion<double>& Orientation = central_node.FastGetSolutionStepValue(ORIENTATION);
        
        NodesArrayType::iterator i_begin = mListOfNodes.begin(); 
        NodesArrayType::iterator i_end = mListOfNodes.end();
        int iter = 0; 

        for (ModelPart::NodeIterator i = i_begin; i != i_end; ++i) {

            GeometryFunctions::QuaternionVectorLocal2Global(Orientation, mListOfCoordinates[iter], global_relative_coordinates); 
            array_1d<double, 3>& node_position = i->Coordinates(); 
            array_1d<double, 3>& delta_displacement = i->FastGetSolutionStepValue(DELTA_DISPLACEMENT);
            array_1d<double, 3> previous_position;
            noalias(previous_position) = node_position;
            noalias(node_position)= central_node.Coordinates() + global_relative_coordinates;
            noalias(delta_displacement) = node_position - previous_position;
            noalias(i->FastGetSolutionStepValue(VELOCITY)) = rigid_body_velocity;
            iter++;
        }        
    }
    
    void RigidBodyElement3D::UpdateAngularDisplacementAndVelocityOfNodes() {
        
        Node<3>& central_node = GetGeometry()[0]; //CENTRAL NODE OF THE RBE
        array_1d<double, 3> global_relative_coordinates;      
        array_1d<double, 3> linear_vel_due_to_rotation;
        array_1d<double, 3>& rigid_body_velocity = central_node.FastGetSolutionStepValue(VELOCITY);
        array_1d<double, 3>& rigid_body_angular_velocity = central_node.FastGetSolutionStepValue(ANGULAR_VELOCITY);
        array_1d<double, 3>& rigid_body_delta_rotation = central_node.FastGetSolutionStepValue(DELTA_ROTATION);
        Quaternion<double>& Orientation = central_node.FastGetSolutionStepValue(ORIENTATION);

        array_1d<double, 3> previous_position;
        NodesArrayType::iterator i_begin = mListOfNodes.begin(); 
        NodesArrayType::iterator i_end = mListOfNodes.end(); 
        int iter = 0;
        
        for (ModelPart::NodeIterator i = i_begin; i != i_end; ++i) {

            GeometryFunctions::QuaternionVectorLocal2Global(Orientation, mListOfCoordinates[iter], global_relative_coordinates);
            GeometryFunctions::CrossProduct( rigid_body_angular_velocity, global_relative_coordinates, linear_vel_due_to_rotation );
            array_1d<double, 3>& velocity = i->FastGetSolutionStepValue(VELOCITY);
            noalias(velocity) = rigid_body_velocity + linear_vel_due_to_rotation;                                    
            noalias(i->FastGetSolutionStepValue(ANGULAR_VELOCITY)) = rigid_body_angular_velocity;
            noalias(i->FastGetSolutionStepValue(DELTA_ROTATION)) = rigid_body_delta_rotation;
            iter++;
        }                        
    }
    
    void RigidBodyElement3D::CollectForcesAndTorquesFromTheNodes() {

        Node<3>& central_node = GetGeometry()[0]; //CENTRAL NODE OF THE RBE
        array_1d<double, 3>& center_forces = central_node.FastGetSolutionStepValue(TOTAL_FORCES);
        array_1d<double, 3>& center_torque = central_node.FastGetSolutionStepValue(PARTICLE_MOMENT);
        center_forces[0] = center_forces[1] = center_forces[2] = center_torque[0] = center_torque[1] = center_torque[2] = 0.0;

        array_1d<double, 3> center_to_node_vector;
        array_1d<double, 3> additional_torque;

        NodesArrayType::iterator i_begin = mListOfNodes.begin();
        NodesArrayType::iterator i_end = mListOfNodes.end();
                
        for (ModelPart::NodeIterator i = i_begin; i != i_end; ++i) {
                        
            array_1d<double, 3>& node_forces = i->FastGetSolutionStepValue(CONTACT_FORCES);
            center_forces[0] += node_forces[0];
            center_forces[1] += node_forces[1];
            center_forces[2] += node_forces[2];
                        
            array_1d<double, 3>& node_position = i->Coordinates();
            
            center_to_node_vector[0] = node_position[0] - central_node.Coordinates()[0];
            center_to_node_vector[1] = node_position[1] - central_node.Coordinates()[1];
            center_to_node_vector[2] = node_position[2] - central_node.Coordinates()[2];
            GeometryFunctions::CrossProduct(center_to_node_vector, node_forces, additional_torque);
            center_torque[0] += additional_torque[0];
            center_torque[1] += additional_torque[1];
            center_torque[2] += additional_torque[2];
        }
    }
    
    void RigidBodyElement3D::GetRigidBodyElementsForce(const array_1d<double,3>& gravity) {
        
        CollectForcesAndTorquesFromTheNodes();
        ComputeAdditionalForces(gravity);
        AddUpAllForcesAndMoments();
    }
    
    void RigidBodyElement3D::ComputeAdditionalForces(const array_1d<double,3>& gravity) {

        // Gravity
        noalias(GetGeometry()[0].FastGetSolutionStepValue(TOTAL_FORCES)) += mMass * gravity;
    }
    
    void RigidBodyElement3D::AddUpAllForcesAndMoments() {
    
        KRATOS_TRY
        
        const array_1d<double, 3> external_applied_force  = GetGeometry()[0].FastGetSolutionStepValue(EXTERNAL_APPLIED_FORCE);
        const array_1d<double, 3> external_applied_torque = GetGeometry()[0].FastGetSolutionStepValue(EXTERNAL_APPLIED_MOMENT);
        noalias(GetGeometry()[0].FastGetSolutionStepValue(TOTAL_FORCES)) += external_applied_force;
        noalias(GetGeometry()[0].FastGetSolutionStepValue(PARTICLE_MOMENT)) += external_applied_torque;
        
        KRATOS_CATCH("")
    }
    
    void RigidBodyElement3D::SetInitialConditionsToNodes(const array_1d<double,3>& velocity) {
        for (unsigned int i=0; i<mListOfCoordinates.size(); i++) {
            this->GetGeometry()[i].FastGetSolutionStepValue(VELOCITY) = velocity;
        } 
    }
    
    void RigidBodyElement3D::Move(const double delta_t, const bool rotation_option, const double force_reduction_factor, const int StepFlag ) {
//         SymplecticEulerScheme A;
//         A.MoveRigidBodyElement(this, GetGeometry()[0], delta_t, rotation_option, force_reduction_factor, StepFlag);
//         //GetIntegrationScheme().MoveRigidBodyElement(this, GetGeometry()[0], delta_t, rotation_option, force_reduction_factor, StepFlag);
        GetTranslationalIntegrationScheme().MoveRigidBodyElement(this, GetGeometry()[0], delta_t, force_reduction_factor, StepFlag);
        if (rotation_option) {
            GetRotationalIntegrationScheme().RotateRigidBodyElement(this, GetGeometry()[0], delta_t, force_reduction_factor, StepFlag);
        }
    }
} // namespace Kratos
