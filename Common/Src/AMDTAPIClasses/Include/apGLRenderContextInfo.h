//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLRenderContextInfo.h
///
//==================================================================================

//------------------------------ apGLRenderContextInfo.h ------------------------------

#ifndef __APGLRENDERCONTEXTINFO_H
#define __APGLRENDERCONTEXTINFO_H

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/apAllocatedObject.h>

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLRenderContextInfo
// General Description:
//  Represents an OpenGL Render Context, and transfers its details.
//  Currently holds:
//      The context's internal (spy) ID
//      The context's object sharing context ID if there is one or -1 if there isn't
//      The context's active pixel format ID
// Author:  AMD Developer Tools Team
// Creation Date:        16/6/2008
// ----------------------------------------------------------------------------------
class AP_API apGLRenderContextInfo : public apAllocatedObject
{
public:
    apGLRenderContextInfo();

    int spyID() const { return _spyID; };
    int sharingContextID() const { return _sharingContextID; };
    void setSpyID(const int contextID) { _spyID = contextID; };
    void setSharingContextID(const int sharedID) { _sharingContextID = sharedID; };

    void setOpenCLSpyID(int openCLContextID) { _openCLContextID = openCLContextID;}
    int openCLSpyID() const {return _openCLContextID;}

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const { return OS_TOBJ_ID_GL_RENDER_CONTEXT_INFO; };
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

private:
    // The context's internal ID:
    int _spyID;

    // The context's object sharing context ID:
    int _sharingContextID;

    // The OpenCL context ID:
    int _openCLContextID;
};


#endif //__APGLRENDERCONTEXTINFO_H
