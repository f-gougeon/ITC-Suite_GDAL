/* 
Program name: 	ima_thr_g.cpp

Author: 	François A. Gougeon

Description:

	To threshold (a range) an image channel, typically to produce the non-forest mask
	needed for itcvfol_g of the ITC-Suite (similar to IMTHRES circa 1995-2015)

	François A. Gougeon, Ph.D.
	Remote Sensing Research	
	Natural Resources Canada
	Canadian Forest Service 
	Pacific Forestry Centre
	506 West Burnside Rd.
	Victoria, British Columbia, 
	Canada, V8Z 1M5	

History:


 v1.0a	June  2021	François Gougeon

	- Quick modification of itc_afav_g.cpp
	
	- For people that dont have easy access to an image analysis system, 
	there is a need for a thresholding program, mostly to create a non-forest mask.
	For example, a simple threshold on the NIR band, a NDVI image, or on a DCM, 
	could create a decent initial non-forest mask.
	
	- Like most of my ITC programs, it deals with 8-bit or 16-bit input images and
	specific channel in multi-channel .pix or .tif input files
	
	- Using base file name to create its name, output image will be a 1-bit tif, 
	for example:  Secteur_THR2.tif				# original channel no. is in file name


 v1.1a	June  2021	François Gougeon

	- Modification to add (i.e., OR) results to an existing bitmap in order to possibly do
	  a series of thresholds on different channels (and/or a Digital Canopy Model(DCM))
	  and add to the same resulting bitmap

 v1.2	Nov.  2021	François Gougeon

	- Modifications for stability 
		
	
v1.3	Dec  2021	François Gougeon

	- Playing with output bitmap color table (via itc_io_g / write_bitmap()) to suit PCI, ArcGIS, ...
	
	- Org. so that user can create an new output bitmap, 
		but with a specified name (no automatic name)
	  In other words, the output filename can be specified whether the user wants to "add to it" 
	  OR create a new one.
		

 François Gougeon  v1.4a		May 2023

			- NOT to assume that all files are in the default directory from which the program is run ANYMORE
				Previously, everything was assumed in same directory and run from a cmd window from there.
				NOW, 
				the **path used with the main input file** is used when creating the default input/output file names
				This was necessary for ArcGIS Toolkit integration.

			- To rely on automatic name for output file the user can used "-" or "#" as second argument
			which implies no input bitmap to "add to" (# was necessary for ArcGIS Toolkit integration).
			
François Gougeon  v1.5a		Nov. 2023

			- Because we now have BMCOMBO_G that allow bitmaps to be combined (typically to create the 
			ultimate non-forest mask) there is no need to add to an existing bitmap anymore
			and this was creating confusion on reruns, so removed that feature
			
François Gougeon  v1.6	July 2026

			- Mod. to deal more flexibly with argv using argv[argcount] in order to specify 
			a specific input channel as a separate param or following main file name with a comma


	
	
c***************************************************************	
	
#### PROGRAM USAGE

### Program will create a default output file name

> ima_thr_g Main_File.ext,CH# - thres1,thres2  	

### IF adding(OR) to an existing bitmap, or wanting a specific output file name

> ima_thr_g Main_File.ext,CH# Secteur_NonFor.tif thres1,thres2  




Using the base file name to create its name, output will be a 1-bit tif (i.e., bitmap)
for example:  Secteur_THR2.tif					# channel no. 2 is used in the file name
	


*******************************************************************************

*** To compile with Visual Studio (VS14)  (see VS14_GDAL_compile.txt)

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64

set INCLUDE=C:\gdal-2.1.1_v2\include;C:\libtiff-4.0.6_64b\tiff-4.0.6\libtiff;%INCLUDE%
set LIB=C:\gdal-2.1.1_v2\lib;C:\libtiff-4.0.6_64b\tiff-4.0.6\libtiff;%LIB%

set LINK=gdal_i.lib  libtiff_i.lib User32.lib

set CL= /MD

CL xxxxxxxxx.cpp /EHsc

	 Partially static (needs gdal300.gll to run)

set CL= /MD
set LINK=gdal_i.lib tiff.lib User32.lib /subsystem:console /incremental:no /machine:x64 /STACK:0X200000


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



*********************
Ackowlegment to GDAL:

GDAL - Geospatial Data Abstraction Library: Version 2.1.1 (July2016, 64bit), 
GDAL - Geospatial Data Abstraction Library: Version 3.0.0 (Dec. 2019, 64bit),
Open Source Geospatial Foundation, 
Thanks Frank (Warmerdam)


*******************************************************************************************

	- NOTE: Many more global variables needed to use function in itc_io_g
	
********************************************************************************
********************************************************************************
*/

