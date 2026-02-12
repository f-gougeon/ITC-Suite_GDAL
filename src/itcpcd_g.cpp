/*
C+
c
c Program name: 	itcpcd_g.cpp (64bit) (for GDAL)
c
c	From itcpcd.cpp (for PCI env.)
c
c	Author: 	François A. Gougeon
c   Date:		Aug-Nov 2024
c
c	USAGE:  itcpcd_g BaseFilename ISOLBIT CLASSBIT POLYLAYR ODETAIL PROVINCE HEIGHTCH PCDFILE
c
c
c	EXEMPLE: itcpcd_g ./PRF_4x4km.tif PRF_ISOL.tif PRF_Class1.tif,1-8 PRF_ForInv_4x4km.shp ....
c								...   HIGH ONT PRF_4x4km_DCM,tif  PRF_PCD_Out.shp 
c
c Description:
c
c	Individual Tree Crown Polygon Content Description 
c	(Fast version for 64 bit machine with sizeable memory (i.e., 4-6X imagesize)
c
C	ITCPCD ouputs statistics about classified ITCs within each polygon of a vector layer.
c	Typically used with forest stand polygons (either from an existing forest inventory
c	or freshly generated with the ITC suite or some texture measure or other means) to
c	get information about their content as expressed by an ITC image analysis.
c
C	The following information is generated for each polygon:
C
C		o Polygon area
C		o Number of crowns, total and for each class
C		o Crown Closure percentage, total and for each class
C		o Stems/Hectare, total and for each class
C		o Class percentage of total stem density
C		o Average ITC diameter, total and for each class
c		o Average ITC height, total and for each class
C
c	The information is either stored as additional polygon attributes of POLYLAYR
c	or can be written to a plain text file (PCDFILE).
C
C
C1	PARAMETERS
C
C
C	ITCPCD is controlled by the following global parameters:
C
C	Name		Prompt						Count	Type
C
C	FILE		PCI Database File Name				132	Char
C	ISOLBIT		Bitmap of ITCs (from ITCISOL)			1	Int
C	CLASSBIT	Class Bitmaps (from ITCISC)			16	Int
C	POLYLAYR	Input Vector Layer containing polygons  	1	Int
c	ODETAILS	Output level of detail required			64	Char
C	PROVINCE	Prov. jurisdiction of interest (affects CC%)	64	Char
C	PCDFILE		Optional text file to get ITC info. by stand	64	Char
c	HEIGHTCH	Optional height channel (i.e., a DCM)		1	Int
C	REPORT		Report Mode: TERM/OFF/filename			132	Char
C
C
C2	FILE
C
C	Specifies the name of the PCI file containing the input/output bitmaps and vector
c	layers.
C
C	EASI>FILE="filename"
C
C2	ISOLBIT
C
C	Typically specifies an Individual Tree Crown (ITC) bitmap produced by ITCISOL.
c	Optionnaly, it can also be a TT (tree tops) bitmap, or even a bitmap containning
c	a bit of both (typically, TTs for very small trees, ITCs for the other).
c
c	EASI>ISOLBIT=n
C
C2	CLASSBIT
C
C	Specifies a list of Class bitmaps, the class for which to report as stand content 
c	These bitmaps typically contain ITCs classified by ITCSC, each bitmap representing
c	a single class (species). They can also contain classified TTs (tree tops).
C
C	EASI>CLASSBIT=n,n,n...     OR	EASI>CLASSBIT=n,-m  OR  both notations combined
C
C2	POLYLAYR
C
C	The input layer which holds the inventory polygons. 
C
C	EASI>POLYLAYR=n
C
c
c2	ODETAILS	Output level of detail required. 	
c
c	Can be one of HIGH/MEDIUM/LOW.
c
c	When asking for a printed output:
c	LOW		Polygon area (pixels and hectare) and ITC counts
c	MEDIUM	Same as HIGH
c	HIGH	Reports on all of the species in the polygon (same order as bitmaps)
c
c	When adding attributes to vector:
c	LOW		Polygon area (pixels and hectare) and ITC counts
c	MEDIUM	Reports on the five most important species in the polygon (ordered by crown closure)
c	HIGH	Reports on all of the species in the polygon (same order as bitmaps)
c
c 	To get more control over the species ordering when adding attributes to vector, 
c	variations of the above parameters were introduced. They are:
c
c	HIGHSP		All of the species in the same order as that of the bitmaps.
c	HIGHCC		All of the species ordered by crown closure.
c	HIGHITC		All of the species ordered by ITC counts.
c	MEDIUMCC	The five (5) main species as ordered by crown closure.
c	MEDIUMITC	The five (5) main species as ordered by ITC counts.
c
c
C2	PROVINCE
C
C	Specifies the provincial jurisdiction of interest (QUE,ONT,BC,NB,AB)
c	As this point in time, it only affects the way crown closure (CC) 
c	is reported.
c	
c	For example, provinces may have a different crown closure definition:
c		 a) Qu�bec species CCs sum up to stand CC (say 66%)
c		 b) Ontario species CCs sum up to 100%	
c		 c) Other provinces, species CCs sum up to 100%
c
c	EASI>PROVINCE="QUE
c
c
C2	PCDFILE
C
C	Specifies an optional "plain text" output file in which to write the  
c	polygon (forest stand) ITC content information.  
c
c	Information for all species (not just the most dominant five) will be reported 
c	in the same order as that of the classification bitmaps (CLASSBIT) order, 
c	whether a MEDIUM or HIGH level of detail was requested.
c
c	***NOTE***  	
c	If this parameter is used, the information is NOT appended to the vector layer
c	as additional attributes (i.e., the vector layer remains UNTOUCHED)
c
c	EASI>PCDFILE="filename"
c
c
c2	HEIGHTCH	Optional height channel (i.e., DCM)
c
c	A channel (8 or 16 bit) containning a height-related image, like a  
c	Digital Canopy Model (DCM) from Lidar returns (DCM = DSM - DTM). 
c	If specified, height information will be reported using the maximum height
c	within each tree crowns (ITC).
c	HINT:	If low memory availability, make sure to use an 8bit image for HEIHTCH
c			as 16bit images typically load as 32bit (requiring 4 times more memory)
c			and 256 levels are enough to convey height up to 64m in 0.25m increment.
c
c	EASI>HEIGHTCH=n
c
C2	REPORT
C
C	Specifies the file to which the generated report should be appended.
C
C	EASI>REPORT = "filename"
c
c	Note:  The following names have special meaning:
c
c	EASI> REPORT = "TERM"	| generates reports on your terminal
c	EASI> REPORT = "DISK"	| generates reports on file "IMPRPT.LST"
c	EASI> REPORT = "OFF"	| (may) switch off report generation
C
C1	DETAILS
C
C	
C       ITCPCD reports on the crowns found in forest inventory (or any other) polygons 
c
C       The following information is generated:
C
C		o Polygon area
C		o Polygon total number of crowns (ITCs)
C		o Polygon total Crown Closure
C		o Polygon total density (stems/hectare)
c		o Class number of crowns (ITCs)
C		o Class percentage of total stem density
C		o Class percentage of total crown closure
C		o Class density: stems/hectare
C		o Average ITC diameter (in metre) for that class
C		o Average ITC height for that class
C
C
C
C2	GENERAL PROCESS  	**** not up-to-date for v3.0 ****
C
C	The following process is used to generate the class area statistics:
C
C	First the input vector layer (POLYLAYR) is burned into a raster image in memory where
C	each polygon is given a grey level value equal to its shapeID + 1.  This is because
C	gray level 0 is needed to distinguish non-polygon areas of the image.  If a polygon's
C	vertices are not all within the image boundaries it is not burned into the raster
C	image.
C
C	The crown isolation bitmap (ISOLBIT) and the first class bitmap (CLASSBIT) are 
C	loaded into memory by the system.  The class bitmap may contain filled areas, 
C	discrete points, or filled tree crowns.  The ISOLBIT and the CLASSBIT are ANDed 
C	together bitwise to form a third bitmap, which is used to determine tree positions.   
C	The raster image generated from POLYLAYR, in which the pixel value of each polygon 
C	is the polygon shapeID-1, is used to determine which polygon the tree is in, and the  
C	statistic array element for the correct polygon number (or pixel value) is adjusted.
C
C	The number of pixels which make up the class iclass is now determined.  This
C	will help us to determine the "area" contained in the class bitmap. We do a count of the
C	number of pixels in the class bitmap which are not in the isolation bitmap.  This gives
C	us a count of the area between the tree crowns (if any).  Since the bitmap can have filled
C	areas, filled crowns, and/or individual points, now we just want to count the number of
C	pixels in the bitmap between crowns, and then later when we process the crowns we will add
C	their area to the total to give us the total area.  
C
C	The new third bitmap is scanned until a tree is found. All partial trees are considered.
C	With a tree we do a pixel count of the number of pixels contained in the tree crown.
C	This value is added to the total number of pixels contained in all tree crowns.  We also
C	keep track of the number of trees we have found in the bitmap.  The entire bitmap is scanned
C	until we have considered all the trees. If a tree is on the border between polygons
C	it will be counted only in the polygon containing the majority  of the pixels.  However, for
C	crown closure calcuations, the count of the number of  pixels contained in the tree crown
C	for each polygon will include only those residing inside the individual polygon.  The count
C	for the entire tree will be used to calculate average crown size. We will have two totals 
C	from this operation.  A total number of trees in the class area, and the total number of 
C	pixels that make up the tree crowns in that particular polygon.  The number of pixels that
C	make up the tree crowns is added to the total number of pixels in the class area to form the
C	true total number of pixels in the entire class area.  The results are stored.
C
C	If there is more than one CLASSBIT specified, then the above process continues
C	with the next class bitmap.  When all the bitmaps have been processed and
C	the results accumulated, the actual calculations are done to determine the stats
C	we want.
C
C	By checking the pixel size of the image file we determine how large each pixel is
C	(eg:  1 metre by 1 metre).  From this we determine the area of the class bitmap
C	in units given by the pixel size (eg:  Sq Metre).  We also give the area in pixels.
C	The average crown area is determined, as well as the Stems Per Hectare, and the
C	Crown Closure.  Once all the values have been determined, they are displayed in
C	a list.
C
C	The statistics are stored in an array with members for each polygon shapeID-1.
C	Each time that a total is to be increased, the pixel value in the raster image for 
C	the part of the image in question is considered.  Each polygon has a different pixel
C	value representing its polygon shapeID-1.  The array member for this polygon number,
C	or pixel value, is increased.
C
C2      BITMAP MASKS AND BOUNDARIES  	**** not up-to-date for v2.0 ****
C
C       Two different types of bitmap masks are used by ITCPCD.  The first kind is
C       a bitmap mask generated by ITCISOL and specified by ISOLBIT.  This bitmap
C       contains a mask of the isolated tree crowns.  This bitmap is scanned in order
C       to locate a tree to be processed.
C
C       The second kind of bitmap is a series of class bitmaps (CLASSBIT) which have
C       been constructed by the user.  This can consist of a filled region (all trees
C       contained in a region), filled tree crowns (an individual tree), or individual
C       points (also indicating individual trees to be considered).  Each bitmap
C       contains trees of one particular species.  The signatures of trees in each bitmap
C       are used to create the average or species signature.  Each CLASSBIT in turn is
C       ANDed together bitwise with the ISOLBIT.  This new bitmap is scanned to
C       find the location of a tree.
C
C       
C       There is an "inclusive" border on the CLASSBITs.  That is trees which are partially
C	in a region (but not entirely) are included. 
C
C       Inclusive:
C		    o Counts trees on borders between polygons in the polygon containing the most   
C		      pixels, but only the number of pixels lying in the particular polygon are 
C                     counted for crown closure. 
C		    o Allows a CLASSBIT to contain filled area, filled crowns, and individual
C                     points to specify trees.
C                   o Partially filled trees (trees on area boundaries) are kept.
C                   o Should be used for most situations (Default).
C
C2	GENERATED STATISTICS  	**** not up-to-date for v3.0 ****
C
C	A number of statistics are generated for a class area.  These include:
C               
C		o Polygon area
C		o Total number of crowns
C		o Total Crown Closure
C		o Stems/Hectare, total and for each class
C		o Total area covered by the isolation bitmap and each class bitmap
C		o Class percentage of total crown closure
C		o Average Crown Area
C
C	Polygon area is determined by simply checking each pixel in the image and incrementing
C	the pixel count for the polygon whose number matches the pixel value, since the polygons 
C	were assigned the grey level corresponding to their polygon ID number.
C
C	Total number of crowns is determined as noted above, incrementing a count in the same way
C 	as the area.  If a tree is on a polygon border, a count of the pixels in each polygon is 
C	kept.  Whichever polygon contains the most pixels is considered to contain the stem.
C 
C	Total Crown Closure is the ratio of the total crown area to the total area contained 
C	in the polygon, expressed as a percentage.  It is obtained simply by dividing the 
C	pixel count for ISOLBIT crowns in a particular polygon by its area in pixels.  If a 
C	crown is on a border, only the pixels actually contained in the polygon are counted, 
C	not the entire tree crown.  Therefore, a polygon with zero stems can still have coverage.
C
C	Stems Per Hectare is calculated by using the ratio of the number of trees in either the 
C	ISOLBIT or a CLASSBIT to the area of the polygon in metres.  The area in meters is found 
C	by multiplying the pixel count by the geographic size of the pixel (eg: 1 metre by 1 metre).
C
C	The Total area covered by the isolation bitmap and each class bitmap is found as described 
C	above, and multiplying the obtained pixel count by the geographic size of the pixel.
C
C	The class percentage of total crown closure in terms of area is calculated by dividing the 
C	pixel count for the CLASSBIT by that of the ISOLBIT.  The percentage in terms of stems is 
C	the tree count for the CLASSBIT divided by that of the ISOLBIT.
C
C	Average Crown Area is calculated by taking the total number of pixels in the crowns, divided 
C	by the number of tree crowns.  Using the pixel size value of the image, this is displayed 
C	both in pixels, and in specified units (eg:  Sq Metre).  The pixel count of the entire 
C	crown is used, even if it is on a border.
C
c
C
C1	REVISION HISTORY
C
c	François A. Gougeon, Ph.D.
c	Remote Sensing Research
c	
C	(�)Natural Resources Canada
c	Canadian Forest Service 
c
c
c
c v1.0	Aug 1997	Shannon Kolind	
c			- As ITCPCDU, working with an ITCPCD.EAS, and various
c			  other programs and functions on ARC/INFO
c			- User had to go back and forth between PCI and ARC/INFO a few times
c			- Based on ITCTAS by Ron Petrick (1994) for François Gougeon
c			- Generates statistics for each polygon burned in to a raster 
c			  image, instead of for training areas as in ITCTAS
c  			- Generate more statistics
c
c v2.0	March 2001	François Gougeon
c			- Modernized and reactivated (was not used since 1997)
c			- Mod. to deal with more than 255 polygons (i.e., polygons can be 
c			  burned into a 16 bit channel, rather than only an 8bit channel)
c			- Change BITBOUND to take (and only take) AUTO by default
c			  since INC/EXC does not make sense in this context
c			  Here, AUTO means it goes to polygon which contain MOST of the ITC.
c			  (i.e., ITC can be at the junction of three polygons)
c			  (** NOTE: This is different from ITCTAS/AUTO => 50% in)
c			- Added parameter OUTPUT="LOW/MEDIUM/HIGH to allow control  
c			  over output information going to flat file.
c			- Started reorganization to use bitops functions or macros
c			  (as opposed to local functions)
c			- Prep for future modifications for MODERN "ITCPCD" v3.0
c			  that will deal with PCI polygons directly and add polygon content 
c			  information in polygon fields (attributes).
c			- Debugging: count_isol() was not counting ITCs properly.
c
c v2.1	April 2001	François Gougeon
c			- Debugging: Polygon area was wrong (multiplied only once by pixelsize)
c
c v3.0	Dec   2001	Geoff Savinkoff
c			- updated to use check_mem and safety_zone functions from itc_io
c			- Replaced most IDB functions with GDB functions (Not IDBPixelSize as 
c	 	          there is no equivelant GDB function).
c			- defined Pixels, Lines, Channels as global variables
c			- Updated to accept a vector layer as input, and burn it to a
c			  raster image in memory. The processing method to generate the output 
c			  information is still the same. 
c			- Updated to store the results back into the input vector layer as
c			  attributes.  User still has the option to write the results to a file.
c			  Only the 5 most dominant species are stored in the input layer.  If
c			  output is to file, then all species are reported.
c			- removed BITBOUND parameter. Now always uses auto to determine what
c			  polygon a tree is in.
c			- Removed functions related to BITBOUND parameter (eg. fill_exc)
c			- renamed POLYCHAN parameter to POLYLAYR.  Also renamed tree structure to
c			  class_info and class_info_node. 
c			- Updated help file with linking information and EASI prompts
c
c v3.1	Jan 2002	François Gougeon
c			- Debugging and cleaning up  Geoff's unfinished business
c
c v3.2	April 2002	François Gougeon
c			- Mod. to work in southern hemisphere (or with fake UTM that are actually
c			  related to line and pixel numbers)
c
c v3.3	May 2002	François Gougeon
c
c			- Mod. for Qu�bec vs Ontario definition of species closure
c			  a) Ontario species CCs sum up to 100%	(default version)
c			  b) Qu�bec species CCs sum up to stand CC (say 66%) (V3.3q)
c			- Mod. for field labels of type "SP1" instead of "1st_SP", as some GIS,
c			  spreadsheet, or database programs dont like fields starting with numbers.
c			- Mod. for narrower field width and labels	
c			- Big mod. to deal with non-sequential shape numbers (via shapenum[])
c			- Mod. to ouput different levels of information in vector attribution mode
c			  (Previously ODETAILS only applied when PCDFILE was used)
c			  ODETAILS="LOW     Reports only on area, ITCs, closure and density
c			  ODETAILS="MEDIUM  Reports on FIVE most prevalent species (ITC-wise)
c			  ODETAILS="HIGH    Reports on All species in CLASSBIT
c			- Mod.to use half the memory by making internal image of polygons
c			  a 16bit image rather than a 32bit image (i.e., max. number of 
c			  polygons is now 65535) using "uint16" -> unsigned short integer
c			  Also, it reports of its memory needs early in the program.		
c
c
c v3.3a	Nov. 2002	François Gougeon
c			- Task TTPCD to report on TTs rather than ITCs can easily be created
c			  by changing
c					#define MIN_TREE_PIXELS 4 	to 
c					#define MIN_TREE_PIXELS 1
c
c v3.4	July 2004	François Gougeon
c
c			- Added HIGHSP || HIGHCC || HIGHITC (and MEDIUMCC || MEDIUMITC ) so the user
c			can select a species order for the forest stand's ITC attributes.
c			Species will be in same order as input classification bitmaps (CLASSBIT),
c			or sorted by importance relative to crown closure (CC) or no. of ITCs (ITC).
c			NOTE: This does not affect the dump to a "plain file" mode (with a PCDFILE),
c			where the dump is either low or high (medium=high), and if so, dump by the order
c			of the classification bitmaps (CLASSBIT).
c			- Formalized the adaptabilty to various provincial definitions by introducing
c			the input parameter PROVINCE.
c			For example, provinces may have a different crown closure definition:
c			  a) Qu�bec species CCs sum up to stand CC (say 66%) (the default)
c			  b) Ontario species CCs sum up to 100%	
c
c
c v3.5	Oct. 2007	François Gougeon
c
c				- Updated to report average ITC height (& crown area) for each polygon 
c				and for each class within each polygon (for plain text output, as well as,
c				as new fields in modified vector layer)
c				- Adding GDBSync(idb_fp) at the end of new field creation section makes them
c				immediately available and the program can put data into them in "one shot".
c				In the past, you often had to run the prog. twice, once to create the new
c				fields and a second time to use them, before data would show up.
c
c v3.6	Dec. 2007	François Gougeon
c
c				- Program was crashing on polygons intercepting an image line more 
c				  than 10 times, which could easely happen with a flat horizontal line.
c				  Function vect2ras( ) had beeen designed for crowns, not stand polygons.
c				  So introduced internally defined MAX_INTERCEPT and set it to 1000
c				  (using 16 KB of memory) (i.e., polygons with horizontal lines of the 
c				   order of 1000 pixels can now be tolerated)
c				- Now uses a bit less memory, as we dont need to have more than one class  
c				  bitmap resident in memory at any given time.
c
c
c v3.7	Dec. 2007	François Gougeon
c
c				- Now deals better with "donut" polygons (polygons with islands, holes, lakes,
c				  other forest polygons in them) AND with stands that have two distinct
c				  areas associated with the same shape number (like stands split in two by a road
c				  OR like stands that have a big part outside our area of interest and 
c				  end up (after truncation) having two or three separate parts inside our
c				  image area, but all refered by the same shape number).
c				- This is accomplished (and only works) if the RINGSTART attribute exist
c				  to indicate the beginning of every island in the polygon.
c				- Previous versions of ITCPCD wanted "simple" polygons that started and
c				  finished at EXACTLY the same geographic coordinates and PCD was just checking
c				  that the first and last point of a shape were EXACTLY the same.
c
c v3.7a	Dec. 2007		François Gougeon
c
c				- NOW, in order to support reporting on TTs and ITCs in the same classified image,
c				  MIN_TREE_PIXELS is set to 1 permanently. Tree tops (TTs) are typically
c				  used for trees that are too small for ITCISOL to find (e.g., young regen)
c				  Thus, theoretically, even combination of ITCs and TTs within the same
c				  stand polygon can be reported on, however in such case the average crown area 
c				  may be less meaningful.
c				  NORMALLY, TTs and ITCS are not found in the same stand.
c				  Of course, for TT-based stands, crown area and crown closure are meaningless.
c
c v3.7b	Jan. 2008		François Gougeon
c
c				- To be a bit less verbose (i.e., not to run out of screen space)
c				  and better report on total memory needed by the program.
c
c v3.8	March 2008		François Gougeon
c
c				- Should be more inclusive of polygons that were closed at image borders.
c				  For example, when a vector layer is moved to a smaller image, it could
c				  have been adapted to that area so that polygons that would normally
c				  stick out of the image area get closed at image boundaries (+-1 pixel).
c
c				- Polygons that are un-used by a particular run of ITCPCD for whatever
c				  reason (e.g., sticking out, can't handle rings) will have their ITCPCD
c				  polygon area and ITC counts set to zero as EVIDENCE. Otherwise, you can't
c				  tell if a particular record was populated this run or from a previous run
c				  (e.g., a previous run on a bigger image where the polygon was considered in)
c
c				  NOTE: This is not fool-proof. There is still potential for confusion if
c				  moving a vector layer (or shp files) from small to big images, and vice-versa,
c				  because if the PCD-related field exist, they will be used by whatever is the
c				  next PCD run. Thus, some records and some fields may have been populated by one
c				  run and some by the other.
c
c
c v3.8a	May 2008		François Gougeon
c
c				- Link adaptations for PCI v10.n and its new incompatible PRM.PRM file 
c				  and use of compiler (VS*8) wanting to produce more secure code.
c				  Most PCI routines are in PCI1000.dll, but not all.
c				  For example, IMPTime() and IMPReturn() are now in Core1000.dll
c				  and IMPCounter() in Counter1000.dll
c				  Since I dont have PCISDK or PCI/ProSDK, I had to create LIBs 
c				  from their DLLs to compile my progs (same for PCI 10.1)
c
c				 - Problems with the REPORT (*Report) Variable in Core1000.dll
c				   First use of report makes program CRASH (immediately after the start message)
c				   So declared the global variable	 FILE *Report;
c				   and did, immediately after the call to IMPSTATUS() :		
c				   if(EQUALN(report,"TERM",4)) 
c					{Report=stdout;}else{Report = fopen(report, "w");}
c
c
c v3.9	June 2008		François Gougeon
c
c				- Should deal even better with "multi-donut" polygons (polygons with islands, holes, 
c				lakes, or other forest polygons in them) AND with stands that have two distinct
c				areas associated with the same shape number, even though these polygons may not
c				be following the Open GIS Consortium convention of having a "RingStart" attribute
c				(as typically done in PCI)
c
c				NOTE:	ITCPCD may still not be ROBUST ENOUGH to deal with all types of polygons.
c					For example, it does not deal well with rivers and, in general, long and narrow
c					features, so make sure you clean up your act and feed ITCPCD only reasonnably
c					well behaved forest stand polygons.
c
c				-Displays ITCPCD results to the plain text file (PCDFILE) and as polygon attributes
c				in a cleaner format (i.e., zeros instead of -1.$ when no result is warranted).
c
c
c v3.91	Oct. 2008		François Gougeon
c
c				- Change the fields named SPnn to BMnn to better reflect reality. That is, the field
c				contains a bitmap number, corresponding to an ITC class, which typically corresponds 
c				to a species, but not really a species name. ALSO, because of potential conflicts 
c				when adding fields an existing forest inventory that may have used such fields (SPnn).
c				For example, ITCPCD wanted to add its info to an inventory that already had an SP10
c				field. It crashed when wrting to it, with no explanation. It turned out that their SP10
c				was for "text only", while ITCPCD SP10 field is for integers.
c
c				** NOTE ** 
c				More generally, one should be careful that the existing vector layer PCD is to write to 
c				does not already contain fields with the same name as the one PCD wants to create. In
c				such cases, ITCPCD may crash (if field type is different) OR the data in that field may get 
c				overwritten (if field type is the same). ITCPCD is organized to overwrite its own fields 
c				to allow for successive runs of PCD without trouble. If the results of successive runs 
c				of PCD are to be kept, then use multiple instances of the vector layer.
c				If an existing forest inventory that you want PCD to augment has similar field names, 
c				just keep that vector layer aside and create a vector layer that contains only the 
c				polygons (shapes), and not their attributes. You can always combine the two 
c				later (e.g., via dbf to Excel) for comparative analysis.
c
c
c v4.0	Jan. 2009		François Gougeon
c
c				- Added new output and field (HeightSD) - the standard deviation of ITC heights
c				  within a stand (i,e., within a polygon).
c
c v4.1	March 2010		François Gougeon
c
c				- Mod. to take a range in CLASSBIT (e.g., CLASSBIT=40,-49 )
c				- Capable of dealing with FILENAME (thus path) up to 132 chr long. as per 
c				new standards (PCI > v10.n)
c
c
c v4.2	June 2010		François Gougeon
c
c				- Mod. to use an internal 8bit image when a 16 bit image is not
c				necessary (i.e., less than 256 polygons), thus save memory and
c				being able to run ITCPCD on bigger images.
c
c
c v4.3	Nov 2011		François Gougeon
c
c				- Mod. to use int64 with newer convert_bitnum(), setbit(),clearbit(), etc.
c				from bitops.c, so that bitmaps can have more than 2G positions (i.e., was int) 
c
c v5.0	Feb 2013		François Gougeon
c
c				- Program is now called ITCPCDB (B for Big images)
c				- Mod. NOT TO USE an internal image in which to first paint ALL stand polygons
c				(i.e., it was getting too big for Windows XP memory, specially when a 16bit image 
c				was needed (more than 255 stands, which was usually the case).
c				- It does the forest stands one by one via a temp. bitmap, similarly to ITCTAS
c				- Thus, deals well with stands (i.e., not using an internal image), but it is 
c				still limited in its size of DCM image (about 3GB) and overall memory use of 4GB
c				(Windows XP allows prog. to use 2GB (3GB possible). W7 allows up to 4GB (WoW)
c				- Capable of dealing with FILENAME (thus path) up to 132 chr long. as per 
c				new standards (PCI > v10.n)
c				- Capable of dealing with 32 class bitmaps (by parameter expansion)
c				- vect2ras() was improved for speed
c				- Took 12 hours to run on 20km x 10km x 0.4 cm/pixel image with 11 classes
c				- Note: 
c				    The rare ITCs spanning three or more stands will not be dealt with properly.
c
c v5.1	April-June 2014		François Gougeon
c
c				- Some minor mods and test cause often crashing and not making it through what are 
c				sometimes 27-48 hrs runs.
c				(This could have been related to Firefox memory leaks ? OR see v5.2 below)
c
c
c v5.2	Jan  2015	François Gougeon
c
c			- First attempt as compiling ITC Suite without "Visual Studio" (left on XP laptop)
c			by using "32bit MinGW gcc" (presently at C:\Program Files (x86)\CodeBlocks\MinGW\bin)
c			which now takes DLL directly (no need to create other lib)
c
c			Typical compile line:
c
c			gcc  -I"C:\pcisdk_v101\lib" -I"C:\ITC-Sources" -o itcpcdb itc_io.c error.c itcpcdb.c
c			-L "C:\Program Files (x86)\PCIGeomatics\Geomatica_V101\exe" 
c			-lpcic1010  -lcore1010 -lcounter1010  -Wl,--large-address-aware
c
c
c			- Seem to work properly, but images must be smaller. Only tolerates DCM of about 1GB.
c			*** NO ***  Made "large address aware" by compiling with "-Wl,--large-address-aware"
c			(see above) Can now use almost up to 3.9 GB of memory on WOW64 (i.e., 32 bit side of Windows)
c
c
c			- Static assignment of storage PREFERED to dynamic assignment and erasure
c			Windows WANTS to manage and clean up memory - no need  for user to do it (and prefered not to)
c			 ( i.e., WoW64 does not like HFree(tmpVertices) )
c			So, I just assigned temp storage for one thousand (1,000) vertices :
c				  tmpVertices = HMalloc(1000 * sizeof(GDBVertex));
c
c	    		- BYPASS invisible ring connectors needs a "-1" as we bypass vector element before ringstart
c	   		 Code is now: if ( RingFlag2 && (kk == (RingStart[kkk]-1) ) ) {kk++; kkk++;} 
c			NOTE: Be careful that vector element start at zero in prog but at one in PCI Imageworks
c
c			- Changed the name of function vect2rast() to Poly2bitmap() to:
c				- reflect more the reality of what the function is doing (i.e., the name)
c				- differentiate from the other vect2rast() which was meant for vector ITC to raster
c				or simple training/test areas to raster and did not consider the existence of
c				possible "island" in polygons
c
c			Also, the other version of vect2rast() assumes that vertices where converted from
c			georeferenced coordinates to image coordinates and if needed to sub-area coordinates
c			BEFORE entering vect2rast(), while this one does that convertion internally
c
c v5.3	March 2015	François Gougeon
c
c			- Cleaning-up, plus and additional efforts to speed-up the program 
c			( SUCCESS !!! - back to running in one night (say 18-24 hrs) )
c			Some functions (e.g., count_isol2()) were scanning the whole bitmap 
c			everytime, instead of concentrating only on the stand area
c
c			- Converting pasVertices (in-situ) into image coordinate before Poly2bitmap()
c			implying no duplication of vertices (i.e., less storage)
c
c
c v5.4	June 2015	François Gougeon
c
c			- Got rid of "stderr" which seems to create some instabilities (now all print to "stdout")
c
c v5.41	Dec 2015	François Gougeon
c
c			- Some ***instabilities*** due to writing zeros past the array size	
c			Around Line 1086  		for (i = 0; i <= polymax; i++)
c			Replaced by 			for (i = 0; i < polymax; i++)
c
c
c v5.41a Dec 2015	François Gougeon
c
c			- Issues about not writing ITC info to last record of vector layer (It's OK for plain text output)
c			in spite of using PCI GDBSync()   ### not fixed yet.
c
c
c
c v5.5 March 2016	François Gougeon
c
c			- Fixed problems with generic and species-specific crown closure
c
c v5.51 March 2016	François Gougeon
c
c			- Species-specific crown closures were still wrong when output as vector field
c
c v5.52 March 2016	François Gougeon
c
c			- All ITCs partially in a stand (whether considered in or out of the stand) 
c			should have some contribution to crown closure
c			- *** Tons of issues *** running ITCPCDB over the network: it crashes OR appears to
c			run properly but with no real ITC info. in vector layer
c
c v6.0	March 2016  	François Gougeon
c
c			- Changed to a C++ program to deal with versions of PCI > v10.2
c				int main (), ".cpp" name, and extern "C" around .h include files
c			- Mod to some call to fit new library definitions (as per .def demangling)
c			  which also means mods to corresponding declaration .h files (e.g., gdb.h)
c			  (Since I dont have PCISDK or PCI/ProSDK, I have to create LIB and DEF from
c			  from their DLLs (via DUMPBIN and LIB) in order to compile my progs
c			- Modules in files like gdb.h need to be declared 'extern "C++" to link
c			  with proper name mangling (MSVC++ mangling)
c			- This is typically enough for PCI 10.3 that migrated its LIB to c++ ... 
c
c			- HOWEVER, more stuff is needed for PCI 2015 (64bit)
c			- PCI LIB need to be /MACHINE:x64 
c			  and prog compile with 64b version (i.e., vcvarsall amd64)
c			  cause all the call to lib are now with 64bit pointers 
c			- IMPStatus()  uses "char const *", so mod. that in ccltask.h (no mod in prog.)
c			- Similar mods. to other library modules may be needed, for example:
c			     int  DLL_ENTRY GDBAddField(GDBLayer, char const *pszName, GDBFieldType eType, GDBField *psDefault);
c			  DEF2 (demangled) will tell you what "types" of variables are needed when calling a function
c			  You have to adjust their descriptions in the .h file and their call in .cpp program
c
c v7.0	March 2016	François Gougeon
c			
c			- Renames ITCPCBF.cpp (fast version) as explained below:
c			  Since we are now using 64 bit machines with access to sizeable memory
c			  (i.e., 3-4 times the imagesize), we can go back to the concept of
c			  writing all stand polygons into an internal image (i.e., pre-2013 era)
c			  rather than re-reading these polygons many times (disk access slows things down,
c			  specially across network, plus network-related instabilities)
c			  In version 6.0 (ITCPCDB), we were "drawing and filling" the polygons one-by-one (many times) 
c			  via an internal bitmap
c
c
c
c v7.1	August 2016	François Gougeon
c
c			- Bug calculating needed memory for huge images and their bitmaps. (int64)casting at bad positions
c			- Depending if LT 255 polygons, reorg to work with internal 8 or 16 bit image of polygons 
c			  to save on memory when possible (here, saving 17GB on a huge Road Regen image)
c			- Issues with alloc_read_bmp() from itc_io.cpp:
c				It use to be that GDBBitmapIO() could not deal directly with bitmaps > 2G positions,
c				so these were split into 2 sections read by two GDBBitmapIO(), putting them in proper
c				place in memory. 
c			- NOW (PCI V2105), GDBBitmapIO()  can deal directly with bitmaps > 2G positions, however it is
c			debatable whether it "consistantly deal properly" with bitmap > 2GB
c			So, alloc_read_bmp() was modified to deal with big bitmap in sections less than 2GB (when needed).
c			- view_results() was passing (xsize*ysize) as int32. Was going to fix it to pass as int64,
c			but decided no point cause image_size (an int64) is available as a global variable
c
c
c
c v7.2	May 2023	François Gougeon
c
c			- Renamed field "closure" to "ITC CC" not to conflict with Photo Interpreters assesment
c				often also called "closure"
c				
c
c v8.0		Aug. - Oct 2024	François Gougeon		(*** GDAL version ***)
c
c			- From the PCI version, mod for GDAL, tif and shp files 
c
c			- First prog. to be ported to Linux Mint and g++ (during vacations)
c
c			- Major clean up of vect2rast_g() issue
c
c			- *NOW* creates an output shape file distinct from the input one
c				Previous versions were modifying the original vector layer in the PCI file
c2				by just adding the newer fields to it		
c
c			- view_results() better separate moving to plain text file from moving to shp file 
c				Uses Copy_Shape_File(), Prep_New_Fields() and Add_New_Info()
c
c
c François Gougeon  v8.1		Aug. 2025
c
c       - To deal with Linux use of "slash" versus Windows "backslash", 
c           finally realizing that Windows can do both, so prog for Linux
c           and using path_len rather than jj
c	
c François Gougeon  v8.2		Dec. 2025 - Jan 2026
c
c       - To deal with situations when there are no classes. We just want generic ITC/TT info. 
c			Then, "-" can be used as third argument instead of a file name.
c
c		- No need to specify an output shape file. If using the minimum of 4 parameters,
c			the default will be "Area_PCD_Out.shp". 
c			To get the default text file "Area_PCD_Out.txt", parameter eight(8)  must be "X"
c			In any cases, parameter eight(8) can always be used to declare what one wants
c			as the file extension will be used to decide
c
c François Gougeon  v8.3		Jan 2026
c
c		- Fixed a few bugs and tidy-up
c
c
C1   NOTES:	
c
c			a) ITCPCD does not actually finds 2x2 pixels ITCs (the minimum) like other Suite 
c			programs. It finds ITC that have at least 4 pixels.
C			However, at this stage of the game everything should be clean (i.e., no debris)
c
c			b) PRESENTLY, in order to ALWAYS support reporting on TTs and ITCs in the same  
c			classified image, MIN_TREE_PIXELS is set to 1 permanently.
c			Thus, theoretically, even combination of ITCs and TTs within the same stand
c			polygon can be reported on. HOWEVER, in such case the average crown area 
c			or crown closure, ...  will be meaningless. However, NORMALLY, TTs and ITCS 
c			are not found in the same stand.
c
c	
C
c
*/

