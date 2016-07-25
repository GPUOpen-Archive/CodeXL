//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csContextMonitor.cpp
///
//==================================================================================

//------------------------------ csContextMonitor.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/apCLContext.h>
#include <AMDTAPIClasses/Include/apFrameTerminators.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Spies Utilities:
#include <AMDTServerUtilities/Include/suAPIConnector.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>
#include <AMDTServerUtilities/Include/suIKernelDebuggingManager.h>
#include <AMDTServerUtilities/Include/suInterceptionMacros.h>

// Local:
#include <src/csAMDKernelDebuggingFunctionPointers.h>
#include <src/csContextMonitor.h>
#include <src/csDevicesMonitor.h>
#include <src/csGlobalVariables.h>
#include <src/csMonitoredFunctionPointers.h>
#include <src/csOpenCLMonitor.h>

typedef bool (* GSSHAREGLCONTEXTWITHCLCONTEXTPROC)(oaOpenGLRenderContextHandle glRenderContextOSHandle, int clContextID, int& glContextID);

static const char* stat_stubProgramSource = "\
__kernel void stubKernelFunction(__global int* dummyBuffer) \
{ \
} ";
static const char* stat_stubKernelFunctionName = "stubKernelFunction";

// ---------------------------------------------------------------------------
// Name:        csContextMonitor::csContextMonitor
// Description: Constructor.
// Arguments: contextHandle - Handle to the OpenCL context.
// Author:      Yaki Tebeka
// Date:        16/11/2009
// ---------------------------------------------------------------------------
csContextMonitor::csContextMonitor(oaCLContextHandle contextHandle, int contextId, apMonitoredFunctionId creationFunc)
    : suContextMonitor(apContextID(AP_OPENCL_CONTEXT, contextId)),
      _contextInformation(contextHandle, contextId), m_updatingContextDeletionStatus(false), _hContext(contextHandle), m_eventsMonitor(contextId), _programsAndKernelsMonitor(contextId), _samplersMonitor(contextId), _isInComputationFrame(false),
      _hStubKernelContainingProgram(OA_CL_NULL_HANDLE), _stubKernelContainingProgramBuilt(false), _hStubKernel(OA_CL_NULL_HANDLE), _hStubBuffer(OA_CL_NULL_HANDLE)
{
    _contextMinimalOpenCLVersion[0] = -1;
    _contextMinimalOpenCLVersion[1] = -1;

    // Set the textures & buffers monitor spy id:
    _texturesAndBuffersMonitor.setSpyContextId(contextId);

    // Register in the allocated objects monitor:
    su_stat_theAllocatedObjectsMonitor.registerAllocatedObject(_contextInformation);

    // Update the OpenCL context object information:
    bool rcCTXInfo = updateOpenCLContextObjectInfo();
    GT_ASSERT(rcCTXInfo);

    // Set the minimal supported version into monitors that need it:
    _texturesAndBuffersMonitor.setContextOpenCLVersion(_contextMinimalOpenCLVersion[0], _contextMinimalOpenCLVersion[1]);

    // Create a calls history logger:
    _pCallsHistoryLogger = new csCallsHistoryLogger(contextId, creationFunc, nullptr);
}


// ---------------------------------------------------------------------------
// Name:        csContextMonitor::~csContextMonitor
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        16/11/2009
// ---------------------------------------------------------------------------
csContextMonitor::~csContextMonitor()
{
}

// ---------------------------------------------------------------------------
// Name:        csContextMonitor::onContextMarkedForDeletion
// Description: Called when the context is marked for deletion
//              (calling clReleaseContext with the reference count at 1)
// Author:      Uri Shomroni
// Date:        31/3/2010
// ---------------------------------------------------------------------------
void csContextMonitor::onContextMarkedForDeletion()
{
    // Mark the context as deleted:
    _contextInformation.onContextMarkedForDeletion();

    // Clean up the stub objects:
    clearStubObjects();
}

// ---------------------------------------------------------------------------
// Name:        csContextMonitor::onFrameTerminatorCall
// Description: Is called when a frame terminator is called
// Author:      Uri Shomroni
// Date:        18/2/2010
// ---------------------------------------------------------------------------
void csContextMonitor::onFrameTerminatorCall()
{
    // Call the base class implementation:
    suContextMonitor::onFrameTerminatorCall();

    // Perform command queues frame termination:
    _commandQueuesMonitor.onFrameTerminatorCall();
}

// ---------------------------------------------------------------------------
// Name:        csContextMonitor::onProgramCreation
// Description: Is called when a program is created with source code.
// Arguments: programHandle - The newly created program handle.
//            programSourceCode - The program's source code.
// Author:      Yaki Tebeka
// Date:        16/11/2009
// ---------------------------------------------------------------------------
void csContextMonitor::onProgramCreation(cl_program programHandle, const gtASCIIString& programSourceCode)
{
    _programsAndKernelsMonitor.onProgramCreation(programHandle, programSourceCode);
}

