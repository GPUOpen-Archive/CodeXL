//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCLEnqueuedCommands.cpp
///
//==================================================================================

//------------------------------ apCLEnqueuedCommands.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apCLEnqueuedCommands.h>

#include <algorithm>

// ------------------------------- apCLEnqueuedCommand ------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLEnqueuedCommand::apCLEnqueuedCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLEnqueuedCommand::apCLEnqueuedCommand()
    : osTransferableObject(), _queuedTime(0), _submittedTime(0), _executionStartedTime(0), _executionEndedTime(0), m_executionCompletedTime(0),
      _areTimesUpdated(false), _commandEventHandle(OA_CL_NULL_HANDLE), _wasEventCreatedBySpy(false), _event(OA_CL_NULL_HANDLE)
{

}

// ---------------------------------------------------------------------------
// Name:        apCLEnqueuedCommand::apCLEnqueuedCommand
// Description: Constructor to be used (only) by subclasses, allows setting the
//              events wait list and the (user-created) event handle.
// Author:  AMD Developer Tools Team
// Date:        3/3/2010
// ---------------------------------------------------------------------------
apCLEnqueuedCommand::apCLEnqueuedCommand(cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
    : osTransferableObject(), _queuedTime(0), _submittedTime(0), _executionStartedTime(0), _executionEndedTime(0), m_executionCompletedTime(0),
      _areTimesUpdated(false), _commandEventHandle(OA_CL_NULL_HANDLE), _wasEventCreatedBySpy(false), _event(OA_CL_NULL_HANDLE)
{
    // Add the event wait list, if given:
    if (num_events_in_wait_list > 0)
    {
        GT_IF_WITH_ASSERT(event_wait_list != NULL)
        {
            for (cl_uint i = 0; i < num_events_in_wait_list; i++)
            {
                _event_wait_list.push_back((oaCLEventHandle)event_wait_list[i]);
            }
        }
    }

    // Add the event, if given:
    if (event != NULL)
    {
        _event = (oaCLEventHandle)(*event);
    }
}

// ---------------------------------------------------------------------------
// Name:        apCLEnqueuedCommand::~apCLEnqueuedCommand
// Description: Destrcutor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLEnqueuedCommand::~apCLEnqueuedCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLEnqueuedCommand::relatedKernelHandle
// Description: Returns the related kernel handles. To be overriden by commands
//              that have related kernels.
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
void apCLEnqueuedCommand::getRelatedKernelHandles(gtVector<oaCLKernelHandle>& kernelHandles) const
{
    kernelHandles.clear();
}

// ---------------------------------------------------------------------------
// Name:        apCLEnqueuedCommand::relatedSrcMemHandle
// Description: Returns the related src mem (buffer / texture) handles. To be
//              overriden by commands that have src mem objects.
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
void apCLEnqueuedCommand::getRelatedSrcMemHandles(gtVector<oaCLMemHandle>& srcMemHandles) const
{
    srcMemHandles.clear();
}

// ---------------------------------------------------------------------------
// Name:        apCLEnqueuedCommand::relatedDstMemHandle
// Description: Returns the related dst mem (buffer / texture) handles. To be
//              overriden by commands that have dst mem objects.
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
void apCLEnqueuedCommand::getRelatedDstMemHandles(gtVector<oaCLMemHandle>& dstMemHandles) const
{
    dstMemHandles.clear();
}

// ---------------------------------------------------------------------------
// Name:        apCLEnqueuedCommand::eventWaitListParameterAvailable
// Description: Returns true iff the specific class has an "Event wait list"
//              parameter. Since most commands have this parameter, this is
//              to be overwritten by commands that don't have it.
// Author:  AMD Developer Tools Team
// Date:        4/3/2010
// ---------------------------------------------------------------------------
bool apCLEnqueuedCommand::eventWaitListParameterAvailable() const
{
    return true;
}

// ---------------------------------------------------------------------------
// Name:        apCLEnqueuedCommand::eventParameterAvailable
// Description: Returns true iff the specific class has an "Event" parameter.
//              Since most commands have this parameter, this is to be
//              overwritten by commands that don't have it.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        4/3/2010
// ---------------------------------------------------------------------------
bool apCLEnqueuedCommand::eventParameterAvailable() const
{
    return true;
}

// ---------------------------------------------------------------------------
// Name:        apCLEnqueuedCommand::isCLEnqueuedCommandObject
// Description: Returns true iff this is a sub-class of apCLEnqueuedCommand.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// Implementation notes:
//   By implementing it in apCLEnqueuedCommand, all sub-classes inherit this
//   implementation that answers "true".
// ---------------------------------------------------------------------------
bool apCLEnqueuedCommand::isCLEnqueuedCommandObject() const
{
    return true;
}

// ---------------------------------------------------------------------------
// Name:        apCLEnqueuedCommand::writeSelfIntoChannel
// Description: Writes the command into an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLEnqueuedCommand::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _queuedTime;
    ipcChannel << _submittedTime;
    ipcChannel << _executionStartedTime;
    ipcChannel << _executionEndedTime;
    ipcChannel << m_executionCompletedTime;
    ipcChannel << _areTimesUpdated;

    ipcChannel << (gtUInt64)_commandEventHandle;
    ipcChannel << _wasEventCreatedBySpy;

    gtUInt32 num_events_in_wait_list = (gtUInt32)_event_wait_list.size();
    ipcChannel << num_events_in_wait_list;

    for (gtUInt32 i = 0; i < num_events_in_wait_list; i++)
    {
        ipcChannel << (gtUInt64)_event_wait_list[i];
    }

    ipcChannel << (gtUInt64)_event;

    return true;
}

// ---------------------------------------------------------------------------
// Name:        apCLEnqueuedCommand::readSelfFromChannel
// Description: Reads the command from an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLEnqueuedCommand::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _queuedTime;
    ipcChannel >> _submittedTime;
    ipcChannel >> _executionStartedTime;
    ipcChannel >> _executionEndedTime;
    ipcChannel >> m_executionCompletedTime;
    ipcChannel >> _areTimesUpdated;

    gtUInt64 commandEventHandleAsUInt64 = OA_CL_NULL_HANDLE;
    ipcChannel >> commandEventHandleAsUInt64;
    _commandEventHandle = (oaCLEventHandle)commandEventHandleAsUInt64;
    ipcChannel >> _wasEventCreatedBySpy;

    _event_wait_list.clear();
    gtUInt32 num_events_in_wait_list = 0;
    ipcChannel >> num_events_in_wait_list;

    for (gtUInt32 i = 0; i < num_events_in_wait_list; i++)
    {
        gtUInt64 event_wait_listAsUInt64 = 0;
        ipcChannel >> event_wait_listAsUInt64;
        _event_wait_list.push_back((oaCLEventHandle)event_wait_listAsUInt64);
    }

    gtUInt64 eventAsUInt64 = 0;
    ipcChannel >> eventAsUInt64;
    _event = (oaCLEventHandle)eventAsUInt64;

    return true;
}

