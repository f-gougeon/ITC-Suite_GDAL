/* 
Program name: 	glinv_g.cpp 
Author: 	Francois A. Gougeon

Description:

C
C	Program to invert grey levels in images.
c	Works with 8-bit, 16-bit or 32-bit images, but only to 8-bit output image
C
C
C1	PARAMETERS
C
C	GLINV is controlled by the following global parameters:
C
C	Name		Prompt							Count	Type
C
C	FILE		Database File Name					64	Char
C	DBIC		Input channel						1	Int	
C	DBOC		Output channel 						1	Int
c	NORMCRIT	Normalization criteria(Raw/Navg/Nmax)	64	Char
C	REPORT		Reporting device					64	Char
C

c	NORMCRIT	Normalization criteria (Raw/Navg/Nmax)
C
c	NORMCRIT (normalization criteria) selects the type of normalization (or not)
c	used in generating the output image (displacement of data range).
c	Otherwise, grey levels would all end up in upper part of full range.
c
c		Raw	no normalization on channel data
c			(i.e., gl 0-59 end up at 255-195, for an 8bit image)
c		Navg	data normalized by the channel average grey level
c		Nmax	data normalized by the channel maximum useful grey level



Opening a "pix" (or other format) file, printing no of pixels and lines, geographic info, ...
and read specified channel (as third argument) and write a reversed grey level image to 
the output file (2nd argument), often a "tif" file OR a specific channel of an existing file.

PROGRAM USAGE:

Run as (for examples):

>  glinv_g input_file output_file NORMCRIT	

>

c *****************************

History:

V1.1	Oct. 2023 	Francois Gougeon

	- From very old glinv.cpp and ndvi_ima_g

	- NOT to assume that all files are in the default directory from which the program is run 
			
	- Allow program to automatically create output file name base file name (using "-" or "#")
			
	
*******************************************************************************

NOTES:	
	- If given images have  32 bit channels, use gdal_translate to make them 16 b or 8bit
	
	- 32 bit images are not really 32 bits, barely 16 bits. It is a format!
	
	 > gdal_translate -ot Byte  -b 4 -scale 0 1400 nl_pp_833591_lc_img_1.tif nl_pp_833591_lc_img_8b_NIR.tif

*********************
Ackowlegment to GDAL:

GDAL - Geospatial Data Abstraction Library: Version 2.1.1 (July2016, 64bit), 
GDAL - Geospatial Data Abstraction Library: Version 3.0.0 (Dec. 2019, 64bit),
Open Source Geospatial Foundation, 
Thanks Frank (Warmerdam)


*******************************************************************************************
******************************************************************************************
*/

#define VERSION "v1.1"
#define PROG_NAME "GLINV"

#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <time.h>       // time_t, struct tm, time, localtime 
#include <math.h>


#include "gdal_priv.h"		// For GDAL library
#include "ogrsf_frmts.h"	// For OGR

#include "ITC-Suite_g.h"		// my newest GDAL variable setup
#include "itc_io_g.h"		// my newest GDAL image/bitmap input/output
#include "bitops.h"		// bit operations on bitmaps (mostly macros to be faster)

#include "hist_g.h"

//#include "cpl_string.h"
// #include "tiffio.h"

// function declarations 

void upper_case(char *);

typedef unsigned char  PixVal;
//typedef unsigned long  uint64;
typedef unsigned short int uint16;

	
hist_inf *ph; 		// pointer to structure to store info extracted from histogram by ana_hist()

int	Pixels, Lines, Channels;

time_t rawtime;
struct tm * timeinfo;


