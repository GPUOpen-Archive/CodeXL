//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file GLWindow.cpp
///
//==================================================================================

//------------------------------ GLWindow.cpp ------------------------------

#include <FL/Fl.H>
#include <FL/gl.h>
#include <FL/Fl_Box.H>
#include <wchar.h>
#include <string>
#include <iconv.h>
#include <FL/fl_ask.H>
#include "GLWindow.h"
// C++.
#include <sstream>
#include <string>
#include <iomanip>

#pragma GCC diagnostic ignored "-Wformat-security"

#define CL_GL_SHARING_ACTIVE "OpenCL-OpenGL Sharing is Active"
#define CL_GL_SHARING_INACTIVE "OpenCL-OpenGL Sharing is Inactive"

const int KEYPAD_PLUS = FL_KP + 43;
const int KEYPAD_MINUS = FL_KP + 45;

#define TP_SAFE_DELETE(p) { if (p != NULL) { delete p; p = NULL; } }
// This is a workaraound for a bug in gcc 4.7.2: when processing the
// NEW_CS_FROM_WCS macro, gcc 4.7.2 crashes with a segfault.
char* NarrowStr(const wchar_t* str)
{
    char* ret = NULL;
    const size_t MINUS_ONE = -1;
    const size_t sz = wcslen(str);
    ret = new char[sz + 1];
    memset(ret, 0, sz);

    if (wcstombs(ret, str, sz == MINUS_ONE))
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

/// Idle callback passed a pointer to GLWindow.
/// Calls GLWindow::IdleEvent and updates the status bar parts (fltk boxes).
static void Idle_CB(void* data)
{
    GLWindow* pWin = (GLWindow*)data;

    if (pWin != NULL)
    {
        pWin->IdleEvent();
        pWin->UpdateStatusBarInfo();
    }
    else
    {
        Fl::remove_idle(Idle_CB);
    }
}

/// Constructor
GLWindow::GLWindow(int X, int Y, int W, int H, const char* Label, Fl_Box* boxFPS, Fl_Box* boxCLGL, Fl_Box* boxError)
    : Fl_Gl_Window(X, Y, W, H, Label), _pAMDTTeapotOGLCanvas(0), errStrSmkNone(0), errStrSmkHdr(0), hlpStrTtl(0),
      hlpStrDesc(0), hlpStrCprt(0), m_pBoxFPS(boxFPS), m_pBoxCLGL(boxCLGL), m_pBoxError(boxError)
{

    // Convert error strings char* strings and assign
    // them to relevant members.
    errStrSmkNone = NarrowStr(TP_SMOKE_DIALOG_NO_ERROR);
    errStrSmkHdr  = NarrowStr(TP_SMOKE_DIALOG_HEADER);
    hlpStrTtl     = NarrowStr(TP_ABOUT_DIALOG_TITLE);
    hlpStrDesc    = NarrowStr(TP_ABOUT_DIALOG_APPLICATION_DESCRIPTION);
    hlpStrCprt    = NarrowStr(TP_ABOUT_DIALOG_COPYRIGHT);
}


void GLWindow::draw()
{
    if (NULL == _pAMDTTeapotOGLCanvas)
    {
        _pAMDTTeapotOGLCanvas = new AMDTTeapotOGLCanvas();
    }

    if (NULL != _pAMDTTeapotOGLCanvas)
    {
        _pAMDTTeapotOGLCanvas->onPaint();
    }


    swap_buffers();

    // Add idle event only once
    static bool onlyOnce = true;

    if (onlyOnce)
    {
        onlyOnce = false;
        Fl::add_idle(Idle_CB, (void*)this);
    }
}

// HANDLE WINDOW RESIZING
//    If window reshaped, need to readjust viewport/ortho
//
void GLWindow::resize(int X, int Y, int W, int H)
{
    _pAMDTTeapotOGLCanvas->onSize(W, H);
    swap_buffers();
    Fl_Gl_Window::resize(X, Y, W, H);
}

int GLWindow::handle(int event)
{
    switch (event)
    {

        case FL_PUSH:
            if (_pAMDTTeapotOGLCanvas != NULL)
            {
                _pAMDTTeapotOGLCanvas->onLeftMouseDown(Fl::event_x(), Fl::event_y());
                draw();
            }

            return 1;

        case FL_MOVE:
            if (_pAMDTTeapotOGLCanvas != NULL)
            {
                _pAMDTTeapotOGLCanvas->onMouseMove(Fl::event_x(), Fl::event_y());
                draw();
            }

            return 1;

        case FL_DRAG:
            _pAMDTTeapotOGLCanvas->onMouseMove(Fl::event_x(), Fl::event_y());
            draw();
            //... mouse moved while down event ...
            return 1;

        case FL_RELEASE:
            if (_pAMDTTeapotOGLCanvas != NULL)
            {
                _pAMDTTeapotOGLCanvas->onLeftMouseUp();
                draw();
            }

            return 1;

        case FL_FOCUS :
        case FL_UNFOCUS :
            //... Return 1 if you want keyboard events, 0 otherwise
            return 1;

        default:
            // pass other events to the base class...
            return Fl_Gl_Window::handle(event);
    }
}


GLWindow::~GLWindow()
{
    TP_SAFE_DELETE(_pAMDTTeapotOGLCanvas);
    TP_SAFE_DELETE(errStrSmkNone);
    TP_SAFE_DELETE(errStrSmkHdr);
    TP_SAFE_DELETE(hlpStrTtl);
    TP_SAFE_DELETE(hlpStrDesc);
    TP_SAFE_DELETE(hlpStrCprt);
}


void GLWindow::ID_DUMMY_handler(
    Event& e)
{
    ASSERT(0, "The dummy ID handler should not be called");
}

void GLWindow::ID_COMMAND_MAKE_CRASH_handler(
    Event& e)
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->SetCrashFlag();
    }
}

