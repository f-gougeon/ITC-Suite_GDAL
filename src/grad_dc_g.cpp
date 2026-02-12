/*
C+             
C       grad_dc_g.cpp (CPP main prog to call GRAD_DC2_g.f)


c	Program that produces an output image related to the amount
c	of gradient directionality found in small areas (blocks)
c	of the input image in a direction commensurate with SUNANG
c

c 	Variables usedw hen call via  PCI/EASI

c	FILE	Database File Name				1-64	Char
c	DBIC	Database Input Channel				1	Int
c	DBOC	Database Output Channel				1	Int
c	IBS		Size of blocks used (IBS*IBS)			1	Int
c	SUNANG	Angle of sun relative to the image		1	Int	
c	REPORT	Specifies the file to append generated report 	1-64	Char
c




c
c  v3.0	June 2008 François Gougeon
c
c	- Unable to make imthres works as a "simple" Fortran program calling C functions 
c	for PCI-related tasks (via IMTSTS.FTN) because unable to define a decent PCI C file pointer
c	and related structure in the Fortran main program. 
c
c	- So decided to create a C main prog. (GRAD_DC.c) that does very little, 
c	but has a FILE pointer and runs IMPStatus() and a few cosmetic functiona
c	and calls the Fortran prog. (GRAD_DC2.f) for all main operations
c
c	- See parameter details, development history, etc. in GRAD_DC2.f
c
c
c	François Gougeon  v4.0	April 2022 
c
c	- Ajustements as PCI libraries moved away from FORTRAN to C and later C++ only
c	  and, from 32bit to 64bit
c	- Now CPP main prog (GRAD_DC.cpp) calls GRAD_DC2.f as a extern C subroutine
c		and passes all IMPStatus parameters to it (by REFERENCE)
c	- My CFUNCT.FTN is used to setup for call to C functions in PCI lib
c	- "GRAD_DC2.f" also calls some PCI library functions that only exist in C++ 
c	  via C "wrapper" functions, like "IDBPixelSizeC" or "GDBWriteHistoryC"
c	  presently in main (GRAD_DC.cpp) (*** NOW *** in C_Wrappers.cpp)
c	- Equivalence between IPIXEL and LPIX to get unsigned byte (was IDUM,BDUM)
c	- BTW, this is mostly to test all these approaches:
c	  Procedures for a C++ program to call a F77 prog. that calls C & C++ functions
c	  so I can revive "mfpwbrdf" program under PCI 2007 and later create a GDAL version
c	  that will also require that kind of calls to C or C++ functions
c
c
c   François Gougeon	v4.1	July 2023	
c
c			- Mod. to also run from a cmd line (Useful for ArcGIS integration)
c
c	????		For example:    >  grad_dc PP1493386_GDAL_Tests.pix 1 5 7 31 160
c
c
c
c   François Gougeon	v4.2	Oct. 2023	
c
c			- Created a GDAL version (named grad_dc_g.cpp AND grad_dc2_g.f) )
c				and made OK for ArcGIS integration

c			- grad_dc_g.cpp calls grad_dc_g.f to do the work which calls CFUNCT_g.FTN
c				 to access library functions writen in C or C++
c
c			USAGE:    >  grad_dc_g input_ima.tif output_ima.tif IBS SUNANGLE
c
c			USAGE:    >  grad_dc_g input_ima.tif - 20 160
c
c			USAGE:    >  grad_dc_g input_ima.pix,1 output_ima.tif IBS SUNANGLE



   François Gougeon		v4.3 	Dec  2023	
 
			-Cleaning up many things
			
			- Pickup from ave_filter_g code  to deal with input file and 
			its attached (or not) channel number
			
			- Provenance description in the output file has a SHORT version 
			of the input file name (not the full path, which could be long
			when using ArcGIS)
			


c
C-
*/

#define VERSION	"v4.3a"
#define PROG_NAME "GRAD_DC_G"

#include <string.h>
#include <math.h>

#include "ITC-Suite_g.h"		// my newest GDAL variable setup
#include "itc_io_g.h"		// my newest GDAL image/bitmap input/output
#include "bitops.h"		// bit operations on bitmaps (mostly macros to be faster)
#include "error.h"

//FILE *Report;   /* To compensate for "faulty" Report variable from core1000.dll *

