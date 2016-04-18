//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suContextMonitor.h
///
//==================================================================================

//------------------------------ suContextMonitor.h ------------------------------

#ifndef __SUCONTEXTMONITOR_H
#define __SUCONTEXTMONITOR_H

// Infra:
#include <AMDTAPIClasses/Include/apContextID.h>

// Local:
#include <AMDTServerUtilities/Include/suCallsHistoryLogger.h>
#include <AMDTServerUtilities/Include/suCallsStatisticsLogger.h>
#include <AMDTServerUtilities/Include/suNullContextCallsHistoryLogger.h>

// ----------------------------------------------------------------------------------
// Class Name:           suContextMonitor
// General Description:  Base class for OpenCL / OpenGL context monitors
// Author:               Sigal Algranaty
// Creation Date:        21/3/2010
// ----------------------------------------------------------------------------------
class SU_API suContextMonitor
{
public:
    suContextMonitor(apContextID contextID);
    virtual ~suContextMonitor();

    // Events:
    virtual void onFrameTerminatorCall();
    virtual void onMonitoredFunctionCall();

    // Virtual functions:
    virtual bool updateContextDataSnapshot(bool sendEvents = false);
    virtual void beforeUpdatingContextDataSnapshot();
    virtual void afterUpdatingContextDataSnapshot();
    virtual void afterMonitoredFunctionExecutionActions(apMonitoredFunctionId calledFunctionId);

    // Calls history:
    const suCallsHistoryLogger* callsHistoryLogger() const { return _pCallsHistoryLogger; };
    suCallsHistoryLogger* callsHistoryLogger() { return _pCallsHistoryLogger; };

    // Function statistics logger:
    const suCallsStatisticsLogger& callsStatisticsLogger() const { return _callsStatisticsLogger; };
    suCallsStatisticsLogger& callsStatisticsLogger() { return _callsStatisticsLogger; };

    // Context Id:
    apContextID contextID() const {return _contextID;};

    // Add function call:
    virtual void addFunctionCall(apMonitoredFunctionId calledFunctionId, int argumentsAmount, va_list& pArgumentList, apFunctionDeprecationStatus functionDeprecationStatus);

    // Holds the default context instance:
    static suContextMonitor& defaultContextInstance();

    void onDebuggedProcessTerminationAlert();

    // Context data update:
    bool isDuringContextDataUpdate() const { return _isDuringContextDataUpdate; };

protected:
    // Hold the context ID:
    apContextID _contextID;

    // A logger that logs the context calls history:
    suCallsHistoryLogger* _pCallsHistoryLogger;

    // A pointer to this class single instance:
    static suContextMonitor* _pDefaultContextSingleInstance;

    // The calls statistics logger:
    suCallsStatisticsLogger _callsStatisticsLogger;

private:

    // Contains true iff we are during the update of a context data snapshot:
    bool _isDuringContextDataUpdate;

private:
    // Allow access to the singeltons deleter:
    friend class suSingletonsDelete;
};


#endif //__suContextMonitor_H