/***************************************************************************************/
#define DEBUG 		// use to exclude sections of code when debugging 
/***************************************************************************************/

#define VERSION "V8.3"
#define PROG_NAME "ITCPCD_g"

#define MIN_TREE_PIXELS 1
#define MAX_INTERCEPT 100
#define FILENAME 132
#define BITMAPS  32

/* #define NO_MACROS */

/* 
#include <stddef.h>		// standart C inclusions
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <time.h>       // time_t, struct tm, time, localtime

#include "gdal_priv.h"		// For GDAL library
#include "ogrsf_frmts.h"	// For OGR

 */
#include "ITC-Suite_g.h"		// my newest GDAL variable setup
#include "itc_io_g.h"		// my newest GDAL image/bitmap input/output
#include "bitops.h"		// bit operations on bitmaps (mostly macros to be faster)



/* #include "windows.h"		 for VirtualAlloc() tests */

/* structure definition for class node */

typedef struct class_info {
	int                 *BitNumber;				/* bitmap number for class */
	int                 *ClassCount;			/* no. of ITCs of that class within the stand */
	int                 *ClassCrownArea;			/* accumulates full crown areas of ITC of that class of that stand */
	int                 *ClassClosure;			/* accumulates inside crown areas of ITC judged inside stand*/
	int		    		*ClassHeight;			/* accumulates max height within ITCs */
	struct class_info   *link;
} class_info_node;				


/* function declarations */

void	fill_isol(int, int, int, unsigned char *, int *);
void    add_class_list(class_info_node *, class_info_node *);
void    scan_for_trees_xxx(int, int, unsigned char *, unsigned char *, class_info_node *);
void	count_isol(int, int, unsigned char *);
void    view_results(FILE *, FILE *, class_info_node *, int *);


// For each new field, create it, and populate output layer with PCD-generated data

void	Copy_Shape_File(char *, char *);	//  Copy Input polygon file info to output shape file
void 	Prep_New_Fields(OGRLayer *);		//// create all the needed PCD fields in the output shape file
void 	Add_New_Info(OGRLayer *, class_info_node *);		// populate all the new PCD fields in the output shape file



// void	convert_bitnum(int64 bitnum, int *byte, int *bit); 

//void 	Poly2raster(int, GDBVertex *, int, GDBShapeId, GDBLayer, int); 		 // burns a polygon into an image 

void    Upper_Case(char *);
int  	between(float a, float b, float c);
//void 	prep_fields(GDBLayer);
void 	param_expand(int, int *, int *, int *);
char       *itostr(int *, int);

class_info_node *new_class_info_node(void);

// Pointer that allows us to deal with many types of image (8,16u,16s) 
// Now defined in itc_io_g 

extern PIX_FUN_PTR get_pix_val;		// will get it from itc_io_g when image gets checked

//***************************************************************************************

// Global Vars 
 
FILE     	*idb_fp;  

char 	polygonfile[FILENAME], pcdfile[FILENAME], heightfile[FILENAME];


uint16 		*stand_image, *ppp;		/* pointer to internal image (16 bit) to store polygons */
unsigned char 		*stand_ima,   *ppp8;		/* pointer to internal image (8 bit) to store polygons */

int		flag8b = 0;			// default is more than 256 polygons, thus a 16it internal image

int		ima_type;

char		*test_image, *test_image2;
int 		polymax, numofpoly, details, order; 
int 		counter, count_in, count_out, max_val, p_val;
int 		dumpflag = 0; 
int			height_flag = 0;
FILE        *odb_fp, *dummy_fp;
FILE		 *class_fp;
float		xpixsz, ypixsz, apixsz;
char		pixunit[40];
double 		topleftX, transformX, topleftY, transformY, botrightX, botrightY;  /* for geographic mapping */
char		geosys[17], history[156];
char		Fname[10];  		// Field name
int 		Lines, Pixels, Channels;
int64		bmsize;
char 		Extension[10]="", extension[10]="";
int			xcg, ycg;		/* center of gravity (aprox.) of current man tree */

int 	tot_class_ITCcount = 0;			// total ITCs for a given class
int 	Class_ITCcount[32];				// total ITCs for  class n
int64	ITC_Total_Count=0;

//GDBLayer	slayer;
//ProjInfo_t	sProj;
////GDBShapeId	hShapeId;
//GDBVertex	*pasVertices;
//GDBField	sField;
int			nVertex, iField, oField, numclasses;
char		*mpszOptions;
int			*shapenum;
int 		data_type;
void		*himage;
int 		*TotalIsolArea, *TotalClosure, *IsolCount, *IsolHeight, *IsolSQHeight;
int			*polyarea, iarea;
int			* poly_pixcount;

extern int 		Poly_Outside;
extern int 		fill_count;				// count of fill pixels from vect2rast_g()


int			bylines_flag;			// default is to read/write by image (faster),
int	by_lines=0, by_image=1;			// default is to read/write by image (faster), 


//GDBField 	*RingStart_p;
int		RingStart[100], RingField, RingStartCount;	/* at max., 100 sections to a shape */
int 		RingFlag;			/* Flag that some shapes may have rings as per a RingStart attribute*/
int 		RingFlag2;		/* Flag on if rings are present within this particular shape */ 

int 		ixmin, ixmax, iymin, iymax;

FILE *Report;		/* Global Vars - To compensate for "faulty" Report variable from core1000.dll */

// Global variables to read images, bitmaps and vector layers with GDAL (Specially with itc_io_g.cpp)

GDALDataset	*ima_in, *ima_out, *seg_in, *poDS, *ht_in;
GDALDriver 	*piDriver, *poDriver;
GDALRasterBand	*piBand,*piBand2,*poBand;

OGRLayer    *piLayer, *poLayer; 		//poLayer & poFeature are only used when creating a separate output shp file
OGRFeature 	*piFeature, *poFeature;
OGRFeature 	*gpiFeature, *gpoFeature;		// usefull to pass Feature pointer to other function (was a test)
OGRFeatureDefn	*piFDefn, *poFDefn;
OGRFieldDefn	*piFieldDefn;
OGRFieldDefn	*poFieldDefn;
OGRFieldDefn	*newFieldDefn;

OGRSpatialReference *piSRS,  *poSRS;
OGRGeometry 	*piGeometry, *poGeometry;

GDBVertex2D * pasVertices;


int 	iFeat, layer_count, feat_count, field_count, last_feat, last_field;
int 	BM_ERR=0, VEC_ERR=0;

PixVal		*pafScanline;
uint16		*pafScanline16;

char **papszOptions = NULL;
char **papszMetadata;

