//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:		 BSD License
//					 Kratos default license: kratos/license.txt
//
//  Main authors:    Pooyan Dadvand
//

// System includes


// External includes


// Project includes
#include "testing/testing.h"
#include "processes/structured_mesh_generator_process.h"
#include "processes/tetrahedra_mesh_edge_swapping_process.h"
#include "processes/find_nodal_neighbours_process.h"
#include "geometries/tetrahedra_3d_4.h"
#include "geometries/hexahedra_3d_8.h"
#include "includes/gid_io.h"
#include "utilities/geometry_utilities.h"


namespace Kratos {
	namespace Testing {


		KRATOS_TEST_CASE_IN_SUITE(TetrahedraMeshEdgeSwappingRotatingBlock3x3, KratosCoreFastSuite)
		{
			Point<3>::Pointer p_point1(new Point<3>(-10.00, -10.00, -10.00));
			Point<3>::Pointer p_point2(new Point<3>(10.00, -10.00, -10.00));
			Point<3>::Pointer p_point3(new Point<3>(10.00, 10.00, -10.00));
			Point<3>::Pointer p_point4(new Point<3>(-10.00, 10.00, -10.00));
			Point<3>::Pointer p_point5(new Point<3>(-10.00, -10.00, 10.00));
			Point<3>::Pointer p_point6(new Point<3>(10.00, -10.00, 10.00));
			Point<3>::Pointer p_point7(new Point<3>(10.00, 10.00, 10.00));
			Point<3>::Pointer p_point8(new Point<3>(-10.00, 10.00, 10.00));

			Hexahedra3D8<Point<3> > geometry(p_point1, p_point2, p_point3, p_point4, p_point5, p_point6, p_point7, p_point8);

			ModelPart model_part("Test");

			Parameters mesher_parameters(R"(
            {
                "number_of_divisions":3,
                "element_name": "Element3D4N"
            }  )");

			//std::size_t number_of_divisions = mesher_parameters["number_of_divisions"].GetInt();

			StructuredMeshGeneratorProcess(geometry, model_part, mesher_parameters).Execute();

			GidIO<> gid_io("c:/temp/coarsening/edge_swapping_test", GiD_PostAscii, SingleFile, WriteDeformed, WriteConditions);
			gid_io.InitializeMesh(0.00);
			gid_io.WriteMesh(model_part.GetMesh());
			gid_io.FinalizeMesh();

			ModelPart& r_skin_model_part = model_part.GetSubModelPart("Skin");

			for (auto i_node = r_skin_model_part.NodesBegin(); i_node != r_skin_model_part.NodesEnd(); i_node++)
				i_node->Set(BOUNDARY);

			FindNodalNeighboursProcess(model_part).Execute();

			TetrahedraMeshEdgeSwappingProcess(model_part).Execute();
			
			 gid_io.InitializeMesh(1.00);
			 gid_io.WriteMesh(model_part.GetMesh());
			 gid_io.FinalizeMesh();

			 std::vector<Node<3>*> moving_nodes; // Is not the way to store nodes outside the test!
			 for (auto& node : model_part.Nodes())
			 {
				 double dx = node.X();
				 double dy = node.Y();
				 double distance = std::sqrt(dx*dx+dy*dy);
				 if (distance < 6. && node.Z() < 5.5 && node.Z() > -5.50)
					 moving_nodes.push_back(&node);
			 }
			 int inverted_element_counter = 0;
			 for (auto& element : model_part.Elements()) {
				 if (element.GetGeometry().Volume() < 0.00/*std::numeric_limits<double>::epsilon()*/)
					 inverted_element_counter++;
			 }
			 
			 KRATOS_WATCH(inverted_element_counter);


			 constexpr double pi = 3.141592653589793238462643383279;
			 for (int i = 10; i < 180; i+=10) {
				 const double alpha = 10.00 * pi / 180.00;
				 const double s = sin(alpha);
				 const double c = cos(alpha);
				 for (auto p_node : moving_nodes) {
					 double new_x = p_node->X()*c - p_node->Y()*s;
					 double new_y = p_node->X()*s + p_node->Y()*c;
					 p_node->X() = new_x;
					 p_node->Y() = new_y;
				 }

				 FindNodalNeighboursProcess(model_part).Execute();

				 TetrahedraMeshEdgeSwappingProcess swapping_process(model_part);
				 swapping_process.Execute();
				 KRATOS_WATCH(swapping_process);

				 int inverted_element_counter = 0;
				 for (auto& element : model_part.Elements()) {
					 if (element.GetGeometry().Volume() < std::numeric_limits<double>::epsilon())
						 inverted_element_counter++;
				 }

				 KRATOS_WATCH(inverted_element_counter);
				 gid_io.InitializeMesh(i);
				 gid_io.WriteMesh(model_part.GetMesh());
				 gid_io.FinalizeMesh();

			 }


		}
		
