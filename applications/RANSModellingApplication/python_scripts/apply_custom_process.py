import KratosMultiphysics as Kratos
import KratosMultiphysics.RANSModellingApplication as KratosRANS


def Factory(settings, Model):
    if (type(settings) != Kratos.Parameters):
        raise Exception(
            "expected input shall be a Parameters object, encapsulating a json string"
        )
    if (type(Model) != Kratos.Model):
        raise Exception("expected input shall be a Model object")

    allowed_processes_list = [
        ["ApplyFlagProcess", KratosRANS.RansApplyFlagProcess],
        ["FindNodalNeighboursProcess", FindNodalNeighboursProcess],
        [
            "FindConditionParentProcess",
            KratosRANS.RansFindConditionParentProcess
        ],
        [
            "ApplyScalarCellCenteredAveragingProcess",
            KratosRANS.RansScalarCellCenterAveragingProcess
        ],
        [
            "ApplyVectorCellCenteredAveragingProcess",
            KratosRANS.RansVectorCellCenterAveragingProcess
        ], ["ApplyVectorAlignProcess", KratosRANS.RansVectorAlignProcess],
        ["CalculateNormalsProcess", CalculateNormalsProcess],
        [
            "WallDistanceCalculationProcess",
            KratosRANS.RansWallDistanceCalculationProcess
        ],
        [
            "LogarithmicYPlusCalculationProcess",
            KratosRANS.RansLogarithmicYPlusCalculationProcess
        ],
        ["CheckScalarBoundsProcess", KratosRANS.RansCheckScalarBoundsProcess],
        [
            "EpsilonWallFunctionProcess",
            KratosRANS.RansEpsilonWallFunctionProcess
        ], ["NuTKWallFunctionProcess", KratosRANS.RansNutKWallFunctionProcess],
        [
            "NuTHighReCalculationProcess",
            KratosRANS.RansNutHighReCalculationProcess
        ],
        [
            "ApplyKTurbulentIntensityInletProcess",
            KratosRANS.RansKTurbulentIntensityInletProcess
        ],
        [
            "ApplyEpsilonTurbulentMixingLengthInletProcess",
            KratosRANS.RansEpsilonTurbulentMixingLengthInletProcess
        ],
        [
            "ApplyKWallFrictionVelocityProcess",
            KratosRANS.RansKWallFrictionVelocityProcess
        ],
        [
            "ApplyEpsilonWallFrictionVelocityProcess",
            KratosRANS.RansEpsilonWallFrictionVelocityProcess
        ],
        [
            "ClipScalarVariableProcess",
            KratosRANS.RansClipScalarVariableProcess
        ],
        [
            "ApplyExactNodalPeriodicConditionProcess",
            KratosRANS.RansApplyExactNodalPeriodicConditionProcess
        ],
        [
            "ApplyNutYPlusWallFunctionProcess",
            KratosRANS.RansNutYPlusWallFunctionProcess
        ],
        [
            "NuTLowReCalculationProcess",
            KratosRANS.RansNutLowReCalculationProcess
        ],
        [
            "CheckScalarConditionBoundsProcess",
            KratosRANS.RansCheckScalarConditionBoundsProcess
        ]
    ]

    process_name = settings["process_name"].GetString()

    process_names_list = [
        allowed_processes_list[i][0]
        for i in range(len(allowed_processes_list))
    ]
    process_list = [
        allowed_processes_list[i][1]
        for i in range(len(allowed_processes_list))
    ]

    if (process_name not in process_names_list):
        msg = "Unknown process_name=\"" + process_name + "\". \nFollowing process names are allowed:\n    "
        msg += "\n    ".join(sorted(process_names_list))
        raise Exception(msg + "\n")

    Kratos.Logger.PrintInfo("RANSApplyCustomProcess",
                            "Creating " + process_name)

    return process_list[process_names_list.index(process_name)](
        Model, settings["Parameters"])


class FindNodalNeighboursProcess(Kratos.Process):
    def __init__(self, Model, settings):
        Kratos.Process.__init__(self)

        default_parameters = Kratos.Parameters("""
            {
                "model_part_name"          : "PLEASE_CHOOSE_MODEL_PART_NAME",
                "average_neighbour_nodes"  : 10,
                "average_neighbour_element": 10
            }  """)

        settings.ValidateAndAssignDefaults(default_parameters)
        self.settings = settings

        self.model_part = Model[settings["model_part_name"].GetString()]
        self.process = Kratos.FindNodalNeighboursProcess(
            self.model_part,
            self.settings["average_neighbour_element"].GetInt(),
            self.settings["average_neighbour_nodes"].GetInt())

    def Check(self):
        self.process.Check()

    def ExecuteInitialize(self):
        self.process.Execute()
        Kratos.Logger.PrintInfo(
            "FindNodalNeighboursProcess",
            "Nodal neighbours found for nodes in " + self.model_part.Name +
            ".")

    def Execute(self):
        self.process.Execute()
        Kratos.Logger.PrintInfo(
            "FindNodalNeighboursProcess",
            "Nodal neighbours found for nodes in " + self.model_part.Name +
            ".")


class CalculateNormalsProcess(Kratos.Process):
    def __init__(self, Model, settings):
        Kratos.Process.__init__(self)

        default_parameters = Kratos.Parameters("""
            {
                "model_part_name"          : "PLEASE_CHOOSE_MODEL_PART_NAME"
            }  """)

        settings.ValidateAndAssignDefaults(default_parameters)
        self.settings = settings

        self.model_part = Model[settings["model_part_name"].GetString()]
        self.domain_size = self.model_part.ProcessInfo[Kratos.DOMAIN_SIZE]
        self.process = Kratos.NormalCalculationUtils()

    def ExecuteInitialize(self):
        self.process.CalculateOnSimplex(self.model_part, self.domain_size)
        Kratos.Logger.PrintInfo(
            "NormalCalculationUtils", "Nodal normals calculated for nodes in "
            + self.model_part.Name + ".")

    def Execute(self):
        self.process.CalculateOnSimplex(self.model_part, self.domain_size)
        Kratos.Logger.PrintInfo(
            "NormalCalculationUtils", "Nodal normals calculated for nodes in "
            + self.model_part.Name + ".")
