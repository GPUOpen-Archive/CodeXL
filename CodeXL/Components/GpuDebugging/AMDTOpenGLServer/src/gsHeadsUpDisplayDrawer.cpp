//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsHeadsUpDisplayDrawer.cpp
///
//==================================================================================

//------------------------------ gsHeadsUpDisplayDrawer.cpp ------------------------------

... Yaki 04 / 09 / 2005 - This file is not used anymore(we replaced the FPS heads up
                                                        display drawing by a preformance counter)

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/apFPSDisplayItem.h>

// Local:
#include <inc/gsHeadsUpDisplayDrawer.h>
#include <inc/gsOpenGLMonitor.h>
#include <inc/gsOpenGLWrappers.h>

// The interval between two updates of the F/S heads up display (measured in seconds)
#define GS_FPS_FRAMES_UPDATE_INTERVAL 0.5


// ---------------------------------------------------------------------------
// Name:        gsHeadsUpDisplayDrawer::gsHeadsUpDisplayDrawer
// Description: Constructor
// Author:      Yaki Tebeka
// Date:        10/2/2005
// ---------------------------------------------------------------------------
gsHeadsUpDisplayDrawer::gsHeadsUpDisplayDrawer(int spyContextId)
    : _spyContextId(spyContextId),
      _frameCounter(0), _accumulativeTimeInterval(0.0),
      _activeProgramName(0), _activeProgramType(AP_ARB_SHADER_OBJECTS_PROGRAM),
      _glUseProgramObjectARB(NULL), _glUseProgram(NULL)
{
}


// ---------------------------------------------------------------------------
// Name:        gsHeadsUpDisplayDrawer::onFirstTimeContextMadeCurrent
// Description: Is called at the first time that my render context is made current.
// Author:      Yaki Tebeka
// Date:        13/4/2005
// ---------------------------------------------------------------------------
void gsHeadsUpDisplayDrawer::onFirstTimeContextMadeCurrent()
{
    // If my monitored context is not the NULL context:
    if (_spyContextId != gsRenderContextMonitor::nullContextId())
    {
        _glUseProgramObjectARB = (PFNGLUSEPROGRAMOBJECTARBPROC)(gs_stat_realFunctionPointers.wglGetProcAddress("glUseProgramObjectARB"));
        _glUseProgram = (PFNGLUSEPROGRAMPROC)(gs_stat_realFunctionPointers.wglGetProcAddress("glUseProgram"));
    }
}


// ---------------------------------------------------------------------------
// Name:        gsHeadsUpDisplayDrawer::setFontTexture
// Description: Sets the name and parameters of the texture that contains the
//              text font.
// Arguments:   fontTextureName - The OpenGL name of the texture that contains
//                                the font.
//              fontWidth - Font char width.
//              fontHeight - Font char hight.
// Author:      Yaki Tebeka
// Date:        1/3/2005
// ---------------------------------------------------------------------------
void gsHeadsUpDisplayDrawer::setFontTexture(GLuint fontTextureName, GLuint fontWidth, GLuint fontHeight)
{
    if (fontTextureName != 0)
    {
        _textDrawer.setFontTexture(fontTextureName, fontWidth, fontHeight);
    }
    else
    {
        gtAssert(0);
    }
}


