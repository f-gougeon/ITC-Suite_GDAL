/*
C+
c	ITCISOL.c	(Individual Tree Crown Isolation)
c
c	ITCISOL produces an output bitmap segment showing
c	distinct individual tree crowns (ITC). ITCISOL uses
c	a rule-based approach to continue and formalize the 
c	outlines of tree crowns and tree clusters partially 
c	delineated by ITCVFOL (valley-following approach).
c	
c1	PARAMETERS
c
c	ITCISOL is controlled by the following global parameters:
c
c	Name	Prompt									Count	Type
c
c	FILE	Database File Name						1-64	Char
c	VFOLBIT	Input Bitmap from ITCVFOL				1		Int
c	ISOLBIT	Output Bitmap to contain ITCs 			0-1		Int
c	FORTYPE	Forest type (MATURE/REGEN/TROPIC/PROMPT	64 	Char
C	REPORT	Report Mode: TERM/OFF/filename			64	Char
c
c2	FILE
c
c	Specifies name of PCIDSK file to which the database bitmap
c	segments are read and written.
c
c	EASI>FILE="filename"
c
c2	VFOLBIT
c
c	Specifies the input segment number of the bitmap produced
c	by ITCVFOL with tree crowns partially separated.
c
c	EASI>VFOLBIT=n      Only one bitmap.
c
c2	ISOLBIT
c
c	Specifies the output segment number to contain the bitmap
c	of individual tree crowns (ITCs). If ISOLBIT is
c	specified, then that bitmap segment is overwritten.
c	If ISOLBIT is not specified, then a new bitmap segment
c	is created.
c
c	EASI>ISOLBIT=n      Only one bitmap.
c
c2	FORTYPE
c
c	EASI>FORTYPE="forest type"
c
c	Specifies the type of forest to analyse in the image.
c	Possible values are MATURE, REGEN, TROPIC and PROMPT (defaults to MATURE).
c
c	If MATURE, REGEN or TROPIC are specified, the maximum pixel 'bridge' length 
c	will be calculated from the pixel size (1m, 0.5m and 2m, respectively) 
c	If PROMPT is specified (or anything alse), the user will be prompted for a 
c	maximum jump distance, in pixels.
c
c
c2	REPORT
c
c	Specifies the file to append generated report to:
c
c	EASI>REPORT = "filename"
c
c	Note:  The following names have special meaning:
c
c	EASI> REPORT = "TERM"	| generates reports on your terminal
c	EASI> REPORT = "DISK"	| generates reports on file "IMPRPT.LST"
c	EASI> REPORT = "OFF"	| (may) switch off report generation
c
c
c1	DETAILS
c
c
c	ITCISOL uses "rules" to do the finishing touches on the
c	tree crowns relatively well isolated by ITCVFOL.
c	These rules deal with all sorts of specific circumstances
c	in delineating an individual tree crown (ITC) completely
c	by attempting to follow its countour in a clockwise fashion.
c	Generally, it should breakdown tree clusters (given sufficient hints
c	that its not simply a big crown) into two or more smaller crowns.
c
c	It starts by finding a meter square of crown matter...
c
c1	HISTORY
c
c
C	Francois A. Gougeon (c)
C	Natural Resources Canada
c	Canadian Forest Service
c	(Petawawa National Forestry Institute)
c	Pacific Forestry Centre
C	Victoria, BC, Canada
C	
c
C	Revision History:
C	
C Francois Gougeon	v1.0	Apr 91-Aug 93	
c				Development research work under
c				various incarnations on ARIES
c
c Carmen Arimescu	v2.0    Nov.93-Mar 94	
c				Moved from the VAX/ARIES environment
c				to the UNIX/PCI environment
c				(Different image files, and bitmaps
c				versus theme files)
c				From ISOLATION4.FOR, renamed ISOL.F
c
c Simon Alexander	v3.1	Oct 96	
c				As itcisol.c, converted to c from fortran
c				source {ITCISOL.F}
c 				Note:	most structures/algorithms retained from
c					{monolithic} fortran code
c
c Simon Alexander	v3.2	Nov 96	
c				Bugfix and integration with other itc progs
c
c Simon Alexander	v3.3	Dec 96	
c				Clean up memory allocation, fix help, etc.
c
c Simon Alexander	v3.4	Dec 96	
c				Introduced FORTYPE (MATURE,REGEN and PROMPT)
c
c Francois Gougeon	v4.0	Dec 97	
c				Various cleanups and changed I/O parameter
c				names (VFOLBIT, ISOLBIT) for faster
c				consecutive rerun of the whole ITC suite
c
c Francois Gougeon	v4.1	Aug 98
c				Various cleanups & PCI6.2 adaptation.
c
c Francois Gougeon	v4.2	Aug-Sept 98
c				Putting in watershed-based breakup of tree clusters
c				(i.e., rule level 10) and numerous cleaning up
c				and simplifications (and debugging).
c				(Watershed-based breakup of tree clustershad been coded
c				by Simon Alexander in Dec 96, but not put into the rule system)
c				Watershed-based breakup algorithm comes from book 
c				"Practical computer vision using C" by J.R. Parker, p. 302.
c
c Francois Gougeon	v4.3	Sept 98
c				- Inlets into crown are not acted upon immediately depending
c				on max_jump as before, nor are inlets erased (as before)
c				-Crowns with inlets are kept for later by painting them
c				as BADCROWN (see crowns.postponed counter)
c				- v4.3 starts with a minimun max_jump (1 metre for mature
c				trees and 0.5 metres for regen) to let
c				most of the lower level rule do most of the work first.
c				- Then, max_jump is incremented slowly up to MAX_JUMP_MAX
c				- Crowns with inlets that have not been dealt with and/or
c				crowns judged excentric (one diameter three times the other)
c				are passed on, at the very end, to the watershed algorithm
c				(i.e., rule level 10). Level 11 was introduced as a way of getting
c				back to level 1 with some reinitialisation (level 10 creates
c				VFOL-type separation breaking big crowns, but need levels 1-5
c				to be activated to actually delineate the smaller resulting crowns.
c
c
c Francois Gougeon	v4.4	Oct 98
c
c				- Various cleanups
c				- Return to not increasing maxjump and not using level10
c				rules (unstable)
c				- Introduced "last_run" variable so that non-ideal crowns
c				(excentric, too big ...) are finally "painted" like regular crown
c				at the end (i.e., their crown area is useful in overall crown closure
c				assessments and may not affect too much average crown areas)
c
c Francois Gougeon	v4.5	March 2000
c
c				- Removed input parameter DBIW which had never been used
c				  in the program, nor elsewhere in the ITC-suite, to make 
c				  things more consistent and less confusing for newcomers.
c				- Various other cleanups
c
c
c Francois Gougeon	v4.6	Aug. - Sept. 2000
c
c				- Various cleanups to run on PCs (1st ever port to PC world)
c				- args, argcnt, report and some others can not be global vars.
c				  (i.e., just doing STATUS ITCISOL gives a memory access error)
c				- Problems with vars. that are global being also passed as
c				  parameters in functions calls.
c				  (i.e., same type of memory access error)
c				- Problem with TREELOG file -- "/dev/null" is OK for Unix
c				  but need to point to "NUL:" on PCs. Deal with here in the program
c				  so that we dont have to have a different DEFITC.EAS for PCs.
c
c
c Geoff Savinkoff	v4.7	Oct. 2001
c
c				- Added global Lines, Pixels, Channels (so compatible with itc_io.c)
c
c
c				Nov. 2001
c
c				- Updated help file with dependency and linking information
c				- Replaced most IDB functions with GDB functions (not IDBPixelSize or IDBSegInfoIO
c				  as there are no equivalent GDB functions)
c				- Added georeferencing information with GDBGeorefIO
c				- Defined xsize, ysize, channels as local variables to pass to functions to be
c				  consistent with other programs.  Pixels, Lines now the external variables for
c				  isolrules.c 
c				- Changed segment description to make more generic.
c				- added junk pixel filter (from itcsfil.c) to remove all crowns <2x2 pixels 
c				  from final bitmap
c
c
c François Gougeon	v4.8	March 2003
c
c				-Using upper_case() and strncmp() organized prog. such that
c				FORTYPE (MATURE, REGEN and PROMPT) can be in small case letters
c				and that only 3 letters are necessary (e.g., "mat" and "reg")
c				Also variable pix_size was not defined so jump factor was always
c				going to max_jump, which is six pixels.
c				Note: PROMPT does not really exist. Anything other than
c				"mat" and "reg" should triger the prompt mode. That is why
c				MAN for "manual" also work as PROMPT.
c
c
c François Gougeon	v4.9	July, Sept. & Oct. 2004
c
c				- Mods. to deal with the bigger tree crowns of tropical forests.
c				  Changed maximum # of possible boundary elements from 250 to 1000
c				  (i.e., LISTSIZE is defined in "isoldefs.h")
c				- Removed MAXSTEP (cause potential unbalanced control)
c				- Frequent checks for infinite loops in list of boundary elements
c				  (i.e., possibly finding a crown, but from a different tree then
c				  originally expected, thus not back to original x,y position)
c				- Added FORTYPE="TROPIC, for tropical forest analysis. Among other
c				  things, set maxjump to 2 metres and dont call level 10 rule.
c				- Variable "rule_level" was changed to "rule_group", not to be confused
c				  with the rules themselves which are often refered to as "rule levels"
c				  (i.e., higher level rules (like 10,11) are part of a higher level group 3)
c				  Group 2 is a sort of a fake higher level group, as it essentially
c				  uses "group one rules", but changes the maxjump factor.
c				- FORTYPE="MAT3, REG3, and  TRO3 can be used to force further breakdown 
c				  of left over big objects (ISOLs) using a binary watershed 
c				  approach (rule 10) after improvement made is less than 1%
c
c
c François Gougeon	v5.0	March 2007
c
c				- Mods. to deal with the bigger tree crown clusters that sometime
c				  happen at high res. (10-30cm/pixel) when grey-levels are a bit 
c				  saturated and thus, impenetrable by ITCVFOL.
c				  Changed maximum # of possible boundary elements to 2000
c				  (i.e., LISTSIZE is defined in "isoldefs.h") so that these
c				  big clusters can be successfully burned in on final exiting pass,
c				  or be OK to pass-on to rule level 10 (rule group 3)
c
c François Gougeon	v5.1	April 2007
c	
c				- Some big isol (clusters) with semi-flat north side were not getting 
c				picked up, cause west side of isol (VF material) was too far away.
c				Now checking twice for west side.
c
c François Gougeon	v5.2	April 2007
c
c				- Still some instabilities using rule 10, level 3 (e.g., when FORTYPE="MAT3).
c				- Not resolved yet
c
c
c Francois Gougeon	v5.3	April 2007
c
c				- Minor adaptations for PC-PCI v10 and its new PRM.PRM file 
c				  Most PCI routines are in PCI1000.dll, but not all.
c				  For example, IMPTime() and IMPReturn() are now in Core1000.dll
c				  and IMPCounter() in Counter1000.dll
c				  Since I dont have PCISDK or PCIProSDK, I had to create LIBs 
c				  from their DLLs to compile my progs.
c
c				  Also, problem with the REPORT (*Report) Variable
c
c Francois Gougeon	v5.4	May 2007
c
c				- Introduced FORTYPE="PRESET and the EASI variable ISOLJUMP
c				to allow ITCISOL to run in batch mode with a manual maxjump.
c	
c
c Francois Gougeon	v5.5	Jan. 2008
c
c				- Fixed occational instability due to a very particular situation:
c				  from west-side starting point, following immediatly a crown
c				  inlet and then, backtracking, cause going nowhere, was causing
c				  the prog. to assume crown completion (cause back to countour origin).
c				  Trying to burn the tree crown as good, but using an ill-defined centre
c				  of gravity (sometimes going nowhere, so stuck in an infinite loop, or
c				  giving an immediate program crash)
c
c
c Francois Gougeon	v5.6	March 2009
c
c				- Removed writing to log file (TREELOG), because even when not in use
c				  (i.e., writing to NUL:), it was slowing down ITCISOL terribly.
c				  Now need to set a variable DBG to turn this on.
c				 -Also, removed TREELOG as an EASI variable and set that filename 
c				  permanently to "ITCISOL_Rules_Usuage.log" when used (i.e., DBG=1)
c				- Made writing to the output bitmap a separate function (write_bitmap),
c				  in order to use it multiple times and get partial results on every pass,
c				  so that even when ISOL crashes, you get ITC results of the previous pass.
c				  - Changed maximum # of possible boundary elements to 5000
c				  (i.e., LISTSIZE is defined in "isoldefs.h") so that these
c				  big clusters can be successfully burned in as good on final exiting pass
c				  or be OK to pass-on to rule level 10 (rule group 3)
c
c François A. Gougeon	 v5.7	 Dec 2011	
c
c				- Modified to handle bitmaps bigger than 2G positions by reading them
c				in two sections (i.e., the old PCI functions that I have access to
c				are not up to part yet). Itc_io.c was modified to take care of that.
c				- Bitops.c was also modified to use int64 vars to use with pointers 
c				for operations on bitmaps. Calls to setbit(), testbit(), etc.
c				have to be modified to use an (int64) pointer, typically by casting.
c				- Thus, "bitnum" need to be declared "int64 bitnum" everywhere
c				- Be careful of parenthesis as they may prevent "auto casting" to int64
c				from working (i.e., calculation gets done in int32, then cast to int64
c				with bad results. For exemple:
c				"bitnum = (int64) ((y-1)*Pixels) + x-1;" 
c				should be: "bitnum = (y-1)*(int64)Pixels + x-1;"
c
c François A. Gougeon	 v5.8	 June 2012
c
c				- Debugged last run (i.e., remaining poor crowns painted as good)  
c				that was often crashing. BADCROWNs can not be painted from the same
c				initial location as GOODCROWNs. Introduced BAGGOOD crowns to help.
c				- 
c
c
c François Gougeon	v6.0	Feb 2016
c
c			- Changed to a C++ program to deal with versions of PCI > v10.2
c				int main (), ".cpp" name, and extern "C" around .h include files
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
c			- IMPStatus()  uses "char const *", so mod. that in ccltask.h (no mod needed in prog.)
c
c
c François Gougeon	v6.1	Sept. 2016
c
c			- Changed from dealing with bitmaps > 2G positions (irrelevant now: PCI 2015) 
c			  to dealing with bitmaps > 2GB (read in two parts) (PCI2015 still unstable
c			  bitmaps > 2GBytes )
c
c
c
c François Gougeon	v7.0a	Oct 2018	
c
c			- GDAL version (named itcisol_g.cpp)
c	
c
c François Gougeon	v7.1	April 2021
c
c			- Made completely independent of PCI 
c
c
c
c François Gougeon	v7.2	April 2023
c
c			- Not to assume that all files are in the default directory from which the program is run
c				Previously, everything was assumed in same directory and run from a cmd window from there.
c				NOW, the path used with the main input file is used when creating the default output files
c				This was necessary for ArcGIS Toolkit integration.
c
c
c  François Gougeon  v7.3	Nov. 2023	
c
c			- Some code clean-up 
c
c			- Introduced color table and description within output file
c
c  François Gougeon  v7.4	Nov. 2024	
c
c			- Allow user to specify an output file name (previously, was only using default name)
c
c		USAGE >   itcisol_g PRF_NIR_VFOL.tif Output_Filename MATURE
c	
c		USAGE >   itcisol_g PRF_NIR_VFOL.tif - MATURE (prog, will create output filename)
c
c		USAGE >   itcisol_g PRF_NIR_VFOL.tif - - 		(MATURE assumed)
c
c1	REFERENCES	
c
c		All references (and other papers) available in pdf format at:
c
c		https://cfs.nrcan.gc.ca/employees/read/fgougeon
c
c
c	Main crown delineation references:
c
c	Gougeon, F.A. 1995. A crown-following approach to the automatic delineation of
c	individual tree crowns in high spatial resolution aerial images. Can. J. Rem.
c	Sens. 21(3):274-284.
c	
c	Gougeon, F.A. 1998. Automatic individual tree crown delineation using a
c	valley-following algorithm and rule-based system. Proc. Int'l Forum on
c	Automated Interpretation of High Spatial Resolution  Digital Imagery for
c	Forestry. February 10-12, Victoria, B.C., Canada.  12 p.
c	
c	Main ITC Suite references:
c
c	Gougeon, F.A. 2010. The ITC Suite Manual : A Semi-Automatic Individual Tree Crown (ITC) 
c	Approach to Forest Inventories. Natural Resources Canada, Canadian Forest Service, 
c	Pacific Forestry Centre, Victoria, B.C. Canada. June 2010. 92  p. 
c 
c	Gougeon, F.A.; Leckie, D.G. 2003. Forest information extraction from high spatial
c	resolution images using an individual tree crown approach. PFC Information Report.
c	BC-X-396 (and in French BC-X-396-F ) Natural Resources Canada, Canadian Forest
c	Service, Victoria, B.C., Canada. 26 p.
c	
c	
C-
C	Dependencies:
C	
C	This program runs under the PCI EASI/PACE Image Analysis System.
C	EASI/PACE is a copyright of 	PCI Inc., 50 West Wilmot St., 
C					Richmond Hill, Ontario, Canada
C	
C
C		Libraries:	pcilib.a	(PCI EASI/PACE lib)
C				libm.a		(Standard Math Library)
C		
C		Includes:	pci.h		(PCI EASI/PACE)
C				string.h	(Standard C string header file)
C				math.h		(Standard C math header file)
C				stdarg.h	(Standard C header file)			
C				itc_io.h	(ITC suite Main IO functions)
C				bitops.h	(ITC suite bit operation functions)
C				error.h		(ITC suite error handling)
C				isoldefs.h	(ITC suite Isolation definitions)
c
c
c	Linking information (or use the Makefile)
c
c	gcc -c -O  -I/package/pci7.0/lib  -o itcisol.o itcisol.c
c	gcc -c -O  -I/package/pci7.0/lib  -o isolrules.o isolrules.c
c	g++ -O -o ./exe/ITCISOL.EXE itcisol.o isolrules.o error.o bitops.o itc_io.o 
c			       /package/pci7.0/lib/pci.a -lm 
c
c
c*************************************************************************
c   Old ARIES description:
c
C	NAME:		ISOLATION4.for
C	DESCRIPTION:	Program that produces an output theme image showing
c			distinct individual tree crowns.
c
c			It typically uses the best band of the image (nIR) and
c			the thematic image of partially isolated crowns
c			resulting from VALLEYFOL.FOR.
c
c			It uses rules to do the finishing touches on the
c			tree crowns relatively well isolated by VALLEYFOL.FOR.
c			These rules deal with all sorts of particular
c			circumstances in delineating the tree crowns completely.
c
c			It starts by finding a meter square of crown matter.
c
c			GOAL:  to completely delineate individual tree crowns
C
C	NOTE:		ASSUMES YOU ARE AT THE IMAGE DIRECTORY
C	LINK:		ISOLATION4,DIPIXLIB/LIB
C
C	AUTHOR:		FRANÇOIS A. GOUGEON
C	DATE:		April 91 - Oct. 91 (IN VARIOUS INCARNATIONS)
c			May 93 - Aug 93 (ISOLATION4.FOR (levels 1-5))
c*************************************************************************
c
c 	Revision 2.0    CARMEN ARIMESCU for FRANÇOIS A. GOUGEON
c	Revision Nov.93-Mar 94 of ISOLATION4.FOR (renamed ISOL.F)
c	Initial Revision to modify software to operate under EASI/PACE v5.2
c	1992, PCI Inc., 50 West Wilmot St., RICHMOND Hill, Ontario, Canada
c
c	These revisions also change the comenting style to be compatibile
c	with PCI's format so that in-code documentation can be extracted
c	for use in user manuals and for HELP files.
c
c*************************************************************************
c Version 3.1 Simon Alexander (itcisol.c) (OCT 96)
c
c       converted to c from fortran source {ITCISOL.F}
c
c       note: 	most structures/algorithms retained from
c				{monolithic} fortran code
c
c Version 3.2 bugfix and integration with other itc progs
c
c Version 3.3 clean up memory allocation, fix help, etc.
c
C********************************************************************

**** To compile with Visual Studio (VS14)  (see VS14_GDAL_compile.txt)

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64

set INCLUDE=C:\gdal-2.1.1_v2\include;C:\libtiff-4.0.6_64b\tiff-4.0.6\libtiff;%INCLUDE%
set LIB=C:\gdal-2.1.1_v2\lib;C:\libtiff-4.0.6_64b\tiff-4.0.6\libtiff;%LIB%

set LINK=gdal_i.lib  libtiff_i.lib User32.lib

set CL= /MD

CL xxxxxxxxxx.cpp /EHsc

**************************************************************

NOTE: Newest GDAL library have included "libtiff" no need to have it anymore

set INCLUDE=D:\GDAL_3.0.0\include;%INCLUDE%
set LIB=D:\GDAL_3.0.0\lib;%LIB%
set LINK=gdal_i.lib  libtiff_i.lib User32.lib
set CL= /MD
CL xxxxxxxxx.cpp /EHsc

set path=D:\GDAL_3.0.0;%PATH%		// to run programs

rem  GDAL v3.0.0 has problem getting to its projection library
set PROJ_LIB=D:\GDAL_3.0.0\projlib

*********************
Ackowlegment to GDAL:

GDAL - Geospatial Data Abstraction Library: Version 2.1.1 (July2016, 64bit), 
GDAL - Geospatial Data Abstraction Library: Version 3.0.0 (Dec. 2019, 64bit),
Open Source Geospatial Foundation, 
Thanks Frank (Warmerdam)






********************************************************************************
	PROGRAM USAGE

   	was:	IMPStatus("FILE,VFOLBIT,ISOLBIT,FORTYPE,REPORT;",

	now:	itcisol_g   vfol_bitmap.tif isol_bitmap.tif forest_type


	Exemples:	itcisol_g PRF_NIR_VFOL.tif PRF_NIR_ISOL_v2.tif MATURE

			 itcisol_g PRF_NIR_VFOL.tif -  MAT (prog, will create output filename)

		Here, the output file will typically be named "BaseName_ISOL.tif"  
			(may be renamed if needed afterward)

			itcisol_g PRF_NIR_VFOL.tif -  -  (Mature will be the default)


******************************************************************

*/



