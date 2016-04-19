//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csCommandQueuesMonitor.h
///
//==================================================================================

//------------------------------ csCommandQueuesMonitor.h ------------------------------

#ifndef __CSCOMMANDQUEUESMONITOR_H
#define __CSCOMMANDQUEUESMONITOR_H

// Infra:
#include <AMDTBaseTools/Include/gtAutoPtr.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTAPIClasses/Include/apCLEnqueuedCommands.h>

// Local:
#include <src/csCommandQueueMonitor.h>


// ----------------------------------------------------------------------------------
// Class Name:          csCommandQueuesMonitor
// General Description: Monitors OpenCL command queue.
// Author:              Sigal Algranaty
// Creation Date:       1/12/2009
// ----------------------------------------------------------------------------------
class csCommandQueuesMonitor
{
public:
    csCommandQueuesMonitor();
    virtual ~csCommandQueuesMonitor();

    // Update context data snapshot:
    bool updateContextDataSnapshot();

    // Events:
    void onCommandQueueCreation(cl_command_queue commandQueueHandle, cl_context context, cl_device_id deviceId, cl_command_queue_properties properties, cl_uint queueSize = 0);
    void onCommandQueueCreationWithProperties(cl_command_queue commandQueueHandle, cl_context context, cl_device_id deviceId, const cl_queue_properties* properties);
    void onCommandQueuePropertiesSet(cl_command_queue commandQueueHandle, cl_command_queue_properties properties, cl_bool enable);
    void onDebuggedProcessSuspended();
    void onDebuggedProcessResumed();
    void onEnqueueNDRangeKernel(oaCLCommandQueueHandle commandQueueHandle);
    void onFrameTerminatorCall();

    // Reference count checking:
    void checkForReleasedQueues();

    // Command queues:
    int amountOfQueues() const { return (int)_commandQueuesMonitors.size(); };
    int amountOfNotDeletedQueues() const;
    int commandQueueIndex(oaCLCommandQueueHandle commandQueueHandle) const;
    const csCommandQueueMonitor* commandQueueMonitor(int queueIndex) const;
    csCommandQueueMonitor* commandQueueMonitor(int queueIndex);

private:
    // Do not allow use of the = operator for this class. Use reference or pointer transferral instead
    csCommandQueuesMonitor& operator=(const csCommandQueuesMonitor& otherMonitor);
    csCommandQueuesMonitor(const csCommandQueuesMonitor& otherMonitor);

private:
    // Command queues' monitors:
    gtPtrVector<csCommandQueueMonitor*> _commandQueuesMonitors;

    // A watch used to make sure that we do not sample the AMD counters more then once in 100MS:
    osStopWatch _amdCountersSampleWatch;
};


#endif //__CSCOMMANDQUEUESMONITOR_H