int main(int argc, char* argv[])
{

GDALDataset	*ima_in, *ima_out;
double		adfGeoTransform[6];
GDALDriver 	*piDriver,*poDriver;

GDALRasterBand	*piBand, *poBand;

PixVal		*In_line1, *In_line2, *Out_line;
uint16		*In_line1_16b, *In_line2_16b;
float		NIRp, REDp;

char 	**papszMetadata;
char	ima_description[64];

int		i, j, ii=0, jj, k, kk;
int 	ch_out=1, ch_in=1, input_ch[2], no_ch=1;
int 	data_type = 1;			// 8bit images as input by default - output always 8bit


char 	Proj[80], ans[80], fullfilename[130], shortfilename[130];
char	 *file_out;
char	fullfileout[130];
char 	*filename, *extension, temp[130];
char 	*tstring, *p, *pch;


char	*basefname;						// ** just a pointer **
char	basefilname[130];
int		basef_len;	

char normaliz[64];


int 	histo[65536], m_count;
int 	hsize, first, last, mode;

int 	xsize, ysize;
int 	pix, max, nzcount;
float	avg;
int 	avg_gl[8], min_gl[8], max_gl[8];
int 	type1, type2, ppos;


// Generic stuff -- presently output image is always 8bit

PixVal		*pafScanline, *image_8b, *image_8b_out;
uint16		*pafScanline16, *image_16b, *image_16b_out;
//uint32		*pafScanline32, *image_32b, *image_32b_out;

PIX_FUN_PTR	get_pix_val; 		 // pointer that allows us to deal with  many types of image (8,16u,16s) 

float InPix, OutPix;

//*************************************************
//*************************************************

/* Print Program Header and time */

	time (&rawtime);
	timeinfo = localtime (&rawtime);

	fprintf(stdout,"\n\n\t\tStarting %s (%s) at %s\n", PROG_NAME, VERSION, asctime(timeinfo));

// Check input parameters (i.e., agrv[*])

	if (argv[1]== NULL) 
	  {printf("\n\n PROBLEM with input image %s \n",argv[1]);
	  printf("\tHave an INPUT image as first argument on command line \n");
	  printf("USAGE:   glinv_g input_file output_file NORMCRIT \n\n");
	  exit(1);
	  } 

	if (argv[2] == NULL) 
	  {printf("\n\n PROBLEM with output image %s \n",argv[2]);
	  printf("\tHave an OUTPUT image as second argument on command line \n\n");
	  printf("USAGE: glinv_g input_file output_file NORMCRIT \n\n");
	  exit(1);
	  } 

	if (argv[3] == NULL) 
	  {
	  printf("\n\t ### ERROR ### a third argument is needed \n");
	  printf("\nThat third argument is the Normalization Criteria(Raw/Navg/Nmax) \n");
	  printf("USAGE: glinv_g input_file output_file NORMCRIT \n\n");
	  exit(1);
	  }

	 // exit(-1);			// for debugging
	 
	 
// Check if a channel is mentionned with the input image filename (via a comma)
// NB : Not applicable to ArcGIS interface (ie, for cmd line)

	strncpy(temp, argv[1], 250);

	p = strtok(temp, ","); 	 		
	p = strtok(NULL, ",");
	
	ii = 0;
	while(p != NULL)
	  {
 	  //printf("%s\n", p); 
	  input_ch[ii++]= strtol(p,NULL, 10);
	  p = strtok(NULL, ",");
	  }
	no_ch = ii;	 
	 
	 
	 if(no_ch == 0)  ch_in = 1;
	 if(no_ch == 1)  ch_in = input_ch[0];
	 if(no_ch == 2)	 {printf("\n\n** ERRORR** Only one input channel is tolerated\n\n"); exit(-1);}
	 
	 //printf("\nInput Channel to be considered = %d \n\n", ch_in);

	 //exit(-1);			// for debugging
	
	
//************************

// Open INPUT  image file
  
	GDALAllRegister();  

	ima_in = (GDALDataset *) GDALOpen( argv[1], GA_ReadOnly );

	if (ima_in == NULL) 
	  	{printf("\n\n PROBLEM opening input image file %s \n\n", argv[1]); exit(1);}

	printf("***File opened for reading :  %s\n\n", argv[1]);


// Detect input image file type (via extension)

	strcpy(fullfilename, argv[1]);
	filename = strtok(fullfilename,".");
	extension = strtok(NULL," "); 	
//	printf("Filename:  %s \n", filename);
//	printf("Fullfilename:  %s \n", fullfilename);
//	printf("Extension:  %s \n", extension);

	if (strncmp(extension,"tif",3) == 0)
	  piDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
	if (strncmp(extension,"pix",3) == 0)
	  piDriver = GetGDALDriverManager()->GetDriverByName("PCIDSK");
	if (strncmp(extension,"dat",3) == 0)
	  piDriver = GetGDALDriverManager()->GetDriverByName("ENVI");

	if(piDriver == NULL) 
	   {
	   printf("\n ###Cant find proper driver for this type of file ...\n\n"); 
	   printf("Filename:  %s \n", filename); printf("Extension:  %s \n", extension);
	   exit( -1 );
	   }

// Print generic info (driver used, ... )

	// printf( "Driver: %s/%s\n",
          // ima_in->GetDriver()->GetDescription(),
          // ima_in->GetDriver()->GetMetadataItem( GDAL_DMD_LONGNAME ) );

	printf( "Image size is %d x %d x %d\n",
          Pixels = ima_in->GetRasterXSize(), Lines = ima_in->GetRasterYSize(), Channels = ima_in->GetRasterCount() );

// Print geographic info

	if( ima_in->GetProjectionRef() != NULL )  
	  {
	  strncpy(Proj, ima_in->GetProjectionRef(), 30);
	  printf("Projection is '%s'\n", Proj);
   	  //printf( "Projection is '%s'\n\n", ima_in->GetProjectionRef() );
	  }

	if( ima_in->GetGeoTransform( adfGeoTransform ) == CE_None )
	  {
	  printf( "Origin = (%.3f,%.3f)\n",
            adfGeoTransform[0], adfGeoTransform[3] );
	  printf( "Pixel Size = (%.3f,%.3f)\n\n",
            adfGeoTransform[1], adfGeoTransform[5] );
	  }

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
//	printf("Shortfilename Last:  %s \n", shortfilename);
	
	
//	 exit(-1);			// for debugging


// Get pointer to band (channel)
	
	piBand = ima_in->GetRasterBand(ch_in);
	
	printf("Input Channel %d is of Type = %s\n", ch_in, GDALGetDataTypeName(piBand->GetRasterDataType()) );

// Check input raster type (8 or 16 bit) and set flag using PCI data Type (CHN_8U=1 CHN_16U=3)

	if(EQUALN(GDALGetDataTypeName(piBand->GetRasterDataType()),"Byte",4)) data_type=CHN_8U; 
	if(EQUALN(GDALGetDataTypeName(piBand->GetRasterDataType()),"UInt16",6)) data_type=CHN_16U;	
//	if(EQUALN(GDALGetDataTypeName(piBand->GetRasterDataType()),"UInt32",6)) data_type=CHN_32U;	




 

//*******************

// Create base file name   (prepare if need to create outut file name)
	
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
	
//	printf("Base file name ::  %s \n", basefilname);	

	// exit(-1);			// for debugging



//************************

// Create OUTPUT image file name (if none given) else use given


if ( EQUALN(argv[2],"-",1 ) || EQUALN(argv[2],"#",1 ) )			
	{
	//basefname = strtok(fullfilename,".");
	//basefname = strtok(basefname,"_");				
	//printf("basefname  :  %s \n", basefname);	
	
	printf("\nSecond argument is \"-\" or \"#\", which implies no output filename given\n");
	printf("A new output image file name will be created from the \"base file name\" \n\n"); 
	basefname = temp;	 				// fake to init basefname as a char array (just a pointer)
	strcpy(basefname, basefilname);	
	printf("Base file name ::  %s \n", basefilname);
	
	file_out = strncat(basefname,"_Inv",4);	  	
	strncat(file_out,".tif",4);					// output image is forced to be a tif	
	}	
else								// if output file name given, just use that name
	{
	//basefname = strtok(fullfilename,".");		// fake to init basefname
	//strcpy(basefname,argv[2]);
	//basefname = strtok(basefname,".");
	file_out = temp;				// fake to init file_out as char array
	strcpy(file_out,argv[2]);	
	}

	strcpy(fullfileout, file_out);
 	printf("\n*Output image file wil be named \"%s\" \n", fullfileout);

	//exit(-1);			// for debugging

//************************

// With user given name, check for unacceptable file extension, thus file type

	strcpy(temp, fullfileout);
	filename = strtok(temp,".");
	extension = strtok(NULL," "); 
//	printf("\nFilename:  %s \n", filename);
//	printf("Extension:  %s \n", extension);

	if (strncmp(extension,"tif",3) != 0)
	  {printf("\n\t*** ERROR *** Output file need to be a tif -- EXITING !!! \n"); exit(-1); }


//************************

// Output file is to be TIFF  only 

	  poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");


//	exit(-1);			// for debugging
			
		
// **********************		
		
	
// Check Normalization criteria to used (i.e., check argv[3] )

	printf("\n\t*Normalization criteria to use: %s \n", argv[3]);
	
	strcpy(normaliz,argv[3]);		// put into normaliz variable
		
	upper_case(normaliz);			// change to upper cases to facilitate checks

	if(strncmp(normaliz,"RAW",3)==0)  
	fprintf(stdout,"\t\t-> No special normalization will be applied ...\n");
	else if(strncmp(normaliz,"NAVG",3)==0)
	fprintf(stdout,"\t\t->Using average of image to situate output distribution ...\n");
	else if(strncmp(normaliz,"NMAX",3)==0)
	fprintf(stdout,"\t\t->Using useful range of image to situate output distribution....\n");
	else
	  {
	  printf("\n\t ### ERROR ### The third argument has to be one of Raw/Navg/Nmax) \n");
	  printf("\nThat third argument is the Normalization Criteria(Raw/Navg/Nmax) \n");
	  printf("Typical Usage: glinv_g input_file output_file Raw \n\n");
	  exit(1);
	  }
	

// *************************************************	

// First, check if OUTPUT  file already exist (ask to overwrite)

	//ima_out = (GDALDataset *) GDALOpen( argv[2], GA_Update );
 	//printf("\n\t*Output image file will be named \"%s\" \n", fullfileout);
 	
	ima_out = (GDALDataset *) GDALOpen( fullfileout, GA_Update );

	if (ima_out == NULL) goto Create;		//file does not exist, so need to create
		
	if (ima_out != NULL) 
	  {
	  printf("\n\n ### OUTPUT Image %s already exist \n", fullfileout); 
	  printf("\n\t OK to overwrite FULL output image file (Y/N)?   ");  fgets(ans,80,stdin);

	  if (ans[0] == 'y' || ans[0] == 'Y') goto Data_IO;

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

		  printf("\n\t\t ### OUTPUT channel will be overwriten (or created??) \n\n");
		  goto Data_IO;
		  }

		}

	  }
 
 
 // *************************************

