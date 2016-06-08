//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCLCommandQueue.h
///
//==================================================================================

//------------------------------ apCLCommandQueue.h ------------------------------

#ifndef __APCLCOMMANDQUEUE_H
#define __APCLCOMMANDQUEUE_H

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSAPIWrappers/Include/oaOpenCLIncludes.h>
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>

// Local:
#include <AMDTAPIClasses/Include/apAllocatedObject.h>
// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCLCommandQueue : public apAllocatedObject
// General Description:
//   Represents an OpenCL command queue object.
//
// Author:  AMD Developer Tools Team
// Creation Date:        22/11/2009
// ----------------------------------------------------------------------------------
class AP_API apCLCommandQueue : public apAllocatedObject
{
public:
    // Self functions:
    apCLCommandQueue();
    apCLCommandQueue(oaCLCommandQueueHandle commandQueueHandle, oaCLContextHandle context, int deviceIndex);
    apCLCommandQueue(const apCLCommandQueue& other);
    virtual ~apCLCommandQueue();

    // Handle:
    oaCLCommandQueueHandle commandQueueHandle() const {return _commandQueueHandle;};

    // Context Handle:
    oaCLContextHandle contextHandle() const {return _contextHandle;};

    // Deletion status:
    bool wasMarkedForDeletion() const {return _wasMarkedForDeletion;};
    void onCommandQueueMarkedForDeletion() {_wasMarkedForDeletion = true;};

    // Device index:
    int deviceIndex() const {return _deviceIndex;};

    // Command Queue properties:
    bool outOfOrderExecutionModeEnable() const {return _outOfOrderExecutionModeEnable;};
    void setOutOfOrderExecutionModeEnable(bool outOfOrderExecutionModeEnable) {_outOfOrderExecutionModeEnable = outOfOrderExecutionModeEnable;};
    gtUInt32 referenceCount() const {return _referenceCount;};
    void setReferenceCount(gtUInt32 referenceCount) {_referenceCount = referenceCount;};
    bool profilingModeEnable() const {return _profilingModeEnable;};
    void setProfilingModeEnable(bool profilingModeEnable) {_profilingModeEnable = profilingModeEnable;};
    bool queueOnDevice() const { return m_queueOnDevice; };
    void setQueueOnDevice(bool queueOnDevice) { m_queueOnDevice = queueOnDevice; };
    bool isDefaultOnDeviceQueue() const { return m_isDefaultOnDeviceQueue; };
    void setIsDefaultOnDeviceQueue(bool isDefaultOnDeviceQueue) { m_isDefaultOnDeviceQueue = isDefaultOnDeviceQueue; };
    gtUInt32 queueSize() const { return m_queueSize; };
    void setQueueSize(gtUInt32 queueSize) { m_queueSize = queueSize; };

    // cl_gremedy_object_naming:
    const gtString& queueName() const {return _queueName;};
    void setQueueName(const gtString& name) {_queueName = name;};

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

private:
    // Command Queue handle:
    oaCLCommandQueueHandle _commandQueueHandle;

    // Context handle:
    oaCLContextHandle _contextHandle;

    // Deletion status:
    bool _wasMarkedForDeletion;

    // Device index:
    int _deviceIndex;

    // Out of order execution mode enable:
    bool _outOfOrderExecutionModeEnable;

    // Profiling mode enable:
    bool _profilingModeEnable;

    // On-device queues:
    bool m_queueOnDevice;
    bool m_isDefaultOnDeviceQueue;
    gtUInt32 m_queueSize;

    // The context's reference count:
    gtUInt32 _referenceCount;

    // cl_gremedy_object_naming:
    gtString _queueName;
};



#endif //__APCLCOMMANDQUEUE_H

