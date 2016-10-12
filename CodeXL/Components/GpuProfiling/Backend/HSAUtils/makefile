#MAKE FILE FOR HSAUtils

DEPTH = ../..
include $(DEPTH)/Build/Common.mk

TARGETLIB = lib$(GPU_PROFILER_LIB_PREFIX)HSAUtils$(TARGET_SUFFIX).a

INCLUDES = \
	-isystem$(HSA_DIR)/include \
	-I$(TSINGLETON_DIR) \
	-I$(HSAUTILS_DIR) \
	-I$(DYNAMICLIBRARYMODULE_DIR)

LIB_OBJS = \
	./$(OBJ_DIR)/HSAUtils.o \
	./$(OBJ_DIR)/HSAModule.o \

include $(DEPTH)/Build/CommonTargets.mk

# END OF MAKE FILE