/* Modern C (Visual Studio 8 (2005)) do not want to use the std lib but the more secure 
	newer library and thus will complain (DEPRECATE) unless these two are set   
*/
	
#define _CRT_SECURE_NO_DEPRECATE 1
#define _CRT_NONSTDC_NO_DEPRECATE 1

//#define DEBUG_ME 1		// if defined, prog. will show more info for debugging

#define VERSION "v7.4a"
#define PROG_NAME "ITCISOL_G"

#define FILENAME 132

#include <stddef.h>
#include <stdio.h>
//#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <limits.h>
#include <time.h>       // time_t, struct tm, time, localtime
//#include <direct.h> 	// _getcwd
//#include <windows.h>		//for Module Handle
//#include <process.h>		// for getpid()
//#include  <psapi.h>


//#include "pci.h"		// not anymore

#include "ITC-Suite_g.h"
#include "itc_io_g.h"

#include "isoldefs.h"

#include "error.h"
#include "bitops.h"

//#include "gdal_priv.h"	// For GDAL library, now included from "ITC-Suite_g.h"


// Global declarations

FILE	*logfile;
int	Lines, Pixels, Channels;
PixVal	*vfolbmp, *bitmap, *isolbmp;
PixVal	*vfol_bmp;

PixVal	*good_crown_bmp;
PixVal	*bad_crown_bmp;
PixVal  *tempbitbuffer;
int	*wshed, wshed_line, wshed_size, wshed_col;
int             max_jump, max_jump_org;
Crown_info      crowns;
float		pix_size;
int		prev_num_found;
int		rule_group = 1;		// Rules of higher levels originally turned OFF 
int		last_run = 0;		// To signify that impossible to break clusters should be output anyway
float           pix_xsize, pix_ysize;
char            pix_units[9], timedate[17];
int             xoff = 0, yoff = 0;
int             vfolbit, isolbit;
FILE           *idb_fp;

