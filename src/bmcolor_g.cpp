/* 

Program name: 	bmcolor_g.cpp

Author: 		François A. Gougeon

Date:			Nov 2025

Version 1.1  	Aug 2026   Debugging (and for Linux)

Description:

	Program to colorize a bitmap

	- This program allows changing the default colour of GDAL-related bitmaps (.tif, NBITS=1)
	for display programs that are unable to do so OR to help within Automation scripts

	For example, we may like a water mask  to be blue, ITCs to be green, etc.

		
	> bmcolor_g PRF_Water.tif 0,0,100   ! change default color of bitmap to blue
	
	
	François A. Gougeon, Ph.D.
	Remote Sensing Research	
	(©)Natural Resources Canada
	Canadian Forest Service 
	Pacific Forestry Centre
	506 West Burnside Rd.
	Victoria, British Columbia, 
	Canada, V8Z 1M5	


*****************************************
*/

#define VERSION	"v1.1a"
#define PROG_NAME "BMCOLOR_G"
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

int	bylines_flag = 0;		// by default proceed by full images (not by lines)
int	by_lines=0, by_image=1;			// default is to read/write by image (faster),

float		xpixsz, ypixsz;
char 		*Proj, *Proj2, *Datum, *Datum2, *Temp,*token;

double 		topleftX, transformX, topleftY, transformY; 	 /* for geographic mapping */

int 		ch_in, ch_out, in_ch[10], segm_in, in_segm[10], segm_out;

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
int 	input_color[4];

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
	  printf("\nUSAGE: bmcolor_g PRF_Water.tif 0,0,100  \n\n");  
	  exit(-1);
	  }  
	  	  
	  strcpy(file_in, argv[argcount]);		// if filename by itself (probably a tiff bitmap)	  
	  segm_in = ch_no = 1;	dbic[0]=dbic[1]=1;	 // default value, if nothing changes


// Get CLEAN file name and extension
	
	strcpy(temp, argv[argcount]);  
	cptr = strtok(temp, ", "); 				// go to comma (or space) and put a null there
//	printf(" Temp :  %s \n", temp);
	strcpy(file_in, temp);	
	printf(" Filename :  %s \n\n", file_in);
	
	strcpy(temp, argv[argcount]); 		// reload temp
	//printf(" Temp :  %s \n", temp);		
	cptr = strtok(temp,"."); 			// goto DOT 
	//printf(" Temp :  %s \n", temp);	
	cptr = strtok(NULL,", ");			// goto comma or space
	strncpy(extens, cptr, 3);
	extens[3] = '\0';   					// null character manually added cause strncpy() does not do the job
//	printf(" Extension : %s\n", extens);
	
//	exit(-1);


//***************************	


NEXT_PARAM:


// Check next input parameter

	argcount++;			// next argument
	printf("As per user, colours to impose to bitmap : %s \n", argv[argcount]);

  ii=0;
  strcpy(temp, argv[argcount]);
  p = strtok(temp, ",");

  while(p != NULL) 
	  {
 	  //printf("Token as a string: %s\n", p); 
	  input_color[ii++] = strtol(p,NULL, 10);
	  //printf("%d Token as an integer: %d\n", ii-1, input_color[ii-1]);	
	  p = strtok(NULL, ",");
	  }


// *************************************************	

IN_IMA:

// exit(-1);		// useful when testing only the input parameters


	printf("\n\t*Opening input bitmap file to modify : %s \n", file_in);	
	
 	//ima_in = open_imaFile(file_in);			// uses itc_io_g function to open image (bitmap) file (a GDALDataset pointer)

	//xsize = Pixels; ysize = Lines;

//	const GDALAccess eAccess = GA_Update;
//	ima_in = (GDALDataset *) GDALOpen( file_in, eAccess);


	ima_in = (GDALDataset *) GDALOpen( file_in,  GA_Update);		//pointer to file dataset for update


//exit(-1);				// for debugging

// Verify that it is actually a bitmap (tif, nbits=1) ie., not another tif type

	piBand = ima_in->GetRasterBand(1);

	if ( piBand->GetMetadataItem("NBITS","IMAGE_STRUCTURE") == NULL)
	  {
	  printf("\n\t*** Specified file %s may not be a bitmap... EXITING !!!\n", file_in);
	  exit(-1);
	  }





