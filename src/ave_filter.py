
#   ave_filter_v2.py
#   Author: Francois Gougeon
#   Date: April 2025, May 2025

# Asked  Copilot
# "for arcgispro write a python script that calls an executable with parameters"

import os
import sys
import subprocess
import arcpy
import time



# Define the path to the executable and its path

# executable_path = r"C:\ITC-Suite_GDAL_v3.11\exe\ave_filter_g.exe"
#exec_path = "C:\\ITC-Suite_GDAL_v3.11\\exe\\"

exec_path = os.environ['GDAL_EXE']

prog_name = "ave_filter_g.exe"

# Define the parameters for the executable

#parameters = ["param1", "param2", "param3"]
#parameters = [Image_path +"PRF_ADS_4x4km.tif", Image_path + "Out_file_10.tif", "3", "ArcGIS"]

parameters = [arcpy.GetParameterAsText(i) for i in range(arcpy.GetArgumentCount())]

#parameters = [arcpy.GetParameterAsText(i) for i in range(arcpy.GetParameterCount())]

arcpy.AddMessage(f" Parameters from GUI :  {parameters} " )



# ArcMap used to have "#" for empthy (optional) params, but ArcGISPRO has ' '
#if len(parameters[2]) == 0: parameters[2]= "#"
#if len(parameters[3]) == 0: parameters[3]= "#"

for i in range(arcpy.GetArgumentCount()) :
	if len(parameters[i]) == 0: parameters[i]= "#"





# Combine the executable path and parameters into a single command

# command = [exec_path] + [prog_name] + parameters        # command is a list
# command = parameters
# command.push_front(prog_name)
# command.push_front(exec_path)

slash = "\\"
command = [exec_path + slash + prog_name] + parameters		# path and prog need to be together (no space)

#command = [exec_path + prog_name] + [parameters]

stringrun = ' '.join(command)                   # now we have a string with space separators

#stringrun = r'C:\ITC-Suite_GDAL_v333\exe\ave_filter_g.exe PRF_ADS_4x4km.tif Out_file_13.tif 3 ArcGIS'

#print("Command to run :  ", command)
#print("   ")
#arcpy.AddMessage(f"Command to run :  {command} " )

arcpy.AddMessage(f"String to run :  {stringrun} " )


# *******************

arcpy.AddMessage("     *** Running the program perse ***")

# Run the command (in foreground)

os.system(stringrun)

time.sleep(5) 					# Delay for 5 seconds



# Run the command  (in background)
"""
try:
    result = subprocess.run(command, check=True, capture_output=True, text=True)
    #print("Output:", result.stdout)
    #print("Errors:", result.stderr)
except subprocess.CalledProcessError as e:
    #print("An error occurred while running the executable:", e)
"""

# ------------------


arcpy.AddMessage("     *** DONE ***")
#print("     *** DONE *** ")
#time.sleep(5) # Delay for 5 seconds


"""

# Example of using arcpy to get a parameter from a tool

input_feature_class = arcpy.GetParameterAsText(0)
parameters.append(input_feature_class)


"""


