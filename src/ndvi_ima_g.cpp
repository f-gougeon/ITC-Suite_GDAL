/* 
Program name: 	ndvi_ima_g.cpp (64bit)
Author: 	Francois A. Gougeon

Description:

	From two input channels assumed nIR and RED, this
	program writes in an ouput channel an NDVI image
	using the standard formula:

			nIR - red
			---------
			nIR + red

*** This is a GDAL version of ITC Suite's NDVI_FG.cpp (for PCI environment)

Opening a "pix" (or other format) file, printing no of pixels and lines, geographic info, ...
and read two specified channels (as third argument) and write an NDVI image to 
the output file (2nd argument), often a "tif" file OR a specific channel of an existing file.

PROGRAM USAGE:

Run as (for examples):

>  ndvi_ima input_file output_file Channels(e.g., 1,2)

>  ndvi_ima SECTEUR7-1_FG_Sub.pix test_v11.tif 1,2

>   ndvi_ima test9.tif test_v11.tif 1,2

>   ndvi_ima Roads_12_Sub.pix Roads_FakeNDVI.tif 1,2

>   ndvi_ima Roads_12_Sub.pix Roads_12_Sub.pix 1,2 
		(for an existing output file, prog. will ask to overwrite file or a specific channel)


History:

V1.1	Oct. 2016 	Francois Gougeon

	- October 2016 (from NDVI_FG.cpp for the PCI environment)

	- June 2017 (cleaning up, improved, comments and re-tests)

V1.2	Oct. 2018 	Francois Gougeon

	- Mods. to be more versatile on output (new file, overwrite file (or not), 
	overwrite channel of existing file (say a PCI file), ...

	- As output file, deals with .tif, .pci, and ENVI (.dat) files

V1.3 	March-April 2020 	Francois Gougeon

	- Minor mods. for GDAL 3.0.0 (was GDAL 2.1.1) and some cleaning up

V1.4 	October 2021 	Francois Gougeon

	- Minor mods. (now at  GDAL 3.0.2, version 3.2.2 is nogo) and some cleaning up


 v1.5a		May 2023  François Gougeon

			- NOT to assume that all files are in the default directory from which the program is run ANYMORE
				Previously, everything was assumed in same directory and run from a cmd window from there.
				NOW, 
				the **path used with the main input file** is used when creating the default input/output file names
				This was necessary for ArcGIS Toolkit integration.

			- Also renamed ndvi_ima_g  to be consistent with other GDAL programs
			
			- Allow program to automatically create output file name base file name (using "-" or "#")
			
	
*******************************************************************************

*** To compile with Visual Studio (VS14)  (see VS14_GDAL_compile.txt)

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64

set INCLUDE=C:\gdal-2.1.1_v2\include;C:\libtiff-4.0.6_64b\tiff-4.0.6\libtiff;%INCLUDE%
set LIB=C:\gdal-2.1.1_v2\lib;C:\libtiff-4.0.6_64b\tiff-4.0.6\libtiff;%LIB%

set LINK=gdal_i.lib  libtiff_i.lib User32.lib

set CL= /MD

CL xxxxxxxxx.cpp /EHsc


*******************************************************************************************

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
******************************************************************************************
*/

#define VERSION "v1.5a"
#define PROG_NAME "NDVI_IMA"

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

int	Pixels, Lines, Channels;

time_t rawtime;
struct tm * timeinfo;


