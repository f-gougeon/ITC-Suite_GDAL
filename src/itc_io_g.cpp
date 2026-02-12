/*
	itcio_g.cpp

	Main IO functions used by the GDAL version of the ITC Suite programs.

	Author:	François A. Gougeon

	Date:		April-Sept 2020
	Version:	1.1a
	
	Date:		Nov. 2021
	Version:	1.1
	
	Date:		June 2022
	Version:	1.2
	
	- Made write_image() safer, not to overwrite wrong channel (or full file) by bad user inputs
	
	Date:		Nov. 2024 
	Version:	1.3
	
	- Introduced "fill_count" and "Poly_Outside" variables to vect2rast()

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
******************************************************************************************
*/

#define VERSION "v1.3"

#include "ITC-Suite_g.h"
#include "itc_io_g.h"

// The following should all be in "itc_io_g.h" now

/*
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>     // malloc, free, rand
#include <string.h>
#include <math.h>
#include <limits.h>
#include <time.h>       // time_t, struct tm, time, localtime
#include "gdal_priv.h"		// For GDAL library
#include "ogrsf_frmts.h"	// For OGR library (vector related)
#include "bitops.h"	// bit operations on bitmaps (mostly macros to be faster)
*/

// For PCI vertices (info from ccltypes.h,     NOW ALSO in itc_io_g.h and conditional)

/*
typedef struct
{
    double              x;
    double              y;
} GDBVertex2D;
*/

// The following should now be in itc_io_g.h

/*
typedef unsigned char  PixVal;		
typedef long           int64;
typedef unsigned long  uint64;
*/


//	Should now all be in "itc_io_g.h" 

/*
static 	int32 get_u8(void *image_8b, int64 pos);
static 	int32 get_u16(void *image_16b, int64 pos);

GDALDataset* open_imaFile(char *);
void* 		read_image(char *, int);
void* 		write_image(void *, char *)

PixVal*		read_bitmap(char *, int);
void 		write_bitmap(PixVal*, char *);

PixVal* 	vector2bitmap(char *,  int);
void 		vect2rast_g(int, GDBVertex2D *, unsigned char *, int, int, int);

void 		check_mem(void *);
void 		safety_zone(unsigned char *);
*/

//************************************************



// Global and al variables

extern	int		Lines, Pixels, Channels;
extern int64 	bmsize;					// size of bitmap in bytes 


extern	float		xpixsz, ypixsz;
extern	char 		*Proj, *Proj2, *Datum, *Datum2, *Temp,*token;
extern	double		adfGeoTransform[6], adfGeoTransform2[6];
extern	double 		topleftX, transformX, topleftY, transformY; 	 /* for geographic mapping */

extern	int 		ch_in, ch_out, in_ch[], segm_in, in_segm[10];

extern	GDALRasterBand	*piBand,*piBand2,*poBand;
extern	char **papszOptions;

extern	int imaFile_opened;
extern  GDALDataset	*ima_in, *seg_in;

int 	Poly_Outside;			// flag that the polygon is considered outside the image
int 	fill_count;				// count of fill pixels from vect2rast_g() (NB could be negative if removing an area)

extern int  by_lines, by_image;		// default is to read/write by image (faster), 

//int  by_lines=0, by_image=1;		// default is to read/write by image (faster), 
extern	int bylines_flag;		// user can specify bylines (slower) if not enough memory to go by imag

extern int 	data_type;			//  CHN_8U = 1, CHN_16U = 3

extern	char 	Description[];
extern	char 	Extension[]; 		// global variable for other prog.
//extern	char 	Path[]; 


PIX_FUN_PTR	get_pix_val; 		 // pointer that allows us to deal with  many types of image (8,16u,16s) 
#define CHN_8U 		1
#define CHN_16U 	3		// to continue to use PCI nomenclature when all PCI stuff gone 
#define CHN_32U 	4	
#define CHN_32R 	5	



//******************************************************

// ### GDAL Open an image file, read georeferencing, read image

//******************************************************


GDALDataset * open_imaFile(char * filename)
{

char		fullfilename[250];
void		*image_pointer;
GDALRasterBand	*piBand;
//GDALDataset	*ima_in;

PixVal		*pafScanline, *image_8b;
uint16		*pafScanline16, *image_16b;
uint32		*pafScanline32, *image_32b, *image_32b_out;

//int 		data_type, CHN_8U = 1, CHN_16U = 3;   // NOW  global variables
int		i, j;
int64		bitnum;
char 		*Fname;		// just a pointer
char 		*extension;

if(bylines_flag) { by_lines=1; by_image=0; }		// if by lines flag is set

// Open input image file (PCI or TIF) via GDAL

	//fprintf(stdout,"\n\t*Trying to open file '%s' for reading\n\n", filename);

	ima_in = (GDALDataset *) GDALOpen( filename, GA_ReadOnly );		//pointer to file dataset 
	seg_in = ima_in;

	if (ima_in == NULL) 
	  {printf("\n\n PROBLEM opening image file %s \n\n",filename); exit(1);}

	fprintf(stdout,"\n\tFile <<%s>> was opened for read only\n\n", filename);


// Print generic info of input file (driver used,... )

	printf( "Driver: %s/%s\n",
	ima_in->GetDriver()->GetDescription(),
 	ima_in->GetDriver()->GetMetadataItem( GDAL_DMD_LONGNAME ) );

	Pixels = ima_in->GetRasterXSize();
	Lines =  ima_in->GetRasterYSize();
        Channels = ima_in->GetRasterCount();

	printf( "Image size(P,L) is %d x %d x %d channels\n\n", Pixels, Lines, Channels );

	bmsize =  ((Pixels*(int64)Lines+ 7) / 8);    // size of bitmaps in byte

	//printf( "Internal bitmap  size will be %Ii \n\n",bmsize);


// Get Proper Geo projection of main image

	Proj = (char *) CPLMalloc(2000);			// to store geo projection info
	Datum = (char *) CPLMalloc(2000);		// to store Datum info
	Temp = (char *) CPLMalloc(2000);			

	strncpy(Proj,ima_in->GetProjectionRef(),2000);
	// if( ima_in->GetProjectionRef() != NULL)  printf( "Projection is '%s'\n\n", Proj);
 

// Print geographic projection info of input file

	if( ima_in->GetProjectionRef() != NULL ) 
	  {
	  strncpy(Temp, ima_in->GetProjectionRef(),2000);
	  strtok(Temp, "[[\"");			
	  //printf("1st section : %s \n", Temp);
 	  token = strtok(NULL, "[\"");     printf("Projection: %s \n", token);
 	  token = strtok(NULL, "[\"");	  //printf("Temp: %s \n", token);
 	  token = strtok(NULL, "[\"");	  //printf("GEOGCS: %s \n", token);
	  token = strtok(NULL, "[\"");	  //printf("Temp: %s \n", token);
 	  token = strtok(NULL, "[\"");	  printf("Datum: %s \n", token);
	  }


// Get GeoTransform of  image fle

	if( ima_in->GetGeoTransform( adfGeoTransform ) == CE_None )
	  {
	  topleftX = adfGeoTransform[0];		
	  topleftY = adfGeoTransform[3];
	  transformX = adfGeoTransform[1];	
	  transformY = adfGeoTransform[5];
	  xpixsz = fabs(transformX); 
	  ypixsz = fabs(transformY); 	  
	  
  	 // printf( "\nOrigin (top left)= (%.2f,%.2f)\n", adfGeoTransform[0], adfGeoTransform[3] );
	  printf( "\nOrigin (top left)= (%.2f,%.2f)\n", topleftX, topleftY );
	  printf( "Pixel Size = (%.2f,%.2f)\n", xpixsz, ypixsz );
      printf( "GeoTransform = (%.2f,%.2f)\n\n", adfGeoTransform[1], adfGeoTransform[5] );
	  
	  }

// Check if tif file or PCI file. If multi-channel,  ask channel number

//	printf("\nChecking image file extension ...\n\n");

	strcpy(fullfilename, filename);
	
	printf("Full Filename:  %s \n", fullfilename);		// print full file name to verify

	Fname = strtok(fullfilename,".");
	extension = strtok(NULL," "); 
	strcpy(Extension,extension);		// global variable for other prog.

//	printf("Filename:  %s \n", Fname);
//	printf("Extension:  %s \n", extension);

	if (strncmp(extension,"tif",3) == 0) printf("File extension is \"tif\" \n");
	if (strncmp(extension,"pix",3) == 0) printf("File extension is \"pix\" \n");

	

// 	Get full directory path to that file (maybe to be used by others)

/* 	
	int path_len;	jj=0;

	strcpy(fullfilename, filename);
	
	for (ii=0; ii < strlen(fullfilename); ii++)
	  {
	  jj = strlen(fullfilename) - ii;
	  if(fullfilename[jj] == '/') {path_len = jj;	break;}		// find last slash in full file name
	  }
	//printf("\nPath Length:  %d \n", path_len);
	
	strncpy(path, fullfilename, path_len);
	path[path_len] = '\0';   					// null character manually added
	
	//printf("Full Filename:  %s \n", fullfilename);
	printf("Path of file ::  %s \n", path);			// may have full path to directory

*/

	
	


// Check raster type (8 or 16 bit) and set flag PCI data Type CHN_8U=1 CHN_16U=3

	piBand = ima_in->GetRasterBand(1);
	if(EQUALN(GDALGetDataTypeName(piBand->GetRasterDataType()),"Byte",4)) data_type=CHN_8U;  	//PCI data types
	if(EQUALN(GDALGetDataTypeName(piBand->GetRasterDataType()),"Uint16",6)) data_type=CHN_16U;



// Return the fact that the file is opened and its pointer

	imaFile_opened = 1;	// set flag that image is already open (so other functions know)
	return(ima_in);	
		
}		// end of open_imaFile function

