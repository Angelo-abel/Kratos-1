from __future__ import print_function, absolute_import, division  # makes KratosMultiphysics backward compatible with python 2.6 and 2.7

import KratosMultiphysics as KM
import KratosMultiphysics.KratosUnittest as KratosUnittest
import basic_mapper_tests
import os

@KratosUnittest.skipUnless("EMPIRE_API_LIBSO_ON_MACHINE" in os.environ, "Empire libs not found")
class EmpireNearestNeighborMapper(basic_mapper_tests.BasicMapperTests):
    @classmethod
    def setUpClass(cls):
        mapper_params = KM.Parameters("""{
            "mapper_type": "empire_nearest_neighbor",
            "echo_level" : 0
        }""")
        super(EmpireNearestNeighborMapper, cls).setUpMapper(mapper_params, False, "origin.surface_tri", "destination.surface_quad")

    @classmethod
    def tearDownClass(cls):
        del(cls.mapper)


@KratosUnittest.skipUnless("EMPIRE_API_LIBSO_ON_MACHINE" in os.environ, "Empire libs not found")
class EmpireNearestElementMapper(basic_mapper_tests.BasicMapperTests):
    @classmethod
    def setUpClass(cls):
        mapper_params = KM.Parameters("""{
            "mapper_type": "empire_nearest_element",
            "echo_level" : 0
        }""")
        super(EmpireNearestElementMapper, cls).setUpMapper(mapper_params, False, "origin.surface_tri", "destination.surface_quad")

    @classmethod
    def tearDownClass(cls):
        del(cls.mapper)


@KratosUnittest.skipUnless("EMPIRE_API_LIBSO_ON_MACHINE" in os.environ, "Empire libs not found")
class EmpireBarycentricMapper(basic_mapper_tests.BasicMapperTests):
    @classmethod
    def setUpClass(cls):
        mapper_params = KM.Parameters("""{
            "mapper_type": "empire_barycentric",
            "echo_level" : 0
        }""")
        super(EmpireBarycentricMapper, cls).setUpMapper(mapper_params, False, "origin.surface_tri", "destination.surface_quad")

    @classmethod
    def tearDownClass(cls):
        del(cls.mapper)


@KratosUnittest.skipUnless("EMPIRE_API_LIBSO_ON_MACHINE" in os.environ, "Empire libs not found")
class EmpireMortarMapper(basic_mapper_tests.BasicMapperTests):
    @classmethod
    def setUpClass(cls):
        mapper_params = KM.Parameters("""{
            "mapper_type": "empire_mortar",
            "echo_level" : 0
        }""")
        super(EmpireMortarMapper, cls).setUpMapper(mapper_params, False, "origin.surface_tri", "destination.surface_quad")

    @classmethod
    def tearDownClass(cls):
        del(cls.mapper)


if __name__ == '__main__':
    # KM.Logger.GetDefaultOutput().SetSeverity(KM.Logger.Severity.WARNING)
    import KratosMultiphysics.KratosUnittest as KratosUnittest
    KratosUnittest.main()