		KRATOS_TEST_CASE_IN_SUITE(Tetrahedra3to2EdgeSwappingProcess, KratosCoreFastSuite)
		{
			ModelPart model_part("Test");

			model_part.CreateNewNode(1, 0.00, 0.00, 0.00);
			model_part.CreateNewNode(2, 10.00, 0.00, 0.00);
			model_part.CreateNewNode(3, 10.00, 10.00, 0.00);
			model_part.CreateNewNode(100, 1.00, 1.00, -10.00);
			model_part.CreateNewNode(101, 1.00, 1.00, 10.00);

			Properties::Pointer p_properties(new Properties(0));
			model_part.CreateNewElement("Element3D4N", 1, { 100,101,3,1 }, p_properties);
			model_part.CreateNewElement("Element3D4N", 2, { 100,101,2,3 }, p_properties);
			model_part.CreateNewElement("Element3D4N", 3, { 101,100,2,1 }, p_properties);


			FindNodalNeighboursProcess(model_part).Execute();

			GidIO<> gid_io("c:/temp/coarsening/edge_swapping_3to2_test", GiD_PostAscii, SingleFile, WriteDeformed, WriteConditions);
			gid_io.InitializeMesh(0.00);
			gid_io.WriteMesh(model_part.GetMesh());
			gid_io.FinalizeMesh();
			TetrahedraMeshEdgeSwappingProcess(model_part).Execute();

			KRATOS_CHECK_EQUAL(model_part.NumberOfElements(), 2);
			KRATOS_CHECK_GREATER(model_part.GetElement(1).GetGeometry().Volume(), 166.);
			KRATOS_CHECK_GREATER(model_part.GetElement(2).GetGeometry().Volume(), 166.);

			 gid_io.InitializeMesh(1.00);
			 gid_io.WriteMesh(model_part.GetMesh());
			 gid_io.FinalizeMesh();



		}

		KRATOS_TEST_CASE_IN_SUITE(Tetrahedra4to4EdgeSwappingProcess, KratosCoreFastSuite)
		{
			ModelPart model_part("Test");

			model_part.CreateNewNode(1, 0.00, 0.00, 0.00);
			model_part.CreateNewNode(2, 10.00, 0.00, 0.00);
			model_part.CreateNewNode(3, 10.00, 10.00, 0.00);
			model_part.CreateNewNode(4, 0.00, 10.00, 0.00);

			model_part.CreateNewNode(100, 1.00, 1.00, -10.00);
			model_part.CreateNewNode(101, 1.00, 1.00, 10.00);

			Properties::Pointer p_properties(new Properties(0));
			model_part.CreateNewElement("Element3D4N", 1, { 100,101,3,4 }, p_properties);
			model_part.CreateNewElement("Element3D4N", 2, { 100,101,2,3 }, p_properties);
			model_part.CreateNewElement("Element3D4N", 3, { 101,100,2,1 }, p_properties);
			model_part.CreateNewElement("Element3D4N", 4, { 101,100,1,4 }, p_properties);


			FindNodalNeighboursProcess(model_part).Execute();

			GidIO<> gid_io("c:/temp/coarsening/edge_swapping_4to4_test", GiD_PostAscii, SingleFile, WriteDeformed, WriteConditions);
			gid_io.InitializeMesh(0.00);
			gid_io.WriteMesh(model_part.GetMesh());
			gid_io.FinalizeMesh();
			TetrahedraMeshEdgeSwappingProcess(model_part).Execute();

			KRATOS_CHECK_EQUAL(model_part.NumberOfElements(), 4);
			KRATOS_CHECK_GREATER(model_part.GetElement(1).GetGeometry().Volume(), 166.);
			KRATOS_CHECK_GREATER(model_part.GetElement(2).GetGeometry().Volume(), 166.);
			KRATOS_CHECK_GREATER(model_part.GetElement(3).GetGeometry().Volume(), 166.);
			KRATOS_CHECK_GREATER(model_part.GetElement(4).GetGeometry().Volume(), 166.);

			gid_io.InitializeMesh(1.00);
			gid_io.WriteMesh(model_part.GetMesh());
			gid_io.FinalizeMesh();
		}

