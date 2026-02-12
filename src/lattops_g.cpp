/* 
Program name: 	lattops_g.cpp

Author: 	François A. Gougeon

Description:

	Find treetops in dense areas and treetops with specific shadows in
	more open areas, as designated by the directionality mask (DIRMASK).


	François A. Gougeon, Ph.D.
	Remote Sensing Research	
	Natural Resources Canada
	Canadian Forest Service 
	Pacific Forestry Centre
	506 West Burnside Rd.
	Victoria, British Columbia, 
	Canada, V8Z 1M5	

			
Parameter Description:

C+
C	LATTOPS (Locally Adaptive Tree Tops)
C
c	Find treetops in dense areas and treetops with specific shadows in
c	more open areas, as designated by the directionality mask (DIRMASK).
c
C	This program moves a window of a given size on an input image and
C	detects the locally centered grey-level maximum of each window's instance.
C	For the areas covered by the directionality mask (DIRMASK)
c 	it keeps only those max. with a significant low grey level area 
C	(i.e., a shadow) in the direction commensurate with SUNANG (180o + SUNANG).
c	(N.B.: The angle assumes North at top of the image)
c
c	Optionally: Operation under the directionality mask (open areas)
c		    are done on a smoothed version of the image, if supplied
c		    (i.e., if ILLUMCH = a,b). This may improve results.
C
c	Optionally: If DIRMASK=1, trees from the whole image (except under NFMASK)
c		    are considered to have discrete shadows (was SHADOW_TT)
C
C1	PARAMETERS
C
C	LATTOPS is controlled by the following global parameters:
c
c	***** Changed parameters order compared to PCI version (i.e., NFMASK before DIRMASK)
C
C	Name		Prompt					Count	Type
C
C	FILE		Database file name (for input/output)	128	Char
C	ILLUMCH		Input Illumination (B&W) Channel	2	Int
C	DIRMASK		High directionality mask		1	Int
C	NFMASK		Bitmap to mask out non-forested areas	1	Int
C	WINDSIZ		Size of moving window (e.g.,3,5,7)	1	Int
c	THRSHADE	Threshold to removed shaded areas	1	Int
c	SUNANG		Angle of sun relative to image top	1	Int
c	FORTYPE		Forest type (MATURE/REGEN/BIG/PROMPT)	64 	Char
C	TTBITM		Output bitmap of tree tops		1	Int	
C	REPORT		Reporting device			64	Char
C
C
C2	FILE
C
C	Specifies the name of the PCIDSK file containing the input and output
C	channels (images) or bitmaps (themes).
C
C	EASI>FILE="filename"
C
c2	ILLUMCH
c
c	Specifies the input illumination channel, typically a nIR band, or an
c	intensity channel (from IHS), or a vegetation index channel, ...
c
c	Note: A nIR image may not be ideal to fing snags.
c
c	Optionally: Operation under the directionality mask (open areas)
c		    are done on a extra smoothed version of the image, if supplied
c		    (i.e., if ILLUMCH = a,b). This improves results.
C
c	EASI>ILLUMCH=n		for one illumination channel
c	EASI>ILLUMCH=n,m	for a channel and its smoothed version
c
C2	DIRMASK			** Changed parameters order compared to PCI version (i.e., NFMASK before DIRMASK)
C
C	Input mask with bit set for areas of high directionality due to
C	specific shadows on the ground (typically from GRAD_DC and THR)
c	where LATTOPS will be looking for individual shadows (at inverse
c	azimuth from sun's) before a local maxima is declared a tree top.
c
C	EASI>DIRMASK=n
c
c	** Special cases with DIRMASK ** 
c
c	DIRMASK=1, for which the whole image is considered directional
c	(i.e., trees generally have individual shadows everywhere in the image,
c	except non-forest areas (NFMASK)) (i.e., equivalent to the old SHADOW_TT)
c
c	When "DIRMASK=-", the regular tree top (local maxima) algorithm is run
c	throughout the image (except non-forest areas (NFMASK)). In this case,
c	of course, SUNANG is not used at all.
c	
C
C2	NFMASK	** Changed parameters order compared to PCI version (i.e., NFMASK before DIRMASK)
c
c	Bitmap to mask out non-forested area
c	Use this parameter to prevent tree tops being found in
c	non-forested (or any undesired) areas.
c
c	EASI>NFMASK=n
c
C2	WINDSIZ		
c
c	Size of moving window (e.g.,3,5,7)
c	Local maxima will be gathered from center of window of
c	size WINDSIZ*WINDSIZ. (If unspecified, 3x3 is default)
c
c	EASI>WINDSIZ=n
c
c2	THRSHADE,THRSHADOW	
c
c	Threshold to removed shaded areas. Use this parameter to prevent tree
c	tops from being found in purely shaded areas. Also used when finding,
c	tree shadow: four pixels lower than THRSHADOW must be found in a 3x3
c	window at suspected shadow location (based on SUNANG) (i.e., shadows are
c	assumed brighter than deep shade between trees in dense forest)
c
c	HINT:	- Use the typical value found at interface between crown
c		material and shaded area in a DENSE forest
c		- If only interested in trees with shadows (DIRMASK=1),
c		use the typical value found at interface between crowns
c		and their shadows 
c		- If a second value is not specified, THRSHADOW is calculated
c		as follows:   THRSHADOW = 1.3 * THRSHADE
c
c	EASI>THRSHADE=n,m
c
c2	SUNANG
c
c	Specifies the sun azimuthal angle (in degrees) relative to image.
c	NOTES:	This is not always the sun azimuth (relative to true North).
c	This is the angle of sun  relative to the image, as if North was
c	atop the image. It is sun azimuth only if the image was geometrically
c	corrected. Also, this is not the angle of the shadows, but that of the sun's.
c	The shadows are in the direction of the sun illumination rays (180o more).
c
c	EASI>SUNANG=n
c
c
c2	FORTYPE
c
c	EASI>FORTYPE="forest type"
c
c	Specifies the type of forest to analyse. Possible values are MATURE, REGEN, BIG 
c	and PROMPT, defaults to MATURE. Depending on this parameter, LATTOPS looks
c	at different distances for a shadow area (in the direction opposite to SUNANG)
c
c	If REGEN, MATURE, or BIG is specified, distances of 2, 4 and 6 metres
c	will be calculated from the pixel size. Think of it this way: shadows have to
c	start at a distance that is as a bit more than the typical crown diameter.
c	If PROMPT is specified (or anything else), the user will be prompted for a 
c	single distance, in pixels.
c
C2	TTBITM
C
C	Specifies an optionnal output bitmap number in which to write
c	the tree tops. (If none specified, a new bitmap is created)
C
C	EASI>TTBITM=n
C
C2	REPORT
C
C	Specifies the file to append generated report to:
C
C	EASI> REPORT = "filename"
C
C	Note:  The following names have special meaning:
C
C	EASI> REPORT = "TERM"	| generates reports on your terminal
C	EASI> REPORT = "DISK"	| generates reports on file "IMPRPT.LST"
C	EASI> REPORT = "OFF"	| (may) switch off report generation
C
c
c ******************************
c
C1	REVISION HISTORY
C
c	François A. Gougeon, Ph.D.
c	Remote Sensing Research
c	
C	(©)Natural Resources Canada
c	Canadian Forest Service 
c	
c
C	
C v 1.0	Nov.96		François Gougeon
c
c			- (ANSI C Version, PCI V5.3)
C			Based on TREETOPS.c and SHADO_TT.c combined
C			
c v 2.0 Nov.97		François Gougeon
c
c			- Added capability to deal with 16-bit images by 
c			using "itc_io.h", cleaned up by using "bitops.h"
c			and introduced NFMASK, WINDSIZ, THRSHADE
c
c v 2.1 Dec 97		François Gougeon
c
c			- Minor cleanups (e.g., DIRMASK rather than DBIB)
c
c v 2.2 Feb. 98		François Gougeon
c
c			- Cleaned up, intro. "sunang" as outside param.			
C
C
c v 2.3 Feb.99		François Gougeon
c
c			- Instead of DBIC and DBOC, now using ILLUMCH and TTBITM.
c
C
c v 2.4 Feb.2000	François Gougeon
c	
c			- Better calculation of shadow angle in image relative
c			  to azimuthal sun angle
c			- Criteria is now four(4) pixels of shade in a 3x3 window
c			  centered on shadow area at distance of five(5) pixels.
c			- In future(maybe), introduce sun elevation as 2nd parameter 
c			  in SUNANG (i.e., SUNANG = (azimuth, elevation))
c
c v 2.5 Feb.2000	François Gougeon
c
c			- Introduced the concept that grey levels of shadows can be
c			  as high as 30% higher than grey levels of shade, thus
c				thrshadow = 1.3 * THRSHADE
c
c v 2.6 March 2000	François Gougeon
c
c			- Separated the image processing "under" the directionality mask 
c			  (more open forest areas) from that "not under" the directionality 
c			  mask (more densely forested areas) in two separate loops so that 
c			  the former can be done on smoothed illumination channel if 
c			  available (i.e., if ILLUMCH = a,b).
c
c v 2.7 Dec. 2001	François Gougeon
c
c			- Renamed to lattops.c from tt_both.c
c			- Updated help file with linking information and EASI prompts
c			- Replaced most IDB functions with GDB functions (not IDBPixelSize or ISBSegInfoIO
c			  as there are no equivelant GDB functions)
c			- Added IDBPixelSize and GDBGeorefIO
c			- Updated to use IDBSegInfoIO, GDBSegDescIO to change segment description
c			- Added local variables (xsize,ysize,channels) for passing to functions
c			- Altered history so under 56 characters			
c
c v 2.8 Dec. 2003	François Gougeon
c
c			- Fixed a bug preventing proper use on 16bit images (two vars where
c			  still defined as PixVal rather than integer)
c
c v 2.9 March 2004	François Gougeon
c
c			- Introduced input parameter FORTYPE (see definition above)
c			- Made sure that masked pixels (NFMASK) are not considered when
c			  when evaluating local maxima (thus, brite lichen beside crown,
c			  that may have been masked out, do not influence max. detection)
c			- Made sure that masked pixels (NFMASK) are not considered when
c			  when evaluating shadow area (thus, water, if masked out, 
c			  can not be considered shadow, thus no trees on the beach)
c			- Introduced capability to input shadow threshold directly, as
c			  second paramater on THRSHADE, as opposed to: 
c				thrshadow =  1.3 * thrshade
c			  which is still the default if no shadow thres is entered.
c			- If distance to shadow is small, thus small trees, dont look
c			  for as many shade pixels (2 instead of 4) in 3x3 shadow window.
c
c v2.99	April-June 2007	François Gougeon
c
c			- Minor adaptations for PCI v10 and its new PRM.PRM file 
c			  Most PCI routines are in PCI1000.dll, but not all.
c			  For example, IMPTime() and IMPReturn() are now in Core1000.dll
c			  and IMPCounter() in Counter1000.dll
c			  Since I dont have PCISDK or PCIProSDK, I had to create LIBs 
c			  from their DLLs to compile my progs.
c
c			  Also, problem with the REPORT (*Report) Variable
c
c
c v3.0	Feb 2016	François Gougeon
c
c			- Changed to a C++ program to deal with versions of PCI > v10.2
c				int main (), ".cpp" name, and extern "C" around .h include files
c			- mod to some call to fit new library definitions (as per .def demangling)
c			  which also means mods to corresponding declaration .h files (e.g., gdb.h)
c			  (Since I dont have PCISDK or PCI/ProSDK, I have to create LIB and DEF from
c			  from their DLLs (via DUMPBIN and LIB) in order to compile my progs
c			- modules in files like gdb.h need to be declared 'extern "C++" to link
c			  with proper name mangling (MSVC++ mangling)
c			- ***WORKED**  for PCI 10.3 that migrate its LIB to c++, 
c
c			- HOWEVER, more stuff is needed for PCI 2015 (64bit)
c			- PCI LIB need to be /MACHINE:x64 
c			  and prog compile with 64b version (i.e., vcvarsall amd64)
c			  cause all the call to lib are now with 64bit pointers 
c			- IMPStatus()  uses "char const *"
c  
c
c v3.1	Sept 2016	François Gougeon
c
c			- Now 128 character Filename, thus longer path
c			- Org. for bitmaps > 2GB via (int64) in bitnum and bmsize, and doing
c			  bitmap output via itc_io.cpp/write_bmp()
c
c v3.2	Feb 2023	François Gougeon
c
c			- Fixed bug: using BIG could get it to look for shadows off the image
c
c
c
c
c  v3.3	July 2023	François Gougeon 
c
c			- Migration to GDAL as lattops_g.cpp
c			- Not done a migration in a while, so lots of issues, notably
c			  - Tons of checking on input parameters (line parameters)
c			  - Some problems with get_pix_val()
c			- Org. for full path and ArcGIS compatibility and default output : basefilename_TTs.tif
c			- User can specify an output file name if preferred (OR one gets created)
c
c			- Changed parameters order compared to PCI version (I.e., NFMASK before DIRMASK)
c			- Org. such that you can use either infile.pci,1,4 OR infile.pci 1,4 (as separate param, for ArcGIS)
c			- Org. for more flexibility introding new parameter by addressing them as arcv[argcount]
c
c		** NOTE: I will not migrate TREETOPS to GDAL, as LATTOPS can do it all (skip directionality mask)
c			
c			> lattops_g FILE,ILLUMCH NFMASK - WINDSIZ THRSHADE - - -	//  TREETOPS equivalent
c		
c  v3.4	Nov. 2023	François Gougeon 	
c
c		- Making sure the the 2nd phase (directionality based - open areas shadows) is not used
c		when not called for (i.e., no DIRMASK)  and no need for irelevanr input parameters
c		in such case, thus a cleaner TREETOPS equivalent. For example:
c			> lattops_g PP833591_RED_Inv_FAV.tif 1 PP833591_Veg.tif - 3 100 - -
c	
c  		- Fixed a bug that was creating bad TTs (blobs) with 3x3 runs ("<" instead of "<="), speudo 2x2
c
c		- Reintroduced (with DIRMASK = 1) the concept of SHADOW_TT, where the whole image is
c			considered open forest where individual shadows are visible
c
c
c		- NOTE:	This program (and other of my GDAL progs) always complain when overwriting an exiting 
c				output file. Most of the time it is not an issue. However, sometimes when ArcGIS has
c				control of the file, it may not overwrite at all and leave you with previous result
c
c
c  v3.5	March 2025	François Gougeon 
c
c		- To accept a FOREST mask instead of a non-forest mask by using ",-1" with the file name
c
c
c  v3.6	Aug. 2025	François Gougeon 
c
c		- To not detect TTs adjascent to non-forest mask 
c  			(i.e., if NF is part of the 3x3 or 5x5 window)
c
c
*******************************************************************************
c
*** To compile with Visual Studio see GDAL_ITC-Suite_Compile.txt
c
c***************************************************************	
	
#### GDAL PROGRAM USAGE

Generally, 
the info about the parameters for PCI(as specified above) apply to the GDAL version


> lattops_g FILE,CH1,CH2 NFMASK DIRMASK WINDSIZ THRSHADE,THRSHADOW SUNANG FORTYPE TTBITM

> lattops_g FILE ILLUMCH NFMASK DIRMASK WINDSIZ THRSHADE,THRSHADOW SUNANG FORTYPE TTBITM

> lattops_g FILE,ILLUMCH NFMASK - WINDSIZ THRSHADE - - -	//  TREETOPS equivalent - no open areas shadows

> lattops_g FILE ILLUMCH NFMASK 1 WINDSIZ THRSHADE,THRSHADOW SUNANG FORTYPE TTBITM  // SHADOW_TT equivalent

NOTE:	Adding a ",-1" to the NFMASK file name make it a FOREST mask

*/