#define VERSION "v1.6"
#define PROG_NAME "IMA_THR"
#define FILENAME 250


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




//#include "cpl_string.h"
// #include "tiffio.h"

/*
typedef unsigned char  PixVal;			// all in ITC-Suite_g.h and itc_io_g.h now
typedef long           int64;
typedef unsigned long  uint64;
typedef unsigned short     uint16;
int  CHN_8U = 1, CHN_16U = 3;

*/

// Global declarations - Tons are needed to use itc_io_g.cpp

int		Pixels, Lines, Channels;	// pixel and line starting at 1,1
int		xsize, ysize;			// pixel and line starting at 0,0
int 	data_type;

int64 	bmsize;		// size of bitmap in bytes 

int	bylines_flag = 0;		// by default proceed by full images (not by lines)
int	by_lines=0, by_image=1;			// default is to read/write by image (faster),


float		xpixsz, ypixsz;
char 		*Proj, *Proj2, *Datum, *Datum2, *Temp,*token;

double		adfGeoTransform[6], adfGeoTransform2[6];
double 		topleftX, transformX, topleftY, transformY; 	 /* for geographic mapping */

int 		ch_in, ch_out, in_ch[10], segm_in, in_segm[10],thres[2];


GDALRasterBand	*piBand,*piBand2,*poBand;
char **papszOptions = NULL;


int imaFile_opened;
GDALDataset	*ima_in, *seg_in;



//char 	description[80];
char 	Extension[10]; 		// global variable for other prog.

char	Description[200];		// for output image


int	xcg, ycg;	

PixVal *ima_buf_in, *ima_buf_out;
PixVal * maskbitbuf;


time_t rawtime;
struct tm * timeinfo;


//PIX_FUN_PTR	get_pix_val; 		 // pointer that allows us to deal with  many types of image (8,16u,16s) 

//************************************

