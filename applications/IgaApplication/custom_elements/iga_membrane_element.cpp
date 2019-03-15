/*
//  KRATOS  _____________
//         /  _/ ____/   |
//         / // / __/ /| |
//       _/ // /_/ / ___ |
//      /___/\____/_/  |_| Application
//
//  Main authors:   Anna Bauer
//                  Thomas Oberbichler
//                  Tobias Teschemacher
*/

// System includes

// External includes

// Project includes
#include "iga_membrane_element.h"


namespace Kratos 
{

Element::Pointer IgaMembraneElement::Create(
    IndexType NewId,
    NodesArrayType const& ThisNodes,
    PropertiesType::Pointer pProperties) const
{
    auto geometry = GetGeometry().Create(ThisNodes);

    return Kratos::make_shared<IgaMembraneElement>(NewId, geometry,
        pProperties);
}

void IgaMembraneElement::GetDofList(
    DofsVectorType& rElementalDofList,
    ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY;

    rElementalDofList.resize(NumberOfDofs());

    for (std::size_t i = 0; i < NumberOfNodes(); i++) {
        SetElementDof(rElementalDofList, i, 0, DISPLACEMENT_X);
        SetElementDof(rElementalDofList, i, 1, DISPLACEMENT_Y);
        SetElementDof(rElementalDofList, i, 2, DISPLACEMENT_Z);
    }

    KRATOS_CATCH("")
}

void IgaMembraneElement::EquationIdVector(
    EquationIdVectorType& rResult,
    ProcessInfo& rCurrentProcessInfo)
{
    KRATOS_TRY;

    rResult.resize(NumberOfDofs());

    for (std::size_t i = 0; i < NumberOfNodes(); i++) {
        SetElementEquationId(rResult, i, 0, DISPLACEMENT_X);
        SetElementEquationId(rResult, i, 1, DISPLACEMENT_Y);
        SetElementEquationId(rResult, i, 2, DISPLACEMENT_Z);
    }

    KRATOS_CATCH("") 
}

    void IgaMembraneElement::Initialize() 
    {
       KRATOS_TRY
       //Constitutive Law initialisation
        if (GetProperties()[CONSTITUTIVE_LAW] != nullptr) {
            mConstitutiveLaw = GetProperties()[CONSTITUTIVE_LAW]->Clone();
            mConstitutiveLaw->InitializeMaterial(
                GetProperties(),
                GetGeometry(),
                GetValue(SHAPE_FUNCTION_VALUES)
            );
        }
        else
            KRATOS_ERROR << "A constitutive law needs to be specified for the element with ID " << this->Id() << std::endl;


        MetricVariables initial_metric(3);
        CalculateMetric(initial_metric);
        mInitialMetric = initial_metric;

        KRATOS_CATCH("")
    }


