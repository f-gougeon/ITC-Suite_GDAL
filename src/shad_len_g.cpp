/* 

SHAD_LEN - To find length of tree shadows in sparsely treed areas

Program name: 	shad_len_g.cpp
Author: 	François A. Gougeon
Date:		July 2025 (GDAL version)


	François A. Gougeon, Ph.D.
	Remote Sensing Research	
	(©)Natural Resources Canada
	Canadian Forest Service 
	Pacific Forestry Centre
	506 West Burnside Rd.
	Victoria, British Columbia, 
	Canada, V8Z 1M5	

Description 

This is directed at the sparse forested areas in northern Canada (e.g. NWT, Yukon)
where individual trees and their rather specific shadows are often visible.

From a bitmap showing tree shadows (often acquired by simple thresholding),
this program generates a bitmap containing some shadow length as detected within
a certain range of lengths, using lower and upper thresholds (LENG_THR in metres)
and certain range of widths (BLOB_WID in cm).

Using the above parameters, one can hopefully bypass irrelevant blobs or blobs that
are a combination of multiple shadows in the X or Y directions

It is able to report for the full image or for a certain Test Area (maybe a tif bitmap).

** MORE IMPORTANTLY **, it is able to report the average shadow length within the 
polygons (stands) of the test area (if TESTSEG is such)

<h2>Notes</h2>
The original shadow bitmap, if obtained by simple thresholding, should try to avoid
water bodies and large shaded areas (e.g., darken mountain sides).

The intent here, is to infer tree heights from shadow lengths. With a sun elevation of
about 45 degrees in Canada's Northern areas, shadow lengths could correspond to 
tree heights.

Info about shadow lengths in a given area (a stand) could provide info about 
whether we are dealing with mature trees or regeneration or severely stunted trees.
This could direct the use of MATURE vs REGEN flag in program like LATTOPS.

Parameters

SHAD_LEN is controlled by the following parameters:

SHAD_BM		BM-In of crude tree shadows			
SHAD_LEN	BM-Out of measured shadow lengths	
TESTSEG		Test Area Segments (BM/Vect)		
LENG_THR	Min & Max length (in metres) to include	
BLOB_WID	Min & Max blob width (in cm) to include	
SUNANG		Sun azimuth and elevation		

Program Usage

> shad_len_g SHAD_BM SHAD_LEN TESTSEG LENG_THR(m) BLOB_WID(cm) SUNANG
	  
> shad_len_g Input_BM.tif Out_Bitmap.tif - 2,15 50,400 - 		// does FULL image (SUNANG at 180,45)

> shad_len_g PP784481_Shadows.tif PP784481_Length.tif Test_Area.tif  - - - -

> shad_len_g PP784481_Shadows.tif PP784481_Length.tif PP784481_Segm26.shp  - - - -



c
c	SUNANG		Sun azimuth and elevation
c
C	EASI>SUNANG=n,m  
c
c
c	Given the image acquisition location, date, and time, you can get the sun'azimuth 
c	and elevation from the NOAA Apps (or other) at :
c
c	https://gml.noaa.gov/grad/solcalc/azel.html
c
c	This is required to get proper tree heights from their specific shadows.
C
c	If not known (i.e., no image acquisition info), sun azimuth can be measured on
c	the image looking at shadows (with a reporter or guessing) and 45 degree elevation
c	can be used (typical of such northern latitude) 
C	
C	Importantly, sun azimuth is used to look for shadows in the proper direction, thus
c	maximizing the chances of getting the end point (corresponding the tree tree top)
c

C	
c
c	*********************************
c
C	Revision History:
c
c
c v1.0a	Dec 2019 	François Gougeon
c
c	- Modified from CWD_SFIL.cpp
c
c v1.1a	Dec 2019 	François Gougeon
c
c	- To report for many forest stands as polygons in a vector segment (TESTSEG)
c
c
c v1.2a	Feb2019 	François Gougeon
c
c	PHASE I - From bitmap of tree shadows, goes down objects and get numerous vertical lengths 
c	from each, making sure they are within LEN_THR=n,m. In any case, lengths smaller than 2m are dropped,
c	producing an intermediate bitmap for Phase II.
c
c	PHASE II
c	Now that small shadow blobs (< 2m in HT) have been removed and that serious shadow blobs 
c	have been trim to their essential vertical components (removing 2m components), 
c	pickups a single height (longest line) for that blob then, write it to output bitmap
c	and summarize that for each stand polygon (also HT histogram of stand)
c
c
c v1.3a	Feb2019 	François Gougeon
c
C	Introduced BLOB_WID to eliminate shadow blob that are obviously too big
c	and meaningless, often in areas where there is shade rather than specific shadows
c1	often due to tree clusters
C
C	Range of blob width (in cm) to consider in analysis  
C
C	EASI>BLOB_WID=n,m    | keeps blobs with width between n and m cm
c
C
c
c
c v1.4a		July-Sept 2021	 	François Gougeon
c
c			- Mods. for a new "ana_hist()" passing back info about crude stand histogram (via a structure)
c				ana_hist() is in hist.cpp of general ITC Suite use (maybe I should make a special one here)
c
c
c v1.5a		Fevrier 2022	 	François Gougeon
c
c			- Using  ana_hist() to report on MEAN above 90 % of height within Stand (polygon)
c			- Using  ana_hist() to report on count of height (i.e., sample size) within Stand (polygon)
c			
c
c v1.6a		October 2022	 	François Gougeon
c
c			- Using  ana_hist() to report on standard dev (HT_St_Dev) of height within Stand (polygon)
c			- Using  ana_hist() to report on MODE of height within Stand (polygon)
c
c
c
c v1.7a		March 2025	 	François Gougeon
c 
c1			- Introduced SUNANG parameter(azimuth and elevation)
c
c1			- Previous versions (for simplicity sake) were assuming SUN from the south (180 degrees)
c1			  with an elevation of 45 degree (often the case in Northern regions), thus shadow length
c			  was equivalent to tree height.   Was measuring vertical length of shadows.
c 
c 			- Major mod to deal with shadows pointing in other directions (as per new SUNANG)
c
c
c
c v1.8a		July-Aug 2025	 	François Gougeon
c
c 
			- Migrated to GDAL from PCI version (inspired by itcpcd_g.cpp)

			- Still playing with ana_hist(). Do we want top 90 or 95% heights 
				(histo are sometimes very sparse)
			
			- Reintroduces "Report" (internal and somehow different from PCI REPORT),
				which can be used to print more stuff in debugging mode.
C
c v1.9a		Sept. 2025	 	François Gougeon

			- Minor bug fixing and cleaning 
			
			- If output file already exist, ask what to do	
			
			- Reintroduced histogram printout(Histograms.txt)
			
			- Fixed write_stem_generic() to write on top of blob
c
c 
c
c	*********************************
c	Possible Linking information
c
c
c #	For PCI 10.1  (with MinGW gcc)
c 
c	gcc  -I"C:\pcisdk_v101\lib" -I"C:\ITC-Sources"  -o logsfil itc_io.c error.c logsfil.c -L 
c	"C:\Program c	Files (x86)\PCI Geomatics\Geomatica_V101\exe" -lpcic1010  -lcore1010 -lcounter1010
c
c
c
c	To Set up Visual Studio 2015 (v14) environment variables (for 64 bit versions)
c	and to compile for 64 bit program using 64bit pointers
c
c	call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall" amd64
c
c	set CL= /MD /W3 /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fo"%RELEASE_OBJDIR%\\" /FD
c	set CL=%CL% /D "_CRT_SECURE_NO_DEPRECATE" /D "_CRT_NONSTDC_NO_DEPRECATE" /wd4996 /wd4244 /wd4101
c
c	set LINK=pcic201500.lib core201500.lib counter201500.lib  /subsystem:console /incremental:no /machine:x64 /DEFAULTLIB:MSVCRT
c
c	*********************************

REMM	 More modern compiling info (see also ITC-SuiteREL_V2022.txt)

REM		For Visual Studio 2022   v17.5(x64) (on Ciara) (on DELL Laptop)

call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall" amd64

REM	Normal link parameters 

set LINK=pcic222200.lib core222200.lib counter222200.lib  /subsystem:console /incremental:no /machine:x64 /DEFAULTLIB:MSVCRT /STACK:0X200000



cl %SRCDIR%\hist.c /c /TP

cl %SRCDIR%\shad_len.cpp error.obj bitops.obj itc_io.obj hist.obj /link /out:"%RELEASE_DIR%\shad_len.exe"


REM		Compiling GDAL Version

CL %SRCDIR%\shad_len_g.cpp  itc_io_g.obj bitops.obj  hist_g.obj error.obj /EHsc


*/


#define DEBUG_ME 1
#define NO_SKIP 1
#define SKIP 1

#define PROG_NAME "SHAD_LEN_G"
#define VERSION "v1.9a"

#define MAX_INTERCEPT 100

/* 
extern "C" {
 #include "pci.h"
 #include "itc_io.h"
 #include "error.h"
 #include "bitops.h"
 #include "hist.h"
}
 */
 
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <time.h>       // time_t, struct tm, time, localtime 
#include <math.h>


#include "gdal_priv.h"		// For GDAL library
#include "ogrsf_frmts.h"	// For OGR

#include "ITC-Suite_g.h"		// some definitions 
#include "itc_io_g.h"		// my newest GDAL image/bitmap input/output
#include "bitops.h"		// bit operations on bitmaps (mostly macros to be faster)
#include "hist_g.h"

#include "ITC-Suite_GDAL_IO.h"	// some GDAL variable setup

#define FILENAME    128
#define pi 3.14159
#define BITMAPS     	16


/* structure - list of tree crown encountered and parameters */
/* In LOGSFIL, crown are objects like logs, stumps, ... */
/* In SHAD_LEN crown are objects like tree shadows */

typedef struct crown_info 		// tree shadow info
	{
	int area;			/* object  area */
	int xmin;			/* diam in both directions*/
	int xmax;
	int ymin;
	int ymax;
	float UTM_East;			/* UTM centroid of object */
	float UTM_North;
	float length;			/* object length (m) */
	float width;			/* object width (cm) */
	float volume;			/* object volume */
	int removed;			/* object removed flag */
	struct crown_info *link;
	} crown;

/*
// Structure to transfer info extracted from histogram (** NOW ** in hist_g.h)

typedef struct histo_info 
	{
	int samples;		// number of samples in histogram
	int first;			// first bin of interest
	int last;			// last bin of interest
	int range;			// range of useful value
	int m_count;		// count at mode
	int mode;			//mode position in histogram
	int min_count;		//count threshold to remove not so well populated bins
	float mean;			// crude mean as gathered from histogram
	float st_dev;		// crude standard deviation as gathered from histogram
	int ipos95;			// position of 90% 
	float mean95;		// mean of values in top 90% of histogram
	} hist_inf;
*/

/* function declarations */

void fill_log(int, int, crown *);
void fill_blob(int, int, crown *, unsigned char *);
void fill_8c_blob(int, int, crown *, unsigned char *);
void fill_vert(int, int, crown *, unsigned char *, unsigned char *);

void write_stem_vert(crown *, unsigned char *);
void write_stem_generic(crown *, unsigned char *);

void erase_log(int, int, crown *);
void erase_blob(int, int, crown *, unsigned char *);
void erase_vert(int, int, crown *, unsigned char *);
void erase_8c_blob(int, int, crown *);
void erase_some_input_blob(int, int, crown *);
void init_rec(crown *);

