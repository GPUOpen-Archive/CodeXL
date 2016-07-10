//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTMainWin.cpp
///
//==================================================================================

//------------------------------ AMDTMainWin.cpp ------------------------------

#if defined(WINDOWS) || defined(WIN32) || defined(_WIN32)
    #include <Windows.h>
#endif  // #if defined(WINDOWS) || defined(WIN32) || defined(_WIN32)

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <wchar.h>
#if defined(__linux__)
#include <iconv.h>
#endif //#if defined(__linux__)
#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#include "AMDTMisc.h"
#include "AMDTDebug.h"
#include "AMDTMainWin.h"
#include "inc/AMDTTeapotOGLCanvas.h"
#include "inc/AMDTTeapotOCLSmokeSystem.h"

MainWin* MainWin::_instance = NULL;
#define TP_SAFE_DELETE(p) { if (p != NULL) { delete p; p = NULL; } }

// This is a workaraound for a bug in gcc 4.7.2: when processing the
// NEW_CS_FROM_WCS macro, gcc 4.7.2 crashes with a segfault.
static char* NarrowStr(const wchar_t* str)
{
    char* ret = NULL;
    const char MINUS_ONE = -1;
    const size_t sz = wcslen(str);
    ret = new char[sz + 1];
    memset(ret, 0, sz);

    size_t retVal = wcstombs(ret, str, sz);
    if (retVal > 0)
    {
        for (unsigned int rd = 0, wr = 0; rd < wcslen(str); rd++)
        {
            switch (wctomb(&ret[wr], str[rd]))
            {
                case -1:
                    break;

                case 0:
                    break;

                default:
                    wr++;
                    break;
            }
        }
    }

    ret[sz] = '\0';

    return ret;
}

// ---------------------------------------------------------------------------
// Name:        MainWin::MainWin
// Description: Constructor.
// ---------------------------------------------------------------------------
MainWin::MainWin() :
    _pAMDTTeapotOGLCanvas(NULL)
#ifdef __linux__
    , errStrSmkNone(NULL), errStrSmkHdr(NULL), hlpStrTtl(NULL), hlpStrDesc(NULL), hlpStrCprt(NULL)
#endif

{
    char* errStrSmkNone = NarrowStr(TP_SMOKE_DIALOG_NO_ERROR);

#if defined(__linux__)
    _pAMDTTeapotOGLCanvas = new AMDTTeapotOGLCanvas();

    // Convert error strings char* strings and assign
    // them to relevant members.
    errStrSmkNone = NarrowStr(TP_SMOKE_DIALOG_NO_ERROR);
    errStrSmkHdr  = NarrowStr(TP_SMOKE_DIALOG_HEADER);
    hlpStrTtl     = NarrowStr(TP_ABOUT_DIALOG_TITLE);
    hlpStrDesc    = NarrowStr(TP_ABOUT_DIALOG_APPLICATION_DESCRIPTION);
    hlpStrCprt    = NarrowStr(TP_ABOUT_DIALOG_COPYRIGHT);

#endif
}

// ---------------------------------------------------------------------------
// Name:        MainWin::~MainWin
// Description: Destructor.
// ---------------------------------------------------------------------------
MainWin::~MainWin()
{
    TP_SAFE_DELETE(_pAMDTTeapotOGLCanvas);
#if defined(WINDOWS) || defined(WIN32) || defined(_WIN32)
    DisableOpenGL();
#else
#if defined(__linux__)
    TP_SAFE_DELETE(errStrSmkNone);
    TP_SAFE_DELETE(errStrSmkHdr);
    TP_SAFE_DELETE(hlpStrTtl);
    TP_SAFE_DELETE(hlpStrDesc);
    TP_SAFE_DELETE(hlpStrCprt);
#endif  // #if defined(__linux__)
#endif  // #if defined(WINDOWS) || defined(WIN32) || defined(_WIN32)
}

