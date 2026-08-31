
/* 
Program name :		Gdal_test_vector.cpp

	Francois A. Gougeon
	October 2016, June 2017

	Program to test GDAL vector layer access and print some info to screen
	AND possibly create a small output shp file (as a test) with a subset of the original polygon layer


*** PROGRAM USAGE

> Gdal_Test_vector test.shp

> Gdal_Test_vector Roads_12_Sub.pix layer#

> Gdal_Test_vector temp2.shp 1 sub_set.shp

> gdal_test_vector ..\Full_Image\OysterR_CWD_7.pix 6


Version 1.2a	Aug. 2020	Francois Gougeon

	- To create an output shp file (as test) with a subset of the original polygon layer
	
Version 1.3a	Dec. 2021	Francois Gougeon

	- Fixed issues with accessing SHP file layers (and PCI vector layers)
	
	- Also tested accessing shp layer within a directory that was passed as 2nd argument	
	
	
Version 1.4a	July 2024	Francois Gougeon	


Version 1.5a	July 2026	Francois Gougeon	
		
		- Issues with shp file access  with GDAL v3.11 (all OK on v3.10)
			N.B.: Other programs not using shp file have no problem
		
		- Trying to migrate to GDAL v3.14 -- issue with driver perse and shp file layers
			N.B.: Other programs not using shp file have no problem IF not asking driver info

*******************************************************************************

*** To compile with Visual Studio (VS14)  (see VS14_GDAL_compile.txt)

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64

set INCLUDE=C:\gdal-2.1.1_v2\include;C:\libtiff-4.0.6_64b\tiff-4.0.6\libtiff;%INCLUDE%
set LIB=C:\gdal-2.1.1_v2\lib;C:\libtiff-4.0.6_64b\tiff-4.0.6\libtiff;%LIB%

set LINK=gdal_i.lib  libtiff_i.lib User32.lib

set CL= /MD

CL xxxxxxxx.cpp /EHsc


**** Ackowlegment to GDAL:

GDAL - Geospatial Data Abstraction Library: Version 2.1.1 (July2016, 64bit), 
Open Source Geospatial Foundation, 
Thanks Frank (Warmerdam)

********************************************************************************
*/

#define VERSION "v1.5a"
#define PROG_NAME "Gdal_test_vector"


#define Debug  1    // to skip unfinished sections

typedef unsigned char  PixVal;

#include <stddef.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <time.h>       	// time_t, struct tm, time, localtime 
#include <iostream>
#include <vector>
#include <errno.h>

#include "gdal.h"		// For GDAL library
#include "gdal_priv.h"		// For GDAL library

#include "ogrsf_frmts.h"	// For OGR
#include "ogr_spatialref.h"
#include "ogr_geometry.h"

#include "ogr_feature.h"	// Necessary ??
#include "ogr_core.h"		// Necessary ??
#include "ogr_api.h"		// Necessary ??
#include "cpl_conv.h"		// Necessary ??

//#include "ogr_vector_cpp.h"		// apparently needed for GDAL v3.12+

#include <exception>		// for try and catch
using namespace std;

#include "ITC-Suite_g.h"

//extern int errno;

int	Pixels, Lines, Channels;
time_t rawtime;
struct tm * timeinfo;

char 	Proj[80];


// Data structures for polygons


//data structure for points

	typedef struct MyPoint2D
	{
	double dX;
	double dY;
	} MyPoint2D;

//data structure for lines

	typedef struct MyLine2D
	{
	std::vector<MyPoint2D>LineString;
	} MyLine2D;

//data structure for a line feature

	typedef struct LineFeature
	{
	std::vector<MyLine2D>LinesOfFeature;
	} LineFeature;

//Holds Coordinates of Line Shapefile

	std::vector<LineFeature> LineLayer;


//data structure for rings

	typedef struct MyRing2D
	{
	std::vector<MyPoint2D>RingString;
	bool IsClockwised;
	} MyRing2D;


//data structure for polygons

	typedef struct MyPolygon2D
	{
	std::vector<MyRing2D> Polygon;
	} MyPolygon2D;

//data structure for a polygon feature

	typedef struct PolygonFeature
	{
	std::vector<MyPolygon2D>PolygonsOfFeature;
	}PolygonFeature;

//Holds Coordinates of Polygon Shapefile

	std::vector<PolygonFeature> PolygonLayer;