#define VERSION "v3.6"
#define PROG_NAME "LATTOPS"
#define FILENAME    250

#include <stddef.h>		// standart C inclusions
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <time.h>       // time_t, struct tm, time, localtime

#include "gdal_priv.h"		// For GDAL library
#include "ogrsf_frmts.h"	// For OGR


#include "ITC-Suite_g.h"		// my newest GDAL variable setup
#include "itc_io_g.h"		// my newest GDAL image/bitmap input/output
#include "bitops.h"		// bit operations on bitmaps (mostly macros to be faster)


		/* Function Declarations */

void    upper_case(char *);

extern PIX_FUN_PTR	get_pix_val; 	 // will get it from itc_io_g

		/* Global Variables */
		
// Mostly to share with functions in itc_io_g.cpp

char 		*Proj, *Proj2, *Datum, *Datum2, *Temp, *token; 	// For geographic mapping
double		adfGeoTransform[6], adfGeoTransform2[6];
double 		topleftX, transformX, topleftY, transformY; 	

int 	ch_in, ch_out, in_ch[10], segm_in, in_segm[10];		// For channels, filename...
int 	data_type;
char 	Description[256];
char 	Extension[5]; 	

int	by_lines=0, by_image=1;		// default is to read/write by image (faster), 
					// user can specify bylines (slower) if not enough memory to go by image