// ---------------------------------------------------------------------------
// Name:        csContextMonitor::onProgramMarkedForDeletion
// Description: Called when the Program is marked for deletion
//              (calling clReleaseProgram with the reference count at 1)
// Author:      Uri Shomroni
// Date:        31/3/2010
// ---------------------------------------------------------------------------
void csContextMonitor::onProgramMarkedForDeletion(cl_program program)
{
    _programsAndKernelsMonitor.onProgramMarkedForDeletion(program);
}

// ---------------------------------------------------------------------------
// Name:        csContextMonitor::onKernelCreation
// Description: Is called when an OpenCL kernel is created
// Arguments: cl_program program - the kernel program
//            cl_kernel kernel - the kernel handle
//            const gtString& kernelName - the kernel name
// Author:      Sigal Algranaty
// Date:        22/11/2009
// ---------------------------------------------------------------------------
void csContextMonitor::onKernelCreation(cl_program program, cl_kernel kernel, const gtString& kernelName)
{
    _programsAndKernelsMonitor.onKernelCreation(program, kernel, kernelName);
}

// ---------------------------------------------------------------------------
// Name:        csContextMonitor::onKernelMarkedForDeletion
// Description: Called when the kernel is marked for deletion
//              (calling clReleaseKernel with the reference count at 1)
// Author:      Uri Shomroni
// Date:        31/3/2010
// ---------------------------------------------------------------------------
void csContextMonitor::onKernelMarkedForDeletion(cl_kernel kernel)
{
    _programsAndKernelsMonitor.onKernelMarkedForDeletion(kernel);
}

// ---------------------------------------------------------------------------
// Name:        csContextMonitor::onKernelArgumentSet
// Description: Called when kernel's arg_index-th argument is set
// Author:      Uri Shomroni
// Date:        18/1/2010
// ---------------------------------------------------------------------------
void csContextMonitor::onKernelArgumentSet(cl_kernel kernel, cl_uint arg_index, size_t arg_size, const void* arg_value, bool isSVMPointer)
{
    _programsAndKernelsMonitor.onKernelArgumentSet(kernel, arg_index, arg_size, arg_value, isSVMPointer);
}

// ---------------------------------------------------------------------------
// Name:        csContextMonitor::onMemObjectMarkedForDeletion
// Description: Called when the mem object is marked for deletion
//              (calling clReleaseMemObject with the reference count at 1)
// Author:      Uri Shomroni
// Date:        31/3/2010
// ---------------------------------------------------------------------------
void csContextMonitor::onMemObjectMarkedForDeletion(cl_mem memobj)
{
    _texturesAndBuffersMonitor.onMemObjectMarkedForDeletion(memobj);
}

// ---------------------------------------------------------------------------
// Name:        csContextMonitor::onSamplerCreation
// Description: Called when a sampler is created
// Author:      Uri Shomroni
// Date:        24/1/2010
// ---------------------------------------------------------------------------
void csContextMonitor::onSamplerCreation(cl_sampler samplerHandle, cl_bool normalizedCoords, cl_addressing_mode addressingMode, cl_filter_mode filterMode)
{
    _samplersMonitor.onSamplerCreation(samplerHandle, normalizedCoords, addressingMode, filterMode);
}

// ---------------------------------------------------------------------------
// Name:        csContextMonitor::onSamplerCreationWithProperties
// Description: Called when a sampler is created
// Author:      Uri Shomroni
// Date:        29/9/2014
// ---------------------------------------------------------------------------
void csContextMonitor::onSamplerCreationWithProperties(cl_sampler samplerHandle, const cl_sampler_properties* properties)
{
    _samplersMonitor.onSamplerCreationWithProperties(samplerHandle, properties);
}

// ---------------------------------------------------------------------------
// Name:        csContextMonitor::onSamplerMarkedForDeletion
// Description: Called when the sampler is marked for deletion
//              (calling clReleaseSampler with the reference count at 1)
// Author:      Uri Shomroni
// Date:        31/3/2010
// ---------------------------------------------------------------------------
void csContextMonitor::onSamplerMarkedForDeletion(cl_sampler samplerHandle)
{
    _samplersMonitor.onSamplerMarkedForDeletion(samplerHandle);
}

