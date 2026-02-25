#!/bin/bash
#
#		 Automatic analysis of satellite photo plots via GDAL ITC-Suite (Linux)
#			~/Scripts/ITC-Suite_Linux_GDAL_AutoAna.sh

#
#		 TEST on Linux version (via Oracle VM - Linux  Mint)
#
#			Francois A. Gougeon
#				Nov-Dec 2025, Feb 2026
#
#	 Nov 2025 -- Making the txt file (that I ran manually) in a shell (bash) script
#
# You need some to ANSWER some questions at beginning to point to your image locations
#	PRESENTLY -- Script is fixed (using 3386,591,596) not to have questions to ANSWER
#
#
#  You may need some to ANSWER at some point if file already exist (i.e.overwrite them)
#  Although, "apriori" multiple RETURNs (ENTER) can make the run  more automatic 
#
echo "    "
echo "Script to analyse satellite photo plots with the ITC-Suite (Linux version)"
echo "******************************************************************"

#  Ask the user what is the original images (16 or 32 bit) file name

echo "   "
echo "Where is the original image(s) (8, 16 or 32 bit) located ?"
#read Orig_Dir
# for quick testing
#Orig_Dir="/home/fgougeon/VM-Mint-Shared/Test_PP3386/Org_Image"
Orig_Dir="/home/fgougeon/Test_PP833591/Org_Image"
#Orig_Dir="/home/fgougeon/Test_PP833596/Org_Image"


echo "    "
echo  "As per user, the directory of original image is: " 
echo $Orig_Dir

echo "   "
echo "What is the original image name ?"
#read Orig_Ima
# for quick testing
#Orig_Ima="bc_pp_1493386_lc_img_1.tif"
Orig_Ima="nl_pp_833591_lc_img_1.tif"
#Orig_Ima="nl_pp_833596_lc_img_1.tif"

echo "    "
echo  "As per user, the original image is: " 
echo $Orig_Ima

echo "    "
echo  "As per user, the original image full path is: " 
Orig_File=$Orig_Dir"/"$Orig_Ima
# for quick testing
#Orig_File=PP3386_5CH.tif
#Orig_File="/home/fgougeon/Test_PP833591/Org_Image/nl_pp_833591_lc_img_1.tif"
#Orig_File="/home/fgougeon/Test_PP833596/Org_Image/nl_pp_833596_lc_img_1.tif"
echo $Orig_File

echo "   "
echo "What is the main area name (base image name) to use for this project?"
#read Base_Name
echo "    "
# for quick testing
#Base_Name=PP3386
Base_Name=PP833591_v2
#Base_Name=PP833596
echo  "As per user, the base file name to be used for this project is: " 
echo $Base_Name

echo "  "
echo "**********************************************************"

#exit

#
#
#  	Working directory :
#
#cd  /home/fgougeon/Test_PP3386

echo "    "
echo "The working directory will be the present default directory :"
pwd
echo "    "
# ls *.tif

#exit

# ITC-Suite  Location

echo " "
echo "The Linux-GDAL version of the ITC-Suite is assumed at : "
echo  "/home/fgougeon/ITC-Suite_GDAL_Src_2025"
echo " "

export PATH=$PATH:/home/fgougeon/ITC-Suite_GDAL_Src_2025


echo "**********************************************************"
#exit


#   Image CONVENIENT to LOOK AT to decide on NIR chanel number

echo " "
echo "For visualization, making two three(3) 8-bit channels version of the original image"
echo "From original image :  "  $Orig_File
echo " "

echo " "
echo "RGB image assuming CH 1,2,3 as RGB as ${Base_Name}_3CH_RGB.tif"
echo " "

gdal_translate -b 1 -b 2 -b 3 -ot Byte -scale 0 2000 0 255 $Orig_File  ${Base_Name}_3CH_RGB.tif
display -quiet -resize 800x600 ${Base_Name}_3CH_RGB.tif  &

echo " "
echo "FCIR image assuming NIR is CH4 as  ${Base_Name}_3CH_FCIR.tif"
echo " "

gdal_translate -b 4 -b 1 -b 2 -ot Byte -scale 0 2000 0 255 $Orig_File  ${Base_Name}_3CH_FCIR.tif
display -quiet -resize 800x600 ${Base_Name}_3CH_FCIR.tif  &

#exit

