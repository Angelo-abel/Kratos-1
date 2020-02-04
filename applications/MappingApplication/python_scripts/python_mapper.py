import KratosMultiphysics as KM

class PythonMapper(object):
    """Baseclass for python based mappers in Kratos
    The inteface matches the C++ version ("custom_mappers/mapper.h")
    The py-mappers are intentionally NOT derived from the c++ version.
    Reasons:
    - Doing so would require some special treatment of the pure virtual functions exposed to python
    - They are more or less temporary until Kratos has more Mappers
    """
    def __init__(self, model_part_origin, model_part_destination, mapper_settings):
        self.model_part_origin = model_part_origin
        self.model_part_destination = model_part_destination

        if mapper_settings.Has("interface_submodel_part_origin"):
            self.model_part_origin = self.model_part_origin.GetSubModelPart(
                mapper_settings["interface_submodel_part_origin"].GetString())

        if mapper_settings.Has("interface_submodel_part_destination"):
            self.model_part_destination = self.model_part_destination.GetSubModelPart(
                mapper_settings["interface_submodel_part_destination"].GetString())

        self.mapper_settings = mapper_settings
        self.mapper_settings.ValidateAndAssignDefaults(self._GetDefaultSettings())

        self.echo_level = self.mapper_settings["echo_level"].GetInt()

    def Map(self, variable_origin, variable_destination, mapper_flags=KM.Flags()):
        raise NotImplementedError('"Map" was not implemented for "{}"'.format(self._ClassName()))

    def InverseMap(self, variable_origin, variable_destination, mapper_flags=KM.Flags()):
        raise NotImplementedError('"InverseMap" was not implemented for "{}"'.format(self._ClassName()))

    def UpdateInterface(self):
        raise NotImplementedError('"UpdateInterface" was not implemented for "{}"'.format(self._ClassName()))

    @classmethod
    def _GetDefaultSettings(cls):
        return KM.Parameters("""{
            "mapper_type" : "",
            "echo_level"  : 0,
            "interface_submodel_part_destination": "",
            "interface_submodel_part_origin": ""
        }""")

    @classmethod
    def _ClassName(cls):
        return cls.__name__