#if defined(WINDOWS) || defined(WIN32) || defined(_WIN32)
// ---------------------------------------------------------------------------
// Name:        MainWin::init
// Description: initialize MSWin requirements and Teapot handler.
// ---------------------------------------------------------------------------
void MainWin::init(
    HWND hWnd,
    HDC* hDC,
    HGLRC* hRC)
{
    _hWnd = hWnd;
    _hDC  = hDC;
    _hRC  = hRC;
    EnableOpenGL();
    _pAMDTTeapotOGLCanvas = new AMDTTeapotOGLCanvas();
}

// ---------------------------------------------------------------------------
// Name:        MainWin::onPaint
// Description: Paint the Teapot.
// ---------------------------------------------------------------------------
void MainWin::onPaint()
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->onPaint();
    }
}

// ---------------------------------------------------------------------------
// Name:        MainWin::onIdle
// Description:
// ---------------------------------------------------------------------------
void MainWin::onIdle()
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->onIdle();
    }
}

// ---------------------------------------------------------------------------
// Name:        MainWin::onPaint
// Description: Paint the Teapot.
// ---------------------------------------------------------------------------
void MainWin::onSize(
    Event& e)
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        if ((e.sizeEvent.width > 0) && (e.sizeEvent.height > 0))
        {
            _pAMDTTeapotOGLCanvas->onSize(e.sizeEvent.width, e.sizeEvent.height);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        MainWin::EnableOpenGL
// Description: Enables OpenGL and allocates resources
// ---------------------------------------------------------------------------
void MainWin::EnableOpenGL()
{
    int format;
    // set the pixel format for the DC
    PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),  // strcut size
        1,                      // Version number
        PFD_DRAW_TO_WINDOW |    // Flags, draw to a window,
        PFD_SUPPORT_OPENGL |    // use OpenGL
        PFD_DOUBLEBUFFER,       // double buffered
        PFD_TYPE_RGBA,          // RGBA pixel values
        32,                     // 32-bit color
        0, 0, 0,                // RGB bits & shift sizes.
        0, 0, 0,                // Don't care about them
        0, 0,                   // No alpha buffer info
        0, 0, 0, 0, 0,          // No accumulation buffer
        32,                     // 32-bit depth buffer
        0,                      // No stencil buffer
        0,                      // No auxiliary buffers
        PFD_MAIN_PLANE,         // Layer type
        0,                      // Reserved (must be 0)
        0,                      // No layer mask
        0,                      // No visible mask
        0                       // No damage mask
    };

    // get the device context (DC)
    *_hDC = GetDC(_hWnd);
    ASSERT(((format = ChoosePixelFormat(*_hDC, &pfd)) != 0),    "Error: Failed to ChoosePixelFormat");
    ASSERT((SetPixelFormat(*_hDC, format, &pfd)),               "Error: Failed to SetPixelFormat");

    // create and enable the render context (RC)
    ASSERT(((*_hRC = ::wglCreateContext(*_hDC)) != NULL),       "Error: Failed to wglCreateContext");
    ASSERT((::wglMakeCurrent(*_hDC, *_hRC)),                    "Error: Failed to wglMakeCurrent");
}

// ---------------------------------------------------------------------------
// Name:        MainWin::DisableOpenGL
// Description: Disables OpenGL and releases resources
// ---------------------------------------------------------------------------
void MainWin::DisableOpenGL()
{
    ::wglMakeCurrent(NULL, NULL);
    ::wglDeleteContext(*_hRC);
    ::ReleaseDC(_hWnd, *_hDC);
}
#else
#if defined(__linux__)
// ---------------------------------------------------------------------------
// Name:        MainWin::realize
// Description: runs the corresponding logics for the realize Gtk signal
// ---------------------------------------------------------------------------
void MainWin::realize(
    Event& e)
{
}

