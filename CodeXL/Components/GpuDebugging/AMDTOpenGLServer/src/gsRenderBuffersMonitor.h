//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsRenderBuffersMonitor.h
///
//==================================================================================

//------------------------------ gsRenderBuffersMonitor.h ------------------------------

#ifndef __GSRENDERBUFFERSMONITOR
#define __GSRENDERBUFFERSMONITOR

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTAPIClasses/Include/apFileType.h>
#include <AMDTAPIClasses/Include/apGLRenderBuffer.h>

// Local:
#include <src/gsRenderBuffersMonitor.h>
#include <src/gsTextureUnitMonitor.h>


// ----------------------------------------------------------------------------------
// Class Name:           gsRenderBuffersMonitor
// General Description:  Monitors Render buffer Objects.
// Author:      Sigal Algranaty
// Date:        26/5/2008
// ---------------------------------------------------------------------------
class gsRenderBuffersMonitor
{
public:
    gsRenderBuffersMonitor(int spyContextId);
    ~gsRenderBuffersMonitor();

    // On event functions:
    void onFirstTimeContextMadeCurrent();
    void onContextDeletion();
    void onRenderBufferObjectsGeneration(GLsizei amountOfGeneratedRenderBuffers, GLuint* renderBuffersNames);
    void onRenderBufferObjectsDeletion(GLsizei amountOfDeletedRenderBuffers, const GLuint* renderBuffersNames);

    // Render buffer data:
    bool updateRenderBufferRawData(apGLRenderBuffer* pRenderBufferObject, GLuint currentlyActiveFBO);
    void updateContextDataSnapshot();
    void clearContextDataSnapshot();

    // Get data functions:
    int amountOfRenderBufferObjects() const { return _amountOfRenderBufferObjects; };
    apGLRenderBuffer* getRenderBufferObjectDetails(GLuint renderBufferName) const;
    bool getRenderBufferObjectName(int renderBufferObjIndex, GLuint& renderBufferName) const;

    // Memory:
    bool calculateBuffersMemorySize(gtUInt64& buffersMemorySize) const ;

private:
    apGLRenderBuffer* createRenderBufferObjectMonitor(GLuint renderBufferObjectName);
    int  getRenderBufferObjMonitorIndex(GLuint renderBufferName) const;
    void generateRenderBufferFilePath(GLuint bufferName, osFilePath& bufferFilePath) const;

    // Utilities for update raw data:
    bool activateRenderBufferFBO(apGLRenderBuffer* pRenderBufferObject, GLuint currentlyActiveFBO, bool& needToRestoreFBO);
    bool restoreCurretlyActiveFBO(apGLRenderBuffer* pRenderBufferObject, GLuint currentlyActiveFBO, bool needToRestoreFBO);

    // Do not allow use of the = operator for this class. Use reference or pointer transferral instead
    gsRenderBuffersMonitor& operator=(const gsRenderBuffersMonitor& otherMonitor);
    gsRenderBuffersMonitor(const gsRenderBuffersMonitor& otherMonitor);

private:
    // The Spy id of my monitored render context:
    int _spyContextId;

    // Holds the amount of allocated texture objects:
    int _amountOfRenderBufferObjects;

    // Hold the parameters of render buffers that reside in this render context:
    gtVector<apGLRenderBuffer*> _renderBuffers;

    // Contains _renderBuffers array indices that are free
    // (belonged to render buffers that were deleted)
    gtVector<int> _freeRenderBufferIndices;

    // Maps render buffer OpenGL name to the render buffers vector index:
    gtMap<GLuint, int> _renderBufferOpenGLNameToIndex;

    // Checks if the functions we need to use from the frame buffer object extension are supported:
    bool m_isOpenGL3FBOSupported;
    bool m_isFrameBufferExtSupported;

    // An FBO used for render buffer with no FBO attached:
    GLuint _dummyFBOName;

    // OpenGL 3.0 function pointers:
    PFNGLGENFRAMEBUFFERSPROC m_glGenFramebuffers;
    PFNGLBINDFRAMEBUFFERPROC m_glBindFramebuffer;
    PFNGLFRAMEBUFFERRENDERBUFFERPROC m_glFramebufferRenderbuffer;
    PFNGLDELETEFRAMEBUFFERSPROC m_glDeleteFramebuffers;

    // GL_frame_buffer_EXT function pointers:
    PFNGLGENFRAMEBUFFERSEXTPROC m_glGenFramebuffersEXT;
    PFNGLBINDFRAMEBUFFEREXTPROC m_glBindFramebufferEXT;
    PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC m_glFramebufferRenderbufferEXT;
    PFNGLDELETEFRAMEBUFFERSEXTPROC m_glDeleteFramebuffersEXT;
};


#endif  // __GSRENDERBUFFERSMONITOR
