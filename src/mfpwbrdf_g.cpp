/*
C+       
C       mfpwbrdfs_g.cpp -- C++ main prog that calls mfpwbrdf2_g.f (to do the work)
c      
c
c	François Gougeon  v4.0	June 2008 
c
c	- Versions 10 and higher of PCI do not support Fortran program anymore.
c	So I had to improvise.
c
c	- The main C prog. does very little, but declares a C FILE pointer and
c	runs IMPStatus() and a few cosmetic functions. It then calls
c	the Fortran prog. for all main operations, which in turn, calls PCI C functions
c	from the PCI library (via IMTSTS.FTN which translates calling parameters)
c
c	- See parameter details, development history, etc. in MFPWBRDF2.f
c
c
c
c	François Gougeon  v5.0	April 2022 
c
c	- Now CPP main prog (mfpwbrdf.cpp) calls "mfpwbrdf2.f" as a extern C subroutine
c		and passes all IMPStatus parameters to it (by REFERENCE)
c	- My CFUNCT.FTN is used to setup for call to C functions in PCI lib
c	- "mfpwbrdf2.f" also calls some PCI library functions that only exist as C++ 
c	  via C "wrapper functions", like "IDBPixelSizeC" or "GDBWriteHistoryC"
c	  presently in the main program (mfpwbrdf.cpp) ***NOW* in C_Wrappers.cpp
c	- Equivalence between IPIXEL and LPIX to get "unsigned byte" (in FOR program)
c	- Lots of antique junk left in, but commented out
c	- Leaned about "alignas()" to allign char variables to word boundaries 
c	(lots of instabilities before that)
c
c
c	François Gougeon  v5.0g	May-June 2022 		*** GDAL version ***
c
c	- Now CPP main prog (mfpwbrdf_g.cpp) calls "mfpwbrdf2_g.f" as a extern C subroutine
c		and passes all parameters to it (by REFERENCE)
c	- My CFUNCT.FTN is used to setup for call to C functions in GDAL lib
c	- Equivalence between IPIXEL and LPIX to get "unsigned byte" (in FOR program)
c	- Lots of antique junk left in, but commented out
c	- Leaned about "alignas()" to allign char variables to word boundaries 
c	(lots of instabilities before that)




Program usage: 		(at this point in time)

---- with a PCI file

 	mfpwbrdf_g imafile.pix,dbic,dbib,dboc  ... other param.

	>  mfpwbrdf_g secteur_v3.pix,2,12,7 y a 50

----  with TIFF files

	 mfpwbrdf_g imafile.tif,n bitmap.tif outfile.tif ... other param.

	> mfpwbrdf_g Secteur_IRGB.tif,1 Secteur_MG.tif BRDF_Out.tif yes auto 50

Here, other parameters are 

	NADCEN	Is nadir centered? (yes/no)	
	MODO	Mode of operation (AUTO/MANUAL)
	NVAL	Normalization Value (to inforce)


NOTES:

	-At this point in time, the program does not deal with :
		multiple channels in one single run (use multiple runs)
		multiple correction curves (softwoods & hardwoods)(use multiple runs)
		multiple flight lines within a mosaic
		
	Similar to original program for PCI, the program operates on North-South images
	while most flight lines are acquired East-West, so images need to be rotated

	The original program went North-South to access the image "line-by-line"
	to minimize memory usage and work on huge images, huge mosaic.

C-
*/

#define VERSION	"v5.0g"
#define PROG_NAME "MFPWBRDF_G"

#include "ITC-Suite_g.h"		// my newest GDAL variable setup
#include "itc_io_g.h"		// my newest GDAL image/bitmap input/output
#include "bitops.h"		// bit operations on bitmaps (mostly macros to be faster)

#define CHN_8U 		1

FILE *Report;   /* To compensate for "faulty" Report variable from core1000.dll */

// Declare FORTRAN program 
// REMEMBER everything is to be pass by reference to FORTRAN prog.

//extern "C" int (idb_fp,file,dbic,dbib,dboc,&nadcen,&modo,nval,&step,&pres,posts,argcnt);

//extern "C" int MFPWBRDF2_G(GDALDataset*, char*, int[8], int[8], int[8], char*, char*, int[64], char*, char*, int [8], int*)