void GLWindow::ID_COMMAND_OUTPUT_SAMPLE_DEBUG_handler(
    Event& e)
{
    std::string textStr(TP_MENU_COMMANDS_OUTPUT_SAMPLE_DEBUG_DESCRIPTION_LINUX);
    fl_alert(textStr.c_str());

    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->outputDebugStringExample();
    }
}

void GLWindow::ID_COMMAND_GENERATE_OPENGL_ERROR_handler(
    Event& e)
{

    std::string textStr(TP_MENU_COMMANDS_GENERATE_OPENGL_ERROR_DESCRIPTION_LINUX);
    fl_alert(textStr.c_str());

    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->generateOpenGLError();
    }
}

void GLWindow::ID_COMMAND_GENERATE_OPENCL_ERROR_handler(
    Event& e)
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->generateOpenCLError();
    }
}

void GLWindow::ID_COMMAND_GENERATE_BREAK_POINT_handler(
    Event& e)
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->throwBreakPointException();
    }
}

void GLWindow::ID_COMMAND_DETECTED_ERROR_handler(
    Event& e)
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->generateDetectedError();
    }
}

void GLWindow::ID_COMMAND_TOGGLE_SHADING_PARAMETERS_handler(
    Event& e)
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->changeObjectShininess();
    }
}

void GLWindow::ID_COMMAND_TOGGLE_MODEL_VIEW_MATRIX_handler(
    Event& e)
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->moveObjectOutOfView();
    }
}

void GLWindow::ID_COMMAND_TOGGLE_FRAGMENT_SHADERS_handler(
    Event& e)
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->toggleFragmentShaders();
    }
}


void GLWindow::ID_VIEW_TOGGLE_BACKGROUND_COLOR_handler(
    Event& e)
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->cycleBackgroundColor();
    }
}

void GLWindow::ID_VIEW_TOGGLE_SHADING_PROGRAM_handler(
    Event& e)
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->cycleShadingProgram();
    }
}

void GLWindow::ID_VIEW_TOGGLE_GEOMETRY_SHADER_handler(
    Event& e)
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->cycleGeometryShader();
    }
}

void GLWindow::ID_VIEW_TOGGLE_WIREFRAME_MODE_handler(
    Event& e)
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->cycleRasterModes();
    }
}

void GLWindow::ID_VIEW_INCREASE_TEXTURE_INFLUENCE_handler(
    Event& e)
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->increaseTextureInfluence();
    }
}

void GLWindow::ID_VIEW_DECREASE_TEXTURE_INFLUENCE_handler(
    Event& e)
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->decreaseTextureInfluence();
    }
}

void GLWindow::ID_VIEW_INCREASE_SPIKINESS_handler(
    Event& e)
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->decreaseSpikiness();
    }
}

void GLWindow::ID_VIEW_DECREASE_SPIKINESS_handler(
    Event& e)
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->increaseSpikiness();
    }
}

void GLWindow::ID_SMOKE_ENABLED_handler(
    Event& e)
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        SmokeSystemCommandToggle ssc = SmokeSystemCommandToggle(SSCID_ENABLE, e.toggle.checked);
        _pAMDTTeapotOGLCanvas->processSmokeSystemCommand(&ssc);
    }
}

void GLWindow::ID_SMOKE_USE_GLCL_SHARING_handler(
    Event& e)
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        SmokeSystemCommandToggle ssc = SmokeSystemCommandToggle(SSCID_USE_GLCL_SHARING, e.toggle.checked);
        _pAMDTTeapotOGLCanvas->processSmokeSystemCommand(&ssc);
    }
}