// ---------------------------------------------------------------------------
// Name:        apCLEnqueuedCommand::waitForSubmit
// Description: Returns the amount of time the command waited to be submitted
//              (time between being queued and submitted) in nanoseconds.
// Author:  AMD Developer Tools Team
// Date:        3/3/2010
// ---------------------------------------------------------------------------
gtUInt64 apCLEnqueuedCommand::waitForSubmit() const
{
    gtUInt64 retVal = 0;

    // If not all times are updated for this, return 0:
    if (areTimesUpdated())
    {
        GT_IF_WITH_ASSERT(_submittedTime >= _queuedTime)
        {
            retVal = (_submittedTime - _queuedTime);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLEnqueuedCommand::waitForExecution
// Description: Returns the amount of time the command waited to be executed
//              (time between being submitted and executed) in nanoseconds.
// Author:  AMD Developer Tools Team
// Date:        3/3/2010
// ---------------------------------------------------------------------------
gtUInt64 apCLEnqueuedCommand::waitForExecution() const
{
    gtUInt64 retVal = 0;

    // If not all times are updated for this, return 0:
    if (areTimesUpdated())
    {
        GT_IF_WITH_ASSERT(_executionStartedTime >= _submittedTime)
        {
            retVal = (_executionStartedTime - _submittedTime);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLEnqueuedCommand::executionDuration
// Description: Returns the execution time in nanoseconds
// Author:  AMD Developer Tools Team
// Date:        20/1/2010
// ---------------------------------------------------------------------------
gtUInt64 apCLEnqueuedCommand::executionDuration() const
{
    gtUInt64 retVal = 0;

    // If not all times are updated for this, return 0:
    if (areTimesUpdated())
    {
        if (_executionEndedTime >= _executionStartedTime)
        {
            retVal = (_executionEndedTime - _executionStartedTime);
        }
        else // _executionEndedTime < _executionStartedTime
        {
            // An unterminated idle can have an end time of 0. Other commands
            // should not have this problem:
            GT_ASSERT((type() == OS_TOBJ_ID_CL_QUEUE_IDLE_TIME) && (_executionEndedTime == 0));
        }
    }

    return retVal;
}

// --------------------------- apCLAcquireGLObjectsCommand ---------------------------


// ---------------------------------------------------------------------------
// Name:        apCLAcquireGLObjectsCommand::apCLAcquireGLObjectsCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLAcquireGLObjectsCommand::apCLAcquireGLObjectsCommand()
    : apCLEnqueuedCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLAcquireGLObjectsCommand::apCLAcquireGLObjectsCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLAcquireGLObjectsCommand::apCLAcquireGLObjectsCommand(cl_uint num_objects, const cl_mem* mem_objects, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
    : apCLEnqueuedCommand(num_events_in_wait_list, event_wait_list, event)
{
    // Add the mem objects:
    GT_IF_WITH_ASSERT((num_objects > 0) && (mem_objects != NULL))
    {
        for (cl_uint i = 0; i < num_objects; i++)
        {
            _mem_objects.push_back((oaCLMemHandle)mem_objects[i]);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        apCLAcquireGLObjectsCommand::~apCLAcquireGLObjectsCommand
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLAcquireGLObjectsCommand::~apCLAcquireGLObjectsCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLAcquireGLObjectsCommand::commandNameAsString
// Description: Gets the command name as a string
// Author:  AMD Developer Tools Team
// Date:        4/12/2009
// ---------------------------------------------------------------------------
void apCLAcquireGLObjectsCommand::commandNameAsString(gtString& commandName) const
{
    commandName = L"AcquireGLObjects";
}

// ---------------------------------------------------------------------------
// Name:        apCLAcquireGLObjectsCommand::relatedDstMemHandle
// Description: Returns the related dst mem (buffer / texture) handles.
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
void apCLAcquireGLObjectsCommand::getRelatedDstMemHandles(gtVector<oaCLMemHandle>& dstMemHandles) const
{
    dstMemHandles.clear();

    // Copy the mem objects into the output vector:
    int numberOfMemObjects = (int)_mem_objects.size();

    for (int i = 0; i < numberOfMemObjects; i++)
    {
        dstMemHandles.push_back(_mem_objects[i]);
    }
}

// ---------------------------------------------------------------------------
// Name:        apCLAcquireGLObjectsCommand::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLAcquireGLObjectsCommand::type() const
{
    return OS_TOBJ_ID_CL_ACQUIRE_GL_OBJECTS_COMMAND;
}

// ---------------------------------------------------------------------------
// Name:        apCLAcquireGLObjectsCommand::writeSelfIntoChannel
// Description: Writes the command into an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLAcquireGLObjectsCommand::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apCLEnqueuedCommand::writeSelfIntoChannel(ipcChannel);

    gtUInt32 num_objects = (gtUInt32)_mem_objects.size();
    ipcChannel << num_objects;

    for (gtUInt32 i = 0; i < num_objects; i++)
    {
        ipcChannel << (gtUInt64)_mem_objects[i];
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        apCLAcquireGLObjectsCommand::readSelfFromChannel
// Description: Reads the command from an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLAcquireGLObjectsCommand::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apCLEnqueuedCommand::readSelfFromChannel(ipcChannel);

    _mem_objects.clear();
    gtUInt32 num_objects = 0;
    ipcChannel >> num_objects;

    for (gtUInt32 i = 0; i < num_objects; i++)
    {
        gtUInt64 mem_objectAsUInt64 = 0;
        ipcChannel >> mem_objectAsUInt64;
        _mem_objects.push_back((oaCLMemHandle)mem_objectAsUInt64);
    }

    return retVal;
}

// ------------------------------- apCLBarrierCommand -------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLBarrierCommand::apCLBarrierCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLBarrierCommand::apCLBarrierCommand()
    : apCLEnqueuedCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLBarrierCommand::~apCLBarrierCommand
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLBarrierCommand::~apCLBarrierCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLBarrierCommand::commandNameAsString
// Description: Gets the command name as a string
// Author:  AMD Developer Tools Team
// Date:        4/12/2009
// ---------------------------------------------------------------------------
void apCLBarrierCommand::commandNameAsString(gtString& commandName) const
{
    commandName = L"Barrier";
}

// ---------------------------------------------------------------------------
// Name:        apCLBarrierCommand::eventWaitListParameterAvailable
// Description: Returns true iff the specific class has an "Event wait list" parameter.
// Author:  AMD Developer Tools Team
// Date:        4/3/2010
// ---------------------------------------------------------------------------
bool apCLBarrierCommand::eventWaitListParameterAvailable() const
{
    return false;
}

// ---------------------------------------------------------------------------
// Name:        apCLBarrierCommand::eventParameterAvailable
// Description: Returns true iff the specific class has an "Event" parameter.
// Author:  AMD Developer Tools Team
// Date:        4/3/2010
// ---------------------------------------------------------------------------
bool apCLBarrierCommand::eventParameterAvailable() const
{
    return false;
}

// ---------------------------------------------------------------------------
// Name:        apCLBarrierCommand::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLBarrierCommand::type() const
{
    return OS_TOBJ_ID_CL_BARRIER_COMMAND;
}

// ---------------------------------------------------------------------------
// Name:        apCLBarrierCommand::writeSelfIntoChannel
// Description: Writes the command into an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLBarrierCommand::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apCLEnqueuedCommand::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLBarrierCommand::readSelfFromChannel
// Description: Reads the command from an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLBarrierCommand::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apCLEnqueuedCommand::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ------------------------------ apCLCopyBufferCommand -----------------------------

// ---------------------------------------------------------------------------
// Name:        apCLCopyBufferCommand::apCLCopyBufferCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLCopyBufferCommand::apCLCopyBufferCommand()
    : apCLEnqueuedCommand(), _src_buffer(OA_CL_NULL_HANDLE), _dst_buffer(OA_CL_NULL_HANDLE), _src_offset(0), _dst_offset(0), _cb(0)
{

}

// ---------------------------------------------------------------------------
// Name:        apCLCopyBufferCommand::apCLCopyBufferCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLCopyBufferCommand::apCLCopyBufferCommand(oaCLMemHandle src_buffer, oaCLMemHandle dst_buffer, gtSize_t src_offset, gtSize_t dst_offset,
                                             gtSize_t cb, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
    : apCLEnqueuedCommand(num_events_in_wait_list, event_wait_list, event), _src_buffer(src_buffer), _dst_buffer(dst_buffer), _src_offset(src_offset), _dst_offset(dst_offset), _cb(cb)
{

}

// ---------------------------------------------------------------------------
// Name:        apCLCopyBufferCommand::~apCLCopyBufferCommand
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLCopyBufferCommand::~apCLCopyBufferCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLCopyBufferCommand::commandNameAsString
// Description: Gets the command name as a string
// Author:  AMD Developer Tools Team
// Date:        4/12/2009
// ---------------------------------------------------------------------------
void apCLCopyBufferCommand::commandNameAsString(gtString& commandName) const
{
    commandName = L"CopyBuffer";
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyBufferCommand::relatedSrcMemHandle
// Description: Returns the related src mem (buffer / texture) handles.
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
void apCLCopyBufferCommand::getRelatedSrcMemHandles(gtVector<oaCLMemHandle>& srcMemHandles) const
{
    srcMemHandles.clear();
    srcMemHandles.push_back(_src_buffer);
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyBufferCommand::relatedDstMemHandle
// Description: Returns the related dst mem (buffer / texture) handles.
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
void apCLCopyBufferCommand::getRelatedDstMemHandles(gtVector<oaCLMemHandle>& dstMemHandles) const
{
    dstMemHandles.clear();
    dstMemHandles.push_back(_dst_buffer);
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyBufferCommand::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLCopyBufferCommand::type() const
{
    return OS_TOBJ_ID_CL_COPY_BUFFER_COMMAND;
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyBufferCommand::writeSelfIntoChannel
// Description: Writes the command into an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLCopyBufferCommand::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apCLEnqueuedCommand::writeSelfIntoChannel(ipcChannel);

    ipcChannel << (gtUInt64)_src_buffer;
    ipcChannel << (gtUInt64)_dst_buffer;
    ipcChannel << (gtUInt64)_src_offset;
    ipcChannel << (gtUInt64)_dst_offset;
    ipcChannel << (gtUInt64)_cb;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyBufferCommand::readSelfFromChannel
// Description: Reads the command from an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLCopyBufferCommand::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apCLEnqueuedCommand::readSelfFromChannel(ipcChannel);

    gtUInt64 src_bufferAsUInt64 = 0;
    ipcChannel >> src_bufferAsUInt64;
    _src_buffer = (oaCLMemHandle)src_bufferAsUInt64;
    gtUInt64 dst_bufferAsUInt64 = 0;
    ipcChannel >> dst_bufferAsUInt64;
    _dst_buffer = (oaCLMemHandle)dst_bufferAsUInt64;
    gtUInt64 src_offsetAsUInt64 = 0;
    ipcChannel >> src_offsetAsUInt64;
    _src_offset = (gtSize_t)src_offsetAsUInt64;
    gtUInt64 dst_offsetAsUInt64 = 0;
    ipcChannel >> dst_offsetAsUInt64;
    _dst_offset = (gtSize_t)dst_offsetAsUInt64;
    gtUInt64 cbAsUInt64 = 0;
    ipcChannel >> cbAsUInt64;
    _cb = (gtSize_t)cbAsUInt64;

    return retVal;
}

// ------------------------------ apCLCopyBufferRectCommand -----------------------------

// ---------------------------------------------------------------------------
// Name:        apCLCopyBufferRectCommand::apCLCopyBufferRectCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        26/12/2010
// ---------------------------------------------------------------------------
apCLCopyBufferRectCommand::apCLCopyBufferRectCommand()
    : apCLEnqueuedCommand(), _src_buffer(OA_CL_NULL_HANDLE), _dst_buffer(OA_CL_NULL_HANDLE), _src_row_pitch(0),
      _src_slice_pitch(0), _dst_row_pitch(0), _dst_slice_pitch(0)
{
    _src_origin[0] = 0;
    _src_origin[1] = 0;
    _src_origin[2] = 0;
    _dst_origin[0] = 0;
    _dst_origin[1] = 0;
    _dst_origin[2] = 0;
    _region[0] = 0;
    _region[1] = 0;
    _region[2] = 0;
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyBufferRectCommand::apCLCopyBufferRectCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        26/12/2010
// ---------------------------------------------------------------------------
apCLCopyBufferRectCommand::apCLCopyBufferRectCommand(oaCLMemHandle src_buffer, oaCLMemHandle dst_buffer, const gtSize_t src_origin[3], const gtSize_t dst_origin[3],
                                                     const gtSize_t region[3], gtSize_t src_row_pitch, gtSize_t src_slice_pitch, gtSize_t dst_row_pitch, gtSize_t dst_slice_pitch,
                                                     cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
    : apCLEnqueuedCommand(num_events_in_wait_list, event_wait_list, event), _src_buffer(src_buffer), _dst_buffer(dst_buffer), _src_row_pitch(src_row_pitch),
      _src_slice_pitch(src_slice_pitch), _dst_row_pitch(dst_row_pitch), _dst_slice_pitch(dst_slice_pitch)
{
    _src_origin[0] = src_origin[0];
    _src_origin[1] = src_origin[1];
    _src_origin[2] = src_origin[2];
    _dst_origin[0] = dst_origin[0];
    _dst_origin[1] = dst_origin[1];
    _dst_origin[2] = dst_origin[2];
    _region[0] = region[0];
    _region[1] = region[1];
    _region[2] = region[2];
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyBufferRectCommand::~apCLCopyBufferRectCommand
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        26/12/2010
// ---------------------------------------------------------------------------
apCLCopyBufferRectCommand::~apCLCopyBufferRectCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLCopyBufferRectCommand::commandNameAsString
// Description: Gets the command name as a string
// Author:  AMD Developer Tools Team
// Date:        26/12/2010
// ---------------------------------------------------------------------------
void apCLCopyBufferRectCommand::commandNameAsString(gtString& commandName) const
{
    commandName = L"CopyBufferRect";
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyBufferRectCommand::relatedSrcMemHandle
// Description: Returns the related src mem (buffer / texture) handles.
// Author:  AMD Developer Tools Team
// Date:        26/12/2010
// ---------------------------------------------------------------------------
void apCLCopyBufferRectCommand::getRelatedSrcMemHandles(gtVector<oaCLMemHandle>& srcMemHandles) const
{
    srcMemHandles.clear();
    srcMemHandles.push_back(_src_buffer);
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyBufferRectCommand::relatedDstMemHandle
// Description: Returns the related dst mem (buffer / texture) handles.
// Author:  AMD Developer Tools Team
// Date:        26/12/2010
// ---------------------------------------------------------------------------
void apCLCopyBufferRectCommand::getRelatedDstMemHandles(gtVector<oaCLMemHandle>& dstMemHandles) const
{
    dstMemHandles.clear();
    dstMemHandles.push_back(_dst_buffer);
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyBufferRectCommand::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        26/12/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apCLCopyBufferRectCommand::type() const
{
    return OS_TOBJ_ID_CL_COPY_BUFFER_RECT_COMMAND;
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyBufferRectCommand::writeSelfIntoChannel
// Description: Writes the command into an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        26/12/2010
// ---------------------------------------------------------------------------
bool apCLCopyBufferRectCommand::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apCLEnqueuedCommand::writeSelfIntoChannel(ipcChannel);

    ipcChannel << (gtUInt64)_src_buffer;
    ipcChannel << (gtUInt64)_dst_buffer;
    ipcChannel << (gtUInt64)_src_origin[0];
    ipcChannel << (gtUInt64)_src_origin[1];
    ipcChannel << (gtUInt64)_src_origin[2];
    ipcChannel << (gtUInt64)_dst_origin[0];
    ipcChannel << (gtUInt64)_dst_origin[1];
    ipcChannel << (gtUInt64)_dst_origin[2];
    ipcChannel << (gtUInt64)_region[0];
    ipcChannel << (gtUInt64)_region[1];
    ipcChannel << (gtUInt64)_region[2];
    ipcChannel << (gtUInt64)_src_row_pitch;
    ipcChannel << (gtUInt64)_src_slice_pitch;
    ipcChannel << (gtUInt64)_dst_row_pitch;
    ipcChannel << (gtUInt64)_dst_slice_pitch;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyBufferRectCommand::readSelfFromChannel
// Description: Reads the command from an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        26/12/2010
// ---------------------------------------------------------------------------
bool apCLCopyBufferRectCommand::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apCLEnqueuedCommand::readSelfFromChannel(ipcChannel);

    gtUInt64 src_bufferAsUInt64 = 0;
    ipcChannel >> src_bufferAsUInt64;
    _src_buffer = (oaCLMemHandle)src_bufferAsUInt64;
    gtUInt64 dst_bufferAsUInt64 = 0;
    ipcChannel >> dst_bufferAsUInt64;
    _dst_buffer = (oaCLMemHandle)dst_bufferAsUInt64;
    gtUInt64 src_origin0AsUInt64 = 0;
    ipcChannel >> src_origin0AsUInt64;
    _src_origin[0] = (gtSize_t)src_origin0AsUInt64;
    gtUInt64 src_origin1AsUInt64 = 0;
    ipcChannel >> src_origin1AsUInt64;
    _src_origin[1] = (gtSize_t)src_origin1AsUInt64;
    gtUInt64 src_origin2AsUInt64 = 0;
    ipcChannel >> src_origin2AsUInt64;
    _src_origin[2] = (gtSize_t)src_origin2AsUInt64;
    gtUInt64 dst_origin0AsUInt64 = 0;
    ipcChannel >> dst_origin0AsUInt64;
    _dst_origin[0] = (gtSize_t)dst_origin0AsUInt64;
    gtUInt64 dst_origin1AsUInt64 = 0;
    ipcChannel >> dst_origin1AsUInt64;
    _dst_origin[1] = (gtSize_t)dst_origin1AsUInt64;
    gtUInt64 dst_origin2AsUInt64 = 0;
    ipcChannel >> dst_origin2AsUInt64;
    _dst_origin[2] = (gtSize_t)dst_origin2AsUInt64;
    gtUInt64 region0AsUInt64 = 0;
    ipcChannel >> region0AsUInt64;
    _region[0] = (gtSize_t)region0AsUInt64;
    gtUInt64 region1AsUInt64 = 0;
    ipcChannel >> region1AsUInt64;
    _region[1] = (gtSize_t)region1AsUInt64;
    gtUInt64 region2AsUInt64 = 0;
    ipcChannel >> region2AsUInt64;
    _region[2] = (gtSize_t)region2AsUInt64;
    gtUInt64 src_row_pitchAsUInt64 = 0;
    ipcChannel >> src_row_pitchAsUInt64;
    _src_row_pitch = (gtSize_t)src_row_pitchAsUInt64;
    gtUInt64 src_slice_pitchAsUInt64 = 0;
    ipcChannel >> src_slice_pitchAsUInt64;
    _src_slice_pitch = (gtSize_t)src_slice_pitchAsUInt64;
    gtUInt64 dst_row_pitchAsUInt64 = 0;
    ipcChannel >> dst_row_pitchAsUInt64;
    _dst_row_pitch = (gtSize_t)dst_row_pitchAsUInt64;
    gtUInt64 dst_slice_pitchAsUInt64 = 0;
    ipcChannel >> dst_slice_pitchAsUInt64;
    _dst_slice_pitch = (gtSize_t)dst_slice_pitchAsUInt64;

    return retVal;
}

// -------------------------- apCLCopyBufferToImageCommand --------------------------

// ---------------------------------------------------------------------------
// Name:        apCLCopyBufferToImageCommand::apCLCopyBufferToImageCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLCopyBufferToImageCommand::apCLCopyBufferToImageCommand()
    : apCLEnqueuedCommand(), _src_buffer(OA_CL_NULL_HANDLE), _dst_image(OA_CL_NULL_HANDLE), _src_offset(0)
{
    _dst_origin[0] = 0;
    _dst_origin[1] = 0;
    _dst_origin[2] = 0;
    _region[0] = 0;
    _region[1] = 0;
    _region[2] = 0;
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyBufferToImageCommand::apCLCopyBufferToImageCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLCopyBufferToImageCommand::apCLCopyBufferToImageCommand(oaCLMemHandle src_buffer, oaCLMemHandle dst_image, gtSize_t src_offset, const gtSize_t dst_origin[3],
                                                           const gtSize_t region[3], cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
    : apCLEnqueuedCommand(num_events_in_wait_list, event_wait_list, event), _src_buffer(src_buffer), _dst_image(dst_image), _src_offset(src_offset)
{
    _dst_origin[0] = dst_origin[0];
    _dst_origin[1] = dst_origin[1];
    _dst_origin[2] = dst_origin[2];
    _region[0] = region[0];
    _region[1] = region[1];
    _region[2] = region[2];
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyBufferToImageCommand::~apCLCopyBufferToImageCommand
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLCopyBufferToImageCommand::~apCLCopyBufferToImageCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLCopyBufferToImageCommand::commandNameAsString
// Description: Gets the command name as a string
// Author:  AMD Developer Tools Team
// Date:        4/12/2009
// ---------------------------------------------------------------------------
void apCLCopyBufferToImageCommand::commandNameAsString(gtString& commandName) const
{
    commandName = L"CopyBufferToImage";
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyBufferToImageCommand::relatedSrcMemHandle
// Description: Returns the related src mem (buffer / texture) handles.
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
void apCLCopyBufferToImageCommand::getRelatedSrcMemHandles(gtVector<oaCLMemHandle>& srcMemHandles) const
{
    srcMemHandles.clear();
    srcMemHandles.push_back(_src_buffer);
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyBufferToImageCommand::relatedDstMemHandle
// Description: Returns the related dst mem (buffer / texture) handles.
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
void apCLCopyBufferToImageCommand::getRelatedDstMemHandles(gtVector<oaCLMemHandle>& dstMemHandles) const
{
    dstMemHandles.clear();
    dstMemHandles.push_back(_dst_image);
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyBufferToImageCommand::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLCopyBufferToImageCommand::type() const
{
    return OS_TOBJ_ID_CL_COPY_BUFFER_TO_IMAGE_COMMAND;
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyBufferToImageCommand::writeSelfIntoChannel
// Description: Writes the command into an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLCopyBufferToImageCommand::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apCLEnqueuedCommand::writeSelfIntoChannel(ipcChannel);

    ipcChannel << (gtUInt64)_src_buffer;
    ipcChannel << (gtUInt64)_dst_image;
    ipcChannel << (gtUInt64)_src_offset;
    ipcChannel << (gtUInt64)_dst_origin[0];
    ipcChannel << (gtUInt64)_dst_origin[1];
    ipcChannel << (gtUInt64)_dst_origin[2];
    ipcChannel << (gtUInt64)_region[0];
    ipcChannel << (gtUInt64)_region[1];
    ipcChannel << (gtUInt64)_region[2];

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyBufferToImageCommand::readSelfFromChannel
// Description: Reads the command from an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLCopyBufferToImageCommand::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apCLEnqueuedCommand::readSelfFromChannel(ipcChannel);

    gtUInt64 src_bufferAsUInt64 = 0;
    ipcChannel >> src_bufferAsUInt64;
    _src_buffer = (oaCLMemHandle)src_bufferAsUInt64;
    gtUInt64 dst_imageAsUInt64 = 0;
    ipcChannel >> dst_imageAsUInt64;
    _dst_image = (oaCLMemHandle)dst_imageAsUInt64;
    gtUInt64 src_offsetAsUInt64 = 0;
    ipcChannel >> src_offsetAsUInt64;
    _src_offset = (gtSize_t)src_offsetAsUInt64;
    gtUInt64 dst_origin0AsUInt64 = 0;
    ipcChannel >> dst_origin0AsUInt64;
    _dst_origin[0] = (gtSize_t)dst_origin0AsUInt64;
    gtUInt64 dst_origin1AsUInt64 = 0;
    ipcChannel >> dst_origin1AsUInt64;
    _dst_origin[1] = (gtSize_t)dst_origin1AsUInt64;
    gtUInt64 dst_origin2AsUInt64 = 0;
    ipcChannel >> dst_origin2AsUInt64;
    _dst_origin[2] = (gtSize_t)dst_origin2AsUInt64;
    gtUInt64 region0AsUInt64 = 0;
    ipcChannel >> region0AsUInt64;
    _region[0] = (gtSize_t)region0AsUInt64;
    gtUInt64 region1AsUInt64 = 0;
    ipcChannel >> region1AsUInt64;
    _region[1] = (gtSize_t)region1AsUInt64;
    gtUInt64 region2AsUInt64 = 0;
    ipcChannel >> region2AsUInt64;
    _region[2] = (gtSize_t)region2AsUInt64;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyBufferToImageCommand::amountOfPixels
// Description: Amount of pixels requested for write in this command
// Return Val:  gtSize_t
// Author:  AMD Developer Tools Team
// Date:        6/5/2010
// ---------------------------------------------------------------------------
gtSize_t apCLCopyBufferToImageCommand::amountOfPixels()
{
    gtSize_t retVal = 1;

    // Calculate the amount of pixels according to the
    if (_region[0] != 0)
    {
        retVal *= _region[0];
    }

    if (_region[1] != 0)
    {
        retVal *= _region[1];
    }

    if (_region[2] != 0)
    {
        retVal *= _region[2];
    }

    return retVal;
}

// ------------------------------ apCLCopyImageCommand ------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLCopyImageCommand::apCLCopyImageCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLCopyImageCommand::apCLCopyImageCommand()
    : apCLEnqueuedCommand(), _src_image(OA_CL_NULL_HANDLE), _dst_image(OA_CL_NULL_HANDLE)
{
    _src_origin[0] = 0;
    _src_origin[1] = 0;
    _src_origin[2] = 0;
    _dst_origin[0] = 0;
    _dst_origin[1] = 0;
    _dst_origin[2] = 0;
    _region[0] = 0;
    _region[1] = 0;
    _region[2] = 0;
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyImageCommand::apCLCopyImageCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLCopyImageCommand::apCLCopyImageCommand(oaCLMemHandle src_image, oaCLMemHandle dst_image, const gtSize_t src_origin[3], const gtSize_t dst_origin[3],
                                           const gtSize_t region[3], cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
    : apCLEnqueuedCommand(num_events_in_wait_list, event_wait_list, event), _src_image(src_image), _dst_image(dst_image)
{
    _src_origin[0] = src_origin[0];
    _src_origin[1] = src_origin[1];
    _src_origin[2] = src_origin[2];
    _dst_origin[0] = dst_origin[0];
    _dst_origin[1] = dst_origin[1];
    _dst_origin[2] = dst_origin[2];
    _region[0] = region[0];
    _region[1] = region[1];
    _region[2] = region[2];
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyImageCommand::~apCLCopyImageCommand
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLCopyImageCommand::~apCLCopyImageCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLCopyImageCommand::commandNameAsString
// Description: Gets the command name as a string
// Author:  AMD Developer Tools Team
// Date:        4/12/2009
// ---------------------------------------------------------------------------
void apCLCopyImageCommand::commandNameAsString(gtString& commandName) const
{
    commandName = L"CopyImage";
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyImageCommand::relatedSrcMemHandle
// Description: Returns the related src mem (buffer / texture) handles.
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
void apCLCopyImageCommand::getRelatedSrcMemHandles(gtVector<oaCLMemHandle>& srcMemHandles) const
{
    srcMemHandles.clear();
    srcMemHandles.push_back(_src_image);
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyImageCommand::relatedDstMemHandle
// Description: Returns the related dst mem (buffer / texture) handles.
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
void apCLCopyImageCommand::getRelatedDstMemHandles(gtVector<oaCLMemHandle>& dstMemHandles) const
{
    dstMemHandles.clear();
    dstMemHandles.push_back(_dst_image);
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyImageCommand::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLCopyImageCommand::type() const
{
    return OS_TOBJ_ID_CL_COPY_IMAGE_COMMAND;
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyImageCommand::writeSelfIntoChannel
// Description: Writes the command into an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLCopyImageCommand::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apCLEnqueuedCommand::writeSelfIntoChannel(ipcChannel);

    ipcChannel << (gtUInt64)_src_image;
    ipcChannel << (gtUInt64)_dst_image;
    ipcChannel << (gtUInt64)_src_origin[0];
    ipcChannel << (gtUInt64)_src_origin[1];
    ipcChannel << (gtUInt64)_src_origin[2];
    ipcChannel << (gtUInt64)_dst_origin[0];
    ipcChannel << (gtUInt64)_dst_origin[1];
    ipcChannel << (gtUInt64)_dst_origin[2];
    ipcChannel << (gtUInt64)_region[0];
    ipcChannel << (gtUInt64)_region[1];
    ipcChannel << (gtUInt64)_region[2];

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyImageCommand::readSelfFromChannel
// Description: Reads the command from an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLCopyImageCommand::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apCLEnqueuedCommand::readSelfFromChannel(ipcChannel);

    gtUInt64 src_imageAsUInt64 = 0;
    ipcChannel >> src_imageAsUInt64;
    _src_image = (oaCLMemHandle)src_imageAsUInt64;
    gtUInt64 dst_imageAsUInt64 = 0;
    ipcChannel >> dst_imageAsUInt64;
    _dst_image = (oaCLMemHandle)dst_imageAsUInt64;
    gtUInt64 src_origin0AsUInt64 = 0;
    ipcChannel >> src_origin0AsUInt64;
    _src_origin[0] = (gtSize_t)src_origin0AsUInt64;
    gtUInt64 src_origin1AsUInt64 = 0;
    ipcChannel >> src_origin1AsUInt64;
    _src_origin[1] = (gtSize_t)src_origin1AsUInt64;
    gtUInt64 src_origin2AsUInt64 = 0;
    ipcChannel >> src_origin2AsUInt64;
    _src_origin[2] = (gtSize_t)src_origin2AsUInt64;
    gtUInt64 dst_origin0AsUInt64 = 0;
    ipcChannel >> dst_origin0AsUInt64;
    _dst_origin[0] = (gtSize_t)dst_origin0AsUInt64;
    gtUInt64 dst_origin1AsUInt64 = 0;
    ipcChannel >> dst_origin1AsUInt64;
    _dst_origin[1] = (gtSize_t)dst_origin1AsUInt64;
    gtUInt64 dst_origin2AsUInt64 = 0;
    ipcChannel >> dst_origin2AsUInt64;
    _dst_origin[2] = (gtSize_t)dst_origin2AsUInt64;
    gtUInt64 region0AsUInt64 = 0;
    ipcChannel >> region0AsUInt64;
    _region[0] = (gtSize_t)region0AsUInt64;
    gtUInt64 region1AsUInt64 = 0;
    ipcChannel >> region1AsUInt64;
    _region[1] = (gtSize_t)region1AsUInt64;
    gtUInt64 region2AsUInt64 = 0;
    ipcChannel >> region2AsUInt64;
    _region[2] = (gtSize_t)region2AsUInt64;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apCLCopyImageCommand::amountOfPixels
// Description: Amount of pixels requested for write in this command
// Return Val:  gtSize_t
// Author:  AMD Developer Tools Team
// Date:        6/5/2010
// ---------------------------------------------------------------------------
gtSize_t apCLCopyImageCommand::amountOfPixels()
{
    gtSize_t retVal = 1;

    // Calculate the amount of pixels according to the
    if (_region[0] != 0)
    {
        retVal *= _region[0];
    }

    if (_region[1] != 0)
    {
        retVal *= _region[1];
    }

    if (_region[2] != 0)
    {
        retVal *= _region[2];
    }

    return retVal;
}


// -------------------------- apCLCopyImageToBufferCommand --------------------------

// ---------------------------------------------------------------------------
// Name:        apCLCopyImageToBufferCommand::apCLCopyImageToBufferCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLCopyImageToBufferCommand::apCLCopyImageToBufferCommand()
    : apCLEnqueuedCommand(), _src_image(OA_CL_NULL_HANDLE), _dst_buffer(OA_CL_NULL_HANDLE), _dst_offset(0)
{
    _src_origin[0] = 0;
    _src_origin[1] = 0;
    _src_origin[2] = 0;
    _region[0] = 0;
    _region[1] = 0;
    _region[2] = 0;
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyImageToBufferCommand::apCLCopyImageToBufferCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLCopyImageToBufferCommand::apCLCopyImageToBufferCommand(oaCLMemHandle src_image, oaCLMemHandle dst_buffer, const gtSize_t src_origin[3], const gtSize_t region[3],
                                                           gtSize_t dst_offset, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
    : apCLEnqueuedCommand(num_events_in_wait_list, event_wait_list, event), _src_image(src_image), _dst_buffer(dst_buffer), _dst_offset(dst_offset)
{
    _src_origin[0] = src_origin[0];
    _src_origin[1] = src_origin[1];
    _src_origin[2] = src_origin[2];
    _region[0] = region[0];
    _region[1] = region[1];
    _region[2] = region[2];
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyImageToBufferCommand::~apCLCopyImageToBufferCommand
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLCopyImageToBufferCommand::~apCLCopyImageToBufferCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLCopyImageToBufferCommand::commandNameAsString
// Description: Gets the command name as a string
// Author:  AMD Developer Tools Team
// Date:        4/12/2009
// ---------------------------------------------------------------------------
void apCLCopyImageToBufferCommand::commandNameAsString(gtString& commandName) const
{
    commandName = L"CopyImageToBuffer";
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyImageToBufferCommand::relatedSrcMemHandle
// Description: Returns the related src mem (buffer / texture) handles.
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
void apCLCopyImageToBufferCommand::getRelatedSrcMemHandles(gtVector<oaCLMemHandle>& srcMemHandles) const
{
    srcMemHandles.clear();
    srcMemHandles.push_back(_src_image);
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyImageToBufferCommand::relatedDstMemHandle
// Description: Returns the related dst mem (buffer / texture) handles.
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
void apCLCopyImageToBufferCommand::getRelatedDstMemHandles(gtVector<oaCLMemHandle>& dstMemHandles) const
{
    dstMemHandles.clear();
    dstMemHandles.push_back(_dst_buffer);
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyImageToBufferCommand::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLCopyImageToBufferCommand::type() const
{
    return OS_TOBJ_ID_CL_COPY_IMAGE_TO_BUFFER_COMMAND;
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyImageToBufferCommand::writeSelfIntoChannel
// Description: Writes the command into an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLCopyImageToBufferCommand::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apCLEnqueuedCommand::writeSelfIntoChannel(ipcChannel);

    ipcChannel << (gtUInt64)_src_image;
    ipcChannel << (gtUInt64)_dst_buffer;
    ipcChannel << (gtUInt64)_src_origin[0];
    ipcChannel << (gtUInt64)_src_origin[1];
    ipcChannel << (gtUInt64)_src_origin[2];
    ipcChannel << (gtUInt64)_region[0];
    ipcChannel << (gtUInt64)_region[1];
    ipcChannel << (gtUInt64)_region[2];
    ipcChannel << (gtUInt64)_dst_offset;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyImageToBufferCommand::readSelfFromChannel
// Description: Reads the command from an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLCopyImageToBufferCommand::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apCLEnqueuedCommand::readSelfFromChannel(ipcChannel);

    gtUInt64 src_imageAsUInt64 = 0;
    ipcChannel >> src_imageAsUInt64;
    _src_image = (oaCLMemHandle)src_imageAsUInt64;
    gtUInt64 dst_bufferAsUInt64 = 0;
    ipcChannel >> dst_bufferAsUInt64;
    _dst_buffer = (oaCLMemHandle)dst_bufferAsUInt64;
    gtUInt64 src_origin0AsUInt64 = 0;
    ipcChannel >> src_origin0AsUInt64;
    _src_origin[0] = (gtSize_t)src_origin0AsUInt64;
    gtUInt64 src_origin1AsUInt64 = 0;
    ipcChannel >> src_origin1AsUInt64;
    _src_origin[1] = (gtSize_t)src_origin1AsUInt64;
    gtUInt64 src_origin2AsUInt64 = 0;
    ipcChannel >> src_origin2AsUInt64;
    _src_origin[2] = (gtSize_t)src_origin2AsUInt64;
    gtUInt64 region0AsUInt64 = 0;
    ipcChannel >> region0AsUInt64;
    _region[0] = (gtSize_t)region0AsUInt64;
    gtUInt64 region1AsUInt64 = 0;
    ipcChannel >> region1AsUInt64;
    _region[1] = (gtSize_t)region1AsUInt64;
    gtUInt64 region2AsUInt64 = 0;
    ipcChannel >> region2AsUInt64;
    _region[2] = (gtSize_t)region2AsUInt64;
    gtUInt64 dst_offsetAsUInt64 = 0;
    ipcChannel >> dst_offsetAsUInt64;
    _dst_offset = (gtSize_t)dst_offsetAsUInt64;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLCopyImageToBufferCommand::amountOfPixels
// Description: Amount of pixels requested for write in this command
// Return Val:  gtSize_t
// Author:  AMD Developer Tools Team
// Date:        6/5/2010
// ---------------------------------------------------------------------------
gtSize_t apCLCopyImageToBufferCommand::amountOfPixels()
{
    gtSize_t retVal = 1;

    // Calculate the amount of pixels according to the
    if (_region[0] != 0)
    {
        retVal *= _region[0];
    }

    if (_region[1] != 0)
    {
        retVal *= _region[1];
    }

    if (_region[2] != 0)
    {
        retVal *= _region[2];
    }

    return retVal;
}

// ------------------------------ apCLMapBufferCommand ------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLMapBufferCommand::apCLMapBufferCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLMapBufferCommand::apCLMapBufferCommand()
    : apCLEnqueuedCommand(), _buffer(OA_CL_NULL_HANDLE), _blocking_map(0), _map_flags(0), _offset(0), _cb(0), _errcode_ret(OS_NULL_PROCEDURE_ADDRESS_64)
{

}

// ---------------------------------------------------------------------------
// Name:        apCLMapBufferCommand::apCLMapBufferCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLMapBufferCommand::apCLMapBufferCommand(oaCLMemHandle buffer, cl_bool blocking_map, cl_map_flags map_flags, gtSize_t offset, gtSize_t cb,
                                           cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event, osProcedureAddress64 errcode_ret)
    : apCLEnqueuedCommand(num_events_in_wait_list, event_wait_list, event), _buffer(buffer), _blocking_map(blocking_map), _map_flags(map_flags), _offset(offset), _cb(cb), _errcode_ret(errcode_ret)
{

}

// ---------------------------------------------------------------------------
// Name:        apCLMapBufferCommand::~apCLMapBufferCommand
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLMapBufferCommand::~apCLMapBufferCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLMapBufferCommand::commandNameAsString
// Description: Gets the command name as a string
// Author:  AMD Developer Tools Team
// Date:        4/12/2009
// ---------------------------------------------------------------------------
void apCLMapBufferCommand::commandNameAsString(gtString& commandName) const
{
    commandName = L"MapBuffer";
}

// ---------------------------------------------------------------------------
// Name:        apCLMapBufferCommand::relatedDstMemHandle
// Description: Returns the related dst mem (buffer / texture) handles.
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
void apCLMapBufferCommand::getRelatedDstMemHandles(gtVector<oaCLMemHandle>& dstMemHandles) const
{
    dstMemHandles.clear();
    dstMemHandles.push_back(_buffer);
}

// ---------------------------------------------------------------------------
// Name:        apCLMapBufferCommand::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLMapBufferCommand::type() const
{
    return OS_TOBJ_ID_CL_MAP_BUFFER_COMMAND;
}

// ---------------------------------------------------------------------------
// Name:        apCLMapBufferCommand::writeSelfIntoChannel
// Description: Writes the command into an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLMapBufferCommand::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apCLEnqueuedCommand::writeSelfIntoChannel(ipcChannel);

    ipcChannel << (gtUInt64)_buffer;
    ipcChannel << (gtUInt32)_blocking_map;
    ipcChannel << (gtUInt32)_map_flags;
    ipcChannel << (gtUInt64)_offset;
    ipcChannel << (gtUInt64)_cb;
    ipcChannel << (gtUInt64)_errcode_ret;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLMapBufferCommand::readSelfFromChannel
// Description: Reads the command from an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLMapBufferCommand::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apCLEnqueuedCommand::readSelfFromChannel(ipcChannel);

    gtUInt64 bufferAsUInt64 = 0;
    ipcChannel >> bufferAsUInt64;
    _buffer = (oaCLMemHandle)bufferAsUInt64;
    gtUInt32 blocking_mapAsUInt32 = 0;
    ipcChannel >> blocking_mapAsUInt32;
    _blocking_map = (cl_bool)blocking_mapAsUInt32;
    gtUInt32 map_flagsAsUInt32 = 0;
    ipcChannel >> map_flagsAsUInt32;
    _map_flags = (cl_map_flags)map_flagsAsUInt32;
    gtUInt64 offsetAsUInt64 = 0;
    ipcChannel >> offsetAsUInt64;
    _offset = (gtSize_t)offsetAsUInt64;
    gtUInt64 cbAsUInt64 = 0;
    ipcChannel >> cbAsUInt64;
    _cb = (gtSize_t)cbAsUInt64;
    gtUInt64 errcode_retAsUInt64 = 0;
    ipcChannel >> errcode_retAsUInt64;
    _errcode_ret = (osProcedureAddress64)errcode_retAsUInt64;

    return retVal;
}

// ------------------------------ apCLMapImageCommand -------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLMapImageCommand::apCLMapImageCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLMapImageCommand::apCLMapImageCommand()
    : apCLEnqueuedCommand(), _image(OA_CL_NULL_HANDLE), _blocking_map(0), _map_flags(0), _image_row_pitch(0), _image_slice_pitch(0), _errcode_ret(OS_NULL_PROCEDURE_ADDRESS_64)
{
    _origin[0] = 0;
    _origin[1] = 0;
    _origin[2] = 0;
    _region[0] = 0;
    _region[1] = 0;
    _region[2] = 0;
}

// ---------------------------------------------------------------------------
// Name:        apCLMapImageCommand::apCLMapImageCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLMapImageCommand::apCLMapImageCommand(oaCLMemHandle image, cl_bool blocking_map, cl_map_flags map_flags, const gtSize_t origin[3],
                                         const gtSize_t region[3], gtSize_t* image_row_pitch, gtSize_t* image_slice_pitch,
                                         cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event, osProcedureAddress64 errcode_ret)
    : apCLEnqueuedCommand(num_events_in_wait_list, event_wait_list, event), _image(image), _blocking_map(blocking_map), _map_flags(map_flags),
      _image_row_pitch(0), _image_slice_pitch(0), _errcode_ret(errcode_ret)
{
    GT_IF_WITH_ASSERT(image_row_pitch != NULL)
    {
        _image_row_pitch = *image_row_pitch;
    }

    if (image_slice_pitch != NULL)
    {
        _image_slice_pitch = *image_slice_pitch;
    }

    _origin[0] = origin[0];
    _origin[1] = origin[1];
    _origin[2] = origin[2];
    _region[0] = region[0];
    _region[1] = region[1];
    _region[2] = region[2];
}

// ---------------------------------------------------------------------------
// Name:        apCLMapImageCommand::~apCLMapImageCommand
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLMapImageCommand::~apCLMapImageCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLMapImageCommand::commandNameAsString
// Description: Gets the command name as a string
// Author:  AMD Developer Tools Team
// Date:        4/12/2009
// ---------------------------------------------------------------------------
void apCLMapImageCommand::commandNameAsString(gtString& commandName) const
{
    commandName = L"MapImage";
}

// ---------------------------------------------------------------------------
// Name:        apCLMapImageCommand::relatedDstMemHandle
// Description: Returns the related dst mem (buffer / texture) handles.
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
void apCLMapImageCommand::getRelatedDstMemHandles(gtVector<oaCLMemHandle>& dstMemHandles) const
{
    dstMemHandles.clear();
    dstMemHandles.push_back(_image);
}

// ---------------------------------------------------------------------------
// Name:        apCLMapImageCommand::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLMapImageCommand::type() const
{
    return OS_TOBJ_ID_CL_MAP_IMAGE_COMMAND;
}

// ---------------------------------------------------------------------------
// Name:        apCLMapImageCommand::writeSelfIntoChannel
// Description: Writes the command into an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLMapImageCommand::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apCLEnqueuedCommand::writeSelfIntoChannel(ipcChannel);

    ipcChannel << (gtUInt64)_image;
    ipcChannel << (gtUInt32)_blocking_map;
    ipcChannel << (gtUInt32)_map_flags;
    ipcChannel << (gtUInt64)_origin[0];
    ipcChannel << (gtUInt64)_origin[1];
    ipcChannel << (gtUInt64)_origin[2];
    ipcChannel << (gtUInt64)_region[0];
    ipcChannel << (gtUInt64)_region[1];
    ipcChannel << (gtUInt64)_region[2];
    ipcChannel << (gtUInt64)_image_row_pitch;
    ipcChannel << (gtUInt64)_image_slice_pitch;
    ipcChannel << (gtUInt64)_errcode_ret;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLMapImageCommand::readSelfFromChannel
// Description: Reads the command from an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLMapImageCommand::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apCLEnqueuedCommand::readSelfFromChannel(ipcChannel);

    gtUInt64 imageAsUInt64 = 0;
    ipcChannel >> imageAsUInt64;
    _image = (oaCLMemHandle)imageAsUInt64;
    gtUInt32 blocking_mapAsUInt32 = 0;
    ipcChannel >> blocking_mapAsUInt32;
    _blocking_map = (cl_bool)blocking_mapAsUInt32;
    gtUInt32 map_flagsAsUInt32 = 0;
    ipcChannel >> map_flagsAsUInt32;
    _map_flags = (cl_map_flags)map_flagsAsUInt32;
    gtUInt64 origin0AsUInt64 = 0;
    ipcChannel >> origin0AsUInt64;
    _origin[0] = (gtSize_t)origin0AsUInt64;
    gtUInt64 origin1AsUInt64 = 0;
    ipcChannel >> origin1AsUInt64;
    _origin[1] = (gtSize_t)origin1AsUInt64;
    gtUInt64 origin2AsUInt64 = 0;
    ipcChannel >> origin2AsUInt64;
    _origin[2] = (gtSize_t)origin2AsUInt64;
    gtUInt64 region0AsUInt64 = 0;
    ipcChannel >> region0AsUInt64;
    _region[0] = (gtSize_t)region0AsUInt64;
    gtUInt64 region1AsUInt64 = 0;
    ipcChannel >> region1AsUInt64;
    _region[1] = (gtSize_t)region1AsUInt64;
    gtUInt64 region2AsUInt64 = 0;
    ipcChannel >> region2AsUInt64;
    _region[2] = (gtSize_t)region2AsUInt64;
    gtUInt64 image_row_pitchAsUInt64 = 0;
    ipcChannel >> image_row_pitchAsUInt64;
    _image_row_pitch = (gtSize_t)image_row_pitchAsUInt64;
    gtUInt64 image_slice_pitchAsUInt64 = 0;
    ipcChannel >> image_slice_pitchAsUInt64;
    _image_slice_pitch = (gtSize_t)image_slice_pitchAsUInt64;
    gtUInt64 errcode_retAsUInt64 = 0;
    ipcChannel >> errcode_retAsUInt64;
    _errcode_ret = (osProcedureAddress64)errcode_retAsUInt64;

    return retVal;
}

// -------------------------------- apCLMarkerCommand -------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLMarkerCommand::apCLMarkerCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLMarkerCommand::apCLMarkerCommand()
    : apCLEnqueuedCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLMarkerCommand::apCLMarkerCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLMarkerCommand::apCLMarkerCommand(cl_event* event)
    : apCLEnqueuedCommand(0, NULL, event)
{

}

// ---------------------------------------------------------------------------
// Name:        apCLMarkerCommand::~apCLMarkerCommand
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLMarkerCommand::~apCLMarkerCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLMarkerCommand::commandNameAsString
// Description: Gets the command name as a string
// Author:  AMD Developer Tools Team
// Date:        4/12/2009
// ---------------------------------------------------------------------------
void apCLMarkerCommand::commandNameAsString(gtString& commandName) const
{
    commandName = L"Marker";
}

// ---------------------------------------------------------------------------
// Name:        apCLMarkerCommand::eventWaitListParameterAvailable
// Description: Returns true iff the specific class has an "Event wait list" parameter.
// Author:  AMD Developer Tools Team
// Date:        4/3/2010
// ---------------------------------------------------------------------------
bool apCLMarkerCommand::eventWaitListParameterAvailable() const
{
    return false;
}

// ---------------------------------------------------------------------------
// Name:        apCLMarkerCommand::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLMarkerCommand::type() const
{
    return OS_TOBJ_ID_CL_MARKER_COMMAND;
}

// ---------------------------------------------------------------------------
// Name:        apCLMarkerCommand::writeSelfIntoChannel
// Description: Writes the command into an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLMarkerCommand::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apCLEnqueuedCommand::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLMarkerCommand::readSelfFromChannel
// Description: Reads the command from an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLMarkerCommand::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apCLEnqueuedCommand::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------- apCLNativeKernelCommand -----------------------------

// ---------------------------------------------------------------------------
// Name:        apCLNativeKernelCommand::apCLNativeKernelCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLNativeKernelCommand::apCLNativeKernelCommand()
    : apCLEnqueuedCommand(), _user_func(OS_NULL_PROCEDURE_ADDRESS_64), _args(OS_NULL_PROCEDURE_ADDRESS_64), _cb_args(0)
{

}

// ---------------------------------------------------------------------------
// Name:        apCLNativeKernelCommand::apCLNativeKernelCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLNativeKernelCommand::apCLNativeKernelCommand(osProcedureAddress64 user_func, osProcedureAddress64 args, gtSize_t cb_args, cl_uint num_mem_objects,
                                                 const cl_mem* mem_list, const void** args_mem_loc, cl_uint num_events_in_wait_list,
                                                 const cl_event* event_wait_list, cl_event* event)
    : apCLEnqueuedCommand(num_events_in_wait_list, event_wait_list, event), _user_func(user_func), _args(args), _cb_args(cb_args)
{
    // Add the mem handles and memory locations:
    for (cl_uint i = 0; i < num_mem_objects; i++)
    {
        _mem_list.push_back((oaCLMemHandle)mem_list[i]);
        _args_mem_loc.push_back((osProcedureAddress64)args_mem_loc[i]);
    }
}

// ---------------------------------------------------------------------------
// Name:        apCLNativeKernelCommand::~apCLNativeKernelCommand
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLNativeKernelCommand::~apCLNativeKernelCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLNativeKernelCommand::commandNameAsString
// Description: Gets the command name as a string
// Author:  AMD Developer Tools Team
// Date:        4/12/2009
// ---------------------------------------------------------------------------
void apCLNativeKernelCommand::commandNameAsString(gtString& commandName) const
{
    commandName = L"NativeKernel";
}

// ---------------------------------------------------------------------------
// Name:        apCLNativeKernelCommand::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLNativeKernelCommand::type() const
{
    return OS_TOBJ_ID_CL_NATIVE_KERNEL_COMMAND;
}

// ---------------------------------------------------------------------------
// Name:        apCLNativeKernelCommand::writeSelfIntoChannel
// Description: Writes the command into an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLNativeKernelCommand::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apCLEnqueuedCommand::writeSelfIntoChannel(ipcChannel);

    ipcChannel << (gtUInt64)_user_func;
    ipcChannel << (gtUInt64)_args;
    ipcChannel << (gtUInt64)_cb_args;
    gtUInt32 num_mem_objects = (gtUInt32)_mem_list.size();
    gtUInt32 amountOfArgMemLocations = (gtUInt32)_args_mem_loc.size();

    if (num_mem_objects != amountOfArgMemLocations)
    {
        GT_ASSERT(num_mem_objects == amountOfArgMemLocations);
        num_mem_objects = std::min(num_mem_objects, amountOfArgMemLocations);
    }

    ipcChannel << (gtUInt32)num_mem_objects;

    for (gtUInt32 i = 0; i < num_mem_objects; i++)
    {
        ipcChannel << (gtUInt64)_mem_list[i];
        ipcChannel << (gtUInt64)_args_mem_loc[i];
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLNativeKernelCommand::readSelfFromChannel
// Description: Reads the command from an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLNativeKernelCommand::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apCLEnqueuedCommand::readSelfFromChannel(ipcChannel);

    gtUInt64 user_funcAsUInt64 = 0;
    ipcChannel >> user_funcAsUInt64;
    _user_func = (osProcedureAddress64)user_funcAsUInt64;
    gtUInt64 argsAsUInt64 = 0;
    ipcChannel >> argsAsUInt64;
    _args = (osProcedureAddress64)argsAsUInt64;
    gtUInt64 cb_argsAsUInt64 = 0;
    ipcChannel >> cb_argsAsUInt64;
    _cb_args = (gtSize_t)cb_argsAsUInt64;
    gtUInt32 num_mem_objects = 0;
    ipcChannel >> num_mem_objects;

    for (gtUInt32 i = 0; i < num_mem_objects; i++)
    {
        gtUInt64 mem_listAsUInt64 = 0;
        ipcChannel >> mem_listAsUInt64;
        _mem_list.push_back((oaCLMemHandle)mem_listAsUInt64);
        gtUInt64 args_mem_locAsUInt64 = 0;
        ipcChannel >> args_mem_locAsUInt64;
        _args_mem_loc.push_back((osProcedureAddress64)args_mem_locAsUInt64);
    }

    return retVal;
}

// ---------------------------- apCLNDRangeKernelCommand ----------------------------

// ---------------------------------------------------------------------------
// Name:        apCLNDRangeKernelCommand::apCLNDRangeKernelCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLNDRangeKernelCommand::apCLNDRangeKernelCommand()
    : apCLEnqueuedCommand(), _kernel(OA_CL_NULL_HANDLE), _work_dim(0)
{

}

// ---------------------------------------------------------------------------
// Name:        apCLNDRangeKernelCommand::apCLNDRangeKernelCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLNDRangeKernelCommand::apCLNDRangeKernelCommand(oaCLKernelHandle kernel, cl_uint work_dim, const gtSize_t* global_work_offset, const gtSize_t* global_work_size,
                                                   const gtSize_t* local_work_size, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
    : apCLEnqueuedCommand(num_events_in_wait_list, event_wait_list, event), _kernel(kernel), _work_dim(work_dim)
{
    // Add the work size data:
    for (cl_uint i = 0; i < work_dim; i++)
    {
        // Add the global work offset coordinates:
        if (global_work_offset != NULL)
        {
            _global_work_offset.push_back(global_work_offset[i]);
        }
        else
        {
            _global_work_offset.push_back(0);
        }

        // Add the global work size:
        if (global_work_size != NULL)
        {
            _global_work_size.push_back(global_work_size[i]);
        }
        else
        {
            // Only assert once here:
            GT_ASSERT_EX((i != 0), L"global_work_size != NULL");
            _global_work_size.push_back(0);
        }

        // Add the local work size:
        if (local_work_size != NULL)
        {
            _local_work_size.push_back(local_work_size[i]);
        }
        else
        {
            _local_work_size.push_back(0);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        apCLNDRangeKernelCommand::~apCLNDRangeKernelCommand
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLNDRangeKernelCommand::~apCLNDRangeKernelCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLNDRangeKernelCommand::commandNameAsString
// Description: Gets the command name as a string
// Author:  AMD Developer Tools Team
// Date:        4/12/2009
// ---------------------------------------------------------------------------
void apCLNDRangeKernelCommand::commandNameAsString(gtString& commandName) const
{
    commandName = L"NDRangeKernel";
}

// ---------------------------------------------------------------------------
// Name:        apCLNDRangeKernelCommand::relatedKernelHandle
// Description: Returns the related kernel handles.
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
void apCLNDRangeKernelCommand::getRelatedKernelHandles(gtVector<oaCLKernelHandle>& kernelHandles) const
{
    kernelHandles.clear();
    kernelHandles.push_back(_kernel);
}

// ---------------------------------------------------------------------------
// Name:        apCLNDRangeKernelCommand::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLNDRangeKernelCommand::type() const
{
    return OS_TOBJ_ID_CL_ND_RANGE_KERNEL_COMMAND;
}

// ---------------------------------------------------------------------------
// Name:        apCLNDRangeKernelCommand::writeSelfIntoChannel
// Description: Writes the command into an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLNDRangeKernelCommand::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apCLEnqueuedCommand::writeSelfIntoChannel(ipcChannel);

    ipcChannel << (gtUInt64)_kernel;
    ipcChannel << (gtUInt32)_work_dim;
    cl_uint gOffsetDim = (cl_uint)_global_work_offset.size();
    cl_uint gSizeDim = (cl_uint)_global_work_size.size();
    cl_uint lSizeDim = (cl_uint)_local_work_size.size();

    for (cl_uint i = 0; i < _work_dim; i++)
    {
        // If one of our vectors is too small by error, assert it, but write the
        // missing members as 0:
        GT_IF_WITH_ASSERT(i < gOffsetDim)
        {
            ipcChannel << (gtUInt64)_global_work_offset[i];
        }
        else
        {
            ipcChannel << (gtUInt64)0;
        }

        GT_IF_WITH_ASSERT(i < gSizeDim)
        {
            ipcChannel << (gtUInt64)_global_work_size[i];
        }
        else
        {
            ipcChannel << (gtUInt64)0;
        }

        GT_IF_WITH_ASSERT(i < lSizeDim)
        {
            ipcChannel << (gtUInt64)_local_work_size[i];
        }
        else
        {
            ipcChannel << (gtUInt64)0;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLNDRangeKernelCommand::readSelfFromChannel
// Description: Reads the command from an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLNDRangeKernelCommand::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apCLEnqueuedCommand::readSelfFromChannel(ipcChannel);

    gtUInt64 kernelAsUInt64 = 0;
    ipcChannel >> kernelAsUInt64;
    _kernel = (oaCLKernelHandle)kernelAsUInt64;
    gtUInt32 work_dimAsUInt32 = 0;
    ipcChannel >> work_dimAsUInt32;
    _work_dim = (cl_uint)work_dimAsUInt32;

    for (gtUInt32 i = 0; i < work_dimAsUInt32; i++)
    {
        gtUInt64 global_work_offsetAsUInt64 = 0;
        ipcChannel >> global_work_offsetAsUInt64;
        _global_work_offset.push_back((gtSize_t)global_work_offsetAsUInt64);
        gtUInt64 global_work_sizeAsUInt64 = 0;
        ipcChannel >> global_work_sizeAsUInt64;
        _global_work_size.push_back((gtSize_t)global_work_sizeAsUInt64);
        gtUInt64 local_work_sizeAsUInt64 = 0;
        ipcChannel >> local_work_sizeAsUInt64;
        _local_work_size.push_back((gtSize_t)local_work_sizeAsUInt64);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apCLNDRangeKernelCommand::amountOfWorkItems
// Description: Return the amount of work items
// Return Val:  gtUInt64
// Author:  AMD Developer Tools Team
// Date:        6/5/2010
// ---------------------------------------------------------------------------
gtUInt64 apCLNDRangeKernelCommand::amountOfWorkItems()
{
    gtUInt64 retVal = 1;

    for (int i = 0; i < (int)_global_work_size.size(); i++)
    {
        if (_global_work_size[i] != 0)
        {
            retVal *= _global_work_size[i];
        }
    }

    return retVal;
}

// ----------------------------------------- apCLReadBufferCommand -----------------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLReadBufferCommand::apCLReadBufferCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLReadBufferCommand::apCLReadBufferCommand()
    : apCLEnqueuedCommand(), _buffer(OA_CL_NULL_HANDLE), _blocking_read(0), _offset(0), _cb(0), _ptr(OS_NULL_PROCEDURE_ADDRESS_64)
{

}

// ---------------------------------------------------------------------------
// Name:        apCLReadBufferCommand::apCLReadBufferCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLReadBufferCommand::apCLReadBufferCommand(oaCLMemHandle buffer, cl_bool blocking_read, gtSize_t offset, gtSize_t cb, osProcedureAddress64 ptr,
                                             cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
    : apCLEnqueuedCommand(num_events_in_wait_list, event_wait_list, event), _buffer(buffer), _blocking_read(blocking_read), _offset(offset), _cb(cb), _ptr(ptr)
{

}

// ---------------------------------------------------------------------------
// Name:        apCLReadBufferCommand::~apCLReadBufferCommand
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLReadBufferCommand::~apCLReadBufferCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLReadBufferCommand::commandNameAsString
// Description: Gets the command name as a string
// Author:  AMD Developer Tools Team
// Date:        4/12/2009
// ---------------------------------------------------------------------------
void apCLReadBufferCommand::commandNameAsString(gtString& commandName) const
{
    commandName = L"ReadBuffer";
}

// ---------------------------------------------------------------------------
// Name:        apCLReadBufferCommand::relatedSrcMemHandle
// Description: Returns the related src mem (buffer / texture) handles.
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
void apCLReadBufferCommand::getRelatedSrcMemHandles(gtVector<oaCLMemHandle>& srcMemHandles) const
{
    srcMemHandles.clear();
    srcMemHandles.push_back(_buffer);
}

// ---------------------------------------------------------------------------
// Name:        apCLReadBufferCommand::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLReadBufferCommand::type() const
{
    return OS_TOBJ_ID_CL_READ_BUFFER_COMMAND;
}

// ---------------------------------------------------------------------------
// Name:        apCLReadBufferCommand::writeSelfIntoChannel
// Description: Writes the command into an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLReadBufferCommand::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apCLEnqueuedCommand::writeSelfIntoChannel(ipcChannel);

    ipcChannel << (gtUInt64)_buffer;
    ipcChannel << (gtUInt32)_blocking_read;
    ipcChannel << (gtUInt64)_offset;
    ipcChannel << (gtUInt64)_cb;
    ipcChannel << (gtUInt64)_ptr;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLReadBufferCommand::readSelfFromChannel
// Description: Reads the command from an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLReadBufferCommand::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apCLEnqueuedCommand::readSelfFromChannel(ipcChannel);

    gtUInt64 bufferAsUInt64 = 0;
    ipcChannel >> bufferAsUInt64;
    _buffer = (oaCLMemHandle)bufferAsUInt64;
    gtUInt32 blocking_readAsUInt32 = 0;
    ipcChannel >> blocking_readAsUInt32;
    _blocking_read = (cl_bool)blocking_readAsUInt32;
    gtUInt64 offsetAsUInt64 = 0;
    ipcChannel >> offsetAsUInt64;
    _offset = (gtSize_t)offsetAsUInt64;
    gtUInt64 cbAsUInt64 = 0;
    ipcChannel >> cbAsUInt64;
    _cb = (gtSize_t)cbAsUInt64;
    gtUInt64 ptrAsUInt64 = 0;
    ipcChannel >> ptrAsUInt64;
    _ptr = (osProcedureAddress64)ptrAsUInt64;

    return retVal;
}

// ----------------------------------------- apCLReadBufferRectCommand -----------------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLReadBufferRectCommand::apCLReadBufferRectCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        26/12/2010
// ---------------------------------------------------------------------------
apCLReadBufferRectCommand::apCLReadBufferRectCommand()
    : apCLEnqueuedCommand(), _buffer(OA_CL_NULL_HANDLE), _blocking_read(0), _buffer_row_pitch(0),
      _buffer_slice_pitch(0), _host_row_pitch(0), _host_slice_pitch(0), _ptr(OS_NULL_PROCEDURE_ADDRESS_64)
{
    _buffer_origin[0] = 0;
    _buffer_origin[1] = 0;
    _buffer_origin[2] = 0;
    _host_origin[0] = 0;
    _host_origin[1] = 0;
    _host_origin[2] = 0;
    _region[0] = 0;
    _region[1] = 0;
    _region[2] = 0;
}

// ---------------------------------------------------------------------------
// Name:        apCLReadBufferRectCommand::apCLReadBufferRectCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        26/12/2010
// ---------------------------------------------------------------------------
apCLReadBufferRectCommand::apCLReadBufferRectCommand(oaCLMemHandle buffer, cl_bool blocking_read,
                                                     const gtSize_t buffer_origin[3], const gtSize_t host_origin[3], const gtSize_t region[3],
                                                     gtSize_t buffer_row_pitch, gtSize_t buffer_slice_pitch, gtSize_t host_row_pitch, gtSize_t host_slice_pitch,
                                                     osProcedureAddress64 ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
    : apCLEnqueuedCommand(num_events_in_wait_list, event_wait_list, event), _buffer(buffer), _blocking_read(blocking_read), _buffer_row_pitch(buffer_row_pitch),
      _buffer_slice_pitch(buffer_slice_pitch), _host_row_pitch(host_row_pitch), _host_slice_pitch(host_slice_pitch), _ptr(ptr)
{
    _buffer_origin[0] = buffer_origin[0];
    _buffer_origin[1] = buffer_origin[1];
    _buffer_origin[2] = buffer_origin[2];
    _host_origin[0] = host_origin[0];
    _host_origin[1] = host_origin[1];
    _host_origin[2] = host_origin[2];
    _region[0] = region[0];
    _region[1] = region[1];
    _region[2] = region[2];
}

// ---------------------------------------------------------------------------
// Name:        apCLReadBufferRectCommand::~apCLReadBufferRectCommand
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        26/12/2010
// ---------------------------------------------------------------------------
apCLReadBufferRectCommand::~apCLReadBufferRectCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLReadBufferRectCommand::commandNameAsString
// Description: Gets the command name as a string
// Author:  AMD Developer Tools Team
// Date:        26/12/2010
// ---------------------------------------------------------------------------
void apCLReadBufferRectCommand::commandNameAsString(gtString& commandName) const
{
    commandName = L"ReadBufferRect";
}

// ---------------------------------------------------------------------------
// Name:        apCLReadBufferRectCommand::relatedSrcMemHandle
// Description: Returns the related src mem (buffer / texture) handles.
// Author:  AMD Developer Tools Team
// Date:        26/12/2010
// ---------------------------------------------------------------------------
void apCLReadBufferRectCommand::getRelatedSrcMemHandles(gtVector<oaCLMemHandle>& srcMemHandles) const
{
    srcMemHandles.clear();
    srcMemHandles.push_back(_buffer);
}

// ---------------------------------------------------------------------------
// Name:        apCLReadBufferRectCommand::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        26/12/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apCLReadBufferRectCommand::type() const
{
    return OS_TOBJ_ID_CL_READ_BUFFER_RECT_COMMAND;
}

// ---------------------------------------------------------------------------
// Name:        apCLReadBufferRectCommand::writeSelfIntoChannel
// Description: Writes the command into an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        26/12/2010
// ---------------------------------------------------------------------------
bool apCLReadBufferRectCommand::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apCLEnqueuedCommand::writeSelfIntoChannel(ipcChannel);

    ipcChannel << (gtUInt64)_buffer;
    ipcChannel << (gtUInt32)_blocking_read;
    ipcChannel << (gtUInt64)_buffer_origin[0];
    ipcChannel << (gtUInt64)_buffer_origin[1];
    ipcChannel << (gtUInt64)_buffer_origin[2];
    ipcChannel << (gtUInt64)_host_origin[0];
    ipcChannel << (gtUInt64)_host_origin[1];
    ipcChannel << (gtUInt64)_host_origin[2];
    ipcChannel << (gtUInt64)_region[0];
    ipcChannel << (gtUInt64)_region[1];
    ipcChannel << (gtUInt64)_region[2];
    ipcChannel << (gtUInt64)_buffer_row_pitch;
    ipcChannel << (gtUInt64)_buffer_slice_pitch;
    ipcChannel << (gtUInt64)_host_row_pitch;
    ipcChannel << (gtUInt64)_host_slice_pitch;
    ipcChannel << (gtUInt64)_ptr;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLReadBufferRectCommand::readSelfFromChannel
// Description: Reads the command from an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        26/12/2010
// ---------------------------------------------------------------------------
bool apCLReadBufferRectCommand::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apCLEnqueuedCommand::readSelfFromChannel(ipcChannel);

    gtUInt64 bufferAsUInt64 = 0;
    ipcChannel >> bufferAsUInt64;
    _buffer = (oaCLMemHandle)bufferAsUInt64;
    gtUInt32 blocking_readAsUInt32 = 0;
    ipcChannel >> blocking_readAsUInt32;
    _blocking_read = (cl_bool)blocking_readAsUInt32;
    gtUInt64 buffer_origin0AsUInt64 = 0;
    ipcChannel >> buffer_origin0AsUInt64;
    _buffer_origin[0] = (gtSize_t)buffer_origin0AsUInt64;
    gtUInt64 buffer_origin1AsUInt64 = 0;
    ipcChannel >> buffer_origin1AsUInt64;
    _buffer_origin[1] = (gtSize_t)buffer_origin1AsUInt64;
    gtUInt64 buffer_origin2AsUInt64 = 0;
    ipcChannel >> buffer_origin2AsUInt64;
    _buffer_origin[2] = (gtSize_t)buffer_origin2AsUInt64;
    gtUInt64 host_origin0AsUInt64 = 0;
    ipcChannel >> host_origin0AsUInt64;
    _host_origin[0] = (gtSize_t)host_origin0AsUInt64;
    gtUInt64 host_origin1AsUInt64 = 0;
    ipcChannel >> host_origin1AsUInt64;
    _host_origin[1] = (gtSize_t)host_origin1AsUInt64;
    gtUInt64 host_origin2AsUInt64 = 0;
    ipcChannel >> host_origin2AsUInt64;
    _host_origin[2] = (gtSize_t)host_origin2AsUInt64;
    gtUInt64 region0AsUInt64 = 0;
    ipcChannel >> region0AsUInt64;
    _region[0] = (gtSize_t)region0AsUInt64;
    gtUInt64 region1AsUInt64 = 0;
    ipcChannel >> region1AsUInt64;
    _region[1] = (gtSize_t)region1AsUInt64;
    gtUInt64 region2AsUInt64 = 0;
    ipcChannel >> region2AsUInt64;
    _region[2] = (gtSize_t)region2AsUInt64;
    gtUInt64 buffer_row_pitchAsUInt64 = 0;
    ipcChannel >> buffer_row_pitchAsUInt64;
    _buffer_row_pitch = (gtSize_t)buffer_row_pitchAsUInt64;
    gtUInt64 buffer_slice_pitchAsUInt64 = 0;
    ipcChannel >> buffer_slice_pitchAsUInt64;
    _buffer_slice_pitch = (gtSize_t)buffer_slice_pitchAsUInt64;
    gtUInt64 host_row_pitchAsUInt64 = 0;
    ipcChannel >> host_row_pitchAsUInt64;
    _host_row_pitch = (gtSize_t)host_row_pitchAsUInt64;
    gtUInt64 host_slice_pitchAsUInt64 = 0;
    ipcChannel >> host_slice_pitchAsUInt64;
    _host_slice_pitch = (gtSize_t)host_slice_pitchAsUInt64;
    gtUInt64 ptrAsUInt64 = 0;
    ipcChannel >> ptrAsUInt64;
    _ptr = (osProcedureAddress64)ptrAsUInt64;

    return retVal;
}

// ------------------------------ apCLReadImageCommand ------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLReadImageCommand::apCLReadImageCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLReadImageCommand::apCLReadImageCommand()
    : apCLEnqueuedCommand(), _image(OA_CL_NULL_HANDLE), _blocking_read(0), _row_pitch(0), _slice_pitch(0),
      _ptr(OS_NULL_PROCEDURE_ADDRESS_64)
{
    _origin[0] = 0;
    _origin[1] = 0;
    _origin[2] = 0;
    _region[0] = 0;
    _region[1] = 0;
    _region[2] = 0;
}

// ---------------------------------------------------------------------------
// Name:        apCLReadImageCommand::apCLReadImageCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLReadImageCommand::apCLReadImageCommand(oaCLMemHandle image, cl_bool blocking_read, const gtSize_t origin[3], const gtSize_t region[3],
                                           gtSize_t row_pitch, gtSize_t slice_pitch, osProcedureAddress64 ptr, cl_uint num_events_in_wait_list,
                                           const cl_event* event_wait_list, cl_event* event)
    : apCLEnqueuedCommand(num_events_in_wait_list, event_wait_list, event), _image(image), _blocking_read(blocking_read), _row_pitch(row_pitch), _slice_pitch(slice_pitch), _ptr(ptr)
{
    _origin[0] = origin[0];
    _origin[1] = origin[1];
    _origin[2] = origin[2];
    _region[0] = region[0];
    _region[1] = region[1];
    _region[2] = region[2];
}

// ---------------------------------------------------------------------------
// Name:        apCLReadImageCommand::~apCLReadImageCommand
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLReadImageCommand::~apCLReadImageCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLReadImageCommand::commandNameAsString
// Description: Gets the command name as a string
// Author:  AMD Developer Tools Team
// Date:        4/12/2009
// ---------------------------------------------------------------------------
void apCLReadImageCommand::commandNameAsString(gtString& commandName) const
{
    commandName = L"ReadImage";
}

// ---------------------------------------------------------------------------
// Name:        apCLReadImageCommand::relatedSrcMemHandle
// Description: Returns the related src mem (buffer / texture) handles.
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
void apCLReadImageCommand::getRelatedSrcMemHandles(gtVector<oaCLMemHandle>& srcMemHandles) const
{
    srcMemHandles.clear();
    srcMemHandles.push_back(_image);
}

// ---------------------------------------------------------------------------
// Name:        apCLReadImageCommand::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLReadImageCommand::type() const
{
    return OS_TOBJ_ID_CL_READ_IMAGE_COMMAND;
}

// ---------------------------------------------------------------------------
// Name:        apCLReadImageCommand::writeSelfIntoChannel
// Description: Writes the command into an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLReadImageCommand::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apCLEnqueuedCommand::writeSelfIntoChannel(ipcChannel);

    ipcChannel << (gtUInt64)_image;
    ipcChannel << (gtUInt32)_blocking_read;
    ipcChannel << (gtUInt64)_origin[0];
    ipcChannel << (gtUInt64)_origin[1];
    ipcChannel << (gtUInt64)_origin[2];
    ipcChannel << (gtUInt64)_region[0];
    ipcChannel << (gtUInt64)_region[1];
    ipcChannel << (gtUInt64)_region[2];
    ipcChannel << (gtUInt64)_row_pitch;
    ipcChannel << (gtUInt64)_slice_pitch;
    ipcChannel << (gtUInt64)_ptr;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLReadImageCommand::readSelfFromChannel
// Description: Reads the command from an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLReadImageCommand::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apCLEnqueuedCommand::readSelfFromChannel(ipcChannel);

    gtUInt64 imageAsUInt64 = 0;
    ipcChannel >> imageAsUInt64;
    _image = (oaCLMemHandle)imageAsUInt64;
    gtUInt32 blocking_readAsUInt32 = 0;
    ipcChannel >> blocking_readAsUInt32;
    _blocking_read = (cl_bool)blocking_readAsUInt32;
    gtUInt64 origin0AsUInt64 = 0;
    ipcChannel >> origin0AsUInt64;
    _origin[0] = (gtSize_t)origin0AsUInt64;
    gtUInt64 origin1AsUInt64 = 0;
    ipcChannel >> origin1AsUInt64;
    _origin[1] = (gtSize_t)origin1AsUInt64;
    gtUInt64 origin2AsUInt64 = 0;
    ipcChannel >> origin2AsUInt64;
    _origin[2] = (gtSize_t)origin2AsUInt64;
    gtUInt64 region0AsUInt64 = 0;
    ipcChannel >> region0AsUInt64;
    _region[0] = (gtSize_t)region0AsUInt64;
    gtUInt64 region1AsUInt64 = 0;
    ipcChannel >> region1AsUInt64;
    _region[1] = (gtSize_t)region1AsUInt64;
    gtUInt64 region2AsUInt64 = 0;
    ipcChannel >> region2AsUInt64;
    _region[2] = (gtSize_t)region2AsUInt64;
    gtUInt64 row_pitchAsUInt64 = 0;
    ipcChannel >> row_pitchAsUInt64;
    _row_pitch = (gtSize_t)row_pitchAsUInt64;
    gtUInt64 slice_pitchAsUInt64 = 0;
    ipcChannel >> slice_pitchAsUInt64;
    _slice_pitch = (gtSize_t)slice_pitchAsUInt64;
    gtUInt64 ptrAsUInt64 = 0;
    ipcChannel >> ptrAsUInt64;
    _ptr = (osProcedureAddress64)ptrAsUInt64;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apCLReadImageCommand::amountOfPixels
// Description: Amount of pixels requested for read in this command
// Return Val:  gtSize_t
// Author:  AMD Developer Tools Team
// Date:        6/5/2010
// ---------------------------------------------------------------------------
gtSize_t apCLReadImageCommand::amountOfPixels()
{
    gtSize_t retVal = 1;

    // Calculate the amount of pixels according to the
    if (_region[0] != 0)
    {
        retVal *= _region[0];
    }

    if (_region[1] != 0)
    {
        retVal *= _region[1];
    }

    if (_region[2] != 0)
    {
        retVal *= _region[2];
    }

    return retVal;
}

// -------------------------- apCLReleaseGLObjectsCommand ---------------------------

// ---------------------------------------------------------------------------
// Name:        apCLReleaseGLObjectsCommand::apCLReleaseGLObjectsCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLReleaseGLObjectsCommand::apCLReleaseGLObjectsCommand()
    : apCLEnqueuedCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLReleaseGLObjectsCommand::apCLReleaseGLObjectsCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLReleaseGLObjectsCommand::apCLReleaseGLObjectsCommand(cl_uint num_objects, const cl_mem* mem_objects, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
    : apCLEnqueuedCommand(num_events_in_wait_list, event_wait_list, event)
{
    GT_IF_WITH_ASSERT((num_objects > 0) && (mem_objects != NULL))
    {
        for (cl_uint i = 0; i < num_objects; i++)
        {
            _mem_objects.push_back((oaCLKernelHandle)mem_objects[i]);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        apCLReleaseGLObjectsCommand::~apCLReleaseGLObjectsCommand
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLReleaseGLObjectsCommand::~apCLReleaseGLObjectsCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLReleaseGLObjectsCommand::commandNameAsString
// Description: Gets the command name as a string
// Author:  AMD Developer Tools Team
// Date:        4/12/2009
// ---------------------------------------------------------------------------
void apCLReleaseGLObjectsCommand::commandNameAsString(gtString& commandName) const
{
    commandName = L"ReleaseGLObjects";
}

// ---------------------------------------------------------------------------
// Name:        apCLReleaseGLObjectsCommand::relatedSrcMemHandle
// Description: Returns the related src mem (buffer / texture) handles.
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
void apCLReleaseGLObjectsCommand::getRelatedSrcMemHandles(gtVector<oaCLMemHandle>& srcMemHandles) const
{
    srcMemHandles.clear();

    // Copy the mem objects into the output vector:
    int numberOfMemObjects = (int)_mem_objects.size();

    for (int i = 0; i < numberOfMemObjects; i++)
    {
        srcMemHandles.push_back(_mem_objects[i]);
    }
}

// ---------------------------------------------------------------------------
// Name:        apCLReleaseGLObjectsCommand::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLReleaseGLObjectsCommand::type() const
{
    return OS_TOBJ_ID_CL_RELEASE_GL_OBJECTS_COMMAND;
}

// ---------------------------------------------------------------------------
// Name:        apCLReleaseGLObjectsCommand::writeSelfIntoChannel
// Description: Writes the command into an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLReleaseGLObjectsCommand::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apCLEnqueuedCommand::writeSelfIntoChannel(ipcChannel);

    gtUInt32 num_objects = (gtUInt32)_mem_objects.size();
    ipcChannel << num_objects;

    for (gtUInt32 i = 0; i < num_objects; i++)
    {
        ipcChannel << (gtUInt64)_mem_objects[i];
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLReleaseGLObjectsCommand::readSelfFromChannel
// Description: Reads the command from an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLReleaseGLObjectsCommand::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apCLEnqueuedCommand::readSelfFromChannel(ipcChannel);

    gtUInt32 num_objects = 0;
    ipcChannel >> num_objects;

    for (gtUInt32 i = 0; i < num_objects; i++)
    {
        gtUInt64 mem_objectsAsUInt64 = 0;
        ipcChannel >> mem_objectsAsUInt64;
        _mem_objects.push_back((oaCLMemHandle)mem_objectsAsUInt64);
    }

    return retVal;
}

// -------------------------------- apCLTaskCommand ---------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLTaskCommand::apCLTaskCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLTaskCommand::apCLTaskCommand()
    : apCLEnqueuedCommand(), _kernel(OA_CL_NULL_HANDLE)
{

}

// ---------------------------------------------------------------------------
// Name:        apCLTaskCommand::apCLTaskCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLTaskCommand::apCLTaskCommand(oaCLKernelHandle kernel, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
    : apCLEnqueuedCommand(num_events_in_wait_list, event_wait_list, event), _kernel(kernel)
{

}

// ---------------------------------------------------------------------------
// Name:        apCLTaskCommand::~apCLTaskCommand
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLTaskCommand::~apCLTaskCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLTaskCommand::commandNameAsString
// Description: Gets the command name as a string
// Author:  AMD Developer Tools Team
// Date:        4/12/2009
// ---------------------------------------------------------------------------
void apCLTaskCommand::commandNameAsString(gtString& commandName) const
{
    commandName = L"Task";
}

// ---------------------------------------------------------------------------
// Name:        apCLTaskCommand::relatedKernelHandle
// Description: Returns the related kernel handles.
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
void apCLTaskCommand::getRelatedKernelHandles(gtVector<oaCLKernelHandle>& kernelHandles) const
{
    kernelHandles.clear();
    kernelHandles.push_back(_kernel);
}

// ---------------------------------------------------------------------------
// Name:        apCLTaskCommand::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLTaskCommand::type() const
{
    return OS_TOBJ_ID_CL_TASK_COMMAND;
}

// ---------------------------------------------------------------------------
// Name:        apCLTaskCommand::writeSelfIntoChannel
// Description: Writes the command into an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLTaskCommand::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apCLEnqueuedCommand::writeSelfIntoChannel(ipcChannel);

    ipcChannel << (gtUInt64)_kernel;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLTaskCommand::readSelfFromChannel
// Description: Reads the command from an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLTaskCommand::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apCLEnqueuedCommand::readSelfFromChannel(ipcChannel);

    gtUInt64 kernelAsUInt64 = 0;
    ipcChannel >> kernelAsUInt64;
    _kernel = (oaCLKernelHandle)kernelAsUInt64;

    return retVal;
}

// ---------------------------- apCLUnmapMemObjectCommand ---------------------------

// ---------------------------------------------------------------------------
// Name:        apCLUnmapMemObjectCommand::apCLUnmapMemObjectCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLUnmapMemObjectCommand::apCLUnmapMemObjectCommand()
    : apCLEnqueuedCommand(), _memobj(OA_CL_NULL_HANDLE), _mapped_ptr(OS_NULL_PROCEDURE_ADDRESS_64)
{

}

// ---------------------------------------------------------------------------
// Name:        apCLUnmapMemObjectCommand::apCLUnmapMemObjectCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLUnmapMemObjectCommand::apCLUnmapMemObjectCommand(oaCLMemHandle memobj, osProcedureAddress64 mapped_ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
    : apCLEnqueuedCommand(num_events_in_wait_list, event_wait_list, event), _memobj(memobj), _mapped_ptr(mapped_ptr)
{

}

// ---------------------------------------------------------------------------
// Name:        apCLUnmapMemObjectCommand::~apCLUnmapMemObjectCommand
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLUnmapMemObjectCommand::~apCLUnmapMemObjectCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLUnmapMemObjectCommand::commandNameAsString
// Description: Gets the command name as a string
// Author:  AMD Developer Tools Team
// Date:        4/12/2009
// ---------------------------------------------------------------------------
void apCLUnmapMemObjectCommand::commandNameAsString(gtString& commandName) const
{
    commandName = L"UnmapMemObject";
}

// ---------------------------------------------------------------------------
// Name:        apCLUnmapMemObjectCommand::relatedSrcMemHandle
// Description: Returns the related src mem (buffer / texture) handles.
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
void apCLUnmapMemObjectCommand::getRelatedSrcMemHandles(gtVector<oaCLMemHandle>& srcMemHandles) const
{
    srcMemHandles.clear();
    srcMemHandles.push_back(_memobj);
}

// ---------------------------------------------------------------------------
// Name:        apCLUnmapMemObjectCommand::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLUnmapMemObjectCommand::type() const
{
    return OS_TOBJ_ID_CL_UNMAP_MEM_OBJECT_COMMAND;
}

// ---------------------------------------------------------------------------
// Name:        apCLUnmapMemObjectCommand::writeSelfIntoChannel
// Description: Writes the command into an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLUnmapMemObjectCommand::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apCLEnqueuedCommand::writeSelfIntoChannel(ipcChannel);

    ipcChannel << (gtUInt64)_memobj;
    ipcChannel << (gtUInt64)_mapped_ptr;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLUnmapMemObjectCommand::readSelfFromChannel
// Description: Reads the command from an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLUnmapMemObjectCommand::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apCLEnqueuedCommand::readSelfFromChannel(ipcChannel);

    gtUInt64 memobjAsUInt64 = 0;
    ipcChannel >> memobjAsUInt64;
    _memobj = (oaCLMemHandle)memobjAsUInt64;
    gtUInt64 mapped_ptrAsUInt64 = 0;
    ipcChannel >> mapped_ptrAsUInt64;
    _mapped_ptr = (osProcedureAddress64)mapped_ptrAsUInt64;

    return retVal;
}

// ---------------------------- apCLWaitForEventsCommand ----------------------------

// ---------------------------------------------------------------------------
// Name:        apCLWaitForEventsCommand::apCLWaitForEventsCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLWaitForEventsCommand::apCLWaitForEventsCommand()
    : apCLEnqueuedCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLWaitForEventsCommand::apCLWaitForEventsCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLWaitForEventsCommand::apCLWaitForEventsCommand(cl_uint num_events, const cl_event* event_list)
    : apCLEnqueuedCommand(num_events, event_list, NULL)
{

}

// ---------------------------------------------------------------------------
// Name:        apCLWaitForEventsCommand::~apCLWaitForEventsCommand
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLWaitForEventsCommand::~apCLWaitForEventsCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLWaitForEventsCommand::commandNameAsString
// Description: Gets the command name as a string
// Author:  AMD Developer Tools Team
// Date:        4/12/2009
// ---------------------------------------------------------------------------
void apCLWaitForEventsCommand::commandNameAsString(gtString& commandName) const
{
    commandName = L"WaitForEvents";
}

// ---------------------------------------------------------------------------
// Name:        apCLWaitForEventsCommand::eventParameterAvailable
// Description: Returns true iff the specific class has an "Event" parameter.
// Author:  AMD Developer Tools Team
// Date:        4/3/2010
// ---------------------------------------------------------------------------
bool apCLWaitForEventsCommand::eventParameterAvailable() const
{
    return false;
}

// ---------------------------------------------------------------------------
// Name:        apCLWaitForEventsCommand::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLWaitForEventsCommand::type() const
{
    return OS_TOBJ_ID_CL_WAIT_FOR_EVENTS_COMMAND;
}

// ---------------------------------------------------------------------------
// Name:        apCLWaitForEventsCommand::writeSelfIntoChannel
// Description: Writes the command into an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLWaitForEventsCommand::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apCLEnqueuedCommand::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLWaitForEventsCommand::readSelfFromChannel
// Description: Reads the command from an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLWaitForEventsCommand::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apCLEnqueuedCommand::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ----------------------------- apCLWriteBufferCommand -----------------------------

// ---------------------------------------------------------------------------
// Name:        apCLWriteBufferCommand::apCLWriteBufferCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLWriteBufferCommand::apCLWriteBufferCommand()
    : apCLEnqueuedCommand(), _buffer(OA_CL_NULL_HANDLE), _blocking_write(0), _offset(0), _cb(0), _ptr(OS_NULL_PROCEDURE_ADDRESS_64)
{

}

// ---------------------------------------------------------------------------
// Name:        apCLWriteBufferCommand::apCLWriteBufferCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLWriteBufferCommand::apCLWriteBufferCommand(oaCLMemHandle buffer, cl_bool blocking_write, gtSize_t offset, gtSize_t cb, osProcedureAddress64 ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
    : apCLEnqueuedCommand(num_events_in_wait_list, event_wait_list, event), _buffer(buffer), _blocking_write(blocking_write), _offset(offset), _cb(cb), _ptr(ptr)
{

}

// ---------------------------------------------------------------------------
// Name:        apCLWriteBufferCommand::~apCLWriteBufferCommand
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLWriteBufferCommand::~apCLWriteBufferCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLWriteBufferCommand::commandNameAsString
// Description: Gets the command name as a string
// Author:  AMD Developer Tools Team
// Date:        4/12/2009
// ---------------------------------------------------------------------------
void apCLWriteBufferCommand::commandNameAsString(gtString& commandName) const
{
    commandName = L"WriteBuffer";
}

// ---------------------------------------------------------------------------
// Name:        apCLWriteBufferCommand::relatedDstMemHandle
// Description: Returns the related dst mem (buffer / texture) handles.
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
void apCLWriteBufferCommand::getRelatedDstMemHandles(gtVector<oaCLMemHandle>& dstMemHandles) const
{
    dstMemHandles.clear();
    dstMemHandles.push_back(_buffer);
}

// ---------------------------------------------------------------------------
// Name:        apCLWriteBufferCommand::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLWriteBufferCommand::type() const
{
    return OS_TOBJ_ID_CL_WRITE_BUFFER_COMMAND;
}

// ---------------------------------------------------------------------------
// Name:        apCLWriteBufferCommand::writeSelfIntoChannel
// Description: Writes the command into an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLWriteBufferCommand::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apCLEnqueuedCommand::writeSelfIntoChannel(ipcChannel);

    ipcChannel << (gtUInt64)_buffer;
    ipcChannel << (gtUInt32)_blocking_write;
    ipcChannel << (gtUInt64)_offset;
    ipcChannel << (gtUInt64)_cb;
    ipcChannel << (gtUInt64)_ptr;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLWriteBufferCommand::readSelfFromChannel
// Description: Reads the command from an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLWriteBufferCommand::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apCLEnqueuedCommand::readSelfFromChannel(ipcChannel);

    gtUInt64 bufferAsUInt64 = 0;
    ipcChannel >> bufferAsUInt64;
    _buffer = (oaCLMemHandle)bufferAsUInt64;
    gtUInt32 blocking_writeAsUInt32 = 0;
    ipcChannel >> blocking_writeAsUInt32;
    _blocking_write = (cl_bool)blocking_writeAsUInt32;
    gtUInt64 offsetAsUInt64 = 0;
    ipcChannel >> offsetAsUInt64;
    _offset = (gtSize_t)offsetAsUInt64;
    gtUInt64 cbAsUInt64 = 0;
    ipcChannel >> cbAsUInt64;
    _cb = (gtSize_t)cbAsUInt64;
    gtUInt64 ptrAsUInt64 = 0;
    ipcChannel >> ptrAsUInt64;
    _ptr = (osProcedureAddress64)ptrAsUInt64;

    return retVal;
}

// ----------------------------- apCLWriteBufferRectCommand -----------------------------

// ---------------------------------------------------------------------------
// Name:        apCLWriteBufferRectCommand::apCLWriteBufferRectCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        26/12/2010
// ---------------------------------------------------------------------------
apCLWriteBufferRectCommand::apCLWriteBufferRectCommand()
    : apCLEnqueuedCommand(), _buffer(OA_CL_NULL_HANDLE), _blocking_write(0), _buffer_row_pitch(0),
      _buffer_slice_pitch(0), _host_row_pitch(0), _host_slice_pitch(0), _ptr(OS_NULL_PROCEDURE_ADDRESS_64)
{
    _buffer_origin[0] = 0;
    _buffer_origin[1] = 0;
    _buffer_origin[2] = 0;
    _host_origin[0] = 0;
    _host_origin[1] = 0;
    _host_origin[2] = 0;
    _region[0] = 0;
    _region[1] = 0;
    _region[2] = 0;
}

// ---------------------------------------------------------------------------
// Name:        apCLWriteBufferRectCommand::apCLWriteBufferRectCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        26/12/2010
// ---------------------------------------------------------------------------
apCLWriteBufferRectCommand::apCLWriteBufferRectCommand(oaCLMemHandle buffer, cl_bool blocking_write,
                                                       const gtSize_t buffer_origin[3], const gtSize_t host_origin[3], const gtSize_t region[3],
                                                       gtSize_t buffer_row_pitch, gtSize_t buffer_slice_pitch, gtSize_t host_row_pitch, gtSize_t host_slice_pitch,
                                                       osProcedureAddress64 ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
    : apCLEnqueuedCommand(num_events_in_wait_list, event_wait_list, event), _buffer(buffer), _blocking_write(blocking_write), _buffer_row_pitch(buffer_row_pitch),
      _buffer_slice_pitch(buffer_slice_pitch), _host_row_pitch(host_row_pitch), _host_slice_pitch(host_slice_pitch), _ptr(ptr)
{
    _buffer_origin[0] = buffer_origin[0];
    _buffer_origin[1] = buffer_origin[1];
    _buffer_origin[2] = buffer_origin[2];
    _host_origin[0] = host_origin[0];
    _host_origin[1] = host_origin[1];
    _host_origin[2] = host_origin[2];
    _region[0] = region[0];
    _region[1] = region[1];
    _region[2] = region[2];

}

// ---------------------------------------------------------------------------
// Name:        apCLWriteBufferRectCommand::~apCLWriteBufferRectCommand
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        26/12/2010
// ---------------------------------------------------------------------------
apCLWriteBufferRectCommand::~apCLWriteBufferRectCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLWriteBufferRectCommand::commandNameAsString
// Description: Gets the command name as a string
// Author:  AMD Developer Tools Team
// Date:        26/12/2010
// ---------------------------------------------------------------------------
void apCLWriteBufferRectCommand::commandNameAsString(gtString& commandName) const
{
    commandName = L"WriteBufferRect";
}

// ---------------------------------------------------------------------------
// Name:        apCLWriteBufferRectCommand::relatedDstMemHandle
// Description: Returns the related dst mem (buffer / texture) handles.
// Author:  AMD Developer Tools Team
// Date:        26/12/2010
// ---------------------------------------------------------------------------
void apCLWriteBufferRectCommand::getRelatedDstMemHandles(gtVector<oaCLMemHandle>& dstMemHandles) const
{
    dstMemHandles.clear();
    dstMemHandles.push_back(_buffer);
}

// ---------------------------------------------------------------------------
// Name:        apCLWriteBufferRectCommand::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        26/12/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apCLWriteBufferRectCommand::type() const
{
    return OS_TOBJ_ID_CL_WRITE_BUFFER_RECT_COMMAND;
}

// ---------------------------------------------------------------------------
// Name:        apCLWriteBufferRectCommand::writeSelfIntoChannel
// Description: Writes the command into an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        26/12/2010
// ---------------------------------------------------------------------------
bool apCLWriteBufferRectCommand::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apCLEnqueuedCommand::writeSelfIntoChannel(ipcChannel);

    ipcChannel << (gtUInt64)_buffer;
    ipcChannel << (gtUInt32)_blocking_write;
    ipcChannel << (gtUInt64)_buffer_origin[0];
    ipcChannel << (gtUInt64)_buffer_origin[1];
    ipcChannel << (gtUInt64)_buffer_origin[2];
    ipcChannel << (gtUInt64)_host_origin[0];
    ipcChannel << (gtUInt64)_host_origin[1];
    ipcChannel << (gtUInt64)_host_origin[2];
    ipcChannel << (gtUInt64)_region[0];
    ipcChannel << (gtUInt64)_region[1];
    ipcChannel << (gtUInt64)_region[2];
    ipcChannel << (gtUInt64)_buffer_row_pitch;
    ipcChannel << (gtUInt64)_buffer_slice_pitch;
    ipcChannel << (gtUInt64)_host_row_pitch;
    ipcChannel << (gtUInt64)_host_slice_pitch;
    ipcChannel << (gtUInt64)_ptr;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLWriteBufferRectCommand::readSelfFromChannel
// Description: Reads the command from an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        26/12/2010
// ---------------------------------------------------------------------------
bool apCLWriteBufferRectCommand::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apCLEnqueuedCommand::readSelfFromChannel(ipcChannel);

    gtUInt64 bufferAsUInt64 = 0;
    ipcChannel >> bufferAsUInt64;
    _buffer = (oaCLMemHandle)bufferAsUInt64;
    gtUInt32 blocking_writeAsUInt32 = 0;
    ipcChannel >> blocking_writeAsUInt32;
    _blocking_write = (cl_bool)blocking_writeAsUInt32;
    gtUInt64 buffer_origin0AsUInt64 = 0;
    ipcChannel >> buffer_origin0AsUInt64;
    _buffer_origin[0] = (gtSize_t)buffer_origin0AsUInt64;
    gtUInt64 buffer_origin1AsUInt64 = 0;
    ipcChannel >> buffer_origin1AsUInt64;
    _buffer_origin[1] = (gtSize_t)buffer_origin1AsUInt64;
    gtUInt64 buffer_origin2AsUInt64 = 0;
    ipcChannel >> buffer_origin2AsUInt64;
    _buffer_origin[2] = (gtSize_t)buffer_origin2AsUInt64;
    gtUInt64 host_origin0AsUInt64 = 0;
    ipcChannel >> host_origin0AsUInt64;
    _host_origin[0] = (gtSize_t)host_origin0AsUInt64;
    gtUInt64 host_origin1AsUInt64 = 0;
    ipcChannel >> host_origin1AsUInt64;
    _host_origin[1] = (gtSize_t)host_origin1AsUInt64;
    gtUInt64 host_origin2AsUInt64 = 0;
    ipcChannel >> host_origin2AsUInt64;
    _host_origin[2] = (gtSize_t)host_origin2AsUInt64;
    gtUInt64 region0AsUInt64 = 0;
    ipcChannel >> region0AsUInt64;
    _region[0] = (gtSize_t)region0AsUInt64;
    gtUInt64 region1AsUInt64 = 0;
    ipcChannel >> region1AsUInt64;
    _region[1] = (gtSize_t)region1AsUInt64;
    gtUInt64 region2AsUInt64 = 0;
    ipcChannel >> region2AsUInt64;
    _region[2] = (gtSize_t)region2AsUInt64;
    gtUInt64 buffer_row_pitchAsUInt64 = 0;
    ipcChannel >> buffer_row_pitchAsUInt64;
    _buffer_row_pitch = (gtSize_t)buffer_row_pitchAsUInt64;
    gtUInt64 buffer_slice_pitchAsUInt64 = 0;
    ipcChannel >> buffer_slice_pitchAsUInt64;
    _buffer_slice_pitch = (gtSize_t)buffer_slice_pitchAsUInt64;
    gtUInt64 host_row_pitchAsUInt64 = 0;
    ipcChannel >> host_row_pitchAsUInt64;
    _host_row_pitch = (gtSize_t)host_row_pitchAsUInt64;
    gtUInt64 host_slice_pitchAsUInt64 = 0;
    ipcChannel >> host_slice_pitchAsUInt64;
    _host_slice_pitch = (gtSize_t)host_slice_pitchAsUInt64;
    gtUInt64 ptrAsUInt64 = 0;
    ipcChannel >> ptrAsUInt64;
    _ptr = (osProcedureAddress64)ptrAsUInt64;

    return retVal;
}

// ------------------------------ apCLWriteImageCommand -----------------------------

// ---------------------------------------------------------------------------
// Name:        apCLWriteImageCommand::apCLWriteImageCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLWriteImageCommand::apCLWriteImageCommand()
    : apCLEnqueuedCommand(), _image(OA_CL_NULL_HANDLE), _blocking_write(0), _input_row_pitch(0), _input_slice_pitch(0), _ptr(OS_NULL_PROCEDURE_ADDRESS_64)
{
    _origin[0] = 0;
    _origin[1] = 0;
    _origin[2] = 0;
    _region[0] = 0;
    _region[1] = 0;
    _region[2] = 0;
}

// ---------------------------------------------------------------------------
// Name:        apCLWriteImageCommand::apCLWriteImageCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLWriteImageCommand::apCLWriteImageCommand(oaCLMemHandle image, cl_bool blocking_write, const gtSize_t origin[3], const gtSize_t region[3], gtSize_t input_row_pitch, gtSize_t input_slice_pitch, osProcedureAddress64 ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
    : apCLEnqueuedCommand(num_events_in_wait_list, event_wait_list, event), _image(image), _blocking_write(blocking_write), _input_row_pitch(input_row_pitch), _input_slice_pitch(input_slice_pitch), _ptr(ptr)
{
    _origin[0] = origin[0];
    _origin[1] = origin[1];
    _origin[2] = origin[2];
    _region[0] = region[0];
    _region[1] = region[1];
    _region[2] = region[2];
}

// ---------------------------------------------------------------------------
// Name:        apCLWriteImageCommand::~apCLWriteImageCommand
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLWriteImageCommand::~apCLWriteImageCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLWriteImageCommand::commandNameAsString
// Description: Gets the command name as a string
// Author:  AMD Developer Tools Team
// Date:        4/12/2009
// ---------------------------------------------------------------------------
void apCLWriteImageCommand::commandNameAsString(gtString& commandName) const
{
    commandName = L"WriteImage";
}

// ---------------------------------------------------------------------------
// Name:        apCLWriteImageCommand::relatedDstMemHandle
// Description: Returns the related dst mem (buffer / texture) handles.
// Author:  AMD Developer Tools Team
// Date:        2/3/2010
// ---------------------------------------------------------------------------
void apCLWriteImageCommand::getRelatedDstMemHandles(gtVector<oaCLMemHandle>& dstMemHandles) const
{
    dstMemHandles.clear();
    dstMemHandles.push_back(_image);
}

// ---------------------------------------------------------------------------
// Name:        apCLWriteImageCommand::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLWriteImageCommand::type() const
{
    return OS_TOBJ_ID_CL_WRITE_IMAGE_COMMAND;
}

// ---------------------------------------------------------------------------
// Name:        apCLWriteImageCommand::writeSelfIntoChannel
// Description: Writes the command into an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLWriteImageCommand::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apCLEnqueuedCommand::writeSelfIntoChannel(ipcChannel);

    ipcChannel << (gtUInt64)_image;
    ipcChannel << (gtUInt32)_blocking_write;
    ipcChannel << (gtUInt64)_origin[0];
    ipcChannel << (gtUInt64)_origin[1];
    ipcChannel << (gtUInt64)_origin[2];
    ipcChannel << (gtUInt64)_region[0];
    ipcChannel << (gtUInt64)_region[1];
    ipcChannel << (gtUInt64)_region[2];
    ipcChannel << (gtUInt64)_input_row_pitch;
    ipcChannel << (gtUInt64)_input_slice_pitch;
    ipcChannel << (gtUInt64)_ptr;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLWriteImageCommand::readSelfFromChannel
// Description: Reads the command from an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLWriteImageCommand::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apCLEnqueuedCommand::readSelfFromChannel(ipcChannel);

    gtUInt64 imageAsUInt64 = 0;
    ipcChannel >> imageAsUInt64;
    _image = (oaCLMemHandle)imageAsUInt64;
    gtUInt32 blocking_writeAsUInt32 = 0;
    ipcChannel >> blocking_writeAsUInt32;
    _blocking_write = (cl_bool)blocking_writeAsUInt32;
    gtUInt64 origin0AsUInt64 = 0;
    ipcChannel >> origin0AsUInt64;
    _origin[0] = (gtSize_t)origin0AsUInt64;
    gtUInt64 origin1AsUInt64 = 0;
    ipcChannel >> origin1AsUInt64;
    _origin[1] = (gtSize_t)origin1AsUInt64;
    gtUInt64 origin2AsUInt64 = 0;
    ipcChannel >> origin2AsUInt64;
    _origin[2] = (gtSize_t)origin2AsUInt64;
    gtUInt64 region0AsUInt64 = 0;
    ipcChannel >> region0AsUInt64;
    _region[0] = (gtSize_t)region0AsUInt64;
    gtUInt64 region1AsUInt64 = 0;
    ipcChannel >> region1AsUInt64;
    _region[1] = (gtSize_t)region1AsUInt64;
    gtUInt64 region2AsUInt64 = 0;
    ipcChannel >> region2AsUInt64;
    _region[2] = (gtSize_t)region2AsUInt64;
    gtUInt64 input_row_pitchAsUInt64 = 0;
    ipcChannel >> input_row_pitchAsUInt64;
    _input_row_pitch = (gtSize_t)input_row_pitchAsUInt64;
    gtUInt64 input_slice_pitchAsUInt64 = 0;
    ipcChannel >> input_slice_pitchAsUInt64;
    _input_slice_pitch = (gtSize_t)input_slice_pitchAsUInt64;
    gtUInt64 ptrAsUInt64 = 0;
    ipcChannel >> ptrAsUInt64;
    _ptr = (osProcedureAddress64)ptrAsUInt64;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLWriteImageCommand::amountOfPixels
// Description: Amount of pixels requested for write in this command
// Return Val:  gtSize_t
// Author:  AMD Developer Tools Team
// Date:        6/5/2010
// ---------------------------------------------------------------------------
gtSize_t apCLWriteImageCommand::amountOfPixels()
{
    gtSize_t retVal = 1;

    // Calculate the amount of pixels according to the
    if (_region[0] != 0)
    {
        retVal *= _region[0];
    }

    if (_region[1] != 0)
    {
        retVal *= _region[1];
    }

    if (_region[2] != 0)
    {
        retVal *= _region[2];
    }

    return retVal;
}

// ---------------------------------- apCLQueueIdle ---------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLQueueIdle::apCLQueueIdle
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLQueueIdle::apCLQueueIdle()
    : apCLEnqueuedCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLQueueIdle::~apCLQueueIdle
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
apCLQueueIdle::~apCLQueueIdle()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLQueueIdle::commandNameAsString
// Description: Gets the command name as a string
// Author:  AMD Developer Tools Team
// Date:        4/12/2009
// ---------------------------------------------------------------------------
void apCLQueueIdle::commandNameAsString(gtString& commandName) const
{
    commandName = L"Idle";
}

// ---------------------------------------------------------------------------
// Name:        apCLQueueIdle::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLQueueIdle::type() const
{
    return OS_TOBJ_ID_CL_QUEUE_IDLE_TIME;
}

// ---------------------------------------------------------------------------
// Name:        apCLQueueIdle::writeSelfIntoChannel
// Description: Writes the idle item into an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLQueueIdle::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apCLEnqueuedCommand::writeSelfIntoChannel(ipcChannel);

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        apCLQueueIdle::readSelfFromChannel
// Description: Reads the idle item from an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool apCLQueueIdle::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apCLEnqueuedCommand::readSelfFromChannel(ipcChannel);

    return retVal;
}


//////////////////////////////////////////////////////////////////////////
// OpenCL 1.2 Commands                                                  //
//////////////////////////////////////////////////////////////////////////

// ---------------------------------- apCLFillBufferCommand ---------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLFillBufferCommand::apCLFillBufferCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
apCLFillBufferCommand::apCLFillBufferCommand()
    : apCLEnqueuedCommand(), m_filledBuffer(OA_CL_NULL_HANDLE), m_filledPattern(NULL), m_filledPatternSize(0), m_filledOffset(0), m_filledSize(0)
{

}

// ---------------------------------------------------------------------------
// Name:        apCLFillBufferCommand::apCLFillBufferCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
apCLFillBufferCommand::apCLFillBufferCommand(oaCLMemHandle buffer, const void* pattern, gtSize_t pattern_size, gtSize_t offset, gtSize_t size,
                                             cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
    : apCLEnqueuedCommand(num_events_in_wait_list, event_wait_list, event), m_filledBuffer(buffer), m_filledPattern(NULL), m_filledPatternSize(pattern_size), m_filledOffset(offset), m_filledSize(size)
{
    // Copy the pattern:
    m_filledPattern = new gtUByte[m_filledPatternSize];


    for (gtSize_t i = 0; i < m_filledPatternSize; i++)
    {
        m_filledPattern[i] = ((gtUByte*)pattern)[i];
    }
}

// ---------------------------------------------------------------------------
// Name:        apCLFillBufferCommand::~apCLFillBufferCommand
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
apCLFillBufferCommand::~apCLFillBufferCommand()
{
    if (NULL != m_filledPattern)
    {
        delete[] m_filledPattern;
    }
}

// ---------------------------------------------------------------------------
// Name:        apCLFillBufferCommand::commandNameAsString
// Description: Gets the command name as a string
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
void apCLFillBufferCommand::commandNameAsString(gtString& commandName) const
{
    commandName = L"FillBuffer";
}

// ---------------------------------------------------------------------------
// Name:        apCLFillBufferCommand::relatedDstMemHandle
// Description: Returns the related dst mem (buffer / texture) handles.
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
void apCLFillBufferCommand::getRelatedDstMemHandles(gtVector<oaCLMemHandle>& dstMemHandles) const
{
    dstMemHandles.clear();
    dstMemHandles.push_back(m_filledBuffer);
}

// ---------------------------------------------------------------------------
// Name:        apCLFillBufferCommand::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
osTransferableObjectType apCLFillBufferCommand::type() const
{
    return OS_TOBJ_ID_CL_FILL_BUFFER_COMMAND;
}

// ---------------------------------------------------------------------------
// Name:        apCLFillBufferCommand::writeSelfIntoChannel
// Description: Writes the command into an ipc channel
// Return Val:  bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
bool apCLFillBufferCommand::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apCLEnqueuedCommand::writeSelfIntoChannel(ipcChannel);

    ipcChannel << (gtUInt64)m_filledBuffer;
    ipcChannel << (gtUInt64)m_filledPatternSize;

    for (gtSize_t i = 0; i < m_filledPatternSize; i++)
    {
        ipcChannel << m_filledPattern[i];
    }

    ipcChannel << (gtUInt64)m_filledOffset;
    ipcChannel << (gtUInt64)m_filledSize;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLFillBufferCommand::readSelfFromChannel
// Description: Reads the command from an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
bool apCLFillBufferCommand::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apCLEnqueuedCommand::readSelfFromChannel(ipcChannel);

    gtUInt64 filledBufferAsUInt64 = 0;
    ipcChannel >> filledBufferAsUInt64;
    m_filledBuffer = (oaCLMemHandle)filledBufferAsUInt64;
    gtUInt64 filledPatternSizeAsUInt64 = 0;
    ipcChannel >> filledPatternSizeAsUInt64;
    m_filledPatternSize = (gtSize_t)filledPatternSizeAsUInt64;
    m_filledPattern = new gtUByte[m_filledPatternSize];
    // Do not ASSERT_ALLOCATION, to avoid hanging the pipe:
    GT_ASSERT(NULL != m_filledPattern);
    gtUByte patternByte = 0;

    for (gtSize_t i = 0; i < m_filledPatternSize; i++)
    {
        ipcChannel >> patternByte;

        if (NULL != m_filledPattern)
        {
            m_filledPattern[i] = patternByte;
        }
    }

    gtUInt64 filledOffsetAsUInt64 = 0;
    ipcChannel >> filledOffsetAsUInt64;
    m_filledOffset = (gtSize_t)filledOffsetAsUInt64;
    gtUInt64 filledSizeAsUInt64 = 0;
    ipcChannel >> filledSizeAsUInt64;
    m_filledSize = (gtSize_t)filledSizeAsUInt64;

    return retVal;
}


// ---------------------------------- apCLFillImageCommand ---------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLFillImageCommand::apCLFillImageCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
apCLFillImageCommand::apCLFillImageCommand()
    : apCLEnqueuedCommand(), m_filledImage(OA_CL_NULL_HANDLE), m_fillColor((osProcedureAddress64)NULL)
{
    m_filledOrigin[0] = 0;
    m_filledOrigin[1] = 0;
    m_filledOrigin[2] = 0;
    m_filledRegion[0] = 0;
    m_filledRegion[1] = 0;
    m_filledRegion[2] = 0;
}

// ---------------------------------------------------------------------------
// Name:        apCLFillImageCommand::apCLFillImageCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
apCLFillImageCommand::apCLFillImageCommand(oaCLMemHandle image, const void* fill_color, const gtSize_t origin[3], const gtSize_t region[3],
                                           cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
    : apCLEnqueuedCommand(num_events_in_wait_list, event_wait_list, event), m_filledImage(image), m_fillColor((osProcedureAddress64)fill_color)
{
    m_filledOrigin[0] = origin[0];
    m_filledOrigin[1] = origin[1];
    m_filledOrigin[2] = origin[2];
    m_filledRegion[0] = region[0];
    m_filledRegion[1] = region[1];
    m_filledRegion[2] = region[2];
}

// ---------------------------------------------------------------------------
// Name:        apCLFillImageCommand::~apCLFillImageCommand
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
apCLFillImageCommand::~apCLFillImageCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLFillImageCommand::commandNameAsString
// Description: Gets the command name as a string
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
void apCLFillImageCommand::commandNameAsString(gtString& commandName) const
{
    commandName = L"FillImage";
}

// ---------------------------------------------------------------------------
// Name:        apCLFillImageCommand::relatedDstMemHandle
// Description: Returns the related dst mem (buffer / texture) handles.
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
void apCLFillImageCommand::getRelatedDstMemHandles(gtVector<oaCLMemHandle>& dstMemHandles) const
{
    dstMemHandles.clear();
    dstMemHandles.push_back(m_filledImage);
}

// ---------------------------------------------------------------------------
// Name:        apCLFillImageCommand::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
osTransferableObjectType apCLFillImageCommand::type() const
{
    return OS_TOBJ_ID_CL_FILL_IMAGE_COMMAND;
}

// ---------------------------------------------------------------------------
// Name:        apCLFillImageCommand::writeSelfIntoChannel
// Description: Writes the command into an ipc channel
// Return Val:  bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
bool apCLFillImageCommand::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apCLEnqueuedCommand::writeSelfIntoChannel(ipcChannel);

    ipcChannel << (gtUInt64)m_filledImage;
    ipcChannel << (gtUInt64)m_fillColor;
    ipcChannel << (gtUInt64)m_filledOrigin[0];
    ipcChannel << (gtUInt64)m_filledOrigin[1];
    ipcChannel << (gtUInt64)m_filledOrigin[2];
    ipcChannel << (gtUInt64)m_filledRegion[0];
    ipcChannel << (gtUInt64)m_filledRegion[1];
    ipcChannel << (gtUInt64)m_filledRegion[2];

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLFillImageCommand::readSelfFromChannel
// Description: Reads the command from an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
bool apCLFillImageCommand::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apCLEnqueuedCommand::readSelfFromChannel(ipcChannel);

    gtUInt64 filledImageAsUInt64 = 0;
    ipcChannel >> filledImageAsUInt64;
    m_filledImage = (oaCLMemHandle)filledImageAsUInt64;
    gtUInt64 fillColorAsUInt64 = 0;
    ipcChannel >> fillColorAsUInt64;
    m_fillColor = (osProcedureAddress64)fillColorAsUInt64;
    gtUInt64 filledOrigin0AsUInt64 = 0;
    ipcChannel >> filledOrigin0AsUInt64;
    m_filledOrigin[0] = (gtSize_t)filledOrigin0AsUInt64;
    gtUInt64 filledOrigin1AsUInt64 = 0;
    ipcChannel >> filledOrigin1AsUInt64;
    m_filledOrigin[1] = (gtSize_t)filledOrigin1AsUInt64;
    gtUInt64 filledOrigin2AsUInt64 = 0;
    ipcChannel >> filledOrigin2AsUInt64;
    m_filledOrigin[2] = (gtSize_t)filledOrigin2AsUInt64;
    gtUInt64 filledRegion0AsUInt64 = 0;
    ipcChannel >> filledRegion0AsUInt64;
    m_filledRegion[0] = (gtSize_t)filledRegion0AsUInt64;
    gtUInt64 filledRegion1AsUInt64 = 0;
    ipcChannel >> filledRegion1AsUInt64;
    m_filledRegion[1] = (gtSize_t)filledRegion1AsUInt64;
    gtUInt64 filledRegion2AsUInt64 = 0;
    ipcChannel >> filledRegion2AsUInt64;
    m_filledRegion[2] = (gtSize_t)filledRegion2AsUInt64;

    return retVal;
}


// ---------------------------------- apCLMigrateMemObjectsCommand ---------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLMigrateMemObjectsCommand::apCLMigrateMemObjectsCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
apCLMigrateMemObjectsCommand::apCLMigrateMemObjectsCommand()
    : apCLEnqueuedCommand(), m_migrationFlags(0)
{

}

// ---------------------------------------------------------------------------
// Name:        apCLMigrateMemObjectsCommand::apCLMigrateMemObjectsCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
apCLMigrateMemObjectsCommand::apCLMigrateMemObjectsCommand(cl_uint num_mem_objects, const cl_mem* mem_objects, cl_mem_migration_flags flags,
                                                           cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
    : apCLEnqueuedCommand(num_events_in_wait_list, event_wait_list, event), m_migrationFlags(flags)
{
    for (cl_uint i = 0; i < num_mem_objects; i++)
    {
        m_migratedMemObjects.push_back((oaCLMemHandle)mem_objects[i]);
    }
}

// ---------------------------------------------------------------------------
// Name:        apCLMigrateMemObjectsCommand::~apCLMigrateMemObjectsCommand
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
apCLMigrateMemObjectsCommand::~apCLMigrateMemObjectsCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLMigrateMemObjectsCommand::commandNameAsString
// Description: Gets the command name as a string
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
void apCLMigrateMemObjectsCommand::commandNameAsString(gtString& commandName) const
{
    commandName = L"MigrateMemObjects";
}

// ---------------------------------------------------------------------------
// Name:        apCLMigrateMemObjectsCommand::relatedSrcMemHandle
// Description: Returns the related src mem (buffer / texture) handles.
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
void apCLMigrateMemObjectsCommand::getRelatedSrcMemHandles(gtVector<oaCLMemHandle>& srcMemHandles) const
{
    srcMemHandles.clear();
    int numberOfMemObjects = (int)m_migratedMemObjects.size();

    for (int i = 0; i < numberOfMemObjects; i++)
    {
        srcMemHandles.push_back(m_migratedMemObjects[i]);
    }
}

// ---------------------------------------------------------------------------
// Name:        apCLMigrateMemObjectsCommand::relatedDstMemHandle
// Description: Returns the related dst mem (buffer / texture) handles.
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
void apCLMigrateMemObjectsCommand::getRelatedDstMemHandles(gtVector<oaCLMemHandle>& dstMemHandles) const
{
    dstMemHandles.clear();
    int numberOfMemObjects = (int)m_migratedMemObjects.size();

    for (int i = 0; i < numberOfMemObjects; i++)
    {
        dstMemHandles.push_back(m_migratedMemObjects[i]);
    }
}

// ---------------------------------------------------------------------------
// Name:        apCLMigrateMemObjectsCommand::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
osTransferableObjectType apCLMigrateMemObjectsCommand::type() const
{
    return OS_TOBJ_ID_CL_MIGRATE_MEM_OBJECTS_COMMAND;
}

// ---------------------------------------------------------------------------
// Name:        apCLMigrateMemObjectsCommand::writeSelfIntoChannel
// Description: Writes the command into an ipc channel
// Return Val:  bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
bool apCLMigrateMemObjectsCommand::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apCLEnqueuedCommand::writeSelfIntoChannel(ipcChannel);

    gtInt32 numberOfMemObjects = (gtInt32)m_migratedMemObjects.size();
    ipcChannel << numberOfMemObjects;

    for (gtInt32 i = 0; i < numberOfMemObjects; i++)
    {
        ipcChannel << (gtUInt64)m_migratedMemObjects[i];
    }

    ipcChannel << (gtInt32)m_migrationFlags;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLMigrateMemObjectsCommand::readSelfFromChannel
// Description: Reads the command from an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
bool apCLMigrateMemObjectsCommand::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apCLEnqueuedCommand::readSelfFromChannel(ipcChannel);

    gtInt32 numberOfMemObjects = 0;
    ipcChannel >> numberOfMemObjects;

    for (gtInt32 i = 0; i < numberOfMemObjects; i++)
    {
        gtUInt64 migratedMemObjectAsUInt64 = 0;
        ipcChannel >> migratedMemObjectAsUInt64;
        m_migratedMemObjects.push_back((oaCLMemHandle)migratedMemObjectAsUInt64);
    }

    gtInt32 migrationFlagsAsInt32 = 0;
    ipcChannel >> migrationFlagsAsInt32;
    m_migrationFlags = (cl_mem_migration_flags)migrationFlagsAsInt32;

    return retVal;

}

// ---------------------------------- apCLMarkerWithWaitListCommand ---------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLMarkerWithWaitListCommand::apCLMarkerWithWaitListCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
apCLMarkerWithWaitListCommand::apCLMarkerWithWaitListCommand()
    : apCLEnqueuedCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLMarkerWithWaitListCommand::apCLMarkerWithWaitListCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
apCLMarkerWithWaitListCommand::apCLMarkerWithWaitListCommand(cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
    : apCLEnqueuedCommand(num_events_in_wait_list, event_wait_list, event)
{

}

// ---------------------------------------------------------------------------
// Name:        apCLMarkerWithWaitListCommand::~apCLMarkerWithWaitListCommand
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
apCLMarkerWithWaitListCommand::~apCLMarkerWithWaitListCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLMarkerWithWaitListCommand::commandNameAsString
// Description: Gets the command name as a string
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
void apCLMarkerWithWaitListCommand::commandNameAsString(gtString& commandName) const
{
    commandName = L"MarkerWithWaitList";
}

// ---------------------------------------------------------------------------
// Name:        apCLMarkerWithWaitListCommand::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
osTransferableObjectType apCLMarkerWithWaitListCommand::type() const
{
    return OS_TOBJ_ID_CL_MARKER_WITH_WAIT_LIST_COMMAND;
}

// ---------------------------------------------------------------------------
// Name:        apCLMarkerWithWaitListCommand::writeSelfIntoChannel
// Description: Writes the command into an ipc channel
// Return Val:  bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
bool apCLMarkerWithWaitListCommand::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apCLEnqueuedCommand::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLMarkerWithWaitListCommand::readSelfFromChannel
// Description: Reads the command from an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
bool apCLMarkerWithWaitListCommand::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apCLEnqueuedCommand::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------- apCLBarrierWithWaitListCommand ---------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLBarrierWithWaitListCommand::apCLBarrierWithWaitListCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
apCLBarrierWithWaitListCommand::apCLBarrierWithWaitListCommand()
    : apCLEnqueuedCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLBarrierWithWaitListCommand::apCLBarrierWithWaitListCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
apCLBarrierWithWaitListCommand::apCLBarrierWithWaitListCommand(cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
    : apCLEnqueuedCommand(num_events_in_wait_list, event_wait_list, event)
{

}

// ---------------------------------------------------------------------------
// Name:        apCLBarrierWithWaitListCommand::~apCLBarrierWithWaitListCommand
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
apCLBarrierWithWaitListCommand::~apCLBarrierWithWaitListCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLBarrierWithWaitListCommand::commandNameAsString
// Description: Gets the command name as a string
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
void apCLBarrierWithWaitListCommand::commandNameAsString(gtString& commandName) const
{
    commandName = L"BarrierWithWaitList";
}

// ---------------------------------------------------------------------------
// Name:        apCLBarrierWithWaitListCommand::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
osTransferableObjectType apCLBarrierWithWaitListCommand::type() const
{
    return OS_TOBJ_ID_CL_BARRIER_WITH_WAIT_LIST_COMMAND;
}

// ---------------------------------------------------------------------------
// Name:        apCLBarrierWithWaitListCommand::writeSelfIntoChannel
// Description: Writes the command into an ipc channel
// Return Val:  bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
bool apCLBarrierWithWaitListCommand::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apCLEnqueuedCommand::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLBarrierWithWaitListCommand::readSelfFromChannel
// Description: Reads the command from an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
bool apCLBarrierWithWaitListCommand::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apCLEnqueuedCommand::readSelfFromChannel(ipcChannel);

    return retVal;
}


//////////////////////////////////////////////////////////////////////////
// OpenCL 2.0 Commands                                                  //
//////////////////////////////////////////////////////////////////////////

// ---------------------------------- apCLSVMFreeCommand ---------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLSVMFreeCommand::apCLSVMFreeCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        15/12/2013
// ---------------------------------------------------------------------------
apCLSVMFreeCommand::apCLSVMFreeCommand()
    : apCLEnqueuedCommand(), m_pfnFreeFunc(OS_NULL_PROCEDURE_ADDRESS_64), m_userData(OS_NULL_PROCEDURE_ADDRESS_64)
{

}

// ---------------------------------------------------------------------------
// Name:        apCLSVMFreeCommand::apCLSVMFreeCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        15/12/2013
// ---------------------------------------------------------------------------
apCLSVMFreeCommand::apCLSVMFreeCommand(cl_uint num_svm_pointers, void* svm_pointers[], apCLSVMFreeCommand::apSVMFreeCallback pfn_free_func, void* user_data, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
    : apCLEnqueuedCommand(num_events_in_wait_list, event_wait_list, event), m_pfnFreeFunc((osProcedureAddress64)pfn_free_func), m_userData((osProcedureAddress64)user_data)
{
    GT_IF_WITH_ASSERT(NULL != svm_pointers)
    {
        for (cl_uint i = 0; i < num_svm_pointers; i++)
        {
            m_svmPointers.push_back((osProcedureAddress64)(svm_pointers[i]));
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        apCLSVMFreeCommand::~apCLSVMFreeCommand
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        15/12/2013
// ---------------------------------------------------------------------------
apCLSVMFreeCommand::~apCLSVMFreeCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLSVMFreeCommand::commandNameAsString
// Description: Gets the command name as a string
// Author:  AMD Developer Tools Team
// Date:        15/12/2013
// ---------------------------------------------------------------------------
void apCLSVMFreeCommand::commandNameAsString(gtString& commandName) const
{
    commandName = L"SVMFree";
}

// ---------------------------------------------------------------------------
// Name:        apCLSVMFreeCommand::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        15/12/2013
// ---------------------------------------------------------------------------
osTransferableObjectType apCLSVMFreeCommand::type() const
{
    return OS_TOBJ_ID_CL_SVM_FREE_COMMAND;
}

// ---------------------------------------------------------------------------
// Name:        apCLSVMFreeCommand::writeSelfIntoChannel
// Description: Writes the command into an ipc channel
// Return Val:  bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        15/12/2013
// ---------------------------------------------------------------------------
bool apCLSVMFreeCommand::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apCLEnqueuedCommand::writeSelfIntoChannel(ipcChannel);

    gtUInt32 numberOfPointers = (gtUInt32)m_svmPointers.size();
    ipcChannel << numberOfPointers;

    for (gtUInt32 i = 0; i < numberOfPointers; i++)
    {
        ipcChannel << (gtUInt64)(m_svmPointers[i]);
    }

    ipcChannel << (gtUInt64)m_pfnFreeFunc;
    ipcChannel << (gtUInt64)m_userData;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLSVMFreeCommand::readSelfFromChannel
// Description: Reads the command from an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
bool apCLSVMFreeCommand::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apCLEnqueuedCommand::readSelfFromChannel(ipcChannel);

    // Clear previous members:
    m_svmPointers.clear();

    gtUInt32 numberOfPointers = 0;
    ipcChannel >> numberOfPointers;

    for (gtUInt32 i = 0; i < numberOfPointers; i++)
    {
        gtUInt64 currentPointerAsUInt64 = 0;
        ipcChannel >> currentPointerAsUInt64;
        m_svmPointers.push_back((osProcedureAddress64)currentPointerAsUInt64);
    }

    gtUInt64 pfnFreeFuncAsUInt64 = 0;
    ipcChannel >> pfnFreeFuncAsUInt64;
    m_pfnFreeFunc = (osProcedureAddress64)pfnFreeFuncAsUInt64;
    gtUInt64 userDataAsUInt64 = 0;
    ipcChannel >> userDataAsUInt64;
    m_userData = (osProcedureAddress64)userDataAsUInt64;

    return retVal;
}

// ---------------------------------- apCLSVMMemcpyCommand ---------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLSVMMemcpyCommand::apCLSVMMemcpyCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        15/12/2013
// ---------------------------------------------------------------------------
apCLSVMMemcpyCommand::apCLSVMMemcpyCommand()
    : apCLEnqueuedCommand(), m_blockingCopy(CL_FALSE), m_dstPointer(OS_NULL_PROCEDURE_ADDRESS_64), m_srcPointer(OS_NULL_PROCEDURE_ADDRESS_64), m_size(0)
{

}

// ---------------------------------------------------------------------------
// Name:        apCLSVMMemcpyCommand::apCLSVMMemcpyCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        15/12/2013
// ---------------------------------------------------------------------------
apCLSVMMemcpyCommand::apCLSVMMemcpyCommand(cl_bool blocking_copy, void* dst_ptr, const void* src_ptr, gtSize_t size, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
    : apCLEnqueuedCommand(num_events_in_wait_list, event_wait_list, event), m_blockingCopy(blocking_copy), m_dstPointer((osProcedureAddress64)dst_ptr), m_srcPointer((osProcedureAddress64)src_ptr), m_size(size)
{

}

// ---------------------------------------------------------------------------
// Name:        apCLSVMMemcpyCommand::~apCLSVMMemcpyCommand
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        15/12/2013
// ---------------------------------------------------------------------------
apCLSVMMemcpyCommand::~apCLSVMMemcpyCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLSVMMemcpyCommand::commandNameAsString
// Description: Gets the command name as a string
// Author:  AMD Developer Tools Team
// Date:        15/12/2013
// ---------------------------------------------------------------------------
void apCLSVMMemcpyCommand::commandNameAsString(gtString& commandName) const
{
    commandName = L"SVMMemcpy";
}

// ---------------------------------------------------------------------------
// Name:        apCLSVMMemcpyCommand::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        15/12/2013
// ---------------------------------------------------------------------------
osTransferableObjectType apCLSVMMemcpyCommand::type() const
{
    return OS_TOBJ_ID_CL_SVM_MEMCPY_COMMAND;
}

// ---------------------------------------------------------------------------
// Name:        apCLSVMMemcpyCommand::writeSelfIntoChannel
// Description: Writes the command into an ipc channel
// Return Val:  bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        15/12/2013
// ---------------------------------------------------------------------------
bool apCLSVMMemcpyCommand::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apCLEnqueuedCommand::writeSelfIntoChannel(ipcChannel);

    ipcChannel << (gtUInt32)m_blockingCopy;
    ipcChannel << (gtUInt64)m_dstPointer;
    ipcChannel << (gtUInt64)m_srcPointer;
    ipcChannel << (gtUInt64)m_size;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLSVMMemcpyCommand::readSelfFromChannel
// Description: Reads the command from an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
bool apCLSVMMemcpyCommand::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apCLEnqueuedCommand::readSelfFromChannel(ipcChannel);

    gtUInt32 blockingCopyAsUInt32 = 0;
    ipcChannel >> blockingCopyAsUInt32;
    m_blockingCopy = (cl_bool)blockingCopyAsUInt32;
    gtUInt64 dstPointerAsUInt64 = 0;
    ipcChannel >> dstPointerAsUInt64;
    m_dstPointer = (osProcedureAddress64)dstPointerAsUInt64;
    gtUInt64 srcPointerAsUInt64 = 0;
    ipcChannel >> srcPointerAsUInt64;
    m_srcPointer = (osProcedureAddress64)srcPointerAsUInt64;
    gtUInt64 sizeAsUInt64 = 0;
    ipcChannel >> sizeAsUInt64;
    m_size = (gtSize_t)sizeAsUInt64;

    return retVal;
}

// ---------------------------------- apCLSVMMemFillCommand ---------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLSVMMemFillCommand::apCLSVMMemFillCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        15/12/2013
// ---------------------------------------------------------------------------
apCLSVMMemFillCommand::apCLSVMMemFillCommand()
    : apCLEnqueuedCommand(), m_svmPtr(OS_NULL_PROCEDURE_ADDRESS_64), m_filledPattern(NULL), m_filledPatternSize(0), m_size(0)
{

}

// ---------------------------------------------------------------------------
// Name:        apCLSVMMemFillCommand::apCLSVMMemFillCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        15/12/2013
// ---------------------------------------------------------------------------
apCLSVMMemFillCommand::apCLSVMMemFillCommand(void* svm_ptr, const void* pattern, gtSize_t pattern_size, gtSize_t size, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
    : apCLEnqueuedCommand(num_events_in_wait_list, event_wait_list, event), m_svmPtr((osProcedureAddress64)svm_ptr), m_filledPattern(NULL), m_filledPatternSize(pattern_size), m_size(size)
{
    if (0 < pattern_size)
    {
        // Copy the pattern:
        m_filledPattern = new gtUByte[m_filledPatternSize];


        for (gtSize_t i = 0; i < m_filledPatternSize; i++)
        {
            m_filledPattern[i] = ((gtUByte*)pattern)[i];
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        apCLSVMMemFillCommand::~apCLSVMMemFillCommand
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        15/12/2013
// ---------------------------------------------------------------------------
apCLSVMMemFillCommand::~apCLSVMMemFillCommand()
{
    m_filledPatternSize = 0;
    delete[] m_filledPattern;
    m_filledPattern = NULL;
}

// ---------------------------------------------------------------------------
// Name:        apCLSVMMemFillCommand::commandNameAsString
// Description: Gets the command name as a string
// Author:  AMD Developer Tools Team
// Date:        15/12/2013
// ---------------------------------------------------------------------------
void apCLSVMMemFillCommand::commandNameAsString(gtString& commandName) const
{
    commandName = L"SVMMemFill";
}

// ---------------------------------------------------------------------------
// Name:        apCLSVMMemFillCommand::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        15/12/2013
// ---------------------------------------------------------------------------
osTransferableObjectType apCLSVMMemFillCommand::type() const
{
    return OS_TOBJ_ID_CL_SVM_MEM_FILL_COMMAND;
}

// ---------------------------------------------------------------------------
// Name:        apCLSVMMemFillCommand::writeSelfIntoChannel
// Description: Writes the command into an ipc channel
// Return Val:  bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        15/12/2013
// ---------------------------------------------------------------------------
bool apCLSVMMemFillCommand::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apCLEnqueuedCommand::writeSelfIntoChannel(ipcChannel);

    ipcChannel << (gtUInt64)m_svmPtr;
    ipcChannel << (gtUInt64)m_filledPatternSize;

    for (gtSize_t i = 0; i < m_filledPatternSize; i++)
    {
        ipcChannel << m_filledPattern[i];
    }

    ipcChannel << (gtUInt64)m_size;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLSVMMemFillCommand::readSelfFromChannel
// Description: Reads the command from an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
bool apCLSVMMemFillCommand::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apCLEnqueuedCommand::readSelfFromChannel(ipcChannel);

    // Clear previous members:
    m_filledPatternSize = 0;
    delete[] m_filledPattern;
    m_filledPattern = NULL;

    gtUInt64 svmPtrAsUInt64 = 0;
    ipcChannel >> svmPtrAsUInt64;
    m_svmPtr = (osProcedureAddress64)svmPtrAsUInt64;

    gtUInt64 filledPatternSizeAsUInt64 = 0;
    ipcChannel >> filledPatternSizeAsUInt64;
    m_filledPatternSize = (gtSize_t)filledPatternSizeAsUInt64;
    m_filledPattern = new gtUByte[m_filledPatternSize];
    // Do not ASSERT_ALLOCATION, to avoid hanging the pipe:
    GT_ASSERT(NULL != m_filledPattern);
    gtUByte patternByte = 0;

    for (gtSize_t i = 0; i < m_filledPatternSize; i++)
    {
        ipcChannel >> patternByte;

        if (NULL != m_filledPattern)
        {
            m_filledPattern[i] = patternByte;
        }
    }

    gtUInt64 sizeAsUInt64 = 0;
    ipcChannel >> sizeAsUInt64;
    m_size = (gtSize_t)sizeAsUInt64;

    return retVal;
}


// ---------------------------------- apCLSVMMapCommand ---------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLSVMMapCommand::apCLSVMMapCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        15/12/2013
// ---------------------------------------------------------------------------
apCLSVMMapCommand::apCLSVMMapCommand()
    : apCLEnqueuedCommand(), m_blockingMap(CL_FALSE), m_flags(0), m_svmPtr(OS_NULL_PROCEDURE_ADDRESS_64), m_size(0)
{

}

// ---------------------------------------------------------------------------
// Name:        apCLSVMMapCommand::apCLSVMMapCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        15/12/2013
// ---------------------------------------------------------------------------
apCLSVMMapCommand::apCLSVMMapCommand(cl_bool blocking_map, cl_map_flags flags, void* svm_ptr, gtSize_t size, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
    : apCLEnqueuedCommand(num_events_in_wait_list, event_wait_list, event), m_blockingMap(blocking_map), m_flags(flags), m_svmPtr((osProcedureAddress64)svm_ptr), m_size(size)
{

}

// ---------------------------------------------------------------------------
// Name:        apCLSVMMapCommand::~apCLSVMMapCommand
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        15/12/2013
// ---------------------------------------------------------------------------
apCLSVMMapCommand::~apCLSVMMapCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLSVMMapCommand::commandNameAsString
// Description: Gets the command name as a string
// Author:  AMD Developer Tools Team
// Date:        15/12/2013
// ---------------------------------------------------------------------------
void apCLSVMMapCommand::commandNameAsString(gtString& commandName) const
{
    commandName = L"SVMMap";
}

// ---------------------------------------------------------------------------
// Name:        apCLSVMMapCommand::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        15/12/2013
// ---------------------------------------------------------------------------
osTransferableObjectType apCLSVMMapCommand::type() const
{
    return OS_TOBJ_ID_CL_SVM_MAP_COMMAND;
}

// ---------------------------------------------------------------------------
// Name:        apCLSVMMapCommand::writeSelfIntoChannel
// Description: Writes the command into an ipc channel
// Return Val:  bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        15/12/2013
// ---------------------------------------------------------------------------
bool apCLSVMMapCommand::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apCLEnqueuedCommand::writeSelfIntoChannel(ipcChannel);

    ipcChannel << (gtUInt32)m_blockingMap;
    ipcChannel << (gtUInt32)m_flags;
    ipcChannel << (gtUInt64)m_svmPtr;
    ipcChannel << (gtUInt64)m_size;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLSVMMapCommand::readSelfFromChannel
// Description: Reads the command from an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
bool apCLSVMMapCommand::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apCLEnqueuedCommand::readSelfFromChannel(ipcChannel);
    gtUInt32 blockingMapAsUInt32 = 0;
    ipcChannel >> blockingMapAsUInt32;
    m_blockingMap = (cl_bool)blockingMapAsUInt32;
    gtUInt32 flagsAsUInt32 = 0;
    ipcChannel >> flagsAsUInt32;
    m_flags = (cl_map_flags)flagsAsUInt32;
    gtUInt64 svmPtrAsUInt64 = 0;
    ipcChannel >> svmPtrAsUInt64;
    m_svmPtr = (osProcedureAddress64)svmPtrAsUInt64;
    gtUInt64 sizeAsUInt64 = 0;
    ipcChannel >> sizeAsUInt64;
    m_size = (gtSize_t)sizeAsUInt64;

    return retVal;
}

// ---------------------------------- apCLSVMUnmapCommand ---------------------------------

// ---------------------------------------------------------------------------
// Name:        apCLSVMUnmapCommand::apCLSVMUnmapCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        15/12/2013
// ---------------------------------------------------------------------------
apCLSVMUnmapCommand::apCLSVMUnmapCommand()
    : apCLEnqueuedCommand(), m_svmPtr(OS_NULL_PROCEDURE_ADDRESS_64)
{

}

// ---------------------------------------------------------------------------
// Name:        apCLSVMUnmapCommand::apCLSVMUnmapCommand
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        15/12/2013
// ---------------------------------------------------------------------------
apCLSVMUnmapCommand::apCLSVMUnmapCommand(void* svm_ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event)
    : apCLEnqueuedCommand(num_events_in_wait_list, event_wait_list, event), m_svmPtr((osProcedureAddress64)svm_ptr)
{

}

// ---------------------------------------------------------------------------
// Name:        apCLSVMUnmapCommand::~apCLSVMUnmapCommand
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        15/12/2013
// ---------------------------------------------------------------------------
apCLSVMUnmapCommand::~apCLSVMUnmapCommand()
{

}

// ---------------------------------------------------------------------------
// Name:        apCLSVMUnmapCommand::commandNameAsString
// Description: Gets the command name as a string
// Author:  AMD Developer Tools Team
// Date:        15/12/2013
// ---------------------------------------------------------------------------
void apCLSVMUnmapCommand::commandNameAsString(gtString& commandName) const
{
    commandName = L"SVMUnmap";
}

// ---------------------------------------------------------------------------
// Name:        apCLSVMUnmapCommand::type
// Description: Returns my Transferable object type.
// Author:  AMD Developer Tools Team
// Date:        15/12/2013
// ---------------------------------------------------------------------------
osTransferableObjectType apCLSVMUnmapCommand::type() const
{
    return OS_TOBJ_ID_CL_SVM_UNMAP_COMMAND;
}

// ---------------------------------------------------------------------------
// Name:        apCLSVMUnmapCommand::writeSelfIntoChannel
// Description: Writes the command into an ipc channel
// Return Val:  bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        15/12/2013
// ---------------------------------------------------------------------------
bool apCLSVMUnmapCommand::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = apCLEnqueuedCommand::writeSelfIntoChannel(ipcChannel);

    ipcChannel << (gtUInt64)m_svmPtr;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLSVMUnmapCommand::readSelfFromChannel
// Description: Reads the command from an ipc channel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        12/8/2013
// ---------------------------------------------------------------------------
bool apCLSVMUnmapCommand::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = apCLEnqueuedCommand::readSelfFromChannel(ipcChannel);

    gtUInt64 svmPtrAsUInt64 = 0;
    ipcChannel >> svmPtrAsUInt64;
    m_svmPtr = (osProcedureAddress64)svmPtrAsUInt64;

    return retVal;
}


