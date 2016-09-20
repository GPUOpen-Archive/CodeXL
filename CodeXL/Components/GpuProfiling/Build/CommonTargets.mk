default: $(TARGET) $(TARGETSO) $(TARGETLIB)

# build targets
$(TARGET) : makedir $(OBJS)
	$(CC) $(LINKFLAGS_EXE) $(PLATFORM_LFLAG) $(OBJS) $(LIBPATH) $(LIBS) -o $(TARGET) $(STATIC_LIBS)

$(TARGETLIB) : makedir $(LIB_OBJS)
	ar rcs $(TARGETLIB) $(LIB_OBJS)

$(TARGETSO) : makedir $(SO_OBJS)
	$(CC) $(LINKFLAGS_SO) $(PLATFORM_LFLAG) $(SO_OBJS) $(LIBPATH) $(LIBS) -o $(TARGETSO) $(STATIC_LIBS)

x86:
	$(MAKE) -f $(MAKEFILENAME) $(BUILD_X86_OVERRIDES)

Internal:
	$(MAKE) -f $(MAKEFILENAME) $(BUILD_INTERNAL_OVERRIDES)

Internalx86:
	$(MAKE) -f $(MAKEFILENAME) $(BUILD_INTERNAL_X86_OVERRIDES)

Dbg:
	$(MAKE) -f $(MAKEFILENAME) $(BUILD_DEBUG_OVERRIDES)

Dbgx86:
	$(MAKE) -f $(MAKEFILENAME) $(BUILD_DEBUG_X86_OVERRIDES)

DbgInternal:
	$(MAKE) -f $(MAKEFILENAME) $(BUILD_DEBUG_INTERNAL_OVERRIDES)

DbgInternalx86:
	$(MAKE) -f $(MAKEFILENAME) $(BUILD_DEBUG_INTERNAL_X86_OVERRIDES)

makedir:
	mkdir -p $(OBJ_DIR)
	mkdir -p $(BACKEND_OUTPUT_DIR)

all: default x86 Internal Internalx86 Dbg Dbgx86 DbgInternal DbgInternalx86

# source file targets

BUILD_SRC=$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) -c $< -o $@

$(OBJ_DIR)/%.o: %.cpp
	$(BUILD_SRC)

$(OBJ_DIR)/%.o: $(BACKEND_COMMON_DIR)/%.cpp
	$(BUILD_SRC)

$(OBJ_DIR)/%.o: $(BACKEND_CLTRACEAGENT_DIR)/%.cpp
	$(BUILD_SRC)

$(OBJ_DIR)/%.o: $(BACKEND_CLTHREADTRACEAGENT_DIR)/%.cpp
	$(BUILD_SRC)

$(OBJ_DIR)/%.o: $(BACKEND_CLPROFILEAGENT_DIR)/%.cpp
	$(BUILD_SRC)

$(OBJ_DIR)/%.o: $(BACKEND_CLOCCUPANCYAGENT_DIR)/%.cpp
	$(BUILD_SRC)

$(OBJ_DIR)/%.o: $(BACKEND_HSAFDNCOMMON_DIR)/%.cpp
	$(BUILD_SRC)

$(OBJ_DIR)/%.o: $(BACKEND_HSAFDNTRACE_DIR)/%.cpp
	$(BUILD_SRC)

$(OBJ_DIR)/%.o: $(DYNAMICLIBRARYMODULE_DIR)/%.cpp
	$(BUILD_SRC)

$(OBJ_DIR)/%.o: $(DEVICEINFO_DIR)/%.cpp
	$(BUILD_SRC)

$(OBJ_DIR)/%.o: $(AMDTMUTEX_DIR)/%.cpp
	$(BUILD_SRC)

$(OBJ_DIR)/%.o: $(ACLMODULEMANAGER_DIR)/%.cpp
	$(BUILD_SRC)

$(OBJ_DIR)/%.o: $(GPUPERFAPIUTILS_DIR)/%.cpp
	$(BUILD_SRC)

$(OBJ_DIR)/%.o: $(GPUTHREADTRACEUTILS_DIR)/%.cpp
	$(BUILD_SRC)

$(OBJ_DIR)/%.o: $(ADLUTIL_DIR)/%.cpp
	$(BUILD_SRC)

$(OBJ_DIR)/%.o: $(BACKEND_SPROFILE_DIR)/%.cpp
	$(BUILD_SRC)

$(OBJ_DIR)/%.o: $(BACKEND_SANALYZE_DIR)/%.cpp
	$(BUILD_SRC)

$(OBJ_DIR)/%.o: $(CELF_DIR)/Src/%.cpp
	$(BUILD_SRC)

$(OBJ_DIR)/%.o: $(COMMON_SRC)/HSATestCommon/%.cpp
	$(BUILD_SRC)

$(OBJ_DIR)/%.o: $(HSAUTILS_DIR)/%.cpp
	$(BUILD_SRC)

# clean targets
clean:
	rm -f $(OBJS) $(LIB_OBJS) $(SO_OBJS) $(TARGET) $(TARGETLIB) $(TARGETSO)

cleanx86:
	$(MAKE) -f $(MAKEFILENAME) clean $(BUILD_X86_OVERRIDES)

cleanInternal:
	$(MAKE) -f $(MAKEFILENAME) clean $(BUILD_INTERNAL_OVERRIDES)

cleanInternalx86:
	$(MAKE) -f $(MAKEFILENAME) clean $(BUILD_INTERNAL_X86_OVERRIDES)

cleanDbg:
	$(MAKE) -f $(MAKEFILENAME) clean $(BUILD_DEBUG_OVERRIDES)

cleanDbgx86:
	$(MAKE) -f $(MAKEFILENAME) clean $(BUILD_DEBUG_X86_OVERRIDES)

cleanDbgInternal:
	$(MAKE) -f $(MAKEFILENAME) clean $(BUILD_DEBUG_INTERNAL_OVERRIDES)

cleanDbgInternalx86:
	$(MAKE) -f $(MAKEFILENAME) clean $(BUILD_DEBUG_INTERNAL_X86_OVERRIDES)

spotless: clean cleanx86 cleanInternal cleanInternalx86 cleanDbg cleanDbgx86 cleanDbgInternal cleanDbgInternalx86
	rm -rf $(BASE_OBJDIR)