		KRATOS_TEST_CASE_IN_SUITE(Tetrahedra5to6EdgeSwappingProcess, KratosCoreFastSuite)
		{
			ModelPart model_part("Test");

			model_part.CreateNewNode(1, 0.00, 0.00, 0.00);
			model_part.CreateNewNode(2, 10.00, 0.00, 0.00);
			model_part.CreateNewNode(3, 10.00, 10.00, 0.00);
			model_part.CreateNewNode(4, 0.00, 10.00, 0.00); 
			model_part.CreateNewNode(5, 4.00, 6.00, 0.00);

			model_part.CreateNewNode(100, 1.00, 1.00, -10.00);
			model_part.CreateNewNode(101, 1.00, 1.00, 10.00);

			Properties::Pointer p_properties(new Properties(0));
			model_part.CreateNewElement("Element3D4N", 1, { 100,101,3,4 }, p_properties);
			model_part.CreateNewElement("Element3D4N", 2, { 100,101,2,3 }, p_properties);
			model_part.CreateNewElement("Element3D4N", 3, { 101,100,2,1 }, p_properties);
			model_part.CreateNewElement("Element3D4N", 4, { 101,100,5,4 }, p_properties);
			model_part.CreateNewElement("Element3D4N", 5, { 101,100,1,5 }, p_properties);

			FindNodalNeighboursProcess(model_part).Execute();

			//GidIO<> gid_io("c:/temp/coarsening/edge_swapping_5to6_test", GiD_PostAscii, SingleFile, WriteDeformed, WriteConditions);
			//gid_io.InitializeMesh(0.00);
			//gid_io.WriteMesh(model_part.GetMesh());
			//gid_io.FinalizeMesh();
			TetrahedraMeshEdgeSwappingProcess(model_part).Execute();

			KRATOS_CHECK_EQUAL(model_part.NumberOfElements(), 6);
			KRATOS_CHECK_GREATER(model_part.GetElement(1).GetGeometry().Volume(), 66.);
			KRATOS_CHECK_GREATER(model_part.GetElement(2).GetGeometry().Volume(), 66.);
			KRATOS_CHECK_GREATER(model_part.GetElement(3).GetGeometry().Volume(), 99.);
			KRATOS_CHECK_GREATER(model_part.GetElement(4).GetGeometry().Volume(), 99.);
			KRATOS_CHECK_GREATER(model_part.GetElement(5).GetGeometry().Volume(), 99.);
			KRATOS_CHECK_GREATER(model_part.GetElement(6).GetGeometry().Volume(), 99.);

			//gid_io.InitializeMesh(1.00);
			//gid_io.WriteMesh(model_part.GetMesh());
			//gid_io.FinalizeMesh();
		}