char            treelog[64];
char		forest_type[64];
int         	xsize, ysize, channels;
int		DBG = 0;		// set DBG to one to activate logfile
int		loop_count;


// GDAL global variables to deal with files and images (that are used by itc_io_g)

float		xpixsz, ypixsz;
int         blocks, option;
int64 		bmsize;		// size of bitmap in bytes 
char 		*Proj, *Proj1, *Proj2, *Datum, *Datum2, *temp, *Temp, *token;

GDALDataset	*bmp_in, *bmp_out;
GDALDriver 	*piDriver, *poDriver;
char 		**papszOptions = NULL;

GDALRasterBand	*piBand, *poBand;
PixVal		*pafScanline;
char 		**papszMetadata;
double		adfGeoTransform[6], adfGeoTransform2[6];
double 		topleftX, transformX, topleftY, transformY; 

int 		ch_in, ch_out, in_ch[1], segm_in, in_segm[1];

int			xcg, ycg;					// center of gravity of current polygon

//extern	GDALRasterBand	*piBand,*piBand2,*poBand;

int	 	imaFile_opened;
GDALDataset	*ima_in, *seg_in;

int	by_lines=0, by_image=1;		// default is to read/write by image (faster), 
					// user can specify bylines (slower) if not enough memory to go by image

int 	bylines_flag;	// user can specify bylines (slower) if not enough memory to go by imag