// extern void __stdcall IMTHRES2( int *);   // OK for C not for C++
//extern  void GRAD_DC2(FILE *);

//extern "C" int  GRAD_DC2(FILE *);		// FORTRAN prog. use C calling convention

//extern "C" int GRAD_DC2(idb_fp, argcnt, args);

//extern "C" int GRAD_DC2(FILE *, int *, void **);

/* Global vars list */

//int Lines, Pixels, Channels;

/* 
FILE *idb_fp;
void *args[7];
int argcnt[7]; 
*/

/*
extern int PRMCOM JPRCNT(7),DBIC(1),DBIB(1),DBOB(1),THRES0,THRES1
extern char PRMCMC FILENAME,REPORT
*/

// Setup for the FORTRAN program call for the GDAL version
// For exemple: GRAD_DC2_G(image_8b, image_8b_out, &ibs, &sunang);

extern "C" int GRAD_DC2_G(PixVal*, PixVal*, int*, int*);



// Fortran common block to pass variables (mostly pointers to objects
//	COMMON /CBNAME/ IMA_DS, PIBAND, IMA_OUT_DS	

// USING A COMMON BLOCK to pass some parameters to FORTRAN program
/* 
extern "C" struct block{
GDALDataset* ima_in;
GDALDataset* ima_out;
	} CBNAME;			// FORTRAN COMMON BLOCK
 */


// USING A COMMON BLOCK to pass some parameters to FORTRAN program

extern "C" struct block{
int		Pixels;
int		Lines;
int		data_type;
int		IOPR;			// GRAD_MAG=1 vs GRAD_DIR=2 vs GRAD_UNI=3
	} CBNAME;			// FORTRAN COMMON BLOCK





	
// Global declarations - Tons are needed to use itc_io_g.cpp

int		Pixels, Lines, Channels;	// pixel and line starting at 1,1
int		xsize, ysize;			// pixel and line starting at 0,0
int 	data_type, IOPR;

int64 	bmsize;		// size of bitmap in bytes 
int	bylines_flag = 0;		// by default proceed by full images (not by lines)
int	by_lines=0, by_image=1;			// default is to read/write by image (faster),


float		xpixsz, ypixsz;
char 		*Proj, *Proj2, *Datum, *Datum2, *Temp,*token;
double		adfGeoTransform[6], adfGeoTransform2[6];
double 		topleftX, transformX, topleftY, transformY; 	 /* for geographic mapping */

int 		ch_in, ch_out, in_ch[10], segm_in, in_segm[10],ibsize[2];

GDALRasterBand	*piBand,*piBand2,*poBand;
char **papszOptions = NULL;


int imaFile_opened;
GDALDataset	*ima_in, *ima_out;
GDALDataset	*seg_in;


char 	Extension[10]; 		// global variable for other prog.
char	Description[200];		// for output image


int	xcg, ycg;	

PixVal *ima_buf_in, *ima_buf_out;
PixVal * maskbitbuf;

time_t rawtime;
struct tm * timeinfo;


/***** main prog. ******/