GDALDataset	*piDS, *poDS;
OGRLayer  	*piLayer, *poLayer;
OGRFeature 	*piFeature, *poFeature;
OGRFeatureDefn	*piFDefn, *poFDefn;
OGRFieldDefn	*piFieldDefn, *poFieldDefn;
OGRSpatialReference *piSRSIn,  *poSRSIn;
OGRGeometry 	*piGeometry, *poGeometry;


int NumberOfInnerRings; 
int NumberOfExteriorRingVertices;


//************************************************************************

int main(int argc, const char * argv[])
{
	
int 	iFeat, iField, feat_count, field_count, last_feat, last_field;
int 	layer_toget=0;
int 	layer_count=0;

int		oFeat, oField;
char 	*piFieldName;
char 	**papszOptions;

char	description[80];

// Register all OGR drivers
    OGRRegisterAll();

// Print Program Header and time 

	time (&rawtime);
	timeinfo = localtime (&rawtime);
	fprintf(stdout,"\n\n\t\tStarting %s (%s) at %s\n", PROG_NAME, VERSION, asctime(timeinfo));

// Check input parameters (i.e., argv[*])

	if (argv[1]== NULL) 
	  {printf("\n\n PROBLEM with input PCI or shape file %s \n",argv[1]);
	  printf("\tHave an INPUT PCI or shape file as first argument on command line \n\n");
	  exit(-1);}  

	//if (argv[2]== NULL) 
	// if (argc == 2) 		// NOT 3
	  // printf("\n** No particular layer mentionned as 2nd argument. Displaying first layer.  \n");

	if (argc > 4 ) 		
	  {printf("\n\n** Too many parameters on command line. *EXISTING*  \n"); exit(-1); }


/*
// Mini TEST of command line arguments

   int counter;
    printf("\nProgram Name Is: %s",argv[0]);
    if(argc == 1)
        printf("\nNo Extra Command Line Argument Passed Other Than Program Name");
    if(argc >= 2)
    {
        printf("\nNumber Of Arguments Passed: %d",argc);
        printf("\n----Following Are The Command Line Arguments Passed----");
        for(counter=0; counter<argc; counter++)
            printf("\nargv[%d]: %s",counter,argv[counter]);
    }

*/

//*******************************************************************

// Register drivers for all formats and OPEN file

	GDALAllRegister();
		
    const char *pszDriverName = "ESRI Shapefile";	
    GDALDriver *piDriver;

// Force ESRI driver (for debugging)
	
	// printf( "\n\t*** Will use as Input driver << %s >> \n", pszDriverName );

    // piDriver = GetGDALDriverManager()->GetDriverByName(pszDriverName );
    // if( piDriver == NULL )
    // {
        // printf( "Input driver %s not available.\n", pszDriverName );
        // exit( 1 );
    // }
	

// 	Open shape file  (should pickup proper driver on its own based on ".shp"


    const char* pszShapefile = argv[1];
	
	//piDS = (GDALDataset *) GDALOpenEx( argv[1], GDAL_OF_VECTOR, NULL, NULL, NULL );
	//piDS = (GDALDataset *) GDALOpen( argv[1],  GA_ReadOnly );		//legacy method
	//piDS = (GDALDataset *) GDALOpenEx( argv[1], GA_ReadOnly, NULL, NULL, NULL );
	
	piDS = (GDALDataset *) GDALOpenEx( pszShapefile, GDAL_OF_VECTOR | GDAL_OF_READONLY, NULL, NULL, NULL );
	
	
	if( piDS == NULL )
		{fprintf(stderr, "\nFailed to open input file. Error : %s \n", strerror(errno)); exit( -1 );}

	fprintf(stdout,"\n\t**File '%s' was opened for reading\n\n",argv[1]);

	// goto TEST1;


// Print generic info (driver used, ... )		### often does not work for GDAL > 3.11
// Reports  --- ERROR 6: PRF_ForInv_4x4km.shp: Dataset does not support the AddBand() method.

	// printf("\t Driver info\n");
	
	// printf( "\tDriver: %s/%s\n",
          // piDS->GetDriver()->GetDescription(),
          // piDS->GetDriver()->GetMetadataItem( GDAL_DMD_LONGNAME ) );

// Print generic info (driver used, ... )		### does not work for GDAL > 3.10
// Reports  --- ERROR 6: PRF_ForInv_4x4km.shp: Dataset does not support the AddBand() method.

	
//		exit(-1);			// ERROR 6 ...



// Open layer pointed to by user via argv[2] OR the "only" layer

	layer_count = piDS->GetLayerCount();					// C++ approach  ** NOGO *** says 3 layers
	//layer_count = GDALDatasetGetLayerCount(piDS);			// C handler  approach -- says 1 layer	
	printf("\n\t**Number of layers in vector file is %d \n", layer_count);
		
	   if(argc == 2) {layer_toget = 0;} 					// nothing entered, get first layer 
		else {layer_toget = (atoi(argv[2]) - 1) ;}			// Layer numbers start at zero

	if(layer_toget+1 > layer_count) 
	{printf("\tNo such layer %d \n", layer_toget + 1); exit(-1);}


	//layer_toget = 4;		// For testing  -- does not trigger an other error
		
	printf("\tAccessing layer %d \n", layer_toget + 1);	
	
	piLayer = piDS->GetLayer(layer_toget);				// layer number start at zero	

			
	// OGRLayerH piLayer =  GDALDatasetGetLayer( piDS, layer_toget);	
	// piLayer = (OGRLayerH *) GDALDatasetGetLayer( piDS, layer_toget);
	// piLayer  = piDS->GetLayer(1);	
	
	if( piLayer == NULL )
		{fprintf(stderr, "Failed to access layer %d - %s \n", layer_toget, strerror(errno)); exit( -1 );}	

	printf("\tGot access to layer %d \n\n", layer_toget + 1);
	
	//		exit(-1);	// above can also reports ERROR 6 ...

//	printf("\n\tPointers before the call: %p  %p \n", piDS, piLayer);

/*

// TEST - Trying to access shp layer (within the directory) that was passed as 2nd argument 

	printf("\n\tTrying to access layer by name \"%s\" \n\n", "test2");
	
    piLayer = piDS->GetLayerByName("test2");

	if( piLayer == NULL )
		{fprintf(stderr, "Failed to access layer by name %s \n", strerror(errno)); exit( -1 );}
 
	printf("\n\tGot access to layer by name \"%s\" \n\n", "test2");
*/




// Not really needed here, but a good practice

	// fprintf(stdout,"Reseting Reading on  layer %d \n\n", layer_toget+1);	
	// piLayer->ResetReading();

	//printf("\n\tGOT HERE - 1  \n\n"); 

 
	//printf("\n\tPointers before the call: %p  %p\n", piDS, piLayer);
	

// Number of features (i.e., polygons) in that layer and number of fields in that layer
	
	// try {
		// piFDefn = piLayer->GetLayerDefn();			// ### trouble ###
	// }
	// catch(exception& e) {
		// printf("\nError happened here!\n");
	// }
 
 TEST1:		//   ### Below does not work for GDAL > 3.10
 
	printf("\tLayer definition for layer %d \n", layer_toget + 1);

	
	piFDefn =  piLayer->GetLayerDefn();			// ### trouble for GDAL > v3.10  ###	
    if (piFDefn == nullptr) 
		{printf("\tCant get layer definition for layer %d \n", layer_toget + 1); exit(-1);}
		
	printf("\tLayer Name: %s\n", piLayer->GetName());
    printf("\tGeometry Type: %s\n", OGRGeometryTypeToName(piFDefn->GetGeomType()));
    printf("\tField Count: %d\n", piFDefn->GetFieldCount());

	
			
	//exit(-1);		// for debugging 
			
			
	
	//piFDefn =  (OGRFeatureDefn *) OGR_L_GetLayerDefn(piLayer);	//trying C version rather than C++ 

	//printf("\n\tPointers after the call to GetLayerDefn() : %p  %p %p\n", piDS, piLayer, piFDefn);
	
	if( piFDefn == NULL )
		{printf( "Failed to access layer definition - %s \n", strerror(errno)); exit( -1 );}

	//printf("\n\t Was able to access layer definition\n\n"); 



//	feat_count = piLayer->GetFeatureCount();
	
	feat_count = OGR_L_GetFeatureCount(piLayer,0);		//trying C version rather than C++
	
	
	if( feat_count == NULL )
		{printf( "Failed to access layer FeatureCount - %s \n", strerror(errno)); exit( -1 );}

	//printf("\n\t*Layer %d of current file has %d features\n\n", layer_toget+1, feat_count);
				
	//printf("\n\tGOT HERE - 2\n\n"); //exit(-1);			// for faster debugging
	
	field_count = piFDefn->GetFieldCount();
	if( field_count == NULL )
		{printf( "Failed to access layer FieldCount - %s \n", strerror(errno)); exit( -1 );}

	printf("\nINFO:\t Layer %d of current file has %d features with %d fields each\n\n", 
				layer_toget+1, feat_count, field_count);


	//printf("\n\tGOT HERE - 3\n\n");    exit(-1);			// for faster debugging
	
	
	
//************************************************************************

// 			Getting polygon vertices



   PolygonFeature Polygon, poPolygon;
   OGRPoint ptTemp, ptTemp2, ptTemp3, ptTemp4;
   
//  	goto FeatureOnly;			// bypass reading polygon vertices
   

// Got through all of the features (here MAX =5)

//	printf("\nLooping through all the features ...\n\n");

	last_feat = feat_count;
	if(last_feat > 5) last_feat = 2 ;
	printf("\n\tLooping through %d  features for polygon vertices ...\n\n", last_feat);

	//piLayer->ResetReading();			// VERY IMPORTANT (if you use GetNextFeature() 
	


//while( (piFeature = piLayer->GetNextFeature()) != NULL )
	
  for (iFeat=0; iFeat<last_feat; iFeat++)
 // for (iFeat=2; iFeat<6; iFeat++)
	{
	// ??? last 3 lines all worked  ???
		
	  //piFeature = piLayer->GetNextFeature();		// NOGO with GDALv3.11
	 
	  //piFeature =  (OGRFeature *) OGR_L_GetNextFeature(piLayer);	// C version is better (NOGO with GDALv3.11)

	  piFeature =  (OGRFeature *) OGR_L_GetFeature(piLayer,iFeat);  // C version is better ** YES **
	  
	  //piFeature = piLayer->GetFeature(iFeat);  // works specifically with "iFeat"
	  
	  
	  OGRGeometry * piGeometry = piFeature->GetGeometryRef();
	
	  printf("\n ** Feature %d : Geometry is : %d \n", iFeat, wkbFlatten(piGeometry->getGeometryType()));




	  if ( piGeometry != NULL && wkbFlatten(piGeometry->getGeometryType()) == wkbPolygon )

	    {
	    OGRPolygon *piPolygon = ( OGRPolygon * )piGeometry;

	    //Polygon.PolygonsOfFeature.resize(1);
		
		// Number of innner rings
	    NumberOfInnerRings = piPolygon->getNumInteriorRings();
			
	    printf("Number of inner rings = %d \n", NumberOfInnerRings);
		
	
	
	
	
	// Below is same as GDAL demo and is NOGO for v3.11 (OK for v3.10)
	
       OGRLinearRing * piExteriorRing = piPolygon->getExteriorRing();

       //Polygon.PolygonsOfFeature.at(0).Polygon.resize(NumberOfInnerRings+1);
			// ABOVE IS AN ISSUE
			
       //Polygon.PolygonsOfFeature.at(0).Polygon.at(0).IsClockwised = piExteriorRing->isClockwise();
			// ABOVE IS AN ISSUE
 
 
		
		NumberOfExteriorRingVertices = piExteriorRing->getNumPoints();
		
			// ABOVE IS AN ISSUE	


		//NumberOfExteriorRingVertices = 10;		// ##### TEST that WORKS ####
		
		
	    printf("NumberOfExteriorRingVertices = %d \n", NumberOfExteriorRingVertices);




// Get those vertices

            // Polygon.PolygonsOfFeature.at(0).Polygon.at(0).RingString.resize(NumberOfExteriorRingVertices);

            // for ( int k = 0; k < NumberOfExteriorRingVertices; k++ )
             // {
               // piExteriorRing->getPoint(k,&ptTemp);
               // MyPoint2D pt;
               // pt.dX = ptTemp.getX();
               // pt.dY = ptTemp.getY();
               // Polygon.PolygonsOfFeature.at(0).Polygon.at(0).RingString.at(k) = pt;
             // }
			 
			 
			 
			 

/* 
         for ( int h = 1; h <= NumberOfInnerRings; h++ )
           {
               OGRLinearRing *piInteriorRing = piPolygon ->getInteriorRing(h-1);

               Polygon.PolygonsOfFeature.at(0).Polygon.at(h).IsClockwised = piInteriorRing->isClockwise();
               int NumberOfInteriorRingVertices = piInteriorRing->getNumPoints();
	       //printf("NumberOfInteriorRingVertices = %d \n", NumberOfInteriorRingVertices);

               Polygon.PolygonsOfFeature.at(0).Polygon.at(h).RingString.resize(NumberOfInteriorRingVertices);

		   for ( int k = 0; k < NumberOfInteriorRingVertices; k++ )
		       {
			   piInteriorRing ->getPoint(k,&ptTemp);
			   MyPoint2D pt;
			   pt.dX = ptTemp.getX();
			   pt.dY = ptTemp.getY();
			   Polygon.PolygonsOfFeature.at(0).Polygon.at(h).RingString.at(k) = pt;
		       }
           } 
*/


              PolygonLayer.push_back(Polygon);

	
		
		}





// ######   NEED to convert FAKE POLYGON  to real polygon here 


		
	  if ( piGeometry != NULL && wkbFlatten(piGeometry->getGeometryType()) == wkbLineString )
		{
		//printf("Program does not deal with Pseudo-Polygon (LINESTRING, ala PCI) yet - So, Exiting\n\n");
		//exit(-1);	

		printf("\n ** GOT HERE because Feature %d : Geometry is LINESTRING(2) \n", iFeat );

		OGRLineString * piLineString = ( OGRLineString * ) piGeometry;
		OGRLineString * poLineString = ( OGRLineString * ) piGeometry;
   		OGRPolygon * poPolygon = ( OGRPolygon * ) OGR_G_CreateGeometry(wkbPolygon);

		//OGR_G_ForceToPolygon(poPolygon);

	    //poPolygon.PolygonsOfFeature.resize(1);

	    int NumberOfPoints = piLineString->getNumPoints();

	    printf("Line String Number of points = %d \n\n", NumberOfPoints);
		

		
// ####		

	    //Polygon.PolygonsOfFeature.resize(1);		
		
	      //Polygon.PolygonsOfFeature.at(0).Polygon.at(0).RingString.resize(NumberOfPoints);

	
            //for ( int k = 0; k < NumberOfPoints; k++ )
			for ( int k = 0; k < 5; k++ )
            {
               piLineString->getPoint(k,&ptTemp);

               MyPoint2D pt;
               pt.dX = ptTemp.getX();
               pt.dY = ptTemp.getY();
			   if(k<5) printf("LineIn Point %d : %.2f %.2f \n",k, pt.dX, pt.dY);

				poLineString->setPoint(k,&ptTemp); 

				poLineString->getPoint(k,&ptTemp2);
 				if(k<5) printf("* LineOut Point %d : %.2f %.2f \n", k, ptTemp2.getX(), ptTemp2.getY() );

            //    poPolygon.PolygonsOfFeature.at(0).Polygon.at(0).RingString.at(k) = pt;

            }

			for ( int k = 0; k < 5; k++ )
			{
				poLineString->getPoint(k,&ptTemp3);
 				if(k<5) printf("* AGAIN LineOut Point %d : %.2f %.2f \n", k, ptTemp3.getX(), ptTemp3.getY() );
			}
			
			//OGR_G_CloseRings(poLineString);   // said not to work 

  			poPolygon =  ( OGRPolygon * ) OGR_G_ForceToPolygon(poLineString); // after that poLineString not availbale anymore
			OGR_G_CloseRings(poPolygon);
		



// 	Double checking

/* 

		printf("\n ##### Double Checking output polygon points ... \n");

			for ( int kk = 0; kk < 5; kk++ )
             {
			   piLineString->getPoint(kk,&ptTemp4); 		///Double checking input LineString points 	-- WORKS
               //poLineString->getPoint(kk,&ptTemp4);	//Double checking output LineString points		-- WORKS 
			  //poPolygon->getPoint(kk,&ptTemp4);		///Double checking output Polygon  points 

				if(kk<5) printf("* Repeat Point %d : %.2f %.2f \n", kk, ptTemp4.getX(), ptTemp4.getY() );
			 }
 */

		printf("** Done FAKE polygone **\n");	

       }		// end of section for FAKE polygon (LineString)
			 
	
		

	}	// End of FEATURE LOOP  




//goto Exit;			// ### for testing ####



// Get polygon spatial reference		** NO GO **

//	strncpy(Proj,piLayer->GetProjectionRef( ),20);
//	if( piLayer->GetProjectionReF( ) != NULL)  printf( "Projection is '%s'\n\n", Proj);


	// printf("\n\tGOT HERE - 4\n\n");    exit(-1);			// for faster debugging
	




//************************************************************************
//************************************************************************
//************************************************************************

FeatureOnly:


// Print some feature FIELDS (#### working well)

	last_feat = feat_count;
	if(last_feat > 5) last_feat = 2 ;
	
	piLayer->ResetReading();	// VERY IMPORTANT (if you use GetNextFeature() is NOGO
	
	printf("\n\n\t\tLooping through some(%d) features for FIELD INFO  ...\n\n", last_feat);

	
	for (iFeat=0; iFeat<last_feat; iFeat++)

	  {
	  //piFeature = piLayer->GetNextFeature();		// NOGO on v3.11
	  
	  //piFeature =  (OGRFeature *) OGR_L_GetNextFeature(piLayer);	// NOGO on v3.11
	  
// Asking for aspecific feature works	  
	  
	  piFeature =  (OGRFeature *) OGR_L_GetFeature(piLayer,iFeat);  // C version is better ** YES **
	  

//	Field name as a single line
/*
	for( iField = 0; iField < 5 ; iField++ )
	  {
	  piFieldDefn = piFDefn->GetFieldDefn( iField ); 
	  printf("%s,",piFieldDefn->GetNameRef( ) );
	  }
*/

//	  printf("\nData in the fields of feature %d \n", iFeat);
//	  for( iField = 0; iField < piFDefn->GetFieldCount(); iField++ )


// Print some field names and their data


	last_field = field_count;
	if(last_field > 5) last_field = 5 ;
	
	  printf("\nSome(%d) fields and their data for feature %d \n", last_field, iFeat);

	  
	  for( iField = 0; iField < last_field; iField++ )
		{
		piFieldDefn = piFDefn->GetFieldDefn( iField );  // get next field definition

		printf("\t\t %s : ", piFieldDefn->GetNameRef( ) );	 	//field name

		if( piFieldDefn->GetType() == OFTInteger )
        	  printf( "%d \n", piFeature->GetFieldAsInteger( iField ) );
		else if( piFieldDefn->GetType() == OFTInteger64 )
        	  printf(  "%I64d \n", piFeature->GetFieldAsInteger64( iField ) );
		else if( piFieldDefn->GetType() == OFTReal )
        	  printf( "%.3f \n", piFeature->GetFieldAsDouble(iField) );
		else if( piFieldDefn->GetType() == OFTString )
       		  printf( "%s \n", piFeature->GetFieldAsString(iField) );
		else
        	  printf( "%s ", piFeature->GetFieldAsString(iField) );
		  
		}

	  printf("\n");
	  
	  OGRFeature::DestroyFeature( piFeature );

	  } 			//  end of while (in this case for loop of 5)



	 //printf("\n\tGOT HERE - 5\n\n");    exit(-1);			// for faster debugging
	


//***************************************************************************************
//***************************************************************************************

// Creating  an output SHP  file  Following exemple from: http://www.gdal.org/ogr_apitut.html

//*******************************************************************************************
//***************************************************************************************


// If no name for output file, skip output file creation and copy process
//  (i.e., only display of input file fields was needed)

if ( (argc < 4) || (argv[3]== NULL) ) goto Exit;


	printf("\n\n*****************************************\n");
	printf( "\n\t**Creating a subset of the specified SHP file in \"%s\"  \n", argv[3]);

// Get the driver for "ESRI Shapefile"

	const char *outDriverName = "ESRI Shapefile";
	
	GDALDriver *poDriver;

	poDriver = GetGDALDriverManager()->GetDriverByName(outDriverName );

	if( poDriver == NULL )
	{
          printf( "%s driver not available.\n", outDriverName );
          exit( 1 );
	}


// Create the output layer AND copy fields definitions AND copy features ### NOT THERE ###

/*
	 if( poLayer->CopyLayer(piLayer, "test_out.shp", NULL ) != OGRERR_NONE )
	   {
 	   printf( "Copying layer 1 to file %s failed.\n" , argv[3] );
	   exit( 1 );
	   }

	 printf("\n\nCopying layer 1 to file %s .\n" , argv[3] );
	 
	 goto Exit;
*/	


// CREATE the output SHP file based on third argument (if present)

	poDS = poDriver->Create( argv[3], 0, 0, 0, GDT_Unknown, NULL );

	if( poDS == NULL )
	{
	    printf( "Creation of output file failed.\n" );
	    exit( 1 );
	}
	printf( "\n** Created output SHP file : %s \n", argv[3]);

// Create the output layer in the shape file

	poLayer = poDS->CreateLayer( argv[3], NULL, wkbPolygon, NULL );

	if( poLayer == NULL )
	{
	  printf( "Layer creation failed.\n" );
	  exit( 1 );
	}
	printf( "\n\n\tCreated a polygon layer in : %s  \n", argv[3]);


// Copy ???? GeoTransform, projection, ...

//	printf("\n\t*Writing geographic projection for output file %s\n", argv[3]);	

//	poDS->SetSpatialRef(piDS->GetSpatialRef());	
//	poLayer->SetSpatialRef( piLayer->GetSpatialRef );


//		 printf("\n\tGOT HERE - 5\n\n");    exit(-1);			// for faster debugging
	

	

// Create seven fields (attributes) for that output layer


printf("\n\nCreating up to seven(7) fields from polygon layer %d from file \"%s\"  \n\n", layer_toget+1, argv[1]);
 
	last_field = field_count;
	if(last_field > 7) last_field = 7 ;
	
	for( oField = 0; oField < last_field; oField++ )		// field counter
	{
	poFieldDefn = piFDefn->GetFieldDefn( oField ); 		// from input fields


	  if( poLayer->CreateField(poFieldDefn, TRUE ) != OGRERR_NONE )
	    {
 	    printf( "*** Creating field %hS FAILED.\n" , poFieldDefn->GetNameRef() );
	    exit( 1 );
	    }

	//printf( "Creating field %d : %hS \n" , oField, poFieldDefn->GetNameRef() );

	}

// Double checking 

  printf("\n\tDouble checking seven(7) fields in output shape file \n\n");

  poFDefn = poLayer->GetLayerDefn();

  for (iField=0; iField < poFDefn->GetFieldCount(); iField++)
    {
	poFieldDefn = poFDefn->GetFieldDefn(iField);
	printf("Field %d, Name: %s, Field Type %d, Field Width %d, Precision %d\n", 
		iField, poFieldDefn->GetNameRef(), poFieldDefn->GetType(), poFieldDefn->GetWidth(), poFieldDefn->GetPrecision());
	}
	
	// printf("\n\tGOT HERE - 6\n\n");    exit(-1);			// for faster debugging
	
//#####################


/*
				WRITE A FEATURE
		
  To WRITE A FEATURE to disk, we must create a local OGRFeature, set attributes and attach geometry 
  before trying to write it to the layer. It is imperative that this feature
   be instantiated from the OGRFeatureDefn associated with the layer it will be written to.
*/

	last_feat = feat_count;
	if(last_feat > 5) last_feat = 5 ;
	printf( "\n\nCopying (up to 5) features (polygons) from input to layer 1 of : %s \n\n",  argv[3]);
	
	piLayer->ResetReading();	// VERY IMPORTANT (cause we use GetNextFeature() 
	
   // PolygonFeature Polygon;

	for (iFeat=0; iFeat<last_feat; iFeat++)			// for no. of features  *** MAIN LOOP

	  {
	  piFeature = piLayer->GetNextFeature();
	  
	  piGeometry = piFeature->GetGeometryRef();

	// Create a new output feature
		
	  poFeature = OGRFeature::CreateFeature( poLayer->GetLayerDefn() );		//Add SCHEMA to that feature (1st)

	  //poFeature->SetGeometry( piFeature->GetGeometryRef() );		// ????
 


//			Move attribute data (FIELDS) for THIS feature


// For each field in the input layer, populate output layer with same data 

  for (iField=0; iField < last_field; iField++)		// #####
    {
    piFieldDefn = piFDefn->GetFieldDefn( iField );
    poFieldDefn = poFDefn->GetFieldDefn( iField );

 //   printf("Output Field %d, Field Type %d, Field Width %d, Precision %d \n", 
//		iField, piFieldDefn->GetType(), poFieldDefn->GetWidth(), poFieldDefn->GetPrecision());


// Get and Copy (Should use CASE as per tutorial example)

        switch( piFieldDefn->GetType() )
            {
                case OFTInteger:
                    //printf( "%d \n", piFeature->GetFieldAsInteger( iField ) );
                    poFeature->SetField(iField, piFeature->GetFieldAsInteger(iField) );
                    break;
                case OFTInteger64:
                    //printf( CPL_FRMT_GIB "\n", piFeature->GetFieldAsInteger64( iField ) );
     		    poFeature->SetField(iField, piFeature->GetFieldAsInteger64(iField) );
                    break;
                case OFTReal:
                    //printf( "%.3f \n", piFeature->GetFieldAsDouble(iField) );
     		    poFeature->SetField(iField, piFeature->GetFieldAsDouble(iField) );
                    break;
                case OFTString:
                    //printf( "%s \n", piFeature->GetFieldAsString(iField) );
     		    poFeature->SetField(iField, piFeature->GetFieldAsString(iField) );
                    break;
                default:
                    //printf( "%s \n", piFeature->GetFieldAsString(iField) );
     		    poFeature->SetField(iField, piFeature->GetFieldAsString(iField) );
                    break;
            }

/*
		if( (iFeat == 3) && (iField == 2) )
				poFeature->SetField(2, 55 );		// TESTING - set 55 at 4th feature, 3rd field
*/			
			
// Other tests
   //poFeature->SetField(iField, piFeature->GetType(iField) );
    //poFeature->SetField(iField, piFieldDefn->GetType() );
    //poFeature->SetField(iField, piFeature->FieldValue(iField));    // should work ??
    //poFeature->SetField(iField, iField );		// WORKS : put field # in field for all features

    }		// end of loop for fields
		
	  printf("\nAll field \"definitions\" moved for feature(poly) %d\n", iFeat);
	

	
//	 printf("\n\tGOT HERE - 7\n\n");    exit(-1);			// for faster debugging
	
//	printf("\n\tGOT HERE - 7\n\n"); 


//*****************************************************************************
//*********************************************************************************	



//	MOVE FEATURE (Polygon) perse

printf( "Copying feature(polygon) %d from input to polygon %d (layer 1) of %s \n",  iFeat, iFeat, argv[3]);


//	piLayer->GetNextFeature();
//	poLayer->GetNextFeature();

	
    // NumberOfInnerRings = Polygon->getNumInteriorRings();

    //OGRLinearRing *poExteriorRing = piPolygon->getExteriorRing();


	// Geographic Info

	//piSRSIn = OGRGeomFieldDefn::GetSpatialRef( piFDefn ) 		
	//poLayer->SetSpatialRef( piSRSIn ) 	

	// oPointField.SetSpatialRef(poSRS);
	//poFeature->SetGeometry( &pt );





// MOVE VERTICES of polygon (i.e., geometry) to output feature

	  if (wkbFlatten(piGeometry->getGeometryType()) == wkbPolygon )
	    	poFeature->SetGeometry( piGeometry );						// **** THAT IS ALL  (if both are real polygons)



// ### IF IT WAS  A FAKE POLYGON


	  if (wkbFlatten(piGeometry->getGeometryType()) == wkbLineString )
		{	
			
 			//poFeature->SetGeometry(GetGeometryType(poPolygon));	

			//poFeature.Geometry = getGeometryType(poPolygon);

			//OGR_F_SetGeometry(poFeature, poPolygon);



	// Need to move the feature (polygons) and its attributes to the output file

	// Now we create a feature in the file

	     if( poLayer->CreateFeature( poFeature ) != OGRERR_NONE )
 	    {
        	printf( "\t**Failed to create feature(poly) in shapefile.\n" );
        	exit( 1 );
 	    }
	     
	
	  }



// Prepare for next feature	(in the loop)
		 
		OGRFeature::DestroyFeature( poFeature );

		poLayer->GetNextFeature();
 			
		
	  printf("Polygon %d (attrib & vertices) was moved to output SHP file \n\n", iFeat);
	  
	  
		
	  }		// **** MAIN LOOP ***  for NEXT feature 

	  

	  
// ****************************************
// ****************************************
	  

// Create description for that file

	printf("\n\n\n** All (up to 5) polygons and their attributes were moved to output SHP file %s\n\n", argv[3]);

	printf("\nCreating Layer Description ... \n");

	//printf("\nInput Layer Description: %s \n", piLayer->GetDescription() );	
	
	strcpy(description, "Subset from ");		// create a description
	strcat(description, argv[1]);
	strcat(description, " layer ");
	strcat(description, argv[2]);
	poLayer->SetDescription(description);
	poDS->SetDescription(description);
	
	printf("Output Layer Description:      %s \n", poLayer->GetDescription() );
	
//*********************************************************************

// Close input and output datasets 

Exit:	printf("\n**** Closing files and exiting program. \n");


	GDALClose(piDS);			// close input/output dataset

	if (argv[3] != NULL) GDALClose(poDS);

	time (&rawtime);
	timeinfo = localtime (&rawtime);
	fprintf(stdout,"\n\n_______________________________\n");
	fprintf(stdout,"\n %s (%s) finished at %s\n\n", PROG_NAME, VERSION,  asctime(timeinfo));

} 		// end of main function

