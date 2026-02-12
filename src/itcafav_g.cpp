/* 
Program name: 	itcafav_g.cpp (64bit) (for GDAL)

Author: 	François A. Gougeon

Description:

	To produce a smoothed image useful (needed) for itcvfol_g of the ITC-Suite

	To smooth various image areas "more or less" depending on needs.

	The process is guided by the small tree crown mask (STCMASK) and 
	the non-forest mask (NFMASK). Typically, it smoothed the small tree
	areas with a 3x3 average filter, and the big tree areas with a 5x5
	filter on top of the 3x3 smoothing.
	When finished, it writes the image to the output channel.
	
	François A. Gougeon, Ph.D.
	Remote Sensing Research	
	(©)Natural Resources Canada
	Canadian Forest Service 
	Pacific Forestry Centre
	506 West Burnside Rd.
	Victoria, British Columbia, 
	Canada, V8Z 1M5	

History:

	(Originally as ave_filter.cpp)

	- October 2016 (from NDVI_FG.cpp for thePCI environment)

	- June 2017 (cleaning up, improved, comments and re-tests)

	- Oct. 2018 Mods. to be more versatile on output (new file, overwrite file (or not), 
	overwrite channel of existing file (say a PCI file), ..

	As output file, deals with .tif, .pci, and ENVI (.dat) files


 v1.1a	April 2020	François Gougeon	(as ave_filter.cpp)

	- Lots of clean-up and simplifications
	
	- Org. to deal with input/output of 16bit images 

	- Better checking of user inputs

 v1.2a	May  2021	François Gougeon	(as itcafav_g.cpp)

	- Mod. of ave_filter.cpp to make it similar to AFAV, thus smooth different image sections differently
	based on NFMASK (non-forested areas) and STMASK (small tree areas)
	That is: non-forested areas are not smoothed, small tree areas are smoothed a bit, and the rest,
	considered large tree areas, is smoothed more (based on WINDSIZE)
	
	- NOTE: Many more global variables needed to use function in itc_io_g
	

c
c François Gougeon  v1.3a		May 2023
c
c			- NOT to assume that all files are in the default directory from which the program is run ANYMORE
c				Previously, everything was assumed in same directory and run from a cmd window from there.
c				NOW, 
c				the **path used with the main input file** is used when creating the default input/output file names
c				This was necessary for ArcGIS Toolkit integration.
c
c	
	
	
#### PROGRAM USAGE

> itcafav_g Main_File.ext,CH# NFMASK STCMASK WINDSIZ

Run as (for examples):


> itcafav_g Secteur_IRGB.tif Secteur_NF.tif Secteur_STC.tif 3,5		# channel 1 is implied

> itcafav_g Secteur_IRGB.tif,2 Secteur_NF.tif Secteur_STC.tif 3,5	# use channel 2 in tif file

Using base file name to create its name, output image will be a tif, 
for example:  Secteur_AFAV2.tif						# channel no. is in file name
	


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

********************************************************************************
********************************************************************************
*/

#define VERSION "v1.3a"
#define PROG_NAME "ITCAFAV"


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

int		bylines_flag = 0;		// by default proceed by full images (not by lines)


float		xpixsz, ypixsz;
char 		*Proj, *Proj2, *Datum, *Datum2, *Temp,*token;
double		adfGeoTransform[6], adfGeoTransform2[6];
double 		topleftX, transformX, topleftY, transformY; 	 /* for geographic mapping */

int 		ch_in, ch_out, in_ch[10], segm_in, in_segm[];


GDALRasterBand	*piBand,*piBand2,*poBand;
char **papszOptions;


int imaFile_opened;
GDALDataset	*ima_in, *seg_in;


char 	description[80];

char 	Extension[10]; 		// global variable for other prog.


int	xcg, ycg;	

PixVal *ima_buf_in, *ima_buf_out;



time_t rawtime;
struct tm * timeinfo;


//PIX_FUN_PTR	get_pix_val; 		 // pointer that allows us to deal with  many types of image (8,16u,16s) 

char	Description[80];		// for output image

//************************************