char 		*Proj, *Proj2, *Datum, *Datum2, *Temp,*token;
double		adfGeoTransform[6], adfGeoTransform2[6];
char 		province[64];
char 	 	description[128];		// for PCI
char 	 	Description[128];		// for GDAL via write_bitmap()
char		answer[40];

int 		ch_in, ch_out, in_ch[10], segm_in;
int			imaFile_opened;
int64		bitnum;


//int 	BM_ERR=0, VEC_ERR=0;
//int	SEG_BIT=101, SEG_VEC=116;		//PCI nomemclature

//int		data_type;			// need to be global	
//int		segtype[32];
int    sp_indx = 0 ; 			//temp for debugging
int 	class_field;

class_info_node * class_list_beg;		//beginning of list containing classs-related info  



//************************************************ */

int main(int argc, char *argv[])
{

/* var list */


int      i, j, k, ii, jj, kk,h;
int64		iii;


char	file[FILENAME];
int		isolbit;
int		classbit[BITMAPS], iclassbit[16];
char	report[FILENAME],  odetails[64];

char 	filename[FILENAME], temp[FILENAME], temp2[FILENAME], fullfilename[FILENAME];
char	path[FILENAME], main_path[FILENAME], tst_path[FILENAME], cls_path[FILENAME];

int 	path_len, main_path_flag, tst_path_flag, cls_path_flag;

char	file_ITC[FILENAME], given_fname[FILENAME];
//char 	file_TAS[BITMAPS][FILENAME];
char 	file_CLASS[BITMAPS][FILENAME];
int 	name_given = 0;		// flag if base name was given for testing areas
char 	*file_in, *p;
char	ext[4], seg_no[3];

char 	* pch;			// useful char pointer
char	*basefname;						// ** just a pointer **
char	basefilname[FILENAME];
int		basef_len;	

int NumberOfInnerRings, NumberOfInteriorRingVertices, NumberOfExteriorRingVertices;
int Poly_Out_Count =0;

int 		polylayr;

int			heightch;
int64		mem_required=0, image_size;
int			segtype;
char		segflag, segname[9];
long 		start, length;
int 		set_flag;


int		Gtemp, outsideflag, old_num_fields, new_field_nb;
int             iclass, poly, painted, notpainted;
int             xsize, ysize, channels;

unsigned char  *isolbitbuffer, *classbuffer, *tempbitbuffer;
char 		seg_description[81]="NADA";
char		seg_history[81]="NADA";
class_info_node  *class_list;
class_info_node  *new_class;
float 		imgarea;
int 		North, notclosed;
void            *test_ptr;

int64 		checksum;

int 	PCI_File = 0; 		// initial flags for file types
int 	TIF_File = 0; 
int 	TXT_File = 0; 
int 	SHP_File = 0;	
 

time_t 	rawtime, rawtime0;
struct 	tm * timeinfo0;
struct 	tm * timeinfo;


/* 
// Assign parameter pointers to argumnets 

args[0] = (void *) file;
args[1] = (void *) &isolbit;
args[2] = (void *) iclassbit;
args[3] = (void *) &polylayr;
args[4] = (void *) odetails;
args[5] = (void *) province;
args[6] = (void *) pcdfile;
args[7] = (void *) &heightch;
args[8] = (void *) report;

// Setup standard PCI/EASI interface

IMPStatus("FILE, ISOLBIT, CLASSBIT, POLYLAYR, ODETAILS, PROVINCE, PCDFILE, HEIGHTCH, REPORT;",
		  "C,    I,       I,	    I,        C,   	C,   	   C,      I,        C;",
		  "132   1,       16,       1,        64,   	64,  	   64,     1,        132;",
		  "1,    1,       1,	    1,        3,   	3,   	   0,      0,        1;",
		  "ITCPCDF.", "FORCE", argcnt, args, argc, argv);

// To compensate for "faulty" Report variable from core1000.dll 

if(EQUALN(report,"TERM",4)) {Report=stdout;}else{Report = fopen(report, "w");}

 */

	Report = stdout;

//	strcpy(pcdfile,"PCD_Output.shp");			// default if nothing (not even"-") is specified

// ****************************************************************************

// Print Program Header 

	time(&rawtime0);  
	timeinfo0 = localtime(&rawtime0);
	printf("\n\t\tStarting %s (%s) at %s\n", PROG_NAME, VERSION, asctime(timeinfo0));

	//printf("\n\t\tStarting min: %d sec: %d \n", timeinfo0->tm_min, timeinfo0->tm_sec);		
	//exit(0);
	
	
// Registers for all types of files with GDAL

	GDALAllRegister(); 	

// ***************

// Check arguments on command line

// for debugging remind me of parameters 

  //printf("USAGE:  itcpcd_g BaseFilename ISOLBIT CLASSBIT POLYLAYR ODETAIL PROVINCE HEIGHTCH PCDFILE\n");


	if (argc < 4) 		// minimum number of parameters
	  {
	  printf("\n\t *** PROBLEM with INPUTS parameters (at min 4 parameters, other to be defaulted \n");
	  printf("USAGE:  itcpcd_g BaseFilename ISOLBIT CLASSBIT POLYLAYR ODETAIL PROVINCE HEIGHTCH PCDFILE\n");
	  exit(-1);
	  }   



// ***************

Arg1:



// Check *First argument* on command line

// OPEN image FILE  and get info (needed for input parameteers checking)
// Will report if PCI or TIF file

	strcpy(filename, argv[1]);
	
	ima_in = open_imaFile(filename);		// OPEN  image file (PCI or TIF) and get info about it
	
	xsize = Pixels; ysize = Lines;			// use throughout program

	//printf("Image Description: %s \n", ima_in->GetDescription() );

	image_size =  Pixels * (int64)Lines;			/* size of an 8bit image */
	bmsize =  (( Pixels * (int64)Lines + 7) / 8 );  		/* bitmap size in bytes */
	//printf("\nImage size (in bytes): %Illd \n", image_size);	
	//printf("Bitmap size (in bytes): %Illd \n", bmsize);

	// Extension is a global parameter set by Open_imaFile()
	
	if (EQUALN(Extension,"pix",3)) PCI_File = 1;	// everything is in the PCI file
	if (EQUALN(Extension,"tif",3)) TIF_File = 1;	// everything is in directory, mostly as tif files

	if (! ( PCI_File || TIF_File))
	  {
	  printf("\n\n**ERROR** Program not able to deal with image file of type %s\n\n", Extension);
	  printf("\nRead error message before program exits   ..."); 	// for user time to read when using ArcGIS
	  answer[0] = getc(stdin); 		// gets any answer or plain <CR>	  
	  exit(-1);
      }

	// Create base file name  
	 
	
// Need to get "basefilename" when full path is involved
// Basefile name has path + head of file name (typically correspond to "named area" of study e.g. PRF)
// Area name is assumed separated from rest of file name by an underscore
// However, be careful as there could be underscores in the path

	strcpy(fullfilename,argv[1]);			// main input filename (and possibly its dir)
	
	for (ii=0; ii < strlen(fullfilename); ii++)		// search for last underscore position
	  {
	  jj = strlen(fullfilename) - ii;				// start from the end
	  //printf("Count back: %d",j);
	  if(fullfilename[jj] == '_') {basef_len = jj;	break;}	// find last underscore in full file name
	  }
	//printf("\nBase Filename Length:  %d \n", basef_len);
  	
	strncpy(basefilname, fullfilename,  basef_len);		// get that part of  the full file name
	basefilname[basef_len] = '\0';   					// make it a string to be safe
	
	printf("\nBase file name ::  %s \n", basefilname);	

	//exit(-1);			//for debugging 



// 	Get full directory  PATH to that MAIN file (possibly used by others)

 
	strcpy(fullfilename,argv[1]);			// main input filename (and possibly its dir)
	//printf("Full Filename:  %s \n", fullfilename);
	//printf("\n Fullfilename string length is  %d \n", (int)strlen(fullfilename));
	
	jj = 0; path_len = 0;
	for (ii=0; ii < strlen(fullfilename); ii++)			// could be slash or backslash
	  {
	  jj = strlen(fullfilename) - ii;		// from the end of full file name
	  if( (fullfilename[jj] == '/') || (fullfilename[jj] == '\\') ){path_len = jj;	break;}	// find last slash in full file name
	  }  
	//printf("\nPath Length:  %d \n", path_len);
	//printf("\n jj =  %d \n", jj);	


	if(path_len == 0) strcpy(main_path, "./");				// no path  - impose a default path
	else
	  {
	  strncpy(path, fullfilename, path_len+1);		// get the path (inc. last slash)
	  path[path_len+1] = '\0';   					// null character manually added 
	  strcpy(main_path, path);	  
	  }

	printf("\nPath of main file ::  %s \n", main_path);			// may have full path to directory
	
	// exit(-1);		// for debugging
 		

// *****************************

// Check argument argv[2] on command line : ISOLBM 

Arg2:

	printf("\n\tAs per user input, ISOLBIT number (or file) to use: %s \n", argv[2]);	

	// First check - no multiple comma-separated entries allowed for this one
	
	char * pt;
	strcpy(temp, argv[2]);  pt = strtok(temp, ","); ii = 0;
	while(pt != NULL) 
	  {
 	  //printf("%s\n", pt); 
	  ii = ii + 1;
	  pt = strtok(NULL, ",");
	  }
	int no_items = ii;

	if(no_items > 1) 	
	  {
	  printf("\nAs per user input, ISOLBIT channel number to use: %s \n", argv[2]);
	  printf("\n\t** ERROR This require a single entry (and no range allowed)\n\n");
	  printf("\nRead error message before program exits   ..."); 	// for user time to read when using ArcGIS
	  answer[0] = getc(stdin); 		// gets any answer or plain <CR>
	  }
	  
	// Special notation cases (ITC or ISOL)
	basefname = temp;	 				// necessary cause just a pointer no space assigned to it
	strcpy(basefname, basefilname);	

	if(EQUALN(argv[2],"ITC",3)) 
		{
		file_in = strcat(basefname,"_ITC");  
		strcat(file_in,".tif");
		strcpy(file_ITC, file_in);
		printf("ISOLBIT file_in  :  %s \n", file_ITC); 
		goto Arg3; 
		}	
	
	if(EQUALN(argv[2],"ISOL",4)) 
		{
		file_in = strcat(basefname,"_ISOL");  	
		strcat(file_in,".tif");
		strcpy(file_ITC, file_in);
		printf("ISOLBIT file_in  :  %s \n", file_ITC); 
		goto Arg3; 
		}
 
 
	isolbit = strtol(argv[2],NULL,10);		// pickup ISOLBM number - used with both file types
	

	// Check if user entry is a full file name (i.e., it"s not just a plain number)
	
	if (isolbit == 0)				// if not number, then it's a specific file name
	  {
	  strcpy(file_ITC, argv[2]);
	  //printf("\n** Will use ITC file %s \n", file_ITC);		// that's all
	  goto Arg3;
	  }
 
	  
	// If number was used, Create AUTOMATIC input file name for ISOLBM tif file	e.g.: SECTEUR_BM4.tif

	
	file_in = strcat(basefname,"_BM"); strcat(file_in,argv[2]); 	 	// Specific bitmap number
	
	strcat(file_in,".tif");
	strcpy(file_ITC, file_in);
	
	printf("ISOLBIT file_in  :  %s \n", file_ITC);   

		  
// *****************************

// Check CLASSBIT  argument argv[3] on command line :  

Arg3:

	if (EQUALN(argv[3],"-",1) || EQUALN(argv[3],"#",1)) // no class to be considered
	{
	printf("\n\tAs per user input, NO CLASS to be considered, just generic ITC info to be reported\n");
	details=1; 
	goto Arg4;
	}	

	strcpy(temp, argv[3]);

	printf("\n\tAs per user input, CLASSBIT number (or file) to use: %s \n", argv[3]);	
	printf("\tClass bitmap numbers should correspond to separate <<.tif>>  files\n");	
	printf("\tClass bitmaps typically look like <xxxx_Class1.tif>> \n")	;	

	// Check if user entry has a file name already in it (i.e., not just numbers)
	
	name_given = 0;		// assume no file name was given

	
		
	  if (strtol(argv[3],NULL,10) == 0)				// if not numbers, then it's a specific file name mentionned 
	    {
		pt = strtok(temp, ",");  		// remove numbers after file name
		pt = strtok(temp, "."); 		// remove extension after file name
		strcpy(ext, strtok(NULL, " "));		// get extension to put back later
		strcpy(given_fname,temp);			// get filename (and possibly a path)
		//printf("\n\nClasses Given_fname %s and its length %zd \n", given_fname, strlen(given_fname));
		
		// check if already a number in filename and REMOVE IT (usefull for cut & paste of a filename)
		if(isdigit(given_fname[strlen(given_fname)-1]))  given_fname[strlen(given_fname)-1]='\0'; //rm one digit
		if(isdigit(given_fname[strlen(given_fname)-1]))  given_fname[strlen(given_fname)-1]='\0'; //rm a 2nd digit		
	
	    printf("\n\t*Will use given file prefix (given name) \"%s\" for class bitmaps\n\n", given_fname);	
		name_given = 1;			// flag that a specific base name for training area was given
		//exit(-1);
		}	  

//	printf("   temp = %s \n", temp);
//	exit(-1);
	
	
// Adjust for "common" notation (e.g.: 6-13), different than PCI notation (e.g.: 6,-13)

// On detecting a minus sign, not preceeded by a comma, 
//	insert a comma and keep the minus sign needed by   param_expand( )


	for(ii = 0; temp[ii] != '\0'; ii++) { temp[ii] = ' '; }   // clean up temp

	strcpy(temp2, argv[3]);
	jj = 0;
	for (ii=0; ii < strlen(temp2); ii++)
	  { 
	  temp[jj] = temp2[ii]; 	  // just copy 	  
	  if ( (temp2[ii] == '-') && (temp2[ii-1] != ',') ) { temp[jj] = ','; temp[++jj] = '-'; } // deal minus and comma 
	  jj++;
	  }
	  temp[jj] = '\0';


//	printf("    new_temp = %s \n", temp);
//	exit(-1);




// Prepare to deal with numbers after file name *OR* channel numbers only			  
	  
	// For numbers after file name, goto 1st comma, then 2nd comma and to get the 1st digit(at pointer p)
	
	if(name_given) { pt = strtok(temp, ","); pt = strtok(NULL, ","); }

	//printf("\n\n #2   argv[3]=%s  temp=%s  pt=%s  \n\n", argv[3], temp, pt);
	//exit(-1);
	
	//	 For channel numbers only -- goto 1st comma  and to get the 1st digit (at pointer p)

	if(!name_given) { pt = strtok(temp, ",");  }
	
	//printf("\n\n #3   argv[3]=%s  temp=%s  pt=%s  \n", argv[3], temp, pt);
	//exit(-1);


// Separate class bitmaps (files)  that are separated by commas

	
	ii = 0;
	while(pt != NULL) 
	  {
 	  //printf("%s\n", pt); 
	  iclassbit[ii++] = strtol(pt,NULL, 10);			//  possible list of test areas  to use
	  pt = strtok(NULL, ",");
	  }
	no_items = ii;

//	printf("\nNumber of Classes to consider = %d \n", no_items);
	
	
	
/* Expand list of class bitmaps if ranges are being used by user*/
/* This make possible short hand notation using PCI ranges and in this case,
   also make possible for up to 32 signatures (classes) to be used */

	param_expand(no_items, iclassbit, classbit, &numclasses);

	printf("\nNumber of Classes to consider = %d \n", numclasses);
	
	if( numclasses > BITMAPS ) 
			{ 
			fprintf(stderr,"\n ERROR - max # of classes(32)  exceeded \n");	
			printf("\nRead error message before program exits   ..."); 	// for user time to read when using ArcGIS
			answer[0] = getc(stdin); 		// gets any answer or plain <CR>
			exit(-1);
			}

/* 
	printf("\nInput class bitmap to use : \n");
	for(i=0; i<numclasses; i++) 	printf("Class %d is %d \t", i+1, classbit[i]);
	printf("\n\n");

 */
	
// 	Get full directory  PATH for these  files 

	path_len=0;	jj=0; 
	cls_path_flag = 1 ;		// assume that path was supplied within test file string
 
	strcpy(fullfilename, argv[3]);
//	printf("Full Filename argv[4]:  %s \n", fullfilename);
//	printf("\n Fullfilename string length is  %d \n", (int)strlen(fullfilename));
	
	for (ii=0; ii < strlen(fullfilename); ii++)			// could be slash or backslash
	  {
	  jj = strlen(fullfilename) - ii;		// from the end of full file name
	  if( (fullfilename[jj] == '/') || (fullfilename[jj] == '\\') ){path_len = jj;	break;}	// find last slash in full file name
	  }   
	  fullfilename[path_len] = '\0';
	  
 	  strcpy(cls_path, fullfilename);			// when a path is given 
	  
	    
//	printf("\nPath Length:  %d \n", path_len);	
//	printf("\nPotential path for classes:  %s \n", fullfilename);

	if(path_len == 0)			// NO PATH was supplied with class  file string
	  {
	  cls_path_flag = 0; 				//no path was supplied with shp file string
	  //strcpy(path, ".\\");				// no path  - impose a default path
	  strcpy(cls_path, main_path);				// OR use MAIN file path	  
	  //strcpy(basefname, fullfilename);		//so base_fname is fullfilename
	  }

	printf("\nPath for classes is or will be :  %s \n\n", cls_path);





 	// Create input file names from given numbers		e.g.: SECTEUR_BM14.tif

	for(i=0; i<numclasses; i++)
		{		

		strcpy(basefname, basefilname);		
		
	    file_in = strcat(basefname,"_Class"); 
	    //printf("file_in  :  %s \n", file_in);    
  	    //itoa(classbit[i],seg_no,10); 
		sprintf(seg_no,"%d",classbit[i]);		
	    strcat(file_in,seg_no);	  	  	
 	    strcat(file_in,".tif");
	    //printf("file_in  :  %s \n", file_in); 					
		strcpy(file_CLASS[i], file_in);  
	    printf("%d Class bitmap  file_in  :  %s \n", i+1, file_CLASS[i]); 	
	
		// check if that file exist
		  
		class_fp = fopen(file_CLASS[i],"r");
		//if (!class_fp) { printf("\n   *** File may not exist !!! \n\n"); exit(-1); }
		if (class_fp == NULL) { printf("\n   *** File may/does not exist !!! \n\n"); exit(-1); }
		fclose(class_fp);		
		
		}		
	

	//exit(-1); 		// For testing

// *****************************

// Check ODETAIL argument argv[4] on command line

Arg4:		

	//printf("\n\tAs per user input, POLYLAYR number (or file) to use: %s \n", argv[4]);	

	strcpy(polygonfile, argv[4]);

	printf("\n\tAs per user input, POLYLAYR number (or file) to use: %s \n", polygonfile);		

	//exit(-1); 		// For testing





Arg5:

	if (argc < 6) goto Proceed;


	printf("\n\tAs per user input, ODETAIL to use is: %s \n", argv[5]);	

 	if ( EQUALN(argv[5],"-",1) || EQUALN(argv[5],"#",1) || EQUALN(argv[3],"-",1) || EQUALN(argv[3],"#",1) )
	  {
 		strcpy(odetails, "LOW");
		printf("\n** Level of details not specified. It will be  set to %s \n", odetails);
	  }

	else strcpy(odetails, argv[5]);

	//exit(-1); 		// For testing




/* Check the level of details needed and how to order the ITC info (predominant species) for each stand */

Upper_Case(odetails);
if (strncmp(odetails,"LOW",3) == 0) details = 1;
if (strncmp(odetails,"MEDIUM",3) == 0) {details = 2; order = 2;}	/* default order for MED is by CC */
if (strncmp(odetails,"HIGH",3) == 0) {details = 3; order = 1;}

if (strncmp(odetails,"HIGHSP",6) == 0) order = 1;	/* order species info as per bitmap (CLASSBIT) order */
if (strncmp(odetails,"HIGHCC",6) == 0) order = 2;	/* order species info by crown closure dominance */
if (strncmp(odetails,"HIGHITC",6) == 0) order = 3;	/* order species info by ITC count dominance */
if (strncmp(odetails,"MEDIUMCC",7) == 0) order = 2;	/* order species info by crown closure dominance */
if (strncmp(odetails,"MEDIUMITC",7) == 0) order = 3;	/* order species info by ITC count dominance */

//	printf("\n\tAs per user input, DETAIL number to use is: %d \n", order);	

	//exit(-1); 		// For testing


Arg6:
	if (argc < 7) goto Proceed;

	if ( EQUALN(argv[6],"-",1) || EQUALN(argv[6],"#",1) )
	  {
 		strcpy(province, "ONT");
		printf("\n** PROVINCE to use is not specified. It will be  set to %s \n", province);
	  }		
	else
	  {		
	  strcpy(province, argv[6]);
	  Upper_Case(province);
	  printf("\n\tAs per user input, PROVINCE to use is: %s \n", province);
	  }	

	//exit(-1); 		// For testing


Arg7:
	if (argc < 8) goto Proceed;

	printf("\n\tAs per user input, HEIGHTCH to use is: %s \n", argv[7]);	

	if ( EQUALN(argv[7],"-",1) || EQUALN(argv[7],"#",1) )
	  {
 		height_flag = 0;	
		printf("\n\tUser did not specify any HEIGHTCH to use so ... \n");
		printf("\n\t** HEIGHT output field will be empty\n");
		strcpy(heightfile, "NADA");
	  }	
	else 
	  {
	  height_flag = 1; 
	  strcpy(heightfile, argv[7]);
	  //printf("\nHEIGHT will be reported in output file or as output field \n");
	  }	

	//exit(-1); 		// For testing


Arg8:

// Mostly default to shape file rather than text file (at V8.2)

	
// if no input, OR "-" OR "#"    use default name for the shape file
	

	if (  (argc < 9) || EQUALN(argv[8],"-",1) || EQUALN(argv[8],"#",1) )		
	  {  
	  dumpflag = 0; 		SHP_File = 1;
	  strcpy(basefname, basefilname);			//construct file name of default output shape file
	  strcat(basefname, "_PCD_Out.shp"); 
	  strcpy(pcdfile,basefname);	
	  strcpy(extension,"shp"); 	  
	  printf("\n\tAs per default, output shape file (PCDFILE) will be produced:  <<%s>> \n", pcdfile);  
	  }
	  
	if (argc >= 9) 
	  { 
		if ( EQUALN(argv[8],"X",1) || EQUALN(argv[8],"x",1) )
		  {
		  dumpflag = 1; 		TXT_File = 1;
		  strcpy(basefname, basefilname);			
		  strcat(basefname,"_PCD_Out.txt "); 
		  strcpy(pcdfile,basefname);  
		  strcpy(extension,"txt"); 
		  printf("\n\tAs per default, plain text outputfile (PCDFILE) will be produced:  <<%s>> \n", pcdfile);	
		  }
	
	else		// IF there is a file mentionned, use it
		  {
		  strcpy(pcdfile, argv[8]);
		  strtok(argv[8],".");		// go pass the dot and get the extension
		  pch = strtok(NULL," "); 
		  strcpy(extension,pch);	 
			  	    
//	Extension should be ".txt" OR ".shp"	
	  
	if (EQUALN(extension,"txt",3))  
		{
		dumpflag = 1; 		TXT_File = 1;
		printf("\n\tAs per user, PCDFILE (plain text file) to produce will be:  <<%s>> \n", pcdfile);		
		}
	  
	if (EQUALN(extension,"shp",3)) 
		{
		dumpflag = 0; 		SHP_File = 1;		
		printf("\n\tAs per user, PCDFILE (shape file) to produce will be:  <<%s>> \n", pcdfile);
		}
  
	if (! ( TXT_File || SHP_File))
		{ 
		printf("\n\n**ERROR** Program not able to deal with PCD file of type %s\n\n", extension);	
		printf("\nRead error message before program exits   ..."); 	// for user time to read when using ArcGIS
		answer[0] = getc(stdin); 		// gets any answer or plain <CR>
		exit(-1);
		}
	   }
	  }  

/* Open output file in which to dump polygon content information (if required) */

if (dumpflag)
  {
  if (strncmp(odetails,"HIGH",4) == 0) strcpy(odetails,"HIGH");	/* to truncate SP or CC (not available) */
  if (strncmp(odetails,"MED",3) == 0) strcpy(odetails,"HIGH");	/* MEDIUM not available in this mode */
  printf("\nPolygon content description to be written to PLAIN file <<%s>>\n", pcdfile);
  printf("\nLevel of details as previously requested: %s \n\n", odetails);
  
  odb_fp = fopen(pcdfile, "w");		/* Open file for database output */
  
  if (odb_fp == NULL) 
		{
		printf( "File System Error.  Can't create file.\n"); 
		printf("\nRead error message before program exits   ..."); 	// for user time to read when using ArcGIS
		answer[0] = getc(stdin); 		// gets any answer or plain <CR>
		exit(-1);
		}
  }

if (!dumpflag)
  {
  printf("\nPolygon content description to be written to vector file  <<%s>>\n", pcdfile);
  //printf("New fields will be added to existing ones (unless they already exist).\n");
  printf("\nLevel of details as previously requested for this run : %s \n\n", odetails); 
  }


// Stopping program to read screen (or for debugging)

printf("\n\n\t OK to continue(y/n)? ");
answer[0] = getc(stdin); 		// gets any answer or <CR>
if(answer[0]=='n')  exit(1);


//**************************************************************************
//**************************************************************************

Proceed:

	time (&rawtime);  
	timeinfo = localtime (&rawtime);
	printf("\n\n\n\t\t %s (%s) proceeding %s\n", PROG_NAME, VERSION, asctime(timeinfo));
 

// Open INPUT POLYGON LAYER  (shp file) pointed to by user


	// Both modes are to open file in READONLY as we now have a separate output shape file
	// That is, we are not updating an existing shp file or PCI vector segment

		seg_in = (GDALDataset*) GDALOpenEx(polygonfile,  GDAL_OF_VECTOR, NULL, NULL, NULL );
		if( seg_in == NULL ){printf( "**** Failed to open input polygon file %s\n", polygonfile); exit(-1 );}
		printf("\n\t**File '%s' was opened for reading only : single layer mode (vector)\n\n", polygonfile);



	int layer_count = seg_in->GetLayerCount();

	//printf("\n\t*Number of vector layers in input file is %d \n\n", layer_count);
	
	if(layer_count > 1) 
		{
		printf("\n\tERROR - Can only deal with one layer at this time \n\n"); 
		printf("\nRead error message before program exits   ..."); 	// for user time to read when using ArcGIS
		answer[0] = getc(stdin); 		// gets any answer or plain <CR>
		exit(-1);
		}



	//exit(-1); 		// For testing


	 printf("\n**Accessing input vector layer %d \n", 1);

	piLayer = seg_in->GetLayer(0);		// Layer numbers start at zero

	piLayer->ResetReading();	// just to be on the safe side (good practice)

// Get polygon spatial reference		** NOGO **  Use GetSpatialRef() for shp files

//	strncpy(temp, seg_in->GetSpatialRef(),30);
//	if( seg_in->GetSpatialRef()( ) != NULL) printf( "Projection is '%s'\n\n", temp);

	printf("\t **Shape file geographic info : \n");
	printf("%s\n", (char *) piLayer->GetSpatialRef());


//	printf("\nGot here 1\n");

// Get polygon spatial reference (shp file spatial reference)

  	//printf( "shp file Full projection is %s \n\n", seg_in->GetSpatialRef() );

//	printf("\n shp geo info : %s \n", seg_in->GetLayer(0)->GetSpatialRef->ExportToWkt());

/* 
printf("\n shp geo info : %s \n", (char *) seg_in->GetLayer(0)->GetSpatialRef());

	printf("\nGot here 2 \n");

	Proj2 = (char *) CPLMalloc(200);

	if( seg_in->GetSpatialRef() != NULL ) 
	  {
	  strncpy(temp, (char *) seg_in->GetLayer(0)->GetSpatialRef(),30);
	  strtok(temp, "\"");
	  //printf("1st section : %s \n", temp);
 	  Proj2 = strtok(NULL, "\"");
	  printf("* Shp file Projection: %s \n", Proj2 );
	  }
*/
	//	exit(-1);
 

// Number of FEATURES (i.e., polygons) in that layer AND number of fields in that layer

	piFDefn = piLayer->GetLayerDefn();
	feat_count = piLayer->GetFeatureCount();
	field_count = piFDefn->GetFieldCount();

	printf("\n**Layer %d of current file has %d features (shapes) with %d fields each\n\n", 
				1, feat_count, field_count);

	//exit(-1); 		// For testing
/* 
	feat_count = 10;		//for testing 
		printf("\n** FOR TESTING **  pretend %d features(shapes) with %d fields each\n\n", 
					feat_count, field_count);
	 */				
					

	
// To DOUBLE CHECK - Print ALL existing FIELDS (for PCI or TIF main files)
/* 
  printf("\nDouble checking on some SHP file FIELDS  existance \n\n");

//  for (iField=0; iField < field_count; iField++)
   for (iField=0; iField < 5; iField++)			//print five(5) fields
    {
	piFieldDefn = piFDefn->GetFieldDefn(iField);
	printf("Input field %d, Type %d, Field Width %d, Precision %d Name: %s\n",
		iField, piFieldDefn->GetType(), piFieldDefn->GetWidth(), piFieldDefn->GetPrecision(),piFieldDefn->GetNameRef() );  
	}
 
 */

//	exit(-1); 		// For testing



//************************************************************************

// Getting polygon vertices

   PolygonFeature Polygon, poPolygon;
   OGRPoint ptTemp, ptTemp2, ptTemp3, ptTemp4;

// Got through all of the features

//	printf("\nLooping through all the features ...\n\n");
//	while( (piFeature = piLayer->GetNextFeature()) != NULL )

/* 
	printf("\n\tLooping through SOME (max. 5) features for polygon vertices ...\n\n");

	last_feat = feat_count;
	if(last_feat > 5) last_feat = 5 ;				// for TESTING check five(5) features (polygons)
	
	piLayer->ResetReading();			// VERY IMPORTANT (if you use GetNextFeature() 

	for (iFeat=0; iFeat<last_feat; iFeat++)
	{
	  piFeature = piLayer->GetNextFeature();
	  piGeometry = piFeature->GetGeometryRef();
	
	  printf("\n ** Feature %d : Geometry is : %d \n", iFeat, wkbFlatten(piGeometry->getGeometryType()));


	  if ( piGeometry != NULL && wkbFlatten(piGeometry->getGeometryType()) == wkbPolygon )

	    {
	    OGRPolygon *piPolygon = ( OGRPolygon * )piGeometry;

	    Polygon.PolygonsOfFeature.resize(1);

		// Number of innner rings
	    NumberOfInnerRings = piPolygon->getNumInteriorRings();
	    printf("Number of inner rings = %d \n", NumberOfInnerRings);


            OGRLinearRing *piExteriorRing = piPolygon->getExteriorRing();

            Polygon.PolygonsOfFeature.at(0).Polygon.resize(NumberOfInnerRings+1);
            Polygon.PolygonsOfFeature.at(0).Polygon.at(0).IsClockwised = piExteriorRing ->isClockwise();


        NumberOfExteriorRingVertices = piExteriorRing->getNumPoints();

	    printf("NumberOfExteriorRingVertices = %d \n", NumberOfExteriorRingVertices);

            Polygon.PolygonsOfFeature.at(0).Polygon.at(0).RingString.resize(NumberOfExteriorRingVertices);

            for ( int k = 0; k < NumberOfExteriorRingVertices; k++ )
             {
               piExteriorRing->getPoint(k,&ptTemp);
               MyPoint2D pt;
               pt.dX = ptTemp.getX();
               pt.dY = ptTemp.getY();
               Polygon.PolygonsOfFeature.at(0).Polygon.at(0).RingString.at(k) = pt;
             }


         for ( int h = 1; h <= NumberOfInnerRings; h++ )
           {
               OGRLinearRing *piInteriorRing = piPolygon ->getInteriorRing(h-1);

               Polygon.PolygonsOfFeature.at(0).Polygon.at(h).IsClockwised = piInteriorRing->isClockwise();
            	NumberOfInteriorRingVertices = piInteriorRing->getNumPoints();
	       //printf("NumberOfInteriorRingVertices = %d \n", NumberOfInteriorRingVertices);

               Polygon.PolygonsOfFeature.at(0).Polygon.at(h).RingString.resize(NumberOfInteriorRingVertices);

		   for ( int k = 0; k < NumberOfInteriorRingVertices; k++ )
		       {
			   piInteriorRing ->getPoint(k,&ptTemp);
			   MyPoint2D pt;
			   pt.dX = ptTemp.getX();
			   pt.dY = ptTemp.getY();
			   Polygon.PolygonsOfFeature.at(0).Polygon.at(h).RingString.at(k) = pt;
		       }
           } 
              //PolygonLayer.push_back(Polygon);

		}

	}

 */


	//exit(-1); 		// For testing


/* 

// Get geographic projection information 

GDBGetProjectionInfo(slayer, &sProj);      
printf("\nOpening POLYLAYR vector layer %d (%s)\n",polylayr,sProj.Units);
  
if( ! EQUALN(sProj.Units, geosys, 5) )
  {
  printf("\n\n ### ERROR ### \n");
  printf("\nGeographic projection of POLYLAYR vector layer not the same as that of the image.\n\n");
  exit(-1);
  }	 

 */

/* 
// Check if field "RingStart" exist, possibly indicating shapes within shapes 

iField = GDBGetFieldIndex(slayer,"RingStart");		// check if field exist 
if (iField == -1)  RingFlag = 0;				// if field not there, then ring flag = false 
if (iField != -1)	{RingFlag = 1; RingField = iField ; }	// Flag some shapes may have rings so check 
if (iField != -1) 
  {
  printf("\nNote: \nPossible rings (donuts) within shapes. There's a RingStart field at field %d. \n", RingField+1);
  printf("This version of ITCPCD should be able to deal with rings and split stands (e.g., a road through it) \n");
  }

 */

/* 
	feat_count = 10;		//for testing 
		printf("\n** FOR TESTING **  pretend %d features(shapes) with %d fields each\n\n", 
					feat_count, field_count);
 */					
					
 RingFlag = 0;				// #####  for the moment  assume no island in forest stands
    

printf("\n\tNumber of forest polygons to burn in a 16bit internal image = %d \n\n", feat_count );

// flag to use an 8bit (rather than 16bit) internal image for polygons 

//if(feat_count < 255) flag8b=1 ;			// cancel that for the moment, modern PC have lots of memory

/* Mention memory requirements up-front */

if(flag8b)		// if less than  255 polygons, an 8bit image can be used 
	{
	//printf("\nITCPCD needs %Illd MB of memory for an internal 8bit image to burn-in polygons (i.e., faster).\n", image_size/1024/1024);
	mem_required = mem_required + image_size;
	}
	else
	{
	//printf("\nITCPCD needs %Illd MB of memory for an internal 16 bit image to burn-in polygons (i.e., faster).\n", 2*image_size/1024/1024);
	mem_required = mem_required + 2*image_size;
	}


//printf("\nProgram also needs about %Illd MB of memory for 3 internal bitmaps.\n",  3*(bmsize/1024/1024) );
/* printf("\nProgram will need about %Illd bytes of memory for 3 internal bitmaps, etc.\n", 3*bmsize ); */

mem_required = mem_required + 3*bmsize;

if (height_flag) 
	{
	//printf("Program will also need %Illd MB for the height channel (i.e., Digital Canopy Model). \n", image_size/1024/1024);
	mem_required = mem_required + image_size;
	}

//printf("\n*** IN TOTAL: Program will need about %Illd MB of memory \n", mem_required/1024/1024);

//printf("\n*** NOTE *** \nIf this is more than REASONNABLY available, \n\t please use ITCPCDB instead (slower, but uses less memory)\n");
//printf("\tAND/OR    try without a DCM (however, no height will be reported)\n");

//printf( "Press [Enter] to Continue: ");  getchar();


/* Read in height image (if needed) */

if (height_flag)
  {
  printf("\n\tReading 8bit height image (DCM) from file %s  ...\n", heightfile);

  ht_in  = open_imaFile(heightfile);

  // printf("\n\tdata_type : %d \n", data_type);

  if (data_type != CHN_8U) 
	{
	printf("\n\nInvalid channel type: only 8 bit image supported as DCM - EXITING \n\n");
	GDALClose(ht_in);
	exit(-1); 
	}

  //printf("\nMemory required for HEIGHTCH (i.e., Digital Canopy Model) : %Illd MB\n", image_size/1024/1024);

  himage = read_image(heightfile,1);

	// check that georef are same as main image

	  
// Check same image size as main image

	if ( (ht_in->GetRasterXSize() != ima_in->GetRasterXSize()) || (ht_in->GetRasterYSize() != ima_in->GetRasterYSize()) )
		{printf("\n\n PROBLEM with image SIZE compatibilty of file %s with previous image \n\n", heightfile); exit(1);}

  }



// *******************************

// Setup an internal 8 or 16 bit image in which polygons will be burned 
   


if(flag8b) 	// if less than  255 polygons, an 8bit image can be used 
	{
	//printf("\nRequesting %Illd MB for an 8bit raster image in which to burn the stand polygons ...\n", image_size/1024/1024);
	stand_ima = (uint8_t *) malloc(sizeof(char) * Pixels * Lines); 
	check_mem(stand_ima); 
	ppp8 = stand_ima;
	for(iii=0; iii<Pixels*(int64)Lines; iii++) *ppp8++ = 0; 
	//printf("\nInternal 8bit image for polygons secured and cleared to zero\n");
	/* printf("\nInternal 8bit image memory from address %Illd to %Illd \n",stand_ima,ppp8); */
	}
else
	{
	//printf("\nRequesting %Illd MB for a 16bit raster image in which to burn the stand polygons ...\n", 2*image_size/1024/1024);
	stand_image = (uint16 *) malloc(sizeof(uint16) * Pixels * Lines); 
	check_mem(stand_image); 
	ppp = stand_image;
	for(iii=0; iii< Pixels*(int64)Lines; iii++) *ppp++ = 0; 
	//printf("\nInternal 16bit image for polygons secured and cleared to zero\n");
	/* printf("\nInternal 16bit image memory from address %Illd to %Illd \n",stand_image,ppp); */
	}


/* Allocate memory for bitmap buffers and read bitmaps into buffers */


printf( "\n\t\tOpening the ISOL bitmap\n");

//isolbitbuffer = alloc_read_bmp(idb_fp, xsize, ysize, isolbit);      /* Open the ITC bitmap */

isolbitbuffer = read_bitmap(file_ITC, 1);

// check that georef are same as main image and HT image  
// ** NO**  read_bitmap() does that automatically


// ### TEST ###  - suspected ISOL invertion

/* 
	for ( i = 1 ; i <= (Lines) ; i++ )		// For compatibility with PCI(x,y), I kept image coordinates start at (1,1)
	for ( j = 1 ; j <= (Pixels) ; j++ ) 		// However, GDAL images start at zero (similar to bitmaps), so bytenum=bitnum
	  {
	  bitnum = (i-1)*(int64)Pixels + j-1 ;		// bitnum starts at zero, so does image, so bytenum=bitnum
	  flipbit(isolbitbuffer, bitnum);
	  }
 
 */

//printf( "\nCreating safety zone around bitmap\n");
safety_zone(isolbitbuffer);
/* 
printf( "\nCalculating checksum of bitmap ...\n");
checksum = 0;
for (iii = 0; iii < bmsize; iii++) checksum = checksum + isolbitbuffer[iii];
printf( "\nChecksum of ITC Bitmap : %Illd\n",checksum);

 */

//printf( "\nReserving %Illd MB of memory for another temporary bitmap\n",bmsize/1024/1024);
//tempbitbuffer = alloc_read_bmp(idb_fp, xsize, ysize, 0);  
tempbitbuffer = (unsigned char *) calloc(bmsize,1);


/* Add new ITC inventory attributes to shapes in layer */

/* 
if (!(dumpflag))
    {
    old_num_fields = GDBGetNumFields(slayer);
    printf("\nFor vector layer %d\n", polylayr);
    printf("Adding (if needed) new ITC inventory attributes to polygons (shapes)\n");
    prep_fields(slayer);
    printf("The vector layer now has %d fields. \n", GDBGetNumFields(slayer) );
    printf("The original vector layer had %d fields. \n",  old_num_fields);
    new_field_nb = GDBGetNumFields(slayer) - old_num_fields;
    }

 */


//Get bottom right geo. image coords (to double check)
    
botrightX = (transformX * Pixels) + topleftX;
botrightY = (transformY * Lines) + topleftY;
    
//printf("\n Image (botrightX, botrightY) = (%f %f)", botrightX, botrightY );
//printf("\n Image (transformX, transformY) = (%f %f)", transformX, transformY );

/* Initialize various stats gathering vectors */

polymax = feat_count;

TotalIsolArea = (int *) malloc(sizeof(int) * polymax);
check_mem(TotalIsolArea);
TotalClosure = (int *) malloc(sizeof(int) * polymax);
check_mem(TotalClosure);
IsolCount = (int *) malloc(sizeof(int) * polymax);
check_mem(IsolCount);
IsolHeight = (int *) malloc(sizeof(int) * polymax);
check_mem(IsolHeight);
IsolSQHeight = (int *) malloc(sizeof(int) * polymax);
check_mem(IsolSQHeight);
shapenum = (int *) malloc(sizeof(int) * polymax);
check_mem(shapenum);
polyarea = (int *) malloc(sizeof(int) * polymax);
check_mem(polyarea);
poly_pixcount = (int *) malloc(sizeof(int) * polymax);
check_mem(poly_pixcount);
	

for (i = 0; i < polymax; i++) 
	{ TotalIsolArea[i] = 0; TotalClosure[i] = 0; IsolCount[i] = 0; IsolHeight[i] = 0; 
	IsolSQHeight[i] = 0; shapenum[i]=0; polyarea[i]=0; poly_pixcount[i]=0; }

// Stopping program to read screen

printf("\n\n\t OK to continue(y/n)? ");
answer[0] = getc(stdin); 		// gets any answer or <CR>
if(answer[0]=='n')  exit(1);

printf("\n\n_______________________________\n");

//printf("\n\n\t Skiping lots for debuggong ... \n\n");

//goto LOOP4;			// DEBUGGING

//******************************************************************************************************

//			FIRST LOOP 

//	To get all stand polygons within the input vector layer and BURN THEM into an internal 8/16 bit image

//******************************************************************************************************

LOOP1:

numofpoly = painted = notpainted = 0;
int TVertices;

time(&rawtime);  timeinfo = localtime (&rawtime);
printf("\n\n_______________________________\n");
printf("\n\n\t *** Main loop through all polygons - %s (%s) at %s\n", PROG_NAME, VERSION, asctime(timeinfo));

printf("\n\n\t  Stand polygons are BURNED-IN (drawned and filled) to an internal 8/16 bit image ....\n\n");


//			FAST TEST
/* 
	last_feat = feat_count;
	if(last_feat > 5) last_feat = 5 ;				// for faster testing
	
	piLayer->ResetReading();			// VERY IMPORTANT (if you use GetNextFeature() 

	for (iFeat=0; iFeat<last_feat; iFeat++)
	{
	  piFeature = piLayer->GetNextFeature();
	  piGeometry = piFeature->GetGeometryRef();
	  printf("\n ** Feature %d : Geometry is : %d \n", iFeat, wkbFlatten(piGeometry->getGeometryType()));
	}
	exit(-1);		// for debugging

 */
 
//****************************************

// Getting polygon AND multipolygon vertices

// prep list of vertices for my vect2rast() - assuming island in main poly are smaller poly

	nVertex =  10000;		// arbitrary big number
	
  	pasVertices = (GDBVertex2D *)  CPLMalloc(nVertex * sizeof(GDBVertex));

	piLayer->ResetReading();			// VERY IMPORTANT (if you use GetNextFeature() ###

	
	printf("\n\n\t *************************************************************\n");
	printf("\n\t **** Looping through features for polygon vertices ...\n\n");

	TVertices = 0, iFeat=0;

	// Making  sure tempbitbuffer is zeroed before putting stuff in it

  	  for ( ii = 0 ; ii < bmsize ; ii++ ) tempbitbuffer[ii] = 0; 


for (i = 0; i < polymax; i++) 
	{TotalIsolArea[i] = 0; TotalClosure[i] = 0; IsolCount[i] = 0; IsolHeight[i] = 0; 
	IsolSQHeight[i] = 0; shapenum[i]=0; polyarea[i]=0; poly_pixcount[i]=0; }


//****************************************

// 			*****  MAIN LOOP per se  ******
	
	for (iFeat=0; iFeat<feat_count; iFeat++)		//looping through  polygon to burn into an internal  bitmap

	//for (iFeat=100; iFeat<150; iFeat++)			//looping through REDUCED SET of polygon for faster TESTING
	//for (iFeat=0; iFeat<5; iFeat++)
	//for (iFeat=0; iFeat<50; iFeat++)			
	{
	//piFeature = piLayer->GetNextFeature();				// ####
	piFeature = piLayer->GetFeature(iFeat);
	
	
	Poly_Outside = 0;						// assume that polygon is inside the image area (vect2rast may say otherwise)
	shapenum[iFeat]= iFeat;


//			Bugs in  PRF dataset (PRF_For_Inv_2007.shp) use for testing

    //if (iFeat == 540 ) goto CONT_LOOP;			// skip that one for testing purposes (buggy in this PRF dataset)
 	//if (iFeat == 603 ) goto CONT_LOOP;			// skip that one for testing purposes (buggy in this PRF dataset)
	//if (iFeat == 10) goto CONT_LOOP;
	//if (iFeat == 73) goto CONT_LOOP;
	
	
	
	  //printf("\n* For feature %d \n", iFeat);

	  TVertices = 0;	// reset total no. vertices of shape (this is to check that all were considered)
	
	  piGeometry = piFeature->GetGeometryRef();

	 // printf("Geometry is of type : %d \n", piGeometry->getGeometryType());		// ###


// #### For polygons

	  if ( piGeometry != NULL && wkbFlatten(piGeometry->getGeometryType()) == wkbPolygon )
	    {
	    OGRPolygon *piPolygon = (OGRPolygon *)piGeometry;

	    Polygon.PolygonsOfFeature.resize(1);
		
		NumberOfInnerRings = piPolygon->getNumInteriorRings();
		OGRLinearRing *piExteriorRing = piPolygon->getExteriorRing();
		if ( NumberOfInnerRings > 0 )
			printf("Polygon %d - Number of inner rings = %d \n", iFeat, NumberOfInnerRings);

		Polygon.PolygonsOfFeature.at(0).Polygon.resize(NumberOfInnerRings+1);
		Polygon.PolygonsOfFeature.at(0).Polygon.at(0).IsClockwised = piExteriorRing ->isClockwise();

		NumberOfExteriorRingVertices = piExteriorRing->getNumPoints();
		Polygon.PolygonsOfFeature.at(0).Polygon.at(0).RingString.resize(NumberOfExteriorRingVertices);
		//printf("Polygon %d - NumberOfExteriorRingVertices = %d \n", iFeat, NumberOfExteriorRingVertices);
		
		
		//printf("\ntransformY = %5.1f , ypixsz  = %5.1f \n\n", transformY, ypixsz);
		

// FIRST -- Get ALL Exterior Ring Vertices

            for ( k = 0; k < NumberOfExteriorRingVertices; k++ )
               {
               piExteriorRing->getPoint(k,&ptTemp);
               MyPoint2D pt;
               pt.dX = ptTemp.getX();
               pt.dY = ptTemp.getY();		  
               Polygon.PolygonsOfFeature.at(0).Polygon.at(0).RingString.at(k) = pt;
               //pasVertices[k].x = pt.dX ;
               //pasVertices[k].y = pt.dY ;
			   	pasVertices[k].x = ( ptTemp.getX()- topleftX +1 ) / xpixsz;		// convert UTM to image coordinates(based 1,1)
				pasVertices[k].y = ( topleftY - ptTemp.getY() +1 ) / ypixsz;
				//pasVertices[k].y = ( ptTemp.getY() - topleftY  ) / transformY; // more geo correct (two negatives)
               }

// Using my vector to raster (to bitmap) painting routine

	   nVertex = k;

	   //printf("\n* Painting polygon %d as raster in memory (nVertex=%d) \n", iFeat, nVertex);		// for debugging

	// Debugging Print

/* 	  for (kk=0; kk<nVertex; kk++)
		{
		if (kk<10)  printf(" %d = %5.1f,%5.1f ", kk+1, pasVertices[kk].x, pasVertices[kk].y);
		if ( ((kk+1)/5)*5 == (kk+1) ) fprintf(stdout,"\n");
		}
	  fprintf(stdout,"\n");
 */
	//	exit(-1);			// for debugging
   
	  	vect2rast_g(nVertex, pasVertices, tempbitbuffer, xsize, iFeat, set_flag=1);   
	   
	   polyarea[iFeat] = fill_count;		//result from vect2rast_g()
	
// debugging 	
	    //if (iFeat == 310 )   printf("\nFor polygon %d,  vect2rast() pixel count= %d \n", iFeat, polyarea[iFeat]);
		

//	   if (!Poly_Outside)  printf("\n Polygons %d  - Poly Area: %d \n", iFeat, polyarea[iFeat]); 	// For debugging 
 
 	   //if (polyarea[iFeat] == 0) Poly_Out_Count++;   		//using zero polygon area as another criteria (in but zero)
	   

	   
	   if (Poly_Outside)    // using a flag from vect2rast_g() - Polygon is outside the image
	   { 
		Poly_Out_Count++; 
		//polyarea[iFeat]=0;  
	    printf("\n ****** Polygons %d considered outside the image area\n", iFeat);   
		goto CONT_LOOP;		// if this Ploygon was outside the image, dont do anything with  it 
	   }

	   TVertices = TVertices  + nVertex ;
	 


	//printf("\nFor polygon %d - Total number of pixels indide its main area: %d \n", iFeat, polyarea[iFeat] );	 
	   
	//goto CONT_LOOP;		// for testing -- just use the outside of simple polygon
	   
	   

// Get ALL Interior Ring Vertices AND remove that area

	//if( NumberOfInnerRings > 0) printf("Erasing %d inner rings\n", NumberOfInnerRings);

         for ( h = 1; h <= NumberOfInnerRings; h++ )
           {
               OGRLinearRing *piInteriorRing = piPolygon->getInteriorRing(h-1);

               Polygon.PolygonsOfFeature.at(0).Polygon.at(h).IsClockwised = piInteriorRing->isClockwise();
               NumberOfInteriorRingVertices = piInteriorRing->getNumPoints();
				//printf("NumberOfInteriorRingVertices = %d \n", NumberOfInteriorRingVertices);

               Polygon.PolygonsOfFeature.at(0).Polygon.at(h).RingString.resize(NumberOfInteriorRingVertices);

               for ( k = 0; k < NumberOfInteriorRingVertices; k++ )
                 {
                 piInteriorRing ->getPoint(k,&ptTemp);
                 MyPoint2D pt;
                 pt.dX = ptTemp.getX();
                 pt.dY = ptTemp.getY();
                 Polygon.PolygonsOfFeature.at(0).Polygon.at(h).RingString.at(k) = pt;
			   	 pasVertices[k].x = ( ptTemp.getX()- topleftX  +1 ) / xpixsz;
				 pasVertices[k].y = ( topleftY - ptTemp.getY()  +1  ) / ypixsz;
                 }

// Remove that area using my vector to bitmap painting routine with set_flag=0

		//if (iFeat == 310) fprintf(stdout,"\n\n* Un-painting area of feature %d in raster in memory (nVertex=%d) \n", iFeat, k);
			
			nVertex = k;			
			vect2rast_g(nVertex, pasVertices, tempbitbuffer, xsize, iFeat, set_flag=0);

			TVertices = TVertices  + nVertex ;
	      //polyarea[iFeat] = polyarea[iFeat] - fill_count;
			polyarea[iFeat] = polyarea[iFeat] + fill_count;  // fill_count should already be a negative value out of vect2rast()
 
// debugging 	
			//if (iFeat == 310 )   printf("\nFor polygon %d, vect2rast() pixel count now = %d \n", iFeat, polyarea[iFeat]);

			}	// burn (actually unburn) each inner ring separately


	//printf("For feature %d total number of vertices considered %d \n", iFeat, TVertices);

	//printf("\nFor feature %d - total number of pixels (out-in) for area : %d \n", iFeat, polyarea[iFeat] );
	
	
	   }		// end of if polgygon


	//goto CONT_LOOP;	// for testing -- just use outside-inside of simple polygon (no multipolygon)


// ####  For MULTIPOLYGON (for multiple geometries)


	  if ( piGeometry != NULL && 
		( wkbFlatten(piGeometry->getGeometryType()) == wkbMultiPolygon ) )
	    {
	    OGRMultiPolygon *piMultiPolygon = (OGRMultiPolygon *)piGeometry;
            int NumberOfGeometries = piMultiPolygon ->getNumGeometries();
	    Polygon.PolygonsOfFeature.resize(NumberOfGeometries);


           for ( j = 0; j < NumberOfGeometries; j++ )
           {
               OGRGeometry *piPolygonGeometry = piMultiPolygon ->getGeometryRef(j);
               OGRPolygon *piPolygon = ( OGRPolygon * )piPolygonGeometry;
               NumberOfInnerRings = piPolygon ->getNumInteriorRings();
               OGRLinearRing *piExteriorRing = piPolygon ->getExteriorRing();

               Polygon.PolygonsOfFeature.at(j).Polygon.resize(NumberOfInnerRings+1);
	           printf("MultiPoly %d - Number of inner rings = %d \n", iFeat, NumberOfInnerRings);

               Polygon.PolygonsOfFeature.at(j).Polygon.at(0).IsClockwised = piExteriorRing ->isClockwise();
               NumberOfExteriorRingVertices = piExteriorRing ->getNumPoints();
               Polygon.PolygonsOfFeature.at(j).Polygon.at(0).RingString.resize(NumberOfExteriorRingVertices);
               printf("MultiPoly %d - NumberOfExteriorRingVertices = %d \n", iFeat, NumberOfExteriorRingVertices);

               for ( k = 0; k < NumberOfExteriorRingVertices; k++ )
                 {
                 piExteriorRing->getPoint(k,&ptTemp);
                 MyPoint2D pt;
                 pt.dX = ptTemp.getX();
                 pt.dY = ptTemp.getY();
                 Polygon.PolygonsOfFeature.at(j).Polygon.at(0).RingString.at(k) = pt;
                //pasVertices[k].x = pt.dX ;
                //pasVertices[k].y = pt.dY ;
			   	pasVertices[k].x = ( ptTemp.getX()- topleftX  +1 ) / xpixsz;
				pasVertices[k].y = ( topleftY - ptTemp.getY()  +1 )  / ypixsz;
                 }

	       nVertex = k;
		   
	       //printf("\n* Painting polygon %d as raster in memory (nVertex=%d) \n", iFeat, nVertex);		// ###
		   
	       vect2rast_g(nVertex, pasVertices, tempbitbuffer, xsize, iFeat, set_flag=1);
		   
	       TVertices = TVertices  + nVertex ;
		   
			polyarea[iFeat] = polyarea[iFeat] + fill_count;
 
// degugging 	
			//if (iFeat == 310 )   printf("\nFor MultiPolygon %d,  vect2rast() pixel count= %d \n", iFeat, polyarea[iFeat]);
	   
// get ALL interior ring vertices (to substrct from area)

	      if( NumberOfInnerRings > 0) printf("Erasing %d inner rings\n",NumberOfInnerRings);

              for ( h = 1; h <= NumberOfInnerRings; h++ )
               {
                   OGRLinearRing *piInteriorRing = piPolygon ->getInteriorRing(h-1);
                   Polygon.PolygonsOfFeature.at(j).Polygon.at(h).IsClockwised = piInteriorRing ->isClockwise();
                   NumberOfInteriorRingVertices = piInteriorRing ->getNumPoints();
                   Polygon.PolygonsOfFeature.at(j).Polygon.at(h).RingString.resize(NumberOfInteriorRingVertices);
	           //printf("NumberOfInteriorRingVertices = %d \n", NumberOfInteriorRingVertices);

                   for ( k = 0; k < NumberOfInteriorRingVertices; k++ )
                     {
                     piInteriorRing->getPoint(k,&ptTemp);
                     MyPoint2D pt;
                     pt.dX = ptTemp.getX();
                     pt.dY = ptTemp.getY();
                     Polygon.PolygonsOfFeature.at(j).Polygon.at(h).RingString.at(k) = pt;
                    //pasVertices[k].x = pt.dX ;
               		//pasVertices[k].y = pt.dY ;
			   		pasVertices[k].x = ( ptTemp.getX()- topleftX + 1 ) / xpixsz;
					pasVertices[k].y = ( topleftY - ptTemp.getY()  +1 ) / ypixsz;
                     }

			  nVertex = k;
			  //fprintf(stdout,"\n\n* Un-painting feature %d in raster in memory (nVertex=%d) \n", iFeat, nVertex);
			  
			  vect2rast_g(nVertex, pasVertices, tempbitbuffer, xsize, iFeat, set_flag=0);
			  TVertices = TVertices  + nVertex ;
			  //polyarea[iFeat] = polyarea[iFeat] - fill_count;
			  polyarea[iFeat] = polyarea[iFeat] + fill_count;  // fill_count should already be a negative value out of vect2rast()
 
// degugging 	
			//if (iFeat == 310 )   printf("\nFor MultiPolygon %d,  vect2rast() pixel count= %d \n", iFeat, polyarea[iFeat]);

			  }	// end of number of inner rings

	    }		//  end of number of Geometries

             //PolygonLayer.push_back(Polygon);

	     printf("For MULTIPOLYGON %d total number of vertices considered %d \n", iFeat, TVertices);

	    }		//end of multipolygon 


//	BURN (paint) into "stand_image" from tempbitbuffer 
//	** Because vect2rast_g() actually only creates a bitmap (and NOT a raster image)


	for (i = 1; i <= Lines; i++)
		for ( j = 1 ; j <= Pixels ; j++ )
	{ 
	bitnum = (i-1)*(int64)Pixels + j-1 ; 		// here bytenum = bitnum
	if(testbit(tempbitbuffer, bitnum))	stand_image[bitnum] = (uint16) (iFeat +1);	 // paint as feature (poly) number
			// in that image we use (poly +1 )
	}



	//if (!Poly_Outside)  printf("\n Polygons %d  - Poly Area: %d \n", iFeat, polyarea[iFeat]); 	// For debugging 


 // for debugging
 /* 
	for (i = 1; i <= Lines; i++)
		for ( j = 1 ; j <= Pixels ; j++ )			// rescan the image 
	{ 
	bitnum = (i-1)*(int64)Pixels + j-1 ; 
	
	if(stand_image[bitnum] != 0)  // for debugging
	  {
	  printf("Some  poly %d image data: \n", iFeat); 
	  for (kk=1; kk<=10; kk++) printf(" %d ", stand_image[bitnum+kk]);  
	  }
	break;
	}
 */

CONT_LOOP:

//	if (iFeat < 10) printf("\t %d polygons done so far ... \r", iFeat);					// display progress
//	if( (iFeat/10)*10 == iFeat) printf("\t %d polygons done so far ... \r", iFeat);		// display progress
	if( (iFeat/10)*10 == iFeat) printf("\t %d polygons burned so far ... \n", iFeat);		// display progress


if( polyarea[iFeat] > 0 )
  {
   //printf("\nvect2ras_g() burned %d pixels for polygon %d\n", polyarea[iFeat] , iFeat);
   //printf("Polygon %d area is : %.2f (ha) \n", iFeat, polyarea[iFeat]*xpixsz*ypixsz/100/100);     
  }
   
   
// Prep tempbitbuffer for next iteration

  	  for ( ii = 0 ; ii < bmsize ; ii++ ) tempbitbuffer[ii] = 0; 
	  

	}			// END  of feature loop -- finished looping through all polygons (features)
	
	

	printf("\n\n\t** Number of polygons painted into internal forest stand image  : %d\n", iFeat);
	printf("\t** Number of polygon that are outside the image area : %d\n", Poly_Out_Count);
	
	time(&rawtime);  timeinfo = localtime (&rawtime);
	printf("\n\n_______________________________\n");

	 printf("\n\t OK to continue(y/n)? ");
	 answer[0] = getc(stdin); 		// gets any answer or <CR>
	 if(answer[0]=='n')  exit(1);


	//exit(-1); 			// for debugging

//******************************************************************************************************

//				SECOND LOOP 

//		Doing generic ITC count within  polygons 

//******************************************************************************************************

LOOP2:



printf("\n\n\t*** Doing generic ITC count in polygons - %s (%s) at %s\n\n", PROG_NAME, VERSION, asctime(timeinfo));

//timeinfo0 = localtime(&rawtime0);
//printf("\nHad started at %s\n\n", asctime(timeinfo0));
	
	
/* Scan all ITCs (in the generic ITC bitmap) that are spanning the stand (an internal 8/18 bit image) */
/* Generate ITC count, pixel count (for closure), ITC stems, ITC heights, ... for that stand */

memcpy(tempbitbuffer, isolbitbuffer, bmsize);		// full ISOL bitmap to temp bitmap 

checksum = 0;
for (iii = 0; iii < bmsize; iii++) checksum = checksum + tempbitbuffer[iii];
printf( "\nChecksum of temp ITC Bitmap : %Illd\n",checksum);


count_isol(xsize, ysize, tempbitbuffer);		// doing generic ITC count in polygons


if (details == 1) goto Output;		/* bypass second loop if class info not required */



// To allow user to read info
 
	  printf("\n\t OK to continue(y/n)? ");
	  answer[0] = getc(stdin); 		// gets any answer or <CR>
	  if(answer[0]=='n')  exit(1);
	  
printf("\n\n_______________________________\n");
	  
/**************************************************************************************/

//			THIRD LOOP 

//	To deal with EACH CLASS bitmap and store species info for each stand in the list

/*********************************************************************************************/

LOOP3:

time(&rawtime);  timeinfo = localtime (&rawtime);
printf("\n\n_______________________________\n");
printf("\n\n\t*** Main loop through all the classes -  %s (%s) at %s\n", PROG_NAME, VERSION,  asctime(timeinfo));
printf( "\nProcessing EACH CLASS (species) within each stand polygon *** \n");


/* Generate information per class - loop for each class bitmap */

class_list = new_class_info_node();		/* empty node to start the list */


for (iclass = 0; iclass < numclasses; iclass++) 
  {
  //classbuffer = alloc_read_bmp(idb_fp, xsize, ysize, classbit[iclass]);			/* read in class bitmap */

  classbuffer = read_bitmap(file_CLASS[iclass], 1);

  time(&rawtime);  timeinfo = localtime (&rawtime);
  printf("\nRe-processing all polygons (internal image) for class %d at %s\n", classbit[iclass], asctime(timeinfo));

  new_class = new_class_info_node();			/* prepare info node for this class, it convers all polygon */	
  for (poly = 0; poly < polymax; poly++)	 
		new_class->BitNumber[poly] = classbit[iclass];
  add_class_list(class_list, new_class);			/* link to previous node */


  memcpy(tempbitbuffer, isolbitbuffer, bmsize); 		// copy isol bitmap in temp

  /* For specified class, process stand  and gather information on trees */
	  	  	
  scan_for_trees_xxx(xsize, ysize, tempbitbuffer, classbuffer, new_class);
	
  printf("\n\n\tTotal number of ITCs relative to class %d : %d ITCs\n", iclass+1, tot_class_ITCcount);
  Class_ITCcount[iclass] = tot_class_ITCcount ;		// store for future reference
	  
  /* free memory, because via alloc_read_bmp(), the next iteration will again call malloc() 
  and we want to economize and dont need more than one class bitmap resident at the time */

  free(classbuffer); 

  //GDBSync(idb_fp);	/* Supposed to clear the cache and move all info to vector layer in pix file */

  }		/* end of for each class */


	time(&rawtime);  timeinfo = localtime (&rawtime);
	printf("\n\n_______________________________\n");
	
// To allow user to read info
 
	  printf("\n\t OK to continue(y/n)? ");
	  answer[0] = getc(stdin); 		// gets any answer or <CR>
	  if(answer[0]=='n')  exit(1);
	  

/*****************************************************/

// OUTPUT - If PLAIN output file specified, dump info to that file 

/*****************************************************/


Output:

	printf("\n\n_______________________________\n");

	if (dumpflag)
	  {
	  printf( "\n *** Storing polygon information in plain text output file: %s *** \n", pcdfile);
	  //printf( "\t N.B.: Input Vector Layer will not be modified.\n");
	  		
	  /* print information about image, bitmaps, ... at top of output file */
	  
	  time(&rawtime);  timeinfo = localtime(&rawtime);
	  fprintf(odb_fp,"Polygon Content Description from ITCPCD_G(%s) at %s\n\n", VERSION, asctime(timeinfo));
	  
	  fprintf(odb_fp,"From base image file %s \n", argv[1]);
	  fprintf(odb_fp,"Pixels across: %d  Lines:  %d  Channels: %d\n",Pixels,Lines,Channels);
	  fprintf(odb_fp,"Pixel size is %g x %g %s ", xpixsz, ypixsz, pixunit);
	  imgarea =  (Pixels*xpixsz) * (Lines*ypixsz) /100 /100;
	  fprintf(odb_fp,"\tArea size is %g ha\n", imgarea);
	  
	  fprintf(odb_fp,"\nImage geographic referencing information: %s \n", geosys);
	  fprintf(odb_fp,"(Offset, multiplier) relative to pixel & line coord: \n (%f, %f) (%f, %f) \n\n", 
					topleftX, xpixsz, topleftY, ypixsz);
					
	  fprintf(odb_fp,"Individual Tree Crown (ITC) bitmap used: %s \n", file_ITC);
	  fprintf(odb_fp,"Polygon vector layer used: %s \n", polygonfile);
	  fprintf(odb_fp,"Height channel (DCM) used: %s \n", heightfile);
	  fprintf(odb_fp,"Provincial jurisdition:  %3s\n", province);    
	  fprintf(odb_fp,"Level of details requested: %s \n", odetails);
	  fprintf(odb_fp,"The present information is in file: %s \n", pcdfile);
	  fprintf(odb_fp,"-------------------------------------------\n\n");
	  fprintf(odb_fp,"Total number of ITCs within considered area : %lld ITCs\n", ITC_Total_Count);
	  fprintf(odb_fp,"-------------------------------------------\n\n");	  
	  


// Image wide information for each class 

	  if (details >= 1)
	  {

	  for (iclass = 0; iclass < numclasses; iclass++) 
	    {
	    fprintf(odb_fp,"Class %d - File: %s \n", iclass+1, file_CLASS[iclass]);
	    //GDBSegDescIO(idb_fp, GDB_READ,SEG_BIT,classbit[iclass], seg_description);
	    fprintf(odb_fp,"Description: %s\n", seg_description);	    
	    //seg_history = GDBReadHistory(idb_fp, SEG_BIT, classbit[iclass],1);
	    fprintf(odb_fp,"Hist.: %s\n", seg_history);
		
		fprintf(odb_fp,"Total number of ITCs relative to class %d : %d ITCs\n", iclass+1, Class_ITCcount[iclass]);
		
		fprintf(odb_fp,"-------------------------------------------\n");
	    }

	  view_results(idb_fp, odb_fp, class_list, classbit);
	  }


	
	printf("\n--------------------------------------------------\n");	
	printf("\n** ITC information was moved to the output TXT file <<%s>> as required.\n\n", pcdfile);

	// Show finishing time in plain text file	
	
	time (&rawtime);  timeinfo = localtime (&rawtime);
	fprintf(odb_fp,"\n\n_______________________________\n");
	fprintf(odb_fp,"\n%s (%s) finished at %s\n\n", PROG_NAME, VERSION, asctime(timeinfo));

	fprintf(odb_fp,"\n\n Forest stand polygons originally from %s \n\n",  polygonfile);
	
	fclose(odb_fp);
	}



/*****************************************************/
	
// 		Write to output(NEW) shape file
		
/*****************************************************/			
	
LOOP4:
	
	if (!dumpflag) 		// Write to output (new) shape file
	  {
//	  strcpy(basefname, basefilname);			//construct file name
//	  strcat(basefname, "_PCD_Out.shp"); 
//	  strcpy(pcdfile,basefname);			


	  printf( "\n\n Will be reporting ITC inventory information in output shape file <<%s>> ...\n\n", pcdfile);
	  printf("\n--------------------------------------------------\n");

	  view_results(idb_fp, odb_fp, class_list, classbit);	

	  printf("\n--------------------------------------------------\n");	
	  printf("\n\t** All polygons and their attributes were moved to <<%s>>\n\n", pcdfile);
		
	  char   ShpFileDesc[80];
	  strcpy(ShpFileDesc, piLayer->GetDescription());
	  strcat(ShpFileDesc, " with ITC Info");

	  printf("\tOutput Shape file description : %s \n", ShpFileDesc);
	
	  poLayer->SetDescription(ShpFileDesc);

	  GDALClose( poDS );		// Close that data set (shp file)
	 }	
	
	
	
// Show finishing time (and staring time) to time prog.	
	
	time (&rawtime);  timeinfo = localtime (&rawtime);
	printf("\n\n_______________________________\n");
	printf("\n%s (%s) finished at %s\n\n", PROG_NAME, VERSION, asctime(timeinfo));

	
	timeinfo0 = localtime(&rawtime0);
	//printf("\n\tN.B.:  Program had started at %s\n\n", asctime(timeinfo0));



// For people using this program via ArcGIS, give then some time to examine the results (before disappearing)

if  (EQUALN("ArcGIS ",argv[argc-1],3) )
	{
	printf("\n\n######\n");
	printf("\n Type anything to make this detailed window disappear and terminate %s ",PROG_NAME);
	answer[0] = getc(stdin); 		// gets any answer or <CR>
    }
	

	exit(0);				// exit properly 

}		// END OF MAIN PROGRAM 


