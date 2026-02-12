/*
C+             
C       imthres_g.cpp (C++ main prog that calls imthres2_g.f)
c
c
c	François A. Gougeon, Ph.D.
c	Remote Sensing Research	
c	Natural Resources Canada
c	Canadian Forest Service 
c	Pacific Forestry Centre
c	506 West Burnside Rd.
c	Victoria, British Columbia, 
c	Canada, V8Z 1M5	
c	
c
c	HISTORY
c
c
c
c	François Gougeon  v1.0a May 2022
c
c	#####    Purely to test FORTRAN program under GDAL (in prep for mfpwbrdf.f transition)
c	as we already have ima_thr_g.cpp to do that work well as a pure C++ version
c

c  NOTES from the PCI side (making FORTRAN prog. work after 10-12 years)

c
c	- Unable to make imthres works as a "simple" Fortran program calling C functions 
c	for PCI-related tasks (via IMTSTS.FTN) because unable to define a decent PCI C file pointer
c	and related structure in the Fortran main program. So decided to create a
c	C main prog. (imthres.c) that does very little, but has a FILE pointer and
c	runs IMPStatus() and a few cosmetic functions
c	calls the Fortran prog. (imthres2.f) for all main operations
c
c	- See parameter details, devlopment history, etc. in IMTHRES2.f
c
c
c	François Gougeon  v2.2	April 2022		 (PCI version)
c
c	-  Now CPP main prog (was a c prog)to call imthres2.f subroutine
c		and passes all IMPStatus parameters
c
c	François Gougeon  v2.2a	May 2022 			(GDAL version)
c
c	-  Now CPP main prog (imthres_g.cpp) that calls imthres2_g.f subroutine
c		in the ### GDAL environment ###
C-

	
#### PROGRAM USAGE

### Program will create a default output file name

> imthres_g Main_File.ext,CH# - thres1,thres2  	

### IF adding(OR) to an existing bitmap, or wanting a specific output file name

> imthres_g Main_File.ext,CH# Secteur_NonFor.tif thres1,thres2  

#### To compile

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

#define VERSION "v2.2a"
#define PROG_NAME "IMTHRES_G"

/*
#include <stddef.h>		// standart C inclusions
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <time.h>       // time_t, struct tm, time, localtime

#include "gdal_priv.h"		// For GDAL library
#include "ogrsf_frmts.h"	// For OGR
*/

#include "ITC-Suite_g.h"		// my newest GDAL variable setup
#include "itc_io_g.h"		// my newest GDAL image/bitmap input/output
#include "bitops.h"		// bit operations on bitmaps (mostly macros to be faster)


//#include "cpl_string.h"
// #include "tiffio.h"


//typedef unsigned char  PixVal;			// all in ITC-Suite_g.h and itc_io_g.h now
//typedef long           int64;
//typedef unsigned long  uint64;
//typedef unsigned short     uint16;
//int  CHN_8U = 1, CHN_16U = 3;						// PCI no.
//int  CHN_8U = 1, CHN_16U = 2, CHN_16 = 3;			// for reference, GDAL no.

// DEclare the FORTRAN program (now a SUBROUTINE) as a C prog.

//extern "C" int IMTHRES2_G(GDALDataset*, PixVal*, PixVal*, int*, int*);		// has to be in CAPITAL LETTERS  //

extern "C" int IMTHRES2_G(PixVal*, PixVal*, int*, int*);

// Fortran common block to pass variables (mostly pointers to objects
//	COMMON /CBNAME/ IMA_DS, PIBAND, IMA_OUT_DS	

extern "C" struct block{
GDALDataset* ima_in;
GDALRasterBand*	piBand;
GDALDataset* ima_out;
	} CBNAME;			// FORTRAN COMMON BLOCK

//extern "C" struct block CBNAME_ ;
	
	
	
// Global declarations - Tons are needed to use itc_io_g.cpp

int		Pixels, Lines, Channels;	// pixel and line starting at 1,1
int		xsize, ysize;			// pixel and line starting at 0,0
int 	data_type;

int64 	bmsize;		// size of bitmap in bytes 


int	by_lines=0, by_image=1;			// default is to read/write by image (faster),
int		bylines_flag = 0;		// by default proceed by full images (not by lines)

float		xpixsz, ypixsz;
char 		*Proj, *Proj2, *Datum, *Datum2, *Temp,*token;
double		adfGeoTransform[6], adfGeoTransform2[6];
double 		topleftX, transformX, topleftY, transformY; 	 /* for geographic mapping */

int 		ch_in, ch_out, in_ch[10], segm_in, in_segm[],thres[2];
int			thres0, thres1;


