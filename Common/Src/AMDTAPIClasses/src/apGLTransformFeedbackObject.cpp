//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLTransformFeedbackObject.cpp
///
//==================================================================================

// -----------------------------   apGLTransformFeedbackObject.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <AMDTAPIClasses/Include/apGLTransformFeedbackObject.h>

// ---------------------------------------------------------------------------
// Name:        apGLTransformFeedbackObject::apGLTransformFeedbackObject
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        30/4/2014
// ---------------------------------------------------------------------------
apGLTransformFeedbackObject::apGLTransformFeedbackObject(GLuint name)
    : apAllocatedObject(), m_name(name), m_status(AP_TRANSFORM_FEEDBACK_UNINITIALIZED), m_mode(GL_NONE)
{
}


// ---------------------------------------------------------------------------
// Name:        apGLTransformFeedbackObject::apGLTransformFeedbackObject
// Description: Copy constructor
// Arguments: other - The other object from which I am copied.
// Author:  AMD Developer Tools Team
// Date:        30/4/2014
// ---------------------------------------------------------------------------
apGLTransformFeedbackObject::apGLTransformFeedbackObject(const apGLTransformFeedbackObject& other)
{
    apGLTransformFeedbackObject::operator=(other);
}


// ---------------------------------------------------------------------------
// Name:        apGLTransformFeedbackObject::~apGLTransformFeedbackObject
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        30/4/2014
// ---------------------------------------------------------------------------
apGLTransformFeedbackObject::~apGLTransformFeedbackObject()
{
}

// ---------------------------------------------------------------------------
// Name:        apGLTransformFeedbackObject::operator=
// Description: Assignment operator
// Arguments:   other - The other object from which I am copied.
// Author:  AMD Developer Tools Team
// Date:        30/4/2014
// ---------------------------------------------------------------------------
apGLTransformFeedbackObject& apGLTransformFeedbackObject::operator=(const apGLTransformFeedbackObject& other)
{
    m_name = other.m_name;
    m_status = other.m_status;
    m_mode = other.m_mode;

    setAllocatedObjectId(other.getAllocatedObjectId(), true);

    return *this;
}

// ---------------------------------------------------------------------------
// Name:        apGLTransformFeedbackObject::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        30/4/2014
// ---------------------------------------------------------------------------
osTransferableObjectType apGLTransformFeedbackObject::type() const
{
    return OS_TOBJ_ID_GL_TRANSFORM_FEEDBACK_OBJECT;
}

// ---------------------------------------------------------------------------
// Name:        apGLTransformFeedbackObject::writeSelfIntoChannel
// Description: Writes this class into a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        30/4/2014
// ---------------------------------------------------------------------------
bool apGLTransformFeedbackObject::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    ipcChannel << (gtUInt32)m_name;
    ipcChannel << (gtInt32)m_status;
    ipcChannel << (gtUInt32)m_mode;

    apAllocatedObject::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLTransformFeedbackObject::readSelfFromChannel
// Description: Read self from a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        30/4/2014
// ---------------------------------------------------------------------------
bool apGLTransformFeedbackObject::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    gtUInt32 nameAsUInt32 = 0;
    ipcChannel >> nameAsUInt32;
    m_name = (GLenum)nameAsUInt32;

    gtInt32 statusAsInt32 = -1;
    ipcChannel >> statusAsInt32;
    m_status = (TransformFeedbackObjectStatus)statusAsInt32;

    gtUInt32 modeAsUInt32 = 0;
    ipcChannel >> modeAsUInt32;
    m_mode = (GLenum)modeAsUInt32;

    apAllocatedObject::readSelfFromChannel(ipcChannel);

    return retVal;
}


