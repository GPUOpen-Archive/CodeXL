//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdThreadsEventObserver.h
///
//==================================================================================

//------------------------------ gdThreadsEventObserver.h ------------------------------

#ifndef __GDTHREADSEVENTOBSERVER_H
#define __GDTHREADSEVENTOBSERVER_H

// Forward decelerations:
class afGlobalVariableChangedEvent;
class apDebuggedProcessRunSuspendedEvent;

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>
#include <AMDTApplicationComponents/Include/acDefinitions.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           gdThreadsEventObserver : public wxComboBox, public apIEventsObserver
// General Description:
//  A combobox containing a list of all threads, also contains a static gtMap which maps thread IDs
//  to Internal IDs (zero based - i.e. main thread will be 0, next thread will be 1 and so on)
// Author:               Uri Shomroni
// Creation Date:        12/5/2008
// ----------------------------------------------------------------------------------
class GD_API gdThreadsEventObserver : public apIEventsObserver
{
public:

    virtual ~gdThreadsEventObserver();

    static gdThreadsEventObserver& instance();
    static unsigned int getThreadInternalID(osThreadId threadId);
    static osThreadId getThreadIDFromInternalID(unsigned int internalID);
    static void clearThreadIDsVector();

protected:

    // Overrides apIEventsObserver:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);
    virtual const wchar_t* eventObserverName() const { return L"ThreadsCombobox"; };

private:

    // Do not allow the use of my constructor:
    gdThreadsEventObserver();

private:

    // Single instance:
    static gdThreadsEventObserver* m_spMySingleInstance;

    // Contains true iff the debugged process exists:
    bool _debuggedProcessExists;

    // Contains true iff the debugged process is suspended:
    bool _isDebuggedProcessSuspended;

    // Maps OS thread id to our internal thread number:
    static gtMap<osThreadId, unsigned int> _OSThreadIdToInternalID;

    // Holds the amount of debugged application threads:
    static unsigned int _amountOfDebuggedApplicationThreads;
};

#endif //__GDTHREADSEVENTOBSERVER_H
