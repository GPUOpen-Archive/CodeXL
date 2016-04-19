//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file GLWindow.h
///
//==================================================================================

//------------------------------ GLWindow.h ------------------------------

#ifndef GLWINDOW_H_
#define GLWINDOW_H_
#include <string>
#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <GL/gl.h>
#include <FL/fl_ask.H>
#include "AMDTTeapotConstants.h"
#include "AMDTTeapotOGLCanvas.h"
#include "AMDTTeapotOCLSmokeSystem.h"
#include "AMDTMisc.h"
#include "AMDTDebug.h"

class Fl_Box;

// ----------------------------------------------------------------------------------
// Class Name:           Event
// General Description:
//  A generalizing union used to pass the relevant data from the GUI implementation
//  to the TeaPot logics forgoing the knowledge of the GUI system.
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
};

/// Fl_Gl_Window derived widget to keep the OpenGL "context",
/// so that changes to the lighting and projection may be reused
/// between redraws. Fl_Gl_Window also flushes the OpenGL streams
/// and swaps buffers after draw() returns.
class GLWindow : public Fl_Gl_Window
{
public:
    GLWindow(int X, int Y, int W, int H, const char* Label, Fl_Box* boxFPS, Fl_Box* boxCLGL, Fl_Box* boxError);
    ~GLWindow();

    /// Calls _pAMDTTeapotOGLCanvas->processSmokeSystemCommand
    void smokeDevHandler(void* data);

    /// Calls _pAMDTTeapotOGLCanvas->onIdle
    void IdleEvent();

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

    const char*      getProgressMessage()
    {
        return _pAMDTTeapotOGLCanvas->getProgressMessage();
    }

    // Teapot menu callback handlers
    void ID_DUMMY_handler(Event& e);
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
    AMDTTeapotOGLCanvas* _pAMDTTeapotOGLCanvas;
    char* errStrSmkNone;
    char* errStrSmkHdr;
    char* hlpStrTtl;
    char* hlpStrDesc;
    char* hlpStrCprt;
    Fl_Box* m_pBoxFPS;
    Fl_Box* m_pBoxCLGL;
    Fl_Box* m_pBoxError;
    std::string m_strFPS;



public:
    void  draw();
    void  resize(int X, int Y, int W, int H);
    int   handle(int);
    void  UpdateStatusBarInfo();

};

#endif /* GLWINDOW_H_ */
