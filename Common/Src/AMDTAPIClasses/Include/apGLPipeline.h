//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLPipeline.h
///
//==================================================================================

//------------------------------ apGLPipeline.h ------------------------------

#ifndef __APGLPIPELINE
#define __APGLPIPELINE

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/apAllocatedObject.h>


// ----------------------------------------------------------------------------------
// Class Name:          AP_API apGLPipeline : public osTransferableObject
// General Description: Represents an OpenGL pipeline object.
// Author:  AMD Developer Tools Team
// Creation Date:       30/4/2014
// ----------------------------------------------------------------------------------
class AP_API apGLPipeline : public apAllocatedObject
{
public:
    // Self functions:
    apGLPipeline(GLuint name = 0);
    apGLPipeline(const apGLPipeline& other);
    virtual ~apGLPipeline();
    apGLPipeline& operator=(const apGLPipeline& other);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    GLuint pipelineName() const { return m_name; };

    // Set whether the pipeline is bound or not.
    void setIsPipelineBound(bool isBound);

    // Sets the active shader program for the pipeline object.
    void setActiveProgram(GLuint program);

    // Sets program as the current program object for the shader stages specified by stages.
    void useProgramStages(GLbitfield stages, GLuint program);

    bool isPipelineBound() const;

    // Getters.
    GLuint getActiveProgram() const;
    GLuint getVertexShader() const;
    GLuint getGeometryShader() const;
    GLuint getFragmentShader() const;
    GLuint getComputeShader() const;
    GLuint getTessCtrlShader() const;
    GLuint getTessEvaluationShader() const;

private:

    // Pipeline binding name.
    GLuint m_name;

    // Active program name.
    GLuint m_activeProgram;

    // Name of current vertex shader.
    GLuint m_vertexShaderName;

    // Name of current geometry shader.
    GLuint m_geometryShaderName;

    // Name of current fragment shader.
    GLuint m_fragmentShaderName;

    // Name of current compute shader.
    GLuint m_computeShaderName;

    // Name of current compute shader.
    GLuint m_tessEvaluationShaderName;

    // Name of current TCS program shader.
    GLuint m_tessControlShaderName;

    // Currently not used.
    //// Validation status.
    //bool m_isValid;

    // Currently unused:
    // Indicates whether or not the pipeline object has been bound (by a call to BindProgramPipeline).
    // Note that a pipeline may be bound but not the currently active pipeline (if another pipeline is
    // bound after it).
    bool m_isBound;

};


#endif  // __APGLPIPELINE