// ---------------------------------------------------------------------------
// Name:        csContextMonitor::checkForReleasedObjects()
// Description: Checks if the context or any of its subordinate objects have been released
// Author:      Uri Shomroni
// Date:        26/8/2013
// ---------------------------------------------------------------------------
void csContextMonitor::checkForReleasedObjects(bool checkSelf)
{
    if (!_contextInformation.wasMarkedForDeletion())
    {
        // Start by checking objects in reverse hierarchical order (events before
        // queues, kernels before programs, etc.)
        OS_OUTPUT_DEBUG_LOG(L"Checking for released events", OS_DEBUG_LOG_EXTENSIVE);
        m_eventsMonitor.checkForReleasedEvents();
        OS_OUTPUT_DEBUG_LOG(L"Checking for released queues", OS_DEBUG_LOG_EXTENSIVE);
        _commandQueuesMonitor.checkForReleasedQueues();
        OS_OUTPUT_DEBUG_LOG(L"Checking for released programs and kernels", OS_DEBUG_LOG_EXTENSIVE);
        _programsAndKernelsMonitor.checkForReleasedObjects();
        OS_OUTPUT_DEBUG_LOG(L"Checking for released memory objects", OS_DEBUG_LOG_EXTENSIVE);
        _texturesAndBuffersMonitor.checkForReleasedObjects();
        OS_OUTPUT_DEBUG_LOG(L"Checking for released samplers", OS_DEBUG_LOG_EXTENSIVE);
        _samplersMonitor.checkForReleasedSamplers();
        OS_OUTPUT_DEBUG_LOG(L"Finished checking for released subordinate objects", OS_DEBUG_LOG_EXTENSIVE);

        // This function calls checkIfContextWasDeleted, which calls updateContextDataSnapshot,
        // which calls this function.
        // Don't do that loop more than once:
        if (checkSelf && !m_updatingContextDeletionStatus)
        {
            m_updatingContextDeletionStatus = true;
            // Now check if the context itself was released:
            OS_OUTPUT_DEBUG_LOG(L"Checking for context release", OS_DEBUG_LOG_EXTENSIVE);
            csOpenCLMonitor::instance().checkIfContextWasDeleted((cl_context)_contextInformation.contextHandle());
            OS_OUTPUT_DEBUG_LOG(L"Finished checking for context release", OS_DEBUG_LOG_EXTENSIVE);
            m_updatingContextDeletionStatus = false;
        }

        OS_OUTPUT_DEBUG_LOG(L"Finished checking for released objects", OS_DEBUG_LOG_EXTENSIVE);
    }
}

// ---------------------------------------------------------------------------
// Name:        csContextMonitor::onDebuggedProcessSuspended
// Description: Called before the debugged process is suspended
// Author:      Uri Shomroni
// Date:        10/12/2009
// ---------------------------------------------------------------------------
void csContextMonitor::onDebuggedProcessSuspended()
{
    // Notify the queues monitor:
    _commandQueuesMonitor.onDebuggedProcessSuspended();
}
// ---------------------------------------------------------------------------
// Name:        csContextMonitor::onDebuggedProcessResumed
// Description: Called after the debugged process is resumed
// Author:      Uri Shomroni
// Date:        10/12/2009
// ---------------------------------------------------------------------------
void csContextMonitor::onDebuggedProcessResumed()
{
    // Notify the queues monitor:
    _commandQueuesMonitor.onDebuggedProcessResumed();
}

// ---------------------------------------------------------------------------
// Name:        csContextMonitor::updateContextDataSnapshot
// Description: Updates the context data snapshot, including release check
// Return Val:  bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        2/11/2015
// ---------------------------------------------------------------------------
bool csContextMonitor::updateContextDataSnapshot(bool sendEvents) /* override */
{
    return updateContextDataSnapshot(sendEvents, true);
}

