# ITC-Suite_GDAL

## Description of the GDAL version of the ITC-Suite

	François A. Gougeon (Ph.D.)

	Nov 2018, April 2020, July 2020, April-June 2021, June 2022, 
	July 2023, Oct 2023, July 2024, Oct-Nov 2024, Jan-June 2025
	Jan-Feb 2026

### Introduction   
	The ITC-Suite was first developed for the ARIES image analysis system in the 90's.
	Later, it mostly ran under PCI/XPACE, then PCI/EASI, with PCI/ImageWorks and PCI/Geomatica 
	to display and deal with PCI's ".pix" files (PCI v2022 at this time)

	Instead of a PCI file containning all, it is now assumed that all will be in 
	a single "project directory": images as tif files, bitmap as 1-bit tif files, 
	vector layers as shp files, signatures as discrete files, etc., and
	that ArcGIS (or other software) may be used to view all these.

	In order to deal with less items on the command line, file nomenclature is
	often predefined, but often using a "base file name" from the main image file.

	Having an image file name such as "BaseName_IRGB.tif" for example
	will ensure that other files will be of the type "BaseName_aaaa.ext"
	For examples, "BaseName_VFOL.tif"   as output from itcvfol_g.exe
	or "BaseName_ISOL.tif"  as output from itcisol_g.exe
	Of course the "BaseName" should reflects the name of the main project in that directory.
	The underscore is use to separate the "base file name" from the rest of the file name.
	Full directory path is now mostly supported (i.e., not fully tested yet as the philosophy 
	was to do everything in the same directory from a "cmd windows"). However, it was needed
	for the integration of the GDAL version of the Suite used with a ArcGIS Toolbox (GUI).

	The GDAL version of the ITC-Suite can be used from ArcGIS or ArcGISPro via appropriate
	toolboxes, allowing users to stay completely within that environment.


	NOTE:	Partial use of a PCI file is often possible (as input). However,
		user entered segment numbers will be that of GDAL, not of PCI.
		Use "gdalinfo" and "ogrinfo" on the PCI file to find that order.
		(e.g., bitmaps are numbered after images, vector layers are separate,
		signature segments are not visible, ...)

	NOTE:   The "ITC-Suite Manual" and "ITC-Suite.chm" are still very relevant to all this 
		but should be used as guides, not gospel.

	NOTE:	My GDAL programs always complain when overwriting an exiting output file.
		Most of the time, it is not an issue. However, sometimes when ArcGIS has control 
		of the file, it may not overwrite at all and leave you with previous results

### Ackowlegment to GDAL (Geospatial Data Abstraction Library) 

- GDAL - Geospatial Data Abstraction Library: Version 2.1.1 (July 2016)
- GDAL - Geospatial Data Abstraction Library: Version 3.0.0 (Dec. 2019)
- GDAL - Geospatial Data Abstraction Library: Version 3.3.3 (Dec. 2021)
- GDAL - Geospatial Data Abstraction Library: Version 3.11  (Feb. 2025)

Open Source Geospatial Foundation, 

Thanks Frank (Warmerdam)

### Additional Info 

For additional info on how to run the ITC-Suite, one can check the ITC-Suite Manual 
from 2010 (meant for the PCI environment).

Gougeon, F.A. 2010 
The ITC Suite Manual : A Semi-Automatic Individual Tree Crown (ITC) Approach to Forest Inventories
Natural Resources Canada, Canadian Forest Service, Pacific Forestry Centre, 
Victoria, B.C. Canada.  June 2010. 92  p. 