char		filename[FILENAME], fullfilename[FILENAME], shortfilename[FILENAME];
char		basefname[FILENAME], mainfname[FILENAME], directory[FILENAME];
char		file_out[FILENAME], extension[10];
char 		*pch;

int 		data_type;
char 		description[128];		// for PCI
char 		Description[128];		// for GDAL via itc_io_g.cpp
char 		Extension[10]="";
char 		answer[5]="";

int 		PCI_File = 0;			// initial flags for image file type
int 		TIF_File = 0;
int 		name_given = 0;		// flag output file name given(1) or not(0)

time_t rawtime;
struct tm * timeinfo;

/* function declarations */

void	isol(char *);
void	pass_report();
void	prep_bmp(PixVal *, int, int);
void	fill_clears(int, int);
void    upper_case(char *);
void    write_bitmap(PixVal *);


/* extern "C" {int iterate_over_image(int);
} */

int iterate_over_image(int); 

//***************************************************************************

// main program 

//***************************************************************************

int main(int argc, char *argv[])
{

/* local variables */

int		i, j, j_count, ii, jj, iii, jjj;
int64		bitnum;
char		geosys[17];
double		topleftX,transformX,topleftY,transformY;
//char		report[FILENAME];
void		*args[5];
int		argcnt[5];

int64		count;




//*****************************************************************************

/*

// Init PCI interface 

	args[0] = (void *) file;
	args[1] = (void *) &vfolbit;
	args[2] = (void *) &isolbit;
	args[3] = (void *) forest_type;
	args[4] = (void *) report;

	IMPStatus("FILE,VFOLBIT,ISOLBIT,FORTYPE,REPORT;",
		  "C   ,I      ,I      ,C      ,C     ;",
		  "132 ,1      ,1      ,6      ,132   ;",
		  "1   ,1      ,0      ,0      ,0     ;",
		  PROG_NAME, "FORCE", argcnt, args, argc, argv);		  

// Print Header 

IMPTime(timedate,4); 
fprintf(stdout,"\n\t\tStarting %s (%s) at %s\n", PROG_NAME, VERSION, timedate); 

// open PCI database file - test if file name exists 

ALLRegister();

idb_fp = GDBOpen( file,"r+" );
if (idb_fp == NULL) 
	{
	fprintf(stderr,"Unable to open the PIX file for read and write access.  Aborting.\n");
	exit(-1);
	}
fprintf(stdout,"\nOpening PCI file: %s  \n", file);

*/

//*****************************************************************************


// Print GDAL-related version Program Header 

	time (&rawtime);  timeinfo = localtime (&rawtime);
	fprintf(stdout,"\n\t\tStarting %s (%s) at %s\n", PROG_NAME, VERSION, asctime(timeinfo));

// Check arguments on command line

	if (argv[1] == NULL) 
	  { 
	  printf("\n\n PROBLEM with INPUT VFOL bitmap on command line: %s \n",argv[1]);
	  printf("\tHave a VFOL bitmap as first argument  \n\n");
	  printf("USAGE: itcisol_g Area_VFOL.tif Output_Filename Mature|Regen\n");
	  printf("USAGE: itcisol_g Area_VFOL.tif -|# Mature|Regen\n");
	  exit(-1);  
	  }
 
	  
 if ( EQUALN(argv[2],"-",1 ) || EQUALN(argv[2],"#",1 ) )
	  {
	  printf("\nSecond argument is \"-\" or \"#\", which implies no output file given \n");
	  printf("A new output bitmap will be created based on \"base file name\" \n\n"); 
	  name_given = 0;
	  }	
 else
	{
	strcpy(file_out,argv[2]);
	name_given = 1;
	}
 
 
 
	if (argv[3] == NULL) 
	  { printf("\n\n PROBLEM with forest type variable on command line: %s \n",argv[3]);
	  printf("\tHave forest type (e.g., MATURE, REGEN) as third argument  \n\n");
	  exit(1);} 

	 strncpy(forest_type, argv[3], 9);

	  
 if ( EQUALN(argv[3],"-",1 ) || EQUALN(argv[3],"#",1 ) )
	  {
	  printf("\nThird argument is \"-\" or \"#\", which implies MATURE assumed by default\n");
	  strcpy(forest_type,"MATURE");
	  }	
 
 

 	if (argc < 3)	  
	  {
	  printf("\n\t PROBLEM with input parameters \n\n");
	  printf("USAGE: itcisol_g Area_VFOL.tif Output_Filename Mature|Regen\n");
	  printf("USAGE: itcisol_g Area_VFOL.tif -|# Mature|Regen\n");
	  exit(-1);
	  }  
 	  
	  

//******************************************************


// Open an output log file (if debugging mode, otherwise too big)


	strcpy(treelog, "ITCISOL_Rules_Usage.log");

	if(DBG) 
	{
	  fprintf(stdout,"As requested, very detailed step-by-step info goes to : %s \n", treelog);
	  logfile = fopen(treelog, "w");
	}


// Open input PCI or TIF file via GDAL

	GDALAllRegister(); 	 // registers for all types of image files

// Open and read bitmap file containning the VFOL bitmap
	
// Check if tif file or PCI file. If multi-channel,  ask channel number

	strcpy(fullfilename, argv[1]);
	printf("Full Input Filename:  %s \n\n", fullfilename);
	
	pch = strtok(fullfilename,".");				// put a NULL where "." was
	strncpy(mainfname, fullfilename, strlen(fullfilename));
	
	printf("  Mainfname:  %s \n", mainfname);	
	
	pch = strtok(NULL," \0"); 			//continue search on original fullfilename 
	strncpy(extension, pch, 3);
	
	printf("  Extension : %s\n", extension);	
	//exit(0);
	
	
	strcpy(Extension, extension);		// global variable for some other prog.	
	
	
	
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
	//printf("Shortfilename:  %s \n", shortfilename);

	
// Extension is a global parameter set by Open_imaFile()
	
	if (EQUALN(Extension,"pix",3)) PCI_File = 1;	// everything is in the PCI file
	if (EQUALN(Extension,"tif",3)) TIF_File = 1;	// everything is in directory, mostly as tif files

	//if (! ( PCI_File || TIF_File))
	if (! TIF_File)	
	  {
	  printf("\n\n**ERROR** Program not able to deal with image file of type %s\n\n", Extension);
	  exit(-1);
      }
	
	
	
	bmp_in =  (GDALDataset *) GDALOpen( argv[1], GA_ReadOnly );		// bmp_in is pointer to file dataset

	if (bmp_in == NULL) 
	  { printf("\n\n PROBLEM opening image file %s \n\n",argv[1]); exit(1); }

	fprintf(stdout,"\n\t*File %s was opened for reading\n\n",argv[1]);

// Get and printgeneric info of input file (driver used,... )

	printf( "Driver: %s/%s\n",
          bmp_in->GetDriver()->GetDescription(),
          bmp_in->GetDriver()->GetMetadataItem( GDAL_DMD_LONGNAME ) );

	Pixels = xsize = bmp_in->GetRasterXSize();
	Lines = ysize = bmp_in->GetRasterYSize();
        Channels = channels = bmp_in->GetRasterCount();

	printf( "Image size is %d x %d x %d\n\n", Pixels, Lines, Channels );

	bmsize =  sizeof(unsigned char) * ((Pixels*(int64)Lines + 7) / 8);

	fprintf(stdout,"Size of one bitmap: %I64d bytes \n\n", bmsize);

// Get and print geographic info of input file

	Proj1 = (char *) CPLMalloc(200);
	//strncpy(Proj1,bmp_in->GetProjectionRef(),200);
	temp = (char *) CPLMalloc(200);

// Get Proper Geo projection of ILLUMIN image

	if( bmp_in->GetProjectionRef() != NULL ) 
	  {
	  strncpy(temp, bmp_in->GetProjectionRef(),200);
	  strtok(temp, "\"");
	  printf("Proj 1st section : %s \n", temp);
 	  Proj1 = strtok(NULL, "\"");
	  printf("Projection: %s \n", Proj1 );
	  }	  
	 	
	
	
	//if( bmp_in->GetProjectionRef() != NULL)  printf( "Projection is '%s'\n\n", Proj);


	if( bmp_in->GetGeoTransform( adfGeoTransform ) == CE_None )
	  {
	  printf( "Origin = (%.2f,%.2f)\n", adfGeoTransform[0], adfGeoTransform[3] );
	  printf( "Pixel Size = (%.2f,%.2f)\n", adfGeoTransform[1], adfGeoTransform[5] );
	  }


// allocate memory for the internal bitmap

	isolbmp = (PixVal *) calloc(bmsize,1);				


// Read VFOL bitmap (via an 8bit buffer)


	printf("\n\t*Reading VFOL bitmap: %s \n\n", argv[1]);

	vfolbmp = (PixVal *) calloc(bmsize,1);				// allocate memory for the internal bitmap
	for (iii=0; iii<bmsize; iii++) *(vfolbmp+iii) = 0;		// make sure it is all zeroed
	
	vfol_bmp = vfolbmp ;			// some processing functions are using "vfol_bmp" as global vars

	bitmap = (PixVal *) calloc(sizeof(PixVal)*Pixels*Lines,1); 	 // allocate memory for a "Full 8bit image" to read data in

	piBand = bmp_in->GetRasterBand( 1 );

	printf("Raster number=%d and Type = %s \n", piBand->GetRasterDataType(), GDALGetDataTypeName(piBand->GetRasterDataType()) );

// Check that incoming channel is a bitmap before doing anything (otherwise you'll get junk)

	//if ( strncmp(piBand->GetMetadataItem("NBITS","IMAGE_STRUCTURE"),"1",1 ) != 0)
	if ( piBand->GetMetadataItem("NBITS","IMAGE_STRUCTURE") == NULL)
	  {
	  printf("\n\t*** Apparently, the specified input file is not a bitmap layer\n");
	  printf("\tPlease double check (Hint: use gdalinfo on the file).... Exiting\n\n");
	  goto Exit;
	  }



// Read input bitmap (1bit tif) in one shot


	piBand->RasterIO(GF_Read, 0, 0, Pixels, Lines, bitmap, Pixels, Lines, GDT_Byte,0, 0 );


// For testing 

#ifdef DEBUG_ME
	count = 0;
	for (ii=0; ii<Pixels*Lines; ii++) if (bitmap[ii] == 1) count++;
	printf("\n Pixels set in input 8bit bitmap = %Ii \n", count);
#endif

// Turn 8bit image into an internal  bitmap for further processing
// (i.e.,to be compatible with PCI version of main code)

	printf("\n** Turning input tif (nbits=1, read as 8bit) into a bitmap in memory\n");

	for ( i = 1 ; i <= (ysize) ; i++ )		// For compatibility with PCI(x,y), I kept image coordinates start at (1,1)
	for ( j = 1 ; j <= (xsize) ; j++ ) 		// However, GDAL images start at zero (similar to bitmaps), so bytenum=bitnum

	  {
	  bitnum = (i-1)*(int64)Pixels + j-1 ;		// bitnum starts at zero, so does image, so bytenum=bitnum
	  if (bitmap[bitnum])  setbit(vfolbmp, bitnum);
	  }


// For testing 
#ifdef DEBUG_ME
	count = 0;
	for (ii=0; ii<bmsize*8; ii++) if (testbit(vfolbmp, ii)) count++;
	printf("\n Pixels set in input VFOL bitmap = %Ii \n", count);
#endif


// For initial testing, bypass the work and just copy input bitmap to output bitmap
/* 
 	printf("\n\n******* Moving VFOL bitmap to ISOL bitmap (for quick in/out TEST) *****\n\n");
	memcpy(isolbmp, vfolbmp, bmsize);
	goto END;
  */


//*******************************************


fprintf(stdout, "\nAllocating memory for two temp bitmaps\n");

// good_crown_bmp = alloc_read_bmp(idb_fp, xsize, ysize, 0);
// bad_crown_bmp = alloc_read_bmp(idb_fp, xsize, ysize, 0);

good_crown_bmp = (PixVal *) calloc(bmsize,1);			// allocate memory for the bitmap
bad_crown_bmp = (PixVal *) calloc(bmsize,1);			// allocate memory for the bitmap

/* setting up protective border around the bitmap */

prep_bmp(vfolbmp, xsize, ysize);
	
/*	fprintf(stdout,"\nTEST: Writing VFOL bitmap to file: %s  \n", file);
	write_bitmap(vfolbmp); 
*/

/***************************/

/* MAIN PROCESSING */

/***************************/


	isol(forest_type); 

//goto END;


/***************************/

/* CLEANING UP all trees that are less than 2x2 */

fprintf(stdout,"\nCleaning up small (lost) crown segments ( < 2x2 pixels)\n");

free(vfolbmp); free(bad_crown_bmp);  	 /* free memory to make more room for tempbitbuffer */

//tempbitbuffer = alloc_read_bmp(idb_fp, xsize, ysize, 0);

tempbitbuffer = (PixVal *) CPLMalloc(bmsize);			// allocate memory for the bitmap


memcpy(tempbitbuffer, good_crown_bmp, bmsize);
safety_zone(tempbitbuffer);		/* safety border is just one pixel wide (set to zero) */

printf("\nNOTE:\tIf ITCSC crashed during this cleanup process, you may have a huge tree cluster somewhere.\n");
printf("\tView the VFOL bitmap to verify. If so, rerun ITCVFOL with better thresholds or smoothing\n");
printf("\tOf course, if that area is troublesome, you can also mask that area\n");

/* 
		Scan image, filling all trees that are AT LEAST 2x2 pixels in size
		(i.e., remove all good trees in tempbitbuffer so you are left with odd pixels 
*/

for ( i = 4 ; i < (ysize-4) ; i++ )
{
for ( j = 4 ; j < (xsize-4) ; j++ )
  {
  bitnum = i * (int64) xsize + j  ;

  // Find initial 2x2 area of crown material 
  
  if ( testbit(tempbitbuffer,  bitnum)
	&& testbit(tempbitbuffer,  bitnum+1)
	&& testbit(tempbitbuffer,  bitnum+xsize) 
	&& testbit(tempbitbuffer,  bitnum+xsize+1) ) 	 
	{
	fill_clears(j,i);		  // erase crown found 
	}
	
  }  /* end of scan image loop */

  // if ( (i/100)*100 == i)  printf("%d lines done\r", i);

}

/* fprintf(stdout,"\nDONE  Cleaning out good crown segments \n"); */

/* Scan image again, clearing all bad bits (i.e., small segments) in the good_crown_bmp 
  based on what is presently left in tempbitbuffer  */

j_count = 0;


for ( i = 0 ; i < ysize ; i++ )
for ( j = 0 ; j < xsize ; j++ ) 

  {
  bitnum = i * (int64)xsize + j ;
  if ( testbit(tempbitbuffer,  bitnum) ) 
    {
    clearbit(good_crown_bmp,  bitnum);
    j_count = j_count + 1;
    }
  }

fprintf(stdout,"\n\n %d junk pixels removed \n", j_count);


/****************************************/

// fprintf(stdout,"\nWriting final ISOL bitmap to file: %s  \n", file);

// write_bitmap(good_crown_bmp);			// this library module was created later

/****************************************/

/* close file and finish program properly */


END:

// Presently, a fixed file name is used. Less parameters to put on command line
// The user can always change the name immediatly after the run if needed (or in a script)

// ALSO, originaly assumed everything is done in the same directory (input and output files)
// which is OK with running from a "cmd window" and OK with our philosophy of a single dir as DB,
	//file_out="xxxx_ISOL.tif";
	//extension="tif";
// but not OK with ArcGIS who needs to know precisely where to put the output file (crashes otherwise)
// SO, need to deal with full path


	strcpy(fullfilename,argv[1]);			// main input filename (and possibly its dir)
//	printf("Full Filename:  %s \n", fullfilename);
//	printf("Full Filename Length:  %zd \n",  strlen(fullfilename));

	int basef_len;		
	for (i=0; i < strlen(fullfilename); i++)
	  {
	  j = strlen(fullfilename) - i;
	  //printf("Count back: %d",j);
	  if(fullfilename[j] == '_') {basef_len = j;	break;}	// find last underscore in full file name
	  }
//	printf("\nBase Filename Length:  %d \n", basef_len);
	
	strncpy(basefname, fullfilename, basef_len);
	basefname[basef_len] = '\0';   					/* null character manually added */
	
	//printf("Full Filename:  %s \n", fullfilename);
	//printf("\nBasefname ::  %s \n", basefname);			// may have full path to directory
	
	//exit(0);
	
	
	
//	file_out = strncat(basefname,"_ISOL",5); 


	if(!name_given)		// if no output file name given create one
	  {
	  pch = strncat(basefname,"_ISOL",5); 	
	  strncpy(file_out, pch, strlen(basefname));			 			  	  	
 	  strncat(file_out,".tif",4);
	  //printf("File_out  :  %s \n", file_out); 
	  }
	
printf( "\nWriting ISOL ouput bitmap as a 1bit tif, as %s ...\n", file_out);

	poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");

	if(poDriver == NULL) 
	   {
	   printf("\n ###Cant find proper driver for this file type %s \n\n", extension); 
	   exit( 1 );
	   }

// Create a tif file equivalent of a bitmap (NBITS=1)

  	papszOptions = CSLSetNameValue( papszOptions, "NBITS", "1" );
    	//papszOptions = CSLSetNameValue( papszOptions, "COMPRESS", "PACKBITS" );

	bmp_out = (GDALDataset *) poDriver->Create( file_out, Pixels, Lines, 1, GDT_Byte, papszOptions );

// check that opening worked 

	if (bmp_out == NULL) 
	  { printf("\n\n PROBLEM opening output image file %s \n\n", file_out); exit(1); }

	fprintf(stdout,"\n\t*File %s was opened for writing\n", file_out);


// Copy the metadata (GeoTransform, projection, ...)

	fprintf(stdout,"\nWriting Geographic Projection data to file %s\n", file_out);	

	bmp_out->SetGeoTransform(adfGeoTransform);

	bmp_out->SetProjection(bmp_in->GetProjectionRef() );


// Write "full bitmap" data out to a 1-bit tif file (*** but via an 8-bit buffer)

	fprintf(stdout,"\nWriting ISOL bitmap data to file %s\n", file_out);

//	isolbmp = good_crown_bmp; 		// ##### Bypass for debugging (with full analysis bypass)
	isolbmp = good_crown_bmp; 


#ifdef DEBUG_ME
	count = 0;
	for (ii=0; ii<bmsize*8; ii++) if (testbit(isolbmp, ii)) count++;
	printf("\n Pixels set in output ISOL bitmap = %Ii \n\n", count);
#endif


// Turn a bitmap in memory into a 1-bit tif image (written via  a byte, yet nbits=1 tif) 

	printf("\n** Turning a bitmap in memory into a tif image (written via 8bit, yet nbits=1) \n");

	for ( i = 1 ; i <= (ysize) ; i++ )		// For compatibility with PCI(x,y), I kept image coordinates start at (1,1)
	for ( j = 1 ; j <= (xsize) ; j++ ) 		// However, GDAL images start at zero (similar to bitmaps), so bytenum=bitnum

	  {
	  bitnum = (i-1)*(int64)Pixels + j-1 ;		// bitnum starts at zero, so does image, so bytenum=bitnum
	  bitmap[bitnum] = 0 ;
	  if (testbit(isolbmp, bitnum)) bitmap[bitnum] = 1;
	  }

#ifdef DEBUG_ME
	count = 0;
	for (ii=0; ii<Pixels*Lines; ii++) if (bitmap[ii] == 1) count++;
	printf("\n Pixels set in output 8bit bitmap = %Ii \n\n", count);
#endif



// Write image (here, a bitmap) to file in one shot

	poBand = bmp_out->GetRasterBand( 1 );

	poBand->RasterIO(GF_Write, 0, 0, Pixels, Lines, bitmap, Pixels, Lines, GDT_Byte,0, 0 );



// Write description to output file

	sprintf(description,"ITCISOL - ITCs type = %s from %s ", forest_type, shortfilename);
	
	printf( "\nDescription within output file will be : %s\n", description);

	poBand->SetDescription(description);


// Prep an OUTPUT color table 

	//poBand->SetColorInterpretation(GDALColorInterp::GCI_PaletteIndex);
		
	GDALColorTable * col_tab_out = nullptr;		// declare output colour table pointer
	col_tab_out =  &GDALColorTable();			// construct output colour table	
	//col_tab_out =  &GDALColorTable(GPI_RGB);			// construct output colour table
	
// Color entries
	
    GDALColorEntry * col_ent_out = nullptr;		// declare
	col_ent_out = &GDALColorEntry();			// construct (i.e., reserve memory)
	
	col_ent_out->c1 = 0; col_ent_out->c2 = 0; col_ent_out->c3 = 0; col_ent_out->c4 = 255;
	col_tab_out->SetColorEntry(0, col_ent_out);
	//col_tab_out->SetColorEntry(1, col_ent_out);		// for TESTING
	
	col_ent_out->c1 = 255; col_ent_out->c2 = 255; col_ent_out->c3 = 255; col_ent_out->c4 = 255;	
	col_tab_out->SetColorEntry(1, col_ent_out);


// Set "nodata values" for software that use that
	
	poBand->SetNoDataValue(0);
		
// Write Color Table out to bitmap(tif)  file
			
	poBand->SetColorTable(col_tab_out);	
		




// Close input and output images

Exit:	printf("\n\t*Closing all image files and exiting program. \n");

	GDALClose(bmp_in);	
	GDALClose(bmp_out);


time (&rawtime);
timeinfo = localtime (&rawtime);
fprintf(stdout,"\n\n_______________________________\n");
fprintf(stdout,"\n %s (%s) finished at %s\n\n", PROG_NAME, VERSION,  asctime(timeinfo));



/***********************************************

// Check if running from ArcGIS (to delay running window disapperance)

//	if(ArcGIS)
	
//	  printf("\n#### You will loose detailed info about the run info if answered\n");  
//	  printf("\n Type anything to make detailed window disappear :   ");
//	  answer[0] = getc(stdin); 		// gets any answer or <CR>

	HMODULE moduleHandle,  moduleHandle2;

// Check if running from ArcGIS (to delay window disapperance for user to have time to read it)

//	if(ArcGIS)
	
    // moduleHandle = GetModuleHandle("ESRI.ArcGIS.Framework.dll");
	// moduleHandle = GetModuleHandle("ESRI.ArcGIS.DeskTop.AddIns.Factory.dll");
	
	 moduleHandle = GetModuleHandle("ArcMap.exe");	   // get ArcMap handle
	 moduleHandle2 = GetModuleHandle(NULL);				// get present program handle
	 
   // if (moduleHandle != NULL)	
	if (moduleHandle == moduleHandle2)
	{
	  printf("\n#### You will loose detailed info about this run when you answer ###\n");  
	  printf("\n After reading (or copying) it, type anything to make detailed window disappear :   ");
	  answer[0] = getc(stdin); 		// gets any answer or <CR>
	}
 
 
 
//wchar_t buffer[256] = {0};
//DWORD Olength = 0;

//GetUserObjectInformation(GetProcessWindowStation(), UOI_NAME, buffer, 256, &Olength);


char buffer[256] = {0};

GetUserObjectInformation(GetProcessWindowStation(), UOI_NAME, buffer, 256,0);

//if (!strncmp(buffer, "WinSta0",7)) {
if (strncmp(buffer, "WinSta0",7) == 0) {
  printf("\n Normal user session\n");
} else {
  printf("\n Service session \n");
  printf("\n Type anything to make detailed window disappear :   ");
  answer[0] = getc(stdin); 		// gets any answer or <CR>
}

printf("\n Process name : %s \n", &buffer);

//if ( _getpid() == getppid() ) printf("\n Running as a main process  \n");



//	 moduleHandle = GetModuleHandle("ArcMap.exe");
//	 moduleHandle = GetModuleHandle("ITCISOL.exe");
	 moduleHandle = GetModuleHandle(NULL);	 	// get present program handle
	 
	for (i=0; i< 256; i++) buffer[i] = 0 ;

//GetProcessImageFileNameA(GetProcessWindowStation(), buffer, 256);

GetProcessImageFileNameA(moduleHandle, buffer, 256);

printf("\n Process Image name : %s \n", &buffer);

 */


// Print program arguments
//	for (i=0; i<argc; i++) printf(" %d %s  \n  ",  i, argv[i]);
	
//if ( (argc == 4 ) && (strncmp("ArcGIS ",argv[3],3) == 0) )
	

// For people using this program via ArcGIS, give then some time to examine the results (before disappearing)

if ((strncmp("ArcGIS ",argv[argc-1],3) == 0) )		// if last argument is ArcGIS or ArcMap
	{
	fprintf(stdout,"\n\n######\n");
	printf("\n ArcGIS - Type anything to make this detailed window disappear and terminate %s ",PROG_NAME);
	answer[0] = getc(stdin); 		// gets any answer or <CR>
    }
	
	
exit(0);				// exit properly 

}				// END OF MAIN PROGRAM