    void IgaMembraneElement::CalculateAll(
        MatrixType& rLeftHandSideMatrix,
        VectorType& rRightHandSideVector,
        ProcessInfo& rCurrentProcessInfo,
        const bool CalculateStiffnessMatrixFlag,
        const bool CalculateResidualVectorFlag
    )
    {
        KRATOS_TRY 
        // definition of problem size
        const unsigned int number_of_nodes = GetGeometry().size();
        unsigned int mat_size = number_of_nodes * 3;
/*
KRATOS_WATCH(number_of_nodes)
KRATOS_WATCH(rRightHandSideVector)
KRATOS_WATCH(rLeftHandSideMatrix)
*/

        //set up properties for Constitutive Law
        ConstitutiveLaw::Parameters Values(GetGeometry(), GetProperties(), rCurrentProcessInfo);

        Values.GetOptions().Set(ConstitutiveLaw::USE_ELEMENT_PROVIDED_STRAIN, true);
        Values.GetOptions().Set(ConstitutiveLaw::COMPUTE_STRESS);
        Values.GetOptions().Set(ConstitutiveLaw::COMPUTE_CONSTITUTIVE_TENSOR);

        //resize in iga_base_element

        //reading in of integration weight, shape function values and shape function derivatives
        //double integration_weight = this->GetValue(INTEGRATION_WEIGHT);
        Vector   N     = this->GetValue(SHAPE_FUNCTION_VALUES);
        Matrix  DN_De  = this->GetValue(SHAPE_FUNCTION_LOCAL_DERIVATIVES);
        
        //Metric and Constitutive Variables
        MetricVariables actual_metric(3);
        CalculateMetric(actual_metric);
        ConstitutiveVariables constitutive_variables_membrane(3);
        CalculateConstitutiveVariables(actual_metric,
            constitutive_variables_membrane,
            Values, ConstitutiveLaw::StressMeasure_PK2);

        //Calculate B Matrix 
        Matrix BMembrane = ZeroMatrix (3, mat_size);
        CalculateBMembrane(BMembrane, actual_metric);

        //KRATOS_WATCH(BMembrane)

        //Matrix B11;
        //Matrix B22;
        //Matrix B11;

        // Nonlinear Deformation
        SecondVariations second_variations_strain(mat_size);
        CalculateSecondVariationStrain(
            second_variations_strain,
            actual_metric);

        double integration_weight = this->GetValue(INTEGRATION_WEIGHT) * mInitialMetric.dA * GetProperties()[THICKNESS];
/*
        KRATOS_WATCH(rSecondVariationStrain.B11)
        KRATOS_WATCH(rSecondVariationStrain.B22)
        KRATOS_WATCH(rSecondVariationStrain.B12)
*/
        // Define Prestress
        double thickness = this->GetProperties().GetValue(THICKNESS);
        
        Vector S_prestress = this->GetProperties().GetValue(PRESTRESS);
        //Vector S_prestress_nichttransformiert = this->GetProperties().GetValue(PRESTRESS);
        //Vector S_prestress = prod(mInitialMetric.Q, S_prestress_nichttransformiert); //* thickness; 
        constitutive_variables_membrane.S = ZeroVector(3);
        Vector S_total = constitutive_variables_membrane.S + S_prestress;
        //Vector S_total = S_prestress;

        //KRATOS_WATCH(thickness)
        /*KRATOS_WATCH(S_prestress)
        KRATOS_WATCH(constitutive_variables_membrane.E)
        KRATOS_WATCH(constitutive_variables_membrane.S)
        KRATOS_WATCH(constitutive_variables_membrane.D)
        KRATOS_WATCH(S_total)*/

    //KRATOS_WATCH(rLeftHandSideMatrix)


        // LEFT HAND SIDE MATRIX

        if (CalculateStiffnessMatrixFlag == true) //aus shell_kl_discrete_element.cpp Zeile93
        {
            //adding membrane contributions to the stiffness matrix
            CalculateAndAddKm(
                rLeftHandSideMatrix,
                BMembrane, 
                constitutive_variables_membrane.D, 
                integration_weight);

    /*KRATOS_WATCH(rLeftHandSideMatrix)
    KRATOS_WATCH(constitutive_variables_membrane.D)*/

            // adding  non-linear-contribution to Stiffness-Matrix
            CalculateAndAddNonlinearKm(rLeftHandSideMatrix,
                second_variations_strain,
                S_total, //constitutive_variables_membrane.S,
                integration_weight);

    //KRATOS_WATCH(rLeftHandSideMatrix)

        }

        //RIGHT HAND SIDE 
          if (CalculateResidualVectorFlag == true) //calculation of the matrix is required
        {
            // operation performed: rRightHandSideVector -= Weight*IntForce
            //noalias(rRightHandSideVector) -= integration_weight * prod(trans(BMembrane), constitutive_variables_membrane.S);
            noalias(rRightHandSideVector) -= integration_weight * prod(trans(BMembrane), S_total);
        }

   /* KRATOS_WATCH(rLeftHandSideMatrix)
    KRATOS_WATCH(rRightHandSideVector)*/
    if(this->Id() == 0){KRATOS_WATCH(S_total)}
    

        KRATOS_CATCH("");
    }
        
      
void IgaMembraneElement::PrintInfo(std::ostream& rOStream) const
{
    rOStream << "\"IgaMembraneElement\" #" << Id();
}


// BENÖTIGE FUNKTIONEN

void IgaMembraneElement::CalculateMetric( 
        MetricVariables& metric
    )
    {
        const Matrix& DN_De = this->GetValue(SHAPE_FUNCTION_LOCAL_DERIVATIVES);
        const Matrix& DDN_DDe = this->GetValue(SHAPE_FUNCTION_LOCAL_SECOND_DERIVATIVES);

        
        //Calculate Jacobi
        IgaGeometryUtilities::CalculateJacobian(
            GetGeometry(),
            DN_De,
            3, //WorkingSpaceDimension,
            2, //LocalSpaceDimension,
            metric.J
        );
       
        metric.g1[0] = metric.J(0, 0);
        metric.g2[0] = metric.J(0, 1);
        metric.g1[1] = metric.J(1, 0);
        metric.g2[1] = metric.J(1, 1);
        metric.g1[2] = metric.J(2, 0);
        metric.g2[2] = metric.J(2, 1);

        //basis vector g3
        MathUtils<double>::CrossProduct(metric.g3, metric.g1, metric.g2);
        //differential area dA
        metric.dA = norm_2(metric.g3);
        //normal vector _n
        Vector n = metric.g3 / metric.dA;


        //GetcovariantMetric
        metric.gab[0] = pow(metric.g1[0], 2) + pow(metric.g1[1], 2) + pow(metric.g1[2], 2);
        metric.gab[1] = pow(metric.g2[0], 2) + pow(metric.g2[1], 2) + pow(metric.g2[2], 2);
        metric.gab[2] = metric.g1[0] * metric.g2[0] + metric.g1[1] * metric.g2[1] + metric.g1[2] * metric.g2[2];

       
        //contravariant metric gab_con and base vectors g_con
        //Vector gab_con = ZeroVector(3);
        double invdetGab = 1.0 / (metric.gab[0] * metric.gab[1] - metric.gab[2] * metric.gab[2]);
        metric.gab_con[0] = invdetGab*metric.gab[1];
        metric.gab_con[2] = -invdetGab*metric.gab[2];
        metric.gab_con[1] = invdetGab*metric.gab[0];


        array_1d<double, 3> g_con_1 = metric.g1*metric.gab_con[0] + metric.g2*metric.gab_con[2];
        array_1d<double, 3> g_con_2 = metric.g1*metric.gab_con[2] + metric.g2*metric.gab_con[1];


        //local cartesian coordinates
        double lg1 = norm_2(metric.g1);
        array_1d<double, 3> e1 = metric.g1 / lg1;
        double lg_con2 = norm_2(g_con_2);
        array_1d<double, 3> e2 = g_con_2 / lg_con2;

        Matrix mG = ZeroMatrix(2, 2);
        mG(0, 0) = inner_prod(e1, g_con_1);
        mG(0, 1) = inner_prod(e1, g_con_2);
        mG(1, 0) = inner_prod(e2, g_con_1);
        mG(1, 1) = inner_prod(e2, g_con_2);

        metric.Q = ZeroMatrix(3, 3);
        metric.Q(0, 0) = pow(mG(0, 0), 2);
        metric.Q(0, 1) = pow(mG(0, 1), 2);
        metric.Q(0, 2) = 2.00*mG(0, 0)*mG(0, 1);

        metric.Q(1, 0) = pow(mG(1, 0), 2);
        metric.Q(1, 1) = pow(mG(1, 1), 2);
        metric.Q(1, 2) = 2.00*mG(1, 0) * mG(1, 1);

        metric.Q(2, 0) = 2.00 * mG(0, 0) * mG(1, 0);
        metric.Q(2, 1) = 2.00 * mG(0, 1)*mG(1, 1);
        metric.Q(2, 2) = 2.00 * (mG(0, 0) * mG(1, 1) + mG(0, 1)*mG(1, 0));

        //Matrix T_G_E = ZeroMatrix(3, 3);
        //Transformation matrix T from contravariant to local cartesian basis
        double eG11 = inner_prod(e1, metric.g1);
        double eG12 = inner_prod(e1, metric.g2);
        double eG21 = inner_prod(e2, metric.g1);
        double eG22 = inner_prod(e2, metric.g2);

        //metric.Q = ZeroMatrix(3, 3);
        //metric.Q(0, 0) = eG11*eG11;
        //metric.Q(0, 1) = eG12*eG12;
        //metric.Q(0, 2) = 2.0*eG11*eG12;
        //metric.Q(1, 0) = eG21*eG21;
        //metric.Q(1, 1) = eG22*eG22;
        //metric.Q(1, 2) = 2.0*eG21*eG22;
        //metric.Q(2, 0) = 2.0*eG11*eG21;
        //metric.Q(2, 1) = 2.0*eG12*eG22;
        //metric.Q(2, 2) = 2.0*eG11*eG22 + eG12*eG21;

        metric.T = ZeroMatrix(3, 3);
        metric.T(0, 0) = eG11*eG11;
        metric.T(0, 1) = eG21*eG21;
        metric.T(0, 2) = 2.0*eG11*eG21;
        metric.T(1, 0) = eG12*eG12;
        metric.T(1, 1) = eG22*eG22;
        metric.T(1, 2) = 2.0*eG12*eG22;
        metric.T(2, 0) = eG11*eG12;
        metric.T(2, 1) = eG21*eG22;
        metric.T(2, 2) = eG11*eG22 + eG12*eG21;
    }