/***************************************************************************************/
/***************************************************************************************/

/* FUNCTIONS */



//***************************************************************************************

// Looping for all of the ISOLs in the generic ITC bitmap (and get even unclassified ITCs) 

// For each polygon, count pixels (for closure), ITC stems, ITC heights, ... from the ITC bitmap 

//***************************************************************************************


void count_isol(int xsize, int ysize, unsigned char *full_bitmap)
{
	int 	x, y, i;
	int64	bitnum;
	int 	poly_to_incr, top_poly_count;
 

	for (y = 1; y <= ysize; y++)			// scan the image area
	for (x = 1; x <= xsize; x++) 
	  {
	  bitnum = ((y - 1)*(int64)xsize ) + x-1;
	
	  if (testbit(full_bitmap, bitnum))	/* Check if we have a possible ITC OR even a treetop */
	    {
	    counter = max_val = 0;
	    for (i = 0; i < polymax; i++) poly_pixcount[i] = 0;		// for ITC contrib to each polygon 

	    fill_isol(x, y, xsize, full_bitmap, poly_pixcount);
				
	    if ((counter >= MIN_TREE_PIXELS)) 		// Check if a valid tree (or TT) -- counter is from the fill_isol()
		{
		top_poly_count = 0;
		poly_to_incr = -99;

		for (i = 0; i < polymax; i++)		/* find polygon ITC is most connected with (to count in)*/
		  {
		  TotalClosure[i] += poly_pixcount[i];		/* for crown closure in each polygon */

		  if (poly_pixcount[i] > top_poly_count)	/* find most connected polygon */
		    {
		    poly_to_incr = i;
		    top_poly_count = poly_pixcount[i];	
		    }

		  }
		  
		ITC_Total_Count++;

		if ( (poly_to_incr >= 0) && (poly_pixcount[poly_to_incr] > 0.5 * counter) )	/* poly not -99 - ITC GT 50% in */
		  {
		  IsolCount[poly_to_incr]++;				/* add this ITC to count for this most connected polygon */
		  IsolHeight[poly_to_incr] += max_val;			/* add this ITC to total height for this polygon */
		  IsolSQHeight[poly_to_incr] += (max_val * max_val);	/* add this ITC to total squared height for this polygon */
	  	  TotalIsolArea[poly_to_incr] += counter;		/* for average crown area use "full" crown */
		  
		  
		  }
		}	/* end of if valid tree */
		  	  
	    }	/* end of if tree */

	  if ( (x==1) && (y == (y/100)*100 ) ) printf( "  %d line done. %Illd ITCs found so far\r", y, ITC_Total_Count);

	  /* if ( (y >= 21100) ) printf( "Line = %d, Pixel = %d\n",y,x); */

	  }		// End of bitmap scan 

	//free(poly_pixcount2); 
	
		printf( "\n \n 	%Illd generic ITCs found in the image area\n", ITC_Total_Count);
	
}			// End of count_isol()

