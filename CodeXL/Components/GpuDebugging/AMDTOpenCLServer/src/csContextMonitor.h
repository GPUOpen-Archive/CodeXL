//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csContextMonitor.h
///
//==================================================================================

//------------------------------ csContextMonitor.h ------------------------------

#ifndef __CSCONTEXTMONITOR_H
#define __CSCONTEXTMONITOR_H

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTOSAPIWrappers/Include/oaOpenCLIncludes.h>
#include <AMDTAPIClasses/Include/apCLContext.h>
#include <AMDTAPIClasses/Include/apCLProgram.h>

// Spies utilities:
#include <AMDTServerUtilities/Include/suContextMonitor.h>

// Local:
#include <src/csCommandQueuesMonitor.h>
#include <src/csEventsMonitor.h>
#include <src/csProgramsAndKernelsMonitor.h>
#include <src/csSamplersMonitor.h>
#include <src/csImagesAndBuffersMonitor.h>
#include <src/csCallsHistoryLogger.h>


// ----------------------------------------------------------------------------------
// Class Name:           csContextMonitor
// General Description:
//   Monitors an OpenCL Context.
//
// Author:               Yaki Tebeka
// Creation Date:        16/11/2009
// ----------------------------------------------------------------------------------
class csContextMonitor : public suContextMonitor
{
public:
    csContextMonitor(oaCLContextHandle contextHandle, int spyContextId, apMonitoredFunctionId creationFunc);
    virtual ~csContextMonitor();

    // Events:
    void onContextMarkedForDeletion();
    void onFrameTerminatorCall();
    void onProgramCreation(cl_program programHandle, const gtASCIIString& programSourceCode);
    void onProgramMarkedForDeletion(cl_program program);
    void onKernelCreation(cl_program program, cl_kernel kernel, const gtString& kernelName);
    void onKernelMarkedForDeletion(cl_kernel kernel);
    void onKernelArgumentSet(cl_kernel kernel, cl_uint arg_index, size_t arg_size, const void* arg_value, bool isSVMPointer);
    void onMemObjectMarkedForDeletion(cl_mem memobj);
    void onSamplerCreation(cl_sampler samplerHandle, cl_bool normalizedCoords, cl_addressing_mode addressingMode, cl_filter_mode filterMode);
    void onSamplerCreationWithProperties(cl_sampler samplerHandle, const cl_sampler_properties* properties);
    void onSamplerMarkedForDeletion(cl_sampler samplerHandle);

    // Reference count checking:
    void checkForReleasedObjects(bool checkSelf);

    void onDebuggedProcessSuspended();
    void onDebuggedProcessResumed();

    virtual bool updateContextDataSnapshot(bool sendEvents = false) override; // Overrides suContextMonitor
    bool updateContextDataSnapshot(bool sendEvents, bool checkRelease);       // csContextMonitor-only variant

    oaCLContextHandle contextHandle() const { return _hContext; };
    void markHandleAsReused() {_hContext = OA_CL_NULL_HANDLE;};
    const apCLContext& contextInformation() const { return _contextInformation; };

    // Monitors:
    const csCommandQueuesMonitor& commandQueuesMonitor() const { return _commandQueuesMonitor; };
    csCommandQueuesMonitor& commandQueuesMonitor() { return _commandQueuesMonitor; };

    const csEventsMonitor& eventsMonitor() const { return m_eventsMonitor; };
    csEventsMonitor& eventsMonitor() { return m_eventsMonitor; };

    const csProgramsAndKernelsMonitor& programsAndKernelsMonitor() const { return _programsAndKernelsMonitor; };
    csProgramsAndKernelsMonitor& programsAndKernelsMonitor() { return _programsAndKernelsMonitor; };

    const csImagesAndBuffersMonitor& imagesAndBuffersMonitor() const { return _texturesAndBuffersMonitor; };
    csImagesAndBuffersMonitor& imagesAndBuffersMonitor() { return _texturesAndBuffersMonitor; };

    const csSamplersMonitor& samplersMonitor() const {return _samplersMonitor;};
    csSamplersMonitor& samplersMonitor() {return _samplersMonitor;};

    // Context Id:
    int spyId() const {return (int)(_contextInformation.APIID());};
    int openGLSharedContextSpyId() const {return (int)(_contextInformation.openGLSpyID());};

    // Forced modes:
    oaCLKernelHandle stubKernelHandle();
    oaCLMemHandle stubBufferHandle();
    oaCLMemHandle stubImageHandle(const cl_image_format& imageFormat);
    void clearStubObjects();

    // cl_gremedy_computation_frame:
    bool isInComputationFrame() const {return _isInComputationFrame;};
    void onComputationFrameStarted();
    void onComputationFrameEnded();

    // cl_gremedy_object_naming:
    const gtString& contextName() const {return _contextInformation.contextName();};
    void setContextName(const gtString& name) {_contextInformation.setContextName(name);};

private:
    struct StubImageData
    {
        cl_image_format _imageFormat;
        oaCLMemHandle _hImage;
    };

private:
    bool updateOpenCLContextObjectInfo();
    bool updateOpenCLContextObjectReferenceCount();
    bool updateOpenCLContextCreationProperties();
    bool updateOpenCLContextDeviceList();
    bool updateOpenCLContextGLSharedContext(oaOpenGLRenderContextHandle renderContextOSHandle);

private:
    // Holds context information, exported by the API:
    apCLContext _contextInformation;
    bool m_updatingContextDeletionStatus;

    // The Context's Minimal OpenCL version:
    int _contextMinimalOpenCLVersion[2];

    // Handle to the OpenCL context:
    oaCLContextHandle _hContext;

    // Command Queues monitor:
    csCommandQueuesMonitor _commandQueuesMonitor;

    // Events monitor:
    csEventsMonitor m_eventsMonitor;

    // Programs and Kernels monitor:
    csProgramsAndKernelsMonitor _programsAndKernelsMonitor;

    // Monitors buffers and textures:
    csImagesAndBuffersMonitor _texturesAndBuffersMonitor;

    // Samplers monitor:
    csSamplersMonitor _samplersMonitor;

    // cl_gremedy_computation_frame:
    bool _isInComputationFrame;

    // Stub objects:
    oaCLProgramHandle _hStubKernelContainingProgram;
    bool _stubKernelContainingProgramBuilt;
    oaCLKernelHandle _hStubKernel;
    oaCLMemHandle _hStubBuffer;
    gtVector<StubImageData> _stubImageHandles;
};


#endif //__CSCONTEXTMONITOR_H