//******************************************************

// ### GDAL Open an image file, read georeferencing, read image

//******************************************************


void * read_image(char * filename, int ch_no)
{
void		*image_pointer;
GDALRasterBand	*piBand;
//GDALDataset	*ima_in;

PixVal		*pafScanline, *image_8b;
uint16		*pafScanline16, *image_16b;

//int 		data_type, CHN_8U = 1, CHN_16U = 3; // NOW  global variables
int		i, j;
int64		bitnum;
char 		*Fname, *extension;

if(bylines_flag) { by_lines=1; by_image=0; }		// if by lines flag is set

if (imaFile_opened) goto FileOpened;

// Open input image file (PCI or TIF) via GDAL

	//fprintf(stdout,"\n\t*Trying to open file '%s' for reading\n\n", filename);

	ima_in = (GDALDataset *) GDALOpen( filename, GA_ReadOnly );		//pointer to file dataset 

	if (ima_in == NULL) 
	  {printf("\n\n PROBLEM opening image file %s \n\n",filename); exit(1);}

	fprintf(stdout,"\n**File '%s' was opened to read a channel\n\n",filename);


// Print generic info of input file (driver used,... )

	printf( "Driver: %s/%s\n",
	ima_in->GetDriver()->GetDescription(),
 	ima_in->GetDriver()->GetMetadataItem( GDAL_DMD_LONGNAME ) );

	Pixels = ima_in->GetRasterXSize();
	Lines =  ima_in->GetRasterYSize();
    Channels = ima_in->GetRasterCount();

	printf( "Image size(P,L) is %d x %d x %d channels\n\n", Pixels, Lines, Channels );

	bmsize =  ((Pixels*(int64)Lines+ 7) / 8);    // size of bitmaps in byte

	//printf( "Internal bitmap  size will be %Ii \n\n",bmsize);


// Get Proper Geo projection of main image

	Proj = (char *) CPLMalloc(2000);			// to store geo projection info
	Datum = (char *) CPLMalloc(500);		// to store Datum info
	Temp = (char *) CPLMalloc(2000);			

	strncpy(Proj,ima_in->GetProjectionRef(),2000);
	// if( ima_in->GetProjectionRef() != NULL)  printf( "Projection is '%s'\n\n", Proj);
 

// Print geographic projection info of input file

	if( ima_in->GetProjectionRef() != NULL ) 
	  {
	  strncpy(Temp, ima_in->GetProjectionRef(),2000);
	  strtok(Temp, "[[\"");			
	  //printf("1st section : %s \n", Temp);
 	  token = strtok(NULL, "[\"");     printf("Projection: %s \n", token);
 	  token = strtok(NULL, "[\"");	  //printf("Temp: %s \n", token);
 	  token = strtok(NULL, "[\"");	  //printf("GEOGCS: %s \n", token);
	  token = strtok(NULL, "[\"");	  //printf("Temp: %s \n", token);
 	  token = strtok(NULL, "[\"");	  printf("Datum: %s \n", token);
	  }


// Get GeoTransform of image 

	if( ima_in->GetGeoTransform( adfGeoTransform ) == CE_None )
	  {
	  topleftX = adfGeoTransform[0];		
	  topleftY = adfGeoTransform[3];
	  transformX = adfGeoTransform[1];	
	  transformY = adfGeoTransform[5];
	  xpixsz = fabs(transformX); 
	  ypixsz = fabs(transformY); 
	  
  	 // printf( "\nOrigin (top left)= (%.2f,%.2f)\n", adfGeoTransform[0], adfGeoTransform[3] );
	  printf( "\nOrigin (top left)= (%.2f,%.2f)\n", topleftX, topleftY );
	  printf( "Pixel Size = (%.2f,%.2f)\n", xpixsz, ypixsz );
      printf( "GeoTransform = (%.2f,%.2f)\n\n", adfGeoTransform[1], adfGeoTransform[5] );
	  }

// check file extension and feed to glabal vars Extension

	Fname = strtok(filename,".");
	extension = strtok(NULL," "); 
	strcpy(Extension,extension);		// global variable for other prog.
//	printf("Filename:  %s \n", Fname);
//	printf("Extension:  %s \n", extension);

// File is now judged opened

	imaFile_opened = 1;

// **************

// File is now opened (just now or previously) and lots of info was gathered

FileOpened:

	ch_in = 1 ;			// by default

	if (ch_no) ch_in  = ch_no;	// channel was specified when function was called

// IF more than one channel in file and channel not specified so far, ask user which one

	if ( (!ch_no) && (Channels > 1 ) )
	  {
	  printf("\nMore than one channel in image file. Which one to use? ");
	  scanf("%d",in_ch);	ch_in = in_ch[0];	
	  }

	//printf("\n Got here. ch_in = %d \n", ch_in);

	piBand = ima_in->GetRasterBand(ch_in);

	printf("\n**Opening input channel %d of Type = %s \n ",ch_in,GDALGetDataTypeName(piBand->GetRasterDataType()) );

// Check raster type (8 or 16 bit) and set flag PCI data Type CHN_8U=1 CHN_16U=3

	if(EQUALN(GDALGetDataTypeName(piBand->GetRasterDataType()),"Byte",4)) data_type=CHN_8U;  	//PCI data types
	if(EQUALN(GDALGetDataTypeName(piBand->GetRasterDataType()),"Uint16",6)) data_type=CHN_16U;


	if ( data_type==CHN_8U) get_pix_val = get_u8;
	if ( data_type==CHN_16U) get_pix_val = get_u16;

	printf("Channel %d , RasterDataType =%d, Type = %s, PCI data_type = %d \n",
		ch_in, piBand->GetRasterDataType(), GDALGetDataTypeName(piBand->GetRasterDataType()), data_type );



//  Read the input image (whole or by lines)

  	printf("*Allocating memory to the image\n");

	if(data_type==CHN_8U) 
		{image_8b = (PixVal *) CPLMalloc(sizeof(PixVal)*Pixels*(int64)Lines); check_mem(image_8b);}
	if(data_type==CHN_16U) 
		{image_16b = (uint16 *) CPLMalloc(sizeof(uint16)*Pixels*(int64)Lines); check_mem(image_16b);}

	if(data_type==CHN_8U)
	  printf("\n Allocated memory (%I64d bytes) to the image. ",Pixels*(int64)Lines);
	if(data_type==CHN_16U)
	  printf("\n Allocated memory (%I64d bytes) to the image. ",2*Pixels*(int64)Lines);



// Read the whole input image IN ONE SHOT 

	if(by_image)
	{
 	printf("\n\n\t*Reading full image into memory ... \n\n");

	if(data_type==CHN_8U) 
		piBand->RasterIO(GF_Read, 0, 0, Pixels, Lines, image_8b, Pixels, Lines, GDT_Byte, 0, 0 );
	if(data_type==CHN_16U) 
		piBand->RasterIO(GF_Read, 0, 0, Pixels, Lines, image_16b, Pixels, Lines, GDT_UInt16, 0, 0 );

	}		// end of if(by_image)


// Read the image "line by line" instead (may use less memory)

	if(by_lines)
	{
 	printf("\n\n\t*Reading (by line) image into memory ... \n\n");

	if(data_type==CHN_8U) 
		{pafScanline = (PixVal *) CPLMalloc(sizeof(PixVal)*Pixels); check_mem(pafScanline); }	// create an 8-bit line buffer

	if(data_type==CHN_16U) 
		{pafScanline16 = (uint16 *) CPLMalloc(sizeof(uint16)*Pixels); check_mem(pafScanline16); }	// create a 16-bit line buffer

	for (i = 1; i <= Lines; i++)		// using  PCI notation image starting at (1,1)
	  {

	  if(data_type==CHN_8U) 
	    {
	    piBand->RasterIO(GF_Read, 0, i-1, Pixels, 1, pafScanline, Pixels, 1, GDT_Byte, 0, 0);
	    for ( j = 1 ; j <= Pixels ; j++ ) { bitnum = (i-1)*(int64)Pixels + j-1 ; image_8b[bitnum] = pafScanline[j-1]; }
	    }

	  if(data_type==CHN_16U) 
	    {
	    piBand->RasterIO(GF_Read, 0, i-1, Pixels, 1, pafScanline16, Pixels, 1, GDT_UInt16, 0, 0);
	    for ( j = 1 ; j <= Pixels ; j++ ) { bitnum = (i-1)*(int64)Pixels + j-1 ; image_16b[bitnum] = pafScanline16[j-1]; }
	    }

 	  if( (i/100)*100 == i ) printf("%d lines done\r", i);

	  }
	printf("\n");
	}		// endof if(by_lines)




// Close the file and return a pointer to image in memory


	//if (!imaFile_opened) GDALClose(ima_in);	// close only if managing on our own

	if(data_type==CHN_8U)	image_pointer = image_8b;
	if(data_type==CHN_16U)	image_pointer = image_16b;

	return(image_pointer);		// return pointer to memory containing image

}	// end of function Read_image()