//*********************************************************************

// For testing .... use a separate output file

/*

fprintf(stdout,"\nReading input bitmap\ ...n");

	if(segm_in == 0) segm_in = 1;		// always 1 for tiff  file bitmap, different for PCI files
	inbitbuffer = read_bitmap(file_in, segm_in);	 
	safety_zone(inbitbuffer);

  
 // Writing output  bitmap 

	strcpy(file_out, "test.tif");		// for testing purposes
	ima_out = open_imaFile(file_out);
	printf("\n\t **Writing input bitmap into output file : %s \n\n", file_out);
	
	write_bitmap(inbitbuffer, file_out);

*/

//*********************************************************************

// 		Create colour table

//	GDALColorTable * col_tab_out = nullptr;		// declare output colour table pointer
//	col_tab_out =  &GDALColorTable();			// construct output colour table	

    GDALColorTable *col_tab_out = piBand->GetColorTable();	// get existing color table

	
// 	Create Color entries
	
    // GDALColorEntry * col_ent_out = nullptr;		// declare
	
//	GDALColorEntry * col_ent_out;		// declare	
//	col_ent_out = &GDALColorEntry();			// construct (i.e., reserve memory)

	GDALColorEntry col_ent_out;		// declare
	
	col_ent_out.c1 = 0; col_ent_out.c2 = 0; col_ent_out.c3 = 0; col_ent_out.c4 = 0;	
	
	col_tab_out->SetColorEntry(0, &col_ent_out);


	col_ent_out.c1 = input_color[0]; col_ent_out.c2 = input_color[1]; 		// user entered colours
	col_ent_out.c3 = input_color[2]; col_ent_out.c4 = input_color[3];	
//	col_ent_out.c1 = 0; col_ent_out.c2 = 100; col_ent_out.c3 = 0; col_ent_out.c4 = 255;	// for TESTING Green

	col_tab_out->SetColorEntry(1, &col_ent_out);

//	col_tab_out->SetColorEntry(2, col_ent_out);  // For testing

//	col_tab_out->SetColorEntry(2, (0,0,120));   // For testing




//  Print resulting color table to VERIFY

/* 	const GDALColorEntry * col_ent = nullptr;			// declare
	col_ent = &GDALColorEntry();			// construct (i.e., reserve memory)	 
	
	col_ent = col_tab_out->GetColorEntry(0);
 	printf("\t\t* Color entries for slot  0  : %d %d %d %d \n", col_ent->c1, col_ent->c2, col_ent->c3, col_ent->c4);

	col_ent = col_tab_out->GetColorEntry(1);
 	printf("\t\t* Color entries for slot  1  : %d %d %d %d \n", col_ent->c1, col_ent->c2, col_ent->c3, col_ent->c4);
 */
//	col_ent = col_tab_out->GetColorEntry(2);
// 	printf("\t\t* Color entries for slot  2  : %d %d %d %d \n\n", col_ent->c1, col_ent->c2, col_ent->c3, col_ent->c4);




// Write Color Table out to bitmap(tif) file

//	printf("\n\tWriting NEW Color Table to file \"%s\" \n", file_out);


	printf("\n\tWriting NEW Color Table to file \"%s\" \n", file_in);

	//piBand->SetColorTable(col_tab_out);
	
	piBand->SetColorTable(col_tab_out);
	
	piBand->SetColorInterpretation(GCI_PaletteIndex);			// typically not necessary
	
// Set "nodata values" for software that need  that

	//printf("\n\n\tWriting NoDataValue and Description  \n\n");	
	piBand->SetNoDataValue(0);			// typically not necessary




// If using a separate output file

//	poBand = ima_out->GetRasterBand( 1 );
//	poBand->SetColorTable(col_tab_out);		


//	poBand->SetColorInterpretation(GCI_PaletteIndex);			// typically not necessary

// Set "nodata values" for software that use that
	
//	poBand->SetNoDataValue(0);

/*
//	printf("\n\t **Writing modified bitmap into: %s \n\n", file_out);
	
//	write_bitmap(inbitbuffer, file_out);

*/



// Close input and output images

Exit:	printf("\nClosing image file (a bitmap) with new colors \n");

//	GDALClose(file_in);		// creates problems ????
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