// ---------------------------------------------------------------------------
// Name:        csContextMonitor::updateContextDataSnapshot
// Description: Updates the context data snapshot
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/11/2009
// ---------------------------------------------------------------------------
bool csContextMonitor::updateContextDataSnapshot(bool sendEvents, bool checkRelease)
{
    (void)(sendEvents); // unused
    bool retVal = false;
    OS_OUTPUT_DEBUG_LOG(L"Starting OpenCL Context Update", OS_DEBUG_LOG_EXTENSIVE);

    if (!_contextInformation.wasMarkedForDeletion())
    {
        retVal = true;
        OS_OUTPUT_DEBUG_LOG(L"Context is active. Checking for released objects", OS_DEBUG_LOG_EXTENSIVE);
        checkForReleasedObjects(checkRelease);

        OS_OUTPUT_DEBUG_LOG(L"Released object check complete.", OS_DEBUG_LOG_EXTENSIVE);

        if (!_contextInformation.wasMarkedForDeletion())
        {
            OS_OUTPUT_DEBUG_LOG(L"Context is still active. Starting Command queue update", OS_DEBUG_LOG_EXTENSIVE);
            // Update the command queues monitor:
            // NOTICE: this should be the first operation, since it perform flush to OpenCL queues,
            // and this makes the other objects be valid for monitoring:
            bool rcQueueMtr = _commandQueuesMonitor.updateContextDataSnapshot();
            GT_ASSERT(rcQueueMtr);

            OS_OUTPUT_DEBUG_LOG(L"Command queue update complete. Starting Program and Kernel update.", OS_DEBUG_LOG_EXTENSIVE);
            // Update the program and kernels data snapshot:
            bool rcProgs = _programsAndKernelsMonitor.updateContextDataSnapshot();
            GT_ASSERT(rcProgs);

            OS_OUTPUT_DEBUG_LOG(L"Program and Kernel update complete. Starting Image and Buffer update.", OS_DEBUG_LOG_EXTENSIVE);
            // Update the images and buffers data snapshot:
            bool rcImages = _texturesAndBuffersMonitor.updateContextDataSnapshot();

            OS_OUTPUT_DEBUG_LOG(L"Image and Buffer update complete. Starting Context object update.", OS_DEBUG_LOG_EXTENSIVE);
            // Update the OpenCL context object information;
            bool rcCTXInfo = updateOpenCLContextObjectInfo();
            retVal = rcQueueMtr && rcProgs && rcImages && rcCTXInfo;
        }
    }

    OS_OUTPUT_DEBUG_LOG(L"Ending OpenCL Context Update", OS_DEBUG_LOG_EXTENSIVE);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csContextMonitor::stubKernelHandle
// Description: Returns the stub kernel's handle. Creates it if this is the first
//              this function was called for this context
// Author:      Uri Shomroni
// Date:        7/12/2010
// ---------------------------------------------------------------------------
oaCLKernelHandle csContextMonitor::stubKernelHandle()
{
    oaCLKernelHandle retVal = _hStubKernel;

    if (retVal == OA_CL_NULL_HANDLE)
    {
        // If the program was not yet create it, do so now:
        if (_hStubKernelContainingProgram == OA_CL_NULL_HANDLE)
        {
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clCreateProgramWithSource);
            cl_program programHandle = cs_stat_realFunctionPointers.clCreateProgramWithSource((cl_context)_hContext, 1, &stat_stubProgramSource, NULL, NULL);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clCreateProgramWithSource);

            if (programHandle != NULL)
            {
                _hStubKernelContainingProgram = (oaCLProgramHandle)programHandle;
            }
        }

        if (_hStubKernelContainingProgram != OA_CL_NULL_HANDLE)
        {
            cl_int retCode = CL_SUCCESS;

            // If the program was not yet built, build it:
            if (!_stubKernelContainingProgramBuilt)
            {
                // Build the program for all devices:
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clBuildProgram);
                retCode = cs_stat_realFunctionPointers.clBuildProgram((cl_program)_hStubKernelContainingProgram, 0, NULL, "", NULL, NULL);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clBuildProgram);

                if (retCode == CL_SUCCESS)
                {
                    _stubKernelContainingProgramBuilt = true;
                }
            }

            if (_stubKernelContainingProgramBuilt)
            {
                // Create the kernel from the program
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clCreateKernel);
                cl_kernel kernelHandle = NULL;
                bool isCSKernelDebuggingEnabled = cs_stat_pIKernelDebuggingManager->isAMDKernelDebuggingEnabled() && (suIKernelDebuggingManager::CS_OPENCL_SOFTWARE_KERNEL_DEBUGGER == cs_stat_pIKernelDebuggingManager->kernelDebuggerType());

                if (isCSKernelDebuggingEnabled)
                {
                    // We must create the kernel with the amdclIntercept version of this function:
                    kernelHandle = cs_stat_amdKernelDebuggingFunctionPointers.amdclInterceptCreateKernel((cl_program)_hStubKernelContainingProgram, stat_stubKernelFunctionName, NULL);
                }
                else
                {
                    // Call the normal function:
                    kernelHandle = cs_stat_realFunctionPointers.clCreateKernel((cl_program)_hStubKernelContainingProgram, stat_stubKernelFunctionName, NULL);
                }

                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clCreateKernel);

                if (kernelHandle != NULL)
                {
                    // Set the kernel argument:
                    cl_mem stubBuffer = (cl_mem)stubBufferHandle();
                    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clSetKernelArg);

                    if (isCSKernelDebuggingEnabled)
                    {
                        retCode = cs_stat_amdKernelDebuggingFunctionPointers.amdclInterceptSetKernelArg(kernelHandle, 0, sizeof(cl_mem), &stubBuffer);
                    }
                    else
                    {
                        retCode = cs_stat_realFunctionPointers.clSetKernelArg(kernelHandle, 0, sizeof(cl_mem), &stubBuffer);
                    }

                    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clSetKernelArg);

                    // Set the handle into the member and return it:
                    _hStubKernel = (oaCLKernelHandle)kernelHandle;
                    retVal = _hStubKernel;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csContextMonitor::stubBufferHandle