/************************************************************************/

/* 		FUNCTIONS 						*/

/************************************************************************/

/*	Implements the isolation algorithm.
        (mostly using  iterate_over_image() found in "isolrules.c")	
        Controls the number of time we loop (passage at the image) in an effort
        to continue to improve crown delineation (as the delineation of some crowns
        makes, all of a suden, other easier to delineate the next time around).
        Also, controls when to switch to HIGHER level rules based on overall
        delineation progress made, or lack thereof, in previous pass at the full image.
*/

void isol(char * forest_type)
{


int		 i ,j;
float	tmp = 0.0;
char	chr[256];
int 	actval, numbuf[16];

/* calculate (or ask for) maxjump value (to bridge gaps in countour) */

upper_case(forest_type);
if (strncmp(forest_type,"MATURE",3) == 0)			/* 1 metre for mature */
	max_jump = (int) (1.0 / pix_size + 0.51 );
if (strncmp(forest_type,"REGEN",3) == 0)			/* 0.5 metre for regeneration */
	max_jump = (int) (0.5 / pix_size + 0.51 );
if (strncmp(forest_type,"TROPIC",3) == 0)			/* 2 metre for tropical trees  */
	max_jump = (int) (2.0 / pix_size + 0.51 );
if ( (strncmp(forest_type,"MAN",3) == 0) || (strncmp(forest_type,"PRO",3) == 0) ) /* manual mode */
	{	
	fprintf(stdout, "\nEnter maximum jump value (in pixels) : "); 
	if(fgets(chr,256,stdin))
		if(chr[0]) sscanf(chr,"%d",&max_jump);
	fprintf(stdout, "\n");
	strcpy(forest_type,"MATU_P\0");
	}
	
	
/*	
if (strncmp(forest_type,"PRE",3) == 0)		//preset mode
  {
  fprintf(stdout,"\nN.B.: The FORTYPE=PRESET mode is being used. Maximum jump factor is\n");
  fprintf(stdout,"\t being picked up from the ISOLJUMP EASI variable.\n");
  
 // actval = IMPGetNumeric("ISOLJUMP","I",numbuf,1);
  
  if (actval == -1) 
    {
    fprintf(stderr,"\n\n The FORTYPE=PRESET mode was selected, but the jump factor\n"); 
    fprintf(stderr,"could not be found in EASI parameter ISOLJUMP.\n\n");
    fprintf(stderr,"\tABORTING ....\n");
    exit(-1);   
    }
  if (actval >= 1) max_jump = numbuf[0];
  }
*/


if (max_jump <= 0) 
	{
	max_jump = 1;
	fprintf(stdout,"\n\t NOTE: Maximum jump was set by default (no value given) to 1.\n");
	}

if ( max_jump >= MAX_JUMP_MAX ) 
	{
	max_jump = MAX_JUMP_MAX;
	fprintf(stdout,"\n\t NOTE: Maximum jump is now at largest allowable value.\n");
	}
max_jump_org = max_jump;


fprintf(stdout,"\nMaximum jump value to be used initially (in pixels): %d \n", max_jump);
fprintf(stdout,"\tForest type is %6s \n", forest_type);



crowns.found = 0;
prev_num_found = 0;
	
	
/* 	
			Main loop.
	Loops several times (up to NUMLOOPS) over the image (bitmap) to find crowns 
	that become possible to delineate only after others have been found. 
	(Loop until only 0.1% improvement)
*/

fprintf(stdout,"\nDelineating tree crowns using rules ...\n");

wshed_size=10000;			/* storage for level 10 rules (nedeed in integer) */
wshed = (int *) malloc(sizeof(int)*wshed_size);	
memset(wshed,0,sizeof(int)*wshed_size);  	/* clear to zero before using */


for (loop_count = 1; loop_count <= NUMLOOPS; loop_count++) 
  {

/* reset crowns counters for each loop */

	crowns.encountered = 0;
	crowns.failed = 0;
	crowns.too_big = 0;
	crowns.too_small = 0;
	crowns.need_more_rules = 0;
	crowns.need_level_10 = 0;
	crowns.discrepancies = 0;
	crowns.postponed = 0;

/******************************************/

/* 
	Main process in loop - numerous passages over the image (up to NUMLOOPS)
  	 Scan (iterates and recurs) over the whole image "once" to find crowns 
*/

	iterate_over_image(loop_count);


/* report on finding during this pass  */

	pass_report();
	
	//fprintf(stdout,"\nWriting intermediate ISOL bitmap to file: %s  \n", file);
	//write_bitmap(good_crown_bmp);


	if (last_run) break;

/******************************************/

/* 	Get ready to do another passage over the image

      REMOVE TEMPORARY FILLING OF DIFFICULT AREAS (the so called bad crowns)
      (SO THAT THEY CAN BE REASSESSED DURING NEXT PASS THROUGH IMAGE)
*/

	for (i = BORDER; i < Lines-BORDER; i++)
	for (j = BORDER; j < Pixels-BORDER; j++) 
	  {
	  if (testbit(bad_crown_bmp, (int64) i*Pixels + j)) 
	    {
	    clearbit(bad_crown_bmp, (int64) i*Pixels + j);
	    clearbit(vfolbmp, (int64) i*Pixels + j);		/* reclear vfolbmp in order to revisit area next loop */
	    }
	  }


/******************************************/

/*	Controls when to switch on higher rule levels during the multiple
	passages over the image (VFOL bitmap). 
	NOTE: Numerous debugging modes and trial modes commented out.
*/

/******************************************/

/* If improvement is less than 1% (or 0.1%), no point re-running. 
   However, do one final run so that crowns (excentric, clusters?) previously labelled
   as "bad crowns" get finally finally filled (i.e., their crown area is useful for
   crown closure estimation and they often classify well (cause single species))
   Also, the resulting bitmap will be more estetically pleasing.
*/

	
/* If higher levels of rules (2 & 3 ) are not to be used (turned off by flag rule_group=1),  
   and, improvement is less than  0.1%
	then, its time for a final clean-up run */

/*
	if ( (rule_group == 1) && 
		((crowns.found - prev_num_found) <= (crowns.found / 1000)) ) 
	  {
	  last_run = 1;
	  printf("\nNOTE:\tDue to lack of crown delineation progress ( <0.1%% improvement) \n");  
	  printf("\tthe following pass will be the LAST PASS through the image.\n");
	  printf("\tTree crowns and clusters that could use further treatements will\n");
	  printf("\tbe PAINTED AS GOOD crowns \n\n");
	  }
*/


/* If "HIGHEST level of rules" is ON and thus was used for one passage over the image */

/*	DONT clean up (break now) and thus leave final bad crowns out of the ISOL bitmap 
	to easely see them in order to debug or for further improvement of ITCISOL */	
/*	if ( (rule_group == 3) ) break; */


/* If "HIGHEST level of rules" is already ON (i.e., has already made an attempt at cluster breakage)
    and improvement is less than 0.1%, no point re-running
	Have a last clean-up run to force bad crowns as good */	
	

	if ( (rule_group == 3) && 
		((crowns.found - prev_num_found) <= (crowns.found / 1000)) ) 
	  {
	  last_run = 1;
	  printf("\nNOTE:\tDue to lack of crown delineation progress ( < 0.1%% improvement) \n");
	  printf("\tAND that even existing higher level rules have been explored \n");
	  printf("\tthe next pass will be the LAST PASS through the image.\n");
	  printf("\tTree crowns and clusters that could use further treatements will\n");
	  printf("\tbe PAINTED AS GOOD CROWNS because we have exhausted our rules.  \n\n");
	  }


/* if improvement less than 0.1% with Level 2 runs, turn on third level of rules so that
	it may break up excentric crowns (big tree clusters)    */

/*	if ( (rule_group == 2) && ( max_jump >= MAX_JUMP_MAX ) &&
		((crowns.found - prev_num_found) <= (crowns.found / 1000)) ) 
	  {
	  rule_group = 3;
	  fprintf(stdout,"\n-------------------------\n");
	  fprintf(stdout,"\n### NOTE: THIRD LEVEL OF RULES WAS TURNED ON ...\n");
	  fprintf(stdout,"\t... Level 10 rule was turned on.\n\n");
	  }
*/

/* if improvement less than 0.1% with Level 2 runs, increase maxjump again  
	Increase maxjump is the essence of Level 2 runs and  *** NOT THAT GREAT ***  */

/*
	if ( (rule_group == 2) && 
		((crowns.found - prev_num_found) <= (crowns.found / 1000)) ) 
	  {
	  fprintf(stdout,"\n-------------------------\n");
	  fprintf(stdout,"\n### NOTE: SECOND LEVEL CONTINUES, BUT ...\n");
	  max_jump = max_jump + max_jump_org;
	  if ( max_jump > MAX_JUMP_MAX ) max_jump = MAX_JUMP_MAX;
	  fprintf(stdout,"\t... max_jump now at: %d pixels.\n\n",max_jump);
	  }
*/

/*	For regeneration, mature and tropical forests, ...
	if the delineation improvement is less than 0.1% and FORTYPE= MAT3 || REG3 || TRO3)
	turn on higher (3rd) level of rules, so that you may possibly break up a few 
	left over ISOL (typically big crown clusters)
	IF NOT THE CASE (i.e., no MAT3 || REG3 || TRO3)
	turn on "last run flag" that will label left-over crowns and clusters as good crowns */

	if ( (rule_group == 1) &&
		((crowns.found - prev_num_found) <= (crowns.found / 1000)) )
	{
	if ( (strncmp(forest_type,"MAT3",4) == 0) || 
		(strncmp(forest_type,"REG3",4) == 0) ||
				(strncmp(forest_type,"TRO3",4) == 0) )
	  {
	  rule_group = 3;
	  fprintf(stdout,"\n-------------------------\n");
	  fprintf(stdout,"\n Progress is stagnating, so ...\n");
	  fprintf(stdout,"### HIGHER LEVEL RULES (THIRD GROUP) WERE TURNED ON ...\n");
	  fprintf(stdout,"\t... (Rule 10 (a binary watershed approach) was turned on)\n\n");
	  }
	else	
	  {
	  last_run = 1;
	  printf("\nNOTE:\tDue to lack of crown delineation progress ( <0.1%% improvement) \n");  
	  printf("\tthe following pass will be the LAST PASS through the image.\n");
	  printf("\tTree crowns and clusters that could use further treatements will\n");
	  printf("\tbe PAINTED AS GOOD crown (i.e., useful for crown closure assesments)\n\n");
	  }
	}

/* if improvement less than 0.1% with Level 1 runs, turn on Second level of rules */

/*
	if ( (rule_group == 1) &&
		((crowns.found - prev_num_found) <= (crowns.found / 1000)) )
	  {
	  rule_group = 2;
	  fprintf(stdout,"\n-------------------------\n");
	  fprintf(stdout,"\n### NOTE: SECOND LEVEL TURNED ON ...\n");
	  fprintf(stdout,"\t... max_jump will be gradually increased ...\n\n");
	  max_jump = max_jump + max_jump_org;
	  if ( max_jump > MAX_JUMP_MAX) max_jump = MAX_JUMP_MAX;
	  fprintf(stdout,"\t... max_jump now at: %d pixels.\n\n",max_jump);
	  }
*/


/* prep for next loop */

	prev_num_found = crowns.found;

  }	/* end of looping for passes through the image */


/* Finished looping through the whole image */

fprintf(stdout,"\n *** ITC delineation finished *** \n");

	free(wshed);

}	// end of function isol()