// PCI version
//void get_shadow_lengths(unsigned char *, GDBLayer, GDBShapeId);
//void prep_fields(GDBLayer);
//void paint_plot_ana(GDBLayer);
//void vect2rast(int, GDBVertex *, unsigned char *, int, GDBShapeId, GDBLayer);

void get_shadow_lengths(unsigned char *, OGRLayer *, OGRFeature *);
//void prep_fields(OGRLayer);

void vect2rast(int, GDBVertex *, unsigned char *, int, OGRFeature, OGRLayer);

int  between(float a, float b, float c);

// For each new field, create it, and populate output layer with PCD-generated data

void	Copy_Shape_File(char *, char *);	//  Copy Input polygon file info to output shape file
void 	Prep_New_Fields(OGRLayer *);		//// create all the needed PCD fields in the output shape file



void Paint_Plot_Ana_g(OGRLayer *);	// paint each polygon to testbitbuf and analyse its content


// Initialize structure about histogram information to transfer back to main
//void init_histo_struct(hist_inf *);		// NOW declared in hist_g.h


/* global vars list */

//int Lines, Pixels, Channels;			// now define in ITC-Suite_GDAL_IO.h
//int 	bmsize;
int		blocks;
crown *List = NULL;
unsigned char *isolbitbuf, *outbitbuf, *midbitbuf, *testbitbuf, *tempbitbuf;
//unsigned char *bitbuf;		// not a real bitbuf, just a pointer
char answer[10];

int	vecmode = 0;			//flag on when dealing with vectors rather than bitmap	

//double 	topleftX, transformX, topleftY, transformY, botrightX, botrightY;
//float	xpixsz, ypixsz, xcg_adjust, ycg_adjust;

GDBVertex *tmpVertices;
int	RingStart[24], RingField, RingStartCount;	/* at max., 24 sections to a shape */
int 	RingFlag2;		/* Flag on if rings are present within this particular shape */ 
int 	North;
//int	xcg, ycg;
int	p_maxlen, no_stands=0, iarea=0, prev_xpos;

int 	t_crown = 0, e_crown = 0, s_crown = 0;
int 	t_blob = 0, e_blob = 0, s_blob = 0; 
int	tot_s_crown = 0;


float	x_length, y_length, log_length, tot_length=0.0, tot_tot_length=0.0, prev_length=0.0;
float 	tot_mode = 0.0, tot_mean95 = 0.0;

int	leng_thr[2];
int	blob_wid[2];

crown *p1; 	// pointer to object to store vertical line segments 

hist_inf *ph; // pointer to structure to store info extracted from histogram by ana_hist()

//FILE *idb_fp;	// pointer for PCI file


FILE   *shadows_fp, *histo_fp, *diam_60Plus_fp;

FILE *Report; 		// used to print (or not) some debugging info

int 	sunang[2];
int		xsize, ysize, channels;
int 	count = 0;
char  	*cptr;	// generic pointer to char


int 	segnum;
int 	segtype;
int     testsegI[BITMAPS];

int 	NumberOfInnerRings, NumberOfInteriorRingVertices, NumberOfExteriorRingVertices;
int 	Poly_Out_Count =0;
int 	polyarea;

int		basef_len;	
char	 *file_out, *Fname;		// just pointers
char	fullfilename[250], fullfileout[250], shortfilename[250], testfilename[250];
char	basefilname[250], shapefileout[250];
char	basefname[250];	

//GDALDriver *poDriver;
//char **papszOptions = NULL;

int PhaseI_bypass = 0;		// flag to bypass PhaseI processing

/****************************** main prog. ********************************/