//*********************************************************************************

// ### GDAL Write an 8bit or 16bit image

//*********************************************************************************


//void  write_image(void *image_pointer, char * file_out, int out_ch = 1)

void  write_image(void *image_pointer, char * file_out)
{

//char 		*fullfilename="", *filename;
char		fullfilename[250], *filename;
char 		*extension;

GDALRasterBand	*piBand;
GDALDataset	*ima_out;
GDALDriver 	*poDriver;

PixVal		*pafScanline, *image_8b;
uint16		*pafScanline16, *image_16b;

int 		data_type=1;
int		i, j;
int64		bitnum;

char 	ans[80];
int	by_lines=0; by_image=1;

int out_ch = 1;

if (ch_out != 0) out_ch = ch_out;		// "ch_out" should be a global variable
if (ch_out == 0) out_ch = 1;

//***************

// if by lines flag is set

if(bylines_flag)  { by_lines=1; by_image=0; }		

//***************

// Open (or create) an output image file


// Select the type of output file to produce (TIF or PCI) based on extension

//	printf("Output Filename as stated:  %s \n", file_out);
	
//	if (ch_out !=0) 
//	printf("Output channel as stated:  %d \n", ch_out);	

// Use fullfilename not to perturbe argv[2] - cause some chr operators are destructive

	strcpy(fullfilename, file_out);
	//printf("FULL Filename:  %s \n", fullfilename);

	filename = strtok(fullfilename,".");
	extension = strtok(NULL," "); 
	strcpy(Extension,extension);		// global variable for other prog.

//	printf("Filename:  %s \n", filename);
//	printf("Extension:  %s \n", extension);

	if (strncmp(extension,"tif",3) == 0) printf("\n\tOutput file extension is tif \n");
	if (strncmp(extension,"pix",3) == 0) printf("\n\tOutput file extension is pix \n");

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


// First, check if output file already exist (ask to overwrite)

	ima_out = (GDALDataset *) GDALOpen( file_out, GA_Update );

// IF PCI file, it is assumed to exist and we want to address a specific channel (likely given)

	if ( (ima_out != NULL) && (strncmp(extension,"pix",3) == 0) && (ch_out !=0) ) goto MAIN;	

	if (ima_out != NULL) 			// file exist
	  {
	  printf("\n\n ### Image %s already exist \n", file_out); 
	  printf("\n\t OK to overwrite FULL image file (Y/N)? \t");  fgets(ans,80,stdin);

	  if (ans[0] == 'y' || ans[0] == 'Y') goto CREATE;

	  //if (ans[0] == 'n' || ans[0] == 'N')
	  else	  		  // ans=NO or junk
		{
		printf("\n\tWould you like to overwrite a specific raster channel? \t");  fgets(ans,80,stdin);

	  	//if (ans[0] == 'n' || ans[0] == 'N')  {printf("\n\t** Will not overwrite anything ** \n"); goto EXIT;}
		
	  	if (ans[0] == 'y' || ans[0] == 'Y') 
		  {
		  printf("\n\tWhich channel would you overwrite ?? \t");  fgets(ans,80,stdin);
		  out_ch = (int) ans[0] - 48;		// temporary

		  printf("\n\t\t ### OUTPUT channel %d will be overwriten \n\n", out_ch);
		  goto MAIN;
		  }			
		  else {printf("\n\t** Will not overwrite anything ** \n"); goto EXIT;}   // ans=NO or junk

		}

	  }

//*****

CREATE:

//  Create the output tif 8-bit or 16-bit image file in which to move data

	if(data_type==CHN_8U) 
		ima_out = (GDALDataset *) poDriver->Create(file_out, Pixels, Lines, 1, GDT_Byte, NULL);
	if(data_type==CHN_16U) 
		ima_out = (GDALDataset *) poDriver->Create(file_out, Pixels, Lines, 1, GDT_UInt16, NULL);

	if (ima_out == NULL) 
	  {printf("\n\n PROBLEM opening output image file %s \n\n",file_out); exit(1);}

	fprintf(stdout,"\n\t*File %s was created and is opened for writing\n", file_out);

// Copy the metadata (GeoTransform, projection, ...)

	fprintf(stdout,"\nWriting image size, coordinates & projection to file %s\n",file_out);	

	ima_out->SetGeoTransform(adfGeoTransform);
	ima_out->SetProjection(Proj);
	
	//if( ima_in->GetProjectionRef() != NULL ) ima_out->SetProjection(ima_in->GetProjectionRef());


MAIN:

	if(by_image)
	{

	printf("\n\tWriting output image to file in one shot\n");

	//if (strncmp(extension,"tif",3) == 0) poBand = ima_out->GetRasterBand(1);	// ??only channel one as output for tif file 
	//if (strncmp(extension,"pix",3) == 0) poBand = ima_out->GetRasterBand(out_ch);
	
	poBand = ima_out->GetRasterBand(out_ch);		// more generic, could allow other muli-channel files

	if(data_type==CHN_8U)
		poBand->RasterIO(GF_Write, 0, 0, Pixels, Lines, image_pointer, Pixels, Lines, GDT_Byte,0, 0 );

	if(data_type==CHN_16U)
		poBand->RasterIO(GF_Write, 0, 0, Pixels, Lines, image_pointer, Pixels, Lines, GDT_UInt16,0, 0 );

	}		// endof if(by_image)


	if(by_lines)
	{

// Prep line buffers depending on 8 or 16 bit channel

	if(data_type==CHN_8U) 
		{pafScanline = (PixVal *) CPLMalloc(sizeof(PixVal)*Pixels); check_mem(pafScanline); }	// create an 8-bit line buffer
	if(data_type==CHN_16U) 
		{pafScanline16 = (uint16 *) CPLMalloc(sizeof(uint16)*Pixels); check_mem(pafScanline16); }	// create a 16-bit line buffer


// Read image lines of the specific channel(s) and copy to output image file

	//if (strncmp(extension,"tif",3) == 0) poBand = ima_out->GetRasterBand(1);	// ??only channel one as output for tif file 
	//if (strncmp(extension,"pix",3) == 0) poBand = ima_out->GetRasterBand(out_ch);
	
	poBand = ima_out->GetRasterBand(out_ch);		// more generic, could allow other muli-channel files


	printf("\nWriting image data to channel %d of file %s\n\n", 1, file_out);

	for (int y = 0; y < Lines; y++)
	  {

	  if(data_type==CHN_8U) 
	    {
	    poBand->RasterIO(GF_Write, 0, y, Pixels, 1, pafScanline, Pixels, 1, GDT_Byte,0, 0 );
	    }

	  if(data_type==CHN_16U) 
	    {
	    poBand->RasterIO(GF_Write, 0, y, Pixels, 1, pafScanline16, Pixels, 1, GDT_UInt16,0, 0 );	
	    }


 	  if( (y/100)*100 == y ) printf("%d lines done\r", y);

	  }
	printf("\n");

	}		// endof if(by_lines)

	
// Write file description (as created in main prohgram)
	
	//printf("\nDescription to Output image : %s \n", Description);
	poBand->SetDescription(Description);
	
	//if (!imaFile_opened) GDALClose(ima_out);	// close only if managing on our own

EXIT:
	return;

}	// end of function write_image()