    void IgaMembraneElement::CalculateStrain(
        Vector& StrainVector,
        Vector& gab,
        Vector& gab0)

    {
        KRATOS_TRY

        StrainVector[0] = 0.5 * (gab[0] - gab0[0]);
        StrainVector[1] = 0.5 * (gab[1] - gab0[1]);
        StrainVector[2] = 0.5 * (gab[2] - gab0[2]);

        KRATOS_CATCH("")
    }


    void IgaMembraneElement::CalculateConstitutiveVariables(
        MetricVariables& rActualMetric,
        ConstitutiveVariables& rThisConstitutiveVariablesMembrane,
        ConstitutiveLaw::Parameters& rValues,
        const ConstitutiveLaw::StressMeasure ThisStressMeasure
    )
    {
        Vector strain_vector = ZeroVector(3);

       /* if (this->Id() == 0) {
            const rThisConstitutiveVariablesMembrane.E == [0, 0, 0 ]; }
        else {
        CalculateStrain(strain_vector, rActualMetric.gab, mInitialMetric.gab);
        rThisConstitutiveVariablesMembrane.E = prod(mInitialMetric.T, strain_vector); //geändert Q->T
        }*/
        CalculateStrain(strain_vector, rActualMetric.gab, mInitialMetric.gab);
        rThisConstitutiveVariablesMembrane.E = prod(mInitialMetric.T, strain_vector);

        //Constitive Matrices DMembrane and DCurvature
        rValues.SetStrainVector(rThisConstitutiveVariablesMembrane.E); //this is the input parameter
        rValues.SetStressVector(rThisConstitutiveVariablesMembrane.S);    //this is an ouput parameter
        rValues.SetConstitutiveMatrix(rThisConstitutiveVariablesMembrane.D); //this is an ouput parameter
        
        
        //rValues.CheckAllParameters();
        mConstitutiveLaw->CalculateMaterialResponse(rValues, ThisStressMeasure);
        //KRATOS_WATCH(rThisConstitutiveVariablesMembrane.D)

//        //double thickness = this->GetProperties().GetValue(THICKNESS);
        
        //Local Cartesian Forces and Moments
        rThisConstitutiveVariablesMembrane.S = prod(
            trans(rThisConstitutiveVariablesMembrane.D), rThisConstitutiveVariablesMembrane.E);
    }


