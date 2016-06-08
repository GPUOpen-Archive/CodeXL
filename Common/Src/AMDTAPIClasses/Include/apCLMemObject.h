//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCLMemObject.h
///
//==================================================================================

//------------------------------ apCLMemObject.h ------------------------------

#ifndef __APCLMEMOBJECT_H
#define __APCLMEMOBJECT_H

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local
#include <AMDTAPIClasses/Include/apAllocatedObject.h>
#include <AMDTAPIClasses/Include/apOpenCLParameters.h>


// ----------------------------------------------------------------------------------
// Class Name:          AP_API apCLMemObject : public apAllocatedObject
// General Description: Represents an object that has a cl_mem handle, i.e. an OpenCL
//                      texture or buffer.
//                      Parent class for apCLBuffer and apCLTexture.
// Author:  AMD Developer Tools Team
// Creation Date:       17/2/2010
// ----------------------------------------------------------------------------------
class AP_API apCLMemObject : public apAllocatedObject
{
public:
    apCLMemObject();
    virtual ~apCLMemObject();

    // Mem object CL handle:
    oaCLMemHandle memObjectHandle() const {return _memHandle;};
    void setMemObjectHandle(oaCLMemHandle memObjectHandle) {_memHandle = memObjectHandle;};

    // Deletion status:
    bool wasMarkedForDeletion() const {return _wasMarkedForDeletion;};
    void onMemObjectMarkedForDeletion() {_wasMarkedForDeletion = true;};

    // Reference count:
    gtUInt32 referenceCount() const {return _referenceCount;};
    void setReferenceCount(gtUInt32 refCount) {_referenceCount = refCount;};

    // Memory flags:
    const apCLMemFlags& memoryFlags() const {return _memoryFlags;};
    void setMemoryFlags(cl_mem_flags flags) {_memoryFlags.setFlags(flags);};

    // cl_gremedy_object_naming:
    const gtString& memObjectName() const {return _memObjectName;};
    void setMemObjectName(const gtString& name) {_memObjectName = name;};

    // Destructor callback:
    const osProcedureAddress64& destructorPfnNotify() const {return m_destructorPfnNotify;};
    const osProcedureAddress64& destructorUserData() const {return m_destructorUserData;};
    void setDestructorCallback(osProcedureAddress64 pfnNotify, osProcedureAddress64 userData) {m_destructorPfnNotify = pfnNotify; m_destructorUserData = userData;};

    // Write the first write operation status:
    bool wasFirstWriteOperationPerformed() const {return _wasFirstWriteOperationPerformed;};
    void markWriteOperationPerform() {_wasFirstWriteOperationPerformed = true;};

    // Overrides osTransferableObject:
    // do not implement "virtual osTransferableObjectType type()" const, this is a virtual class
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

protected:
    // Mem object CL handle:
    oaCLMemHandle _memHandle;

    // Was this object marked for deletion?
    bool _wasMarkedForDeletion;

    // The objects' reference count:
    gtUInt32 _referenceCount;

    // The object's memory flags:
    apCLMemFlags _memoryFlags;

    // cl_gremedy_object_naming:
    gtString _memObjectName;

    // Destructor callback:
    osProcedureAddress64 m_destructorPfnNotify;
    osProcedureAddress64 m_destructorUserData;

    // Was this memory object used in write operation:
    bool _wasFirstWriteOperationPerformed;
};

#endif //__APCLMEMOBJECT_H

