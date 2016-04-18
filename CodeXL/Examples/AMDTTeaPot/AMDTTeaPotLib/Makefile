##==================================================================================
## Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
##
## \author AMD Developer Tools Team
## \file Makefile
## 
##==================================================================================

## ------------------------------ Makefile ------------------------------

##############################################################################
# This is the Makefile used to build the AMDTTeaPot library
##############################################################################

# Some basic build configurations
ifndef TRACE_LEVEL
TRACE_LEVEL		= 20
endif
ifndef DEBUG_LEVEL
DEBUG_LEVEL		= 100
endif
ifndef BUILD
# Assumption - we only have two build modes, debug and release
BUILD			= release
endif

#######################################
# Use the default tools, not aliases
# and other stuff
#######################################
LS				:= /bin/ls
RM				:= /bin/rm
SED				:= /usr/bin/sed
PKG_CONFIG		:= /usr/bin/pkg-config

#######################################
# Specifically required for library
#######################################
CC				:= g++
SRCDIR			:= src
SOURCES			:= $(shell $(LS) $(SRCDIR)/*cpp 2> /dev/null)
OBJECTS			:= $(addsuffix .o,$(basename $(SOURCES)))
DEPENDS			:= $(subst .o,.d,$(OBJECTS))
INCDIRS			:= -Iinc -I. -I../../../CommonProjects/AMDTOSWrappers/Include -I/opt/AMDAPP/include -I../AMDTTeaPot/inc \
					-I/usr/local/lib/gtkglext-1.0/include -I/usr/local/include/gtkglext-1.0 -I/usr/include/GL \
					`$(PKG_CONFIG) --cflags gtk+-2.0 gthread-2.0 gobject-2.0 `
CPPDEFS			:= -D_FILE_OFFSET_BITS=64 -D_LARGE_FILES 
CPPDEFS			+= -DTRACE_LEVEL=$(TRACE_LEVEL) -DDEBUG_LEVEL=$(DEBUG_LEVEL)
ifeq ($(BUILD),debug)
CPPDEFS			+= -D_DEBUG
else
CPPDEFS			+= -DNDEBUG
endif
CFLAGS			:= -c -Wall -pthread -Wundef -fmessage-length=0 $(CPPDEFS) $(INCDIRS)
ifeq ($(BUILD),debug)
CFLAGS			+= -g
else
CFLAGS			+= -O2
endif
AR				:= ar
ARFLAGS			:= rcsv
EXEC			:= AMDTTeaPot

.PHONY: all help clean glut AMDTTeaPot

#######################################
# Rules 
#######################################

%.d: %.cpp
	$(CC) -M $(CFLAGS) $< > $@.$$$$	&&							\
	$(SED) 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@	&&	\
	$(RM) -f $@.$$$$

%.o: %.cpp
	$(CC) $(CFLAGS) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

help:
	@echo 'Usage: make [all | help]'
	@echo '       all:      build all parts of the tutorial (when in doubt, use this)'
	@echo '       help:     print this messege'

all: $(SOURCES) $(EXEC)

$(EXEC): lib$(EXEC).a

lib$(EXEC).a: $(OBJECTS)
	@$(AR) $(ARFLAGS) $@ $(OBJECTS)

clean:
	@echo '***************************************'
	@echo '****     Cleaning AMDTTeaPotLib    ****'
	@echo '***************************************'
	@$(RM) -f $(OBJECTS) $(DEPENDS)
	@$(RM) -f lib$(EXEC).a

# DO NOT DELETE

-include $(DEPENDS)