GDALRasterBand	*piBand,*piBand2,*poBand;
char **papszOptions = NULL;


int imaFile_opened;
GDALDataset	*ima_in, *ima_out;
GDALDataset	*seg_in;


//char 	description[80];
char 	Extension[10]; 		// global variable for other prog.

char	Description[80];		// for output image


int	xcg, ycg;	

PixVal *ima_buf_in, *ima_buf_out;
PixVal * maskbitbuf;


time_t rawtime;
struct tm * timeinfo;


//PIX_FUN_PTR	get_pix_val; 		 // pointer that allows us to deal with  many types of image (8,16u,16s) 

//************************************

int main(int argc, char* argv[])
{

PixVal *nfmask, *stcmask;			// two mask bitmaps
GDALDriver 	*piDriver, *poDriver;
GDALRasterBand	*piBand, *poBand;
//double		adfGeoTransform[6];
char 		**papszMetadata;

int  	ithres;
float 	sum;

char 	ans[80], fullfilename[80], *basefname, *file_out;
char 	*filename, *extension, temp[80];
char 	*tstring, *cptr, ach_in[5];

PixVal		*pafScanline, *image_8b, *image_8b_out;
uint16		*pafScanline16, *image_16b, *image_16b_out;


int 	ch_out=1, no_ch=1, input_ch[10];

int	i, j, ii, jj, ofs, count;
int64 	bitnum, bitnum2;

//Proper Geo projection of main image

Proj = (char *) CPLMalloc(500);			// to store geo projection info
Datum = (char *) CPLMalloc(500);		// to store Datum info
Temp = (char *) CPLMalloc(500);			


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
	  printf("Have an INPUT image as first argument on command line \n\n");
	  printf("USAGE: imthres_g Main_File.ext,CH# - thres1,thres2 \n");
	  printf("USAGE: imthres_g Main_File.ext,CH# bitmap thres1,thres2 \n\n");
	  exit(-1);
	  }  

	if (argc < 3)	  
	  {
	  printf("\n\t PROBLEM with input parameters \n\n");
	  printf("USAGE: imthres_g Main_File.ext,CH# - thres1,thres2 \n");
	  printf("USAGE: imthres_g Main_File.ext,CH# bitmap thres1,thres2 \n\n");
	  exit(-1);
	  }  

	if ( EQUALN(argv[2],"-",1 ))
	  {
	  printf("\nSecond argument is \"-\" which implies no input bitmap to \"add to\" \n");
	  printf("A new output bitmap will be created based on \"base file name\" \n\n"); 
	  }	 
	  
	if (argv[3] == NULL) 
	  {
	  printf("\n\n PROBLEM with third argument%s \n",argv[3]);
	  printf("\tHave a threshold (or range) as third argument on command line \n\n");
	  printf("USAGE: imthres_g Main_File.ext,CH# - thres1,thres2 \n");
	  printf("USAGE: imthres_g Main_File.ext,CH# bitmap thres1,thres2 \n\n");
	  exit(-1);
	  } 

//***************************	  
	  
// Check that only one channel is mentionned with the input image file

	strncpy(temp, argv[1], 80);

	cptr =strtok(temp, ","); 	 
	strcpy(fullfilename,cptr);		
	cptr =strtok(NULL, ",");
	
	ii = 0;
	while(cptr != NULL) 
	  {
 	  //printf("%s\n", p); 
	  input_ch[ii++]= strtol(cptr,NULL, 10);
	  cptr =strtok(NULL, ",");
	  }
	no_ch = ii;

	if(no_ch == 0) 
	  {
	  input_ch[0]=1; 
      printf("Input channel number to use is \"unspecified\" will use first channel(1)\n\n");
	  }
	
	//printf("\nNo. of channel to use %d  AND Input channel number to use: %d \n", no_ch, input_ch[0]);

	if ( no_ch > 1) 
	  {
	  fprintf(stdout,"\n\t##### ERROR - No. of channels to use to create a smoothed image must be one #####\n");
	  //printf("\n\tYou present parameters are %s %s %s %s\n\n", argv[1], argv[2], argv[3], argv[4]);
	  goto Exit;
	  }

	ch_in = input_ch[0];
	if(no_ch == 1) printf("As per user: Input image channel number to use: %d \n\n", ch_in);

// #### Check code relevent to output bitmap after code opening main file
	
