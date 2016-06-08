//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLSync.h
///
//==================================================================================

//------------------------------ apGLSync.h ------------------------------

#ifndef __APGLSYNC
#define __APGLSYNC

// Infra:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/apAllocatedObject.h>

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLSync : public apAllocatedObject
//
// General Description:
//   Represents an OpenGL sync object.
//   See GL_ARB_sync extension documentation for more details.
// Author:  AMD Developer Tools Team
// Creation Date:        28/10/2009
// ----------------------------------------------------------------------------------
class AP_API apGLSync : public apAllocatedObject
{
public:
    // Self functions:
    apGLSync();
    apGLSync(const apGLSync& other);
    virtual ~apGLSync();
    apGLSync& operator=(const apGLSync& other);

    // Sync ID (CodeXL-assigned name):
    int syncID() const {return _syncID;};
    void setSyncID(int id) {_syncID = id;};

    // Sync OpenGL handle:
    oaGLSyncHandle syncHandle() const {return _syncHandle;};
    void setSyncHandle(oaGLSyncHandle sync) {_syncHandle = sync;};

    // Sync condition:
    GLenum syncCodition() const {return _syncCondition;}
    void setSyncCodition(GLenum condition) {_syncCondition = condition;}

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

private:

    // The OpenGL sync handle:
    int _syncID;
    oaGLSyncHandle _syncHandle;
    GLenum _syncCondition;
};


#endif  // __APGLSYNC
