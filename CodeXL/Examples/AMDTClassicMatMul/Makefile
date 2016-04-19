## ------------------------------ Makefile ------------------------------

## ------------------------------------------------------------
##   © 2015 Advanced Micro Devices, Inc. All rights reserved.
## ------------------------------------------------------------

###################################################################################
# This is the Makefile used to build the AMD Classic Matrix Multiplication sample
#
# author: Ilan Gal, ilan.gal@amd.com
###################################################################################


#######################################
# Use the default tools, not aliases
# and other stuff
#######################################
LS				:= /bin/ls
RM				:= /bin/rm
LN				:= /bin/ln
CP				:= /bin/cp
MKDIR				:= /bin/mkdir

#######################################
# AMDTClassicMatMul 
#######################################

AMDTCMM_EXE		:= AMDTClassicMatMul.exe

.PHONY: all clean exe

all: AMDTClassicMatMul
AMDTClassicMatMul: AMDTClassicMatMul.cpp
	g++ AMDTClassicMatMul.cpp -g -o $(AMDTCMM_EXE)

clean:
	$(RM) -f $(AMDTCMM_EXE)


# DO NOT DELETE