int main(int argc, char *argv[])
{

/* local vars list */

char 	file[FILENAME];
int 	dbib, dbob, iLayer;


char	ans[10], temp_s[10], extension[3];
void 	*args[8];
int 	argcnt[8];

int		i, j, ii, jj;

char * cptr;

int newoutBM = 0;

int	lsizthr,hsizthr;
char	segflag, segname[9];
char    seg_history[81], seg_desc[81], geosys[17];
long 	start, seg_len;
char 	pix_units[9], timedate[17];



//Report = fopen("NUL", "w");			// to get rid of lots of printing
Report = stdout;	  				// to all the debugging info

 PhaseI_bypass = 1;		// flag to bypass PhaseI processing

// pointer to object to store vertical line segments
p1 = (crown *) malloc(sizeof(crown)); 

// pointer to structure to store info extracted from histogram by ana_hist()
ph = (hist_inf *) malloc(sizeof(hist_inf)); 


// ProjInfo_t	pProj;				// PCI definition
//GDBLayer 	pLayer, dLayer=0;		// dLayer is a dummy layer
//GDBShapeId	dShapeId=0;		// dShapeID is a dummy shapeID


// assign parameter pointers to argumnets 

args[0] = (void *) file;
args[1] = (void *) &dbib;
args[2] = (void *) &dbob;
args[3] = (void *) testsegI;
args[4] = (void *) leng_thr;
args[5] = (void *) blob_wid;
args[6] = (void *) &sunang;
//args[7] = (void *) report;


/* 
// Setup standard PCI interface 

IMPStatus ( "FILE,SHAD_BM,SHAD_LEN,TESTSEG,LENG_THR,BLOB_WID,SUNANG, REPORT;",
            "C   ,I      ,I       ,I       ,I      ,I      ,I 		,C;", 
            "128 ,1      ,1       ,1       ,2      ,2      ,2 		,128;",
            "1   ,1      ,0       ,0       ,0      ,0      ,0		,0;",
            "SHAD_LEN.","FORCE", argcnt, args, argc, argv );

// To compensate for "faulty" Report variable from core1000.dll

if(EQUALN(report,"TERM",4)) {Report=stdout;}else{Report = fopen(report, "w");}


//**********************************************************************

// Print input file information 

//********************************************************************

// Print PCI Header 

IMPTime(timedate,4);
fprintf(stdout,"\n\t\tStarting %s (%s) at %s\n", PROG_NAME, VERSION, timedate);
if (Report != stdout)
  fprintf(Report,"\n\t\tStarting %s (%s) at %s\n", PROG_NAME, VERSION, timedate);

// open database file - test if file name exists

ALLRegister();

 */



// Print GDAL prog Header 
 

	time (&rawtime);
	timeinfo = localtime (&rawtime);
	printf("\n\t\tStarting %s (%s) at %s\n", PROG_NAME, VERSION, asctime(timeinfo));

// Registers for all types of files with GDAL

	GDALAllRegister(); 	

// *******************************************

// Check arguments on command line 	

// Check *First argument* on command line (here, Input file name)

	if ( (argc < 6) || (argv[1]== NULL) )
	  {
	  printf("\n\t *** PROBLEM with arguments on the command line\n");
	  printf("\tThe following are the parameters for shad_len_g : \n\n");
	  printf("\tshad_len_g SHAD_BM SHAD_LEN TESTSEG LENG_THR(m) BLOB_WID(cm) SUNANG\n\n");
	  
	  printf("Typical: shad_len_g Shadows_BM.tif Lengths_BM.tif - 2,15 50,400 - \n\n");
	  
	  printf("Typical: shad_len_g PP784481_Shadows.tif PP784481_Lengths.tif PP784481_For_Inv.shp 3,15 50,300 180,45 \n\n");
	  exit(1);
	  }   

// Create base file name   (for output file name)
	
// Need to get "basefilename" when full path is involved
// Basefile name has path + head of file name (typically correspond to "named area" of study e.g. PRF)
// Area name is assumed separated from rest of file name by an underscore
// However, be careful as there could be underscores in the path

	strcpy(fullfilename,argv[1]);			// main input filename (and possibly its dir)

	printf("\n** Input Filename as stated:  %s \n", fullfilename);
	
	cptr = strtok(fullfilename, ","); 	 		// get rid of comma and item (channel) after comma

	for (ii=0; ii < strlen(fullfilename); ii++)		// search for last underscore position
	  {
	  jj = strlen(fullfilename) - ii;				// start from the end
	  //printf("Count back: %d",j);
	  if(fullfilename[jj] == '_') {basef_len = jj;	break;}	// find last underscore in full file name
	  }
	//printf("\nBase Filename Length:  %d \n", basef_len);
  	
	strncpy(basefilname, fullfilename,  basef_len);		// get that part of  the full file name
	basefilname[basef_len] = '\0';   					// make it a string to be safe
	
	printf("\n\t Base file name ::  %s \n", basefilname);	


	//exit(-1);			// for debugging


// Create SHORT file name (no dir) for display convenience and later, for output file description

	strcpy(shortfilename, argv[1]);
//	printf("Shortfilename :  %s \n", shortfilename);

	cptr = strtok(shortfilename,"/\\");
	while(cptr != NULL)
	  {
		//printf ("cptr : %s\n",cptr);
		strcpy(shortfilename, cptr);
		cptr = strtok(NULL, "/\\");
	  }		
	printf("\t Shortfilename:  %s \n", shortfilename);


	printf("\n\t\t*Opening file of input bitmap to use ...\n");	
	
 	ima_in = open_imaFile(fullfilename);			// uses itc_io_g function to open image (bitmap) file (a GDALDataset pointer)

	xsize = Pixels; ysize = Lines;	

	printf("\n xsize=%d ysize=%d Pixels=%d Lines=%d \n",xsize, ysize, Pixels, Lines);

// Checking on output bitmap (shadow length)

if ( EQUALN(argv[2],"-",1 ) || EQUALN(argv[2],"#",1 ) )		// need to CREATE an output file name
	{
	//basefname = strtok(fullfilename,".");
	//basefname = strtok(basefname,"_");				
	//printf("basefname  :  %s \n", basefname);	
	
	printf("\nOutput file argument is \"-\" or \"#\", which implies no output filename was given\n");
	printf("A new output image file name will be created from the \"base file name\" \n\n"); 
	//basefname = temp;	 				// fake to init basefname as a char array (just a pointer)
	strcpy(basefname, basefilname);	
	
	file_out = strncat(basefname,"_Length",7);	  	
	//itoa(ch_in,ach_in,10); 	 
	//sprintf(ach_in, "%d" , ch_in);	 	
	//strncat(file_out,ach_in,3);
	strncat(file_out,".tif",4);					// output image is forced to be a tif	
	strcpy(fullfileout, file_out);
	printf("\nOutput image file will be named \"%s\" \n\n", fullfileout);	
	}

else								// if  input bitmap, just add to that name
	{
	//basefname = strtok(fullfilename,".");		// fake to init basefname
	//strcpy(basefname,argv[argcount]);
	//basefname = strtok(basefname,".");
	//file_out = temp;				// fake to init file_out as char array
	//strcpy(file_out,argv[2]);	
	//strcpy(fullfileout, file_out);
	strcpy(fullfileout, argv[2]);
	printf("\n** Output Filename as stated:  %s \n\n", fullfileout);	
	}


// Check if OUTPUT file already exist (if so, ask to overwrite)

	ima_out = (GDALDataset *) GDALOpen( fullfileout, GA_Update );

	if (ima_out != NULL) 
	  {
	  printf("\n\n ### Output file %s already exist \n", fullfileout); 
	  printf("\n\t OK to overwrite FULL image file (Y/N)? \t");  fgets(ans,80,stdin);

	  if (ans[0] == 'n' || ans[0] == 'N')	  exit(1);		//dont overwrite the output file
 
	  printf("\n ### Existing OUTPUT file %s will be overwriten\n\n", fullfileout);
	  	    
	  }


fprintf(stdout,"\nReading input bitmaps AND allocating memory for output bitmap\n");

	if(segm_in == 0) segm_in = 1;
	isolbitbuf = read_bitmap(fullfilename, segm_in);			// always 1 for tiff  file bitmap, different for PCI files 
	safety_zone(isolbitbuf);
 
	printf("\nGetting memory for output bitmaps:\n");
	
	bmsize =  ((Pixels*(int64)Lines+ 7) / 8);    // size of bitmaps in byte
	outbitbuf = (PixVal *) calloc(bmsize,1);		// allocate (and zero) memory for an output bitmap 
	check_mem(outbitbuf);
	safety_zone(outbitbuf);

	
	//exit(-1);		// for degugging



// Allocate memory for some intermediate bitmap buffers

	printf("\nGetting memory for some intermediate bitmaps:\n");
	
	midbitbuf = (PixVal *) calloc(bmsize,1);		// allocate (and zero) memory for an output bitmap 
	check_mem(midbitbuf);
	safety_zone(midbitbuf);

	tempbitbuf = (PixVal *) calloc(bmsize,1);		// allocate (and zero) memory for an output bitmap 
	check_mem(tempbitbuf);	
	safety_zone(tempbitbuf);
	
	
// Check if forest stands (polygons) mentionned to write results to
	
	
if ( EQUALN(argv[3],"-",1 ) || EQUALN(argv[3],"#",1 ) )		// no polygon layer to write info to
	{
	printf("\n\t**No polygon layer (forest stands) specified to write to\n");
	printf("\n\t**You will get global results and details in a plain text file\n\n");
	goto More_Check;
	}
	

// If testing area was used, check if is it a bitmap or a shape file

	printf("\nChecking file extension of testing area \n\n");

	strcpy(testfilename,argv[3]);			
	
	printf("\tFile name of testing area \"%s\"  \n", testfilename);	
	
	cptr = strtok(testfilename,".");	
	cptr = strtok(NULL,", ");			// goto comma or space
	if(cptr == NULL) { printf("\nTesting Area needs a file extension \n\n"); exit(1); }
	strncpy(extension, cptr, 3);
	printf("\tFile extension of testing area %s\n\n", extension);
	
	if (EQUALN(extension,"tif",3)) segtype = SEG_BIT;
	if (EQUALN(extension,"shp",3)) segtype = SEG_VEC;	
	
// For shape file, create an output shape file (not to mess-up original forest inventory)
 	
	if (EQUALN(extension,"shp",3))
	{
	//printf("\tFile name of testing area \"%s\"  \n", testfilename);		
	strcpy(shapefileout, testfilename);
	strcat(shapefileout, "_v2.");
	strcat(shapefileout, extension);
	printf("\t** File name of output shape file will be :  \"%s\"  \n", shapefileout);	
	}
	
//exit(-1);  // for debugging
	
/**********************************************************************/

//	Org. defaults for some input parameters 

/**********************************************************************/

More_Check:

	printf("\nChecking the other minor parameters ...\n");
	
// Set all the default values  

	leng_thr[0] = 2; 	leng_thr[1] = 50; 	
	blob_wid[0] = 50;	blob_wid[1] = 300;
	sunang[0] = 180;	sunang[1] = 45;
	

// If LENG_THR (m) is left blank OR user entered

if ( EQUALN(argv[4],"-",1 ) || EQUALN(argv[4],"#",1 ) ) 
	{
	//leng_thr[0] = 2; leng_thr[1] = 50; 	
	printf("\n\t**Using default shadows of length between %d and %d metres\n", leng_thr[0], leng_thr[1]);
	}
else
	{		//	Read user shadows length
	strcpy(temp_s, argv[4]);  
	cptr = strtok(temp_s, ", "); 				// check for comma to separate
	//printf("\n Temp_s :  %s \n", temp_s);
	leng_thr[0]  = strtol(cptr, NULL, 10);		// change to integer			
	cptr = strtok(NULL, ", ");					// go after the comma	
	if(cptr != NULL) leng_thr[1]  = strtol(cptr, NULL, 10);		// change to integer	
		
	printf("\n\t**Keeping shadows of length between %d and %d metres\n", leng_thr[0], leng_thr[1]);		
	}

	
p_maxlen = (int) ( leng_thr[1] / ypixsz );	// max length in pixels



// IF BLOB_WID(cm) is left blank OR user entered

if ( EQUALN(argv[5],"-",1 ) || EQUALN(argv[5],"#",1 ) )
	{
	//blob_wid[0] = 50;	blob_wid[1] = 300;
	printf("\n\t**Using default shadows of width between %d and %d cm\n", blob_wid[0], blob_wid[1]);
	}
else
	{	// 	Read user Shadows Width
	strcpy(temp_s, argv[5]);  
	cptr = strtok(temp_s, ", "); 					// check for comma  to separate
	//printf("\n Temp_s :  %s \n", temp_s);
	blob_wid[0]  = strtol(cptr, NULL, 10);			// change to integer			
	cptr = strtok(NULL, ", ");	 					// go after the comma	
	if(cptr != NULL) blob_wid[1]  = strtol(cptr, NULL, 10);			// change to integer	
	
	printf("\n\t**Keeping shadows of width between %d and %d cm\n", blob_wid[0], blob_wid[1]);	
	}

// IF SUNANG is left blank OR user entered

if ( (argc < 7)  || EQUALN(argv[6],"-",1 ) || EQUALN(argv[6],"#",1 )  ) 
//if ( EQUALN(argv[6],"-",1 ) || EQUALN(argv[6],"#",1 ) || EQUALN(argv[6]," ",1 ) )
//if ( EQUALN(argv[6],"-",1 ) || EQUALN(argv[6],"#",1 ) ||  argv[6] = NULL) 	
	{
	//sunang[0] = 180;	sunang[1] = 45;
	printf("\n\t**Using default sun azimuth of %d and sun elevation of %d degrees\n", sunang[0], sunang[1]);
	}
else
	{		// 	Read user Sun angle
	strcpy(temp_s, argv[6]);   
	cptr = strtok(temp_s, ", "); 				// check for comma  to separate/
	//printf("\n Temp_s :  %s \n", temp_s);
	sunang[0]  = strtol(cptr, NULL, 10);			// change to integer	
	cptr = strtok(NULL, ", ");					// go after the comma	check for a space
	if(cptr != NULL) sunang[1] = strtol(cptr, NULL, 10);	// change 2nd item to integer	
		
	printf("\n\t**Keeping sun azimuth of %d and sun elevation of %d degrees\n", sunang[0], sunang[1]);	
	}
	




/**********************************************************************/

/* 		Open an output text files to get some specific results */

/**********************************************************************/

printf("\n\n**Preparing output text file of shadows meeting criteria\n\n");

shadows_fp = fopen("Shadows.txt","w");

histo_fp = fopen("Histograms.txt","w");

time (&rawtime); timeinfo = localtime (&rawtime);
fprintf(shadows_fp,"\n\tFrom program %s (%s) at %s\n\n", PROG_NAME, VERSION, asctime(timeinfo));

fprintf(shadows_fp,"Input Filename :  %s \n", fullfilename);

fprintf(shadows_fp,"Output Filename :  %s \n\n", fullfileout);

fprintf(shadows_fp,"\tTree shadows with length between %d and %d metres\n",leng_thr[0],leng_thr[1]);

fprintf(shadows_fp,"\tAND with width between %d and %d cm\n",blob_wid[0],blob_wid[1]);

fprintf(shadows_fp,"\tAND sun azimuth of %d and sun elevation of %d degrees\n", sunang[0], sunang[1]);



fprintf(shadows_fp,"\n\nShadows Initial Position (P,L), length(m), width(m), area(p) \n");

printf("\n---------------------------------------------------------------\n");

/**********************************************************************/

//		 If NO Test Area (bitmap OR stand polygons in a shp file),  do full area

/**********************************************************************/


/* 
if (argcnt[3] > 1 ) 
	{ printf("\n\n *** ERROR - Multiple Bitmap Test Areas not allowed at this point \n\n"); exit(-1); }

printf("\n\t** Dealing with full image OR bitmap/vector segment (TESTSEG) of areas to consider ...\n");
 */

if ( EQUALN(argv[3],"-",1 ) || EQUALN(argv[3],"#",1 ) ) 	// No test area, so analyse full image
	{
	printf("\n\tDoing full image ...\n");
	
	//get_shadow_lengths(isolbitbuf, piLayer, piFeature);	// **** Go down objects and get length from each
	get_shadow_lengths(isolbitbuf, NULL, NULL);		// **** Go down objects and get length from each 

	printf("\n\tBack from Analysing full image\n");	

	goto Conclude;
	}



/**********************************************************************/

// If given a Test Area, open the test area (bitmap or vector)

/**********************************************************************/

	
	if (segtype == SEG_BIT) 
		{
		printf("\nReading input Test Area Bitmap \n");
		seg_in =  open_imaFile(argv[3]);	
		
		testbitbuf = read_bitmap(argv[3], 1);			// always 1 for tiff  file bitmap, different for PCI files 
		safety_zone(testbitbuf);
		

printf("\n--------------------------------------------------\n");		

printf("\n\tAnalysing for bitmap test area ...\n");		
		
		// Use testseg as a mask for isolbit to work only on that area 

		for (i = 0; i < bmsize; i++)  *(testbitbuf + i) = (*(isolbitbuf + i)) & (*(testbitbuf + i));

		//get_shadow_lengths(testbitbuf, piLayer, piFeature);		// **** Go down objects and get length from each 
		get_shadow_lengths(testbitbuf, NULL, NULL);		// **** Go down objects and get length from each 

		//printf("\n\tBack from Analysing for bitmap test area ...\n");	
		
		goto Conclude;
		}
	
	
/**********************************************************************/

// If given a Test Area that is a SHAPE file (SEG_VEC = 1) , 

printf("\nReading input Test Area from a Shape File \n");
		

int iField, iFeature, layer_toget;

	vecmode = 1;	// test area(s) is a shp file 
	
	
// OPEN the shp file

	piDS = (GDALDataset *) GDALOpenEx( argv[3], GDAL_OF_VECTOR, NULL, NULL, NULL );
	//piDS = (GDALDataset *) GDALOpen( argv[3],  GA_ReadOnly );

	
	if( piDS == NULL )
		{fprintf(stderr, "\nFailed to open input file. Error : %s \n", strerror(errno)); exit( -1 );}

	printf("\n\t**Test area file '%s' was opened for reading\n\n",argv[3]);

// Print generic info (driver used, ... )

	printf( "Driver: %s/%s\n",
          piDS->GetDriver()->GetDescription(),
          piDS->GetDriver()->GetMetadataItem( GDAL_DMD_LONGNAME ) );


// Open layer one (1) first and only layer (count starts at zero)

	layer_toget = 0;

	layer_count = piDS->GetLayerCount();					// C++ approach  ** NOGO ***
	//layer_count = GDALDatasetGetLayerCount(piDS);			// C handler  approach 
	
	printf("\n\t**Number of layers in vector file is %d \n", layer_count);

	printf("\tAccessing layer %d \n", layer_toget + 1);
		
	piLayer = piDS->GetLayer(layer_toget);				// layer number start at zero
	//OGRLayerH piLayer =  GDALDatasetGetLayer( piDS, layer_toget);		// C handler  approach 
	//piLayer = (OGRLayerH *) GDALDatasetGetLayer( piDS, layer_toget);
	//piLayer  = piDS->GetLayer(1);	
	if( piLayer == NULL )
		{fprintf(stderr, "Failed to access layer - %s \n", strerror(errno)); exit( -1 );}	

	//printf("\tGot access to layer %d \n\n", layer_toget + 1);


// Not really needed here, but a good practice

	//fprintf(stdout,"Reseting Reading on  layer %d \n\n", layer_toget+1);	
	piLayer->ResetReading();
	
//	piFDefn =  piLayer->GetLayerDefn();			// ### trouble ###	

	piFDefn =  (OGRFeatureDefn *) OGR_L_GetLayerDefn(piLayer);	//trying C version rather than C++ 
	//printf("\n\tPointers after the call to GetLayerDefn() : %p  %p %p\n", piDS, piLayer, piFDefn);
	if( piFDefn == NULL )
		{printf( "Failed to access layer definition - %s \n", strerror(errno)); exit( -1 );}

//	Get feature count and field count
	
	feat_count = OGR_L_GetFeatureCount(piLayer,0);		//trying C version rather than C++		
	if( feat_count == NULL )
		{printf( "Failed to access layer FeatureCount - %s \n", strerror(errno)); exit( -1 );}
	
	field_count = piFDefn->GetFieldCount();
	if( field_count == NULL )
		{printf( "Failed to access layer FieldCount - %s \n", strerror(errno)); exit( -1 );}

	printf("\nINFO:\t Layer %d of current file has %d features with %d fields each\n\n", 
				layer_toget+1, feat_count, field_count);


//************************************************************************

#ifdef DEBUG_ME

// Print some feature FIELDS 

	last_feat = 2 ;		// for testing
	last_feat = 0 ;		// for debugging
	
	if(last_feat > 0)
		printf("\n\n\t\tLooping through some(%d) features for FIELD INFO  ...\n\n", last_feat);
	
	piLayer->ResetReading();	// VERY IMPORTANT (if you use GetNextFeature() ;

	for (iFeat=0; iFeat<last_feat; iFeat++)			// loop through all the features (polygons)

	  {
	  piFeature = piLayer->GetNextFeature();

//	Field name as a single line

/*
	for( iField = 0; iField < 5 ; iField++ )
	  {
	  piFieldDefn = piFDefn->GetFieldDefn( iField ); 
	  printf("%s,",piFieldDefn->GetNameRef( ) );
	  }
*/

//	  printf("\nData in the fields of feature %d \n", iFeat);
//	  for( iField = 0; iField < piFDefn->GetFieldCount(); iField++ )


// Print some field names and their data


	last_field = field_count;
	if(last_field > 5) last_field = 5 ;
	
	  printf("\nSome(%d) fields and their data for feature %d \n", last_field, iFeat);

	  
	  for( iField = 0; iField < last_field; iField++ )
		{
		piFieldDefn = piFDefn->GetFieldDefn( iField );  // get next field definition

		printf("\t\t %s : ", piFieldDefn->GetNameRef( ) );	 	//field name

		if( piFieldDefn->GetType() == OFTInteger )
        	  printf( "%d \n", piFeature->GetFieldAsInteger( iField ) );
		else if( piFieldDefn->GetType() == OFTInteger64 )
        	  printf(  "%I64d \n", piFeature->GetFieldAsInteger64( iField ) );
		else if( piFieldDefn->GetType() == OFTReal )
        	  printf( "%.3f \n", piFeature->GetFieldAsDouble(iField) );
		else if( piFieldDefn->GetType() == OFTString )
       		  printf( "%s \n", piFeature->GetFieldAsString(iField) );
		else
        	  printf( "%s ", piFeature->GetFieldAsString(iField) );
		}

	  printf("\n");
	  
	  OGRFeature::DestroyFeature( piFeature );

	  } 			//  end of while (in this case for loop of 2)

#endif

//**************************************************************************


// Allocate memory for a "testbitbuf" to burn filled polygons into

	//testbitbuf = (PixVal *) malloc(sizeof(bmsize));   // ### BAD 
	//testbitbuf = (PixVal *) malloc(bmsize); 
	
	testbitbuf = (PixVal *) calloc(bmsize, 1 );
	check_mem(testbitbuf);


printf("\n--------------------------------------------------\n");	
printf("\n** Shadow lengths within polygons will be reported to output SHP file : <<%s>> \n\n", shapefileout); 
			
printf("\tFIRST, by creating it and copying existing Fields and Features (Polygons) to it ... \n");		

	strcpy(testfilename,argv[3]);	// reset filename

	Copy_Shape_File(testfilename, shapefileout);

printf("\n--------------------------------------------------\n");
printf("\n\t *** Preparing new fields to report SHADLEN info to output SHP file... \n\n");

	Prep_New_Fields(poLayer);	

printf("\n--------------------------------------------------\n");	
printf("\n\t *** REPORTING tree heights (shadow lengths) to output SHP file ... \n\n");
     
 	Paint_Plot_Ana_g(poLayer);			// paint AND ANALYSE all polygons (one by one )for shadow length

	goto Conclude;

	


/**********************************************************
	OUTPUT BITMAP TO PCI FILE AND CLOSE
************************************************************/

Conclude:

// Scanning output bitmap to verify some content		FOR DEBUGGING
/* 
count = 0;
for ( i = 3 ; i < (ysize-3) ; i++ )		// image starts at (1,1) 
for ( j = 3 ; j < (xsize-3) ; j++ ) 
  {
  bitnum = (i-1)*(int64)Pixels + j-1 ;		// bitnum starts at zero 
  if (testbit(outbitbuf, bitnum)) count++;
  }
  
printf("\n---------------------------------------------------------------\n");
printf("\n Count of set pixel in Output Bitmap  = %d\n", count);



printf("\n---------------------------------------------------------------\n");

printf("\n***** Writing out output layer (polayer) ***** \n\n");

printf("\n***** Closing SHAPE  file ***** \n");

GDALClose( poDS );		// Close that data set (shp file)

*/

printf("\n---------------------------------------------------------------\n");


if (segtype == 0)
  {
  printf("\n\t\t Full Image SUMMARY \n");  
  
  printf("\n\t %d objects selected and %d erased, out of %d \n", s_crown, e_crown, t_crown);
  printf("\n\t Average shadow length within test area %.2f metre \n\n",	tot_length/s_crown );

  fprintf(shadows_fp,"\n\t\t Full Image SUMMARY \n");  
  fprintf(shadows_fp,"\n\t %d objects selected and %d erased, out of %d \n", s_crown, e_crown, t_crown);
  fprintf(shadows_fp,"\n\t Average shadow length within test area %.2f metre\n\n",	tot_length/s_crown );
  }

if ( (segtype == SEG_BIT) ||(no_stands == 1) )
  {
  printf("\n\t\t  SUMMARY shadow length within the test area\n"); 
  
  printf("\n\t %d objects selected and %d erased, out of %d \n", s_crown, e_crown, t_crown);
  printf("\n\t Average shadow length within test area %.2f metre \n\n",	tot_length/s_crown );
  
  fprintf(shadows_fp,"\n\t\t  SUMMARY shadow length within the test area\n");
  fprintf(shadows_fp,"\n\t %d objects selected and %d erased, out of %d \n", s_crown, e_crown, t_crown);
  fprintf(shadows_fp,"\n\t Average shadow length within test area %.2f metre\n\n",	tot_length/s_crown );
  }


if(no_stands > 1)
	{
	printf("\n\t\t  SUMMARY shadow length within all polygons\n"); 
	printf("\n\t Average shadow length within the WHOLE test area %.2f \n", tot_tot_length/tot_s_crown );
	fprintf(Report,"\n\t Average MODE of shadow length within the WHOLE test area %.2f \n\n", ((float)tot_mode/10)/no_stands );
	fprintf(Report,"\n\t Average top 90%% of shadow length within the WHOLE test area %.2f \n\n", ((float)tot_mean95/10)/no_stands );

	fprintf(shadows_fp,"\n\t\t  SUMMARY shadow length within all polygons\n"); 
	fprintf(shadows_fp,"\n\t Average shadow length within the WHOLE test area %.2f \n", tot_tot_length/tot_s_crown );
	
	}

printf("\n---------------------------------------------------------------\n");


// 		Writing output  bitmap 

printf("\n\t** Writing Tree Stems of specified length to <<%s>>\n\n", fullfileout);

// Prep output image description

if (segtype == 0)
	sprintf(Description,"Tree Lengths(%d,%d) from %s reported for full image ", 
				leng_thr[0], leng_thr[1], fullfilename);

if (segtype != 0)
	sprintf(Description,"Tree Lengths(%d,%d) from %s reported for %s", 
			leng_thr[0], leng_thr[1],fullfilename, testfilename);	
	
	write_bitmap(outbitbuf, fullfileout);


// 		Writing description to output shape file : shapefileout

if(vecmode)
	{		
	char 	Description2[80];
	printf("\n\n** All polygons and their attributes (old & new) were moved to SHP file <<%s>>\n", shapefileout);

	strcpy(Description2,"Copy of " );
	//strcat(Description2,	 piLayer->GetDescription());
	strcat(Description2, testfilename);
	strcat(Description2," with additional SHAD_LEN info" );
	
	poLayer->SetDescription(Description2);
	printf("\nOutput layer DESCRIPTION:\n %s \n", poLayer->GetDescription() );
	
	GDALClose( poDS );		// Close that data set (shp file)
	}



/* 

// Scanning midbitbuf bitmap to verify some content			FOR DEBUGGING

count = 0;
for ( i = 3 ; i < (ysize-3) ; i++ )		
for ( j = 3 ; j < (xsize-3) ; j++ ) 
  {
  bitnum = (i-1)*(int64)Pixels + j-1 ;		
  if (testbit(midbitbuf, bitnum)) count++;
  }
printf("\n Count of set pixel in MiddleBitmap  = %d\n", count);

// Scanning output bitmap to verify some content		FOR DEBUGGING

count = 0;
for ( i = 3 ; i < (ysize-3) ; i++ )		
for ( j = 3 ; j < (xsize-3) ; j++ ) 
  {
  bitnum = (i-1)*(int64)Pixels + j-1 ;		
  if (testbit(outbitbuf, bitnum)) count++;
  }
printf("\n Count of set pixel in Output Bitmap  = %d\n", count);

 */


Exit:

//	printf("\n\n\n\n\t *** We are still in testing mode *** \n");

// For some reason if you close files here, it does not do the rest	
//	GDALClose(ima_in);
//	GDALClose(ima_out);


time (&rawtime);
timeinfo = localtime (&rawtime);
fprintf(stdout,"\n\n_______________________________\n");
fprintf(stdout,"\n %s (%s) finished at %s\n\n", PROG_NAME, VERSION,  asctime(timeinfo));


// For people using this program via ArcGIS, give then some time to examine the results (before disappearing)

if ((strncmp("ArcGIS ",argv[argc-1],3) == 0) )		// if last argument is ArcGIS or ArcMap
	{
	fprintf(stdout,"\n\n######\n");
	printf("\n Type anything to make this detailed window disappear and terminate %s ",PROG_NAME);
	answer[0] = getc(stdin); 		// gets any answer or <CR>
    }

	
	printf("\nClosing all files and exiting program. \n");

	GDALClose(ima_in);		// NOTE: you cannot close files too early
	GDALClose(ima_out);
	GDALClose( poDS );		// Close that data set (shp file)
	
exit(0);				// exit properly 



}		/* END OF MAIN */


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

		/* FUNCTIONS */

