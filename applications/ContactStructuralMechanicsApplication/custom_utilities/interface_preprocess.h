// KRATOS  ___|  |                   |                   |
//       \___ \  __|  __| |   |  __| __| |   |  __| _` | |
//             | |   |    |   | (    |   |   | |   (   | |
//       _____/ \__|_|   \__,_|\___|\__|\__,_|_|  \__,_|_| MECHANICS
//
//  License:             BSD License
//                                       license: StructuralMechanicsApplication/license.txt
//
//  Main authors:    Vicente Mataix Ferrandiz
// 


#if !defined(KRATOS_INTERFACE_PREPROCESS_CONDITION_H_INCLUDED )
#define  KRATOS_INTERFACE_PREPROCESS_CONDITION_H_INCLUDED

// System includes
#include <iostream>

// External includes

// Project includes
#include "includes/model_part.h"
#include "includes/define.h"
#include "includes/kratos_parameters.h"

namespace Kratos
{
///@name Kratos Globals
///@{
    
///@}
///@name Type Definitions
///@{

///@}
///@name  Enum's
///@{
    
///@}
///@name  Functions
///@{
    
///@}
///@name Kratos Classes
///@{
  
/**
 * @ingroup ContactStructuralMechanicsApplication
 * @class InterfacePreprocessCondition
 * @brief Creates Model Parts containing the interface
 * @todo Add parallelization
 * @author Vicente Mataix Ferrandiz
 */
class KRATOS_API(CONTACT_STRUCTURAL_MECHANICS_APPLICATION) InterfacePreprocessCondition
{
public:
    ///@name Type Definitions
    ///@{
    
    /// Geometric definitions
    typedef Point                                              PointType;
    typedef Node<3>                                             NodeType;
    typedef Geometry<NodeType>                              GeometryType;
    typedef Geometry<PointType>                        GeometryPointType;

    /// The index type
    typedef std::size_t                                        IndexType;

    /// The size type
    typedef std::size_t                                         SizeType;

    /// Definition of the entities container
    typedef ModelPart::NodesContainerType                 NodesArrayType;
    typedef ModelPart::ElementsContainerType           ElementsArrayType;
    typedef ModelPart::ConditionsContainerType       ConditionsArrayType;
    
    /// Pointer definition of ExactMortarIntegrationUtility
    KRATOS_CLASS_POINTER_DEFINITION(InterfacePreprocessCondition);
    
    ///@}
    ///@name Life Cycle
    ///@{
    
    /// Constructor
    
    /**
     * @brief This is the default constructor
     * @param rMainModelPrt The model part to consider
     */
    InterfacePreprocessCondition(ModelPart& rMainModelPrt)
    :mrMainModelPart(rMainModelPrt)
    {
    }
    
    /// Destructor.
    virtual ~InterfacePreprocessCondition() = default;
    
    ///@}
    ///@name Operators
    ///@{
    
    ///@}
    ///@name Operations
    ///@{