/*****************************************************/

/*
*   REPORT OF FINDINGS FOR FULL IMAGE
*/


void pass_report()
{
//IMPTime(timedate,4); 

time (&rawtime);  timeinfo = localtime (&rawtime);
fprintf(stdout,"\n---------------------------\n");
fprintf(stdout,"At %s \n", asctime(timeinfo));
fprintf(stdout,"\n### For pass number %d through the image:\n\n", loop_count);

fprintf(stdout,"Number of potential crowns encountered: %d\n",crowns.encountered);
fprintf(stdout,"Number of crowns well delineated: %d\n", crowns.found-prev_num_found);
if(last_run)fprintf(stdout,"N.B.: LAST PASS, crown clusters were FORCED well delineated as crowns.\n");
fprintf(stdout,"Number of failure to delineate (- # below): %d\n", crowns.failed);
fprintf(stdout,"Number of postponed delineation: %d\n\n", crowns.postponed);

fprintf(stdout,"Number of crowns that are too big: %d\n", crowns.too_big);
fprintf(stdout,"Number of crowns too small (<max_jump): %d\n", crowns.too_small);
fprintf(stdout,"Number of size discrepancies: %d\n", crowns.discrepancies);
fprintf(stdout,"Number of needs for additional rules: %d\n", crowns.need_more_rules);
fprintf(stdout,"Number of excentric crowns (needing rule 10): %d\n\n", crowns.need_level_10);

fprintf(stdout,"\nNumber of crowns well delineated (so far): %d\n\n", crowns.found);
}

