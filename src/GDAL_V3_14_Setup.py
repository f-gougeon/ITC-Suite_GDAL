import os
import sys
import arcpy
import time

arcpy.AddMessage(" ")
arcpy.AddMessage("Some Setups for ITC-Suite with GDAL v3.14")
time.sleep(3) # Delay for 3 seconds

arcpy.AddMessage("     *** Adding GDAL path and projections ***")   
time.sleep(2) 

# Add GDAL main directory to system PATH

gdal_path = r'C:\gdal_v3.14'
# gdal_path = r'C:\Program Files\GDAL'
os.environ['PATH'] = gdal_path + ';' + os.environ['PATH']
sys.path.append(gdal_path)

arcpy.AddMessage("  Path to GDAL  is: " +  gdal_path )
arcpy.AddMessage(" ")

# Add GDAL projection info directory to system PATH


proj_lib = r'C:\gdal_v3.14\projlib'
# proj_lib = r'C:\Program Files\GDAL\projlib'
os.environ['PROJ_LIB'] = proj_lib

arcpy.AddMessage("     *** Adding GDAL ITC-Suite to path ***")   
time.sleep(3) # Delay for 3 seconds

# Add Suite_GDAL directory to system PATH

Suite_gdal_path = r'C:\ITC-Suite_GDAL_v3.14\exe'
os.environ['PATH'] = Suite_gdal_path + ';' + os.environ['PATH']
sys.path.append(Suite_gdal_path)


# Add GDAL_EXE environment variable to point to desired version

arcpy.AddMessage("     *** Adding GDAL_EXE environment variable ***")   
time.sleep(3) # Delay for 3 seconds

os.environ['GDAL_EXE'] = Suite_gdal_path

arcpy.AddMessage("  Path to ITC-Suite exe is: " +  arcpy.GetSystemEnvironment("GDAL_EXE") )
time.sleep(3) # Delay for 3 seconds


arcpy.AddMessage(" ")
arcpy.AddMessage("     *** GDAL v3.14 and corresponding ITC-Suite Setups DONE ***")
arcpy.AddMessage(" ")
time.sleep(3) # Delay for 3 seconds


