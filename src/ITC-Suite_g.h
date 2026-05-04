/*
	ITC-Suite_g.h
	Francois Gougeon
	April 2021

	Declares useful Suite wide variable and definitions
	to be used by all program and be independent of PCI.h
*/

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>     // malloc, free, rand
#include <string.h>
#include <math.h>
#include <limits.h>
#include <time.h>       // time_t, struct tm, time, localtime
#include "gdal_priv.h"		// For GDAL library
#include "ogrsf_frmts.h"	// For OGR library (vector related)

		// Microsoft Visual C++
/*
typedef unsigned char  PixVal; 
typedef signed __int64 int64;
typedef signed __int32 int32;
typedef unsigned __int32 uint32;
typedef unsigned long  uint64;
typedef unsigned short uint16;

 */

typedef unsigned char  PixVal; 

//typedef signed __int64 int64;		// Microsoft Visual C++
typedef  int64_t int64;
//typedef signed __int32 int32;		// Microsoft Visual C++
typedef  int32_t int32;

typedef uint32_t uint32;
typedef uint64_t  uint64;

typedef uint16_t uint16;


#define CHN_8U            1
#define CHN_16S           2
#define CHN_16U           3
#define CHN_32U           4
#define CHN_32R           5

#define SEG_BIT		101
#define SEG_VEC		116

//#define EQUALN(a,b,n) (strncasecmp(a,b,n)==0)
// now defined in C:\gdal_v3.10\include\cpl_port.h


#define TRUE 1
#define FALSE 0