int main(int argc, char* argv[])
{

GDALDataset	*ima_in, *ima_out;
double		adfGeoTransform[6];
GDALDriver 	*poDriver;

GDALRasterBand	*piBand1, *piBand2, *poBand;

PixVal		*In_line1, *In_line2, *Out_line;
uint16		*In_line1_16b, *In_line2_16b;
float		NIRp, REDp;

char 	**papszMetadata;
char	ima_description[64];

int		i, j, ii=0, jj, k, kk;
int 	ch_out=1, input_ch[2], no_ch=1;
//int 	CHN_8U = 1, CHN_16U = 3;
int 	data_type = 1;			// 8bit images as input by default - output always 8bit
int 	data_type1 = 1, data_type2 = 1;	

char 	Proj[80], ans[80], fullfilename[130];
char	 *file_out;
char	fullfileout[130];
char 	*filename, *extension, temp[130];
char 	*tstring, *p;


char	*basefname;						// ** just a pointer **
char	basefilname[130];
int		basef_len;	



//*************************************************

/* Print Program Header and time */

	time (&rawtime);
	timeinfo = localtime (&rawtime);

	fprintf(stdout,"\n\t\tStarting %s (%s) at %s\n", PROG_NAME, VERSION, asctime(timeinfo));

// Check input parameters (i.e., agrv[*])

	if (argv[1]== NULL) 
	  {printf("\n\n PROBLEM with input image %s \n",argv[1]);
	  printf("\tHave an INPUT image as first argument on command line \n");
	  printf("USAGE:   ndvi_ima input_file output_file  Channels(e.g., 1,2) \n\n");
	  exit(1);
	  } 

	if (argv[2] == NULL) 
	  {printf("\n\n PROBLEM with output image %s \n",argv[2]);
	  printf("\tHave an OUTPUT image as second argument on command line \n\n");
	  printf("USAGE: ndvi_ima SECTEUR_FG.pix SECTEUR_NDVI.tif 1,2 \n\n");
	  exit(1);
	  } 

	if (argv[3] == NULL) 
	  {
	  printf("\n\t ### ERROR ### a third argument is needed \n");
	  printf("\nThat third argument is typically '1,2' (i.e., NIR and RED channels) \n");
	  printf("USAGE: ndvi_ima SECTEUR_FG.pix SECTEUR_NDVI.tif 1,2 \n\n");
	  exit(1);
	  }

//************************

// Open input image file
  
	GDALAllRegister();  

	ima_in = (GDALDataset *) GDALOpen( argv[1], GA_ReadOnly );

	if (ima_in == NULL) 
	  	{printf("\n\n PROBLEM opening input image file %s \n\n",argv[1]); exit(1);}

	fprintf(stdout,"\n\t*File %s was opened for reading\n\n",argv[1]);

// Print generic info (driver used, ... )

	// printf( "Driver: %s/%s\n",
          // ima_in->GetDriver()->GetDescription(),
          // ima_in->GetDriver()->GetMetadataItem( GDAL_DMD_LONGNAME ) );

	printf( "Image size is %d x %d x %d\n\n",
          Pixels = ima_in->GetRasterXSize(), Lines = ima_in->GetRasterYSize(), Channels = ima_in->GetRasterCount() );

// Print geographic info

	if( ima_in->GetProjectionRef() != NULL )  
	  {
	  strncpy(Proj, ima_in->GetProjectionRef(), 30);
	  printf("Projection is '%s'\n\n", Proj);
   	  //printf( "Projection is '%s'\n\n", ima_in->GetProjectionRef() );
	  }

	if( ima_in->GetGeoTransform( adfGeoTransform ) == CE_None )
	  {
	  printf( "Origin = (%.3f,%.3f)\n",
            adfGeoTransform[0], adfGeoTransform[3] );
	  printf( "Pixel Size = (%.3f,%.3f)\n\n",
            adfGeoTransform[1], adfGeoTransform[5] );
	  }


//*******************

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


	// exit(-1);			// for debugging



//************************

// Open (create) output image file


if ( EQUALN(argv[2],"-",1 ) || EQUALN(argv[2],"#",1 ) )			
	{
	//basefname = strtok(fullfilename,".");
	//basefname = strtok(basefname,"_");				
	//printf("basefname  :  %s \n", basefname);	
	
	printf("\nSecond argument is \"-\" or \"#\", which implies no output filename given\n");
	printf("A new output image file name will be created from the \"base file name\" \n\n"); 
	basefname = temp;	 				// fake to init basefname as a char array (just a pointer)
	strcpy(basefname, basefilname);	
	
	file_out = strncat(basefname,"_NDVI",5);	  	
	strncat(file_out,".tif",4);					// output image is forced to be a tif	
	}	
else								// if  input bitmap, just add to that name
	{
	//basefname = strtok(fullfilename,".");		// fake to init basefname
	//strcpy(basefname,argv[2]);
	//basefname = strtok(basefname,".");
	file_out = temp;				// fake to init file_out as char array
	strcpy(file_out,argv[2]);	
	}

	strcpy(fullfileout, file_out);
 	printf("\n\t*Output image file will be named \"%s\" \n", fullfileout);

	//exit(-1);			// for debugging


// Detect input image file type (extension)

	strcpy(fullfilename, argv[1]);
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
	   printf("Filename:  %s \n", filename);
	   exit( -1 );
	   }

	//exit(-1);			// for debugging
	
	
