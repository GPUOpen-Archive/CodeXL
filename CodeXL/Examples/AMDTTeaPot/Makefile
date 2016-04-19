## ------------------------------ Makefile ------------------------------

## ------------------------------------------------------------
##   © 2011 Advanced Micro Devices, Inc. All rights reserved.
## ------------------------------------------------------------

##############################################################################
# This is the Makefile used to build the tutorial
#
# author: Lior Greenblatt, lior.greenblatt@amd.com
##############################################################################


#######################################
# Use the default tools, not aliases
# and other stuff
#######################################
LS				:= /bin/ls
RM				:= /bin/rm
LN				:= /bin/ln
CP				:= /bin/cp
MKDIR			:= /bin/mkdir

#######################################
# AMDTTeaPot library
#######################################
AMDTTP_LIB_DIR		:= AMDTTeaPotLib
AMDTTP_LIB			:= libAMDTTeaPot.a

#######################################
# WrapperGUI 
#######################################
AMDTTP_GUI_DIR		:= AMDTTeaPot
AMDTTP_GUI_EXE		:= AMDTTeaPot.exe

.PHONY: all help clean exe AMDTTeaPotLib

#######################################
# Rules 
#######################################
help:
	@echo 'Usage: make [all | exe | AMDTTeaPotLib | help]'
	@echo '       all:           build all parts of the tutorial (when in doubt, use this)'
	@echo '       exe:           build the tutorial executable'
	@echo '       AMDTTeaPotLib: build the TeaPot library'
	@echo '       help:          print this messege'

all: $(AMDTTP_LIB_DIR)/$(AMDTTP_LIB) $(AMDTTP_GUI_EXE)

exe: $(AMDTTP_GUI_EXE)

AMDTTeaPotLib: $(AMDTTP_LIB_DIR)/$(AMDTTP_LIB)

$(AMDTTP_GUI_EXE): $(AMDTTP_LIB_DIR)/$(AMDTTP_LIB) $(AMDTTP_GUI_DIR)/$(AMDTTP_GUI_EXE)
	@$(LN) -fs $(AMDTTP_GUI_DIR)/$(AMDTTP_GUI_EXE)
	@$(MKDIR) -p res
	@$(CP) -rf $(AMDTTP_LIB_DIR)/res/* res/.
	@$(CP) -rf $(AMDTTP_GUI_DIR)/res/* res/.

$(AMDTTP_GUI_DIR)/$(AMDTTP_GUI_EXE): FORCE
	@echo '***************************************'
	@echo '****   Building the tutorial GUI   ****'
	@echo '***************************************'
	@$(MAKE) -C $(AMDTTP_GUI_DIR) $(AMDTTP_GUI_EXE)

$(AMDTTP_LIB_DIR)/$(AMDTTP_LIB): FORCE
	@echo '***************************************'
	@echo '**** Building the AMDTTeaPot library ****'
	@echo '***************************************'
	@$(MAKE) -C $(AMDTTP_LIB_DIR) $(AMDTTP_LIB)
	@$(RM) -f $(AMDTTP_GUI_EXE)
	@$(RM) -f $(AMDTTP_GUI_DIR)/$(AMDTTP_GUI_EXE)

clean:
	$(MAKE) -C $(AMDTTP_GUI_DIR) $@
	$(MAKE) -C $(AMDTTP_LIB_DIR) $@
	$(RM) -f $(AMDTTP_GUI_EXE)
	$(RM) -rf res

FORCE:

# DO NOT DELETE