/***************************************************************************************/

/* This function scans the iclass bitmap to find trees.  The iclass is scanned, looking for
   a pixel.  If a pixel is found, its position is checked to make sure that it falls
   within a tree.  If it is, then a tree has been found and fill() is called to gather
   pixel count of the tree.  The information is stored in a tree node for that bitmap.
   Once a tree has been processed, it will not be processed again.  So, two or more
   pixels in the same tree will only cause the generation of a single set of values
   for that tree.
*/

void scan_for_trees_xxx(int xsize, int ysize, unsigned char *full_bitmap, unsigned char *mask_data, class_info_node *class_list)
{
	int	x, y, i;
	int	byte, bit;
	int64	bitnum;
	int	 poly_to_incr, top_poly_count;
	
	tot_class_ITCcount = 0;				// total ITCs for  a given class 
	
	for (y = 1; y <= ysize; y++)
	for (x = 1; x <= xsize; x++)
	  {
	  bitnum = ((y - 1)*(int64)xsize) + x-1;

	  if ( testbit(mask_data, bitnum)  && testbit(full_bitmap, bitnum) )
	    {
	    counter = 0; max_val = 0;
		
	    for (i = 0; i < polymax; i++) poly_pixcount[i] = 0;
		
	    fill_isol(x, y, xsize, full_bitmap, poly_pixcount);
				
	    if ((counter >= MIN_TREE_PIXELS)) 		/* Check if a good tree */
		{
		top_poly_count = 0;
		poly_to_incr = -99;

		for (i = 0; i < polymax; i++)		/* find polygon ITC is most connected with (to count in)*/
		  {

		  class_list->ClassClosure[i] += poly_pixcount[i];

		  if (poly_pixcount[i] > top_poly_count)
		    {
		    poly_to_incr = i;
		    top_poly_count = poly_pixcount[i];	
		    }

		  }
		  
	    tot_class_ITCcount++;				// total ITCs for a given class
	  
		if ( (poly_to_incr >= 0) && (poly_pixcount[poly_to_incr] > 0.5 * counter) )	/* poly not -99 - ITC GT 50% in */
		  {
		  class_list->ClassCount[poly_to_incr]++;
		  class_list->ClassHeight[poly_to_incr]+= max_val;
		  class_list->ClassCrownArea[poly_to_incr]+= counter;
	
		  /* class_list->ClassSQHeight[poly_to_incr]+= max_val * max_val; */
		  /* class_list->NotCounted[poly_to_incr] += (counter - poly_pixcount3[poly_to_incr]); */
		  }

		}
	    }		
		
	  if ( (x==1) && (y == (y/100)*100 ) ) printf( "%d line done...\r", y);  
	  //if ( (x==1) && (y == (y/100)*100 ) ) printf( "%d line done. %d Class ITCs found so far\r", y, tot_class_ITCcount);
	  
	  }	// 	End of bitmap scan -- 
	
	//free(poly_pixcount3); 			//End of scan_for_trees_xxx()
}