        void IgaMembraneElement::CalculateBMembrane(
        Matrix& rB,
        const MetricVariables& metric)
    {
        const Matrix& DN_De = this->GetValue(SHAPE_FUNCTION_LOCAL_DERIVATIVES);

        const unsigned int number_of_control_points = GetGeometry().size();
        const unsigned int mat_size = number_of_control_points * 3;

        if (rB.size1() != mat_size || rB.size2() != mat_size)
            rB.resize(mat_size, mat_size);
        rB = ZeroMatrix(3, mat_size);

        for (int r = 0; r<static_cast<int>(mat_size); r++)
        {
            // local node number kr and dof direction dirr
            int kr = r / 3;
            int dirr = r % 3;

            Vector dE_curvilinear = ZeroVector(3);
            // strain
            dE_curvilinear[0] = DN_De(kr, 0)*metric.g1(dirr);
            dE_curvilinear[1] = DN_De(kr, 1)*metric.g2(dirr);
            dE_curvilinear[2] = 0.5*(DN_De(kr, 0)*metric.g2(dirr) + metric.g1(dirr)*DN_De(kr, 1));

            rB(0, r) = mInitialMetric.T(0, 0)*dE_curvilinear[0] + mInitialMetric.T(0, 1)*dE_curvilinear[1] + mInitialMetric.T(0, 2)*dE_curvilinear[2];  //geändert Q->T
            rB(1, r) = mInitialMetric.T(1, 0)*dE_curvilinear[0] + mInitialMetric.T(1, 1)*dE_curvilinear[1] + mInitialMetric.T(1, 2)*dE_curvilinear[2];  //geändert Q->T
            rB(2, r) = mInitialMetric.T(2, 0)*dE_curvilinear[0] + mInitialMetric.T(2, 1)*dE_curvilinear[1] + mInitialMetric.T(2, 2)*dE_curvilinear[2];  //geändert Q->T
        }
    }

void IgaMembraneElement::CalculateSecondVariationStrain(
        SecondVariations& rSecondVariationsStrain,
        const MetricVariables& rMetric)
    {
        const int number_of_control_points = GetGeometry().size();
        const int mat_size = number_of_control_points * 3;

        const Matrix& DN_De = this->GetValue(SHAPE_FUNCTION_LOCAL_DERIVATIVES);

        for (int r = 0; r<mat_size; r++)
        {
            // local node number kr and dof direction dirr
            int kr = r / 3;
            int dirr = r % 3;

            for (int s = 0; s <= r; s++)
            {
                // local node number ks and dof direction dirs
                int ks = s / 3;
                int dirs = s % 3;

                // strain
                Vector ddE_curvilinear = ZeroVector(3);
                if (dirr == dirs)
                {
                    ddE_curvilinear[0] = DN_De(kr, 0)*DN_De(ks, 0);
                    ddE_curvilinear[1] = DN_De(kr, 1)*DN_De(ks, 1);
                    ddE_curvilinear[2] = 0.5*(DN_De(kr, 0)*DN_De(ks, 1) + DN_De(kr, 1)*DN_De(ks, 0));
                }

                rSecondVariationsStrain.B11(r, s) = mInitialMetric.T(0, 0)*ddE_curvilinear[0] + mInitialMetric.T(0, 1)*ddE_curvilinear[1] + mInitialMetric.T(0, 2)*ddE_curvilinear[2];
                rSecondVariationsStrain.B22(r, s) = mInitialMetric.T(1, 0)*ddE_curvilinear[0] + mInitialMetric.T(1, 1)*ddE_curvilinear[1] + mInitialMetric.T(1, 2)*ddE_curvilinear[2];
                rSecondVariationsStrain.B12(r, s) = mInitialMetric.T(2, 0)*ddE_curvilinear[0] + mInitialMetric.T(2, 1)*ddE_curvilinear[1] + mInitialMetric.T(2, 2)*ddE_curvilinear[2];
            }
        }
    }
    