GDALRasterBand	*piBand, *piBand2, *poBand;		// For images
int 		Lines, Pixels, Channels;
float		xpixsz, ypixsz, pix_size; 
int64 		bmsize;
char 		**papszOptions = NULL;
int 		bylines_flag;
int 		imaFile_opened;

GDALDataset	*ima_in, *seg_in;

PixVal  *ttbuf, *dirmaskbuf, *nfmaskbuf;	// For bitmap buffers

int	xcg, ycg;								// For polygons in shape files



//FILE *Report;   /* To compensate for "faulty" Report variable from core1000.dll */



/***** main prog. ******/

int main(int argc, char *argv[])
{

//  local variables list

//FILE 	*idb_fp;
//char 	file[FILENAME];
int 	dbic[2], dirmask, nfmask, windsiz=3, sunang=0, shadowang, dbob;
int		dirmaskflag = 1, nfmaskflag = 1, shadowflag = 0;  // these masks are assumed to be used by default
int 	thrshad[2], thrshade, thrshadow;
char 	forest_type[10];
//char 	report[64];
//void 	*args[10];
//int 	argcnt[10];


int 	i, j, ii, jj, si, sj;
int 	blocks,segnum;
int64 	bitnum, bitnum2, iii;
int	count;
int 	xsize, ysize, channels;
int 	ofs, ioffs, joffs, sha_dist;
int 	sofs, swindsiz;
char  	*cptr;	// generic pointer to char
unsigned char  *ptr;	// generic pointer to 8bit bytes
char	chr[256];
float 	imgarea;
char 	answer[10];

int pix, max, imax, jmax;
PixVal bpix;

char	*pch;		// generic dummy pointer to chr array
char seg_history[56];
char seg_description[64], timedate[17], geosys[17], pix_units[9];
float ave_wid, rad_ang, pix_xsize, pix_ysize;
int TTcount=0;
int STTcount=0;
int segtype;
char segflag, segname[9];
long start, length;
double	topleftX,transformX,topleftY,transformY;

time_t rawtime;
struct tm * timeinfo;

char 		file_ima[250];
PixVal		*pafScanline;
char 		*Proj, *Proj2, *Datum, *Datum2, *Temp, *token;
double		adfGeoTransform[6], adfGeoTransform2[6];


char	*filename, *filename1, file_out[250], temp[250];
char	fullfilename[250], extension[10], extension2[10];
char	basefname[250], nfmaskfname[250], dirmaskfname[250],shortfilename[250];
int		basef_len;



GDALDataset	*ima_in, *bmp_in, *bmp_out, *seg_in;
GDALDataset	*vfol_bmp, *nf_bmp;


int 		PCI_File = 0;			// initial flags for image file type
int 		TIF_File = 0;
int			no_items, ch_no, no_ch;
int  		ithres, thres[2];
//void 		*Image, *Simage;
PixVal 		*Image, *Simage;

int 		argcount;		// counting input arguments as they are used

//  End of local variables list

/* assign parameter pointers to argumnets */

/* args[0] = (void *) file;
args[1] = (void *) dbic;
args[2] = (void *) &dirmask;
args[3] = (void *) &nfmask;
args[4] = (void *) &windsiz;
args[5] = (void *) thrshad;
args[6] = (void *) &sunang;
args[7] = (void *) forest_type;
args[8] = (void *) &dbob;
args[9] = (void *) report;
 */
/*************************************************************
	SETUP PCI ENVIRONMENT AND INPUT/OUPUT
*****************************************************************/

/* setup standard interface */
/* 
IMPStatus ( "FILE,ILLUMCH,DIRMASK,NFMASK,WINDSIZ,THRSHADE,SUNANG,FORTYPE,TTBITM,REPORT;",
            "C   ,I      ,I      ,I     ,I      ,I       ,I     ,C      ,I     ,C     ;", 
            "128 ,2      ,1      ,1     ,1      ,2       ,1     ,6      ,1     ,64    ;",
            "1   ,1      ,0      ,0     ,0      ,0       ,0     ,0      ,0     ,0     ;",
            "LATTOPS.","FORCE", argcnt, args, argc, argv );


//	To compensate for "faulty" Report variable from core1000.dll 

//if(EQUALN(report,"TERM",4)) {Report=stdout;}else{Report = fopen(report, "w");}

 */


// *************************************************************
//		SETUP GDAL ENVIRONMENT AND INPUT/OUPUT
// *****************************************************************

// Print Program Header 

	time (&rawtime);  timeinfo = localtime (&rawtime);
	printf("\n\t\tStarting %s (%s) at %s\n", PROG_NAME, VERSION, asctime(timeinfo));

// Registers for all types of files with GDAL

	GDALAllRegister(); 	

	//printf("\n\tPresent parameters are %s %s %s %s\n\n", argv[1], argv[2], argv[3], argv[4]);

	
//***************************
 
// Check input parameters on command line (i.e., agrv[*])

//***************************

// Check *First argument* on command line

	argcount = 1;
	
	//printf("1st argument - Illumination image full entry : %s \n\n", argv[1]);
	
	if (argv[argcount]== NULL) 
	  {
	  printf("\n\t **PROBLEM** with input image %s \n",argv[argcount]);
	  printf("Have an INPUT image as first argument on command line \n\n");  
	  
	  printf("\nUSAGE: lattops_g FILE,ILLUMCH NFMASK DIRMASK WINDSIZ THRSHADE SUNANG FORTYPE TTBITM\n");	  
	  printf("USAGE: lattops_g Main_File.ext,CH# bitmap bitmap windsiz thres1,thres2 SUNANG FORTYPE TT_fname\n");
	  printf("USAGE: lattops_g Main_File.ext CH,CH bitmap bitmap windsiz thres1,thres2 SUNANG FORTYPE TT_fname\n");
	  exit(-1);
	  }  


// Check if a channel number is attached to file name (following a comma)

	ch_no = 1;	dbic[0]=dbic[1]=0;	 // default value, if nothing changes
	
	strcpy(temp, argv[argcount]);  
	cptr = strtok(temp, " ,"); 				// go to comma (or space) and put a null there
	//printf(" Temp :  %s \n", temp);
	strcpy(file_ima, temp);					// get a clean file name devoid of comma related items
	//printf(" Filename :  %s \n", file_ima);
	
	cptr = strtok(NULL, " ,");			// go to comma (or space) and put a null there
	
	if(cptr == NULL)				// channel numbers are in next parameter on the cmd line
	  {
	  argcount++;			// next argument
	  //printf("Argument %d: %s \n\n", argcount, argv[argcount]);

	  strcpy(temp, argv[argcount]);
	  cptr = strtok(temp, " ,"); 				// go to comma (or space) and put a null there
	  }
	
// channel numbers are connected to file name
	
	if(cptr != NULL) ch_no = strtol(cptr,NULL, 10);		// change to integer
	dbic[0]= ch_no;				// for PCI code coompatibility

	if(cptr != NULL) 			// continue
	  {
	  cptr = strtok(NULL, " ,");			// next item
	  if(cptr != NULL) {ch_no = strtol(cptr,NULL, 10); dbic[1]= ch_no;}			
	  }
	
	printf("Image Filename :  %s \n", file_ima);
	
	printf("Main channel to use : %d \n", dbic[0]);
	if(dbic[1] != 0) printf("Secondary channel to use : %d \n", dbic[1]);


// Create SHORT file name (no dir) for display convenience and later, for output file description

	strcpy(shortfilename, argv[1]);
//	printf("Shortfilename :  %s \n", shortfilename);

	pch = strtok(shortfilename,"/");
	while(pch != NULL)
	  {
		//printf ("pch : %s\n",pch);
		strcpy(shortfilename, pch);
		pch = strtok(NULL, "/");
	  }		
	printf("Shortfilename:  %s \n", shortfilename);

	//exit(-1);				// for debugging
		
		
		
// 	Check input parameter  NFMASK (i.e., agrv[2])

	argcount++;			// next argument --- NFMASK 
	
	//printf("\nArgument %d: %s \n\n", argcount, argv[argcount]);
	
	if (argv[argcount]== NULL) 
	  {
	  printf("\n\t **PROBLEM** with input bitmap %s \n",argv[argcount]);
	  printf("Have an INPUT NFMASK bitmap as second argument on command line \n\n");  
	  
	  printf("\nUSAGE: lattops_g FILE,ILLUMCH NFMASK DIRMASK WINDSIZ THRSHADE SUNANG FORTYPE TTBITM\n");	  
	  printf("USAGE: lattops_g Main_File.ext,CH# bitmap bitmap windsiz thres1,thres2 SUNANG FORTYPE TT_fname\n");
	  exit(-1);
	  }  
	
	
	if ( EQUALN(argv[argcount],"-",1 ) || EQUALN(argv[argcount],"#",1 ) )
	  {
	  printf("\nArgument %d is \"-\" or \"#\", which implies no input NFMASK bitmap to \"consider\" \n",argcount); 
	  printf("\nThis implies that the regular \"tree top\" (local maxima) algorithm is run throughout\n");
	  printf("OR, if DIRMASK is present, the \"SHADOW_TT\"algorithm is run throughout the image\n");
	  nfmaskflag = 0;
	  }
	
	
// Check for a comma cause ",-1" implies NFMASK is a FOREST mask
	
	strcpy(temp, argv[argcount]);  
	cptr = strtok(temp, " ,"); 				// go to comma (or space) and put a null there	
	strcpy(nfmaskfname, temp);		// save the file name (save a clean file name)
//	printf(" NFMASK Filename (no extra):  %s \n", nfmaskfname);	


	cptr = strtok(NULL, " ,");			// continue

	if(cptr != NULL) 
	{
	nfmaskflag = strtol(cptr, NULL, 10);	

//	printf(" nfmaskflag :  %d \n", nfmaskflag);
//	printf(" FOREST MASK Filename :  %s \n", nfmaskfname);

	if (nfmaskflag != -1) 
	  {
	  printf("\n\t **PROBLEM** with NFMASK bitmap %s \n",argv[argcount]);
	  printf("Only a -1 is acceptable here to signal a FORMASK instead of a NFMASK \n\n");  
	  
	  printf("\nUSAGE: lattops_g FILE,ILLUMCH NFMASK,-1 DIRMASK WINDSIZ THRSHADE SUNANG FORTYPE TTBITM\n");	  
	  printf("USAGE: lattops_g Main_File.ext,CH# bitmap bitmap windsiz thres1,thres2 SUNANG FORTYPE TT_fname\n");
	  exit(-1);
	  }  
	  
	printf("\n Second tif file < %s > now considered a FOREST MASK due to a ',-1' \n", nfmaskfname);
	}

	//exit(-1);				// for debugging
		
	
	
	
// Check input parameter  DIRMASK (i.e., agrv[3])

	argcount++;			// next argument
	//printf("\nArgument %d: %s \n\n", argcount, argv[argcount]);

	if (argv[argcount]== NULL) 
	  {
	  printf("\n\t **PROBLEM** with DIRMASK input bitmap %s \n",argv[argcount]);
	  printf("\t Have an INPUT bitmap as argument %d on command line \n\n",argcount);  
	  
	  printf("\nUSAGE: lattops_g FILE,ILLUMCH NFMASK DIRMASK WINDSIZ THRSHADE SUNANG FORTYPE TTBITM\n");	  
	  printf("USAGE: lattops_g Main_File.ext,CH# bitmap bitmap windsiz thres1,thres2 SUNANG FORTYPE TT_fname\n");
	  exit(-1);
	  }  
	
	if ( EQUALN(argv[argcount],"-",1 ) || EQUALN(argv[argcount],"#",1 ) )
	  {
	  printf("\nArgument %d is \"-\" or \"#\", which implies no input DIRMASK bitmap to \"consider\" \n",argcount); 
	  printf("***** This implies that the \"regular TREETOPS\" (local maxima) algorithm is run throughout\n");
	  printf("the image (except non-forest areas (if specified)). Thus, SUNANG is not used at all.\n");
	  dirmaskflag = 0;
	  }	


	if ( EQUALN(argv[argcount],"1",1  ) )
	  {
	  printf("\nArgument %d is \"1\" , which implies that the whole image has tree-specific shadows \n",argcount); 
	  printf("except the non-forest areas (if specified)). SUNANG is necessary\n");	  
	  printf("This implies that the \"regular TREETOPS\" (local maxima) algorithm will not be used\n");
	  shadowflag = 1;		// analyse only for shadows, no plain TT mode 
	  dirmaskflag = 1;
	  }	

	strcpy(dirmaskfname, argv[argcount]);			// save the file name



	
	  
// Check input parameter WINDSIZ (i.e., agrv[4])
	argcount++;			// next argument
	//printf("\nArgument %d: %s \n\n", argcount, argv[argcount]);
	
	
	if (argv[argcount]== NULL) 
	  {
	  printf("\n\t **PROBLEM** with input window size  %s \n",argv[argcount]);
	  printf("Have an INPUT window size as argument %d on command line \n\n", argcount);  
	  
	  printf("\nUSAGE: lattops_g FILE,ILLUMCH NFMASK DIRMASK WINDSIZ THRSHADE SUNANG FORTYPE TTBITM\n");	  
	  printf("USAGE: lattops_g Main_File.ext,CH# bitmap bitmap windsiz thres1,thres2 SUNANG FORTYPE TT_fname\n");
	  exit(-1);
	  }  
	
	
	if ( EQUALN(argv[argcount],"-",1) || EQUALN(argv[argcount],"#",1) )
	  {
	  printf("\nArgument %d is \"-\" or \"#\", which implies no WINDSIZ given \n",argcount); 
	  printf("This implies that the window size will be 3. \nLocal maxima will be from a 3x3 window\n");
	  windsiz = 3;
	  }	
	
	else windsiz = strtol( argv[argcount],NULL, 10);		// Input window size (WINDSIZ)

	printf("\nLocal maxima will be from a %dx%d window\n", windsiz, windsiz);	
	
	//exit(-1);



// Check input parameter  THRSHADE (i.e., agrv[5])	
	
	argcount++;			// next argument
	//printf("\nArgument %d: %s \n\n", argcount, argv[argcount]);


	if (argv[argcount]== NULL) 
	  {
	  printf("\n\t **PROBLEM** with input shade threshold  %s \n",argv[argcount]);
	  printf("Have at least one INPUT THRSHADE as fifth argument on command line \n\n"); 
	  
	  printf("\nUSAGE: lattops_g FILE,ILLUMCH NFMASK DIRMASK WINDSIZ THRSHADE SUNANG FORTYPE TTBITM\n");	  
	  printf("USAGE: lattops_g Main_File.ext,CH# bitmap bitmap windsiz thres1,thres2 SUNANG FORTYPE TT_fname\n");
	  exit(-1);
	  }  
 

// Check if a second thrshold is given as "shadow" threshold  

// Check if two thresholds are used is needed (e.g., argv[5] = 15,66)

	strcpy(temp, argv[argcount]);
	cptr = strtok(temp, ",");

	ithres = 0;
	while(cptr != NULL) 
	  {
	  thres[ithres++]= strtol(cptr,NULL,10); 	  
	  cptr = strtok(NULL, ",");
	  }
	  
	if(ithres == 1) 
	  {
	  thrshade = thres[0]; 
	  printf("As per user, the shade threshold will be %d\n", thres[0]);
	  thrshadow = 1.3 * thrshade ;			// by default - heuristic
	  }
	  
	if(ithres == 2)
	  {
	  thrshade = thres[0]; printf("As per user, the shade threshold will be %d\n", thres[0]); 
	  thrshadow = thres[1]; printf("As per user, the shadow threshold will be %d\n", thres[1]);
	  }	

	if(ithres > 2) 
		{printf("\n\n ### ERROR -- Only two threshold entries are acceptable \n\n"); exit(-1);}

		  
	//exit(-1);		// useful when testing only input parameters


// Check input parameter SUNANG (i.e., agrv[6])

	argcount++;			// next argument
	//printf("\nArgument %d: %s \n\n", argcount, argv[argcount]);
	
	if (argv[argcount]== NULL) 
	  {
	  printf("\n\t **PROBLEM** with input SUNANG   %s \n", argv[argcount]);
	  printf("Have an INPUT SUNANG as sixth  argument on command line \n");  
	  printf("However, this could be \"-\" or \"#\", if SUNANG not needed (no DIRMASK to consider) \n");
	  
	  printf("\nUSAGE: lattops_g FILE,ILLUMCH NFMASK DIRMASK WINDSIZ THRSHADE SUNANG FORTYPE TTBITM\n");	  
	  printf("USAGE: lattops_g Main_File.ext,CH# bitmap bitmap windsiz thres1,thres2 SUNANG FORTYPE TT_fname\n");
	  exit(-1);
	  }  
	
	
	// if ( ( EQUALN(argv[argcount],"-",1) || EQUALN(argv[argcount],"#",1) ) &&
		// ! ( EQUALN(argv[3],"-",1 ) || EQUALN(argv[3],"#",1 ) ) )
	  // {
	  // printf("\n\t **PROBLEM** with input SUNANG   %s \n", argv[argcount]);	  
	  // printf("Have an INPUT SUNANG as sixth  argument on command line \n\n"); 
	  // printf("\nA SUNANG must be given to use with the directionalty mask (DIRMASK)\n");
	  // exit(-1);	  
	  // }	
	  
	sunang = strtol(argv[argcount], NULL, 10);		// Input  sun angle (SUNANG)
//	printf("As per user, sun angle was set to %d\n", sunang);

	
// Check input parameter FORTYPE (i.e., agrv[7])

	argcount++;			// next argument
	//printf("\nArgument %d: %s \n\n", argcount, argv[argcount]);
	
	if (argv[argcount]== NULL) 
	  {
	  printf("\n\t **PROBLEM** with input FORTYPE   %s \n", argv[argcount]);
	  printf("Have an INPUT FORTYPE as seventh argument on command line (MATURE, REGEN, BIG or PROMPT)\n");  
	  printf("However, this could be \"-\" or \"#\". Then, FORTYPE will default to MATURE \n");
	  
	  printf("\nUSAGE: lattops_g FILE,ILLUMCH NFMASK DIRMASK WINDSIZ THRSHADE SUNANG FORTYPE TTBITM\n");	  
	  printf("USAGE: lattops_g Main_File.ext,CH# bitmap bitmap windsiz thres1,thres2 SUNANG FORTYPE TT_fname\n");
	  exit(-1);
	  }	

	strcpy(forest_type,argv[argcount]);		// put into forest_type variable
	
	if ( EQUALN(argv[argcount],"-",1) || EQUALN(argv[argcount],"#",1) ) strcpy(forest_type, "MATURE"); // set the default

//	printf("As per user, forest type was set to %s\n", forest_type);	
	
	//exit(-1);		// useful when testing only input parameters
	
	
	
// Check on possible user given OUTPUT  file name
	
// Check if output file name given for Tree Tops bitmap (if not, default filename will be used)

	argcount++;			// next argument
	//printf("Argument %d: %s \n\n", argcount, argv[argcount]);

// IF this  argument is ArcGIS or ArcMap, need to create an output file name

	if ((strncmp("ArcGIS ",argv[argc-1],3) == 0) )  goto OUT_NAME; // create one, using base file name
	
	//upper_case(argv[argc-1]);
	//if ((strncmp(argv[argc-1],"ARCGIS ",3) == 0) )  goto OUT_NAME; // create one, using base file name	
	
	if ( (argv[argcount] == NULL)  || EQUALN(argv[argcount],"-",1) || EQUALN(argv[argcount],"#",1) ) goto OUT_NAME; // create one, using base file name
	
	strcpy(file_out, argv[argcount]);
	printf("File_out  :  %s \n", file_out); 
	goto IN_IMA;
	
	//exit(-1);

// If still here, then last parameter IS A filename 

//if (argc == 9) {strcpy(file_out,argv[8]); goto IN_IMA; }


//*******************

// Create base file name   
	
// Need to get "basefilename" when full path is involved
// Basefile name has path + head of file name (typically correspond to "named area" of study e.g. PRF)
// Area name is assumed separated from rest of file name by an underscore
// However, be careful as there could be underscores in the path

OUT_NAME:

	strcpy(fullfilename,argv[1]);			// main input filename (and possibly its dir)
	
	cptr = strtok(fullfilename, ","); 	 		// get rid of comma and item (channel) after comma

	for (ii=0; ii < strlen(fullfilename); ii++)		// search for last underscore position
	  {
	  jj = strlen(fullfilename) - ii;				// start from the end
	  //printf("Count back: %d",j);
	  if(fullfilename[jj] == '_') {basef_len = jj;	break;}	// find last underscore in full file name
	  }
	//printf("\nBase Filename Length:  %d \n", basef_len);
  	
	strncpy(basefname, fullfilename,  basef_len);		// get that part of  the full file name
	basefname[basef_len] = '\0';   					// make it a string to be safe
	
//	printf("\n\nBase file name ::  %s \n", basefname);	
	
//	Creeate default output file name, concatenate TTs to base file name (path + file name beginning)	

	strncat(basefname,"_TTs",4);
 	strncat(basefname,".tif",4);
	strcpy(file_out, basefname);
	
//	printf("File_out  :  %s \n", file_out); 
	//printf("\n Output Filename Length:  %zd \n", strlen(file_out));



 
// *************************************************
 
// Open input file(PCI or TIF) via GDAL and read image (Doing one or two channels as needed)

// *************************************************	

IN_IMA:

	//exit(-1);		// useful when testing only the input parameters

	
 	ima_in = open_imaFile(file_ima);			// uses itc_io_g functions

	printf("\n\t\t*Reading main channel to use : %d \n", dbic[0]);	
	
	Image =  (PixVal *) read_image(file_ima, dbic[0]);		// Read input image channel via itc_io_g.ccp, (pointer to image data)

	xsize = Pixels; ysize = Lines;

	//printf("\nMain File Extension : %s \n", Extension); 
		
// Extension is a global parameter set by Open_imaFile()
	
	if (EQUALN(Extension,"pix",3)) PCI_File = 1;	// everything is in the PCI file
	if (EQUALN(Extension,"tif",3)) TIF_File = 1;	// everything is in directory, mostly as tif files

	if (! ( PCI_File || TIF_File))
	  {
	  printf("\n\n**ERROR** Program not able to deal with image file of type %s\n\n", Extension);
	  exit(-1);
      }

	
// Check if a second channel number is attached to file name
// Typically, this a more smoothed version of same channel (or possibly a different band)
// to be used under the directionality mask when looking for tree shadows.

	if(dbic[1] != 0)
	  {	
	  printf("\n\t\t*Reading secondary channel to use : %d \n", dbic[1]);
	  Simage =  (PixVal *) read_image(file_ima, dbic[1]);	
	  }	
	else Simage = Image;	// if only one image use same image
	

//	Reserve memory for input and output bitmaps

//	printf("\nReserving memory for output bitmap\n");
//	printf("Bitmap Size in bytes : %lld \n", bmsize);
	
	ttbuf = (PixVal *) calloc(bmsize,sizeof(PixVal));		// prep output bitmap GDAL calloc -- zeros memory
	check_mem(ttbuf);	
	



// Read NFMASK (or forest mask) bitmap (function does the memory allocation)

	if(nfmaskflag == 1)
	{		
	  printf("\n\t\t*Reading non-forest mask to use\n");	
	  nfmaskbuf = read_bitmap(nfmaskfname,1);  //  read-in non-forest  bitmap
	  if(nfmaskbuf == NULL) 
	    {printf("\n\t**ERROR** Program not able to find non-forest mask  %s\n\n", argv[2]); exit(-1);}
	}
	if(nfmaskflag == 0)		// still need something 
	{
	nfmaskbuf = (PixVal *) calloc(bmsize,sizeof(PixVal));	// clear memory for non-forest mask bitmap 
	check_mem(nfmaskbuf); 
	}

	if(nfmaskflag == -1)			// dealing with a FOREST mask instead of non-forest mask
	{		
	 printf("\n\t\t*Reading FOREST mask to use\n");	
	 nfmaskbuf = read_bitmap(nfmaskfname,1);  //  read-in FOREST  bitmap
	 if(nfmaskbuf == NULL) 
	    {printf("\n\t**ERROR** Program not able to find forest mask  %s\n\n", argv[2]); exit(-1);}

	for ( i = 1 ; i <= Lines ; i ++ )			// make forest bitmap into a non-forest bitmap
	for ( j = 1 ; j <= Pixels ; j ++ ) 
	  {
	  bitnum = (int64)xsize*(i-1) + j-1;			// bitnum starts at zero 
	  flipbit(nfmaskbuf, bitnum );				// flip all the bit
	  }  
    }

	//exit(-1);		// useful when testing only the input parameters
	

	if( dirmaskflag && !shadowflag)
	{  
	  printf("\n\t\t*Reading directionatity mask to use\n");	
	  dirmaskbuf = read_bitmap(dirmaskfname,1);  //  read-in directionailty  bitmap 	
	  if(dirmaskbuf == NULL) 
	    {printf("\n\t**ERROR** Program not able to find directionatity mask  %s\n\n", argv[3]); exit(-1);}
	}
	else 		
	{
	dirmaskbuf = (PixVal *) calloc(bmsize,sizeof(PixVal));	//  clear memory for directionailty  bitmap 
	check_mem(dirmaskbuf);
	}

	if(shadowflag)			// whole image considered to have tree-specific shadows (except non-forested areas)
	{ 	
	ptr=dirmaskbuf;
	for (iii=0; iii<bmsize; iii ++) *ptr++ = 0xFF;			// set all bits in bitmap  		
	}
	
	
//	exit(-1);		// useful when testing only the input parameters
	

// Shadow angle and position calculation 

/* Program will check in a direction commensurate with sun angle (add 180o)
  (and "sha_dist" pixels away) for a tree shadow area (3x3)  */

/* calculate (or ask for) distance from TT to main part of shadow  */
/* 1.5, 3.0, 4.5 metres for regen, mature and big, respectively */

	if(dirmaskflag)
	{ 
	pix_size = ( fabs(xpixsz) + fabs(ypixsz) ) /2 ;

	upper_case(forest_type);
	if (strncmp(forest_type,"BIG",3) == 0) sha_dist = (int) (6.0 / pix_size + 0.5 );
	else if (strncmp(forest_type,"MATURE",3) == 0) sha_dist = (int) (4.0 / pix_size + 0.5 );
	else if (strncmp(forest_type,"REGEN",3) == 0) sha_dist = (int) (2.0 / pix_size + 0.5 );
	else	{
		fprintf(stdout, "\nEnter distance (in pixels) from treetop to shadow: ");
		if(fgets(chr,256,stdin))
			if(chr[0]) sscanf(chr,"%d",&sha_dist);
		fprintf(stdout, "\n");
		}
		
		
	/* For shadow angle (sun azimuth + 180o (shadow angle = sun rays angle)), 
	   and also coordinate reversal (between cartesian y  and image y). 
	   So, add 90o to sun azimuth.
	*/
	  
	shadowang = sunang + 90;	
	rad_ang = (shadowang * 2 * 3.14159) / 360;	 /* shadow angle in radians */

	ioffs = (int) (sha_dist * sin(rad_ang)+0.5);
	joffs = (int) (sha_dist * cos(rad_ang)+0.5);

	fprintf(stdout,"\nSun azimuthal angle supplied was %d degrees.\n", sunang);
	fprintf(stdout,"Shadow angle being used (under the dir. mask) is %d degrees (azimuthal).\n", 
						sunang + 180);
	fprintf(stdout,"Shadow look distances(P,L) being used are: %d %d\n", joffs, ioffs);

	}


//	exit(-1);		// useful when testing only the input parameters


   
/**********************************************************
	OPERATION ON CHANNEL
************************************************************/

WORK:

//  Image considered to have only tree-specific shadow areas (except non-forested areas)
if(shadowflag)	goto dir_loop;		// skip "plain" TREETOPS analysis

printf("\n******************************************************\n");
printf("\nImage processing loop for the densely forested areas ...\n"); 
printf("(forested areas not under directionality nor non-forest mask)\n\n");
	
/* default window size */

ofs = windsiz / 2 ;
//printf("\nSize of moving window being used: %dx%d\n",windsiz,windsiz);
//printf("\nSize of offset %d \n", ofs);
//printf("\nShade threshold %d \n", thrshade); 
//printf("\nShadow threshold %d \n", thrshadow); 

// Scan FULL IMAGE, but use "unmasked areas" only

for ( i = 1+ofs ; i <= Lines-ofs ; i ++ )	// image coord. start at (1,1)
for ( j = 1+ofs ; j <= Pixels-ofs ; j ++ ) 	// full image scan
  {
	
  //if ( (j == 5) && ((i/100)*100) == i) printf("Doing line %d \n", i);			// for occasional debuging
  //if ( (j == 5) && ((i/100)*100) == i)  printf("* \t", i);		// show progress with stars
	 
  bitnum = (int64) Pixels*(i-1) + j-1;			// bitnum starts at zero 
 
 
// Forget it doing it if masked out
  
  if ( ! ( testbit(dirmaskbuf,bitnum) || testbit(nfmaskbuf,bitnum) )  )   // Do it if not masked out 
    {

// Find maximum within roaming window, typically 3x3  (without considering masked pixels)

    max = imax = jmax = 0;		// reset max

    for ( ii = i-ofs ; ii <= i+ofs ; ii++ )			// look around at i,j
    for ( jj = j-ofs ; jj <= j+ofs ; jj++ ) 
	{
	bitnum2 = (int64) Pixels*(ii-1) + jj-1;  		// bitnum2 is image position around main position (bitnum)
	//pix = (*get_pix_val) (Image, bitnum2);	
	
	//bpix = (*get_pix_val) (Image, bitnum2);	
	//pix = (int) bpix;
	//pix = (int) (*get_pix_val) (Image, bitnum2);	
	//pix = abs( (*get_pix_val) (Image, bitnum2) );	
	//pix = (int) Image[bitnum2];					// that works well ????



// IF ANY LOCAL pixel (ofs x ofs) is under the mask, skip that local area completely

	if (testbit(nfmaskbuf,bitnum2)) goto SKIP;
	
	if (!testbit(nfmaskbuf,bitnum2)) 	// if pixel is under mask dont consider it for max detection
	 {
	 pix = (*get_pix_val) (Image, bitnum2);	
	 	
	//bpix = (*get_pix_val) (Image, bitnum2);	
	//pix = (int) bpix;
	 //pix = abs( (*get_pix_val) (Image, bitnum2) );	
	 //pix = (int) Image[bitnum2];	 		// that works well  ???
	 if (pix > max ) 
	  {
	  max = pix;
	  imax = ii;
	  jmax = jj;
	  }
	 } 
	}	// end of 3x3 window  LOCAL scan 
  
// Max is only interesting if at center of window, thus real local max.
 
 	 
    if ( (imax == i) && (jmax == j) && ( max > thrshade) ) {setbit(ttbuf, bitnum); TTcount++;}

	//  TTcount above for TESTING  various counts of pixels under mask	
	
	
	// Skip local 3x3, 5x5 scan if any pixel is under the mask (prevent boundary problems)
	
SKIP:  ; // This is a no-op
	
	   
     }	// in full image scan, skip it if masked out by NFMASK or DIRMASK
   
//	if ( (j == 5) && ( (i/1000) * 1000) == i)  printf("%d lines done\r", i);
//	 if ( (j == 5) && ( (i/1000) * 1000) == i)  printf("* \t", i);			// print stars are counters 
	 
  }			// end of for every pixel in image
  
  
printf("\n\t%d lines done\n", Lines);

/* Report results */

printf("\nFor the densely forested areas, %d tree tops (local maxima) were found.\n", TTcount);



	//exit(-1);		// useful when testing 


//******************************************

//		Second phase (if needed)

//*******************************************

dir_loop:

printf("\n******************************************************\n");
printf("\nPhase II ...\n");

if(!dirmaskflag) 
	{ 
	printf("\nLooking for TTs with specific shadows in more open areas was NOT called for ...\n");
	goto WRITE_IMAGE;
	}

printf("\nLooking for TT with specific shadows in the more open forested areas ...\n"); 
printf("(i.e., areas under the directionality mask, sparsely forested)\n\n");

//IMPCounter(-1.0);

// Taking into consideration ioffs + sofs (i.e. a 3x3 blob at ioffs,joffs from max position)

for ( i = 1+12; i <= Lines-12 ; i ++ )		/* image coord. start at (1,1) */
for ( j = 1+12 ; j <= Pixels-12 ; j ++ ) 
  {
  bitnum = (int64)Pixels*(i-1) + j-1;			/* BM starts at zero */
  
  if ( (j == 20) && ((i/100)*100) == i)  printf("* \t", i);				// show progress with stars
	
  //Under directionality mask but, forget it if masked out by non-forest mask

  if ( testbit(dirmaskbuf, bitnum)  && (!testbit(nfmaskbuf,bitnum)) )
   {

//	Find MAXIMIM within roaming window  (without NF masked pixels) 

   max = imax = jmax = 0;
   
    for ( ii = i-ofs ; ii <= i+ofs ; ii++ )
    for ( jj = j-ofs ; jj <= j+ofs ; jj++ ) 
	{
	bitnum2 = (int64)Pixels*(ii-1) + jj-1;  	
	if (!testbit(nfmaskbuf,bitnum2)) 	// forget it if part of non-forest mask
	 {
	 pix = (*get_pix_val) (Simage, bitnum2);	
	 if (pix > max ) 
	  {
	  max = pix;
	  imax = ii;
	  jmax = jj;
	  }
	 }
	}


/* Max is only interesting if at center of window, thus real local max. */

    if ( (imax == i)&& (jmax == j) )
     {

/* check in direction commensurate with sun angle (add 180o)
  (and "sha_dist" pixels away) for a tree shadow area (3*3)  */

     si = i + ioffs;
     sj = j + joffs;

/* check if at least four(4) shadow pixels in the 3x3 neighbourhood */
/* when shadow distance less or eq 3 pixels, only look for two shadow pixels in 3x3 */

     swindsiz = 3;	
     sofs = swindsiz / 2 ;
     count = 0;
     for ( ii = si-sofs ; ii <= si+sofs ; ii++ )
     for ( jj = sj-sofs ; jj <= sj+sofs ; jj++ ) 
	{
	bitnum2 = (int64)xsize*(ii-1) + jj-1;  		
	if (!testbit(nfmaskbuf,bitnum2))	 // forget it if part of non-forest mask
	 {
	 pix = (*get_pix_val) (Simage, bitnum2);
	 if (pix < thrshadow) count++;
	 }
	}

/* keep only as max. if significant shadow (>4 pixels) in shadow dir. */

     if ( (count >= 4) || ((count >= 2) && (sha_dist <= 3)) )
	 {
     //pixel must still be greater than shadow threshold	 
	 if (max > thrshadow) {setbit(ttbuf,bitnum); STTcount++;}
	 } 

    } 			/* end of found max at center of window */


   }	/* end of if directionaity and not non-forest mask */
   
  }	/* end of for every pixel in image */
 

printf("\n\t%d lines done\n", Lines);

/* Report results */

printf("\nFor the more open areas, %d tree tops (local maxima) were found.\n", STTcount);

printf("\nOver the whole forested areas, a total of %d tree tops were found.\n\n\n", TTcount+STTcount);

//	exit(-1);		// useful when testing 


//******************************************************

WRITE_IMAGE:

printf("\n******************************************************\n");

// Write resulting bitmap to output file

	printf("\nWriting resulting tree tops to output bitmap file %s \n\n", file_out);

if(!dirmaskflag)
	sprintf(Description,"LATTOPS - Plain Tree Tops(%dx%d), thresholds(%d) from %s",windsiz,windsiz,thrshade,shortfilename);
if(dirmaskflag)
	sprintf(Description,"LATTOPS - Tree tops(%dx%d), with and without ind. shadows, thresholds(%d,%d) from %s",windsiz,windsiz,thrshade,thrshadow,shortfilename);
if(shadowflag)
	sprintf(Description,"LATTOPS - Tree tops(%dx%d), with individual shadows, thresholds(%d) from %s",windsiz,windsiz,thrshadow,shortfilename);
	
	write_bitmap(ttbuf, file_out);		//was bitmap_out(maskbitbuf, file_out);
	
	printf( "\nDescription within output file : %s\n", Description);
		
		
// Print Program Trailer 

	time (&rawtime);  timeinfo = localtime (&rawtime);
	printf("\n\t\t%s (%s) finished at %s\n", PROG_NAME, VERSION, asctime(timeinfo));


// For people using this program via ArcGIS, give then some time to examine the results (before disappearing)

if ((strncmp("ArcGIS ",argv[argc-1],3) == 0) )		// if last argument is ArcGIS or ArcMap
	{
	fprintf(stdout,"\n\n######\n");
	printf("\n Type anything to make this detailed window disappear and terminate %s ",PROG_NAME);
	answer[0] = getc(stdin); 		// gets any answer or <CR>
    }
	

exit(0);				// exit properly 

}		/* END OF MAIN */

/**********************************************************************/


/* This function converts a string to upper case characters */

void upper_case(char *c)
{
	while (*c != '\0') {
		if (*c >= 'a' && *c <= 'z')
			*c += ('A' - 'a');
		c++;
	}
}
