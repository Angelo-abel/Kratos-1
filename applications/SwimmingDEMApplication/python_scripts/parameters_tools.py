from __future__ import print_function, absolute_import, division # makes KratosMultiphysics backward compatible with python 2.6 and 2.7
import KratosMultiphysics as KM
from KratosMultiphysics.FluidDynamicsApplication import *
from KratosMultiphysics.DEMApplication import *
from KratosMultiphysics.SwimmingDEMApplication import *


def CanBeIterated(my_object):
    if isinstance(my_object, str):
        return False
    else:
        try:
            _ = my_object[0]
            return True
        except Exception:
            return False

def RecursiveFindTrueBoolInParameters(parameters, key):

    if parameters.IsSubParameter():
        def Condition(p, k):
            if p.Has(k):
                return p[k].GetBool()

        if Condition(parameters, key):
            return True
        else:
            for k in parameters.keys():
                if RecursiveFindTrueBoolInParameters(parameters[k], key):
                    return True
            return False

    elif CanBeIterated(parameters): # there could be lists

        for i, _ in enumerate(parameters):
            if RecursiveFindTrueBoolInParameters(parameters[i], key):
                return True
        return False

    else:
        return False

def RecursiveFindParametersWithCondition(parameters, key, condition=lambda value: True):

    if parameters.IsSubParameter():
        def Condition(p, k):
            if p.Has(k):
                if condition(p[k]):
                    return True
                else:
                    return False

        if Condition(parameters, key):
            return True
        else:
            for k in parameters.keys():
                if RecursiveFindParametersWithCondition(parameters[k], key, condition):
                    return True
            return False

    elif CanBeIterated(parameters): # there could be lists

        for i, _ in enumerate(parameters):
            if RecursiveFindParametersWithCondition(parameters[i], key, condition):
                return True
        return False

    else:
        return False
