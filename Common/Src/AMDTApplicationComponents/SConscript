# -*- Python -*-

import os

Import('*')
from CXL_init import *

libName = "CXLApplicationComponents"

env = CXL_env.Clone()

env.Append( CPPPATH = [ 
    "./",
    "./inc",
    env['CXL_commonproj_dir'],
    env['CXL_commonproj_dir'] + "/AMDTOSAPIWrappers/Include",
    env['CXL_commonproj_dir'] + '/../../CodeXL',
    "/usr/include/gdk-pixbuf-2.0/", # [Suravee] Needed for Ubuntu-11.10 
])


UseGtk(env)
UseQScintilla(env)
UseQt4(env)
UseAPPSDK(env)
UseQCustomPlot(env)

moc_files = Split(
                " Include/acListCtrl.h"
                + " Include/acBarsGraph.h"
                + " Include/acChartWindow.h"
                + " Include/acCustomPlot.h"
                + " Include/acDataView.h"
                + " Include/acDataViewGridTable.h"
                + " Include/acDoubleSlider.h"
                + " Include/acFrozenColumnTreeView.h"
                + " Include/acHeaderView.h"
                + " Include/acImageManager.h"
                + " Include/acImageManagerModel.h"
                + " Include/acImageView.h"
                + " Include/acLineEdit.h"
                + " Include/acFindWidget.h"
                + " Include/acSourceCodeView.h"
                + " Include/acSplitter.h"
                + " Include/acTabWidget.h"
                + " Include/acNavigationChart.h"
                + " Include/acMultiLinePlot.h"
                + " Include/acRibbonManager.h"
                + " Include/acToolBar.h"
                + " Include/acQHTMLWindow.h"
                + " Include/acQTextFilterCtrl.h"
                + " Include/acTreeCtrl.h"
                + " Include/Timeline/acTimeline.h"
                + " Include/Timeline/acTimelineBranch.h"
                + " Include/Timeline/acTimelineGrid.h"
                + " Include/Timeline/acTimelineItem.h"
                + " Include/Timeline/acTimelineItemCurve.h"
                + " Include/Timeline/acTimelineFiltersDialog.h"
                + " Include/acVectorLineGraph.h"
                + " Include/acVirtualListCtrl.h"
                + " Include/acVirtualListCtrlModel.h"
                + " Include/acProgressDlg.h"
                + " Include/acProgressAnimationWidget.h"
                + " Include/acSendErrorReportDialog.h"
                + " Include/acSoftwareUpdaterWindow.h"
                + " Include/acSoftwareUpdaterProxySetting.h"
                + " Include/acThumbnailView.h"
                + " Include/acHelpAboutDialog.h"
                + " Include/acEulaDialog.h"
                + " Include/acGoToLineDialog.h"
                )

qrc_files = [ "Include/res/icons/acIcons.qrc" ]

# Source files:
sources = \
[
    "src/acBarsGraph.cpp",
    "src/acChartWindow.cpp",
    "src/acColours.cpp",
    "src/acColoredBarsGraph.cpp",
    "src/acColorSampleBox.cpp",
    "src/acCustomPlot.cpp",
    "src/acDataView.cpp",
    "src/acDataViewItem.cpp",
    "src/acDataViewGridTable.cpp",
    "src/acDialog.cpp",
    "src/acDisplay.cpp",
    "src/acDoubleSlider.cpp",
    "src/acEulaDialog.cpp",
    "src/acFindParameters.cpp",
    "src/acFindWidget.cpp",
    "src/acFrozenColumnTreeView.cpp",
    "src/acFunctions.cpp",
    "src/acGoToLineDialog.cpp",
    "src/acGroupedBarsGraph.cpp",
    "src/acHeaderView.cpp",
    "src/acHelpAboutDialog.cpp",
    "src/acIcons.cpp",
    "src/acImageDataProxy.cpp",
    "src/acImageItem.cpp",
    "src/acImageItemDelegate.cpp",
    "src/acImageManager.cpp",
    "src/acImageManagerModel.cpp",
    "src/acImageView.cpp",
    "src/acItemDelegate.cpp",
    "src/acLineEdit.cpp",
    "src/acListCtrl.cpp",
    "src/acMessageBox.cpp",
    "src/acMultiLinePlot.cpp",
    "src/acQCPColoredBars.cpp",
    "src/acQHTMLWindow.cpp",
    "src/acQMessageDialog.cpp",
    "src/acNavigationChart.cpp",
    "src/acProgressAnimationWidget.cpp",
    "src/acProgressDlg.cpp",
    "src/acQTextFilterCtrl.cpp",
    "src/acRawDataExporter.cpp",
    "src/acRawFileHandler.cpp",
    "src/acRibbonManager.cpp",
    "src/acSendErrorReportDialog.cpp",
    "src/acSoftwareUpdaterProxySetting.cpp",
    "src/acSoftwareUpdaterWindow.cpp",
    "src/acSourceCodeLanguageHighlighter.cpp",
    "src/acSourceCodeLexers.cpp",
    "src/acSourceCodeView.cpp",
    "src/acStackedBarGraph.cpp",
    "src/acSplitter.cpp",
    "src/acTabWidget.cpp",
    "src/acThumbnailView.cpp",
    "src/acTimeline.cpp",
    "src/acTimelineBranch.cpp",
    "src/acTimelineFiltersDialog.cpp",
    "src/acTimelineGrid.cpp",
    "src/acTimelineItem.cpp",
    "src/acTimelineItemCurve.cpp",
    "src/acToolBar.cpp",
    "src/acTreeCtrl.cpp",
    "src/acValidators.cpp",
    "src/acVectorLineGraph.cpp",
    "src/acVirtualListCtrl.cpp",
    "src/acVirtualListCtrlModel.cpp",
]

commonLinkedLibraries = \
[
    "CXLBaseTools",
    "CXLOSWrappers",
    "CXLOSAPIWrappers",
    "CXLAPIClasses"
]

linuxVariantLinkedLibraries = \
[
    "libGL", "libgtk-x11-2.0"

]

linkedLibraries = commonLinkedLibraries + linuxVariantLinkedLibraries
env.Prepend ( LIBS = linkedLibraries )

# Set the ELF hash generation mode:
# - When building on new systems, we would like to generate both sysv and gnu ELF hashes.
#   This enables running the executable also on old systems, that support only the sysv ELF hash.
# - When building on old systems, we need to set the GR_GENERATE_ONLY_DEFAULT_ELF_HASH environment
#   variable (preferably in the .bashrc file). Otherwise the link will fail when trying to
#   generate an ELF hash type that the old linker does not recognize.
# [Yaki 7/7/2009]
linkerFlags = [] 
shouldGenerateOnlyDefaultELFHash = os.environ.get('GR_GENERATE_ONLY_DEFAULT_ELF_HASH')
if shouldGenerateOnlyDefaultELFHash is None:
    linkerFlags += [ "-Wl,--hash-style=both" ]

MOC_Generated = []
for moc_file in moc_files:
    MOC_Generated += env.MocBld(moc_file)

QRC_Generated = []
for qrc_file in qrc_files:
    QRC_Generated += env.RccBld(qrc_file)

# Creating shared libraries
soFiles = env.SharedLibrary(
	target = libName, 
	source = sources + MOC_Generated + QRC_Generated,
	LINKFLAGS = linkerFlags)

# Installing libraries
libInstall = env.Install( 
    dir = env['CXL_lib_dir'], 
    source = (soFiles))

Return('libInstall')
