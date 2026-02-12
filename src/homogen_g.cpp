/* 
Program name: 	homogen_g.cpp

Author: 	François A. Gougeon, Ph.D.
			Remote Sensing Research	
			(©)Natural Resources Canada
			Canadian Forest Service 
			Pacific Forestry Centre
			506 West Burnside Rd.
			Victoria, British Columbia, 
			Canada, V8Z 1M5	


Description:
c
c	Produces an output image convaying the texture "homogeneity"
c	of the input image based on a specific variable (HOMOVAR)
c
c	By default, it conveys a measure of the normalized variance (NVARS)
c	for a pixel, as calculated within a window of IWS*IWS pixels
c	around the pixel of interest, and summarize by bigger blocks
c	IBS*IBS to produce a smoother image. 
c
c	Such an image can later be thresholded to separate regions 
c	with different greylevels and/or textures.
c
C*******************************************8

C1	PCI PARAMETERS
C
C	HOMOGEN is controlled by the following global parameters:
C
C	Name		Prompt					Count	Type
C
C	FILE		Database file name (for input/output)	64	Char
C	DBIC		Input Illumination (B&W) Channel	1	Int
c	HOMOVAR		Homogeneity basis (MEAN/VARI/...)	64	Char
C	IWS			Size of moving window (e.g.,3,5,7)	1	Int
c	IBS			Size of block to summarize info		1	Int
C	DBOC		Channel for resulting variance image	1	Int
C	REPORT		Reporting  device			64	Char
C



HOMOVAR 	Variable to use in judging homogeneity

HOMOVAR="MEAN || VARI || NVAR || STDEV || COVAR || MIN || MAX

MEAN:	Multispectral mean within the IWSxIWS area
VARI:	Variance within the IWSxIWS area
NVAR:	Normalized Variance within the IWSxIWS area
STDEV:	Standard deviation within the IWSxIWS area
COVAR:	Coeficient of Variation within the IWSxIWS area
MIN:	Minimum within the IWSxIWS area
MAX:	Maximum within the IWSxIWS area
MIN10:	Below minimum+10% within the IBSxIBS area
MAX10:	Above Maximum-10% within the IBSxIBS area


**********************************

History:


 v1.1	Oct. 2021	François Gougeon

	- Created from ave_filter_g.cpp and of course from HOMOGEN.cpp

	
 v1.2a		Oct. 2023  François Gougeon
 
			- Major adaptation after two years and adaptation to run from my ArcGIS toolbox

			- NOT to assume that all files are in the default directory from which the program is run ANYMORE
				Previously, everything was assumed in same directory and run from a cmd window from there.
				NOW, 
				the **path used with the main input file** is used when creating the default input/output file names
				This was necessary for ArcGIS Toolkit integration.
			
			- Allow program to automatically create output file name from base file name (using "-" or "#")
			
			- Allow user to specify input channel, like Test_Illum.pix,2 from the cmd line OR
				as a separate item, mostly for ArcGIS
			
			
*******************************************	

USAGE :

	>  homogen_g input_ima output_ima HOMOVAR IWS,IBS


Run as (for examples):

	>  homogen_g Test_Illum.tif test_Var.tif NVAR 7,31

	>  homogen_g Test_Illum.tif - NVAR 7,31			! output file will be created

	>  homogen_g Test_Illum.pix,2 - NVAR 7,31		! OK to specify input channel

	>  homogen_g Test_Illum.pix 2 - NVAR 7,31		! OK to specify input channel
	
*******************************************************************************

*** To compile with Visual Studio (VS14)  (see VS14_GDAL_compile.txt)

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64

set INCLUDE=C:\gdal-2.1.1_v2\include;C:\libtiff-4.0.6_64b\tiff-4.0.6\libtiff;%INCLUDE%
set LIB=C:\gdal-2.1.1_v2\lib;C:\libtiff-4.0.6_64b\tiff-4.0.6\libtiff;%LIB%

set LINK=gdal_i.lib  libtiff_i.lib User32.lib

set CL= /MD

CL xxxxxxxxx.cpp /EHsc

***********************************************

NOTE: Newest GDAL library have included "libtiff" no need to have it anymore

set INCLUDE=D:\GDAL_3.0.0\include;%INCLUDE%
set LIB=D:\GDAL_3.0.0\lib;%LIB%
set LINK=gdal_i.lib  libtiff_i.lib User32.lib
set CL= /MD
CL xxxxxxxxx.cpp /EHsc

set path=D:\GDAL_3.0.0;%PATH%		// to run programs

***  GDAL v3.0.0 has problem getting to its projection library, so do:

set PROJ_LIB=D:\GDAL_3.0.0\projlib


***********************************************
***********************************************

REM	TO COMPILE: with Visual Studio 2019 v16.3.10 (x64) on W-VIC-A144879 (on Ciara)

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall" amd64

set CL= /MD 

set LINK=gdal_i.lib  proj_i.lib  User32.lib /subsystem:console /incremental:no /machine:x64 /DEFAULTLIB:MSVCRT /STACK:0X20000000

	REM for version 3.3.3

set LIB=C:\gdal_3.3.3\lib;%LIB%
set INCLUDE=C:\gdal_3.3.3\include;%INCLUDE%
set DESTDIR=C:\ITC-Suite_GDAL_v333\exe

CL %SRCDIR%\homogen_g.cpp  /EHsc

***********************************************
***********************************************

REM 	TO RUN programs - For GDAL version 3.3.3  (on Ciara, on Z80)

REM	Where the ITC-Suite is located

set PATH=C:\ITC-Suite_GDAL_v333\exe;%PATH%

REM 	Where the GDAL libraies are located

set PATH=C:\GDAL_3.3.3\bin;%PATH%

REM   For the apps of GDAL (like gdalinfo)

set path=C:\gdal_3.3.3\bin\gdal\apps;%PATH%

REM   For the projection info

set PROJ_LIB=C:\GDAL_3.3.3\bin\proj7\share

*********************
Ackowlegment to GDAL:

GDAL - Geospatial Data Abstraction Library: Version 2.1.1 (July2016, 64bit), 
GDAL - Geospatial Data Abstraction Library: Version 3.0.0 (Dec. 2019, 64bit),
Open Source Geospatial Foundation, 
Thanks Frank (Warmerdam)


*******************************************************************************************

********************************************************************************
********************************************************************************
*/

