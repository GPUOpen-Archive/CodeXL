//==============================================================================
// Copyright (c) 2012-2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools
/// \file
/// \brief This class manages the dynamic loading of OpenGL.
//==============================================================================

#ifndef _OPENGL_MODULE_H_
#define _OPENGL_MODULE_H_

#include "DynamicLibraryModule.h"

#ifdef _WIN32

    // WGL functions:
    typedef int (WINAPI* wglChoosePixelFormat_fn)(HDC a, CONST PIXELFORMATDESCRIPTOR* b);
    typedef BOOL(WINAPI* wglCopyContext_fn)(HGLRC a, HGLRC b, UINT c);
    typedef HGLRC(WINAPI* wglCreateContext_fn)(HDC a);
    typedef HGLRC(WINAPI* wglCreateLayerContext_fn)(HDC a, int b);
    typedef BOOL(WINAPI* wglDeleteContext_fn)(HGLRC a);
    typedef BOOL(WINAPI* wglDescribeLayerPlane_fn)(HDC a, int b, int c, UINT d, LPLAYERPLANEDESCRIPTOR e);
    typedef int (WINAPI* wglDescribePixelFormat_fn)(HDC a, int b, UINT c, LPPIXELFORMATDESCRIPTOR d);
    typedef HGLRC(WINAPI* wglGetCurrentContext_fn)(void);
    typedef HDC(WINAPI* wglGetCurrentDC_fn)(void);
    typedef PROC(WINAPI* wglGetDefaultProcAddress_fn)(LPCSTR a);
    typedef int (WINAPI* wglGetLayerPaletteEntries_fn)(HDC a, int b, int c, int d, COLORREF* e);
    typedef int (WINAPI* wglGetPixelFormat_fn)(HDC a);
    typedef PROC(WINAPI* wglGetProcAddress_fn)(LPCSTR a);
    typedef BOOL(WINAPI* wglMakeCurrent_fn)(HDC a, HGLRC b);
    typedef BOOL(WINAPI* wglRealizeLayerPalette_fn)(HDC a, int b, BOOL c);
    typedef int (WINAPI* wglSetLayerPaletteEntries_fn)(HDC a, int b, int c, int d, CONST COLORREF* e);
    typedef BOOL(WINAPI* wglSetPixelFormat_fn)(HDC a, int b, CONST PIXELFORMATDESCRIPTOR* c);
    typedef BOOL(WINAPI* wglShareLists_fn)(HGLRC a, HGLRC b);
    typedef BOOL(WINAPI* wglSwapBuffers_fn)(HDC a);
    typedef BOOL(WINAPI* wglSwapLayerBuffers_fn)(HDC hdc, UINT fuPlanes);
    typedef DWORD(WINAPI* wglSwapMultipleBuffers_fn)(UINT a, const WGLSWAP* b);
    typedef BOOL(WINAPI* wglUseFontBitmapsA_fn)(HDC a, DWORD b, DWORD c, DWORD d);
    typedef BOOL(WINAPI* wglUseFontBitmapsW_fn)(HDC a, DWORD b, DWORD c, DWORD d);
    typedef BOOL(WINAPI* wglUseFontOutlinesA_fn)(HDC a, DWORD b, DWORD c, DWORD d, FLOAT e, FLOAT f, int g, LPGLYPHMETRICSFLOAT h);
    typedef BOOL(WINAPI* wglUseFontOutlinesW_fn)(HDC a, DWORD b, DWORD c, DWORD d, FLOAT e, FLOAT f, int g, LPGLYPHMETRICSFLOAT h);

    // OGL functions:
    typedef unsigned char GLubyte;
    typedef unsigned int GLenum;
    typedef const GLubyte* (WINAPI* glGetString_fn)(GLenum glEnum);

#else

    // Linux code: should be added here.

#endif


/// This class handles the dynamic loading of OpenCL.dll.
/// \note There will typically be one of these objects.
///       That instance will be global.
///       There is a trap for the unwary.
///       The order of global ctors is only defined within a single compilation unit.
///       So, one should not use these interfaces before "main" is reached.
///       This is different than calling these functions when the .dll/.so is linked against.
class OpenGLModule
{
public:
    /// Which shared image is loaded?
    enum OpenGLVersion
    {
        OpenGL_None,              ///< No OpenCL version is loaded
        OpenGL_3_0,               ///< OpenCL V3.0 is loaded
    };

    /// Default name to use for construction.
    /// This is usually OpenGL.dll
    static const char* s_DefaultModuleName;

    /// Constructor
    /// \param module to load.
    OpenGLModule(const std::string& moduleName);

    /// destructor
    ~OpenGLModule();

    /// Checks id the module is loaded
    bool IsModuleLoaded() const
    {
        return m_bIsModuleLoaded;
    }

    /// Unload OpenCL.dll
    void UnloadModule();

    /// Which OpenCL have we got?
    /// \return enumeration value to answer query.
    OpenGLVersion OpenGLLoaded();

    // OpenGl functions
    wglChoosePixelFormat_fn     m_wglChoosePixelFormat_fn;
    wglCopyContext_fn           m_wglCopyContext_fn;
    wglCreateContext_fn         m_wglCreateContext_fn;
    wglCreateLayerContext_fn    m_wglCreateLayerContext_fn;
    wglDeleteContext_fn         m_wglDeleteContext_fn;
    wglDescribeLayerPlane_fn    m_wglDescribeLayerPlane_fn;
    wglDescribePixelFormat_fn   m_wglDescribePixelFormat_fn;
    wglGetCurrentContext_fn     m_wglGetCurrentContext_fn;
    wglGetCurrentDC_fn          m_wglGetCurrentDC_fn;
    wglGetDefaultProcAddress_fn m_wglGetDefaultProcAddress_fn;
    wglGetLayerPaletteEntries_fn m_wglGetLayerPaletteEntries_fn;
    wglGetPixelFormat_fn         m_wglGetPixelFormat_fn;
    wglGetProcAddress_fn         m_wglGetProcAddress_fn;
    wglMakeCurrent_fn            m_wglMakeCurrent_fn;
    wglRealizeLayerPalette_fn    m_wglRealizeLayerPalette_fn;
    wglSetLayerPaletteEntries_fn m_wglSetLayerPaletteEntries_fn;
    wglSetPixelFormat_fn         m_wglSetPixelFormat_fn;
    wglShareLists_fn             m_wglShareLists_fn;
    wglSwapBuffers_fn            m_wglSwapBuffers_fn;
    wglSwapLayerBuffers_fn       m_wglSwapLayerBuffers_fn;
    wglSwapMultipleBuffers_fn    m_wglSwapMultipleBuffers_fn;
    wglUseFontBitmapsA_fn        m_wglUseFontBitmapsA_fn;
    wglUseFontBitmapsW_fn        m_wglUseFontBitmapsW_fn;
    wglUseFontOutlinesA_fn       m_wglUseFontOutlinesA_fn;
    wglUseFontOutlinesW_fn       m_wglUseFontOutlinesW_fn;
    glGetString_fn               m_glGetString_fn;

private:
    /// Initialize the internal data
    void Initialize();



    /// Is module loaded
    OpenGLVersion m_openGLVersion;



#ifdef _WIN32
    typedef HMODULE ImageHandle_t;
#else
    typedef void* ImageHandle_t;
#endif

    /// Handle/pointer to .so/.dll.
    ImageHandle_t m_Module;
    std::string m_ModuleName;
    bool m_bIsModuleLoaded;
};


#endif