/***************************************************************************************/


/* View (or write to plain text file) information on polygon content 
   * OR * add information to polygons (shapes in vector layer */
   
/***************************************************************************************/

void view_results(FILE *idb_fp, FILE *odb_fp, class_info_node *class_list, int *classbit)
{
	class_info_node      *classN, *sign_class;
	float                avgcrownsz, avgcrownht, avgcrowndiam, stemsperhec, ClsPercntArea, ClsPercntStms;
	float 		     TotClosure, TotStmsPerHec, TotAvCrownA, TotAvHeight, TotSDHeight;
	int 		     i, poly=0, shapecount=0;
	int		     tempval, iclass, repclasses;
	int 	last_shape;
	int 	redone_flag=0;

	printf("\n\tSummarizing ITC information for each input forest polygon ... \n\n");

    if (!dumpflag) goto Shp_File;


/* If Output file was requested by user, dump results to the file and do not modify the input layer */

    if (dumpflag)
       {
		printf("\n\tPolygon content will be reported to plain txt file ... \n\n"); 
		if(details == 1) 
	     fprintf(odb_fp,"\nPoly # ShapeID Area(ha) ITCs  Closure Density AveCrArea AveHeight HeightSD\n\n");
	   }
	   
	poly = 0;
	numofpoly = feat_count;			// number of polygons in input shp file


	//if(details > 1)	printf("\n\tSummarizing ITC information species content ... \n\n"); 
	

	//while (poly < numofpoly)			/* for every polygon considered in the image (numofpoly) */
	
	for (poly=0; poly < numofpoly; poly++)
    //for (poly=600; poly<650; poly++)	// ### looping through REDUCED SET of polygon for faster TESTING
	  {	



		
	  if( polyarea[poly] == 0 ) goto nextpoly;		// skip polygon outside image or empty  ###

	  
	  //TotClosure = ( (float)TotalClosure[poly] / (float)polyarea[poly] ) * 100.0 ;
	  
	  if( polyarea[poly] > 0 )  
		{
	    TotClosure =  (float)TotalClosure[poly] / (float)polyarea[poly]  * 100.0 ;	
	    TotStmsPerHec = ((float)IsolCount[poly] * 10000.0) / ((float)polyarea[poly] * xpixsz * ypixsz);
	    TotAvCrownA = ( (float)TotalIsolArea[poly] * xpixsz * ypixsz ) / (float)IsolCount[poly] ;
	    TotAvHeight =	(float)IsolHeight[poly] / (float)IsolCount[poly] ;
	    TotSDHeight =	sqrt (  ( (float)IsolSQHeight[poly] - 
							(float)IsolHeight[poly]*(float)IsolHeight[poly]/(float)IsolCount[poly] ) 
									/ ((float)IsolCount[poly]-1) ) ;
		}
		  
		  
	  if ((dumpflag) && (details == 1))
	    {	
	    fprintf(odb_fp,"%4d %7d %8.2f %5d %7d %7d %9.1f %9.1f %9.1f \n", poly, shapenum[poly], polyarea[poly]*xpixsz*ypixsz/10000, 
			IsolCount[poly], (int)TotClosure, (int)TotStmsPerHec, TotAvCrownA, TotAvHeight, TotSDHeight);
	    goto nextpoly;
	    }
	  
// SPECIES (class)  info within polygon	  
		
	  /* 	Detailed information (MEDIUM or HIGH are same here) 
			Dump all species info in CLASSBIT order */ 
	  if (dumpflag)
	  {
	    fprintf(odb_fp,"\nPolygon# %d  (ShapeID # %d)\n",poly, shapenum[poly]);		
	    fprintf(odb_fp,"Area(ha): %6.2f \tITCs: %d \tClosure: %d%% \tDensity: %d \n\n", 
	        fabs(polyarea[poly]*xpixsz*ypixsz/10000), IsolCount[poly], (int)TotClosure, (int)TotStmsPerHec);		 
	    fprintf(odb_fp,"Class Bitmap ITCs  %%stems  %%closure Density Mean_Diam Mean_Height \n");
	  }
	  
	  i = 0;
	  class_list_beg = class_list->link;			/* beginning of the linked list */
	  classN = class_list_beg;
	  

	  while (classN) 		/* for each class (i.e., go down the list until no more items */
	    {
	    /* fprintf(odb_fp,"Class %d - Bitmap #%d \n", i+1, classbit[i]); */

	    avgcrownsz = ((float) classN->ClassCrownArea[poly]) / ((float) classN->ClassCount[poly]);
	    stemsperhec = (((float) classN->ClassCount[poly]) * 10000.0) / (polyarea[poly] * xpixsz * ypixsz);
	    ClsPercntStms = (((float) classN->ClassCount[poly]) / ((float) IsolCount[poly])) * 100.0;
 	    avgcrownht = ((float) classN->ClassHeight[poly]) / ((float) classN->ClassCount[poly]);
	    avgcrowndiam = sqrt(((avgcrownsz*xpixsz*ypixsz*4)/3.14159));

	    ClsPercntArea = (((float) classN->ClassClosure[poly]) / ((float) TotalClosure[poly])) * 100.0;

	    if (strncmp(province,"QUE",3) == 0)	    
			ClsPercntArea = (((float) classN->ClassClosure[poly]) / ((float) polyarea[poly])) * 100.0;
	    
	    if (strncmp(province,"ONT",3) == 0)
	       ClsPercntArea = (((float) classN->ClassClosure[poly]) / ((float) TotalClosure[poly])) * 100.0; 

		if(classN->ClassCount[poly] == 0) 
			{ClsPercntStms=0; ClsPercntArea=0; stemsperhec=0; avgcrowndiam=0; avgcrownht=0;}


	    if (dumpflag) fprintf(odb_fp,"%3d %7d %4d %7.1f  %9.1f %7.1f %6.1f %9.1f\n",
			i+1, classN->BitNumber[poly],classN->ClassCount[poly], ClsPercntStms, ClsPercntArea, stemsperhec, avgcrowndiam, avgcrownht);


	    classN = classN->link;
	    i++;
	    }		/* end of for each class */
		  
nextpoly:
	 
			printf("\t\tPolygons %d done  \r", poly+1); 	
 
			if ((poly+1) == numofpoly) printf("\t\t %d Polygons done  \n", poly+1); 
	//poly++;

	    }		/* end of for each polygon */
	  


//*********************************************************************

//	 Report to a shape file rather than plain text file

//*********************************************************************

Shp_File:

    if (!(dumpflag))	// Dealing with shp files - First copy(features and fields)then add new fields to it
		
		{
			

		printf("\n--------------------------------------------------\n");	
		printf("\n\tReporting polygon content to output SHP file (N.B.: not the plain txt file) : <<%s>> \n\n", pcdfile); 			
		printf("\tFIRST, by creating it and copying existing Fields and Features (Polygons) to it ... \n");		
		
		Copy_Shape_File(polygonfile, pcdfile);
		
		printf("\n--------------------------------------------------\n");
		printf("\n\tPreparing new fields to report PCD info to output SHP file... \n\n");
		
		Prep_New_Fields(poLayer);	


// Stopping program to read screen (or for debugging)

		printf("\n\n\t OK to continue(y/n)? ");
		answer[0] = getc(stdin); 		// gets any answer or <CR>
		if(answer[0]=='n')  exit(1);


		printf("\n--------------------------------------------------\n");	
		printf("\n\tReporting ITCPCD assessment to output SHP file ... \n\n");
		
		Add_New_Info(poLayer, class_list);		      

		}

     printf("\n\n\tITC INFO was written for each polygon found to be inside the image \n");
			
}		/* end of function view_results() */


 
/***************************************************************************************/

/* This function converts an array of integers to a string.  
	Note that the maximum length of the integer is 4 digits. */

char   *  itostr(int *nums, int size)
{
	int      i, offset;
	char     *string;

	string = (char *) malloc(size * 5);
        check_mem(string);
	for (i=0; i<size*5; i++) *(string + i) = '\0'; 

	offset = 0;
	for (i = 0; i < size; i++) 
	{
		sprintf((string + offset), "%d ", nums[i]);
		offset = (int) strlen(string);
	}
	//sprintf((string + offset), '\0');
	*(string + offset) =  '\0';
	return (string);
}





/***************************************************************************************/

/* This function allocates memory for a new class_info type node and initializes the node */

class_info_node      * new_class_info_node(void)
{
	int                  poly;
	
	class_info_node      *new_class_list;		/* local pointer that will be returned */

	new_class_list = (class_info_node *) malloc((unsigned) sizeof(class_info_node));
	check_mem(new_class_list);

	new_class_list->BitNumber = (int *) malloc(sizeof(int) * polymax);
	check_mem(new_class_list->BitNumber);	
	new_class_list->ClassCount = (int *) malloc(sizeof(int) * polymax);
	check_mem(new_class_list->ClassCount);	
	new_class_list->ClassCrownArea = (int *) malloc(sizeof(int) * polymax);
	check_mem(new_class_list->ClassCrownArea);	
	new_class_list->ClassClosure = (int *) malloc(sizeof(int) * polymax);
	check_mem(new_class_list->ClassClosure);	
	new_class_list->ClassHeight = (int *) malloc(sizeof(int) * polymax);
	check_mem(new_class_list->ClassHeight);

	/* Initialize fields to zero */
	
	for (poly = 0; poly < polymax; poly++)
	  {
	  new_class_list->BitNumber[poly] = 0;
	  new_class_list->ClassCount[poly] = 0;
	  new_class_list->ClassCrownArea[poly] = 0;
	  new_class_list->ClassClosure[poly] = 0;
	  new_class_list->ClassHeight[poly] = 0;
	  }
	new_class_list->link = NULL;


	return (new_class_list);
}

/***************************************************************************************/


/* This function adds a new class node to the class list, at the end of the list 
*/

void add_class_list(class_info_node *class_list, class_info_node *new_node)
{
	while (class_list->link)			/* go down the list until the end (until NULL) */
		class_list = class_list->link;
	class_list->link = new_node;		/* replace NULL by pointer to new node */
}


/***************************************************************************************/

/***************************************************************************************/

/***************************************************************************************/




/***************************************************************************************/

/* This function converts a string to upper case characters */

void Upper_Case(char *c)
{
	while (*c != '\0') {
		if (*c >= 'a' && *c <= 'z')
			*c += ('A' - 'a');
		c++;
	}
}
/*********************************************************************************/

/* This function expand parameter vector that contain ranges 
	in_vect		input segment(channel) list possibly including ranges
	out_vect	output expanded list of segments (channels)
	in_count	number of item in input list
	out_count	number of item in output list
*/

void param_expand(int in_count, int *in_vect,  int *out_vect, int *out_count)
{
	int ii, jj, kk;
	
	for(ii=0; ii<BITMAPS; ii++) out_vect[ii]=0;	/* zero vector to init good */
	
	if(in_count == 0)  { *out_count = 0; return; }

	if(in_vect[0] < 0)
	  	{ printf("\n##### ERROR - A range (neg.#) can not be first item.\n"); exit(-1); }
	kk=0;
	for (ii = 0; ii < in_count; ii++)
	  {
	  if(in_vect[ii] == 0) 
	  	{ printf("\n##### ERROR - Segment number can not be zero \n"); exit(-1); }
	  if(in_vect[ii] > 0) 
	  	{ out_vect[kk] = in_vect[ii]; kk++; }
	  if( (in_vect[ii] < 0) && (abs(in_vect[ii]) <= in_vect[ii-1]) )
	  	{ printf("\n##### ERROR - A range appears to be unacceptable.\n"); exit(-1); }
	  if(in_vect[ii] < 0) 
	    {
	    if ( (abs(in_vect[ii]+in_vect[ii-1]) + kk) > BITMAPS )
	  	{printf("\n##### ERROR - Expanded list limited to %d segments.\n",BITMAPS); exit(-1); }
	    for(jj=0; jj<(abs(in_vect[ii]+in_vect[ii-1])); jj++) 
	    		{ out_vect[kk] = out_vect[kk-1] + 1; kk++; }
	    }
	  }
	*out_count = kk;

}	// End of function param_expand()


