//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apMemoryLeakEvent.h
///
//==================================================================================

//------------------------------ apMemoryLeakEvent.h ------------------------------

#ifndef __APMEMORYLEAKEVENT_H
#define __APMEMORYLEAKEVENT_H

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>

// Foreward declarations:
class gtString;

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apMemoryLeakEvent
// General Description: This event is sent from the memory viewer to the debugged
//                      process events view when a memory leak occurs
// Author:  AMD Developer Tools Team
// Creation Date:        2/11/2008
// ----------------------------------------------------------------------------------
class AP_API apMemoryLeakEvent : public apEvent
{
public:
    apMemoryLeakEvent();
    virtual ~apMemoryLeakEvent();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

    enum apMemoryLeakType
    {
        AP_INDEPENDENT_GL_ALLOCATED_OBJECT_LEAK,// The application was terminated before all the OpenGL memory objects were deleted.
        AP_INDEPENDENT_CL_ALLOCATED_OBJECT_LEAK,// The application was terminated before all the OpenCL memory objects were deleted.
        AP_GL_CONTEXT_ALLOCATED_OBJECT_LEAK,    // A render context was deleted before all its allocated objects were.
        AP_CL_CONTEXT_ALLOCATED_OBJECT_LEAK,    // A compute context was deleted before all its allocated objects were.
        AP_CL_PROGRAM_ALLOCATED_OBJECT_LEAK,    // A computation program was deleted before all its sub-objects (kernels) were.
    };

    struct apLeakedSyncObjectID
    {
        int _syncID;
        oaGLSyncHandle _syncHandle;
    };

    // Leak type:
    void setLeakType(apMemoryLeakType leakType);
    apMemoryLeakType leakType() const {return _leakType;};

    //////////////////////////////////////////////////////////////////////////
    // if this is a AP_INDEPENDENT_GL_ALLOCATED_OBJECT_LEAK:
    // Returns the list of leaking render contexts as a string:
    bool leakingRenderContextsAsString(gtString& contextsList) const;

    // Returns the list of leaking render contexts as a list:
    bool leakingRenderContexts(gtVector<unsigned int>& listOfRenderLeakingContext) const;

    // The number of leaking Render Contexts:
    int numberOfLeakingRenderContexts() const;

    // Add a leaking RC:
    void addLeakingRenderContext(unsigned int contextID);

    // Returns the list of leaking PBuffers as a string
    bool leakingPBuffersAsString(gtString& buffersList) const;
    bool leakingPBuffers(gtVector<unsigned int>& leakingPBuffers) const;

    // The number of leaking PBuffers:
    int numberOfLeakingPBuffers() const;

    // Add a leaking PB:
    void addLeakingPBuffer(unsigned int bufferID);

    // Add a leaking sync object:
    void addLeakingSyncObject(apLeakedSyncObjectID syncObjectID);

    // Returns the list of leaking Sync Objects as a string
    bool leakingSyncObjectsAsString(gtString& buffersList) const;
    bool leakingSyncObjects(gtVector<apLeakedSyncObjectID>& leakingSyncObjects) const;

    // The number of leaking sync objects:
    int numberOfLeakingSyncObjects() const;

    //////////////////////////////////////////////////////////////////////////
    // if this is a AP_INDEPENDENT_CL_ALLOCATED_OBJECT_LEAK:
    // Returns the list of leaking compute contexts as a string:
    bool leakingComputeContextsAsString(gtString& contextsList) const;

    // Returns the list of leaking compute contexts as a list:
    bool leakingComputeContexts(gtVector<unsigned int>& listOfLeakingComputeContexts) const;

    // The number of leaking compute contexts:
    int numberOfLeakingComputeContexts() const;

    // Add a leaking compute context:
    void addLeakingComputeContext(unsigned int contextID);



    //////////////////////////////////////////////////////////////////////////
    // if this is a AP_GL_CONTEXT_ALLOCATED_OBJECT_LEAK:
    // The number of each kind of leaking objects
    bool leakingAllocatedGLObjects(int& leakTex, int& leakSmp, int& leakRBO, int& leakFBO, int& leakVBO, int& leakProg, int& leakShad, int& leakPipe, int& leakList) const;
    bool setLeakingGLAllocatedObjects(int leakTex, int leakSmp, int leakRBO, int leakFBO, int leakVBO, int leakProg, int leakShad, int leakPipe, int leakList);