    /**
     * @brief Generate a new ModelPart containing only the interface. It will contain the conditions addressed in the call
     * @param rInterfacePart The interface model part
     * @param ThisParameters The configuration parameters
     */
    template<const std::size_t TDim>
    void GenerateInterfacePart(
        ModelPart& rInterfacePart,
        Parameters ThisParameters =  Parameters(R"({})")
        )
	{
		KRATOS_TRY;

		Parameters default_parameters = Parameters(R"(
		{
			"simplify_geometry"                    : false,
			"contact_property_id"                  : 0
		})");

		ThisParameters.ValidateAndAssignDefaults(default_parameters);

		const bool simplest_geometry = ThisParameters["simplify_geometry"].GetBool();
		const int contact_property_id = ThisParameters["contact_property_id"].GetInt();

		IndexType cond_counter = 0;

		// Generate Conditions from original the edges that can be considered interface
		if (rInterfacePart.Conditions().size() > 0) { // We use the already existant conditions geometry (recommended)
			cond_counter = rInterfacePart.Conditions().size();
			// Check and creates the properties
			CheckAndCreateProperties(rInterfacePart);
		} else if (rInterfacePart.Nodes().size() > 0) { // Only in case we have assigned the flag directly to nodes (no conditions)
													  // We reorder the conditions
			IndexType cond_id = ReorderConditions();

			// Store new properties in a map
			std::unordered_map<IndexType, Properties::Pointer> new_properties;
			if (contact_property_id == 0) new_properties = CreateNewProperties();

			if (TDim == 2) {
				// We iterate over the elements and check the nodes on the interface
				for (auto it_elem = mrMainModelPart.ElementsBegin(); it_elem != mrMainModelPart.ElementsEnd(); ++it_elem) {
					GeometryType& this_geometry = it_elem->GetGeometry();
					Properties::Pointer p_prop = (contact_property_id == 0) ? new_properties[it_elem->pGetProperties()->Id()] : mrMainModelPart.CreateNewProperties(contact_property_id);
					KRATOS_DEBUG_ERROR_IF(p_prop == nullptr) << "ERROR:: Property not well initialized" << std::endl;

					if (this_geometry.LocalSpaceDimension() == 2) {
						for (IndexType i_edge = 0; i_edge < this_geometry.EdgesNumber(); ++i_edge)
							GenerateEdgeCondition(rInterfacePart, p_prop, this_geometry.Edges()[i_edge], simplest_geometry, cond_counter, cond_id);
					} else {
						GenerateEdgeCondition(rInterfacePart, p_prop, this_geometry, simplest_geometry, cond_counter, cond_id);
					}
				}
			} else {
				// Generate Conditions from original the faces that can be considered interface
				for (auto it_elem = mrMainModelPart.ElementsBegin(); it_elem != mrMainModelPart.ElementsEnd(); ++it_elem) {
					GeometryType& this_geometry = it_elem->GetGeometry();
					Properties::Pointer p_prop = (contact_property_id == 0) ? new_properties[it_elem->pGetProperties()->Id()] : mrMainModelPart.CreateNewProperties(contact_property_id);
					KRATOS_DEBUG_ERROR_IF(p_prop == nullptr) << "ERROR:: Property not well initialized" << std::endl;

					if (this_geometry.LocalSpaceDimension() == 3) {
						for (IndexType i_face = 0; i_face < this_geometry.FacesNumber(); ++i_face)
							GenerateFaceCondition(rInterfacePart, p_prop, this_geometry.Faces()[i_face], simplest_geometry, cond_counter, cond_id);
					} else {
						GenerateFaceCondition(rInterfacePart, p_prop, this_geometry, simplest_geometry, cond_counter, cond_id);
					}
				}
			}
		} else {
			KRATOS_ERROR << "ERROR:: Nor conditions or nodes on the interface. Check your flags" << std::endl;
		}

		// NOTE: Reorder ID if parallellization

		const IndexType num_nodes = rInterfacePart.Nodes().size();
		PrintNodesAndConditions(num_nodes, cond_counter);

		KRATOS_CATCH("");
	}
    
protected:
    ///@name Protected static Member Variables
    ///@{

    ///@}
    ///@name Protected member Variables
    ///@{

    ///@}
    ///@name Protected Operators
    ///@{

    ///@}
    ///@name Protected Operations
    ///@{

    ///@}
    ///@name Protected  Access
    ///@{

    ///@}
    ///@name Protected Inquiry
    ///@{

    ///@}
    ///@name Protected LifeCycle
    ///@{
    ///@}
private:
    ///@name Static Member Variables
    ///@{
    
    ///@}
    ///@name Member Variables
    ///@{

    ModelPart& mrMainModelPart; /// The main model part storing all the information
    
    ///@}
    ///@name Private Operators
    ///@{

    ///@}
    ///@name Private Operations
    ///@{

    /**
     * @brief Check if the existing conditions have properties and if doesn't it creates it
     * @param rInterfacePart The interface model part
     */
    void CheckAndCreateProperties(ModelPart& rInterfacePart);