/**************************************************************************/





// function to initialize crown records (here, shadow length records)

void init_rec(crown *p )

{
	 p->area = 0;		// crown area, here, length
	 p->xmin = Pixels;	/* diam in both directions*/
	 p->xmax = 0;
	 p->ymin = Lines;
	 p->ymax = 0;
	 p->UTM_East = 0.0;
	 p->UTM_North = 0.0;
	 p->length = 0.0;	
	 p->width = 0.0;
	 p->volume = 0.0;
	 p->removed = 0;	/* flag */

}




//*************************************************************

// Initialize structure about histogram information  (NOW in hist.c and hist_g.h)

/*
void init_histo_struct(hist_inf *p)
	{
	p-> first = 0;			// first bin of interest
	p->last = 0;			// last bin of interest
	p->range = 0;			// range of useful value
	p->m_count = 0;		// count at mode
	p->mode = 0;			//mode position in histogram
	p->min_count = 0;		//count threshold to remove not so well populated bins
	p->mean = 0.0;			// crude mean as gathered from histogram
	p->st_dev = 0.0;		// crude standard deviation as gathered from histogram
	p->ipos95 = 0;			// position of 90% 
	p->mean95 = 0.0;		// mean of values in top 90% of histogram
	}
	 

*/


/**********************************************************************/