// Check if list of channel to used (i.e., check argv[3] )

	printf("\nInput channel(s) to use: %s \n", argv[3]);

	strncpy(temp, argv[3], 10);
	ii = 0;
	p = strtok(temp, ",");
	while(p != NULL) 
	  {
 	  //printf("%s\n", p); 
	  input_ch[ii++] = strtol(p,NULL, 10);
	  p = strtok(NULL, ",");
	  }

	no_ch = ii;
	//printf("\nNo. of channels to use %d \n", no_ch);

	if ( no_ch != 2) 
	  {
	  fprintf(stdout,"\n\t##### ERROR - No. of channels to create an NDVI image has to be two #####\n");
	  fprintf(stdout,"\n\tYou present parameters are %s %s %s \n\n", argv[1], argv[2], argv[3]);
	  printf("USAGE: ndvi_ima SECTEUR_FG.pix SECTEUR_NDVI.tif n,m (i.e., NIR and RED channels)\n\n");
	  goto Exit;
	  }

	if ( input_ch[0] == input_ch[1]) 
	  {
	  fprintf(stdout,"\n\t##### ERROR - The two channels must be different to create an NDVI image #####\n");
	  fprintf(stdout,"\n\tYou present parameters are %s %s %s \n\n", argv[1], argv[2], argv[3]);
	  printf("USAGE: ndvi_ima SECTEUR_FG.pix SECTEUR_NDVI.tif n,m (i.e., NIR and RED channels)\n\n");
	  goto Exit;
	  }


// First, check if output file already exist (ask to overwrite)

	//ima_out = (GDALDataset *) GDALOpen( argv[2], GA_Update );


 	//printf("\n\t*Output image file will be named \"%s\" \n", fullfileout);
	
	ima_out = (GDALDataset *) GDALOpen( fullfileout, GA_Update );
	
	if (ima_out != NULL) 
	  {
	  printf("\n\n ### Image %s already exist \n",fullfileout); 
	  printf("\n\t OK to overwrite FULL image file (Y/N)?   ");  fgets(ans,80,stdin);

	  //if (ans[0] == 'y' || ans[0] == 'Y') goto Create;

	  if (ans[0] == 'y' || ans[0] == 'Y') goto Data_write;

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

		  printf("\n\t\t ### OUTPUT channel will be overwriten or created?? \n\n");
		  goto Data_write;
		  }

		}

	  }


//  Create the output image file in which to write one channel (always 8bit)

Create:	ima_out = (GDALDataset *) poDriver->Create( fullfileout, Pixels, Lines, 1, GDT_Byte, NULL );

	if (ima_out == NULL) 
	  {printf("\n\n PROBLEM opening output image file %s \n\n",fullfileout); exit(1);}

	fprintf(stdout,"\n\t*File %s was created and is opened for writing\n",fullfileout);


// Copy the metadata (GeoTransform, projection, ...)

	fprintf(stdout,"\n\t*Writing GeoTransform and Projection to output file %s\n",fullfileout);	

	ima_out->SetGeoTransform(adfGeoTransform);
	ima_out->SetProjection(ima_in->GetProjectionRef() );


//**************************************

// Read image lines of the specific channel(s) and copy to output image file

Data_write:

