from __future__ import print_function, absolute_import, division #makes KratosMultiphysics backward compatible with python 2.6 and 2.7
import KratosMultiphysics as KM
from KratosCSharpWrapperApplication import *
application = KratosCSharpWrapperApplication()
application_name = "KratosCSharpWrapperApplication"
application_folder = "CSharpWrapperApplication"

KM._ImportApplicationAsModule(application, application_name, application_folder, __path__)