//	Function to go down objects (shadow blobs) and get vertical length from each vertical line
//	then in Phase II, reduce it to a single max length line
//	If used with a PURE bitmap only (testbitbuf), pLayer and hShapeId are set to zero



void 	get_shadow_lengths(unsigned char *bitbuf, OGRLayer *poLayer, OGRFeature *poFeature)
{
int i, j, k, ii;		// internal vars
int64 bitnum;
//int	xsize, ysize, channels;
//GDBField sField; 
OGRFieldDefn	*piFieldDefn;
int	 iField;
int 	histo[500], m_count, mode, first, last;

// Intitialize some variables

s_blob=0; e_blob=0; t_blob=0;		// number of selected, total, erased  shadow blobs
tot_length = 0.0;

for ( k = 0 ; k < 500 ; k++ ) histo[k]=0;		// empty histogram

xsize = Pixels; ysize = Lines;

//printf("\n xsize=%d ysize=%d Pixels=%d Lines=%d \n",xsize, ysize, Pixels, Lines);

if(PhaseI_bypass) goto PHASE_II;
	

//*************************************************************************************************

//				PHASE I


fprintf(Report,"\n\t *** Starting PHASE I to parametrize tree shadows ...\n\n");


//goto PhaseII;		// to skip Phase I work


/* Scanning input bitmap for objects with possible tread going down */

for ( i = 3 ; i < (ysize-3) ; i++ )		/* image starts at (1,1) */
for ( j = 3 ; j < (xsize-3) ; j++ ) 

  {
  bitnum = (i-1)*(int64)Pixels + j-1 ;		/* bitnum starts at zero */

  /* Find an initial part of shadow going down (at least 4 pixel long) */
  /* Continue down in that object and record parameters */

 if (   testbit(bitbuf, bitnum)
	&& testbit(bitbuf,bitnum+xsize)	)		//Next line precisely down	//at least 2 pixel long
	//&& testbit(bitbuf,bitnum+2*xsize)		//Next line
	//&& testbit(bitbuf,bitnum+3*xsize) )		//Next line (at least 4 pixel long)
	{

	init_rec(p1);				// to store info about present blob only
	t_blob = t_blob + 1 ;			// count initial blob

	fill_vert(j, i, p1, bitbuf, midbitbuf);	// VERTICAL (down) recursive fill 
	//and as it does it writes to intermediate BM (for 2nd phase) and erase from input BM 



	x_length = xpixsz * (p1->xmax - p1->xmin +1);	// in metres (float) 
	y_length = ypixsz * (p1->ymax - p1->ymin +1);

	// From LOG_IVOL code (Tree log can be sitting diagonally relative to image)
	//p1->length = sqrt(x_length * x_length + y_length * y_length );
	//p1->width = (p1->area * xpixsz * ypixsz) / p1->length; 
	//p1->volume = 3.14159 * pow((p1->width/2),2) * p1->length ;

	p1->length = y_length;		// in metres (float)
	p1->width = x_length;

	p1->UTM_East = topleftX + transformX * (p1->xmax + p1->xmin)/2 ;	// centroid UTM position
	p1->UTM_North = topleftY + transformY * (p1->ymax + p1->ymin)/2 ;



/**********************************************************************/

/* CHECK objects based on user input criteria */

/**********************************************************************/


/* 	If line is too short (as per user input or the 2m minimum) remove (erase) what was written in the output bitmap 
	This removes shadows smaller than 2 metres and removes part of bigger shadows smaller than 2m, sometimes
	useful in separating touching shadows
*/

	if ( (p1->length < leng_thr[0]) || (p1->length > leng_thr[1]) )
	  {
	  erase_vert(j,i,p1,midbitbuf);		// erase line already in intermediate bitmap (line too short to be worthwhile)
	  e_blob = e_blob + 1 ;
	  //fprintf(shadows_fp,"Removed short line at (P,L): %d %d %.3f %.3f %d \n ", j,i,p1->length,p1->width,p1->area);
	  continue;
	  }


// If blob is OK, count it

	s_blob = s_blob + 1 ;
	tot_length = tot_length  + p1->length;		// accumulate length to get average length (in m) 

	}	/* end of "initial shadow 4 pixel segment" loop  */


//   if( ((i/100)*100 == i) && (j == 3) ) IMPCounter( (float) i / (float) Lines); 


  }	/* end of full bitmap (test area or polygon) scanning loop */

	fprintf(Report,"       In PHASE I \n");
	fprintf(Report,"%d line segments selected and %d erased, out of %d \n", s_blob, e_blob, t_blob);
	fprintf(Report,"Average shadow blob length within test area %.2f \n",	tot_length/s_blob );


//*************************************************************************************************

/*
					PHASE II

	Now that small shadow blobs (< 2m in HT) have been removed and that serious shadow blobs 
	have been trim to their essential vertical components (i.e., removing 2m components), 
	pickup a single height (longest line) for that blob then, write it to output bitmap
	and summarize only that (also HT histogram of stand)

*/


// Using tempbitbuf for Phase II work

PHASE_II:

// printf("\n xsize=%d ysize=%d Pixels=%d Lines=%d \n",xsize, ysize, Pixels, Lines);


//If bypassing Phase I, use bitbuf directly

if(PhaseI_bypass)
	for (i = 0; i < bmsize; i++)  *(tempbitbuf + i) = *(bitbuf + i);

//If NOT bypassing Phase I, use midbitbuf 

if( ! PhaseI_bypass)
   for (i = 0; i < bmsize; i++)  *(tempbitbuf + i) = *(midbitbuf + i);  //dealing with the intermediate bitmap

safety_zone(tempbitbuf);

fprintf(Report,"\n\t *** Starting PHASE II to parametrize  tree shadows ...\n\n");

PhaseII:					// when skipping Phase I you get here 

s_crown = 0; t_crown = 0; e_crown = 0;				// number of selected, total, erased  shadow blobs
tot_length = 0.0;


//goto PhaseIII;			// to skip Phase II work (single line phase)


for ( k = 0 ; k < 500 ; k++ ) histo[k]=0;	// empty histogram from previous run (previous polygon (forest stand))
	
// can the whole bitmap resulting from Phase I cleaning

for ( i = 3 ; i < (ysize-3) ; i++ )		// line number
//{
  for ( j = 3 ; j < (xsize-3) ; j++ ) 	// pixel number
  {
  bitnum = (i-1)*(int64)Pixels + j-1 ;		/* bitnum starts at zero */


  /* Find an initial shadow blob going down at least 4 pixel long */
  /* Continue into that object and find longest vertical dimension and record parameters */


 if (   testbit(tempbitbuf, bitnum)
	&& testbit(tempbitbuf,bitnum+xsize)	)	//Next line precisely down 	//at least 2 pixel long
	//&& testbit(tempbitbuf,bitnum+2*xsize)	
	//&& testbit(tempbitbuf,bitnum+3*xsize) )	//checking at least 4 pixel long
	{
		
	init_rec(p1);				// to store info about present blob only (HERE, area is area)

	t_crown = t_crown + 1 ;			// count initial blob

	fill_8c_blob(j, i, p1, tempbitbuf);		//Fill 8-connected blob in the INPUT bitmap (also erases it)

	
	x_length = xpixsz * (p1->xmax - p1->xmin +1);	// in metres (float) 
	y_length = ypixsz * (p1->ymax - p1->ymin +1);

	// From LOG_IVOL code (Tree log can be sitting diagonally relative to image)
	//p1->length = sqrt(x_length * x_length + y_length * y_length );
	//p1->width = (p1->area * xpixsz * ypixsz) / p1->length; 
	//p1->volume = 3.14159 * pow((p1->width/2),2) * p1->length ;


	p1->width = x_length ;		// in metres (float)
	p1->length = y_length;		// in metres

	p1->UTM_East = topleftX + transformX * (p1->xmax + p1->xmin)/2 ;	// centroid UTM position
	p1->UTM_North = topleftY + transformY * (p1->ymax + p1->ymin)/2 ;


// 			####   For debugging  ###
//	if (i == 5282)
	if ( i > 5280 && i < 5283 && j > 600 && j < 700 )		
	  printf("** Shadow starting at %d %d (P,L): %d %d  %d %d %.3f %.3f %d \n", 
				j, i, p1->xmin, p1->xmax, p1->ymin, p1->ymax, p1->length ,p1->width, p1->area);
							
							
							
//	IF stem is too short or too long, dont use it

	if ( (p1->length < leng_thr[0]) || (p1->length > leng_thr[1]) ) 
	{
	//erase_blob(j, i, p1, tempbitbuf);		// not needed as the fill_blob() already destroyed it
	e_crown= e_crown +1;
	continue;				// should continue scanning the bitmap and re-init object p1
	}

//	IF blob is too small or too wide, dont use it (BLOB_WID is in cm, p1->width is n metre)

	if ( (100*p1->width < blob_wid[0]) || (100*p1->width > blob_wid[1]) ) // compare in cm
	{
	//erase_blob(j, i, p1, tempbitbuf);		// not needed as the fill_blob() already destroyed it
	e_crown= e_crown +1;
	continue;
	}


// Write TREE STEM to output bitmap (outbitbuf)

//	write_stem_vert(p1, outbitbuf);		//write a VERTICAL line

	if (sunang[0] == 180)  write_stem_vert(p1, outbitbuf);			//write a VERTICAL line

	if (sunang[0] != 180)  write_stem_generic(p1, outbitbuf);		//write line in REAL direction


	s_crown = s_crown + 1 ;				// number of selected crowns
	tot_length = tot_length  + p1->length;		// accumulate length to get average length (in m) 
	
	
	//histo[(int) (10*p1->length)]++;		// create histogram of heights within this test area (in cm)
										// x10 to have cm precision in integer array (2.4m is 24)
										// above creates a very sparse histogram (as we work at 50cm)
										
										
	histo[(int)round(p1->length)]++;
	
	
	fprintf(shadows_fp,"Shadow starting at (P,L): %d %d %.3f %.3f %d \n", 
							j, i, p1->length ,p1->width, p1->area);


	}		/* end of initial blob loop  */

 
// if( ((i/100)*100 == i) && (j == 3) )	printf("Done: %d %%\r", (int)((float)i/(float)Lines*100));

  }	
//}		// End of full bitmap (test area or polygon) scanning loop 

printf("\nPHASE II Done: 100%%\n\n");

//******************************************************************************************************

//	 Report on Phase II results 

	fprintf(Report,"       In PHASE II \n");
	fprintf(Report," %d objects selected and %d erased, out of %d \n", s_crown, e_crown, t_crown);
	fprintf(Report,"Average shadow length(m) within test area %.2f \n\n",	tot_length/s_crown );

// 	Analyse the histogram of shadows (heights)

	
	init_histo_struct(ph);			// to store histogram analysis results from ana_hist()

	ana_hist(histo, 500, ph);
	
	tot_s_crown = tot_s_crown + s_crown;			// accumulate for full image
	tot_tot_length = tot_tot_length + tot_length;
	tot_mode = tot_mode + ph->mode;
	tot_mean95 = tot_mean95 + ph->mean95;	
	
	
	fprintf(histo_fp,"\n\t\tHistogram of shadow lengths (m) seen in  %s \n\n", fullfileout);
	
	fprintf(histo_fp,"\nHistogram(cm) First=%d, Last=%d, Mode=%d, Count at mode=%d, Position95%% =%d Mean95%%=%.2f\n", 
				ph->first, ph->last, ph->mode, ph->m_count, ph->ipos95, ph->mean95);

	fprintf(histo_fp," %d objects selected and %d erased, out of %d \n", s_crown, e_crown, t_crown);
	fprintf(histo_fp,"Average shadow length(m) within test area %.2f \n\n",	tot_length/s_crown );
	

//	for (ii = 0; ii < ph->last; i++) {printf("%d ", histo[ii]); }
		
//	for (ii = 0; ii < 20; ii++) {printf("%d ", histo[ii]); }				
	
	for (ii = 0; ii < ph->last; ii++) {fprintf(histo_fp,"%d ", histo[ii]); }		
		
		
//******************************************************

// IF VECTOR MODE --- Write info to appropriate record in output layer

//	if (poLayer >0)			// in VECTOR MODE 
	if (poLayer != NULL)			// in VECTOR MODE 
	  {
	 // printf("\nAdding summarizing info to << vector layer %d >>\n", poFeature);
	 
	 //printf("\nAdding summarizing info to output vector layer: <<%s>> \n\n", shapefileout);
	 
	 
	//poFDefn = poLayer->GetLayerDefn();
	   
	   

	// Get field "Area_SL" and populate
	
	iField =  poFeature->GetFieldIndex("Area_SL");		
	if(iField == -1) { printf("ERROR Getting field index for Area_SL\n\n"); exit(-1);}
	fprintf(Report,"iField for Area_SL : %d\n", iField);	
    //poFeature->SetField(iField, 3.55 );			// to TEST
    poFeature->SetField(iField, polyarea*xpixsz*ypixsz/10000);		
	fprintf(Report,"Area_SL put in field (ha) : %.2f \n", polyarea*xpixsz*ypixsz/10000);	


// Get field "AvHeightSL" and populate
	
	iField =  poFeature->GetFieldIndex("AvHeightSL");		
	if(iField == -1) { printf("ERROR Getting field index for AvHeightSL\n\n"); exit(-1);}
	fprintf(Report,"iField for AvHeightSL : %d\n", iField);		
	poFeature->SetField(iField,  ph->mean);		
	fprintf(Report,"AvHeightSL put in field (m) : %.2f \n", ph->mean);	


// Get field "HTSamples" and populate
	
	iField =  poFeature->GetFieldIndex("HTSamples");		
	if(iField == -1) { printf("ERROR Getting field index for HTSamples\n\n"); exit(-1);}
	fprintf(Report,"iField for HTSamples: %d\n", iField);	
    poFeature->SetField(iField, ph->samples);		
	fprintf(Report,"HTSamples put in field (m) : %d \n", ph->samples);	

// Get field "HT_ST_DEV" and populate
	
	iField =  poFeature->GetFieldIndex("HT_ST_DEV");		
	if(iField == -1) { printf("ERROR Getting field index for HT_ST_DEV\n\n"); exit(-1);}
	fprintf(Report,"iField for HT_ST_DEV: %d\n", iField);	
	
    poFeature->SetField(iField, ph->st_dev);		
	fprintf(Report,"HT_ST_DEV put in field (m) : %.2f \n", ph->st_dev);	



// Get field "HT_mean95" and populate
	
	iField =  poFeature->GetFieldIndex("HT_Mean_95");		
	if(iField == -1) { printf("ERROR Getting field index for HT_mean95\n\n"); exit(-1);}
	fprintf(Report,"iField for HT_Mean_95: %d\n", iField);	
	
    poFeature->SetField(iField, ph->mean95);		
	fprintf(Report,"HT_mean95 put in field (m) : %.2f \n", ph->mean95);	


// Get field "HT_Mode" and populate
	
	iField =  poFeature->GetFieldIndex("HT_Mode");		
	if(iField == -1) { printf("ERROR Getting field index for HT_Mode\n\n"); exit(-1);}
	fprintf(Report,"iField for HT_Mode: %d\n", iField);	
	
    poFeature->SetField(iField, ph->mode);		
	fprintf(Report,"HT_Mode put in field (m) : %d \n", ph->mode);	

  
 
// Need to move the feature (polygons and its attributes) to the output shp file
// from memory --- Now we create a feature in the file

 	 //if( poLayer->CreateFeature( poFeature ) != OGRERR_NONE )
			 
      if( poLayer->SetFeature( poFeature ) != OGRERR_NONE )			 
 	      {
        	fprintf(Report,"\t**Failed to create feature(poly) in shapefile.\n" );
        	exit( 1 );
 	      }
 

	  }				// end of IF VECTOR MODE

 



PhaseIII:		// there is no Phase III, just a way to skip Phase II when testing

fprintf(Report,"\nEnd of get_shadow_lengths() for that test area (or polygon)\n\n");
//fprintf(Report,"\n------------------------------------------------\n");

}		// *** End of function   get_shadow_lengths()


