//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apContextID.cpp
///
//==================================================================================

//------------------------------ apContextID.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <AMDTAPIClasses/Include/apContextID.h>
#include <AMDTAPIClasses/Include/apStringConstants.h>


// ---------------------------------------------------------------------------
// Name:        apContextID::apContextID
// Description: Constructor.
// Author:  AMD Developer Tools Team
// Date:        15/11/2009
// ---------------------------------------------------------------------------
apContextID::apContextID(apContextType type , int contextId)
    : _contextType(type), _contextId(contextId)
{
    if (_contextId == 0)
    {
        _contextType = AP_NULL_CONTEXT;
    }
}


// ---------------------------------------------------------------------------
// Name:        apContextID::~apContextID
// Description: Destructor
// Return Val:
// Author:  AMD Developer Tools Team
// Date:        15/11/2009
// ---------------------------------------------------------------------------
apContextID::~apContextID()
{

}

// ---------------------------------------------------------------------------
// Name:        apContextID::type
// Description: Returns my transferable object type
// Author:  AMD Developer Tools Team
// Date:        15/11/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apContextID::type() const
{
    return OS_TOBJ_ID_CONTEXT_ID;
}


// ---------------------------------------------------------------------------
// Name:        apContextID::writeSelfIntoChannel
// Description: Write myself into an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        15/11/2009
// ---------------------------------------------------------------------------
bool apContextID::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtInt32)_contextType;
    ipcChannel << (gtInt32)_contextId;
    return true;
}

// ---------------------------------------------------------------------------
// Name:        apContextID::readSelfFromChannel
// Description: Read myself from an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        15/11/2009
// ---------------------------------------------------------------------------
bool apContextID::readSelfFromChannel(osChannel& ipcChannel)
{
    gtInt32 contextTypeAsInt32 = 0;
    ipcChannel >> contextTypeAsInt32;
    _contextType = (apContextType)contextTypeAsInt32;

    gtInt32 contextIdAsInt32 = 0;
    ipcChannel >> contextIdAsInt32;
    _contextId = (int)contextIdAsInt32;

    return true;
}

// ---------------------------------------------------------------------------
// Name:        apContextID::operator==
// Description: Context id comparison operator
// Arguments: const apContextID& other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        15/11/2009
// ---------------------------------------------------------------------------
bool apContextID::operator==(const apContextID& other) const
{
    bool retVal = false;

    if ((_contextType == other._contextType) && (_contextId == other._contextId))
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apContextID::operator==
// Description: Context id comparison operator
// Arguments: const apContextID& other
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        15/11/2009
// ---------------------------------------------------------------------------
bool apContextID::operator!=(const apContextID& other) const
{
    bool retVal = false;

    if ((_contextType != other._contextType) || (_contextId != other._contextId))
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apContextID::operator=
// Description: Copy other content into self.
// Author:  AMD Developer Tools Team
// Date:        15/11/2009
// ---------------------------------------------------------------------------
apContextID& apContextID::operator=(const apContextID& other)
{
    _contextType = other._contextType;
    _contextId = other._contextId;
    return *this;
}

// ---------------------------------------------------------------------------
// Name:        apContextID::toString
// Description: Convert a context id to a string
// Arguments:   gtString& contextStr
//              bool wasDeleted - was the context deleted
//              int glSharingContextId - the shared OpenGL context id
//              int clSharedContextID - the shared OpenCL context id
// Return Val:  void
// Author:  AMD Developer Tools Team
// Date:        17/11/2009
// ---------------------------------------------------------------------------
void apContextID::toString(gtString& contextStr, bool wasDeleted, int glSharingContextId, int clSharedContextID) const
{
    // Empty the string:
    contextStr.makeEmpty();

    switch (_contextType)
    {
        case AP_OPENGL_CONTEXT:
        {
            contextStr.appendFormattedString(AP_STR_GL AP_STR_Space);
            break;
        }

        case AP_OPENCL_CONTEXT:
        {
            contextStr.appendFormattedString(AP_STR_CL AP_STR_Space);
            break;
        }

        case AP_NULL_CONTEXT:
            break;
    }

    if (isDefault())
    {
        contextStr.append(AP_STR_NoContext);
    }
    else
    {
        // Build the context base name:
        contextStr.appendFormattedString(AP_STR_ContextName, _contextId);

        // Build the shared contexts string:
        gtString sharedContextsStr;

        if (glSharingContextId > 0)
        {
            sharedContextsStr.appendFormattedString(AP_STR_ContextGLName, glSharingContextId);
        }

        if (clSharedContextID > 0)
        {
            sharedContextsStr.appendFormattedString(AP_STR_ContextCLName, clSharedContextID);
        }

        if (!sharedContextsStr.isEmpty())
        {
            // If the context is shared, add the shared string:
            contextStr.appendFormattedString(AP_STR_ContextSharedPoxtfix, sharedContextsStr.asCharArray());
        }

        if (wasDeleted)
        {
            contextStr.append(AP_STR_ContextDeletedPostfix);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        apContextID::operator<
// Description: Context id comparison
// Arguments:   other - Other context to which I am compared to.
// Author:  AMD Developer Tools Team
// Date:        24/11/2009
// ---------------------------------------------------------------------------
bool apContextID::operator<(const apContextID& other) const
{

    bool retVal = false;

    if ((int)_contextType < (int)other._contextType)
    {
        retVal = true;
    }
    else if ((int)_contextType == (int)other._contextType)
    {
        retVal = (_contextId < other._contextId);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apContextTypeToString
// Description: Translates a context type to a human readable string.
// Author:  AMD Developer Tools Team
// Date:        22/3/2010
// ---------------------------------------------------------------------------
void apContextTypeToString(apContextType contextType, gtString& contextTypeAsStr)
{
    if (contextType == AP_NULL_CONTEXT)
    {
        contextTypeAsStr = AP_STR_ContextTypeNULL;
    }
    else if (contextType == AP_OPENGL_CONTEXT)
    {
        contextTypeAsStr = AP_STR_ContextTypeOpenGL;
    }
    else if (contextType == AP_OPENCL_CONTEXT)
    {
        contextTypeAsStr = AP_STR_ContextTypeOpenCL;
    }
    else
    {
        GT_ASSERT(false);
        contextTypeAsStr = AP_STR_ContextTypeUnknown;
    }
}