// ---------------------------------------------------------------------------
// Name:        gsHeadsUpDisplayDrawer::draw
// Description: Draws the heads up display.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        10/2/2005
// ---------------------------------------------------------------------------
bool gsHeadsUpDisplayDrawer::draw(const gtVector<const apHeadsUpDisplayItem*>& headsUpDisplayItems)
{
    bool retVal = true;

    // Set OpenGL states for heads up display:
    setOpenGLStatesForHeadsUpDisplay();

    // Iterate the heads up display items:
    int amountOfElements = headsUpDisplayItems.size();

    for (int i = 0; i < amountOfElements; i++)
    {
        // Get the current heads up display item:
        const apHeadsUpDisplayItem* pCurrentItem = headsUpDisplayItems[i];

        if (pCurrentItem)
        {
            // Get the current item type:
            osTransferableObjectType currentElemType = pCurrentItem->type();

            // Draw it:
            switch (currentElemType)
            {
                case OS_TOBJ_ID_FPS_DISPLAY_ITEM:
                    retVal = drawFPSItem((const apFPSDisplayItem&)(*headsUpDisplayItems[i]));
                    break;

                default:
                    // An unknown element type:
                    gtAssert(0);
                    break;
            }
        }
    }

    // Clean up:
    restoreOpenGLStates();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsHeadsUpDisplayDrawer::setOpenGLStatesForHeadsUpDisplay
// Description: Sets OpenGL for drawing the heads up display: model view and
//              projection matrices, Z buffer, etc
// Author:      Yaki Tebeka
// Date:        14/2/2005
// ---------------------------------------------------------------------------
void gsHeadsUpDisplayDrawer::setOpenGLStatesForHeadsUpDisplay()
{
    // TO_DO: Replace all the below glGet calls with the appropriate
    //        values logging by the gsRenderContextWrapper.

    // Store the current matrix mode:
    gs_stat_realFunctionPointers.glGetIntegerv(GL_MATRIX_MODE, (int*)(&_storedMatrixMode));

    // Store the current view-port:
    gs_stat_realFunctionPointers.glGetIntegerv(GL_VIEWPORT, _storedViewPort);

    // Initialize the texture matrix:
    gs_stat_realFunctionPointers.glMatrixMode(GL_TEXTURE);
    gs_stat_realFunctionPointers.glPushMatrix();
    gs_stat_realFunctionPointers.glLoadIdentity();

    // Initialize the projection matrix to contain a simple [0,1] orthographic projection:
    gs_stat_realFunctionPointers.glMatrixMode(GL_PROJECTION);
    gs_stat_realFunctionPointers.glPushMatrix();
    gs_stat_realFunctionPointers.glLoadIdentity();
    gs_stat_realFunctionPointers.glOrtho(0.0f, 1.0f, 0.0f, 1.0f, -1.0, 1.0);

    // Initialize the Model view matrix:
    gs_stat_realFunctionPointers.glMatrixMode(GL_MODELVIEW);
    gs_stat_realFunctionPointers.glPushMatrix();
    gs_stat_realFunctionPointers.glLoadIdentity();

    // Get my render context monitor:
    const gsOpenGLMonitor& theOpenGLMonitor = gsOpenGLMonitor::instance();
    const gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = theOpenGLMonitor.renderContextMonitor(_spyContextId);

    if (pCurrentThreadRenderContextMonitor)
    {
        // Initialize the view-port to be the "real" view port:
        // (Not the forced single pixel view-port):
        const gsForcedModesManager& forcedModesMgr = pCurrentThreadRenderContextMonitor->forcedModesManager();
        const GLint* viewPort = forcedModesMgr.realViewPort();
        gs_stat_realFunctionPointers.glViewport(viewPort[0], viewPort[1], viewPort[2], viewPort[3]);
    }

    // Disable the depth test:
    _isDepthTestEnabled = gs_stat_realFunctionPointers.glIsEnabled(GL_DEPTH_TEST);

    if (_isDepthTestEnabled)
    {
        gs_stat_realFunctionPointers.glDisable(GL_DEPTH_TEST);
    };

    // Store the enabled texturing modes:
    // TO_DO: Check if the 3D and cube map extensions are supported:
    _is1DTexturingEnabled = gs_stat_realFunctionPointers.glIsEnabled(GL_TEXTURE_1D);

    _is2DTexturingEnabled = gs_stat_realFunctionPointers.glIsEnabled(GL_TEXTURE_2D);

    _is3DTexturingEnabled = gs_stat_realFunctionPointers.glIsEnabled(GL_TEXTURE_3D);

    _isCubeMapTexturingEnabled = gs_stat_realFunctionPointers.glIsEnabled(GL_TEXTURE_CUBE_MAP);

    // Enable 2D texturing:
    if (_is1DTexturingEnabled) { gs_stat_realFunctionPointers.glDisable(GL_TEXTURE_1D); };

    if (!_is2DTexturingEnabled) { gs_stat_realFunctionPointers.glEnable(GL_TEXTURE_2D); };

    if (_is3DTexturingEnabled) { gs_stat_realFunctionPointers.glDisable(GL_TEXTURE_3D); };

    if (_isCubeMapTexturingEnabled) { gs_stat_realFunctionPointers.glDisable(GL_TEXTURE_CUBE_MAP); };

    // Store the bounded 2D texture:
    // TO_DO: Check this extension !!
    gs_stat_realFunctionPointers.glGetIntegerv(GL_TEXTURE_2D_BINDING_EXT, (int*)&_bounded2DTexture);

    // Set the shading model:
    gs_stat_realFunctionPointers.glGetIntegerv(GL_SHADE_MODEL, (int*)(&_storedShadeModel));

    gs_stat_realFunctionPointers.glShadeModel(GL_FLAT);

    // Set the polygon raster mode:
    gs_stat_realFunctionPointers.glGetIntegerv(GL_POLYGON_MODE, (int*)_storedPolygonRasterMode);

    gs_stat_realFunctionPointers.glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Get my context monitor:
    const gsRenderContextMonitor* pRenderContextMtr = theOpenGLMonitor.renderContextMonitor(_spyContextId);

    if (pRenderContextMtr != NULL)
    {
        // Get the used shading program:
        const gsProgramsAndShadersMonitor& progsMtr = pRenderContextMtr->programsAndShadersMonitor();
        _activeProgramName = progsMtr.activeProgramName();

        // If there is a used shading program:
        if (_activeProgramName != 0)
        {
            // Get the object that represents the active program:
            const apGLProgram* pActiveProg = progsMtr.programObjectDetails(_activeProgramName);

            if (pActiveProg)
            {
                // Get the program type:
                _activeProgramType = pActiveProg->programType();

                // Remove it from being the active program:
                if ((_activeProgramType == AP_ARB_SHADER_OBJECTS_PROGRAM) && (_glUseProgramObjectARB != NULL))
                {
                    _glUseProgramObjectARB(0);
                }
                else if ((_activeProgramType == AP_2_0_PROGRAM) && (_glUseProgram != NULL))
                {
                    _glUseProgram(0);
                }
            }
        }
    }

}


// ---------------------------------------------------------------------------
// Name:        gsHeadsUpDisplayDrawer::restoreOpenGLStates
// Description: Restores the OpenGL states to the value they had before
//              setOpenGLStatesForHeadsUpDisplay() was called.
// Author:      Yaki Tebeka
// Date:        28/2/2005
// ---------------------------------------------------------------------------
void gsHeadsUpDisplayDrawer::restoreOpenGLStates()
{
    // Restore the texture matrix:
    gs_stat_realFunctionPointers.glMatrixMode(GL_TEXTURE);
    gs_stat_realFunctionPointers.glPopMatrix();

    // Restore the projection matrix:
    gs_stat_realFunctionPointers.glMatrixMode(GL_PROJECTION);
    gs_stat_realFunctionPointers.glPopMatrix();

    // Restore the model-view matrix:
    gs_stat_realFunctionPointers.glMatrixMode(GL_MODELVIEW);
    gs_stat_realFunctionPointers.glPopMatrix();

    // Restore the matrix mode:
    gs_stat_realFunctionPointers.glMatrixMode(_storedMatrixMode);

    // Restore the view port:
    gs_stat_realFunctionPointers.glViewport(_storedViewPort[0], _storedViewPort[1],
                                            _storedViewPort[2], _storedViewPort[3]);

    // Restore the depth test mode:
    if (_isDepthTestEnabled) {  gs_stat_realFunctionPointers.glEnable(GL_DEPTH_TEST); };

    // Restore the enabled texture modes:
    // TO_DO: Check if the 3D and cube map extensions are supported:
    if (_is1DTexturingEnabled) { gs_stat_realFunctionPointers.glEnable(GL_TEXTURE_1D); };

    if (!_is2DTexturingEnabled) { gs_stat_realFunctionPointers.glDisable(GL_TEXTURE_2D); };

    if (_is3DTexturingEnabled) { gs_stat_realFunctionPointers.glEnable(GL_TEXTURE_3D); };

    if (_isCubeMapTexturingEnabled) { gs_stat_realFunctionPointers.glEnable(GL_TEXTURE_CUBE_MAP); };

    // Restore the bounded 2D texture:
    gs_stat_realFunctionPointers.glBindTexture(GL_TEXTURE_2D, _bounded2DTexture);

    // Restore the shade model:
    gs_stat_realFunctionPointers.glShadeModel(_storedShadeModel);

    // Restore the polygon mode:
    gs_stat_realFunctionPointers.glPolygonMode(GL_FRONT, _storedPolygonRasterMode[0]);

    gs_stat_realFunctionPointers.glPolygonMode(GL_BACK, _storedPolygonRasterMode[1]);

    // If there was an active program:
    if (_activeProgramName != 0)
    {
        // Restore it:
        if ((_activeProgramType == AP_ARB_SHADER_OBJECTS_PROGRAM) && (_glUseProgramObjectARB != NULL))
        {
            _glUseProgramObjectARB(_activeProgramName);
        }
        else if ((_activeProgramType == AP_2_0_PROGRAM) && (_glUseProgram != NULL))
        {
            _glUseProgram(_activeProgramName);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gsHeadsUpDisplayDrawer::drawFPSItem
// Description: Draws frame per second element
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        14/2/2005
// ---------------------------------------------------------------------------
bool gsHeadsUpDisplayDrawer::drawFPSItem(const apFPSDisplayItem& item)
{
    bool retVal = true;

    // If the stop-watch is not running - start it:
    if (!(_stopWatch.isRunning()))
    {
        _stopWatch.start();
    }
    else
    {
        // Increment the frames count:
        _frameCounter++;

        // Get the stop watch time interval:
        double timeInterval = _stopWatch.getTimeInterval();

        // Add it to the accumulative F/S interval:
        _accumulativeTimeInterval += timeInterval;

        // If we reached the F/S update interval:
        if (_accumulativeTimeInterval >= GS_FPS_FRAMES_UPDATE_INTERVAL)
        {
            // Calculate the average F/S:
            float avarageFPS = 1.0 / (_accumulativeTimeInterval / _frameCounter);

            // Translate it to a string:
            _avarageFPSString = "FPS: ";

            // If the average F/S is smaller than 10 - display one digit after the dot:
            if (avarageFPS < 10.0)
            {
                _avarageFPSString.appendFormattedString("%.1f", avarageFPS);
            }
            else
            {
                _avarageFPSString.appendFormattedString("%.0f", avarageFPS);
            }


            // Re-Initialize counters:
            _frameCounter = 0;
            _accumulativeTimeInterval = 0.0;
        }

        // Draw the average F/S string:
        retVal = _textDrawer.draw(_avarageFPSString);


        // Restart the stop watch:
        _stopWatch.start();
    }

    return retVal;
}