//********************************************

//	Functions

//********************************************

/* This function is a 4-connected recursive fill routine for a blob in a bitmap.
   It is destructive of the object in that bitmap (i.e.: clears bit).
   X goes in pixel direction. Y goes in line direction.   
   It accumulates crown area and min&max in both direction */

void fill_blob(int x, int y, crown *pb, unsigned char *bitbuf)

{
	int64 bitnum;
	bitnum = (y-1)*(int64)Pixels + x-1 ;
	clearbit(bitbuf, bitnum);		// clear designated bitbuf
	pb->area = pb->area + 1 ;
	if( x > pb->xmax ) pb->xmax = x;
	if( y < pb->ymin ) pb->ymin = y;
	if( y > pb->ymax ) pb->ymax = y;
	if( x < pb->xmin ) pb->xmin = x;

	if ( testbit(bitbuf, bitnum+Pixels) ) 	fill_blob(x,y+1,pb,bitbuf);
	if ( testbit(bitbuf, bitnum-1) ) 	fill_blob(x-1,y,pb,bitbuf);
	if ( testbit(bitbuf, bitnum-Pixels) ) 	fill_blob(x,y-1,pb,bitbuf);
	if ( testbit(bitbuf, bitnum+1) ) 	fill_blob(x+1,y,pb,bitbuf);
}

/********************************************/

/* This function is a VERTICAL(down) recursive fill routine for an object in the INPUT bitmap.
	It traces its path in the OUTPUT bitmap as possible shadow length (up to a maximun)
   X goes in pixel direction. Y goes in line direction.   
   It accumulates crown area and min&max in both direction */

void fill_vert(int x, int y, crown *pb, unsigned char *inbitbuf, unsigned char *outbitbuf)

{
	int64 bitnum;
	bitnum = (y-1)*(int64)Pixels + x-1 ;
	clearbit(inbitbuf, bitnum);		// remove from input bitmap (isol or test bitbuf)
	setbit(outbitbuf, bitnum);		// write it to middle bitmap

	pb->area = pb->area + 1 ;			// here "area" is length (in P)
	if( x > pb->xmax ) pb->xmax = x;	// accumulate info about blob
	if( y < pb->ymin ) pb->ymin = y;
	if( y > pb->ymax ) pb->ymax = y;
	if( x < pb->xmin ) pb->xmin = x;

	if (pb->area < p_maxlen) 
	  {
	  if ( testbit(inbitbuf, bitnum+Pixels) ) fill_vert(x,y+1,pb,inbitbuf,outbitbuf);
	  }
}

/********************************************/

/* This function is a recursively erases a vertical line from a given bitmap.
   Typically, used to erase a line that was decided "not long enough"  */

void erase_vert(int x, int y, crown *p, unsigned char *bitbuf)

{
	int64 bitnum;
	bitnum = (y-1)*(int64)Pixels + x-1 ;
	clearbit(bitbuf, bitnum);
	
	if ( testbit(bitbuf, bitnum+Pixels) ) 	erase_vert(x,y+1,p,bitbuf);
}



/********************************************/

// This function write the VERTICAL tree stem of a shadow blob
// assuming sun is at 180 degrees

void	write_stem_vert(crown * p1, unsigned char *outbitbuf)

{
	int64 bitnum;
	int i, xpos;	

	xpos = p1->xmin + (p1->xmax - p1->xmin)/2;		// aprox. x center of blob

	//for ( i = p1->ymax ; i >=  p1->ymin ; i-- )		// write in the y direction going up in the image
	for ( i = p1->ymin ; i <=  p1->ymax ; i++ )		// write in y  going down in the shadow
	  {
	  bitnum = (i-1)*(int64)Pixels + xpos-1 ;
	  setbit(outbitbuf, bitnum);
	  }
}

/********************************************/

// This function write a GENERIC tree stem of a shadow blob
// It considers SUNANG

void	write_stem_generic(crown * p1, unsigned char *outbitbuf)

{
	int64 bitnum;
	int i, j;
	float xpos;	

	xpos = p1->xmin;
	j = round(xpos);

	for ( i = p1->ymin ; i <=  p1->ymax ; i++)			// write in the y direction (line direction - down image )
	//for ( j = p1->xmin ; j <=  p1->xmax ; j++ )		// should create a box
	//for ( j = xpos ; j <  xpos+2 ; j++ )				// should be two  pixel wide	
	  {
	  bitnum = (i-1)*(int64)Pixels + j-1 ;		// image coordinates start at (1,1)
	  setbit(outbitbuf, bitnum);
	  //j = j--;									// should be diagonal from bottom
//	  xpos =  xpos + tan(sunang[0]*3.14159/180);    //Note: that tan() is negative // sub a small increment
	  xpos =  xpos + tan( (180-sunang[0]) *3.14159/180); // "i" is assumed to increment by one
	  j = round(xpos);
	  }
}



/********************************************/

/* This function is an 8-connected recursive fill routine for a crown in a bitmap.
   It is destructive of the object in that bitmap (i.e.: clears bit).
   X goes in pixel direction. Y goes in line direction.   
   It accumulates crown area and min&max in both direction */

void fill_8c_blob(int x, int y, crown *pb, unsigned char *bitbuf)