// ---------------------------------------------------------------------------
// Name:        MainWin::expose_event
// Description: runs the corresponding logics for the expose_event Gtk signal
// ---------------------------------------------------------------------------
void MainWin::expose_event(
    Event& e)
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->onSize(e.exposeEvent.width, e.exposeEvent.height);
        _pAMDTTeapotOGLCanvas->onPaint();
    }
}

// ---------------------------------------------------------------------------
// Name:        MainWin::configure_event
// Description: runs the corresponding logics for the configure_event Gtk signal
// ---------------------------------------------------------------------------
void MainWin::configure_event(
    Event& e)
{
    if (e.configureEvent.width > 0 && e.configureEvent.width < 10000 && e.configureEvent.height > 0 && e.configureEvent.height < 10000)
    {
        glViewport(0, 0, e.configureEvent.width, e.configureEvent.height);
    }
}
#endif  // #if defined(__linux__)
#endif  // #if defined(WINDOWS) || defined(WIN32) || defined(_WIN32)

// ---------------------------------------------------------------------------
// Name:        MainWin::mouse_down_event
// Description: runs the corresponding logics for the mouse_down_event Gtk signal
// ---------------------------------------------------------------------------
void MainWin::mouse_down_event(
    Event& e)
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->onLeftMouseDown(e.mouse.x, e.mouse.y);
    }
}

// ---------------------------------------------------------------------------
// Name:        MainWin::mouse_up_event
// Description: runs the corresponding logics for the mouse_up_event Gtk signal
// ---------------------------------------------------------------------------
void MainWin::mouse_up_event(
    Event& e)
{
    (void)(e);

    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->onLeftMouseUp();
    }
}

// ---------------------------------------------------------------------------
// Name:        MainWin::mouse_move_event
// Description: runs the corresponding logics for the mouse_move_event Gtk signal
// ---------------------------------------------------------------------------
void MainWin::mouse_move_event(
    Event& e)
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->onMouseMove(e.mouse.x, e.mouse.y);
    }
}

// ---------------------------------------------------------------------------
// Name:        MainWin::idle_event
// Description: runs the corresponding logics for the mouse_move_event Gtk signal
// ---------------------------------------------------------------------------
void MainWin::idle_event()
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->onIdle();
    }
}

#if defined(__linux__)
// ---------------------------------------------------------------------------
// Name:        MainWin::callbackHandler
// Description: runs the corresponding logics for each callback
// ---------------------------------------------------------------------------
void MainWin::callbackHandler(
    int id,
    Event& e)
{
    if (id == (ID_DUMMY))
    {
        // XXX - Shouldn't get here, but in case we do leave this here so as
        // not to repeat with "if"s calls generated by the macros below
        ASSERT(0, "The dummy ID handler should not be called");
        return;
    }
}

void MainWin::ID_DUMMY_handler(
    Event& e)
{
    ASSERT(0, "The dummy ID handler should not be called");
}
#endif      // #if defined(__linux__)

void MainWin::ID_COMMAND_MAKE_CRASH_handler(
    Event& e)
{
    (void)(e);

    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->crashThisApplication();
    }
}

void MainWin::ID_COMMAND_OUTPUT_SAMPLE_DEBUG_handler(
    Event& e)
{
    (void)(e);

    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->outputDebugStringExample();
    }
}

void MainWin::ID_COMMAND_GENERATE_OPENGL_ERROR_handler(
    Event& e)
{
    (void)(e);

    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->generateOpenGLError();
    }
}

void MainWin::ID_COMMAND_GENERATE_OPENCL_ERROR_handler(
    Event& e)
{
    (void)(e);

    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->generateOpenCLError();
    }
}

void MainWin::ID_COMMAND_GENERATE_BREAK_POINT_handler(
    Event& e)
{
    (void)(e);

    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->throwBreakPointException();
    }
}

void MainWin::ID_COMMAND_DETECTED_ERROR_handler(
    Event& e)
{
    (void)(e);

    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->generateDetectedError();
    }
}

