from __future__ import print_function, absolute_import, division  # makes KratosMultiphysics backward compatible with python 2.6 and 2.7

# Importing the Kratos Library
import KratosMultiphysics

# other imports
from KratosMultiphysics.MeshMovingApplication.ale_fluid_solver import AleFluidSolver
import KratosMultiphysics.CompressiblePotentialFlowApplication.potential_flow_solver as potential_flow_solver

def CreateSolver(model, solver_settings, parallelism):
    return AlePotentialFlowSolver(model, solver_settings, parallelism)


class AlePotentialFlowSolver(AleFluidSolver):
    def __init__(self, model, solver_settings, parallelism):
        super(AlePotentialFlowSolver, self).__init__(model, solver_settings, parallelism)
        self.fluid_solver.min_buffer_size = 2

    def _CreateFluidSolver(self, solver_settings, parallelism):
        return potential_flow_solver.CreateSolver(self.model, solver_settings)

    def AdvanceInTime(self, current_time):
        return 0.0 # potential flow is steady state

    def SolveSolutionStep(self):
        for mesh_solver in self.mesh_motion_solvers:
            mesh_solver.SolveSolutionStep()
        self.fluid_solver.SolveSolutionStep()