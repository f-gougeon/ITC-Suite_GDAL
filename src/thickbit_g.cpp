/* 
Program name: 	thickbit_g.cpp

Author: 	François A. Gougeon

Description:

	Thicken bits in a bitmap (e.g., from treetops)

	This program thickens the bits of an input bitmap and saves the results
	in an output bitmap (typically, to make them more visible) 
	Can also produce a cross at the treetop location (Factor=100)
	
	This is typically done with a treetop (TT) bitmap for printing or 
	image recording purposes for the thicken treetops to be more visible. 

	It can also be used as a "dialation" program (i.e., morphological operator).
	Also, this can be used to make TTs more like ITCs so that programs only meant 
	to run on ITCs can be used with TTs (e.g., ITCPCD to get species composition
	of forest stands after TTs have been classified into species).

	François A. Gougeon, Ph.D.
	Remote Sensing Research	
	(©)Natural Resources Canada
	Canadian Forest Service 
	Pacific Forestry Centre
	506 West Burnside Rd.
	Victoria, British Columbia, 
	Canada, V8Z 1M5	


C*****************************************

C1	PCI PARAMETERS
C
C
C	THICKBIT is controlled by the following global parameters:
C
C	Name		Prompt					Count	Type
C
C	FILE		Database File Name			128	Char
C	DBIB		Input bitmap (e.g.: of tree tops)	1	Int	
C	DBOB		Output bitmap with thicken bits		1	Int
C	EFACTOR		Enlargement factor to thicken bits	1	Int
C	REPORT		Reporting device			128	Char
C
C
C2	FILE
C
C	Specifies the name of the PCIDSK file containing the input and output
C	channels (images) or bitmaps (themes).
C
C2	DBIB
C
C	Input bitmap (e.g.: of tree tops)	
C
C2	DBOB
C	
C	Output bitmap with thicken bits	
c
C2	EFACTOR
C	
C	Enlargement factor by which to thicken bits, typically an odd number.
c	A factor of 3 will take every treetop and enlarge it into a 3x3 block
c	of set pixels in the output bitmap around the location of that pixel.
c	
c	Default efactor=2, a 2x2 block is set for each originally set bit 
c				(hangs right and low)
c	Factor=100 produces a cross at the treetop location
c
c	Apart from special factors 2 and 100, THICKBIT will inforce odd numbers
c	so that location of tree is not offset.
c	
C
C2	REPORT
C
C	Progress reports can be sent to terminal (TERM) or to a file.
C
c
C1	HISTORY
c
c	By François A. Gougeon
c
c	 © Canadian Forest Service
c	Pacific Forestry Centre
c	Victoria, BC, Canada
c
c
c
c V1.0	Nov.96		François A. Gougeon
c
c			- From TT_STOCK.c V1.1
c
c V2.0	April 2000	François A. Gougeon
c
c			- Good clean up and modernization 
c			  (e.g., to use bitops and itc_io)
c			- To be used as a morphological "dialation" operator.
c			- EFACTOR as external variable
c
c V2.1	Sept 2002	François A. Gougeon
c
c			- Made "global" variables Lines, Pixels, Channels
c
c V2.2	Nov. 2002	François A. Gougeon
c
c			- Problem with special case EFACTOR=2 not producing anything
c
c
c V2.3	June 2008	 François Gougeon
c
c				- Link adaptations for PCI v10.n and its new incompatible PRM.PRM file and
c				  use of compiler (VisualStudiov8) wanting to produce more secure code.
c				  Most PCI routines are in PCI1000.dll, but not all.
c				  For example, IMPTime() and IMPReturn() are now in Core1000.dll
c				  and IMPCounter() in Counter1000.dll
c				  Since I dont have PCISDK or PCI/ProSDK, I had to create LIBs 
c				  from their DLLs to compile my progs (same for PCI 10.1)
c
c				 - Problems with the REPORT (*Report) Variable in Core1000.dll
c				   First use of report makes program CRASH (immediately after the start message)
c				   So declared the global variable	 FILE *Report;
c				   and did, immediately after the call to IMPSTATUS() :		
c				   if(EQUALN(report,"TERM",4)) 
c					{Report=stdout;}else{Report = fopen(report, "w");}
c v3.0	May 2016	François Gougeon
c
c			- Changed to a C++ program to deal with versions of PCI > v10.2
c			     int main (), ".cpp" name, and extern "C" around .h include files
c			- mod to some call to fit new library definitions (as per .def demangling)
c			  which also means mods to corresponding declaration .h files (e.g., gdb.h)
c			  (Since I dont have PCISDK or PCI/ProSDK, I have to create LIB and DEF from
c			  from their DLLs (via DUMPBIN and LIB) in order to compile my progs
c			- modules in files like gdb.h need to be declared 'extern "C++" to link
c			  with proper name mangling (MSVC++ mangling)
c			- ***WORKED**  for PCI 10.3 that migrate its LIB to c++, 
c
c			- HOWEVER, more stuff is needed for PCI 2015 (64bit)
c			- PCI LIB need to be /MACHINE:x64 
c			  and prog compile with 64b version (i.e., vcvarsall amd64)
c			  cause all the call to lib are now with 64bit pointers 
c			- IMPStatus()  uses "char const *"
c
c v3.1	Sept 2016	François Gougeon
c
c			- Now 128 character Filename, thus longer path
c			- Org. for bitmaps > 2GB via (int64) in bitnum and bmsize, and doing
c			  bitmap output via itc_io.cpp/write_bmp()
c
c v3.2	Nov. 2022	François Gougeon
c
c			- Buggy -- Missing ALLRegister()	
c
c v3.3	July 2023	François Gougeon
c
c			- Mod. to also run from a cmd line (Useful for ArcGIS integration)
c
c			For example:    >  thickbit PP1493386_Tests.pix 11 15 3
c
c
c v3.4	August  2023	François Gougeon	(thickbit_g.cpp)
c
c			- Migrated to the GDAL environment (cause very useful to see treetops
c			- Also useful to thicken mask, specially when done at low resolution & back
c			- Made cmd line input parameters very flexible
c

Run as (for examples):


### > thickbit_g test9.pix,4   output_file.tif	EFACTOR 		# channel to use connected to filename

### > thickbit_g test9.pix  4  output_file.tif	3 (OR 5 OR 100)		# channel to use as separate parameter for ArcGIS	

### > thickbit_g PP1493386_GDAL_TTs.tif PP1493386_GDAL_Thick.tif  3 (OR 5 OR 100)   # 100 makes a cross at TT

### > thickbit_g PP1493386_GDAL_TTs.tif # #				! default output bitmap to be created, default efactor=2
	
C-

*/

