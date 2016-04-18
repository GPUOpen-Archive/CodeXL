//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTOpenGLHelper.h
///
//==================================================================================

//------------------------------ AMDTOpenGLHelper.h ------------------------------

#ifndef __AMDTOPENGLHELPER_H
#define __AMDTOPENGLHELPER_H

// Platform specific includes:
#if defined (__APPLE__)
    #include <dlfcn.h>
#elif defined(__linux__)
    #include <signal.h>
#elif defined(_WINDOWS) || defined(WIN32)
    #include <Windows.h>
#endif

// Forward decelerations:

// OpenGL:
#define GL_GLEXT_LEGACY
#if defined(__APPLE__)
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif
#include <GL/glext.h>
#include <GL/GRemedyGLExtensions.h>

// Linux specific includes:
#if defined(__linux__)
    #include <GL/glx.h>
    #include <string.h>
#endif

// Type definitions:
typedef void (*oglProcedureAddress)(void);
typedef void (APIENTRYP PFNGLACTIVETEXTUREPROC)(GLenum texture);

#if defined(__APPLE__)
    typedef void (* PFNGLGENBUFFERSPROC)(GLsizei n, GLuint* buffers);
    typedef void (* PFNGLDELETEBUFFERSPROC)(GLsizei n, const GLuint* buffers);
    typedef void (* PFNGLBINDBUFFERPROC)(GLenum target, GLuint buffer);
    typedef void (* PFNGLBUFFERDATAPROC)(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
#endif

// Local definitions

/******************************************************************************
 *
 * AMDTOpenGLHelper
 * --------------
 *
 * This is a singleton class. At present it is not thread-safe. When the
 * singleton is created, it loads the OpenGL library (in the case of MacOSX,
 * otherwise it is assumed that OpenGL has been linked with the app), imports
 * specific extension function pointers and sets flags to indicate which
 * extensions were found on this platform.
 *
 ******************************************************************************/
class AMDTOpenGLHelper
{
public:
    // GL_AMDTEMEDY_string_marker function pointer:
    PFNGLSTRINGMARKERGREMEDYPROC _glStringMarkerGREMEDY;

    // GL_GREMEDY_frame_terminator function pointers:
    PFNGLFRAMETERMINATORGREMEDYPROC _glFrameTerminatorGREMEDY;

    // GL_ARB_shader_objects extension functions:
    bool _isGL_ARB_shader_objectsSupported;
    PFNGLCREATESHADEROBJECTARBPROC _glCreateShaderObjectARB;
    PFNGLSHADERSOURCEARBPROC _glShaderSourceARB;
    PFNGLCOMPILESHADERARBPROC _glCompileShaderARB;
    PFNGLGETOBJECTPARAMETERIVARBPROC _glGetObjectParameterivARB;
    PFNGLCREATEPROGRAMOBJECTARBPROC _glCreateProgramObjectARB;
    PFNGLATTACHOBJECTARBPROC _glAttachObjectARB;
    PFNGLLINKPROGRAMARBPROC _glLinkProgramARB;
    PFNGLUSEPROGRAMOBJECTARBPROC _glUseProgramObjectARB;
    PFNGLGETINFOLOGARBPROC _glGetInfoLogARB;
    PFNGLGETUNIFORMLOCATIONARBPROC _glGetUniformLocationARB;
    PFNGLUNIFORM1FARBPROC _glUniform1fARB;
    PFNGLDELETEOBJECTARBPROC _glDeleteObjectARB;
    PFNGLDETACHOBJECTARBPROC _glDetachObjectARB;

    // GL_ARB_vertex_buffer_object extension functions:
    bool _isGL_ARB_vertex_buffer_objectSupported;
    PFNGLGENBUFFERSARBPROC _glGenBuffersARB;
    PFNGLBINDBUFFERARBPROC _glBindBufferARB;
    PFNGLBUFFERDATAARBPROC _glBufferDataARB;
    PFNGLDELETEBUFFERSARBPROC _glDeleteBuffersARB;
    PFNGLBUFFERSUBDATAARBPROC _glBufferSubDataARB;

    // OpenGL 1.2 function pointers
    PFNGLTEXIMAGE3DPROC _glTexImage3D;

    // OpenGL 1.3 extension functions:
    PFNGLACTIVETEXTUREPROC _glActiveTexture;

    // OpenGL 1.5 extension functions:
    bool _isOpenGL1_5Supported;
    PFNGLGENBUFFERSPROC _glGenBuffers;
    PFNGLBINDBUFFERPROC _glBindBuffer;
    PFNGLBUFFERDATAPROC _glBufferData;
    PFNGLDELETEBUFFERSPROC _glDeleteBuffers;
    PFNGLBUFFERSUBDATAPROC _glBufferSubData;
    PFNGLGETBUFFERSUBDATAPROC _glGetBufferSubData;

    // GL_EXT_geometry_shader4 objects extension functions:
    bool _isGL_EXT_geometry_shader4Supported;
    PFNGLPROGRAMPARAMETERIEXTPROC _glProgramParameteriEXT;

    // GL_ARB_texture_float
    bool _isGL_ARB_texture_floatSupported;

public:
    // Get a reference to the singleton. If this is the first call, the
    // singleton is created.
    static AMDTOpenGLHelper* GetInstance();

    // Remove reference to the singleton. If there are no more references,
    // the singleton is destroyed.
    void ReleaseInstance();

    // Check if the helper has initialized successfully - OpenGL library was
    // loaded, extension function pointers were queried.
    bool isReady() const;

private:
    // Make constructor/deconstructor private.
    AMDTOpenGLHelper();
    virtual ~AMDTOpenGLHelper();

    // Map all needed OpenCL functions and extensions.
    bool initialize();

    // Lookup proc address in OpenGL library
    oglProcedureAddress getProcAddress(const char* pProcName);

    // Check if an OpenGL extension is defined in an extension string.
    bool isExtensionSupported(
        char* extensionName,
        const char* extensionsString);

private:
    // Singleton instance.
    static AMDTOpenGLHelper*      _instance;

    // Number of references to this singleton.
    unsigned int                _refCounter;

    // If the OpenGL library is loaded and extensions were successfully
    // qqueried, this is set to true.
    bool                        _ready;

    // Handled to the System's OpenGL Framework (used on Mac OS X only):
    void* _hSystemOpenGLFramework;

    // Handle to the Graphic Remedy's OpenGL server (used on Mac OS X only):
    void* _hGremedyOpenGLServer;
};

#endif //__AMDTOPENGLHELPER_H
