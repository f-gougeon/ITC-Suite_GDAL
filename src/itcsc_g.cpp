/*
Program name: 	itcsc_g.cpp (for GDAL)
Author: 	Francois A. Gougeon
Date:		June-Aug 2020 (COVID-19 version)

*********************************
C+
c	ITCSC.cpp

c	Individual Tree Crown Supervised Classifier
c
C	ITCSC classifies the individual tree crowns (ITCs) of an image into  
c	different species using a Maximum-Likelihood (ML) decision rule.
c
c	The classification is based on comparing the signature of each ITC, 
C	one by one, with the ITC-based signatures of the various species. 
c	The species signatures are those which were produced by the ITCSSG 
c	program. A series of bitmaps is produced showing the results of the
c	classification. One bitmap per species is generated showing the ITCs
c	classified to that species.
C	Optionaly, a database of the individual tree crown signatures may be
c	generated in the file specified by DBOUT. 
c
c	Optionnaly, for RESEARCH PURPOSES, the bitmap of automatically  
c	delineated crowns (ISOLBIT) can now be replaced by a vector layer of  
c	manually delineated crowns. TREETYPE must be set to SLAYER.
c	In that case, the class bitmaps are not generated.
c	Instead, the resulting class for each tree is stored in 
c	a new attribute (field) in the vector layer.
c	The user will be prompted for a field name (normal mode), unless
c	CLASSR is set ahead of time (useful for batch mode).
c	
C
C
C1	PARAMETERS
C
C
C	ITCSC is controlled by the following global parameters:
C
C	Name		Prompt					Count	Type
C
C	FILE		Database File Name			64	Char
C	DBIC		Database Input Channel List		8	Int
c	SCHINDX		Signature channel index			8	Int
C	ISOLBIT		Bitmap from ITCISOL			1	Int
C 	SIGNSEG		Input Signature Segments		16	Int
C	CLASSBIT	Classes bitmaps	(classif. results)	16	Int
C	CLASCODE	Class codes				16	Int
C	EXTBIT		External Bitmap Mask			1	Int
C	SIGTYPE		MEAN/TCL/TCL2/TEXTUR/STRUCT		64	Char
C	TREETYPE	Tree Type:  TT/ITC/SLAYER		64	Char
C	THRESHLD	Threshold (0-1)				1	Real
C	DBOUT		Output Database Filename		64	Char
C	REPORT		Report Mode: TERM/OFF/filename		64	Char
C
C
C2	FILE
C
C	Specifies the name of the PCI file containing the images and
c	the signatures and the bitmaps.
C
C2	DBIC
C
C	Specifies the PCI database channel(s) holding the input images.
c
C
c2	SCHINDX
c
c	OPTIONNAL index (1-8) to select by their order which channels in the  
c	species signatures match the DBIC channels (i.e., which channels are
c	spectrally compatible). 
c
c	Useful when signatures were ported from another "normalized" image 
c	(i.e., "signature extension" with ITCSSM ot ITCSSM2) where channel
c	numbers are completely different. 
c	For example, signatures were based on channels 13,14,15,16 in their 
c	original PCI file and the corresponding channel numbers in this PCI
c	file are 7,8,9,10 and, in addition, the user only wants to use three
c	out of four channels (DBIC=7,8,10). This is accomplish by using
c	DBIC=7,8,10 and SCHINDX=1,2,4.
c
c	NOTE:	Not necessary if channels match perfectly or if only a subset
c		of matching channels is required or if the channels are only
c		in a different order.
c
C2	ISOLBIT
C
C	Specifies a bitmap produced by the ITCISOL (Individual Tree Crown
C	Isolation) program to be used as a mask of the image.  This bitmap
C	is scanned to determine the locations and crown extent of the individual
c	tree crowns and thereby, build up the individual tree signatures from
c	the multispectral values gathered underneath that mask (this can be
c	modified by EXTBIT)
c
c	** Optionnaly ***, for research purposes, the bitmap of automatically  
c	delineated crowns (ISOLBIT) can now be replaced by a vector layer of
c	all manually delineated crowns to be classified (TREETYPE=SLAYER).
c	In such case, class bitmaps (CLASSBIT) will not be generated and the 
c	resulting class for each tree will be found in a new field in the 
c	input layer (ISOLBIT). ITCSC will generally prompt the user for a  
c	field name in which to put the classification results. 
c
c	If a hidden parameter, CLASSR, is set to a field name (rather than
c	its default of NOTSET), the user will not be prompted and the name
c	specified by CLASSR will be used. This is very useful when ITCSC is
c	to be run in automatic, unattended scripts, as these scripts have no
c	capability of answering questions from "C" programs.
c
c	Just define CLASSR in advance of running ITCSC in your script:
c	 	CLASSR = "class_lit_3ch"
c		run ITCSC
c
c	If you'll rather be prompted, make sure CLASSR = "NOTSET"
c	or CLASSR =    , which will do the above by default.
c
c
C2	SIGNSEG
C
C	Specifies a list of the segments containing species signature information
c	generated from individual tree crowns and training areas by ITCSSG.
C	The species information is used by the maximum-likelihood classifer to
C	classify the individual trees.  The signature segment contains the information
C	for each of the possible signature types.  Only the necessary information
C	is used by the ML classifier, depending on the specified SIGTYPE value.
C
C       Note that the segment type number for ITC signatures is now 140 
c	(140 = standard PCI TEX type segment) and will now show under
c	Imageworks PCI file browser. Unfortunately, as such, these segments
c	do not get moved properly by PCI software. (*NOTE*: They will appear
c	to have been moved properly, but the segments will be devoid of content)
c	To move or copy these signatures, please use ITCSSM in non-merging mode.
C
C2	CLASSBIT
C
C	Specifies a list of segments to which the generated bitmaps (the results
C	of the classification) will be written to.  If a full segment list is
C	not specified, or if a segment is not valid, a new segment at the end
C	of the file will be created to contain the information.
C
c	Optionnaly, for research purposes, when the bitmap of automatically  
c	delineated crowns (ISOLBIT) is replaced by a vector layer of 
c	manually delineated crowns, class bitmaps (CLASSBIT) are not generated.
c	Instead, the resulting class for each tree is stored in a new attribute
c	called CLASSR in the vector layer.
C
C2	CLASCODE	Optional class codes
c
c	This parameter allows the user (if desired) to enter a specific class
c	code (e.g., 183) for each class to improve reporting of classification 
c	results in output layer (when ITCSC is used in vector mode). 
c	Otherwise, classes are reported with just an index code corresponding
c	to their order (0-15) in SIGNSEG.
c
C2	EXTBIT
C
C	Specifies a segment which contains an External Bitmap Mask.  This is an
C	additional bitmap which is used to indicate which subparts of the crowns 
C	should be used to generate the signature (e.g., LIT side),
c	Generally, this is a bitmap generated by the ITCMG program. 
c 
c	For example, say a bitmap of type LIT is generated by ITCMG and specified
C	as EXTBIT.  The bitmap points out the pixels that form the lit sides of
C	all of the tree crowns. When the tree is being processed, only those pixels
C	that are in the LIT mask are used to create the individual tree signature.
C	Nevertheless, after the classification, the full crown will be depicted 
c	(painted) in the classifier's output BMs.
C
C
C2	SIGTYPE
C
C	Specifies which type of signature is to be used.
C  	Possible types include:  MEAN/TCL/TCL2/TEXTUR/STRUCT.
C
C	Signature Description
C
C	MEAN:	Multispectral mean of each ITC were averaged to create 
c		species signature. MEAN can be used with EXTBIT to create
c		LIT side MEAN and TT (tree top) MEAN signatures.
c		Multispectral mean of each ITC will be compared with each
c		species mean of means to decide (ML) on species class.
C	TCL:	First eigen vector & corresponding intercept of each ITC 
c		were averaged to create the species signature.
C	TCL2:	First eigen vector & corresponding intercept & eigenvalues
c		were averaged to create species signature.
C	TEXTUR:	Multispectral variance of each ITC (a simple texture parameter)
c		were averaged to create species textural signature.
C	STRUCT:	3D moments (now Kurtosis) of each ITC
c		were averaged to create species structural signature.
c
c	Version 2.0 introduced these all new redefined SIGTYPEs to ITCSSG/SC.
c	The user can select more than one SIGTYPE, for exemple (m=3), 
c		SIGTYPE = MEAN, TEXTUR, STRUCT
c	A covariance of appropriate size (mn**2), describing the covariance
c	of each feature selected was created by ITCSSG and added to the vectors.
c	If the user wants other signatures, than ITCSSG is run again to  
c	create other segments. (Here, n is number of channels).
c
c
C2      TREETYPE        Tree Type:  ITC/TT/SLAYER
C
C	Secifies whether signature generation (SSG) and classification (SC)
c	are to be done based on a full crown per tree (type = ITC) 
C	or only one pixel per tree crown (type = TT), the so called
c	"tree tops" type of ITCMG (local maximum *within* an ITC).
C	The default is ITC. TT can only be used with SIGTYPE=MEAN
c	and an external bitmap (EXTBIT) of tree tops.
c	LIT and SHADE external bitmaps are used with TREETYPE=ITC.
c
c	SLayer -> Permits the classification of manually delineated tree 
c		  crowns (vector polygons) from a single input layer found
c		  in ISOLBIT (see info. under ISOLBIT for details).
c
c
C2	THRESHLD
C
C	Specifies a classification threshold level for the maximum-likelihood classifier. 
c	Values may be between 0. and 1. (typical values are 0.95 and 0.99).
c	Trees that are outside the confidence interval (e.g., 0.95, 0.99) of the class 
c	they are "the closest to" are left unclassified.
c	For NOW, leaving this value empty is equivalent to entering a value of 0.99.
c	A value of 1 indicates that all trees should be classified (i.e., a tree is assigned
c	to the closest class independent to how far from its cluster it may be). 
c	This is not appropriate when some significant classes are known to be missing.
c	For exemple, having no hardwood class when hardwood are known to be present.
C
C2	DBOUT
C
C	Specifies the Output Database Filename. If a filename is specified, the individual
C	signature information for each tree crown is written out to a text file.  Signature
C	information for the specified SIGTYPE is written in each case, except for a SIGTYPE
C	of ALL in which signature information for all types is written.  If no filename is
C	specified, then no database is created.
c	Mostly used for debugging and in depth analysis.	FILE COULD BE HUGE !!!
C
C2	REPORT
C
C	Specifies the file to which the generated report should be appended.
C
C
C
C1	DETAILS
C
C	ITCSC
C
C       ITCSC classifies individual trees in an image into different species, based
C       on a Maximum-Likelihood (ML) classification of the individual tree signature,
C       compared to the signature of the species.  The species signatures are those
C       which were produced by the ITCSSG program.  A series of bitmaps are produced
C       indicating the results of the classification.  One bitmap per species is
C       generated showing the positions of the trees as a mask to the image.  A database
C	of the individual tree signatures may be generated.
C
C
C2	GENERAL PROCESS
C
C	The following general process is used to classify the trees:
C
C	The channels of the image are loaded in from the specified image file.  As well, a
C	bitmap mask from ITCISOL (Individual Tree Crown Isolation) is loaded in (ISOLBIT).
C	The signatures that were generated by ITCSSG are also loaded (SIGNSEG).  
c	As well, if an External Bitmap Mask (EXTBIT) was specified, it is loaded.  
c	The signatures that were
C	read in are checked and are possibly modified to allow only a subset of the images
C	channels (that were originally used) to be used in the classification.
C
C	Next, the signature information for each individual tree in the bitmap mask (ISOLBIT)
C	is generated.  The ISOLBIT mask is scanned until a tree is found.  Once a tree is
C	found, its signature is build up.  Information is gathered and generated, depending
C	on the signature type that was specified.  As well, pixel information is gathered
C	depending on the EXTBIT value.  If specified, the pixel must also be in the External
C	Bitmap Mask in order to be used, otherwise it is rejected.  If there is no EXTBIT
C	value, all pixels in a tree crown are used to gather and generate signature
C	information.  This process repeats for all trees in the ISOLBIT.
C
C	As well, for each tree a multi-spectral variance is calculated.  This information is
C	not used by the classifier, but is included in the output database.
C
C	When all the trees have been processed a global covariance matrix is generated for the
C	image.  Then, the trees are classified.  If the SIGTYPE specified is ALL, then the
C	classification process is skipped.  Otherwise the trees are then processed one at a
C	time, and classified according to a maximum-likelihood classifier.  The signature of
C	the individual tree is compared with signatures of the tree species and classifcation
C	decisions are made based on that information.  In order to be used by the ML classifier,
C	the signature is converted to a vector.  Only the parts of the species signature which
C	are needed for the specified signature type are used by the ML classifier.
C	There is however a special case with the MCOV signature.  In the current MCOV
C	classification only the first three channels worth of covariance data are used.
C	That is, if we have a large covariance matrix from the MCOV signature, only the upper
C	left 2x2 matrix is used as part of the vector.  The entire covariance matrix from
C	MCOV is loaded in, but only the values needed are used.  The entire matrix is kept,
C	allowing for possible future changes to this classification.  As well, there is a
C	threshold level (THRESHLD) specified by the user, which is taken into consideration
C	by the classifier when classifying the trees.  When the patterns have been generated
C	by the classifier, those which have probabilities for all classes below the threshold
C	are not classified.  Once all the trees have been classified, a series of
C	bitmaps are generated based on the results of the classification.
C
C	One bitmap is generated for each of the species types.  The list of trees is traversed
C	again one at a time.  Depending on how the tree has been classified, its position is
C	determined, and then its crown shape is copied from the ISOLBIT onto the individual
C	bitmaps.  Basically, the bitmaps a type of partition of the ISOLBIT, with each bitmap's
C	trees being mutually exclusive of trees of the other bitmaps.  If all the generated
C	bitmaps are overlayed, we have the original ISOLBIT again.  The bitmap generation then
C	essentially break apart the ISOLBIT into individual bitmaps based on the classification
C	results.  These bitmaps are then written to the database file into either specified or
C	newly-created segments.
C
C	If a DBOUT filename is specified then the individual tree information for all the
C	trees is written to a text file.  If the SIGTYPE is ALL, then the classification
C	process was skipped, no segments were written, and only the output database is created.
C
C
C2	BITMAP MASKS
C
C	ITCSC used two different bitmap masks as input.  One of these is an isolation
C	bitmap (ISOLBIT) which is produced by the ITCISOL program.  This is used to
C	determine the locations of the individual trees in the image.
C
C	The second bitmap is the External Bitmap Mask (EXTBIT).  This is used by the
C	program to determine which of the pixels in the tree should be processed in
C	order to generate the individual tree signature.
C
C	The ISOLBIT is scanned until a tree is found.  Once found, it is processed
C	and the infomation in the tree used to form its signature.  If no EXTBIT is
C	specified, then the information from all pixels contained in the tree are used.
C	If an EXTBIT value is used, only pixels that are also in the EXTBIT are kept for
C	that tree.  This allows parts of the tree (for example the lit side) to be used,
C	while part of it is rejected.
C
C
C2	CLASS SIGNATURES
C
C	The following class signatures may be used by ITCSC to classify the trees:
C
C       Tree Average (AVG)
C       The mean multispectral vector of all pixels contained in a tree crown.
C
C       Mean vector & first principal component (MPC1)
C       The "Tree Colour Line" approach in multiple dimensions - the distribution
C       of pixels for each tree crown is represented by its first eigenvector
C       (its colour line direction) and its mean vector (to anchor the line in
C       multidimensional space).
C
C       Mean vector & first principal component & eigenvalues (MPC1EV)
C       As above (MPC1) but eigenvalues are added to describe the spread of the
C       distribution in the various directions.
C
C       Mean and covariance matrix (MCOV)
C       The mean and covariance matrix of the distribution of pixels in
C       multispectral space for each tree crown.
C
C	Species signatures those that have been generated by the ITCSSG program.  They
C	contain values which may be used my the ML Classifier with any of the above
C	signature types.  When the ML Classifier uses a species signature it takes the
C	information it needs, to match the information of the specified signature.
C	By having ITCSSG generate information to classify by any of the above types,
C	classifications can be re-run with different specified types, without having
C	to generate another species signature for that specific type.
C
C	When a species signature is read in by the system it is checked that all the
C	information contained within it is valid and may be used by the classifier.
C
C	As well, a subset of the species signature may be used.  If the user specifies
C	a set of channels for classification which is a subset of the total number
C	of channels used in classification, the parts of the signature that are
C	needed is modified to accomodate this change.
C
C
C2	MAXIMUM LIKELIHOOD CLASSIFICATION
C
C	ITCSC uses a maximum-likelihood classification method in order to classify
C	the trees to a particular species.  A global covariance matrix is created for
C	the image, based on the information contained in the various trees.  This is
C	used by the classifier.  As well, the signatures of the various species which
C	had been generated by the ITCSSG program are loaded in.  The signatures for
C	each of the individual trees in the image are generated.  The general formula
C	is:
C					    t      -1
C		fi(x) = -ln|Sigma| - (x - mi) (Sigma) (x - mi)
C
C	where,
C	x is a vector of the signature information for the individual tree to be classified,
C	mi is a vector of signature information for the ith species,
C	Sigma is the global covariance matrix.
C
C	fi(x) is calculated for each species.  The values are compared and the species
C	which generated the greatest value becomes the classification type of the tree.
C	That is fi(x) > fj(x) for all j >< i.  This process is repeated for all of the
C	trees in the image.
C
C	A threshold value (THRESHLD) may also be specified by the user.  This adds the
C	condition that fi(x) > Ti, where Ti is defined as:
C
C		Ti = -(Chi Squ.) - ln|Sigma|
C
C	Chi Squ. is the critical value (P) of Chi Squared with degrees of freedom equal to
C	the number of input channels and area under the curve on the right of  P
c	given by (1- THRESHLD) (i.e., THRESHLD correspond to curve main area on the left)
C
C
C2	OUTPUT DATABASE STRUCTURE
C
C	Is a DBOUT filename is specified, then the signature information for individual
C	trees is written to a text file.  If the SIGTYPE is ALL, then information for
C	all the signature types is written.  The database will have the following
C	structure for each entry (individual tree):
C
C	number pixel line UTMpixel UTMline crown_area area_pixels classification
C	multi-spectral channel mean values
C	multi-spectral channel variance values
C	eigenvectors and eigenvalues
C	covariance matrix
C
C	where:
C	'number' is the assigned tree number
C	'pixel','line' form the position of the tree in the image
C	'UTMpixel', 'UTMline' are the converted coordinates taking georeferencing information
C		into account
C	'crown_area' is the area of the tree in METERS
C	'area_pixels' is the area of the tree in pixels
C	'classification' is the species number this tree was classified as - not present if
C		SIGTYPE is ALL
C
C	The other lines in the database refer to values created by different signature types.
C	Some or all of these may be present, depending on which signature type was specified.
C
C	Also note that multi-spectral variance is generated for each tree but is not used (yet)
C	in any of the classifications.
C
C1	HISTORY
C
c	Fran�ois A. Gougeon, Ph.D.,
c	Digital Remote Sensing Research
c	
c	Dept. of Natural Resources,
c	Canadian Forest Service �
c	
c	Pacific Forestry Centre,
c	506 West Burnside Rd.,
c	Victoria, British Columbia, 
c	Canada, V8Z 1M5
c	
c	formerly at:
c
c	Petawawa National Forestry Institute
c
c Revision History:
c
c  Version 0.9 Sept 94	Ron Petrick (@PNFI)
c			- Prog. creation (concept Francois Gougeon)
c
c  Version 1.0 Nov 94	
c			- ANSI C Conversion, upgrade to PCI V5.3
c
c
c  Version 1.1 Dec 96	Simon Alexander (@PFC)
c			- Convert to handle 16bit images, clean
c			up memory handling, start made on
c			integrating itc suite.
c
c  Version 1.2	Feb 97	Fran�ois Gougeon (@PFC)
c			- Various cleanups of old code and debugging,
c			change in parameters nomemclature
c
c  Version 1.3	May 97	
c			- To speedup by using macros instead of functions
c			for bit operations and 2x2 detection of minimun
c			crown size
c
c  Version 1.4	Aug 97	
c			- To use covariance instead of correlation matrix
c			when calculating eigenvectors-based signatures.
c			(Maybe the "mD tree color line" approach has not been
c			as succesful as predicted because of this error. Pre-
c			ITC-suite work had been done using the covariance. It
c			is more representative of the color line, because the
c			correlation matrix normalizes everything.)
c
c  Version 1.5	Jan 98	
c			- To easily switch, by commenting out diff. lines,
c			from using global total covariance matrix,
c			to using global species covariance matrix, 
c			to using species specific covariance matrix,
c			to using Mahalanobis distances.
c					
c  Version 1.6	May 98	
c			- To do the above via a single variable (cov_mat_type).
c			By default now, species specific covariance matrices
c			are used.
c
c  Version 2.0	June 98	
c			- Introduced all new redefined SIGTYPEs to ITCSC. They are
c			MEAN/TCL/TCL2/TEXTUR/STRUCT. The user is now 
c			able to select more than one SIGTYPE. 
c			For exemple, SIGTYPE = MEAN, TEXTUR, STRUCT
c			A covariance matrix of appropriate size, describing the covariance of
c			each feature selected is created.
c			Now ITCSSG must be run every time before ITCSC to create specific
c			signatures, before they were all created in one ITCSSG run.
c			SIGTYPE = STRUCT not implemented yet (no time).
c
c			- In version 1.x, all signatures were generated at the same
c			time and a specific signature type was only selected for
c			running ITCSC. This meant that one could not have a "species"
c			covariance matrix of the ITCs, only a "global"
c			covariance matrix could be used (generated from all ITCs in
c			ITCSC at running time) and the cov. matrix with the signatures
c			were that of the pixel covariance, not the ITC covariance.
c			In previous versions, SIGTYPE were AVG/MPC1/MPC1EV/MCOV/ALL.
c
c
c Version 2.1	July 98	
c			- New trees are NOW added to the bottom of the list.
c			(Past versions added to the top of the list, making 
c			tree numbers incompatible with program that are just 
c			scanning the image)
c			- ITCs with less pixels than the dimension of the
c			multispectral space (# of channels) are not used
c			and are reported as such (i.e., unused).
c			- Introduced tree type parameter (TREETYPE = ITC/TT)
c			to simplify dealing with TT --> the only situation where
c			a single pixel per tree is used and eigens() is not called.
c			- Org. to bypass using EXTBIT if user still in pointer mode
c			(BITBOUND=POINTS) because of high probabilty that EXTBIT bitmap
c			contains only pointers used in ITCSSG and not data to process 
c			exclusively like in LIT, TT or SHADE case. LATER, we could have 
c			a pointer mode that points to the only tree crowns to classify
c			(but that could be more confusing). If it came to that, we would
c			have to really separate EXTBIT and a POINTBM. (By-the-way,
c			EXTBIT, ISOLBIT and others, should be renamed EXTBM, ISOLBM, ...
c			because these are more appropraite names.)
c
c
c Version 2.2	Oct 98	
c			- Adding a structure signature (STRUCT) based on 3D moments,
c			similar to Yi Min Yan and Geoffrey Edwards work at Laval Univ.
c			as seen in AQT, May 96 paper, CD-ROM Proceedings.
c			- Function itc_structure() was introduced (Kurtosis is prefered).
c			- Removal of function process_tree_sums(), moved its functionalty
c			into process_tree_image_data();
c
c Version 2.3	Apr 99	
c			- Accepting the use of ranges (e.g., 2,-6) in SIGNSEG and CLASSBIT
c			  and allowing up to 32 classes(signatures).
c
c Version 3.0	June 99	
c			- To allow manual tree crowns (vector polygons) to be used as
c			  input, instead of ITC Bitmap (ISOLBIT), and be classified.
c			  Class bitmaps (CLASSBIT) are not generated. Instead, resulting
c			  class for each tree is stored in a new attribute called
c			  CLASS in the vector layer. ( * RESEARCH * )
c			- Also introduced the element "class" in the structure "tree", to
c			  properly keep track of classification (process_count was used)
c			  leaving process_count available to report on # LIT pixels used
c			  in the dump file. So count, process_count, and class are
c			  reported for every ITC in the dump file.
c			- To allow user to select a name for the new field (rather than CLASS)
c
c Version 3.1	June 99	
c			- Introduced SPCODE, to allow the user to enter specific species codes
c			  for each class (if desired) to improve reporting of classification
c			  in output layer when ITCSC is used in vector mode. Otherwise, classes
c			  are reported with just an index code corresponding to the order (0-15)
c			  used in SIGNSEG.
c
c Version 3.2	June 99	
c			- Introduced parameter SCHINDX to select the channels to be used  
c			  out of the signatures (using index 1-8) that will match spectrally
c			  with the DBIC channels (for signature extension work)
c
c Version 3.3	Sep. 99	
c			- Removed requirements of minimum of 2x2 block for
c			  manual (vector) tree crowns.
c			- Maintained requirement that center of gravity
c			  must be inside tree crown for manual crowns.
c			- Counting and reporting on tree crowns that are
c			  not meeting the above criteria with "unusedITC".
c
c Version 3.4	Sep. 99	
c			- Reorganized so that parameter SCHINDX may or may not be used.
c			  Channel subseting and reordering are deal with automatically as  
c			  in versions previous to the introduction of SCHINDX (< v3.2).
c			  SCHINDX need only be used if channels differ between signatures
c			  and the present DBIC of this ITCSC run.
c			- Display of tree numbers to show progress when using input layer.
c
c Version 3.5	Sep. 99	
c			- Organized so that when using a manual tree layer (as ISOLBIT)
c			  instead of an ITC bitmap (for research) which should get ITCSC to  
c			  prompt the user for a field name in which to put the classification 
c			  results, if a hidden parameter CLASSR is set to a field name (rather
c			  than its default of NOTSET), the user will not be prompted and the name
c			  specified by CLASSR will be used. This is very useful when ITCSC is
c			  to be run in automatic, unattended scripts, as these scripts have no
c			  capability of answering questions from "C" programs.
c
c			  - Just define CLASSR in advance of running ITCSC in your script:
c			  	CLASSR = "class_lit_3ch"
c				run ITCSC
c
c			  - If you'll rather be prompted, make sure CLASSR = "NOTSET"
c			  or CLASSR =    , which will do the above by default.
c
c
c Version 3.6	Sep. 99 
c			- Minor modifications so STRUCTURE signatures function properly with
c			  ITCSC when a subset of channels is used via SCHINDX or otherwise.
c
c Version 3.61	Sep 99	
c			- Parameter SPCODE changed to CLASCODE (more appropriate) and to 
c			  synchronize with ITCSSG(v3.7) and ITCMARA(v2.91). Class code is
c			  better because we often deal with species under various situations,
c			  like: in open or close canopy stands, healthy trees or not, 
c			  normal or brighly lit ...
c
c Version 3.62	Oct 99	
c			- To enforce the use of TREETYPE="SLAYER when a single vector layer
c			  of manually delineated tree crowns is used in ISOLBIT. Previously,
c			  ITCSC was deciding based on the segment type of ISOLBIT.
c			  This mod. makes things more verbose (for batch runs) and thus, safer
c			  overall. User intentions are clearer. They are float checked.
c
c Version 3.7	Dec 99	- Mod. to allow for less than three (1 & 2) channels classifications.
c			  (The important mod. was to cmatrix2.c to calculate one and two dimension
c			  determinant - following a need in ITCSSBD)
c
c Version 3.8	Aug. - Sept. 2000
c
c			- Various cleanups to run on PCs (1st ever port to PC world)
c			- args, argcnt, report and some others can not be global vars.
c			  (i.e., just doing status on "TASK" gives a memory access error)
c			- Problems with vars. that are global being also passed as
c			  parameters in functions calls.
c			  (i.e., same type of memory access error)
c			- Making the "tree list" a global variable
c
c
c Version 3.9	Feb. 2001
c
c			- Org. to have safety buffer around ITC bitmap in case it was
c			  generated by subsetting of a bigger image and thus, lost it.
c
c Version 4.0	Feb. 2001
c
c			- Modified ITCSC to run on big images by adding the function
c			  process_tree_cleanup() containning the free() function.
c			  ITCSC could not run on big images because it was continuously
c			  accumulating MSS pixel data and never cleanning up memory.
c			  Data accumulating under info_list could occupy on average 
c			  3.5 KB per tree. The tree list is also a problem, using about
c			  5.5 KB per tree. For big images, ITCSC runs out of memory and
c			  then out of swap space and crashes. Just running out of memory,
c			  (and calling on swap space) is annoying because it can increase
c			  processing time by a factor of 7-10. 

c			- MAJOR REORGANIZATION to run on big images. The list of ITC signatures 
c			  and their info_list (raw data) were running out of memory.
c			  ITCSC (<v4.0) was creating a full list of ITCs signatures and then,
c			  classifying them one by one by going through the huge list.
c			  So, reorganization to create an ITC signature and classify it immediately.
c			  It is still possible to get an ITC signature list if needed when DBOUT is
c			  present, but it will only work on small research type images.
c			- Lost possibility of using "global total" covariance matrix mode,
c			  however, it was not being used anyway.
c		
c
c Version 4.1	Sept. 2001
c
c			- Mod. to deal simultaneously with 8 and 16 bit images.
c	
c Version 4.2	April 2002
c
c			- Removed sorting ofchannel in ascending order (i.e., qsort)
c			because it was screwing-up with SCHINDX that allows for signatures 
c			generated with different channels or channel order.
c
c Version 4.3	Nov. 2002
c
c			- Modified for better determinant (detlu()) calculations
c
c
c Francois Gougeon	v4.4	April-May 2007
c
c				- Minor adaptations for PCI v10 and its new PRM.PRM file 
c				  Most PCI routines are in PCI1000.dll, but not all.
c				  For example, IMPTime() and IMPReturn() are now in Core1000.dll
c				  and IMPCounter() in Counter1000.dll
c				  Since I dont have PCISDK or PCIProSDK, I had to create LIBs 
c				  from their DLLs to compile my progs.
c
c				  Also, problem with the REPORT (*Report) Variable
c
c Francois Gougeon	v4.5	Sept. 2007
C
c				- Updated to be more flexible with number of vertices per polygon
c				  (previous version was limited to 150 vertices of storage)
c				- Fixed a problem with the convertion from georeferenced vectors to
c				  image coordinates (i.e., remove two fabs() in equations), mostly
c				  so that Northing diminishes with increasing Y image positions.
c				  (i.e.,  transformY should always be a negative number)
c
c
c Francois Gougeon	v4.6	Sept. 2007
c
c				- Fixed a bug that made ITCSC crash with LIT mask.
c				- Modernized (GDB rather than IDB) class bitmap writing (write_segment)
c				  Because this part was occasionally crashing the program, 
c				  as it was asking for additional memory (a temp buffer)
c				- Reorg output segments descrition and history
c
c
c Francois Gougeon	v4.7	Dec. 2007
c
c				- Not a real modification of the program, but a factor in the compiling
c				  and linking of the program that allow the task to use more memory (up to 3GB)
c				  under a 32-bit Windows XP (max. 4GB), if allowed by the OS (and if you have 4GB)
c				- This is done by using the /LargeAddressAware switch on linking the prog.
c				- The OS allows this if the switch /3GB is used in boot.ini for that OS.
c				  This changes the default 2GB for applications and 2GB for the OS
c				  to 3GB for applications and 1GB for the OS (REBOOTING IS NEEDED)
c				- You can get to the boot.ini file from the System control panel, then
c					System Properties/Advanced/Startup&Recovery_settings/Edit
c
c
c Fran�ois A. Gougeon	 v5.0	 July 2009	
c
c				- Modified to handle bigger images (bigger than available memory)
c				by reading and analysing images and bitmaps by sections, with of
c				course, some overlap between section to let crown fill and/or
c				refill function properly.
c				***** This implied a major reorganization of the code.
c				NOTE:	Manual ITCs (in vector form) classification is not available when
c						the image has to be treated by sections.
c
c
c
c Francois Gougeon	v5.1	Sept. 2009
c
c				- Change (a bit) the nature of the textural and structural signatures in that
c				they only gather their info. from the first channel (not all channels).
c				Getting it from all channels was a bit redundant and created big covarinace
c				matrices. Creating all these redundant channels (as many as the no. of multispectral 
c				channels) was puting too much emphasis on these features at the expense of
c				the multispectral features, often leading to worse classifications.
c				In addition, getting texture from all channels was preventing us to use
c				texture at the same time as a crown size feature (from ITCAREAS), as
c				such feature has no texture (creating singular covariance matrices)
c
c				This changes the nature of the ITC signatures (in the signature segments),
c				and may make older signatures incompatible with the newer software. 
c				They have to be regenerated (by rerunning ITCSSG). 
c				NOTE: Most of the same info. is there, but the cov. matrix "content" is smaller.
c						HOWEVER, lots of the info was changed from "double" to "float" making it
c						seriously incompatible with previous versions.
c				NOTE: This affects ITCSSG, ITCSC and ITCDSI, which all have to be compatible.
c
c
c
c Francois Gougeon	v5.2	Sept. 2009
c
c				- Made sure that using a subset of features to classify the image
c				(compare to the features in the signatures) worked well.
c				That option had been neglected in some past revisions of ITCSC.
c				This way you can generated signatures with all the features available 
c				(e.g., four CHs, a crown area feature, MEAN(lit), TEXT, STRUCT) and just 
c				play with runs of ITCSC & ITCCA to test various comparaisons.
c
c				- Fixed a bug with the creation of new output bitmaps
c
c Francois Gougeon	v5.3	Dec. 2009
c
c				- Fixed a bug -> When working by image sections (because image was too big for 
c				existing computer memory), the program was iterating on the last image section 
c				if that section was only a few (2-3) lines in size.
c
c Fran�ois A. Gougeon	 v5.4	 Dec. 2009	
c
c				- Taking 16bit images into consideration when calculating image sections
c				to fit available memory. It assumes 1GB available for images, and about
c				1GB for available for bitmaps, the program, and odds (very crude).
c
c Fran�ois A. Gougeon	 v5.5	 March 2010
c
c				- Making better calculations to use images "by sections" by reading the
c				amount of memory that the computer actually has at its disposal.
c				- Capable of dealing with FILENAME (thus path) up to 132 chr long. as per 
c				new standards (PCI > v10.n)
c
c Fran�ois A. Gougeon	 v5.6	 July 2010
c
c				- Making better calculations when using images and output bitmaps "by sections"
c				  (i.e., previous version was doing output bitmaps by sections, but considering them
c				  whole while calculating available memory for image sections).
c				- Also, if all fails, the user can now impose the quantity of total memory
c				  to use (within XP limits, at the moment) using the new ITC/PCI environment variable
c				  named ITC_MEM (specified in GB, e.g.: ITC_MEM = 2.5). If not already in DEFITC.EAS,
c				  use the following line to define the new PCI env. vars. :
c				  define ITC_MEM=n,1.0,3.0,-1,"Memory to use (in GB) according to user"
c
c Fran�ois A. Gougeon	 v5.7	 Jan. 2012
c
c				- Modified to handle bitmaps bigger than 2G positions by reading them
c				in two sections (i.e., the old PCI functions that I have access to
c				are not up to part yet). Itc_io.c was modified to take care of that.
c				- Bitops.c was also modified to use int64 vars to use with pointers 
c				for operations on bitmaps. Calls to setbit(), testbit(), etc.
c				have to be modified to use an (int64) pointer, typically by casting.
c				- Thus, "bitnum" need to be declared "int64 bitnum" everywhere
c				- Be careful of parenthesis as they may prevent "auto casting" to int64
c				from working (i.e., calculation gets done in int32, then cast to int64
c				with bad results. For exemple:
c				"bitnum = (int64) ((y-1)*Pixels) + x-1;" 
c				should be: "bitnum = (y-1)*(int64)Pixels + x-1;"
c
c Fran�ois A. Gougeon	 v5.8	March 2014
c
c				- Problem in SLayer (unused for years): WoW64 does not like HFree()
c				- No need to create output bitmaps as output (in SLayer mode) goes to a field
c				of the input vector layer of manually delineated trees.
c				- Other debugging (i.e., not working with LIT mask)
c
c Fran�ois A. Gougeon	 v5.9	April 2015
c
c				- Fixed to have "SLayer mode" functionnal with big images done by sections
c				(worked with 10,814 manual trees in a 75,000 X 27,500 image done by 4 sections)
c				- To handle better when using an existing "output field"
c				- Fixed to create a single treelist (even though by sections) using DBOUT
c				- Fixed some issues of "needed memory" calculations
c				- HINT: Always try to keep image section memory need to < 2GB
c				For example, if it is known that BMs will take 0.9GB, set ITC_MEM=2.9
c

c Fran�ois A. Gougeon	 v6.0	April 2015
c
c				- Fixed "SLayer mode" for instabilities: occasionally crashed on crowns 
c				close to image section boundaries.
c				This was due to unused crowns (bypassed) in previous section still having a presence
c				in the internal bitmap (i.e., not erased, just bypassed), a presence that connected
c				with current crown, leading fill() out of the image section.
c				- Solution: decision to bypass made prior to burning (vect2rast) in bitmap
c				- Mod. for CLASCODE to deal with up to possibly 32 individual codes
c
c
c Fran�ois Gougeon	v7.0	April 2016
c
c			- Changed to a C++ program to deal with versions of PCI > v10.2
c				int main (), ".cpp" name, and extern "C" around .h include files
c			- Mods to some call to fit new library definitions (as per .def demangling)
c			  which also means mods to the corresponding declaration .h files (e.g., gdb.h)
c			  (Since I dont have PCISDK or PCI/ProSDK, I have to create LIB and DEF from
c			  from their DLLs (via DUMPBIN and LIB) in order to compile my progs
c			- Modules in files like gdb.h need to be declared 'extern "C++" to link
c			  with proper name mangling (MSVC++ mangling)
c			- ***WORKED**  for PCI 10.3 (a few tests) that migrated its LIB to C++, 
c
c			- HOWEVER, more stuff is needed for PCI 2015 (64bit)
c			- PCI LIB need to be /MACHINE:x64 
c			  and prog compile with 64b version (i.e., vcvarsall amd64)
c			  cause all the call to lib are now with 64bit pointers 
c			- IMPStatus()  and others uses "char const *"
c
c			- Minor formatting mods to blas.c z.c chisq.c cmatrix2.c for the more
c			rigourous C++ standards (e.g., "float poz(float z)" instead of "float poz()"
c			with a "z" declaration on the following line) 
c			- "Class" is a reserved word (a variable type) in C++, so I used "classI" in
c			the tree stucture and class_sum[] instead of class[] to sum results
c
c
c
c Fran�ois Gougeon	v7.1	Sept 2016
c
c			- Had forgotten to do (int64)bytenum everywhere
c			- bmsize needs to be (int64) everywhere to deal with bitmaps > 2GB
c			- using new write_bmp() from itc_io.cpp to write bitmap > 2GB
c
c
c Fran�ois Gougeon	v7.2	Nov. 2016
c
c			- ITCSC Was CRASHING due to a huge ITC (here, borders of agri. area, often borders of roads)
c			However, in previous version of Suite (PCI 10.1, MinGW) ITCSC was not crashing 
c			on the same dataset (ITCs in that dataset had been done with programs made with MinGW compiler)
c			The MAIN ISSUE is that MinGW reserves 2MB for its stack and VS14 only 1MB
c			Sol.:  set LINK= ..... /STACK:0X200000
c
c			NOTEs: 
c			- The whole 2015 version of the Suite (VS14, MSVC++, 64 bit) should be recompiled
c			with the above to be more compatible with previous results.
c			- Also, programs could check for "overgrown" ITCs (see SIZE_LIMIT here)
c
c Fran�ois Gougeon	v7.3	Jan 2017
c
c			- To ensure that structural signatures are based on FULL ITC while LIT is used with MSS data, I
c			  replaced "info_list" by "Proc_pix_list" to be clearer (list of "process" pixel multispectral values,
c			  which may be of full ITC or ITC LIT side within training areas (or designated ITCs) depending on EXTBIT) 
c			  and, to be able to carry also "FITC_pix_list" (the "full ITC" list of pixel multispectral values)
c			  using new function "process_ITC_image_data()". BTW, because "ITC" and "tree" were used in variable names,
c			  I started using "Fitc" to differentiate FULL ITC info from process ITC info (which could be LIT-based)
c
c			- Introduced SIZE_LIMIT to make sure that fill() does not blow the stack.
c			  Will report blobs bigger than SIZE_LIMIT and continue counting them as unused
c			  Note: Left-over parts of that blob may qualify as trees and get classified.
c			  However,it's better than crashing the process all the time one such blob is encountered
c			  Theoretically, these blobs should not have made it this far and should be part of NFMASK.
c
c			- Introduced (a special research mode) the use of PCI env. vars. SC_CLASS2 to get 2nd most likely class
c			  instead of first (i.e., 2nd big likelihood). Only works in "Slayer mode" with manual tree polygons.
c
c
c
c Fran�ois Gougeon	v8.0a	March-July 2020
c
c			- Modified program for the GDAL environment and for variables on the command line
c			from ITCSC.cpp v7.3 coded for the PCI environment (prog. now called itcssg_g.cpp)
c
c			- Program has tons of inputs parameters and tons of variations thereof to deal with
c			See program usage (not everyting is implemented yet)
c			
c			- Functionning by image sections (rather than full images) has not been updated	
c				and may not be as computers are now bigger and better at managing memory
c
c
c Fran�ois Gougeon	v8.1a	Aug. 2020
c
c			- Mostly organized for SLAYER mode with PCI file and TIF main file (thus, .shp, .sig)
c
c
c Fran�ois Gougeon	v8.2a	May 2021
c
c			- Modified program for complete independence from the PCI environment (no call to lib)
c
c			- Introduced more flexibity to specify input files, specially signature files
c
c Fran�ois Gougeon  v8.3a		April 2023
c
c			- NOT to assume that all files are in the default directory from which the program is run ANYMORE
c				Previously, everything was assumed in same directory and run from a cmd window from there.
c				NOW, 
c				the **path used with the main input file** is used when creating the default input/output file names
c				This was necessary for ArcGIS Toolkit integration.
c
c
c
c François Gougeon  v8.4		Nov. 2024  
c
c			- Introduced more path variables. 
c			When running from ArcGISPro where training areas (shp) are specified as just a string, 
c			(e.g. PRF_Vec.shp,6,-13) to be modified (i.e., PRF_Vec6.shp, PRF_Vec7.shp, ...)
C			path is unknown if not typed in directly (i.e., no default path)
C			Thus,default path is considered to be main file default path
c
c
c François Gougeon  v8.5		Aug. 2025
c
c       - To deal with Linux use of "slash" versus Windows "backslash", 
c           finally realizing that Windows can do both, so prog for Linux
c           and using path_len rather than jj
c		
c1	REFERENCES
c
c	Main signature generation reference:
c	
c	Gougeon, F.A. 1995. Comparison of possible multispectral classification
c	schemes for tree crowns individually delineated on high spatial resolution
c	MEIS images. Can. J. Rem. Sens. 21(1):1-9.
c	
c	Other ITC suite references:
c
c	Gougeon, F.A. 1995. A crown-following approach to the automatic delineation of
c	individual tree crowns in high spatial resolution aerial images. Can. J. Rem.
c	Sens. 21(3):274-284.
c	
c	Gougeon, F.A. 1998. Automatic individual tree crown delineation using a
c	valley-following algorithm and rule-based system. Proc. Int'l Forum on
c	Automated Interpretation of High Spatial Resolution  Digital Imagery for
c	Forestry. February 10-12, Victoria, B.C., Canada.  12 p.
c
c	Gougeon, F.A., D. Leckie, I. Scott and D. Paradine. 1998. Individual tree
c	crown species recognition: the Nahmint study. Proc. Int'l Forum on Automated
c	Interpretation of High Spatial Resolution Digital Imagery for Forestry.
c	February 10-12, Victoria, British Columbia, Canada. 14.p.
c	
c	Leckie D.G. and F.A. Gougeon. 1998. An assessment of both visual and automated
c	tree counting and species identification with high spatial resolution
c	multispectral imagery. Proc. Int'l Forum on Automated Interpretation of High
c	Spatial Resolution Digital Imagery for Forestry. February 10-12, Victoria,
c	B.C., Canada.  12 p. (In press).
c	
c	Gougeon, F.A. 1997. Recognizing the forest from the trees: individual tree
c	crown delineation, classification and regrouping for inventory purposes. Pages
c	807-814 in Proc. Third Int. Airborne Rem. Sens. Conf. and Exh., Vol. II.
c	Copenhagen, Denmark, July 7-10, 1997
c	
c	Gougeon, F.A. 1996. Vers l'inventaire forestier automatis�: Reconna�tre
c	l'arbre ou la for�t? In CD-ROM of Proc. 9�me Congr�s de L'Association 
c	qu�b�coise de t�l�d�tection, Qu�bec, May.1-3, 1996. 10 p.
c	
c	Gougeon, F.A. 1995. A system for individual tree crown classification of
c	conifer stands at high spatial resolution. Pages 635-642 in Epp, H.; Taylor,
c	C., eds. Proc. 17th Can. Symp. Rem. Sens, Saskatoon, Saskatchewan, Canada,
c	June 13-15, 1995.
c	
c	Gougeon, F.A. 1993. Individual tree identification from high resolution MEIS
c	images. Pages 117-128 in Leckie, D.G.; Gillis, M.D., eds. Intl. Forum Airborne
c	Multispectral Scanning for Forestry and Mapping (with Emphasis on MEIS).
c	Val-Morin, Qu�bec, Canada, April 13-16, 1992. Inf. Rep. PI-X-113, Forestry
c	Canada, Petawawa Natl. For. Inst.
c	
c	Gougeon, F.A.; Moore, T. 1989. Classification individuelle des arbres � partir
c	d'images � haute r�solution spatiale. Pages 185-196 in Bernier, M., et al.,
c	eds. T�l�d�tection et gestion des ressources Vol. VI - 6e congr�s de
c	L'association qu�b�coise de t�l�d�tection. Sherbrooke, Qu�bec, Canada, May
c	4-6, 1988.
c	
C-
C1  Dependencies:
c	
c	Libraries:	pcilib.a	(PCI EASI/PACE lib)
c			libm.a		(Standard Math Library)
c	
c	Includes:	pci.h		(PCI EASI/PACE )
c			eigens.c	(1991 Stephen L. Moshier)
c			blas.c		(1991 Stephen L. Moshier)
c			cmatrix2.c	(by Nigel Salt)
c			detlu.c		(from Numerical Recipes, Press et al.)
c			chisq.c.	(Gary Perlman, Wang Institute)
c			z.c.		(Gary Perlman, Wang Institute)
c			string.h	(Standard C string header file)
c			math.h		(Standard C math header file)
c			stdlib.h	(Standard C Library header file)
c 			itc_io.h	standard itc suite I/O routines
c 			bitops.h	to use macros for bit operations instead of functions,
c					 to speed up (less pushing and poping)
c
c
c	EASI/PACE is a copyright of:
c	PCI Enterprises Inc., 50 West Wilmot St., Richmond Hill, Ontario, CANADA
c
C
c********************************************************************************
c
c	PROGRAM USAGE EXAMPLE
c
#### GDAL PROGRAM USAGE:

In order to deal with simple numbers on command line, file nomenclature 
is often predefined, but always using base file name, here "SECTEUR"


>  itcsc_g FILE DBIC ISOLBIT EXTBIT SIGSEG SIGTYPE TREETYPE

>  itcsc_g SECTEUR_IRGB.tif 1,-3 10 11 12,-14 MEAN ITC

>  itcsc_g SECTEUR.pix 1,-3 10 11 13,-15

	Will pickup all info from the PCI file, but with GDAL channel ordering
	Except for SIGSEG which are the real PCI signature(text) segment numbers

>  itcsc_g SECTEUR_IRGB.tif 1,-3 10 11 12,-14 MEAN ITC

		Image channels 1,2,3 are in a tif file
		
		As per user input, ISOLBIT number to use: 10
		ISOLBIT file_in  :  SECTEUR_BM10.tif

		As per user, EXTBIT  number to use: 11
		EXTMASK file_in  :  SECTEUR_BM11.tif
		
		Signature file_in  :  SECTEUR_SIG_BM12.sig 
		Signature file_in  :  SECTEUR_SIG_BM13.sig		
		Signature file_in  :  SECTEUR_SIG_BM14.sig	
		
	meaning that signature were generated by "itcssg_g" from training area bitmaps 12,13,14	
		
		
### The output from the classifier will be bitmaps (1-bit tifs) named:

	SECTEUR_BM_Class1.tif, SECTEUR_BM_Class2.tif, SECTEUR_BM_Class3.tif, ...	
		
		
		
To classify individual tree polygons all in a single vector layer in a PCI or a shp file

>	itcsc_g SECTEUR.tif 1,-3 2 18 12,-14 MEAN SLAYER		// single layer is segm 2, LIT is BM 18

>	itcsc_g SECTEUR.pix 1,-3 3 - 13,-15 MEAN SLAYER		// single layer is third (but segm 20), sign are PCI TEX segm.


********************************************************************************

*************************************************************************

*** To compile with Visual Studio (VS14)  (see VS14_GDAL_compile.txt)


set INCLUDE=D:\GDAL_3.0.0\include;%INCLUDE%
set LIB=D:\GDAL_3.0.0\lib;%LIB%
set LINK=gdal_i.lib  libtiff_i.lib User32.lib
set CL= /MD
CL xxxxxxxxx.cpp /EHsc

set path=D:\GDAL_3.0.0;%PATH%		// to run programs

***  GDAL v3.0.0 has problem getting to its projection library, so do:

set PROJ_LIB=D:\GDAL_3.0.0\projlib


*********************
Ackowlegment to GDAL:
 
GDAL - Geospatial Data Abstraction Library: Version 3.0.0 (Dec. 2019, 64bit),
Open Source Geospatial Foundation, 
Thanks Frank (Warmerdam)


Still in transistion mode and some PCI left in it:

REM	**** For the moment **** (while still a lot of PCI functions in prog.)

set LIB=d:\pcisdk_v2017\lib;%LIB%

set INCLUDE=d:\pcisdk_v2017\lib;%INCLUDE%

set LINK=pcic201700.lib core201700.lib counter201700.lib %LINK%

REM 	To run programs in partial convertion mode (some PCI left in prog.)

set path=D:\pcisdk_v2017\lib;D:\PCI Geomatics\Geomatica 2017\exe;%PATH%

REM To compile:

>CL itcssg_g.cpp  eigens.obj  itc_io_g.obj bitops.obj itc_io.obj /EHsc




*/		// End of documentation

