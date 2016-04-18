#MAKE FILE FOR PreloadXInitThreads

DEPTH = ../..
include $(DEPTH)/Build/Common.mk

# make sure CallXInitThreads gets called during initialization
ADDL_LINKFLAGS = -Wl,-init,CallXInitThreads
TARGETSO = $(BACKEND_OUTPUT_DIR)/lib$(GPU_PROFILER_LIB_PREFIX)PreloadXInitThreads$(TARGET_SUFFIX).so

INCLUDES = \
	-I. \

LIBS = -lX11

LIBPATH = $(COMMON_LIB_PATHS)

SO_OBJS =  \
	./$(OBJ_DIR)/XInitThreadsPreload.o \

include $(DEPTH)/Build/CommonTargets.mk

# END OF MAKE FILE
