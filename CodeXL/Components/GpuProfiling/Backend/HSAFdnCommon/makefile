#MAKE FILE FOR HSACommon

DEPTH = ../..
include $(DEPTH)/Build/Common.mk

TARGETLIB = lib$(GPU_PROFILER_LIB_PREFIX)HSACommon$(TARGET_SUFFIX).a

ADDL_DEFINES = -DAMD_INTERNAL_BUILD
ADDL_CFLAGS = $(HSAFDN_CFLAGS)

INCLUDES = \
	-I. \
	-I../Common \
	-I$(COMMON_PROJ_DIR) \
	-I$(HSA_DIR)/include \
	-I$(DYNAMICLIBRARYMODULE_DIR) \
	-I$(TSINGLETON_DIR) \

LIB_OBJS =  \
	./$(OBJ_DIR)/HSAFunctionDefsUtils.o \
	./$(OBJ_DIR)/HSAModule.o \
	./$(OBJ_DIR)/HSAToolsRTModule.o \
	./$(OBJ_DIR)/HSAAgentUtils.o \
	./$(OBJ_DIR)/HSAAgentIterateReplacer.o \
	./$(OBJ_DIR)/HSAAPITableVersions.o \

include $(DEPTH)/Build/CommonTargets.mk

# END OF MAKE FILE
