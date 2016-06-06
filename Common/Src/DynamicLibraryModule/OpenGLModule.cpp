//==============================================================================
// Copyright (c) 2012-2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools
/// \file
/// \brief This class manages the dynamic loading of OpenGL.
//==============================================================================

#include <string>
#include <vector>
#include <iostream>
#include "OpenGLModule.h"

// fixme 64/32 bit
#if _WIN32
    const char* OpenGLModule::s_DefaultModuleName = "opengl32.dll";
#else
    const char* OpenGLModule::s_DefaultModuleName = "libGL.so";
#endif


OpenGLModule::OpenGLModule(const std::string& moduleName) : m_wglChoosePixelFormat_fn(NULL),
    m_wglCopyContext_fn(NULL),
    m_wglCreateContext_fn(NULL),
    m_wglCreateLayerContext_fn(NULL),
    m_wglDeleteContext_fn(NULL),
    m_wglDescribeLayerPlane_fn(NULL),
    m_wglDescribePixelFormat_fn(NULL),
    m_wglGetCurrentContext_fn(NULL),
    m_wglGetCurrentDC_fn(NULL),
    m_wglGetDefaultProcAddress_fn(NULL),
    m_wglGetLayerPaletteEntries_fn(NULL),
    m_wglGetPixelFormat_fn(NULL),
    m_wglGetProcAddress_fn(NULL),
    m_wglMakeCurrent_fn(NULL),
    m_wglRealizeLayerPalette_fn(NULL),
    m_wglSetLayerPaletteEntries_fn(NULL),
    m_wglSetPixelFormat_fn(NULL),
    m_wglShareLists_fn(NULL),
    m_wglSwapBuffers_fn(NULL),
    m_wglSwapLayerBuffers_fn(NULL),
    m_wglSwapMultipleBuffers_fn(NULL),
    m_wglUseFontBitmapsA_fn(NULL),
    m_wglUseFontBitmapsW_fn(NULL),
    m_wglUseFontOutlinesA_fn(NULL),
    m_wglUseFontOutlinesW_fn(NULL),
    m_glGetString_fn(NULL),
    m_Module(NULL),
    m_bIsModuleLoaded(false)
{
    if (moduleName.empty())
    {
        m_ModuleName = moduleName;
    }
    else
    {
        m_ModuleName = s_DefaultModuleName;
    }

    Initialize();


#ifndef _WIN32

    // Bug 8385 - The automatic installer of Catalyst 12.1 on CentOS6
    // installs libOpenGL.so.1, but does not install a symbolic link to libOpenGL.so
    // It does not appear to be possible to dynamically construct a vector
    // inside the namespace declaration where the dispatch table is constructed
    // in the DebugAPI code, so we have to do that here, and I was not sure how to
    // modify that code accordingly
    if ((moduleName == OpenGLModule::s_DefaultModuleName)
        && (OpenGLLoaded() == OpenGL_None))
    {
        std::vector<std::string>    vMods;

        // Generic first, followed in descending order by most-recent
        // a .2 version does not yet exist
        vMods.push_back("libOpenGL.so");
        vMods.push_back("libOpenGL.so.2");
        vMods.push_back("libOpenGL.so.1");

        m_OpenGLVersion = LoadModule(vMods);

        // But if this fails, we have no fallback...
    }

#endif
}

OpenGLModule::~OpenGLModule()
{
    UnloadModule();
}


void OpenGLModule::UnloadModule()
{
    if (m_Module != NULL)
    {
#ifdef _WIN32
        FreeLibrary(m_Module);
#else
        dlclose(m_Module);
#endif
        m_Module = NULL;
    }

    m_bIsModuleLoaded = false;
}

OpenGLModule::OpenGLVersion OpenGLModule::OpenGLLoaded()
{
    OpenGLVersion moduleLoaded = OpenGL_None;

    return moduleLoaded;
}