//*********************************************************************************

// ### GDAL Open an 8bit tif bitmap, make it a 1bit bitmap for processing, and return pointer

//*********************************************************************************

PixVal * read_bitmap(char * filename, int bmp_no)

{
int 		i, j, ii;
int64 		bitnum, count;
char 		*extension;

PixVal 		*bitmap_8b, *bitmap_p;
GDALRasterBand	*piBand2;
GDALDataset	*bmp_in;
PixVal		*pafScanline;

	if(bylines_flag) { by_lines = 1; by_image = 0; }		// if by lines flag is set

/*
	if (imaFile_opened)		 // if file was already opened by other funct or prog
	  {
	  bmp_in = ima_in;		// only useful for PCI file, other are always distinct files
	  goto FileOpened;	
	  }
*/

	bmp_in = (GDALDataset *) GDALOpen(filename, GA_ReadOnly);		//pointer to file dataset 

	if (bmp_in == NULL) 
	     //{printf("\nPROBLEM opening bitmap image file %s \n\n", filename); exit(1);}
	     {printf("\nPROBLEM opening bitmap image file \"%s\" \n\n", filename); return(NULL);}
		 
	printf("\n\t**File '%s' was opened to read a bitmap \n",filename);

	//printf( "Driver: %s/%s\n",bmp_in->GetDriver()->GetDescription(),bmp_in->GetDriver()->GetMetadataItem( GDAL_DMD_LONGNAME ) );
	//printf( "Image size is %d x %d x %d\n\n", bmp_in->GetRasterXSize(), bmp_in->GetRasterYSize(),bmp_in->GetRasterCount() );


// Get and maybe Print geographic info of that bitmap file

	Proj2 = (char *) CPLMalloc(2000);			// to store geo projection info
	strncpy(Proj2,bmp_in->GetProjectionRef(),2000);
//	if( bmp_in->GetProjectionRef() != NULL)  printf( "Projection is %s \n\n", Proj2);

	  
// Check same image size as main image

	if ( (bmp_in->GetRasterXSize() != Pixels) || (bmp_in->GetRasterYSize() != Lines) )
		{printf("\n\n PROBLEM with image SIZE compatibilty of file %s \n\n", filename); exit(1);}


// Check same geographic region

	bmp_in->GetGeoTransform( adfGeoTransform2 );
	
 /*
	if( bmp_in->GetGeoTransform( adfGeoTransform2 ) == CE_None )
	  {
	  printf( "Origin = (%.2f,%.2f)\n", adfGeoTransform2[0], adfGeoTransform2[3] );
	  printf( "Pixel Size = (%.2f,%.2f)\n", adfGeoTransform2[1], adfGeoTransform2[5] );
	  }
 */
 
//	  printf( "Origin = (%.2f,%.2f)\n", adfGeoTransform[0], adfGeoTransform[3] );
//	  printf( "Pixel Size = (%.2f,%.2f)\n", adfGeoTransform[1], adfGeoTransform[5] );
	 

/*  
	if( (adfGeoTransform2[0] != adfGeoTransform[0]) || (adfGeoTransform2[3]!= adfGeoTransform[3]) ||
		(adfGeoTransform2[1]!= adfGeoTransform[1]) || (adfGeoTransform2[5]!= adfGeoTransform[5]) )
			{printf("\n\n PROBLEM with image GeoRef compatibilty of files %s \n\n",filename); exit(1);}
  */
 
 // Less precise check than above, but precise enough ( +-5cm), and NO ERROR

	if( (abs(adfGeoTransform2[0]-adfGeoTransform[0]) > 0.005) ||
		(abs(adfGeoTransform2[3]-adfGeoTransform[3]) > 0.005) ||
		(abs(adfGeoTransform2[1]-adfGeoTransform[1]) > 0.005) || 
		(abs(adfGeoTransform2[5]-adfGeoTransform[5]) > 0.005) )
		{
		printf("\n\n PROBLEM with image GeoRef compatibilty of files %s \n\n", filename); 
		printf("adfGeoTransform2[0]-adfGeoTransform[0] = %.3f\n", adfGeoTransform2[0]-adfGeoTransform[0]);
		printf("adfGeoTransform2[3]-adfGeoTransform[3] = %.3f\n", adfGeoTransform2[3]-adfGeoTransform[3]);		
		printf("adfGeoTransform2[1]-adfGeoTransform[1] = %.3f\n", adfGeoTransform2[1]-adfGeoTransform[1]);
		printf("adfGeoTransform2[5]-adfGeoTransform[5] = %.3f\n", adfGeoTransform2[5]-adfGeoTransform[5]);
		exit(1); 
		}


	printf("Image sizes and GeoRef coordinates are compatible\n");


// Check same Geo projection as ILLUMIN

/*
	if( bmp_in->GetProjectionRef() != NULL )  
	  {
	  strncpy(Temp, bmp_in->GetProjectionRef(),200);
	  strtok(Temp, "[[\"");			
	  //printf("1st section : %s \n", Temp);

 	  Proj2 = strtok(NULL, "[\"");     printf("Proj2: %s \n", Proj2);
 	  Datum2 = strtok(NULL, "[\"");	  //printf("Temp: %s \n", Datum2);
 	  Datum2 = strtok(NULL, "[\"");	  //printf("GEOGCS: %s \n", Datum2 );
	  Datum2 = strtok(NULL, "[\"");	  //printf("Temp: %s \n", Datum2);
 	  Datum2 = strtok(NULL, "[\"");	  printf("Datum: %s \n", Datum2 );
	  }

	if (Proj2 != Proj) printf("\n\n NOTE: It appears that ''%s'' geo. projection is not the same as main image file\n\n",argv[2]);
*/

	// if (Proj2 != Proj) {printf("\n\n PROBLEM: Geographic projection of ''%s'' is not the same as main image file\n\n",argv[2]); exit(1);}

FileOpened:

// Create bitmap to eventually output

	piBand2 = bmp_in->GetRasterBand(bmp_no);		// assuming one bitmap in that file (for the moment)

	if(piBand2 == NULL)  
		{ printf("\n\t*Unable to get bitmap layer as specified\n\n"); exit(-1);}
	
	//printf("Metadata item (NBITS): %s \n", piBand2->GetMetadataItem("NBITS","IMAGE_STRUCTURE") );	

	if ( piBand2->GetMetadataItem("NBITS","IMAGE_STRUCTURE") == NULL)
	  {
	  printf("\n\t*** Specified channel (%d) in file %s may not be a bitmap\n", bmp_no, filename);
	  exit(-1);
	  }


//  Reserve memory for the bitmap "perse"

	//printf("** Bitmap size in bytes: %I64d \n", bmsize);
	//bitmap_p = (PixVal *) malloc(bmsize);
	bitmap_p = (PixVal *) calloc(bmsize,1);	
	check_mem(bitmap_p);						
	

	if(by_image)
	{

// allocate memory for a "Full 8bit image" to read data in

	bitmap_8b = (PixVal *) malloc(sizeof(PixVal)*Pixels*Lines);  
	check_mem(bitmap_8b);

// Read input bitmap (1bit tif) in one shot as 8bit

	//printf("\n\n\tReading FULL 8bit Bitmap image into memory ... \n\n");

	piBand2->RasterIO(GF_Read, 0, 0, Pixels, Lines, bitmap_8b, Pixels, Lines, GDT_Byte,0, 0 );

// For testing as 8bit bitmap

	count = 0;
	for (ii=0; ii<Pixels*Lines; ii++) if (bitmap_8b[ii] == 1) count++;
	printf("\nPixels set in input 8bit bitmap = %Ii \n", count);


// Turn 8bit image into a bitmap for further processing (i.e.,to be compatible with PCI version of main code)
//	Into "isolbitbuf"

	printf("Turning input tif (nbits=1, read as 8bit) into a 1bit bitmap in memory\n");

	for ( i = 1 ; i <= (Lines) ; i++ )		// For compatibility with PCI(x,y), I kept image coordinates start at (1,1)
	for ( j = 1 ; j <= (Pixels) ; j++ ) 		// However, GDAL images start at zero (similar to bitmaps), so bytenum=bitnum
	  {
	  bitnum = (i-1)*(int64)Pixels + j-1 ;		// bitnum starts at zero, so does image, so bytenum=bitnum
	  if (bitmap_8b[bitnum])  setbit(bitmap_p, bitnum);
	  }
	//printf("\tFinished Turning input tif (nbits=1, read as 8bit) into a 1bit bitmap in memory\n");
	
	}	// endof if(by_image)


// *******

// Read bitmap line by line (may save memory) and create bitmap in that loop

	if(by_lines)
	{

	printf("\n\t*Reading bitmap (line by line) from %s \n\n", filename);

	pafScanline = (PixVal *) CPLMalloc(sizeof(PixVal)*Pixels);	// create an 8-bit single line buffer

	for ( i = 1 ; i <= (Lines) ; i++ )		// For compatibility with PCI(x,y), I kept image coordinates start at (1,1)
	  {
	  piBand2->RasterIO(GF_Read, 0, i-1, Pixels, 1, pafScanline, Pixels, 1, GDT_Byte, 0, 0 );	// read line

	for ( j = 1 ; j <= (Pixels) ; j++ ) 		// convert to real bitmap
		{
		bitnum = (i-1)*(int64)Pixels + j-1 ; 
		if (pafScanline[j-1]==1) setbit(bitmap_p,bitnum);			// convert to bit
		}

 	  if( (i/100)*100 == i ) printf("%d lines done\r", i);
	  }

	printf("\n");

	}		// endof if(by_lines)


// For testing 

	count = 0;
	for (ii=0; ii<bmsize*8; ii++) if (testbit(bitmap_p, ii)) count++;
	printf("Pixels set in input 1-bit bitmap = %Ii \n", count);

	safety_zone(bitmap_p);


	//if (!imaFile_opened) GDALClose(bmp_in);	// close only if managing on our own

	return(bitmap_p);		// return pointer to memory containing 1bit bitmap 


}	// End of read_bitmap()