int main(int argc, char* argv[])
{

GDALDataset	*ima_in, *ima_out;
PixVal *nfmask, *stcmask;			// two mask bitmaps
GDALDriver 	*piDriver, *poDriver;
GDALRasterBand	*piBand, *poBand;
//double		adfGeoTransform[6];
char 		**papszMetadata;

int  	iwind, windsiz[10];
float 	sum;

char 	Proj[2000], ans[80], fullfilename[250], *file_out;
char 	*filename, *extension, temp[250];
char 	*tstring, *p, ach_in[5];


char	*basefname;						// ** just a pointer **
char	basefilname[250];
int		basef_len;	


PixVal		*pafScanline, *image_8b, *image_8b_out;
uint16		*pafScanline16, *image_16b, *image_16b_out;

int	by_lines=0, by_image=1;			// default is to read/write by image (faster),

int 	ch_out=1, no_ch=1, input_ch[10];

int	i, j, ii, jj, ofs, count;
int64 	bitnum, bitnum2;

/* Print Program Header and time */

	time (&rawtime);
	timeinfo = localtime (&rawtime);

	fprintf(stdout,"\n\t\tStarting %s (%s) at %s\n", PROG_NAME, VERSION, asctime(timeinfo));

//***************************

// Check input parameters (i.e., agrv[*])

//***************************

	//printf("\n\tPresent parameters are %s %s %s %s\n\n", argv[1], argv[2], argv[3], argv[4]);

	if (argv[1]== NULL) 
	  {
	  printf("\n\t PROBLEM with input image %s \n",argv[1]);
	  printf("Have an INPUT image as first argument on command line \n");
	  printf("USAGE: itcafav_g Main_File.ext,CH# NFMASK STCMASK WINDSIZ\n\n");
	  exit(-1);
	  }  

	if (argv[2] == NULL) 
	  {
	  printf("\n\n PROBLEM with second argument%s \n",argv[2]);
	  printf("\tHave a NFMASK bitmap (non forest mask) as second argument on command line \n\n");
	  printf("USAGE: itcafav_g Main_File.ext,CH# NFMASK STCMASK WINDSIZ\n\n"); 
	  exit(-1);
	  } 

	if (argv[3] == NULL) 
	  {
	  printf("\n\n PROBLEM with third argument%s \n",argv[3]);
	  printf("\tHave a STCMASK  bitmap (small tree mask) as third argument on command line \n\n");
	  printf("USAGE: itcafav_g Main_File.ext,CH# NFMASK STCMASK WINDSIZ\n\n"); 
	  exit(-1);
	  } 


	if (argv[4] == NULL) 
	  {
	  printf("\n\n PROBLEM with fourth argument%s \n",argv[4]);
	  printf("\tHave a WINDSIZ  as fourth argument on command line \n\n");
	  printf("USAGE: itcafav_g Main_File.ext,CH# NFMASK STCMASK WINDSIZ\n\n"); 
	  exit(-1);
	  } 
	  
	 // exit(-1);		// for debugging inputs
	  
	  
// Check that only one channel is mentionned with the input image file

	strncpy(temp, argv[1], 250);

	p = strtok(temp, ","); 	 
	strcpy(fullfilename,p);		
	p = strtok(NULL, ",");
	
	ii = 0;
	while(p != NULL) 
	  {
 	  //printf("%s\n", p); 
	  input_ch[ii++]= strtol(p,NULL, 10);
	  p = strtok(NULL, ",");
	  }
	no_ch = ii;
	if(no_ch == 0) {input_ch[0]=1; printf("\nInput channel number to use is *unspecified* will use first channel(1)\n\n");}


	
	//printf("\nNo. of channel to use %d  AND Input channel number to use: %d \n", no_ch, input_ch[0]);

	if ( no_ch > 1) 
	  {
	  fprintf(stdout,"\n\t##### ERROR - No. of channels to use to create a smoothed image must be one #####\n");
	  //printf("\n\tYou present parameters are %s %s %s %s\n\n", argv[1], argv[2], argv[3], argv[4]);
	  goto Exit;
	  }

	ch_in = input_ch[0];
	if(no_ch == 1) printf("As per user: Input image channel number to use: %d \n\n", ch_in);


// Check smoothing window size

	if (argv[4] == NULL) 
	  {
	  printf("\n\t ### ERROR ### A fourth argument is needed \n");
	  printf("\nThe fourth argument is the window size to use for the smoothing (often 3, for 3x3 window)\n");
	  exit(1);
	  }

	//windsiz[0] = strtol(argv[4],NULL, 10);
	//printf("\nAs per user, filtering window will be of size %dx%d \n", windsiz[0], windsiz[0]);

// Check if a second smoothing is needed (e.g., argv[4] = 3,5)

	strncpy(temp, argv[4], 20);
	p = strtok(temp, ",");

	iwind = 0;
	while(p != NULL) 
	  {
 	  //printf("%s\n", p); 
	  windsiz[iwind++]= strtol(p,NULL, 10);
	  p = strtok(NULL, ",");
	  }
/*
	printf("\nAs per user, the image will be smoothed %d time(s)\n", iwind);
	if(iwind==1) printf("Filtering window will be of size %dx%d \n", windsiz[0], windsiz[0]);
	if(iwind==2) printf("Smoothing with a %dx%d, then a %dx%d window.\n",windsiz[0],windsiz[0],windsiz[1],windsiz[1]);
*/

	printf("No smoothing will be done on areas under the nonforest mask (e.g., lakes, forest openings)\n");
	if(iwind==2) 
		printf("Smoothing using a %dx%d kernel on small tree areas and, additionally, a %dx%d kernel on big tree areas\n",
					windsiz[0],windsiz[0],windsiz[1],windsiz[1]);
	printf("Big tree areas are inferred -> not non-forest areas and not small tree areas\n");	
	
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

	printf( "Driver: %s/%s\n",
          ima_in->GetDriver()->GetDescription(),
          ima_in->GetDriver()->GetMetadataItem( GDAL_DMD_LONGNAME ) );

 	Pixels = ima_in->GetRasterXSize();
	Lines = ima_in->GetRasterYSize();
	Channels = ima_in->GetRasterCount();
	xsize = Pixels;     ysize = Lines;
	printf( "Image size is %d x %d x %d\n", Pixels, Lines, Channels);
	bmsize = (Pixels * (int64)Lines + 7) / 8; 
	printf("Bitmap size (in bytes): %I64d \n", bmsize);	
	printf("Image Description: %s \n", ima_in->GetDescription() );

// Print geographic info

	//if( ima_in->GetProjectionRef() != NULL ) printf( "Projection is '%s'\n\n", ima_in->GetProjectionRef() );

	strncpy(Proj,ima_in->GetProjectionRef(),30);
	printf("\nProjection is '%s'\n", Proj);

	if( ima_in->GetGeoTransform( adfGeoTransform ) == CE_None )
	  {
	  printf( "Origin = (%.6f,%.6f)\n", adfGeoTransform[0], adfGeoTransform[3] );
	  printf( "Pixel Size = (%.6f,%.6f)\n\n", adfGeoTransform[1], adfGeoTransform[5] );
	  }
	  
	  //exit(-1);		// for degugging

// Open input  channel

	piBand = ima_in->GetRasterBand(ch_in);
	
	printf("Channel Description: %s \n", piBand->GetDescription() );

// Check raster type (8 or 16 bit) and set flag PCI data Type CHN_8U=1 CHN_16U=3

	if(EQUALN(GDALGetDataTypeName(piBand->GetRasterDataType()),"Byte",4)) data_type=CHN_8U;  	//PCI data types
	if(EQUALN(GDALGetDataTypeName(piBand->GetRasterDataType()),"Uint16",6)) data_type=CHN_16U;

	printf("Channel %d , RasterDataType =%d, Type = %s, PCI data_type = %d \n\n",
		ch_in, piBand->GetRasterDataType(), GDALGetDataTypeName(piBand->GetRasterDataType()), data_type );

// Set function to acquire pixel accordingly
/*
	if ( data_type==CHN_8U) get_pix_val = get_u8;
	if ( data_type==CHN_16U) get_pix_val = get_u16;
*/

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
	
	printf("Base file name ::  %s \n", basefilname);	
	//exit(-1);
		  

//*************************************************

// Open NFMASK file

//*************************************************

// Open INPUT image file


	printf("\n\t*Opening NFMASK file \"%s\" for reading\n",argv[2]);

	nfmask = read_bitmap(argv[2], 1);

	//exit(-1);		// for degugging	
	

//*************************************************

// 	Open STCMASK file (small tree area mask)

//*************************************************

	printf("\n\t*Opening STCMASK file \"%s\" for reading\n",argv[3]);

	stcmask = read_bitmap(argv[3], 1);

	//exit(-1);		// for degugging	
	

//*************************************************

// Open (or create) an OUTPUT image file name	(for example:  Secteur_AFAV1.tif)

//*************************************************

	
/* 	basefname = strtok(fullfilename,".");
	basefname = strtok(basefname,"_");				
	printf("basefname  :  %s \n", basefname); */		
	
	// Special notation cases (ITC or ISOL)
	basefname = temp;	 				// necessary cause just a pointer no space assigned to it
	strcpy(basefname, basefilname);	
	
	
	file_out = strncat(basefname,"_AFAV",5); 
	//printf("file_out :  %s \n", file_out);   
	//itoa(ch_in,ach_in,10); 	 
	sprintf(ach_in, "%d" , ch_in); 	
	strncat(file_out,ach_in,3);	  	  	
	strncat(file_out,".tif",4);					// output image is forced to be a tif
 	printf("\nOutput image file will be named :  %s \n", file_out);
		  
	//exit(-1);		// for degugging
	 
	
// With ave_filter.cpp output file was specified by the ise. 
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

		  printf("\n\t\t ### OUTPUT channel %d will be overwriten or created?? \n\n",ch_out);
		  goto MAIN;
		  }

		}

	  }


