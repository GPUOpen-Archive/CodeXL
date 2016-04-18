#MAKE FILE FOR APP Profiler's QtClient

# platform specific
PLATFORM_DIR = x86_64
FRAMEWORK_PLATFORM_DIR=x64
LIB_NAME = libAMDTGPUProfiling.so
PLATFORM_CFLAG =
PLATFORM_LINKFLAG =
LINKFLAGS = -shared -Wl,-z,defs $(PLATFORM_LINKFLAG)
PLATFORM =
OBJ_DIR = OBJx64
BUILD_DEF = -DGDT_BUILD_SUFFIX=
QT_BUILD_DIR_PLATFORM=x86_64/release
TINYXML_BUILD_DIR_PLATFORM=release/x86_64
DEBUG_SUFFIX=
DEBUG_SUFFIX_UNDERSCORE=
DEBUG_SUFFIX_NODASH=

# shared
BIN_DIR = ../bin/$(PLATFORM_DIR)
TARGET = $(BIN_DIR)/$(LIB_NAME)
CC = g++
OPTIMIZE = -O3 -DNDEBUG
DEFINES = -D_LINUX -DGDT_BUILD_SUFFIX="" -DQT_DLL -DQT_GUI_LIB -DQT_CORE_LIB -DQT_THREAD_SUPPORT -D__WXGTK__ -DwxUSE_GUI -D_LARGEFILES -D_FILE_OFFSET_BITS=64 -DUSE_POINTER_SINGLETON
COMMON_DIR = ../../../../Common
TSINGLETON_INCLUDE_DIR=$(COMMON_DIR)/Src/TSingleton
ADLUTIL_DIR=$(COMMON_DIR)/Src/ADLUtil
DEVICEINFO_DIR=$(COMMON_DIR)/Src/DeviceInfo
ADL_INCLUDE_DIR=$(COMMON_DIR)/Lib/AMD/ADL/include
CAL_INCLUDE_DIR=$(COMMON_DIR)/Lib/AMD/CAL/8.95/include
QT_BUILD_DIR=$(COMMON_DIR)/Lib/Ext/Qt/4.7.4/Build/CentOS6.2
QT_BIN_DIR=$(QT_BUILD_DIR)/$(QT_BUILD_DIR_PLATFORM)/bin
QT_INCLUDE_DIR=$(QT_BUILD_DIR)/$(QT_BUILD_DIR_PLATFORM)/include
QT_LIB_DIR = $(QT_BUILD_DIR)/$(QT_BUILD_DIR_PLATFORM)/lib
QSCINTILLA_DIR=$(COMMON_DIR)/Lib/Ext/QScintilla/2.5.1
QSCINTILLA_BUILD_DIR=$(QSCINTILLA_DIR)/Build/CentOS6.2
QSCINTILLA_INCLUDE_DIR=$(QSCINTILLA_DIR)/Qt4
QSCINTILLA_LIB_DIR = $(QSCINTILLA_BUILD_DIR)/$(QT_BUILD_DIR_PLATFORM)/lib
WX_BUILD_DIR=$(COMMON_DIR)/Lib/Ext/wxWidgets/2.9.2/Build/CentOS6.2
WX_INCLUDE_DIR=$(WX_BUILD_DIR)/$(QT_BUILD_DIR_PLATFORM)/include/wx-2.9
WX_LIB_DIR=$(WX_BUILD_DIR)/$(QT_BUILD_DIR_PLATFORM)/lib
WX_GTK_INCLUDE_DIR=$(WX_LIB_DIR)/wx/include/gtk2-unicode-2.9
TINYXML_DIR=$(COMMON_DIR)/Lib/Ext/tinyxml
TINYXML_BUILD_DIR=$(TINYXML_DIR)/2.5.2/Build/CentOS6.2
TINYXML_INCLUDE_DIR=$(TINYXML_DIR)
TINYXML_LIB_DIR = $(TINYXML_BUILD_DIR)/$(TINYXML_BUILD_DIR_PLATFORM)/lib
AMDTSHAREDPROFILINGDIR=$(COMMON_DIR)/Lib/AMD/AMDTSharedProfiling/0.5/Build/CentOS6.2/$(QT_BUILD_DIR_PLATFORM)
AMDTSHAREDPROFILING_INCLUDE_DIR=$(AMDTSHAREDPROFILINGDIR)/include
AMDTSHAREDPROFILING_LIB_DIR=$(AMDTSHAREDPROFILINGDIR)/lib
AMDTBASETOOLSDIR=$(COMMON_DIR)/Lib/AMD/AMDTBaseTools/1.0
AMDTBASETOOLS_INCLUDE_DIR=$(AMDTBASETOOLSDIR)/Include
AMDTBASETOOLS_LIB_DIR=$(AMDTBASETOOLSDIR)/Lib/$(FRAMEWORK_PLATFORM_DIR)
AMDTOSWRAPPERSDIR=$(COMMON_DIR)/Lib/AMD/AMDTOSWrappers/1.0
AMDTOSWRAPPERS_INCLUDE_DIR=$(AMDTOSWRAPPERSDIR)/Include
AMDTOSWRAPPERS_LIB_DIR=$(AMDTOSWRAPPERSDIR)/Lib/$(FRAMEWORK_PLATFORM_DIR)
AMDTAPICLASSESDIR=$(COMMON_DIR)/Lib/AMD/AMDTAPIClasses/1.0
AMDTAPICLASSES_INCLUDE_DIR=$(AMDTAPICLASSESDIR)/Include
AMDTAPICLASSES_LIB_DIR=$(AMDTAPICLASSESDIR)/Lib/$(FRAMEWORK_PLATFORM_DIR)
AMDTAPPCOMPONENTSDIR=$(COMMON_DIR)/Lib/AMD/AMDTApplicationComponents/1.0
AMDTAPPCOMPONENTS_INCLUDE_DIR=$(AMDTAPPCOMPONENTSDIR)/Include
AMDTAPPCOMPONENTS_LIB_DIR=$(AMDTAPPCOMPONENTSDIR)/Lib/$(FRAMEWORK_PLATFORM_DIR)
AMDTAPPFRAMEWORKDIR=$(COMMON_DIR)/Lib/AMD/AMDTApplicationFramework/2.0
AMDTAPPFRAMEWORK_INCLUDE_DIR=$(AMDTAPPFRAMEWORKDIR)/Include
AMDTAPPFRAMEWORK_LIB_DIR=$(AMDTAPPFRAMEWORKDIR)/Lib/$(FRAMEWORK_PLATFORM_DIR)
AMDTQTCONTROLSDIR=$(COMMON_DIR)/Lib/AMD/AMDTQtControls/1.0
AMDTQTCONTROLS_INCLUDE_DIR=$(AMDTQTCONTROLSDIR)/Include
AMDTQTCONTROLS_LIB_DIR=$(AMDTQTCONTROLSDIR)/Lib/$(PLATFORM_DIR)
LIBS = -lrt -ldl -lpthread $(QT_LIBS) $(WX_LIBS) $(TINYXML_LIBS) $(FRAMEWORK_LIBS) $(QSCINTILLA_LIBS) $(QTCONTROLS_LIBS) $(SHARED_PROFILING_LIBS)
SHARED_CFLAGS = -fPIC -Wall -Wno-unknown-pragmas $(PLATFORM_CFLAG)
CFLAGS = $(OPTIMIZE) $(SHARED_CFLAGS) $(DEBUG_CFLAGS)
DEBUG_CFLAGS =
MOC = $(QT_BIN_DIR)/moc
UIC = $(QT_BIN_DIR)/uic
RCC = $(QT_BIN_DIR)/rcc
MOC_OUTPUT_DIR = GeneratedFiles