		KRATOS_TEST_CASE_IN_SUITE(Tetrahedra6to8EdgeSwappingProcess, KratosCoreFastSuite)
		{
			ModelPart model_part("Test");

			model_part.CreateNewNode(1, 0.00, 0.00, 2.00);
			model_part.CreateNewNode(2, 10.00, 0.00, 0.00);
			model_part.CreateNewNode(3, 10.00, 10.00, 2.00);
			model_part.CreateNewNode(4, 0.00, 10.00, 0.00);
			model_part.CreateNewNode(5, -2.00, 6.00, 2.00);
			model_part.CreateNewNode(6, -2.00, 2.00, 0.00);

			model_part.CreateNewNode(100, 1.00, 1.00, -10.00);
			model_part.CreateNewNode(101, 1.00, 1.00, 10.00);

			Properties::Pointer p_properties(new Properties(0));
			model_part.CreateNewElement("Element3D4N", 1, { 100,101,3,4 }, p_properties);
			model_part.CreateNewElement("Element3D4N", 2, { 100,101,2,3 }, p_properties);
			model_part.CreateNewElement("Element3D4N", 3, { 101,100,2,1 }, p_properties);
			model_part.CreateNewElement("Element3D4N", 4, { 101,100,5,4 }, p_properties);
			model_part.CreateNewElement("Element3D4N", 5, { 101,100,6,5 }, p_properties);
			model_part.CreateNewElement("Element3D4N", 6, { 101,100,1,6 }, p_properties);

			FindNodalNeighboursProcess(model_part).Execute();

			GidIO<> gid_io("c:/temp/coarsening/edge_swapping_6to8_test", GiD_PostAscii, SingleFile, WriteDeformed, WriteConditions);
			gid_io.InitializeMesh(0.00);
			gid_io.WriteMesh(model_part.GetMesh());
			gid_io.FinalizeMesh();
			TetrahedraMeshEdgeSwappingProcess(model_part).Execute();

			KRATOS_CHECK_EQUAL(model_part.NumberOfElements(), 8);
			KRATOS_WATCH(model_part.GetElement(1).GetGeometry().Volume());
			KRATOS_WATCH(model_part.GetElement(2).GetGeometry().Volume());
			KRATOS_WATCH(model_part.GetElement(3).GetGeometry().Volume());
			KRATOS_WATCH(model_part.GetElement(4).GetGeometry().Volume());
			KRATOS_WATCH(model_part.GetElement(5).GetGeometry().Volume());
			KRATOS_WATCH(model_part.GetElement(6).GetGeometry().Volume());
			KRATOS_WATCH(model_part.GetElement(7).GetGeometry().Volume());
			KRATOS_WATCH(model_part.GetElement(8).GetGeometry().Volume());
			KRATOS_CHECK_GREATER(model_part.GetElement(1).GetGeometry().Volume(), 199.);
			KRATOS_CHECK_GREATER(model_part.GetElement(2).GetGeometry().Volume(), 133.);
			KRATOS_CHECK_GREATER(model_part.GetElement(3).GetGeometry().Volume(), 103.);
			KRATOS_CHECK_GREATER(model_part.GetElement(4).GetGeometry().Volume(), 29.);
			KRATOS_CHECK_GREATER(model_part.GetElement(5).GetGeometry().Volume(), 159.);
			KRATOS_CHECK_GREATER(model_part.GetElement(6).GetGeometry().Volume(), 106.);
			KRATOS_CHECK_GREATER(model_part.GetElement(7).GetGeometry().Volume(), 18.);
			KRATOS_CHECK_GREATER(model_part.GetElement(8).GetGeometry().Volume(), 7.);

			gid_io.InitializeMesh(1.00);
			gid_io.WriteMesh(model_part.GetMesh());
			gid_io.FinalizeMesh();
		}