//#define PARTIAL_GDAL	1	// still in the process of converting, thus some PCI lib still needed
							// ALSO, important to resolve conflicts between gdb.h and itc_io_g.h
							// about GDBVertex


#define PROG_NAME "ITCSC_G"
#define VERSION "v8.5"
#define FILENAME 132
#define CHANNELS    8		/* max. number of channels that can be handled */
	/* ** DONT CHANGE CHANNELS ** as this affect signature size/org in sign segm. */
#define VECTORSIZE  CHANNELS
#define BITMAPS     32		/* max. number of classes that can be handled if param expand() used */
#define SIZE_LIMIT 40000		// limit on size of ITCs (in pixels), not to blow the program stack


#define SEGTYPEBASE 	140	/* Type of segment, now TEX to showup in Imageworks' browser */

#define SIGTYPES	5		/* signature types = 5 */
#define	MEAN		1		/* average MSS value of ITCs */ 
						/* or LIT or SHADE sides, or TT, if EXTBIT is used */
#define TCL		2		/* ITC Tree Colour Line */
#define TCL2		3
#define TEXTUR		4		/* ITC Texture (using variance) */
#define STRUCT		5		/* ITC Structure (using 3D moments) */

				/* covariance matrix type */
#define GT    1			/* Global Total - from all ITCs in image */
#define GS    2			/* Global Species - from an average of species cov.mat. */
#define SP    3			/* Species - ind. species cov. mat. are used in distances */
#define MA    4			/* Mahalanobis distances */

#define COUNT_ONLY  0
#define BIT_FILL    100
#define UNCLASS  999
//#define UNKNOWN  998    // conflict with gdal_priv.h UNKNOWN
#define UNSET    99999

#define NaN(X)  (((union { float d; struct { unsigned :1, e:11; } s; } *)&X)->s.e == 0x7ff)

// Header files to include

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>     /* malloc, free, rand */
#include <string.h>
#include <math.h>
#include <limits.h>
#include <time.h>       // time_t, struct tm, time, localtime
//#include <stdarg.h>
//#include <search.h>

//	PCI stuff - Only needed for transistion period

/*
#ifdef PARTIAL_GDAL

extern "C" {
#include "pci.h"
#include "itc_io.h"
#include "error.h"
#include "bitops.h"
#include "idb.h"		// IDB still works apparently 
}

#endif

*/


extern "C" {
#include "eigens.c"			// instead, just put eigens.obj on link line
#include "blas.c"
#include "cmatrix2.c"
#include "detlu.c"
#include "chisq.c"		// chisq.c & z.c implement the inverse chi-squared values 
#include "z.c"

}


// Note: at some point in time all the float were double: do I need to go back for precision ?
// 	 If you change back - Dont forget that multiple programs are affected


#include "ITC-Suite_g.h"	// define lots of vars for the Suite
//#include "gdal_priv.h"		// For GDAL library (now in above)
//#include "ogrsf_frmts.h"		// For OGR
#include "itc_io_g.h"		// my newest GDAL image/bitmap input/output
#include "bitops.h"		// bit operations on bitmaps (mostly macros to be faster)


//#include "ogr_api.h"
//#include "ogr_srs_api.h"
//#include "commonutils.h"

#include "cpl_port.h"
 

 
//**********************

/* Information node structure */

typedef struct info {
	int             xpos;
	int             ypos;
	int  		data[CHANNELS];
	struct info    *next_info;
} info_node;

/* structure definition for tree node */

typedef struct tree {
	int             xpos;
	int             ypos;
	int             count;			/* size of full crown in pixel */
	int             process_count;		/* number of pixels of crown used to generate signature (could be under LIT) */
	int             ITC_count;		// pixel of full crown
	float           itc_mean[CHANNELS];	// process related mean (e.g. under LIT mask, EXTBIT)
	float           Fitc_mean[CHANNELS];	// Full ITC based mean
	float           variance[CHANNELS];
	float           eigvector[CHANNELS * CHANNELS];
	float           intercept[CHANNELS];
	float           eigvalues[CHANNELS];
	float           covariance[CHANNELS*3 * CHANNELS*3];
	float			structure_param[CHANNELS*2];
	int             cg_xpos;
	int             cg_ypos;
	int				classI;		/* to hold classification results (class # start at zero)*/
	int				shapeId;	/* to hold corresponding shapeId in vector mode */
	int				class_2nd;	/* to hold 2nd possible class */
	struct tree    *next_tree;
} tree_node;

/* Results (a misnomer here) structure --> SPECIES SIGNATURES generated by ITCSSG */
// Has to be EXACTLY the same as in ITCSSG as it is read in binary

typedef struct result {
	int 	mask;			/* segment #  of training area */
	int 	number;			/* number of ITCs used */
	int		ITC_mask;		/* ITC bitmap used */
	int		EXT_mask;		/* external bitmap used */
	int 	SIG_type;		/* signature type used, first*1, second*10, third*100 */
	int 	channels;		/* number of bands used */
	int  	dbic[CHANNELS];
	float  	mean[CHANNELS];
	float	variance[CHANNELS];			// pixel variance (a texture signature)
	float	eigvector[CHANNELS * CHANNELS];
	float	intercept[CHANNELS];
	float	eigvalues[CHANNELS];
	float	correlation[CHANNELS*3 * CHANNELS*3];	// by 3, cause possibly 3 types of signatures
	float	covariance[CHANNELS*3 * CHANNELS*3];	// covariance of trees (not pixels)
	float	structure_param[CHANNELS*2];
	float	crown_area;
	} result_node;


/* function declarations */

void            check_param2(char *, int);
void            check_param4(char *);
void            fill(int, int, int, unsigned char *, tree_node *, int);
void            process_image_data(int, int, int64, int64, tree_node *, int);
void            process_tree_image_data(int, int, int64, tree_node *);
void            process_ITC_image_data(int, int, int64, tree_node *);
void            process_tree_var(tree_node *);
void            process_build_bitmap(int64, tree_node *);
void            process_tree_eigen(tree_node *);
void 		process_tree_cleanup(tree_node *);
void            eigvsort(float *, float *, int);
void            itc_intercept(float *, float *,float *, int);
void 		itc_structure(tree_node *);
void            process_tree_avg(tree_node *);
tree_node		*new_tree_node(void);
result_node		*new_result_node(void);
void            add_to_tree_list(tree_node *, tree_node *);
void            scan_for_tree(int, int, unsigned char *);
void            view_results(result_node *, int);
void            write_BM_histo(FILE *, int *, int, int *, int *, int, int, int);

void			write_GDAL_BM(char *, int *, int, int *, int *, int);

void            class_bitmaps(FILE *, int *, int, int *, int *, int, int, int);
result_node    *read_segment(FILE *, int);
void            process_global_covariance();
void            convert_to_vector(tree_node *, float *);
void            convert_to_vector_sign(result_node *, float *);
void            vector_subtract(float *, float *, int);
void            view_vector(float *, int);
void            classifi();
void            generate_bitmaps(int, unsigned char *);
result_node    *mod_sign_org(result_node *, int *, int *);
void            db_output(FILE *, char *);
void            output_sign_info(FILE *, tree_node *);
void            check_signature_results(result_node *, int, int, int);
void            upper_case(char *);
char           *itostr(int *, int);
/* int             compare_ints(const void *, const void *); */
void 		print_sig_types(int);
void 		separate(char *);
void 		param_expand(int, int *, int *, int *);

//void        loop_for_tree(int, int, unsigned char *, GDBLayer);
void        loop_for_tree_g(int, int, unsigned char *);

//void 		vect2rast(int, GDBVertex *, unsigned char *, int, GDBShapeId, GDBLayer);

void 		vect2rast_g(int, GDBVertex2D *, unsigned char *, int, int, int);


int  		between(float a, float b, float c);
void 		classify(tree_node *);
void 		cov_matrix();

void		create_output_file();


/* global vars */

void		*imagebuffer[CHANNELS];

//PIX_FUN_PTR	get_pix_val = get_u8;
extern PIX_FUN_PTR	get_pix_val;	// will get it from itc_io_g when image gets checked
PIX_FUN_PTR	get_Pval[CHANNELS];

unsigned char  *isolbitbuffer, *tempbitbuffer, *tempbitbuf2;
unsigned char  *classbitbuffer[BITMAPS], *pp;
unsigned char  *extmaskbuffer;

int64		bmsize;
char 		Extension[10]="";

static char    *SigTypes[SIGTYPES] = {"MEAN", "TCL", "TCL2", "TEXTUR", "STRUCT"};
int 		spa_dim_mul[SIGTYPES+1];
int		st1, st2, st3, st[3];				/* up to three signature types */
char		sigtypeO[65], classr[65];
int             stype, spa_size, lt_mask, sign_spa_size;

info_node      *Proc_pix_list;		// MS data from process (could be full ITC crown or LIT side or pointers)
info_node      *FITC_pix_list;		// MS data from full ITC crown (to be used by signatures such as structure)


	
float          matrix1[CHANNELS * CHANNELS];
result_node    *current_results;
int             ibufs, isigns, iclass, sign_bufs;
float          global_covariance[VECTORSIZE*3 * VECTORSIZE*3];
result_node    *current_signs[BITMAPS];
int             ExtMaskFlag = 0 , PointsFlag = 0;
char            boundtype[65];
int		cov_mat_type = SP;
int		unusedITC=0, speciescode;
int 		mean_flag, eigen_flag, tcl2_flag, textur_flag, struct_flag;
int		xcg, ycg;		/* center of gravity (aprox.) of current man tree */

float       threshold = 0.95;		// for initial GDAL  test
int 		DBOUTPUT;
char		segm_histo[120];
int 		ixmin, ixmax, iymin, iymax;

char 		ChList[80];
char		class2_in[65];
int			class2_flag = 0; 		// if = 1 special research mode to get 2nd class (2nd likelihood) instead of 1st

int 		poly_done[100000];	/* flags about polygon (shape) done or not -- in order to only do once */

/* for covariance matrices and maximum likelihood vector and matrix calculations */

float          vector1[VECTORSIZE*3], vector2[VECTORSIZE*3], tempvec1[VECTORSIZE*3];
float          inv_covariance[VECTORSIZE*3 * VECTORSIZE*3];
float          species_cov_mat[BITMAPS] [VECTORSIZE*3 * VECTORSIZE*3];
float          species_inv_cov[BITMAPS] [VECTORSIZE*3 * VECTORSIZE*3];
float          glob_sp_cov_mat [VECTORSIZE*3 * VECTORSIZE*3];
float          glob_sp_inv_cov [VECTORSIZE*3 * VECTORSIZE*3];
float          species_det[BITMAPS];
float          small_dist[BITMAPS];
float          determinant, glob_sp_det, critval, log2use;
int            class_sum[BITMAPS];


/*  main list of ITCs */

tree_node      *tree_list, *previous_tree;

/* additional global variables to deal with input vector layer */
int 		vmode = 0;  	// vector mode flag - equivalent slayer mode in itcssg_g
							//TRUE when polygon manual tree crowns are to be classified

//GDBLayer 	hlayer, tlayer;
double 		topleftX, transformX, topleftY, transformY; 	 /* for geographic mapping */
//ProjInfo_t	sProj;
char 		geosys[17];
//int		segtype;
const char 	*FieldFormat;
//GDBField	sField, *pField;
int			iField, oField;
int			cField;			//  cField is the main class result field index in SLAYER mode
GDBVertex	*tmpVertices;
char		timedate[17];	
char		pixunit[40], field_name[65], answer[40];
int 		node_cleared = 0;
int		Pixels, Lines, Channels;
int     	classified=0, unclassified=0;
int 		rast_pcount=0;


/* Global variable similar to  PCI parameters */ 

int	clascode[BITMAPS+1];
int	isolbit, extmask=0;

FILE *Report;   /* To compensate for "faulty" Report variable from core1000.dll */
	
int 		PCI_File = 0;			// initial flags for image file type
int 		TIF_File = 0;


/* Global variables to read images and bitmaps by sections to deal with huge images */

	int		dbiw[4], dbow[4];
	int		sect_count = 1;				/* section counter */
	int		sect_siz;				/* by sections of "sect_siz" lines */
	int		sect_limit = 0;			/* to keep track of last line in section (not to go over) */
	int		sect_overlap;
	int		next_sect_flag = 0;		/* flag to get another section of image */
	int		next_sect_start = 1;		/* line to start next section from (in lines starting at 1,1 */ 
	int		sect_end;				/* end of section - last line processed before fill() when out of section */
	int		line_offset = 0;		/* offset in lines to full image positions */
	int		by_sections = 0;		/* by_sections flag */
	int		schindx_flag = 0;		/* set --> user has supplied an index for EQUIVALENT channel position in signature */

	


// Global variables to read images, bitmaps and vector layers with GDAL (Specially with itc_io_g.cpp)

GDALDataset	*ima_in, *ima_out, *seg_in, *poDS;
GDALDriver 	*piDriver, *poDriver;
GDALRasterBand	*piBand,*piBand2,*poBand;

OGRLayer    *psLayer, *poLayer; 		//poLayer & poFeature are only used when creating a separate output shp file
OGRFeature 	*piFeature, *poFeature;
//OGRFeature 	*gpiFeature, *gpoFeature;		// usefull to pass Feature pointer to other function (was a test)
OGRFeatureDefn	*piFDefn, *poFDefn;
OGRFieldDefn	*piFieldDefn;
OGRFieldDefn	*poFieldDefn;

OGRSpatialReference *piSRSIn,  *poSRSIn;
OGRGeometry 	*piGeometry, *poGeometry;

PixVal		*pafScanline;
uint16		*pafScanline16;

char **papszOptions = NULL;
char **papszMetadata;

float		xpixsz, ypixsz;
char 		*Proj, *Proj2, *Datum, *Datum2, *Temp,*token;
double		adfGeoTransform[6], adfGeoTransform2[6];
char 	 	description[128];		// for PCI
char 	 	Description[128];		// for GDAL via write_bitmap()
int 		ch_in, ch_out, in_ch[10], segm_in, in_segm[10];
int 		imaFile_opened;

int			bylines_flag;			// default is to read/write by image (faster),
int	by_lines=0, by_image=1;			// default is to read/write by image (faster), 

int 	iFeat, layer_count, feat_count, field_count, last_feat, last_field;
int 	BM_ERR=0, VEC_ERR=0;

int		data_type;			// need to by global	
int		segtype;
int    sp_indx = 0 ; 			//temp for debugging
int 	class_field;

time_t rawtime;
struct tm * timeinfo;

char	*basefname;						// ** just a pointer **
char	basefilname[FILENAME];
int		basef_len;	


//***************************************************************************

// Main program 