    /**
     * @brief Check if the existing combination exists on the geometry
     * @param IndexVector The vector containing the indexes of the nodes in the condition
     * @param rElementGeometry The element geometry
     */
    bool CheckOnTheFace(
        const std::vector<std::size_t>& rIndexVector,
        GeometryType& rElementGeometry
        );

    /**
     * @brief Creates a new properties (contaning just values related with contact)
     * @details These values are removed from the original property (in order to reduce overload of properties on the original elements)
     * @return A map containing new properties
     */
    std::unordered_map<IndexType, Properties::Pointer> CreateNewProperties();

    /**
     * @brief Copies a value from the original property to the new one
     * @param pOriginalProperty The original property
     * @param pNewProperty The new property
     * @param rVariable The variable to copy an erase
     */
    template<class TClass>
    void CopyProperties(
        Properties::Pointer pOriginalProperty,
        Properties::Pointer pNewProperty,
        const Variable<TClass>& rVariable,
        const bool AssignZero = true
        )
    {
        if(pOriginalProperty->Has(rVariable)) {
            const TClass& value = pOriginalProperty->GetValue(rVariable);
            pNewProperty->SetValue(rVariable, value);
        } else if (AssignZero) {
            KRATOS_INFO("InterfacePreprocessCondition") << "Property " << rVariable.Name() << " not available. Assigning zero value" << std::endl;
            pNewProperty->SetValue(rVariable, rVariable.Zero());
        }
    }

    /**
     * @brief Creates a new condition with a giving name
     * @param prThisProperties The pointer to the element
     * @param rGeometry The  geometry considered
     * @param CondId The Id of the condition
     * @param rCondition The base condition
     */

    void CreateNewCondition(
        Properties::Pointer prThisProperties,
        GeometryType& rGeometry,
        const IndexType CondId,
        Condition const& rCondition
        );
    
    /**
     * @brief This method assign the corresponding master/slave flag to the condition in function of its nodes
     * @param pCond The pointer to the condition
     */
    void AssignMasterSlaveCondition(Condition::Pointer pCond);

    /**
     * @brief It prints the nodes and conditions in the interface, gives an error otherwise there are not
     * @param NodesCounter Number of nodes in the interface
     * @param CondCounter Number of conditions in the interface
     */
    void PrintNodesAndConditions(
        const IndexType NodesCounter,
        const IndexType CondCounter
        );
    
    /**
     * @brief It reorders the Ids of the conditions
     * @return cond_id: The Id from the last condition
     */
    IndexType ReorderConditions();
    
    /**
     * @brief This method creates the conditions for the edges
     * @param rInterfacePart The model part of the interface
     * @param prThisProperties The properties of the base element
     * @param EdgeGeometry Geometry considered
     * @param SimplestGeometry If consider or not the simplest geometry
     * @param CondCounter The counter of conditions
     * @param CondId The condition id
     */
    inline void GenerateEdgeCondition(
        ModelPart& rInterfacePart,
        Properties::Pointer prThisProperties,
        GeometryType& EdgeGeometry,
        const bool SimplestGeometry,
        IndexType& CondCounter,
        IndexType& CondId
        );
    
    /**
     * @brief This method creates the conditions for the faces
     * @param rInterfacePart The model part of the interface
     * @param prThisProperties The properties of the base element
     * @param FaceGeometry Geometry considered
     * @param SimplestGeometry If consider or not the simplest geometry
     * @param CondCounter The counter of conditions
     * @param CondId The condition id
     */
    inline void GenerateFaceCondition(
        ModelPart& rInterfacePart,
        Properties::Pointer prThisProperties,
        GeometryType& FaceGeometry,
        const bool SimplestGeometry,
        IndexType& CondCounter,
        IndexType& CondId
        );
    
    ///@}
    ///@name Private  Access
    ///@{
    ///@}

    ///@}
    ///@name Serialization
    ///@{

    ///@name Private Inquiry
    ///@{
    ///@}

    ///@name Unaccessible methods
    ///@{
    ///@}
}; // Class InterfacePreprocessCondition
}
#endif  /* KRATOS_INTERFACE_PREPROCESS_CONDITION_H_INCLUDED defined */