//*********************************************************************************

// ### GDAL - Take a 1bit bitmap (pointer), make it an 8bit bitmap, write it into a tif file
//		that knows (NBITS=1) it's actually a 1bit bitmap (thus size based on 1bit)

//*********************************************************************************

void	write_bitmap(PixVal * bitbuf, char * file_out)
{

PixVal 		*bitmap_8b;
int 		i, j, ii;
int64 		bitnum, count;
GDALDataset 	*bmp_out;
GDALDriver	*poDriver;
PixVal		*pafScanline;

	poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");

	if(poDriver == NULL) 
	   {
	   printf("\n ###Cant find proper driver for this file type %s \n\n","GTiff"); 
	   exit( 1 );
	   }

// Create a tif file equivalent of a bitmap (NBITS=1)

  	papszOptions = CSLSetNameValue( papszOptions, "NBITS", "1" );
    	//papszOptions = CSLSetNameValue( papszOptions, "COMPRESS", "PACKBITS" );

	bmp_out = (GDALDataset *) poDriver->Create( file_out, Pixels, Lines, 1, GDT_Byte, papszOptions );

	if (bmp_out == NULL) {printf("\n\n ** PROBLEM opening output image file %s \n\n",file_out); exit(1);}

	fprintf(stdout,"\n\t**File '%s' was opened for writing.\n", file_out);


// Copy the metadata (GeoTransform, projection, ...)

	//fprintf(stdout,"\nWriting geographic data to file \"%s\" from original image.\n", file_out);	

	bmp_out->SetGeoTransform(adfGeoTransform);

	if( Proj != NULL ) bmp_out->SetProjection(Proj);
	if( ima_in->GetProjectionRef() != NULL ) bmp_out->SetProjection(ima_in->GetProjectionRef() );				

	poBand = bmp_out->GetRasterBand( 1 );

// Test 1-bit bitmap

	count = 0;
	//for (ii=0; ii<bmsize*8; ii++) if (testbit(bitbuf, ii)) count++;
	//printf("\n Pixels set in output 1-bit bitmap = %Ii \n", count);

	if(by_image)
	{
// Write output bitmap data out to a 1-bit tif file (*** but via an 8-bit buffer) in ONE SHOT

	//fprintf(stdout,"\n\t * Writing (in one shot) data to file %s  \n", file_out);

	bitmap_8b = (PixVal *) CPLMalloc(sizeof(PixVal)*Pixels*Lines); 	 // allocate memory for a "Full 8bit image" to write data out
	check_mem(bitmap_8b);

// Turn a bitmap in memory into a tif image (wrote via  8bit, yet nbits=1)

	//printf("** Turning bitmap in memory into a 1-bit tif image (wrote via 8bit, yet nbits=1) \n");

	for ( i = 1 ; i <= Lines ; i++ )		// For compatibility with PCI(x,y), I kept image coordinates start at (1,1)
	for ( j = 1 ; j <= Pixels ; j++ ) 		// However, GDAL images start at zero (similar to bitmaps), so bytenum=bitnum
	  {
	  bitnum = (i-1)*(int64)Pixels + j-1;		
	  if (testbit(bitbuf, bitnum)) bitmap_8b[bitnum] = 1; else bitmap_8b[bitnum] = 0;
	  //if( (i/100)*100 == i ) printf("%d lines done\r", i);
	  }

// Write image (here, a bitmap) in ONE SHOT

	poBand->RasterIO(GF_Write, 0, 0, Pixels, Lines, bitmap_8b, Pixels, Lines, GDT_Byte,0, 0 );

// Test count on 8bit bitmap

	count = 0;
	//for (ii=0; ii<Pixels*Lines; ii++) if (bitmap_8b[ii] == 1) count++;
	//printf("\n Pixels set in output 8bit bitmap (after out by image) = %Ii \n", count);

	}	// end of (by_image)


// Write output bitmap data out to a 1-bit tif file (*** but via an 8-bit buffer) one LINE at the time 

	count = 0;

	if(by_lines)
	{
	pafScanline = (PixVal *) CPLMalloc(sizeof(PixVal)*Pixels);	// create an 8-bit line buffer

	fprintf(stdout,"\nWriting output bitmap data \"line by line\" via an 8 bit buffer ....\n");

	for ( i = 1 ; i <= (Lines) ; i++ )		// For compatibility with PCI(x,y), I kept image coordinates start at (1,1)
	 {
	 for ( j = 1 ; j <= (Pixels) ; j++ ) 		// prep a line of image
	   {
	   bitnum = (i-1)*(int64)Pixels + j-1 ;			// bitnum starts at zero, so does image, so bytenum=bitnum
	   pafScanline[j-1] = 0;
	   if (testbit(bitbuf, bitnum)) { pafScanline[j-1] = 1; count++; }
	   }
	 poBand->RasterIO(GF_Write, 0, i-1, Pixels, 1, pafScanline, Pixels, 1, GDT_Byte, 1, 1);	// write a line
 	 if( (i/100)*100 == i ) printf("%d lines done\r", i);
	 }

	printf("\n Pixels set in output 8bit bitmap (after out by lines) = %Ii \n\n", count);

	}		// end of if(by_lines)
		



// Creating  and writing a decent COLOR TABLE  for a bitmap

	//printf("\n\t Making Color Table  \n");

// Prep an OUTPUT color table 

	//poBand->SetColorInterpretation(GDALColorInterp::GCI_PaletteIndex);	// not necessary 
		
	GDALColorTable * col_tab_out = nullptr;		// declare output colour table pointer
	col_tab_out =  &GDALColorTable();			// construct output colour table	
	//col_tab_out =  &GDALColorTable(GPI_RGB);			// construct output colour table
	
// Color entries
	
    GDALColorEntry * col_ent_out = nullptr;		// declare
	col_ent_out = &GDALColorEntry();			// construct (i.e., reserve memory)
	
	col_ent_out->c1 = 0; col_ent_out->c2 = 0; col_ent_out->c3 = 0; col_ent_out->c4 = 0;	
	//col_ent_out->c1 = 0; col_ent_out->c2 = 0; col_ent_out->c3 = 0; col_ent_out->c4 = 255;
	col_tab_out->SetColorEntry(0, col_ent_out);
	//col_tab_out->SetColorEntry(1, col_ent_out);		// for TESTING
	
	col_ent_out->c1 = 255; col_ent_out->c2 = 255; col_ent_out->c3 = 255; col_ent_out->c4 = 255;	
	col_tab_out->SetColorEntry(1, col_ent_out);
	//col_tab_out->SetColorEntry(0, col_ent_out);	// for TESTING

/*	
// NOTE: 
// For PCI, any STUPID color table will do as long as there is an "official" color table within the bitmap
// However, ArcGIS will respect the colors and even allow you to modify them.
// For example:
	col_ent_out->c1 = 10; col_ent_out->c2 = 20; col_ent_out->c3 = 30; col_ent_out->c4 = 255;		
	col_tab_out->SetColorEntry(0, col_ent_out);		// for TESTING
	col_ent_out->c1 = 40; col_ent_out->c2 = 50; col_ent_out->c3 = 60; col_ent_out->c4 = 255;
	col_tab_out->SetColorEntry(1, col_ent_out);		// for TESTING
*/


//  Print resulting color table to VERIFY
	
/*
	const GDALColorEntry * col_ent = nullptr;			// declare
	col_ent = &GDALColorEntry();			// construct (i.e., reserve memory)	 
	
	col_ent = col_tab_out->GetColorEntry(0);
 	//printf("\t\t* Color entries for OUTPUT  0  : %d %d %d %d \n\n", col_ent->c1, col_ent->c2, col_ent->c3, col_ent->c4);

	col_ent = col_tab_out->GetColorEntry(1);
 	//printf("\t\t* Color entries for OUTPUT  1  : %d %d %d %d \n\n", col_ent->c1, col_ent->c2, col_ent->c3, col_ent->c4);

*/
	
	//poBand->SetColorInterpretation(GCI_PaletteIndex);			// not necessary 
	
	//poBand->SetPaletteInterpretation(GPI_RGB);			// not necessary 

// Write Color Table out to bitmap(tif)  file

	printf("\n\tWriting Color Table to file \"%s\" \n", file_out);
			
	poBand->SetColorTable(col_tab_out);		


// Set "nodata values" for software that use that

	//printf("\n\n\tWriting NoDataValue and Description  \n\n");	
	poBand->SetNoDataValue(0);
		
// Write file description (as created in main prohgram)
	
	//printf("\nDescription to Output Bitmap : %s \n", Description);
	poBand->SetDescription(Description);			

// close file

	GDALClose(bmp_out);

	return;

} 				// End of write_bitmap()





