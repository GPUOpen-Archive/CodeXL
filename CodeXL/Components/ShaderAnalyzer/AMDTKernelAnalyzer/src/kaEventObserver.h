//------------------------------ kaEventObserver.h ------------------------------

#ifndef __KAEVENTOBSERVER_H
#define __KAEVENTOBSERVER_H

class afApplicationCommands;
class afApplicationTree;
class afApplicationTreeItemData;

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>
#include <AMDTAPIClasses/Include/Events/apMonitoredObjectsTreeEvent.h>
#include <AMDTAPIClasses/Include/Events/apMDIViewActivatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apMDIViewCreateEvent.h>
#include <AMDTKernelAnalyzer/Include/kaAMDTKernelAnalyzerDLLBuild.h>
#include <AMDTApplicationFramework/Include/afIRunModeManager.h>

// ----------------------------------------------------------------------------------
// Class Name:           kaEventObserver : public apIEventsObserver
// General Description: An (apEvent) event observer, responsible of translating apEvent-s
//                      to events that can be consumed by Visual studio.
// Author:               Gilad Yarnitzky
// Creation Date:        5/8/2013
// ----------------------------------------------------------------------------------
class KA_API kaEventObserver : public apIEventsObserver, public afIRunModeManager
{
public:
    kaEventObserver();
    virtual ~kaEventObserver();

    // Overrides apIEventsObserver:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);
    virtual const wchar_t* eventObserverName() const { return L"KaEventObserver"; };

protected:

    void getApplicationTree();
    afApplicationTreeItemData* getTreeItemData(QTreeWidgetItem* pTreeItem) const;
    bool activateItem(QTreeWidgetItem* pItemToActivate);

    // afIRunModeManager derived functions:
    virtual afRunModes getCurrentRunModeMask();

    virtual bool canStopCurrentRun();
    virtual bool stopCurrentRun();

    virtual bool getExceptionEventDetails(const apExceptionEvent& exceptionEve, osCallStack& exceptionCallStack, bool& openCLEnglineLoaded, bool& openGLEnglineLoaded, bool& kernelDebuggingEnteredAtLeastOnce);

    // Register to source save signals
    void RegisterToSaveSignal(const apMDIViewCreateEvent& createEvent);

    // Handle mdi activation event
    void OnMdiActivateEvent(const apMDIViewActivatedEvent& activateEvent);

    // Get the application commands instance:
    afApplicationCommands* m_pApplicationCommands;
    afApplicationTree* m_pApplicationTree;
};

#endif //__GWEVENTOBSERVER_H