//  Create the output image file in which to write output image (same type as input (8b or 16b) 
 
 Create:	

	if(data_type==CHN_8U)
		ima_out = (GDALDataset *) poDriver->Create( fullfileout, Pixels, Lines, 1, GDT_Byte, NULL );
	
	if(data_type==CHN_16U)
		ima_out = (GDALDataset *) poDriver->Create( fullfileout, Pixels, Lines, 1, GDT_UInt16, NULL );	

	if (ima_out == NULL) 
	  {printf("\n\n PROBLEM opening output image file %s \n\n", fullfileout); exit(1);}

	printf("\n\t*File %s was created and is opened for writing\n", fullfileout);


// Copy the metadata (GeoTransform, projection, ...)

	fprintf(stdout,"\n\t*Writing GeoTransform and Projection to output file %s\n", fullfileout);	

	ima_out->SetGeoTransform(adfGeoTransform);
	ima_out->SetProjection(ima_in->GetProjectionRef() );

//	exit(-1);			// for debugging	
	
	
	
//**************************************
//**************************************

// Allocate memory and Read image of the specific channel 

Data_IO:
		
	
// Get pointer to output channel 

	poBand = ima_out->GetRasterBand(ch_out);
		
	printf("\nOutput Channel %d is of Type = %s\n",
		ch_out, GDALGetDataTypeName(poBand->GetRasterDataType()) );
	
	