{
	int64 bitnum;
	bitnum = (y-1)*(int64)Pixels + x-1 ;
	clearbit(bitbuf, bitnum);
	pb->area = pb->area + 1 ;
	if( x > pb->xmax ) pb->xmax = x;
	if( y < pb->ymin ) pb->ymin = y;
	if( y > pb->ymax ) pb->ymax = y;
	if( x < pb->xmin ) pb->xmin = x;

	if ( testbit(bitbuf, bitnum-Pixels-1) ) 	fill_8c_blob(x-1,y-1,pb,bitbuf);
	if ( testbit(bitbuf, bitnum-Pixels) ) 	fill_8c_blob(x,y-1,pb,bitbuf);
	if ( testbit(bitbuf, bitnum-Pixels+1) ) 	fill_8c_blob(x+1,y-1,pb,bitbuf);
	if ( testbit(bitbuf, bitnum-1) ) 		fill_8c_blob(x-1,y,pb,bitbuf);
	if ( testbit(bitbuf, bitnum+1) ) 		fill_8c_blob(x+1,y,pb,bitbuf);
	if ( testbit(bitbuf, bitnum+Pixels-1) ) 	fill_8c_blob(x-1,y+1,pb,bitbuf);	
	if ( testbit(bitbuf, bitnum+Pixels) ) 	fill_8c_blob(x,y+1,pb,bitbuf);
	if ( testbit(bitbuf, bitnum+Pixels+1) ) 	fill_8c_blob(x+1,y+1,pb,bitbuf);
}

/********************************************/

/* This function is a  recursive fill routine for a log in a bitmap.
   It is destructive of the object in that bitmap (i.e.: clears bit).
   X goes in pixel direction. Y goes in line direction.   
   It accumulates crown area and min&max in both direction */

void fill_log(int x, int y, crown *pb)

{
	int64 bitnum;
	bitnum = (y-1)*(int64)Pixels + x-1 ;
	clearbit(isolbitbuf, bitnum);
	pb->area = pb->area + 1 ;
	if( x > pb->xmax ) pb->xmax = x;
	if( y < pb->ymin ) pb->ymin = y;
	if( y > pb->ymax ) pb->ymax = y;
	if( x < pb->xmin ) pb->xmin = x;
	if( (y > 3796) && (y < 3803) && (x > 4624)) printf(" %d %d \n",x,y);

	if ( testbit(isolbitbuf, bitnum+1) ) 		fill_log(x+1,y,pb);
	if ( testbit(isolbitbuf, bitnum+Pixels+1) ) 	fill_log(x+1,y+1,pb);
	if ( testbit(isolbitbuf, bitnum+Pixels) ) 	fill_log(x,y+1,pb);
	if ( testbit(isolbitbuf, bitnum+Pixels-1) ) 	fill_log(x-1,y+1,pb);
	if ( testbit(isolbitbuf, bitnum-Pixels+1) ) 	fill_log(x+1,y-1,pb);
}

/********************************************************/


/* This function is a 4-connected recursively erases a crown from a bitmap.
   X goes in pixels direction. Y goes in line direction.   */

void erase_log(int x, int y, crown *pb)

{
	int64 bitnum;
	bitnum = (y-1)*(int64)Pixels + x-1 ;
	clearbit(outbitbuf, bitnum);
	
	if ( testbit(outbitbuf, bitnum+1) ) 		erase_log(x+1,y,pb);
	if ( testbit(outbitbuf, bitnum+Pixels+1) ) 	erase_log(x+1,y+1,pb);
	if ( testbit(outbitbuf, bitnum+Pixels) ) 	erase_log(x,y+1,pb);
	if ( testbit(outbitbuf, bitnum+Pixels-1) ) 	erase_log(x-1,y+1,pb);	
	if ( testbit(outbitbuf, bitnum-Pixels+1) ) 	erase_log(x+1,y-1,pb);
}



/********************************************/

/* This function is a recursively erases a crown from a bitmap.
   X goes in pixels direction. Y goes in line direction.   */

void erase_blob(int x, int y, crown *pb, unsigned char *bitbuf)

{
	int64 bitnum;
	bitnum = (y-1)*(int64)Pixels + x-1 ;
	clearbit(bitbuf, bitnum);
	
	if ( testbit(bitbuf, bitnum+Pixels) ) 	erase_blob(x,y+1,pb,bitbuf);
	if ( testbit(bitbuf, bitnum-1) ) 		erase_blob(x-1,y,pb,bitbuf);
	if ( testbit(bitbuf, bitnum-Pixels) ) 	erase_blob(x,y-1,pb,bitbuf);
	if ( testbit(bitbuf, bitnum+1) ) 		erase_blob(x+1,y,pb,bitbuf);	
}



/********************************************************/
/* This function is a 8-connected recursively to erase a blob in the output bitmap.
   X goes in pixels direction. Y goes in line direction.   */

void erase_8c_blob(int x, int y, crown *pb)

{
	int64 bitnum;
	bitnum = (y-1)*(int64)Pixels + x-1 ;
	clearbit(outbitbuf, bitnum);

	if ( testbit(outbitbuf,bitnum-Pixels-1) ) 	erase_8c_blob(x-1,y-1,pb);
	if ( testbit(outbitbuf,bitnum-Pixels) ) 	erase_8c_blob(x,y-1,pb);
	if ( testbit(outbitbuf,bitnum-Pixels+1) ) 	erase_8c_blob(x+1,y-1,pb);
	if ( testbit(outbitbuf,bitnum-1) ) 		erase_8c_blob(x-1,y,pb);
	if ( testbit(outbitbuf,bitnum+1) ) 		erase_8c_blob(x+1,y,pb);
	if ( testbit(outbitbuf,bitnum+Pixels-1) ) 	erase_8c_blob(x-1,y+1,pb);	
	if ( testbit(outbitbuf,bitnum+Pixels) ) 	erase_8c_blob(x,y+1,pb);
	if ( testbit(outbitbuf,bitnum+Pixels+1) ) 	erase_8c_blob(x+1,y+1,pb);
}


/********************************************************/
// This function is a 8-connected recursive "partial" erases of a blob going "down" in the input bitmap.

void erase_some_input_blob(int x, int y, crown *pb)

{
	int64 bitnum;
	bitnum = (y-1)*(int64)Pixels + x-1 ;
	clearbit(isolbitbuf, bitnum);

	if ( testbit(isolbitbuf,bitnum-1) ) 		erase_some_input_blob(x-1,y,pb);
	if ( testbit(isolbitbuf,bitnum+1) ) 		erase_some_input_blob(x+1,y,pb);

	if( bitnum+Pixels < pb->ymax ) 		// dont erase blob lower than accounted shadow length
	  {
	  if ( testbit(isolbitbuf,bitnum+Pixels-1) ) 	erase_some_input_blob(x-1,y+1,pb);	
	  if ( testbit(isolbitbuf,bitnum+Pixels) ) 	erase_some_input_blob(x,y+1,pb);
	  if ( testbit(isolbitbuf,bitnum+Pixels+1) ) 	erase_some_input_blob(x+1,y+1,pb);
	  }

}

/**************************************************************/

// tell whether the first value (a) is between the other two (b,c) 

/* 
int between(float a, float b, float c)
{
  if( (c > b) && (a<c) && (a>=b) ) return(TRUE);
  if( (b > c) && (a<b) && (a>=c) ) return(TRUE);
  return(FALSE);
}
 */

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


/****************************************************************/
/****************************************************************/



//****************************************************************

// Function to copy an existing shape file to a new one (fiels and features (Polygons)

//****************************************************************
			
void	Copy_Shape_File(char * polygonfile, char * outshpfile)
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

	printf("\t**Layer %d of current file has %d features (shapes) with %d fields each\n\n", 
				ivec, feat_count, field_count);


// GET DRIVER for output ".shp" file 

	//printf("\nGetting driver \"%s\" for output shape file\n",pszDriverName);

	poDriver = GetGDALDriverManager()->GetDriverByName(pszDriverName);
	if( poDriver == NULL )
	  {
          printf( "%s driver not available.\n", pszDriverName );
          exit( 1 );
	  }

// CREATE  output ".shp"  file 

	printf("\n\t*Creating output shape file <<%s>> \t\n", outshpfile);

	poDS = poDriver->Create( outshpfile, 0, 0, 0, GDT_Unknown, NULL );
	if( poDS == NULL )
	  {
	  printf( "*** ERROR ***  Creation of output file failed.\n" );
	  exit( 1 );
	  }

// Create one polygon layer in output ".shp"  file

	printf("\nCreating a layer in output shape file\n");
	
	//poLayer = poDS->CreateLayer("Poly_out", NULL, wkbPolygon, NULL );
	poLayer = poDS->CreateLayer("Poly_out", piLayer->GetSpatialRef(), wkbPolygon, NULL );
	if( poLayer == NULL )
	  {
	  printf( "Layer creation failed.\n" );
	  exit( 1 );
	  }

// Copy ???? GeoTransform, projection, ... NOT a distinc function Done inn above

//	printf("\n\t*Writing projection ???? to file %s\n", outshpfile);	

//	poDS->SetSpatialRef(seg_in->GetSpatialRef());	
	

//*************************************************************************

// 		Read and write polygon data (1st schema, then field data, then vertices)

// Creating fields of output shape file must be done before moving the features (polygons)

//************************************************************************

int iField;

  printf("\n\tCREATING (copying) FIELDS in output shape file (i.e., copying input file SCHEMA)\n\n");

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
	
	//if (strcmp(poFieldDefn->GetNameRef(),"AREA") == 0)  poFieldDefn->SetWidth(10);
 
	}			// END OF  for all field in schema//***********************************


printf("\n\t** Created(copied) %d fields in output shape file <<%s>> \n\n", iField, outshpfile);


 


printf("\n--------------------------------------------------\n");

/* 

// 				DOUBLE CKECKING on some output fields

  printf("\n\tDouble checking on some (5) fields in output shape file\n\n");

  poFDefn = poLayer->GetLayerDefn();			// feature definition
	  
   for (iField=0; iField < 5; iField++)		 	// only 5 to debug
  //for (iField=0; iField < poFDefn->GetFieldCount(); iField++)
    {  	
	poFieldDefn = poFDefn->GetFieldDefn(iField);	// field  definition
	
	printf("Output Field %d, Field Type %d, Field Width %d, Precision %d Name %s\n", 
		iField, poFieldDefn->GetType(), poFieldDefn->GetWidth(), poFieldDefn->GetPrecision(), poFieldDefn->GetNameRef() );	
    }	// endof for all field in output 
 	
//	printf("\n Exiting \n"); exit(-1);


 */



// ***************************************************
// ***************************************************

// 		Now that SCHEMA  is established (i.e., field definition)

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
	   
    printf("  For feature(POLYGON) %d, move its field DATA to the output  shape file\r", iFeat);

	
    poFeature = OGRFeature::CreateFeature( poLayer->GetLayerDefn() );  // creating output feature

	