#define VERSION "v1.2a"
#define PROG_NAME "HOMOGEN_G"
#define FILENAME 250


#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <time.h>       // time_t, struct tm, time, localtime

#include "gdal_priv.h"	// For GDAL library

//#include "cpl_string.h"
// #include "tiffio.h"

/* Function declarations */

void 	check_mem(void *);
void    check_param(char *);
void    upper_case(char *);

typedef unsigned char  uchar;
typedef unsigned char  PixVal;
typedef long           int64;
typedef unsigned long  uint64;
typedef unsigned short uint16;
typedef signed __int32 int32;

/* Global declarations */

int	Pixels, Lines, Channels;	// pixel and line starting at 1,1
int	xsize, ysize;			// pixel and line starting at 0,0

PixVal *ima_buf_in, *ima_buf_out, *out_line;

time_t rawtime;
struct tm * timeinfo;

int	bylines_flag = 0;		// by default proceed by full images (not by lines)
int	by_lines=0, by_image=1;			// default is to read/write by image (faster),

int 	data_type, CHN_8U = 1, CHN_16U = 3;

//PIX_FUN_PTR	get_pix_val; 		 // pointer that allows us to deal with  many types of image (8,16u,16s) 


//************************************

int main(int argc, char* argv[])
{

GDALDataset	*ima_in, *ima_out;
void	*Image, *Int_Ima;
GDALDriver 	*piDriver, *poDriver;
GDALRasterBand	*piBand, *poBand;
double		adfGeoTransform[6];
char 		**papszMetadata;

int 	ch_out=1, ch_in=1, no_ch=1, no_ch_out=1, input_ch[10];
int  	iwind, windsize[10], windsiz;
int 	pix, pixout, ofs, data_type, data_type_in, sum, blocksiz, half, min, max, min10, max10;
//float 	sum;

char	 *proj, *proj2;
char 	Proj[250], ans[80], fullfilename[FILENAME];
char 	*filename, *extension, temp[FILENAME];

char	file_in[FILENAME], file_out[FILENAME];
char 	*tstring;
char  	*cptr;				// generic pointer to char
int		argcount;			// counter to go through input line arguments

PixVal		*pafScanline, *image_8b, *image_8b_out;
uint16		*pafScanline16, *image_16b, *image_16b_out;

int	i, j, ii, jj;
int icount = 0;
int	ch_next_flag = 0;
int64 	bitnum, bitnum2, pixnum, pixnum2;

int 	skip_phaseII = 0, phase3_only =0 ;		// flags
char 	homovar[64];
char 	ima_history[80], ima_description[64], timedate[40];
float	mean, variance;

char	*basefname;						// ** just a pointer **
char	basefilname[FILENAME], ach_in[5];
int		basef_len;	


//********************************************************************

/* Print Program Header and time */

	time (&rawtime);
	timeinfo = localtime (&rawtime);

	fprintf(stdout,"\n\t\tStarting %s (%s) at %s\n", PROG_NAME, VERSION, asctime(timeinfo));

//********************************************************************

// Check input parameters (i.e., agrv[*])


	argcount = 1;		// Check *First argument* on command line
	
	//printf("\n\tPresent parameters are %s %s %s %s\n\n", argv[1], argv[2], argv[3], argv[4]);

	if (argv[argcount] == NULL) 
	  {printf("\n\t PROBLEM with input image %s \n",argv[1]);
	  printf("Have an INPUT image as first argument on command line \n");
	  printf("USAGE: > homogen_g input_ima output_ima HOMOVAR IWS,IBS \n\n");
	  exit(1);
	  }  
	  
	 
// Check if only one channel # and only one (or none) is attached to input file (pix or tif)
// If none, it must be in the next argument on the line

	strncpy(temp, argv[argcount], 250);

	cptr = strtok(temp, " ,");			// get filemane	(no comma, if any)
	strcpy(file_in, temp);
	printf("\n**Input file name: %s \n\n", file_in);
	
	cptr = strtok(NULL, " ,");		
	
	
// Check channel number connected to filename (mostly PCI files or big tif file)

	if (cptr != NULL)
	  {
		icount = 0;
		while(cptr != NULL) 
		  {
		  //printf("%s\n", p); 
		  input_ch[icount++]= strtol(cptr, NULL, 10);
		  cptr = strtok(NULL, " ,");
		  }
		no_ch = icount;
		//printf("\nNo. of channel to use %d \n", no_ch);

		if ( no_ch > 1) 
		  {
		  fprintf(stdout,"\n\t##### ERROR - No. of channels to use to create an output image must be one #####\n");
		  //printf("\n\tYou present parameters are %s %s %s %s\n\n", argv[1], argv[2], argv[3], argv[4]);
		  exit(-1);
		  }

		if ( no_ch == 0) ch_in = 1;					// if NO CHANNEL mentionned, assume single channel file
		if ( no_ch == 1) ch_in = input_ch[0];
	  }
	
// Channel numbers COULD BE next parameter on the cmd line
				
	else				
	  {
	  argcount++;			// next argument
	  //printf("Argument %d: %s \n\n", argcount, argv[argcount]);
	  strcpy(temp, argv[argcount]);
	  //cptr = strtok(temp, " "); 				// go to space and put a null there
	  //ch_in = strtol(cptr, NULL, 10);
	  if( (strlen(temp) == 1 ) && (temp[0] != '-') && (temp[0] != '#') )  ch_in = strtol(temp, NULL, 10);	
	  else  argcount--;			// second parameter is probably output filename
	  }
 
	
	printf("Input channel to use: %d \n", ch_in);	
		
//	exit(-1);		// for testing
	  
	  

//*************************************************

// Create BaseFileName   that may be used to create an OUTPUT image filename

//*************************************************
	

// Create base file name   
	
// Need to get "basefilename" when full path is involved
// Basefile name has path + head of file name (typically correspond to "named area" of study e.g. PRF)
// Area name is assumed separated from rest of file name by an underscore
// However, be careful as there could be underscores in the path

	strcpy(fullfilename,argv[1]);			// main input filename (and possibly its dir)
	
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
	
	printf("\nBase file name ::  %s \n\n", basefilname);	

	//exit(-1);			// for debugging
  

// check next argument on command line (originally argv[2])


	argcount++;			// check next argument on command line

 	if (argv[argcount] == NULL) 
	  {printf("\n\n PROBLEM with output image %s \n",argv[argcount]);
	  printf("\tHave an OUTPUT image as second argument on command line \n\n");
	  printf("USAGE: > homogen_g input_ima output_ima HOMOVAR IWS,IBS \n\n");
	  printf("OR, at least \"-\" or \"#\" for an output file name to be generated automatically\n\n");
	  exit(1);
	  } 

//If no output specified, use base file name to create output name, plus info

if ( EQUALN(argv[argcount],"-",1 ) || EQUALN(argv[argcount],"#",1 ) )
	
	{
	//basefname = strtok(fullfilename,".");
	//basefname = strtok(basefname,"_");				
	//printf("basefname  :  %s \n", basefname);	

//	basefname = temp;	 				// fake to init basefname as a char array (just a pointer)
//	strcpy(basefname, basefilname);	
	
	printf("\nSecond main argument is \"-\" or \"#\", which implies no output file name given \n");
	printf("A new output filename will be created based on \"base file name\" \n\n"); 
	  
	  
	strncat(file_out, basefilname, basef_len);
	strncat(file_out,"_Homog_",7); 
	strncat(file_out,argv[argcount+1],4); 			//HOMOGEN VARS
	//printf("file_out :  %s \n", file_out);   
		
	//itoa(ch_in,ach_in,10); 	 
	sprintf(ach_in, "%d" , ch_in);	 	
	strncat(file_out,ach_in,3);	  	
	strncat(file_out,".tif",4);					// output image is forced to be a tif	
	printf("\n\t** Output filename will be \"%s\" \n", file_out);
	}	
else								// if  input bitmap, just add to that name
	{
	//basefname = strtok(fullfilename,".");		// fake to init basefname
	//strcpy(basefname,argv[2]);
	//basefname = strtok(basefname,".");
	//printf("\n\n\t *** Output Filename as stated:  %s \n", argv[2]);
	strcpy(file_out,argv[argcount]);	
	printf("\n\t** Output filename will be \"%s\" \n", file_out);

	}


//	exit(-1);		// for degugging


	argcount++;			// check next argument on command line  (originally argv[3])
	
	if (argv[argcount] == NULL) 
	  {
	  printf("\n\t ### ERROR ### A third argument is needed \n");
	  printf("\nThat third argument (HOMOVAR)is typically one of: \n");  
	  printf("HOMOVAR = MEAN || VARI || NVAR || STDEV || COVAR || MIN || MAX \n");
	  printf("\nUSAGE: > homogen_g input_ima output_ima HOMOVAR IWS,IBS \n\n"); 
	  exit(1);
	  }
	  
	  //exit(-1);		// for testing




//	Check third input parameters : HOMOVAR 

	strcpy(homovar, argv[argcount]);

	check_param(homovar);
	fprintf(stdout,"\nFeature to be assessed by Homogen_g is %s \n\n", homovar);


// Check window size and block size

	argcount++;			// check next argument on command line  (originally argv[4])
	
	if (argv[argcount] == NULL) 
	  {
	  printf("\n\t ### ERROR ### A fourth argument is needed \n");
	  printf("\nThe fourth argument is the window size and blocksize to use (often 7,31)\n");
	  printf("\nUSAGE: > homogen_g input_ima output_ima HOMOVAR IWS,IBS \n\n"); 
	  exit(1);
	  }

	//windsize[0] = strtol(argv[4],NULL, 10);
	//printf("\nAs per user, filtering window will be of size %dx%d \n", windsize[0], windsize[0]);

// Check if a second smoothing is needed (e.g., argv[4] = 5,3)

	strncpy(temp, argv[argcount], 20);
	cptr = strtok(temp, ",");

	iwind = 0;
	while(cptr != NULL) 
	  {
 	  //printf("%s\n", cptr); 
	  windsize[iwind++]= strtol(cptr,NULL, 10);
	  cptr = strtok(NULL, " ,");
	  }

	//printf("\nDetection window will be of size %dx%d \n", windsize[0], windsize[0]);
	//printf("Reporting using a %dx%d blocksize.\n", windsize[1],windsize[1]);

	windsiz = windsize[0];
	blocksiz = windsize[1];

/* Check window size input parameters */

if (windsize[0] == 0)  windsiz = 7;			// default window size 
ofs = windsiz / 2 ;

if(windsiz == (windsiz/2)*2)
  {
  fprintf(stderr,"\n\n**Error: IWS must be odd (e.g., 3,5,7,... )\n\n");
  exit(-1);
  }

fprintf(stdout,"Phase I - Size of moving window being used: %dx%d\n", windsiz,windsiz);


/* Check block size input parameters */


/* if blocksiz not defined ("empty"), use default value of 31 */
if (windsize[1] == 0)  blocksiz = 31;
half = blocksiz / 2 ;

fprintf(stdout,"Phase II will summarize info by blocks of %dx%d around each pixel. \n\n", blocksiz, blocksiz);

if(blocksiz == (blocksiz/2)*2)
  {
  fprintf(stderr,"\n\n**Error: IBS must be odd (e.g., 11,21,31,... )\n\n");
   exit(-1);
  }
 
	
	//exit(1);		// useful when testing input parameters


//****************************************************************

// Registers for all types of files with GDAL and Open INPUT image file

//************************************

	GDALAllRegister(); 

// Open INPUT image file

	//ima_in = (GDALDataset *) GDALOpen( argv[1], GA_ReadOnly );
	ima_in = (GDALDataset *) GDALOpen( file_in, GA_ReadOnly );

	if (ima_in == NULL) 
	  	{printf("\n\n PROBLEM opening input image file %s \n\n", file_in); exit(1);}
	fprintf(stdout,"\n\t*File %s was opened for reading\n\n", file_in);

// Print generic info (driver used, ... )

	printf( "Driver: %s/%s\n",
          ima_in->GetDriver()->GetDescription(),
          ima_in->GetDriver()->GetMetadataItem( GDAL_DMD_LONGNAME ) );

 	Pixels = ima_in->GetRasterXSize();
	Lines = ima_in->GetRasterYSize();
	Channels = ima_in->GetRasterCount();
	xsize = Pixels;     ysize = Lines;

	printf( "Image size is %d x %d x %d\n\n", Pixels, Lines, Channels);

// Print geographic info

	//if( ima_in->GetProjectionRef() != NULL ) printf( "Full Projection is : '%s'\n\n", ima_in->GetProjectionRef() );


	proj = (char *) CPLMalloc(200);
	if( ima_in->GetProjectionRef() != NULL ) 
	  {
	  strncpy(temp, ima_in->GetProjectionRef(),30);
	  strtok(temp, "\"");
	  //printf("1st section : %s \n", temp);
 	  proj = strtok(NULL, "\"");
	  printf("Projection: %s \n", proj );
	  }


	if( ima_in->GetGeoTransform( adfGeoTransform ) == CE_None )
	  {
	  printf( "Origin = (%.6f,%.6f)\n", adfGeoTransform[0], adfGeoTransform[3] );
	  printf( "Pixel Size = (%.6f,%.6f)\n\n", adfGeoTransform[1], adfGeoTransform[5] );
	  }

	
// Open given channel

	piBand = ima_in->GetRasterBand(ch_in);

// Check raster type (8 or 16 bit) and set flag PCI data Type CHN_8U=1 CHN_16U=3

	if(EQUALN(GDALGetDataTypeName(piBand->GetRasterDataType()),"Byte",4)) data_type=CHN_8U;  	//PCI data types
	if(EQUALN(GDALGetDataTypeName(piBand->GetRasterDataType()),"Uint16",6)) data_type=CHN_16U;

	printf("Channel %d , RasterDataType =%d, Type = %s, PCI data_type = %d \n",
		ch_in, piBand->GetRasterDataType(), GDALGetDataTypeName(piBand->GetRasterDataType()), data_type );

// Set function to acquire pixel accordingly
/*
	if ( data_type==CHN_8U) get_pix_val = get_u8;
	if ( data_type==CHN_16U) get_pix_val = get_u16;
*/

	//exit(1);		// useful when testing input parameters

//*************************************************

// Open (and/or create) an OUTPUT image file name	

//*************************************************



// Use fullfilename not to perturbe argv[2] - cause some chr operators are destructive

	strcpy(fullfilename, file_out);

	filename = strtok(fullfilename,".");
	extension = strtok(NULL," "); 

	//printf("Filename:  %s \n", filename);
	//printf("Extension:  %s \n", extension);

	if (strncmp(extension,"tif",3) == 0)
	  poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
	if (strncmp(extension,"pix",3) == 0)
	  poDriver = GetGDALDriverManager()->GetDriverByName("PCIDSK");
	if (strncmp(extension,"dat",3) == 0)
	  poDriver = GetGDALDriverManager()->GetDriverByName("ENVI");

	if(poDriver == NULL) 
	   {
	   printf("\n ###Cant find proper driver for this file type %s \n\n", extension); 
	   exit( 1 );
	   }


// First, check if OUTPUT file already exist (ask to overwrite)

	ima_out = (GDALDataset *) GDALOpen( file_out, GA_Update );

	if (ima_out != NULL) 
	  {
	  printf("\n\n ### Image %s already exist \n", file_out); 
	  printf("\n\t OK to overwrite FULL image file (Y/N)? \t");  fgets(ans,80,stdin);

	  if (ans[0] == 'y' || ans[0] == 'Y') goto MAIN;

	  //if (ans[0] == 'n' || ans[0] == 'N')  {printf("\n\t** Will not overwrite anything ** \n"); goto Exit;}

	  if (ans[0] == 'n' || ans[0] == 'N')
		{
		printf("\n\tWould you like to overwrite a specific raster channel? \t");  fgets(ans,80,stdin);

	  	if (ans[0] == 'n' || ans[0] == 'N')  {printf("\n\t** Will not overwrite anything ** \n"); goto Exit;}
	  	if (ans[0] == 'y' || ans[0] == 'Y') 
		  {
		  printf("\n\tWhich channel would you overwrite ?? \t");  fgets(ans,80,stdin);
		  //ch_out = (int) ans[0] - 48;		// temporary
		  ch_out = strtol(ans,NULL, 10);

		  printf("\n\t\t ### OUTPUT channel %d will be overwriten or created?? \n\n", ch_out);
		  goto MAIN;
		  }

		}

	  }


//  Create the output image file in which to write one channel

CREATE:
	if(data_type==CHN_8U)
	  ima_out = (GDALDataset *) poDriver->Create( file_out, Pixels, Lines, no_ch_out, GDT_Byte, NULL );

	if(data_type==CHN_16U)
	  ima_out = (GDALDataset *) poDriver->Create( file_out, Pixels, Lines, no_ch_out, GDT_UInt16, NULL );

	if (ima_out == NULL) 
	  {printf("\n\n PROBLEM opening output image file %s \n\n",file_out); exit(1);}

	fprintf(stdout,"\n\t*File %s was created and is opened for writing\n",file_out);


// Copy the metadata (GeoTransform, projection, ...)

	fprintf(stdout,"\n\t*Writing metadata to output file %s\n",file_out);	

	ima_out->SetGeoTransform(adfGeoTransform);
	ima_out->SetProjection(ima_in->GetProjectionRef() );


	//exit(1);		// useful when testing input parameters
	

//***********************************************************************************

//		Main processing

//************************************************************************************


MAIN:

// Create buffers for reading/writing images

	printf("\nCreating image size buffers for reading/writing images\n"); 

// Allocate memory for images depending if 8b or 16b image 

  	//printf("*Allocating memory to the image\n");

	if(data_type==CHN_8U) 
		{
		image_8b = (PixVal *) CPLMalloc(sizeof(PixVal)*Pixels*(int64)Lines); check_mem(image_8b);
		image_8b_out = (PixVal *) CPLMalloc(sizeof(PixVal)*Pixels*(int64)Lines); check_mem(image_8b);
		}

	if(data_type==CHN_16U) 
		{
		image_16b = (uint16 *) CPLMalloc(sizeof(uint16)*Pixels*(int64)Lines); check_mem(image_16b);
		image_16b_out = (uint16 *) CPLMalloc(sizeof(uint16)*Pixels*(int64)Lines); check_mem(image_16b);
		}

// Info
	if(data_type==CHN_8U)
	  printf("\nAllocated %ld bytes to the input image (same for output)",Pixels*(int64)Lines);
	if(data_type==CHN_16U)
	  printf("\nAllocated %ld bytes to the input image (same for output) ",2*Pixels*(int64)Lines);

// Read the whole input image IN ONE SHOT 

	printf("\nReading the whole input image in one shot ...\n");

	if(by_image)
	{
 	printf("\n\n\t*Reading full input image into memory ... \n\n");

	if(data_type==CHN_8U) 
		piBand->RasterIO(GF_Read, 0, 0, Pixels, Lines, image_8b, Pixels, Lines, GDT_Byte, 0, 0 );
	if(data_type==CHN_16U) 
		piBand->RasterIO(GF_Read, 0, 0, Pixels, Lines, image_16b, Pixels, Lines, GDT_UInt16, 0, 0 );

	}		// end of if(by_image)


// TEST :  Just copy image for the moment ### OLD

/*
	printf("\n\tJust copy image for the moment ...\n");

	for ( i = 1 ; i <= Lines; i++ )		// For compatibility with PCI(x,y), I kept image coordinates start at (1,1)
	for ( j = 1 ; j <= Pixels ; j++ ) 		// However, GDAL images start at zero (similar to bitmaps), so bytenum=bitnum
	  {
	  bitnum = (i-1)*(int64)Pixels + j-1;		
	  image_8b_out[bitnum] = image_8b[bitnum];
	  if( (i/1000)*1000 == i ) printf("%d lines done\r", i);
	  }
	goto IMA_OUT;
*/


//************************************************************************************


/* Allocate space for intermediate image */

fprintf(stdout,"\n\tAllocating same amount of memory for an intermediate image\n");

if (data_type == CHN_8U)  Int_Ima = (PixVal *) CPLMalloc(sizeof(char) * xsize * ysize);
if (data_type == CHN_16U) Int_Ima = (int32 *) CPLMalloc(sizeof(int32) * xsize * ysize);

check_mem(Int_Ima);


 
 /* Certain functions are only worth doing by blocks (i.e., by bigger areas) */
 
  if(blocksiz == 0) skip_phaseII = 1;

  if (!strncmp(homovar,"MIN10",5))  {phase3_only = 1; goto PhaseIII;}

  if (!strncmp(homovar,"MAX10",5))  {phase3_only = 1; goto PhaseIII;}
  
  
  //exit(-1);

/**********************************************************
	OPERATION ON CHANNEL
************************************************************/

PhaseI:

//IMPTime(timedate,4);
time (&rawtime);  timeinfo = localtime (&rawtime); strcpy(timedate,asctime(timeinfo));
fprintf(stdout,"\nFirst processing (Phase I) loop ... \n\n"); 
fprintf(stdout,"\nGathering information from a window of size: %dx%d - %s \n", windsiz,windsiz,timedate);

for ( i = 1+ofs ; i <= ysize-ofs ; i ++ )	/* image start at (1,1) */
for ( j = 1+ofs ; j <= Pixels-ofs ; j ++ ) 
  {
 
   /* calculate window mean and gather max and min */
  
  sum = 0 ; max = 0 ; min = 65000;
  for ( ii = i-ofs ; ii <= i+ofs ; ii++ )
  for ( jj = j-ofs ; jj <= j+ofs ; jj++ ) 
	{
	pixnum = (int64)xsize*(ii-1) + jj-1;  			/* pixnum starting at zero */

	//pix = (*get_pix_val) (Image, pixnum);			
	if (data_type == CHN_8U) pix = (int) *( (uchar *)image_8b + pixnum);		// it is faster then above by1/3
	  else pix = (int) *((int *)image_16b + pixnum);

	sum += pix;
	if(pix < min) min=pix;			/* minimum of window */
	if(pix > max) max=pix;			/* maximum of window */
	}
  mean = sum / (windsiz*windsiz);
	
  /* calculate variance around centre pixel */
  
  sum =0 ;
  for ( ii = i-ofs ; ii <= i+ofs ; ii++ )
  for ( jj = j-ofs ; jj <= j+ofs ; jj++ ) 
	{	
	pixnum = (int64)xsize*(ii-1) + jj-1;  		
	//pix = (*get_pix_val) (ima_in, pixnum);	

	if (data_type == CHN_8U)  pix = (int) *( (uchar *)image_8b + pixnum);
	  else pix = (int) *((int *)image_16b + pixnum);

	sum += (pix-mean)*(pix-mean);
	}
	
  variance = sum / (windsiz*windsiz-1);
  
  /* Output results to intermediate image */

   pixnum = (int64)xsize*(i-1) + j-1; 
 

  /* Intermediate image based on mean within a IWS*IWS window around pixel */

  if (!strncmp(homovar,"MEAN",4))
  {
 	if (data_type == CHN_8U) *((uchar *) Int_Ima + pixnum) =  (uchar) mean ;
	else *((int32 *) Int_Ima + pixnum) =  (int32) mean;
  }  

  /* Intermediate image based on Variance  within a IWS*IWS window around pixel */
  if (!strncmp(homovar,"VARI",4)) 
  {
 	if (data_type == CHN_8U) *((uchar *) Int_Ima + pixnum) =  (uchar) variance ;
	else *((int32 *) Int_Ima + pixnum) =  (int32) variance;
  }  
  
  /* Intermediate image based on Normalized variance  within a IWS*IWS window around pixel */
  if (!strncmp(homovar,"NVAR",4))
  {
 	if (data_type == CHN_8U) *((uchar *) Int_Ima + pixnum) =  (uchar) (variance/mean) ;
	else *((int32 *) Int_Ima + pixnum) =  (int32) (variance/mean);
  }  
 
  /* Intermediate image based on standard deviation within a IWS*IWS window around pixel */
  if (!strncmp(homovar,"STDEV",5)) 
  {
 	if (data_type == CHN_8U) *((uchar *) Int_Ima + pixnum) =  (uchar) sqrt(variance) ;
	else *((int32 *) Int_Ima + pixnum) =  (int32) sqrt(variance);
  }  
 
  /* Intermediate image based on coefficient of variation within a IWS*IWS window around pixel */
  if (!strncmp(homovar,"COVAR",5)) 
  {
 	if (data_type == CHN_8U) *((uchar *) Int_Ima + pixnum) =  (uchar) (sqrt(variance)*100)/mean ;
	else *((int32 *) Int_Ima + pixnum) =  (int32) (sqrt(variance)*100)/mean;
  }  
    
  /* Intermediate image based on minimum within a IWS*IWS window around pixel */
  if (!strncmp(homovar,"MIN",3)) 
  {
 	if (data_type == CHN_8U) *((uchar *) Int_Ima + pixnum) =  (uchar) min ;
	else *((int32 *) Int_Ima + pixnum) =  (int32) min;
  }  
 	
  /* Intermediate image based on maximum within a IWS*IWS window around pixel */
  if (!strncmp(homovar,"MAX",3)) 
  {
 	if (data_type == CHN_8U) *((uchar *) Int_Ima + pixnum) =  (uchar) max ;
	else *((int32 *) Int_Ima + pixnum) =  (int32) max;
  }  
  

  /************* TEMP RESEARCH MODS  *****************/
  
  /* Candidates for possible VFOL adjustable lower threshold 
  at this point in time, only good for 8bit images (cause output is 8 bit ) */
  
   /* Int_Ima[pixnum] =  (mean - sqrt(variance)); */
   
   /* Int_Ima[pixnum] =  (mean); */
   
   /* Int_Ima[pixnum] =  (mean - 2*sqrt(variance)); */

  /************* END of TEMP RESEARCH MODS  *****************/
   

  if ((j==5)&&(i==((i/1000)*1000))) fprintf(stdout,"Phase I - %d lines done \r",i);
    
  }	/* end of scanning the image */
  
   fprintf(stdout,"Phase I - %d lines done \n\n",Lines);
  
 
/*
if(skip_phaseII)  		// move results from intermediatre image to output image
  {
	for ( i = 1+ofs ; i <= ysize-ofs ; i ++ )		// image start at (1,1) 
	for ( j = 1+ofs ; j <= Pixels-ofs ; j ++ ) 
	{
	pixnum = (int64)xsize*(i-1) + j-1; 
	//if (data_type == CHN_8U) image_8b_out + pixnum = *((PixVal *) Int_Ima + pixnum);
		//else *((int32 *) image_16b_out + pixnum = Int_Ima + pixnum);	  
		
	if (data_type == CHN_8U)  (PixVal *) (image_8b_out + pixnum) = (PixVal *) (Int_Ima + pixnum);
		//else *((int32 *) image_16b_out + pixnum = Int_Ima + pixnum);			
			
	}
  goto IMA_OUT;
  }  
*/

/************************************************************

	Phase II

**********************************************************/

//if(skip_phaseII) goto End_of_PhaseI;

//if(skip_phaseII) goto IMA_OUT;

/* Summarizing by blocks of IBSxIBS around each pixel. */

PhaseII:

/* 

// Zero "Image" which will now be used as output image


for ( i = 1 ; i <= ysize ; i ++ )	// image start at (1,1) 
for ( j = 1 ; j <= Pixels ; j ++ ) 
  {
  pixnum = (int64)xsize*(i-1) + j-1;
  if (data_type == CHN_8U) *((uchar *) ima_in + pixnum) = 0 ;
  else *((int32 *) ima_in + pixnum) =  0;
  }

*/


// Do Phase II summary by blocks (working from Intermediate Image)

ofs = blocksiz / 2 ;

//IMPTime(timedate,4);
time (&rawtime);  timeinfo = localtime (&rawtime); strcpy(timedate, asctime(timeinfo));
fprintf(stdout,"\n\nSecond processing (Phase II) loop ... \n"); 
fprintf(stdout,"Summarizing mean texture feature by blocks of %dx%d - %s \n", blocksiz, blocksiz,timedate);
fprintf(stdout,"Painting mean texture by blocks of 10x10 pixels  (so not too blocky)\n\n");

for ( i = 1+ofs ; i <= ysize-ofs ; i+=10 )	
for ( j = 1+ofs ; j <= Pixels-ofs ; j+=10) 
  {
   /* calculate block mean, min, max ... */ 
  sum = 0 ; max = 0 ; min = 65000;
  for ( ii = i-ofs ; ii <= i+ofs ; ii++ )
  for ( jj = j-ofs ; jj <= j+ofs ; jj++ ) 
	{
	pixnum2 = (int64)xsize*(ii-1) + jj-1;  		/* pixnum2 starting at zero */
	/* pix = (int) Int_Ima[pixnum2];*/
        if (data_type == CHN_8U) pix = *((uchar *) Int_Ima + pixnum2);
           else pix = *((int32 *) Int_Ima + pixnum2);
	sum += pix;
  	if(pix < min) min=pix;			/* minimum of block */
	if(pix > max) max=pix;			/* maximum of block */
	}
  mean = sum / (blocksiz*blocksiz);
  pixout = mean;
  if (!strncmp(homovar,"MIN",3)) pixout = min;
  if (!strncmp(homovar,"MAX",3)) pixout = max;

  // Paint a 10x10 area with same info. around pixel of interest (so not too blocky)

  for ( ii = i-5 ; ii < i+5 ; ii++ )
  for ( jj = j-5 ; jj < j+5 ; jj++ ) 
  {
  pixnum = (int64)xsize*(ii-1) + jj-1;
  if (data_type == CHN_8U) *((uchar *) image_8b_out + pixnum) =  (uchar) pixout ;
    else *((int32 *) image_16b_out + pixnum) =  (int32) pixout;
  }

  if ( (j == 5) && (i-1 == (((i-1)/1000)*1000) ) ) fprintf(stdout,"Phase II - %d lines done \r",i);
 

  }	/* end of scanning the image for block by block summaries */
  
  
 fprintf(stdout,"Phase II - %d lines done \n\n",Lines);

//goto End_of_PhaseII;
 
	goto IMA_OUT;
/**********************************************************/


PhaseIII:

/* Do Phase III summary by blocks of a few features that skip Phase I and II*/
/* Certain functions are only worth doing by blocks (i.e., by bigger areas) */

ofs = blocksiz / 2 ;

//IMPTime(timedate,4);
time (&rawtime);  timeinfo = localtime (&rawtime); strcpy(timedate,asctime(timeinfo));
fprintf(stdout,"\n\nPhase III processing loop (N.B.: Phase I&II were skipped) - %s \n", timedate); 
fprintf(stdout,"Estimating %s within blocks of %dx%d around each pixel. \n\n", homovar, blocksiz, blocksiz);


/* Do it every 10 pixels for faster runs in FAST testing mode 

for ( i = 1+ofs ; i <= ysize-ofs ; i+=10)	
for ( j = 1+ofs ; j <= Pixels-ofs ; j+=10 ) */

for ( i = 1+ofs ; i <= ysize-ofs ; i ++ )	
for ( j = 1+ofs ; j <= Pixels-ofs ; j ++ ) 

  {
   /* calculate block mean, min, max ... */
  
  sum = 0 ; max = 0 ; min = 65000;
  for ( ii = i-ofs ; ii <= i+ofs ; ii++ )
  for ( jj = j-ofs ; jj <= j+ofs ; jj++ ) 
	{
	pixnum2 = (int64)xsize*(ii-1) + jj-1;  		/* pixnum2 starting at zero */
	/* pix = (int) Int_Ima[pixnum2];*/
    if (data_type == CHN_8U) pix = *((uchar *) image_8b + pixnum2);
	 else pix = *((int32 *) image_16b + pixnum2);
	sum += pix;
  	if(pix < min) min=pix;			/* minimum of block */
	if(pix > max) max=pix;			/* maximum of block */
	}
  mean = sum / (blocksiz*blocksiz);
 

/*  case for MIN10 situation  */

  if (!strncmp(homovar,"MIN10",5))
  {
    if (data_type == CHN_8U) pix = *((uchar *) image_8b + pixnum);
      else pix = *((int32 *) image_16b + pixnum);
    if ( pix < (min + 0.1*(max-min)) ) min10 = pix; else min10 = 0;
    pixout = min10;
  }

/*  case for MAX10 situation  */

  if (!strncmp(homovar,"MAX10",5))
  {
    if (data_type == CHN_8U) pix = *((uchar *) image_8b + pixnum);
      else pix = *((int32 *) image_16b + pixnum);
    if ( pix > (max - 0.1*(max-min)) ) max10 = pix; else max10 = 0;
    pixout = max10;
  }

  /* OUTPUT results to output image */


    pixnum = xsize*(i-1) + j-1;
    if (data_type == CHN_8U) *((uchar *) image_8b_out + pixnum) =  (uchar) pixout ;
    else *((int32 *) image_16b_out + pixnum) =  (int32) pixout;


/* Paint a 10x10 area with same info. around pixel of interest 

  for ( ii = i-5 ; ii < i+5 ; ii++ )
  for ( jj = j-5 ; jj < j+5 ; jj++ ) 
  {
    pixnum = xsize*(ii-1) + jj-1;
    if (data_type == CHN_8U) *((uchar *) Int_Ima + pixnum) =  (uchar) pixout ;
    else *((int32 *) Int_Ima + pixnum) =  (int32) pixout;
  }
*/

  if ( (j == 5) && (i-1 == (((i-1)/1000)*1000) ) ) fprintf(stdout,"Phase III - %d lines done \r",i);
 

  }	/* end of scanning the image for block by block summaries */

fprintf(stdout,"Phase III - %d lines done \n\n", Lines);


//****************************************

// Write output image in ONE SHOT

//****************************************

IMA_OUT:

// Time to check memory usage with task manager

/*	  printf("\nCheck memory use. OK to continue(y/n): ");
	  scanf("%s",ans);			// gets any answer
	  if(ans[0] == 'n') exit(-1);
*/
	

	printf("\n\t* Writing output image to file \"%s\" \n\n", file_out);

	poBand = ima_out->GetRasterBand(ch_out);
	
	printf("Output Channel %d will be of Type = %s\n", ch_out, GDALGetDataTypeName(poBand->GetRasterDataType()) );	

	
// Full image in one shot

    if(data_type==CHN_8U)
	poBand->RasterIO(GF_Write, 0, 0, Pixels, Lines, image_8b_out, Pixels, Lines, GDT_Byte,0, 0 );
    if(data_type==CHN_16U)
	poBand->RasterIO(GF_Write, 0, 0, Pixels, Lines, image_16b_out, Pixels, Lines, GDT_UInt16,0, 0 );


//	out_line = (PixVal *) CPLMalloc(sizeof(PixVal)*Pixels);		// for 8bit output image 
	
// Image out line by line  TESTING #####
/*
	for (int y = 0; y < Lines; y++)
	  {
	  for (int x = 0; x < Pixels; x++) out_line[x] = image_8b_out[y*Pixels+x];
	  poBand->RasterIO(GF_Write, 0, y, Pixels, 1, out_line, Pixels, 1, GDT_Byte,0, 0 );
 	  if( (y/1000)*1000 == y ) printf("%d lines done\r", y);
	  }
*/


// Writing description (history) ...
 
	sprintf(ima_description,"Homogen_g/%s (%d,%d) from %s CH %d \0", homovar, windsiz, blocksiz, file_in, ch_in);
	
	poBand->SetDescription(ima_description);
	printf( "\nDescription of output: %s\n", ima_description);
	
// Close input and output images

Exit:	printf("\nClosing both image files and exiting program. \n");

	GDALClose(ima_in);
	GDALClose(ima_out);

time (&rawtime);
timeinfo = localtime (&rawtime);
fprintf(stdout,"\n\n_______________________________\n");
fprintf(stdout,"\n %s (%s) finished at %s\n\n", PROG_NAME, VERSION,  asctime(timeinfo));
	
// For people using this program via ArcGIS, give then some time to examine the results (before disappearing)

if  (EQUALN("ArcGIS ",argv[argc-1],3) )
	{
	fprintf(stdout,"\n\n######\n");
	printf("\n Type anything to make this detailed window disappear and terminate %s ",PROG_NAME);
	ans[0] = getc(stdin); 		// gets any answer or <CR>
    }
	
	
	exit(0);				// exit properly 
} 		// end of main function