// Double checking on image types	

	if ( poBand->GetRasterDataType()  !=  piBand->GetRasterDataType() )
		{
		printf("\n\t*** ERROR *** Input & output channels need to be of the same type -- EXITING !!! \n"); 

		printf("\n\tInput Channel %d , RasterDataType =%d, Type = %s  \n",
			ch_in, piBand->GetRasterDataType(), GDALGetDataTypeName(piBand->GetRasterDataType()) );
		
		printf("\n\tOutput Channel %d , RasterDataType =%d, Type = %s \n",
			ch_out, poBand->GetRasterDataType(), GDALGetDataTypeName(poBand->GetRasterDataType()) );

		exit(-1);
		}

	//exit(-1);			 // for debugging


// Create buffers for reading/writing images

	printf("\nCreating image size buffers for reading/writing images\n"); 

// Allocate memory for images depending if 8b or 16b or 32b image 

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
/* 
	if(data_type==CHN_32U) 
		{
		image_32b = (uint32 *) CPLMalloc(sizeof(uint32)*Pixels*(int64)Lines); check_mem(image_32b);
		//image_32b_out = (uint32 *) CPLMalloc(sizeof(uint32)*Pixels*(int64)Lines); check_mem(image_32b_out);
		}
 */	
 
	
		
// Info

	if(data_type==CHN_8U)
	  printf("\nAllocated %Id bytes to 8bit input image (same for output)", Pixels*(int64)Lines);
	if(data_type==CHN_16U)
	  printf("\nAllocated %Id bytes to 16bit input image (same for output) ", 2*Pixels*(int64)Lines);
  
