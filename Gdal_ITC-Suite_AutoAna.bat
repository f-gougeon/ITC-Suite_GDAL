:: Gdal_ITC-Suite_AutoAna.bat
:: 
:: 		 Automatic analysis of Photo Plot via GDAL ITC-Suite
:: 
:: 	Example of an ITC analysis with the GDAL version of the ITC-Suite
:: 	(as done from a cmd window - same can be done under ArcGIS, but more manually)
:: 
:: 			Francois A. Gougeon
:: 				Aug.-Sept. 2025 

:: Version 1-5 : As xxxx.txt, coding directly what needed to be done 
::	for specific images (e.g., 591,596,3086). Mostly run manually
:: 	by "cut and paste" of sections of code and monitorig results
:: 
:: Oct 2025 (v5-6-7) - Making it MORE AUTOMATIC (running as a BATCH  file)
::	With just a few questions at the unset to know where project files are
:: 
:: 	Version 7 -- Cleans up  -- Lots of trials and tribulations 
::
:: 	Version 8 -- For "image independent" ITC Analysis: asking user about image
:: 

::   ******************************************************

:: NOTE:   REM  need to be :  REM  spaces --- not REM  tab --- to not interfere 

::  	From your Working directory (here assumed for quick testing):

::    F:/NFI_Photo_Plots/Dead_Standing/%FName%/Analysis/596_GDAL_RealAuto_nn

::   	Original Image :

::    F:\NFI_Photo_Plots\Dead_Standing\%FName%\Org_Image\nl_pp_833596_lc_img_1.tif


@ECHO OFF 
ECHO ---  
ECHO ***********************************************************************************
ECHO	 *** Automatic Analysis of an Aerial Image with the ITC-Suite (GDAL version) ***
ECHO ***********************************************************************************

ECHO ---

::   Ask the USER where are the original images (16 or 32 bit)

:: SET /P Orig_Dir=Where is the original image(s) (16 or 32 bit) located ?

:: for quick testing
SET  Orig_Dir=F:\NFI_Photo_Plots\Dead_Standing\%FName%\Org_Image

ECHO The directory is: %Orig_Dir%

:: Ask the user what is the original images (16 or 32 bit) file name

ECHO ---

:: SET /P Orig_FName=What is the file name of the original image ?
:: for quick testing
  SET Orig_FName=nl_pp_833596_lc_img_1.tif
  SET Orig_FName=bc_pp_1493386_lc_img_1.TIF

ECHO Original file name is: %Orig_FName%


:: Full file name to get access to it

REM ECHO ---
REM ECHO Full File name : %Orig_Dir%\%Orig_FName%
REM ECHO ---

:: Main file name for this project

ECHO ---
::SET /P FName= What will be the main file name for this project?

:: for quick testing
SET FName=PP833596
SET FName=PP833591
SET FName=PP1493386

ECHO Main file name for this project will be : %FName% 
ECHO ---

REM   PAUSE 

:: goto Cont

REM 	*****************************************************
REM 	*****************************************************


:: ECHO OFF

SETLOCAL EnableDelayedExpansion 

ECHO    Converting each channel from original image to 8bit from ...
ECHO ---

gdalinfo %Orig_Dir%\%Orig_FName% | grep  Mean=  > temp_all.txt
cat temp_all.txt
ECHO ---

SET /P NIR_CH=  Which channel no. is the Near-Infrared channel?

:: for quick testing
:: SET /A NIR_CH=4

ECHO As per user, channel %NIR_CH% is the Near-Infrared channel

ECHO ---- 

REM 			##### MAIN LOOP to get images #####

FOR /L %%I IN (1,1,4) DO (

 ECHO     Checking channel %%I ...
 sed -n '%%Ip' temp_all.txt > temp.txt
 cat temp.txt 
 
ECHO ---- 
 

REM gdalinfo %Orig_Dir%\%Orig_FName% | grep  Mean= | head -%%I > temp.txt

    FOR /F "tokens=3,4 delims=," %%A IN (temp.txt) DO ( 
    SET TextMEAN=%%A
    SET TextSTDEV=%%B
    ECHO TextMEAN is set to !TextMEAN!
    ECHO TextSTDEV is set to  !TextSTDEV!
    )


REM SET /A RMEAN=!TextMEAN:~-7!
REM SET RMEAN=!TextMEAN:~6,3!
REM SET RMEAN=!TextMEAN:~6,4!

SET /a RMEAN= !TextMEAN! + 0
REM SET /A RMEAN= !TextMEAN!:~
ECHO RMEAN is set to !RMEAN!

REM SET RSTDEV=!TextSTDEV:~-7!
SET /a RSTDEV=!TextSTDEV:~8,3!
REM SET /A RSTDEV = !TextSTDEV!
ECHO RSTDEV is set to !RSTDEV!


REM   )

