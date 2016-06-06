//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCLEnqueuedCommands.h
///
//==================================================================================

//------------------------------ apCLEnqueuedCommands.h ------------------------------

#ifndef __APCLENQUEUEDCOMMANDS_H
#define __APCLENQUEUEDCOMMANDS_H

// Forward declarations:
class gtString;

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osTransferableObject.h>
#include <AMDTOSAPIWrappers/Include/oaOpenCLIncludes.h>
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>

#define AP_NANOSECOND_TO_MILLISECOND (1000 * 1000)

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLEnqueuedCommand : public osTransferableObject
// General Description: A class used to represent a command added to an OpenCL queue
//                      by one of the clEnqueueXXXX functions. This is a virtual class,
//                      used as a base for classes that represent each command type.
// Author:  AMD Developer Tools Team
// Creation Date:       1/12/2009
// ----------------------------------------------------------------------------------
class AP_API apCLEnqueuedCommand : public osTransferableObject
{
public:
    apCLEnqueuedCommand();
    apCLEnqueuedCommand(cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
    virtual ~apCLEnqueuedCommand();

    // Specific command data:
    virtual void commandNameAsString(gtString& commandName) const = 0;
    virtual void getRelatedKernelHandles(gtVector<oaCLKernelHandle>& kernelHandles) const;
    virtual void getRelatedSrcMemHandles(gtVector<oaCLMemHandle>& srcMemHandles) const;
    virtual void getRelatedDstMemHandles(gtVector<oaCLMemHandle>& dstMemHandles) const;
    virtual bool eventWaitListParameterAvailable() const;
    virtual bool eventParameterAvailable() const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const = 0;
    virtual bool isCLEnqueuedCommandObject() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Execution Times:
    const gtUInt64& queuedTime() const {return _queuedTime;};
    const gtUInt64& submittedTime() const {return _submittedTime;};
    const gtUInt64& executionStartedTime() const {return _executionStartedTime;};
    const gtUInt64& executionEndedTime() const {return _executionEndedTime;};
    const gtUInt64& executionCompletedTime() const {return m_executionCompletedTime;};
    void setEventTimes(const gtUInt64& queuedTime, const gtUInt64& submittedTime, const gtUInt64& executionStartedTime, const gtUInt64& executionEndedTime, const gtUInt64& executionCompletedTime)
    {_queuedTime = queuedTime; _submittedTime = submittedTime; _executionStartedTime = executionStartedTime; _executionEndedTime = executionEndedTime; m_executionCompletedTime = executionCompletedTime;};
    gtUInt64 waitForSubmit() const;
    gtUInt64 waitForExecution() const;
    gtUInt64 executionDuration() const;
    bool areTimesUpdated() const {return _areTimesUpdated;};
    void onTimesUpdated() {_areTimesUpdated = true;};

    // Event:
    oaCLEventHandle commandEventHandle() const {return _commandEventHandle;};
    oaCLEventHandle commandEventExternalHandle() const {return _wasEventCreatedBySpy ? OA_CL_NULL_HANDLE : _commandEventHandle;};
    bool wasEventCreatedBySpy() const {return _wasEventCreatedBySpy;};
    void setCommandEventHandle(oaCLEventHandle commandEvent, bool wasCreatedBySpy) {_commandEventHandle = commandEvent; _wasEventCreatedBySpy = wasCreatedBySpy;};

    // Parameters shared by many commands:
    const gtVector<oaCLEventHandle>& eventWaitList() const {return _event_wait_list;};
    const oaCLEventHandle& userGeneratedEvent() const {return _event;};

private:
    gtUInt64 _queuedTime;
    gtUInt64 _submittedTime;
    gtUInt64 _executionStartedTime;
    gtUInt64 _executionEndedTime;
    gtUInt64 m_executionCompletedTime;
    bool _areTimesUpdated;

    oaCLEventHandle _commandEventHandle;
    bool _wasEventCreatedBySpy;

    gtVector<oaCLEventHandle> _event_wait_list;
    oaCLEventHandle _event;
};


// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLAcquireGLObjectsCommand : public apCLEnqueuedCommand
// General Description: A class used to represent a command added to a queue by a
//                      clEnqueueAcquireGLObjects function call.
// Author:  AMD Developer Tools Team
// Creation Date:       1/12/2009
// ----------------------------------------------------------------------------------
class AP_API apCLAcquireGLObjectsCommand : public apCLEnqueuedCommand
{
public:
    apCLAcquireGLObjectsCommand();
    apCLAcquireGLObjectsCommand(cl_uint num_objects, const cl_mem* mem_objects, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
    virtual ~apCLAcquireGLObjectsCommand();

    // Overrides apCLEnqueuedCommand:
    virtual void commandNameAsString(gtString& commandName) const;
    virtual void getRelatedDstMemHandles(gtVector<oaCLMemHandle>& dstMemHandles) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    const gtVector<oaCLMemHandle>& memObjects() const {return _mem_objects;};

private:
    // Function parameters:
    gtVector<oaCLMemHandle> _mem_objects;
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLBarrierCommand : public apCLEnqueuedCommand
// General Description: A class used to represent a command added to a queue by a
//                      clEnqueueBarrier function call
// Author:  AMD Developer Tools Team
// Creation Date:       1/12/2009
// ----------------------------------------------------------------------------------
class AP_API apCLBarrierCommand : public apCLEnqueuedCommand
{
public:
    apCLBarrierCommand();
    virtual ~apCLBarrierCommand();

    // Overrides apCLEnqueuedCommand:
    virtual void commandNameAsString(gtString& commandName) const;
    virtual bool eventWaitListParameterAvailable() const;
    virtual bool eventParameterAvailable() const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLCopyBufferCommand : public apCLEnqueuedCommand
// General Description: A class used to represent a command added to a queue by a
//                      clEnqueueCopyBuffer function call
// Author:  AMD Developer Tools Team
// Creation Date:       1/12/2009
// ----------------------------------------------------------------------------------
class AP_API apCLCopyBufferCommand : public apCLEnqueuedCommand
{
public:
    apCLCopyBufferCommand();
    apCLCopyBufferCommand(oaCLMemHandle src_buffer, oaCLMemHandle dst_buffer, gtSize_t src_offset, gtSize_t dst_offset,
                          gtSize_t cb, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
    virtual ~apCLCopyBufferCommand();

    // Overrides apCLEnqueuedCommand:
    virtual void commandNameAsString(gtString& commandName) const;
    virtual void getRelatedSrcMemHandles(gtVector<oaCLMemHandle>& srcMemHandles) const;
    virtual void getRelatedDstMemHandles(gtVector<oaCLMemHandle>& dstMemHandles) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    oaCLMemHandle sourceBuffer() const {return _src_buffer;};
    oaCLMemHandle destinationBuffer() const {return _dst_buffer;};
    gtSize_t sourceOffset() const {return _src_offset;};
    gtSize_t destinationOffset() const {return _dst_offset;};
    gtSize_t copiedBytes() const {return _cb;};

private:
    oaCLMemHandle _src_buffer;
    oaCLMemHandle _dst_buffer;
    gtSize_t _src_offset;
    gtSize_t _dst_offset;
    gtSize_t _cb;
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLCopyBufferRectCommand : public apCLEnqueuedCommand
// General Description: A class used to represent a command added to a queue by a
//                      clEnqueueCopyBufferRect function call
// Author:  AMD Developer Tools Team
// Creation Date:       26/12/2010
// ----------------------------------------------------------------------------------
class AP_API apCLCopyBufferRectCommand : public apCLEnqueuedCommand
{
public:
    apCLCopyBufferRectCommand();
    apCLCopyBufferRectCommand(oaCLMemHandle src_buffer, oaCLMemHandle dst_buffer, const gtSize_t src_origin[3], const gtSize_t dst_origin[3],
                              const gtSize_t region[3], gtSize_t src_row_pitch, gtSize_t src_slice_pitch, gtSize_t dst_row_pitch, gtSize_t dst_slice_pitch,
                              cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
    virtual ~apCLCopyBufferRectCommand();

    // Overrides apCLEnqueuedCommand:
    virtual void commandNameAsString(gtString& commandName) const;
    virtual void getRelatedSrcMemHandles(gtVector<oaCLMemHandle>& srcMemHandles) const;
    virtual void getRelatedDstMemHandles(gtVector<oaCLMemHandle>& dstMemHandles) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    oaCLMemHandle sourceBuffer() const {return _src_buffer;};
    oaCLMemHandle destinationBuffer() const {return _dst_buffer;};
    gtSize_t sourceOriginX() const {return _src_origin[0];};
    gtSize_t sourceOriginY() const {return _src_origin[1];};
    gtSize_t sourceOriginZ() const {return _src_origin[2];};
    gtSize_t destinationOriginX() const {return _dst_origin[0];};
    gtSize_t destinationOriginY() const {return _dst_origin[1];};
    gtSize_t destinationOriginZ() const {return _dst_origin[2];};
    gtSize_t copiedRegionX() const {return _region[0];};
    gtSize_t copiedRegionY() const {return _region[1];};
    gtSize_t copiedRegionZ() const {return _region[2];};
    gtSize_t sourceRowPitch() const {return _src_row_pitch;};
    gtSize_t sourceSlicePitch() const {return _src_slice_pitch;};
    gtSize_t destinationRowPitch() const {return _dst_row_pitch;};
    gtSize_t destinationSlicePitch() const {return _dst_slice_pitch;};

private:
    oaCLMemHandle _src_buffer;
    oaCLMemHandle _dst_buffer;
    gtSize_t _src_origin[3];
    gtSize_t _dst_origin[3];
    gtSize_t _region[3];
    gtSize_t _src_row_pitch;
    gtSize_t _src_slice_pitch;
    gtSize_t _dst_row_pitch;
    gtSize_t _dst_slice_pitch;
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLCopyBufferToImageCommand : public apCLEnqueuedCommand
// General Description: A class used to represent a command added to a queue by a
//                      clEnqueueCopyBufferToImage function call
// Author:  AMD Developer Tools Team
// Creation Date:       1/12/2009
// ----------------------------------------------------------------------------------
class AP_API apCLCopyBufferToImageCommand : public apCLEnqueuedCommand
{
public:
    apCLCopyBufferToImageCommand();
    apCLCopyBufferToImageCommand(oaCLMemHandle src_buffer, oaCLMemHandle dst_image, gtSize_t src_offset, const gtSize_t dst_origin[3],
                                 const gtSize_t region[3], cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
    virtual ~apCLCopyBufferToImageCommand();

    // Overrides apCLEnqueuedCommand:
    virtual void commandNameAsString(gtString& commandName) const;
    virtual void getRelatedSrcMemHandles(gtVector<oaCLMemHandle>& srcMemHandles) const;
    virtual void getRelatedDstMemHandles(gtVector<oaCLMemHandle>& dstMemHandles) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    oaCLMemHandle sourceBuffer() const {return _src_buffer;};
    oaCLMemHandle destinationImage() const {return _dst_image;};
    gtSize_t sourceOffset() const {return _src_offset;};
    gtSize_t destinationOriginX() const {return _dst_origin[0];};
    gtSize_t destinationOriginY() const {return _dst_origin[1];};
    gtSize_t destinationOriginZ() const {return _dst_origin[2];};
    gtSize_t copiedRegionX() const {return _region[0];};
    gtSize_t copiedRegionY() const {return _region[1];};
    gtSize_t copiedRegionZ() const {return _region[2];};

    // Amount of pixels written:
    gtSize_t amountOfPixels();

private:
    oaCLMemHandle _src_buffer;
    oaCLMemHandle _dst_image;
    gtSize_t _src_offset;
    gtSize_t _dst_origin[3];
    gtSize_t _region[3];
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLCopyImageCommand : public apCLEnqueuedCommand
// General Description: A class used to represent a command added to a queue by a
//                      clEnqueueCopyImage function call
// Author:  AMD Developer Tools Team
// Creation Date:       1/12/2009
// ----------------------------------------------------------------------------------
class AP_API apCLCopyImageCommand : public apCLEnqueuedCommand
{
public:
    apCLCopyImageCommand();
    apCLCopyImageCommand(oaCLMemHandle src_image, oaCLMemHandle dst_image, const gtSize_t src_origin[3], const gtSize_t dst_origin[3],
                         const gtSize_t region[3], cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
    virtual ~apCLCopyImageCommand();

    // Overrides apCLEnqueuedCommand:
    virtual void commandNameAsString(gtString& commandName) const;
    virtual void getRelatedSrcMemHandles(gtVector<oaCLMemHandle>& srcMemHandles) const;
    virtual void getRelatedDstMemHandles(gtVector<oaCLMemHandle>& dstMemHandles) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    oaCLMemHandle sourceImage() const {return _src_image;};
    oaCLMemHandle destinationImage() const {return _dst_image;};
    gtSize_t sourceOriginX() const {return _src_origin[0];};
    gtSize_t sourceOriginY() const {return _src_origin[1];};
    gtSize_t sourceOriginZ() const {return _src_origin[2];};
    gtSize_t destinationOriginX() const {return _dst_origin[0];};
    gtSize_t destinationOriginY() const {return _dst_origin[1];};
    gtSize_t destinationOriginZ() const {return _dst_origin[2];};
    gtSize_t copiedRegionX() const {return _region[0];};
    gtSize_t copiedRegionY() const {return _region[1];};
    gtSize_t copiedRegionZ() const {return _region[2];};

    // Amount of pixels written:
    gtSize_t amountOfPixels();

private:
    oaCLMemHandle _src_image;
    oaCLMemHandle _dst_image;
    gtSize_t _src_origin[3];
    gtSize_t _dst_origin[3];
    gtSize_t _region[3];
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLCopyImageToBufferCommand : public apCLEnqueuedCommand
// General Description: A class used to represent a command added to a queue by a
//                      clEnqueueCopyImageToBuffer function call
// Author:  AMD Developer Tools Team
// Creation Date:       1/12/2009
// ----------------------------------------------------------------------------------
class AP_API apCLCopyImageToBufferCommand : public apCLEnqueuedCommand
{
public:
    apCLCopyImageToBufferCommand();
    apCLCopyImageToBufferCommand(oaCLMemHandle src_image, oaCLMemHandle dst_buffer, const gtSize_t src_origin[3], const gtSize_t region[3],
                                 gtSize_t dst_offset, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
    virtual ~apCLCopyImageToBufferCommand();

    // Overrides apCLEnqueuedCommand:
    virtual void commandNameAsString(gtString& commandName) const;
    virtual void getRelatedSrcMemHandles(gtVector<oaCLMemHandle>& srcMemHandles) const;
    virtual void getRelatedDstMemHandles(gtVector<oaCLMemHandle>& dstMemHandles) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    oaCLMemHandle sourceImage() const {return _src_image;};
    oaCLMemHandle destinationBuffer() const {return _dst_buffer;};
    gtSize_t sourceOriginX() const {return _src_origin[0];};
    gtSize_t sourceOriginY() const {return _src_origin[1];};
    gtSize_t sourceOriginZ() const {return _src_origin[2];};
    gtSize_t copiedRegionX() const {return _region[0];};
    gtSize_t copiedRegionY() const {return _region[1];};
    gtSize_t copiedRegionZ() const {return _region[2];};
    gtSize_t destinationOffset() const {return _dst_offset;};

    // Amount of pixels written:
    gtSize_t amountOfPixels();

private:
    oaCLMemHandle _src_image;
    oaCLMemHandle _dst_buffer;
    gtSize_t _src_origin[3];
    gtSize_t _region[3];
    gtSize_t _dst_offset;
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLMapBufferCommand : public apCLEnqueuedCommand
// General Description: A class used to represent a command added to a queue by a
//                      clEnqueueMapBuffer function call
// Author:  AMD Developer Tools Team
// Creation Date:       1/12/2009
// ----------------------------------------------------------------------------------
class AP_API apCLMapBufferCommand : public apCLEnqueuedCommand
{
public:
    apCLMapBufferCommand();
    apCLMapBufferCommand(oaCLMemHandle buffer, cl_bool blocking_map, cl_map_flags map_flags, gtSize_t offset, gtSize_t cb,
                         cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event, osProcedureAddress64 errcode_ret);
    virtual ~apCLMapBufferCommand();

    // Overrides apCLEnqueuedCommand:
    virtual void commandNameAsString(gtString& commandName) const;
    virtual void getRelatedDstMemHandles(gtVector<oaCLMemHandle>& dstMemHandles) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    oaCLMemHandle mappedBuffer() const {return _buffer;};
    cl_bool isBlockingMap() const {return _blocking_map;};
    cl_map_flags mapFlags() const {return _map_flags;};
    gtSize_t mappedDataOffset() const {return _offset;};
    gtSize_t mappedDataBytes() const {return _cb;};
    osProcedureAddress64 errorCodeRet() const {return _errcode_ret;};

private:
    oaCLMemHandle _buffer;
    cl_bool _blocking_map;
    cl_map_flags _map_flags;
    gtSize_t _offset;
    gtSize_t _cb;
    osProcedureAddress64 _errcode_ret;
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLMapImageCommand : public apCLEnqueuedCommand
// General Description: A class used to represent a command added to a queue by a
//                      clEnqueueMapImage function call
// Author:  AMD Developer Tools Team
// Creation Date:       1/12/2009
// ----------------------------------------------------------------------------------
class AP_API apCLMapImageCommand : public apCLEnqueuedCommand
{
public:
    apCLMapImageCommand();
    apCLMapImageCommand(oaCLMemHandle image, cl_bool blocking_map, cl_map_flags map_flags, const gtSize_t origin[3],
                        const gtSize_t region[3], gtSize_t* image_row_pitch, gtSize_t* image_slice_pitch,
                        cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event, osProcedureAddress64 errcode_ret);
    virtual ~apCLMapImageCommand();

    // Overrides apCLEnqueuedCommand:
    virtual void commandNameAsString(gtString& commandName) const;
    virtual void getRelatedDstMemHandles(gtVector<oaCLMemHandle>& dstMemHandles) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    oaCLMemHandle mappedImage() const {return _image;};
    cl_bool isBlockingMap() const {return _blocking_map;};
    cl_map_flags mapFlags() const {return _map_flags;};
    gtSize_t mappedDataOriginX() const {return _origin[0];};
    gtSize_t mappedDataOriginY() const {return _origin[1];};
    gtSize_t mappedDataOriginZ() const {return _origin[2];};
    gtSize_t mappedDataRegionX() const {return _region[0];};
    gtSize_t mappedDataRegionY() const {return _region[1];};
    gtSize_t mappedDataRegionZ() const {return _region[2];};
    gtSize_t hostDataRowPitch() const {return _image_row_pitch;};
    gtSize_t hostDataSlicePitch() const {return _image_slice_pitch;};
    osProcedureAddress64 errorCodeRet() const {return _errcode_ret;};

private:
    oaCLMemHandle _image;
    cl_bool _blocking_map;
    cl_map_flags _map_flags;
    gtSize_t _origin[3];
    gtSize_t _region[3];
    gtSize_t _image_row_pitch;
    gtSize_t _image_slice_pitch;
    osProcedureAddress64 _errcode_ret;
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLMarkerCommand : public apCLEnqueuedCommand
// General Description: A class used to represent a command added to a queue by a
//                      clEnqueueMarker function call
// Author:  AMD Developer Tools Team
// Creation Date:       1/12/2009
// ----------------------------------------------------------------------------------
class AP_API apCLMarkerCommand : public apCLEnqueuedCommand
{
public:
    apCLMarkerCommand();
    apCLMarkerCommand(cl_event* event);
    virtual ~apCLMarkerCommand();

    // Overrides apCLEnqueuedCommand:
    virtual void commandNameAsString(gtString& commandName) const;
    virtual bool eventWaitListParameterAvailable() const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

private:
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLNativeKernelCommand : public apCLEnqueuedCommand
// General Description: A class used to represent a command added to a queue by a
//                      clEnqueueNativeKernel function call
// Author:  AMD Developer Tools Team
// Creation Date:       1/12/2009
// ----------------------------------------------------------------------------------
class AP_API apCLNativeKernelCommand : public apCLEnqueuedCommand
{
public:
    apCLNativeKernelCommand();
    apCLNativeKernelCommand(osProcedureAddress64 user_func, osProcedureAddress64 args, gtSize_t cb_args, cl_uint num_mem_objects,
                            const cl_mem* mem_list, const void** args_mem_loc, cl_uint num_events_in_wait_list,
                            const cl_event* event_wait_list, cl_event* event);
    virtual ~apCLNativeKernelCommand();

    // Overrides apCLEnqueuedCommand:
    virtual void commandNameAsString(gtString& commandName) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    osProcedureAddress64 userFunctionAddress() const {return _user_func;};
    osProcedureAddress64 userArgumentsPointer() const {return _args;};
    gtSize_t sizeOfArgumentsData() const {return _cb_args;};
    const gtVector<oaCLMemHandle>& memoryObjectArguments() const {return _mem_list;};
    const gtVector<osProcedureAddress64>& memoryObjectArgumentMemoryLocations() const {return _args_mem_loc;};

private:
    osProcedureAddress64 _user_func;
    osProcedureAddress64 _args;
    gtSize_t _cb_args;
    gtVector<oaCLMemHandle> _mem_list;
    gtVector<osProcedureAddress64> _args_mem_loc;
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLNDRangeKernelCommand : public apCLEnqueuedCommand
// General Description: A class used to represent a command added to a queue by a
//                      clEnqueueNDRangeKernel function call
// Author:  AMD Developer Tools Team
// Creation Date:       1/12/2009
// ----------------------------------------------------------------------------------
class AP_API apCLNDRangeKernelCommand : public apCLEnqueuedCommand
{
public:
    apCLNDRangeKernelCommand();
    apCLNDRangeKernelCommand(oaCLKernelHandle kernel, cl_uint work_dim, const gtSize_t* global_work_offset, const gtSize_t* global_work_size,
                             const gtSize_t* local_work_size, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
    virtual ~apCLNDRangeKernelCommand();

    // Overrides apCLEnqueuedCommand:
    virtual void commandNameAsString(gtString& commandName) const;
    virtual void getRelatedKernelHandles(gtVector<oaCLKernelHandle>& kernelHandles) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    oaCLKernelHandle kernelHandle() const {return _kernel;};
    cl_uint workDimensions() const {return _work_dim;};
    const gtVector<gtSize_t>& globalWorkOffset() const {return _global_work_offset;};
    const gtVector<gtSize_t>& globalWorkSize() const {return _global_work_size;};
    const gtVector<gtSize_t>& localWorkSize() const {return _local_work_size;};

    // Amount of work items:
    gtUInt64 amountOfWorkItems();

private:
    oaCLKernelHandle _kernel;
    cl_uint _work_dim;
    gtVector<gtSize_t> _global_work_offset;
    gtVector<gtSize_t> _global_work_size;
    gtVector<gtSize_t> _local_work_size;
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLReadBufferCommand : public apCLEnqueuedCommand
// General Description: A class used to represent a command added to a queue by a
//                      clEnqueueReadBuffer function call
// Author:  AMD Developer Tools Team
// Creation Date:       1/12/2009
// ----------------------------------------------------------------------------------
class AP_API apCLReadBufferCommand : public apCLEnqueuedCommand
{
public:
    apCLReadBufferCommand();
    apCLReadBufferCommand(oaCLMemHandle buffer, cl_bool blocking_read, gtSize_t offset, gtSize_t cb, osProcedureAddress64 ptr,
                          cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
    virtual ~apCLReadBufferCommand();

    // Overrides apCLEnqueuedCommand:
    virtual void commandNameAsString(gtString& commandName) const;
    virtual void getRelatedSrcMemHandles(gtVector<oaCLMemHandle>& srcMemHandles) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    oaCLMemHandle readBuffer() const {return _buffer;};
    cl_bool isBlockingRead() const {return _blocking_read;};
    gtSize_t readDataOffset() const {return _offset;};
    gtSize_t readDataBytes() const {return _cb;};
    osProcedureAddress64 hostDataPointer() const {return _ptr;};

private:
    oaCLMemHandle _buffer;
    cl_bool _blocking_read;
    gtSize_t _offset;
    gtSize_t _cb;
    osProcedureAddress64 _ptr;
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLReadBufferRectCommand : public apCLEnqueuedCommand
// General Description: A class used to represent a command added to a queue by a
//                      clEnqueueReadBufferRect function call
// Author:  AMD Developer Tools Team
// Creation Date:       26/12/2010
// ----------------------------------------------------------------------------------
class AP_API apCLReadBufferRectCommand : public apCLEnqueuedCommand
{
public:
    apCLReadBufferRectCommand();
    apCLReadBufferRectCommand(oaCLMemHandle buffer, cl_bool blocking_read,
                              const gtSize_t buffer_origin[3], const gtSize_t host_origin[3], const gtSize_t region[3],
                              gtSize_t buffer_row_pitch, gtSize_t buffer_slice_pitch, gtSize_t host_row_pitch, gtSize_t host_slice_pitch,
                              osProcedureAddress64 ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
    virtual ~apCLReadBufferRectCommand();

    // Overrides apCLEnqueuedCommand:
    virtual void commandNameAsString(gtString& commandName) const;
    virtual void getRelatedSrcMemHandles(gtVector<oaCLMemHandle>& srcMemHandles) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    oaCLMemHandle readBuffer() const {return _buffer;};
    cl_bool isBlockingRead() const {return _blocking_read;};
    gtSize_t readDataOriginX() const {return _buffer_origin[0];};
    gtSize_t readDataOriginY() const {return _buffer_origin[1];};
    gtSize_t readDataOriginZ() const {return _buffer_origin[2];};
    gtSize_t hostDataOriginX() const {return _host_origin[0];};
    gtSize_t hostDataOriginY() const {return _host_origin[1];};
    gtSize_t hostDataOriginZ() const {return _host_origin[2];};
    gtSize_t readDataRegionX() const {return _region[0];};
    gtSize_t readDataRegionY() const {return _region[1];};
    gtSize_t readDataRegionZ() const {return _region[2];};
    gtSize_t readDataRowPitch() const {return _buffer_row_pitch;};
    gtSize_t readDataSlicePitch() const {return _buffer_slice_pitch;};
    gtSize_t hostDataRowPitch() const {return _host_row_pitch;};
    gtSize_t hostDataSlicePitch() const {return _host_slice_pitch;};
    osProcedureAddress64 hostDataPointer() const {return _ptr;};

private:
    oaCLMemHandle _buffer;
    cl_bool _blocking_read;
    gtSize_t _buffer_origin[3];
    gtSize_t _host_origin[3];
    gtSize_t _region[3];
    gtSize_t _buffer_row_pitch;
    gtSize_t _buffer_slice_pitch;
    gtSize_t _host_row_pitch;
    gtSize_t _host_slice_pitch;
    osProcedureAddress64 _ptr;
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLReadImageCommand : public apCLEnqueuedCommand
// General Description: A class used to represent a command added to a queue by a
//                      clEnqueueReadImage function call
// Author:  AMD Developer Tools Team
// Creation Date:       1/12/2009
// ----------------------------------------------------------------------------------
class AP_API apCLReadImageCommand : public apCLEnqueuedCommand
{
public:
    apCLReadImageCommand();
    apCLReadImageCommand(oaCLMemHandle image, cl_bool blocking_read, const gtSize_t origin[3], const gtSize_t region[3],
                         gtSize_t row_pitch, gtSize_t slice_pitch, osProcedureAddress64 ptr, cl_uint num_events_in_wait_list,
                         const cl_event* event_wait_list, cl_event* event);
    virtual ~apCLReadImageCommand();

    // Overrides apCLEnqueuedCommand:
    virtual void commandNameAsString(gtString& commandName) const;
    virtual void getRelatedSrcMemHandles(gtVector<oaCLMemHandle>& srcMemHandles) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    oaCLMemHandle readImage() const {return _image;};
    cl_bool isBlockingRead() const {return _blocking_read;};
    gtSize_t readDataOriginX() const {return _origin[0];};
    gtSize_t readDataOriginY() const {return _origin[1];};
    gtSize_t readDataOriginZ() const {return _origin[2];};
    gtSize_t readDataRegionX() const {return _region[0];};
    gtSize_t readDataRegionY() const {return _region[1];};
    gtSize_t readDataRegionZ() const {return _region[2];};
    gtSize_t hostDataRowPitch() const {return _row_pitch;};
    gtSize_t hostDataSlicePitch() const {return _slice_pitch;};
    osProcedureAddress64 hostDataPointer() const {return _ptr;};

    // Amount of pixels written:
    gtSize_t amountOfPixels();

private:
    oaCLMemHandle _image;
    cl_bool _blocking_read;
    gtSize_t _origin[3];
    gtSize_t _region[3];
    gtSize_t _row_pitch;
    gtSize_t _slice_pitch;
    osProcedureAddress64 _ptr;
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLReleaseGLObjectsCommand : public apCLEnqueuedCommand
// General Description: A class used to represent a command added to a queue by a
//                      clEnqueueReleaseGLObjects function call
// Author:  AMD Developer Tools Team
// Creation Date:       1/12/2009
// ----------------------------------------------------------------------------------
class AP_API apCLReleaseGLObjectsCommand : public apCLEnqueuedCommand
{
public:
    apCLReleaseGLObjectsCommand();
    apCLReleaseGLObjectsCommand(cl_uint num_objects, const cl_mem* mem_objects, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
    virtual ~apCLReleaseGLObjectsCommand();

    // Overrides apCLEnqueuedCommand:
    virtual void commandNameAsString(gtString& commandName) const;
    virtual void getRelatedSrcMemHandles(gtVector<oaCLMemHandle>& srcMemHandles) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    const gtVector<oaCLMemHandle>& memObjects() const {return _mem_objects;};

private:
    gtVector<oaCLMemHandle> _mem_objects;
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLTaskCommand : public apCLEnqueuedCommand
// General Description: A class used to represent a command added to a queue by a
//                      clEnqueueTask function call
// Author:  AMD Developer Tools Team
// Creation Date:       1/12/2009
// ----------------------------------------------------------------------------------
class AP_API apCLTaskCommand : public apCLEnqueuedCommand
{
public:
    apCLTaskCommand();
    apCLTaskCommand(oaCLKernelHandle kernel, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
    virtual ~apCLTaskCommand();

    // Overrides apCLEnqueuedCommand:
    virtual void commandNameAsString(gtString& commandName) const;
    virtual void getRelatedKernelHandles(gtVector<oaCLKernelHandle>& kernelHandles) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    oaCLKernelHandle kernelHandle() const {return _kernel;};

private:
    oaCLKernelHandle _kernel;
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLUnmapMemObjectCommand : public apCLEnqueuedCommand
// General Description: A class used to represent a command added to a queue by a
//                      clEnqueueUnmapMemObject function call
// Author:  AMD Developer Tools Team
// Creation Date:       1/12/2009
// ----------------------------------------------------------------------------------
class AP_API apCLUnmapMemObjectCommand : public apCLEnqueuedCommand
{
public:
    apCLUnmapMemObjectCommand();
    apCLUnmapMemObjectCommand(oaCLMemHandle memobj, osProcedureAddress64 mapped_ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
    virtual ~apCLUnmapMemObjectCommand();

    // Overrides apCLEnqueuedCommand:
    virtual void commandNameAsString(gtString& commandName) const;
    virtual void getRelatedSrcMemHandles(gtVector<oaCLMemHandle>& srcMemHandles) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    oaCLMemHandle mappedMemoryObject() const {return _memobj;};
    osProcedureAddress64 mappedPointer() const {return _mapped_ptr;};

private:
    oaCLMemHandle _memobj;
    osProcedureAddress64 _mapped_ptr;
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLWaitForEventsCommand : public apCLEnqueuedCommand
// General Description: A class used to represent a command added to a queue by a
//                      clEnqueueWaitForEvents function call
// Author:  AMD Developer Tools Team
// Creation Date:       1/12/2009
// ----------------------------------------------------------------------------------
class AP_API apCLWaitForEventsCommand : public apCLEnqueuedCommand
{
public:
    apCLWaitForEventsCommand();
    apCLWaitForEventsCommand(cl_uint num_events, const cl_event* event_list);
    virtual ~apCLWaitForEventsCommand();

    // Overrides apCLEnqueuedCommand:
    virtual void commandNameAsString(gtString& commandName) const;
    virtual bool eventParameterAvailable() const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

private:
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLWriteBufferCommand : public apCLEnqueuedCommand
// General Description: A class used to represent a command added to a queue by a
//                      clEnqueueWriteBuffer function call
// Author:  AMD Developer Tools Team
// Creation Date:       1/12/2009
// ----------------------------------------------------------------------------------
class AP_API apCLWriteBufferCommand : public apCLEnqueuedCommand
{
public:
    apCLWriteBufferCommand();
    apCLWriteBufferCommand(oaCLMemHandle buffer, cl_bool blocking_write, gtSize_t offset, gtSize_t cb, osProcedureAddress64 ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
    virtual ~apCLWriteBufferCommand();

    // Overrides apCLEnqueuedCommand:
    virtual void commandNameAsString(gtString& commandName) const;
    virtual void getRelatedDstMemHandles(gtVector<oaCLMemHandle>& dstMemHandles) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    oaCLMemHandle writtenBuffer() const {return _buffer;};
    cl_bool isBlockingWrite() const {return _blocking_write;};
    gtSize_t writtenDataOffset() const {return _offset;};
    gtSize_t writtenDataBytes() const {return _cb;};
    osProcedureAddress64 hostDataPointer() const {return _ptr;};

private:
    oaCLMemHandle _buffer;
    cl_bool _blocking_write;
    gtSize_t _offset;
    gtSize_t _cb;
    osProcedureAddress64 _ptr;
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLWriteBufferRectCommand : public apCLEnqueuedCommand
// General Description: A class used to represent a command added to a queue by a
//                      clEnqueueWriteBufferRect function call
// Author:  AMD Developer Tools Team
// Creation Date:       26/12/2010
// ----------------------------------------------------------------------------------
class AP_API apCLWriteBufferRectCommand : public apCLEnqueuedCommand
{
public:
    apCLWriteBufferRectCommand();
    apCLWriteBufferRectCommand(oaCLMemHandle buffer, cl_bool blocking_write,
                               const gtSize_t buffer_origin[3], const gtSize_t host_origin[3], const gtSize_t region[3],
                               gtSize_t buffer_row_pitch, gtSize_t buffer_slice_pitch, gtSize_t host_row_pitch, gtSize_t host_slice_pitch,
                               osProcedureAddress64 ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
    virtual ~apCLWriteBufferRectCommand();

    // Overrides apCLEnqueuedCommand:
    virtual void commandNameAsString(gtString& commandName) const;
    virtual void getRelatedDstMemHandles(gtVector<oaCLMemHandle>& dstMemHandles) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    oaCLMemHandle writtenBuffer() const {return _buffer;};
    cl_bool isBlockingWrite() const {return _blocking_write;};
    gtSize_t writtenDataOriginX() const {return _buffer_origin[0];};
    gtSize_t writtenDataOriginY() const {return _buffer_origin[1];};
    gtSize_t writtenDataOriginZ() const {return _buffer_origin[2];};
    gtSize_t hostDataOriginX() const {return _host_origin[0];};
    gtSize_t hostDataOriginY() const {return _host_origin[1];};
    gtSize_t hostDataOriginZ() const {return _host_origin[2];};
    gtSize_t writtenDataRegionX() const {return _region[0];};
    gtSize_t writtenDataRegionY() const {return _region[1];};
    gtSize_t writtenDataRegionZ() const {return _region[2];};
    gtSize_t writtenDataRowPitch() const {return _buffer_row_pitch;};
    gtSize_t writtenDataSlicePitch() const {return _buffer_slice_pitch;};
    gtSize_t hostDataRowPitch() const {return _host_row_pitch;};
    gtSize_t hostDataSlicePitch() const {return _host_slice_pitch;};
    osProcedureAddress64 hostDataPointer() const {return _ptr;};

private:
    oaCLMemHandle _buffer;
    cl_bool _blocking_write;
    gtSize_t _buffer_origin[3];
    gtSize_t _host_origin[3];
    gtSize_t _region[3];
    gtSize_t _buffer_row_pitch;
    gtSize_t _buffer_slice_pitch;
    gtSize_t _host_row_pitch;
    gtSize_t _host_slice_pitch;
    osProcedureAddress64 _ptr;
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLWriteImageCommand : public apCLEnqueuedCommand
// General Description: A class used to represent a command added to a queue by a
//                      clEnqueueWriteImage function call
// Author:  AMD Developer Tools Team
// Creation Date:       1/12/2009
// ----------------------------------------------------------------------------------
class AP_API apCLWriteImageCommand : public apCLEnqueuedCommand
{
public:
    apCLWriteImageCommand();
    apCLWriteImageCommand(oaCLMemHandle image, cl_bool blocking_write, const gtSize_t origin[3], const gtSize_t region[3], gtSize_t input_row_pitch, gtSize_t input_slice_pitch, osProcedureAddress64 ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
    virtual ~apCLWriteImageCommand();

    // Overrides apCLEnqueuedCommand:
    virtual void commandNameAsString(gtString& commandName) const;
    virtual void getRelatedDstMemHandles(gtVector<oaCLMemHandle>& dstMemHandles) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    oaCLMemHandle writtenImage() const {return _image;};
    cl_bool isBlockingWrite() const {return _blocking_write;};
    gtSize_t writtenDataOriginX() const {return _origin[0];};
    gtSize_t writtenDataOriginY() const {return _origin[1];};
    gtSize_t writtenDataOriginZ() const {return _origin[2];};
    gtSize_t writtenDataRegionX() const {return _region[0];};
    gtSize_t writtenDataRegionY() const {return _region[1];};
    gtSize_t writtenDataRegionZ() const {return _region[2];};
    gtSize_t hostDataRowPitch() const {return _input_row_pitch;};
    gtSize_t hostDataSlicePitch() const {return _input_slice_pitch;};
    osProcedureAddress64 hostDataPointer() const {return _ptr;};

    // Amount of pixels written:
    gtSize_t amountOfPixels();

private:
    oaCLMemHandle _image;
    cl_bool _blocking_write;
    gtSize_t _origin[3];
    gtSize_t _region[3];
    gtSize_t _input_row_pitch;
    gtSize_t _input_slice_pitch;
    osProcedureAddress64 _ptr;
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLQueueIdle : public apCLEnqueuedCommand
// General Description: A class used to represent a wait between commands added to a
//                      queue by a clEnqueueXXXX function calls.
// Author:  AMD Developer Tools Team
// Creation Date:       1/12/2009
// ----------------------------------------------------------------------------------
class AP_API apCLQueueIdle : public apCLEnqueuedCommand
{
public:
    apCLQueueIdle();
    virtual ~apCLQueueIdle();

    // Overrides apCLEnqueuedCommand:
    virtual void commandNameAsString(gtString& commandName) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);
};


//////////////////////////////////////////////////////////////////////////
// OpenCL 1.2 Commands                                                  //
//////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLFillBufferCommand : public apCLEnqueuedCommand
// General Description: A class used to represent a command added to a queue by a
//                      clEnqueueFillBuffer function call
// Author:  AMD Developer Tools Team
// Creation Date:       11/8/2013
// ----------------------------------------------------------------------------------
class AP_API apCLFillBufferCommand : public apCLEnqueuedCommand
{
public:
    apCLFillBufferCommand();
    apCLFillBufferCommand(oaCLMemHandle buffer, const void* pattern, gtSize_t pattern_size, gtSize_t offset, gtSize_t size,
                          cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
    virtual ~apCLFillBufferCommand();

    // Overrides apCLEnqueuedCommand:
    virtual void commandNameAsString(gtString& commandName) const;
    virtual void getRelatedDstMemHandles(gtVector<oaCLMemHandle>& dstMemHandles) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    oaCLMemHandle filledBuffer() const {return m_filledBuffer;};
    void filledPattern(const gtUByte*& pattern, gtSize_t& patternSize) const {pattern = m_filledPattern; patternSize = m_filledPatternSize;};
    gtSize_t filledOffset() const {return m_filledOffset;};
    gtSize_t filledSize() const {return m_filledSize;};

private:
    oaCLMemHandle m_filledBuffer;
    gtUByte* m_filledPattern;
    gtSize_t m_filledPatternSize;
    gtSize_t m_filledOffset;
    gtSize_t m_filledSize;
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLFillImageCommand : public apCLEnqueuedCommand
// General Description: A class used to represent a command added to a queue by a
//                      clEnqueueFillImage function call
// Author:  AMD Developer Tools Team
// Creation Date:       11/8/2013
// ----------------------------------------------------------------------------------
class AP_API apCLFillImageCommand : public apCLEnqueuedCommand
{
public:
    apCLFillImageCommand();
    apCLFillImageCommand(oaCLMemHandle image, const void* fill_color, const gtSize_t origin[3], const gtSize_t region[3],
                         cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
    virtual ~apCLFillImageCommand();

    // Overrides apCLEnqueuedCommand:
    virtual void commandNameAsString(gtString& commandName) const;
    virtual void getRelatedDstMemHandles(gtVector<oaCLMemHandle>& dstMemHandles) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    oaCLMemHandle filledImage() const {return m_filledImage;};
    osProcedureAddress64 fillColor() const {return m_fillColor;};
    gtSize_t filledOriginX() const {return m_filledOrigin[0];};
    gtSize_t filledOriginY() const {return m_filledOrigin[1];};
    gtSize_t filledOriginZ() const {return m_filledOrigin[2];};
    gtSize_t filledRegionX() const {return m_filledRegion[0];};
    gtSize_t filledRegionY() const {return m_filledRegion[1];};
    gtSize_t filledRegionZ() const {return m_filledRegion[2];};

private:
    oaCLMemHandle m_filledImage;
    osProcedureAddress64 m_fillColor;
    gtSize_t m_filledOrigin[3];
    gtSize_t m_filledRegion[3];
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLMigrateMemObjectsCommand : public apCLEnqueuedCommand
// General Description: A class used to represent a command added to a queue by a
//                      clEnqueueMigrateMemObjects function call
// Author:  AMD Developer Tools Team
// Creation Date:       11/8/2013
// ----------------------------------------------------------------------------------
class AP_API apCLMigrateMemObjectsCommand : public apCLEnqueuedCommand
{
public:
    apCLMigrateMemObjectsCommand();
    apCLMigrateMemObjectsCommand(cl_uint num_mem_objects, const cl_mem* mem_objects, cl_mem_migration_flags flags,
                                 cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
    virtual ~apCLMigrateMemObjectsCommand();

    // Overrides apCLEnqueuedCommand:
    virtual void commandNameAsString(gtString& commandName) const;
    virtual void getRelatedSrcMemHandles(gtVector<oaCLMemHandle>& srcMemHandles) const;
    virtual void getRelatedDstMemHandles(gtVector<oaCLMemHandle>& dstMemHandles) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    const gtVector<oaCLMemHandle>& migratedMemObjects() const {return m_migratedMemObjects;};
    cl_mem_migration_flags migrationFlags() const {return m_migrationFlags;};

private:
    gtVector<oaCLMemHandle> m_migratedMemObjects;
    cl_mem_migration_flags m_migrationFlags;
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLMarkerWithWaitListCommand : public apCLEnqueuedCommand
// General Description: A class used to represent a command added to a queue by a
//                      clEnqueueMarkerWithWaitList function call
// Author:  AMD Developer Tools Team
// Creation Date:       11/8/2013
// ----------------------------------------------------------------------------------
class AP_API apCLMarkerWithWaitListCommand : public apCLEnqueuedCommand
{
public:
    apCLMarkerWithWaitListCommand();
    apCLMarkerWithWaitListCommand(cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
    virtual ~apCLMarkerWithWaitListCommand();

    // Overrides apCLEnqueuedCommand:
    virtual void commandNameAsString(gtString& commandName) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLBarrierWithWaitListCommand : public apCLEnqueuedCommand
// General Description: A class used to represent a command added to a queue by a
//                      clBarrierWithWaitList function call
// Author:  AMD Developer Tools Team
// Creation Date:       11/8/2013
// ----------------------------------------------------------------------------------
class AP_API apCLBarrierWithWaitListCommand : public apCLEnqueuedCommand
{
public:
    apCLBarrierWithWaitListCommand();
    apCLBarrierWithWaitListCommand(cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
    virtual ~apCLBarrierWithWaitListCommand();

    // Overrides apCLEnqueuedCommand:
    virtual void commandNameAsString(gtString& commandName) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);
};


//////////////////////////////////////////////////////////////////////////
// OpenCL 2.0 Commands                                                  //
//////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLSVMFreeCommand : public apCLEnqueuedCommand
// General Description: A class used to represent a command added to a queue by a
//                      clEnqueueSVMFreeCommand function call
// Author:  AMD Developer Tools Team
// Creation Date:       15/12/2013
// ----------------------------------------------------------------------------------
class AP_API apCLSVMFreeCommand : public apCLEnqueuedCommand
{
public:
    typedef void (CL_CALLBACK* apSVMFreeCallback)(cl_command_queue queue, cl_uint num_svm_pointers, void* svm_pointers[], void* user_data);

public:
    apCLSVMFreeCommand();
    apCLSVMFreeCommand(cl_uint num_svm_pointers, void* svm_pointers[], apSVMFreeCallback pfn_free_func, void* user_data, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
    virtual ~apCLSVMFreeCommand();

    // Overrides apCLEnqueuedCommand:
    virtual void commandNameAsString(gtString& commandName) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    const gtVector<osProcedureAddress64>& svmPointers() const {return m_svmPointers;};
    osProcedureAddress64 pfnFreeFunc() const {return m_pfnFreeFunc;};
    osProcedureAddress64 userData() const {return m_userData;};

private:
    gtVector<osProcedureAddress64> m_svmPointers;
    osProcedureAddress64 m_pfnFreeFunc;
    osProcedureAddress64 m_userData;
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLSVMMemcpyCommand : public apCLEnqueuedCommand
// General Description: A class used to represent a command added to a queue by a
//                      clEnqueueSVMMemcpyCommand function call
// Author:  AMD Developer Tools Team
// Creation Date:       15/12/2013
// ----------------------------------------------------------------------------------
class AP_API apCLSVMMemcpyCommand : public apCLEnqueuedCommand
{
public:
    apCLSVMMemcpyCommand();
    apCLSVMMemcpyCommand(cl_bool blocking_copy, void* dst_ptr, const void* src_ptr, gtSize_t size, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
    virtual ~apCLSVMMemcpyCommand();

    // Overrides apCLEnqueuedCommand:
    virtual void commandNameAsString(gtString& commandName) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    cl_bool isBlockingCopy() const {return m_blockingCopy;};
    osProcedureAddress64 destinationPointer() const {return m_dstPointer;};
    osProcedureAddress64 sourcePointer() const {return m_srcPointer;};
    gtSize_t copiedDataSize() const {return m_size;};

private:
    cl_bool m_blockingCopy;
    osProcedureAddress64 m_dstPointer;
    osProcedureAddress64 m_srcPointer;
    gtSize_t m_size;
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLSVMMemFillCommand : public apCLEnqueuedCommand
// General Description: A class used to represent a command added to a queue by a
//                      clEnqueueSVMMemFillCommand function call
// Author:  AMD Developer Tools Team
// Creation Date:       15/12/2013
// ----------------------------------------------------------------------------------
class AP_API apCLSVMMemFillCommand : public apCLEnqueuedCommand
{
public:
    apCLSVMMemFillCommand();
    apCLSVMMemFillCommand(void* svm_ptr, const void* pattern, gtSize_t pattern_size, gtSize_t size, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
    virtual ~apCLSVMMemFillCommand();

    // Overrides apCLEnqueuedCommand:
    virtual void commandNameAsString(gtString& commandName) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    osProcedureAddress64 filledSVMPointer() const {return m_svmPtr;};
    void filledPattern(const gtUByte*& pattern, gtSize_t& patternSize) const {pattern = m_filledPattern; patternSize = m_filledPatternSize;};
    gtSize_t filledDataSize() const {return m_size;};

private:
    osProcedureAddress64 m_svmPtr;
    gtUByte* m_filledPattern;
    gtSize_t m_filledPatternSize;
    gtSize_t m_size;
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLSVMMapCommand : public apCLEnqueuedCommand
// General Description: A class used to represent a command added to a queue by a
//                      clEnqueueSVMMapCommand function call
// Author:  AMD Developer Tools Team
// Creation Date:       15/12/2013
// ----------------------------------------------------------------------------------
class AP_API apCLSVMMapCommand : public apCLEnqueuedCommand
{
public:
    apCLSVMMapCommand();
    apCLSVMMapCommand(cl_bool blocking_map, cl_map_flags flags, void* svm_ptr, gtSize_t size, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
    virtual ~apCLSVMMapCommand();

    // Overrides apCLEnqueuedCommand:
    virtual void commandNameAsString(gtString& commandName) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    cl_bool isBlockingMap() const {return m_blockingMap;};
    cl_map_flags mapFlags() const {return m_flags;};
    osProcedureAddress64 mappedSVMPointer() const {return m_svmPtr;};
    gtSize_t mappedDataSize() const {return m_size;};

private:
    cl_bool m_blockingMap;
    cl_map_flags m_flags;
    osProcedureAddress64 m_svmPtr;
    gtSize_t m_size;
};

// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLSVMUnmapCommand : public apCLEnqueuedCommand
// General Description: A class used to represent a command added to a queue by a
//                      clEnqueueSVMUnmapCommand function call
// Author:  AMD Developer Tools Team
// Creation Date:       15/12/2013
// ----------------------------------------------------------------------------------
class AP_API apCLSVMUnmapCommand : public apCLEnqueuedCommand
{
public:
    apCLSVMUnmapCommand();
    apCLSVMUnmapCommand(void* svm_ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* event);
    virtual ~apCLSVMUnmapCommand();

    // Overrides apCLEnqueuedCommand:
    virtual void commandNameAsString(gtString& commandName) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    osProcedureAddress64 unmappedSVMPointer() const {return m_svmPtr;};

private:
    osProcedureAddress64 m_svmPtr;
};

#endif //__APCLENQUEUEDCOMMANDS_H