		KRATOS_TEST_CASE_IN_SUITE(Tetrahedra7to10EdgeSwappingProcess, KratosCoreFastSuite)
		{
			ModelPart model_part("Test");

			model_part.CreateNewNode(1, 0.00, 0.00, 1.00);
			model_part.CreateNewNode(2, 10.00, 0.00, 0.00);
			model_part.CreateNewNode(3, 10.00, 10.00, 5.00);
			model_part.CreateNewNode(4, 0.00, 10.00, 1.00);
			model_part.CreateNewNode(5, -2.00, 8.00, 1.00);
			model_part.CreateNewNode(6, -4.00, 5.00, 1.00);
			model_part.CreateNewNode(7, -2.00, 2.00, 1.00);

			model_part.CreateNewNode(100, 1.00, 1.00, -10.00);
			model_part.CreateNewNode(101, 1.00, 1.00, 10.00);

			Properties::Pointer p_properties(new Properties(0));
			model_part.CreateNewElement("Element3D4N", 1, { 100,101,3,4 }, p_properties);
			model_part.CreateNewElement("Element3D4N", 2, { 100,101,2,3 }, p_properties);
			model_part.CreateNewElement("Element3D4N", 3, { 101,100,2,1 }, p_properties);
			model_part.CreateNewElement("Element3D4N", 4, { 101,100,5,4 }, p_properties);
			model_part.CreateNewElement("Element3D4N", 5, { 101,100,6,5 }, p_properties);
			model_part.CreateNewElement("Element3D4N", 6, { 101,100,7,6 }, p_properties);
			model_part.CreateNewElement("Element3D4N", 7, { 101,100,1,7 }, p_properties);

			KRATOS_WATCH(model_part.GetElement(1).GetGeometry().Volume());
			KRATOS_WATCH(model_part.GetElement(2).GetGeometry().Volume());
			KRATOS_WATCH(model_part.GetElement(3).GetGeometry().Volume());
			KRATOS_WATCH(model_part.GetElement(4).GetGeometry().Volume());
			KRATOS_WATCH(model_part.GetElement(5).GetGeometry().Volume());
			KRATOS_WATCH(model_part.GetElement(6).GetGeometry().Volume());
			KRATOS_WATCH(model_part.GetElement(7).GetGeometry().Volume());
			FindNodalNeighboursProcess(model_part).Execute();

			GidIO<> gid_io("c:/temp/coarsening/edge_swapping_7to10_test", GiD_PostAscii, SingleFile, WriteDeformed, WriteConditions);
			gid_io.InitializeMesh(0.00);
			gid_io.WriteMesh(model_part.GetMesh());
			gid_io.FinalizeMesh();
			TetrahedraMeshEdgeSwappingProcess(model_part).Execute();

			KRATOS_WATCH(model_part.GetElement(1).GetGeometry().Volume());
			KRATOS_WATCH(model_part.GetElement(2).GetGeometry().Volume());
			KRATOS_WATCH(model_part.GetElement(3).GetGeometry().Volume());
			KRATOS_WATCH(model_part.GetElement(4).GetGeometry().Volume());
			KRATOS_WATCH(model_part.GetElement(5).GetGeometry().Volume());
			KRATOS_WATCH(model_part.GetElement(6).GetGeometry().Volume());
			KRATOS_WATCH(model_part.GetElement(7).GetGeometry().Volume());
			KRATOS_WATCH(model_part.GetElement(8).GetGeometry().Volume());
			KRATOS_WATCH(model_part.GetElement(9).GetGeometry().Volume());
			KRATOS_WATCH(model_part.GetElement(10).GetGeometry().Volume());
			KRATOS_CHECK_EQUAL(model_part.NumberOfElements(), 10);
			KRATOS_CHECK_GREATER(model_part.GetElement(1).GetGeometry().Volume(), 189.);
			KRATOS_CHECK_GREATER(model_part.GetElement(2).GetGeometry().Volume(), 143.);
			KRATOS_CHECK_GREATER(model_part.GetElement(3).GetGeometry().Volume(), 189.);
			KRATOS_CHECK_GREATER(model_part.GetElement(4).GetGeometry().Volume(), 143.);
			KRATOS_CHECK_GREATER(model_part.GetElement(5).GetGeometry().Volume(), 36.);
			KRATOS_CHECK_GREATER(model_part.GetElement(6).GetGeometry().Volume(), 29.);
			KRATOS_CHECK_GREATER(model_part.GetElement(7).GetGeometry().Volume(), 21.);
			KRATOS_CHECK_GREATER(model_part.GetElement(8).GetGeometry().Volume(), 17.);
			KRATOS_CHECK_GREATER(model_part.GetElement(9).GetGeometry().Volume(), 21.);
			KRATOS_CHECK_GREATER(model_part.GetElement(10).GetGeometry().Volume(), 17.);

			gid_io.InitializeMesh(1.00);
			gid_io.WriteMesh(model_part.GetMesh());
			gid_io.FinalizeMesh();
		}

	}
}  // namespace Kratos.