/******************************************************************/

/* This is a function to convert a SINGLE tree crown (OR any polygon) 
	from a vector-form crown outline to a filled bitmap crown. 

	Also used to paint training areas into bitmaps.

	Also used to paint a single stand polygon into a bitmap
	possibly to be migrated to a stand image later
	
   NOTES:	
		- This version of vect2rast() assumes that vertices **where converted** 
		from georeferenced coordinates to image coordinates and if needed,
		to sub-area coordinates (DBIW) before entering vect2rast()
		- For border pixels: it is using center of pixel in or out as criteria
   		for that pixel being burned into the output bitmap
		- It is assumed that memory was already reserved for the output bitmap

		- Reintroduced set_flag (last parameter) to be used to erase islands 
		within polygon (set_flag=0) in some runs
*/

/*	For info.cause should be in itc_io_g.h
typedef struct
	{
	 double              x;
	 double              y;
	} GDBVertex2D;				//PCI Vertices
*/

void vect2rast_g(int nVertex, GDBVertex2D *pasVertices, unsigned char *bm_data, int xsize, int iFeat, int set_flag)

  {

   int  between(float a, float b, float c);

  int 	ii, jj, kk, iii, jjj, x, y, icount, ipos; 
  int	ixmin, ixmax, iymin, iymax;
  int64 bitnum;
  float xx, yy, xmin, xmax, ymin, ymax, slope, xp;
  float line_xx[30], line_xp[30], n_line_xx[30], n_line_xp[30];	//Dec 2024 - now 30 intersections, was 10

  extern int	xcg, ycg;		// center of gravity of current polygon to be passed to main

// for debugging - Print vertices

  //printf("\n***Now in vect2rast_g() ...\n"); 

// Debugging Print
/*  
  printf("\n Coordinates in vect2rast()\n");
  for (kk=0; kk<nVertex; kk++)
    {
    if (kk<10) { printf(" %d = %5.1f  %5.1f ", kk+1, pasVertices[kk].x, pasVertices[kk].y);
    if ( ((kk+1)/5)*5 == (kk+1) ) fprintf(stdout,"\n"); }
    }
  printf("\n");
 */ 
 
 	fill_count = 0;				// init pixel counts (to do area later) a VIP return parameter
	Poly_Outside = 0 ; 		// assume polygon is inside the image (by default)

  // gather x and y minimums and maximums positions (bounding box for polygon)/

  xmin = xmax = pasVertices[0].x;
  ymin = ymax = pasVertices[0].y;

  for (kk=1; kk< nVertex; kk++)
    {
    if (pasVertices[kk].x > xmax) xmax = pasVertices[kk].x;
    if (pasVertices[kk].x < xmin) xmin = pasVertices[kk].x;
    if (pasVertices[kk].y > ymax) ymax = pasVertices[kk].y;
    if (pasVertices[kk].y < ymin) ymin = pasVertices[kk].y;
    }

  ixmin = (int) (xmin); iymin = (int) (ymin);	
// ixmax = (int) (xmax+0.5); iymax = (int) (ymax+0.5);
 ixmax = (int) (xmax+1); iymax = (int) (ymax+1);			// be on the generous side
  xcg = (int) (0.5 + (xmax + xmin)/2);	
  ycg = (int) (0.5 + (ymax+ymin)/2);   /* approx center of grav. */

// For debugging - Print min and max

  // printf("xmin,xmax,ymin,ymax = %6.2f  %6.2f  %6.2f  %6.2f\n", xmin,xmax,ymin,ymax); 

    //printf("Integer ixmin,ixmax  iymin,iymax  xcg,ycg = %d,%d  %d,%d  %d,%d\n", ixmin,ixmax,iymin,iymax,xcg,ycg);	

// Check if part of polygon is **OUT OF IMAGE**

 
   if ( (ixmin < 1) || (iymin < 1) || (ixmax > Pixels) || (iymax > Lines) ) 
	{
	//printf("NOTE: A part of this polygon is outside the image area.\n");
	//printf("NOTE: This polygon will be DISREGARDED in signature generation.\n");
	//printf("vect2rast() : Polygon ID = %4d -- Considered outside the area\n", iFeat);
	//printf("ixmin=%d, ixmax=%d, iymin=%d, iymax=%d\n", ixmin, ixmax, iymin, iymax);
	//printf("xmin=%.2f, xmax=%.2f, ymin=%.2f, ymax=%.2f\n", xmin, xmax, ymin, ymax);	
	fill_count = 0;
	Poly_Outside = 1 ;		// flag
	return;
	}
 
 
// Display action
	
	//printf("\n\t Burning shp file polygon %d into given bitmap\n", iFeat);

// scan the square area delineated by xmin,xmax,ymin,ymax 

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



	// printf("\t For line %d, icount = %d \n", y, icount);

	if ( icount == 1) 
	{
	fprintf(stdout,"\n #### ERROR  ### Shape %d in current layer may not be a closed shape\n", iFeat); 
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

/*	FILL crown between pairs of line intersections.
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
	  //setbit(bm_data,bitnum); fill_count++;
	   if(set_flag) 
		{ setbit(bm_data,bitnum); fill_count++; }
	  else
		{ clearbit(bm_data,bitnum); fill_count--; }
	  x++;
	  }
	}

     }		/* end of for each line */


/* FOR DEGUGGING */

/*
  fprintf(stdout,"\nRasterized tree crown: \n");
  fprintf(stdout," Covers area xmin,xmax,ymin,ymax = %d,%d,%d,%d\n", ixmin,ixmax,iymin,iymax);


  for(iii = iymin; iii <= iymax; iii++)
   {
    for(jjj = ixmin; jjj <= ixmax; jjj++)
	{
	bitnum = (iii-1)*(int64)xsize + jjj-1;
	fprintf(stdout," ");
	if(testbit(bm_data,bitnum)) fprintf(stdout,"X");
	else  fprintf(stdout,".");
	}
   fprintf(stdout,"\n");
   }
   fprintf(stdout,"\n");

*/

   //printf("\nvect2ras_g() burned %d pixels for this crown or polygon \n", fill_count);

  }			// end of function vect2rast_g() 

//**********************************************************************************

//*******************************************************
//*******************************************************
//*******************************************************


// ### GDAL - Function to read a layer of closed polygons (from a file)
// and create a bitmap for ITC Suite processing ** Returns pointer to that bitmap

// Mostly used for simple polygon (no island) like polygon ITCs or TAs

// Convertion to image coordinates is done within vector2bitmap()

// By *comparison*, for vect2rast_g() convertion to image coordinates is done
// before calling the function and it typically deals with more complex polygons
// and is often called recursively to deal with outside, then inside vertices
// as in more complex forest stands polygons

// However, vector2bitmap() actuall calls vect2rast_g()

//*******************************************************


PixVal * vector2bitmap(char * filename, int segm_in)
{

#include "ogrsf_frmts.h"	// For OGR

int  	between(float a, float b, float c);

extern	int	Lines, Pixels, Channels;
extern int64 	bmsize;		// size of bitmap in bytes 

int64	bitnum;
int		kk, nVertex;
int		layer_count, feat_count, field_count, iFeat;
int		tdbiw[4];
PixVal	*bitmap_p;
char	answer[40];

GDBVertex2D	*pasVertices;

//GDALDataset 	*seg_in;

OGRLayer  		*piLayer, *poLayer;
OGRFeature 		*piFeature, *poFeature;
OGRFeatureDefn	*piFDefn, *poFDefn;
OGRFieldDefn	*piFieldDefn, *poFieldDefn;
OGRSpatialReference *piSRS,  *poSRS;
OGRGeometry 	*piGeometry, *poGeometry;
OGRPolygon 		*piPolygon;
OGRLinearRing	*piExteriorRing;

	
//Holds Coordinates of Polygon Shapefile

std::vector<PolygonFeature> PolygonLayer;
PolygonFeature Polygon;

OGRPoint ptTemp;
int NumberOfExteriorRingVertices, NumberOfInnerRings; 

// reserve space for 500 vertices (for the moment)
			
	pasVertices = (GDBVertex2D *) calloc(500,16);


//if (imaFile_opened) goto FileOpened;		// if file already opened bypass file opening

// Open for reading vector file (shp or PCI vector layer)

	seg_in = (GDALDataset*) GDALOpenEx(filename, GDAL_OF_VECTOR, NULL, NULL, NULL );

	if( seg_in == NULL )
	{
	    printf( "Failed to open input file.\n" );
	    exit(-1 );
	}

	fprintf(stdout,"\n**File '%s' was opened to read vector layer\n", filename);


// Open layer pointed to by user via 2nd argument (segm_in)

	layer_count = seg_in->GetLayerCount();
	//fprintf(stdout,"\n\t*Number of layers in vector file is %d \n\n",layer_count);
	if(segm_in > layer_count) {printf("\n\n ERROR - Requested layer > available layers \n\n"); exit(-1);}

FileOpened:

	piLayer = seg_in->GetLayer(segm_in-1);			// Layer numbers start at zero
	//piLayer->ResetReading();				// VERY IMPORTANT (if you use GetNextFeature() 
	//fprintf(stdout,"\n\tAccessing layer %d \n\n",segm_in);


// Number of features (i.e., polygons) in that layer AND number of fields in that layer

	piFDefn = piLayer->GetLayerDefn();
	feat_count = piLayer->GetFeatureCount();
	field_count = piFDefn->GetFieldCount();
	
	printf("Layer %d of current file has %d features (shapes) with %d fields each\n\n", 
				segm_in, feat_count, field_count);

// Create output bitmap to burn-in polygon

	//bitmap_p = (PixVal *) CPLMalloc(bmsize);	// memory space for output bitmap
	bitmap_p = (PixVal *) calloc(bmsize,1);	// memory space for output bitmap	
 	check_mem(bitmap_p);
	//printf("\nMemory reserved for bitmap in which to burn vector delineated areas\n");


//**********************************************************************

// LOOP to burn each polygon into a bitmap

//*********************************************************************

	//printf("\n###  OK to continue(y/n): ");  scanf("%s",answer);
	
// Loop through all the individual shapes (trees, polygons)

	piLayer->ResetReading();

    for (iFeat=0; iFeat<feat_count; iFeat++)
    //for (iFeat=0; iFeat<2; iFeat++)		// Only a few polygons for testing
    {   
	//printf("\nAccessing feature %d \n", iFeat);				// #####
	
	//piFeature = piLayer->GetFeature(iFeat);			// for loop with iFeat, rather than next feature
    piFeature = piLayer->GetNextFeature();
	if(piFeature == NULL) 	
		{printf("Unable to point to feature %d of %d in layer %d \n\n", iFeat, feat_count,segm_in); exit(-1); }	
	
    piGeometry = piFeature->GetGeometryRef(); 
	//printf("\n For feature %d \t Geometry is : %d \n", iFeat, wkbFlatten(piGeometry->getGeometryType()));
	
    if ( piGeometry != NULL && wkbFlatten(piGeometry->getGeometryType()) == wkbPolygon )
	{
	piPolygon = (OGRPolygon *) piGeometry;
	
	//Polygon.PolygonsOfFeature.resize(1);

	NumberOfInnerRings = piPolygon->getNumInteriorRings();	
	//printf("Number of inner rings = %d \n", NumberOfInnerRings);

	if(NumberOfInnerRings > 0)
	  {
	  printf("Number of inner rings = %d \n", NumberOfInnerRings);
	  printf("\t *** CAN NOT DEAL with InnerRings in trees - EXIT \n");
	  exit(-1);
	  }

	piExteriorRing = piPolygon->getExteriorRing();
	NumberOfExteriorRingVertices = piExteriorRing->getNumPoints();

// Convert GEOGRAPHIC COORDINATES  to image coordinates

	for ( int k = 0; k < NumberOfExteriorRingVertices; k++ )
             {
             piExteriorRing->getPoint(k,&ptTemp);
			 
		   	pasVertices[k].x = ( ptTemp.getX()- topleftX ) / xpixsz;		// convert UTM to image coordinates
			pasVertices[k].y = ( topleftY - ptTemp.getY()  ) / ypixsz;
			//if (k < 10) printf( "\tIma:  %.1f  %.1f\n", pasVertices[k].x, pasVertices[k].y); 	// PRINT some IMA vertices


// Check if outside the image

		if( (pasVertices[k].x < 0) || (pasVertices[k].y < 0) || (pasVertices[k].x > Pixels) || (pasVertices[k].y > Lines))
		  {
		   printf("\nShape %d in layer appears outside the image and will be skipped\n",iFeat);
		   break;
		  }


             }		// end of loop for vertices to convert to image coordinates

              //PolygonLayer.push_back(Polygon);

	    }		// end of if it's a polygon

	//printf("NumberOfExteriorRingVertices converted to image coordinates = %d \n", NumberOfExteriorRingVertices);

	 
// Paint tree crown in the output bitmap

	nVertex = NumberOfExteriorRingVertices;

	vect2rast_g(nVertex, pasVertices, bitmap_p, Pixels, iFeat, segm_in);	// for full crown analysis

    OGRFeature::DestroyFeature( piFeature );		// erase content on GDAL Heap to use for next one
   
    }	//end of feature (polygon) loop


    return(bitmap_p);		// return pointer to memory containing 1bit bitmap 

}	// end of function vector2bitmap


//*******************************************************


//*******************************************************

// check return value from MALLOC to make sure it is OK 

void check_mem(void *memory_ptr)
{
	if (memory_ptr == NULL )
	{
	fprintf(stderr,"\n ITC program - MEMORY ALLOCATION ERROR. \n");
	exit(-1);
	}
}

//*******************************************************

// This function creates a safety zone around a bitmap for fills() not
// to get out of image array and crash the program 

void safety_zone(unsigned char *bitmap)
{
  extern int Lines, Pixels, Channels;
  int i,j;

  for (j=1, i=1 ; i<=(Lines) ; i++ )	  clearbit(bitmap,((i-1)*(int64)Pixels + j-1) );
  for (j=Pixels, i=1 ; i<=(Lines) ; i++ ) clearbit(bitmap,((i-1)*(int64)Pixels + j-1) );
  for (i=1, j=1 ; j<=(Pixels) ; j++ )	  clearbit(bitmap,((i-1)*(int64)Pixels + j-1) );
  for (i=Lines, j=1 ; j<=(Pixels) ; j++ ) clearbit(bitmap,((i-1)*(int64)Pixels + j-1) );
}

//******************************************************************


// This function tell whether the first value (a) is between the other two (b,c)

int between(float a, float b, float c)
{
  if( (c > b) && (a<c) && (a>=b) ) return(TRUE);
  if( (b > c) && (a<b) && (a>=c) ) return(TRUE);
  return(FALSE);
}


//***************************************************************

// functions to read 8bit image data (now, with int64 position) 
 

int32 get_u8(void * image_8b, int64 pos)
  {
  return *((PixVal *)image_8b + pos);
  }

/*******************************************************/

int32 get_u16(void * image_16b, int64 pos)
 {
  return *((uint16 *)image_16b + pos);
  }

  
/*******************************************************/

int32 get_u32(void * image_32b, int64 pos)
 {
  return *((uint32 *)image_32b + pos);
  }
/*************************************************/
/*************************************************/