/***************************************************************************************/
/* This function 'fills' a tree shape (erasing it from the bitmap while processing)
   and gathers information about how many pixels of the crown are IN each stand polygon
   AND  checks for max height of ITC while at it (from HEIGHTCH)
   ( ## ASSUMING LiDAR DCM ITCs (in HEIGHTCH) and image ITCs are relatively well lined-up)
*/

void fill_isol(int x, int y, int xsize, unsigned char *class_BM, int * poly_pixcount)
  {
  int64  bitnum;
  int	 polyno;

   bitnum = ((y - 1)*(int64)xsize ) + x-1;

  /* check for heights within tree crown and pickup highest */

  if(height_flag)
    {
    p_val = (*get_pix_val) (himage, bitnum);
    if (p_val >= max_val) max_val = p_val;
    }

    /* Get pixel count (in general and, in each polygon (Stand) */
	
  counter++;				/* pixel count in ITC */

  if(flag8b)   polyno =  (int) *(stand_ima + bitnum) -1 ;	/* pick-up value at that place in 8bit image (polygon number+1) */
  if(!flag8b)  polyno =  (int) *(stand_image + bitnum) -1 ;	/* pick-up value at that place in 16bit image (polygon number+1) */
	// stand images store as  poly+1

  if(polyno >= 0) poly_pixcount [ polyno ] ++;		 /* pixel count of ITC in poly nn */

  clearbit(class_BM, bitnum);		/* turn off bit - fill current position */

  if (testbit(class_BM,bitnum+1))	fill_isol(x + 1, y, xsize, class_BM, poly_pixcount);
  if (testbit(class_BM,bitnum-xsize))	fill_isol(x, y - 1, xsize, class_BM, poly_pixcount);
  if (testbit(class_BM,bitnum-1))	fill_isol(x - 1, y, xsize, class_BM, poly_pixcount);
  if (testbit(class_BM,bitnum+xsize))	fill_isol(x, y + 1, xsize, class_BM, poly_pixcount);

  }


//****************************************************************

// Function to copy an existing shape file to a new one (fiels and features (Polygons)


//****************************************************************
			
void	Copy_Shape_File(char * polygonfile, char * pcdfile)
{
	int ivec = 1;		//user 1 , software 0
	
	const char *pszDriverName = "ESRI Shapefile";

	int NumberOfInnerRings, NumberOfInteriorRingVertices, NumberOfExteriorRingVertices;

// Open INPUT layer pointed to by user within PCI file

	// REopen shp file in READONLY as we now have a separate output shape file

	seg_in = (GDALDataset*) GDALOpenEx(polygonfile,  GDAL_OF_VECTOR, NULL, NULL, NULL );
	if( seg_in == NULL ){printf( "**** Failed to open input polygon file %s\n", polygonfile); exit(-1 );}
	
	printf("\n\t**File <<%s>> was opened for reading only : single layer mode (vector)\n\n", polygonfile);

	
	layer_count = seg_in->GetLayerCount();
	//printf("\n\t*Number of vector layers in input shp file is %d \n\n", layer_count);

	if( ivec > layer_count )
	  {
	  printf("\n\t\t*** Specified  layer does not exist ****\n");
	  printf("\n\tNumber of vector layers in PCI input file is %d \n\n", layer_count);
	  exit(-1);
	  }


//	printf("\n\t Accessing input vector layer %d \n", ivec);
	piLayer = seg_in->GetLayer(ivec-1);		// Layer numbers start at zero

	piLayer->ResetReading();	// just to be on the safe side (good practice)
	//fprintf(stdout,"\n\tAccessed layer %d \n\n",ivec);


// Number of features (i.e., polygons) in that layer AND number of fields in that layer

	piFDefn = piLayer->GetLayerDefn();
	feat_count = piLayer->GetFeatureCount();
	field_count = piFDefn->GetFieldCount();

	printf("\n\t**Layer %d of current file has %d features (shapes) with %d fields each\n\n", 
				ivec, feat_count, field_count);
/* 				
	feat_count = 10;		//for testing 
		printf("\n** FOR TESTING **  pretend %d features(shapes) with %d fields each\n\n", 
					feat_count, field_count);
 */
// GET DRIVER for output ".shp" file 

	//printf("\nGetting driver \"%s\" for output shape file\n",pszDriverName);

	poDriver = GetGDALDriverManager()->GetDriverByName(pszDriverName);
	if( poDriver == NULL )
	  {
          printf( "%s driver not available.\n", pszDriverName );
          exit( 1 );
	  }

// CREATE  output ".shp"  file 

	//printf("\n\t*Creating output shape file <<%s>> \t\n", pcdfile);

	poDS = poDriver->Create( pcdfile, 0, 0, 0, GDT_Unknown, NULL );
	if( poDS == NULL )
	  {
	  printf( "Creation of output file failed.\n" );
	  exit( 1 );
	  }

// Create one polygon layer in output ".shp"  file

	//printf("\nCreating a layer in output shape file\n");
	
	//poLayer = poDS->CreateLayer("Poly_out", NULL, wkbPolygon, NULL );
	poLayer = poDS->CreateLayer("Poly_out", piLayer->GetSpatialRef(), wkbPolygon, NULL );
	if( poLayer == NULL )
	  {
	  printf( "Layer creation failed.\n" );
	  exit( 1 );
	  }

// Copy ???? GeoTransform, projection, ... NOT a distinc function Done inn above

//	printf("\n\t*Writing projection ???? to file %s\n", pcdfile);	

//	poDS->SetSpatialRef(seg_in->GetSpatialRef());	
	

//*************************************************************************

// 		Read and write polygon data (1st schema, then field data, then vertices)

// Creating fields of output shape file must be done before moving the features (polygons)

//************************************************************************


  printf("\n\tCREATING FIELDS in output shape file (i.e., copying input file SCHEMA)\n\n");

	for (iField=0; iField < piFDefn->GetFieldCount(); iField++)
    {
	piFieldDefn = piFDefn->GetFieldDefn(iField);
	//printf("Input Field %d, Field Type %d, Field Width %d and Precision %d Name %s\n",	
	//	iField, piFieldDefn->GetType(), piFieldDefn->GetWidth(), piFieldDefn->GetPrecision(), piFieldDefn->GetNameRef() ); 
/* 	
	poFieldDefn = piFieldDefn;	// a **NONO** as it still talks to pi so cant modify anything

	for i in range(lyr_def.GetFieldCount()):			// Python example 
    out_lyr.CreateField ( lyr_def.GetFieldDefn(i) )
 */
 
// Create output field  (similar to input field)

	if( poLayer->CreateField( piFieldDefn) != OGRERR_NONE ) 
	  { printf( "Creating field %d failed.\n", iField ); exit( 1 ); } 



 // FORCE  width and/or precision  --->  some "PRF_For_Inv_2007.shp" specific issue
 
	poFDefn = poLayer->GetLayerDefn();
  	poFieldDefn = poFDefn->GetFieldDefn(iField); 
	//if (strcmp(poFieldDefn->GetNameRef(),"AREA") == 0)  poFieldDefn->SetPrecision(2);
	//if (strcmp(poFieldDefn->GetNameRef(),"PERIMETER") == 0)  poFieldDefn->SetPrecision(2);

 
	}			// END OF  for all field in schema//***********************************


printf("\n\t** Created %d fields in output shape file\n\n", iField);
 
 

printf("\n--------------------------------------------------\n");



// 				DOUBLE CKECKING on output fields

/*  
  printf("\n\tDouble checking some (5) fields in output shape file\n\n");

  poFDefn = poLayer->GetLayerDefn();
	  
   for (iField=0; iField < 5; iField++)		 	// only 5 to debug
  //for (iField=0; iField < poFDefn->GetFieldCount(); iField++)
    {  	
	poFieldDefn = poFDefn->GetFieldDefn(iField);
	
	printf("Output Field %d, Field Type %d, Field Width %d, Precision %d Name %s\n", 
		iField, poFieldDefn->GetType(), poFieldDefn->GetWidth(), poFieldDefn->GetPrecision(), poFieldDefn->GetNameRef() );	
    }	// endof for all field in output 
 	
	printf("\n Exiting \n"); exit(-1);
 */





// ***************************************************
// ***************************************************

// 		Now that schema is established (i.e., field definition)

//		 For each polygon (feature)) MOVE FIELD DATA from input shp file to output shp file

// *****************************************************

//	int myFeat=0;				// my own count for display purposes
	piLayer->ResetReading();
	poLayer->ResetReading();	// just to be on the safe side (good practice)
 
 
	printf("\n--------------------------------------------------\n");
	printf("\n\t ** Moving field (attribute) data from input shp file to output shp file\n\n"); 

  

  //for (iFeat=0; iFeat < 10; iFeat++)				// for testing

  for (iFeat=0; iFeat < piLayer->GetFeatureCount(); iFeat++)	
   {
	piFeature = piLayer->GetFeature(iFeat);	
	   
    //printf("  For feature(POLYGON) %d, move its field DATA to the output  shape file\r", iFeat);

	
    poFeature = OGRFeature::CreateFeature( poLayer->GetLayerDefn() );  // creating output feature

	
// For EACH  field in the input layer, POPULATE output layer with same data as input layer

 //for (iField=0; iField < 20; iField++)			// for testing
 
 for (iField=0; iField < piFDefn->GetFieldCount(); iField++)
    {
    piFieldDefn = piFDefn->GetFieldDefn( iField );

//	printf("Input Field %d, Field Type %d, Field Width %d and Precision %d Name %s\n",	
//			iField, piFieldDefn->GetType(), piFieldDefn->GetWidth(), piFieldDefn->GetPrecision(), piFieldDefn->GetNameRef() ); 
	
    poFieldDefn = poFDefn->GetFieldDefn( iField );
	
	//printf("Output Field %d, Field Type %d, Field Width %d and Precision %d Name %s\n",	
	//		iField, poFieldDefn->GetType(), poFieldDefn->GetWidth(), poFieldDefn->GetPrecision(), poFieldDefn->GetNameRef() ); 

		
// Get and Copy (Should use CASE as per tutorial example)
 

        switch( poFieldDefn->GetType() )
            {
                case OFTInteger:
                    //printf( "Int: %d \n", piFeature->GetFieldAsInteger(iField) );
                    poFeature->SetField(iField, piFeature->GetFieldAsInteger(iField) );
					//if (strcmp(poFieldDefn->GetNameRef(),"AREA") == 0)  poFieldDefn->SetPrecision(2);
					//if(iFeat<=3) printf( "iField %d Int: %d \n", iField, poFeature->GetFieldAsInteger(iField) );
                    break;
                case OFTInteger64:
                    //printf( "Int64: \n", piFeature->GetFieldAsInteger64( iField ) );
					poFeature->SetField(iField, piFeature->GetFieldAsInteger64(iField) );
					//if(iFeat<=3) printf( "Int64: \n", poFeature->GetFieldAsInteger64( iField ) );
                    break;
                case OFTReal:
					//poFeature->SetField(iField, piFeature->GetFieldAsDouble(iField) );
					poFeature->SetField(iField, (float) piFeature->GetFieldAsDouble(iField) );
					//if(iFeat<=3)printf( "iField %d Real: %.5f \n", iField, piFeature->GetFieldAsDouble(iField) );
                    break;					
                case OFTString:
                    //printf( "String : %s \n", piFeature->GetFieldAsString(iField) );
					poFeature->SetField(iField, piFeature->GetFieldAsString(iField) );
					//if(iFeat<=3) printf( "String : %s \n", poFeature->GetFieldAsString(iField) );
                    break;
                default:
                    //printf( "String: %s \n", piFeature->GetFieldAsString(iField) );
					poFeature->SetField(iField, piFeature->GetFieldAsString(iField) );
                    break;
            }
			
	

		


// Other tests
   //poFeature->SetField(iField, piFeature->GetType(iField) );
    //poFeature->SetField(iField, piFieldDefn->GetType() );
    //poFeature->SetField(iField, piFeature->FieldValue(iField));    // should work ??
	
    //poFeature->SetField(iField, iField );		// WORKS : put field # in field for all features
	

	//printf("\n***\n");
	
    }		// END OF LOOP   for field data for that feature
	
		//poFeature->SetField(73, "TEST" );			//  test
		
		//poLayer->SetFeature(poFeature);			//  NEED  to write it out (otherwise just in memory)
		
 		 
		 
/* 			 
      if( poLayer->SetFeature( poFeature ) != OGRERR_NONE )			 // NOGO 
 	      {
        	printf( "\t**Failed to create feature(poly) in shapefile.\n" );
        	exit( 1 );
 	      }
  */	 



	//  NEED  to write it out (otherwise just in memory)
	
     if( poLayer->CreateFeature( poFeature ) != OGRERR_NONE )
		  {
        	printf( "\t**Failed to create feature(poly) in shapefile.\n" );
        	exit( 1 );
 	      }
 			 
	


   }		// end of feature (polygon) loop
		

    printf("\n\t Field (attribute) data of %d features copied to the output shape file\n", iFeat);	

	
		//exit(-1);
		
		//goto END_CSF;			// debugging speed-up - dont move polygons
 
//**************************************************************************

// 		For each feature (polygon) ** MOVING POLYGON VERTICES **

//***************************************************************************

	printf("\n--------------------------------------------------\n");
	
	printf("\n\t ** For each polygon -- moving polygon vertices\n\n");

    OGRPolygon *piPolygon, *poPolygon;
    OGRLinearRing *piExteriorRing;

	piLayer->ResetReading();	
	poLayer->ResetReading();	
			

//  for (iFeat=0; iFeat < 20; iFeat++)				// for testing 
 for (iFeat=0; iFeat <  piLayer->GetFeatureCount(); iFeat++)	   
	{
	piFeature = piLayer->GetFeature(iFeat);
	poFeature = poLayer->GetFeature(iFeat);
	
    //printf("\nReading vertices of polygon %d of input vector layer\n", iFeat);


    piGeometry = piFeature->GetGeometryRef();	// get its geometry, hoping polygon (3)
	
   // printf("For feature %d \t Geometry is : %d \n", iFeat, wkbFlatten(piGeometry->getGeometryType()));

  //     newFeature = ogr.Feature(newLayerDef)
 //           newFeature.SetGeometry(poly)
 //           newFeature.SetFID(featureID)
 //           newLayer.CreateFeature(newFeature)

    if ( piGeometry != NULL && wkbFlatten(piGeometry->getGeometryType()) == wkbPolygon )		// if polygon
	{
	piPolygon = (OGRPolygon *)piGeometry;

	//Polygon.PolygonsOfFeature.resize(1);		// resize some generic polygon storage ???

	NumberOfInnerRings = piPolygon->getNumInteriorRings();	
	//printf("Number of inner rings = %d \n", NumberOfInnerRings);		//info

	//Polygon.PolygonsOfFeature.at(0).Polygon.resize(NumberOfInnerRings+1);		// resize storage

	piExteriorRing = piPolygon->getExteriorRing();
	NumberOfExteriorRingVertices = piExteriorRing->getNumPoints();
	//printf("NumberOfExteriorRingVertices = %d \n", NumberOfExteriorRingVertices);

	//Polygon.PolygonsOfFeature.at(0).Polygon.at(0).RingString.resize(NumberOfExteriorRingVertices);
	//Polygon.PolygonsOfFeature.at(0).Polygon.at(0).IsClockwised = piExteriorRing->isClockwise();

	}
 
	//printf("\n Got here 1  -- Feat  %d \n", iFeat);


	//poPolygon = (OGRPolygon *)piGeometry;				//  OK
	
	

// MOVE VERTICES of REAL polygon (i.e., geometry) to output feature
// **** THAT IS ALL  you need to do (if both are real polygons)


	if (wkbFlatten(piGeometry->getGeometryType()) == wkbPolygon )
	{
		//printf("\n Got here 2  -- Feat  %d \n", iFeat);
	  	poFeature->SetGeometry( piGeometry );					// *** WORKS  -- moves all vertices (inner and outer)
	  	//poFeature->SetGeometry( piPolygon );
	  	//poFeature->SetFID( iFeat );		
		//poLayer->CreateFeature(poFeature);
		//poFeature->SetGeometry( piGeometry->getGeometryType( ));		
	    //poPolygon = (OGRPolygon *)piGeometry;					//  OK
	     //poFeature = (OGRPolygon *) piGeometry;	
		//printf("\n Got here 2.5  -- Feat  %d \n", iFeat);
	}
	
	//printf("\n Got here 3  -- Feat  %d \n", iFeat);
	
	

// 		### IF IT WAS  A FAKE POLYGON  ( a la PCI)

/* 
	  if (wkbFlatten(piGeometry->getGeometryType()) == wkbLineString )
		{

 			poFeature->SetGeometry(GetGeometryType(poPolygon));	

			//poFeature.Geometry = GetGeometryType(poPolygon);

			//OGR_F_SetGeometry(poFeature, poPolygon);

	    }
 */
 
 
 
 
	// Need to move the feature (polygons) and its attributes to the output file
	// from memory --- Now we create a feature in the file

 
 	     //if( poLayer->CreateFeature( poFeature ) != OGRERR_NONE )
			 
      if( poLayer->SetFeature( poFeature ) != OGRERR_NONE )			 
 	      {
        	printf( "\t**Failed to create feature(poly) in shapefile.\n" );
        	exit( 1 );
 	      }
 	     
/* 	
     if( poLayer->SetFeature( poPolygon ) != OGRERR_NONE )
 	      {
        	printf( "\t**Failed to update feature(poly) in shapefile.\n" );
        	exit( 1 );
 	      }
	     
 */



// Prepare for next feature	(in the loop)
		 
		//OGRFeature::DestroyFeature( poFeature );

		//poLayer->GetNextFeature();		// done above
 			
		
	//  printf("Polygon %d vertices were moved to output SHP file \n\n", iFeat);
	 //printf("Polygon %d vertices were moved to output SHP file \r", iFeat);	  
	  
		
	  }		// **** MAIN LOOP ***  for NEXT feature 







// ***********************

/* 

// 	READ input vertices and WRITE them to output polygon

	// Strangely, no need to move points (in fact if you move them, you get point features)

	printf("\n\tReading input vertices and **writing ** them to output shape...\n");

	for ( int k = 0; k < NumberOfExteriorRingVertices; k++ )   //for all vertices (points)

             {
              piExteriorRing->getPoint(k,&ptTemp);	// get that point in ptTemp, an OGRPoint
	      if (k < 3) printf( "%.2f  %.2f\t", ptTemp.getX(), ptTemp.getY() );   // Print some

             //Polygon.PolygonsOfFeature.at(0).Polygon.at(0).RingString.at(k) = ptTemp;



              MyPoint2D pt;
              pt.dX = ptTemp.getX();
              pt.dY = ptTemp.getY();
	      if (k < 3) printf( "%.2f  %.2f\t", pt.dX, pt.dY );	// Print some UTM vertices

              //Polygon.PolygonsOfFeature.at(0).Polygon.at(0).RingString.at(k) = pt;

              OGRPoint pt;
              pt.setX( ptTemp.getX() );
              pt.setY( ptTemp.getY() );
	      if (k < 3) printf( "%.2f  %.2f\t", pt.getX(), pt.getY() );	// Print some UTM vertices

             }		// end of loop for vertices (points)
*/


// WRITE vertices from  input to  output shape file 

//	       poFeature->SetGeometry( piGeometry );		// that's all ???   #####
		   
	       //poFeature->SetGeometry( Polygon );
	       //poFeature->SetGeometryDirectly(OGRPolygon);

 /*        	if( poLayer->CreateFeature(poFeature) != OGRERR_NONE)		// WRITE out feature to shp file
        	  {
       	     	  printf( "Failed to create feature in shapefile.\n" );
       	     	  exit( 1 );
        	  } 
*/

//	printf("\tPolygon %d was moved to output SHP file \r", iFeat);

        	//OGRFeature::DestroyFeature( poFeature );


 //   }		// end of feature (polygon) loop

	
END_CSF:

	printf("\n** All polygons and existing attributes moved to output SHP file <<%s>>\n", pcdfile);

	//printf("\n\tExisting layer description: %s \n", piLayer->GetDescription() );
	//poLayer->SetDescription(piLayer->GetDescription());

	//GDALClose( poDS );		// Close that data set (shp file)


  }	// END of Copy_Shape_File()




//************************************************************

// Create new fields (GDAL style) to store PCD-generated info

//************************************************************


/* Prepare (create if needed) the necessary fields in the output layers 
   and set their initial values
   
   WARNING: If you create two fields with the same name this will create confusion
		as you will write your data to the latest version however, other programs
		using GDBGetFieldIndex() or Imageworks query will always relate to
		the first instance of the field.
   SOLUTION: Check if a field exist before creating it. If exist, dont create.
   
*/