// Description: Returns the stub buffer's handle. Creates it if this is the first
//              this function was called for this context
// Author:      Uri Shomroni
// Date:        7/12/2010
// ---------------------------------------------------------------------------
oaCLMemHandle csContextMonitor::stubBufferHandle()
{
    oaCLMemHandle retVal = _hStubBuffer;

    if (retVal == OA_CL_NULL_HANDLE)
    {
        // This is the first time this function was called, create the buffer:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clCreateBuffer);
        cl_mem bufferHandle = cs_stat_realFunctionPointers.clCreateBuffer((cl_context)_hContext, 0, 2, NULL, NULL);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clCreateBuffer);

        GT_IF_WITH_ASSERT(bufferHandle != NULL)
        {
            // Set the handle into the member and return it:
            _hStubBuffer = (oaCLMemHandle)bufferHandle;
            retVal = _hStubBuffer;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csContextMonitor::stubImageHandle
// Description: Returns the stub image handle for the format. Creates it if this is the first
//              this function was called for this context / format combination
// Author:      Uri Shomroni
// Date:        7/12/2010
// ---------------------------------------------------------------------------
oaCLMemHandle csContextMonitor::stubImageHandle(const cl_image_format& imageFormat)
{
    oaCLMemHandle retVal = OA_CL_NULL_HANDLE;

    // Find the appropriate image, if it was created yet:
    int numberOfStubImages = (int)_stubImageHandles.size();

    for (int i = 0; i < numberOfStubImages; i++)
    {
        // Compare the formats:
        const StubImageData& currentStubImageData = _stubImageHandles[i];
        const cl_image_format& currentImageFormat = currentStubImageData._imageFormat;

        if ((currentImageFormat.image_channel_data_type == imageFormat.image_channel_data_type) &&
            (currentImageFormat.image_channel_order == imageFormat.image_channel_order))
        {
            // Found the image, return it:
            retVal = currentStubImageData._hImage;
            break;
        }
    }

    if (retVal == OA_CL_NULL_HANDLE)
    {
        // This is the first time this format is used here for the context, create a new image with this format:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clCreateImage2D);
        cl_mem imageHandle = cs_stat_realFunctionPointers.clCreateImage2D((cl_context)_hContext, 0, &imageFormat, 2, 2, 0, NULL, NULL);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clCreateImage2D);

        // If we succeeded:
        if (imageHandle != OA_CL_NULL_HANDLE)
        {
            // Copy this into the vector and the return value:
            StubImageData newStubImage;
            newStubImage._imageFormat = imageFormat;
            newStubImage._hImage = (oaCLMemHandle)imageHandle;
            _stubImageHandles.push_back(newStubImage);
            retVal = (oaCLMemHandle)imageHandle;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csContextMonitor::clearStubObjects
// Description: Releases all the stub objects and clears the members
// Author:      Uri Shomroni
// Date:        7/12/2010
// ---------------------------------------------------------------------------
void csContextMonitor::clearStubObjects()
{
    // Release the stub kernel and its containing program:
    if (_hStubKernel != OA_CL_NULL_HANDLE)
    {
        // We must use the amdclIntercept API if we used it to create the kernel:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clReleaseKernel);

        if (cs_stat_pIKernelDebuggingManager->isAMDKernelDebuggingEnabled() && (suIKernelDebuggingManager::CS_OPENCL_SOFTWARE_KERNEL_DEBUGGER == cs_stat_pIKernelDebuggingManager->kernelDebuggerType()))
        {
            cs_stat_amdKernelDebuggingFunctionPointers.amdclInterceptReleaseKernel((cl_kernel)_hStubKernel);
        }
        else
        {
            cs_stat_realFunctionPointers.clReleaseKernel((cl_kernel)_hStubKernel);
        }

        _hStubKernel = OA_CL_NULL_HANDLE;
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clReleaseKernel);
    }

    if (_hStubKernelContainingProgram != OA_CL_NULL_HANDLE)
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clReleaseProgram);
        cs_stat_realFunctionPointers.clReleaseProgram((cl_program)_hStubKernelContainingProgram);
        _hStubKernelContainingProgram = OA_CL_NULL_HANDLE;
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clReleaseProgram);
    }

    _stubKernelContainingProgramBuilt = false;

    if ((_hStubBuffer != OA_CL_NULL_HANDLE) || (_stubImageHandles.size() != 0))
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clReleaseMemObject);

        // Release the stub buffer:
        if (_hStubBuffer != OA_CL_NULL_HANDLE)
        {
            cs_stat_realFunctionPointers.clReleaseMemObject((cl_mem)_hStubBuffer);
            _hStubBuffer = OA_CL_NULL_HANDLE;
        }

        // Release any stub images and clear the vector:
        int amountOfStubImages = (int)_stubImageHandles.size();

        for (int i = 0; i < amountOfStubImages; i++)
        {
            // Sanity check:
            oaCLMemHandle& currentStubImageHandle = _stubImageHandles[i]._hImage;

            if (currentStubImageHandle != OA_CL_NULL_HANDLE)
            {
                cs_stat_realFunctionPointers.clReleaseMemObject((cl_mem)currentStubImageHandle);
                currentStubImageHandle = OA_CL_NULL_HANDLE;
            }
        }

        _stubImageHandles.clear();

        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clReleaseMemObject);
    }
}