QT_LIBS = \
-L$(QT_LIB_DIR) \
-lQtCoreAmdDt474$(DEBUG_SUFFIX_NODASH) \
-lQtGuiAmdDt474$(DEBUG_SUFFIX_NODASH) \
-lQtWebKitAmdDt474$(DEBUG_SUFFIX_NODASH) \
-lQtXmlAmdDt474$(DEBUG_SUFFIX_NODASH)

QSCINTILLA_LIBS = \
-L$(QSCINTILLA_LIB_DIR) \
-lqscintillaAmdDt251$(DEBUG_SUFFIX_NODASH)

WX_LIBS = \
-L$(WX_LIB_DIR) \
-lwx_baseu-2.9 \
-lwx_gtk2u_core-2.9 \
-lwx_gtk2u_html-2.9 \
-lwx_gtk2u_adv-2.9 \
-lwx_gtk2u_aui-2.9 \
-lwx_gtk2u_gl-2.9

FRAMEWORK_LIBS = \
-L$(AMDTBASETOOLS_LIB_DIR) \
-lAMDTBaseTools$(DEBUG_SUFFIX_UNDERSCORE) \
-L$(AMDTAPICLASSES_LIB_DIR) \
-lAMDTAPIClasses$(DEBUG_SUFFIX_UNDERSCORE) \
-L$(AMDTOSWRAPPERS_LIB_DIR) \
-lAMDTOSWrappers$(DEBUG_SUFFIX_UNDERSCORE) \
-L$(AMDTAPPCOMPONENTS_LIB_DIR) \
-lAMDTApplicationComponents$(DEBUG_SUFFIX_UNDERSCORE) \
-L$(AMDTAPPFRAMEWORK_LIB_DIR) \
-lAMDTApplicationFramework$(DEBUG_SUFFIX_UNDERSCORE)

