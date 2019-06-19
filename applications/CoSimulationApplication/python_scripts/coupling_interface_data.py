from __future__ import print_function, absolute_import, division  # makes KratosMultiphysics backward compatible with python 2.6 and 2.7

# Other imports
from . import co_simulation_tools as cs_tools
import numpy as np

class CouplingInterfaceData(object):
    """This class serves as interface to the data structure (Model and ModelPart)
    that holds the data used during CoSimulation
    """
    def __init__(self, custom_config, model):

        default_config = cs_tools.cs_data_structure.Parameters("""{
            "model_part_name" : "UNSPECIFIED",
            "variable_name"   : "UNSPECIFIED",
            "location"        : "node_historical",
            "dimension"       : -1
        }""")
        custom_config.ValidateAndAssignDefaults(default_config)

        # saving the Model and ModelPart
        self.model = model
        self.model_part_name = custom_config["model_part_name"].GetString()

        # variable used to identify data
        variable_name = custom_config["variable_name"].GetString()
        variable_type = cs_tools.cs_data_structure.KratosGlobals.GetVariableType(variable_name)

        admissible_scalar_variable_types = ["Bool", "Integer", "Unsigned Integer", "Double", "Component"]
        admissible_vector_variable_types = ["Array"]

        if not variable_type in admissible_scalar_variable_types and not variable_type in admissible_vector_variable_types:
            raise Exception('The input for "variable" ("{}") is of variable-type "{}" which is not allowed, only the following options are possible:\n{}, {}'.format(variable_name, variable_type, ", ".join(admissible_scalar_variable_types), ", ".join(admissible_vector_variable_types)))

        self.variable = cs_tools.cs_data_structure.KratosGlobals.GetVariable(variable_name)

        self.is_scalar_variable = variable_type in admissible_scalar_variable_types

        # dimensionality of the data
        # TODO check that sth was assigned
        # TODO check DOMAIN_SIZE against the dimension???
        self.dimension = custom_config["dimension"].GetInt()

        # location of data on ModelPart
        self.location = custom_config["location"].GetString()
        admissible_locations = ["node_historical", "node_non_historical","element","condition","process_info","model_part"]
        if not self.location in admissible_locations:
            raise Exception('"{}" is not allowed as "location", only the following options are possible:\n{}'.format(self.location, ", ".join(admissible_locations)))


    def GetModelPart(self):
        return self.model[self.model_part_name]

    def IsDistributed(self):
        return self.GetModelPart().GetCommunicator().GetDataCommunicator().IsDistributed()

    def Size(self):
        if self.location in ["process_info", "model_part"]:
            return 1
        else:
            return len(self.GetDataContainer())

    def GetBufferSize(self):
        # only historical nodal data can store multiple steps!
        if self.location == "node_historical":
            return self.GetModelPart().GetBufferSize()
        else:
            return 1

    def GetPythonList(self, solution_step_index=0):
        model_part = self.GetModelPart()
        data = [0]*len(model_part.Nodes)*self.dimension
        node_index = 0
        for node in model_part.Nodes:
            data_value = node.GetSolutionStepValue(self.variable,solution_step_index) #TODO what if non-historical?
            for i in range(self.dimension):
                data[node_index*self.dimension + i] = data_value[i]
            node_index+=1
        return data

    def GetNumpyArray(self, solution_step_index=0):
        return np.asarray(self.GetPythonList(solution_step_index), dtype=np.float64)

    def ApplyUpdateToData(self, update):
        model_part = self.GetModelPart()
        node_index = 0
        updated_value = [0]*3 # TODO this is a hack, find better solution! => check var_type
        for node in model_part.Nodes: # #TODO: local nodes to also work in MPI?
            # TODO: aditya the data might also be non-historical => GetValue
            for i in range(self.dimension):
                updated_value[i] = update[node_index*self.dimension + i]

            node.SetSolutionStepValue(self.variable, 0, updated_value)
            node_index += 1