//***************************************************************************
	
	
int main(int argc, char *argv[])
{

/* var list */

	int             i, j, k, ii, jj, kk;
	int             xsize, ysize, size, blocks, mask, channels;
	int64			iii;
	float		xpixsz, ypixsz;	 
	int             signseg[BITMAPS], classbit[BITMAPS];
	int			dbic[CHANNELS], schindx[CHANNELS*3];
	char		segflag, segname[9], segdepinfo[224];
	long 		start, length;
	int			 index[CHANNELS*3];
	FILE        *idb_fp, *sig_fp, *sig_fp2, *temp_fp;
	int64		mem_needed, mem_needed_BM, mem_needed_BM_min, mem_needed_IMA=0, mem_needed_TOT, mem_avail_SECT, mem_avail_SYS;
	int			no_8bCHs=0, no_16bCHs=0, min_BMs;
	int 		actval, numbuf[32];
	float		rnumbuf[1];
	int 	species;


	int 	out_ch, input_ch[20],input_chI[20], no_ch=1, ich=0, no_items;
	int	input_bm[20], no_bm, ibm;
	int	input_vec[20], no_vec_layers, ivec;
	//char 	Proj[200];
	char * 	pEnd;

	char 	filename[FILENAME], temp[FILENAME], fullfilename[FILENAME], file_SHP[FILENAME];
	char	path[FILENAME], main_path[FILENAME], sig_path[FILENAME];
		
	char 	*tstring, *p, *file_in, *file_out, file_ITC[FILENAME], file_EXT[FILENAME];
	char 	file_SIG[BITMAPS][FILENAME], given_fname[FILENAME], base_fname[FILENAME];

	char 	ext[4], ch_no[10], bm_no[10], vec_no[10], seg_no[4];
	int 	no_color=0;
	int 	name_given = 0;		// flag if a base name was given for signature files

	int 	path_len=0, main_path_flag, sig_path_flag;
	
	
/* Define PCI parameters */

	void           *args[13];
	int             argcnt[13];	
	char            file[FILENAME];
	int             idbic[CHANNELS], ischindx[CHANNELS];
	int             isolbitI;
	int             signsegI[32];		/* input parameter info (possibly with ranges) */
	int             iclassbit[32];		/* input parameter info (possibly with ranges) */
	int 		iclascode[32];	
	int             ltmask;
	char            sigtype[64], treetype[64];
	float           ithreshold;
	char            dbout[FILENAME]; 
	char 		report[FILENAME];
	

	

//*************************************************************

/* For documention

//Define PCI parameters 

	args[0] = (void *) file;
	args[1] = (void *) idbic;
	args[2] = (void *) ischindx;
	args[3] = (void *) &isolbitI;
	args[4] = (void *) isignseg;
	args[5] = (void *) iclassbit;
	args[6] = (void *) iclascode;
	args[7] = (void *) &ltmask;
	args[8] = (void *) sigtype;
	args[9] = (void *) treetype;
	args[10] = (void *) &ithreshold;
	args[11] = (void *) dbout;
	args[12] = (void *) report;

// setup standard interface 

	IMPStatus("FILE,DBIC,SCHINDX,ISOLBIT,SIGNSEG,CLASSBIT,CLASCODE,EXTBIT,SIGTYPE,TREETYPE,THRESHLD,DBOUT,REPORT;",
		  "C,   I,   I,      I,      I,      I,       I,       I,     C,      C,       R,       C,    C;",
		  "132, 8,   8,      1,      16,     16,      16,      1,     64,     64,      1,       132,  132;",
		  "1,   1,   **,     1,      1,       0,      0,       0,     1,      1,       0,       0,    0;",
		  "ITCSC.", "FORCE", argcnt, args, argc, argv);

// To compensate for "faulty" Report variable from core1000.dll 

if(EQUALN(report,"TERM",4)) {Report=stdout;}else{Report = fopen(report, "w");}


// make some local variables more global 

	for (i = 0; i < 16; i++) clascode[i] = iclascode[i];

	 printf("\nCLASCODE : \n");
	   for (i = 0; i < 16; i++) printf(" %d ", clascode[i]); printf("\n");
	

	for (i = 0; i < CHANNELS; i++) schindx[i] = ischindx[i];
	for (i = 0; i < CHANNELS; i++) dbic[i] = idbic[i];
	isolbit = isolbitI;
	threshold= ithreshold;


*/	
	
	

//*****************************************************************************

//	Check input parameters (and open image file)

//*****************************************************************************

// Print Program Header 

	time (&rawtime);
	timeinfo = localtime (&rawtime);
	printf("\n\t\tStarting %s (%s) at %s\n", PROG_NAME, VERSION, asctime(timeinfo));

// Registers for all types of files with GDAL

	GDALAllRegister(); 	
	
	Report = stdout;			// for PCI-related old code where "fprint" use Report
	
// Check Arguments on command line

	if (argc < 6) 
	  {
	  printf("\n\t *** PROBLEM with arguments on the command line\n");
	  printf("\tThe following are the minimum compulsary parameters : \n");
	  printf("\tUSAGE: itcsc_g FILE DBIC ISOLBIT EXTBIT SigSEG \n");
	  printf("\tManual trees: itcsc_g FILE DBIC ISOLBIT EXTBIT SIGSEG MEAN SLAYER");
	  exit(1);
	  }   


// Open IMAGE FILE  and get info (needed for input parameteers checking)
// Will report if PCI or TIF file

	strcpy(filename, argv[1]);
	
	ima_in = open_imaFile(filename);		// Open image file (PCI or TIF) and get info about it
	
	xsize = Pixels; ysize = Lines;			// use throughout program (from open_imaFile())
	
	dbiw[0]=0; dbiw[1]=0; dbiw[2]=Pixels; dbiw[3]=Lines;		// for analysis by full image 
	
	bmsize = (Pixels * (int64)Lines + 7) / 8; 
	//printf("\nBitmap size (in bytes): %I64d \n", bmsize);
	
	// Extension is a global parameter set by Open_imaFile()
	
	if (EQUALN(Extension,"pix",3)) PCI_File = 1;	// everything is in the PCI file
	if (EQUALN(Extension,"tif",3)) TIF_File = 1;	// everything is in directory, mostly as tif files
	if (! ( PCI_File || TIF_File))
	  {
	  printf("\n\n**ERROR** Program not able to deal with image file of type %s\n\n", Extension);
	  exit(-1);
      }


// ########
	
// Need to get "basefilename" when full path is involved
// Basefile name has path + head of file name (typically correspond to "named area" of study e.g. PRF)
// Area name is assumed separated from rest of file name by an underscore

	strcpy(fullfilename,argv[1]);			// main input filename (and possibly its dir)
	
	jj = 0;
	for (ii=0; ii < strlen(fullfilename); ii++)		// search for last underscore position
	  {
	  jj = strlen(fullfilename) - ii;				// start from the end
	  //printf("Count back: %d",j);
	  if(fullfilename[jj] == '_') {basef_len = jj;	break;}	// find last underscore in full file name
	  }
	//printf("\nBase Filename Length:  %d \n", basef_len);
  	
	strncpy(basefilname, fullfilename,  basef_len);		// get that part of  the full file name
	basefilname[basef_len] = '\0';   					// make it a string to be safe
	
	printf("Base file name ::  %s \n", basefilname);	
	//exit(-1);
	
	
	

// 	Get full directory  PATH to that MAIN file (maybe to be used by others)

 
	strcpy(fullfilename,argv[1]);			// main input filename (and possibly its dir)
	//printf("Full Filename:  %s \n", fullfilename);
	//printf("\n Fullfilename string length is  %d \n", (int)strlen(fullfilename));
	
	jj = 0;
	for (ii=0; ii < strlen(fullfilename); ii++)			// could be slash or backslash
	  {
	  jj = strlen(fullfilename) - ii;		// from the end of full file name
	  if( (fullfilename[jj] == '/') || (fullfilename[jj] == '\\') ){path_len = jj;	break;}	// find last slash in full file name
	  }  
	printf("\nPath Length:  %d \n", path_len);
	//printf("\n jj =  %d \n", jj);	


	if(path_len == 0) strcpy(main_path, "./");				// no path  - impose a default path
	else
	  {
	  strncpy(path, fullfilename, path_len+1);		// get the path (inc. last slash)
	  path[path_len+1] = '\0';   					// null character manually added 
	  strcpy(main_path, path);	  
	  }

	printf("\n  Path of main file ::  %s \n", main_path);			// may have full path to directory
	
	//exit(-1);       // for degugging
 	
	
//*************
	
// Check list of "CHANNELS to use" in argv[2] and break into its componentds

	printf("\nAs per user, input raster channel number(s) to use: %s \n", argv[2]);

	strcpy(temp, argv[2]);  	strcpy(ChList, argv[2]);
	
	p = strtok(temp, ","); ii = 0;
	while(p != NULL) 
	  {
 	  //printf("%s\n", p); 
	  input_chI[ii++] = strtol(p,NULL, 10);
	  p = strtok(NULL, ",");
	  }
	no_items = no_ch = ii;

	//printf("Number of channels (items) : %d \n", no_items);
	//for(i=0; i<no_ch; i++) 	printf("CH%d is %d \t", i, input_chI[i]);
	//printf("\n");

//	Range of channels may have been used (minus sign), so expand list

	param_expand(no_items, input_chI, input_ch, &no_ch);
	
	if(no_ch > CHANNELS) 
		{fprintf(stderr,"\n ERROR - max # of channels exceeded \n"); exit(-1); }

	printf("\n*After range assessment, input channels to use : \n");
	for(i=0; i<no_ch; i++) 	printf("CH%d is %d \t", i, input_ch[i]);
	printf("\n\n---------------------------\n");

	for(i=0; i<no_ch; i++) 
	{
	if( input_ch[i] > Channels)
	  {
	  printf("\nAs per user, one channel number to use: %d \n", input_ch[i]);
	  printf("\n\t** ERROR this is more than the number of channels in the main file\n\n");
	  exit(-1);
	  }
	piBand = ima_in->GetRasterBand(input_ch[i]);

	if ( piBand->GetMetadataItem("NBITS","IMAGE_STRUCTURE") != NULL)
	  {
	  printf("\n\t*** ERROR - One specified channel (%d) appears to be a bitmap layer\n", input_ch[i]);
	  printf("\nImage: %s \n", ima_in->GetDescription() );
	  printf("Channel Description: %s \n", piBand->GetDescription() );
	  printf("Please double check (Hint: use \"gdalinfo\" on the file).... Exiting\n\n");
	  exit(1);
	  }

	for(ii=i+1; ii<no_ch; ii++) 
	 if (input_ch[i] == input_ch[ii])
	  {
	  printf("\n\t*** ERROR - One specified channel (%d) appears to be duplicated\n", input_ch[ii]);
	  printf("\nImage: %s \n", ima_in->GetDescription() );
	  printf("Channel Description: %s \n", piBand->GetDescription() );
	  printf("Please double check your inputs.... Exiting\n\n");
	  exit(1);
	  }

	}		// end of "i" for loop

// Rest of program uses dbic[] and ibufs

	for(i=0; i<no_ch; i++) dbic[i] = input_ch[i];		
	ibufs = no_ch;
	
	

// *****************************

// Check *Third* argument argv[3] on command line : ISOLBM 
// Could be a bitmap # in the main input PCI file OR a separate tif file
// For special "SLayer" mode, it has to be a vector layer

Arg3:

	printf("\nAs per user input, ISOLBIT number (or file) to use: %s \n", argv[3]);

	 
// Check for SLAYER mode (N.B.: a very special case)
		
	 if ( (argc == 8) && EQUALN(argv[7],"SLayer",6) )	goto SLAYER;	// IF special mode "SLayer" as TREETYPE 
		  

	
	isolbit = strtol(argv[3],NULL,10);		// pickup ISOLBM number
	
	// Check if user entry is a full file name (i.e., it"s not just a plain number)
	
	if (isolbit == 0)				// if not number, then it's a specific file name
	  {
	  strcpy(file_ITC, argv[3]);
	  printf("\n\t*Will use file %s \n", file_ITC);		// that's all
	  
	  // may want to check that it is a bitmap (1 bit tif) or is iit done later

	
	  goto Arg4;
	  }	  	
	
	// If a number, Check that user entry is valid (NO multiple comma-separated entries allowed here)

	strcpy(temp, argv[3]);  p = strtok(temp, ","); ii = 0;
	while(p != NULL) 
	  {
 	  //printf("%s\n", p); 
	  input_chI[ii++] = strtol(p,NULL, 10);
	  p = strtok(NULL, ",");
	  }
	no_items = ii;

	if(no_items > 1) 	
	  {
	  printf("\nAs per user input, ISOLBIT channel number to use: %s \n", argv[3]);
	  printf("\n\t** ERROR This require a single entry (no range allowed)\n\n");
	  exit(-1);
	  }


		  
		  
		  
// Ordinary mode where ITCs are coveyed in a Bitmap (check bitmap no. not too big)	  
	  
	if(EQUALN(Extension,"pix",3) && (isolbit > Channels) )
	  {
	  printf("\nAs per user, ISOLBIT channel number to use: %d \n", isolbit);
	  printf("\n\t** ERROR this is more than the number of channels in the main file\n\n");
	  exit(-1);
	  }

	 
	// Check that it is actually a bitmap (i.e., not an image) within the main  PCI file

	if (EQUALN(Extension,"pix",3))	
	  {	
	  piBand = ima_in->GetRasterBand(isolbit);		// Access bitmap channel (i.e, segment)
		
	  if ( piBand->GetMetadataItem("NBITS","IMAGE_STRUCTURE") == NULL)
	  {
	  printf("\n\t*** The specified channel (%d) for ISOLBIT is not a bitmap layer\n", isolbit);
	  printf("\nImage: %s \n", ima_in->GetDescription() );
	  printf("Channel Description: %s \n", piBand->GetDescription() );
	  printf("Please double check (Hint: use \"gdalinfo\" on the file).... Exiting\n\n");
	  exit(1);
	  }

	 // All is OK
	  
	  printf("\n*As directed, will use ISOLBIT channel number (%d) from the main PCI file.\n", isolbit);
	  printf("Channel description: %s \n", piBand->GetDescription() );
	  goto Arg4;
	  }


// If main image file is a tif, others are discrete files create automatic names

	if (EQUALN(Extension,"tif",3))	
	  {	
  
  		// Create input file name from numbers		e.g.: SECTEUR_SIG_BM14.sig
	
	    strcpy(fullfilename,filename);			// main filename as base
	    //printf("fullfilename :  %s \n", fullfilename);
	    basefname = strtok(fullfilename,".");
	    //basefname = strtok(basefname,"/");
		
		
		basefname = strtok(basefname,"_");
	    //printf("basefname  :  %s \n", basefname);
	    file_in = strncat(basefname,"_BM",3); 
	    //printf("file_in  :  %s \n", file_in);    
  	    //itoa(isolbit,seg_no,10); 
		sprintf(seg_no,"%d",isolbit);
	
	    strncat(file_in,seg_no,3);	  	  	
 	    strncat(file_in,".tif",4);
		strcpy(file_ITC, file_in);
	    printf("ISOLBIT file_in  :  %s \n", file_ITC);   
 	  goto Arg4; 
	  }

	  
SLAYER:

	  printf("\n\"SLayer\" mode detected, ISOLBIT should be vector segment. Segment number to use: %s \n", argv[3]);

	
	if(PCI_File)
	  {
	  // Check that this is a  vector segment (in SLayer mode,ISOLBM is a vector layer) 

	  seg_in = ima_in;	// just to be cleaner (and to match other code) ## FOR NOW all in 
	
	  layer_count = seg_in->GetLayerCount();
	  if(layer_count == NULL) { printf("\n\t*There are no vector layer in this file \n"); exit(-1);}
	  //printf("\n\t*Number of vector layers in PCI input file is %d \n\n", layer_count);

	  
	  
	  psLayer = seg_in->GetLayer(isolbit-1);	// Layer numbers start at zero
	  if(psLayer == NULL)  
		 printf("\n*The layer specified (%d) may not be of individual tree polygons\n",isolbit); 
		
	  piFDefn = psLayer->GetLayerDefn();
	  feat_count = psLayer->GetFeatureCount();
	  field_count = piFDefn->GetFieldCount();
	
	  printf("\n**Current layer %d of %s file has %d features (shapes) with %d fields each\n\n", 
				isolbit, Extension, feat_count, field_count);
				
	  vmode = 1;		// set single layer flag


	  
	  //goto Arg4;
	  }
	
	if(TIF_File)			// CREATE expected shp file name and open it
	    {
		strcpy(fullfilename,filename);			// main filename as base
		//printf("fullfilename :  %s \n", fullfilename);
		basefname = strtok(fullfilename,".");
		//basefname = strtok(basefname,"/");
		basefname = strtok(basefname,"_");
		//printf("basefname  :  %s \n", basefname);	
		file_in = strncat(basefname,"_VEC",4); 
		//printf("file_in  :  %s \n", file_in);    
  	    //itoa(isolbit,seg_no,10); 
		sprintf(seg_no,"%d",isolbit); 	  
		strncat(file_in,seg_no,3);	  	  	
		strncat(file_in,".shp",4);
		strcpy(file_SHP, file_in);
		printf("slayer shp is  :  %s \n", file_SHP);  

		// OPEN  that shp file 
		
		//seg_in = (GDALDataset*) GDALOpenEx(file_SHP, GDAL_OF_VECTOR, NULL, NULL, NULL );
		
		//seg_in = (GDALDataset*) GDALOpen(file_SHP, GA_Update);		// will not work
		
		seg_in = (GDALDataset*) GDALOpenEx(file_SHP, GDAL_OF_UPDATE | GDAL_OF_VECTOR, NULL, NULL, NULL );		
		if( seg_in == NULL )
			{printf( "**** Failed to open input file %s\n", file_SHP); exit(-1 );}

		printf("\n\t**File '%s' was opened for update : single layer mode (vector)\n\n", file_SHP);
				
		layer_count = seg_in->GetLayerCount();
		if(layer_count == NULL) { printf("\n\t*There are no vector layer in this file \n"); exit(-1);}
		//printf("\n\t*Number of vector layers in SHP input file is %d \n\n", layer_count);

		psLayer = seg_in->GetLayer(0);		// Layer numbers start at zero
		if(psLayer == NULL)  
			printf("\n*The layer specified (%d) may not be of individual tree polygons\n", isolbit); 
				
		piFDefn = psLayer->GetLayerDefn();
		feat_count = psLayer->GetFeatureCount();
		field_count = piFDefn->GetFieldCount();
			
		printf("Current layer of file VEC%d has %d features (shapes) with %d fields each\n\n", 
						isolbit, feat_count, field_count);	
			
	    vmode = 1;		// set single layer flag
		
		//printf("MAIN: Address of input Layer  = %p \n",  psLayer);
	
	    }  
	
	
//######################	


  
// For SHP file, CHECK that geographic information and projection are same as the image file

// ######


  
// CONTINUE SLAYER mode for BOTH  (PCI or TIF main files)

  printf("\n\tPresent input SLAYER has the following Fields \n\n");
	
// Print ALL existing field 

  for (iField=0; iField < piFDefn->GetFieldCount(); iField++)
    {
	piFieldDefn = piFDefn->GetFieldDefn(iField);
	printf(" %d, Type %d, Field Width %d, Precision %d Name: %s\n",
		iField, piFieldDefn->GetType(), piFieldDefn->GetWidth(), piFieldDefn->GetPrecision(),piFieldDefn->GetNameRef() );  
	}
 
 
// Check if default field CLASSR exist
 
	strcpy(classr, "CLASSR");		 // set name "CLASSR" by default for classif results field
	strcpy(field_name, "CLASSR");

				
// IF field already exist, ask user to overwrite or use another 
	
L00:	
    if ( (iField = piFDefn->GetFieldIndex(field_name)) > -1) 		// check if field exist 
	{
L0:	printf("\n\n ### Field %s (%d) already exist. Is it OK to overwrite it (y/n)?", field_name, iField);
	scanf("%s",answer);		// gets(answer); 
 	upper_case(answer);
  	if ( (answer[0] != 'Y') && (answer[0] != 'N') ) goto L0;			// need an answer

  	if (answer[0] == 'Y') 
	{
	cField = iField;
	printf("\n\t## Field %d (%s) will be overwritten \n", cField, field_name);  
	}
 	
	if (answer[0] == 'N') 
	  {
	  printf("What is the NAME you wish to give to that field (e.g.: class_lit_3ch) ???");
      scanf("%s",field_name);		// gets(field_name); 
	  goto L00;						// check that one for existance and possible overwrite
	  }	
	}

	
// FOR THE MOMENT, using PCI functions instead of GDAL

#ifdef PARTIAL_GDAL
 
 	if(PCI_File )				
	  {
	  printf("\n For PCI file (using PCI segment nomemclature)   ...\n");
	  if (isolbit == 2)	isolbit = 9;					// for TESTING  use PCI numbering & functions (GDB)
	  if (isolbit == 3)	isolbit = 20;					// for TESTING use PCI numbering & functions (GDB)
	  
	  printf("\nOpening PCI input layer %d of polygons manual tree crowns \n", isolbit);
      hlayer = GDBGetLayer(idb_fp, isolbit);	  
	  if(hlayer == NULL) printf("\n\n\t **ERROR opening PCI vector layer %d \n\n", isolbit);


// Create field (if does not exist) to store classification results on a tree by tree basis 
   
	  if (GDBGetFieldIndex(hlayer, field_name) == -1)	// if field not already there, create it 
		{
	    printf("\nCreating new field (%s) to store classification results in PCI segment %d\n", field_name, isolbit);	
        sField.nInteger = UNSET;		
		iField = GDBAddField(hlayer, field_name, GDBFieldTypeInteger, &sField); 
		if (iField == -1) fprintf(stderr,"\n *** ERROR *** creating field %s \n", field_name);
		printf("\nNOTE: Created new field index is: %d \n",iField);
		field_count++;
		}

// Set field format of that field and confirm 

	  iField = GDBGetFieldIndex(hlayer, field_name);

  
	  /* Generate the field format */
	  FieldFormat = GDBGenFieldFormat(GDBFieldTypeInteger, 4, 0, TRUE);
	  GDBSetFieldFormat(hlayer, iField, FieldFormat);
	  printf("\nInd. tree classification results will be written to field '%s' (%d) of PCI vector layer %d.\n",
								field_name, iField, isolbit);
			
	  //exit(-1);
		
 	  }
	 
#endif	 
 
   if ( TIF_File && (piFDefn->GetFieldIndex(field_name) == -1) )	// NEED TO CREATE a new field
	  {
	  
	// Create the new field to hold classification results

		
		OGRFieldDefn poField( field_name, OFTInteger);		
		
		//poField.SetName(field_name);	
		//poField.SetType(OFTInteger);
		poField.SetWidth(3);
		
		if( psLayer->CreateField( &poField ) != OGRERR_NONE )
		  { printf( "Creating Name field failed.\n" ); exit( 1 ); }  
	  	  
		printf("\n Created new field %s in shp file %s\n", field_name, file_SHP);
		field_count++;	
// Get field index of new field for use later	

		iField = piFDefn->GetFieldIndex(field_name);
				
		//exit(-1);
	
	  }
	  
	  cField = iField;		// From now on, field will be known as "class field" (cField)  
	  
  	  printf("\n\t## Field %d (%s) will be used \n", cField, field_name);

	  //exit(-1);	

	  

// CREATING  a separate output file cause presently input/output layer(psLayer) does not get out properly
//  This is using poLayer and poFeature pointers

/*	  
	printf("\n\t\tCreating an output shp file and copying everything to it \n\n");

	create_output_file(); 		// temp output file for testing
	
	printf("MAIN: Address of auxilliary OUTPUT Layer  = %p \n",  poLayer);	
*/


	
// ****************************

// Check Fourth argument argv[4] on command line : EXTBM (extra bitmap, often LIT)
// Could be a bitmap # in main input file OR a separate tif file

Arg4:		//exit(-1);		// to help with debugging

	printf("\nAs per user, EXTBIT number (or file) to use: %s \n", argv[4]);

	// Having no external mask (e.g., LIT) is possible - Signature  base on full ITC
	
	if ( EQUALN(argv[4],"-",1 )) 
	  {
	  ExtMaskFlag = 0;  // set flag that there is NO external bitmap
	  printf("\nNOTE: As per user, signatures will be done without External Bitmap (e.g., LIT).\n");	  
	  goto Arg5;		// check next argument
	  }

	ExtMaskFlag = 1;					// set flag that there is an external bitmap	  

	extmask = strtol(argv[4],NULL,10);	// pickup EXTMASK number - used with both image file types

	// Check if user entry is a full file name (i.e., it"s not just a plain number)
	
	if (extmask == 0)				// if not number, then it's a specific file name
	  {
	  strcpy(file_EXT, argv[4]);
	  printf("\n\t*Will use file %s \n", file_EXT);		// use it, that's all
	  goto Arg5;
	  }

	// check that user entry is valid - no double number

	strcpy(temp, argv[4]);  p = strtok(temp, ","); ii = 0;
	while(p != NULL) 
	  {
 	  //printf("%s\n", p); 
	  input_chI[ii++] = strtol(p,NULL, 10);
	  p = strtok(NULL, ",");
	  }
	no_items = ii;

	if(no_items > 1)	
	  {
	  printf("\nAs per user input, EXTBIT channel number to use: %s \n", argv[4]);
	  printf("\n\t** ERROR This require a single entry (no range allowed)\n\n");
	  exit(-1);
	  }

	if( EQUALN(Extension,"pix",3) && (extmask > Channels) )
	  {
	  printf("\nAs per user, EXTBIT channel number(s) to use: %d \n", extmask );
	  printf("\n\t** ERROR this is more than the number of channels in the main file\n\n");
	  exit(-1);
	  }

	  
	  
	if (EQUALN(Extension,"pix",3))	
	  {		  
	  
	// Check that it is actually a bitmap (i.e., not an image) with the PCI main file

	piBand = ima_in->GetRasterBand(extmask);		// Access bitmap channel (i.e, segment)

	if ( piBand->GetMetadataItem("NBITS","IMAGE_STRUCTURE") == NULL)
	  {
	  printf("\n\t*** The specified channel (%d) for EXTMASK is not a bitmap layer\n",extmask);
	  printf("\nImage: %s \n", ima_in->GetDescription() );
	  printf("Channel Description: %s \n", piBand->GetDescription() );
	  printf("Please double check (Hint: use \"gdalinfo\" on the file).... Exiting\n");
	  exit(1);
	  }

	  // All is OK

	printf("\n*As directed, will use EXTBIT channel number (%d) from main file.\n", extmask );
	printf("Channel description: %s \n", piBand->GetDescription() );
	ltmask = extmask;

	  }	


// If main image file is a tif, others are discrete files

	
if (EQUALN(Extension,"tif",3))	
	  {	
  
  		// Create input file name from numbers		e.g.: SECTEUR_SIG_BM14.sig
	
	    strcpy(fullfilename,filename);			// main filename as base
	    //printf("fullfilename :  %s \n", fullfilename);
	    basefname = strtok(fullfilename,".");
	    //basefname = strtok(basefname,"/");
		basefname = strtok(basefname,"_");
	    //printf("basefname  :  %s \n", basefname);
	    file_in = strncat(basefname,"_BM",3); 
	    //printf("file_in  :  %s \n", file_in);  
	  	
	    //itoa (classcode[species],seg_no,10);  
  	    //itoa(extmask,seg_no,10); 	
		sprintf(seg_no,"%d",extmask);		
	    strncat(file_in,seg_no,3);	  	  	
 	    strncat(file_in,".tif",4);
		strcpy(file_EXT, file_in);  
	    printf("EXTMASK file_in  :  %s \n", file_EXT);   
		ltmask = extmask; 
	  }
	
	
	

//*********************************

// Check Fifth argument argv[5] on command line : SIGNSEG
	  
Arg5:		//exit(-1); 		// to help with debugging 

	printf("\n\n---------------------------\n");
	
if (EQUALN(Extension,"pix",3))	
	printf("\nAs per user, signature segment number(s) to use within a PCI file : %s \n", argv[5]);

if (EQUALN(Extension,"tif",3))	
	printf("\nAs image file is %s, signature number (%s) should correspond to separate .sig files\n", 
				Extension, argv[5]);
				
				
	  strcpy(temp, argv[5]); 			// move input to temp[]	  

	// Check if user entry has a file name in it(i.e., it"s not just a plain number)
	
	  if (strtol(argv[5],NULL,10) == 0)				// if not number, then it's a specific file name
	    {
		p = strtok(temp, ",");  
		strcpy(given_fname,temp);
		printf("\n\t*Will use base file name %s \n", given_fname);	
		name_given = 1;			// flag that a specific base name for training area was given
		//exit(-1);
		}	  
						
				
// For PCI or tif main file, if specified by simple numbers		
// Separate signature segments (files)  based on commas

	  if(!name_given) { strcpy(temp, argv[5]); p = strtok(temp, ","); }
	  		  
	  if(name_given) p = strtok(NULL, ","); 
	
	ii = 0;
	while(p != NULL) 
	  {
 	  //printf("%s\n", p); 
	  signsegI[ii++] = strtol(p,NULL, 10);			// for PCI, possible signature segments to use
	  p = strtok(NULL, ",");
	  }
	no_items = isigns = ii;

//	A RANGE of signature segments (or files) files may have been used (i.e., minus sign), so expand

	param_expand(no_items, signsegI, signseg, &isigns);
	
	if(isigns > BITMAPS) 
		{fprintf(stderr,"\n ERROR - max # of signatures exceeded \n"); exit(-1); }

	printf("\nInput signature to use : \n");
	for(i=0; i<isigns; i++) 	printf("Segm%d is %d \t", i+1, signseg[i]);
	printf("\n\n");
	
	
// IF base file name already given 	for signatures  

	  if(name_given)
	  {
	  strcpy(temp,given_fname);		// for example: secteur_SIG_VEC.sig
	  
		// separate extension to put back later

	    strcpy(base_fname, strtok(temp,"."));		// get base file name  (e.g., secteur_VEC)
        strcpy(ext, strtok(NULL, " "));		// get extension to put back later
	    //printf("\n\tBase_fname = %s Extension =  %s \n", base_fname, ext);		
		

// 	Get full directory  PATH for these  files 

	path_len=0;	jj=0; 
//	sig_path_flag = 0 ;		// assume no path was supplied with shp file string
 
	strcpy(fullfilename, given_fname);
	//printf("Full Filename:  %s \n", fullfilename);
	//printf("\n Fullfilename string length is  %d \n", (int)strlen(fullfilename));
	
	for (ii=0; ii < strlen(fullfilename); ii++)			// could be slash or backslash
	  {
	  jj = strlen(fullfilename) - ii;		// from the end of full file name
	  if( (fullfilename[jj] == '/') || (fullfilename[jj] == '\\') ){path_len = jj;	break;}	// find last slash in full file name
	  }  
	printf("\nSignature  Path Length:  %d \n", path_len);
	//printf("\n jj =  %d \n", jj);	


	if(path_len == 0)			//no path was supplied with shp file string
	  {
	  sig_path_flag = 0; 				//no path was supplied with shp file string
	  //strcpy(path, "./");				// no path  - impose a default path
	  strcpy(sig_path, main_path);				// OR use MAIN file path
	  
	  strcpy(base_fname, fullfilename);		//so base_fname is fullfilename
	  }
	  
	if(path_len > 0) 				// path was supplied with shp file string
	  {	
	  sig_path_flag = 1; 				// path was supplied with shp file string
	  strncpy(path, fullfilename, path_len+1);		// get the path (inc. last slash)
	  path[path_len+1] = '\0';   					// null character manually added 
	  strcpy(sig_path, path);
	
//	base_fname = fullfilename after path

	  ii =0 ;
	  jj = path_len + 1;			// end of path in fullfilename
	  while(fullfilename[jj] != NULL)  base_fname[ii++] = fullfilename[jj++];
	  }
	
//	printf("SHP Full Filename:  %s \n", fullfilename);

	printf("\n  Path of SIG files is ::  %s \n", sig_path);			// may have full path to directory
							
// Remove extension from base_fname		
		
	  strtok(base_fname,".");	
		
	printf("\n\tBase_fname = %s \t Extension =  %s \n\n", base_fname, ext);	
	//printf("\n\tGiven_fname = %s \t Extension =  %s \n", given_fname, ext);
		
					
	  for(i=0; i<isigns; i++)		// check all possible signatures
		{
		if (!sig_path_flag) strcpy(temp, main_path);		// ArcGISPro requires full path, so add main file path
		if (sig_path_flag) strcpy(temp, sig_path);			// if a path was given with the signatures, add that one
		strcat(temp,base_fname);
		//itoa(signseg[i],seg_no,10); 	  // get segment number
		sprintf(seg_no,"%d",signseg[i]);
	    strncat(temp,seg_no,3);	
	    strcat(temp,".");	
	    strcat(temp,ext);	
		strcpy(file_SIG[i],temp);
		
		printf("Signature file to use  :  %s \n", file_SIG[i]); 	


		// CHECK training area file existence
 
		//printf("\n\t*Checking on existance of File %s  \n", file_SIG[it]);
 
		temp_fp = fopen(file_SIG[i], "r");	
		if (temp_fp == NULL) 
		  {
		  printf("\n\n\t*Cant find file %s -- Please double check on it \n\n", file_SIG[i]); 
		  printf("\n Will abort : Read Message then, type anything  ");
		  answer[0] = getc(stdin); 		// gets any answer or <CR>  
		  exit(-1);
		  }
		

		}		// for all signatures	
		 	  		  
	  }			// for name given
	



	// #################
	
 	// If no signature base file name given, 
	// create AUTOMATIC names from given numbers (e.g.: SECTEUR_SIG_BM14.sig)
	// AND make sure path is also included
		
	  if(!name_given)			// need to try using my automatic names
	  {	
	 
	  basefname = temp;	 				// necessary cause just a pointer no space assigned to it
	  //basefname = (char*) malloc(200 * sizeof(char));		// theoretically
	 
	  for(i=0; i<isigns; i++)
		{				
		  
		//strncpy(basefname, basefilname, basef_len);
		//basefname[basef_len] = '\0'; 
		
		strcpy(basefname, basefilname);	
				
	    file_in = strncat(basefname,"_SIG_BM",7); 			// check for signature from bitmap
	    //printf("file_in  :  %s \n", file_in);    
  	    //itoa(signseg[i],seg_no,10); 	
		sprintf(seg_no,"%d",signseg[i]);	
	    strncat(file_in,seg_no,3);	  	  		// add segment number
 	    strncat(file_in,".sig",4);				// add extension
	    //printf("file_in  :  %s \n", file_in); 	
		strcpy(file_SIG[i], file_in);  
	    printf("%d Possible signature file_in : %s \n", i+1, file_SIG[i]); 	
		
		// Check if file exist as signature from a Bitmap Training Area 
		
		sig_fp = fopen(file_SIG[i],"rb");
		if (sig_fp) { fclose(sig_fp); continue;	}	// if OK, check next signature

		
		if (!sig_fp) 		// if not OK, check if came from a Vector Training Area
		  { 
		  printf("*** As such file does not exist - Will try a different variation\n"); 
		
		  strcpy(basefname, basefilname);	
		
		  file_in = strncat(basefname,"_SIG_VEC",8); 
		  //itoa(signseg[i],seg_no,10); 	
		  sprintf(seg_no,"%d",signseg[i]);		  
		  strncat(file_in,seg_no,3);	  	  	
		  strncat(file_in,".sig",4);
		  //printf("file_in  :  %s \n", file_in); 	
		  strcpy(file_SIG[i], file_in);  
		  printf("%d Possible signature file_in : %s \n", i+1, file_SIG[i]); 
			
		  // Check if that file exist
		  
		  sig_fp = fopen(file_SIG[i],"rb");
		  if (!sig_fp) { printf("\n   *** File may not exist !!! \n\n"); exit(-1); }
		  printf("%d Will use signature file_in : %s \n\n", i+1, file_SIG[i]); 		  
		  fclose(sig_fp);
		  }		// endof check vector (polygon) TAs
  
		}		// endof for each signature

	  }			// endof if no name given and creation of automatic names for signature files

 
//*********************************	
	
// Check Sixth argument argv[6] on command line : SIGTYPE

//printf("\n\t*As per default, signature type to generate : %s \n", argv[6]);

Arg6:	  //exit(-1); 		// to help with debugging 

	if ( (argc < 7) ||  EQUALN(argv[6],"-",1 )) 		
	  {
	  strcpy(sigtype,"MEAN");
	  printf("\n\t*As per default, signature type to generate : %s \n", sigtype);
	  }
	else 
	  {
	  strcpy(sigtype, argv[6]);
	  upper_case(sigtype);
	  printf("\n\t*As per user, signature type to generate : %s\n", sigtype);
	  }
	
	
	
//***************************

// Check 7th argument argv[7] on command line : TREETYPE

	if ((argc < 8) ||  EQUALN(argv[7],"-",1 ))
	  {
	  strcpy(treetype,"ITC");
	  printf("\t*As per default, the default TREETYPE is : %s  \n", treetype);
	  }
	else 
	  {
	  strcpy(treetype, argv[7]); 
	  upper_case(treetype);
	  printf("\t*As per user, TREETYPE is : %s  \n", treetype); 
	  }
	
	
	//exit(1); 		// For testing

	
//***************************	
	
// Check that parameters are valid 

	ibufs = no_ch;					// number of channels 
	
										// CHECK only one for the moment: no MEAN+TEXT+...
	check_param2(sigtype, 1);			// checks on type of signature, acceptable?
										// It also calculates "stype" and "spa_size"

	check_param4(treetype);			// check the TREETYPE parameter
	

	goto Main_Prep;					// bypass PCI specific stuff

//*****************  
	  
	  // **OLD **  PCI STUFF about Inputs ...
	  
	  
#ifdef PARTIAL_GDAL
	
/* Expand list of signatures if ranges are being used by user*/
/* This make possible short hand notation using PCI ranges and in this case,
   also make possible for up to 32 signatures (classes) to be used */
/* check that maximum number (BITMAPS) of classes not execeeded */

	param_expand(argcnt[4], signsegI, signseg, &isigns);
	if(isigns > BITMAPS ) 
		{fprintf(stderr,"\n ERROR - max # of classes(32)  exceeded \n"); exit(-1); }

/*	printf("\nSIGNSEG as expanded : \n");
	for (i = 0; i < isigns; i++) printf(" %d ", signseg[i]); printf("\n"); 
*/

/* possibly need to also expand list of output class bitmaps */
/* check that maximum number (BITMAPS) of classes not execeeded */

	param_expand(argcnt[5], iclassbit, classbit, &iclass);
	if(iclass == 0)	iclass = isigns;
	if(iclass > BITMAPS ) 
		{fprintf(stderr,"\n ERROR - max # of classes(32)  exceeded \n"); exit(-1); }

/* Check that parameters are valid  */

	ibufs = argcnt[1];					/* number of channels */
	/* qsort(dbic, argcnt[1], sizeof(int), compare_ints);  dont sort you'll screw-up SCHINDX */

	check_param2(sigtype, argcnt[8]);	/* checks on type of signature, acceptable? */
  						/* It also calculates "stype" and "spa_size" */

	//check_param4(treetype, argcnt);			// check the TREETYPE parameter 
	check_param4(treetype);			// check the TREETYPE parameter -- GDAL version
	
/* Check if species codes are to be used (otherwise plain index(1-32) are used) */

	speciescode = 1;		/* a speciescode flag NOW USED ALL THE TIME */

	if(argcnt[6] == 0)
	  {
	  for(kk=0; kk<BITMAPS; kk++) clascode[kk] = kk+1; 
	  /* printf("\nCLASCODE : \n"); for (i = 0; i < BITMAPS; i++) printf(" %d ", clascode[i]); printf("\n"); */
	  }

	if(argcnt[6] > 0)
	  {

	  //for(kk=argcnt[6]; kk<=BITMAPS; kk++) clascode[kk] = UNKNOWN; 	// mark the rest of the array as unknown species code 
	  for(kk=argcnt[6]; kk<=BITMAPS; kk++) clascode[kk] = 998; 	// mark the rest of the array as unknown species code 
	  /* printf("\nCLASCODE : \n"); for (i = 0; i < BITMAPS; i++) printf(" %d ", clascode[i]); printf("\n"); */
	  }

	if( (argcnt[6] > 0) && (argcnt[6] < isigns))
	  {

	/* check if this is due to the limit of 16 inputs in IMPStatus() AND that there are actually more codes in CLASCODE */

	  fprintf(stderr,"\n### WARNING - Checking if CLASSCODE has more than 16 entries \n  ");
	  /* printf("\nCLASCODE : \n"); for (i = 0; i < BITMAPS; i++) printf(" %d ", clascode[i]); printf("\n"); */

 	  actval = IMPGetNumeric("CLASCODE","I",numbuf,32);
	  if (actval > 16 ) 
		{
		for(kk=0; kk<isigns; kk++) clascode[kk] = numbuf[kk];
		printf("\nCLASCODE : \n"); for (i = 0; i < BITMAPS; i++) printf(" %d ", clascode[i]); printf("\n");
		goto l22;
		}


l11:	  fprintf(stderr,"\n### WARNING - Not same # of clascode as signatures. OK to continue(y/n): ");


	  scanf("%s",answer);		/* gets(answer); */

 	  upper_case(answer);
  	  if ( (answer[0] != 'Y') && (answer[0] != 'N') ) goto l11;
  	  if (answer[0] == 'N') 
  	    {
  	    fprintf(stderr,"\n ***** Exiting program ***** \n");
  	    exit(-1);
  	    }
  	    if (answer[0] == 'Y') 
  	    {
  	    fprintf(Report,"\n CONTINUING in spite of warning (not same # of clascode as signatures)\n");
  	    }
	  }
	   
/* Check  parameter SCHINDX */	  
	
l22:	if(argcnt[2] > 0) schindx_flag = 1;

	if( (argcnt[2] > 0) && (argcnt[2] < ibufs))
	  {
	  fprintf(stderr,"\n\n### ERROR - Not same # of schindx as channels.\n\n");
	  fprintf(stderr,"Please check the following: SCHINDX may not be needed in your case,\n");
	  fprintf(stderr,"however, if SCHINDX is needed (difference in channel used between signature\n");
	  fprintf(stderr,"creation and this ITCSC run), it must conform to the above criteria.\n\n");
	  exit(-1);
	  }
/* For manually entered index - Program channel indexes need to be zero based  */
	  
	if(argcnt[2] > 0) for (i = 0; i < ibufs; i++) schindx[i] = schindx[i] - 1;	

/* create default channel indexes if none were provided (just in case) */

	if(argcnt[2]==0)  for (i=0; i < argcnt[1]; i++) schindx[i] = i ;
	
/* Covariance matrix type to be used in maximum likelihood equations
  (Note: This is independent of the cov. mat. used to produce PC1 for ITC TCL) */

	cov_mat_type = SP;		/* Use species specific covariance matrices by default */

	/* but for TCL (for now), use covariance matrices wich is an average of the species' cov. mat. */

	for (ii = 0; ii < 3; ii++) if((st[ii]==TCL)||(st[ii]==TCL2)) cov_mat_type = GS; 
	
	// for (ii = 0; ii < 3; ii++) if((st[ii]==TCL)||(st[ii]==TCL2)) cov_mat_type = MA; 

/* Check if no threshold */

	if (argcnt[9] == 0)  threshold = (float) 0.99;     /* default value */

/* A default value of 0.99 will classify a lot, but because of the CHI-SQUARE test, 
   it will not classify tree too ridiculously away from existing signatures.
   Previous default of 1.0 would classify just about anything */



// Check PCI env. vars (in PRM.PRM) about special research mode to get 2nd class (2nd likelihood) instead of 1st


	  IMPGetChar("SC_CLASS2", class2_in, 64);
	  if( strncmp(class2_in,"TRUE",3) == 0 ) class2_flag = 1;
	  if( strncmp(class2_in,"YES",3) == 0 ) class2_flag = 1;

	  if(class2_flag) 
		fprintf(stdout,"\n\n\t #######  SPECIAL VERSION will output 2nd best class in field (vector mode)\n\n");



/* Open database file - test if file name exists */

	idb_fp = IDBOpen(file,"r+" );
	if (idb_fp == NULL) 
	  {
	  fprintf(stderr,"File error. GDBOpen did not work.  *** Aborting.\n");
	  exit(-1);
	  }
	
	fprintf(Report,"\nOpening image file: %s \n", file);


/* Get size of image in file and report it*/

	IDBSizeInfo(idb_fp, &xsize, &ysize, &channels);
	//CheckChan(idb_fp, "DBIC", dbic, argcnt[1]);
	Pixels=xsize; Lines=ysize; Channels=channels;
	
	fprintf(Report, "\nImage information for PCI file: %s\n", file);
	fprintf(Report,"Pixels:  %d \tLines:  %d \tChannels:  %d\n",Pixels, Lines, Channels);

	IDBPixelSize(idb_fp,IDB_READ,&xpixsz,&ypixsz,pixunit);
	pixunit[7] = '\0';
	fprintf(Report,"Pixel size is %g x %g %s \n", xpixsz, ypixsz, pixunit);

	fprintf(Report,"Size of one image channel: %I64d bytes \n", (int64) Pixels*Lines);

	bmsize = (Pixels * (int64)Lines + 7) / 8; 
	printf("\nBitmap size (in bytes): %I64d \n", bmsize);

	/* If there is no need to be analysed image by sections (i.e., images fit in memory) */

	dbiw[0]=0; dbiw[1]=0; dbiw[2]=Pixels; dbiw[3]=Lines;

/* Get general geographic mapping information */

	IDBGeorefIO(idb_fp, IDB_READ, geosys, &topleftX, &transformX, &topleftY, &transformY);
	fprintf(Report,"\nImage geographic referencing information: %s \n", geosys);
	fprintf(Report,"(Offset, multiplier) relative to pixel & line coord: \n (%f, %f) (%f, %f) \n", 
					topleftX, transformX, topleftY, transformY);

/* Check input image type */

	for (i = 0; i < argcnt[1]; i++)
	{
	data_type =  GDBChanType(idb_fp, dbic[i]);
	if (data_type != CHN_8U && data_type != CHN_16U)
	      { fprintf(stderr,"\n** Invalid input channel type **\n\n");  exit(-1);}
	}

/* Read image channels into buffer  */

	/* PREVIOUS VERSIONS, FULL images were read here */

/*	fprintf(Report, "\nReading image into buffers.  Please wait...\n\n");
	for (i = 0; i < argcnt[1]; i++)
		imagebuffer[i] = alloc_read_img(idb_fp,xsize,ysize,dbic[i],&get_pix_val[i]); 
*/


/* Check if using manual crown segment instead of ITC bitmap */

	IDBSegInfoIO(idb_fp, isolbit, IDB_READ, &segflag, &segtype, segname, &start, &length);

	if ( (segtype == SEG_VEC) && (vmode != 1) )
	  {
	  fprintf(stderr,"\n *** ERROR *** \n");
	  fprintf(stderr,"\n Normally, ISOLBIT is a bitmap of ITCs generated by ITCISOL.\n");
	  fprintf(stderr,"Here, ISOLBIT was found to correspond to a vector layer.\n");
	  fprintf(stderr,"If this is your intention, then TREETYPE=SLayer must be used.\n\n");
	  exit(-1);
	  }
	if ( (segtype == SEG_BIT) && (vmode != 0) )
	  {
	  fprintf(stderr,"\n *** ERROR *** \n");
	  fprintf(stderr,"\nISOLBIT was found to correspond to a bitmap, but TREETYPE=SLayer was used.\n");
	  fprintf(stderr,"If ISOLBIT is a bitmap of ITCs, TREETYPE=ITC must be used.\n\n");
	  exit(-1);
	  }


/* Read ITC bitmap into its buffer (in research mode, it could be a manually delineated tree layer) */

	if(!vmode)			// not vector mode, normal mode
	  {
	  fprintf(Report,"\n\t\tReading ITC bitmap (%d) ...\n", isolbit);
	  isolbitbuffer = alloc_read_bmp(idb_fp, xsize, ysize, isolbit);
	  }
	else 
	  {
	  fprintf(Report,"\nAllocating memory for a temporary ITC bitmap\n");
	  isolbitbuffer = alloc_read_bmp(idb_fp, xsize, ysize, 0);
	  }

/* Create protection buffer for fill() operation */
/* (in case ITC bitmap was obtained by subsetting a bigger image, thus lost buffer) */

	safety_zone(isolbitbuffer);

/* Get space and initialize other working bitmaps to zero */

	fprintf(Report,"Allocating memory for other temp bitmaps\n");

	tempbitbuffer = alloc_read_bmp(idb_fp, xsize, ysize, 0);	/* destroyed when searching for crowns */
	tempbitbuf2 = alloc_read_bmp(idb_fp, xsize, ysize, 0);		/* destroyed when painting crowns in class BM*/

	min_BMs=3;
	if (argcnt[7] == 1) 	min_BMs=4;				/* one more for the LIT mask */

	mem_needed_BM_min = min_BMs * bmsize;

	printf("Memory needed for a minimum(%d) of internal full bitmaps : %5.3f GB\n", min_BMs,
				(float) mem_needed_BM_min / pow(1024,3) );
	mem_needed_BM = mem_needed_BM_min;

/* Class bitmap data is generated on the fly in the "by sections" mode. 
   However, they need to be opened now.
   NOTE: Class bitmaps are not generated in vector layer mode */

	if (!vmode)
	{
	fprintf(Report, "\nPreparing bitmap segment for output classes. \n\n");

	class_bitmaps(idb_fp, classbit, iclass, signseg, dbic, Pixels, Lines, isolbit);

	mem_needed_BM = (min_BMs + iclass) * bmsize;

	printf("\nMemory needed IF all internal full bitmaps were used: %5.3f GB\n", (float) mem_needed_BM / pow(1024,3) );
	}

/******/

/* Open input layer of manual tree crowns to be classified and create field CLASS */

  if (vmode)
    {
    fprintf(Report,"\nOpening input layer %d of manual (polygons) tree crowns \n", isolbit);
    hlayer = GDBGetLayer(idb_fp, isolbit);
    if (hlayer == NULL) 
      {
      fprintf(Report,"\n *** ERROR opening vector layer %d \n", isolbit);
      exit(-1);
      }
  
/* Get geographic projection information */

  IDBGetProjectionInfo(hlayer, &sProj); 

  fprintf(Report,"Vectors geographic information:  %s \n", sProj.Units);
  /*fprintf(Report,"(Offset, multiplier) relative to image geographic info: \n (%f, %f) (%f, %f) \n", 
					  sProj.XOff, sProj.XSize, sProj.YOff, sProj.YSize); */
									
  /* this would deal with it better than PCI ImageWorks (i.e., thus results difficult to see) */
  /* if( EQUALN(sProj.Units,"PIXEL",5) )  {topleftX=0; topleftY=0; transformX=1; transformY=1;} */
  /* if( EQUALN(sProj.Units,"METRE",5) )  { topleftY = ysize; transformY = -1; } */
  /*   fprintf(Report,"\n (offset, multiplier) to be used: (%f, %f) (%f, %f) \n", 
					topleftX, transformX, topleftY, transformY); */
  
  if( ! EQUALN(sProj.Units,geosys,5) )
    {
    fprintf(Report,"\n\n ### ERROR ### \n");
    fprintf(Report,"\n Geographic projection of the vector layer not the same as that of the image.\n\n");
    exit(-1);
    }

// Initialize fields to unset value (upon CREATION only)

  sField.nInteger = UNSET;

/* Generate the field format */

  FieldFormat = GDBGenFieldFormat(GDBFieldTypeInteger, 4, 0, TRUE);
    
/* Check if parameter CLASSR already define */
  
  IMPGetChar("CLASSR", classr, 64);
  if( strcmp(classr,"NOTSET") != 0 )
    {
    strcpy(field_name, classr);
    fprintf(Report,"\nAs per preset CLASSR user variable, results will be written in field '%s'.\n",field_name);
    }
    else
    {	 
    /* Ask user for a specific name for the "CLASSR" field */
     
l2:
    fprintf(stdout,"\nClassification results will be found in a new field of the input vector layer %d.\n\n",isolbit);
    fprintf(stdout,"What is the NAME you wish to give to that field (e.g.: class_lit_3ch) ???");

    scanf("%s",field_name);		/* gets(field_name); */
    
    if ( (iField = GDBGetFieldIndex(hlayer, field_name)) > -1) 		/* check if field exist */
	{
l1:	fprintf(stdout,"\n\n ### Field %s (%d) already exist. Is it OK to overwrite it (y/n)?",field_name,iField);
	scanf("%s",answer);		/* gets(answer); */
 	upper_case(answer);
  	if ( (answer[0] != 'Y') && (answer[0] != 'N') ) goto l1;
  	if (answer[0] == 'N') goto l2;
  	if (answer[0] == 'Y') 
  	  fprintf(Report,"\n\t## Field %d (%s) will be overwritten \n", iField, field_name);
	}
     }
 
/* Create field (if does not exist) to store classification results on a tree by tree basis */
  
  if (GDBGetFieldIndex(hlayer, field_name) == -1)	/* if field not already there, create it */
    {
    iField = GDBAddField(hlayer, field_name, GDBFieldTypeInteger, &sField); 
    if (iField == -1) fprintf(stderr,"\n *** ERROR *** creating a field \n");
    }


/* Set field format of that field and confirm */


  iField = GDBGetFieldIndex(hlayer, field_name);
  GDBSetFieldFormat(hlayer, iField, FieldFormat);
  fprintf(Report,"\nIndividual tree classification results will be written to field '%s' (%d) of vector layer %d.\n",
							field_name,iField,isolbit);


  }	/* end of *** if vector mode **** */


/******/
	

/* Check if running in point mode, i.e., BITBOUND=POINTS */

	IMPGetChar("BITBOUND",boundtype,10);
	if (strcmp(boundtype, "POINTS") == 0) PointsFlag = 1;
	if ( (argcnt[7] == 1) && PointsFlag )
	  {
	  fprintf(Report,"\n *** WARNING: Please check on the extra bitmap (EXTBIT): %d \n", ltmask);
	  fprintf(Report,"                because you are still in pointer mode: BITBOUND=POINTS, thus the\n");
	  fprintf(Report,"                EXTBIT may have recently been used as a crown pointer (in SSG?) \n\n");
	  }

/* If an extra mask (e.g., LIT) is used, read it in  */

	/* (unless it was unintentionally left on cause ITCSSG was run in POINTS mode) */
	/* if ( (argcnt[7] == 1) && !(PointsFlag) ) */

	if ( (argcnt[7] == 1) )
	  {
		fprintf(Report,"\n\t\tReading extra mask (LIT,TT,etc.) bitmap ...xsize=%d ysize=%d \n",xsize, ysize);
		extmaskbuffer = alloc_read_bmp(idb_fp, xsize, ysize, ltmask);
		ExtMaskFlag = 1;
		lt_mask=ltmask;
	  }
	else	/* if not, still need an empty buffer */
	  {
		extmaskbuffer = alloc_read_bmp(idb_fp, xsize, ysize, 0);
		ExtMaskFlag = 0;
	  }

/* Is a list of all ITC signatures required ? */

	DBOUTPUT = 0;
	if (argcnt[11]>0) DBOUTPUT = 1;


#endif
	
//*******************************************************************		
	
//*******************************************************************	
	

Main_Prep:




// Get ITC bitmap (if needed) 


	if( ! vmode )		// Read the ITC bitmap
	  {
	  fprintf(Report, "\n\t\t*Reading ITC bitmap into memory.\n");
	  //isolbitbuffer = alloc_read_bmp(idb_fp, xsize, ysize, isolbit);	//PCI version
	  if (EQUALN(Extension,"pix",3))	isolbitbuffer = read_bitmap(filename, isolbit);		// GDAL version for PCI files

	  if (EQUALN(Extension,"tif",3))
	    {
		imaFile_opened = 0;					// kluge
		isolbitbuffer = read_bitmap(file_ITC, 1);					// GDAL version for TIF files	
		imaFile_opened = 1;		// kluge	  
	    }
	  }
	else			/* dont use the ITC bitmap */
	  {
	  //isolbitbuffer = alloc_read_bmp(idb_fp, xsize, ysize, 0);
	  isolbitbuffer = (PixVal *) CPLMalloc(bmsize);
	  for(kk=0; kk<bmsize; kk++) isolbitbuffer[kk] = 0x00; 			// make sure zeroed properly   
	  }


// GET  EXTRA Mask (e.g., LIT) if used

	if (ExtMaskFlag)  
	  {
	  fprintf(Report, "\n\t\t*Reading Extra Bitmap (e.g., LIT) into memory.\n");
		//extmaskbuffer = alloc_read_bmp(idb_fp, xsize, ysize, extmask);		//PCI version

		if (EQUALN(Extension,"pix",3))	extmaskbuffer = read_bitmap(filename, extmask);				// GDAL version for PCI files


	  if (EQUALN(Extension,"tif",3))	
	    {		
		imaFile_opened = 0;		// kluge
		extmaskbuffer = read_bitmap(file_EXT, 1);						// GDAL version for TIF files	
		imaFile_opened = 1;		// kluge	  
	    }		  
	  }
	else
	  {
		//extmaskbuffer = alloc_read_bmp(idb_fp, xsize, ysize, 0);	// allocate empty bitmap array (my style)
		extmaskbuffer = (PixVal *) CPLMalloc(bmsize);				// allocate empty bitmap array
		check_mem(extmaskbuffer);
	  }
	
	

/* Create protection buffer for fill() operation */
/* (in case ITC bitmap was obtained by subsetting a bigger image, thus lost buffer) */

	safety_zone(isolbitbuffer);

/* Get space and initialize other working bitmaps to zero */

	fprintf(Report,"\n\tAllocating memory for other temp bitmaps\n");

	//tempbitbuffer = alloc_read_bmp(idb_fp, xsize, ysize, 0);	/* destroyed when searching for crowns */
	//tempbitbuf2 = alloc_read_bmp(idb_fp, xsize, ysize, 0);		/* destroyed when painting crowns in class BM*/

	tempbitbuffer = (PixVal *) CPLMalloc(bmsize);
	tempbitbuf2 = (PixVal *) CPLMalloc(bmsize);

	//exit(-1);

	printf("\n----------------------------\n\n");

// Read signatures - check that the results are valid and modify if necessary

	fprintf(Report, "\n\t\tReading species signatures\n");
	fprintf(Report, "\t\t----------------------------\n");

	for (species = 0; species < isigns; species++) 
	{
	fprintf(Report, "\nSpecies %d : Reading species signature %d \n", species+1, signseg[species]);

#ifdef PARTIAL_GDAL
	if (EQUALN(Extension,"pix",3))
	  current_signs[species] = read_segment(idb_fp, signseg[species]);			// for PCI files (Use PCI lib)
#endif
	
	if (EQUALN(Extension,"tif",3))
	  {		
	  sig_fp = fopen(file_SIG[species],"rb");								// for separate (.sig) signature files
	  fprintf(Report, "\nReading signature file %s : \n",file_SIG[species]);
	  
	  
	  //current_signs[species] = (result_node *) CPLMalloc(sizeof(result_node));
	  current_signs[species] = (result_node *) CPLMalloc(sizeof(result_node));
		//printf("\nSize of result node as input : %zd \n",  sizeof(result_node));
	  fread(current_signs[species], sizeof(result_node), 1, sig_fp);
	  fclose(sig_fp);
	  
/*	  // DEBUGGING TEST
	  sig_fp2 = fopen("temp.sig","wb");
	  fwrite(current_signs[species], sizeof(result_node), 1, sig_fp2);	  
	  fclose(sig_fp2);		// close that binary file
*/	  
	  
	  }

	//view_results(current_signs[species], signseg[species]);		// display signature just read
	
	
	

	if (current_signs[species]->channels != no_ch )
	  {
	  printf("\n\n\t **ERROR** number of channels in signature and \"presently in use\" dont match\n");
	  printf("\nHistorical channel sub-setting or shuffling not implemented yet	 \n\n"); 
	  exit(-1);
	  }		
	
	

	//check_signature_results(current_signs[i], signseg[i], isolbit, ltmask);
	  
	  /* May be needed to reorg. signature if a subset of features is being used or,
		if SCHINDX is being used (because channels ID are different between the signatures
		and this ITCSC run (e.g., signature extension from another PCI file)) */	

	   //current_signs[i] = mod_sign_org(current_signs[i], dbic, schindx);
	  	 
	}


	//exit(-1);
		
	
/******************************************************************/

/* 		MAIN PROCESSING PART */

/******************************************************************/

//Main:

fprintf(Report, "\n--------------------\n\n");
fprintf(Report, "\n\t\tMAIN PROCESSING PART\n\n");
fprintf(Report,"Generating a signature for each Individual Tree Crown (ITC) and classifying\n");
fprintf(Report,"them into the closest species using a --maximum Likelihood-- decision rule.\n");
fprintf(Report,"\n\t Signature type: "); print_sig_types(stype); 	
fprintf(Report,"\n");


/* Make copies of isolbitbuffer (to be destroyed while gathering MSS info) */

	memcpy(tempbitbuffer, isolbitbuffer, bmsize);

  	//for (iii=0; iii<bmsize; iii++) *(tempbitbuffer+iii) = *(isolbitbuffer+iii); 

	memcpy(tempbitbuf2, isolbitbuffer, bmsize);		/* destroyed burning class bitmaps */

  	//for (iii=0; iii<bmsize; iii++) *(tempbitbuf2+iii) = *(isolbitbuffer+iii); 

	
/* Calculate covariance matrix, and inverse and determinant */

	cov_matrix();
	
	//exit(-1);

/* Create (i.e., start) list of nodes for tree information */

	tree_list = new_tree_node();
 	previous_tree = tree_list;
	
	goto WHOLE;
	
	
	
	
#ifdef PARTIAL_GDAL	
	
	
	
/* Checking memory space needed for image channels  */

	fprintf(Report, "\n\tChecking memory space needed for image channels \n");

	for (i = 0; i < argcnt[1]; i++)
	  {
	  //if (GDBChanType(idb_fp, dbic[i]) == CHN_8U) 
		  
	  if (data_type == CHN_8U) 	    
	  {
	  mem_needed = sizeof(char) * dbiw[2]*dbiw[3]; no_8bCHs++;}
	  else {mem_needed = sizeof(int32) * dbiw[2] * dbiw[3]; no_16bCHs++;}
	  mem_needed_IMA += mem_needed;
	  printf("\nMemory needed for channel %d is %5.3f GB\n", dbic[i], (float)mem_needed/pow(1024,3));
	  }
	fprintf(stdout,"\n*NOTE*: To load all image channels (full image) one would need %5.3f GB\n", (float) mem_needed_IMA/pow(1024,3));
	printf("\n\tAs mentionned earlier, memory needed to load all FULL bitmaps : %5.3f GB\n", (float) mem_needed_BM / pow(1024,3) );
	mem_needed_TOT = mem_needed_BM + mem_needed_IMA;
	fprintf(stdout,"\n\tTo load all FULL channels AND bitmaps one would need %5.3f GB\n\n", (float) mem_needed_TOT/pow(1024,3));


/* Getting info about available memory from a "soso" PCI utility */	
/* All this will need to change when better "int64 IMPGetMemory()" and/or Windows 7 */

	
	mem_avail_SYS =  (unsigned) IMPGetMemory(NULL);		 /* will go negative after 2GB, but still meaningful */
	if (mem_avail_SYS < pow(2,30) ) mem_avail_SYS = mem_avail_SYS + pow(2,31);   	/* if <1GB then probably more than 2GB */
	/*fprintf(stdout,"Memory available on this Windows_XP system is %5.3f GB\n", mem_avail_SYS /pow(1024,3)); */

	/*
	if (mem_avail_SYS > pow(2,31) ) mem_avail_SYS = pow(2,31);   if memory bigger than 2GB, use 2GB 
	mem_avail_SYS = mem_avail_SYS -  mem_avail_SYS/3;	 leave 33% spare room for the prog. and O/S disk accesses 
	fprintf(stdout,"Memory available that will be used %5.3f GB\n", mem_avail_SYS /pow(1024,3));
	*/

/* Ask user (via PCI env. vars) how much memory should be used (in rough GBytes (e.g., 2.5))*/

	actval = IMPGetNumeric("ITC_MEM","R",rnumbuf,1);
	if (actval == 1) 
	  {
	  mem_avail_SYS = (int64) (rnumbuf[0] * pow(1024,3));
	  fprintf(stdout,"\nMemory available as per user (via ITC_MEM env. vars.) is %5.3f GB\n", mem_avail_SYS /pow(1024,3));
	  fprintf(stdout,"\t**** This value will be enforced. ****\n\n");
	  }

/* If enough memory available, deal with full images and full bitmaps */

	if (mem_needed_TOT <= mem_avail_SYS ) goto WHOLE;	


/* if not enough memory to work with full images and bitmaps, WORK BY IMAGE SECTIONS */

	fprintf(stdout,"\n*** Too much memory needed to operate on the full images. %s will function by sections ...\n",PROG_NAME);

	if(vmode)
	  {
	  mem_avail_SECT =  mem_avail_SYS - mem_needed_BM_min;
	  fprintf(stdout,"Memory available (with a min. of full BMs) for image sections is %5.3f GB\n\n",(float) mem_avail_SECT/pow(1024,3));
	  sect_siz = mem_avail_SECT / ( no_8bCHs*Pixels + no_16bCHs*Pixels*4);  	/* No. of lines for each section */
	  sect_siz = (sect_siz/10) *10 ; 	/* just getting a rounder (cuter) number */
	  
	  /*
	  sect_siz = 6000; 		
	  fprintf(stdout,"\n*** IMPOSING to use IMAGE SECTIONS of %d lines ***\n", sect_siz); 
	  */

	  }

	if(!vmode)
	  {
	  mem_avail_SECT =  mem_avail_SYS - mem_needed_BM_min;
	  fprintf(stdout,"Memory available (with a min. of full BMs) for image and output bitmap SECTIONS is %5.3f GB\n\n",
					(float) mem_avail_SECT/pow(1024,3));
	  sect_siz = mem_avail_SECT / ( (no_8bCHs + (iclass/8+1) )*Pixels + no_16bCHs*Pixels*4);	/* No. of lines for each section */
	  sect_siz = (sect_siz/10) *10 ; 	/* just getting a rounder (cuter) number */
	  }


	fprintf(stdout,"%s will load images by reasonable sections of %d lines\n\n", PROG_NAME, sect_siz);

	if(vmode) goto VEC_SECTIONS;

	goto BY_SECTIONS;

#endif

/**************************************************************************************************/

WHOLE:

// Read image channels into buffer  (PCI Stlye)

/*
	fprintf(Report, "\nReading full image (and all requested channels) into buffers.  Please wait...\n\n");
	for (i = 0; i < argcnt[1]; i++)
		imagebuffer[i] = alloc_read_img(idb_fp,Pixels, Lines,dbic[i],&get_Pval[i]);
*/

	
// Read image channels into buffer *GDAL Style)

	fprintf(Report, "\n\t\tReading full images into buffers.  Please wait...\n\n");
	printf("\n\t\t---------------------------------------------------\n");

/*	for (i = 0; i < no_ch; i++)
		imagebuffer[i] = alloc_read_img(idb_fp,xsize,ysize,dbic[i],&get_pix_val[i]); 
*/

// GDAL Style  in FULL IMAGE  MODE

	for (i = 0; i < no_ch; i++)
	  {
	  imagebuffer[i] =  (void *) read_image(filename, input_ch[i]); 		// 8bit only for the moment
	  //get_Pval[i] = get_u8;
	  get_Pval[i] = get_pix_val;	// remember 8bit or 16 bit as returned from previous read_image()
	  printf("\n-----------------------------\n");
	  }


// Get space and initialize OUTPUT class bitmaps to zero */

	if(!vmode)		/* output class bitmaps not use with vector manual crowns */
	  {
	  fprintf(Report,"\nAllocating memory for %d output bitmaps\n",isigns);
	  for (i = 0; i < isigns; i++) 
		//classbitbuffer[i] = alloc_read_bmp(idb_fp, xsize, ysize, 0);
		classbitbuffer[i] =  (PixVal *) CPLMalloc(bmsize);
	  }

	
/* Process WHOLE image and gather information on all trees for specified type of signature */

	sect_siz = Lines;
    //for(k=0; k<100000; k++)  poly_done[k] = 0; 		// flag about polygon (shape) done or not -- to do poly only once if by sections

	printf("\n------------------------------------------------------------------\n");
	fprintf(Report, "\n\t\tClassifying Individual Tree Crowns ...\n\n");

	if(!vmode) scan_for_tree(Pixels, Lines, tempbitbuffer);

	//if(vmode && PCI_File) loop_for_tree(Pixels, Lines, tempbitbuffer, hlayer);	

	if(vmode && TIF_File) loop_for_tree_g(Pixels, Lines, tempbitbuffer);
	

	printf("\n----------------------------------\n");

	goto OUTPUT;



/****************************************************************************************************/

#ifdef PARTIAL_GDAL


/* Process image by SECTIONS and classify all ITCs  */

BY_SECTIONS:


/* Nomemclature is in lines, thus starting at 1 */

	by_sections = 1;		/* by_sections flag */
	sect_count = 1;			/* section counter */
	ysize = sect_siz;		/* deal with sections of size "sect_siz" */
	sect_overlap = 80;		/* MUST BE a multiple of 8 to facilitate bitmap acces */

LOOP:

	next_sect_flag = 0;		/* Reset next section flag */

/* backup not to miss any crown (and need full crowns for good classifiaction */

	if (sect_count>1) next_sect_start = sect_end - sect_overlap;		/* a buffer zone */
	sect_limit = next_sect_start + sect_siz;
	if (sect_limit >= Lines) {sect_limit=Lines; sect_siz=Lines-next_sect_start; ysize=sect_siz;}
	line_offset = next_sect_start-1 ;      /* offset (in lines) when using full image (bitmap) */

	dbiw[0]=0; dbiw[1]=(next_sect_start-1); dbiw[2]=Pixels; dbiw[3]=sect_siz;

	fprintf(stdout,"\n\tReading next image section DBIW= %d %d %d %d \n",dbiw[0],dbiw[1],dbiw[2],dbiw[3]);

/* copy image channel sections into buffers - check for error in data type */

	fprintf(Report, "\nReading image sections (%d channels) into buffers.  Please wait...\n\n",argcnt[1]);
	for (i = 0; i < argcnt[1]; i++)
	  imagebuffer[i] = alloc_read_img_dbiw(idb_fp, Pixels, Lines, dbic[i], &get_Pval[i], dbiw);

/* Allocate memory to output (class) bitmaps SECTIONS (here ysize=sect_siz) */

	fprintf(Report,"\nAllocating memory for %d output bitmap sections\n",isigns);

	for (i = 0; i < isigns; i++) 
		{ 
		classbitbuffer[i] = alloc_read_bmp(idb_fp, xsize, ysize, 0);
		/* fprintf(stdout,"\nFor class %d address pointer to partial bitmap is %I64d \n", i, (int64)classbitbuffer[i]); */
		}

/* Re-generating the ITC bitmaps as parts have been destroyed 
		(and put a safety zone around the working part) */

	memcpy(tempbitbuffer, isolbitbuffer, bmsize);
  	//for (iii=0; iii<bmsize; iii++) *(tempbitbuffer+iii) = *(isolbitbuffer+iii); 
	dbiw_safety_zone(tempbitbuffer,dbiw);

	memcpy(tempbitbuf2, isolbitbuffer, bmsize);	
  	//for (iii=0; iii<bmsize; iii++) *(tempbitbuf2+iii) = *(isolbitbuffer+iii); 
	dbiw_safety_zone(tempbitbuf2,dbiw);


/* Running classifier on that section of image */

	printf("\n----------------------------------\n");
	
	printf("\nClassifying Individual Tree Crowns (ITCs) of that image section...\n\n");

	scan_for_tree(xsize, ysize, tempbitbuffer);

	printf("\n----------------------------------\n");
	

/* Write out sections of class bitmaps to PCI file */

	fprintf(Report, "\n\nWriting class bitmap sections to %d segments.  Please wait...\n\n",isigns);
	
	if (sect_count==1) {dbow[0]=0; dbow[1]=dbiw[1]; dbow[2]=Pixels; dbow[3]=dbiw[3];}

	if (sect_count>1) 		/* only new part is written */
			{dbow[0]=0; dbow[1]=dbiw[1]+sect_overlap; dbow[2]=Pixels; dbow[3]=sect_siz-sect_overlap;}

	fprintf(stdout,"\n Writing class bitmaps section DBOW= %d %d %d %d \n",dbow[0],dbow[1],dbow[2],dbow[3]);

    for (i = 0; i < isigns; i++) 
	{
	  pp = classbitbuffer[i];
	  if (sect_count>1) pp = classbitbuffer[i] + (sect_overlap/8)*Pixels;

	  /* fprintf(stdout,"\nFor class %d address pointer to partial bitmap is %I64d \n", i, (int64)pp); */


 	  fprintf(stdout,"\nWriting bitmap section to PCI file for class %d ... \n", i); 


	  GDBBitmapIO(idb_fp,GDB_WRITE,classbit[i],dbow[0],dbow[1],dbow[2],dbow[3],pp,Pixels,dbow[3]); 


	}

/* Free memory and loop for other sections of images */

	for (i = 0; i < argcnt[1]; i++) free(imagebuffer[i]); 
	for (i = 0; i < isigns; i++) free(classbitbuffer[i]);

	//IMPTime(timedate,4);
	time (&rawtime); timeinfo = localtime (&rawtime);
	fprintf(stdout,"\n\t\t ####  %d section(s) analysed - %s \n", sect_count, asctime(timeinfo));
	sect_count = sect_count + 1; 

	if ( (dbow[1]+dbow[3]) > (Lines - 4) ) goto OUTPUT;


	if (next_sect_flag) goto LOOP;
	

	goto OUTPUT;


/****************************************************************************************************/

/* For Slayer mode, process image by SECTIONS and classify manual (polygon) trees as an entry in a field */

VEC_SECTIONS:

	fprintf(stdout,"\n Got into VEC_SECTIONS ...\n");

/* Nomemclature is in lines, thus starting at 1 */

	by_sections = 1;		/* by_sections flag */
	sect_count = 1;			/* section counter */
	ysize = sect_siz;		/* deal with sections of size "sect_siz" */
	sect_overlap = 80;		/* MUST BE a multiple of 8 to facilitate bitmap acces */
    for(k=0; k<100000; k++)  poly_done[k] = 0; 	/* flag about polygon (shape) done or not -- to do poly only once */

LOOP2:

	next_sect_flag = 0;		/* Reset next section flag */

	/* backup not to miss any crown (and need full crowns for good classifiaction */

	if (sect_count>1) next_sect_start = sect_end - sect_overlap;		/* a buffer zone */


	sect_limit = next_sect_start + sect_siz;

	if (sect_limit >= Lines) {sect_limit=Lines; sect_siz=Lines-next_sect_start+1; ysize=sect_siz;}

	line_offset = next_sect_start-1 ;      /* offset (in lines) when using full bitmap */

	dbiw[0]=0; dbiw[1]=(next_sect_start-1); dbiw[2]=Pixels; dbiw[3]=sect_siz;

	fprintf(stdout,"\nReading next image section (with some overlap) DBIW= %d %d %d %d \n",dbiw[0],dbiw[1],dbiw[2],dbiw[3]);

	/* copy image channel sections into buffers - check for error in data type */

	fprintf(Report, "\nReading image sections (%d channels) into buffers.  Please wait...\n\n",argcnt[1]);

	for (i = 0; i < argcnt[1]; i++)
	  imagebuffer[i] = alloc_read_img_dbiw(idb_fp, Pixels, Lines, dbic[i], &get_Pval[i], dbiw);


	/* Run classifier on that section of image */


	fprintf(Report, "\nClassifying Individual Tree Crowns of that image section...\n\n");

	//loop_for_tree(xsize, ysize, tempbitbuffer, hlayer);



/* Free memory and loop for other sections of images */

	 for (i = 0; i < argcnt[1]; i++) free(imagebuffer[i]); 

	 fprintf(stdout,"\n\t**** %d section(s) analysed.",sect_count);
	 sect_count = sect_count + 1; 
	 sect_end = dbiw[1]+dbiw[3];

	 fprintf(stdout,"\t (Section ended at line %d)\n",sect_end);

	 if ( sect_end > (Lines - 4) ) goto OUTPUT;

	 goto LOOP2;

#endif

/******************************************************************/

// Output classification summary AND write-out results

/******************************************************************/

OUTPUT:

fprintf(Report, "\n\nClassification Results");
fprintf(Report, "\n----------------------\n\n");
	
for (i = 0; i < isigns; i++)
  {
  if(!speciescode) fprintf(Report,"Species %d: %d \n", i + 1, class_sum[i]);
  if(speciescode)  fprintf(Report,"Species %d(%d): %d \n", i + 1, clascode[i], class_sum[i]);
  classified = classified  + class_sum[i];
  }
fprintf(Report, "\nClassified: %d\n", classified);		
fprintf(Report, "Unclassified: %d\n", unclassified);
fprintf(Report, "Unused: %d\n", unusedITC);

printf("\n----------------------------------\n");	

	
/* Output the histogram of all smallest distances */

/*
histo_fp = fopen("classif_dist.hist","w");
for (kk=0; kk<100; kk++) fprintf(histo_fp," %4d %4d \n", kk, hist_dist[kk]);
fclose(histo_fp);
*/

	
/* Writing class BITMAPs to their segments or bitmap files (tif)
	NOTE: Class bitmaps are not generated in vector layer mode (below)
*/		
	
if(!vmode)
	{
	if(by_sections)
	  fprintf(Report, "\nWriting class bitmap history and description to segments\n\n");
	else
	  fprintf(Report, "\nWriting all class bitmaps (plus history+description) to segments.\n\n");
	  printf("----------------------------------\n");

	//write_BM_histo(idb_fp, classbit, iclass, signseg, dbic, Pixels, Lines, isolbit);	// PCI version
	
	write_GDAL_BM(filename, classbit, iclass, signseg, dbic, isolbit);	// GDAL version
	
	}


	
	
// Vector layer mode - Class results output for each tree within CLASSR (or designated) field


if(vmode)
	{

	//psLayer->SyncToDisk();		// does not do anything 
	//poLayer->SyncToDisk();	
	
	
//printf("END: Address of main input Layer  = %p \n",  psLayer);
//printf("END: Address of temp output Layer  = %p \n",  poLayer);	


	
// To DOUBLE CHECK - Print ALL existing field (for PCI or TIF main files)


  if(TIF_File)
  {
  printf("\nDouble checking on  SHP field existance \n\n");
  for (iField=0; iField < piFDefn->GetFieldCount(); iField++)
      {
	  piFieldDefn = piFDefn->GetFieldDefn(iField);
	  printf(" %d, Type %d, Field Width %d, Precision %d Name: %s\n",
		iField, piFieldDefn->GetType(), piFieldDefn->GetWidth(), piFieldDefn->GetPrecision(),piFieldDefn->GetNameRef() );  
	  }
  }
  
#ifdef PARTIAL_GDAL
 
	if(PCI_File)
	{
	GDBField 	*temp_f;
	int temp_val,	  mShapeId = 1;
     printf("\nDouble checking on PCI layer field existance in shape 1\n\n");
   
	for (iField=0; iField < field_count; iField++)
      {	   
	  temp_f = GDBGetFieldValue(hlayer, mShapeId, iField);
	  printf(" PCI Field %d, Name: %s \t\t Value %d \n", iField, GDBGetFieldName(hlayer,iField), temp_f->nInteger);
	  }	  
	}
	  


 // To DOUBLE CHECK - Print cfield for all feature (polygon trees)
  
/*
    printf("\nDouble checking if field (cField) was assigned correctly in SLAYER\n\n");
	
  	//psLayer->ResetReading();		// To start at beginning of Features, ONLY needed if using GetNextFeature()		

	for (iFeat=0; iFeat<feat_count; iFeat++)		// for each Feature (tree polygon) in the layer
      {
	  //OGRFeature *piFeature;  
      //piFeature = psLayer->GetNextFeature();			// for loops rather than while()
      piFeature = psLayer->GetFeature(iFeat);			// for a more direct loop	(without ResetReading())  
	  //piGeometry = piFeature->GetGeometryRef();
      printf("Feature %d Field %d Assigned class %d \n", iFeat, cField,  piFeature->GetFieldAsInteger(cField));	

	  if(iFeat == 2)  
	    {
	    piFeature->SetField(3, 554);			// #### FORCE a 554 to TEST ####
	    psLayer->SetFeature(piFeature);		// to save feature to disk
        printf("Feature %d Field %d Assigned class %d \n", iFeat, 3,  piFeature->GetFieldAsInteger(3));			
	    }
	  
      OGRFeature::DestroyFeature( piFeature );
	  }   
    printf("\n");
  
	  //psLayer->SyncToDisk();	
	  
  	//psLayer->ResetReading();		// To start at beginning of Features if GetNextFeature() used
*/

	if(PCI_File)
	{
    printf("\nDouble checking if field (cField) was assigned correctly in PCI SLAYER\n\n");		
	GDBField 	*temp_f;	
	for (iFeat=0; iFeat<feat_count; iFeat++)		// for each Feature (tree polygon) in the layer
      {  
	  temp_f = GDBGetFieldValue(hlayer, iFeat, cField);
	  printf(" PCI cField %d, Value %d \n", cField, temp_f->nInteger);	  
	  }   
    printf("\n");
	}

#endif

	
// To DOUBLE CHECK - Print ALL existing field (for PCI or TIF main files)
/*
    printf("\nDouble checking on OUTPUT fields existance \n\n");

  for (iField=0; iField < poFDefn->GetFieldCount(); iField++)
      {
	  poFieldDefn = poFDefn->GetFieldDefn(iField);
	  printf(" %d, Type %d, Field Width %d, Precision %d Name: %s\n",
		iField, poFieldDefn->GetType(), poFieldDefn->GetWidth(), poFieldDefn->GetPrecision(),poFieldDefn->GetNameRef() );  
	  }

*/	
	
	
	
/*

// Output to a TEMP output file "temp.shp"
	printf("\n\t\tDetails of OUPUT TEMP shp file for CLASSR field\n\n");
	
	//poLayer->ResetReading();
	poFDefn = poLayer->GetLayerDefn();			// get layer definition (SCHEMA)
	
	for (iFeat=0; iFeat<feat_count; iFeat++)		// for each Feature (tree polygon) in the layer
      {

	  //OGRFeature * poFeature; 			  
      //poFeature = poLayer->GetNextFeature();			// for loops rather than while()
	  poFeature = poLayer->GetFeature(iFeat);			// for a more direct loop	(without ResetReading())	

  if (iFeat == 0) for (iField=0; iField < poFDefn->GetFieldCount(); iField++)
      {
	  poFieldDefn = poFDefn->GetFieldDefn(iField);
	  printf(" %d, Type %d, Field Width %d, Precision %d Name: %s\n",
		iField, poFieldDefn->GetType(), poFieldDefn->GetWidth(), poFieldDefn->GetPrecision(),poFieldDefn->GetNameRef() );  
	  }
 
	  if(iFeat == 3)  poFeature->SetField(cField, 53);			// #### FORCE a 55 
	  if(iFeat == 4)  poFeature->SetField(cField, 54);			// #### FORCE a 55 
	  if(iFeat == 7)  poFeature->SetField(4, 554);			// #### FORCE a 55 

	  
	  poGeometry = poFeature->GetGeometryRef();

	  poLayer->SetFeature(poFeature);		// to save feature to disk
	  
	  poLayer->SyncToDisk();	
		
	  
      printf("Feature %d Field %d Assigned class %d \n", iFeat, cField,  poFeature->GetFieldAsInteger(cField) );
	  
	  OGRFeature::DestroyFeature( poFeature );	  
	  } 

	  printf("\n");	  
*/  
  
	}  			// End of IF(vmode)
 
 
 
 
// Write DESCRIPTIONS and/or history to output layer

#ifdef PARTIAL_GDAL
if(vmode && PCI_File) 
	{
	GDBSync(idb_fp); 	// Clear the cache and move all info to vector layer in pix file
	fprintf(Report,"\nCrown-based classification results will be found in field %s (%d) of PCI layer %d\n",
	  							field_name, cField, isolbit);
	sprintf(segm_histo, "ITCSC(%s) in layer %d(%d),CH=%s,EX=%d,T=%4.2f", 
				VERSION, isolbit, cField, itostr(dbic,ibufs),lt_mask, threshold);
	GDBWriteHistory(idb_fp, SEG_VEC, isolbit, segm_histo);
	fprintf(Report,"\n");
	}
#endif
	
if(vmode && TIF_File) 
	{
	printf("\nCrown-based classification results will be found in field %s (%d) of file \"%s\"  \n", field_name, cField, file_SHP);

	// Write description to layer (does not work)
	
	sprintf(description, "ITCSC(%s) in field %d of layer %d,CH=%s,EX=%d,T=%4.2f", 
				VERSION, cField, isolbit, itostr(dbic,ibufs),lt_mask, threshold);			
	psLayer->SetDescription(description);
	seg_in->SetDescription(description);
	printf("\nShape file description : %s \n", description);
	
		
	}		
	
/* Check if we output a file of ITC signatures */

	//if (DBOUTPUT) db_output(idb_fp, dbout);
	 

/* Close files and return properly */

End:
	fprintf(Report,"\n");
	//IMPTime(timedate,4);

	time (&rawtime); timeinfo = localtime (&rawtime);	
	printf("\n*** ITC supervised classification (%s) completed at %s \n\n", VERSION, asctime(timeinfo));
	if (Report != stdout)
	  fprintf(Report,"\n*** ITC supervised classification (%s) completed at %s \n\n", VERSION, asctime(timeinfo));

	//printf("\n\n\t Closing files \n\n");	
	//GDALClose( seg_in );		// Close that data set (shp file)
	//psLayer->SyncToDisk();		// does not do anything ???
  	if (vmode) GDALClose(file_SHP);		// Close that data set (shp file)
	//GDALClose(poDS);			// redundant output shp file for testing
	
// For people using this program via ArcGIS, give then some time to examine the results (before disappearing)

if  (EQUALN("ArcGIS ",argv[argc-1],3) )
	{
	fprintf(stdout,"\n\n######\n");
	printf("\n Type anything to make this detailed window disappear and terminate %s ",PROG_NAME);
	answer[0] = getc(stdin); 		// gets any answer or <CR>
    }
	
	
exit(0);				// exit properly 

}				// END OF MAIN PROGRAM

