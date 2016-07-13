//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CommandIds.h
///
//==================================================================================

// CommandIds.h
// Command IDs used in defining command bars
//

// do not use #pragma once - used by ctc compiler
#ifndef __COMMANDIDS_H_
#define __COMMANDIDS_H_

///////////////////////////////////////////////////////////////////////////////
// Menu IDs

#define IDM_TLB_IMAGES_BUFFERS 0x0001                   // images & buffers toolbar
#define IDMX_RTF               0x0002                   // context menu
#define IDM_TLB_CURRENT_WORK_ITEM 0x0003                // current work item toolbar
#define IDM_RTFMNU_ALIGN       0x0004
#define IDM_RTFMNU_SIZE        0x0005
#define IDM_TLB_VIEWERS        0x0006                   // Viewers toolbar
#define IDM_TLB_KERNEL_ANALYZER 0x0007              // current work item toolbar



///////////////////////////////////////////////////////////////////////////////
// Menu Group IDs


#define IDG_IMAGES_TLB_ZOOM            0x1001
#define IDG_IMAGES_TLB_SIZE            0x1002
#define IDG_IMAGES_TLB_SELECT          0x1003
#define IDG_IMAGES_TLB_ROTATE          0x1004
#define IDG_IMAGES_TLB_CHANNELS        0x1005
#define IDG_IMAGES_TLB_INVERT          0x1008
#define IDG_CWI_TLB_SELECTION          0x1009
#define IDG_GDB_TLB_LAUNCH_DEBUGGING   0x1010
#define IDG_KA_TLB_SELECTION           0x1011
#define IDG_MODES_TLB_EXPLORER_GROUP   0x1012
#define IDG_IMAGES_TLB_EXPLORER_GROUP  0x1013
#define IDG_CWI_TLB_EXPLORER_GROUP     0x1014
#define IDG_KA_TLB_EXPLORER_GROUP      0x1015

#define cmdidCodeXLMenu                         0x1021
#define cmdidCodeXLStartGroup                   0x1022
#define cmdidCodeXLModesGroup                   0x1023
#define cmdidCodeXLModesGroupTLB                0x1024
#define cmdidCodeXLDebuggingActionsGroup        0x1025
#define cmdidCodeXLDebuggingBreakpointsGroup    0x1026
#define cmdidCodeXLProfileSessionTypesGroup     0x1027
#define cmdidCodeXLGeneralItemsGroup            0x1028
#define cmdidCodeXLViewsGroup                   0x1029
#define cmdidCodeXLWindowsMenu                  0x102a
#define cmdidCodeXLWindowsMenuGroup             0x102b
#define cmdidCodeXLAnalyzeMenuGroup             0x102c
#define cmdidCodeXLHelpGroup                    0x102d
#define cmdidCodeXLHelpMenuGroup                0x102e
#define cmdidCodeXLHelpMenu                     0x102f
#define cmdidCodeXLAboutMenuGroup               0x1030
#define cmdidCodeXLHelpSupportGroup             0x1031
#define cmdidCodeXLProfilingActionsGroup        0x1032
#define cmdidCodeXLProfileDropdownMenu          0x1033
#define cmdidCodeXLProfileDropdownMenuGroup     0x1034
#define cmdidCodeXLKAItemsTLBGroup1             0x1035
#define cmdidCodeXLKAItemsTLBGroup2             0x1036
#define cmdidCodeXLKAItemsTLBGroup3             0x1037
#define cmdidCodeXLKAItemsTLBKernelsGroup       0x1038
#define cmdidCodeXLKAItemsTLBEntryGroup         0x1039
#define cmdidCodeXLKAItemsTLBShaderModelGroup   0x103a
#define cmdidCodeXLKAItemsMenuGroup             0x103b
#define cmdidCodeXLWindowsToolbarGroup          0x103c
#define cmdidCodeXLPPItemsMenuGroup             0x103d
#define cmdidCodeXLHostsGroup                   0x103e

#define cmdidCodeXLDebuggingBreakpointsMenu  0x1050
#define cmdidCodeXLDebuggingBreakpointsMenuGroup 0x1051

#define cmdidCodeXLHostsDropdownMenu 0x1052
#define cmdidCodeXLHostsDropdownMenuGroup 0x1053


///////////////////////////////////////////////////////////////////////////////
// Command IDs
#define cmdidLaunchOpenCLDebugging 0x201
#define cmdidBreakProfiling 0x202
#define cmdidStopProfiling 0x203
#define cmdidAttachProfiling 0x204
#define cmdidConfigureRemoteHost 0x206
#define cmdidStartButtonOnToolbar 0x207

#define cmdidDebugMode 0x301
#define cmdidProfileMode 0x302
#define cmdidFrameAnalysis 0x303
#define cmdidAnalyzeMode 0x304

#define cmdidAPIStep    0x400
#define cmdidFrameStep  0x401
#define cmdidDrawStep   0x402
#define cmdidGDStepOver 0x403
#define cmdidGDStepInto 0x404
#define cmdidGDStepOut  0x405

#define cmdidOpenCLBreakpoints 0x501
#define cmdidOpenPPCounters 0x502