//	MFPWBRDF2_G(ima_in,fullfilename,piBand,piBM,poBand,&argc,argv[]);

//extern "C" int MFPWBRDF2_G(GDALDataset*, char*, GDALRasterBand*, GDALRasterBand*, GDALRasterBand*, int*, char*[]);


//extern "C" int	MFPWBRDF2_G(GDALDataset*, char*, GDALRasterBand*, int*, int*, int*, char*[]);

//Using MFPWBRDF2_G(fullfilename, ima_in, piBand, piBM, poBand );

//extern "C" int	MFPWBRDF2_G(char*, GDALDataset*, GDALRasterBand*, GDALRasterBand*, GDALRasterBand* );

//Using MFPWBRDF2_G(fullfilename)

//extern "C" int	MFPWBRDF2_G(char*); 			// rest is via common block



// To do 	MFPWBRDF2_G(fullfilename,dbic,dbib,dboc,&nadcen,&modo,nval,ROT_Mode, &step,&pres,posts,&argc);
	
//extern "C" int MFPWBRDF2_G(PixVal*, char*, int[8], int[8], int[8], char*, char*, int[64], char*, char*, int [8], int*);  // rest is via common block

extern "C" int MFPWBRDF2_G(PixVal*, PixVal*, PixVal*, char*, int[8], int[8], int[8], char*, char*, int[64], int*,char*, char*, int [8], int*);  // rest is via common block


// Fortran common block to pass variables (mostly pointers to objects
//	COMMON /CBNAME/ IMA_DS, PIBAND, IMA_OUT_DS	

extern "C" struct block{
GDALDataset* ima_in;
GDALRasterBand*	piBand;
GDALRasterBand*	piBM;
GDALRasterBand*	poBand;
//PixVal*	ima_buf_in;
	} CBNAME;			// FORTRAN COMMON BLOCK	
	
//FILE *idb_fp;
//void *args[11];		//PCI version
//int argcnt[11];
	
	
// Global declarations - Tons are needed to use itc_io_g.cpp

int		Pixels, Lines, Channels;	// pixel and line starting at 1,1
int		xsize, ysize;			// pixel and line starting at 0,0
int 	data_type;				// PCI data type flag
GDALDataType GDataType;			// GDAL data type object

int64 	bmsize;		// size of bitmap in bytes 

int	by_lines=0, by_image=1;			// default is to read/write by image (faster),
int		bylines_flag = 0;		// by default proceed by full images (not by lines)


float		xpixsz, ypixsz;
char 		*Proj, *Proj2, *Datum, *Datum2, *Temp,*token;
double		adfGeoTransform[6], adfGeoTransform2[6];
double 		topleftX, transformX, topleftY, transformY; 	 /* for geographic mapping */

int 		ch_in=0, ch_out=0, in_ch[10], segm_in, in_segm[],thres[2];
int			thres0, thres1;

// GDAL Datasets

int imaFile_opened;
GDALDataset	*ima_in, *ima_out;
GDALDataset	*seg_in;

// GDAL Channels

GDALRasterBand	*piBand, *piBM, *poBand;
char **papszOptions = NULL;

//char 	description[80];
char 	Extension[10]; 		// global variable for other prog.
char	Description[80];		// for output image



int	xcg, ycg;	

PixVal *ima_buf_in, *ima_buf_out;			// Only 8bit images for the moment
PixVal * maskbitbuf;

time_t rawtime;
struct tm * timeinfo;

/*
extern int PRMCOM JPRCNT(7),DBIC(1),DBIB(1),DBOB(1),THRES0,THRES1
extern char PRMCMC FILENAME,REPORT
*/

/***** main prog. ******/