// ---------------------------------------------------------------------------
// Name:        csContextMonitor::onComputationFrameStarted
// Description: Called when the cl_gremedy_computation_frame extension function
//              clBeginComputationFrameGREMEDY is called with this context as
//              the parameter (and the context isn't already in a frame)
// Author:      Uri Shomroni
// Date:        18/2/2010
// ---------------------------------------------------------------------------
void csContextMonitor::onComputationFrameStarted()
{
    // Mark we are in a frame:
    _isInComputationFrame = true;

    // Update the statistics logger with the computation frame started:
    _callsStatisticsLogger.onComputationFrameStarted();
}

// ---------------------------------------------------------------------------
// Name:        csContextMonitor::onComputationFrameEnded
// Description: Called when the cl_gremedy_computation_frame extension function
//              clEndComputationFrameGREMEDY is called with this context as
//              the parameter (and the context is already in a frame)
// Author:      Uri Shomroni
// Date:        18/2/2010
// ---------------------------------------------------------------------------
void csContextMonitor::onComputationFrameEnded()
{
    // Mark we aren't in a frame:
    _isInComputationFrame = false;

    // If the cl_gremedy_computation_frame extension is chosen as the frame
    // terminator for OpenCL:
    unsigned int frameTerminators = suFrameTerminatorsMask();

    if ((frameTerminators & AP_CL_GREMEDY_COMPUTATION_FRAME_TERMINATORS) != 0)
    {
        // Terminate the frame for this context. Note we do not directly
        // call this class's onFrameTerminatorCall function, since the OpenCL
        // monitor version does a few more things than just notify us.
        cs_stat_openCLMonitorInstance.onFrameTerminatorCall(_hContext);
    }

    // Update the statistics logger with the computation frame ended:
    _callsStatisticsLogger.onComputationFrameEnded();
}