int main(int argc, char *argv[])
{

/* local vars list */

char file[250];
int i, ii, jj;
int ibs, iws, sunang;
int no_ch,  dbic[10], ch_no, item;
char report[250];
char 	ans[250], fullfilename[250], *basefname, *file_out, file_in[250];
char 	*filename, *extension, temp[250], shortfilename[250];
int commaFlag = 0;

char	basefilname[250];
int		basef_len;	
int	 argcount;			// to help with a variable number of arguments

int xsize, ysize, channels;
int xoff=0, yoff=0;

double	topleftX,transformX,topleftY,transformY;

/* char chn_history[81], chn_desc[81]; */
char	timedate[17], pix_units[9], geosys[17];
int TTcount=0;
float pix_xsize, pix_ysize;

char 	*cptr, answer[10], ach_in[5];;

//PixVal *nfmask, *stcmask;			// two mask bitmaps

GDALDriver 	*piDriver, *poDriver;
GDALRasterBand	*piBand, *poBand;

PixVal		*image_8b, *image_8b_out;		// image pointers to stored image in memory
uint16		*image_16b, *image_16b_out;


//******************************************************************************
//******************************************************************************

/* Print Program Header and time */

	time (&rawtime);
	timeinfo = localtime (&rawtime);

	fprintf(stdout,"\n\t\tStarting %s (%s) at %s\n\n", PROG_NAME, VERSION, asctime(timeinfo));


// Check input parameters (i.e., agrv[*])

	argcount = 1;
	
	//printf("\n\tPresent parameters are %s %s %s %s\n\n", argv[1], argv[2], argv[3], argv[4]);

	if (argv[argcount] == NULL) 
	  {
	  printf("\n\t PROBLEM with input image %s \n",argv[argcount]);
	  printf("Have an INPUT image as first argument on command line \n\n");
	  printf("USAGE: grad_dc_g input_ima.tif output_ima.tif IBS SUNANGLE \n");
	  printf("USAGE: grad_dc_g input_ima.pix,1 output_ima.tif IBS SUNANGLE\n\n");
	  exit(-1);
	  }  

	//printf("\n\t First parameter --  Input image %s \n", argv[1]);
	
	

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

	ch_in = ch_no ;			// chn_in actually used for reading
	//printf(" Channel to use : %d \n", ch_in);

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
	  ch_in = ch_no ;			// chn_in is actually used for reading
	
	
//	exit(-1);				// for debugging
	

//*******************

// Create base file name   (for output file name)
	
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
	
	
	

//********************
	
	argcount++;			// next argument
		
	if (argv[argcount] == NULL) 
	  {
	  printf("\n\n ** PROBLEM ** with second argument%s \n\n", argv[argcount]);
	  printf("Have an output file as second argument on the command line \n\n");
	  printf("USAGE: grad_dc_g input_ima.tif output_ima.tif IBS  SUNANGLE \n");
	  printf("USAGE: grad_dc_g input_ima.pix,1 output_ima.tif IBS SUNANGLE\n\n");
	  printf("Second argument could be \"-\" and a file name will be created \n");	
	  printf("USAGE: grad_dc_g input_ima.pix,1 - IBS SUNNGLE\n\n");	  
	  exit(-1);
	  } 
// Check second input parameter
			
if ( EQUALN(argv[argcount],"-",1 )|| EQUALN(argv[argcount],"#",1 ))	
	  {
	  printf("\nSecond argument is \"-\" or \"#\" which implies no output file given \n");
	  printf("A new output file will be created based on \"base file name\" \n\n"); 
	  }	 
	  	  
//*************************************************

// Open (or create) an OUTPUT image file name	
//	For example:  Secteur_DC1.tif if work was done on channel 1 

//*************************************************

// If NO output file name, 
// use input image file name (and path) as base for output image plus odds
	
if ( EQUALN(argv[argcount],"-",1 )|| EQUALN(argv[argcount],"#",1 ))		
	{
	basefname = strtok(fullfilename,".");  // drop the file extension	

	for (ii=0; ii < strlen(fullfilename); ii++)		// search for LAST underscore position
	  {
	  jj = strlen(fullfilename) - ii;				// start from the end
	  //printf("Count back: %d",j);
	  if(fullfilename[jj] == '_') {basef_len = jj;	break;}	// find LAST underscore in full file name
	  }
	//printf("\nBase Filename Length:  %d \n", basef_len);
  	
	strncpy(basefname, fullfilename,  basef_len);		// get that part of  the full file name
	basefname[basef_len] = '\0';   					// make it a string to be safe
	
//	printf("Base file name ::  %s \n", basefname);	
		
	file_out = strncat(basefname,"_DC",3); 		// add DC for directionnality content
		//printf("file_out :  %s \n", file_out);   
	//itoa(ch_in,ach_in,10); 	 
	sprintf(ach_in, "%d" , ch_in);
	strncat(file_out,ach_in,3);	  	
	strncat(file_out,".tif",4);					// output image is forced to be a tif	
	}	
else								// if output file name given , just use to that name
	{
	//basefname = strtok(fullfilename,".");		// fake to init basefname
	//strcpy(basefname,argv[2]);
	//basefname = strtok(basefname,".");
	file_out = strtok(fullfilename,".");		// fake to init file_out
	strcpy(file_out,argv[argcount]);	
	}

// In any case, print output file name

 	printf("\n\t*Output image file will be named \"%s\" \n\n", file_out);

	//exit(-1);		// for degugging

	
// ******************

	
	argcount++;			// next argument -- typically third
	
	if (argv[argcount] == NULL) 
	  {
	  printf("\n\n ** PROBLEM ** with third argument%s \n",argv[argcount]);
	  printf("\tHave a block factor (eg: 20) as third argument on command line \n\n");
	  printf("USAGE: grad_dc_g input_ima.tif output_ima.tif IBS  SUNANGLE \n");
	  printf("USAGE: grad_dc_g input_ima.pix,1 output_ima.tif IBS SUNANGLE\n\n");
	  exit(-1);
	  } 
	
// Check that two window sizes are used is needed (e.g., argv[3] = 15,66)

	//printf("\n\t Argument three : %s \n\n", argv[3]);

	strcpy(temp, argv[argcount]);
	cptr = strtok(temp, ",");

	item = 0;
	while(cptr != NULL) 
	  {
	  ibsize[item++]= strtol(cptr,NULL,10); 	  
	  cptr = strtok(NULL, ",");
	  }
	  
	
	iws = 7; 	  	ibs = ibsize[0]; 	  
	  
	if(item == 1) 		// Normal situation
			printf("\nAs per user, the reporting block size (IBS) will be %d\n", ibsize[0]);
	  
	if(item == 2)
		{
		printf("\nN.B.: There is no need for two items (IWS is always 7) : %d,%d\n", ibsize[0], ibsize[1]);
		printf("As per user, the reporting block size (IBS) will be %d\n", ibsize[1]);
		ibs = ibsize[1]; 
		}
	
	if(item > 2) 
		{printf("\n ### ERROR -- Only two threshold entries are acceptable \n"); 
		printf("\n \t\t AND only one is preferred (e.g.: IBS=20)\n\n"); 
		exit(-1);}
		
		  
//	exit(-1);		// useful when testing only input parameters
	

// ******************
		
	argcount++;			// next argument -- typically fourth SUN ANGLE
	
	if (argv[argcount] == NULL) 
	  {
	  printf("\n\n ** PROBLEM ** with fourth argument%s \n",argv[argcount]);
	  printf("\tHave a sun angle (degrees relative to North) as fourth argument \n\n");
	  printf("USAGE: grad_dc_g input_ima.tif output_ima.tif IBS  SUNANGLE \n");
	  printf("USAGE: grad_dc_g input_ima.pix,1 output_ima.tif IBS SUNANGLE\n\n");
	  exit(-1);
	  } 	
	
	//printf("\nAs per user, the angle of the sun relative to North is: %s\n", argv[4]);
	
	sunang = strtol(argv[argcount],NULL,10);
	
	printf("\nAs per user, the angle of the sun relative to North is: %d\n", sunang);	

	
// ******************
		
	argcount++;			// next argument -- typically fifth,	BUT OPTIONAL (most user wont need)	
	
//	if (argv[argcount] == NULL) IOPR = 3;
	 
	if (  (argv[argcount] == NULL) || (strncmp("ArcGIS ",argv[argcount],3) == 0) )	IOPR = 3;
		else 	IOPR = strtol(argv[argcount],NULL,10);
		
		if(IOPR == 1) printf("\n\t*Creating a Gradient Magnitude Image (sun angle disregarded)\n");
		if(IOPR == 2) printf("\n\t*Creating a Gradient Direction Image (sun angle disregarded)\n");
		if(IOPR == 3) printf("\n\t*Assessing gradient directionality relative to %do sun angle\n", sunang);

	
	//exit(-1);		// useful when testing only input parameters

//*****************************************

// 	 Open INPUT image file

//************************************

	GDALAllRegister(); 		// Registers for all types of files with GDAL 

// Open INPUT image file

	//ima_in = (GDALDataset *) GDALOpen( file_in, GA_ReadOnly );

	ima_in = open_imaFile(file_in);		// using my function (sets up lots of things in global vars)

	if (ima_in == NULL) 
	  	{printf("\n\n PROBLEM opening input image file %s \n\n",file_in); exit(1);}
	//fprintf(stdout,"\n\t*File %s was opened for reading\n\n",file_in);
	

// Open input  channel

	piBand = ima_in->GetRasterBand(ch_in);
	
	printf("\nChannel Description: %s \n", piBand->GetDescription() );

// Check raster type (8 or 16 bit) and set flag PCI data Type CHN_8U=1 CHN_16U=3

	if(EQUALN(GDALGetDataTypeName(piBand->GetRasterDataType()),"Byte",4)) data_type=CHN_8U;  	//PCI data types
	if(EQUALN(GDALGetDataTypeName(piBand->GetRasterDataType()),"Uint16",6)) data_type=CHN_16U;

	printf("\nChannel %d , RasterDataType =%d, Type = %s, PCI data_type = %d \n\n",
		ch_in, piBand->GetRasterDataType(), GDALGetDataTypeName(piBand->GetRasterDataType()), data_type );


//	  exit(-1);		// for degugging


// Allocate memory for images depending if 8b or 16b image 

  	printf("\n\t\t*Allocating memory to the images\n");

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
	  printf("\nAllocated %Id bytes to the input image (same for output image)",Pixels*(int64)Lines);
	if(data_type==CHN_16U)
	  printf("\nAllocated %Id bytes to the input image (same for output) ",2*Pixels*(int64)Lines);

// Read the whole input image IN ONE SHOT 

 	printf("\n\n\t*Reading full INPUT image into memory ... \n\n");

	if(data_type==CHN_8U) 
		piBand->RasterIO(GF_Read, 0, 0, Pixels, Lines, image_8b, Pixels, Lines, GDT_Byte, 0, 0 );
	if(data_type==CHN_16U) 
		piBand->RasterIO(GF_Read, 0, 0, Pixels, Lines, image_16b, Pixels, Lines, GDT_UInt16, 0, 0 );


// For debugging
/* 
	printf("\nSome pixels from input image : \n");
	int mid_ima = Pixels/2 * Lines/2 ;
	for ( i = mid_ima ; i <= mid_ima+20; i++ )	printf(" %d", image_8b[i]);
	
 */
//*******************************************


printf("\n\n\t*Main CPP prog. calling FORTRAN program here ...\n\n"); 


//printf("\n\t\tPixels, Lines, data_type = %d %d %d \n\n", Pixels, Lines, data_type); 

	CBNAME.Pixels = Pixels;				// FEED stuff to the common block (CBNAME)
	CBNAME.Lines = Lines;	
	CBNAME.data_type = data_type;		
	CBNAME.IOPR = IOPR;
	
   GRAD_DC2_G(image_8b, image_8b_out, &ibs, &sunang);


printf("\n\t*Back from FORTRAN program here ...\n");


//***********************

// 	Writing output image

//***********************

//write_image(image_8b, file_out); 				// for TESTING : input to output

 	printf("\n\n\t*Writing output image to disk ... \n\n");

// Preparing image  description

	if(IOPR == 1)
	sprintf(Description,"Gradient Magnitude of %s, channel %d, with IBS=%d", shortfilename, ch_in, ibs);

	if(IOPR == 2)
	sprintf(Description,"Gradient Direction of %s, channel %d, with IBS=%d", shortfilename, ch_in, ibs);

	if(IOPR == 3)
	sprintf(Description,"Gradient directionnality of %s, channel %d, with IBS=%d, SUNANG=%d", shortfilename, ch_in, ibs, sunang);

// Writing output image to disk

	write_image(image_8b_out, file_out);

	printf("\nProvenance description written in output image : \n %s \n", Description);

//***********************

// Close input and output images

Exit:	

	printf("\nClosing both image files and exiting program. \n");

	//GDALClose(ima_in);
	//GDALClose(ima_out);
	//GDALClose(file_out);		// already close by write_bitmap()

/* Print Program Header and time */

	time (&rawtime);
	timeinfo = localtime (&rawtime);
	fprintf(stdout,"\n\n_______________________________\n");
	fprintf(stdout,"\n%s(%s) finished at %s\n\n", PROG_NAME, VERSION, asctime(timeinfo));


// For people using this program via ArcGIS, give then some time to examine the results (before disappearing)

if ((strncmp("ArcGIS ",argv[argc-1],3) == 0) )		// if last argument is ArcGIS or ArcMap
	{
	fprintf(stdout,"\n\n######\n");
	printf("\n Type anything to make this detailed window disappear and terminate %s ",PROG_NAME);
	answer[0] = getc(stdin); 		// gets any answer or <CR>
    }


}		/* END OF MAIN */



