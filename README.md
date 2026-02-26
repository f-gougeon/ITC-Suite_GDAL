# ITC-Suite_GDAL

## Description of the GDAL version of the ITC-Suite

	François A. Gougeon (Ph.D.)

	Nov 2018, April 2020, July 2020, April-June 2021, June 2022, 
	July 2023, Oct 2023, July 2024, Oct-Nov 2024, Jan-June 2025
	Jan-Feb 2026

### Introduction   
	The ITC-Suite was first developed for the ARIES image analysis system in the 90's.
	Later, it mostly ran under PCI/XPACE, then PCI/EASI, with PCI/ImageWorks and PCI/Geomatica 
	to display and deal with PCI's ".pix" files (PCI v2022 at this time). 
	It has run on numerous platforms (VAX< SUN, SGI, ...).
	It is now available independent of PCI by using public domain software (GDAL).
	Here's the operating philosophy.

	Instead of a PCI file containning all, it is now assumed that all files will be in 
	a single "project directory": images as tif files, bitmap as 1-bit tif files, 
	vector layers as shp files, signatures as discrete files, etc., and
	that ArcGIS (or other software) may be used to view all these.

	In order to deal with less items on the command line, file nomenclature can be
	predefined by using a "base file name" from the main image file, although generally
	you can specify your own filenames.

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

<P>
	NOTE:	Partial use of a PCI file is often possible (as input). However,
		user entered segment numbers will be that of GDAL, not of PCI.
		Use "gdalinfo" and "ogrinfo" on the PCI file to find that order.
		(e.g., bitmaps are numbered after images, vector layers are separate,
		signature segments are not visible, ...)
<P>
	NOTE:   The "ITC-Suite Manual" and "ITC-Suite.chm" are still very relevant to all this 
		but should be used as guides, not gospel. Please rely on  **ITC-Suite_GDAL_Info.txt**  and
		**ITC-Suite_GDAL.chm** for more precise info on the programs parameters.
<P>
	NOTE:	My GDAL programs always complain when overwriting an exiting output file.
		Most of the time, it is not an issue. However, sometimes when ArcGIS (or PCI) 
		 has control of the file, it may not overwrite at all and leave you with previously
		 generated results

### Ackowlegment to GDAL (Geospatial Data Abstraction Library) 

- GDAL - Geospatial Data Abstraction Library: Version 2.1.1 (July 2016)
- GDAL - Geospatial Data Abstraction Library: Version 3.0.0 (Dec. 2019)
- GDAL - Geospatial Data Abstraction Library: Version 3.3.3 (Dec. 2021)
- GDAL - Geospatial Data Abstraction Library: Version 3.11  (Feb. 2025)

Open Source Geospatial Foundation, 

Thanks Frank (Warmerdam)

### Running GDAL version of the ITC-Suite

#### From command prompt window
To run the ITC-Suite from a Windows10/11 "cmd prompt window", see **ITC-Suite_GDALv3.10_Run.txt**.
Copy and paste the three(3) SET lines into your window. 
**Note:** They may need modifiations if the GDAL or ITC-Suite locations are different on your computer.

#### From ArcGIS Pro

To run ITC-Suite programs from the ArcGIS Pro environment (Here for GDAL version 3.10.0) :

One needs to load the appropriate ToolBox (e.g., ITC-Suite_GDAL-v3-10.atbx)
AND first run  "GDAL_V3_10_Setup" before anything else.

This will mimic the three SET lines mentionned above for the ArcGIS environment.

#### For LinuxLinux

To run the ITC-Suite from a Linux "bash shell" check: **Linux_GDAL_ITC-Suite_Compile_v3.txt**
It is mostly about compiling the programs, but has the proper shell setup to run programs.
**ITC-Suite_Linux_GDAL_AutoAna.sh** is a example of a script towards automatic analsis.
<BR>
**Note:** 	This is very exploratory at this point in time.
			However,  programs run under Linux/Mint that I have used via Oracle Virtual Box 


### Additional Info 

For info on how to run the ITC-Suite, one should check the "ITC-Suite_Docs" repository above
and the docs (above) in this repository.

For additional info on how to run the ITC-Suite, one should check the ITC-Suite Manual 
from 2010 (meant for the PCI environment). 
<br>
It is highly recommended to read the <B>first 12 pages</B> to get a good overview of the ITC-Suite.
 
<P> Gougeon, F.A. 2010.
<A HREF="https://ostrnrcan-dostrncan.canada.ca/handle/1845/247283">
The ITC Suite Manual : A Semi-Automatic Individual Tree Crown (ITC) Approach to Forest Inventories. </A>
Natural Resources Canada, Canadian Forest Service, Pacific Forestry Centre, 
Victoria, B.C. Canada.  June 2010. 92 &nbsp;p. </P>