TINYXML_LIBS = \
-L$(TINYXML_LIB_DIR) \
-ltinyXML$(DEBUG_SUFFIX_UNDERSCORE)

QTCONTROLS_LIBS = \
-L$(AMDTQTCONTROLS_LIB_DIR) \
-lAMDTQtControls$(DEBUG_SUFFIX)

SHARED_PROFILING_LIBS = \
-L$(AMDTSHAREDPROFILING_LIB_DIR) \
-lAMDTSharedProfiling$(DEBUG_SUFFIX)

INCLUDES = \
-I. \
-I ./$(MOC_OUTPUT_DIR) \
-I$(TSINGLETON_INCLUDE_DIR) \
-I$(ADLUTIL_DIR) \
-I$(DEVICEINFO_DIR) \
-I$(ADL_INCLUDE_DIR) \
-I$(CAL_INCLUDE_DIR) \
-I$(TINYXML_INCLUDE_DIR) \
-I$(QT_INCLUDE_DIR) \
-I$(QT_INCLUDE_DIR)/QtCore \
-I$(QT_INCLUDE_DIR)/QtGui \
-I$(QT_INCLUDE_DIR)/QtWebKit \
-I$(QT_INCLUDE_DIR)/QtXml \
-I$(QSCINTILLA_INCLUDE_DIR) \
-I$(WX_INCLUDE_DIR) \
-I$(WX_GTK_INCLUDE_DIR) \
-I$(AMDTSHAREDPROFILING_INCLUDE_DIR) \
-I$(AMDTBASETOOLS_INCLUDE_DIR) \
-I$(AMDTAPICLASSES_INCLUDE_DIR) \
-I$(AMDTOSWRAPPERS_INCLUDE_DIR) \
-I$(AMDTAPPCOMPONENTS_INCLUDE_DIR) \
-I$(AMDTAPPFRAMEWORK_INCLUDE_DIR) \
-I$(AMDTQTCONTROLS_INCLUDE_DIR)

MOCFILES =	\
		./$(MOC_OUTPUT_DIR)/moc_AMDTGpuProfilerViewCreator.cpp \
		./$(MOC_OUTPUT_DIR)/moc_CounterSelectionSettingPage.cpp \
		./$(MOC_OUTPUT_DIR)/moc_EditNameValueDialog.cpp \
		./$(MOC_OUTPUT_DIR)/moc_GeneralSettingWindow.cpp \
		./$(MOC_OUTPUT_DIR)/moc_ListViewWindow.cpp \
		./$(MOC_OUTPUT_DIR)/moc_OpenCLTraceSettingPage.cpp \
		./$(MOC_OUTPUT_DIR)/moc_ProfileManager.cpp \
		./$(MOC_OUTPUT_DIR)/moc_ProfileSettingDialog.cpp \
		./$(MOC_OUTPUT_DIR)/moc_SummaryView.cpp \
		./$(MOC_OUTPUT_DIR)/moc_TraceView.cpp \
		./$(MOC_OUTPUT_DIR)/moc_TraceTable.cpp \
		./$(MOC_OUTPUT_DIR)/moc_CodeViewer.cpp \
		./$(MOC_OUTPUT_DIR)/moc_CustomDataTypes.cpp \
		./$(MOC_OUTPUT_DIR)/moc_SessionControl.cpp \
		./$(MOC_OUTPUT_DIR)/moc_SessionWindow.cpp