#  NOTE: Linux default image viewer has issues with 4 or 5 channels TIF

#  	******************************************************
#  	******************************************************

echo " "
echo "Stats of channels within the original image file ${Base_Name} "
echo " "

gdalinfo -stats $Orig_File | grep "Mean=" > temp_all.txt
cat temp_all.txt

echo " "
echo "Looking at stats and/or previous images (or with apriori knowledge)"
echo   "Which channel no. is the Near-Infrared channel?"
#read NIR_CH
# for quick testing
NIR_CH=4
echo "Near-Infrared channel = "$NIR_CH

#exit

echo " "
echo "	 First, create normalized individual 8bit channels to work with ..."
echo " "

for I in {1..4}; do
echo "   "
echo   "Checking channel $I "
sed -n "$I p" temp_all.txt > temp.txt
cat temp.txt 
echo "  "

# Getting mean and stdev to create normaized 8bit images

TextMEAN=$(awk '{print $3}' temp.txt)
#echo "TextMEAN --- " $TextMEAN
Mean=$( echo $TextMEAN | grep -Po '\d+' |head -n 1 )
#echo "MEAN = "  $Mean

TextSTDEV=$(awk '{print $4}' temp.txt)
#echo "TextSTDEV --- " $TextSTDEV
StDev=$( echo $TextSTDEV | grep -Po '\d+' |head -n 1 )
#echo "StDev = " $StDev

High=$(( $Mean + (3 * $StDev) ))
#echo "High = " $High

Low=$(( $Mean - (3 * $StDev) ))
#echo "Low = " $Low

if [ "$Low" -lt 0 ]; then 
	#echo "$Low is less than zero"
	Low=0 
	#echo "Low = " $Low
fi

echo "Creating new normalized and independent image : ${Base_Name}_$I.tif"
echo "   "
gdal_translate  -ot Byte -b $I -scale $Low $High 0 255 $Orig_File ${Base_Name}_$I.tif


done		
# END OF LOOP  for channels(4) convertion to indepenndent 8bit images (or just normalization)


echo "  "
echo "Renaming image files from channel numbers to appropriate colors"
echo " "

if [ "$NIR_CH" -eq 4 ]; then 
mv ${Base_Name}_4.tif ${Base_Name}_NIR.tif
mv ${Base_Name}_1.tif ${Base_Name}_RED.tif
mv ${Base_Name}_2.tif ${Base_Name}_GREEN.tif
mv ${Base_Name}_3.tif ${Base_Name}_BLUE.tif
else
mv ${Base_Name}_1.tif ${Base_Name}_NIR.tif
mv ${Base_Name}_2.tif ${Base_Name}_RED.tif
mv ${Base_Name}_3.tif ${Base_Name}_GREEN.tif
mv ${Base_Name}_4.tif ${Base_Name}_BLUE.tif
fi

# Create a better four(4) channel image to help analysis
# The previous FCIR was just for viewing (to decide on NIR band channel number}
# The channels in this image will convey better pixel grey-level values


#  gdal_merge -o ${Base_Name}_4CH.tif ${Base_Name}_NIR.tif ${Base_Name}_RED.tif ${Base_Name}_GREEN.tif ${Base_Name}_BLUE.tif 



#exit

#  	******************************************************
echo " "
echo "	SMOOTHING THESE IRGB IMAGES (3x3)"
echo " "

ave_filter_g   ${Base_Name}_NIR.tif  ${Base_Name}_NIR_3x3Ave.tif 3

ave_filter_g   ${Base_Name}_RED.tif  ${Base_Name}_RED_3x3Ave.tif 3

ave_filter_g   ${Base_Name}_GREEN.tif  ${Base_Name}_GREEN_3x3Ave.tif 3

ave_filter_g   ${Base_Name}_BLUE.tif  ${Base_Name}_BLUE_3x3Ave.tif 3


#exit





#  	******************************************************

#  		Get HOMOGEN Image

homogen_g ${Base_Name}_NIR.tif ${Base_Name}_NVAR.tif NVAR 7,31


#  	******************************************************

#  		Get NDVI Image

ndvi_ima_g ${Base_Name}_3CH_FCIR.tif ${Base_Name}_NDVI.tif 1,2


#  	******************************************************


#  		Get directionanilty Image (not useful on PP3386, but maybe for other images)

