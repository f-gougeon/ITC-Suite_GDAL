/* 

Program name: 	bmcombo_g.cpp

Author: 		François A. Gougeon

Date:			Nov 2023

Description:

	Program to combine two bitmaps into a third one

	- This program allows for the combination of two GDAL-related bitmaps (.tif, NBITS=1)
	into a third one.
	
	- This is typically done to combine various previously created masks into a new one 
	(eg, a non-forest mask from water, flats areas, rock outcrops, man-made features ...))
	
	- One of the two input masks (bitmaps) can be the same as the output mask 
	  to allow for an iterative addition of items
	  
	- To inverse a bitmap (operand=NOT) just use the same input bitmap twice  
		
	> bmcombo_g PP833596_NonFor.tif PP833596_NonFor.tif PP833596_Forest.tif  NOT
	
	
	François A. Gougeon, Ph.D.
	Remote Sensing Research	
	(©)Natural Resources Canada
	Canadian Forest Service 
	Pacific Forestry Centre
	506 West Burnside Rd.
	Victoria, British Columbia, 
	Canada, V8Z 1M5	


*****************************************


v1.0a		Nov. 2023	François Gougeon

			- Created from thickbit_g.cpp who deals with input and output bitmaps
			
v1.1		Nov. 2023	François Gougeon

			- Minor mods and ajustements

Run as (for examples):

	> bmcombo_g Bitmap1.tif Bitmap2.tif Out_Bitmap.tif Operand  	//Operand can be OR/AND/XOR/SUB

	> bmcombo_g PP833596_NIR_THR1.tif PP833596_NIR_Homog_THR1.tif PP833596_NonFor.tif OR

	> bmcombo_g PP833596_NIR_THR1.tif PP833596_NIR_Homog_THR1.tif PP833596_NonFor.tif -  // OR is the default
		
	> bmcombo_g PP833596_NIR_THR1.tif PP833596_NIR_Homog_THR1.tif - AND 		//Output name to be created

*/

#define VERSION	"v1.1"
#define PROG_NAME "BMCOMBO_G"
#define FILENAME  250

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

GDALDataset	*ima_in, *ima_in2, *ima_out;
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
unsigned char  *inbitbuffer, *inbitbuffer2, *outbitbuffer;
int byte, bit, efactor, ofs;
int64 bitnum;
int 	ch_out=1, ch_in=1, no_ch=1, dbic[10], ch_no;

char operand[10];

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
char	fullfilename[FILENAME], file_in[FILENAME], file_in2[FILENAME];		// for input file
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

	  printf("\nUSAGE: bmcombo_g PP833596_NIR_THR1.tif PP833596_NIR_Homog_THR1.tif PP833596_NonFor.tif  AND \n");	  
	  printf("\nUSAGE: bmcombo_g PP1493386_GDAL.pix,12 PP833596_NonFor.tif  PP833596_NonFor.tif   OR  \n\n");  
	  exit(-1);
	  }  
	  	  
	  strcpy(file_in, argv[argcount]);		// if filename by itself (probably a tiff bitmap)	  
	  segm_in = ch_no = 1;	dbic[0]=dbic[1]=1;	 // default value, if nothing changes


// Get CLEAN file name and extension
	
	strcpy(temp, argv[argcount]);  
	cptr = strtok(temp, ", "); 				// go to comma (or space) and put a null there
	//printf(" Temp :  %s \n", temp);
	strcpy(file_in, temp);	
	//printf(" Filename :  %s \n", file_in);
	
	strcpy(temp, argv[argcount]); 		// reload temp
	//printf(" Temp :  %s \n", temp);		
	cptr = strtok(temp,"."); 			// goto DOT 
	//printf(" Temp :  %s \n", temp);	
	cptr = strtok(NULL,", ");			// goto comma or space
	strncpy(extens, cptr, 3);
	extens[3] = '\0';   					// null character manually added cause strncpy() does not do the job
	//printf(" Extension : %s\n", extens);
	
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
	//printf(" After comma : %d\n", ch_no);	

	segm_in = dbic[0] = ch_in = ch_no ;				// for PCI code coompatibility
	//printf(" Segment to use : %d \n", dbic[0]);

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
	  
	  //printf(" Segment to use : %d \n", ch_no);
	  
	  }	

//	exit(-1);				// for debugging

//***************************	