/*****************************************************/

/*	 This function puts a border of VF matter around VFOL bitmap
	 in order for the ISOL algo. not to stray out of the image
*/


void prep_bmp(PixVal * bmp, int xsize, int ysize)
{
int i, j;

fprintf(stdout, "\nPreparing VFOL bitmap...\n");

for (i = 0; i < BORDER; i++)
  for (j = 0; j < xsize; j++) 
	  setbit(bmp, (int64) i*xsize + j);

for (i = ysize - BORDER; i < ysize; i++) 
  for (j = 0; j < xsize; j++) 
	  setbit(bmp, (int64) i*xsize + j);

for (i = 0; i < ysize ; i++) 
	{
	for (j = 0; j < BORDER; j++) setbit(bmp, (int64) i*xsize + j);
	for (j = xsize - BORDER; j < xsize; j++) setbit(bmp, (int64) i*xsize + j);
	}
/*
fprintf(stdout,"\n Last memory position of bitmap %I64d\n", (int64) ysize*xsize);
fprintf(stdout,"\n OR Last memory position of bitmap %I64d\n",  ysize*(int64)xsize);
*/

}

/**********************************************************************/


/* This function is a recursive fill routine for a crown in a bitmap.
   It is destructive of the crown in that bitmap (i.e.: clears bit).
   X goes in pixel direction. Y goes in line direction.   */

