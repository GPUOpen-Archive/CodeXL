//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afSystemInformationDialog.h
///
//==================================================================================

#ifndef __GDSYSTEMINFORMATIONDIALOG
#define __GDSYSTEMINFORMATIONDIALOG

// Qt:
#include <QtWidgets>

// Forward decelerations:
class gdSystemInformationNotebook;
class afOpenCLDeviceInformationCollector;

// Infra:
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTApplicationComponents/Include/acListCtrl.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:           afSystemInformationDialog : public QDialog
// General Description:
// Author:               Avi Shapira
// Creation Date:        9/11/2003
// ----------------------------------------------------------------------------------
class AF_API afSystemInformationDialog : public QDialog
{
    Q_OBJECT

public:
    // Enumerates the available information tabs:
    enum InformationTabs
    {
        SYS_INFO_SYSTEM,
        SYS_INFO_DISPLAY,
        SYS_INFO_GRAPHIC_CARD,

        // Pixel formats are excluded on Mac:
#if !((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        SYS_INFO_PIXEL_FORMAT,
#endif

        SYS_INFO_OPENGL_EXTENSION,
        SYS_INFO_OPENCL_PLATFORMS,
        SYS_INFO_OPENCL_DEVICES
    };

public:
    afSystemInformationDialog(QWidget* parent, InformationTabs selectTab = SYS_INFO_SYSTEM);
    virtual ~afSystemInformationDialog();

protected slots:
    void onNotebookChangePage(int pageIndex);
    void onListCtrlKeyDown(QKeyEvent* pKeyEvent);
    void onSysInfoSaveButton();
    void onButtonClick(QAbstractButton* pButton);

private:
    void setDialogLayout();
    void GetStringSelection();

    void fillSysInfoDataIntoListCtrl(const gtList< gtList<gtString> >& sysInfoData, acListCtrl* pListCtrl, bool columnAutoResize = true, int minColumnWidth = 0);
    void displayErrorMessage(const QString& errorMsg, acListCtrl* pListCtrl);
    void sortByInteger(int l, int r, int columnIndex);
    void replaceListItems(int line1Index, int line2Index);
    long getItemAsLong(int lineIndex, int columnIndex);
    gtString getItemAsText(int lineIndex, int columnIndex);
    void addSystemInfoIntoOutputString(gtList< gtList <gtString> > infoList, int fieldWidth, bool trimLines = false);
    bool saveSystemInfoFile(const osFilePath& filePath);
    /// uses _openCLDevicesInfoData to load list in open CL device tab
    /// \param[in] errMsg error message to be displayed if no data to display (_openCLDevicesInfoData is empty)
    void LoadOpenCLDeviceInformation(QString errMsg);
    /// Implemented from QObject, function is called when the timer is up
    virtual void timerEvent(QTimerEvent* event);
    /// Stop timer and reset relevant data members
    void StopOpenCLDeviceInformationTimer();

private:

    QTabWidget* _pNotebookInfo;
    acListCtrl* _pCurrentShownListCtrl;
    QLabel* _pNotebookDescription;
    QPushButton* m_pSaveButton;

    // Dialog buttons:
    gtString _sysInfoSaveOutputString;

    // System pane:
    acListCtrl* _pListCtrlSystem;
    bool _systemInfoFirstTime;
    bool _systemInfoFirstTimeToFile;
    gtList< gtList <gtString> > _systemInfoData;
    gtList< gtList <gtString> > _systemInfoDataFile;

    // Display pane:
    acListCtrl* _pListCtrlDisplay;
    bool _displayInfoFirstTime;
    bool _displayInfoFirstTimeToFile;
    gtList< gtList <gtString> > _displayInfoData;
    gtList< gtList <gtString> > _displayInfoDataFile;

    // OpenGL Renderer pane:
    acListCtrl* _pListCtrlOpenGLRenderer;
    bool _openGLRendererInfoFirstTime;
    gtList< gtList <gtString> > _openGLRendererInfoData;
    gtList< gtList <gtString> > _openGLRendererInfoDataFile;

    // Pixel Formats pane:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    // This pane is excluded from CodeXL Mac since
    // CGL pixel formats are objects created and released by need
    // and cannot, by consequence, be enumerated or recreated.
#else
    acListCtrl* _pListCtrlPixelFormats;
    bool _pixelFormatInfoFirstTime;
    gtList< gtList <gtString> > _pixelFormatInfoData;
    gtList< gtList <gtString> > _pixelFormatInfoDataFile;
#endif

    // OpenGL Extension pane:
    acListCtrl* _pListCtrlOpenGLExtensions;
    bool _openGLExtInfoFirstTime;
    gtList< gtList <gtString> > _openGLExtInfoData;
    gtList< gtList <gtString> > _openGLExtInfoDataFile;

    // OpenCL Platforms pane:
    acListCtrl* _pListCtrlOpenCLPlatforms;
    bool _openCLPlatformsInfoFirstTime;
    bool _openCLPlatformsInfoFirstTimeToFile;
    gtList< gtList <gtString> > _openCLPlatformsInfoData;
    gtList< gtList <gtString> > _openCLPlatformsInfoDataFile;

    // OpenCL Devices pane:
    acListCtrl* _pListCtrlOpenCLDevices;
    bool _openCLDevicesInfoFirstTime;
    bool _openCLDevicesInfoFirstTimeToFile;
    gtList< gtList <gtString> > _openCLDevicesInfoData;
    gtList< gtList <gtString> > _openCLDevicesInfoDataFile;

    // The user requested information tab:
    InformationTabs _selectedInformationTab;

    /// Data members related to running a thread to collect open cl device information
    int m_openCLDeviceInfoCollectorTimerId;
    int m_OpenCLDeviceInfoCollectorThreadRunTime;
    /// when true the list is empty and set to display message to user
    bool m_ListCtrlOpenCLDevicesOnMsgMode;
    /// Thread used to collect open cl device information
    afOpenCLDeviceInformationCollector* m_pOpenCLDeviceInfoCollectorThread;

};


#endif  // __GDSYSTEMINFORMATIONDIALOG