//	if(data_type==CHN_32U)
//	  printf("\nAllocated %Id bytes to 32bit input image (8bit for output) ",4*Pixels*(int64)Lines);

	//exit(-1);			 // for debugging

// READ  the whole input image (IN ONE SHOT)

 	printf("\n\n\t*Reading full image into memory ... \n\n");

	if(data_type==CHN_8U) 
		piBand->RasterIO(GF_Read, 0, 0, Pixels, Lines, image_8b, Pixels, Lines, GDT_Byte, 0, 0 );
	if(data_type==CHN_16U) 
		piBand->RasterIO(GF_Read, 0, 0, Pixels, Lines, image_16b, Pixels, Lines, GDT_UInt16, 0, 0 );
//	if(data_type==CHN_32U) 
//		piBand->RasterIO(GF_Read, 0, 0, Pixels, Lines, image_32b, Pixels, Lines, GDT_UInt32, 0, 0 );
	
	
// Set up pixel access

	if ( data_type==CHN_8U) get_pix_val = get_u8;
	if ( data_type==CHN_16U) get_pix_val = get_u16;
//	if ( data_type==CHN_32U) get_pix_val = get_u32;	
	
	//exit(-1);			 // for debugging


// Get channel HISTOGRAM to check for MORE REALISTICS image maximum range

	printf("\n\t ** Generating input image histogram ...\n\n");

	ph = (hist_inf *) malloc(sizeof(hist_inf));   // common block to get info from ana_histo()
	
	if (data_type == CHN_8U) hsize = 256;
	if (data_type == CHN_16U) hsize = USHRT_MAX;
//	if (data_type == CHN_32U) hsize = USHRT_MAX;		// 32 bit image are onot 32 bit, barely 16 bit
	
	for (kk = 0; kk < hsize; kk++) histo[kk] = 0;
 
// Generated histogram of input image

	if ( data_type==CHN_8U) gen_hist(image_8b, get_pix_val, Lines*Pixels, histo);
	if ( data_type==CHN_16U) gen_hist(image_16b, get_pix_val, Lines*Pixels, histo);	
//	if ( data_type==CHN_32U) gen_hist(image_32b, get_pix_val, Lines*Pixels, histo);
	
// Analyse histogram just gathered 	
	
	printf("\n\tFinding grey level range of input image (histogram analysis)\n\n");
	
	init_histo_struct(ph);

	ana_hist(histo, hsize, ph);		// new ana_hist, info passed via structure

	
	printf("\nInput image mean %5.1f \n",  ph->mean);

	printf("Channel useful range (%d,%d) : \n", ph->first, ph->last);
 
 	//exit(-1);			 // for debugging
 
