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
MAKEDEPEND		:= $(CC) $(CFLAGS) $< | sed -n 's/^\# *[0-9][0-9]* *"\([^"]*\)".*/$*.o: \1/p' | sort | uniq > $*.d


#######################################
# Specifically required for library
#######################################
CC				= g++
LIBS			= -lGL -lGLU -lAMDTTeaPot -ldl -lgobject-2.0 -lglib-2.0 -lX11 -lfltk_gl -lfltk -lXfixes -lXext -lXft -lfontconfig -lXcursor -lXinerama

LIBPATHS		= -L../AMDTTeaPotLib -L/usr/local/lib -L"Replace with path to your local FLTK lib folder"
SRCDIR			= src
SOURCES			= $(shell $(LS) $(SRCDIR)/*cpp 2> /dev/null)
OBJECTS			= $(addsuffix .o,$(basename $(SOURCES)))
DEPENDS			= $(subst .o,.d,$(OBJECTS))
INCDIRS			= -Iinc -I. -I../AMDTTeaPot -I../AMDTTeaPot/inc -I../AMDTTeaPotLib -I../AMDTTeaPotLib/inc -I/usr/include/GL -I/opt/AMDAPP/include -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/libpng12 -I/opt/AMDAPP/include -I"Replace with path to your local FLTK headers folder"
CPPDEFS			= -DTRACE_LEVEL=$(TRACE_LEVEL) -DDEBUG_LEVEL=$(DEBUG_LEVEL)
ifeq ($(BUILD),debug)
CPPDEFS			+= -D_DEBUG
else
CPPDEFS			+= -DNDEBUG
endif
CFLAGS			= -c -Wall `$(PKG_CONFIG) --cflags gthread-2.0 gobject-2.0 ` $(CPPDEFS) $(INCDIRS) 
ifeq ($(BUILD),debug)
CFLAGS			+= -g
else
CFLAGS			+= -O2
endif
LDFLAGS			= `$(PKG_CONFIG) --libs gthread-2.0 gobject-2.0` $(LIBPATHS) $(LIBS)
EXEC			= ../release/CXLTeaPot-bin

$(info ***********************************)
$(info Make sure fltk library is installed)
$(info Download fltk from www.fltk.org    )
$(info ***********************************)

.PHONY: all help clean

#######################################
# Rules 
#######################################

%.d: %.cpp
	$(CC) -M $(CFLAGS) $< > $@.$$$$;						\
	$(SED) 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@;	\
	$(RM) -f $@.$$$$;

%.d: %.c
	$(CC) -M $(CFLAGS) $< > $@.$$$$;						\
	$(SED) 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@;	\
	$(RM) -f $@.$$$$;

%.o: %.cpp
	$(CC) $(CFLAGS) $< -o $@

%o: %.c
	$(CC) $(CFLAGS) $< -o $@

help:
	@echo 'Usage: make [all | help]'
	@echo '****************************************************************************'
	@echo 'Make sure fltk library is installed (Download fltk from http://www.fltk.org)'
	@echo 'and -L path for FLTK libs is defined in the LIBPATHS macro, '
	@echo 'and -I path for FLTK headers is defined in the INCDIRS macro in this makefile'
	@echo '****************************************************************************'
	@echo '       all:      build the tutorial executable'
	@echo '       help:     print this messege'

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

clean:
	@echo '***************************************'
	@echo '****     Cleaning the tutorial     ****'
	@echo '***************************************'
	@$(RM) -f $(EXEC) $(OBJECTS) $(DEPENDS)

# DO NOT DELETE

-include $(DEPENDS)