/******************************************************************/

/* FUNCTIONS */

/******************************************************************/

/* This function scans the ITC bitmap (or section) to find trees.  A 2x2 window moves across, starting
   in the upper left and working across, then down to the lower right.  If all bits in the
   2x2 window are 'filled' then a tree has been found and fill() is called to gather the
   information needed to generate the signature type.  This information is stored in
   a tree node for that tree. */

void scan_for_tree(int xsize, int ysize, unsigned char *itc_bitmap)
  {
  int x, y;
  int64 bitnum, bytenum; 
  int option;
  int count, tcount, ii, kk;
  tree_node  *new_tree;
  int notusedITC = 0;		/* like unusedITC but for one image section only */

  printf("\n Got into scan_for_tree %d %d\n",xsize, ysize);
  
  tcount = 0;

  //for (y = 2; y < 10; y++)				// ysize=10 lines for DEBUGGING
  for (y = 2; y < ysize-1; y++)		// ysize here could be SECTION ysize  
  for (x = 2; x < xsize-1; x++) 
	{
	/* NOTE: bitnum (bytenum) starts at zero, but x and y image coordinate start at 1,1  */

 	bitnum = (line_offset+y-1)* (int64)Pixels + x-1;	/* bit (or byte) offset in full bitmap (image) */
	bytenum = (y-1) * (int64)Pixels + x-1;			/* bit (or byte) offset in present image section */

        /* check if 2x2 window is 'filled' - potential ITC */
        
	if ( testbit(itc_bitmap,bitnum) && testbit(itc_bitmap,bitnum+1) &&
		testbit(itc_bitmap,bitnum+xsize) && testbit(itc_bitmap,bitnum+xsize+1) )
	  {
	  //printf("\n Got a tree started \n");
	
	  new_tree = new_tree_node();		/* a node for each tree signature */

	  Proc_pix_list = NULL;			/* a list(of raw data) is created, then disgarded */
	  FITC_pix_list = NULL;			/* a list(of raw data) is created, then disgarded */

	  option = stype;		/* signature type */

	  fill(x, y, xsize, itc_bitmap, new_tree, option);	/* fill ITC and accumulate MSS data */

	  //printf("\n Back from fill Count = %d  %d\n", new_tree->count, new_tree->process_count);

	  if( new_tree->count >= SIZE_LIMIT)		// we dont want to blow the program stack
	    {
	    fprintf(stdout,"\n\n\t ### Encountered an ITC that is way too big! ###\n\n"); 
	    fprintf(stdout,"Starting at position %d %d (P,L) in the image \n", x, y);
	    fprintf(stdout,"ITC that big should be dealt with manually (i.e., via nfmask)\n  \n");
	    fprintf(stdout,"Could be a cut area or a long brite line of trees along a road\n\n");
	    /* exit(1); */
	    }

	  if (next_sect_flag)
	    {
	    sect_end = next_sect_start + y-1;
	    fprintf(Report, "\n** Need to go to next image section. Present line is %d\n", sect_end);
	    printf("For this section: No.trees classified %d  No. not used %d \n", tcount, notusedITC);
	    return;
	    }

	  /* Check if ITC is sizeable enough */

	  if( (new_tree->process_count < ibufs+1) || (new_tree->count >= SIZE_LIMIT) ||
		  ( (eigen_flag || textur_flag || struct_flag) && (new_tree->process_count < 2*ibufs) ) ) 
	    {	
	    process_tree_cleanup(new_tree);	/* delete pixel-based MSS data accumulated for crown */	
	    free(new_tree);	/* Not sizeable enough - remove that tree*/
	    unusedITC++;
	    notusedITC++;
	    }
	  else 		/* sizeable enough - We KEEP the tree */
	    {		
	    if (DBOUTPUT) add_to_tree_list(previous_tree, new_tree);
	    new_tree->xpos = x;		/* initial x and y position */
	    new_tree->ypos = y;		/* needed for painting class bitmaps at the end (y within section) */
	    new_tree->cg_xpos /= new_tree->count; /* center of grav of tree crown */
	    new_tree->cg_ypos /= new_tree->count;  
	    process_tree_avg(new_tree);
	    process_tree_var(new_tree);
	    if (eigen_flag) process_tree_eigen(new_tree); 
	    if (struct_flag) itc_structure(new_tree);

		//printf("\n ITC processed Ave.= %.2f\n",new_tree->itc_mean[1]);
	  
	  
	    classify(new_tree); 		/* CLASSIFY the ITC based on sepectral value from fill() */

		//printf("\n ITC CLASS = %d \n",new_tree->classI);	

// In BITMAP mode : Fill tree in proper bitmap in memory 

		if( (! vmode)	&&(new_tree->classI >= 0) )  
				fill(new_tree->xpos, new_tree->ypos,Pixels, tempbitbuf2, new_tree, BIT_FILL);  // write to bitmap in memory	
		
		
	    process_tree_cleanup(new_tree);		/* delete pixel-based MSS data accumulated for crown */

	    if (DBOUTPUT) previous_tree = new_tree;	/* only for DBOUTPUT do we keep a list of trees */
	    if (! DBOUTPUT) free(new_tree);		/* if not needed for plain text ouput later, free memory */

	    tcount++;
	    //if (tcount == (tcount/10000)*10000) printf("%d ITC classified \r", tcount);

	    }	/* Endof else (ITC is sizeable enough) */

 
	  }		/* Endof if 2x2 loop (Possible ITC found) */ 


	  
	if( (x==2)  && (y/100)*100 == y) printf("%d lines done\r", next_sect_start+y-1);

	/* if((x==2) && (y==ysize-2) &&(next_sect_start+y-1)>6160) )	printf("%d lines done\n", next_sect_start+y-1); */

	/*if( (x==2) && (next_sect_start+y > 42230) ) printf("%d lines done\n", next_sect_start+y);	*/

  }		/* Endof for image (or image section)  loop */

Out:	

  printf("\n\tSections (%d) finished : %d lines done\n", sect_count, next_sect_start+y-1);

  /* tree_list->count = tcount; */

  //IMPTime(timedate,4);
	time (&rawtime); timeinfo = localtime (&rawtime);
  printf("At this point: No.trees clasified %d  No. not used %d - %s \n", tcount, notusedITC, asctime(timeinfo));
  
 
  }			// end of function scan_for_tree()


