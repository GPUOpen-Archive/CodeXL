//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLTransformFeedbackObject.h
///
//==================================================================================

//------------------------------ apGLTransformFeedbackObject.h ------------------------------

#ifndef __APGLTRANSFORMFEEDBACKOBJECT
#define __APGLTRANSFORMFEEDBACKOBJECT

// OpenGL
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/apAllocatedObject.h>


// ----------------------------------------------------------------------------------
// Class Name:          AP_API apGLTransformFeedbackObject : public osTransferableObject
// General Description: Represents an OpenGL transform feedback object.
// Author:  AMD Developer Tools Team
// Creation Date:       30/4/2014
// ----------------------------------------------------------------------------------
class AP_API apGLTransformFeedbackObject : public apAllocatedObject
{
public:
    enum TransformFeedbackObjectStatus
    {
        AP_TRANSFORM_FEEDBACK_UNINITIALIZED,
        AP_TRANSFORM_FEEDBACK_UNBOUND,
        AP_TRANSFORM_FEEDBACK_STOPPED,
        AP_TRANSFORM_FEEDBACK_RUNNING,
        AP_TRANSFORM_FEEDBACK_PAUSED,
        AP_NUMBER_OF_TRANSFORM_FEEDBACK_STATUSES,
    };

public:
    // Self functions:
    apGLTransformFeedbackObject(GLuint name = 0);
    apGLTransformFeedbackObject(const apGLTransformFeedbackObject& other);
    virtual ~apGLTransformFeedbackObject();
    apGLTransformFeedbackObject& operator=(const apGLTransformFeedbackObject& other);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    GLuint transformFeedbackName() const { return m_name; };
    TransformFeedbackObjectStatus transformFeedbackStatus() const { return m_status; };
    void setTransformFeedbackStatus(TransformFeedbackObjectStatus status) { m_status = status; };
    GLenum transformFeedbackMode() const { return m_mode; };
    void setTransformFeedbackMode(GLenum mode) { m_mode = mode; };

private:
    GLuint m_name;
    TransformFeedbackObjectStatus m_status;
    GLenum m_mode;
};


#endif  // __APGLTRANSFORMFEEDBACKOBJECT