// Get second input bitmap file 

	argcount++;			// next argument
	
		if (argv[argcount]== NULL) 
	  {
	  printf("\n\t **PROBLEM** with second input bitmap %s \n",argv[argcount]);
	  printf("\t Have a second bitmap as second argument on command line or '#'\n\n");  

	  printf("\nUSAGE: bmcombo_g PP833596_NIR_THR1.tif PP833596_NIR_Homog_THR1.tif PP833596_NonFor.tif  AND \n");	  
	  printf("\nUSAGE: bmcombo_g PP1493386_GDAL.pix,12 PP833596_NonFor.tif  PP833596_NonFor.tif   OR  \n\n");  
	  exit(-1);
	  }  
	   
	
	strcpy(file_in2, argv[argcount]);		// if filename by itself (probably a tiff bitmap)
	
	//printf(" Second Filename :  %s \n", file_in2);

//	exit(-1);				// for debugging
	

// IF this  argument is ArcGIS or ArcMap, need to create an output file name

//	if ((strncmp("ArcGIS ",argv[argc-1],3) == 0) )  goto OUT_NAME; // create one, using base file name
	
	//upper_case(argv[argc-1]);
	//if ((strncmp(argv[argc-1],"ARCGIS ",3) == 0) )  goto OUT_NAME; // create one, using base file name	
	

// Check if output file name given for output bitmap (if not, default filename will be used)

	argcount++;			// next argument	
	
	if (argv[argcount]== NULL) 
	  {
	  printf("\n\t **PROBLEM** with OUTPUT bitmap %s \n",argv[argcount]);
	  printf("\t Have an OUTPUT bitmap as third argument on command line or '#'\n\n");  

	  printf("\nUSAGE: bmcombo_g PP833596_NIR_THR1.tif PP833596_NIR_Homog_THR1.tif PP833596_NonFor.tif  AND \n");	  
	  printf("\nUSAGE: bmcombo_g PP1493386_GDAL.pix,12 PP833596_NonFor.tif  PP833596_NonFor.tif   OR  \n\n");  
	  exit(-1);
	  }  
	

	if (EQUALN(argv[argcount],"-",1) || EQUALN(argv[argcount],"#",1) ) goto OUT_NAME; // create one, using base file name
	
	strcpy(file_out, argv[argcount]);
	//printf(" File_out  :  %s \n", file_out); 
	
	goto NEXT_PARAM;
	
//	exit(-1);				// for debugging

// If still here, then last parameter IS A filename 

//if (argc == 9) {strcpy(file_out,argv[8]); goto IN_IMA; }


//*******************

// Create base file name   
	
// Need to get "basefilename" when full path is involved
// Basefile name has path + head of file name (typically correspond to "named area" of study e.g. PRF)
// Area name is assumed separated from rest of file name by an underscore
// However, be careful as there could be underscores in the path

OUT_NAME:

	printf("\t **CREATING Output File Name \n");

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

	strncat(basefname,"_combo",6);
 	strncat(basefname,".tif",4);
	strcpy(file_out, basefname);
	
	printf("File_out  :  %s \n", file_out); 
	//printf("\n Output Filename Length:  %zd \n", strlen(file_out));


//*******************

NEXT_PARAM:


//	exit(-1);				// for debugging


// Check operation parameter (AND/OR/XOR/SUB)

// Check next input parameter

	argcount++;			// next argument
	//printf("\nArgument %d: %s \n", argcount, argv[argcount]);


// Check operand

	if (argv[argcount] == NULL) 
	  {
	  printf("\n\t ### ERROR ### A Fourth argument is needed \n");
	  printf("\nThe Fourth argument is the OPERATOR to use\n");
	  exit(1);
	  }

/* Enlargement factor to thicken bits (default) */

	if  ( EQUALN(argv[argcount],"-",1) || EQUALN(argv[argcount],"#",1) )
	{
	strcpy(operand,"OR\0");
	printf("\nWill use default operand of %s  \n", operand);
	goto IN_IMA;
	}

	strcpy(operand,argv[argcount]);

	//printf("\n\t ** Operand will be : %s \n\n", operand);

	if (EQUALN(operand,"NOT",3))	
		printf("\n\t ** With the inversion operand (NOT), second input file will be ignored **\n\n");
	

// *************************************************	

IN_IMA:

//	exit(-1);		// useful when testing only the input parameters