UIFILES = 	\
		./$(MOC_OUTPUT_DIR)/ui_EditNameValue.h \
		./$(MOC_OUTPUT_DIR)/ui_FindToolBar.h \
		./$(MOC_OUTPUT_DIR)/ui_ListViewDialog.h \
		./$(MOC_OUTPUT_DIR)/ui_ProfileSetting.h \
		./$(MOC_OUTPUT_DIR)/ui_SummaryViewBase.h \

QRCFILES = 	\
		./$(MOC_OUTPUT_DIR)/rcc_QtClientMain.qrc \


OBJS =	\
	./$(OBJ_DIR)/AMDTGpuProfilerPlugin.o \
	./$(OBJ_DIR)/AMDTGpuProfilerViewCreator.o \
	./$(OBJ_DIR)/APIColorMap.o \
	./$(OBJ_DIR)/CLAPIDefs.o \
	./$(OBJ_DIR)/CLAPIFilterManager.o \
	./$(OBJ_DIR)/CLPerfMarkerParser.o \
	./$(OBJ_DIR)/CLSummarizer.o \
	./$(OBJ_DIR)/CLTimelineItems.o \
	./$(OBJ_DIR)/CodeViewerControl.o \
	./$(OBJ_DIR)/CounterGroup.o \
	./$(OBJ_DIR)/CounterManager.o \
	./$(OBJ_DIR)/CounterSelectionSettingPage.o \
	./$(OBJ_DIR)/CustomDataTypes.o \
	./$(OBJ_DIR)/EditNameValueDialog.o \
	./$(OBJ_DIR)/FindToolBarView.o \
	./$(OBJ_DIR)/GeneralSettingWindow.o \
	./$(OBJ_DIR)/GlobalSettings.o \
	./$(OBJ_DIR)/GPUProjectHandler.o \
	./$(OBJ_DIR)/KernelOccupancyWindow.o \
	./$(OBJ_DIR)/ListViewWindow.o \
	./$(OBJ_DIR)/OccupancyInfo.o \
	./$(OBJ_DIR)/OpenCLTraceSettingPage.o \
	./$(OBJ_DIR)/OptionsWindow.o \
	./$(OBJ_DIR)/ProfileManager.o \
	./$(OBJ_DIR)/ProfileParam.o \
	./$(OBJ_DIR)/ProfileProcessMonitor.o \
	./$(OBJ_DIR)/ProfileSettingData.o \
	./$(OBJ_DIR)/ProfileSettingDialog.o \
	./$(OBJ_DIR)/Project.o \
	./$(OBJ_DIR)/ProjectSettings.o \
	./$(OBJ_DIR)/QCLCounterNamesEvergreen.o \
	./$(OBJ_DIR)/QCLCounterNamesNorthernIslands.o \
	./$(OBJ_DIR)/QCLCounterNamesSouthernIslands.o \
	./$(OBJ_DIR)/SessionControl.o \
	./$(OBJ_DIR)/Session.o \
	./$(OBJ_DIR)/SessionManager.o \
	./$(OBJ_DIR)/SessionWindow.o \
	./$(OBJ_DIR)/SessionViewTabWidget.o \
	./$(OBJ_DIR)/SummaryView.o \
	./$(OBJ_DIR)/SymbolInfo.o \
	./$(OBJ_DIR)/TraceTable.o \
	./$(OBJ_DIR)/TraceView.o \
	./$(OBJ_DIR)/Util.o \
	./$(OBJ_DIR)/ADLUtil.o \
	./$(OBJ_DIR)/DeviceInfo.o \
	./$(OBJ_DIR)/DeviceInfoUtils.o \
	./$(OBJ_DIR)/moc_AMDTGpuProfilerViewCreator.o \
	./$(OBJ_DIR)/moc_CounterSelectionSettingPage.o \
	./$(OBJ_DIR)/moc_EditNameValueDialog.o \
	./$(OBJ_DIR)/moc_GeneralSettingWindow.o \
	./$(OBJ_DIR)/moc_ListViewWindow.o \
	./$(OBJ_DIR)/moc_OpenCLTraceSettingPage.o \
	./$(OBJ_DIR)/moc_ProfileManager.o \
	./$(OBJ_DIR)/moc_ProfileSettingDialog.o \
	./$(OBJ_DIR)/moc_SummaryView.o \
	./$(OBJ_DIR)/moc_TraceView.o \
	./$(OBJ_DIR)/moc_TraceTable.o \
	./$(OBJ_DIR)/moc_CodeViewer.o \
	./$(OBJ_DIR)/moc_CustomDataTypes.o \
	./$(OBJ_DIR)/moc_SessionControl.o \
	./$(OBJ_DIR)/moc_SessionWindow.o \
	./$(OBJ_DIR)/rcc_QtClientMain.o