ECHO ---  

  
ECHO Range to convert channel %%I

  SET /A HIGH= !RMEAN! + 3 * !RSTDEV!
::  SET /A LOW= !RMEAN! - 3 * !RSTDEV!
::  IF !LOW! LSS 0 set /A LOW=0

:: for better normalization bettween images and for thresholding^
  SET /A LOW=0


  ECHO Range is :  !LOW! !HIGH!

REM )

  ECHO     Running gdal_translate ^(to 8bit^) on channel %%I ...
  ECHO ---  
REM  ######  NOTES #####  Need to escape parentheses even in ECHO ######

REM  ECHO     Running gdal_translate (to 8bit) on channel %%I ...  :: BUGGY
REM  ECHO     "Running gdal_translate (to 8bit) on channel %%I ..."  :: OK
  
REM )  
  
  gdal_translate  -ot Byte -b %%I -scale  !LOW! !HIGH! 0 255 %Orig_Dir%\%Orig_FName% %FName%_%%I.tif
  
  ECHO ---
  
 REM 	End of main loop 
)

  ECHO --- 
  ECHO         ### Individual channel 8bit TIF files were all created ###
  ECHO --------------------------------------------------------------- 
 
PAUSE



  ECHO ---------------------------------------------------------------
  ECHO   Rendering full 32 bit image to 8bit  ^(cause convenient to look at, and NIR first, assumed CH4^)
  
REM  gdal_translate  -ot Byte -b 4 -b 1 -b 2 -b 3 -scale 0 2300  0 254 ..\Org_Image\nl_pp_833596_lc_img_1.tif %FName%_4CHs.tif
  
  gdal_translate  -ot Byte -b 4 -b 1 -b 2 -b 3 -scale 0 2300  0 254  %Orig_Dir%\%Orig_FName% %FName%_4CHs.tif

  ECHO --- 
  ECHO --- 

ECHO  Viewing that Image with PCI FreeView  (if PCI exist on that PC)


ECHO START /B "Starting PCI Free Focus"  "C:\PCI Geomatics\Geomatica FreeView 2017\exe\freeview.exe" %FName%_4CHs.tif

  ECHO ---
  
PAUSE


ECHO ---  
ECHO    Some file renaming using NIR Channel as guide ^(as per user^)
ECHO ---
  
ECHO As per user, using NIR Channel ^(%NIR_CH%^) as guide, other channels following sequentially
  
COPY/Y  %FName%_%NIR_CH%.tif  %FName%_NIR.tif

IF %NIR_CH% EQU 4 (
ECHO Assuming channels Red, Green, and Blue are 1,2,3
COPY/Y  %FName%_1.tif  %FName%_RED.tif
COPY/Y  %FName%_2.tif  %FName%_GREEN.tif
COPY/Y  %FName%_3.tif  %FName%_BLUE.tif
) ELSE (
ECHO Assuming channels Red, Green, and Blue are 2,3,4
COPY/Y  %FName%_2.tif  %FName%_RED.tif
COPY/Y  %FName%_3.tif  %FName%_GREEN.tif
COPY/Y  %FName%_4.tif  %FName%_BLUE.tif
)

 REM   PAUSE

ECHO     ******************************************************
ECHO     *** Starting Automatic Analysis of that Aerial Image 
ECHO     ******************************************************

ECHO            SMOOTHING THESE IRGB IMAGES ^(3x3^)


ave_filter_g   %FName%_NIR.tif  %FName%_NIR_3x3Ave.tif 3

ave_filter_g   %FName%_RED.tif  %FName%_RED_3x3Ave.tif 3

ave_filter_g   %FName%_GREEN.tif  %FName%_GREEN_3x3Ave.tif 3

ave_filter_g   %FName%_BLUE.tif  %FName%_BLUE_3x3Ave.tif 3

 REM  PAUSE