//	printf("\n\t\t*Opening files of the two input bitmaps to use : %s %s \n", file_in, file_in2);

	printf("\n\t\t*Opening files of the two input bitmaps to use ...\n");	
	
 	ima_in = open_imaFile(file_in);			// uses itc_io_g function to open image (bitmap) file (a GDALDataset pointer)

	xsize = Pixels; ysize = Lines;	
	
 	ima_in2 = open_imaFile(file_in2);


// Should check size compatibility  (2nd file Opening will output new Pixels and Lines info)

	if ( (Pixels != xsize) || (Lines != ysize) )
	{
	  printf("\n\t\t*** ERROR*** The two input bitmaps appear not compatible \n\n");
	  exit(-1);
	}




//	exit(-1);				// for debugging


fprintf(stdout,"\nReading input bitmaps and allocating memory for output bitmap\n");

	if(segm_in == 0) segm_in = 1;
	inbitbuffer = read_bitmap(file_in, segm_in);			// always 1 for tiff  file bitmap, different for PCI files 
	safety_zone(inbitbuffer);


 
	segm_in = 1;
	inbitbuffer2 = read_bitmap(file_in2, segm_in);			// always 1 for tiff  file bitmap 
	safety_zone(inbitbuffer2);


// Allocate memory for output bitmap buffer

	//printf("\n\nAllocating memory for output bitmap buffer\n\n");

	outbitbuffer = (PixVal *) calloc(bmsize,1);		// allocate (and zero) memory for an output bitmap 
	check_mem(outbitbuffer);

// allocate memory for bitmap buffers (PCI)

//fprintf(stdout,"\nReading and/or allocating memory for bitmaps.\n");
//inbitbuffer = alloc_read_bmp(idb_fp, xsize, ysize, dbib);
//outbitbuffer = alloc_read_bmp(idb_fp, xsize, ysize, 0);



//	exit(-1);				// for debugging
	
	

//*****************************************************************

//	 Do operation of bitmaps where Operand can be OR/AND/XOR/SUB/NOT

//*********************************************************************

printf("\n\n_______________________________\n");
printf("\n\t **Doing logical operation (%s) between input bitmaps ...\n\n",operand);


	ofs = 1 ;

  for ( i = 1+ofs ; i <= Lines-ofs ; i ++ )	/* image start at (1,1) */
  for ( j = 1+ofs ; j <= Pixels-ofs ; j ++ )    /* buffer needed not to go out of image */
  {
  bitnum = (i-1)* (int64)Pixels + j-1  ;

// The OR Operator


	if (EQUALN(operand,"OR",2))
	  { if ( testbit(inbitbuffer,bitnum) || testbit(inbitbuffer2,bitnum) )     setbit(outbitbuffer, bitnum); }
	  
//  The AND Operator  

  	if (EQUALN(operand,"AND",3))
	  { if ( testbit(inbitbuffer,bitnum) && testbit(inbitbuffer2,bitnum) )     setbit(outbitbuffer, bitnum); }
  
 
// The XOR Operator     if(!A != !B) 

	if (EQUALN(operand,"XOR",3))
//	  { if ( !testbit(inbitbuffer,bitnum) != !testbit(inbitbuffer2,bitnum) )     setbit(outbitbuffer, bitnum); } 
	  { if ( testbit(inbitbuffer,bitnum) != testbit(inbitbuffer2,bitnum) )     setbit(outbitbuffer, bitnum); } 
	  
 // The SUB Operator   

	if (EQUALN(operand,"SUB",3))
	  { if ( testbit(inbitbuffer,bitnum) && !testbit(inbitbuffer2,bitnum) )     setbit(outbitbuffer, bitnum);   } 
  
  // The NOT Operator   

	if (EQUALN(operand,"NOT",3))
	  { if  (! testbit(inbitbuffer,bitnum) )     setbit(outbitbuffer, bitnum);   } 
  
 
  

  if ( (j == Pixels) && (i == (i/100)*100 ) )  printf("%d lines done \r",i);

  }			// end of main loop
 

   printf("%d lines done \n", Lines);
   
   

//*********************************************************************
  
 // Writing output  bitmap 
 

	printf("\n\t **Writing output (combo) bitmap into: %s \n\n", file_out);

// Prep output image description

	if (EQUALN(operand,"NOT",3))
		sprintf(Description,"Inverse of bitmap from file %s ", file_in);	
	else
		sprintf(Description,"Combo(%s) of bitmaps from files %s and %s ", operand, file_in, file_in2);	

	
	write_bitmap(outbitbuffer, file_out);



// Close input and output images

Exit:	printf("\nClosing all image files and exiting program. \n");

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