#MAKE FILE FOR DeviceInfo

DEPTH = ../..
include $(DEPTH)/Build/Common.mk

TARGETLIB = lib$(GPU_PROFILER_LIB_PREFIX)DeviceInfo$(TARGET_SUFFIX).a

INCLUDES = \
	-I. \
	-I../Common \
	-I$(COMMON_PROJ_DIR) \
	-I$(DEVICEINFO_DIR) \
	-I$(TSINGLETON_DIR) \

# for AMD-internal builds
ifneq ("$(wildcard $(DEVICEINFO_DIR)/DeviceInfoInternal.cpp)","")
	INTERNAL_OBJ := ./$(OBJ_DIR)/DeviceInfoInternal.o
endif

LIB_OBJS = \
	./$(OBJ_DIR)/DeviceInfoUtils.o \
	$(INTERNAL_OBJ) \

include $(DEPTH)/Build/CommonTargets.mk

# END OF MAKE FILE