#define cmdidCpuAssessPerformanceProfiling 0x601
#define cmdidCpuIbsProfiling 0x602
#define cmdidCpuL2AccessProfiling 0x603
#define cmdidCpuBranchAccessProfiling 0x604
#define cmdidCpuDataAccessProfiling 0x605
#define cmdidCpuInstructionAccessProfiling 0x606
#define cmdidCpuTimerBasedProfiling 0x607
#define cmdidGpuPerformanceCounterProfiling 0x611
#define cmdidGpuApplicationTraceProfiling 0x612
#define cmdidCpuCustomProfiling 0x613
#define cmdidCpuCacheLineUtilizationProfiling 0x614
#define cmdidPPOnlineProfiling 0x615

#define cmdidSystemInformation 0x701
#define cmdidCodeXLProjectSettings 0x702
#define cmdidOptions 0x703

#define cmdidCodeXLExplorer 0x801
#define cmdidCallsHistoryViewer 0x802
#define cmdidPropertiesView 0x803
#define cmdidMemoryView 0x804
#define cmdidStatisticsView 0x805
#define cmdidOpenCLMultiWatch1 0x806
#define cmdidOpenCLMultiWatch2 0x807
#define cmdidOpenCLMultiWatch3 0x808
#define cmdidAddOpenCLMultiWatchFromSourceCode 0x809
#define cmdidAddOpenCLMultiWatchFromWatchView 0x80a
#define cmdidAddOpenCLMultiWatchFromLocalsView 0x80b
#define cmdidStateVariablesView 0x80c


#define cmdidHelpOpenTeapotSample 0xb01
#define cmdidHelpOpenMatMulSample 0xb02
#define cmdidHelpOpenD3D12MultithreadingSample 0xb03
#define cmdidViewHelp 0xb04
#define cmdidViewQuickStart 0xb05
#define cmdidHelpUpdates 0xb06
#define cmdidHelpGPUForums 0xb07
#define cmdidAboutDialog 0xb08

#define cmdidOpenCLBuild 0xc01
#define cmdidAddOpenCLFile 0xc02
#define cmdidOpenDeviceOptions 0xc03
#define cmdidOpenAnalyzeSettings 0xc04
#define cmdidCancelOpenCLBuild 0xc05
#define cmdidCreateSourceFile 0xc06

#define cmdidRefreshSessions 0xd01
#define cmdidCaptureFrame 0xd02
#define cmdidCaptureFrameGPU 0xd03
#define cmdidCaptureFrameCPU 0xd04

// Images and buffers tools:
#define commandIDBestFit                0x2001
#define commandIDOrigSize               0x2002
#define commandIDSelect                 0x2003
#define commandIDPan                    0x2004
#define commandIDRotateLeft             0x2005
#define commandIDRotateRight            0x2006
#define commandIDChannelRed             0x2007
#define commandIDChannelGreen           0x2008
#define commandIDChannelBlue            0x2009
#define commandIDChannelAlpha           0x200a
#define commandIDChannelInvert          0x200b
#define commandIDChannelGrayscale       0x200c
#define commandIDImageSizeCombo         0x200d
#define commandIDImageSizeComboGetList  0x200e
#define commandIDWorkItemXCoordCombo    0x200f
#define commandIDWorkItemXCoordComboGetList 0x2010
#define commandIDWorkItemYCoordCombo    0x2011
#define commandIDWorkItemYCoordComboGetList 0x2012
#define commandIDWorkItemZCoordCombo    0x2013
#define commandIDWorkItemZCoordComboGetList 0x2014
#define commandIDOptionsCombo               0x2015
#define commandIDOptionsComboGetList        0x2016
#define commandIDBitnessCombo               0x2017
#define commandIDBitnessComboGetList        0x2018
#define commandIDDXShaderModelCombo         0x2019
#define commandIDDXShaderModelComboGetList  0x201a
#define commandIDDXShaderTypeCombo          0x201b
#define commandIDDXShaderTypeComboGetList   0x201c
#define commandIDKernelNameCombo            0x201d
#define commandIDKernelNameComboGetList     0x201e



///////////////////////////////////////////////////////////////////////////////
// Bitmap IDs

// Images and buffers toolbar Bitmap IDs
#define bmpRed                  1
#define bmpGreen                2
#define bmpBlue                 3
#define bmpAlpha                4
#define bmpInvert               5
#define bmpGrayscale            6
#define bmpLeftRotate           7
#define bmpRightRotate          8
#define bmpBestFit              9
#define bmpOrigSize             10
#define bmpPan                  11
#define bmpSelect               12

// Menu Command Bitmap IDs
#define bmpDebugMode            1
#define bmpProfileMode          2
#define bmpFrameAnalysisMode    3
#define bmpAnalyzeMode          4
#define bmpGo                   5
#define bmpPause                6
#define bmpStop                 7
#define bmpCapture              8
#define bmpCaptureCPU           9
#define bmpCaptureGPU           10
#define bmpStepIn               11
#define bmpStepOver             12
#define bmpAPIStep              13
#define bmpDrawStep             14
#define bmpFrameStep            15
#define bmpExplorer             16
#define bmpHistory              17
#define bmpProperties           18
#define bmpMultiWatch           19
#define bmpStateVars            20
#define bmpCommandQueues        21
#define bmpMemory               22
#define bmpStatistics           23
#define bmpSystemInfo           24
#define bmpBreakpoint           25
#define bmpViewHelp             26
#define bmpBuild                27
#define bmpBuildAna             28
#define bmpCancelBuild          29


#endif // __COMMANDIDS_H_
