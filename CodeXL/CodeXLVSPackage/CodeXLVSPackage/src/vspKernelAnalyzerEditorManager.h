//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspKernelAnalyzerEditorManager.h
///
//==================================================================================

//------------------------------ vspKernelAnalyzerEditorManager.h ------------------------------

#ifndef __VSPKERNELANAYLZEREDITORMANAGER_H
#define __VSPKERNELANAYLZEREDITORMANAGER_H

// Qt
#include <QtWidgets>

class vspSingletonsDelete;
class vspKernelAnalyzerEditorDocument;
class QTreeWidgetItem;
class afApplicationCommands;
class afApplicationTree;
class afApplicationTreeItemData;
class kaKernelView;

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>
#include <AMDTAPIClasses/Include/Events/apEvent.h>
#include <AMDTAPIClasses/Include/Events/apMonitoredObjectsTreeEvent.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>

// Framework:
#include <AMDTApplicationFramework/Include/afIRunModeManager.h>

// ----------------------------------------------------------------------------------
// Class Name:          vspKernelAnalyzerEditorManagerHelper : public QObject

// General Description: service class for Qt functionality needed
//
// Author:              Gilad Yarnitzky
// Creation Date:       29/8/2013
// ----------------------------------------------------------------------------------
class vspKernelAnalyzerEditorManagerHelper : public QObject
{
    Q_OBJECT

public:
    vspKernelAnalyzerEditorManagerHelper();
    ~vspKernelAnalyzerEditorManagerHelper() {};

public slots:
    void updateViewsAfterProjectChanged(const QString& projetcName, bool isFinal, bool isUserModelId);
    void onBuildComplete(const gtString& clFilePath);
    void onClearProjectSetting();
};

// ----------------------------------------------------------------------------------
// Class Name:          vspKernelAnalyzerEditorManager

// General Description: manager of the editors for the Kernel analyzer
//
// Author:              Gilad Yarnitzky
// Creation Date:       28/8/2013
// ----------------------------------------------------------------------------------
class vspKernelAnalyzerEditorManager : public apIEventsObserver, public afIRunModeManager
{
    friend class vspSingletonsDelete;
public:

    // Overrides apIEventsObserver:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);
    virtual const wchar_t* eventObserverName() const { return L"vspKernelAnalyzerEditorManager"; };

    // Get my single instance:
    static vspKernelAnalyzerEditorManager& instance();

    // Destructor:
    ~vspKernelAnalyzerEditorManager();

    // Active document information:
    QWidget* activeView() { return m_pActiveView; }
    void setActiveView(QWidget* pActiveView) { m_pActiveView = pActiveView; }

    // handle events:
    bool activateItemInVS(QTreeWidgetItem* pItemToActivate);

    // managing the views:
    void addKernelView(kaKernelView* pView);
    void removeKernelView(kaKernelView* pView);

    void updateViewsAfterProjectChanged(const QString& projetcName, bool isFinal);

    // handle end of build: update all kernel views
    void onBuildComplete(const gtString& clFilePath);

    // afIRunModeManager derived functions:
    virtual afRunModes getCurrentRunModeMask();

    virtual bool canStopCurrentRun();
    virtual bool stopCurrentRun();

    virtual bool getExceptionEventDetails(const apExceptionEvent& exceptionEve, osCallStack& exceptionCallStack, bool& openCLEnglineLoaded, bool& openGLEnglineLoaded, bool& kernelDebuggingEnteredAtLeastOnce);

    /// Adds and saves cl files to tree view from current VS project
    void AddCLFilesToTree();

    /// Load the data for the requested file path
    /// \param pKernelView the kernel view for which the file should be loaded
    /// \param filePath the file path for the kernel view
    void LoadDataForFilePath(kaKernelView* pKernelView, const osFilePath& filePath);

private:

    // Only my instance() method should create my single instance:
    vspKernelAnalyzerEditorManager();

    void getApplicationTree();
    afApplicationTreeItemData* getTreeItemData(QTreeWidgetItem* pTreeItem) const;

    void loadData();

private:

    // Single instance static member:
    static vspKernelAnalyzerEditorManager* m_psMySingleInstance;

    QWidget* m_pActiveView;

    // project name:
    gtString m_curProjectName;

    // Get the application commands instance:
    afApplicationCommands* m_pApplicationCommands;
    afApplicationTree* m_pApplicationTree;
    vspKernelAnalyzerEditorManagerHelper* m_pQtHelper;

    gtVector<kaKernelView*> m_kernelViews;
};

#endif //__VSPKERNELANAYLZEREDITORMANAGER_H