// For EACH  field in the input layer, POPULATE output layer with same data as input layer

 //for (iField=0; iField < 20; iField++)			// for testing
 
 for (iField=0; iField < piFDefn->GetFieldCount(); iField++)
    {
    piFieldDefn = piFDefn->GetFieldDefn( iField );

//	printf("Input Field %d, Field Type %d, Field Width %d and Precision %d Name %s\n",	
//			iField, piFieldDefn->GetType(), piFieldDefn->GetWidth(), piFieldDefn->GetPrecision(), piFieldDefn->GetNameRef() ); 
	
    poFieldDefn = poFDefn->GetFieldDefn( iField );
	
//	printf("Output Field %d, Field Type %d, Field Width %d and Precision %d Name %s\n",	
//			iField, poFieldDefn->GetType(), poFieldDefn->GetWidth(), poFieldDefn->GetPrecision(), poFieldDefn->GetNameRef() ); 

		
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
					poFeature->SetField(iField, piFeature->GetFieldAsDouble(iField) );
					//poFeature->SetField(iField, (float) piFeature->GetFieldAsDouble(iField) );
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
 			 
	
       	//exit( 1 );		// for debugging

   }		// end of feature (polygon) loop
		

    printf("\n\t Field(attribute) data of %d features copied to the output shape file\n", iFeat);	

	
		//exit(-1);
		
		//goto END_CSF;			// debugging speed-up - dont move polygons
 
//**************************************************************************

// 		For each feature (polygon) ** MOVING POLYGON VERTICES **

//***************************************************************************

	   
	printf("\n--------------------------------------------------\n");
	printf("\n\t ** For each polygon moving polygon vertices from input shp file to output shp file\n\n");	   
	   
	  
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
	  	poFeature->SetGeometry( piGeometry );			// *** WORKS  -- moves all vertices (inner and outer)
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



// Prepare for next feature	(in the loop)
		 
		//OGRFeature::DestroyFeature( poFeature );

		//poLayer->GetNextFeature();		// done above
 			
		
	//  printf("Polygon %d vertices were moved to output SHP file \n\n", iFeat);
	 //printf("Polygon %d vertices were moved to output SHP file \r", iFeat);	  
	  
		
	  }		// **** MAIN LOOP ***  for NEXT feature 



    printf("\n\t Polygon vertices of %d features copied to the output shape file\n", iFeat);

	printf("\n--------------------------------------------------\n");
	
//exit(-1);		// For debugging


// ***********************

/* 

// 	READ input vertices and WRITE them to output polygon

	// ######  Strangely ##### no need to move points (in fact if you move them, you get point features)

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

	printf("\n\n** All polygons and existing attributes moved to output SHP file <<%s>>\n", outshpfile);

	//printf("\n\tExisting layer description: %s \n", piLayer->GetDescription() );
	strcat(Description,"Copy of " );
	strcat(Description,	 piLayer->GetDescription());
	poLayer->SetDescription(Description);
	printf("\nOutput layer DESCRIPTION: %s \n", poLayer->GetDescription() );
	
	//GDALClose( poDS );		// Close that data set (shp file)

	//exit(-1);

  }	// END of Copy_Shape_File()



//************************************************************

// Create new fields (GDAL style) to store generated info in stands

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
	
//	printf("\n\tCreating shad_len_g information fields in output shape file \n");

// Area(ha) field (field #1)

	printf( "Creating <Area_SL> field in output shape file.\n" );
	
	OGRFieldDefn oField1( "Area_SL", OFTReal );
	oField1.SetWidth(10); oField1.SetPrecision(2); 
	if( poLayer->CreateField( &oField1 ) != OGRERR_NONE )
	  { printf( "\n\n ### Creating <Area_SL)> field failed.\n" ); exit( 1 ); }
  
  
	printf( "Creating <AvHeightSL> field in output shape file.\n" );

	OGRFieldDefn oField2( "AvHeightSL", OFTReal );
	oField2.SetWidth(3); oField2.SetPrecision(1); 
	if( poLayer->CreateField( &oField2 ) != OGRERR_NONE )
	  { printf( "\n\n ### Creating <AvHeightSL> field failed.\n" ); exit( 1 ); }


	printf( "Creating <HTSamples> field in output shape file.\n" );
	
	OGRFieldDefn oField3( "HTSamples", OFTInteger );
	oField3.SetWidth(4);
	if( poLayer->CreateField( &oField3 ) != OGRERR_NONE )
	  { printf( "\n\n ### Creating <HTSamples> field failed.\n" ); exit( 1 ); }
  

	printf( "Creating <HT_ST_DEV> field in output shape file.\n" );
	
	OGRFieldDefn oField4( "HT_ST_DEV", OFTReal );
	oField4.SetWidth(5); oField4.SetPrecision(2); 
	if( poLayer->CreateField( &oField4 ) != OGRERR_NONE )
	  { printf( "\n\n ### Creating <HT_ST_DEV)> field failed.\n" ); exit( 1 ); }
 
	printf( "Creating <HT_Mean_95> field in output shape file.\n" );
	
	OGRFieldDefn oField5( "HT_Mean_95", OFTReal );
	oField5.SetWidth(5); oField5.SetPrecision(1); 
	if( poLayer->CreateField( &oField5 ) != OGRERR_NONE )
	  { printf( "\n\n ### Creating <HT_Mean_95)> field failed.\n" ); exit( 1 ); }
  

	printf( "Creating <HT_Mode> field in output shape file.\n" );
	
	OGRFieldDefn oField6( "HT_Mode", OFTInteger );
	oField6.SetWidth(4);
	if( poLayer->CreateField( &oField6 ) != OGRERR_NONE )
	  { printf( "\n\n ### Creating <HT_Mode> field failed.\n" ); exit( 1 ); }
 
 

}	//    End of Prep_New_Fields()




//*************************************************************************************
//***********************************************************************************

/*
   This function paints plot boundaries defined as vectors into a plot bitmap
	AND does the shadow length analysis for ALL polygons in a layer
*/

void Paint_Plot_Ana_g(OGRLayer * piLayer)
{			

int		nVertex;
int		 i, j, k, h, kk, notclosed, outsideflag;
int		TVertices;
int 	set_flag;

extern int 	Poly_Outside;
extern int 	fill_count;				// count of fill pixels from vect2rast_g() (NB could be negative if removing an area)
//int 		polyarea;
int64		count;


// reserve space for 500 vertices (for the moment)
			
pasVertices = (GDBVertex2D *) calloc(500,16);

/* Loop to get ALL shapes within the layer */

//printf("\n--------------------------------------------------\n");
//printf("\nPainting vector-defined test areas into a bitmap in memory ... \n\n");  
  
RingFlag2 = 0;
notclosed = 0;
TVertices = 0;
  
OGRPolygon *piPolygon, *poPolygon;
OGRLinearRing *piExteriorRing;

PolygonFeature Polygon;
OGRPoint ptTemp, ptTemp2, ptTemp3, ptTemp4;

piLayer->ResetReading();	
poLayer->ResetReading();
		
//printf("\n\t **** Looping through features for polygon vertices ...\n\n");


for (iFeat=0; iFeat < 2; iFeat++)				// for testing 
//for (iFeat=0; iFeat <  piLayer->GetFeatureCount(); iFeat++)	  // for all features (polygons) in file
	{
	int poly = iFeat;
	piFeature = piLayer->GetFeature(iFeat);
    fprintf(Report,"\nReading vertices of polygon %d of input vector layer\n", iFeat);


    piGeometry = piFeature->GetGeometryRef();	// get its geometry, hoping polygon (3)

 	fprintf(Report,"Geometry is of type : %d \n", piGeometry->getGeometryType());		// ###


// #### First for full  polygons

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
		fprintf(Report,"Polygon %d - NumberOfExteriorRingVertices = %d \n", iFeat, NumberOfExteriorRingVertices);
		
		
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
				//if (k<5)  printf(" %d = %5.1f,%5.1f ", k+1, pasVertices[k].x, pasVertices[k].y); 	// Debugging Print
               }

// Using my vector to raster (to bitmap) painting routine

	   nVertex = k;

	   //printf("\n* Painting polygon %d as raster in memory (nVertex=%d) \n", iFeat, nVertex);		// for debugging
 
	  	vect2rast_g(nVertex, pasVertices, testbitbuf, xsize, iFeat, set_flag=1);   
	   
	   polyarea = fill_count;		//result from vect2rast_g()
	
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
	 


	fprintf(Report,"\nFor polygon %d - Total number of pixels inside %d, its area %.2f \n", 
				iFeat, polyarea, polyarea*xpixsz*ypixsz/100/100 );	 
	   
	goto CONT_LOOP;		// for testing, just use the outside vertices of simple polygon
	   
	   

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
			vect2rast_g(nVertex, pasVertices, testbitbuf, xsize, iFeat, set_flag=0);

			TVertices = TVertices  + nVertex ;
	      //polyarea[iFeat] = polyarea[iFeat] - fill_count;
			polyarea = polyarea + fill_count;  // fill_count should already be a negative value out of vect2rast()
 
// debugging 	
			//if (iFeat == 310 )   printf("\nFor polygon %d, vect2rast() pixel count now = %d \n", iFeat, polyarea[iFeat]);

			}	// burn (actually unburn) each inner ring separately


CONT_LOOP:

fprintf(Report,"For feature %d total number of vertices considered %d \n", iFeat, TVertices);

fprintf(Report,"\nFor polygon %d - Total number of pixels inside %d, its area %.2f \n", 
				iFeat, polyarea, polyarea*xpixsz*ypixsz/100/100 );	 	
	
	   }		// end of burning polgygon to bitmap
	   
	   
/* 
	printf("\n GOT HERE *** \n\n");
	printf("\n xsize = %d, ysize = %d  \n\n", xsize, ysize);
	printf("\n Pixels = %d, Lines = %d  \n\n", Pixels, Lines);
	 */

	// Scanning output bitmap to verify some content		FOR DEBUGGING

count = 0;
for ( i = 3 ; i < (ysize-3) ; i++ )		
for ( j = 3 ; j < (xsize-3) ; j++ ) 
  {
  int64 bitnum = (i-1)*(int64)Pixels + j-1 ;		
  if (testbit(testbitbuf, bitnum)) count++;
  }

fprintf(Report,"\n Count of set pixel in TestBitmap (test area size) = %I64d\n", count);


	// Test area (from polygon) need to be COMBINE with thick shadow areas for ananlysis

for (i = 0; i < bmsize; i++)  *(testbitbuf + i) = (*(isolbitbuf + i)) & (*(testbitbuf + i));
	
	
// Scanning output bitmap to verify some content		FOR DEBUGGING

count = 0;
for ( i = 3 ; i < (ysize-3) ; i++ )		
for ( j = 3 ; j < (xsize-3) ; j++ ) 
  {
  bitnum = (i-1)*(int64)Pixels + j-1 ;		
  if (testbit(testbitbuf, bitnum)) count++;
  }

fprintf(Report,"\n Count of set pixel in test area and shadows areas = %I64d\n", count);



// **ANALYSE** that zone (stand) for shadow lengths

	//printf("\n-----------------------------------------------------\n");
	printf("\tAnalysing polygon %d with Area(p) = %d, its Area(ha) %.2f\n", 
				iFeat, polyarea, polyarea*xpixsz*ypixsz/100/100 );	
				
	poFeature = poLayer->GetFeature(iFeat);
	
	get_shadow_lengths(testbitbuf, poLayer, poFeature);		// **ANALYSE** that zone (stand) for shadow lengths

    no_stands = no_stands + 1;
	
	//printf("\n\tBack from get_shadow_lengths() for shape %d .. \n\n", iFeat);

	/* HFree(pasVertices);	 free vector storage memory (W7 does not like) */

	} 		// end of for each feature (polygon)


  //printf("Getting out of paint_plot_ana() \n");
      
} 		// 	End of function  Paint_Plot_Ana_g() 



//************************************************************************
//************************************************************************
//************************************************************************