    //const std::size_t dof_type_n = GetDofTypeIndex(n);//NEU
    //const std::size_t dof_type_m = GetDofTypeIndex(m);//NEU

               // if (dof_type_n == dof_type_m) {//NEU

    void IgaMembraneElement::CalculateAndAddKm(
        MatrixType& rLeftHandSideMatrix,
        const Matrix& B,
        const Matrix& D,
        const double IntegrationWeight
    )
    {
        KRATOS_TRY
        noalias(rLeftHandSideMatrix) += IntegrationWeight * prod(trans(B), Matrix(prod(D, B)));
        KRATOS_CATCH("")    
    }


    void IgaMembraneElement::CalculateAndAddNonlinearKm(
        Matrix& rLeftHandSideMatrix,
        const SecondVariations& SecondVariationsStrain,
        const Vector& SD,
        const double& rIntegrationWeight)

    {
        KRATOS_TRY
        const int number_of_control_points = GetGeometry().size();
        const int mat_size = number_of_control_points * 3;

        for (int n = 0; n < mat_size; n++)
        {
            for (int m = 0; m <= n; m++)
            {
                //const std::size_t dof_type_n = GetDofTypeIndex(n);//NEU
                //const std::size_t dof_type_m = GetDofTypeIndex(m);//NEU

                //if (dof_type_n == dof_type_m) {//NEU
            
                double nm = (SD[0] * SecondVariationsStrain.B11(n, m)
                    + SD[1] * SecondVariationsStrain.B22(n, m)
                    + SD[2] * SecondVariationsStrain.B12(n, m))*rIntegrationWeight;
                //KRATOS_ERROR_IF(nm > 1e-5) << "Problem" << std::endl;
                rLeftHandSideMatrix(n, m) += nm;
                rLeftHandSideMatrix(m, n) += nm;
                //}//NEU
            }
        }
        KRATOS_CATCH("")
    }
}//namespace Kratos