# ANYWAY, Not available on Linux yet (FORTRAN issue)

#  grad_dc_g ${Base_Name}_3CH.tif 1 ${Base_Name}_DC.tif 7,31 160


#  	******************************************************
#  	******************************************************
#  	******************************************************
#  	******************************************************

#		CREATING MASKS of interest mostly by thresholding

#  	******************************************************

if [ "${Base_Name}" == "PP833596" ]; then

ima_thr_g ${Base_Name}_NIR_3x3Ave.tif ${Base_Name}_Shaded_Areas.tif 0,10

ima_thr_g ${Base_Name}_BLUE_3x3Ave.tif ${Base_Name}_White_Areas.tif 200,255

ima_thr_g ${Base_Name}_NVAR.tif ${Base_Name}_Flat_Areas.tif 0,1

thickbit_g ${Base_Name}_Flat_Areas.tif ${Base_Name}_Flat_Areas.tif 7

#      Combination towards non-forest mask

bmcombo_g ${Base_Name}_Flat_Areas.tif  ${Base_Name}_White_Areas.tif  ${Base_Name}_NonForest.tif OR

fi	# end of IF PP833596



if [[ "${Base_Name}" == PP833591* ]]; then

ima_thr_g ${Base_Name}_NIR_3x3Ave.tif ${Base_Name}_Shaded_Areas.tif 0,10

ima_thr_g ${Base_Name}_BLUE_3x3Ave.tif ${Base_Name}_White_Areas.tif 70,255

thickbit_g ${Base_Name}_White_Areas.tif ${Base_Name}_White_Areas.tif 5

ima_thr_g ${Base_Name}_NVAR.tif ${Base_Name}_Flat_Areas.tif 0,1

thickbit_g ${Base_Name}_Flat_Areas.tif ${Base_Name}_Flat_Areas.tif 7


#		Treed area

ima_thr_g ${Base_Name}_NVAR.tif  ${Base_Name}_Treed.tif 20,100

fi	# end of IF PP833591


if [ "${Base_Name}" == "PP3386" ]; then

#ima_thr_g ${Base_Name}_NIR_3x3Ave.tif ${Base_Name}_Shaded_Areas.tif 0,10

ima_thr_g ${Base_Name}_BLUE_3x3Ave.tif ${Base_Name}_White_Areas.tif 150,255

#   		Flat areas  (Threshold on HOMOGEN Image) 

ima_thr_g ${Base_Name}_NVAR.tif  ${Base_Name}_Flat_Areaa.tif 0,0 

thickbit_g ${Base_Name}_Flat_Areas.tif ${Base_Name}_Flat_Areas.tif 7

fi	# end of IF PP3386




#   **********************************************************
#   **********************************************************

#	OTHER MASKS (possibly useful)

#   **********************************************************
#   **********************************************************

#   		Water Areas (Threshold on NIR Image)

if [ "${Base_Name}" == "PP3386" ]; then
ima_thr_g ${Base_Name}_NIR_3x3Ave.tif ${Base_Name}_Water.tif 0,50
fi

# PP833596 is so bad (see histograms) there is no way to get water

#if [ "${Base_Name}" == "PP833596" ]; then
#ima_thr_g ${Base_Name}_NDVI.tif ${Base_Name}_Water.tif 0,1
#fi

#if [ "${Base_Name}" == "PP833596" ]; then
#ima_thr_g ${Base_Name}_RED.tif ${Base_Name}_Water.tif 0,102
#fi

#   ******************

#   		Vegetated  Areas (Threshold on NDVI Image) 

if [ "${Base_Name}" == "PP3386" ]; then 
ima_thr_g ${Base_Name}_NDVI.tif ${Base_Name}_Veg.tif  160,255	# for 3386
fi

if [[ "${Base_Name}" == PP833591* ]]; then 
ima_thr_g ${Base_Name}_NDVI.tif ${Base_Name}_Veg.tif  100,255	# for 833591_v2
fi


#exit

#  	*****************

	
if [ "${Base_Name}" == "PP3386" ]; then 


#  	*****************

#  	Combine water and flats --- Non-forest to use everywhere

#bmcombo_g ${Base_Name}_Water.tif ${Base_Name}_Flat_Areas.tif  ${Base_Name}_NonFor.tif OR

#  	*****************

#   	Create a healthy veg mask)