/******************************************************************/

/* This function loops through the shapes of the input layer (manual tree crown as polygons),
   paints a single tree crown into itc_bitmap and then fills it to gather the MSS
   information needed to generate the ITC signature of a given type.  
   This information is stored in a tree node for each tree. 
   NOTE: It is capable of working by image sections */

   
   #ifdef PARTIAL_GDAL
   
void loop_for_tree(int xsize, int ysize, unsigned char *itc_bitmap, GDBLayer hlayer)
  {
  int64 bitnum;
  float xmin, xmax, ymin, ymax;
  int option;
  int count, tcount, ii, kk;
  int vec2ras_count=0, fill_count=0, shape_count=0;
  tree_node  *new_tree;
  GDBShapeId	hShapeId;
  GDBVertex	*pasVertices;
  int	nVertex;
  int bypassedITC= 0;
  int notusedITC = 0;		/* like unusedITC bnut for one image section only */

  tcount = 0;			/* count of classified trees */

  fprintf(stdout,"\n *** PCI Loop for trees *** \n\n");
  
  printf("\n\nPCI TopLeft(x,y)  %.2f %.2f Transform(x,y) %.2f %.2f \n\n ", topleftX, topleftY,transformX,transformY);

	
  for ( hShapeId = GDBGetNext(hlayer, GDBNullShapeId);
		hShapeId != GDBNullShapeId; hShapeId = GDBGetNext(hlayer, hShapeId) ) 
	{

 	fprintf(stdout,"\t Doing Shape %d \r", hShapeId); 
	shape_count++;

	/* Dont classify if crown already done (i.e., was done in previous image section) */

	if (poly_done[hShapeId]) 
		{
		bypassedITC++ ;
	   	fprintf(stdout,"Shape %d was previously done \n", hShapeId);
		continue; 
		}	

	/* Convert geographic coordinates to image coordinates (bitmaps use full image coordinates) */

	pasVertices = GDBGetVertices(hlayer, hShapeId, &nVertex);

	for (kk = 0; kk < nVertex; kk++)
	  {
	  pasVertices[kk].x = (pasVertices[kk].x - topleftX) / (transformX);
	  pasVertices[kk].y = (pasVertices[kk].y - topleftY) / (transformY);
	  /* if (hShapeId==9900 ) printf("X and Y vertices: %.2f %.2f  \n",pasVertices[kk].x,pasVertices[kk].y); */
	  }


	/* gather x and y minimums and maximums positions */

	xmin = xmax = pasVertices[0].x;
	ymin = ymax = pasVertices[0].y;

	for (kk=1; kk< nVertex; kk++)
	  {
	  if (pasVertices[kk].x > xmax) xmax = pasVertices[kk].x;
	  if (pasVertices[kk].x < xmin) xmin = pasVertices[kk].x;
 	  if (pasVertices[kk].y > ymax) ymax = pasVertices[kk].y;
	  if (pasVertices[kk].y < ymin) ymin = pasVertices[kk].y;
	  }

	ixmin = (int) (xmin); iymin = (int) (ymin);		/* round-off on the generous side (outside area)*/
	ixmax = (int) (xmax+1); iymax = (int) (ymax+1);

	 /* Dont bother to burn and analyse the crown if not fully in the present image section */

 
	 if(  (iymin <= dbiw[1]) || (iymax >= dbiw[1]+dbiw[3]) )
	   { bypassedITC++ ; 
	   /*fprintf(stdout,"Shape %d outside present image section, iymin and iymax are %d, %d\n", hShapeId, iymin,iymax);*/
	   continue; 
	   }

 //**********************************************************************
 
    /* Paint vector tree crown to a bitmap (the temp itc_bitmap) */

 	 vect2rast(nVertex, pasVertices, itc_bitmap, xsize, hShapeId, hlayer);	
	 vec2ras_count++;
 
	 bitnum = (ycg-1) * (int64)Pixels + xcg-1;
 
	 printf("From Vect2rast() for shape %d, cg is %d %d (bitnum = %I64d), iymax= %d, rast_pcount=%d \n", 
				hShapeId, xcg, ycg, bitnum, iymax, rast_pcount); 


	/* Check that center of gravity is "roughly" in the painted crown */
	/* If ill-formed (cg not in crown) dont use (for now) */
	

	if ( testbit(itc_bitmap,bitnum) || testbit(itc_bitmap,bitnum+1) 
		|| testbit(itc_bitmap,bitnum-1) || testbit(itc_bitmap,bitnum+xsize) )		
	  {
	  new_tree = new_tree_node();		// to store summary info about one specific tree

	  Proc_pix_list = NULL;			/* a list(of raw data) is created, then disgarded */
	  FITC_pix_list = NULL;			/* a list(of raw data) is created, then disgarded */

	  
	  /* Gather multispectral data under crown bitmap (destroying the crown bitmap)*/
	  /* xcg and ycg from vect2rast() are full image position, y need adjustement for image section position */

	  option = stype;		/* signature type */

	  if (hShapeId == 0 ) printf("Shape %d BEFORE fill() cgx,y=(%d,%d),bitnum=%I64d iymax=%d\n", 
												hShapeId, xcg, ycg-line_offset, bitnum, iymax);

	  fill(xcg, (ycg-line_offset), xsize, itc_bitmap, new_tree, option);	  /* wants positions (x, y) for present image section */
	  fill_count++;

	  if (hShapeId == 0 ) printf("Shape %d AFTER fill() count=%d, process_count=%d \n", hShapeId,new_tree->count,new_tree->process_count); 
	  
	  /* Check if ITC is sizeable enough for statistics */

	  if( (new_tree->process_count < ibufs+1) ||
		  ( (eigen_flag || textur_flag || struct_flag) && (new_tree->process_count < 2*ibufs) ) )
	    {	
	    fprintf(stdout,"Shape %d at %d,%d not classified cause: Count= %d Process_Count = %d \n", 
						hShapeId, xcg, ycg, new_tree->count, new_tree->process_count);
	    free(new_tree);
	    unusedITC++;
	    notusedITC++;
	    poly_done[hShapeId] = 1;   		/* mark this shape as done  */
	    }
	  else 			/* KEEP that tree and CLASSIFY it*/
	    {		
	    if (DBOUTPUT) add_to_tree_list(previous_tree, new_tree);
	    new_tree->xpos = xcg;		/* initial x and y position */
	    new_tree->ypos = ycg;		/* needed for painting class bitmaps at the end */
	    new_tree->cg_xpos /= new_tree->count; /* center of grav of tree crown */
	    new_tree->cg_ypos /= new_tree->count;  
	    process_tree_avg(new_tree);
	    process_tree_var(new_tree);
	    if (eigen_flag) process_tree_eigen(new_tree); 
	    if (struct_flag) itc_structure(new_tree);
	    new_tree->shapeId = hShapeId;

//******************************************************************************
		
	    classify(new_tree);				/* classify  PCI MANUAL tree */

	    tcount++;
	    poly_done[hShapeId] = 1;   		/* mark this shape as done  */

			
// In PCI VECTOR mode

		if(vmode && PCI_File)			/* in VECTOR mode, store class in field CLASSR in layer */
		  {

		  sField.nInteger = new_tree->classI +1;
		  //sField.nInteger = new_tree->classI +21;		// for testing
		  
		  if(speciescode) 		/* USED ALL THE TIME NOW - just filled with a plain sequence if not really used */
		    {
		    if(new_tree->classI == -1) sField.nInteger = UNCLASS;
		    if(new_tree->classI >= 0) sField.nInteger = clascode[new_tree->classI];
		    if (class2_flag) sField.nInteger = clascode[new_tree->class_2nd];			// store 2nd class instead
		    }

		  hShapeId = new_tree->shapeId;

      	  GDBSetFieldValue(hlayer, hShapeId, cField, &sField);

//		  printf("Shape %d classified as class %d  \n\n", hShapeId, new_tree->classI +1);
		  printf("Shape %d classified as class %d  \n\n", hShapeId, GDBGetFieldValue(hlayer, hShapeId, cField)->nInteger );
		  
	      //printf("\nShape %d classified as %d and 2ndclass = %d \n", hShapeId, clascode[p->classI], clascode[p->class_2nd] );
      	  }

	    /* 
	    fprintf(stdout,"Shape %d  Class = %d \n", hShapeId, new_tree->classI +1 );
	    fprintf(stdout,"Count=%d  Lit_count=%d Mean=%.2f %.2f %.2f %.2f \n", new_tree->count,new_tree->process_count, 
	        new_tree->itc_mean[0], new_tree->itc_mean[1], new_tree->itc_mean[2], new_tree->itc_mean[3]); 
	    */


	    process_tree_cleanup(new_tree);	/* delete pixel-based MSS data accumulated for that crown */
	    previous_tree = new_tree;
	    if (! DBOUTPUT) free(new_tree);	/* if not needed for plain text ouput later, free memory */

	    }	/* end of enough pixels for stats */


	  }	  /* end of test cg is in the tree */
	
	else 	/* Centre of grav. not in crown */
	  { 
	  unusedITC++;	notusedITC++; 
	  poly_done[hShapeId] = 1;   		/* mark this shape as done  */
	  fprintf(stdout,"Shape %d not used - Centre of grav. not in crown ? \n", hShapeId);
	  fprintf(stdout,"Shape %d at %d,%d not used. iymin=%d and iymax=%d \n", hShapeId, xcg, ycg, iymin, iymax);

	  }
 

	}	/* end of for all shapes in layer  */


	
	/* tree_list->count = tcount; */

	fprintf(stdout," \n\n");

	printf("### For this section - No.trees clasified %d No. bypassed %d No. not used %d \n", tcount,bypassedITC,notusedITC);

	printf("### For this section - shape_count=%d, vec2ras_count=%d, fill_count=%d \n",shape_count,vec2ras_count,fill_count);

	  
	/* free vector storage memory */

	/* HFree(pasVertices); WoW64 does not like HFree() */

  
  }				// End of function loop_for_tree() 
  
#endif

  /******************************************************************/

/* This function loops through the shapes of the input layer (manual tree crown as polygons),
   paints a single tree crown into itc_bitmap and then fills it to gather the MSS
   information needed to generate the ITC signature of a given type.  
   This information is stored in a tree node for each tree. 
   NOTE: It is capable of working by image sections */

//void loop_for_tree_g(int xsize, int ysize, unsigned char *itc_bitmap, OGRLayer * psLayer)

