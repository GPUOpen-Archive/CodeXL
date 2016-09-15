#MAKE FILE FOR CLProfileAgent

DEPTH = ../..
include $(DEPTH)/Build/Common.mk

TARGETSO = $(BACKEND_OUTPUT_DIR)/lib$(GPU_PROFILER_LIB_PREFIX)CLProfileAgent$(TARGET_SUFFIX).so
STREAM_SDK_LIB = -L$(ATI_STREAM_SDK_DIR)/lib/x86_64

INCLUDES = \
	-I. \
	-I$(BACKEND_COMMON_DIR) \
	-I$(BACKEND_CLCOMMON_DIR) \
	-I$(COMMON_PROJ_DIR) \
	-isystem$(GPU_PERF_API_DIR) \
	-isystem$(ATI_STREAM_SDK_DIR)/include \
	-isystem$(CAL_DIR)/include \
	-isystem$(COMMON_SRC)/GPUPerfAPIUtils \
	-I$(DYNAMICLIBRARYMODULE_DIR) \
	-I$(DEVICEINFO_DIR) \
	-I$(TSINGLETON_DIR) \
	-isystem$(BOOST_DIR) \
	-isystem$(CELF_DIR)/Include \
	-I$(UTF8CPP_DIR) \
	-I$(ACL_DIR)/include \
	-I$(ADL_DIR)/include \
	-I$(ADLUTIL_DIR) \
	-I$(AMDTMUTEX_DIR) \
	-I$(ACLMODULEMANAGER_DIR) \

LIBS = \
	$(CLCOMMON_LIBS) \
	$(COMMON_LIBS) \
	$(FRAMEWORK_LIBS) \
	$(STANDARD_LIBS)

LIBPATH = $(COMMON_LIB_PATHS)


SO_OBJS =  \
	./$(OBJ_DIR)/CLProfileAgent.o \
	./$(OBJ_DIR)/CLInitProfiler.o \
	./$(OBJ_DIR)/CLBuffer.o \
	./$(OBJ_DIR)/CLContext.o \
	./$(OBJ_DIR)/CLContextManager.o \
	./$(OBJ_DIR)/CLGPAProfiler.o \
	./$(OBJ_DIR)/CLKernel.o \
	./$(OBJ_DIR)/CLKernelAssembly.o \
	./$(OBJ_DIR)/ACLModuleManager.o \
	./$(OBJ_DIR)/CLUserEvent.o \
	./$(OBJ_DIR)/CLDeferredKernel.o \
	./$(OBJ_DIR)/CLProfilerMineCLEntry.o \
	./$(OBJ_DIR)/CLProfilerMineCLMemory.o \
	./$(OBJ_DIR)/CElf.o \
	./$(OBJ_DIR)/CElfSection.o \
	./$(OBJ_DIR)/CElfStringTable.o \
	./$(OBJ_DIR)/CElfSymbolTable.o \
        ./$(OBJ_DIR)/GPUPerfAPICounterLoader.o \

include $(DEPTH)/Build/CommonTargets.mk

# END OF MAKE FILE