#bmcombo_g ${Base_Name}_Veg.tif ${Base_Name}_NonFor.tif ${Base_Name}_Healthy.tif SUB

bmcombo_g ${Base_Name}_Veg.tif ${Base_Name}_White_Areas.tif ${Base_Name}_Healthy.tif SUB


fi	# end of IF PP3386




#		Treed area (not flat vegetation)

if [[ "${Base_Name}" == PP833591* ]]; then 
ima_thr_g ${Base_Name}_NVAR.tif  ${Base_Name}_Treed.tif 20,100
fi


#exit 
#   ******************************************************************

#   	Use HOMOGEN on BLUE to help create more masks 


homogen_g ${Base_Name}_BLUE_3x3Ave.tif ${Base_Name}_HomoBlue.tif MEAN 7,31


#   	Very brite areas (licken (ash) covered)
if [ "${Base_Name}" == "PP3386" ]; then 
ima_thr_g ${Base_Name}_HomoBlue.tif  ${Base_Name}_Brite.tif 120,255
fi

if [[ "${Base_Name}" == PP833591* ]]; then 
ima_thr_g ${Base_Name}_HomoBlue.tif  ${Base_Name}_Brite.tif 100,255
fi


if [ "${Base_Name}" == "PP833596" ]; then 
ima_thr_g ${Base_Name}_HomoBlue.tif  ${Base_Name}_Brite.tif 100,255 

ima_thr_g ${Base_Name}_HomoBlue.tif  ${Base_Name}_NoAna.tif 0,10
fi


#  	******************************************************
#  	******************************************************

# Detecting Trees (TTs and ITCs)

#  	******************************************************
#  	******************************************************



#  		FOR PP833591 Detect good trees (TTs) Treed_Areas

if [[ "${Base_Name}" == PP833591* ]]; then 

#  	Detect HEALTHY Trees in dense forested areas 

ave_filter_g ${Base_Name}_NIR.tif ${Base_Name}_NIR_Ave_53.tif 5,3

lattops_g  ${Base_Name}_NIR_Ave_53.tif,1 ${Base_Name}_Treed.tif,-1 - 5 50 - MAT ${Base_Name}_TT_Healthy.tif

thickbit_g  ${Base_Name}_TT_Healthy.tif  ${Base_Name}_TT_Healthy_Thick.tif 3

#display -quiet -resize 800x600 ${Base_Name}_TT_Healthy_Thick.tif & # Use FreeView instead



#   Using ITCVFO/ISOL Approach for  treed areas
#   ******************************************************

itcvfol_g ${Base_Name}_NIR_Ave_53.tif ${Base_Name}_Treed.tif,-1 ${Base_Name}_VFOL.tif auto

itcisol_g ${Base_Name}_VFOL.tif ${Base_Name}_ISOL.tif MAT

itcsfil_g  ${Base_Name}_ISOL.tif  ${Base_Name}_ITC.tif - 3,10 2 		 // 3 to 10 metre diameters




#  	Detect  Trees in  Whitish Areas (lichen, ash)   (via NIR inversion)
#   ******************************************************


glinv_g  ${Base_Name}_RED_3x3Ave.tif  ${Base_Name}_InvRED.tif navg


#lattops_g ${Base_Name}_InvRED.tif,1 ${Base_Name}_Brite.tif,-1  - 5 130 - - ${Base_Name}_TT_Bole.tif

lattops_g ${Base_Name}_InvRED.tif,1 ${Base_Name}_White_Areas.tif,-1  - 5 100 - - ${Base_Name}_TT_Bole.tif

thickbit_g  ${Base_Name}_TT_Bole.tif  ${Base_Name}_TT_Bole_Thick.tif 3




#   Using ITCVFO/ISOL in  Whitish Areas (lichen, ash)   (via NIR inversion)
#   ******************************************************

itcvfol_g ${Base_Name}_InvRED.tif ${Base_Name}_White_Areas.tif,-1  ${Base_Name}_BBole_VFOL.tif auto

itcisol_g ${Base_Name}_BBole_VFOL.tif ${Base_Name}_BBole_ISOL.tif MAT

itcsfil_g  ${Base_Name}_BBole_ISOL.tif  ${Base_Name}_BBole_ITC.tif - 3,10 2 	 // 3 to 10 metre diameters