ECHO    ******************************************************
ECHO     Creating images that may be useful in general to create masks
ECHO     ******************************************************

ECHO ---
ECHO      Get HOMOGEN Image on NIR Image ^(NVAR^)

homogen_g %FName%_NIR.tif %FName%_NVAR.tif NVAR 7,31


ECHO     ******************************************************
ECHO ---
ECHO      Get NDVI Image

ndvi_ima_g %FName%_4CHs.tif %FName%_NDVI.tif 1,2


ECHO     ******************************************************
ECHO ---
ECHO      Get directionanilty Image

grad_dc_g %FName%_4CHs.tif 1 %FName%_DC.tif 7,31 160


ECHO     ******************************************************
ECHO ---
ECHO   Get HOMOGEN on BLUE ^(MEAN^)which helps create non-forest masks for various areas


homogen_g %FName%_BLUE_3x3Ave.tif %FName%_HomoBlue.tif MEAN 7,31

  
:Cont  

ECHO      ******************************************************
ECHO      	Creating masks of interest ^(mostly by thresholding^)
ECHO      ******************************************************

ima_thr_g %FName%_NIR_3x3Ave.tif %FName%_Shaded_Areas.tif 0,10

ima_thr_g %FName%_NIR_3x3Ave.tif %FName%_Water.tif 0,20

ima_thr_g %FName%_BLUE_3x3Ave.tif %FName%_White_Areas.tif 200,255

ima_thr_g %FName%_HomoBlue.tif  %FName%_Brite_Areas.tif 120,255

ima_thr_g %FName%_HomoBlue.tif  %FName%_NotBrite_Areas.tif 0,119

ima_thr_g %FName%_NDVI.tif %FName%_Veg.tif  160,255


homogen_g %FName%_NIR.tif %FName%_CH1_NVAR.tif NVAR 7,31

ima_thr_g %FName%_CH1_NVAR.tif %FName%_Flat_Areas.tif 0,1


  PAUSE 




REM        Combination towards non-forest mask

thickbit_g %FName%_Flat_Areas.tif %FName%_Flat_Areas.tif 5

bmcombo_g %FName%_Flat_Areas.tif  %FName%_White_Areas.tif  %FName%_NonForest.tif OR


  PAUSE


REM    *****************************************************

REM  Could go to low res to clean-up these masks (point to a scriptto do it automatically )

REM    #####

 REM  PAUSE

ECHO       *****************************************************

ECHO      Analysis of the majority of the forested areas 

ECHO      ******************************************************


ECHO      The Tree Top Technique  


lattops_g %FName%_NIR_3x3Ave.tif,1 %FName%_NonForest.tif - 3 10 - MAT %FName%_TT.tif

thickbit_g   %FName%_TT.tif  %FName%_TT_Thick.tif 100


ECHO      The ITCVFOL/ISOL Technique 


itcvfol_g %FName%_NIR_3x3Ave.tif %FName%_NonForest.tif %FName%_VFOL.tif auto

itcisol_g %FName%_VFOL.tif %FName%_ISOL.tif MAT

itcsfil_g  %FName%_ISOL.tif  %FName%_ITC.tif - 3,10 2 		 // 3 to 10 metre diameters


REM    ### May want to separate ITCs into two groups (small and big Trees)


 REM  PAUSE



ECHO      ******************************************************

ECHO      Healthy Trees (TTs, ITCs) in Shaded Areas



ECHO       Tree Top Technique  in Shaded Areas

lattops_g %FName%_GREEN_3x3Ave.tif,1 %FName%_Shaded_Areas.tif,-1  - 3 100 - - %FName%_TT_Shaded.tif

thickbit_g  %FName%_TT_Shaded.tif  %FName%_TT_Shaded_Thick.tif 100



ECHO      ITCVFOL/ISOL  Technique in Shaded Areas

itcvfol_g %FName%_GREEN_3x3Ave.tif %FName%_Shaded_Areas.tif,-1 %FName%_VFOL_Shaded.tif auto

itcisol_g %FName%_VFOL_Shaded.tif %FName%_ISOL_Shaded.tif MAT

itcsfil_g  %FName%_ISOL_Shaded.tif  %FName%_ITC_Shaded.tif - 3,10 2 




REM 	******************************************************

REM    Post-processing


REM Combine all trees (ITCs and TTs) for ITCPCD on photo-interpreter stands shp file 