//**************************************************
//******************************************************

	printf("\nCalculating for output image inverted grey levels ... \n");

	for ( i = 0 ; i < Lines ; i++ )
	for ( j = 0 ; j < Pixels ; j++ ) 
	  {
	  ppos = i*Pixels + j ;
	  
	  if (data_type == CHN_8U) InPix = (float)(*get_pix_val)(image_8b,ppos);
	  if (data_type == CHN_16U) InPix = (float)(*get_pix_val)(image_16b,ppos);	  
//	  if (data_type == CHN_32U) InPix = (float)(*get_pix_val)(image_32b,ppos);
	  
	  
	  if(strncmp(normaliz,"RAW",3)==0) 
		{  
		if (data_type == CHN_8U) OutPix = 255. - InPix;
		if (data_type == CHN_16U) OutPix = 65535. - InPix; 
		if (InPix == 0) OutPix = 0;		// special case for zeros
///		if (data_type == CHN_32U) OutPix = 65535 - InPix;  		
		}

	  if(strncmp(normaliz,"NAVG",3)==0)
		{	
		if (data_type == CHN_8U) OutPix = 255. - (128. * InPix/ph->mean);
		if (data_type == CHN_16U) OutPix = 1024. - (512. * InPix/ph->mean);  // 16b images rarely more than 10b
		if (InPix == 0) OutPix = 0;		// special case for zeros
//		if (data_type == CHN_32U) OutPix = 1024. - (512. * InPix/ph->mean);
		}

	  if(strncmp(normaliz,"NMAX",3)==0)
		{
		OutPix = ph->last - InPix;
		if (InPix == 0) OutPix = 0;		// special case for zeros
		}
	 
	  if (data_type == CHN_8U) *( (PixVal *) image_8b_out + (i*Pixels) + j) =  (PixVal) OutPix;
	  
	  if (data_type == CHN_16U) *( (uint16 *) image_16b_out + (i*Pixels) + j) =  (uint16) OutPix;

//	  if (data_type == CHN_32U) *( (uint32 *) image_16b_out + (i*Pixels) + j) =  (uint32) OutPix;
 

// For testing  *** Output image is only 8 bit
//	   *( (PixVal *) image_8b_out + (i*Pixels) + j) =  (PixVal) OutPix;
	  
	  }
 
	printf("\n\t ** Grey level inversion done\n");
	
	
/* 
// Double checking -- Generating histogram for an  **8bit** output image

	hsize = 256 ;			// needed if 8b out with  16b in
	get_pix_val = get_u8;
	for (kk = 0; kk < hsize; kk++) histo[kk] = 0;
	gen_hist(image_8b_out, get_pix_val, Lines*Pixels, histo);
	
*/

// Generating histogram for output image	
	
	for (kk = 0; kk < hsize; kk++) histo[kk] = 0;
	
	if (data_type == CHN_8U)	gen_hist(image_8b_out, get_pix_val, Lines*Pixels, histo);	
		  
	if (data_type == CHN_16U)	gen_hist(image_16b_out, get_pix_val, Lines*Pixels, histo);	
	
	
	
// Analyse histogram just gathered 
	
	printf("\n\tFinding grey level range of output image (histogram analysis)\n\n");

	init_histo_struct(ph);

	ana_hist(histo, hsize, ph);		// new ana_hist, info passed via structure



// Write output image

 	printf("\n\n\t*Writing output image to disk as %s \n", fullfileout);

	if(data_type==CHN_8U) 
		poBand->RasterIO(GF_Write, 0, 0, Pixels, Lines, image_8b_out, Pixels, Lines, GDT_Byte, 0, 0 );
	if(data_type==CHN_16U) 
		poBand->RasterIO(GF_Write, 0, 0, Pixels, Lines, image_16b_out, Pixels, Lines, GDT_UInt16, 0, 0 );


// *** PRESENTLY *** Output image is always 8bit

//		poBand->RasterIO(GF_Write, 0, 0, Pixels, Lines, image_8b_out, Pixels, Lines, GDT_Byte, 0, 0 );
		
		
//**************************
	
	// Writing description (history) ...

	sprintf(ima_description,"Inverted greylevel (%s) of CHs %d of file %s\0", normaliz, ch_in, shortfilename);
	poBand->SetDescription(ima_description);
	printf( "\nDescription : %s\n", ima_description);

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



/***************************************************************/

/* This function converts a string to upper case characters */

void upper_case(char *c)
{
	while (*c != '\0') {
		if (*c >= 'a' && *c <= 'z')
			*c += ('A' - 'a');
		c++;
	}
}


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

