/* 
Program name: 	ave_filter_g.cpp

Author: 	François A. Gougeon

Description:

	To produce a smoothed image useful (needed) for itcvfol_g of the ITC-Suite
	(with PCI, FAV or my AFAV were often used)

	François A. Gougeon, Ph.D.
	Remote Sensing Research	
	(©)Natural Resources Canada
	Canadian Forest Service 
	Pacific Forestry Centre
	506 West Burnside Rd.
	Victoria, British Columbia, 
	Canada, V8Z 1M5	

History:

	(Originally as ndvi_ima.cpp)

	- October 2016 (from NDVI_FG.cpp for thePCI environment)

	- June 2017 (cleaning up, improved, comments and re-tests)

	- Oct. 2018 Mods. to be more versatile on output (new file, overwrite file (or not), 
	overwrite channel of existing file (say a PCI file), ..

	As output file, deals with .tif, .pci, and ENVI (.dat) files


 v1.1a	April 2020	François Gougeon

	- Lots of clean-up and simplifications
	
	- Org. to deal with input/output of 16bit images 

	- Better checking of user inputs
	

 v1.2a	October 2021	François Gougeon

	- Changed program name from "ave_filter" to "ave_filter_g"
		to be more consistent with GDAL series
		
	- Improved input capabilities (to be similar to other prog.)	
	Input channel can be attached to input file name with a comma.

	Was : 
	> ave_filter_g SECTEUR7-1_FG_Sub.pix test_v11.tif CH# 5,3

	NOW :
	> ave_filter_g SECTEUR7-1_FG_Sub.pix,3 test_v11.tif  5,3
	> ave_filter_g SECTEUR7-1_FG_Sub.tif,3 test_v11.tif  5,3	
	
	

 v1.3a		May 2023  François Gougeon

			- NOT ASSUMING that all files are in the default directory from which the program is run ANYMORE
				Previously, everything was assumed in same directory and run from a cmd window from there.
				NOW, 
				the **path used with the main input file** is used when creating the default input/output file names
				This was necessary for ArcGIS Toolkit integration.
			
			- Allow program to automatically create output file name based on input name (using "-" or "#")
				

 v1.4		July  2023  François Gougeon	
 
			- Change input parameter order (and more versatile)

 v1.5		Dec  2023  François Gougeon	
 
			- RE-Change input parameter order (Feels more natural)			
			NOW   > ave_filter_g input.tif output.tif  5,3
			
			- Provenance description in the output file has a SHORT version 
			of the input file name (not the full path, which could be long
			when using ArcGIS

*******************************************************************************	

Run as (for examples):


> ave_filter_g test9.pix,4   output_file.tif	5,3 	# channel to use connected to filename

> ave_filter_g test9.pix  4  output_file.tif	5,3 	# channel to use as separate parameter for ArcGIS

		(for an existing output file, prog. will ask permission to overwrite file
		and/or permission to overwrite a specific channel
		
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

#define VERSION "v1.5a"
#define PROG_NAME "AVE_FILTER_G"
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

typedef unsigned char  PixVal;
typedef long           int64;
typedef unsigned long  uint64;
typedef unsigned short     uint16;

/* Global declarations */

int	Pixels, Lines, Channels;	// pixel and line starting at 1,1
int	xsize, ysize;			// pixel and line starting at 0,0

PixVal *ima_buf_in, *ima_buf_out;

time_t rawtime;
struct tm * timeinfo;

int	bylines_flag = 0;		// by default proceed by full images (not by lines)
int	by_lines=0, by_image=1;			// default is to read/write by image (faster),

int 	data_type, CHN_8U = 1, CHN_16U = 3;			//PCI data types

//PIX_FUN_PTR	get_pix_val; 		 // pointer that allows us to deal with  many types of image (8,16u,16s) 


//************************************