void loop_for_tree_g(int xsize, int ysize, unsigned char *itc_bitmap)
  {
  int64 bitnum;
  float xmin, xmax, ymin, ymax;
  int option;
  int count, tcount, ii, kk;
  int vec2ras_count=0, fill_count=0, shape_count=0;
  tree_node  *new_tree;
  //GDBShapeId	hShapeId;
  GDBVertex2D	*pasVertices;
  int	nVertex;
  int bypassedITC= 0;
  int notusedITC = 0;		/* like unusedITC bnut for one image section only */

  tcount = 0;			/* count of classified trees */
  
  
   // Getting polygon vertices

  std::vector<PolygonFeature> PolygonLayer;

  PolygonFeature Polygon;
  OGRPoint ptTemp;
  OGRPolygon *piPolygon;
  OGRLinearRing *piExteriorRing;
  int NumberOfExteriorRingVertices;
  int NumberOfInnerRings; 
  
// Reserve space for 250 vertices (for the moment) PCI Style

  pasVertices = (GDBVertex2D *) CPLMalloc(4000); // this 4000 bytes, over 250 vertices
  
  
  fprintf(stdout,"\n *** Loop for trees (GDAL)*** \n\n");
  
	//printf("LOOP: Address of input Layer  = %p \n",  psLayer);
	
  // Just double checking info. FOR NOW
  // Number of features (i.e., polygons) in that layer AND number of fields in that layer

	piFDefn = psLayer->GetLayerDefn();		// get layer definition (SCHEMA)
	feat_count = psLayer->GetFeatureCount();
	field_count = piFDefn->GetFieldCount();
	
	printf("\n**Said layer from current file has %d features (shapes) with %d fields each\n", 
				feat_count, field_count);
    printf("TopLeft(x,y)  %.2f %.2f Transform(x,y) %.2f %.2f \n\n ", topleftX, topleftY,transformX,transformY);

// Copy ???? GeoTransform, projection, ...
//	printf("\n\t*Writing geographic projection for TEMP output file \n");	
//	poDS->SetSpatialRef(piDS->GetSpatialRef());		
//	poLayer->SetSpatialRef( piLayer->GetSpatialRef );
//	poFeature->SetGeometry( piFeature->GetGeometryRef() );

  	psLayer->ResetReading();
  	//poLayer->ResetReading();	
	
  //poFDefn = poLayer->GetLayerDefn();			// get layer definition (SCHEMA)of output shp
	
	
  for (iFeat=0; iFeat<feat_count; iFeat++)		// for each Feature (tree polygon) in the layer
  //for (iFeat=0; iFeat<1; iFeat++)		 		// Do Only a few manual trees (feat,polygon) for testing
  //while( (piFeature = psLayer->GetNextFeature()) != NULL )
    {
	//OGRFeature *piFeature; 			// not needed all ready a global vars
	//OGRFeature *poFeature; 		
    //piFeature = psLayer->GetNextFeature();			// for loops rather than while()
    piFeature = psLayer->GetFeature(iFeat);			// for loop with iFeat  rather than while()		
	piGeometry = piFeature->GetGeometryRef();

    //poFeature = poLayer->GetFeature(iFeat);			// for loop with iFeat rather than while() for OUTPUT shape
		
	//printf("\nFor feature %d \t Geometry is : %d \n", iFeat, wkbFlatten(piGeometry->getGeometryType()));	

	//printf("For feature %d piFeature Address = %p\n", iFeat, piFeature);	
    //poFeature = psLayer->GetNextFeature();			// for output shp file 
    //poGeometry = piFeature->GetGeometryRef();
	
	
	shape_count++;

	/* Dont classify if crown already done (i.e., was done in previous image section) */

	if (poly_done[iFeat]) 
		{
		bypassedITC++ ;
	   	fprintf(stdout,"Shape %d was previously done \n", iFeat);
		continue; 
		}	

  
  // If it is a polygo, how many vertices
  
    if ( piGeometry != NULL && (wkbFlatten(piGeometry->getGeometryType()) == wkbPolygon) )		// if polygon
	{
	piPolygon = (OGRPolygon *) piGeometry;

	NumberOfInnerRings = piPolygon->getNumInteriorRings();	
	//printf("Number of inner rings = %d \n", NumberOfInnerRings);

	if(NumberOfInnerRings > 0)
	  {
	  printf("Number of inner rings = %d \n", NumberOfInnerRings);
	  printf("\t *** CAN NOT DEAL with InnerRings in tree crowns - EXIT \n");
	  exit(-1);
	  }
	   
	piExteriorRing = piPolygon->getExteriorRing();	// get exterior ring vertices	
	NumberOfExteriorRingVertices = piExteriorRing->getNumPoints();		// get number of exterior ring vertices	
	
	//printf("\nNumberOfExteriorRingVertices = %d \n\n", NumberOfExteriorRingVertices);

	
// Convert GEOGRAPHIC COORDINATES  to image coordinates

	ptTemp = OGRPoint();
	//printf("\n\nTopLeft(x,y)  %.2f %.2f Transform(x,y) %.2f %.2f \n\n ", topleftX, topleftY,transformX,transformY);
	
	for ( int k = 0; k < NumberOfExteriorRingVertices; k++ )
      {
      piExteriorRing->getPoint(k,&ptTemp);
  
	  //if (k < 2) printf( "%.2f  %.2f\t", ptTemp.getX(), ptTemp.getY() );	// Print some UTM vertices

// Convert geographic coordinates to image coordinates

	  pasVertices[k].x = ( ptTemp.getX()- topleftX ) / transformX;
      pasVertices[k].y = ( ptTemp.getY() - topleftY ) / transformY;
	  
      //if (k < 2) printf( "\tIma:  %.1f  %.1f\n", pasVertices[k].x, pasVertices[k].y); 	// PRINT some IMA cord. vertices

// Check if outside the image

	  if( (pasVertices[k].x < 0) || (pasVertices[k].y < 0) || (pasVertices[k].x > Pixels) || (pasVertices[k].y > Lines))
		{
		printf("\nShape %d in MANVEC layer appears outside the image and will be skipped\n",iFeat);
		break;
		}

     }		// end of loop for exterior vertices to convert to image coordinates
	
	//PolygonLayer.push_back(Polygon);

	}				// end of IF Polygon
 
	//printf("Number of vertices converted to image coordinates = %d \n", NumberOfExteriorRingVertices);

    //PolygonLayer.push_back(Polygon);
	
	nVertex = NumberOfExteriorRingVertices;

	/* gather x and y minimums and maximums positions */

	xmin = xmax = pasVertices[0].x;
	ymin = ymax = pasVertices[0].y;

	for (kk=1; kk< nVertex; kk++)
	  {
	  if (pasVertices[kk].x > xmax) xmax = pasVertices[kk].x;
	  if (pasVertices[kk].x < xmin) xmin = pasVertices[kk].x;
 	  if (pasVertices[kk].y > ymax) ymax = pasVertices[kk].y;
	  if (pasVertices[kk].y < ymin) ymin = pasVertices[kk].y;
	  }

	ixmin = (int) (xmin); iymin = (int) (ymin);		/* round-off on the generous side (outside area)*/
	ixmax = (int) (xmax+1); iymax = (int) (ymax+1);

	 /* Dont bother to burn and analyse the crown if not fully in the present image section */

 
	 if(  (iymin <= dbiw[1]) || (iymax >= dbiw[1]+dbiw[3]) )
	   { bypassedITC++ ; 
	   /*fprintf(stdout,"Shape %d outside present image section, iymin and iymax are %d, %d\n", iFeat, iymin,iymax);*/
	   continue; 
	   }


//******************************************************************
  
    /* PAINT  vector tree crown to a bitmap (the temp itc_bitmap) */
	
 	 vect2rast_g(nVertex, pasVertices, itc_bitmap, xsize, iFeat, isolbit);		 
	 
	 vec2ras_count++;
 
	 bitnum = (ycg-1) * (int64)Pixels + xcg-1;
 
	 //printf("From Vect2rast_g() for shape %d, cg is %d %d (bitnum = %I64d), iymax= %d \n", iFeat, xcg, ycg, bitnum, iymax); 


	/* Check that center of gravity is "roughly" in the painted crown */
	/* If ill-formed (cg not in crown) dont use (for now) */	

	if ( testbit(itc_bitmap,bitnum) || testbit(itc_bitmap,bitnum+1) 
		|| testbit(itc_bitmap,bitnum-1) || testbit(itc_bitmap,bitnum+xsize) )		
	  {
	  new_tree = new_tree_node();		// to store summary info about one specific tree

	  Proc_pix_list = NULL;			/* a list(of raw data) is created, then disgarded */
	  FITC_pix_list = NULL;			/* a list(of raw data) is created, then disgarded */

	  
	  /* Gather multispectral data under crown bitmap (destroying the crown bitmap)*/
	  /* xcg and ycg from vect2rast() are full image position, y need adjustement for image section position */

	  option = stype;		/* signature type */

	  if (iFeat == 0 ) printf("Shape %d BEFORE fill() cgx,y=(%d,%d),bitnum=%I64d iymax=%d\n", 
												iFeat, xcg, ycg-line_offset, bitnum, iymax);

	  fill(xcg, (ycg-line_offset), xsize, itc_bitmap, new_tree, option);	  /* wants positions (x, y) for present image section */
	  fill_count++;

	  if (iFeat == 0 ) printf("Shape %d AFTER fill() count=%d, process_count=%d \n", iFeat,new_tree->count,new_tree->process_count); 
	  
	  /* Check if ITC is sizeable enough for statistics */

	  if( (new_tree->process_count < ibufs+1) ||
		  ( (eigen_flag || textur_flag || struct_flag) && (new_tree->process_count < 2*ibufs) ) )
	    {	
	    fprintf(stdout,"Shape %d at %d,%d not classified cause: Count= %d Process_Count = %d \n", 
						iFeat, xcg, ycg, new_tree->count, new_tree->process_count);
	    free(new_tree);
	    unusedITC++;
	    notusedITC++;
	    poly_done[iFeat] = 1;   		/* mark this shape as done  */
	    }
	  else 			/* KEEP that tree and CLASSIFY it*/
	    {		
	    if (DBOUTPUT) add_to_tree_list(previous_tree, new_tree);
	    new_tree->xpos = xcg;		/* initial x and y position */
	    new_tree->ypos = ycg;		/* needed for painting class bitmaps at the end */
	    new_tree->cg_xpos /= new_tree->count; /* center of grav of tree crown */
	    new_tree->cg_ypos /= new_tree->count;  
	    process_tree_avg(new_tree);
	    process_tree_var(new_tree);
	    if (eigen_flag) process_tree_eigen(new_tree); 
	    if (struct_flag) itc_structure(new_tree);
	    new_tree->shapeId = iFeat;


//******************************************************************

		//printf("\n\t Before single tree classification \n");
			
	    classify(new_tree);									// CLASSIFY MANUAL TREE

		//printf("\n\t Back from single tree classification \n");

			
// In VECTOR mode
/*
		if(vmode && PCI_File)			//in VECTOR mode, store class in field CLASSR in layer 
		  {
		  //sField.nInteger = p->classI;
		  sField.nInteger = new_tree->classI +1;
		  
		  if(speciescode) 		// USED ALL THE TIME NOW - just filled with a plain sequence if not really used 
		    {
		    if(new_tree->classI == -1) sField.nInteger = UNCLASS;
		    if(new_tree->classI >= 0) sField.nInteger = clascode[new_tree->classI];
		    if (class2_flag) sField.nInteger = clascode[new_tree->class_2nd];			// store 2nd class instead
		    }

		  hShapeId = new_tree->shapeId;

      	  GDBSetFieldValue(hlayer, hShapeId, cField, &sField);

		  printf("\nShape %d classified as class %d  \n", hShapeId, new_tree->classI +1);
		  
	      //printf("\nShape %d classified as %d and 2ndclass = %d \n", hShapeId, clascode[p->classI], clascode[p->class_2nd] );
      	  }
*/

		
// Classification result was also stored in "tree" structure: use that 

		if(vmode && TIF_File)
		  piFeature->SetField(cField, new_tree->classI+1);		// update main shp file



	  
		//piFeature->SetField(cField, new_tree->classI+11);		//  (+11 for test) update main shp file (input and output)  (+11 for test)
		//poFeature->SetField(cField, new_tree->classI+11);		// update auxilliary output file (+11 for test)

		//printf( "OUTPUT2: Back in loop_for_tree_g() : cField(%d) = %d \n", cField, piFeature->GetFieldAsInteger(cField) );


		
		//TEST	
		//printf( "Back in loop_for_tree_g() : cField(%d) = %d \n", cField, poFeature->GetFieldAsInteger(cField) );		//TEST
		//printf( "\nAddresses  piFeature= %p  poFeature = %p \n",piFeature, poFeature);

		//printf("LOOP: END Address of input Layer  = %p \n",  psLayer);

	    tcount++;
	    poly_done[iFeat] = 1;   		/* mark this shape as done  */


	    /* 
	    fprintf(stdout,"Shape %d  Class = %d \n", iFeat, new_tree->classI +1 );
	    fprintf(stdout,"Count=%d  Lit_count=%d Mean=%.2f %.2f %.2f %.2f \n", new_tree->count,new_tree->process_count, 
	        new_tree->itc_mean[0], new_tree->itc_mean[1], new_tree->itc_mean[2], new_tree->itc_mean[3]); 
	    */


	    process_tree_cleanup(new_tree);	/* delete pixel-based MSS data accumulated for that crown */
	    previous_tree = new_tree;
	    if (! DBOUTPUT) free(new_tree);	/* if not needed for plain text ouput later, free memory */

	    }	/* end of enough pixels for stats */


	  }	  /* end of test cg is in the tree */
	
	else 	/* Centre of grav. NOT  in crown */
	  { 
	  unusedITC++;	notusedITC++; 
	  poly_done[iFeat] = 1;   		/* mark this shape as done  */
	  fprintf(stdout,"Shape %d not used - Centre of grav. not in crown ? \n", iFeat);
	  fprintf(stdout,"Shape %d at %d,%d not used. iymin=%d and iymax=%d \n", iFeat, xcg, ycg, iymin, iymax);

	  }
 
 
	psLayer->SetFeature(piFeature);		// VIP ----  refresh feature so will show in disk file
	//poLayer->SetFeature(poFeature);
	
    OGRFeature::DestroyFeature( piFeature );		// erase content to use for next one
    //OGRFeature::DestroyFeature( poFeature );
	
	}	/* end of loop for all features (shapes, polygons)in layer  */
	
	

	//psLayer->SyncToDisk();		// does not do anything ???
	//poLayer->SyncToDisk();	
	
	
	/* tree_list->count = tcount; */

	fprintf(stdout," \n\n");

	printf("### For this section (or full image)  - No.trees clasified %d No. bypassed %d No. not used %d \n", tcount,bypassedITC,notusedITC);

	printf("### For this section - shape_count=%d, vec2ras_count=%d, fill_count=%d \n",shape_count,vec2ras_count,fill_count);

	  
	/* free vector storage memory */

	/* HFree(pasVertices); WoW64 does not like HFree() */

  
  }			// End of function loop_for_tree_g() 
  
  

/******************************************************************/
/******************************************************************/
/* This function 'fills' a tree shape, gathering information for processing 
	(and removing it from the ITC bitmap during processing) 
	x and y are coordinates in the current image section */

void fill(int x, int y, int xsize, unsigned char *itc_bitmap, tree_node *tree_info, int option)
  {
  int byte, bit;
  int64 bitnum=0, bytenum=0;

  /* check section boundary issues (dont go off image section) */

  if (!vmode && next_sect_flag) return;			/* for all the other recursions of fill() after below */
  if ( !vmode && by_sections && (y+1 >= sect_siz) )	{next_sect_flag=1; return; }

/* bitnum (bytenum) starts at zero, but x and y image coordinate start at 1,1  */

   bitnum = (line_offset+y-1)* (int64)Pixels + x-1;	/* position in full bitmap (offset)*/
   bytenum = (y-1)* (int64)Pixels + x-1;			/* position in present image section (offset) */

   /* fprintf(stdout,"In fill() x=%d, y=%d, bitnum=%I64d, bytenum=%I64d \n",x,y+line_offset,bitnum, bytenum); */



 if (option != BIT_FILL) 
    {
    (tree_info->count)++;			// ITC (ISOL) pixel count
    tree_info->cg_xpos += x;			 /* accumulate for center of gravity estimation */
    tree_info->cg_ypos += y + line_offset;	/* if by section, y is section coordinate */
    }
    
    if( tree_info->count >= SIZE_LIMIT) return;		// we dont want to blow the stack


/* 
	IDEA : 
	When using LIT mask (cause good for multispectral classif), also produce a full ITC pixel list (not just LIT-related list)
	as that would be better to use for the STRUCTURE, TCL,and TCL2 signatures (which should work better on a full crown) 
	This way you could combine MSS LIT side signatures with a better structure signature)
*/
 
// Accumulate (in a list) FULL ITC crown-based MSS data one pixel at the time
//	Creates FITC_pix_list

	process_ITC_image_data(x, y, bytenum, tree_info);


// Accumulate (in a list) Process (e.g., with LIT) crown-based MSS data one pixel at the time
//	Creates Proc_pix_list

 if ( (option==BIT_FILL)  || (!ExtMaskFlag) || (ExtMaskFlag && (testbit(extmaskbuffer,bitnum))) )	

 	process_image_data(x, y, bitnum, bytenum, tree_info, option);



  /* turn off bit & fill recursively starting at current position */

  clearbit(itc_bitmap,bitnum);

  if (testbit(itc_bitmap,bitnum+1))
	fill(x + 1, y, xsize, itc_bitmap, tree_info, option);

  if (testbit(itc_bitmap,bitnum-xsize))
	fill(x, y - 1, xsize, itc_bitmap, tree_info, option);

  if (testbit(itc_bitmap,bitnum-1))
	fill(x - 1, y, xsize, itc_bitmap, tree_info, option);

  if (testbit(itc_bitmap,bitnum+xsize))
	fill(x, y + 1, xsize, itc_bitmap, tree_info, option);
}

/******************************************************************/


/* This function performs the appropriate processing operation, depending on the
   type of signature (given by stype). */

void process_image_data(int x, int y, int64 bitnum, int64 bytenum, tree_node *tree_info, int option)
{
	switch (option) {
		case BIT_FILL:
			if (by_sections) process_build_bitmap(bytenum, tree_info);
			if (! by_sections) process_build_bitmap(bitnum, tree_info);
			break;
		case COUNT_ONLY:
			(tree_info->process_count)++;
			break;
		case MEAN:
			process_tree_image_data(x, y, bytenum, tree_info);
			break;
		case TCL:
		case TCL2:
		case TEXTUR:
		case STRUCT:
			//process_tree_image_data(x, y, bytenum, tree_info);
			break;
		default:
			process_tree_image_data(x, y, bytenum, tree_info);
			break;
	}
}

/******************************************************************/

/* This function stores the values of the image data contained on each channel
   (for the given pixel) in a MSS information node (it builds up an information
   list for the current tree being processed).  This will be used later to
   calculate eigenvalues/eigenvectors and the covariance matrix as well as
   variance and other parameters. (so you dont re-read images all of the time) */

   // bytenum is (offset) position within present image section 

void process_tree_image_data(int x, int y, int64 bytenum, tree_node *tree_sign)
{
	int             i, p_val;
	info_node      *new_node;

	(tree_sign->process_count)++;

/* every data node corresponding to position and MSS data for one pixel */

	new_node = (info_node *) CPLMalloc(sizeof(info_node));
	new_node->next_info = Proc_pix_list;
	Proc_pix_list = new_node;

	new_node->xpos = x;			/* populate MSS data node */
	new_node->ypos = line_offset + y;
	for (i = 0; i < ibufs; i++)
	  {
	  p_val = (*get_Pval[i]) (imagebuffer[i], bytenum);
	  new_node->data[i] = p_val;
	  tree_sign->itc_mean[i] += p_val;		/* to be used by process_tree_avg() */
	  }
}



/***********************************************************************************/

/* This function stores the values of the image data contained on each channel
   (for THE GIVEN pixel) in a MSS "information node" (it builds up an information
   list (MSS data) for the current tree being processed).  This will be used later to
   calculate parts of its signature like eigenvalues/eigenvectors and the covariance matrix,
   as well as,  variance and other parameters. 
   X and Y are position within full area, 
   bytenum is (offset) position within present image section 
*/

void process_ITC_image_data(int x, int y, int64 bytenum, tree_node *tree_sign)
{
	int             i, p_val;
	info_node      *new_node;

	(tree_sign->ITC_count)++;

	new_node = (info_node *) CPLMalloc(sizeof(info_node));
	check_mem(new_node);
	new_node->next_info = FITC_pix_list;
	FITC_pix_list = new_node;

	new_node->xpos = x;				/* populate MSS data node */
	new_node->ypos = y;
      // fprintf(stdout,"Pixel at position : %d %d \n", x, y);

	for (i = 0; i < ibufs; i++)		// for "ibufs multispectral channels
	  {
	  p_val = (*get_Pval[i]) (imagebuffer[i], bytenum);
	  new_node->data[i] = p_val;
	  tree_sign->Fitc_mean[i] += p_val;		/* to be used by process_Fitc_avg() */

      // fprintf(stdout,"MSS value CH(%d) %d ",i+1,  p_val); 

	  }

       // fprintf(stdout,"\n"); 
}






/******************************************************************/

/* This function processes the multi-spectral data to produce the variance for the tree
	(i.e., a texture signature)  under LIT mask if used */

void process_tree_var(tree_node *tree)
{
	int             n, i;
	float          sumofdiff, mean;
	info_node      *p;

	n = tree->process_count;
	for (i = 0; i < ibufs; i++) 
	{
	  sumofdiff = 0.0;
	  mean = tree->itc_mean[i];
	  p = Proc_pix_list;
	  while (p) 
		{
		sumofdiff += (p->data[i] - mean) * (p->data[i] - mean);
		p = p->next_info;
		}
	  if (n > 1) tree->variance[i] = sumofdiff / ((float) (n - 1));
	}
}
/******************************************************************/

/* This function removes the multispectral pixel data accumulated for 
	a single tree crown, freeing up the memory space (i.e., previous versions 
	of ITCSC were not cleaning-up and thus, could not run on big images without
	running out of memory */

void process_tree_cleanup(tree_node *tree)
{
	info_node *p, *p1;

	p = Proc_pix_list;
	node_cleared = 0;

/* free all process MSS data nodes (one per pixel) accumulated for the tree */
	
	while (p) 
	  {
	  p1 = p->next_info;
	  free(p);
	  node_cleared++;
	  p = p1;
	  }

	p = FITC_pix_list;
	node_cleared = 0;

/* free all MSS data nodes (one per pixel) accumulated for the tree */
	
	while (p) 
	  {
	  p1 = p->next_info;
	  free(p);
	  node_cleared++;
	  p = p1;
	  }
}

/******************************************************************/

/* This function is called by the fill routine (type BIT_FILL) to write the bitmap 
   data onto the actual bitmap. */

void process_build_bitmap(int64 bitnum, tree_node *tree_info)

{
	int byte, bit;
	setbit(classbitbuffer[tree_info->classI],bitnum);
}

/******************************************************************/

/* 	This function processes the FULL ITC pixel list (MSS data)
	for a given tree (ITC). It generates a covariance matrix. 
	From the covariance matrix, the eigenvectors/eigenvalues are
   	generated. Tree colour line intercepts are also calculated.
	The new results are stored in the tree node for that specific tree. */

void process_tree_eigen(tree_node *tree_info)
{
	info_node      *p;
	int             i, j, n, count;
	float          sum1, sum2, sumsquare1, sumsquare2, product1;
	float          sumofsquares1, sumofsquares2, sumofproducts, correlcoef, covariance;

	for (i = 0; i < (ibufs * ibufs); i++)
		matrix1[i] = 0.0;

	n = tree_info->ITC_count;

	count = 0;
	for (i = 0; i < ibufs; i++)
	for (j = 0; j <= i; j++) 	/* for lower triangular form */
		{
		sum1 = sum2 = sumsquare1 = sumsquare2 = product1 = 0.0;

		p = FITC_pix_list;	// Full ITC list of pixel MS values

		/* Calculations */

		while (p) 
			{
			sum1 += p->data[i];
			sum2 += p->data[j];
			sumsquare1 += (p->data[i] * p->data[i]);
			sumsquare2 += (p->data[j] * p->data[j]);
			product1 += (p->data[i] * p->data[j]);

			p = p->next_info;

			sumofsquares1 = sumsquare1 - (sum1 * sum1) / n;
			sumofsquares2 = sumsquare2 - (sum2 * sum2) / n;
			sumofproducts = product1 - (sum1 * sum2) / n;
			correlcoef = sumofproducts / sqrt(sumofsquares1 * sumofsquares2); 
			covariance = sumofproducts / (n - 1);
			}

		/* Setup covariance matrix */

		tree_info->covariance[i * ibufs + j] = covariance; /* full matrix in tree_info */
		tree_info->covariance[j * ibufs + i] = covariance;
		matrix1[count++] = covariance;
		/* matrix1[count++] = correlcoef;		 */
		}

	/* Generate eigenvalues/eigenvectors/intercept for that ITC */

	eigens(matrix1, tree_info->eigvector, tree_info->eigvalues, ibufs);
	eigvsort(tree_info->eigvector, tree_info->eigvalues, ibufs);
	itc_intercept(tree_info->eigvector,tree_info->itc_mean,tree_info->intercept,ibufs);

}

/******************************************************************/


/* This is a simple sort routine that puts the eigenvalues and eigenvectors into
   decreasing order, starting with the principal eigenvector (corresponding to the
   maximum absolute eigenvalue */

void eigvsort(float *ematrix, float *evector, int num)
{
	int             i, j, k, max;
	float          tempval;

	for (i = 0; i < num; i++) {
		max = i;
		for (j = i; j < num; j++) {	/* Find max */
			if (fabs(evector[j]) > fabs(evector[max]))
				max = j;
		}
		if (max != i) {	/* Swap */
			tempval = evector[i];
			evector[i] = evector[max];
			evector[max] = tempval;

			for (k = 0; k < num; k++) {
				tempval = ematrix[i * num + k];
				ematrix[i * num + k] = ematrix[max * num + k];
				ematrix[max * num + k] = tempval;
			}
		}
	}
}

/*********************************************************************/


/* This function processes the current information list(x, y, MSS data) for a tree, 
   generating structural signature parameters like 3D center of gravity (G_x & G_y),
   3D spread E_x & E_y), 3D skewness (D_x & D_y), and
   3D kurtosis (A_x & A_y) for that single tree.
*/

void itc_structure(tree_node *tree_sign)
{
	int i, j;
	int64 sum_int, sum_ix, sum_iy, G_x, G_y;
	float sum_ixg2, sum_iyg2, sum_ixg3, sum_iyg3, sum_ixg4, sum_iyg4;
	float E_x, E_y, D_x, D_y, A_x, A_y;	

	info_node  *p;		/* current information list(MSS data) for a tree */

/* for the various channels */

	/* for (i = 0; i < ibufs; i++) */
		
	for (i = 0; i < 1; i++)
	{

	sum_int=sum_ix=sum_iy=sum_ixg2=sum_iyg2=sum_ixg3=sum_iyg3=sum_ixg4=sum_iyg4=0;

/* Firstly, get info for 3D center of gravity */

	p = FITC_pix_list;	// use full ITC info for structure signatures (i.e., not LIT side, if used for MSS)
	// p = Proc_pix_list;		// TEST ####

	while (p) 
	  {
	  sum_int += p->data[i];	
	  sum_ix  += p->xpos * p->data[i];
	  sum_iy  += p->ypos * p->data[i];
	  p = p->next_info;
	  }

/* Calculate the three dimensional (3-D) center of gravity */

	G_x =  sum_ix / sum_int;
	G_y =  sum_iy / sum_int;

/* Secondly, calculate higher level moments */

	p = FITC_pix_list;	// use full ITC info for structure signatures (i.e., not LIT side, if used for MSS)
	// p = Proc_pix_list;		// TEST ####
	
	while (p) 
		{
		sum_ixg2 += pow((p->xpos - G_x),2) * p->data[i];
		sum_iyg2 += pow((p->ypos - G_y),2) * p->data[i];
		sum_ixg3 += pow((p->xpos - G_x),3) * p->data[i];
		sum_iyg3 += pow((p->ypos - G_y),3) * p->data[i];
		sum_ixg4 += pow((p->xpos - G_x),4) * p->data[i];
		sum_iyg4 += pow((p->ypos - G_y),4) * p->data[i];
		p = p->next_info;
		}

/* 3D spread of distribution - just a well formed factor - excentricity */

	E_x = (float) sum_ixg2 / (float) sum_int;
	E_y = (float) sum_iyg2 / (float) sum_int;

/* 3D shewness - probably mostly influenced by off-nadir view angle */

	D_x =  ( (float) sum_ixg3 / (float) sum_int ) / (pow(E_x,1.5)) ;
	D_y = ( (float) sum_iyg3 / (float) sum_int ) / (pow(E_y,1.5)) ;

/* 3D Kurtosis - may be worth keeping according to studies */

	A_x = ( (float) sum_ixg4 / (float) sum_int ) / (pow(E_x,2)) ;
	A_y = ( (float) sum_iyg4 / (float) sum_int ) / (pow(E_y,2)) ;

/* for debugging */

/*	printf("2Dcg      %d %d  ....    3Dcg       %I64d %I64d \n", 
			tree_sign->xpos, tree_sign->ypos, G_x, G_y);
	printf("Other structure parameters, 3D spread, 3D skewness, 3D kurtosis:\n %f %f %f %f %f %f\n",
			E_x, E_y, D_x, D_y, A_x, A_y);
*/

/* Put a single criteria into tree (ITC) STRUCTURE signature */


// A form factor (but also a "surrogate ???" for crown area)

	if ( (struct_flag == 11) && (E_x > 0.0) && (E_y > 0.0) )
		{
		tree_sign->structure_param[i] = E_x;	
		tree_sign->structure_param[i+ibufs] = E_y;
		}


/* 3D shewness - probably mostly influenced by off-nadir view angle */


	if ( (struct_flag == 12) && (D_x > 0.0) && (D_y > 0.0) )
		{
		tree_sign->structure_param[i] = D_x;	
		tree_sign->structure_param[i+ibufs] = D_y;
		}


// Kurtosis - prefered for conifer/deciduous separation 

	if ( (struct_flag == 13) && (A_x > 0.0) && (A_y > 0.0) )
		{
		tree_sign->structure_param[i] = A_x;	
		tree_sign->structure_param[i+ibufs] = A_y;
		}

// Kurtosis prefered if nothing more specific specified 
//	Note: When ill-defined, is left at zero and filtered out later

	if ( (struct_flag == 1) && (A_x > 0.0) && (A_y > 0.0) )
		{
		tree_sign->structure_param[i] = A_x;	
		tree_sign->structure_param[i+ibufs] = A_y;
		}


	}		// end of do all channels


}			// end of function


/**********************************************************************************/

/* This function calculates an intercept vector for an ITC from its
   first eigenvector and its mean */

void itc_intercept(float *ematrix, float *meanvect, float *intercept, int numd)
{
	int	i, j;
	float	interc[8], intercsum, slope[8];

	for (i = 0; i < numd ; i++) {
	  intercept[i] = 0.0;
	  intercsum = 0.0;
/* various rendition of same intercept */
	  for (j = 0; j < numd ; j++) {
	    slope[j] = ematrix[j]/ematrix[i];
	    interc[j] = meanvect[j] - slope[j] * meanvect[i];
	    intercsum += interc[j];
	  }
/* average intercept (best) */
	  intercept[i] = intercsum / (numd-1);
	}

/* for software testing (& past compatibility) intercept = mean
	for (i = 0; i < numd ; i++) intercept[i] = meanvect[i]; */


/* for test purposes - species line as ratios slope/intercept */


	for (i = 0; i < numd ; i++) 
	  {
	  intercept[i] = 0.0;
	  slope[i]= 0.0;
	  }

/* slope and intercept relative to previous channel only */

 
	  for (i = 0; i < numd ; i++) 
	    {
	    j = i + 1;
	    if (j == numd) j=0;
	    slope[j] = ematrix[j]/ematrix[i];
	    interc[j] = meanvect[j] - slope[j] * meanvect[i];
	    intercept[j] = interc[j] / slope[j];
	    }
 
}

/******************************************************************/

/* This function calculates the average of the itc_mean information */

void process_tree_avg(tree_node *tree)
{
	int   i;

	for (i = 0; i < ibufs; i++)
		tree->itc_mean[i] /= (float) tree->process_count;
}



/******************************************************************/


/* This function allocates memory for a new tree node and initializes the node */

tree_node   *new_tree_node(void)
{
	int             i, j;
	tree_node      *tree;

	tree = (tree_node *) CPLMalloc(sizeof(tree_node));

	/* Initialize fields to zero */
	tree->xpos = 0;	
	tree->ypos = 0;
	tree->count = 0;
	tree->process_count = 0;
	tree->ITC_count = 0;
	for (i = 0; i < CHANNELS; i++) 
		{
		tree->itc_mean[i] = 0.0;
		tree->Fitc_mean[i] = 0.0;
		tree->variance[i] = 0.0;
		tree->intercept[i] = 0.0;
		tree->eigvalues[i] = 0.0;

		for (j = 0; j < CHANNELS; j++) 
		  {
		  tree->eigvector[i * CHANNELS + j] = 0.0;
		  tree->covariance[i * CHANNELS + j] = 0.0;
		  }
		}
	for (i = 0; i < CHANNELS*2; i++) tree->structure_param[i] = 0.0;
	tree->cg_xpos = 0;	
	tree->cg_ypos = 0;
	tree->next_tree = NULL;
	tree->classI = -1;
	tree->shapeId = -1;
	tree->class_2nd = -1;
	return (tree);
}


/******************************************************************/

/* This function allocates memory for a new result(species signature) node
	 and initializes the node */

result_node  * new_result_node(void)
{
	int             i, j;
	result_node    *results;

	results = (result_node *) CPLMalloc((unsigned) sizeof(result_node));
	check_mem(results);
	
	/* Initialize fields to zero */

	for (i = 0; i < CHANNELS; i++) 
	{
		results->dbic[i] = 0;
		results->mean[i] = 0.0;
		results->variance[i] = 0.0;
		results->eigvalues[i] = 0.0;
		results->intercept[i] = 0.0;
		for (j = 0; j < CHANNELS; j++) results->eigvector[i * CHANNELS + j] = 0.0;
		for (j = 0; j < CHANNELS*3; j++) 
		{
		  results->correlation[i * CHANNELS*3 + j] = 0.0;
		  results->covariance[i * CHANNELS*3 + j] = 0.0;
		}
	}

	for (i = 0; i < CHANNELS*2; i++) results->structure_param[i] = 0.0;

	return (results);
}

/******************************************************************/

/* This function adds the tree node tree to the tree list. 
   A dummy node is the first node in the list. 
   New trees are added to the bottom of the list.
   (Past versions added to the top of the list, making easy loops and
   tree numbers incompatible with program that are just scanning the image)
*/

void add_to_tree_list(tree_node *previous_tree, tree_node *tree)
{
	previous_tree->next_tree = tree;	
}


/******************************************************************/


/* View signature information contained in result node */