// ---------------------------------------------------------------------------
// Name:        csContextMonitor::updateOpenCLContextObjectInfo
// Description: Updates the logged OpenCL context object information
// Author:      Yaki Tebeka
// Date:        18/1/2010
// ---------------------------------------------------------------------------
bool csContextMonitor::updateOpenCLContextObjectInfo()
{
    bool retVal = true;

    if ((_contextInformation.APIID() != 0) && (!_contextInformation.wasMarkedForDeletion()))
    {
        // Update the context's reference count:
        OS_OUTPUT_DEBUG_LOG(L"Updating Context reference count", OS_DEBUG_LOG_EXTENSIVE);
        bool rcRefCount = updateOpenCLContextObjectReferenceCount();

        // Update context's properties:
        OS_OUTPUT_DEBUG_LOG(L"Updating Context creation properties", OS_DEBUG_LOG_EXTENSIVE);
        bool rcCreationProp = updateOpenCLContextCreationProperties();

        // Update the context's device list:
        OS_OUTPUT_DEBUG_LOG(L"Updating Context device list", OS_DEBUG_LOG_EXTENSIVE);
        bool rcDevices = updateOpenCLContextDeviceList();

        retVal = rcRefCount && rcCreationProp && rcDevices;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csContextMonitor::updateOpenCLContextObjectReferenceCount
// Description: Updates the logged OpenCL Context's reference count.
// Author:      Yaki Tebeka
// Date:        18/1/2010
// ---------------------------------------------------------------------------
bool csContextMonitor::updateOpenCLContextObjectReferenceCount()
{
    bool retVal = false;

    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetContextInfo);

    // Get the context's amount of creation properties:
    size_t referenceCount = 0;
    cl_uint rcGetRefCnt = cs_stat_realFunctionPointers.clGetContextInfo(cl_context(_hContext), CL_CONTEXT_REFERENCE_COUNT, sizeof(cl_uint), &referenceCount, NULL);
    GT_IF_WITH_ASSERT(rcGetRefCnt == CL_SUCCESS)
    {
        // Subtract 1 for the reference that the debugger adds:
        _contextInformation.setReferenceCount((gtUInt32)((referenceCount > 0) ? (referenceCount - 1) : referenceCount));
        retVal = true;

        if ((0 == referenceCount) && !contextInformation().wasMarkedForDeletion())
        {
            // If this is the first time we see the ref count be 0, update the context as marked for deletion:
            onContextMarkedForDeletion();
        }
    }

    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetContextInfo);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csContextMonitor::updateOpenCLContextGLSharedContext
// Description: Updates the logged OpenCL Context's OpenGL shared context id
// Author:      Sigal Algranaty
// Date:        21/7/2010
// ---------------------------------------------------------------------------
bool csContextMonitor::updateOpenCLContextGLSharedContext(oaOpenGLRenderContextHandle renderContextOSHandle)
{
    bool retVal = false;

    // Get the OpenGL spy, and search for the spy ID for this context:
    osProcedureAddress pProcAddress = NULL;
    suAPIConnector::instance().osGetSpyProcAddress(AP_OPENGL_API_CONNECTION, "gsShareGLContextWithCLContext", pProcAddress);
    GT_IF_WITH_ASSERT(pProcAddress != NULL)
    {
        GSSHAREGLCONTEXTWITHCLCONTEXTPROC pShareGLContextWithCLContextProcAddress = (GSSHAREGLCONTEXTWITHCLCONTEXTPROC)pProcAddress;
        GT_IF_WITH_ASSERT(pShareGLContextWithCLContextProcAddress != NULL)
        {
            // Inform the OpenGL spy with the context share:
            int openGLSpyID = 0;
            bool rc = pShareGLContextWithCLContextProcAddress(renderContextOSHandle, _contextID._contextId, openGLSpyID);
            GT_IF_WITH_ASSERT(rc)
            {
                // Set the spy ID:
                _contextInformation.setOpenGLSpyID(openGLSpyID);
                retVal = true;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csContextMonitor::updateOpenCLContextCreationProperties
// Description: Updates the logged OpenCL Context's creation properties.
// Author:      Yaki Tebeka
// Date:        18/1/2010
// ---------------------------------------------------------------------------
bool csContextMonitor::updateOpenCLContextCreationProperties()
{
    // Context creation properties cannot change. Thus, if we have them, no need to update again:
    bool retVal = (0 < _contextInformation.contextCreationProperties().amountOfProperties());

    if (!retVal)
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetContextInfo);
        size_t propertiesDataSize = 0;
        cl_uint rcGetPropsDataSize = cs_stat_realFunctionPointers.clGetContextInfo(cl_context(_hContext), CL_CONTEXT_PROPERTIES, 0, 0, &propertiesDataSize);
        GT_IF_WITH_ASSERT(rcGetPropsDataSize == CL_SUCCESS)
        {
            retVal = true;

            if (propertiesDataSize > 0)
            {
                // Allocate space to contain the device list:
                static gtVector<cl_context_properties> s_contextProperties;
                if ((s_contextProperties.size() * sizeof(cl_context_properties)) < propertiesDataSize)
                {
                    size_t newSize = (propertiesDataSize / sizeof(cl_context_properties)) + 1;
                    s_contextProperties.resize(newSize);
                }

                cl_context_properties* pContextProperties = &(s_contextProperties[0]);

                // Get the context's associated properties:
                cl_uint rcGetProperties = cs_stat_realFunctionPointers.clGetContextInfo(cl_context(_hContext), CL_CONTEXT_PROPERTIES, propertiesDataSize, pContextProperties, 0);
                GT_IF_WITH_ASSERT(rcGetProperties == CL_SUCCESS)
                {
                    // Set the context properties:
                    _contextInformation.setContextCreationProperties(pContextProperties);

                    // Get the context creation properties:
                    const apCLContextProperties& contextProperties = _contextInformation.contextCreationProperties();

                    // Check if the properties contain an OpenGL context property:
                    oaCLContextProperty openGLContextHandle = contextProperties.clPropertyValue(CL_GL_CONTEXT_KHR);

                    if (openGLContextHandle != 0)
                    {
                        bool rc = updateOpenCLContextGLSharedContext((oaOpenGLRenderContextHandle)openGLContextHandle);
                        GT_ASSERT(rc);
                    }
                }
            }
        }
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetContextInfo);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csContextMonitor::updateOpenCLContextDeviceList
// Description: Updates the logged OpenCL Context's device list.
// Author:      Yaki Tebeka
// Date:        18/1/2010
// ---------------------------------------------------------------------------
bool csContextMonitor::updateOpenCLContextDeviceList()
{
    // Context devices cannot change after creation, so if we have the device list, no need to re-update it.
    bool retVal = (0 < _contextInformation.deviceIDs().size()) && (OA_CL_NULL_HANDLE != _contextInformation.contextPlatform());

    if (!retVal)
    {
        _contextInformation.clearDeviceIDs();
        oaCLPlatformID platform = OA_CL_NULL_HANDLE;
        bool isAMDPlatform = false;

        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetContextInfo);

        // Get the context's associated devices data size:
        size_t devicesDataSize = 0;
        cl_uint rcGetDevDataSize = cs_stat_realFunctionPointers.clGetContextInfo((cl_context)_hContext, CL_CONTEXT_DEVICES, 0, 0, &devicesDataSize);
        GT_IF_WITH_ASSERT(rcGetDevDataSize == CL_SUCCESS)
        {
            retVal = true;

            if (devicesDataSize > 0)
            {
                // Allocate space to contain the device list:
                static gtVector<cl_device_id> s_contextDevices;
                if ((s_contextDevices.size() * sizeof(cl_device_id)) < devicesDataSize)
                {
                    size_t newSize = (devicesDataSize / sizeof(cl_device_id)) + 1;
                    s_contextDevices.resize(newSize);
                }

                cl_device_id* pDeviceIDs = &(s_contextDevices[0]);

                // Get the context's associated devices:
                cl_uint rcGetDevices = cs_stat_realFunctionPointers.clGetContextInfo((cl_context)_hContext, CL_CONTEXT_DEVICES, devicesDataSize, pDeviceIDs, 0);
                GT_IF_WITH_ASSERT(rcGetDevices == CL_SUCCESS)
                {
                    // Log the associated devices:
                    csOpenCLMonitor& theOpenCLMonitor = csOpenCLMonitor::instance();
                    csDevicesMonitor& theDevicesMonitor = theOpenCLMonitor.devicesMonitor();

                    size_t devicesAmount = devicesDataSize / sizeof(cl_device_id);
                    bool isFirstValidDeviceVersion = true;
                    isAMDPlatform = true;

                    for (size_t i = 0; i < devicesAmount; i++)
                    {
                        cl_device_id currDeviceOCLId = pDeviceIDs[i];
                        int deviceAPIID = theDevicesMonitor.getDeviceObjectAPIID((oaCLDeviceID)currDeviceOCLId);
                        GT_IF_WITH_ASSERT(deviceAPIID != -1)
                        {
                            _contextInformation.addDeviceId(deviceAPIID);

                            // Get the device's OpenCL version:
                            const apCLDevice* pCurrentDevice = theDevicesMonitor.getDeviceObjectDetailsByIndex(deviceAPIID);
                            GT_IF_WITH_ASSERT(pCurrentDevice != NULL)
                            {
                                // Get the platform ID:
                                oaCLPlatformID currentPlatform = pCurrentDevice->platformID();

                                if (OA_CL_NULL_HANDLE == platform)
                                {
                                    // Set it to the context:
                                    GT_IF_WITH_ASSERT(OA_CL_NULL_HANDLE != currentPlatform)
                                    {
                                        platform = currentPlatform;
                                    }
                                }
                                else // OA_CL_NULL_HANDLE != platform
                                {
                                    // All devices in the context are expected to belong to the same platform:
                                    GT_ASSERT(platform == currentPlatform);
                                }

                                // If this is the first device or this version is lower than the one we have, copy the value:
                                int currentDeviceCLVersion[2] = {pCurrentDevice->clMajorVersion(), pCurrentDevice->clMinorVersion()};

                                if (isFirstValidDeviceVersion ||
                                    (_contextMinimalOpenCLVersion[0] > currentDeviceCLVersion[0]) ||
                                    ((_contextMinimalOpenCLVersion[0] == currentDeviceCLVersion[0]) && (_contextMinimalOpenCLVersion[1] > currentDeviceCLVersion[1])))
                                {
                                    // Set the minimal version:
                                    isFirstValidDeviceVersion = false;
                                    _contextMinimalOpenCLVersion[0] = currentDeviceCLVersion[0];
                                    _contextMinimalOpenCLVersion[1] = currentDeviceCLVersion[1];
                                }
                            }
                        }
                        else
                        {
                            retVal = false;
                        }

                        isAMDPlatform = isAMDPlatform && theDevicesMonitor.isAMDDevice((oaCLDeviceID)currDeviceOCLId);
                    }
                }
                else
                {
                    retVal = false;
                }
            }
        }

        // Also update the platform id:
        _contextInformation.setContextPlatform(platform, isAMDPlatform);

        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetContextInfo);
    }

    return retVal;
}

