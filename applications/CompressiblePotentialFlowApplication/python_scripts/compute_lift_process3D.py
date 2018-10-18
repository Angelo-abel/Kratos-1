from KratosMultiphysics import *

import KratosMultiphysics

from array import array
from numpy import *
from math import *
import numpy as np
import itertools
#import matplotlib.pyplot as plt



def Factory(settings, Model):
    if(type(settings) != KratosMultiphysics.Parameters):
        raise Exception("expected input shall be a Parameters object, encapsulating a json string")
    return ComputeLiftProcess3D(Model, settings["Parameters"])

##all the processes python processes should be derived from "python_process"
class ComputeLiftProcess3D(KratosMultiphysics.Process):
    def __init__(self, Model, settings ):
        KratosMultiphysics.Process.__init__(self) 
        
        default_parameters = KratosMultiphysics.Parameters( """
            {
                "model_part_name"   : "PLEASE_CHOOSE_MODEL_PART_NAME",
                "upper_surface_model_part_name" : "please specify the model part that contains the upper surface nodes",
                "lower_surface_model_part_name" : "please specify the model part that contains the lower surface nodes",
                "mesh_id"           : 0,
                "velocity_infinity" : [1.0,0.0,0]
            }  """ );
        
        settings.ValidateAndAssignDefaults(default_parameters);
        
        
        self.model_part = Model[settings["model_part_name"].GetString()]
        self.upper_surface_model_part = Model[settings["upper_surface_model_part_name"].GetString()]
        self.lower_surface_model_part = Model[settings["lower_surface_model_part_name"].GetString()]
        '''
        print('Creating aircraft modelpart...')
        self.aircraft_modelpart = self.model_part.CreateSubModelPart("aircraft_modelpart")
        for i in range(settings["skin_parts"].size()):
            mp = Model[settings["skin_parts"][i].GetString()]
            for node in mp.Nodes:
                self.aircraft_modelpart.Nodes.append(node)
            for cond in mp.Conditions:
                self.aircraft_modelpart.Conditions.append(cond)
        print('Finished creating aircraft modelpart')
        '''
        
        #KratosMultiphysics.NormalCalculationUtils().CalculateOnSimplex(self.aircraft_modelpart,self.model_part.ProcessInfo[KratosMultiphysics.DOMAIN_SIZE])  
        
        #self.model_part = Model.get('model_part_name',None)
        self.velocity_infinity = [0,0,0]#array('d', [1.0, 2.0, 3.14])#np.array([0,0,0])#np.zeros(3)#vector(3)
        self.velocity_infinity[0] = settings["velocity_infinity"][0].GetDouble()
        self.velocity_infinity[1] = settings["velocity_infinity"][1].GetDouble()
        self.velocity_infinity[2] = settings["velocity_infinity"][2].GetDouble()
        
        if(self.model_part.ProcessInfo[KratosMultiphysics.DOMAIN_SIZE] == 2): #2D case
            self.reference_area =  1#
        else:
            self.reference_area =  383 #m² # WRONG 489.89 m² 
        
        self.AOAdeg             = 0#°
        
        #convert angle from degrees to radians
        self.AOArad = self.AOAdeg*pi/180  

  
    #def Execute(self):
        #for node in self.lower_surface_model_part.Nodes:
            #node.Set(KratosMultiphysics.BOUNDARY,True)
    
    
    def ExecuteFinalize(self):
         print('COMPUTE LIFT')

         rx = 0.0
         ry = 0.0
         rz = 0.0
         
         counter = 1        
         for cond in itertools.chain(self.upper_surface_model_part.Conditions,self.lower_surface_model_part.Conditions):
           n = cond.GetValue(NORMAL)
           cp = cond.GetValue(PRESSURE)
           #print(n)
           #print(cp)
           
           #if(cp > -100.0):
           rx += n[0]*cp
           ry += n[1]*cp
           rz += n[2]*cp
           counter +=1

         
         #print('Looped over ', counter, ' lifting conditions.')
         RZ = rz/self.reference_area
         RX = rx/self.reference_area
         RY = ry/self.reference_area
         
         Cl = RZ*cos(self.AOArad) - RX*sin(self.AOArad)
         Cd = RZ*sin(self.AOArad) + RX*cos(self.AOArad)
         
         print('RZ = ', RZ)
         print('RX = ', RX)
         print('RY = ', RY)
         
         print('\nCl = ', Cl) 
         print('Cd = ', Cd)
         print('Mach = ', self.velocity_infinity[0]/340) 
         
         '''
         loads_file = open("loads.dat",'w') 
         loads_file.write("FULL POTENTIAL APPLICATION LOADS FILE\n\n")
         loads_file.write("UInf {0:13f} \n".format(self.velocity_infinity[0]))
         loads_file.write("Mach {0:13f} \n\n".format(self.velocity_infinity[0]/340))
         
         loads_file.write("Cl {0:15f} \n".format(Cl))
         loads_file.write("Cd {0:15f} \n\n".format(Cd))
         
         loads_file.write("Lift {0:15f} \n".format(Lift))
         loads_file.write("Drag {0:15f} \n".format(Drag))  
         
         loads_file.flush()
         '''