    // The ID of the render context which was about to be deleted:
    unsigned int leakingObjectsRenderContextID() const;
    bool setLeakingObjectsRenderContextID(unsigned int contextID);

    //////////////////////////////////////////////////////////////////////////
    // if this is a AP_CL_CONTEXT_ALLOCATED_OBJECT_LEAK:
    // The number of each kind of leaking objects
    bool leakingAllocatedCLObjects(int& leakQue, int& leakEve, int& leakBuf, int& leakImg, int& leakSmp, int& leakPrg) const;
    bool setLeakingCLAllocatedObjects(int leakQue, int leakEve, int leakBuf, int leakImg, int leakSmp, int leakPrg);

    // The ID of the compute context which was about to be deleted:
    unsigned int leakingObjectsComputeContextID() const;
    bool setLeakingObjectsComputeContextID(unsigned int contextID);

    //////////////////////////////////////////////////////////////////////////
    // if this is a AP_CL_PROGRAM_ALLOCATED_OBJECT_LEAK:
    // The number of each kind of leaking objects
    bool leakingProgramAllocatedCLObjects(int& leakKer) const;
    bool setLeakingProgramAllocatedCLObjects(int leakKer);

    // The ID of the computation program which was about to be deleted:
    void leakingObjectsComputationProgramID(unsigned int& contextID, unsigned int& programID) const;
    bool setLeakingObjectsComputationProgramID(unsigned int contextID, unsigned int programID);

    // Does memory leak exist:
    bool memoryLeakExists() const;

    // Convert to string:
    bool asString(gtString& memoryLeakStr) const;

    // Memory leak size:
    gtUInt64 memoryLeakSize() const {return _memoryLeakSize;};
    void addMemoryLeakSize(gtUInt64 leakSize) {_memoryLeakSize += leakSize;};

private:
    bool independentGLObjectMemoryLeakAsString(gtString& memoryLeakStr) const;
    bool independentCLObjectMemoryLeakAsString(gtString& memoryLeakStr) const;
    bool openGLContextMemoryLeakAsString(gtString& memoryLeakStr) const;
    bool openCLContextMemoryLeakAsString(gtString& memoryLeakStr) const;
    bool openCLProgramMemoryLeakAsString(gtString& memoryLeakStr) const;

private:
    // What kind of memory leak is this?
    apMemoryLeakType _leakType;

    // The Leaking Render Contexts (either the undeleted contexts in the case of AP_INDEPENDENT_GL_ALLOCATED_OBJECT_LEAK or
    // the deleted context in the case of AP_GL_CONTEXT_ALLOCATED_OBJECT_LEAK):
    gtVector<unsigned int> m_leakingGLContexts;

    // A list of leaking PBuffers
    gtVector<unsigned int> _leakingPBuffers;

    // A list of leaking sync objects
    gtVector<apLeakedSyncObjectID> _leakingSyncObjects;

    // The Leaking Compute Contexts (either the undeleted contexts in the case of AP_INDEPENDENT_CL_ALLOCATED_OBJECT_LEAK or
    // the deleted context in the case of AP_CL_CONTEXT_ALLOCATED_OBJECT_LEAK):
    gtVector<unsigned int> m_leakingCLContexts;

    // The index of the about-to-be-deleted object, if it is not an independant object;
    unsigned int _leakingObjectsContainerIndex;

    // How many leaks do we have of each type?

    // GL context objects:
    int _numberOfLeakingTextures;
    int m_numberOfLeakingGLSamplers;
    int _numberOfLeakingRBOs;
    int _numberOfLeakingFBOs;
    int _numberOfLeakingVBOs;
    int _numberOfLeakingShadingPrograms;
    int _numberOfLeakingShaders;
    int m_numberOfLeakingPipelines;
    int _numberOfLeakingDisplayLists;

    // CL context objects:
    int _numberOfLeakingCommandQueues;
    int m_numberOfLeakingEvents;
    int _numberOfLeakingCLBuffers;
    int _numberOfLeakingImages;
    int _numberOfLeakingCLSamplers;
    int _numberOfLeakingComputationPrograms;

    // CL program objects:
    int _numberOfLeakingKernels;

    // Memory leak size:
    gtUInt64 _memoryLeakSize;
};

#endif //__APMEMORYLEAKEVENT_H