# 	veg makes a good non-forest mask for dead trees

#lattops_g ${Base_Name}_InvRED.tif,1 ${Base_Name}_Veg.tif  - 5 130 - - ${Base_Name}_TT_Bole.tif

#thickbit_g  ${Base_Name}_TT_Bole.tif  ${Base_Name}_TT_Bole_Thick.tif 3

# display -quiet -resize 800x600 ${Base_Name}_TT_Bole_Thick.tif &




#ima_thr_g  ${Base_Name}_InvRED.tif ${Base_Name}_NF_Dead.tif 150,255

#bmcombo_g ${Base_Name}_NF_Dead.tif ${Base_Name}_Veg.tif ${Base_Name}_NF_Dead_v2.tif OR

#lattops_g ${Base_Name}_InvRED.tif,1 ${Base_Name}_NF_Dead_v2.tif - 5 130 - - ${Base_Name}_TT_Bole.tif

#thickbit_g  ${Base_Name}_TT_Bole.tif  ${Base_Name}_TT_Bole_Thick.tif 3


fi		#end of for PP833951




if [ "${Base_Name}" == "PP833596" ]; then


#   *****************************************************

#   Analysis of the majority of the forested areas (above non-forest mask is OK)

#   ******************************************************


#     Tree Top Approach  
#   ******************************************************

lattops_g ${Base_Name}_NIR_3x3Ave.tif,1 ${Base_Name}_NonForest.tif - 3 10 - MAT ${Base_Name}_TT.tif

thickbit_g   ${Base_Name}_TT.tif  ${Base_Name}_TT_Thick.tif 100


#     ITCVFO/ISOL Approach 
#   ******************************************************

itcvfol_g ${Base_Name}_NIR_3x3Ave.tif ${Base_Name}_NonForest.tif ${Base_Name}_VFOL.tif auto

itcisol_g ${Base_Name}_VFOL.tif ${Base_Name}_ISOL.tif MAT

itcsfil_g  ${Base_Name}_ISOL.tif  ${Base_Name}_ITC.tif - 3,10 2 		 // 3 to 10 metre diameters





#******************************************************

#   Healthy Trees (TTs, ITCs) in Shaded Areas

#	******************************************************


#    Tree Top Approach   in Shaded Areas

lattops_g ${Base_Name}_GREEN_3x3Ave.tif,1 ${Base_Name}_Shaded_Areas.tif,-1  - 3 100 - - ${Base_Name}_TT_Shaded.tif

thickbit_g  ${Base_Name}_TT_Shaded.tif  ${Base_Name}_TT_Shaded_Thick.tif 100



#     VFO/ISOL  Approach  in Shaded Areas

itcvfol_g ${Base_Name}_GREEN_3x3Ave.tif ${Base_Name}_Shaded_Areas.tif,-1 ${Base_Name}_VFOL_Shaded.tif auto

itcisol_g ${Base_Name}_VFOL_Shaded.tif ${Base_Name}_ISOL_Shaded.tif MAT

itcsfil_g  ${Base_Name}_ISOL_Shaded.tif  ${Base_Name}_ITC_Shaded.tif - 3,10 2 


fi			# End of For PP833596


#exit 


if [ "${Base_Name}" == "PP833596" ]; then

#  	******************************************************
#  	******************************************************

#  		Detect trees HEALTHY Areas (not realy dense)

#  	******************************************************


lattops_g  ${Base_Name}_NIR_3x3Ave.tif,1 ${Base_Name}_Healthy.tif,-1 - 5 50 - MAT ${Base_Name}_TT_Healthy.tif

thickbit_g  ${Base_Name}_TT_Healthy.tif  ${Base_Name}_TT_Healthy_Thick.tif 3


# via Shadow TT

#lattops_g  ${Base_Name}_NIR_3x3Ave.tif,1 ${Base_Name}_Healthy.tif,-1 ${Base_Name}_Healthy.tif 3 100,120 160 REGEN ${Base_Name}_TT_Healthy_v2.tif

#thickbit_g  ${Base_Name}_TT_Healthy_v2.tif  ${Base_Name}_TT_Healthy_Thick_v2.tif 3

#display -quiet -resize 800x600 ${Base_Name}_TT_Healthy_Thick.tif &







#   Using ITCVFO/ISOL for HEALTHY Trees in dense forested areas (soso cause tree are small crowns)
#   ******************************************************