int main(int argc, char *argv[])
{

/* local vars list */

char file[64], report[64];
int dbic[8], dbib[8], dboc[8], nval[64], posts[8];

alignas(4) char 	ans[80], fullfilename[80], *basefname, file_out[80];
char 	*filename, *extension, temp[80];
char 	*tstring, *p, ach_in[5];
int 	 no_ch=1, input_ch[10];
char	nvaldef;					// flag to use nval default (which is zero), will be calculated

int	i, j, ii, jj, ofs, count;
int64 	bitnum, bitnum2;

GDALDriver 	*piDriver, *poDriver;
//GDALRasterBand	*piBand, *piBM, *poBand;				// global vars now

// ###### The "alignas(4)" is extremely important for this prog to work ####
// making sure these char*1 vars are aligned to 4 byte words for FORTRAN

alignas(4) char nadcen, modo, rotate, step, pres;

int	PCI_Mode=0, TIF_Mode=0, ROT_Mode=0;


//int xsize, ysize, channels, data_type;
int xoff=0, yoff=0;

//Proper Geo projection of main image

Proj = (char *) CPLMalloc(500);			// to store geo projection info
Datum = (char *) CPLMalloc(500);		// to store Datum info
Temp = (char *) CPLMalloc(500);		

double	topleftX,transformX,topleftY,transformY;

/* char chn_history[81], chn_desc[81]; */
char	timedate[17], pix_units[9], geosys[17];
int TTcount=0;
float pix_xsize, pix_ysize;

/* assign parameter pointers to argumnets */
/*
args[0] = (void *) file;
args[1] = (void *) dbic;
args[2] = (void *) dbib;
args[3] = (void *) dboc;
args[4] = (void *) &nadcen;
args[5] = (void *) &modo;
args[6] = (void *) nval;
args[7] = (void *) &step;
args[8] = (void *) &pres;
args[9] = (void *) posts;
args[10] = (void *) report;
*/
//	c	COMMON /PRMCOM/	JPRCNT(11),DBIC(8),DBIB(8),DBOC(8),NVAL(64),POSTS(8)
//	c	COMMON /PRMCMC/	FILE,NADCEN,MODO,STEP,PRES,REPORT

/*************************************************************
	SETUP PCI ENVIRONMENT AND INPUT/OUPUT
*****************************************************************/

// setup standard PCI interface 
/*
IMPStatus (	"FILE,DBIC,DBIB,DBOC,NADCEN,MODO,NVAL,STEP,PRES,POSTS,REPORT;",
			"C   ,I   ,I   ,I    ,C    ,C   ,I   ,C   ,C   ,I    ,C     ;",
			"64  ,8   ,8   ,8    ,1    ,1   ,64  ,1   ,1   ,8    ,64    ;",
			"1   ,1   ,1   ,0    ,1    ,1   ,0   ,1   ,1   ,0    ,0     ;",
			"MFPWBRDF.","FORCE", argcnt, args, argc, argv );
     
// To compensate for "faulty" Report variable from core1000.dll 
if(EQUALN(report,"TERM",4)) {Report=stdout;}else{Report = fopen(report, "w");}

*/

// Print Program Header and time 

time (&rawtime);
timeinfo = localtime (&rawtime);

printf("\n\t\tStarting %s (%s) at %s\n", PROG_NAME, VERSION, asctime(timeinfo));


//***************************

// Check input parameters (i.e., agrv[*])

//***************************

//printf("\n\tPresent parameters are %s %s %s %s\n\n", argv[1], argv[2], argv[3], argv[4]);

if (argv[1]== NULL) 
  {
  printf("\n\t PROBLEM with input image %s \n",argv[1]);
  printf("Have an INPUT image as first argument on command line \n\n");
  printf("USAGE: mfpwbrdf_g imafile.pix,dbic,dbib,dboc  ---- with a PCI file\n");
  printf("USAGE: mfpwbrdf_g imafile.tif,n bitmap.tif outfile.tif ----  with TIFF files\n\n");
  
  printf("USAGE: Additional \"optional\" parametres:  nadcen(Y/N), modo(A/M), nval(nnn), rotate(Y/N) \n");
  printf("USAGE: Some additional parametres can skipped using  \"-\"  \n\n");  
  exit(-1);
  }  

/*
if (argc < 4)	  
  {
  printf("\n\t PROBLEM with input parameters \n\n");
  printf("USAGE: MFPWBRDF_G(IMAfile,dbic,dbib,dboc) at minimum\n");
  printf("USAGE: MFPWBRDF_G(IMAfile,dbic,dbib,dboc,&nadcen,&modo,nval,&step,&pres,posts) \n\n");
  exit(-1);
  }  
*/

//***************************	  
	  
// Get file extension (PIX or TIF)
	  
	  
//printf("Main File to use:  %s \n", argv[1]);
	  
strncpy(fullfilename, argv[1],80);
filename = strtok(fullfilename,".");
extension = strtok(NULL,", "); 
//printf("Filename:  %s \n", filename);
//printf("Extension:  %s \n\n", extension);

if (strncmp(extension,"pix",3) == 0) PCI_Mode = 1;
if (strncmp(extension,"tif",3) == 0) TIF_Mode = 1;



// For PCI file, SPECIAL MODE, all info could be tagged to file (e.g., file.pix,1,7,6 )
// For TIF files, could mention one input channel, others will be discrete tif files  (could be file.tif,2)


// Check channels mentionned with the input image file name

strncpy(temp, argv[1], 80);
p = strtok(temp, ","); 	 
strcpy(fullfilename,p);		
p = strtok(NULL, ",");

ii = 0;
while(p != NULL) 			// get comma separated channel number attached to file name
  {
  //printf("%s\n", p); 
  input_ch[ii++]= strtol(p,NULL, 10);
  p = strtok(NULL, ",");
  }
no_ch = ii;

if(no_ch == 0) 
  {
  input_ch[0]=1; 
  printf("\t#### Input channel number to use is \"unspecified\" so will use first channel(1)\n\n");
  }

//printf("\nNo. of channel to use %d  AND Input channel number to use: %d \n", no_ch, input_ch[0]);

if ( no_ch > 3) 		// Only for PCI files, tif files are more separate
  {
  fprintf(stdout,"\n\t##### ERROR - No. of channels to use must be three for NOW #####\n");
  //printf("\n\tYou present parameters are %s %s %s %s\n\n", argv[1], argv[2], argv[3], argv[4]);
  goto Exit;
  }

//if(no_ch >= 1) printf("As per user: Input image channel number to use: %d \n", input_ch[0]);


if (PCI_Mode)
	{	
	//if(no_ch >= 2) printf("As per user: Input image bitmap number to use: %d \n", input_ch[1]);
	//if(no_ch >= 3) printf("As per user: Output image channel number to use: %d \n", input_ch[2]);

	// Nomenclature needed by FORTRAN program (LATER, many channels can be done at the same time)

	ch_in = dbic[0] = input_ch[0];
	dbib[0]=input_ch[1]; 
	dboc[0]=input_ch[2];
	
	}


if (TIF_Mode)
	{

	if(no_ch > 1)
	  {
	  printf("\n\t***TIF file mode*** Only one input channel allowed \n");
	  printf("\tOther comma-separated entries after filename not allowed \n");
	  printf("\tBitmap and output files need to be discrete TIF files\n\n");	 
	  exit(-1);	  
	  }
	 	
	ch_in = dbic[0] = input_ch[0];
	dbib[0] = 0; 
	dboc[0] = 0;
	
	//strncpy(temp, argv[2], 80);
	//printf("As per user: Input bitmap file to use: %s \n", argv[2]);
	//printf("As per user: Output image filename to use: %s \n", argv[3]);
 
//	exit(-1);		// useful when testing
	}

// Check other input parameters

//if ( (PCI_Mode) && (argc == 1) || (TIF_Mode) && (argc == 3) )				// no other param, use all default parameters
	{nadcen='Y'; modo='A'; rotate='N'; step='Y';  pres='N'; posts[0]=0;}

//printf("argc = %d \n",argc);

if (PCI_Mode) if(argc > 2) strncpy(&nadcen, argv[2], 1);
if (TIF_Mode) if(argc > 4) strncpy(&nadcen, argv[4], 1);
if (nadcen == '-') nadcen = 'Y';
if ( (nadcen != 'Y') && (nadcen != 'y') )
	printf("\t#### User asked for an internal shift of the image ###\n\n");

if (PCI_Mode) if(argc > 3) strncpy(&modo, argv[3], 1);
if (TIF_Mode) if(argc > 5) strncpy(&modo, argv[5], 1);
if (modo == '-') modo = 'A';
if ( (modo != 'A') && (modo != 'a') )
   {printf("At the moment MODO must be A -- Exiting \n\n"); exit(-1);}

nval[0] = 0;
if ((PCI_Mode) && (argc > 4))
	{
	strncpy(&nvaldef, argv[4], 1);
	if (nvaldef == '-') nval[0] = 0;
	else nval[0] = atoi(argv[4]);
	}

if ((TIF_Mode) && (argc > 6))
	{
	strncpy(&nvaldef, argv[6], 1);
	if (nvaldef == '-') nval[0] = 0;
	else nval[0] = atoi(argv[6]);
	}


if (PCI_Mode) if(argc > 5) strncpy(&rotate, argv[5], 1);
if (TIF_Mode) if(argc > 7) strncpy(&rotate, argv[7], 1);
if (rotate == '-') {rotate = 'N'; ROT_Mode=0;}
if ( (rotate == 'Y') || (rotate == 'y') || (rotate == 'R') || (rotate == 'r'))
	{printf("\t#### User asked for an internal rotation of the image ###\n\n"); ROT_Mode=1;}


if ( (PCI_Mode && argc > 6) || (TIF_Mode &&  argc > 8) ) 
	{
	printf("\t#### Too many arguments on command line -- **EXITING** \n\n"); 
	printf("USAGE: mfpwbrdf_g imafile.pix,dbic,dbib,dboc  ---- with a PCI file\n");
	printf("USAGE: mfpwbrdf_g imafile.tif,n bitmap.tif outfile.tif ----  with TIFF files\n\n"); 
	printf("USAGE: Additional \"optional\" parametres:  nadcen(Y/N), modo(A/M), nval(nnn), rotate(Y/N) \n");
	printf("USAGE: Some additional parametres can skipped using  \"-\"  \n\n");  
	exit(-1);
	}


// Print Input info to double check (defined here for QUICK TESTING)

printf("\nInformation from cmd line and/or default variables(for now)\n");

printf("filename,dbic,dbib,dboc,nadcen,modo,nval,rotate,step,pres,posts\n");
printf("%s, %d, %d, %d, %c, %c, %d, %c, %c, %c, %d \n\n",
			fullfilename,dbic[0],dbib[0],dboc[0],nadcen,modo,nval[0],rotate,step,pres,posts[0]);

//exit(-1);		// useful when testing



// Register all types of image formats

//printf("\nUsing GDALAllRegister()\n");
GDALAllRegister(); 

// Open INPUT image FILE ("ima_in" is the dataset(file) pointer) 

printf("\n\t*Opening input file\n");

ima_in = open_imaFile(fullfilename);	

//exit(-1);		// for degugging


// Open input channel and check what type of channel

printf("\n\t*Opening input channel\n\n");

piBand = ima_in->GetRasterBand(ch_in);

printf("Channel Description: %s \n", piBand->GetDescription() );

// Check raster type (8 or 16 bit) and set flag PCI data Type CHN_8U=1 CHN_16U=3

GDataType = piBand->GetRasterDataType();

if(EQUALN(GDALGetDataTypeName(piBand->GetRasterDataType()),"Byte",4)) data_type=CHN_8U;  	//PCI data types
if(EQUALN(GDALGetDataTypeName(piBand->GetRasterDataType()),"Uint16",6)) data_type=CHN_16U;

/* printf("Channel %d , RasterDataType = %d, DataTypeName = %s, PCI data_type = %d \n\n",
//	ch_in, piBand->GetRasterDataType(), GDALGetDataTypeName(piBand->GetRasterDataType()), data_type );		// works
//	ch_in, piBand->GetRasterDataType(), GDALGetDataTypeName(1), data_type );							// NOGO as int
	ch_in, piBand->GetRasterDataType(), GDALGetDataTypeName((GDALDataType) 1), data_type );				// OK if casted
 */

// Read image channel

//printf("\n\t READING input channel\n\n");

ima_buf_in = (PixVal *) read_image(fullfilename,ch_in);			// allocates memory and reads in image

//if(data_type==CHN_8U)  ima_buf_in = (PixVal *) read_image(fullfilename,ch_in);

//if(data_type==CHN_16U) 	 { printf("\nNot supported yet\n"); exit(-1); }	

//if(data_type==CHN_16) 	 { printf("\nNot supported yet\n"); exit(-1); }

//exit(-1);		// for degugging
/* 

// Various Tests 

printf("RECHECKING --Geographic Data within that file\n");

if( ima_in->GetGeoTransform( adfGeoTransform ) == CE_None )
  {
  printf("Image file Geographic Info :\n");
  printf( "Origin = (%.6f,%.6f)\n", adfGeoTransform[0], adfGeoTransform[3] );
  printf( "Pixel Size = (%.6f,%.6f)\n\n", adfGeoTransform[1], adfGeoTransform[5] );
  }
 */

// Various Tests 
// The following is not supposed to work as channels dont have this geo info
/*
printf(" Geographic Data within that channel\n");

if( piBand->GetGeoTransform( adfGeoTransform ) == CE_None )
  {
  printf("Channel Geographic Info :\n");
  printf( "Origin = (%.6f,%.6f)\n", adfGeoTransform[0], adfGeoTransform[3] );
  printf( "Pixel Size = (%.6f,%.6f)\n\n", adfGeoTransform[1], adfGeoTransform[5] );
  }
*/


//********************************

// OPEN input BITMAP file (if needed) and read bitmap


/*
printf("\n\tNOT Reading input bitmap YET, but Opening it \n\n");

if (strncmp(Extension,"pix",3) == 0) piBM = ima_in->GetRasterBand(dbib[0]);
*/

printf("\n\t*Reading mask bitmap\n\n");
	 
if (PCI_Mode)
	{
	printf("\nFrom PCI file, opening bitmap %d (GDAL#) under which stats will be gathered to do BRDF \n", dbib[0]);
	maskbitbuf = read_bitmap(fullfilename, dbib[0]);	
	}
	
if (TIF_Mode)
	{
	printf("\nOpening bitmap tif file \"%s\" under which stats will be gathered to do BRDF \n", argv[2]);
	seg_in = open_imaFile(argv[2]);	  
	maskbitbuf = read_bitmap(argv[2], 1);
	} 
	
//exit(-1);		// for degugging




//********************************


printf("\n\t *Opening output channel \n\n");

if (PCI_Mode) poBand = ima_in->GetRasterBand(dboc[0]);

if (TIF_Mode) 
  { 
  strncpy(file_out, argv[3], 80);
  printf("\nBRDF-corrected output file will be : %s \n", file_out);
  //ima_out = open_imaFile(argv[3]);	 
  }

ima_buf_out = (PixVal *) malloc(sizeof(PixVal)*Pixels*Lines);

//exit(-1);		// for degugging


/* 
// 		FOR TESTING -- Write  Output  channel (just copy input)  **** WORKS WELL 

printf("\n\tFOR TEST Write output image (as input copy)  \n\n");

printf("\nDescription FOR TEST  : %s \n", Description);
	
if (PCI_Mode) write_image(ima_buf_in, fullfilename);   // if PCI, should ask which channel to overwrite
 
if (TIF_Mode) write_image(ima_buf_in, file_out); 
 
exit(-1);		// for degugging
 */


/* 

printf( "\n\tParameters before calling FORTRAN program:\n");

printf( "Fullfilename, ima_in, piBand, piBM, poBand ,ima_buf_in : \n");
//printf("%s, %p, %p, %p, %p, %p \n", fullfilename, ima_in, piBand, piBM, poBand, ima_buf_in);
printf("%s, %lld, %lld, %lld, %lld, %lld, %lld \n",fullfilename, ima_in, piBand, piBM, poBand, ima_buf_in,ima_buf_out);

printf("dbic, dbib, dboc, nadcen, modo, nval, step, pres, posts\n");
printf(" %d, %d, %d, %c, %c, %d, %c, %c, %d \n\n",dbic[0],dbib[0],dboc[0],nadcen,modo,nval[0],step,pres,posts[0]);

 */	
//*******************************

// 			Call FORTRAN main program


printf("\n\t*** Main CPP prog. calling Fortran program here ...\n\n"); 
	
	
	CBNAME.ima_in = ima_in;				// Feed the common block (some handlers)
	CBNAME.piBand = piBand;	
	CBNAME.piBM = piBM;	
	CBNAME.poBand = poBand;	
	//CBNAME.ima_buf_in = ima_buf_in;		

//	MFPWBRDF2_G(fullfilename, ima_in, dbic,dbib,dboc,&nadcen,&modo,nval,&step,&pres,posts,&argc);

//	MFPWBRDF2_G(fullfilename, ima_in, piBand, piBM, poBand ,&argc, argv);

//	MFPWBRDF2_G(fullfilename, ima_in, piBand, dbib, dboc ,&argc, argv);

//	MFPWBRDF2_G(fullfilename, ima_in, piBand, piBM, poBand ,&nadcen,&modo,nval,&step,&pres,posts,&argc);

//	MFPWBRDF2_G(fullfilename, ima_in, piBand, piBM, poBand); 

//	MFPWBRDF2_G(fullfilename);

//	MFPWBRDF2_G(ima_buf_in, fullfilename,dbic,dbib,dboc,&nadcen,&modo,nval,&step,&pres,posts,&argc);	// other stuff via common block


	
	MFPWBRDF2_G(ima_buf_in, maskbitbuf, ima_buf_out, fullfilename,dbic,dbib,dboc,&nadcen,&modo,nval,&ROT_Mode,&step,&pres,posts,&argc);	
	
	
printf("\n\t*** Back from Fortran program here ...\n"); 
	

//*******************************

	time (&rawtime);
	timeinfo = localtime (&rawtime);

	printf("\nWriting BRDF-Corrected output image - %s  \n\n", asctime(timeinfo));

	//snprintf(Description, 80, "BRDF-Corrected Image for testing");
	//strncpy(fullfilename, "testing.tif",20);			// for testing ### WORKS

	snprintf(Description, 80, "BRDF-Corr. Image of CH%d(%d) - %s", dbic[0], dbib[0], asctime(timeinfo));	
	printf("Description:  %s ", Description);
	
	if (PCI_Mode) printf("From file %s into channel %d \n\n", fullfilename, dboc[0]);
	if (TIF_Mode) printf("From file %s into file %s \n\n", fullfilename, file_out);	
	
//	if (strncmp(Extension,"tif",3) == 0) write_image(ima_buf_out, fullfilename);   // if not specified, should used channel 1 by default ### NOGO
	
	ch_out = dboc[0];		// NOTE: could be zero, which default to 1 for tif
//	ch_out = 6;

	if (PCI_Mode) write_image(ima_buf_out, fullfilename);  		 // if PCI and not specified, should ask which channel to overwrite

	if (TIF_Mode) write_image(ima_buf_out, file_out);  

	
/* 
	printf("\nFOR TEST -- input to output : CH%d  to CH%d \n\n", dbic[0], dboc[0]);	
	snprintf(Description, 80, "TEST  input to output : CH%d  to CH%d ", dbic[0], dboc[0]);
	write_image(ima_buf_in, fullfilename);   		// for test input to output --- WORKS WELL
 */





//*******************************

// Finishing message, close PCI file and return to EASI 

// Close input and output images

Exit:	printf("\nClosing image file and exiting program. \n");

//	GDALClose(ima_in);
	//GDALClose(file_out);		// already close by write_bitmap()

	time (&rawtime);
	timeinfo = localtime (&rawtime);

	fprintf(stdout,"\n\t\t %s(%s) finished at %s\n", PROG_NAME, VERSION, asctime(timeinfo));


}		/* END OF MAIN */

//*************************************************************************
//*************************************************************************

//			***NOW*** in C_Wrappers.cpp

/*

// C Function that calls C++ IDBPixelSize function (for FORTRAN to use)
// As in: call IDBPixelSize(idb_fp, IFUNC, pix_xsize, pix_ysize, pix_units)

extern "C" void IDBPixelSizeC(FILE* idb_fp, int IFUNC, float* pix_xsize, float* pix_ysize, char* pix_units)
{
	fprintf(stdout,"\n\t Got into IDBPixelSizeC\n");

	IDBPixelSize(idb_fp, IFUNC, pix_xsize, pix_ysize, pix_units);	//calls lib(dll) module
}

//*********

// C Function that calls C++ GDBWriteHistory function (for FORTRAN to use)

extern "C" void GDBWriteHistoryC()
{
	fprintf(stdout,"\n\t Got into GDBWriteHistoryC\n");

	//GDBWriteHistory();
}

*/

//******************