int main(int argc, char* argv[])
{

GDALDataset	*ima_in, *ima_out;
GDALDriver 	*piDriver, *poDriver;
GDALRasterBand	*piBand, *poBand;
double		adfGeoTransform[6];
char 		**papszMetadata;

int 	ch_out=1, ch_in, no_ch=1, dbic[10], ch_no;
int  	iwind, windsiz[10];
float 	sum;
int	 argcount;

char 	Proj[250], ans[80], answer[10];
char	fullfilename[FILENAME], file_in[FILENAME];		// for input file
char 	*filename, *extension, temp[FILENAME];			// for output file
char	ima_description[128];						// for output file
char 	*tstring, *p, ach_in[5];

PixVal		*pafScanline, *image_8b, *image_8b_out, *Out_line;
uint16		*pafScanline16, *image_16b, *image_16b_out;

int	i, j, ii, jj, ofs;
int64 	bitnum, bitnum2;
int commaFlag = 0;


char	*basefname;						// ** just a pointer **
char	basefilname[250];
int		basef_len;	
char	 *file_out;
char	fullfileout[250],shortfilename[250];
char  	*cptr;	// generic pointer to char



//***************************
//***************************


/* Print Program Header and time */

	time (&rawtime);
	timeinfo = localtime (&rawtime);

	fprintf(stdout,"\n\t\tStarting %s (%s) at %s\n", PROG_NAME, VERSION, asctime(timeinfo));
	

//***************************

// Check *First argument* on command line (here, Input file name)

	argcount = 1;
	
	//printf("1st argument - Illumination image full entry : %s \n\n", argv[1]);
	
	if (argv[argcount]== NULL) 
	  {
	  printf("\n\t **PROBLEM** with input image %s \n",argv[argcount]);
	  printf("\tHave an INPUT image as first argument on command line \n\n");
	  printf("\tUSAGE: ave_filter_g in_file.tif  out_file.tif 3 (OR 5,3)\n"); 
	  printf("\tUSAGE: ave_filter_g in_file.pix,4  out_file.tif 3   (OR 5,3)\n");	  
	  printf("\tUSAGE: ave_filter_g in_file.pix 4  - 3 \n\n");  
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

// Some programs may have a second comma and a second item	

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
	
// Channel numbers may be as a separate parameter on the cmd line (OR NADA)
	
	if(!commaFlag)		
	  { 
	  argcount++;					 // get next argument	  
	  //printf("Argument %d: %s \n", argcount, argv[argcount]);
	  strcpy(temp, argv[argcount]);	  
	  ch_no = strtol(temp,NULL, 10);			// change to integer  
	  if(ch_no == NULL) { ch_no = 1; argcount--; }		// next argument was probably a file name 
	  }	
	  printf(" Input channel to use : %d \n", ch_no);

//	exit(-1);				// for debugging
	

//*******************

// Create base file name   (for output file name)
	
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
	
	printf("\n Base file name ::  %s \n", basefilname);	


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
	//printf(" Shortfilename:  %s \n", shortfilename);


//*************************************************

// Open (or create) an OUTPUT image file

//*************************************************
// Check next input parameter

	argcount++;			// next argument
	//printf("\nArgument %d: %s \n\n", argcount, argv[argcount]);

	if (argv[argcount]== NULL) 
	  {
	  printf("\n\t **PROBLEM** with output image %s \n",argv[argcount]);
	  printf("Have an OUTPUT image on command line OR \"-\" OR \"#\" for automatic filename\n\n");  
	  printf("\tUSAGE: ave_filter_g in_file.tif  out_file.tif 3 (OR 5,3)\n"); 
	  printf("\tUSAGE: ave_filter_g in_file.pix,4  out_file.tif 3   (OR 5,3)\n");	  
	  printf("\tUSAGE: ave_filter_g in_file.pix 4  - 3 \n\n");  
	  exit(-1);
	  }  


if ( EQUALN(argv[argcount],"-",1 ) || EQUALN(argv[argcount],"#",1 ) )			// need to create an output file name
	{
	//basefname = strtok(fullfilename,".");
	//basefname = strtok(basefname,"_");				
	//printf("basefname  :  %s \n", basefname);	
	
	printf("\nOutput file argument is \"-\" or \"#\", which implies no output filename was given\n");
	printf("A new output image file name will be created from the \"base file name\" \n\n"); 
	basefname = temp;	 				// fake to init basefname as a char array (just a pointer)
	strcpy(basefname, basefilname);	
	
	file_out = strncat(basefname,"_Ave",4);	  	
	//itoa(ch_in,ach_in,10); 	 
	sprintf(ach_in, "%d" , ch_in);	 	
	strncat(file_out,ach_in,3);
	strncat(file_out,".tif",4);					// output image is forced to be a tif	
	strcpy(fullfileout, file_out);
	printf("\n\t*Output image file will be named \"%s\" \n\n", fullfileout);	
	}	
else								// if  input bitmap, just add to that name
	{
	//basefname = strtok(fullfilename,".");		// fake to init basefname
	//strcpy(basefname,argv[argcount]);
	//basefname = strtok(basefname,".");
	file_out = temp;				// fake to init file_out as char array
	strcpy(file_out,argv[argcount]);	
	strcpy(fullfileout, file_out);
	printf("\nOutput Filename as stated:  %s \n\n", fullfileout);	
	}

	//exit(-1);			// for debugging
	


//**************************************

// Check smoothing window size

//*****************************************

		
// Check next input parameter

	argcount++;			// next argument
	//printf("\nArgument %d: %s \n\n", argcount, argv[argcount]);

	if (argv[argcount] == NULL) 
	  {
	  printf("\n\t ### ERROR ### A third main argument is needed \n");
	  printf("\nThe third main argument is the window size to use for smoothing (often 3, for 3x3 window)\n");
	  printf("\nWindow size could also be 5,3 implying smoothing by 5x5, followed by 3x3 smoothing\n");
	  exit(1);
	  }

	//windsiz[0] = strtol(argv[4],NULL, 10);
	//printf("\nAs per user, filtering window will be of size %dx%d \n", windsiz[0], windsiz[0]);

// Check if a second smoothing is needed (e.g., argv[3] = 5,3)

	windsiz[1] = 0; 				//if not used set to zero
	strncpy(temp, argv[argcount], 20);
	p = strtok(temp, " ,");

	iwind = 0;
	while(p != NULL) 
	  {
 	  //printf("%s\n", p); 
	  windsiz[iwind++]= strtol(p,NULL, 10);
	  p = strtok(NULL, " ,");
	  }

	printf("\nAs per user, the image will be smoothed %d time(s)\n", iwind);
	if(iwind==1) printf("Filtering window will be of size %dx%d \n", windsiz[0], windsiz[0]);
	if(iwind==2) printf("Smoothing with a %dx%d, then a %dx%d window.\n",windsiz[0],windsiz[0],windsiz[1],windsiz[1]);
	if(iwind==3) {printf("\n\t***Smoothing three(3) times not implemented \n\n"); exit(0);}


	//exit(1);		// useful when testing only input parameters

//****************************************************************

// Registers for all types of files with GDAL and Open INPUT image file

//************************************

	GDALAllRegister(); 

// Open INPUT image file

	ima_in = (GDALDataset *) GDALOpen( file_in, GA_ReadOnly );

	if (ima_in == NULL) 
	  	{ printf("\n\n PROBLEM opening input image file %s \n\n",file_in); exit(1); }
	
	fprintf(stdout,"\n\t*File %s was opened for reading\n\n",file_in);

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

	//if( ima_in->GetProjectionRef() != NULL ) printf( "Projection is '%s'\n\n", ima_in->GetProjectionRef() );

	strncpy(Proj,ima_in->GetProjectionRef(),30);
	printf("Projection is '%s'\n\n", Proj);

	if( ima_in->GetGeoTransform( adfGeoTransform ) == CE_None )
	  {
	  printf( "Origin = (%.3f,%.3f)\n", adfGeoTransform[0], adfGeoTransform[3] );
	  printf( "Pixel Size = (%.3f,%.3f)\n\n", adfGeoTransform[1], adfGeoTransform[5] );
	  }

// Open that channel

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

	
	

//*************************************************

// Open (or create) an OUTPUT image file

	filename = strtok(fullfilename,".");
	extension = strtok(NULL," "); 
/*
	printf("Filename:  %s \n", filename);
	printf("Extension:  %s \n", extension);
	if (strncmp(extension,"tif",3) == 0) printf("\nOutput file extension is 'tif' \n");
	if (strncmp(extension,"pix",3) == 0) printf("\nOutput file extension is 'pix' \n");
	if (strncmp(extension,"dat",3) == 0) printf("\nOutput file extension is 'dat' \n");
*/
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

	ima_out = (GDALDataset *) GDALOpen( fullfileout, GA_Update );

	if (ima_out != NULL) 
	  {
	  printf("\n\n ### Image %s already exist \n",fullfileout); 
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
	if(data_type == CHN_8U)
	  ima_out = (GDALDataset *) poDriver->Create( fullfileout, Pixels, Lines, 1, GDT_Byte, NULL );

	if(data_type == CHN_16U)
	  ima_out = (GDALDataset *) poDriver->Create( fullfileout, Pixels, Lines, 1, GDT_UInt16, NULL );

	if (ima_out == NULL) 
	  {printf("\n\n PROBLEM opening output image file %s \n\n",fullfileout); exit(1);}

	fprintf(stdout,"\n\t*File %s was created and is opened for writing\n",fullfileout);


// Copy the georeferencing data (GeoTransform, projection, ...)

	fprintf(stdout,"\n\t*Writing georeferencing data to output file %s\n",fullfileout);	

	ima_out->SetGeoTransform(adfGeoTransform);
	ima_out->SetProjection(ima_in->GetProjectionRef() );
	
	
	
	
	
	
	

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
	  printf("\nAllocated %ld bytes to the input image (same for output)",Pixels*(int64)Lines);
	if(data_type==CHN_16U)
	  printf("\nAllocated %ld bytes to the input image (same for output) ",2*Pixels*(int64)Lines);

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


// TEST :  Just copy INPUT image for the moment
/*
	printf("\n\tTEST : Just copy INPUT image for the moment ...\n");

	for ( i = 1 ; i <= Lines; i++ )		// For compatibility with PCI(x,y), I kept image coordinates start at (1,1)
	for ( j = 1 ; j <= Pixels ; j++ ) 		// However, GDAL images start at zero (similar to bitmaps), so bytenum=bitnum
	  {
	  bitnum = (i-1)*(int64)Pixels + j-1;		
	  image_8b_out[bitnum] = image_8b[bitnum];
	  if( (i/1000)*1000 == i ) printf("%d lines done\r", i);
	  }
	goto OUT;
*/


// Do the smoothing of the image

	printf("\n\tDoing the main processing (smoothing) ...\n\n");

	ofs = windsiz[0] / 2 ; 		// offset is half of window size - important for boundaries

	// printf("\n windsiz[0], ofs: %d %d \n", windsiz[0], ofs);


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

	  if( (i/1000)*1000 == i ) printf("%d lines done\r", i);

	  }
	printf("%d lines done\n", Lines);

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

	  if( (i/1000)*1000 == i ) printf("%d lines done\r", i);

	  }

	printf("%d lines done\n", Lines);

    }	// end of for 16 bit images

