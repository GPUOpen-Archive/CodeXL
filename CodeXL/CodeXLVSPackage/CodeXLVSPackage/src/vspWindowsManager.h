//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspWindowsManager.h
///
//==================================================================================

//------------------------------ vspWindowsManager.h ------------------------------

#ifndef __VSPWINDOWSMANAGER_H
#define __VSPWINDOWSMANAGER_H

// Core interfaces:
#include <Include/Public/CoreInterfaces/IVscWindowsManagerOwner.h>

// Qt:
#include <QWidget>

#include <AMDTOSWrappers/Include/osFilePath.h>

// Infra:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/dialogs/afSystemInformationDialog.h>

// VS:
#include <vsshell.h>

// Forward declarations:
class gdImagesAndBuffersViewer;
class gdStatisticsPanel;
class gdAPICallsHistoryPanel;
class gdImageAndBufferView;
class afApplicationTree;
class gdMemoryView;
class gdStateVariablesView;
class gdMultiWatchView;
class afPropertiesView;
class SessionExplorerWindow;
class QDialog;


#define VSP_AMOUNT_OF_MULTIWATCH_VIEWS 3
// ----------------------------------------------------------------------------------
// Class Name:          vspWindowsManager
// General Description: This class is a single tone class that is handling the creation
//                      deletion and use of CodeXL dialogs while running in VS package
// Author:              Sigal Algranaty
// Creation Date:       21/9/2010
// ----------------------------------------------------------------------------------
class vspWindowsManager
{
public:
    static vspWindowsManager& instance();
    virtual ~vspWindowsManager();

    // Sets the owner in terms of interaction with VS specific code.
    static void setOwner(IVscWindowsManagerOwner* pOwner);

    // Manage the UI Shell interface:
    void setUIShell(IVsUIShell* piUIShell);
    IVsUIShell* getUIShell() {return _piUIShell;};

    // Update a requested window size:
    bool updateViewSize(int gdWindowCommandID, QSize viewSize);

    // Update the shown / hidden status for one of the multi views:
    void updateMultiwatchViewShowStatus(int windowID, bool isShown);
    int findFirstHiddenMultiWatchViewIndex();

    // Show dialog functions:
    void showOptionsDialog();
    void showSystemInformationDialog(afSystemInformationDialog::InformationTabs selectedTab = afSystemInformationDialog::SYS_INFO_SYSTEM);
    void showAboutDialog();
    void showCheckForUpdateDialog();
    void viewHelp();
    void viewQuickStartGuide();

    /// Opens CodeXL sample with the requested sample id:
    /// \sampleId the requested id (enumeration)
    void OpenSample(afCodeXLSampleID sampleId);

    int showModal(QDialog* dialogWindow, bool show = true);

    /// Display a Message box according to type: question, warning, critical or information
    QMessageBox::StandardButton ShowMessageBox(QMessageBox::Icon type, const QString& title, const QString& text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton);

    // Create WX views methods:
    afApplicationTree* monitoredObjectsTree(QWidget* pParentWindow, QSize viewSize);
    gdMultiWatchView* multiwatchView(QWidget* pParentWindow, QSize viewSize, int windowID);

    // Create QT views methods:
    gdMemoryView* memoryView(QWidget* pParentWindow, QSize viewSize);
    gdStateVariablesView* stateVariablesView(QWidget* pParentWindow, QSize viewSize);
    afPropertiesView* propertiesView(QWidget* pParentWidget, QSize viewSize);
    gdStatisticsPanel* statisticsPanel(QWidget* pParentWidget, QSize viewSize);
    gdAPICallsHistoryPanel* callsHistoryPanel(QWidget* pParentWindow, QSize viewSize);

    // Was view created?
    bool wasAPICallsHistoryViewCreated() const {return (_pAPICallsHistoryPanel != NULL);};
    bool wasPropertiesViewCreated() const {return (_pPropertiesView != NULL);};
    bool wasMemoryViewCreated() const {return (_pMemoryView != NULL);};
    bool wasStateVariablesViewCreated() const { return (_pStateVariablesView != NULL); };
    bool wasStatisticsViewCreated() const {return (_pStatisticsPanel != NULL);};
    bool wasMonitoredObjectsTreeViewCreated() const {return (_pMonitoredObjectsTree != NULL);};

    void updateMultiWatchViewHexMode(bool hexMode);

    int commandIdFromWidget(QWidget* pWidget);

    VsWindowsManagementMode GetVsWindowsManagementModeFromOwner() const ;

private:
    friend class vspSingletonsDelete;

private:
    // Only my instance() method should create my single instance:
    vspWindowsManager();

    // Utilities:
    void createViewers();
private:

    static IVscWindowsManagerOwner* _pOwner;

    // Single instance static member:
    static vspWindowsManager* _pMySingleInstance;

    // Views:
    gdAPICallsHistoryPanel* _pAPICallsHistoryPanel;
    afPropertiesView* _pPropertiesView;
    afApplicationTree* _pMonitoredObjectsTree;
    gdStatisticsPanel* _pStatisticsPanel;
    gdMemoryView* _pMemoryView;
    gdStateVariablesView* _pStateVariablesView;
    gdMultiWatchView* _pMultiWatchViews[VSP_AMOUNT_OF_MULTIWATCH_VIEWS];

    // dialog stack to maintain modal control when moving from dialog to dialog
    gtList<QDialog*> mModalDialogList;

    // UI Shell interface:
    IVsUIShell* _piUIShell;
};


#endif //__VSPWINDOWSMANAGER_H