#define VERSION	"v3.4"
#define PROG_NAME "THICKBIT_G"
#define FILENAME  128

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <time.h>       // time_t, struct tm, time, localtime


#include "gdal_priv.h"		// For GDAL library
#include "ogrsf_frmts.h"	// For OGR

#include "ITC-Suite_g.h"	// define lots of vars for the Suite
#include "bitops.h"		// defines bitmap operations (mostly via macros)
#include "itc_io_g.h"		// my newest GDAL image/bitmap input/output

	
	
// Global declarations - Tons are needed to use with "itc_io_g.cpp"

int		Pixels, Lines, Channels;	// pixel and line starting at 1,1
int		xsize, ysize;			// pixel and line starting at 0,0
int 	data_type;

int64 	bmsize;		// size of bitmap in bytes 

int		bylines_flag = 0;		// by default proceed by full images (not by lines)

float		xpixsz, ypixsz;
char 		*Proj, *Proj2, *Datum, *Datum2, *Temp,*token;

double 		topleftX, transformX, topleftY, transformY; 	 /* for geographic mapping */

int 		ch_in, ch_out, in_ch[10], segm_in, in_segm[], segm_out;

GDALDataset	*ima_in, *ima_out;
GDALDriver 	*piDriver, *poDriver;
GDALRasterBand	*piBand, *poBand;
GDALDataset	 *seg_in, *seg_out;
double		adfGeoTransform[6], adfGeoTransform2[6];
char 		**papszMetadata;
char	 **papszOptions;
int imaFile_opened;

char 	Description[80];
char 	Extension[10]; 		// global variable for other prog.
int	xcg, ycg;	

	
/***************************** main prog. *******************************/