// If only one smoothing is needed, you're done

	if(iwind == 1) goto OUT;


// A second smoothing may have been asked for Copy intermediate image to ima_buf_in


	printf("\n\tCopy intermediate image ...\n");

	for ( i = 1 ; i <= Lines; i++ )			// For compatibility with PCI(x,y), I kept image coordinates start at (1,1)
	for ( j = 1 ; j <= Pixels ; j++ ) 		// However, GDAL images start at zero (similar to bitmaps), so bytenum=bitnum
	  {
	  bitnum = (i-1)*(int64)Pixels + j-1;		
	  //ima_buf_in[bitnum] = ima_buf_out[bitnum];		//old
	  if(data_type==CHN_8U) image_8b[bitnum] = image_8b_out[bitnum];
	  if(data_type==CHN_16U) image_16b[bitnum] = image_16b_out[bitnum];

	  if( (i/1000)*1000 == i ) printf("%d lines done\r", i);
	  }


// Do second image smoothing


	printf("\n\tDoing second smoothing ...\n");

	ofs = windsiz[1] / 2 ; 		// offset is half of window size - important for boundarie


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

	  if( (i/1000)*1000 == i ) printf("%d lines done\r", i);

	  }
	printf("%d lines done\n", i);

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

	  if( (i/1000)*1000 == i ) printf("%d lines done\r", i);

	  }

	printf("%d lines done\n", i);

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

	printf("\n\t* Writing output image to file \"%s\" in one shot\n\n", fullfileout);

	poBand = ima_out->GetRasterBand(ch_out);

    if(data_type==CHN_8U)
	poBand->RasterIO(GF_Write, 0, 0, Pixels, Lines, image_8b_out, Pixels, Lines, GDT_Byte, 0, 0 );
    if(data_type==CHN_16U)
	poBand->RasterIO(GF_Write, 0, 0, Pixels, Lines, image_16b_out, Pixels, Lines, GDT_UInt16, 0, 0 );