// Get pointers to bands

	piBand1 = ima_in->GetRasterBand(input_ch[0]);
	piBand2 = ima_in->GetRasterBand(input_ch[1]);
	poBand = ima_out->GetRasterBand(ch_out);

	printf("\nInput Channel %d is of Type = %s\n",
		input_ch[0], GDALGetDataTypeName(piBand1->GetRasterDataType()) );
	printf("Input Channel %d is of Type = %s\n",
		input_ch[1], GDALGetDataTypeName(piBand2->GetRasterDataType()) );
	printf("Output Channel %d will be of Type = %s\n",
		ch_out, GDALGetDataTypeName(poBand->GetRasterDataType()) );

// Check raster type (8 or 16 bit) and set flag PCI data Type CHN_8U=1 CHN_16U=3 (for historical compatibility)

	if(EQUALN(GDALGetDataTypeName(piBand1->GetRasterDataType()),"UInt16",6)) data_type1=CHN_16U; 
	if(EQUALN(GDALGetDataTypeName(piBand2->GetRasterDataType()),"UInt16",6)) data_type2=CHN_16U;

	Out_line = (PixVal *) CPLMalloc(sizeof(PixVal)*Pixels);		// output image always 8bit

	if(data_type1 == CHN_8U) In_line1 = (PixVal *) CPLMalloc(sizeof(PixVal)*Pixels);
	if(data_type2 == CHN_8U) In_line2 = (PixVal *) CPLMalloc(sizeof(PixVal)*Pixels);

	if(data_type1==CHN_16U) In_line1_16b = (uint16 *) CPLMalloc(sizeof(uint16)*Pixels); 
	if(data_type2==CHN_16U) In_line2_16b = (uint16 *) CPLMalloc(sizeof(uint16)*Pixels); 

	fprintf(stdout,"\nWriting NDVI image of channels %d&%d of %s \n to channel %d of file %s\n\n",
		input_ch[0], input_ch[1], argv[1], ch_out, fullfileout);

	for (int y = 0; y < Lines; y++)
	  {
	  if(data_type1==CHN_8U) 
		piBand1->RasterIO(GF_Read, 0, y, Pixels, 1, In_line1, Pixels, 1, GDT_Byte,0, 0 );
	  if(data_type2==CHN_8U) 
		piBand2->RasterIO(GF_Read, 0, y, Pixels, 1, In_line2, Pixels, 1, GDT_Byte,0, 0 );
	  if(data_type1==CHN_16U) 
		piBand1->RasterIO(GF_Read, 0, y, Pixels, 1, In_line1_16b, Pixels, 1, GDT_UInt16,0, 0 );
	  if(data_type2==CHN_16U) 
		piBand2->RasterIO(GF_Read, 0, y, Pixels, 1, In_line2_16b, Pixels, 1, GDT_UInt16,0, 0 );

	  for (int x = 0; x < Pixels; x++)
		{

		//  NDVIp = 128 + ( 127 * (NIRp - REDp) / (NIRp + REDp) );
		// *(NDVI_image + (i*Pixels) + j) =  (unsigned char) NDVIp;
	
		if(data_type1==CHN_8U) NIRp = (float) In_line1[x];
		if(data_type1==CHN_8U) REDp = (float) In_line2[x];
		if(data_type1==CHN_16U) NIRp = (float) In_line1_16b[x];
		if(data_type1==CHN_16U) REDp = (float) In_line2_16b[x];

		Out_line[x] = (PixVal) (128 + ( 127.0 * (NIRp - REDp) / (NIRp + REDp) ) );
		}


	  poBand->RasterIO(GF_Write, 0, y, Pixels, 1, Out_line, Pixels, 1, GDT_Byte,0, 0 );

 	  if( (y/100)*100 == y ) printf("%d lines done\r", y);

	  }

	printf("%d lines done\r", Lines);

	printf("\n");
	
	// Writing description (history) ...

	sprintf(ima_description,"NDVI image from CHs %d,%d of %s\0", input_ch[0], input_ch[1], argv[1]);
	poBand->SetDescription(ima_description);

	printf( "\nDescription of output: %s\n", ima_description);

//**************************

// Close input and output images

Exit:	printf("\n\t*Closing both image files and exiting program. \n");

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