int main(int argc, char* argv[])
{

GDALDriver 	*piDriver, *poDriver;
GDALRasterBand	*piBand, *poBand;
//double		adfGeoTransform[6];
char 		**papszMetadata;

int  	ithres;
float 	sum;

int 	ch_out=1, ch_in=1, no_ch=1, dbic[10], ch_no;

int  	iwind, windsiz[10];

int	 argcount;

char 	 ans[80], answer[10];
char	fullfilename[FILENAME], file_in[FILENAME];
char 	file_out[FILENAME];
char 	*filename, *extension, temp[FILENAME];
char 	*tstring, *p, ach_in[5];
char 	Proj3[30];

char	basefname[FILENAME];						// ** just a pointer **
char	basefilname[FILENAME];
int		basef_len;	

char  	*cptr;			// generic pointer to char

PixVal		*pafScanline, *image_8b, *image_8b_out;
uint16		*pafScanline16, *image_16b, *image_16b_out;

int	by_lines=0, by_image=1;			// default is to read/write by image (faster),


int	i, j, ii, jj, ofs, count;
int64 	bitnum, bitnum2;
int commaFlag = 0;


//Proper Geo projection of main image

Proj = (char *) CPLMalloc(2000);			// to store geo projection info
// Datum = (char *) CPLMalloc(2000);		// to store Datum info
// Temp = (char *) CPLMalloc(2000);			


/* Print Program Header and time */

	time (&rawtime);
	timeinfo = localtime (&rawtime);

	fprintf(stdout,"\n\t\tStarting %s (%s) at %s\n", PROG_NAME, VERSION, asctime(timeinfo));

//***************************

// Check input parameters (i.e., agrv[*])

//***************************
// Check *First argument* on command line (here, Input file name)

	argcount = 1;		// use "argcount" to move more flexibly between arguments
	
	//printf("\n\tPresent parameters are %s %s %s %s\n\n", argv[1], argv[2], argv[3], argv[4]);

	if (argv[argcount]== NULL) 
	  {
	  printf("\n\t PROBLEM with input image %s \n",argv[argcount]);
	  printf("Have an INPUT image as first argument on command line \n\n");
	  printf("USAGE: ima_thr_g Main_File.ext,CH# - thres1,thres2 \n");
	  printf("USAGE: ima_thr_g Main_File.ext CH# - thres1,thres2 \n");
	  printf("USAGE: ima_thr_g Main_File.ext,CH# outputBM thres1,thres2 \n\n");
	  exit(-1);
	  }  

	if (argc < 3)	  
	  {
	  printf("\n\t PROBLEM with input parameters \n\n");
	  printf("USAGE: ima_thr_g Main_File.ext,CH# - thres1,thres2 \n");
	  printf("USAGE: ima_thr_g Main_File.ext CH# - thres1,thres2 \n");
	  printf("USAGE: ima_thr_g Main_File.ext,CH# outputBM thres1,thres2 \n\n");
	  exit(-1);
	  }  



// Check if channel number(s)is connected to file name by a comma

	ch_in = 1;						// default 
	strcpy(temp, argv[argcount]);  
	//printf(" Temp :  %s \n", temp);
	cptr = strtok(temp, ", "); 			// check for comma after extension
	cptr = strtok(NULL, ", ");			// got after the comma	
	//printf(" temp :  %s \n", temp);		// clean file name
	//printf(" cptr :  %s \n", cptr);
	strcpy(file_in, temp);	
	printf(" Input Filename :  %s \n", file_in);
	
	if(cptr != NULL)					// comma after extension
	{
	commaFlag = 1;	
	ch_no = strtol(cptr,NULL, 10);			// change to integer
	//printf(" After comma : %d\n", ch_no);	

	ch_in = dbic[0] = ch_no ;				// for PCI code coompatibility
	//printf(" Channel to use : %d \n", dbic[0]);

// Some programs may have a second comma and a second item	*NOT THIS ONE*

	cptr = strtok(NULL, ", ");			// check for another channel 
	if(cptr != NULL) {ch_no = strtol(cptr,NULL, 10); dbic[1]= ch_no; no_ch = 2;}					
	if(no_ch == 2) printf("Secondary bitmap to use : %d \n", dbic[1]);

	if ( no_ch > 1) 
	  {
	  fprintf(stdout,"\n\t##### ERROR - Number of input bitmap to use  must be only one #####\n");
	  //printf("\n\tYou present parameters are %s %s %s %s\n\n", argv[1], argv[2], argv[3], argv[4]);
	  exit(-1);
	  }

	}
	
// Channel numbers may be as a SEPARATE PARAM  on the cmd line (OR NADA)
	
	if(!commaFlag)		
	  { 
	  argcount++;					 // get next argument	  
	  //printf("Argument %d: %s \n", argcount, argv[argcount]);
	  strcpy(temp, argv[argcount]);	  
	  ch_no = strtol(temp,NULL, 10);			// change to integer  
	  if(ch_no == NULL) { ch_no = 1; argcount--; }		// next argument was probably a file name 
	  }	
	  
	  ch_in = ch_no;		// ch_in is used in rest of prog
	  
	  // printf(" Input channel to use : %d \n", ch_no);
	  // printf(" Input channel as separate arg   : %d \n", ch_in);
	  // exit(-1);				// for debugging



//***************************	  
	

// Create base file name   
	
// Need to get "basefilename" when full path is involved
// Basefile name has path + head of file name (typically correspond to "named area" of study e.g. PRF)
// Area name is assumed separated from rest of file name by an underscore
// However, be careful as there could be underscores in the path

	strcpy(fullfilename,argv[1]);			// main input filename (and possibly its dir)
	
	p = strtok(fullfilename, ","); 	 		// get rid of comma and item (channel) after comma

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


//	exit(-1);			// for debugging


  argcount++;					 // get next argument

//*************************************************

// Open (or create) an OUTPUT image file name	
//	For example:  Secteur_THR1.tif if thresholding was done on channel 1 
// More complex name if adding to existing bitmap

//*************************************************

// if no output bitmap, use main image file name as base to create output bitmap name (+ CH no.)


if ( EQUALN(argv[argcount],"-",1 ) || EQUALN(argv[argcount],"#",1 ) )
	
	{
	printf("\nAn output bitmap file will be created based on \"base file name\" \n"); 
			  
	//basefname = strtok(fullfilename,".");
	//basefname = strtok(basefname,"_");				
	//printf("basefname  :  %s \n", basefname);	

	//basefname = temp;	 				// fake to init basefname as a char array (just a pointer)
	strcpy(file_out, basefilname);	
	
	strncat(file_out,"_THR",4); 
	//printf("file_out :  %s \n", file_out);   
	//itoa(ch_in,ach_in,10); 	 
	sprintf(ach_in, "%d" , ch_in);	 	
	strncat(file_out,ach_in,3);	  	
	strncat(file_out,".tif",4);					// output image is forced to be a tif	
	printf("\n\t** Output bitmap file will be named \"%s\" \n", file_out);
	}	
else								// if  input bitmap, just add to that name
	{
	//basefname = strtok(fullfilename,".");		// fake to init basefname
	//strcpy(basefname,argv[2]);
	//basefname = strtok(basefname,".");
	//file_out = temp;				// fake to init file_out as char array
	strcpy(file_out,argv[argcount]);	
	printf("\n\t** Output bitmap file is named \"%s\" \n", file_out);
	}

	//exit(-1);		// for degugging

 

    
	  argcount++;					 // get next argument (often argument four)
	  
	if (argv[argcount] == NULL) 
	  {
	  printf("\n\n PROBLEM with last argument%s \n",argv[argcount]);
	  printf("\tHave a threshold (or range) as third argument on command line \n\n");
	  printf("USAGE: ima_thr_g Main_File.ext,CH# - thres1,thres2 \n");
	  printf("USAGE: ima_thr_g Main_File.ext CH# - thres1,thres2 \n");
	  printf("USAGE: ima_thr_g Main_File.ext,CH# outputBM thres1,thres2 \n\n");
	  exit(-1);
	  } 



// ******
	
// Check if two thresholds are used is needed (e.g., argv[3] = 15,66)
	//printf("\n\t Argument three : %s \n\n", argv[3]);

	strcpy(temp, argv[argcount]);
	p = strtok(temp, ",");

	ithres = 0;
	while(p != NULL) 
	  {
	  thres[ithres++]= strtol(p,NULL,10); 	  
	  p = strtok(NULL, ",");
	  }
	  
	if(ithres == 1) 
			printf("As per user, the image will be threshold below %d\n", thres[0]);
	  
	if(ithres == 2)
		printf("As per user, the image will be threshold from %d to %d\n", thres[0], thres[1]);

	if(ithres > 2) 
		{printf("\n\n ### ERROR -- Only two threshold entries are acceptable \n\n"); exit(-1);}

		  
	//exit(-1);		// useful when testing only input parameters


//****************************************************************

// Registers for all types of files with GDAL and Open INPUT image file

//************************************

	GDALAllRegister(); 

// Open INPUT image file

	ima_in = (GDALDataset *) GDALOpen( fullfilename, GA_ReadOnly );

	if (ima_in == NULL) 
	  	{printf("\n\n PROBLEM opening input image file %s \n\n",fullfilename); exit(1);}
	
	fprintf(stdout,"\n\t*File %s was opened for reading\n\n",fullfilename);

// Print generic info (driver used, ... )

	// printf( "Driver: %s/%s\n",
          // ima_in->GetDriver()->GetDescription(),
          // ima_in->GetDriver()->GetMetadataItem( GDAL_DMD_LONGNAME ) );

 	Pixels = ima_in->GetRasterXSize();
	Lines = ima_in->GetRasterYSize();
	Channels = ima_in->GetRasterCount();
	xsize = Pixels;     ysize = Lines;
	printf( "Image size is %d x %d x %d\n", Pixels, Lines, Channels);
	bmsize = (Pixels * (int64)Lines + 7) / 8; 
	printf("Bitmap size (in bytes): %I64d \n", bmsize);	
	printf("Image Description: %s \n", ima_in->GetDescription() );

// Print geographic info

	if( ima_in->GetProjectionRef() != NULL )
	{
	//strncpy(Proj,ima_in->GetProjectionRef(),2000);			// get full projection in to copy to output
	strncpy(Proj,ima_in->GetProjectionRef(),strlen(ima_in->GetProjectionRef()));	// get full projection in to copy to output
	//printf("\nProjection is '%s'\n", Proj);
	strncpy(Proj3,ima_in->GetProjectionRef(),30);
	printf("\nProjection is '%s'\n", Proj3);
	}
	
	else printf("\nNote : No Geographic \"Projection\" within the main file\n");

	printf("Geographic Data within that file\n");
	
	if( ima_in->GetGeoTransform( adfGeoTransform ) == CE_None )
	  {
	  printf("Geographic info :\n");
	  printf( "Origin = (%.6f,%.6f)\n", adfGeoTransform[0], adfGeoTransform[3] );
	  printf( "Pixel Size = (%.6f,%.6f)\n\n", adfGeoTransform[1], adfGeoTransform[5] );
	  }

	  
	//exit(-1);		// for degugging

// Open input  channel

	piBand = ima_in->GetRasterBand(ch_in);
	
	printf("Input Channel Description: %s \n", piBand->GetDescription() );

// Check raster type (8 or 16 bit) and set flag PCI data Type CHN_8U=1 CHN_16U=3

	if(EQUALN(GDALGetDataTypeName(piBand->GetRasterDataType()),"Byte",4)) data_type=CHN_8U;  	//PCI data types
	if(EQUALN(GDALGetDataTypeName(piBand->GetRasterDataType()),"Uint16",6)) data_type=CHN_16U;

	printf("Channel %d , RasterDataType =%d, Type = %s, PCI data_type = %d \n\n",
		ch_in, piBand->GetRasterDataType(), GDALGetDataTypeName(piBand->GetRasterDataType()), data_type );


	  //exit(-1);		// for degugging


/*
//	Section was used when we had the choice of adding to an existing bitmap (choice removed in v 1.5)

if ( EQUALN(argv[2],"-",1 ) || EQUALN(argv[2],"#",1 ) )			// Make a fresh bitmap in memory
	  {
	  printf("\nReserving memory for new output bitmap\n");	
	  maskbitbuf = (PixVal *) calloc(bmsize,1);	// prep memory for output bitmap
	  check_mem(maskbitbuf);
	  }
 	else			// Open INPUT bitmap to add to (or new with given name)
	  {	
	  printf("\nOpening existing bitmap to add results to (to \"OR\" with) \n", file_out);
	  maskbitbuf = read_bitmap(argv[2], 1);	

	  if ( maskbitbuf == NULL ) 		// if no such file, need to create a new file
	    {
	    printf("Specified file \"%s\" does not exist. Thus, can't \"OR\" results with it \n", file_out);
	    printf("Will create a new output file(bitmap) named \"%s\", as per user request\n", file_out);
		
		//printf("\nReserving memory for new output bitmap\n");	
	    maskbitbuf = (PixVal *) calloc(bmsize,1);	// prep memory for output bitmap
	    check_mem(maskbitbuf);	 		
	    }
	
	  }
 */	   
	  
	//exit(-1);		// for degugging
	 
	
// With ave_filter.cpp output file was specified by the user 
//	With itcafav_f.cpp name is created. It is a tif file

/*
	printf("Filename:  %s \n", filename);
	printf("Extension:  %s \n", extension);
	if (strncmp(extension,"tif",3) == 0) printf("\nOutput file extension is 'tif' \n");
	if (strncmp(extension,"pix",3) == 0) printf("\nOutput file extension is 'pix' \n");
	if (strncmp(extension,"dat",3) == 0) printf("\nOutput file extension is 'dat' \n");

	if (strncmp(extension,"tif",3) == 0)
	  poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
	if (strncmp(extension,"pix",3) == 0)
	  poDriver = GetGDALDriverManager()->GetDriverByName("PCIDSK");
	if (strncmp(extension,"dat",3) == 0)
	  poDriver = GetGDALDriverManager()->GetDriverByName("ENVI");
*/
 

/* 
  	poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
	  
	if(poDriver == NULL) 
	   {
	   //printf("\n ###Cant find proper driver for this file type %s \n\n", extension); 
	   printf("\n ###Cant find proper driver for this file type %s \n\n", "GTiff"); 	   
	   exit( -1 );
	   }


// First, check if OUTPUT file already exist (ask to overwrite)

	ima_out = (GDALDataset *) GDALOpen( file_out, GA_Update );

	if (ima_out != NULL) 
	  {
	  printf("\n\n ### Output bitmap file %s already exist \n", file_out); 
	  printf("\n\t OK to overwrite that file (Y/N)? \t");  fgets(ans,80,stdin);

	  if (ans[0] == 'y' || ans[0] == 'Y') goto MAIN;

	  if (ans[0] == 'n' || ans[0] == 'N')  {printf("\n\t** Will not overwrite anything ** \n"); goto Exit;}

	  }



// Copy the metadata (GeoTransform, projection, ...)

	fprintf(stdout,"\n\t*Writing metadata to output file %s\n", file_out);	

	ima_out->SetGeoTransform(adfGeoTransform);
	ima_out->SetProjection(ima_in->GetProjectionRef() );

	//exit(-1);		// for degugging
	  	
*/
	
//***********************************************************************************

//	Main processing

//************************************************************************************


MAIN:	//exit(-1);		// for degugging

// Create buffers for reading/writing images

	printf("\nCreating image size buffers for reading/writing images\n"); 

// Allocate memory for images depending if 8b or 16b image 

  	//printf("*Allocating memory to the image\n");

	if(data_type==CHN_8U) 
		{
		image_8b = (PixVal *) CPLMalloc(sizeof(PixVal)*Pixels*(int64)Lines); check_mem(image_8b);
		image_8b_out = (PixVal *) CPLMalloc(sizeof(PixVal)*Pixels*(int64)Lines); check_mem(image_8b_out);
		}

	if(data_type==CHN_16U) 
		{
		image_16b = (uint16 *) CPLMalloc(sizeof(uint16)*Pixels*(int64)Lines); check_mem(image_16b);
		image_16b_out = (uint16 *) CPLMalloc(sizeof(uint16)*Pixels*(int64)Lines); check_mem(image_16b_out);
		}

// Info
	if(data_type==CHN_8U)
	  printf("\nAllocated %Id bytes to the input image (same for output)",Pixels*(int64)Lines);
	if(data_type==CHN_16U)
	  printf("\nAllocated %Id bytes to the input image (same for output) ",2*Pixels*(int64)Lines);





// Read the whole input image IN ONE SHOT (default NOW)

	printf("\nReading the whole input image in one shot ...\n");

	if(by_image)
	{
 	printf("\n\n\t*Reading full image into memory ... \n\n");

	if(data_type==CHN_8U) 
		piBand->RasterIO(GF_Read, 0, 0, Pixels, Lines, image_8b, Pixels, Lines, GDT_Byte, 0, 0 );
	if(data_type==CHN_16U) 
		piBand->RasterIO(GF_Read, 0, 0, Pixels, Lines, image_16b, Pixels, Lines, GDT_UInt16, 0, 0 );

	}		// end of if(by_image)


	//exit(-1);		// for degugging


//****************************
	
	
	// Preparing output bitmap memory
 
 
	  printf("\nReserving memory for new output bitmap\n");	
	  maskbitbuf = (PixVal *) calloc(bmsize,1);	// prep memory for output bitmap
	  check_mem(maskbitbuf); 
 
 
 
// Do the Thresholding of the image

	printf("\n\tDoing thresholding on image. Range is %d to %d ...\n\n",thres[0],thres[1]);

	
	count = 0;				// for debugging
	
// For 8bit images

	for ( i = 1 ; i <= Lines; i++ )		// For compatibility with PCI(x,y), I kept image coordinates start at (1,1)
	for ( j = 1 ; j <= Pixels; j++ ) 	
	  {

	  
	  bitnum = (i-1)*(int64)Pixels + j-1 ;	//GDAL images start at zero (similar to bitmaps), so bytenum=bitnum
	  clearbit(maskbitbuf,bitnum);
	  
	  if(data_type==CHN_8U)
	    if ( (image_8b[bitnum] >= thres[0]) &&  (image_8b[bitnum] <= thres[1]) ) 
			{setbit(maskbitbuf,bitnum); count++;}

	  if(data_type==CHN_16U)
	    if ( (image_16b[bitnum] >= thres[0]) &&  (image_16b[bitnum] <= thres[1]) ) 
			{setbit(maskbitbuf,bitnum);	 count++; } 
	  
	  if( (i/1000)*1000 == i ) printf("%d lines done\r", i);

	  }
	  
	printf("%d lines done\n", Lines);
	printf("%d pixel set\n", count);


//****************************************

// Write output image in ONE SHOT

//****************************************

OUT:	//exit(-1);		// for degugging

// Time to check memory usage with task manager

/*	  printf("\nCheck memory use. OK to continue(y/n): ");
	  scanf("%s",ans);			// gets any answer
	  if(ans[0] == 'n') exit(-1);
*/

	printf("\n\t* Writing output bitmap to \"%s\" \n", file_out);
	
	
/*	
	if ( EQUALN(argv[2],"-",1 ) || EQUALN(argv[2],"#",1 ) )
	  snprintf(Description, 120, "%s CH%d thresholded using range %d,%d ",fullfilename,ch_in,thres[0],thres[1]);
 	else	
	  snprintf(Description, 120, "%s CH%d thresholded using range %d,%d and added to %s", 
												fullfilename,ch_in,thres[0],thres[1],argv[2]);		
  */ 
  
  	snprintf(Description, 120, "%s CH%d thresholded using range %d,%d ",fullfilename,ch_in,thres[0],thres[1]);
	  
	printf("\nDescription: %s \n\n", Description );
	
	//printf("\n Calling write_bitmap()\n\n");
			
// Write out bitmap ( takes care of GeoTransform, projection, ...)		
			
	write_bitmap(maskbitbuf, file_out);	
	


// Close input and output images

Exit:	printf("\nClosing both image files and exiting program. \n");

	GDALClose(ima_in);
	//GDALClose(file_out);		// already close by write_bitmap()

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
//**************************************************************