void OpenGLModule::Initialize()
{
#ifdef _WIN32
    m_Module = LoadLibraryA(m_ModuleName.c_str());
#else
    m_Module = dlopen(m_ModuleName.c_str(), RTLD_LAZY);
#endif

    if (m_Module != NULL)
    {
#ifdef _WIN32
        m_wglChoosePixelFormat_fn = (wglChoosePixelFormat_fn)::GetProcAddress(m_Module, "wglChoosePixelFormat");
        m_wglCopyContext_fn = (wglCopyContext_fn)::GetProcAddress(m_Module, "wglCopyContext");
        m_wglCreateContext_fn = (wglCreateContext_fn)::GetProcAddress(m_Module, "wglCreateContext");
        m_wglCreateLayerContext_fn = (wglCreateLayerContext_fn)::GetProcAddress(m_Module, "wglCreateLayerContext");
        m_wglDeleteContext_fn = (wglDeleteContext_fn)::GetProcAddress(m_Module, "wglDeleteContext");
        m_wglDescribeLayerPlane_fn = (wglDescribeLayerPlane_fn)::GetProcAddress(m_Module, "wglDescribeLayerPlane");
        m_wglDescribePixelFormat_fn = (wglDescribePixelFormat_fn)::GetProcAddress(m_Module, "wglDescribePixelFormat");
        m_wglGetCurrentContext_fn = (wglGetCurrentContext_fn)::GetProcAddress(m_Module, "wglGetCurrentContext");
        m_wglGetCurrentDC_fn = (wglGetCurrentDC_fn)::GetProcAddress(m_Module, "wglGetCurrentDC");
        m_wglGetDefaultProcAddress_fn = (wglGetDefaultProcAddress_fn)::GetProcAddress(m_Module, "wglGetDefaultProcAddress");
        m_wglGetLayerPaletteEntries_fn = (wglGetLayerPaletteEntries_fn)::GetProcAddress(m_Module, "wglGetLayerPaletteEntries");
        m_wglGetPixelFormat_fn = (wglGetPixelFormat_fn)::GetProcAddress(m_Module, "wglGetPixelFormat");
        m_wglGetProcAddress_fn = (wglGetProcAddress_fn)::GetProcAddress(m_Module, "wglGetProcAddress");
        m_wglMakeCurrent_fn = (wglMakeCurrent_fn)::GetProcAddress(m_Module, "wglMakeCurrent");
        m_wglRealizeLayerPalette_fn = (wglRealizeLayerPalette_fn)::GetProcAddress(m_Module, "wglRealizeLayerPalette");
        m_wglSetLayerPaletteEntries_fn = (wglSetLayerPaletteEntries_fn)::GetProcAddress(m_Module, "wglSetLayerPaletteEntries");
        m_wglSetPixelFormat_fn = (wglSetPixelFormat_fn)::GetProcAddress(m_Module, "wglSetPixelFormat");
        m_wglShareLists_fn = (wglShareLists_fn)::GetProcAddress(m_Module, "wglShareLists");
        m_wglSwapBuffers_fn = (wglSwapBuffers_fn)::GetProcAddress(m_Module, "wglSwapBuffers");
        m_wglSwapLayerBuffers_fn = (wglSwapLayerBuffers_fn)::GetProcAddress(m_Module, "wglSwapLayerBuffers");
        m_wglSwapMultipleBuffers_fn = (wglSwapMultipleBuffers_fn)::GetProcAddress(m_Module, "wglSwapMultipleBuffers");
        m_wglUseFontBitmapsA_fn = (wglUseFontBitmapsA_fn)::GetProcAddress(m_Module, "wglUseFontBitmapsA");
        m_wglUseFontBitmapsW_fn = (wglUseFontBitmapsW_fn)::GetProcAddress(m_Module, "wglUseFontBitmapsW");
        m_wglUseFontOutlinesA_fn = (wglUseFontOutlinesA_fn)::GetProcAddress(m_Module, "wglUseFontOutlinesA");
        m_wglUseFontOutlinesW_fn = (wglUseFontOutlinesW_fn)::GetProcAddress(m_Module, "wglUseFontOutlinesW");
        m_glGetString_fn = (glGetString_fn)::GetProcAddress(m_Module, "glGetString");

        if (m_wglChoosePixelFormat_fn && m_wglCopyContext_fn && m_wglCreateContext_fn && m_wglCreateLayerContext_fn && m_wglDeleteContext_fn && m_wglDescribeLayerPlane_fn && m_wglDescribePixelFormat_fn &&
            m_wglGetCurrentContext_fn && m_wglGetCurrentDC_fn && m_wglGetDefaultProcAddress_fn && m_wglGetLayerPaletteEntries_fn && m_wglGetPixelFormat_fn && m_wglGetProcAddress_fn &&
            m_wglMakeCurrent_fn && m_wglRealizeLayerPalette_fn && m_wglSetLayerPaletteEntries_fn && m_wglSetPixelFormat_fn && m_wglShareLists_fn && m_wglSwapBuffers_fn && m_wglSwapLayerBuffers_fn &&
            m_wglSwapMultipleBuffers_fn && m_wglUseFontBitmapsA_fn && m_wglUseFontBitmapsW_fn && m_wglUseFontOutlinesA_fn && m_wglUseFontOutlinesW_fn && m_glGetString_fn)
        {
            m_bIsModuleLoaded = true;
        }

#else

        // Linux code: should be added here.

        return dlsym(m_Module, name.c_str());
#endif
    }

}