void GLWindow::ID_SMOKE_SHOW_GRID_handler(
    Event& e)
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        SmokeSystemCommandToggle ssc = SmokeSystemCommandToggle(SSCID_SHOW_GRID, e.toggle.checked);
        _pAMDTTeapotOGLCanvas->processSmokeSystemCommand(&ssc);
    }
}

void GLWindow::ID_SMOKE_RESET_handler(
    Event& e)
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        SmokeSystemCommandSelect ssc = SmokeSystemCommandSelect(SSCID_RESET);
        _pAMDTTeapotOGLCanvas->processSmokeSystemCommand(&ssc);
    }
}

void GLWindow::ID_SMOKE_GRID_32_32_64_handler(
    Event& e)
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        SmokeSystemCommandGridSize ssc = SmokeSystemCommandGridSize(SSCID_GRID_SIZE, 32, 32, 64, 1.0f / 32.0f);
        _pAMDTTeapotOGLCanvas->processSmokeSystemCommand(&ssc);
    }
}

void GLWindow::ID_SMOKE_GRID_64_64_64_handler(
    Event& e)
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        SmokeSystemCommandGridSize ssc = SmokeSystemCommandGridSize(SSCID_GRID_SIZE, 64, 64, 64, 1.0f / 64.0f);
        _pAMDTTeapotOGLCanvas->processSmokeSystemCommand(&ssc);
    }
}

void GLWindow::ID_SMOKE_GRID_64_64_128_handler(
    Event& e)
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        SmokeSystemCommandGridSize ssc = SmokeSystemCommandGridSize(SSCID_GRID_SIZE, 64, 64, 128, 1.0f / 64.0f);
        _pAMDTTeapotOGLCanvas->processSmokeSystemCommand(&ssc);
    }
}

void GLWindow::ID_SMOKE_GRID_128_128_128_handler(
    Event& e)
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        SmokeSystemCommandGridSize ssc = SmokeSystemCommandGridSize(SSCID_GRID_SIZE, 128, 128, 128, 1.0f / 128.0f);
        _pAMDTTeapotOGLCanvas->processSmokeSystemCommand(&ssc);
    }
}

void GLWindow::ID_SMOKE_GRID_128_128_256_handler(
    Event& e)
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        SmokeSystemCommandGridSize ssc = SmokeSystemCommandGridSize(SSCID_GRID_SIZE, 128, 128, 256, 1.0f / 128.0f);
        _pAMDTTeapotOGLCanvas->processSmokeSystemCommand(&ssc);
    }
}

void GLWindow::ID_SMOKE_SHOW_ERROR_handler(Event& e)
{
    const char* errStr = NULL;
    errStr = _pAMDTTeapotOGLCanvas->getSmokeSystemLastError();

    if (errStr == NULL)
    {
        fl_alert(errStrSmkNone);
    }
    else
    {
        fl_alert(errStr);
    }
}

void GLWindow::ID_HELP_ABOUT_handler(Event& e)
{
    std::wstring wstr(TP_ABOUT_DIALOG_APPLICATION_DESCRIPTION);
    wstr += L"\n\n";
    wstr += TP_ABOUT_DIALOG_COPYRIGHT;
    wstr += L"\n\n";
    wstr += L"http://developer.amd.com/";
    wstr += L"\n\n";
    std::string wtr(wstr.begin(), wstr.end());
    fl_message(wtr.c_str());
}

void GLWindow::ID_FILE_EXIT_handler(Event& e)
{
    exit(0);
}

void GLWindow::smokeDevHandler(
    void* data)
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->processSmokeSystemCommand((SmokeSystemCommandChangeDevice*)data);
    }
}
void GLWindow::IdleEvent()
{
    if (_pAMDTTeapotOGLCanvas != NULL)
    {
        _pAMDTTeapotOGLCanvas->onIdle();
    }

    draw();
}

void GLWindow::UpdateStatusBarInfo()
{
    if (NULL != m_pBoxError && NULL != m_pBoxFPS && NULL != m_pBoxCLGL)
    {
        std::stringstream outStream(std::stringstream::in | std::stringstream::out);
        outStream << std::fixed << std::setprecision(2) << getFrameRate() << " FPS";
        std::string tmp = outStream.str();

        if (m_strFPS.compare(tmp) != 0)
        {
            m_strFPS = tmp;
        }

        m_pBoxFPS->label(m_strFPS.c_str());
        m_pBoxFPS->redraw_label();
        outStream.clear();

        if (usingGLCLSharing())
        {
            m_pBoxCLGL->label(CL_GL_SHARING_ACTIVE);
        }
        else
        {
            m_pBoxCLGL->label(CL_GL_SHARING_INACTIVE);
        }

        m_pBoxCLGL->redraw_label();
        m_pBoxError->label(getProgressMessage());
        m_pBoxError->redraw_label();
    }
}