void MainWin::ID_COMMAND_TOGGLE_SHADING_PARAMETERS_handler(
    Event& e)
{
    (void)(e);

    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->changeObjectShininess();
    }
}

void MainWin::ID_COMMAND_TOGGLE_MODEL_VIEW_MATRIX_handler(
    Event& e)
{
    (void)(e);

    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->moveObjectOutOfView();
    }
}

void MainWin::ID_COMMAND_TOGGLE_FRAGMENT_SHADERS_handler(
    Event& e)
{
    (void)(e);

    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->toggleFragmentShaders();
    }
}


void MainWin::ID_VIEW_TOGGLE_BACKGROUND_COLOR_handler(
    Event& e)
{
    (void)(e);

    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->cycleBackgroundColor();
    }
}

void MainWin::ID_VIEW_TOGGLE_SHADING_PROGRAM_handler(
    Event& e)
{
    (void)(e);

    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->cycleShadingProgram();
    }
}

void MainWin::ID_VIEW_TOGGLE_GEOMETRY_SHADER_handler(
    Event& e)
{
    (void)(e);

    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->cycleGeometryShader();
    }
}

void MainWin::ID_VIEW_TOGGLE_WIREFRAME_MODE_handler(
    Event& e)
{
    (void)(e);

    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->cycleRasterModes();
    }
}

void MainWin::ID_VIEW_INCREASE_TEXTURE_INFLUENCE_handler(
    Event& e)
{
    (void)(e);

    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->increaseTextureInfluence();
    }
}

void MainWin::ID_VIEW_DECREASE_TEXTURE_INFLUENCE_handler(
    Event& e)
{
    (void)(e);

    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->decreaseTextureInfluence();
    }
}

void MainWin::ID_VIEW_INCREASE_SPIKINESS_handler(
    Event& e)
{
    (void)(e);

    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->decreaseSpikiness();
    }
}

void MainWin::ID_VIEW_DECREASE_SPIKINESS_handler(
    Event& e)
{
    (void)(e);

    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->increaseSpikiness();
    }
}


void MainWin::ID_SMOKE_ENABLED_handler(
    Event& e)
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
#if defined(WINDOWS) || defined(WIN32) || defined(_WIN32)
        SmokeSystemCommandToggle ssc = SmokeSystemCommandToggle(SSCID_ENABLE, !e.toggle.checked);
#else
#if defined(__linux__)
        SmokeSystemCommandToggle ssc = SmokeSystemCommandToggle(SSCID_ENABLE, e.handler.helper_func(e.handler.widget, e.handler.data));
        ssc._checked = e.handler.helper_func(e.handler.widget, e.handler.data);
#endif  // #if defined(__linux__)
#endif  // #if defined(WINDOWS) || defined(WIN32) || defined(_WIN32)
        _pAMDTTeapotOGLCanvas->processSmokeSystemCommand(&ssc);
    }
}

void MainWin::ID_SMOKE_USE_GLCL_SHARING_handler(
    Event& e)
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
#if defined(WINDOWS) || defined(WIN32) || defined(_WIN32)
        SmokeSystemCommandToggle ssc = SmokeSystemCommandToggle(SSCID_USE_GLCL_SHARING, !e.toggle.checked);
#else
#if defined(__linux__)
        SmokeSystemCommandToggle ssc = SmokeSystemCommandToggle(SSCID_USE_GLCL_SHARING, e.handler.helper_func(e.handler.widget, e.handler.data));
        ssc._checked = e.handler.helper_func(e.handler.widget, e.handler.data);
#endif  // #if defined(__linux__)
#endif  // #if defined(WINDOWS) || defined(WIN32) || defined(_WIN32)
        _pAMDTTeapotOGLCanvas->processSmokeSystemCommand(&ssc);
    }
}

void MainWin::ID_SMOKE_SHOW_GRID_handler(
    Event& e)
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
#if defined(WINDOWS) || defined(WIN32) || defined(_WIN32)
        SmokeSystemCommandToggle ssc = SmokeSystemCommandToggle(SSCID_SHOW_GRID, !e.toggle.checked);