$(TARGET) : createdir $(MOCFILES) $(UIFILES) $(RCCFILES) $(OBJS)
	$(CC) $(LINKFLAGS) $(OBJS) $(LIBS) -o $(TARGET) $(ELF_LIB)

DEBUG:
	$(MAKE) "LIB_NAME = libAMDTGPUProfiling-d.so" "DEBUG_CFLAGS = -g -O0 -D_DEBUG -D__WXDEBUG__" "OPTIMIZE = " "RELEASE_LIBS = $(DEBUG_LIBS)" "OBJ_DIR = OBJx64DEBUG" "QT_BUILD_DIR_PLATFORM = x86_64/debug" "TINYXML_BUILD_DIR_PLATFORM = debug/x86_64" "DEBUG_SUFFIX= -d" "DEBUG_SUFFIX_NODASH = d" "DEBUG_SUFFIX_UNDERSCORE = _d"

x86DEBUG:
	$(MAKE) "LIB_NAME = libAMDTGPUProfiling-d.so" "PLATFORM_DIR = x86" "FRAMEWORK_PLATFORM_DIR = x86" "PLATFORM_CFLAG = -m32" "DEBUG_CFLAGS = -g -O0 -D_DEBUG -D__WXDEBUG__" "PLATFORM_LINKFLAG = -m32" "PLATFORM = -DX86" "OPTIMIZE = " "RELEASE_LIBS = $(DEBUG_LIBS)" "OBJ_DIR = OBJx86DEBUG" "QT_BUILD_DIR_PLATFORM = x86/debug" "TINYXML_BUILD_DIR_PLATFORM = debug/x86" "DEBUG_SUFFIX= -d" "DEBUG_SUFFIX_NODASH = d" "DEBUG_SUFFIX_UNDERSCORE = _d"

x86:
	$(MAKE) "PLATFORM_DIR = x86" "FRAMEWORK_PLATFORM_DIR = x86" "PLATFORM_CFLAG = -m32" "PLATFORM_LINKFLAG = -m32" "PLATFORM = -DX86" "OBJ_DIR = OBJx86" "QT_BUILD_DIR_PLATFORM = x86/release" "TINYXML_BUILD_DIR_PLATFORM = release/x86"

all:
	$(MAKE)
	$(MAKE) x86
	$(MAKE) DEBUG
	$(MAKE) x86DEBUG

createdir:
	mkdir -p $(MOC_OUTPUT_DIR) $(OBJ_DIR) $(BIN_DIR)

$(OBJ_DIR)/%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) $(BUILD_DEF) $(PLATFORM) -c $< -o $@

$(OBJ_DIR)/%.o: $(MOC_OUTPUT_DIR)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) $(BUILD_DEF) $(PLATFORM) -c $< -o $@

$(OBJ_DIR)/%.o: $(ADLUTIL_DIR)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) $(BUILD_DEF) $(PLATFORM) -c $< -o $@

$(OBJ_DIR)/%.o: $(DEVICEINFO_DIR)/%.cpp
	$(CC) $(CFLAGS) $(INCLUDES) $(DEFINES) $(BUILD_DEF) $(PLATFORM) -c $< -o $@

./$(MOC_OUTPUT_DIR)/moc_%.cpp: %.h
	$(MOC) $(BUILD_DEF) $(DEFINES) -I. -I$(QT_INCLUDE_DIR) $< -o $@

./$(MOC_OUTPUT_DIR)/ui_%.h: %.ui
	$(UIC) $< -o $@

./$(MOC_OUTPUT_DIR)/rcc_%.cpp: %.qrc
	$(RCC) -o $@ $< 

clean:
	rm -f $(MOCFILES)
	rm -f $(OBJS)

cleandebug:
	$(MAKE) clean "OBJ_DIR = OBJx64DEBUG"

cleanx86:
	$(MAKE) clean "OBJ_DIR = OBJx86"

cleanx86debug:
	$(MAKE) clean "OBJ_DIR = OBJx86DEBUG"

cleanall:
	$(MAKE) clean
	$(MAKE) cleanx86
	$(MAKE) cleandebug
	$(MAKE) cleanx86debug

# END OF MAKE FILE