void fill_clears(int x, int y)

{
	clearbit(tempbitbuffer, (int64)y*Pixels + x);

	if ( testbit(tempbitbuffer, (int64)(y+1)*Pixels + x) )
		{
		fill_clears(x,y+1);
		}
	if ( testbit(tempbitbuffer, (int64)y*Pixels + x-1) )
		{
		fill_clears(x-1,y);
		}
	if ( testbit(tempbitbuffer, (int64)(y-1)*Pixels + x) )
		{
		fill_clears(x,y-1);
		}
	if (  testbit(tempbitbuffer, (int64)y*Pixels + x+1) )
		{
		fill_clears(x+1,y);
		}
}

/**********************************************************************/


/* This function converts a string to upper case characters */

void upper_case(char *c)
{
	while (*c != '\0') {
		if (*c >= 'a' && *c <= 'z')
			*c += ('A' - 'a');
		c++;
	}
}


/***************************************************************************************/
/*******************************************************/

/* This function creates a safety zone around a bitmap for fills() not
   to get out of image array and crash the program */

/*   
void safety_zone(unsigned char *bitmap)
{
  extern int Lines, Pixels, Channels;
  int i,j;

  for (j=1, i=1 ; i<=(Lines) ; i++ )	  clearbit(bitmap,((i-1)*(int64)Pixels + j-1) );
  for (j=Pixels, i=1 ; i<=(Lines) ; i++ ) clearbit(bitmap,((i-1)*(int64)Pixels + j-1) );
  for (i=1, j=1 ; j<=(Pixels) ; j++ )	  clearbit(bitmap,((i-1)*(int64)Pixels + j-1) );
  for (i=Lines, j=1 ; j<=(Pixels) ; j++ ) clearbit(bitmap,((i-1)*(int64)Pixels + j-1) );
}
*/