#else
#if defined(__linux__)
        SmokeSystemCommandToggle ssc = SmokeSystemCommandToggle(SSCID_SHOW_GRID, e.handler.helper_func(e.handler.widget, e.handler.data));
        ssc._checked = e.handler.helper_func(e.handler.widget, e.handler.data);
#endif  // #if defined(__linux__)
#endif  // #if defined(WINDOWS) || defined(WIN32) || defined(_WIN32)
        _pAMDTTeapotOGLCanvas->processSmokeSystemCommand(&ssc);
    }
}

void MainWin::ID_SMOKE_RESET_handler(
    Event& e)
{
    (void)(e);

    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        SmokeSystemCommandSelect ssc = SmokeSystemCommandSelect(SSCID_RESET);
        _pAMDTTeapotOGLCanvas->processSmokeSystemCommand(&ssc);
    }
}

void MainWin::ID_SMOKE_GRID_32_32_64_handler(
    Event& e)
{
    (void)(e);

    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        SmokeSystemCommandGridSize ssc = SmokeSystemCommandGridSize(SSCID_GRID_SIZE, 32, 32, 64, 1.0f / 32.0f);
        _pAMDTTeapotOGLCanvas->processSmokeSystemCommand(&ssc);
    }
}

void MainWin::ID_SMOKE_GRID_64_64_64_handler(
    Event& e)
{
    (void)(e);

    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        SmokeSystemCommandGridSize ssc = SmokeSystemCommandGridSize(SSCID_GRID_SIZE, 64, 64, 64, 1.0f / 64.0f);
        _pAMDTTeapotOGLCanvas->processSmokeSystemCommand(&ssc);
    }
}

void MainWin::ID_SMOKE_GRID_64_64_128_handler(
    Event& e)
{
    (void)(e);

    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        SmokeSystemCommandGridSize ssc = SmokeSystemCommandGridSize(SSCID_GRID_SIZE, 64, 64, 128, 1.0f / 64.0f);
        _pAMDTTeapotOGLCanvas->processSmokeSystemCommand(&ssc);
    }
}

void MainWin::ID_SMOKE_GRID_128_128_128_handler(
    Event& e)
{
    (void)(e);

    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        SmokeSystemCommandGridSize ssc = SmokeSystemCommandGridSize(SSCID_GRID_SIZE, 128, 128, 128, 1.0f / 128.0f);
        _pAMDTTeapotOGLCanvas->processSmokeSystemCommand(&ssc);
    }
}

void MainWin::ID_SMOKE_GRID_128_128_256_handler(
    Event& e)
{
    (void)(e);

    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        SmokeSystemCommandGridSize ssc = SmokeSystemCommandGridSize(SSCID_GRID_SIZE, 128, 128, 256, 1.0f / 128.0f);
        _pAMDTTeapotOGLCanvas->processSmokeSystemCommand(&ssc);
    }
}

void MainWin::ID_SMOKE_SHOW_ERROR_handler(
    Event& e)
{
    (void)(e);
#if defined(__linux__)
    // "This function is obsolete and will be removed in a future CodeXL release."
#endif      // #if defined(__linux__)
}

void MainWin::ID_HELP_ABOUT_handler(
    Event& e)
{
    (void)(e);
#if defined(__linux__)
    // "This function is obsolete and will be removed in a future CodeXL release."
#endif      // #if defined(__linux__)
}

void MainWin::ID_FILE_EXIT_handler(
    Event& e)
{
    (void)(e);
#if defined(__linux__)
    // "This function is obsolete and will be removed in a future CodeXL release."
#endif      // #if defined(__linux__)
}

void MainWin::smokeDevHandler(
    void* data)
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->processSmokeSystemCommand((SmokeSystemCommandChangeDevice*)data);
    }
}