void 	Prep_New_Fields(OGRLayer * poLayer)
{
	
	  
//	Creating GENERIC  ITC info fields in output shape file 
	
	printf("\n\tCreating generic ITC info fields in output shape file \n");


	OGRFieldDefn oField00( "Feature", OFTInteger );
	oField00.SetWidth(7);
	if( poLayer->CreateField( &oField00 ) != OGRERR_NONE )
	  { printf( "Creating <Feature> field failed.\n" ); exit( 1 ); }
  
	OGRFieldDefn oField0( "Area(p)", OFTInteger );
	oField0.SetWidth(10);
	if( poLayer->CreateField( &oField0 ) != OGRERR_NONE )
	  { printf( "Creating <Area(p)> field failed.\n" ); exit( 1 ); }


	OGRFieldDefn oField1( "Area(ha)", OFTReal );
	oField1.SetWidth(7); oField1.SetPrecision(2); 
	if( poLayer->CreateField( &oField1 ) != OGRERR_NONE )
	  { printf( "Creating <Area(ha)> field failed.\n" ); exit( 1 ); }

	OGRFieldDefn oField2( "ITCs", OFTInteger );
	oField2.SetWidth(8);
	if( poLayer->CreateField( &oField2 ) != OGRERR_NONE )
	  { printf( "Creating <ITCs> field failed.\n" ); exit( 1 ); }


	OGRFieldDefn oField3( "ITC CC", OFTReal );
	oField3.SetWidth(5);  oField3.SetPrecision(1); 
	if( poLayer->CreateField( &oField3	) != OGRERR_NONE )
	  { printf( "Creating <ITC CC> field failed.\n" ); exit( 1 ); }


	OGRFieldDefn oField4( "Density", OFTInteger );
	oField4.SetWidth(8);
	if( poLayer->CreateField( &oField4 ) != OGRERR_NONE )
	  { printf( "Creating <Density> field failed.\n" ); exit( 1 ); }

	OGRFieldDefn oField5( "AvCrownA", OFTReal );
	oField5.SetWidth(7);  oField5.SetPrecision(1); 
	if( poLayer->CreateField( &oField5	) != OGRERR_NONE )
	  { printf( "Creating <AvCrownA> field failed.\n" ); exit( 1 ); }
  

  	OGRFieldDefn oField6( "AvHeight", OFTReal );
	oField6.SetWidth(5);  oField6.SetPrecision(1); 
	if( poLayer->CreateField( &oField6	) != OGRERR_NONE )
	  { printf( "Creating <AvHeight> field failed.\n" ); exit( 1 ); }
  
  
  	OGRFieldDefn oField7( "HeightSD", OFTReal );
	oField7.SetWidth(5);  oField7.SetPrecision(1); 
	if( poLayer->CreateField( &oField7	) != OGRERR_NONE )
	  { printf( "Creating <HeightSD> field failed.\n" ); exit( 1 ); }



// Class (species) related information
 
if(details == 1) goto E1;			// IF only generic ITC info is needed

printf("\n\tCreating Class-Related ITC info fields in output shape file \n");


/* SPECIES FIELDS (for ALL species entered or FIVE most prevalent species)  */

int repclasses;			//representative classes (classes of interest)

if(details == 2) repclasses = 5;					// MEDIUM -- FIVE most prevalent species) 
if(repclasses > numclasses) repclasses = numclasses;	// dont display more classes than exist (i.e 5 when 3)
if(details == 3) repclasses = numclasses;			// HIGH --  ALL species entered


//		When needed order classes by importance (by crown closure, by ITC counts)

int iclass; 

for (iclass = 1; iclass <= repclasses; iclass++) 	
{
	
// Species class number  (field #n) 
	
	sprintf(Fname,"Class%.2d", iclass);	
	
 	OGRFieldDefn oField8( Fname, OFTInteger );
	oField8.SetWidth(7);  
	if( poLayer->CreateField( &oField8	) != OGRERR_NONE )
	  { printf( "Creating %s field failed.\n", Fname ); exit( 1 ); }

// Number of ITCs of that class


	sprintf(Fname,"ITCs%.2d", iclass);
	
	OGRFieldDefn oField9( Fname, OFTInteger );
	oField9.SetWidth(8);  
	if( poLayer->CreateField( &oField9	) != OGRERR_NONE )
	  { printf( "Creating %s field failed.\n", Fname ); exit( 1 ); }

//  Percentage of total stems (for that class) 


	sprintf(Fname,"PER%.2d", iclass);

	OGRFieldDefn oField10( Fname, OFTReal );
	oField10.SetWidth(5);  oField10.SetPrecision(1); 
	if( poLayer->CreateField( &oField10	) != OGRERR_NONE )
	  { printf( "Creating %s field failed.\n", Fname ); exit( 1 ); }
  
  

// Percentage of total Closure	(for that class) 
	
	sprintf(Fname,"CC%.2d", iclass);
	OGRFieldDefn oField11( Fname, OFTReal );
	oField11.SetWidth(5);  oField11.SetPrecision(1); 
	if( poLayer->CreateField( &oField11	) != OGRERR_NONE )
	  { printf( "Creating %s field failed.\n", Fname ); exit( 1 ); }
 

// Stem Density 	(for that class)

	sprintf(Fname,"SD%.2d", iclass);
	
	OGRFieldDefn oField12( Fname, OFTInteger );
	oField12.SetWidth(5);  
	if( poLayer->CreateField( &oField12	) != OGRERR_NONE )
	  { printf( "Creating %s field failed.\n", Fname ); exit( 1 ); }


// Average Crown Diameter  	(for that class)

	sprintf(Fname,"DIA%.2d", iclass);

	OGRFieldDefn oField13( Fname, OFTReal );
	oField13.SetWidth(5);  oField13.SetPrecision(1); 
	if( poLayer->CreateField( &oField13	) != OGRERR_NONE )
	  { printf( "Creating %s field failed.\n", Fname ); exit( 1 ); }
 
// Average height 	(for that class)

	sprintf(Fname,"HT%.2d", iclass);

	OGRFieldDefn oField14( Fname, OFTReal );
	oField14.SetWidth(5);  oField14.SetPrecision(1); 
	if( poLayer->CreateField( &oField14	) != OGRERR_NONE )
	  { printf( "Creating %s field failed.\n", Fname ); exit( 1 ); }



}	// End of loop for all species to consider

E1:	field_count = poFDefn->GetFieldCount();
	printf("\n\tDone preparing new fields in output shape file. Now %d fields \n", field_count);

}	//    End of Prep_New_Fields()





//************************************************************

//	Populate ITCPCD fields

//************************************************************

void 	Add_New_Info(OGRLayer * poLayer, class_info_node *class_list)
{
	int 	iField, iFeat, TempInt;
	float 	TempFloat;
	float   avgcrownsz, avgcrownht, avgcrowndiam, stemsperhec, ClsPercntArea, ClsPercntStms;
	float 	TotClosure, TotStmsPerHec, TotAvCrownA, TotAvHeight, TotSDHeight;
	int 	i, poly=0, shapecount=0;
	int		tempval, iclass;

	class_info_node      *classN, *sign_class;	
		
	printf("\n\t Populating ITC Info fields in output shape file \n\n");	
	
	//poLayer->ResetReading();
	feat_count = poLayer->GetFeatureCount();
	field_count = poFDefn->GetFieldCount();
	printf("\nPolygons to populate(%d) in shape file with %d fields each\n\n", feat_count, field_count);

/* 	
	feat_count = 10;		//for testing 
	printf("\n** FOR TESTING **  pretend %d features(shapes) with %d fields each\n\n", 
					feat_count, field_count);
 */
	printf("Writing 'ITC Generic Info' for each polygon \n\n");
	
  for (iFeat=0; iFeat < feat_count; iFeat++)			// for all features (polygons)
  {
	poly = iFeat;
	poFeature = poLayer->GetFeature(iFeat);	

	 if(iFeat/10*10 == iFeat) printf("Writing ITC Generic data for feature (poly) %d \n", iFeat);
	

	iField =  poFeature->GetFieldIndex("Feature");	
	if(iField == -1) { printf("ERROR Getting field index for Feature\n\n"); exit(-1);}
    poFeature->SetField(iField, iFeat );
 // if(iFeat/10*10 == iFeat) printf("iField for Feature %d : %d \n", iFeat, iField);


	// Get field "Area(p)" and populate
	
	iField =  poFeature->GetFieldIndex("Area(p)");	
	if(iField == -1) { printf("ERROR Getting field index for Area(p)\n\n"); exit(-1);}

    //poFieldDefn = poFDefn->GetFieldDefn( iField );
    //poFeature->SetField(iField, 1234 );			// TEST
    poFeature->SetField(iField, polyarea[poly] );	
 //if(iFeat/10*10 == iFeat) printf("iField for Area(p): %d Data=%d \n",iField, polyarea[poly]);
	
	// Get field "Area(ha)" and populate
	
	iField =  poFeature->GetFieldIndex("Area(ha)");	
	if(iField == -1) { printf("ERROR Getting field index for Area(ha)\n\n"); exit(-1);}
	//printf("iField for Area(ha) : %d\n",iField);	
    //poFeature->SetField(iField, 3.5 );			// to TEST
    poFeature->SetField(iField, fabs(polyarea[poly]*xpixsz*ypixsz/10000));		
 // if(iFeat/10*10 == iFeat) printf("iField for Area(ha): %d Data=%f \n",iField, poFeature->GetFieldAsDouble(iField) );	
	
	// Get field "ITCs" and populate		
		
	iField =  poFeature->GetFieldIndex("ITCs");	
	if(iField == -1) { printf("ERROR Getting field index for ITCs\n\n"); exit(-1);}		
	poFeature->SetField(iField, IsolCount[poly]);			
		
	// Get field "ITC CC" and populate		

	iField =  poFeature->GetFieldIndex("ITC CC");	
	if(iField == -1) { printf("ERROR Getting field index for ITCs CC\n\n"); exit(-1);}		
//	poFeature->SetField(iField, (( (float)TotalClosure[poly]) / ( (float)polyarea[poly])) * 100.0);	
	if( polyarea[poly] > 0 ) 
	  {
	  TempFloat =  (float)TotalClosure[poly] / (float)polyarea[poly]  * 100.0 ;	
	  poFeature->SetField(iField, TempFloat);		
	  }


	// Get field "Density" and populate		

	iField =  poFeature->GetFieldIndex("Density");	
	if(iField == -1) { printf("ERROR Getting field index for Density\n\n"); exit(-1);}	
	if( polyarea[poly] > 0 ) 
	  {
	  TempInt = (((int) IsolCount[poly]) * 10000.0) / (polyarea[poly] * xpixsz * ypixsz);
	  poFeature->SetField(iField, TempInt);
	  }

	// Get field "AvCrownA" and populate		

	iField =  poFeature->GetFieldIndex("AvCrownA");	
	if(iField == -1) { printf("ERROR Getting field index for AvCrownA\n\n"); exit(-1);}
	if (IsolCount[poly] != 0)
	{
	TempFloat =  ( (float)TotalIsolArea[poly] * xpixsz * ypixsz ) / (float)IsolCount[poly] ;		
	poFeature->SetField(iField, TempFloat);
	}

	// Get field "AvHeight" and populate		

	iField =  poFeature->GetFieldIndex("AvHeight");	
	if(iField == -1) { printf("ERROR Getting field index for AvHeight\n\n"); exit(-1);}
	if (IsolCount[poly] != 0) 
	{
	TempFloat = (float)IsolHeight[poly] / (float)IsolCount[poly] ;	
	poFeature->SetField(iField, TempFloat);	
	}

	// Get field "HeightSD" and populate		

	iField =  poFeature->GetFieldIndex("HeightSD");	
	if(iField == -1) { printf("ERROR Getting field index for HeightSD\n\n"); exit(-1);}
	if (IsolCount[poly] != 0) 
	{		
	TempFloat = sqrt ( ( (float)IsolSQHeight[poly] - (float)IsolHeight[poly]*(float)IsolHeight[poly]/(float)IsolCount[poly] ) / ((float)IsolCount[poly]-1) ) ;
	poFeature->SetField(iField, TempFloat);	
	}
		 
      if( poLayer->SetFeature( poFeature ) != OGRERR_NONE )			 
 	      {
        	printf( "\t**Failed to create feature(poly) in shapefile.\n" );
        	exit( 1 );
 	      }
 		
	
  }		// End of loop to write generic ITC info to polygons (i.e., next feature)
			
	printf("\n\t ** End of Populating 'ITC Generic Info' fields in output shape file \n\n");



//	if(iFeat/10*10 == iFeat) printf("Generic ITC data for feature %d written \n", iFeat+1);
//  printf("Generic ITC data for feature %d written \n", iFeat+1);

if(details == 1) goto E2;		// IF only generic ITC info needed (no specific classes)


//goto E2;		// ### FOR TESTING  bypass class info


//---------------------------------------

// ITC Info for EACH CLASS (species)

//---------------------------------------


	printf("\n--------------------------------------------------\n");
	printf("\n\t Populating 'ITC Class Info' fields in output shape file \n\n");	
	
	//poLayer->ResetReading();
	feat_count = poLayer->GetFeatureCount();
	field_count = poFDefn->GetFieldCount();
	printf("\tPolygons to populate (%d) in shape file with %d fields each\n\n", feat_count, field_count);

// SPECIES FIELDS (for ALL species entered or FIVE most prevalent species) 

int repclasses;			//representative classes (classes of interest)
if(details == 1) repclasses = 0;		/* NO  species)  */
if(details == 2) repclasses = 5;		/* FIVE most prevalent species)  */
if(repclasses > numclasses) repclasses = numclasses;	// dont display more classes than exist (i.e 5 when 3)
if(details == 3) repclasses = numclasses;	/* for ALL species entered */

printf("\t Number of classes = %d    Detail level = %d    Order = %d \n\n", repclasses, details, order );
	
/* 
//  FOR  TESTING 
	feat_count = 10;
	printf("\n** FOR TESTING **  pretend ONLY %d features(shapes) with %d fields each\n\n", 
					feat_count, field_count);
 */	
	
for (iFeat=0; iFeat < feat_count; iFeat++)			// for all features (polygons)
  {
	poly = iFeat;
	poFeature = poLayer->GetFeature(iFeat);	

	class_list_beg = class_list->link;			/* beginning of the linked list */
	classN = class_list_beg;
	
	if( polyarea[poly] == 0 ) goto nextpoly2;		// skip polygon outside image or empty  ###
	
		
for (iclass = 1; iclass <= repclasses; iclass++) 					// check all classes for significant one
	{	

 // Default - will order info by species (same order as given) 

	if(order == 1) sign_class = classN;  
	  
 // Find NEXT most prevalent species (most significant class) for the current polygon (based on closure)
	 
	if(order == 2)
	{			
	  classN = class_list->link;
	  tempval = 0;	
	  while(classN)						/* go through all classes via links */
	   {
	   if (classN->ClassCrownArea[poly] >= tempval)
	     {									/* Find most significant class - to do first */
	     sign_class = classN;				/* by loop end - sign_class is the class with the most CC for current poly */
	     tempval = classN->ClassCrownArea[poly];  		
	     }
	   classN = classN->link;					/* stop loop when you hit a NULL pointer in last class instance */
	   }	   
	   /*printf("CC tempval = %d\n", tempval);  */
	}
	
	// Find NEXT most prevalent species (significant class) for current polygon (based on ITC count) 
		
	if(order == 3)
	{
	  classN = class_list->link;
	  tempval = 0;
	  while(classN)					/* go through all classes via links */
	   {
	   if (classN->ClassCount[poly] >= tempval)
	     {									/* Find most significant class - to do first */
	     sign_class = classN;				/* by loop end - sign_class is the class with the most trees for current poly */
	     tempval = classN->ClassCount[poly];  		
	     }
	   classN = classN->link;				/* stop loop when you hit a NULL pointer in last class instance */
	   }	   
	  /* printf("#ofITC tempval = %d\n", tempval); */
	}


	    classN = sign_class;			// calculate and output for the significant class as just selected
	
		//printf("\n\t For poly %d -- ClssCount = %d \n\n" , poly, classN->ClassCount[poly]);
	
	    avgcrownsz = ((float) classN->ClassCrownArea[poly]) / ((float) classN->ClassCount[poly]);	
	    stemsperhec = (((float) classN->ClassCount[poly]) * 10000.0) / (polyarea[poly] * xpixsz * ypixsz);
	    ClsPercntStms = (((float) classN->ClassCount[poly]) / ((float) IsolCount[poly])) * 100.0;
 	    avgcrownht = ((float) classN->ClassHeight[poly]) / ((float) classN->ClassCount[poly]);
	    avgcrowndiam = sqrt(((avgcrownsz*xpixsz*ypixsz*4)/3.14159));

	    ClsPercntArea = (((float) classN->ClassClosure[poly]) / ((float) TotalClosure[poly])) * 100.0;

//	    printf("%3d %7d %4d %7.1f  %9.1f %7.1f %6.1f %9.1f\n",
//			poly, classN->BitNumber[poly],classN->ClassCount[poly], ClsPercntStms, ClsPercntArea, stemsperhec, avgcrowndiam, avgcrownht);

	    if (strncmp(province,"QUE",3) == 0)	    
			ClsPercntArea = (((float) classN->ClassClosure[poly]) / ((float) polyarea[poly])) * 100.0;
	    
	    if (strncmp(province,"ONT",3) == 0)
	       ClsPercntArea = (((float) classN->ClassClosure[poly]) / ((float) TotalClosure[poly])) * 100.0; 

		if(classN->ClassCount[poly] == 0) 
			{ClsPercntStms=0; ClsPercntArea=0; stemsperhec=0; avgcrowndiam=0; avgcrownht=0;}


	// Class number			###   need something better here to know which class was rely pickup ###  BitNumber[poly] ???

		sprintf(Fname,"Class%.2d", iclass);	

		iField =  poFeature->GetFieldIndex(Fname);	
		if(iField == -1) { printf("ERROR Getting field index for %s\n\n", Fname); exit(-1);}
		  
		  TempInt = classN->BitNumber[poly];	  
		  
		  poFeature->SetField(iField, TempInt);


	// Number of ITCs of that species (class)

		sprintf(Fname,"ITCs%.2d", iclass);
		
		iField =  poFeature->GetFieldIndex(Fname);	
		if(iField == -1) { printf("ERROR Getting field index for %s\n\n", Fname); exit(-1);}
		  TempInt = classN->ClassCount[poly];
		  poFeature->SetField(iField, TempInt);
		
	// Percentage of stems
		
		sprintf(Fname,"PER%.2d", iclass);
		
		iField =  poFeature->GetFieldIndex(Fname);	
		if(iField == -1) { printf("ERROR Getting field index for AvHeight\n\n"); exit(-1);}
		if (IsolCount[poly] != 0) 
		{
		TempFloat = ClsPercntStms;	
		poFeature->SetField(iField, TempFloat);	
		}	
		
	// Percentage of crown closure	
		
		sprintf(Fname,"CC%.2d", iclass);
		
		iField =  poFeature->GetFieldIndex(Fname);	
		if(iField == -1) { printf("ERROR Getting field index for AvHeight\n\n"); exit(-1);}
		if (IsolCount[poly] != 0) 
		{
		TempFloat = ClsPercntArea ;	
		poFeature->SetField(iField, TempFloat);	
		}

		
	// Stem density of that species (class)	
		
		sprintf(Fname,"SD%.2d", iclass);
		
		iField =  poFeature->GetFieldIndex(Fname);	
		if(iField == -1) { printf("ERROR Getting field index for %s\n\n", Fname); exit(-1);}
		  TempInt = (int) stemsperhec;
		  poFeature->SetField(iField, TempInt);
		

	// Mean_Diam for that species (class)	

		sprintf(Fname,"DIA%.2d", iclass);
		
		iField =  poFeature->GetFieldIndex(Fname);	
		if(iField == -1) { printf("ERROR Getting field index for AvHeight\n\n"); exit(-1);}
		if (IsolCount[poly] != 0) 
		{
		TempFloat = avgcrowndiam;	
		poFeature->SetField(iField, TempFloat);	
		}
		
		
	//  Mean_Height	for that species (class)	
		
		sprintf(Fname,"HT%.2d", iclass);
		
		iField =  poFeature->GetFieldIndex(Fname);	
		if(iField == -1) { printf("ERROR Getting field index for AvHeight\n\n"); exit(-1);}
		if (IsolCount[poly] != 0) 
		{
		TempFloat = avgcrownht;	
		poFeature->SetField(iField, TempFloat);	
		}

		 
      if( poLayer->SetFeature( poFeature ) != OGRERR_NONE )			 
 	      {
        	printf( "\t**Failed to create feature(poly) in shapefile.\n" );
        	exit( 1 );
 	      }


//printf("ITC Species %d data for feature %d written \n", iclass, iFeat+1);
 

// Goto next class depending on class order

	    if(order == 1 ) classN = classN->link;					/* class for next iteration (from linked classes) */	  	  
	    if(order == 2 ) sign_class->ClassCrownArea[poly]= -1;		/* destroy "CC info" so not found again in search loop */
	    if(order == 3 ) sign_class->ClassCount[poly] = -1;		/* destroy "ITC count" so not found again in search loop */

		
	    }		// end of loop for EACH CLASS   (or designated class)
		 
		if(iFeat/10*10 == iFeat) printf("ITC Species data for polygon %d written \n",  iFeat+1);


nextpoly2:		// to bypass poly with area=0 considered outside the image

	if( polyarea[poly] == 0 ) printf("Polygon %d was bypassed cause outside the image or empty \n", poly);
	
  }		// End of writing ITC class info to polygon



// Goto E2 is Used to bypass producing class-related info (when not required)

E2:	
	//if(iFeat/10 == iFeat) printf("\n ITC data for feature(poly) %d written \n", iFeat+1);

	//printf("\t\tPolygons %d done  \r", poly+1); 

	//  ###### NEED ##### to write it out (otherwise just in memory)
	
     if( poLayer->SetFeature( poFeature ) != OGRERR_NONE )
		  {
        	printf( "\t**Failed to mod feature(poly) in shapefile.\n" );
        	exit( 1 );
 	      }


printf("\n\n\t*** Done populating fields with ITC info in the output shape file \n");


}			//    End of Add_New_Info()



	
/* 
		
// To DOUBLE CHECK - Print ALL existing FIELDS (for PCI or TIF main files)

  printf("\nDouble checking on some SHP file FIELDS  existance \n\n");

//  for (iField=0; iField < field_count; iField++)
   for (iField=0; iField < 5; iField++)			//print five(5) fields
    {
	piFieldDefn = piFDefn->GetFieldDefn(iField);
	printf("Input field %d, Type %d, Field Width %d, Precision %d Name: %s\n",
		iField, piFieldDefn->GetType(), piFieldDefn->GetWidth(), piFieldDefn->GetPrecision(),piFieldDefn->GetNameRef() );  
	}
	
*/


//----------------------------------------------------------------

//		End of functions
