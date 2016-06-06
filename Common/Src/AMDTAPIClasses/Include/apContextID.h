//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apContextID.h
///
//==================================================================================

//------------------------------ apContextID.h ------------------------------

#ifndef __APCONTEXTID
#define __APCONTEXTID

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTOSWrappers/Include/osTransferableObject.h>
#include <AMDTAPIClasses/Include/apCounterType.h>


// The context type:
enum apContextType
{
    AP_NULL_CONTEXT,        // Null context
    AP_OPENGL_CONTEXT,      // OpenGL context
    AP_OPENCL_CONTEXT       // OpenCL context
};


// ----------------------------------------------------------------------------------
// Class Name:           apContextID : public osTransferableObject
// General Description: Holds a context id (id + context type).
// Author:  AMD Developer Tools Team
// Creation Date:        20/2/2008
// ----------------------------------------------------------------------------------
class AP_API apContextID : public osTransferableObject
{
public:
    // Constructor / destructor:
    apContextID(apContextType type = AP_OPENGL_CONTEXT, int contextId = 0);
    virtual ~apContextID();

    // Translate the context id to a string;
    void toString(gtString& contextStr, bool wasDeleted = false, int glSharingContextId = -1, int clSharedContextID = -1) const ;

    // Check if this context id is valid:
    bool isValid() const { return (_contextId >= 0);};

    // Validity for certain APIs:
    bool isDefault() const { return ((_contextType == AP_NULL_CONTEXT) || (_contextId == 0));};
    bool isOpenGLContext() const { return ((_contextType == AP_OPENGL_CONTEXT) && (_contextId > 0));};
    bool isOpenCLContext() const { return ((_contextType == AP_OPENCL_CONTEXT) && (_contextId > 0));};

    // To enable the usage of apContextId as a map key:
    bool operator<(const apContextID& other) const;

    // The context type (OpenGL/OpenCL):
    apContextType _contextType;

    // The context id:
    int _contextId;

    bool operator==(const apContextID& other)const ;
    bool operator!=(const apContextID& other)const ;
    apContextID& operator=(const apContextID& other);

public:

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);
};

AP_API void apContextTypeToString(apContextType contextType, gtString& contextTypeAsStr);

#endif  // __APCOUNTERID