//  Create the output image file in which to write one channel

CREATE:

	if(data_type==CHN_8U)
	  ima_out = (GDALDataset *) poDriver->Create( file_out, Pixels, Lines, 1, GDT_Byte, NULL );

	if(data_type==CHN_16U)
	  ima_out = (GDALDataset *) poDriver->Create( file_out, Pixels, Lines, 1, GDT_UInt16, NULL );

	if (ima_out == NULL) 
	  {printf("\n\n PROBLEM opening output image file %s \n\n", file_out); exit(1);}

	fprintf(stdout,"\n\t*File %s was created and is opened for writing\n", file_out);


// Copy the metadata (GeoTransform, projection, ...)

	fprintf(stdout,"\n\t*Writing metadata to output file %s\n", file_out);	

	ima_out->SetGeoTransform(adfGeoTransform);
	ima_out->SetProjection(ima_in->GetProjectionRef() );

	//exit(-1);		// for degugging
	  	
	
//***********************************************************************************

//	Main processing

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
	  printf("\nAllocated %Id bytes to the input image (same for output)",Pixels*(int64)Lines);
	if(data_type==CHN_16U)
	  printf("\nAllocated %Id bytes to the input image (same for output) ",2*Pixels*(int64)Lines);

// Read the whole input image IN ONE SHOT 

	printf("\nReading the whole input image in one shot ...\n");

	if(by_image)
	{
 	printf("\n\n\t*Reading full image into memory ... \n\n");

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
	goto OUT;
*/


	//exit(-1);		// for degugging


//****************************
	
// Do the smoothing of the image

	printf("\n\tDoing the main smoothing (%dx%d) ...\n\n",windsiz[0],windsiz[0]);

	ofs = windsiz[0] / 2 ; 		// offset is half of window size - important for boundaries

	// printf("\n windsiz[0], ofs: %d %d \n", windsiz[0], ofs);
	
	count = 0;				// for debugging
	
// For 8bit images

    if(data_type==CHN_8U)
    {
	for ( i = 1+ofs ; i <= Lines-ofs; i++ )		// For compatibility with PCI(x,y), I kept image coordinates start at (1,1)
	for ( j = 1+ofs ; j <= Pixels-ofs ; j++ ) 	// However, GDAL images start at zero (similar to bitmaps), so bytenum=bitnum
	  {

	  bitnum = (i-1)*(int64)Pixels + j-1 ;
	  
      sum = 0.0;						// get sum of pixels in (ofs,ofs) window
	  for ( ii = i-ofs ; ii <= i+ofs ; ii++ )
	  for ( jj = j-ofs ; jj <= j+ofs ; jj++ ) 
	    {
	    bitnum2 = (ii-1)*(int64)Pixels + jj-1;				/* imapos starting at zero */
	    sum = sum + image_8b[bitnum2];
	    }
	  image_8b_out[bitnum] = (PixVal) (sum /(windsiz[0]*windsiz[0]) + 0.5);	  
	  if(testbit(nfmask,bitnum)) {image_8b_out[bitnum] = image_8b[bitnum]; count++;}	// no smoothing under non-forest mask
	  
	  
	  if( (i/1000)*1000 == i ) printf("%d lines done\r", i);

	  }
	printf("%d lines done\n", Lines);
	printf("Using NFMASK %d pixel skipped\n", count);

    }	// end of for 8bit images


// For 16 bit images

    if(data_type==CHN_16U)
    {
	for ( i = 1+ofs ; i <= Lines-ofs; i++ )		// For compatibility with PCI(x,y), I kept image coordinates start at (1,1)
	for ( j = 1+ofs ; j <= Pixels-ofs ; j++ ) 	// However, GDAL images start at zero (similar to bitmaps), so bytenum=bitnum
	  {

	  bitnum = (i-1)*(int64)Pixels + j-1 ;

    	  sum = 0.0;						// get sum of pixels in (ofs,ofs) window
	  for ( ii = i-ofs ; ii <= i+ofs ; ii++ )
	  for ( jj = j-ofs ; jj <= j+ofs ; jj++ ) 
	    {
	    bitnum2 = (ii-1)*(int64)Pixels + jj-1;				/* imapos starting at zero */
	    sum = sum + image_16b[bitnum2];
	    }
	
	  image_16b_out[bitnum] = (uint16) (sum /(windsiz[0]*windsiz[0]) + 0.5);
	  if(testbit(nfmask,bitnum)) {image_16b_out[bitnum] = image_16b[bitnum]; count++;}	// no smoothing under non-forest mask	
	
	  if( (i/1000)*1000 == i ) printf("%d lines done\r", i);

	  }

	printf("%d lines done\n", Lines);
	printf("Using NFMASK %d pixel skipped\n", count);
	
    }	// end of for 16 bit images

// If only one smoothing is needed, you're done

	if(iwind == 1) goto OUT;


// A second smoothing may have been asked so copy intermediate image to original buffer


	printf("\n\tCopy intermediate image ...\n");

	for ( i = 1 ; i <= Lines; i++ )			// For compatibility with PCI(x,y), I kept image coordinates start at (1,1)
	for ( j = 1 ; j <= Pixels ; j++ ) 		// However, GDAL images start at zero (similar to bitmaps), so bytenum=bitnum
	  {
	  bitnum = (i-1)*(int64)Pixels + j-1;		
	  //ima_buf_in[bitnum] = ima_buf_out[bitnum];		//old
	  if(data_type==CHN_8U) image_8b[bitnum] = image_8b_out[bitnum];		// present output becomes input
	  if(data_type==CHN_16U) image_16b[bitnum] = image_16b_out[bitnum];

	  if( (i/1000)*1000 == i ) printf("%d lines done\r", i);
	  }
	  
	printf("%d lines done\n", Lines); 

//****************************

// Do second image smoothing (big trees areas)

//****************************

	printf("\n\tDoing additional smoothing (%dx%d) on big tree areas ...\n\n",windsiz[1],windsiz[1]);

	ofs = windsiz[1] / 2 ; 		// offset is half of window size - important for boundarie

	count = 0;				// for debugging
	
// For 8bit images

    if(data_type==CHN_8U)
    {
	for ( i = 1+ofs ; i <= Lines-ofs; i++ )		// For compatibility with PCI(x,y), I kept image coordinates start at (1,1)
	for ( j = 1+ofs ; j <= Pixels-ofs ; j++ ) 	// However, GDAL images start at zero (similar to bitmaps), so bytenum=bitnum
	  {

	  bitnum = (i-1)*(int64)Pixels + j-1 ;
	  
     sum = 0.0;						// get sum of pixels in (ofs,ofs) window
	  for ( ii = i-ofs ; ii <= i+ofs ; ii++ )
	  for ( jj = j-ofs ; jj <= j+ofs ; jj++ ) 
	    {
	    bitnum2 = (ii-1)*(int64)Pixels + jj-1;			
	    sum = sum + image_8b[bitnum2];
	    }

	  image_8b_out[bitnum] = (PixVal) (sum /(windsiz[1]*windsiz[1]) + 0.5);
	  
	  if(testbit(nfmask,bitnum) || testbit(stcmask,bitnum)) 	// no extra smoothing under both mask
			{image_8b_out[bitnum] = image_8b[bitnum];	count++;}

	  if( (i/1000)*1000 == i ) printf("%d lines done\r", i);

	  }
	printf("%d lines done\n", Lines);
	printf("Using NFMASK & STCMASK %d pixel skipped\n", count);
	
    }	// end of for 8bit images


// For 16 bit images

    if(data_type==CHN_16U)
    {
	for ( i = 1+ofs ; i <= Lines-ofs; i++ )		// For compatibility with PCI(x,y), I kept image coordinates start at (1,1)
	for ( j = 1+ofs ; j <= Pixels-ofs ; j++ ) 	// However, GDAL images start at zero (similar to bitmaps), so bytenum=bitnum
	  {

	  bitnum = (i-1)*(int64)Pixels + j-1 ;

    	  sum = 0.0;						// get sum of pixels in (ofs,ofs) window
	  for ( ii = i-ofs ; ii <= i+ofs ; ii++ )
	  for ( jj = j-ofs ; jj <= j+ofs ; jj++ ) 
	    {
	    bitnum2 = (ii-1)*(int64)Pixels + jj-1;				/* imapos starting at zero */
	    sum = sum + image_16b[bitnum2];
	    }
	
	  image_16b_out[bitnum] = (uint16) (sum /(windsiz[1]*windsiz[1]) + 0.5);
	  if(testbit(nfmask,bitnum) || testbit(stcmask,bitnum)) 	// no extra smoothing under both mask
			{image_16b_out[bitnum] = image_16b[bitnum];	count++;}
			
	  if( (i/1000)*1000 == i ) printf("%d lines done\r", i);

	  }

	printf("%d lines done\n", Lines);
	printf("Using NFMASK & STCMASK %d pixel skipped\n", count);
	
    }	// end of for 16 bit images



//****************************************

// Write output image in ONE SHOT

//****************************************

OUT:

// Time to check memory usage with task manager

/*	  printf("\nCheck memory use. OK to continue(y/n): ");
	  scanf("%s",ans);			// gets any answer
	  if(ans[0] == 'n') exit(-1);
*/


	printf("\n\t* Writing output image to \"%s\" \n", file_out);

	//poBand = ima_out->GetRasterBand(1);
	poBand = ima_out->GetRasterBand(ch_out);

    if(data_type==CHN_8U)
	poBand->RasterIO(GF_Write, 0, 0, Pixels, Lines, image_8b_out, Pixels, Lines, GDT_Byte,0, 0 );
    if(data_type==CHN_16U)
	poBand->RasterIO(GF_Write, 0, 0, Pixels, Lines, image_16b_out, Pixels, Lines, GDT_UInt16,0, 0 );

//	Description = "Smoothed by regions by ITCAFAV";

	snprintf(Description, 80, "Smoothed by regions - ITCAFAV using %dx%d, plus %dx%d on big tree areas",
			windsiz[0],windsiz[0],windsiz[1],windsiz[1]);
	//printf("Output Channel Description: %s \n", Description );
			
	poBand->SetDescription(Description);
	printf("Description: %s \n", poBand->GetDescription() );

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
//**************************************************************

