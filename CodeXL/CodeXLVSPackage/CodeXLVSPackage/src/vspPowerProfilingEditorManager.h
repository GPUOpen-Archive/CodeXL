//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspPowerProfilingEditorManager.h
///
//==================================================================================

//------------------------------ vspPowerProfilingEditorManager.h ------------------------------

#ifndef __VSPPOWERPROFILINGEDITORMANAGER_H
#define __VSPPOWERPROFILINGEDITORMANAGER_H

// Qt
#include <QtWidgets>

class vspSingletonsDelete;
class vspPowerProfilingEditorDocument;
class QTreeWidgetItem;
class afApplicationCommands;
class afApplicationTree;
class afApplicationTreeItemData;
class ppSessionView;

// Infra:
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>
#include <AMDTAPIClasses/Include/Events/apEvent.h>
#include <AMDTAPIClasses/Include/Events/apMonitoredObjectsTreeEvent.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>

// Framework:
#include <AMDTApplicationFramework/Include/afIRunModeManager.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/ProfileApplicationTreeHandler.h>

// AMDTPowerProfiling:
#include <AMDTPowerProfiling/Include/ppWindowsManagerHelper.h>

// ----------------------------------------------------------------------------------
// Class Name:          vspPowerProfilingEditorManagerHelper : public QObject

// General Description: service class for Qt functionality needed
//
// Author:              Gilad Yarnitzky
// Creation Date:       29/8/2013
// ----------------------------------------------------------------------------------
class vspPowerProfilingEditorManagerHelper : public ppWindowsManagerHelper
{
    Q_OBJECT

public:
    vspPowerProfilingEditorManagerHelper();
    ~vspPowerProfilingEditorManagerHelper() {};

    /// Activate the session window belongs to pSessionData:
    virtual bool ActivateSessionWindow(SessionTreeNodeData* pSessionData);

public slots:
    void onClearProjectSetting();

};

// ----------------------------------------------------------------------------------
// Class Name:          vspPowerProfilingEditorManager

// General Description: manager of the editors for the Kernel analyzer
//
// Author:              Gilad Yarnitzky
// Creation Date:       28/8/2013
// ----------------------------------------------------------------------------------
class vspPowerProfilingEditorManager : public apIEventsObserver, public afIRunModeManager
{
    friend class vspSingletonsDelete;
public:

    // Overrides apIEventsObserver:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);
    virtual const wchar_t* eventObserverName() const { return L"vspPowerProfilingEditorManager"; };

    // Get my single instance:
    static vspPowerProfilingEditorManager& instance();

    // Destructor:
    ~vspPowerProfilingEditorManager();

    // handle events:
    bool activateItemInVS(QTreeWidgetItem* pItemToActivate);

    // afIRunModeManager derived functions:
    virtual afRunModes getCurrentRunModeMask();

    virtual bool canStopCurrentRun();
    virtual bool stopCurrentRun();

    virtual bool getExceptionEventDetails(const apExceptionEvent& exceptionEve, osCallStack& exceptionCallStack, bool& openCLEnglineLoaded, bool& openGLEnglineLoaded, bool& kernelDebuggingEnteredAtLeastOnce);

private:

    // Only my instance() method should create my single instance:
    vspPowerProfilingEditorManager();

    afApplicationTreeItemData* getTreeItemData(QTreeWidgetItem* pTreeItem) const;

private:

    // Single instance static member:
    static vspPowerProfilingEditorManager* m_psMySingleInstance;

    // Get the application commands instance:
    afApplicationCommands* m_pApplicationCommands;

    /// Instance of the windows manager in VS:
    vspPowerProfilingEditorManagerHelper* m_pPPWindowsManager;

};

#endif //__VSPPOWERPROFILINGEDITORMANAGER_H
