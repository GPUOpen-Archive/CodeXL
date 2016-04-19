//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTMainWin.h
///
//==================================================================================

//------------------------------ AMDTMainWin.h ------------------------------

#ifndef __AMDTMAINWIN_H
#define __AMDTMAINWIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "AMDTTeapotConstants.h"
#include "AMDTTeapotOGLCanvas.h"
#include "AMDTTeapotOCLSmokeSystem.h"
#include "AMDTMisc.h"
#include "AMDTDebug.h"

// ----------------------------------------------------------------------------------
// Class Name:           Event
// General Description:
//  A generalizing union used to pass the relevant data from the GUI implementation
//  to the TeaPot logics forgoing the knoledge of the GUI system.
// ----------------------------------------------------------------------------------
union Event
{
    struct ToggleEvent
    {
        int checked;
    } toggle;
    struct MouseEvent
    {
        GLdouble x;
        GLdouble y;
    } mouse;
    struct Handler
    {
        void* widget;
        void* data;
        bool (*helper_func)(void*, void*);
    } handler;
#if defined(WINDOWS) || defined(WIN32) || defined(_WIN32)
    struct SizeEvent
    {
        GLsizei     width;
        GLsizei     height;
    } sizeEvent;
#else       // #if defined(WINDOWS) || defined(WIN32) || defined(_WIN32)
#if defined(__linux__)
    struct ConfigureEvent
    {
        GLint       x;
        GLint       y;
        GLsizei     width;
        GLsizei     height;
    } configureEvent;
    struct Realize
    {
    } realize;
    struct ExposeEvent
    {
        int width;
        int height;
    } exposeEvent;
#endif      // #if defined(__linux__)
#endif      // #if defined(WINDOWS) || defined(WIN32) || defined(_WIN32)
};

// ----------------------------------------------------------------------------------
// Class Name:           MainWin
// General Description:
//  The Teapot application wrapper main class.
// ----------------------------------------------------------------------------------
class MainWin
{
public:
    static MainWin* instance()
    {
        if (_instance == NULL)
        {
            _instance = new MainWin;
        }

        return _instance;
    }
#if defined(WINDOWS) || defined(WIN32) || defined(_WIN32)
    void init(HWND hWnd, HDC* hDC, HGLRC* hRC);
    void onPaint();
    void onIdle();
    void onSize(Event&);
    void getSmokeSystemLastError(const char** s)
    {
        *s = _pAMDTTeapotOGLCanvas->getSmokeSystemLastError();
    }
#else
#if defined(__linux__)
    void realize(Event&);
    void expose_event(Event&);
    void configure_event(Event&);
#endif  // #if defined(__linux__)
#endif  // #if defined(WINDOWS) || defined(WIN32) || defined(_WIN32)
    void mouse_down_event(Event&);
    void mouse_up_event(Event&);
    void mouse_move_event(Event&);
    void idle_event();
    void smokeDevHandler(void* data);

    const OCLInfo*   getOCLInfo()
    {
        return _pAMDTTeapotOGLCanvas->getOCLInfo();
    }
    const OCLDevice* getOCLSmokeSimDevice()
    {
        return _pAMDTTeapotOGLCanvas->getOCLSmokeSimDevice();
    }
    const OCLDevice* getOCLVolSliceDevice()
    {
        return _pAMDTTeapotOGLCanvas->getOCLVolSliceDevice();
    }

    const float      getFrameRate()
    {
        return _pAMDTTeapotOGLCanvas->getFrameRate();
    }
    const bool       usingGLCLSharing()
    {
        return _pAMDTTeapotOGLCanvas->usingGLCLSharing();
    }
    const bool       usingCL()
    {
        return _pAMDTTeapotOGLCanvas->usingCL();
    }
    const bool       canUseCL()
    {
        return _pAMDTTeapotOGLCanvas->canUseCL();
    }
    const char*      getProgressMessage()
    {
        return _pAMDTTeapotOGLCanvas->getProgressMessage();
    }

#if defined(__linux__)
    void callbackHandler(int id, Event&);
#endif      // #if defined(__linux__)

    // Function handlers prototypes
#if defined(__linux__)
    void ID_DUMMY_handler(Event& e);
#endif      // #if defined(__linux__)
    void ID_COMMAND_MAKE_CRASH_handler(Event& e);
    void ID_COMMAND_OUTPUT_SAMPLE_DEBUG_handler(Event& e);
    void ID_COMMAND_GENERATE_OPENGL_ERROR_handler(Event& e);
    void ID_COMMAND_GENERATE_OPENCL_ERROR_handler(Event& e);
    void ID_COMMAND_GENERATE_BREAK_POINT_handler(Event& e);
    void ID_COMMAND_DETECTED_ERROR_handler(Event& e);
    void ID_COMMAND_TOGGLE_SHADING_PARAMETERS_handler(Event& e);
    void ID_COMMAND_TOGGLE_MODEL_VIEW_MATRIX_handler(Event& e);
    void ID_COMMAND_TOGGLE_FRAGMENT_SHADERS_handler(Event& e);
    void ID_VIEW_TOGGLE_BACKGROUND_COLOR_handler(Event& e);
    void ID_VIEW_TOGGLE_SHADING_PROGRAM_handler(Event& e);
    void ID_VIEW_TOGGLE_GEOMETRY_SHADER_handler(Event& e);
    void ID_VIEW_TOGGLE_WIREFRAME_MODE_handler(Event& e);
    void ID_VIEW_INCREASE_TEXTURE_INFLUENCE_handler(Event& e);
    void ID_VIEW_DECREASE_TEXTURE_INFLUENCE_handler(Event& e);
    void ID_VIEW_INCREASE_SPIKINESS_handler(Event& e);
    void ID_VIEW_DECREASE_SPIKINESS_handler(Event& e);
    void ID_SMOKE_ENABLED_handler(Event& e);
    void ID_SMOKE_USE_GLCL_SHARING_handler(Event& e);
    void ID_SMOKE_SHOW_GRID_handler(Event& e);
    void ID_SMOKE_RESET_handler(Event& e);
    void ID_SMOKE_GRID_32_32_64_handler(Event& e);
    void ID_SMOKE_GRID_64_64_64_handler(Event& e);
    void ID_SMOKE_GRID_64_64_128_handler(Event& e);
    void ID_SMOKE_GRID_128_128_128_handler(Event& e);
    void ID_SMOKE_GRID_128_128_256_handler(Event& e);
    void ID_SMOKE_SHOW_ERROR_handler(Event& e);
    void ID_HELP_ABOUT_handler(Event& e);
    void ID_FILE_EXIT_handler(Event& e);

private:
    static MainWin* _instance;
    AMDTTeapotOGLCanvas* _pAMDTTeapotOGLCanvas;

    MainWin();
    MainWin(MainWin const&) {};
    void operator=(MainWin const&) {};
    ~MainWin();
#if defined(WINDOWS) || defined(WIN32) || defined(_WIN32)
    HWND                 _hWnd;
    HDC*                 _hDC;
    HGLRC*               _hRC;

    void EnableOpenGL();
    void DisableOpenGL();
#else
#if defined(__linux__)
    char* errStrSmkNone;
    char* errStrSmkHdr;
    char* hlpStrTtl;
    char* hlpStrDesc;
    char* hlpStrCprt;
#endif  // #if defined(__linux__)
#endif  // #if defined(WINDOWS) || defined(WIN32) || defined(_WIN32)
};

#endif //__AMDTMAINWIN_H