#itcvfol_g ${Base_Name}_NIR_3x3Ave.tif ${Base_Name}_Healthy.tif,-1  ${Base_Name}_Healthy_VFOL.tif auto

#itcvfol_g ${Base_Name}_NIR_3x3Ave.tif ${Base_Name}_Healthy.tif,-1  ${Base_Name}_Healthy_VFOL.tif pre 80,200,1

#itcisol_g ${Base_Name}_Healthy_VFOL.tif ${Base_Name}_Healthy_ISOL.tif MAT

#itcsfil_g  ${Base_Name}_Healthy_ISOL.tif  ${Base_Name}_Healthy_ITC.tif - 2,5 2 	






#  	*****************

#  Detecting  DEAD TREES IN DREBRIS (CWD) areas (brite areas) (with specific shadows?)

#  	*****************


# Clean-up white areas


thickbit_g  ${Base_Name}_White_Areas.tif  ${Base_Name}_White_Areas_Thick.tif 7

bmcombo_g ${Base_Name}_White_Areas_Thick.tif ${Base_Name}_Flat_Areas.tif ${Base_Name}_White_Areas_Thick.tif SUB


#lattops_g  ${Base_Name}_NDVI.tif,1 ${Base_Name}_White_Areas_Thick.tif,-1  ${Base_Name}_White_Areas_Thick.tif 5 100 160 REGEN ${Base_Name}_ShadowTT.tif

#thickbit_g ${Base_Name}_ShadowTT.tif ${Base_Name}_ShadowTT_Thick.tif 3




lattops_g  ${Base_Name}_NDVI.tif,1 ${Base_Name}_White_Areas_Thick.tif,-1 - 5 150 - REGEN ${Base_Name}_WhiteTT.tif

thickbit_g ${Base_Name}_WhiteTT.tif ${Base_Name}_WhiteTT_Thick.tif 3





# other test 

#lattops_g  ${Base_Name}_NIR_3x3Ave.tif,1 ${Base_Name}_DarkForest.tif,-1  ${Base_Name}_DarkForest.tif 5 80 170 MAT ${Base_Name}_ShadowTT.tif

#thickbit_g ${Base_Name}_ShadowTT.tif ${Base_Name}_ShadowTT_Thick.tif 3







#  	*****************

#  	BOTTOM LEFT CORNER:  dead and semi-dead trees (50-50)

#  	Create proper non-forest mask For that area

#  	*****************


ima_thr_g ${Base_Name}_HomoBlue.tif  ${Base_Name}_Dark.tif 10,100

#bmcombo_g ${Base_Name}_Dark.tif ${Base_Name}_Dark.tif ${Base_Name}_NFDark.tif NOT

bmcombo_g ${Base_Name}_Dark.tif ${Base_Name}_Water.tif ${Base_Name}_DarkForest.tif SUB


#   Using SHADOW_TT  on the above


lattops_g  ${Base_Name}_RED_3x3Ave.tif,1 ${Base_Name}_DarkForest.tif,-1 - 5 90 - REGEN ${Base_Name}_Corner_ShadowTT.tif

thickbit_g ${Base_Name}_Corner_ShadowTT.tif ${Base_Name}_Corner_ShadowTT_Thick.tif 3


# 		Combine  the various thick TTs for PCD to report
#  PP3386_Corner_ShadowTT_Thick.tif   PP3386_TT_Healthy_Thick.tif  PP3386_WhiteTT_Thick.tif

bmcombo_g ${Base_Name}_Corner_ShadowTT_Thick.tif ${Base_Name}_TT_Healthy_Thick.tif ${Base_Name}_All_TT_Thick.tif OR

bmcombo_g ${Base_Name}_All_TT_Thick.tif ${Base_Name}_WhiteTT_Thick.tif ${Base_Name}_All_TT_Thick.tif OR






itcpcd_g ./PP3386_FCIR.tif PP3386_All_TT_Thick.tif  PP3386_All_TT_Thick.tif ./shapefile/bc_pp_1493386_lc.shp LOW ONT - PCD_File.txt

# OR

itcpcd_g ./PP3386_FCIR.tif PP3386_All_TT_Thick.tif - ./shapefile/bc_pp_1493386_lc.shp 




fi	# end of IF PP3386