//**************************************************************

// check return value from MALLOC to make sure it is OK 

void check_mem(void *memory_ptr)
{
	if (memory_ptr == NULL )
	{
	fprintf(stderr,"\n ITC program - MEMORY ALLOCATION ERROR. \n");
	exit(-1);
	}
}

//**************************************************************
//**************************************************************


/*************************************************************************/

/* This function checks that the input parameter HOMOVAR is valid.  
   Invalid parameters will produce an error message and cause program exit. 
*/

void check_param(char *homovar)
{
	upper_case(homovar);

	/* Check BITBOUND */

	if ( (strcmp(homovar,"MEAN") != 0) && (strcmp(homovar,"VARI") != 0)
		&& (strcmp(homovar,"NVAR") != 0) && (strncmp(homovar,"STDEV",5)!= 0)
		&& (strncmp(homovar,"COVAR",5) != 0) && (strncmp(homovar,"MIN",3) != 0) 
		&& (strncmp(homovar,"MAX",3) != 0))
	  {
	  fprintf(stderr, "Invalid entry for the HOMOVAR parameter.\n");
	  fprintf(stderr, "Please use one of: MEAN,VARI,NVAR,STDEV,COVAR,MIN,MAX\n");
	  exit(-1);
	  }

}


/*************************************************************************/

/* This function converts a string to upper case characters */

void upper_case(char *c)
{
	while (*c != '\0') {
		if (*c >= 'a' && *c <= 'z') *c += ('A' - 'a');
		c++;
	}
}

/******************************************************************/