// Check if two thresholds are used is needed (e.g., argv[3] = 15,66)

	//printf("\n\t Argument three : %s \n\n", argv[3]);

	strcpy(temp, argv[3]);
	cptr =strtok(temp, ",");

	ithres = 0;
	while(cptr != NULL) 
	  {
	  thres[ithres++]= strtol(cptr,NULL,10); 	  
	  cptr =strtok(NULL, ",");
	  }
	  
	if(ithres == 1) 
			printf("As per user, the image will be threshold below %d\n", thres[0]);
	  
	if(ithres == 2)
		printf("As per user, the image will be threshold from %d to %d\n", thres[0], thres[1]);

	if(ithres > 2) 
		{printf("\n\n ### ERROR -- Only two threshold entries are acceptable \n\n"); exit(-1);}

	thres0 = thres[0]; 	  	thres1 = thres[1]; 	
		  
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

	if( ima_in->GetProjectionRef() != NULL )
	{
	strncpy(Proj,ima_in->GetProjectionRef(),30);
	printf("\nProjection is '%s'\n", Proj);
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
	
	printf("Channel Description: %s \n", piBand->GetDescription() );

// Check raster type (8 or 16 bit) and set flag PCI data Type CHN_8U=1 CHN_16U=3

	if(EQUALN(GDALGetDataTypeName(piBand->GetRasterDataType()),"Byte",4)) data_type=CHN_8U;  	//PCI data types
	if(EQUALN(GDALGetDataTypeName(piBand->GetRasterDataType()),"Uint16",6)) data_type=CHN_16U;

	printf("Channel %d , RasterDataType =%d, Type = %s, PCI data_type = %d \n\n",
		ch_in, piBand->GetRasterDataType(), GDALGetDataTypeName(piBand->GetRasterDataType()), data_type );


//	  exit(-1);		// for degugging

//*************************************************

// Open (or create) an OUTPUT image file name	
//	For example:  Secteur_THR1.tif if thresholding was done on channel 1 
// More complex name if adding to existing bitmap

//*************************************************

if (EQUALN(argv[2],"-",1 ))			//if no input bitmap, use image file name as base for output bitmap plus channel no.
	{
	basefname = strtok(fullfilename,".");
	basefname = strtok(basefname,"_");				
	//printf("basefname  :  %s \n", basefname);		
	file_out = strncat(basefname,"_THR",4); 
	//printf("file_out :  %s \n", file_out);   
	//itoa(ch_in,ach_in,10); 	 
	sprintf(ach_in, "%d" , ch_in); 	
	strncat(file_out,ach_in,3);	  	
	strncat(file_out,".tif",4);					// output image is forced to be a tif	
	}	
else								// if  input bitmap, just add to that name
	{
	//basefname = strtok(fullfilename,".");		// fake to init basefname
	//strcpy(basefname,argv[2]);
	//basefname = strtok(basefname,".");
	file_out = strtok(fullfilename,".");		// fake to init basefname
	strcpy(file_out,argv[2]);	
	}

 	printf("\n\t*Output bitmap file will be named \"%s\" \n", file_out);

	//exit(-1);		// for degugging

 // Prepare initial bitmap in memory


	if ( EQUALN(argv[2],"-",1 ) )			// Make a fresh bitmap in memory
	  {
	  printf("\nReserving memory for new output bitmap\n");	
	  maskbitbuf = (PixVal *) malloc(bmsize);	// prep memory for output bitmap
	  check_mem(maskbitbuf);
	  }
	else			// Open INPUT bitmap to ADD TO (or new with given name)
	  {	
	  printf("\nOpening existing bitmap to add results to (to \"OR\" with) \n", file_out);
	  
	  maskbitbuf = read_bitmap(argv[2], 1);	
	  

	  if ( maskbitbuf == NULL ) 		// if no such file, will need to CREATE a new file (later)
	    {
	    printf("Specified file \"%s\" does not exist. Thus, can't \"OR\" results with it \n", file_out);
	    printf("Will create a new output file(bitmap) named \"%s\", as per user request\n", file_out);
		
		printf("\nReserving memory for new output bitmap\n");	
	    maskbitbuf = (PixVal *) malloc(bmsize);	// prep memory for output bitmap
	    check_mem(maskbitbuf);	 		
	    }
	
	  }
	   
	  
//	exit(-1);		// for degugging 
	 
	
// With ave_filter.cpp output file was specified by the user. 
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
	  printf("\n\n ### Output bitmap file %s already exist \n", file_out); 
	  printf("\n\t OK to overwrite (or add to) that file (Y/N)? \t");  fgets(ans,80,stdin);

	  if (ans[0] == 'y' || ans[0] == 'Y') goto MAIN;

	  if (ans[0] == 'n' || ans[0] == 'N')  {printf("\n\t** Will not overwrite anything ** \n"); goto Exit;}

	  }


	
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
		//image_8b_out = (PixVal *) CPLMalloc(sizeof(PixVal)*Pixels*(int64)Lines); check_mem(image_8b);
		}

	if(data_type==CHN_16U) 
		{
		image_16b = (uint16 *) CPLMalloc(sizeof(uint16)*Pixels*(int64)Lines); check_mem(image_16b);
		//image_16b_out = (uint16 *) CPLMalloc(sizeof(uint16)*Pixels*(int64)Lines); check_mem(image_16b);
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




	//exit(-1);		// for degugging


//****************************

/*
// Do the THRESHOLDING  of the image (without calling the FORTRAN program)

	printf("\n\tDoing thresholding on image. Range is %d to %d ...\n\n",thres[0],thres[1]);
	
	count = 0;				// for debugging
	
// For 8bit ans 16bit images

	for ( i = 1 ; i <= Lines; i++ )		// For compatibility with PCI(x,y), I kept image coordinates start at (1,1)
	for ( j = 1 ; j <= Pixels; j++ ) 	
	  {

	  bitnum = (i-1)*(int64)Pixels + j-1 ;	//GDAL images start at zero (similar to bitmaps), so bytenum=bitnum
	  //clearbit(maskbitbuf,bitnum);
	  
	  if(data_type==CHN_8U)
	    if ( (image_8b[bitnum] >= thres[0]) &&  (image_8b[bitnum] <= thres[1]) ) 
			{setbit(maskbitbuf,bitnum); count++;}

	  if(data_type==CHN_16U)
	    if ( (image_16b[bitnum] >= thres[0]) &&  (image_16b[bitnum] <= thres[1]) ) 
			{setbit(maskbitbuf,bitnum);	 count++; } 
	  
	  if( (i/1000)*1000 == i ) printf("%d lines done\r", i);

	  }
	  
	printf("%d lines done\n\n", Lines);
	
	printf("%d pixel set by thresholds\n", count);
	
	// For testing 

	count = 0;
	for (ii=0; ii<bmsize*8; ii++) if (testbit(maskbitbuf, ii)) count++;
	printf("Total Pixels set in output 1-bit bitmap = %d \n", count);



*/


//*********************************************************
	
// Do the THRESHOLDING of the image	via the FORTRAN program IMTHRES2_G.F			

// 	Call the FORTRAN program "per se", now a SUBROUTINE



fprintf(stdout,"\n\n\t**Main C prog. calling Fortran program here ...\n\n");

//	IMTHRES2(idb_fp);

//	IMTHRES2(idb_fp, argcnt, args);

//	IMTHRES2(idb_fp, file, &dbic, &dbib, &dbob, &thres0, &thres1);

//	IMTHRES2_G(ima_in, image_8b, image_8b_out, &thres0, &thres1);  // passing dataset pointer (ima_in) WORKS !!

//	COMMON /CBNAME/ IMA_DS, PIBAND, IMA_OUT_DS


// 	*** Print before calling the Fortran prog  

printf("\nParameters before calling of Fortran prog. :\n");
printf("file_inP, file_outP , thres0, thres1 : \n");
printf("%I64d  %I64d, %d , %d \n\n", image_8b, maskbitbuf, thres0, thres1);



	CBNAME.ima_in = ima_in;				// feed the common block (some handlers)
	CBNAME.piBand = piBand;	
	CBNAME.ima_out = ima_out;		
	
	IMTHRES2_G(image_8b, maskbitbuf, &thres0, &thres1);				// try passing via common block



	fprintf(stdout,"\n\t**Back from Fortran program here ...\n"); 



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
	
	snprintf(Description, 80, "%s CH%d thresholded using range %d,%d ",fullfilename,ch_in,thres[0],thres[1]);
	if (! EQUALN(argv[2],"-",1 ))	
	  snprintf(Description, 80, "%s CH%d thresholded using range %d,%d and added to %s", 
												fullfilename,ch_in,thres[0],thres[1],argv[2]);		
  
	printf("\nDescription: %s \n\n", Description );
	
	//printf("\n Calling write_bitmap()\n\n");
			
	write_bitmap(maskbitbuf, file_out);	
	

// Close input and output images

Exit:	printf("\nClosing both image files and exiting program. \n");

	GDALClose(ima_in);
	//GDALClose(file_out);		// already close by write_bitmap()

time (&rawtime);
timeinfo = localtime (&rawtime);
fprintf(stdout,"\n\n_______________________________\n");
fprintf(stdout,"\n %s (%s) finished at %s\n\n", PROG_NAME, VERSION,  asctime(timeinfo));

} 		// end of main function


//**************************************************************
//**************************************************************