/*
	printf("\n\t* Writing output image to file %s line by line\n", fullfileout);
	
	Out_line = (PixVal *) CPLMalloc(sizeof(PixVal)*Pixels);		// TEST - HERE, output image always 8bit
	
	for ( i = 1 ; i <= Lines; i++ )		// For compatibility with PCI(x,y), I kept image coordinates start at (1,1)
	for ( j = 1 ; j <= Pixels ; j++ ) 		// However, GDAL images start at zero (similar to bitmaps), so bytenum=bitnum
	  {
	  bitnum = (i-1)*(int64)Pixels + j-1;  
	  Out_line[j-1] = image_8b_out[bitnum];
	  poBand->RasterIO(GF_Write, 0, i-1, Pixels, 1, Out_line, Pixels, 1, GDT_Byte,0, 0 );  
	  if( (i/1000)*1000 == i ) printf("%d lines done\r", i);
	  }
*/
	


// Writing description (history) ...

	sprintf(ima_description,"ave_filter_g (%d,%d) from CH %d of %s \0", windsiz[0], windsiz[1], ch_in, shortfilename);
	poBand->SetDescription(ima_description);
	printf("Output image description : %s \n", ima_description);

// Close input and output images

Exit:	printf("\nClosing both image files and exiting program. \n");

	GDALClose(ima_in);
	GDALClose(ima_out);

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

