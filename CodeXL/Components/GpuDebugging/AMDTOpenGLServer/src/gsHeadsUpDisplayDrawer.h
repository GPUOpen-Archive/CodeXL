//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsHeadsUpDisplayDrawer.h
///
//==================================================================================

//------------------------------ gsHeadsUpDisplayDrawer.h ------------------------------

... Yaki 04 / 09 / 2005 - This file is not used anymore(we replaced the FPS heads up
                                                        display drawing by a preformance counter)

#ifndef __GSHEADSUPDISPLAYDRAWER
#define __GSHEADSUPDISPLAYDRAWER

// Pre-declerations:
class apHeadsUpDisplayItem;
class apFPSDisplayItem;

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osStopWatch.h>
#include <AMDTAPIClasses/Include/apGLProgram.h>

// Local:
#include <src/gsTextDrawer.h>


// ----------------------------------------------------------------------------------
// Class Name:           gsHeadsUpDisplayDrawer
// General Description:
//  Draws the heads up display of a single render context.
//
// Author:               Yaki Tebeka
// Creation Date:        10/2/2005
// ----------------------------------------------------------------------------------
class gsHeadsUpDisplayDrawer
{
public:
    gsHeadsUpDisplayDrawer(int spyContextId);

    // Events:
    void onFirstTimeContextMadeCurrent();

    // Self functions:
    void setFontTexture(GLuint fontTextureName, GLuint fontWidth, GLuint fontHeight);
    bool draw(const gtVector<const apHeadsUpDisplayItem*>& headsUpDisplayItems);

private:
    // Do not allow the use of my default constructor:
    gsHeadsUpDisplayDrawer();

    void setOpenGLStatesForHeadsUpDisplay();
    void restoreOpenGLStates();
    bool drawFPSItem(const apFPSDisplayItem& item);

private:
    // The Spy id of my monitored render context:
    int _spyContextId;

    // Draws text:
    gsTextDrawer _textDrawer;

    // A stop watch that measures the time passed sine the last frame was
    // terminated:
    osStopWatch _stopWatch;

    // A frame counter that counts the amount of frames passed since the last
    // time the F/S heads up display was drawn:
    int _frameCounter;

    // Holds an accumulative time interval:
    double _accumulativeTimeInterval;

    // Holds the average F/S rate (as a string):
    gtString _avarageFPSString;

    // Stored OpenGL modes:
    GLenum _storedMatrixMode;
    GLint _storedViewPort[4];
    GLenum _storedShadeModel;
    GLenum _storedPolygonRasterMode[2];
    GLboolean _is1DTexturingEnabled;
    GLboolean _is2DTexturingEnabled;
    GLboolean _is3DTexturingEnabled;
    GLboolean _isCubeMapTexturingEnabled;
    GLboolean _isDepthTestEnabled;
    GLuint _bounded2DTexture;
    GLuint _activeProgramName;
    apGLShadingObjectType _activeProgramType;

    // Extension function pointers:
    PFNGLUSEPROGRAMOBJECTARBPROC _glUseProgramObjectARB;
    PFNGLUSEPROGRAMPROC _glUseProgram;
};


#endif  // __GSHEADSUPDISPLAYDRAWER
