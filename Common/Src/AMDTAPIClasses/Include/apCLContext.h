//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCLContext.h
///
//==================================================================================

//------------------------------ apCLContext.h ------------------------------

#ifndef __APCLCONTEXT_H
#define __APCLCONTEXT_H

// Infra:
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/apAllocatedObject.h>
#include <AMDTAPIClasses/Include/apCLContextProperties.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCLContext
// General Description:  Holds the information of an OpenCL Compute Context.
// Author:  AMD Developer Tools Team
// Creation Date:        7/11/2009
// ----------------------------------------------------------------------------------
class AP_API apCLContext : public apAllocatedObject
{
public:
    apCLContext(oaCLContextHandle contextHandle = OA_CL_NULL_HANDLE, gtInt32 APIID = -1);
    apCLContext(const apCLContext& other);
    virtual ~apCLContext();
    apCLContext& operator=(const apCLContext& other);

    oaCLContextHandle contextHandle() const {return _contextHandle;};
    bool wasMarkedForDeletion() const {return _wasMarkedForDeletion;};
    void onContextMarkedForDeletion() {_wasMarkedForDeletion = true;};
    gtInt32 APIID() const { return _APIID; };

    gtUInt32 referenceCount() const { return _referenceCount; };
    void setReferenceCount(gtUInt32 refCount) { _referenceCount = refCount; };

    const apCLContextProperties& contextCreationProperties() const { return _contextCreationProperties; };
    void setContextCreationProperties(cl_context_properties* pPropertiesList);

    const gtVector<int>& deviceIDs() const { return _deviceIDs; };
    void clearDeviceIDs() { _deviceIDs.clear(); };
    void addDeviceId(gtInt32 deviceId) { _deviceIDs.push_back(deviceId); };

    oaCLPlatformID contextPlatform() const { return m_platform; };
    bool isAMDPlatform() const { return m_isAMDPlatform; };
    void setContextPlatform(oaCLPlatformID platform, bool isAMD) { m_platform = platform; m_isAMDPlatform = isAMD; };

    // cl_gremedy_object_naming:
    const gtString& contextName() const {return _contextName;};
    void setContextName(const gtString& name) {_contextName = name;};

    // OpenGL - OpenCL interoperability:
    void setOpenGLSpyID(int spyID) {_openGLSpyID = spyID;};
    int openGLSpyID() const {return _openGLSpyID;};

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const { return OS_TOBJ_ID_CL_CONTEXT; };
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

private:
    // The context handle:
    oaCLContextHandle _contextHandle;

    // Was this context marked for deletion?
    bool _wasMarkedForDeletion;

    // The API's ID for this  compute context:
    gtInt32 _APIID;

    // The context's reference count:
    gtUInt32 _referenceCount;

    // Properties specified while the context was created:
    apCLContextProperties _contextCreationProperties;

    // The devices associated which this compute context.
    // (this vector contains the OpenCL Server's id of these devices)
    gtVector<gtInt32> _deviceIDs;

    // The Context platform:
    oaCLPlatformID m_platform;
    bool m_isAMDPlatform;

    // cl_gremedy_object_naming:
    gtString _contextName;

    // OpenGL - OpenCL interoperability:
    int _openGLSpyID;
};


#endif //__APCLCONTEXT_H

