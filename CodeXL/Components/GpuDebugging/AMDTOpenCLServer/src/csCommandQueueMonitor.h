//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csCommandQueueMonitor.h
///
//==================================================================================

//------------------------------ csCommandQueueMonitor.h ------------------------------

#ifndef __CSCOMMANDQUEUEMONITOR_H
#define __CSCOMMANDQUEUEMONITOR_H

// Infra:
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTOSWrappers/Include/osCriticalSection.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSAPIWrappers/Include/oaOpenCLIncludes.h>
#include <AMDTAPIClasses/Include/apCLEnqueuedCommands.h>
#include <AMDTAPIClasses/Include/apCLCommandQueue.h>

// ATI Counters are supported on Windows only:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include <src/csAMDQueuePerformanceCountersReader.h>
#endif

#include <src/csOpenCLQueuePerformanceCountersReader.h>

class csContextMonitor;

// ----------------------------------------------------------------------------------
// Class Name:           csCommandQueueMonitor
// General Description:
//  Monitors an OpenCL Command Queue.
//
// Author:               Yaki Tebeka
// Creation Date:        3/3/2010
// ----------------------------------------------------------------------------------
class csCommandQueueMonitor
{
public:
    csCommandQueueMonitor(int contextSpyId, int commandQueueIndex, oaCLCommandQueueHandle commandQueueHandle,
                          oaCLContextHandle contextHandle, int deviceIndex, bool shouldInitializePerformanceCounters);
    virtual ~csCommandQueueMonitor();

    bool updateContextDataSnapshot();
    bool updateQueueTimesIfDeviceAllowsProfiling();

    // Events:
    void beforeCommandAddedToQueue();
    void afterCommandAddedToQueue();
    void beforeAPIThreadDataAccess();
    void afterAPIThreadDataAccess();
    void onCommandQueueMarkedForDeletion();
    void onCommandAddedToQueue(gtAutoPtr<apCLEnqueuedCommand>& aptrCommand);
    void onCommandQueuePropertiesSet(cl_command_queue_properties properties, cl_bool enable);
    void onEnqueueNDRangeKernel();
    void onDebuggedProcessResumed();

    // Queue information:
    const apCLCommandQueue& commandQueueInfo() const { return _commandQueueInfo; };
    apCLCommandQueue& commandQueueInfo() { return _commandQueueInfo; };
    int contextSpyId() const { return _contextSpyId; };
    int commandQueueIndex() const { return _commandQueueIndex; };

    // Queue commands:
    int amountOfCommandsInQueue() const;
    const apCLEnqueuedCommand* getEnqueuedCommandDetails(int enqueuedCommandIndex) const;

    // Queue Events:
    int amountOfEventsInQueue() const;
    void addEventToQueue(oaCLEventHandle eventHandle, bool retainedBySpy, bool reportEventOverflow, bool& wasEventOverflow, gtString& errorToReport, int& errorContext);
    void removeEventFromQueue(oaCLEventHandle eventHandle);

    // AMD performance counters:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
#ifdef OA_DEBUGGER_USE_AMD_GPA
    csAMDQueuePerformanceCountersReader& AMDPerformanceCountersReader() { return _amdPerformanceCountersReader; };
#endif
#endif

    // OpenCL queue counters reader:
    csOpenCLQueuePerformanceCountersReader& openCLQueueCountersReader() { return _openCLQueueCountersReader;};

private:
    // Queue commands:
    // Uri, 22/8/13 - moving this to private, since event profiling is not currently supported by the debugger:
    void addCommandToQueue(gtAutoPtr<apCLEnqueuedCommand>& aptrCommand);

private:
    bool flushOpenCLQueue();
    bool updateQueueReferenceCount();
    bool updateQueueTimes();
    void updateVendorType();
    void clearQueue();
    bool doesDeviceSupportProfiling() const;
    bool getCommandMemorySize(gtAutoPtr<apCLEnqueuedCommand>& aptrCommand);
    gtInt32 memObjectHandleToPixelSize(oaCLMemHandle memobj);

    // Enqueued commands synchronization:
    void lockAccessToEnqueuedCommands() { _enqueuedCommandsAccessCS.enter(); };
    void unlockAccessToEnqueuedCommands() { _enqueuedCommandsAccessCS.leave(); };

    // Do not allow the use of my default constructor:
    csCommandQueueMonitor();

private:
    // The command queue's context monitor:
    const csContextMonitor* _pContextMonitor;

    // The context spy id:
    int _contextSpyId;

    // The command queue's index in its context:
    int _commandQueueIndex;

    // Holds command queue information:
    // (Can be exported by the API as an apCLCommandQueue object)
    apCLCommandQueue _commandQueueInfo;

    // Contain the vendor type:
    oaVendorType _vendorType;

    // Command queue's enqueued commands:
    gtPtrVector<apCLEnqueuedCommand*> _enqueuedCommands;
    osCriticalSection _enqueuedCommandsAccessCS;

    // A vector holding the command queue's events:
    bool m_logQueueEvents;
    gtVector<oaCLEventHandle> _commandQueueEvents;

    // The maximal numbers of commands we will hold in a queue until we clear it:
    unsigned int _maxCommandsPerQueue;

    // Contains true iff we tried to allocate too many events in a single queue:
    bool _wasEventsOverflowErrorIssued;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
#ifdef OA_DEBUGGER_USE_AMD_GPA
    // AMD performance counters reader:
    csAMDQueuePerformanceCountersReader _amdPerformanceCountersReader;
#endif
#endif

    // OpenCL Queue counters reader:
    csOpenCLQueuePerformanceCountersReader _openCLQueueCountersReader;

    // Contain true iff performance counters should be monitored:
    bool _shouldInitializePerformanceCounters;

    // Do we log profiling information (queue command data)?
    // Uri, 05/06/2012 - all profiling capabilities are handled by the GPUProfiling plugin, so this member is false by default.
    bool m_logEventProfilingInformation;
};


#endif //__CSCOMMANDQUEUEMONITOR_H