void view_results(result_node *results, int sign)
{
	int             i, tempibufs;

	if (results == NULL) {
		fprintf(Report, "WARNING: No signature was contained in SIGNSEG %d.", sign);
		fprintf(Report, "  Check training masks.\n");
		fprintf(Report, "         Signature will not be be used\n");
	} else {
		/* Display header information */
		fprintf(Report, "\nSignature Information for Training Bitmap %d ", results->mask);
		fprintf(Report, "  [Trees Processed:  %d]\n", results->number);
		fprintf(Report,"Using ITC bitmap %d, extra mask %d, signature type=%s \n\n",
					results->ITC_mask, extmask, sigtypeO);

		//sign_spa_size = results->channels;	
		
		//printf("\n Space_size %d \n",  spa_size);
		//printf("\n Sign_Space_size %d \n",  sign_spa_size);

		
		//Show means, eigenvec, eigenvalues 
		for (i = 0; i < results->channels; i++) {
			fprintf(Report, "Mean%d: %6.2f", i + 1, results->mean[i]);
			fprintf(Report, "  St.Dev%d: %6.2f", i + 1, sqrt(results->covariance[i+i*spa_size]));	
			//fprintf(Report, "  Vars%d: %6.2f", i + 1, (results->covariance[i+i*spa_size]));			
			fprintf(Report, "  PVar%d: %6.2f", i + 1, results->variance[i]);
			fprintf(Report, "  EigVec1: %6.2f", results->eigvector[i]);
			fprintf(Report, "  EigVal%d: %6.2f", i + 1, results->eigvalues[i]);
			fprintf(Report, "\n");
		}

		/* Show covariance matrix */
		
		fprintf(Report, "\nCovariance Matrix:\n\n");
		
		//tempibufs = results->channels;
		//if (tempibufs > 3)  tempibufs = 3;
		//for (i = 0; i < (tempibufs * tempibufs); i++)
		for (i = 0; i < (spa_size * spa_size); i++)			
		  {
			fprintf(Report, " %8.2f ", results->covariance[i]);
			//if ((i % tempibufs) == (tempibufs - 1))  fprintf(Report, "\n");
			if ((i % spa_size) == (spa_size - 1))  fprintf(Report, "\n");
  		  }	
		  
		/* Show correlation matrix */

		fprintf(Report, "\nCorrelation matrix of selected features:\n\n");
		
		for (i = 0; i < (spa_size * spa_size); i++) 
		  {
		  fprintf(Report, " %9.4f ", results->correlation[i]);
		  if ((i % spa_size) == (spa_size - 1))  fprintf(Report, "\n");
		  }

		  

	}
	
	fprintf(Report, "\n");
	if (Report == stdout) 
	  { fprintf(Report, "Press [Enter] to Continue.\n"); 
		int c = getchar(); if(c == 'n') exit(-1);}
}			// end of view_results()


/******************************************************************/

/* This function writes the generated class bitmaps to PCI segments (in full image mode)
	and writes description and history info to all output segments*/

#ifdef PARTIAL_GDAL

void write_BM_histo(FILE * idb_fp, int *segs, int segcnt, 
						int *insigs, int *inchans, int xsize, int ysize, int isolbit)
{
	int 		i, j, segnum, size, blocks, segtype;
	int64 		bmsize;
	char            segflag, segname[9], history[120], description[81], segdepinfo[224];
	long int        start, length;

	bmsize = ((Lines * (int64) Pixels + 7) / 8);
	blocks = (bmsize + 511)/512 + 2 ;

	for (i = 0; i < isigns; i++) 
	{
	segnum = segs[i];

		/* Write classification results to bitmap segment (when in FULL IMAGE  mode)  */

		if ( ! (vmode || by_sections) )	
		{
			fprintf(Report, "Writing classification bitmap to bitmap %d. \n", segnum);
			/* GDBBitmapIO(idb_fp,GDB_WRITE,segnum,0,0,Pixels,Lines,classbitbuffer[i],Pixels,Lines); */

			write_bmp(idb_fp, classbitbuffer[i],segnum);  // to deal with bitmaps GT 2GB
		}

		/* write segment history */

		sprintf(history, "%s Sign=%d(%s) ITC=%d DBIC=%s EXT=%d T=%4.2f",
				VERSION, insigs[i], sigtypeO, isolbit, itostr(inchans, ibufs), lt_mask, threshold);
		GDBWriteHistory(idb_fp, SEG_BIT, segnum, history);

		/* Overwrite segment name and segment description  */

		sprintf(description, "ITC Class - From Sign %d(%s) DBIC=%s T=%4.2f",
								 insigs[i], sigtypeO, itostr(inchans, ibufs), threshold);
		IDBSegInfoIO(idb_fp,segnum,IDB_WRITE,NULL,NULL,PROG_NAME,NULL,NULL); 
		GDBSegDescIO(idb_fp, GDB_WRITE, SEG_BIT, segnum, description);

	} /* end of for many classes */
}

#endif

/******************************************************************/
/******************************************************************/

/* For each signature, this function creates output file names (distinct tif bitmaps)
   and writes the generated class bitmaps to them (in full image mode)
	and writes individual description  to all output segments*/

void write_GDAL_BM(char *fullfilename, int *segs, int segcnt, int *insigs, int *inchans, int isolbit)
{
	int 		i, j, segnum, size, blocks, segtype;
	int64 		bmsize;
	char        segflag, segname[9], history[120], segdepinfo[224];
	long int    start, length;
	char 	 *file_out;	
	char	seg_no[3];

	//bmsize = ((Lines * (int64) Pixels + 7) / 8);
	//blocks = (bmsize + 511)/512 + 2 ;

	for (i = 0; i < isigns; i++) 
	{
	segnum = segs[i];
	
	// Create output file name		e.g.: SECTEUR_BM_Class2.tif
	  
	strncpy(basefilname, fullfilename,  basef_len);		// get that part of the full file name
	basefilname[basef_len] = '\0';   				
	basefname = basefilname;				  
   
  // create rest of file name
	  
	  file_out = strncat(basefname,"_Class",6); 
	  
	  //printf("file_out  :  %s \n", file_out);  
	  	
	  //itoa (classcode[species],seg_no,10);  
  	  //itoa(i+1,seg_no,10); 	  
	  sprintf(seg_no,"%d",i+1);
	  strncat(file_out,seg_no,3);	  	  	
 	  strncat(file_out,".tif",4);
	  //printf("file_out  :  %s \n", file_out); 	
	
	
		/* Write classification results to bitmap segment (when in FULL IMAGE  mode)  */

		if ( ! (vmode || by_sections) )	
		{
			//fprintf(Report, "Writing classification bitmap to bitmap %d. \n", segnum);
			//GDBBitmapIO(idb_fp,GDB_WRITE,segnum,0,0,Pixels,Lines,classbitbuffer[i],Pixels,Lines);		// PCI version
			//write_bmp(idb_fp, classbitbuffer[i],segnum);  // to deal with bitmaps GT 2GB (in PCI)
			
			
			printf("\nWriting classification bitmap to file %s \n", file_out);		// GDAL version
	
			sprintf(Description,"From %s(%s): Class %d from Training Area %d DBIC=%s ITC=%d EXT=%d T=%4.2f",
						PROG_NAME, VERSION, i+1, insigs[i], ChList, isolbit, extmask, threshold);

			printf("\n  Description: %s \n",Description);		// write_bitmap from itc_io_g.cpp uses CAP Description
	
			write_bitmap(classbitbuffer[i], file_out);		// GDAL from itc_io_g module
			
			printf("\n----------\n");
		}

		
/*		
		// write segment history    //PCI version

		sprintf(history, "%s Sign=%d(%s) ITC=%d DBIC=%s EXT=%d T=%4.2f",
				VERSION, insigs[i], sigtypeO, isolbit, itostr(inchans, ibufs), lt_mask, threshold);
		GDBWriteHistory(idb_fp, SEG_BIT, segnum, history);

		// Overwrite segment name and segment description  

		sprintf(description, "ITC Class - From Sign %d(%s) DBIC=%s T=%4.2f",
								 insigs[i], sigtypeO, itostr(inchans, ibufs), threshold);
		IDBSegInfoIO(idb_fp,segnum,IDB_WRITE,NULL,NULL,PROG_NAME,NULL,NULL); 
		GDBSegDescIO(idb_fp, GDB_WRITE, SEG_BIT, segnum, description);
*/		
		

	}	 // end of for many classes
	
} 		// end of function


/******************************************************************/

/* This function checks the needed bitmap (class) segments and create new segment as needed  */

#ifdef PARTIAL_GDAL

void class_bitmaps(FILE * idb_fp, int *segs, int segcnt, 
				   int *insigs, int *inchans, int xsize, int ysize, int isolbit)
{
	int             i, j, segnum, size, blocks, segtype;
	char            segflag, segname[9], history[120], description[81], segdepinfo[224];
	long int        start, length;

	size = sizeof(unsigned char) * ((Lines * Pixels + 7) / 8);

	for (i = 0; i < isigns; i++) 
	{

		if (segcnt > i)
			segnum = segs[i];
		else
			segnum = -1;

		/* Check if segment of correct type, and size */

		blocks = (size + 511)/512 + 2 ;
		if (segnum > 0) 
		  {
		  IDBSegInfoIO(idb_fp, segnum, IDB_READ, &segflag, &segtype, segname, &start, &length);
		  if (!(((segflag == 'A') || (segflag == ' ')) && (length >= blocks) && (segtype == SEG_BIT))) 
			  {
			  fprintf(Report, "Specified segment %d not of required size/type.  ", segnum);
			  segnum = -1;
			  }
		  }

		/* Check if need to create new segment (put in a temporary description)*/

		sprintf(description, "ITC Classification ABORTED before full completion");
								
		if (segnum <= 0) 
		  {
		  fprintf(Report,"\nCreating new bitmap for class %d.\n", i);
		  segnum = GDBSegCreate(idb_fp,"ITCSC   ", description, SEG_BIT);
		  segs[i] = segnum;
		  }

		/* Overwrite segment name and segment description  */

		IDBSegInfoIO(idb_fp,segnum,IDB_WRITE,NULL,NULL,PROG_NAME,NULL,NULL); 
		GDBSegDescIO(idb_fp, GDB_WRITE, SEG_BIT, segnum, description);

	} /* end of for many classes */
}




/******************************************************************/

/* This function reads the information contained in a segment and returns a result node */

result_node    *  read_segment(FILE * idb_fp, int segnum)
{
	unsigned char  *inbuffer;
	result_node    *results;
	int             blocks, segtype;
	char            segflag, segname[9];
	long int        start, length;

	IDBSegInfo(idb_fp, segnum, &segflag, &segtype, segname, &start, &length);
	blocks = (sizeof(result_node) + 511) / 512 + 2;

	/* Check if segment type is valid */

/*	if (((segflag == 'A')||(segflag == ' '))&&(blocks >= length)&&(segtype == SEGTYPEBASE)) */
/* for the transition period to v2.0 and new sig_type, dont check signtype */
	if (((segflag == 'A')||(segflag == ' '))&&(length >= blocks)) 
	{
		inbuffer = (unsigned char *) CPLMalloc(sizeof(result_node) + 511);
/*		check_mem(inbuffer);*/
		results = (result_node *) CPLMalloc(sizeof(result_node));
/*		check_mem(results);*/
		IDBSegIO(idb_fp, segnum, 1, blocks - 2, inbuffer, IDB_READ);
		memcpy(results, inbuffer, sizeof(result_node));
		if (!(ibufs <= results->channels)) {
			fprintf(Report, "Signature in %d not of same type/channels.\n", segnum);
			exit(-1);
		}
	} else {
		fprintf(Report, "Segment %d not of correct size/type.\n", segnum);
		exit(-1);
	}

	return (results);
}

#endif

/******************************************************************/


/* This routine calculates the global covariance, based on the 
   covariance of the feature vectors from each tree in the image */

void process_global_covariance()
{
	tree_node      *p;
	int             i, j, k, n, size, tcount;
	float          sum[VECTORSIZE*3];
	float          product[VECTORSIZE*3][VECTORSIZE*3];
	float          tempvec[VECTORSIZE*3];

	fprintf(Report, "\nCalculating Global Covariance.\n");

	for (i = 0; i < spa_size; i++)
		sum[i] = 0.0;

	for (j = 0; j < spa_size; j++)
		for (k = 0; k < spa_size; k++)
			product[j][k] = 0.0;

	tcount = 0;
	p = tree_list->next_tree;

	/* Gather data from all of the trees */

	while (p) 
	{
		convert_to_vector(p, tempvec);
		for (i = 0; i < spa_size; i++) 
		  {
		  sum[i] += tempvec[i];
		  for (j = 0; j < spa_size; j++)
			product[i][j] += (tempvec[i] * tempvec[j]);
		  }
		tcount++;
		p = p->next_tree;
	}

	/* Form global covariance matrix */

	for (j = 0; j < spa_size; j++)
	for (k = 0; k < spa_size; k++)
		global_covariance[j * spa_size + k] = 
			(product[j][k] - ((sum[j] * sum[k]) / tcount)) / (tcount - 1);
}

/******************************************************************/

/* This function converts the data stored in a tree node into
    a "packed" format in a vector. */

void convert_to_vector(tree_node *tree, float *vector)
{
	int  ii, k, kk;
					
	kk=0;

	for (ii = 0; ii < 3; ii++)	/* up to 3 possible signature types */
	{
	  for(k=0;k<ibufs;k++)		/* for each channel */
	    {
	    if (st[ii] == MEAN) vector[kk+k]= tree->itc_mean[k];
	    if ( (st[ii] == TCL) || (st[ii] == TCL2) ) 
	      {
	      vector[kk+k]= tree->intercept[k];
	      vector[kk+ibufs+k]= tree->eigvector[k];
	      }
	    if (st[ii] == TCL2) vector[kk+ibufs*2+k]= tree->eigvalues[k];
	    }

	    if (st[ii] == TEXTUR) vector[kk]= tree->variance[0];

	    if (st[ii] == STRUCT) 
	      {
	      vector[kk]= tree->structure_param[0];
	      vector[kk+1]= tree->structure_param[ibufs];
	      }

	  kk = kk + spa_dim_mul[st[ii]] ;
	}
}


/******************************************************************/

/* This function converts the data stored in a result node (a signature) 
	into a "packed" format in a vector. */

void convert_to_vector_sign(result_node *results, float *vector)
{
	int  ii, k, kk;
					
	kk=0;

	for (ii = 0; ii < 3; ii++)		/* up to 3 possible signature types */
	{
	  for(k=0;k<ibufs;k++)			/* for each channel */
	    {
	    if (st[ii] == MEAN) vector[kk+k]= results->mean[k];
	    if ( (st[ii] == TCL) || (st[ii] == TCL2) ) 
	      {
	      vector[kk+k]= results->intercept[k];
	      vector[kk+ibufs+k]= results->eigvector[k];
	      }
	    if (st[ii] == TCL2) vector[kk+ibufs*2+k]= results->eigvalues[k];
	    }

	    if (st[ii] == TEXTUR) vector[kk]= results->variance[0];

	    if (st[ii] == STRUCT) 
	      {
	      vector[kk]= results->structure_param[0];
	      vector[kk+1]= results->structure_param[ibufs];
	      }

	  kk = kk + spa_dim_mul[st[ii]] ;
	}
}


/******************************************************************/

/* This function performs a vector subtraction, subtracting vector2 from vector1, leaving
   the answer in vector1.  The number of components in the vector is specified by size. */

void vector_subtract(float *vector1, float *vector2, int size)
{
	int             i;

	for (i = 0; i < size; i++)
		vector1[i] -= vector2[i];
}

/******************************************************************/


/* This function displays the components of the specified vector.  The number of components
   given by size are displayed. */

void view_vector(float *vector1, int size)
{
	int             i;

	for (i = 0; i < size; i++)
		fprintf(Report, "  %.2f  ", vector1[i]);
	fprintf(Report, "\n");
}


/*********************************************************/

/* This function calculates the inverse covariance matrix and the 
   determinant for each species (using the species signature)
   to be used in the maximum likelihood equation.
*/   

void cov_matrix() 

{
	int             i, j, k, ii, jj, kk;
	int             classifyI;
	matrix          m1, m2; 
	result_node    *sign;
	tree_node      *p;	
	//GDBShapeId	hShapeId;

	
/* GLOBAL TOTAL COVARIANCE MATRIX (stats from all ITC)*/

  if (cov_mat_type == GT)
    {
    fprintf(stderr,"\nUsing GLOBAL (image-wide) ITC Covariance Matrix.\n");
    fprintf(stderr,"\nThis feature is not available anymore\n");
    exit(-1);
    }		/* end of "if (cov_mat_type == GT)" */


/* GLOBAL SPECIES COVARIANCE MATRIX USING AVERAGE SPECIES COV.*/


    if ((cov_mat_type == GS) || (cov_mat_type == MA))
    {
	/* make an average cov. matrix from each species */

	fprintf(Report, "\nCalculating AVERAGE Species Covariance Matrix.\n");

	for (k = 0; k < isigns; k++) 
	{
	  sign = current_signs[k];

	  for (i = 0; i < spa_size; i++)
	  for (j = 0; j < spa_size; j++) 
	  {
	    glob_sp_cov_mat[i * spa_size + j] += sign->covariance[i * spa_size + j];
	  }
	}

	for (i = 0; i < spa_size; i++)
	for (j = 0; j < spa_size; j++) 
	  {
	    glob_sp_cov_mat[i * spa_size + j] = glob_sp_cov_mat[i * spa_size + j] / isigns;
	  }



	/* calculate inv. cov. matrix and determinant  */

	m1.rows = m2.rows = m1.cols = m2.cols = spa_size;
	m1.block = glob_sp_cov_mat;
	m2.block = glob_sp_inv_cov;

	  if (minv(&m1, &m2) != 0) 
	  {
	    fprintf(Report,"\n***Problem with species covariance matrix. \n");
	    fprintf(Report,"*** Can't form inverse for global species #%d.\n", k+1);
	    fprintf(Report,"\n*** Program exit is forced. \n\n");
	    exit(-1);
	  }

	/* glob_sp_det = detlu(&m1); */
	glob_sp_det = det(&m1);
	

 fprintf(Report,"\nFor global_species, determinant is  %8.3f\n", glob_sp_det ); 
 fprintf(Report,"\nFor global_species, log(determinant) is  %8.3f\n", log(fabs(glob_sp_det)) ); 
 

    }		/* end of "if (cov_mat_type == GS)" */


/* INDIVIDUAL COVARIANCE MATRIX FOR EACH SPECIES */

    if (cov_mat_type == SP)
    {
	/* setup cov. matrix for each species */

	fprintf(Report,"\n\tCalculating based on SPECIES SPECIFIC Covariance Matrices.\n");
	fprintf(Report,"\tSpace dimensions: %d\n",spa_size);

	for (k = 0; k < isigns; k++) 
	{
	  sign = current_signs[k];

	  for (i = 0; i < spa_size; i++)
	  for (j = 0; j < spa_size; j++) 
	  {
	    species_cov_mat[k][i * spa_size + j]=  sign->covariance[i * spa_size + j];

	    /* fprintf(Report," %7.4f ",species_cov_mat[k][i * spa_size + j]);
	    if (j == spa_size-1) fprintf(Report," \n "); */
	  }

	/* calculate inv. cov. matrix and determinant for each species */

	m1.rows = m2.rows = m1.cols = m2.cols = spa_size;
	m1.block = species_cov_mat[k];
	m2.block = species_inv_cov[k];

	  if (minv(&m1, &m2) != 0) 
	  {
	    fprintf(Report,"\n*** Problem with individual species (SP)covariance matrices.\n");
	    fprintf(Report,"*** Can't form inverse for species #%d.\n", k+1);
	    fprintf(Report,"??? Maybe signature type selected in ITCSSG and ITCSC are not compatible.\n");
	    fprintf(Report,"\n*** Program exit is forced. \n\n");
	    exit(-1);
	  }

	/* species_det[k] = detlu(&m1); */
	species_det[k] = (float) det(&m1);
	
	/* fprintf(Report,"For species %d log(determinant) is  %8.3f  \n", k+1,log(fabs(species_det[k])) ); */

	}	/* end of K loop */

    }		/* end of "if (cov_mat_type == SP)" */


/* SIGNIFICANCE LEVEL */

	fprintf(Report,"\n\tCalculating critical distance for significance level %.2f\n\n",threshold);
	
	critval = (float) critchi((1 - threshold), spa_size );
	
	fprintf(Report,"Critical value: %8.3f \n", critval);

/* To me, the significance level should be relative to ibufs (for the
MSS world, spa_size for the other spaces), not isigns, as was in
previous versions.  The number of signatures is irrelevant. By the time
we use CHI-SQUARE we have already made the decision about which class
to select, we just want to know if the distance relative to that
population is getting into the realm of chance events, say 19 times out
of 20 (SIGN_LEV=0.95). It's also not "ibufs-1", because we are
comparing with a different mean (the training mean), thus there are no
constraint about CHI-SQUARE distances summing up to zero. 
 ****** This is confirmed by Richards' book (1986), page 178.  */

}	/* end of function */

/*********************************************************/

/* This function classifies an ITC.  
   It uses the ITC parameter vector (i.e., the ITC signature), species mean vectors
   and covariance matrices (i.e., species signatures)
   parametrizing the distribution of the ITC parameter vectors within each species 
   and calculates distances (e.g., maximum-likelihood) from the tree signature to each of 
   the species signatures. The shortest distance is selected as the most likely species 
   for the ITC, provided it meets a minimum distance criteria (based on chi-square).
*/

void classify(tree_node * p) 

{
	int             i, j, k, ii, jj, kk;
	int             classifyI;
	matrix          m1, m2; 
	result_node    *sign;	
	//GDBShapeId		hShapeId;


	for (i = 0; i < VECTORSIZE*3; i++) tempvec1[i] = 0.0;
	for (i = 0; i < VECTORSIZE*3; i++) vector1[i] = 0.0;
	for (i = 0; i < VECTORSIZE*3; i++) vector2[i] = 0.0;	

	
/*	float  		classif_dist;
	int 		hist_dist[100];
	FILE 		*histo_fp;
	for (i = 0; i < 100; i++) hist_dist[i] = 0;
*/


		for (i = 0; i < isigns; i++) 
		{
		  sign = current_signs[i];
		  
		  //printf("\n Picking up signature %d \n",i);

		  convert_to_vector(p, vector1);
		  //   for (jj = 0; jj < spa_size; jj++) fprintf(Report," %8.3f ",vector1[jj]);fprintf(Report,"\n");  

		  convert_to_vector_sign(sign, vector2);
		   //  for (jj = 0; jj < spa_size; jj++) fprintf(Report," %8.3f ",vector2[jj]);fprintf(Report,"\n");  

		  vector_subtract(vector1, vector2, spa_size);
		  //   for (jj = 0; jj < spa_size; jj++) fprintf(Report," %8.3f ",vector1[jj]);fprintf(Report,"\n");  

		  if (cov_mat_type == SP) mvmpy(spa_size, spa_size, species_inv_cov[i], vector1, tempvec1);
		  if (cov_mat_type == GS) mvmpy(spa_size, spa_size, glob_sp_inv_cov, vector1, tempvec1);
		  if (cov_mat_type == GT) mvmpy(spa_size, spa_size, inv_covariance, vector1, tempvec1);
		  if (cov_mat_type == MA) mvmpy(spa_size, spa_size, glob_sp_inv_cov, vector1, tempvec1); 
		  
		  mvmpy(1, spa_size, vector1, tempvec1, &(small_dist[i]));

		  if (cov_mat_type == SP) small_dist[i] = -log(fabs(species_det[i])) - small_dist[i];
		  if (cov_mat_type == GS) small_dist[i] = -log(fabs(glob_sp_det)) - small_dist[i];
		  if (cov_mat_type == GT) small_dist[i] = -log(fabs(determinant)) - small_dist[i];
		  if (cov_mat_type == MA) small_dist[i] =  - small_dist[i];
		  
		  //fprintf(Report,"\nabs(distance) to class %d is %8.3f \n\n", i , - small_dist[i]);
		}

/* Pickup abs|smallest| distance as winner ( ">" because all negative numbers) */

		classifyI = 0;
		for (i = 0; i < isigns; i++) 
		  {
		  if (small_dist[i] > small_dist[classifyI]) 
			{p->class_2nd = classifyI; classifyI = i;}	//also keep track of 2nd class (previous best)
		  }	  
		  //printf("\n Decided Class :  %d \n", classifyI);

/* all smallest distances to histogram to check CHI-SQUARE behaviour */ 

/*		classif_dist = small_dist[classifyI] + log(fabs(species_det[classifyI]));
		if ( abs(classif_dist) > 99 ) classif_dist = 99;
		hist_dist[(int) abs(classif_dist)] += 1;
*/

/* Check if not too far to be classified - we classify with a threshold level */
/* if too far from even that winning class, consider unclassified */
/* i.e.: if not within a general confidence interval, consider unclassified */

		if (cov_mat_type == SP)  log2use = log(fabs(species_det[classifyI]));
		if (cov_mat_type == GS)  log2use = log(fabs(glob_sp_det));
		if (cov_mat_type == GT)  log2use = log(fabs(determinant));
		if (cov_mat_type == MA)  log2use = 0.0;
	/*	fprintf(Report,"abs(critval+log2use): %8.3f \n",(critval+log2use) ); */

/* Check relative to critical value ( ">" because all negative #)*/

		if ( small_dist[classifyI] > -critval - log2use )
			{
			p->classI = classifyI;
			(class_sum[classifyI])++;
			} 
		else 
			{
			p->classI = -1;
			unclassified++;
			}
			
		//printf("\n Tree classified in class  %d", classifyI+1);


// All the following is now done ONE LEVEL UP in scan_for_tree and loop_for_tree (_g)		
		
/*			
		if(vmode && PCI_File)			// in VECTOR mode, store class in field CLASSR in layer 
		  {
		  //sField.nInteger = p->classI;
		  sField.nInteger = p->classI +1;
		  if(speciescode) 		// USED ALL THE TIME NOW - just filled with a plain sequence if not really used 
		    {
		    if(p->classI == -1) sField.nInteger = UNCLASS;
		    if(p->classI >= 0) sField.nInteger = clascode[p->classI];
		    if (class2_flag) sField.nInteger = clascode[p->class_2nd];			// store 2nd class instead
		    }

		  hShapeId = p->shapeId;

      	  GDBSetFieldValue(hlayer, hShapeId, cField, &sField);

	     	  //fprintf(stdout,"Shape %d classified as %d and 2ndclass = %d \n", hShapeId, clascode[p->classI], clascode[p->class_2nd] );
      	  }



		  if(vmode && TIF_File)			// In VECTOR mode, store class in field CLASSR in layer 
		  {
		  //printf("\n\tPolygon %d classified as %d \n\n", iFeat, p->classI +1);
		  //printf( "classify() cField(%d) BEFORE %d \n", cField, gpiFeature->GetFieldAsInteger( cField ) );		//TEST cField
		  
		  //piFeature->SetField(cField, (p->classI +1) );		// normally
		  //gpiFeature->SetField(cField, (p->classI +11) );		 // DEBUG TEST  
		  //poFeature->SetField(cField, (p->classI +11) );		 // DEBUG TEST

		 // printf( "classify() cField(%d) AFTER %d in main \n", cField, gpiFeature->GetFieldAsInteger( cField ) );		//TEST	cField 		  
		 // printf( "cField(%d) AFTER %d in temp\n", cField, poFeature->GetFieldAsInteger( cField ) );		//TEST	cField  
		  
		  //printf( "\nAddresses  piFeature= %p  poFeature = %p \n",piFeature, poFeature);
		  
		  // Rely simply on newtree() to pass the class info
		  
		  }
			  
			  
			  
		  if(!vmode)		// IF in BITMAP mode, paint the proper class bitmap 
      	  {		
		  
		//if(p->classI > iclass)  fprintf(stdout,"\n\n *** ERROR *** Class out of range \n\n");
		
		  //printf("\n Decided Class :  %d \n", p->classI);
		  //printf("\n Tree Position  :  %d %d\n", p->xpos,p->ypos);
		  
		  // paint  class bitmap using ISOL bitmap as reference pattern 
		  
      	  if(p->classI >= 0)   fill(p->xpos, p->ypos,Pixels, tempbitbuf2, p, BIT_FILL);	// write to bitmap in memory
      	  }
*/


}	// End of function 		classify() 


/*********************************************************/

/* This function generates bitmaps according to how the trees were classified.  
	One bitmap per species is generated. */

void generate_bitmaps(int xsize, unsigned char *itc_bitmap)
{
	tree_node      *p;
	int tree_count=0;

	fprintf(Report, "\nGenerating bitmaps based on results of classification.\n");
	p = tree_list->next_tree;
	while (p) 
	  {
	  if (p->classI >= 0)	
			fill(p->xpos, p->ypos, xsize, itc_bitmap, p, BIT_FILL);
	  p = p->next_tree;
	  tree_count++;
	  }

	fprintf(Report,"\n### Tree count while painting output bitmaps (+unclassified) = %d \n\n", tree_count );
}


/******************************************************************/

/* 
	This function modifies a signature (result) node, based on the features selected
	for the classification (a subset) by using only the relevant part of the information 
	in the signature. After this function, the signatures can be dealt with as signatures
	of the proper size, dimensions and organization, without having to think about the fact
	that they were bigger at one point in time.
  
*/

result_node  *  mod_sign_org(result_node *results, int *dbic, int *index)
{
	int             i, j, count;
	int		sst1, sst2, sst3, sstype;	/* signature type */
	int 		sig_dim_mul[SIGTYPES+1];
	result_node    *new_results;
	float          temp_matrix[CHANNELS*3 * CHANNELS*3];

	/* Clear temp matrix */

	for (i = 0; i < CHANNELS*3 * CHANNELS*3; i++) temp_matrix[i] = 0.0;

	sign_bufs = results->channels;				/* Signature's number of channels */

/* If an index of channels is already supplied by the user (in SCHINDX), bypass  */

	if (schindx_flag) goto text_struct;

	/*	When using the SAME channels to classify but a SUBSET of the channels in the signatures,
		Check to make sure that specified DBICs are indeed a subset of the signature DBICs
		and if so, create an "index" vector to organize selection of proper subset */

	for (i = 0; i < ibufs; i++) 
	  {
	  index[i] = -1;
	  for (j = 0; j < results->channels; j++)
		if (dbic[i] == results->dbic[j]) 
			{
			index[i] = j;
/*			index[i+ibufs] = j + results->channels;
			index[i+ibufs*2] = j + results->channels * 2; */
			}

	  if (index[i] == -1) 
	    {
	    fprintf(Report,"\n\n*** ERROR - ITCSC DBICs do not match signature channels. ***\n\n");
	    fprintf(Report,"Please double check ITCSC DBICs and the signatures DBICs.\n\n");
	    fprintf(Report,"If DBICs do not match because the signatures were imported (ITCSSM)\n");
	    fprintf(Report,"from another image, but you know the corresponding channels, \n");
	    fprintf(Report,"then, rerun ITCSC using SCHINDX to indicate which of the signature\n");
	    fprintf(Report,"channels correspond to the ITCSC channels specified.\n\n");
	    exit(-1);
	    }
	  }

/* Index for TEXTURE and STRUCTURE signatures */

text_struct:

	if (textur_flag) index[ibufs] = sign_bufs;
	if ( (textur_flag) && (struct_flag) )
				for (i = 0; i < 3; i++) index[ibufs+i] = sign_bufs+i;
	if ( !(textur_flag) && (struct_flag) )
				for (i = 0; i < 2; i++) index[ibufs+i] = sign_bufs+(i+1);
/*
	fprintf(Report,"Feature order (index): \n");
	for (i = 0; i < spa_size; i++) fprintf(Report," %d ", index[i]+1);
	fprintf(Report,"\n");
*/

	/* NOTE: if all channels are same, dont change anything and use present signature */
/*
	count = 0;
	for (i = 0; i < ibufs; i++) if(dbic[i] == results->dbic[i]) count++;
	if (count == ibufs ) 
	{
		fprintf(stdout,"\n Existing signature is being used. No reorg. \n");
		return (results);
	}
*/

/* check signature type - diff. type imply diff feature subset and spa-size */

	sstype = results->SIG_type;
	fprintf(Report,"\nSignature is of the following type(s):");
	print_sig_types(sstype);
	fprintf(Report," (Code = %d) \n",sstype);

/* divide the sstype (signature stype) code into individual types */

	sst3 = sstype/100;
	sst2 = (sstype-sst3*100)/10;
	sst1 = (sstype-sst3*100-sst2*10);

	sig_dim_mul[0] = 0; 
	sig_dim_mul[1] = sign_bufs;		/* MEAN takes ibuf (i.e., no. CHs) dimensions */
	sig_dim_mul[2] = 2*sign_bufs;		/* TCL takes 2*ibufs dimensions */
	sig_dim_mul[3] = 3*sign_bufs;		/* TCL2 takes 3*ibufs dimensions */ 
	sig_dim_mul[4] = 1;			/* now, TEXTURE takes only one dimension (was ibufs)*/
	sig_dim_mul[5] = 2;			/* now STRUCTURE take only two dimensions */

/* calculate space size (number of dimensions being used) of signature */

	sign_spa_size = sig_dim_mul[sst1] + sig_dim_mul[sst2] + sig_dim_mul[sst3];
	fprintf(Report,"The dimensionality of the signature space is %d \n", sign_spa_size);

	/* Rebuild new result node based on present DBIC */
	
	if(sign_spa_size == spa_size) 
	  {
	  fprintf(Report,"** Possibly reordering the original channels and/or features.\n");
	  } else 
	  {
	  fprintf(Report,"** Creating new signature using a subset of the original features.\n");
	  }

	new_results = new_result_node();

	new_results->mask = results->mask;
	new_results->number = results->number;
	new_results->ITC_mask = results->ITC_mask;
	new_results->EXT_mask = results->EXT_mask;
	new_results->SIG_type = results->SIG_type;
	new_results->channels = ibufs;					/* new number of channels */



	for (i = 0; i < ibufs; i++) 
	  {
	  new_results->dbic[i] = results->dbic[index[i]];
	  new_results->mean[i] = results->mean[index[i]];
	  new_results->variance[i] = results->variance[index[i]];
	  new_results->intercept[i] = results->intercept[index[i]];
	  new_results->eigvector[i] = results->eigvector[index[i]];
	  new_results->eigvalues[i] = results->eigvalues[i];
	  new_results->structure_param[i] = results->structure_param[i]; 	/* X values */
	  new_results->structure_param[i+ibufs] = results->structure_param[i+ sign_bufs]; /* Ys values*/
	  }

	/* create subset cov. and corr. matrices */

	for (i = 0; i < spa_size; i++)
	{
	  for (j = 0; j < spa_size; j++) 
	    {
	    new_results->covariance[i*spa_size+j] = 
					results->covariance[index[i]*(sign_spa_size)+index[j]];

	    new_results->correlation[i*spa_size+j] = 
					results->correlation[index[i]*(sign_spa_size)+index[j]];
	
	    /* fprintf(Report, "\t%8.2f ", new_results->covariance[i*spa_size+j]); */
	    }
	/* fprintf(Report, "\n"); */
	}

	return (new_results);
}


