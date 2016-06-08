//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCLCommandQueue.cpp
///
//==================================================================================

//------------------------------ apCLCommandQueue.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTAPIClasses/Include/apCLCommandQueue.h>


// ---------------------------------------------------------------------------
// Name:        apCLCommandQueue::apCLCommandQueue
// Description: Constructor
// Arguments: cl_command_queqe commandQueue - The device CL id
// Author:  AMD Developer Tools Team
// Date:        1/12/2009
// ---------------------------------------------------------------------------
apCLCommandQueue::apCLCommandQueue(oaCLCommandQueueHandle commandQueue, oaCLContextHandle context, int deviceIndex)
    : _commandQueueHandle(commandQueue), _contextHandle(context), _wasMarkedForDeletion(false), _deviceIndex(deviceIndex), _outOfOrderExecutionModeEnable(false), _profilingModeEnable(false), m_queueOnDevice(false), m_isDefaultOnDeviceQueue(false), m_queueSize(0), _referenceCount(0)
{
}

// ---------------------------------------------------------------------------
// Name:        apCLCommandQueue::apCLCommandQueue
// Description: Default Constructor
// Author:  AMD Developer Tools Team
// Date:        16/2/2010
// ---------------------------------------------------------------------------
apCLCommandQueue::apCLCommandQueue()
    : _commandQueueHandle(0), _contextHandle(0), _wasMarkedForDeletion(false), _deviceIndex(-1), _outOfOrderExecutionModeEnable(false), _profilingModeEnable(false), m_queueOnDevice(false), m_isDefaultOnDeviceQueue(false), m_queueSize(0), _referenceCount(0)
{

}

// ---------------------------------------------------------------------------
// Name:        apCLCommandQueue::apCLCommandQueue
// Description: Copy Constructor
// Author:  AMD Developer Tools Team
// Date:        4/12/2009
// ---------------------------------------------------------------------------
apCLCommandQueue::apCLCommandQueue(const apCLCommandQueue& other)
    : _commandQueueHandle(other._commandQueueHandle), _contextHandle(other._contextHandle), _wasMarkedForDeletion(other._wasMarkedForDeletion), _deviceIndex(other._deviceIndex),
      _outOfOrderExecutionModeEnable(other._outOfOrderExecutionModeEnable), _profilingModeEnable(other._profilingModeEnable), m_queueOnDevice(other.m_queueOnDevice),
      m_isDefaultOnDeviceQueue(other.m_isDefaultOnDeviceQueue), m_queueSize(other.m_queueSize), _referenceCount(other._referenceCount)
{

}

// ---------------------------------------------------------------------------
// Name:        apCLCommandQueue::~apCLCommandQueue
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        1/12/2009
// ---------------------------------------------------------------------------
apCLCommandQueue::~apCLCommandQueue()
{
}


// ---------------------------------------------------------------------------
// Name:        apCLCommandQueue::type
// Description: Returns this transferable object type: OS_TOBJ_ID_CL_COMMAND_QUEUE.
// Author:  AMD Developer Tools Team
// Date:        1/12/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLCommandQueue::type() const
{
    return OS_TOBJ_ID_CL_COMMAND_QUEUE;
}


// ---------------------------------------------------------------------------
// Name:        apCLCommandQueue::writeSelfIntoChannel
// Description: Writes this object's data into an IPC channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        1/12/2009
// ---------------------------------------------------------------------------
bool apCLCommandQueue::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtUInt64)_commandQueueHandle;
    ipcChannel << (gtUInt64)_contextHandle;
    ipcChannel << _wasMarkedForDeletion;
    ipcChannel << (gtInt32)_deviceIndex;
    ipcChannel << _outOfOrderExecutionModeEnable;
    ipcChannel << _profilingModeEnable;
    ipcChannel << m_queueOnDevice;
    ipcChannel << m_isDefaultOnDeviceQueue;
    ipcChannel << m_queueSize;
    ipcChannel << _referenceCount;
    ipcChannel << _queueName;

    // Write the allocated object Info:
    apAllocatedObject::writeSelfIntoChannel(ipcChannel);

    return true;
}


// ---------------------------------------------------------------------------
// Name:        apCLCommandQueue::readSelfFromChannel
// Description: Reads this object's data from an IPC channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        1/12/2009
// ---------------------------------------------------------------------------
bool apCLCommandQueue::readSelfFromChannel(osChannel& ipcChannel)
{
    gtUInt64 handleUint64 = 0;
    ipcChannel >> handleUint64;
    _commandQueueHandle = (oaCLCommandQueueHandle)handleUint64;
    ipcChannel >> handleUint64;
    _contextHandle = (oaCLContextHandle)handleUint64;

    ipcChannel >> _wasMarkedForDeletion;

    gtInt32 deviceIndexInt32 = 0;
    ipcChannel >> deviceIndexInt32;
    _deviceIndex = (int)deviceIndexInt32;

    ipcChannel >> _outOfOrderExecutionModeEnable;
    ipcChannel >> _profilingModeEnable;
    ipcChannel >> m_queueOnDevice;
    ipcChannel >> m_isDefaultOnDeviceQueue;
    ipcChannel >> m_queueSize;
    ipcChannel >> _referenceCount;
    ipcChannel >> _queueName;

    // Read the allocated object Info:
    apAllocatedObject::readSelfFromChannel(ipcChannel);

    return true;
}
