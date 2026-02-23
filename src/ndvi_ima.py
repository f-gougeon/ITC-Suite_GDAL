
#   ndvi_ima.py
#   Author: Francois Gougeon
#   Date: May,2025

# Python wrappers needed to call ITC-Suite_GDAL program for ArcGISPro above v3.1

import os
import sys
import subprocess
import arcpy
import time

###   *******************************************

# Define the executable and its path

prog_name = "ndvi_ima_g.exe"

#exec_path = "C:\\ITC-Suite_GDAL_v3.11\\exe\\"
exec_path = os.environ['GDAL_EXE']

# Define the parameters for the executable

parameters = [arcpy.GetParameterAsText(i) for i in range(arcpy.GetArgumentCount())]

#arcpy.AddMessage(f" Parameters from GUI :  {parameters} " )

# ArcMap used to have "#" for empthy (optional) params, but ArcGISPRO has ' '

for i in range(arcpy.GetArgumentCount()) :
	if len(parameters[i]) == 0: parameters[i]= "#"

# Combine the executable path and parameters into a single command
slash = "\\"
command = [exec_path + slash + prog_name] + parameters		# path and prog need to be together (no space)

stringrun = ' '.join(command)                   # now we have a string with space separators

arcpy.AddMessage(f"String to run :  {stringrun} " )


# *******************

arcpy.AddMessage("     *** Running the program perse ***")
arcpy.AddMessage("  	 See reporting window with possible interactions")

# Run the command (in foreground)

os.system(stringrun)

time.sleep(3) 					# Delay for 3 seconds



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
time.sleep(3) # Delay for 3 seconds

