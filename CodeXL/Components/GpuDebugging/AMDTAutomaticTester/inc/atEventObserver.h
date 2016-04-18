//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file atEventObserver.h
///
//==================================================================================
#ifndef __ATEVENTOBSERVER_H
#define __ATEVENTOBSERVER_H




// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>
#include <AMDTAPIClasses/Include/Events/apBreakpointHitEvent.h>

#include <AMDTOSWrappers/Include/osCriticalSection.h>

// Local:
#include <inc/atTestData.h>
#include <inc/atTestsLogCommand.h>

class osCallStack;

// ----------------------------------------------------------------------------------
// Class Name:          atEventObserver : public apIEventsObserver
// General Description: Event observer for the automatic tester
// Author:              Merav Zanany
// Creation Date:       30/11/2011
// ----------------------------------------------------------------------------------




/// -----------------------------------------------------------------------------------------------
/// \class Name: atEventObserver : public apIEventsObserver
/// \brief Description:
/// -----------------------------------------------------------------------------------------------
class atEventObserver : public apIEventsObserver
{
public:

    virtual ~atEventObserver();

    // Single instance:
    static atEventObserver& instance();

    // Current test data:
    void setCurrentTest(const atTestData* pTestData);

    // Overrides apIEventsObserver:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);
    virtual const wchar_t* eventObserverName() const { return L"atEventObserver"; };
    virtual void onEventRegistration(apEvent& eve, bool& vetoEvent);
    void onBreakpointHit(const apBreakpointHitEvent& event);

    void outputLogString(gtString& outputStr);

    // Pending debug events flag:
    void setPendingDebugEvents(bool arePending) {_waitingForEventsToArrive = !arePending;};
    bool& waitingForEvents() {return _waitingForEventsToArrive;};
    void beforeAccessingEventsFlag() {_eventsFlagCS.enter();};
    bool beforeWritingEventsFlag() {return _eventsFlagCS.tryEntering();};
    void afterAccessingEventsFlag() {_eventsFlagCS.leave();};

    // Process events
    bool& wasProcessCreatedFlag() { return m_wasProcessCreated; };
    bool& hasProcessEndedFlag() { return m_hasProcessEnded; };
    void resetProcessFlags() { m_wasProcessCreated = false; m_hasProcessEnded = false; };

    const gtVector<gtString>& getTestLogStrings()
    {
        return _testLogStrings;
    }

    void clearTestLogStrings()
    {
        _testLogStrings.clear();
    }

protected:

    // Do not allow public use of my construction:
    atEventObserver();
    void writeBreakpointsIntoResultsFile();
    void initializeCurrentTest();
    bool shouldWaitForMoreBreakpoints();
    bool testVariables(int lineNumber);
    bool outputAllLocals(int lineNumber);
    bool testStepInto(int lineNumber);
    bool testValueForAllWorkItems(int lineNumber);
    void setSteppingWorkItem();
    void storeCurrentLineNumber(const osCallStack& kernelStack, gtString& outputStr, int& lineNumber);

    // Contain the logging for the current test:
    gtVector<gtString> _testLogStrings;

    // My static instance:
    static atEventObserver* _pMyStaticInstance;

    // Contain true iff there are pending debug events:
    bool _waitingForEventsToArrive;

    // Contain true iff a function name breakpoint was hit
    const atTestData* _pCurrentTestData;
    gtString _currentExecutedKernelName;
    gtMap<gtString, bool> _executedKernels;

    osCriticalSection _eventsFlagCS;

    // Flags that determine whether specific events have already happened:
    bool m_wasProcessCreated;
    bool m_hasProcessEnded;
};

#endif //__ATEVENTOBSERVER_H