/******************************************************************/

/* This function dumps the tree_list to a file when needed */

void db_output(FILE * idb_fp, char *dbfile)
{
	FILE           *db_fp;
	tree_node      *p;
	//float           xpixsz, ypixsz;
	char            pxunit[100], geosys[100];
	float          crownarea;
	//double			pA1, pA2, pB1, pB3;
	float          geox, geoy;
	int             number, specode;

	number = 1;

	fprintf(Report,"\nNOTE: Creating database of ITC signature information in FILE=%s \n", dbfile);

	/* Open file for database output */

	db_fp = fopen(dbfile, "w");
	if (db_fp == NULL) {
		fprintf(Report, "File System Error.  Can't create database file.\n");
		exit(-1);
	}

	/* Get Pixel Size and Georeferencing Information*/

	//IDBPixelSize(idb_fp, IDB_READ, &xpixsz, &ypixsz, pxunit);
	//IDBGeorefIO(idb_fp, IDB_READ, geosys, &pA1, &pA2, &pB1, &pB3);
	
	fprintf(db_fp,"\nCrown#, ShapeId, cg_x, cg_y, geox, geoy, crown(m2), count, p_count, class, clascode \n");
	fprintf(db_fp,"Followed by MSS vector \n\n");
	
	/* Process all trees */

	p = tree_list->next_tree;
	while (p) {
		/* Hopefully have x,y as centre of gravity already */
		crownarea = (xpixsz * ypixsz) * (p->count);
		//geox = pA1 + pA2 *  p->cg_xpos;
		//geoy = pB1 + pB3 *  p->cg_ypos;
		
		geox = topleftX + transformX *  p->cg_xpos;
		geoy = topleftY + transformY *  p->cg_ypos;
				
		
		if(p->classI == -1) specode = UNCLASS;
		if(p->classI >= 0) specode = clascode[p->classI];
		fprintf(db_fp, "%d %d %d %d %.2f %.2f %.1f %d %d %d %d", number, p->shapeId, p->cg_xpos, p->cg_ypos,
				geox, geoy, crownarea, p->count, p->process_count, p->classI, specode);
			fprintf(db_fp, "\n");
		output_sign_info(db_fp, p);
		fprintf(db_fp, "\n");
		p = p->next_tree;
		number++;
	}
	fclose(db_fp);
}


/******************************************************************/

/* This function prints into the ouput file (DBOUT) specific stats. 
   (depending on selected signatures) for each individual tree crown (ITC).
*/

void output_sign_info(FILE * db_fp, tree_node *tree)
{
	int i, ii;

	/* Print out means (used in MEAN signatures, but also just for reference) */

	for (i = 0; i < ibufs; i++) fprintf(db_fp, "%.4f ",tree->itc_mean[i]);
	fprintf(db_fp, "\n");

	/* eigenvector information */

	if (eigen_flag)
	  {
	  for (i = 0; i < ibufs; i++) fprintf(db_fp, "%.4f ",tree->intercept[i]);
	  fprintf(db_fp, "\n");
	  for (i = 0; i < ibufs; i++) fprintf(db_fp, "%.4f ",tree->eigvector[i]);
	  fprintf(db_fp, "\n");
	  }

	/* eigenvalue information */

	if (tcl2_flag)
	  {
	  for (i = 0; i < ibufs; i++) fprintf(db_fp, "%.4f ",tree->eigvalues[i]);
	  fprintf(db_fp, "\n");
	  }

	/* Print out variance, our first texture signature */

	if (textur_flag)
	  {
	  for (i = 0; i < ibufs; i++)fprintf(db_fp, "%.4f ",tree->variance[i]);
	  fprintf(db_fp, "\n");
	  }
}


/******************************************************************/

/* This function checks the contents of a result_node (signature) to check that the
   values can be used for the sigtype specified.  Checks that all values are "real". */

void check_signature_results(result_node *results, int segnum, int isolbm, int extbm)
{
	int   i, ii, flag[10], mflag;
	flag[0]=flag[1]=flag[2]=flag[3]=flag[4]=flag[9]=0;


	/* Check that species signature type(stype) is same as user-requested for classification */

	if ( results->SIG_type != stype )
	{
	  fprintf(Report,"\n*** ERROR: Species signature type (stype=%d) of segment %d : ",
					results->SIG_type, segnum); print_sig_types(results->SIG_type);
	  fprintf(Report," \n");
	  fprintf(Report,"\t is NOT the same as user-requested signature type (SIGTYPE = %d) \n",stype);
	  fprintf(Report,"\t for running the ITC classification: ");  print_sig_types(stype);
	  fprintf(Report," \n*** EXITING\n");
	  exit(-1);
	}

	/* Check that ITC and EXT bitmaps are the same as the user-requested ones for classification.
	   Only warnings are issued because signatures may come from manually delineated trees or
	   even a completly different image (after BRDF normalisation)*/

	if ( results->ITC_mask != isolbm && !vmode)
	{
	  fprintf(Report,"\n*** WARNING: The ITC bitmap in use is not the same as that of the signature\n");
	  fprintf(Report,"\t  Please, double check that this is what you really want to do.\n\n");
	  if(Report==stdout){fprintf(Report, "Press [Enter] to Continue: "); getchar();}
	}

	if ( results->EXT_mask != extbm  && !vmode)
	{
	  fprintf(Report,"\n*** WARNING: The EXT bitmap in use is not the same as that of the signature\n");
	  fprintf(Report,"\t  Please, double check that this is what you really want to do.\n\n");
	  if(Report==stdout){fprintf(Report, "Press [Enter] to Continue: "); getchar();}
	}


	/* Check that species signature information if all there */
	/* these NaN() are OK for Unix only, thus bypass now */

/*

	for (i = 0; i < CHANNELS; i++) if (NaN(results->mean[i])) flag[0] = 1;
	for (i = 0; i < CHANNELS; i++) if (NaN(results->eigvector[i])) flag[1] = 1;
	for (i = 0; i < CHANNELS; i++) if (NaN(results->eigvalues[i])) flag[2] = 1;
	for (i = 0; i < CHANNELS; i++) if (NaN(results->variance[i])) flag[3] = 1;
	for (i = 0; i < CHANNELS*2; i++) if (NaN(results->structure_param[i])) flag[4] = 1;
	for (i = 0; i < spa_size*spa_size; i++) if (NaN(results->covariance[i])) flag[9] = 1;
	
	mflag = 0;
	for (ii = 0; ii < 3; ii++)  
	if ( (flag[0]&&(st[ii]==MEAN)) || (flag[1]&&(st[ii]==TCL)) || (flag[2]&&(st[ii]==TCL2))
		 || (flag[3]&&(st[ii]==TEXTUR)) || (flag[4]&&(st[ii]==STRUCT)) || flag[9] ) mflag = 1;
*/

	/* Report about missing information in species signature (if needed) */

/*
	if (mflag)
	{
	  fprintf(Report,"\n*** ERROR: NaN values found in species signature(SIGNSEG=%d).\n",segnum);
	  fprintf(Report,"        Cannot classify with specified signature type.\n");
	  view_results(results, segnum);
	  exit(-1);
	}
*/
		fprintf(Report,"\n");

}

/******************************************************************/


/* This function converts a string to upper case characters */

void upper_case(char *c)
{
	while (*c != '\0') {
		if (*c >= 'a' && *c <= 'z')
			*c += ('A' - 'a');
		c++;
	}
}


/******************************************************************/

/* This function converts an array of integers to a string.  Note that the maximum length
   of the integer is 4 digits. */

char   * itostr(int *nums, int size)

{
	int             i, offset;
	char           *string;

	string = (char *) CPLMalloc(size * 5);
/*	check_mem(string);*/

	offset = 0;
	for (i = 0; i < size; i++) {
		sprintf((string + offset), "%d ", nums[i]);
		offset = (int) strlen(string);
	}
	sprintf((string + offset), "\0");

	return (string);
}


/******************************************************************/

/* This function is used by the qsort library call to compare elements in the array */

/*
int compare_ints(const void *vp, const void *vq)
{
	const int      *p, *q;
	int             x;

	p = vp;
	q = vq;
	x = (*p) - (*q);
	return ((x == 0) ? 0 : ((x < 0) ? -1 : 1));
}
*/

/*************************************************************************/

/* This function checks that the input parameter SIGTYPE is valid.  
   Invalid parameters will produce an error message and cause program exit.
   It also calculates "stype" and "spa_size".
*/

void check_param2(char *sigtype, int sigchrcnt)
{
	int  i, j, k, ii, nsig;

	// fprintf(Report,"\nSIGTYPE = %s, chrs count = %d \n", sigtype, sigchrcnt); 

	upper_case(sigtype);			/* convert all characters to upper case */
	for(k=0;k<65;k++) sigtypeO[k] = sigtype[k];	/* keep intact for ouput displays */
	separate(sigtype);		/* separate items in list separate by nulls for strcmp*/

	/* check the whole string for known types */
	/* create the stype code */

	nsig = stype = 0;
	for (i=0; i<SIGTYPES; i++) 
	{
	  for (k=0; k<sigchrcnt; k++)
	   if (strncmp(sigtype+k, SigTypes[i],4) == 0) 
		{ 
		  stype = stype + pow(10,nsig) * (i + 1);
		  nsig++;
		}
	}

	if ( (nsig == 0) || (nsig > 3) )
	{
	  if (nsig == 0) 
	  	fprintf(stderr,"** Invalid SIGTYPE. Enter at least one of MEAN/TCL/TCL2/TEXTUR/STRUCT.\n");
	  if (nsig > 3)	  
		fprintf(stderr,"** Invalid SIGTYPE. Only three signature types allowed at once.\n");
	  fprintf(stderr,"\n nsig count = %d \n", nsig );
	  fprintf(stderr,"\n stype = %d \n",stype);
	  exit(-1);
	}

	/* divide the stype code into individual types */

	st3 = stype/100;
	st2 = (stype-st3*100)/10;
	st1 = (stype-st3*100-st2*10);
	st[0]= st1; st[1]= st2; st[2]= st3;

	for (ii = 0; ii < 3; ii++) if(st[ii]==MEAN) mean_flag = 1;
	for (ii = 0; ii < 3; ii++) if((st[ii]==TCL)||(st[ii]==TCL2)) eigen_flag = 1;
	for (ii = 0; ii < 3; ii++) if(st[ii]==TCL2) tcl2_flag = 1;
	for (ii = 0; ii < 3; ii++) if(st[ii]==TEXTUR) textur_flag = 1;
	for (ii = 0; ii < 3; ii++) if(st[ii]==STRUCT) struct_flag = 1;	

	/* no point having TCL and TCL2 together, just TCL2 */
	if (stype == 32) {stype = 3; fprintf(Report,"\nSignatures TCL+TCL2 collapsed into TCL2\n"); }
	if (stype == 432){stype = 43; fprintf(Report,"\nSignatures TCL+TCL2 collapsed into TCL2\n"); }
	if (stype == 532){stype = 53; fprintf(Report,"\nSignatures TCL+TCL2 collapsed into TCL2\n"); }

	fprintf(Report,"\nITC classification using the following signature(s):");
	print_sig_types(stype);
	fprintf(Report," (Code = %d) \n",stype);

/* calculate space size (number of dimensions being used) */

	/* WAS sign_dim_mul[SIGTYPES+1] = {0,1,2,3,1,2} but always multiplied by ibufs (i.e., no. CHs) 
	when used -- NOW will have the number of dimension more directly */

	spa_dim_mul[0] = 0; 
	spa_dim_mul[1] = ibufs;		/* MEAN takes ibuf (i.e., no. CHs) dimensions */
	spa_dim_mul[2] = 2*ibufs;		/* TCL takes 2*ibufs dimensions */
	spa_dim_mul[3] = 3*ibufs;		/* TCL2 takes 3*ibufs dimensions */ 
		/* no point doing the following (TEXTURE and STRUCTURE) for all channels */
	spa_dim_mul[4] = 1;			/* now, TEXTURE takes only one dimension (was ibufs)*/
	spa_dim_mul[5] = 2;			/* now STRUCTURE take only two dimensions */

	/* WAS spa_size = ibufs*spa_dim_mul[st1] + ibufs*spa_dim_mul[st2] + ibufs*spa_dim_mul[st3]; */

	spa_size = spa_dim_mul[st1] + spa_dim_mul[st2] + spa_dim_mul[st3];
	fprintf(Report,"\nThe dimensionality of the pattern recognition space: %d \n", spa_size);
 
 
	if ( spa_size > 24 )
	{
	  fprintf(Report,"\n *** 24 dimensions is the present maximum allowed ***\n");
	  fprintf(Report,"Please reselect your signatures and # of channels combination\n");
	  fprintf(Report,"to take that factor into account. \n");
	  fprintf(Report,"\t NOTE: A high dimensionality is not usually advisable. \n\n");
	  exit(-1);
	}

}
 
/*************************************************************************/

/* This function checks that the input parameter TREETYPE is valid.  
   Invalid parameters will produce an error message and cause program exit.
	(TREETYPE = ITC/TT/SLayer )
*/

void check_param4(char *treetype)
{
int i, j;

upper_case(treetype);					/* convert to upper case */

//if (argcnt[9] == 0) strcpy(treetype,"ITC");		/* default treetype is ITC */

if ( (strcmp(treetype,"ITC") != 0) && (strcmp(treetype,"TT") != 0) &&
	   (strcmp(treetype,"SLAYER") != 0) )
  {
  fprintf(stderr,"\n *** ERROR *** \n");
  fprintf(stderr,"Invalid TREETYPE. Use one of: ITC/TT/SLayer \n\n");
  exit(-1);
  }
  
//if ( (strncmp(treetype,"TT",2) == 0) && (argcnt[7] == 0) )
	
if ( (strncmp(treetype,"TT",2) == 0) && (extmask == 0) )
  {
  fprintf(stderr,"\n *** ERROR *** \n");
  fprintf(stderr,"\n **** An external bitmap (EXTBIT) is need with TREETYPE = TT **** \n\n");
  exit(-1);
  }

if ( (strncmp(treetype,"TT",2) == 0) && stype > 1)
  {
  fprintf(stderr,"\n *** ERROR *** \n");
  fprintf(stderr,"\n **** TREETYPE = TT can only be used with SIGTYPE = MEAN **** \n\n");
  exit(-1);
  }


if ( (strcmp(treetype,"MANBM")==0) || (strcmp(treetype,"MANVEC")==0) )
  {
  fprintf(stderr,"\n *** ERROR *** \n");
  fprintf(stderr,"\nModes ManBM or ManVec are not available in ITCSC.\n\n");
  exit(-1);
  }

if ( strcmp(treetype,"SLAYER") == 0 )
  {
  vmode = 1;				/* set vector layer flag */
  printf("\n### NOTE:\n");
  printf("\tYou have selected a mode (SLayer) with assumes that manually\n");
  printf("\tdelineated tree crowns (polygons) are found in ISOLBIT  \n");
  printf("\tinstead of the normal automatically delineated bitmap ITCs.\n\n");
  }


} /* end of procedure */
    
/*************************************************************************/

/* This function print the various signature types in use */

void print_sig_types(int istype)
{
	int i,ist1,ist2,ist3;

	ist3 = istype/100;
	ist2 = (istype-ist3*100)/10;
	ist1 = (istype-ist3*100-ist2*10);

	for (i=1; i<= SIGTYPES; i++) 
	{
	  if (ist1 == i) fprintf(Report," %s ",SigTypes[i-1]);
	  if (ist2 == i) fprintf(Report,"+ %s ",SigTypes[i-1]);
	  if (ist3 == i) fprintf(Report,"+ %s ",SigTypes[i-1]);
	}

}
   
/******************************************************************/

/* This function converts a string where items are separated by commas, spaces,
   or pluses, to items  separated by nulls */

void separate(char *c)
{
	while (*c != '\0') {
		if ( (*c == ',')||(*c == ' ')||(*c == '+') ) *c = '\0';
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
	
	//for(ii=0; ii<BITMAPS; ii++) out_vect[ii]=0;	// zero vector for good measure : CHANNELS ???
	
	if(in_count == 0)  { *out_count = 0; return; }

	if(in_vect[0] < 0)
	  	{ fprintf(stderr,"\n##### ERROR - A range (neg.#) can not be first item.\n"); exit(-1); }
	kk=0;
	for (ii = 0; ii < in_count; ii++)
	  {
	  if(in_vect[ii] == 0) 
	  	{ fprintf(stderr,"\n##### ERROR - Channel/Segment number can not be zero \n"); exit(-1); }
	  if(in_vect[ii] > 0) 
	  	{ out_vect[kk] = in_vect[ii]; kk++; }
	  if( (in_vect[ii] < 0) && (abs(in_vect[ii]) <= in_vect[ii-1]) )
	  	{ fprintf(stderr,"\n##### ERROR - A range appears to be unacceptable.\n"); exit(-1); }
	  if(in_vect[ii] < 0) 
	    {
	    if ( (abs(in_vect[ii]+in_vect[ii-1]) + kk) > BITMAPS )
	  	{fprintf(stderr,"\n##### ERROR - Expanded list limited to %d segments.\n",BITMAPS); exit(-1); }
	    for(jj=0; jj<(abs(in_vect[ii]+in_vect[ii-1])); jj++) 
	    		{ out_vect[kk] = out_vect[kk-1] + 1; kk++; }
	    }
	  }
	*out_count = kk;

}	/* end of function */

/*********************************************************************************/
/******************************************************************/

/* This is a function to convert a single tree crown from a vector-form crown outline
    to a filled bitmap crown (border pixels: using center of pixel in or out as criteria
   for pixel in or out). Also used to paint training areas.

   NOTE:	This version of vect2rast() assumes that vertices where converted from
		georeferenced coordinates to image coordinates and if needed,
		to sub-area coordinates (DBIW) before entering vect2rast()
*/

#ifdef PARTIAL_GDAL

void vect2rast(int nVertex, GDBVertex *pasVertices, unsigned char *mask_data, int xsize, 
							GDBShapeId hShapeId, GDBLayer layer)
  {
  int ii, jj, kk, iii, jjj, x, y, icount, ipos;
  int64 bitnum;
  float xx, yy, xmin, xmax, ymin, ymax, slope, xp;

  float line_xx[10], line_xp[10], n_line_xx[10], n_line_xp[10];

  rast_pcount=0;

/* for debugping - Print vertices

  fprintf(stdout,"Now in vect2rast() \n"); 
  for (kk=0; kk<nVertex; kk++)
    {
    fprintf(stdout," %d = %8.2f,%8.2f ", kk+1, pasVertices[kk].x, pasVertices[kk].y);
    if ( ((kk+1)/4)*4 == (kk+1) ) fprintf(stdout,"\n");
    }
  fprintf(stdout,"\n");
*/

  /* gather x and y minimums and maximums positions */

  xmin = xmax = pasVertices[0].x;
  ymin = ymax = pasVertices[0].y;

  for (kk=1; kk< nVertex; kk++)
    {
    if (pasVertices[kk].x > xmax) xmax = pasVertices[kk].x;
    if (pasVertices[kk].x < xmin) xmin = pasVertices[kk].x;
    if (pasVertices[kk].y > ymax) ymax = pasVertices[kk].y;
    if (pasVertices[kk].y < ymin) ymin = pasVertices[kk].y;
    }

  ixmin = (int) (xmin); iymin = (int) (ymin);		/* round-off on the generous side (outside area)*/
  ixmax = (int) (xmax+1); iymax = (int) (ymax+1);
  xcg = (int) (1.0 + (xmax + xmin)/2);	
  ycg = (int) (1.0 + (ymax+ymin)/2);   /* approx center of grav. */

/*
   fprintf(stdout,"\n xmin,xmax,ymin,ymax = %8.2f,%8.2f,%8.2f,%8.2f\n", xmin,xmax,ymin,ymax); 
   fprintf(stdout,"Integer ixmin,ixmax,iymin,iymax,xcg,ycg = %d,%d,%d,%d,%d,%d\n", ixmin,ixmax,iymin,iymax,xcg,ycg);	
*/

/* The following line may be used to speed things up by not burning all manual trees (**UNTESTED**) */

  /* if ( (ixmin < dbiw[0]) || (iymin < dbiw[1]) || (ixmax > dbiw[0]+dbiw[2]) || (iymax > dbiw[1]+dbiw[3]) ) return; */


   if ( (ixmin < 1) || (iymin < 1) || (ixmax > Pixels) || (iymax > Lines) ) 
	{
	fprintf(stdout,"NOTE: A part of this polygon is outside the image area.\n");
	fprintf(stdout,"NOTE: This polygon will be DISREGARDED in signature generation.\n");
	printf("Shape_ID=%4d -- Outside BM Area\t", hShapeId);
	return;
	}

	 /* printf("Shape_ID=%4d--YES\t", hShapeId); */

/* scan the square area delineated by xmin,xmax,ymin,ymax 
*/

    for(y = iymin; y <= iymax; y++)	/* for each image line */
      {
      icount = 0;
    for(x = ixmin; x <= ixmax; x++)	/* within each image line */
	{
	bitnum = (y-1)*(int64)xsize + x-1;
	xx = (float) x - 0.5; 	 	/* for a given pixel center */
	yy = (float) y - 0.5;

	/* check witch line segment intercepts */

	for (kk=0; kk < (nVertex-1); kk++)
	  {

 	  if ( between(yy,pasVertices[kk].y,pasVertices[kk+1].y) )
		{
		/* compute projections of pixel center on that line */

		slope = (pasVertices[kk+1].y - pasVertices[kk].y) / (pasVertices[kk+1].x - pasVertices[kk].x);
		/* yp = slope * (xx - pasVertices[kk].x) + pasVertices[kk].y; */
		xp = 1/slope * (yy - pasVertices[kk].y) + pasVertices[kk].x;

		if ( (xp >= x-1) && (xp < x) ) /* if intercept within same pixel */
		  {
		  /*fprintf(stdout,"For image line = %d, xx = %8.3f and xp = %8.3f using vector %d,%d \n",
									 y, xx, xp, kk+1,kk+2); 	
		  */
		  line_xx[icount] = xx; 
		  line_xp[icount] = xp;
		  icount++;
		  }	/* end of if within same pixel */
		}	/* end of if between vertices */

	  }	/*end of vertices scanning */
	}	/* end of for each pixel on line */

	/* fprintf(stdout,"\t For line %d, icount = %d \n", y, icount);	*/

	if ( icount == 1) 
	{
	fprintf(stdout,"\n #### ERROR  ### Shape %d in layer %d may not be a closed shape\n", 
						hShapeId, layer->nSegment); 
	exit(-1); 
	}

/* sort intersection info by increasing xp coordinates */

	for (jj = 0; jj<icount; jj++)
	  {
          xmin = line_xp[0]; ipos = 0;
	  for (ii = 1; ii<icount; ii++) { if (line_xp[ii] < xmin) { xmin = line_xp[ii]; ipos = ii; } }
	  n_line_xp[jj] = line_xp[ipos];
	  n_line_xx[jj] = line_xx[ipos];
	  line_xp[ipos] = 100000;	/* arbitrary big number for item not to be used */
	  }

/*	fprintf(stdout,"\t Sort is done \n");
	for (jj = 0; jj<icount; jj++) fprintf(stdout,"\t %f ", n_line_xp[jj]);
	fprintf(stdout,"\n");
*/

/*	Fill between pairs of line intersections.
	Only use the pixel itself ((int)+1) if center of pixel is inside the crown,
	otherwise use the next inside pixel.
	This is implemented via +1.5 on the left and +0.5 on the right.
*/

     for (jj = 0; jj<icount; jj+=2)
	{
	x = (int) (n_line_xp[jj] + 1.5);		/* initial pixel position */
	while( x <= ((int)(n_line_xp[jj+1] + 0.5)) )	/* paint line until next intersection */
	  {
	  bitnum = (y-1)*(int64)xsize + x-1;
	  setbit(mask_data,bitnum); rast_pcount++;
	  /* setbit(extmaskbuffer,bitnum);	  for debugging, extBM as output  */
	  x++;
	  }
	}

     }		/* end of for each line */


/* FOR DEGUGGING - Print out a filled shape on screen */

/*
  fprintf(stdout,"\nRasterized tree crown: \n");
  fprintf(stdout," Covers area xmin,xmax,ymin,ymax = %d,%d,%d,%d\n", ixmin,ixmax,iymin,iymax);


  for(iii = iymin; iii <= iymax; iii++)
   {
    for(jjj = ixmin; jjj <= ixmax; jjj++)
	{
	bitnum = (iii-1)*(int64)xsize + jjj-1;
	fprintf(stdout," ");
	if(testbit(mask_data,bitnum)) fprintf(stdout,"X");
	else  fprintf(stdout,".");
	}
   fprintf(stdout,"\n");
   }
   fprintf(stdout,"\n");
*/

    // fprintf(stdout,"vect2ras() Filled pixels for this tree crown : %d \n", rast_pcount);

  }	/* end of function */

#endif

  
/******************************************************************/


// tell whether the first value (a) is between the other two (b,c) 
/*
int between(float a, float b, float c)
{
  if( (c > b) && (a<c) && (a>=b) ) return(TRUE);
  if( (b > c) && (a<b) && (a>=c) ) return(TRUE);
  return(FALSE);
}
*/


/*********************************************************************************/




//***************************************************************************************
//***************************************************************************************

// CREATE   an output shp file  (copy of the original)

//*******************************************************************************************
//***************************************************************************************

void create_output_file()
{

	char	*file_out_SHP;
	
// Make name of ouput shp file

	file_out_SHP = "temp2.shp";
	
// Get the driver for "ESRI Shapefile"

	const char *pszDriverName = "ESRI Shapefile";
	GDALDriver *poDriver;

	poDriver = GetGDALDriverManager()->GetDriverByName(pszDriverName );

	if( poDriver == NULL )
	{
          printf( "%s driver not available.\n", pszDriverName );
          exit( 1 );
	}
	

// CREATE an OUTPUT SHP FILE 

	poDS = poDriver->Create( file_out_SHP , 0, 0, 0, GDT_Unknown, NULL );

	if( poDS == NULL )
	{
	    printf( "Creation of output file %s failed.\n",file_out_SHP );
	    exit( 1 );
	}

	printf( "\t\t**Created output SHP file : %s \n\n", file_out_SHP);

// Create the output LAYER

	poLayer = poDS->CreateLayer( file_out_SHP, NULL, wkbMultiPolygon , NULL );

	if( poLayer == NULL )
	{
	  printf( "Layer creation failed.\n" );
	  exit( 1 );
	}

	
// Create the output layer AND copy fields definitions AND copy features 
// ### NOT WORKING - CopyLayer not there ###

/*
	 if( poLayer = CopyLayer(psLayer, "copy_of_shp", NULL ) != OGRERR_NONE )
	   {
 	   printf( "Copying layer 1 to file %s failed.\n" , file_out_SHP );
	   exit( 1 );
	   }

	 printf("\n\nCopying layer 1 to file %s .\n" , file_out_SHP );
	 
	 //goto Exit_1;

*/

// Copy GeoTransform, projection, ...


//	printf("\n\t*Writing geographic projection for output file %s\n", argv[3]);	
//	poDS->SetSpatialRef(piDS->GetSpatialRef());	
//	poLayer->SetSpatialRef( piLayer->GetSpatialRef );
//	poFeature->SetGeometry( piFeature->GetGeometryRef() );
//printf( "\n\nCreated a polygon layer in : %s (??with spatial ref. and geom.) \n", file_out_SHP);


	

// Create SCHEMA  (i.e., FIELDS) for that output layer


	//printf( "\n\n\tCopying input fields (SCHEMA) to output polygon layer 1 in : %s \n\n", file_out_SHP);
 
	last_field = field_count;
	
	for( oField = 0; oField < last_field; oField++ )		// field counter
	{
	poFieldDefn = piFDefn->GetFieldDefn( oField ); 		// from input fields
	//poFieldDefn->SetPrecision(4);						// set output precision differently


	  if( poLayer->CreateField(poFieldDefn, TRUE ) != OGRERR_NONE )
	    {
 	    printf( "*** Creating field %hS FAILED.\n" , poFieldDefn->GetNameRef() );
	    exit( 1 );
	    }

	printf("Creating Field %d : %hS Precision: %d \n", oField, poFieldDefn->GetNameRef(), poFieldDefn->GetPrecision() );

	}

	printf("\n");
	
/*	
// Double checking SCHEMA

  printf("\n\tDouble checking fields in output shape file \n\n");

  poFDefn = poLayer->GetLayerDefn();

  for (iField=0; iField < poFDefn->GetFieldCount(); iField++)
    {
	poFieldDefn = poFDefn->GetFieldDefn(iField);
	printf("Field %d, Name: %s, Field Type %d, Field Width %d, Precision %d\n", 
		iField, poFieldDefn->GetNameRef(), poFieldDefn->GetType(), poFieldDefn->GetWidth(), poFieldDefn->GetPrecision());
	}
	
*/


//		COPY FEATURE to new file

/*
  To write a feature to disk, we must create a local OGRFeature, set attributes and attach geometry 
  before trying to write it to the layer. It is imperative that this feature
   be instantiated from the OGRFeatureDefn associated with the layer it will be written to.
*/

    poFDefn = poLayer->GetLayerDefn();			// get layer definition (SCHEMA)just created

	last_feat = feat_count;
	
	//printf( \"\n\n\tCopying features(polygons: attrib & vertices) from input to layer 1 in : %s \n\n",  file_out_SHP);
	
	psLayer->ResetReading();	// VERY IMPORTANT (cause we use GetNextFeature() 
	poLayer->ResetReading();
	
   // PolygonFeature Polygon;

	for (iFeat=0; iFeat<last_feat; iFeat++)			// for no. of features

	  {
	  //OGRFeature *piFeature; 
	  piFeature = psLayer->GetNextFeature();
	  //cField = 6 ;
      printf("Feature %d Field %d Presently assigned class %d\n", iFeat,cField,  piFeature->GetFieldAsInteger(cField) );	 
 	  
	  piGeometry = piFeature->GetGeometryRef();

	// Create a new output feature
	
	  //OGRFeature *poFeature; 
		
	  poFeature = OGRFeature::CreateFeature( poLayer->GetLayerDefn() );		//Add SCHEMA to that feature (1st)

	  poFeature->SetGeometry( piFeature->GetGeometryRef() );		// ????
  

//	MOVE ATRIBUTE DATA for each feature

// For each field in the input layer, populate output layer with same data 

  for (iField=0; iField < last_field; iField++)		// #####
    {
    piFieldDefn = piFDefn->GetFieldDefn( iField );
    poFieldDefn = poFDefn->GetFieldDefn( iField );
/*
    printf("%d Field Name %s, Field Type %d, Field Width %d, Precision %d \n", 
		iField, piFieldDefn->GetNameRef(), piFieldDefn->GetType(), poFieldDefn->GetWidth(), poFieldDefn->GetPrecision());
*/

// Get and Copy (Should use CASE as per tutorial example)

        switch( piFieldDefn->GetType() )
            {
                case OFTInteger:
                    //printf( "%d \n", piFeature->GetFieldAsInteger( iField ) );
                    poFeature->SetField(iField, piFeature->GetFieldAsInteger(iField) );
                    break;
                case OFTInteger64:
                    //printf( CPL_FRMT_GIB "\n", piFeature->GetFieldAsInteger64( iField ) );
     		    poFeature->SetField(iField, piFeature->GetFieldAsInteger64(iField) );
                    break;
                case OFTReal:
                    //printf( "%.3f \n", piFeature->GetFieldAsDouble(iField) );
     		    poFeature->SetField(iField, piFeature->GetFieldAsDouble(iField) );
                    break;
                case OFTString:
                    //printf( "%s \n", piFeature->GetFieldAsString(iField) );
     		    poFeature->SetField(iField, piFeature->GetFieldAsString(iField) );
                    break;
                default:
                    //printf( "%s \n", piFeature->GetFieldAsString(iField) );
     		    poFeature->SetField(iField, piFeature->GetFieldAsString(iField) );
                    break;
            }


		if( (iFeat == 3) && (iField == 2) )
				poFeature->SetField(2, 55 );		// TESTING - set 55 at 4th feature, 3rd field
			
			
// Other tests
   //poFeature->SetField(iField, piFeature->GetType(iField) );
    //poFeature->SetField(iField, piFieldDefn->GetType() );
    //poFeature->SetField(iField, piFeature->FieldValue(iField));    // should work ??
    //poFeature->SetField(iField, iField );		// WORKS : put field # in field for all features

    }		// end of loop for fields
	
	
	
// MOVE VERTICES of polygon (i.e., geometry) to output feature

	       poFeature->SetGeometry( piGeometry );		// that's all ???


// Need to move the feature (polygons) and its attributes to the output file

	// Now we create a feature in the file

	     if( poLayer->CreateFeature( poFeature ) != OGRERR_NONE )
 	    {
        	printf( "Failed to create feature in shapefile.\n" );
        	exit( 1 );
 	    }
	     
		OGRFeature::DestroyFeature( piFeature );
		OGRFeature::DestroyFeature( poFeature );
		
		//poLayer->GetNextFeature();
		
		
	  //printf("\tPolygon %d (attrib & vertices) moved to output SHP file \n", iFeat);
		
	  }		// LOOP  for NEXT feature 

	  
	  
	  
	printf("\t** All polygons (and attributes) moved to output SHP file %s\n", file_out_SHP);		
	printf("\tCurrent layer of this output file has %Id features(shapes) with %d fields each\n\n", 
						 poLayer->GetFeatureCount(), poFDefn->GetFieldCount());	
	
	//printf("\nInput Layer Description: %s \n", piLayer->GetDescription() );	
/*	
	strcpy(description, "Subset from ");		// create a description
	//strcat(description, filename);
	poLayer->SetDescription(description);
	
	printf("\nOutput Layer Description: %s \n", poLayer->GetDescription() );


Exit_1:	

	printf("\n\t*Closing OUTPUT files and exiting program. \n");
	
	GDALClose(poDS);
*/	
	
}	// End of create_output_file()
	
//*********************************************************************
//*********************************************************************