int main(int argc, char *argv[])
{

// Local vars list

 
int i, j, k, ii, jj, kk;
int xsize, ysize;
int blocks,segnum;
unsigned char  *inbitbuffer, *outbitbuffer;
int byte, bit, efactor, ofs;
int64 bitnum;
int 	ch_out=1, ch_in=1, no_ch=1, dbic[10], ch_no;

char seg_history[81], seg_desc[81], timedate[17];
int segtype;
char segflag, segname[9];
long int start, length;

int	 argcount;		
int commaFlag = 0;

time_t rawtime;
struct tm * timeinfo;

//char	*basefname;						// ** just a pointer **
char	basefname[130];
int		basef_len;	
char	fullfileout[130];
char  	*cptr, *cptr2;	// generic pointer to char


char 	Proj[250], ans[80], answer[10];
char	fullfilename[FILENAME], file_in[FILENAME];		// for input file
char 	*filename, *extension, temp[FILENAME],extens[5];			// for output file
char	ima_description[128];						// for output file
char 	*tstring, *p, ach_in[5];
char	file_out[FILENAME];


//***************************


/* Print Program Header and time */

	time (&rawtime);
	timeinfo = localtime (&rawtime);

	fprintf(stdout,"\n\t\tStarting %s (%s) at %s\n", PROG_NAME, VERSION, asctime(timeinfo));
	
// Registers for all types of files with GDAL

	GDALAllRegister(); 	
	
//***************************
 
// Check input parameters on command line (i.e., agrv[*])

//***************************

// Check *First argument* on command line (input image BITMAP)

	argcount = 1;

	if (argv[argcount]== NULL) 
	  {
	  printf("\n\t **PROBLEM** with input image %s \n",argv[argcount]);
	  printf("Have an INPUT bitmap as first argument on command line \n\n");  

	  printf("\nUSAGE: thickbit_g PP1493386_GDAL_TTs.tif PP1493386_GDAL_Thick.tif  3 \n");	  
	  printf("\nUSAGE: thickbit_g PP1493386_GDAL.pix,12 PP1493386_GDAL_Thick.tif  3  \n\n");  
	  exit(-1);
	  }  
	  
	  
	  strcpy(file_in, argv[argcount]);		// if filename by itself (probably a tiff bitmap)	  
	  segm_in = ch_no = 1;	dbic[0]=dbic[1]=1;	 // default value, if nothing changes


// Get CLEAN file name and extension

	
	strcpy(temp, argv[argcount]);  
	cptr = strtok(temp, ", "); 				// go to comma (or space) and put a null there
	//printf(" Temp :  %s \n", temp);
	strcpy(file_in, temp);	
	printf(" Filename :  %s \n", file_in);
	
	strcpy(temp, argv[argcount]); 		// reload temp
	//printf(" Temp :  %s \n", temp);		
	cptr = strtok(temp,"."); 			// goto DOT 
	//printf(" Temp :  %s \n", temp);	
	cptr = strtok(NULL,", ");			// goto comma or space
	strncpy(extens, cptr, 3);
	printf(" Extension : %s\n", extens);
	
//	exit(-1);
	
// check if segment number(s)is connected to file name by a comma

	segm_in = 1;						// default 
	strcpy(temp, argv[argcount]);  
	//printf(" Temp :  %s \n", temp);
	cptr = strtok(temp, ", "); 			// check for comma after extension
	cptr = strtok(NULL, ", ");			// got after the comma	
	//printf(" Temp :  %s \n", temp);
	//printf(" cptr :  %s \n", cptr);
	
	if(cptr != NULL)					// comma after extension
	{
	commaFlag = 1;	
	ch_no = strtol(cptr,NULL, 10);			// change to integer
	printf(" After comma : %d\n", ch_no);	

	segm_in = dbic[0] = ch_in = ch_no ;				// for PCI code coompatibility
	printf(" Segment to use : %d \n", dbic[0]);

// Some programs may have a second comma and a second item	

	cptr = strtok(NULL, ", ");			// check for another channel 
	if(cptr != NULL) {ch_no = strtol(cptr,NULL, 10); dbic[1]= ch_no; no_ch = 2;}					
	if(no_ch == 2) printf("Secondary bitmap to use : %d \n", dbic[1]);

	if ( no_ch > 1) 
	  {
	  fprintf(stdout,"\n\t##### ERROR - No. of input bitmap to use  must be one #####\n");
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
	  
	  printf(" Segment to use : %d \n", ch_no);
	  
	  }	

//	exit(-1);				// for debugging
	


// Check if output file name given for output bitmap (if not, default filename will be used)

	argcount++;			// next argument
	//printf("Argument %d: %s \n\n", argcount, argv[argcount]);

// IF this  argument is ArcGIS or ArcMap, need to create an output file name

//	if ((strncmp("ArcGIS ",argv[argc-1],3) == 0) )  goto OUT_NAME; // create one, using base file name
	
	//upper_case(argv[argc-1]);
	//if ((strncmp(argv[argc-1],"ARCGIS ",3) == 0) )  goto OUT_NAME; // create one, using base file name	
	
	if (argv[argcount]== NULL) 
	  {
	  printf("\n\t **PROBLEM** with second input bitmap %s \n",argv[argcount]);
	  printf("\t Have an OUTPUT bitmap as second argument on command line or '#'\n\n");  

	  printf("\nUSAGE: thickbit_g PP1493386_GDAL_TTs.tif PP1493386_GDAL_Thick.tif  3 \n");	  
	  printf("\nUSAGE: thickbit_g PP1493386_GDAL.pix,12 PP1493386_GDAL_Thick.tif  3  \n\n");  
	  exit(-1);
	  }  
	   
	
	if (EQUALN(argv[argcount],"-",1) || EQUALN(argv[argcount],"#",1) ) goto OUT_NAME; // create one, using base file name
	
	strcpy(file_out, argv[argcount]);
	printf(" File_out  :  %s \n", file_out); 
	
	goto NEXT_PARAM;
	
	//exit(-1);

// If still here, then last parameter IS A filename 

//if (argc == 9) {strcpy(file_out,argv[8]); goto IN_IMA; }


//*******************

// Create base file name   
	
// Need to get "basefilename" when full path is involved
// Basefile name has path + head of file name (typically correspond to "named area" of study e.g. PRF)
// Area name is assumed separated from rest of file name by an underscore
// However, be careful as there could be underscores in the path

OUT_NAME:

	printf("\t **CREATING File_out ... \n");

	strcpy(fullfilename,argv[1]);			// main input filename (and possibly its dir)
	
	cptr = strtok(fullfilename, ","); 	 		// get rid of comma and item (channel) after comma

	for (ii=0; ii < strlen(fullfilename); ii++)		// search for last underscore position
	  {
	  jj = strlen(fullfilename) - ii;				// start from the end
	  //printf("Count back: %d",j);
	  if(fullfilename[jj] == '_') {basef_len = jj;	break;}	// find last underscore in full file name
	  }
	//printf("\nBase Filename Length:  %d \n", basef_len);
  	
	strncpy(basefname, fullfilename,  basef_len);		// get that part of  the full file name
	basefname[basef_len] = '\0';   					// make it a string to be safe
	
	printf("Base file name ::  %s \n", basefname);	
	
//	Creeate default OUTPUT file name, concatenate TTs to base file name (path + file name beginning)	

	strncat(basefname,"_Thick",6);
 	strncat(basefname,".tif",4);
	strcpy(file_out, basefname);
	
	printf("File_out  :  %s \n", file_out); 
	//printf("\n Output Filename Length:  %zd \n", strlen(file_out));


NEXT_PARAM:

// Check next input parameter

	argcount++;			// next argument
	//printf("\nArgument %d: %s \n", argcount, argv[argcount]);


// Check EFACTOR

	if (argv[argcount] == NULL) 
	  {
	  printf("\n\t ### ERROR ### A third argument is needed \n");
	  printf("\nThe third argument is the EFACTOR to use\n");
	  exit(1);
	  }

/* Enlargement factor to thicken bits (default) */

	if  ( EQUALN(argv[argcount],"-",1) || EQUALN(argv[argcount],"#",1) )
	{
	efactor = 2;
	printf("\nUsing default EFACTOR of %d  \n", efactor);
	goto IN_IMA;
	}
	
	efactor = strtol(argv[argcount],NULL,10);
	
	if (efactor < 2) efactor = 2;
	if ( (efactor != 2) && (efactor != 100) )			/* if not special cases */
    {
    if ((efactor/2)*2 == efactor) efactor = efactor-1; 		/* to make it odd */
    }
	
	printf(" EFACTOR will be %d  \n\n", efactor);

	//exit(-1);



// *************************************************	

IN_IMA:

	//exit(-1);		// useful when testing only the input parameters

	
 	ima_in = open_imaFile(file_in);			// uses itc_io_g function to open image file (a GDALDataset pointer)
	

//	printf("\n\t\t*Reading main channel to use : %d \n", dbic[0]);	
	
//	Image =  (PixVal *) read_image(file_ima, dbic[0]);		// Read input image channel via itc_io_g.ccp, (pointer to image data)

	xsize = Pixels; ysize = Lines;

	printf("\nMain File Extension : %s \n", Extension); 



fprintf(stdout,"\nReading and/or allocating memory for bitmaps.\n");

BITMAP: 
	if(segm_in == 0) segm_in = 1;
	inbitbuffer = read_bitmap(file_in, segm_in);			// always one for tiff  file bitmap 
	safety_zone(inbitbuffer);

// allocate memory for output bitmap buffer

	printf("\n\nAllocating memory for output bitmap buffer\n\n");

	outbitbuffer = (PixVal *) calloc(bmsize,1);	// prep memory for an intermediate bitmap 
	check_mem(outbitbuffer);

// allocate memory for bitmap buffers (PCI)

//fprintf(stdout,"\nReading and/or allocating memory for bitmaps.\n");
//inbitbuffer = alloc_read_bmp(idb_fp, xsize, ysize, dbib);
//outbitbuffer = alloc_read_bmp(idb_fp, xsize, ysize, 0);



/* *****************************************************************

	 THICKEN BITMAP 

*********************************************************************/

if (efactor != 100)fprintf(stdout,"\nThickening bits by a factor of %d ...\n", efactor);
if (efactor == 100) fprintf(stdout,"\nWriting a cross for each input treetop ...\n");

ofs = efactor/ 2 ;					/* factor should be odd */
if ( (efactor == 2) || (efactor == 100) ) ofs = 1 ;

  for ( i = 1+ofs ; i <= Lines-ofs ; i ++ )	/* image start at (1,1) */
  for ( j = 1+ofs ; j <= Pixels-ofs ; j ++ )    /* buffer needed not to go out of image */
  {
  bitnum = (i-1)* (int64)Pixels + j-1  ;
  if ( testbit(inbitbuffer, bitnum) ) 
    {
    if (efactor == 2 )
      {
      setbit(outbitbuffer, bitnum);
      setbit(outbitbuffer, bitnum+1);
      setbit(outbitbuffer, bitnum+Pixels);
      setbit(outbitbuffer, bitnum+Pixels+1);
      }
    if ( (efactor > 2) && (efactor < 100 ) )
      {
      for ( ii = i-ofs ; ii <= i+ofs ; ii++ )
      for ( jj = j-ofs ; jj <= j+ofs ; jj++ ) 
        {
        setbit(outbitbuffer, (ii-1)*(int64)Pixels+jj-1);
        }
      }
    if (efactor == 100 )		 /* write a cross (good for tree top id) */
      {
      setbit(outbitbuffer, bitnum);
      setbit(outbitbuffer, bitnum+1);
      setbit(outbitbuffer, bitnum-1);
      setbit(outbitbuffer, bitnum+Pixels);
      setbit(outbitbuffer, bitnum-Pixels);
      }
  
    }

  if ( (j == Pixels) && (i == (i/100)*100 ) )
  fprintf(stdout,"%d lines done \r",i);

  }	/* end of main loop */
 

   fprintf(stdout,"%d lines done \n", Lines);

//*********************************************************************
  
 // Writing output (thicken) bitmap 
 

	printf("\n\t **Writing output (thicken) bitmap into: %s \n", file_out);

	sprintf(Description,"Thicken bit bitmap from %s, segment %d ", file_in, segm_in);

	write_bitmap(outbitbuffer, file_out);


// Close input and output images

Exit:	printf("\nClosing both image files and exiting program. \n");

//	GDALClose(file_in);
//	GDALClose(file_out);

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