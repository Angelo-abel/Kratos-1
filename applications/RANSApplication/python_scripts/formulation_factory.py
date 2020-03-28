# imports from kratos core
import KratosMultiphysics as Kratos
from KratosMultiphysics.process_factory import KratosProcessFactory

# imports from fluid dynamics application
import KratosMultiphysics.FluidDynamicsApplication as KratosCFD

# imports from rans application
import KratosMultiphysics.RANSApplication as KratosRANS
from KratosMultiphysics.RANSApplication.model_part_factory import CreateDuplicateModelPart
from KratosMultiphysics.RANSApplication.strategy_factory import CreateStrategy
from KratosMultiphysics.RANSApplication import RansCalculationUtilities

from KratosMultiphysics.RANSApplication.formulation_fractional_step_k_epsilon_high_re import FractionalStepKEpsilonHighRe
from KratosMultiphysics.RANSApplication.formulation_segregated_vms_k_epsilon_high_re import SegregatedVMSKEpsilonHighRe

def CreateFormulation(model_part, formulation_name, settings, scheme_settings):
    formulations_list = [
        ["segregated_vms_k_epsilon_high_re", SegregatedVMSKEpsilonHighRe],
        ["fractional_step_k_epsilon_high_re", FractionalStepKEpsilonHighRe]
    ]

    formulation_names_list = [formulations_list[i][0] for i in range(len(formulations_list))]
    formulation_list = [formulations_list[i][1] for i in range(len(formulations_list))]

    if (formulation_name not in formulation_names_list):
        msg = "Unknown formulation type=\"" + formulation_name + "\". \nFollowing formulations are allowed:\n    "
        msg += "\n    ".join(sorted(formulation_names_list))
        raise Exception(msg + "\n")

    current_formulation = formulation_list[formulation_names_list.index(formulation_name)](model_part, settings, scheme_settings)

    Kratos.Logger.PrintInfo("RANSFormulationFactory", "Created " + formulation_name + " formulation.")

    return current_formulation