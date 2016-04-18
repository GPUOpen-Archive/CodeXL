//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTOpenGLHelper.cpp
///
//==================================================================================

//------------------------------ AMDTOpenGLHelper.cpp ----------------------------

#include <stdio.h>
#include <string.h>

#include <inc/AMDTOpenGLHelper.h>

#include "AMDTMisc.h"
#include "AMDTDebug.h"

// Static storage for the singleton.
AMDTOpenGLHelper* AMDTOpenGLHelper::_instance = NULL;

// ---------------------------------------------------------------------------
// Name:        AMDTOpenGLHelper::GetInstance()
// Description: Get a reference to this singleton, increasing the reference
//              count by 1.
// ---------------------------------------------------------------------------
AMDTOpenGLHelper* AMDTOpenGLHelper::GetInstance()
{
    if (!_instance)
    {
        _instance = new AMDTOpenGLHelper();
    }

    _instance->_refCounter++;
    return _instance;
}

// ---------------------------------------------------------------------------
// Name:        AMDTOpenGLHelper::ReleaseInstance()
// Description: Release reference to singleton. If there are no other
//              references, delete the singleton.
// ---------------------------------------------------------------------------
void AMDTOpenGLHelper::ReleaseInstance()
{
    if (_instance)
    {
        if (_instance->_refCounter == 1)
        {
            delete _instance;
            _instance = NULL;
        }
        else
        {
            --_instance->_refCounter;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        AMDTOpenGLHelper::AMDTOpenGLHelper
// Description: Constructor. Get function pointers for extension functions
//              and set flags for available extensions.
// ---------------------------------------------------------------------------
AMDTOpenGLHelper::AMDTOpenGLHelper() :
    // GL_AMDTEMEDY_string_marker function pointer:
    _glStringMarkerGREMEDY(NULL),

    // GL_GREMEDY_frame_terminator function pointers:
    _glFrameTerminatorGREMEDY(NULL),

    // GL_ARB_shader_objects extension functions:
    _isGL_ARB_shader_objectsSupported(false),
    _glCreateShaderObjectARB(NULL),
    _glShaderSourceARB(NULL),
    _glCompileShaderARB(NULL),
    _glGetObjectParameterivARB(NULL),
    _glCreateProgramObjectARB(NULL),
    _glAttachObjectARB(NULL),
    _glLinkProgramARB(NULL),
    _glUseProgramObjectARB(NULL),
    _glGetInfoLogARB(NULL),
    _glGetUniformLocationARB(NULL),
    _glUniform1fARB(NULL),
    _glDeleteObjectARB(NULL),
    _glDetachObjectARB(NULL),

    // GL_ARB_vertex_buffer_object extension functions:
    _isGL_ARB_vertex_buffer_objectSupported(false),
    _glGenBuffersARB(NULL),
    _glBindBufferARB(NULL),
    _glBufferDataARB(NULL),
    _glDeleteBuffersARB(NULL),
    _glBufferSubDataARB(NULL),

    // OpenGL 1.2 function pointers
    _glTexImage3D(NULL),

    // OpenGL 1.3 extension functions:
    _glActiveTexture(NULL),

    // OpenGL 1.5 extension functions:
    _isOpenGL1_5Supported(false),
    _glGenBuffers(NULL),
    _glBindBuffer(NULL),
    _glBufferData(NULL),
    _glDeleteBuffers(NULL),
    _glBufferSubData(NULL),
    _glGetBufferSubData(NULL),

    // GL_EXT_geometry_shader4 objects extension functions:
    _isGL_EXT_geometry_shader4Supported(false),
    _glProgramParameteriEXT(NULL),

    // GL_ARB_texture_float
    _isGL_ARB_texture_floatSupported(false),

    _refCounter(0),
    _ready(false),

    // Handled to the System's OpenGL Framework (used on Mac OS X only):
    _hSystemOpenGLFramework(NULL),

    // Handle to the Graphic Remedy's OpenGL server (used on Mac OS X only):
    _hGremedyOpenGLServer(NULL)
{
    // On Mac OS X:
#if defined (__APPLE__)
    // Get a handle to the System's OpenGL Framework:
    _hSystemOpenGLFramework = dlopen("/System/Library/Frameworks/OpenGL.framework/OpenGL", RTLD_LAZY | RTLD_LOCAL);

    if (_hSystemOpenGLFramework == NULL)
    {
        // If the OpenGL main library wasn't where we expected it to be, we try to find the GL library, which
        // holds all the functions we look for (and is reexported by the OpenGL main library):
        _hSystemOpenGLFramework = dlopen("/System/Library/Frameworks/OpenGL.framework/Libraries/libGL.dylib", RTLD_LAZY | RTLD_LOCAL);
    }

    // Get a handle to the Graphic Remedy OpenGL Server:
    _hGremedyOpenGLServer = dlopen("libGROpenGLServer.dylib", RTLD_LAZY | RTLD_LOCAL);
#endif

    if (initialize())
    {
        _ready = true;
    }
}

// ---------------------------------------------------------------------------
// Name:        AMDTOpenGLHelper::~AMDTOpenGLHelper
// Description: Deconstructor. Unload the OpenGL library where appropriate.
// ---------------------------------------------------------------------------
AMDTOpenGLHelper::~AMDTOpenGLHelper()
{
    // On Mac OS X:
#if defined (__APPLE__)
    // Close the OpenGL framework handle:
    if (_hSystemOpenGLFramework != NULL)
    {
        dlclose(_hSystemOpenGLFramework);
        _hSystemOpenGLFramework = NULL;
    }

    // Close the Graphic Remedy OpenGL Server handle:
    if (_hGremedyOpenGLServer != NULL)
    {
        dlclose(_hGremedyOpenGLServer);
        _hGremedyOpenGLServer = NULL;
    }

#endif
}

// ---------------------------------------------------------------------------
// Name:        AMDTOpenGLHelper::isReady()
// Description: Return true if initialization was successful - library was
//              loaded and extensions were queried.
// ---------------------------------------------------------------------------
bool AMDTOpenGLHelper::isReady() const
{
    return _ready;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::getProcAddress
// Description: Inputs an extension function name and returns its pointer
//              iff it is supported by the current render context.
// ---------------------------------------------------------------------------
oglProcedureAddress AMDTOpenGLHelper::getProcAddress(
    const char* pProcName)
{
    oglProcedureAddress retVal = NULL;

#if defined(_WIN32)
    {
        retVal = (oglProcedureAddress)wglGetProcAddress(pProcName);
    }
#elif defined (__APPLE__)
    {
        retVal = getMacOSXExtensionFunctionAddress(pProcName);
    }
#elif defined(__linux__)
    {
        retVal = glXGetProcAddress((const GLubyte*)pProcName);
    }
#endif

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        AMDTOpenGLHelper::isExtensionSupported
// Description: Inputs an extension name and returns true iff the extension is
//              supported in the render context that is the current context of
//              the calling thread.
// ---------------------------------------------------------------------------
bool AMDTOpenGLHelper::isExtensionSupported(
    char* extensionName,
    const char* extensionsString)
{
    bool retVal = false;

    // Search for extensionName in the extensions string.
    // (The use of strstr() is not sufficient because extension names can be prefixes of
    // other extension names).
    char* pCurrentPos = (char*)extensionsString;
    char* pEndPos;
    int extensionNameLen = strlen(extensionName);
    pEndPos = pCurrentPos + strlen(pCurrentPos);

    while (pCurrentPos < pEndPos)
    {
        int n = strcspn(pCurrentPos, " ");

        if ((extensionNameLen == n) && (strncmp(extensionName, pCurrentPos, n) == 0))
        {
            retVal = true;
            break;
        }

        pCurrentPos += (n + 1);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        AMDTOpenGLHelper::initialize()
// Description: Get function pointers for extensions that we are interested in
//              and set flags for those extensions that are supported by this
//              platform.
// ---------------------------------------------------------------------------
bool AMDTOpenGLHelper::initialize()
{
    // Get the extensions string of the this thread current render context:
    const char* extensionsString = (const char*)glGetString(GL_EXTENSIONS);

    if (extensionsString != NULL)
    {
        // If the GL_GREMEDY_string_marker extension is supported:
        if (isExtensionSupported((char*)"GL_GREMEDY_string_marker", extensionsString))
        {
            // Get a pointer to the glStringMarkerGREMEDY function:
            _glStringMarkerGREMEDY = (PFNGLSTRINGMARKERGREMEDYPROC)getProcAddress("glStringMarkerGREMEDY");
        }

        // If the GL_GREMEDY_frame_terminator extension is supported:
        if (isExtensionSupported((char*)"GL_GREMEDY_frame_terminator", extensionsString))
        {
            // Get a pointer to the glStringMarkerGREMEDY function:
            _glFrameTerminatorGREMEDY = (PFNGLFRAMETERMINATORGREMEDYPROC)getProcAddress("glFrameTerminatorGREMEDY");
        }

        // If GL_ARB_shader_objects is supported:
        if (isExtensionSupported((char*)"GL_ARB_shader_objects", extensionsString))
        {
            _glCreateShaderObjectARB = (PFNGLCREATESHADEROBJECTARBPROC)getProcAddress("glCreateShaderObjectARB");
            _glShaderSourceARB = (PFNGLSHADERSOURCEARBPROC)getProcAddress("glShaderSourceARB");
            _glCompileShaderARB = (PFNGLCOMPILESHADERARBPROC)getProcAddress("glCompileShaderARB");
            _glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC)getProcAddress("glGetObjectParameterivARB");
            _glCreateProgramObjectARB = (PFNGLCREATEPROGRAMOBJECTARBPROC)getProcAddress("glCreateProgramObjectARB");
            _glAttachObjectARB = (PFNGLATTACHOBJECTARBPROC)getProcAddress("glAttachObjectARB");
            _glLinkProgramARB = (PFNGLLINKPROGRAMARBPROC)getProcAddress("glLinkProgramARB");
            _glUseProgramObjectARB = (PFNGLUSEPROGRAMOBJECTARBPROC)getProcAddress("glUseProgramObjectARB");
            _glGetInfoLogARB = (PFNGLGETINFOLOGARBPROC)getProcAddress("glGetInfoLogARB");
            _glGetUniformLocationARB = (PFNGLGETUNIFORMLOCATIONARBPROC)getProcAddress("glGetUniformLocationARB");
            _glUniform1fARB = (PFNGLUNIFORM1FARBPROC)getProcAddress("glUniform1fARB");
            _glDeleteObjectARB = (PFNGLDELETEOBJECTARBPROC)getProcAddress("glDeleteObjectARB");
            _glDetachObjectARB = (PFNGLDETACHOBJECTARBPROC)getProcAddress("glDetachObjectARB");

            // We consider the extension as supported only when we managed to get all its
            // function pointers:
            _isGL_ARB_shader_objectsSupported =
                ((_glCreateShaderObjectARB != NULL) && (_glShaderSourceARB != NULL) &&
                 (_glCompileShaderARB != NULL) && (_glGetObjectParameterivARB != NULL) &&
                 (_glCreateProgramObjectARB != NULL) && (_glAttachObjectARB != NULL) &&
                 (_glLinkProgramARB != NULL) && (_glUseProgramObjectARB != NULL) &&
                 (_glGetInfoLogARB != NULL) && (_glGetUniformLocationARB != NULL) &&
                 (_glUniform1fARB != NULL) && (_glDeleteObjectARB != NULL) &&
                 (_glDetachObjectARB != NULL));
        }

        // If GL_EXT_geometry_shader4 is supported
        if (isExtensionSupported((char*)"GL_EXT_geometry_shader4", extensionsString))
        {
            _glProgramParameteriEXT = (PFNGLPROGRAMPARAMETERIEXTPROC)getProcAddress("glProgramParameteriEXT");

            // Only ProgramParameteriEXT is used, so we consider the extension supported if we get its function pointer.
            _isGL_EXT_geometry_shader4Supported = (_glProgramParameteriEXT != NULL);
        }

        // If GL_ARB_vertex_buffer_object is supported:
        if (isExtensionSupported((char*)"GL_ARB_vertex_buffer_object", extensionsString))
        {
            _glGenBuffersARB = (PFNGLGENBUFFERSPROC)getProcAddress("glGenBuffersARB");
            _glBindBufferARB = (PFNGLBINDBUFFERPROC)getProcAddress("glBindBufferARB");
            _glBufferDataARB = (PFNGLBUFFERDATAARBPROC)getProcAddress("glBufferDataARB");
            _glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC)getProcAddress("glDeleteBuffersARB");
            _glBufferSubDataARB = (PFNGLBUFFERSUBDATAARBPROC)getProcAddress("glBufferSubDataARB");

            // We consider the extension as supported only when we managed to get all its
            // function pointers:
            _isGL_ARB_vertex_buffer_objectSupported = ((_glGenBuffersARB != NULL) && (_glBindBufferARB != NULL) &&
                                                       (_glBufferDataARB != NULL) && (_glDeleteBuffersARB != NULL) && (_glBufferSubDataARB != NULL));
        }

        // Get OpenGL 1.2 function pointers:
        _glTexImage3D = (PFNGLTEXIMAGE3DPROC)getProcAddress("glTexImage3D");

        // Get OpenGL 1.3 function pointers:
        _glActiveTexture = (PFNGLACTIVETEXTUREPROC)getProcAddress("glActiveTexture");

        // Get OpenGL 1.5 function pointers:
        _glGenBuffers = (PFNGLGENBUFFERSPROC)getProcAddress("glGenBuffers");
        _glBindBuffer = (PFNGLBINDBUFFERPROC)getProcAddress("glBindBuffer");
        _glBufferData = (PFNGLBUFFERDATAPROC)getProcAddress("glBufferData");
        _glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)getProcAddress("glDeleteBuffers");
        _glBufferSubData = (PFNGLBUFFERSUBDATAPROC)getProcAddress("glBufferSubData");
        _glGetBufferSubData = (PFNGLGETBUFFERSUBDATAPROC)getProcAddress("glGetBufferSubData");

        // We consider the OpenGL 1.5 as supported only when we managed to get all the functions we need:
        _isOpenGL1_5Supported = ((_glGenBuffers != NULL) && (_glBindBuffer != NULL) &&
                                 (_glBufferData != NULL) && (_glDeleteBuffers != NULL) && (_glBufferSubData != NULL));

        if (isExtensionSupported((char*)"GL_ARB_texture_float", extensionsString))
        {
            _isGL_ARB_texture_floatSupported = true;
        }
    }

    return true;
}

// Mac OS X code only:
#if defined (__APPLE__)


// ---------------------------------------------------------------------------
// Name:        AMDTOpenGLHelper::getMacOSXExtensionFunctionAddress()
// Description: Inputs an OpenGL extension function name and returns its address (or NULL in
//              case of failure).
// ---------------------------------------------------------------------------
oglProcedureAddress AMDTOpenGLHelper::getMacOSXExtensionFunctionAddress(
    const char* pProcName)
{
    oglProcedureAddress retVal = NULL;

#if defined (__APPLE__)
    {
        // Sanity check:
        if (pProcName != NULL)
        {
            // If this is a Graphic Remedy extension:
            if (strstr(pProcName, "GREMEDY") != NULL)
            {
                // Get the function pointer out of the Graphic Remedy OpenGL server:
                retVal = (oglProcedureAddress)dlsym(_hGremedyOpenGLServer , pProcName);
            }
            else
            {
                // Get the function pointer out of the System's OpenGL framework:
                retVal = (oglProcedureAddress)dlsym(_hSystemOpenGLFramework , pProcName);
            }
        }
    }
#endif

    return retVal;
}

#endif // Mac OS X code only
