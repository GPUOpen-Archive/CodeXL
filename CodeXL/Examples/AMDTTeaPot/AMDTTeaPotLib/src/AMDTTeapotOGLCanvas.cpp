//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTTeapotOGLCanvas.cpp
///
//==================================================================================

//------------------------------ AMDTTeapotOGLCanvas.cpp ------------------------------

// Platform specific includes:
#if defined (__APPLE__)
    #include <dlfcn.h>
#elif defined(__linux__)
    #include <sys/types.h>
    #include <unistd.h>
    #include <stdio.h>
    #include <signal.h>
    #include <wchar.h>
    #include <string>
#endif

// Local:
#include <src/AMDTTeapotData.dat>
#include <inc/AMDTTeapotConstants.h>
#include <inc/AMDTTeapotOGLCanvas.h>
#include <inc/AMDTTeapotOCLSmokeSystem.h>
#include <inc/AMDTOpenGLMath.h>
#include "AMDTMisc.h"
#include "AMDTDebug.h"
#include "AMDTImage.h"

// Rendered object material:
GLfloat s_ambient[4] = {0.5f, 0.5f, 0.55f , 1.0f };
GLfloat s_diffuse[4] = { 0.44f, 0.44f, 0.44f, 1.0f };
GLfloat s_specular[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
GLfloat s_shininess = 20.0F;


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::AMDTTeapotOGLCanvas
// Description: Constructor
// ---------------------------------------------------------------------------
AMDTTeapotOGLCanvas::AMDTTeapotOGLCanvas() :
    _progressMessage(NULL),
    _OCLInfo(NULL),
    _OCLSmokeSimDevice(NULL),
    _OCLVolSliceDevice(NULL),
    _backgroundColorIndex(1),
    _windowWidth(TP_MAIN_WINDOW_WIDTH),
    _windowHeight(TP_MAIN_WINDOW_HEIGHT),
    _headsUpRasterPosX(0.0f),
    _headsUpRasterPosY(0.0f),
    _mouseInitialPosX(0),
    _mouseInitialPosY(0),
    _mouseDeltaY(0),
    _mouseDeltaX(1),
    _isInViewingMode(false),
    _objectXRot(45.0f),
    _objectYRot(45.0f),
    _objectZRot(0.0f),
    _objectYRotDelta(0.0f),
    _objectXTran(0.0f),
    _objectYTran(-0.2f),
    _objectZTran(0.0f),
    _isModelViewMatrixChanged(false),
    _isObjectShininessChanged(false),
    _fragmentShaderIndex(1),
    _verticesVBOName(0),
    _normalsVBOName(0),
    _texCoordVBOName(0),
    _indicesVBOName(0),
    _indicesBasePointer(NULL),
    _textureObjName(0),
    _rasterMode(GL_FILL),
    _vertexShaderName(0),
    _geometryShaderName(0),
    _fragmentShaderName(0),
    _shadingProgramName(0),
    _textureInfluenceAsInt(7),
    _textureInfluenceAsFloat(0.7f),
    _textureInfluenceUniformLocation(-1),
    _spikinessAsInt(0),
    _spikinessAsFloat(0),
    _spikinessUniformLocation(-1),
    _shadingProgramExists(false),
    _shadingProgramInUse(false),
    _isGeometryShaderCompiled(false),
    _geometryShaderInUse(false),
    _state(),
    _ogl(NULL),
    _smokeSystem(NULL),
    _teapotScalingFactor(0.5F),
    _gridScaleX(1.0f),
    _gridScaleY(1.0f),
    _gridScaleZ(1.0f),
    _numTimeMeasurements(0.0f),
    _totalDeltaTime(0.0f),
    _fps(0.0f),
    _checkGLCLSharing(true),
    _glStringMarkerGREMEDY(NULL),
    _glFrameTerminatorGREMEDY(NULL),
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
    _isGL_ARB_vertex_buffer_objectSupported(false),
    _glGenBuffersARB(NULL),
    _glBindBufferARB(NULL),
    _glBufferDataARB(NULL),
    _glDeleteBuffersARB(NULL),
    _glActiveTexture(NULL),
    _isOpenGL1_5Supported(false),
    _glGenBuffers(NULL),
    _glBindBuffer(NULL),
    _glBufferData(NULL),
    _glDeleteBuffers(NULL),
    _isGL_EXT_geometry_shader4Supported(false),
    _glProgramParameteriEXT(NULL),
    _shouldCrash(false)
{
    memset(_modelMatrix, 0, sizeof(_modelMatrix));

    performGeneralOpenGLInitializations();
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::~AMDTTeapotOGLCanvas
// Description: Destructor.
// ---------------------------------------------------------------------------
AMDTTeapotOGLCanvas::~AMDTTeapotOGLCanvas()
{
    delete _smokeSystem;

    if (_ogl)
    {
        _ogl->ReleaseInstance();
    }
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::onPaint
// Description: Is called when the window needs to be repainted.
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::onPaint(void)
{
    // Repaint the window:
    paintWindow();
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::onSize
// Description:
//  Is called when the window size changes.
//  Adjusts OpenGL's view port to the new window size.
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::onSize(
    int width,
    int height)
{
    // Remember the current aspect
    float currAspect = (float)_windowWidth / (float)_windowHeight;

    // Remember the window dimensions:
    _windowWidth  = width;
    _windowHeight = height;
    _state.setWindowDimensions(_windowWidth, _windowHeight);

    // Set the view port:
    _state.setViewport(0, 0, _windowWidth, _windowHeight);

    // Recalculate the project matrix to get the aspect correct
    // and place the model in the middle of the screen:
    Mat4 proj(true);
#if defined(__linux__)
    proj.Identity();
#endif
    float aspect = (float)_windowWidth / (float)_windowHeight;

#if defined(WINDOWS) || defined(WIN32) || defined(_WIN32)

    if (0.0f > aspect)
    {
        aspect *= -1.0f;
    }
    else if (0.0f == aspect)
    {
        aspect = 1.0f;
    }

    if (1.0f <= aspect)
    {
        proj.SetScale(1.0f, aspect, 1.0f);
    }
    else // 1.0f > aspect
    {
        proj.SetScale(1.0f / aspect, 1.0f, 1.0f);
    }

#endif

    if (aspect < 0.0f)
    {
        float dim = aspect * 0.5f;
        proj.SetFrustum(-dim, dim, -0.5, 0.5, 1.0f, 3.0f);
    }
    else
    {
        float dim = 0.5f / aspect;
        proj.SetFrustum(-0.5, 0.5, -dim, dim, 1.0f, 3.0f);
    }

    _state.setProjectionMatrix(proj);

    // Rescale model so that size doesn't change
    _teapotScalingFactor *= (currAspect / aspect);
    setupGridTransformation();
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::onEraseBackground
// Description:
//   Catch and ignore the "On erase background" message.
//   This avoids the generic windowing system background filling.
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::onEraseBackground(void)
{
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::onLeftMouseDown
// Description: Is called when the left mouse button is pressed down.
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::onLeftMouseDown(
    GLdouble x,
    GLdouble y)
{
    // If we are not in viewing mode (if the button was not already pressed):
    if (!_isInViewingMode)
    {
        // Get into viewing mode:
        _isInViewingMode = true;

        // Store the mouse initial position:
        _mouseInitialPosX = (GLint)x;
        _mouseInitialPosY = (GLint)y;

        // Initialize the mouse delta recording:
        _mouseDeltaX = 0;
        _mouseDeltaY = 0;
    }
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::onLeftMouseUp
// Description: Is called when the mouse left button is released.
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::onLeftMouseUp(void)
{
    // If we are in viewing mode:
    if (_isInViewingMode)
    {
        // Exit viewing mode:
        _isInViewingMode = false;
    }
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::onMouseMove
// Description: Is called when the mouse moves inside the OGL canvas area.
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::onMouseMove(
    GLdouble x,
    GLdouble y)
{
    // If we are in viewing mode:
    if (_isInViewingMode)
    {
        // Get the mouse current position (after the movement):
        GLint mouseCurrentPosX = (GLint)x;
        GLint mouseCurrentPosY = (GLint)y;

        // Calculate the delta in mouse position from the place where
        // it was observed last:
        _mouseDeltaY = mouseCurrentPosY - _mouseInitialPosY;
        _mouseDeltaX = mouseCurrentPosX - _mouseInitialPosX;

        // Calculate the rotation that needs to be performed on the
        // displayed object:
        _objectXRot += 360.0F * _mouseDeltaY / _windowHeight;
        _objectYRotDelta = 360.0F * _mouseDeltaX / _windowWidth;
        _objectYRot += _objectYRotDelta;

        // Update the mouse "last observed" position to contain the current position:
        _mouseInitialPosX = mouseCurrentPosX;
        _mouseInitialPosY = mouseCurrentPosY;

        // Repaint the window:
        paintWindow();
    }
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::setupGridTransformation
// Description: Calculate the new model transformation for the smoke density
//              grid.
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::setupGridTransformation()
{
    // Calculate the fluid grid model transformation. In the cordinate
    // frame of the teapot, we want to position the fluid grid so that
    // the center of the bottom face is positioned over the tea spout
    // and we want to scale the dimensions of the grid. The fluid grid
    // coordinates are -0.5 to 0.5.
    //
    // NOTE: In model space of teapot with 1.0 scaling, spout is located
    //       at (1.17199206, 0.18599232). Maximum/minimum in z is
    //       +/-0.034. Maximum/minimum in x is 1.10599292, 1.2699908.
    _gridTransformation.Identity();
    _gridTransformation.Translate(0.0f, 0.5f, 0.0f);
    _gridTransformation.Scale(_gridScaleX, _gridScaleY, _gridScaleZ);
    _gridTransformation.Translate(1.17199206f, 0.18599232f, 0.0f);
    _gridTransformation.ScaleUniform(_teapotScalingFactor);
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::changeScale
// Description: Change the model zoom factor.
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::changeScale(
    float delta)
{
    _teapotScalingFactor += delta;

    if (_teapotScalingFactor < 0.1f)
    {
        _teapotScalingFactor = 0.1f;
    }
    else if (_teapotScalingFactor > 1.5f)
    {
        _teapotScalingFactor = 1.5f;
    }

    setupGridTransformation();
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::onIdle
// Description: Is called when the system becomes idle.
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::onIdle(void)
{
    // If we are not in viewing mode:
    if (!_isInViewingMode)
    {
        // Calculate the rotation that needs to be performed on the displayed object:
        _objectXRot += 360.0F * TP_AUTO_ROTATION_FACTOR * _mouseDeltaY / _windowHeight;
        _objectYRotDelta = 360.0F * TP_AUTO_ROTATION_FACTOR * _mouseDeltaX / _windowWidth;
        _objectYRot += _objectYRotDelta;

        // Clip the values to [0.0, 360.0):
        while (_objectXRot >= 360.0F)
        {
            _objectXRot -= 360.0F;
        }

        while (_objectXRot < 0.0F)
        {
            _objectXRot += 360.0F;
        }

        while (_objectYRot >= 360.0F)
        {
            _objectYRot -= 360.0F;
        }

        while (_objectYRot < 0.0F)
        {
            _objectYRot += 360.0F;
        }

        // Repaint the window:
        paintWindow();
    }
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::paintWindow()
// Description: Paint the graphic window (redraw it).
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::paintWindow()
{

    // Perform general OpenGL initializations:
    performGeneralOpenGLInitializations();

    // Apply any render state settings.
    _state.apply();

    // Clear the graphic buffers:
    clearGraphicBuffers();


    // Calculate the model transformation for the teapot
    Mat4 modelTrans = getTeapotModelTrans();

    // Draw the scene into the back buffer:
    drawScene();

    // Run smoke simulation and draw smoke
    if (NULL != _smokeSystem)
    {
        // Init the smoke simulation
        // AMDTTeapotOCLSmokeSystem::DrawInitRes res = _smokeSystem->drawInit(_state, modelTrans * _gridTransformation, _objectYRotDelta);

        if (_smokeSystem->draw(_state, modelTrans * _gridTransformation, _objectYRotDelta))
        {
            if (_checkGLCLSharing)
            {
                _checkGLCLSharing = false;
            }
        }
        else
        {
            _progressMessage = _smokeSystem->getLastError();
        }
    }

    // Mark that the frame rendering ended:
    if (_glFrameTerminatorGREMEDY != NULL)
    {
        _glFrameTerminatorGREMEDY();
    }

}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::clearGraphicBuffers
// Description: Clears the color and depth buffers
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::clearGraphicBuffers()
{
    // Set the background color;
    if (_backgroundColorIndex == 0)
    {
        glClearColor(1, 1, 1, 1);
    }
    else
    {
        glClearColor(0, 0, 0, 1);
    }

    // Clear the background (using the above color):
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::drawScene
// Description: Draws the graphic scene.
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::drawScene()
{
    // Setup vertex data
    bindVertexData();

    // Set the current material:
    setMaterial();

    // If we use a shading program:
    if (_shadingProgramExists && _shadingProgramInUse)
    {
        // Set the used shading program:
        _glUseProgramObjectARB(_shadingProgramName);

        // Set the "texture influence" uniform value:
        _glUniform1fARB(_textureInfluenceUniformLocation, _textureInfluenceAsFloat);

        if (_geometryShaderInUse)
        {
            // Set the "spikiness" uniform value. Note that if the geometry shader is
            // not attached or doesn't exist, the vertex shader has this uniform (even
            // though it doesn't use it), so as to avoid an OpenGL error here:
            _glUniform1fARB(_spikinessUniformLocation, _spikinessAsFloat);
        }
    }
    else
    {
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
    }

    // Add a string marker:
    if (_glStringMarkerGREMEDY != NULL)
    {
        _glStringMarkerGREMEDY(0, TP_DRAWING_SCENE_MARKER_STR);
    }

    // Set the used texture:
    glBindTexture(GL_TEXTURE_2D, _textureObjName);

    // Set textures environment parameters:
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    // Enable 2D textures:
    glEnable(GL_TEXTURE_2D);

    // Enable the arrays:
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    // Place the object to be drawn (model transformation)
    glPushMatrix();

    // Calculate the model transformation for the teapot
    glMultMatrixf(_modelMatrix);

    // Draw the object:
    drawTeaPot(_teapotScalingFactor);

    // Clean up - remove the model transformation and disable 2D texturing and arrays:
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    // Perform few redundant state changes:
    glBegin(GL_TRIANGLES);
    GLfloat s_specular1[4] = { 1, 1, 0.8f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, s_specular1);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, s_specular1);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, s_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, s_specular1);
    glEnd();


    // Disable texture:
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);

    // Unbind VBOs
    unbindVertexData();

    // Deactive the shader program
    if (_shadingProgramExists && _shadingProgramInUse)
    {
        _glUseProgramObjectARB(0);
    }
    else
    {
        glDisable(GL_LIGHTING);
        glDisable(GL_LIGHT0);
    }
}

const float AMDTTeapotOGLCanvas::getFrameRate()
{
    return _fps;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::drawTeaPot
// Description: Draws a teapot.
// Arguments:   scale - Scaling (size) of the teapot.
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::drawTeaPot(
    GLfloat scale)
{
    // Add a string marker:
    if (_glStringMarkerGREMEDY != NULL)
    {
        float scaleAsFloat = float(scale);
        char buff[256];
        sprintf(buff, TP_DRAWING_TEAPOT_MARKER_STR, scaleAsFloat);
        _glStringMarkerGREMEDY(0, buff);
    }

    glPushMatrix();
    glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
    glScalef(0.03f * scale, 0.03f * scale, 0.03f * scale);
    glTranslatef(0.0f, 0.0f, -25.0f);

    glDrawElements(GL_TRIANGLES, TP_TEAPOT_INDICES_AMOUNT, GL_UNSIGNED_SHORT, _indicesBasePointer);

    if (glGetError() == GL_NO_ERROR)
    {
        ++_numTimeMeasurements;
    }

    float deltaTime = static_cast<float>(_timer.restart());
    _totalDeltaTime += deltaTime;

    //update FPS only each half second
    if (_totalDeltaTime > 0.5f)
    {
        _fps = _numTimeMeasurements / _totalDeltaTime;
        _numTimeMeasurements = 0.0f;
        _totalDeltaTime = 0.0f;
    }

    glPopMatrix();

    if (_shouldCrash)
    {
        crashThisApplication();
    }
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::isShadingProgramInUse
// Description: Return true if shading program is in use, false otherwise
// ---------------------------------------------------------------------------
bool AMDTTeapotOGLCanvas::isShadingProgramInUse()
{
    return _shadingProgramInUse;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::isGeometryShaderInUse
// Description: Return true if the geometry shader is in use, false otherwise
// ---------------------------------------------------------------------------
bool AMDTTeapotOGLCanvas::isGeometryShaderInUse()
{
    return _geometryShaderInUse;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::setMaterial
// Description: Sets the current OpenGL material
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::setMaterial()
{
    // Add a string marker:
    if (_glStringMarkerGREMEDY != NULL)
    {
        _glStringMarkerGREMEDY(0, TP_SETTING_UP_MATERIAL_MARKER_STR);
    }

    glShadeModel(GL_SMOOTH);

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, s_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, s_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, s_specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, s_shininess);

    // Set the polygon raster mode:
    glPolygonMode(GL_FRONT_AND_BACK, _rasterMode);
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::cycleRasterModes
// Description: Replace the raster mode to the next raster mode.
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::cycleRasterModes()
{
    if (_rasterMode == GL_FILL)
    {
        _rasterMode = GL_LINE;
    }
    else
    {
        _rasterMode = GL_FILL;
    }
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::canCycleShadingProgram
// Description: Returns true iff we can cycle shading programs.
// ---------------------------------------------------------------------------
bool AMDTTeapotOGLCanvas::canCycleShadingProgram()
{
    return _shadingProgramExists;
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::canCycleGeometryShader
// Description: Returns true iff we can cycle the geometry shader.
// ---------------------------------------------------------------------------
bool AMDTTeapotOGLCanvas::canCycleGeometryShader()
{
    return _isGeometryShaderCompiled;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::cycleShadingProgram
// Description: Turn on / off the shading program.
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::cycleShadingProgram()
{
    if (canCycleShadingProgram())
    {
        if (!_shadingProgramInUse)
        {
            useShadingProgram(true);
        }
        else
        {
            useShadingProgram(false);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::cycleGeometryShader
// Description: Turn the geometry shader on / off.
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::cycleGeometryShader()
{
    if (canCycleGeometryShader())
    {
        if (!_geometryShaderInUse)
        {
            useGeometryShader(true);
        }
        else
        {
            useGeometryShader(false);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::cycleBackgroundColor
// Description: Replace the background color with the next background color
//              in the background colors cycle.
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::cycleBackgroundColor()
{
    if (_backgroundColorIndex == 0)
    {
        _backgroundColorIndex = 1;
    }
    else
    {
        _backgroundColorIndex = 0;
    }
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::canIncreaseTextureInfluence()
// Description: Returns true iff the user can increase texture influence.
// ---------------------------------------------------------------------------
bool AMDTTeapotOGLCanvas::canIncreaseTextureInfluence()
{
    bool retVal = (_shadingProgramInUse) && (_textureInfluenceAsInt < 10);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::canDecreaseTextureInfluence()
// Description: Returns true iff the user can decrease texture influence.
// ---------------------------------------------------------------------------
bool AMDTTeapotOGLCanvas::canDecreaseTextureInfluence()
{
    bool retVal = (_shadingProgramInUse) && (_textureInfluenceAsInt > 0);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::canIncreaseSpikiness()
// Description: Returns true iff the user can increase spikiness.
// ---------------------------------------------------------------------------
bool AMDTTeapotOGLCanvas::canIncreaseSpikiness()
{
    bool retVal = (_shadingProgramInUse) && (_geometryShaderInUse) && (_spikinessAsInt < 50);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::canDecreaseSpikiness()
// Description: Returns true iff the user can decrease spikiness.
// ---------------------------------------------------------------------------
bool AMDTTeapotOGLCanvas::canDecreaseSpikiness()
{
    bool retVal = (_shadingProgramInUse)  && (_geometryShaderInUse) && (_spikinessAsInt > -20);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::increaseTextureInfluence
// Description: Increases texture influence (over the shaded material color).
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::increaseTextureInfluence()
{
    if (_shadingProgramInUse)
    {
        if (canIncreaseTextureInfluence())
        {
            _textureInfluenceAsInt += 1;
            _textureInfluenceAsFloat = ((float)_textureInfluenceAsInt / 10.0f);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::decreaseTextureInfluence()
// Description: Decreases texture influence (over the shaded material color)
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::decreaseTextureInfluence()
{
    if (_shadingProgramInUse)
    {
        if (canDecreaseTextureInfluence())
        {
            _textureInfluenceAsInt -= 1;
            _textureInfluenceAsFloat = ((float)_textureInfluenceAsInt / 10.0f);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::increaseSpikiness
// Description: Increases spikiness (protrusion of middle vertex
//              added to each triangle)
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::increaseSpikiness()
{
    if (_shadingProgramInUse && _geometryShaderInUse)
    {
        if (canIncreaseSpikiness())
        {
            _spikinessAsInt += 1;
            _spikinessAsFloat = ((float)_spikinessAsInt / 2.0f);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::decreaseSpikiness()
// Description: Decreases spikiness (protrusion of middle vertex
//              added to each triangle)
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::decreaseSpikiness()
{
    if (_shadingProgramInUse && _geometryShaderInUse)
    {
        if (canDecreaseSpikiness())
        {
            _spikinessAsInt -= 1;
            _spikinessAsFloat = ((float)_spikinessAsInt / 2.0f);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::outputDebugString
// Description:
//   Send a debug string to the debugger that debugs this process.
// Arguments:
//   pDebugStr - A null terminated string to be sent to the debugger.
// Limitations:
//   This functionality is supported on Windows only.
//   On Linux, the string is sent to the standard error file.
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::outputDebugString(const wchar_t* pDebugStr)
{
#if defined(_WIN32)
    // Send a debug string into the debugger:
    ::OutputDebugString(pDebugStr);
#else
    // Send the debug string to the standard error:
    std::wstring wstr(pDebugStr);
    std::string wtr(wstr.begin(), wstr.end());
    ::printf("%s", wtr.c_str());
    ::printf("\n");
    ::fflush(stdout);
#endif
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::generateOpenGLError()
// Description: Generates an OpenGL error.
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::generateOpenGLError()
{
    // Send a debug string into the debugger:
    outputDebugString(TP_OPEN_GL_ERROR_EXAMPLE_STR);
    // Generate an OpenGL error:
    //(Calling glEnable with 0 results with a GL_INVALID_ENUM error):
    glEnable(0);
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::generateOpenCLError()
// Description: Generates an OpenGL error.
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::generateOpenCLError()
{
    // Send a debug string into the debugger:
    outputDebugString(TP_OPEN_CL_ERROR_EXAMPLE_STR);

    _smokeSystem->getContextInfo((cl_context)0, CL_CONTEXT_DEVICES, 1, NULL, NULL);
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::generateDetectedError()
// Description: Generates an OpenGL error to be detected by CodeXL's OpenGL
//              implementation.
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::generateDetectedError()
{
    if (_glActiveTexture)
    {
        // Generate a detected error:
        // (Valid active textures are GL_TEXTURE0, GL_TEXTURE1, ...)
        _glActiveTexture(0);
    }
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::crashThisApplication
// Description: Crashes this application.
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::crashThisApplication()
{
    // Send a debug string into the debugger:
    outputDebugString(TP_CRASH_EXAMPLE_STR);

    // Crash this application (by causing an access violation exception):
    int* i = NULL;
    *i = 3;
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::throwBreakPointException
// Description: Throws a breakpoint exception.
// Limitations:
//   This functionality is supported on Windows only.
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::throwBreakPointException()
{
    // Send a debug string into the debugger:
    outputDebugString(TP_BREAK_POINT_EXAMPLE_STR);

    // Throw a breakpoint exception (platform dependent code):
#if defined(_WIN32)
    {
        DebugBreak();
    }
#else
    {
        pid_t thisProcessId = ::getpid();
        kill(thisProcessId, SIGTRAP);
    }
#endif
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::changeObjectShininess
// Description:
//   Odd calls to this function makes the object shininess wrong.
//   Even calls to this function restores the object shininess.
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::changeObjectShininess()
{
    if (!_isObjectShininessChanged)
    {
        s_shininess = 0.1f;
    }
    else
    {
        s_shininess = 20.0f;
    }

    _isObjectShininessChanged = !_isObjectShininessChanged;
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::outputDebugStringExample
// Description: Send a debug string to the debugger that debugs this application.
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::outputDebugStringExample()
{
    outputDebugString(TP_DEBUG_STRING_EXAMPLE_STR);
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::moveObjectOutOfView
// Description:
//   Odd calls to this function makes the model view matrix render the
//   teapot out of the graphic view.
//   Even calls to this function restores the model view matrix.
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::moveObjectOutOfView()
{
    if (!_isModelViewMatrixChanged)
    {
        _objectXTran = 99999.0;
    }
    else
    {
        _objectXTran = 0.0;
    }

    _isModelViewMatrixChanged = !_isModelViewMatrixChanged;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::createAndLoadTexture
// Description: Creates a texture object and loads the image into it.
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::createAndLoadTexture()
{
    AMDTBMPImage bmpImage = AMDTBMPImage(TP_RESOURCES_PATH TP_TEXTURE_FILE_PATH_BMP);
    bmpImage.readImage();
    bmpImage.toOGLFmt();

    // Create a texture object and make it the bound texture:
    glGenTextures(1, &_textureObjName);
    glBindTexture(GL_TEXTURE_2D, _textureObjName);

    // Set the texture parameters:
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

    // Load the image into it:
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bmpImage._BITMAPINFOHEADER.width, bmpImage._BITMAPINFOHEADER.height, 0, GL_BGRA, GL_UNSIGNED_BYTE, bmpImage._data);
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::readStringFromFile
// Description: Reads a string from a file into a buffer.
// Arguments:
//   filePath - The input file path.
//   bufferLength - The buffer length.
//   pBuffer - The buffer that will receive the string.
// Return Val:  bool - Success / failure.
// ---------------------------------------------------------------------------
bool AMDTTeapotOGLCanvas::readStringFromFile(
    const wchar_t* filePath,
    int bufferLength,
    char* pBuffer)
{
    FILE* fileHandle = NULL;
    bool retVal = false;

    // Sanity check:
    if (bufferLength > 0)
    {
        // Open the text input file for reading:
#if defined(__linux__)
        NEW_CS_FROM_WCS(filePath);
        fileHandle = fopen(_filePath, "rt");
#else
        fileHandle = _wfopen(filePath, L"rt");
#endif

        if (fileHandle != NULL)
        {
            // Read the file content into the buffer:
            int amountOfCharsRead = fread(pBuffer, sizeof(char), bufferLength - 1, fileHandle);

            // NULL terminate the read string:
            pBuffer[amountOfCharsRead] = '\0';

            // Close the file:
            fclose(fileHandle);

            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::initializeExtensions
// Description: Initialized pointers to OpenGL extension functions that will
//              be used.
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::initializeExtensions()
{
    // Get OpenGL function pointers
#define OGLIMPORT(var) _ ## var = _ogl->_ ## var

    // GL_GREMEDY_string_marker function pointer:
    OGLIMPORT(glStringMarkerGREMEDY),

              // GL_GREMEDY_frame_terminator function pointers:
              OGLIMPORT(glFrameTerminatorGREMEDY),

              // GL_ARB_shader_objects extension functions:
              OGLIMPORT(isGL_ARB_shader_objectsSupported);
    OGLIMPORT(glCreateShaderObjectARB);
    OGLIMPORT(glShaderSourceARB);
    OGLIMPORT(glCompileShaderARB);
    OGLIMPORT(glGetObjectParameterivARB);
    OGLIMPORT(glCreateProgramObjectARB);
    OGLIMPORT(glAttachObjectARB);
    OGLIMPORT(glLinkProgramARB);
    OGLIMPORT(glUseProgramObjectARB);
    OGLIMPORT(glGetInfoLogARB);
    OGLIMPORT(glGetUniformLocationARB);
    OGLIMPORT(glUniform1fARB);
    OGLIMPORT(glDeleteObjectARB);
    OGLIMPORT(glDetachObjectARB);

    // GL_ARB_vertex_buffer_object extension functions:
    OGLIMPORT(isGL_ARB_vertex_buffer_objectSupported);
    OGLIMPORT(glGenBuffersARB);
    OGLIMPORT(glBindBufferARB);
    OGLIMPORT(glBufferDataARB);
    OGLIMPORT(glDeleteBuffersARB);

    // OpenGL 1.3 extension functions:
    OGLIMPORT(glActiveTexture);

    // OpenGL 1.5 extension functions:
    OGLIMPORT(isOpenGL1_5Supported);
    OGLIMPORT(glGenBuffers);
    OGLIMPORT(glBindBuffer);
    OGLIMPORT(glBufferData);
    OGLIMPORT(glDeleteBuffers);

    // GL_EXT_geometry_shader4 objects extension functions:
    OGLIMPORT(isGL_EXT_geometry_shader4Supported);
    OGLIMPORT(glProgramParameteriEXT);

}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::setupProgramsAndShaders
// Description: Creates the required shading programs and shaders.
// ---------------------------------------------------------------------------
bool AMDTTeapotOGLCanvas::setupProgramsAndShaders()
{
    bool retVal = false;

    // If the GL_ARB_shader_objects extension is supported:
    if (_isGL_ARB_shader_objectsSupported)
    {
        // Create the shader objects:
        bool rc1 = createShaderObjects();

        if (rc1)
        {
            // Set the shaders source code:
            bool rc2 = setShadersSourceCodes();

            if (rc2)
            {
                // Compile the shader objects:
                bool rc3 = compileShaderObjects();

                if (rc3)
                {
                    // Create the program object:
                    bool rc4 = createProgramObject();

                    if (rc4)
                    {
                        // Attach the shader objects to the program object:
                        bool rc5 = attachShadersToProgram();

                        if (rc5)
                        {
                            // Link the program:
                            bool rc6 = linkProgramObject();

                            if (rc6)
                            {
                                retVal = true;
                            }
                        }
                    }
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::createShaderObjects
// Description: Creates the shader objects.
// ---------------------------------------------------------------------------
bool AMDTTeapotOGLCanvas::createShaderObjects()
{
    bool retVal = false;

    // If the GL_ARB_shader_objects extension is supported:
    if (_isGL_ARB_shader_objectsSupported)
    {
        // Add a string marker:
        if (_glStringMarkerGREMEDY != NULL)
        {
            _glStringMarkerGREMEDY(0, TO_CREATION_GLSL_OBJECTS_MARKER_STR);
        }

        // Create the vertex, geometry and fragment shaders:
        _vertexShaderName = _glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);

        if (_isGL_EXT_geometry_shader4Supported)
        {
            _geometryShaderName = _glCreateShaderObjectARB(GL_GEOMETRY_SHADER_EXT);
        }

        _fragmentShaderName = _glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

        // We don't require the geometric shader creation for success:
        if ((_vertexShaderName != 0) && (_fragmentShaderName != 0))
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::setShadersSourceCodes
// Description: Sets the fragment, geometry and vertex shaders source codes.
// Return Val:  bool - Success / failure.
// ---------------------------------------------------------------------------
bool AMDTTeapotOGLCanvas::setShadersSourceCodes()
{
    bool retVal = false;

    // If the shader objects were create successfully:
    if ((_vertexShaderName != 0) && (_fragmentShaderName != 0))
    {
        // Allocate space for the shaders source codes:
        int sourceCodeBuffLength = 16384;
        char* vertexShaderCode = new char[sourceCodeBuffLength];
        char* fragmentShaderCode = new char[sourceCodeBuffLength];

        char* geometryShaderCode = NULL;

        if (_geometryShaderName != 0)
        {
            geometryShaderCode = new char[sourceCodeBuffLength];
        }

        if ((vertexShaderCode != NULL) && (fragmentShaderCode != NULL))
        {
            // Read the shaders source codes:
            bool vsSourceRead = readStringFromFile(TP_VERTEX_SHADER_CODE_FILE_PATH, sourceCodeBuffLength, vertexShaderCode);
            bool fsSourceRead = readStringFromFile(TP_FRAGMENT_SHADER_CODE_FILE_PATH1, sourceCodeBuffLength, fragmentShaderCode);

            bool gsSourceRead = false;

            if (geometryShaderCode != NULL)
            {
                gsSourceRead = readStringFromFile(TP_GEOMTERY_SHADER_CODE_FILE_PATH, sourceCodeBuffLength, geometryShaderCode);
            }

            if (vsSourceRead && fsSourceRead)
            {
                // Load the source code into the OpenGL shaders:
                _glShaderSourceARB(_vertexShaderName, 1, (const char**)(&vertexShaderCode), NULL);
                _glShaderSourceARB(_fragmentShaderName, 1, (const char**)(&fragmentShaderCode), NULL);

                if (gsSourceRead)
                {
                    _glShaderSourceARB(_geometryShaderName, 1, (const char**)(&geometryShaderCode), NULL);
                }

                retVal = true;
            }

            // Clean up:
            delete[] vertexShaderCode;
            delete[] geometryShaderCode;
            delete[] fragmentShaderCode;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::compileShaderObjects
// Description: Sets the fragment, geometry and vertex shaders source codes.
// Return Val:  bool - Success / failure.
// ---------------------------------------------------------------------------
bool AMDTTeapotOGLCanvas::compileShaderObjects()
{
    bool retVal = false;

    // Sanity check:
    if ((_vertexShaderName != 0) && (_fragmentShaderName != 0))
    {
        // Compile the shaders:
        _glCompileShaderARB(_vertexShaderName);
        _glCompileShaderARB(_fragmentShaderName);

        if (_geometryShaderName != 0)
        {
            _glCompileShaderARB(_geometryShaderName);
        }

        // Verify successful compilation:
        GLint wasVSCompilationSuccessful = 0;
        GLint wasGSCompilationSuccessful = 0;
        GLint wasFSCompilationSuccessful = 0;

        _glGetObjectParameterivARB(_vertexShaderName, GL_OBJECT_COMPILE_STATUS_ARB, &wasVSCompilationSuccessful);
        _glGetObjectParameterivARB(_fragmentShaderName, GL_OBJECT_COMPILE_STATUS_ARB, &wasFSCompilationSuccessful);

        if (_geometryShaderName != 0)
        {
            _glGetObjectParameterivARB(_geometryShaderName, GL_OBJECT_COMPILE_STATUS_ARB, &wasGSCompilationSuccessful);
            wasGSCompilationSuccessful = (wasGSCompilationSuccessful == 1);
        }

        // If we have a compiled geometry shader:
        if (wasGSCompilationSuccessful == 1)
        {
            _isGeometryShaderCompiled = true;
        }

        if ((wasVSCompilationSuccessful == 1) && (wasFSCompilationSuccessful == 1))
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::createProgramObject
// Description: Creates the program object and sets its properties.
// ---------------------------------------------------------------------------
bool AMDTTeapotOGLCanvas::createProgramObject()
{
    bool retVal = false;

    // If the GL_ARB_shader_objects extension is supported:
    if (_isGL_ARB_shader_objectsSupported)
    {
        // Create the program object:
        _shadingProgramName = _glCreateProgramObjectARB();

        if (_shadingProgramName != 0)
        {
            // If supported, set the program's Geometry shader properties:
            if (_isGL_EXT_geometry_shader4Supported)
            {
                _glProgramParameteriEXT(_shadingProgramName, GL_GEOMETRY_INPUT_TYPE_EXT, GL_TRIANGLES);
                _glProgramParameteriEXT(_shadingProgramName, GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_TRIANGLE_STRIP);
                _glProgramParameteriEXT(_shadingProgramName, GL_GEOMETRY_VERTICES_OUT_EXT, 9);
            }

            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::attachShadersToProgram
// Description: Attach the shader objects to the program object.
// ---------------------------------------------------------------------------
bool AMDTTeapotOGLCanvas::attachShadersToProgram()
{
    bool retVal = false;

    // Sanity check:
    if ((_shadingProgramName != 0) && (_vertexShaderName != 0) && (_shadingProgramName != 0))
    {
        // Attach the shaders to the program:
        _glAttachObjectARB(_shadingProgramName, _vertexShaderName);
        _glAttachObjectARB(_shadingProgramName, _fragmentShaderName);

        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::linkProgramObject
// Description: Links the program object and updates its uniforms locations.
// ---------------------------------------------------------------------------
bool AMDTTeapotOGLCanvas::linkProgramObject()
{
    bool retVal = false;

    // Link the program object:
    _glLinkProgramARB(_shadingProgramName);

    // Verify that the link succeeded:
    GLint wasLinkSuccesful = 0;
    _glGetObjectParameterivARB(_shadingProgramName, GL_OBJECT_LINK_STATUS_ARB, &wasLinkSuccesful);

    if (wasLinkSuccesful == 1)
    {
        retVal = true;

        // Get the textureInfluence uniform location:
        _textureInfluenceUniformLocation = _glGetUniformLocationARB(_shadingProgramName, "textureInfluence");

        if (_textureInfluenceUniformLocation == -1)
        {
            retVal = false;
        }

        // Get the spike uniform location:
        if (!_geometryShaderInUse)
        {
            _spikinessUniformLocation = -1;
        }
        else
        {
            _spikinessUniformLocation = _glGetUniformLocationARB(_shadingProgramName, "spike");

            if (_spikinessUniformLocation == -1)
            {
                retVal = false;
            }
        }

    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::useShadingProgram
// Description: Use / don't use the shading program (if exists).
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::useShadingProgram(bool isShadingProgramUsed)
{
    // If the shading program does not exist:
    if (!_shadingProgramExists)
    {
        _shadingProgramInUse = false;
    }
    else
    {
        if (isShadingProgramUsed)
        {
            // Use the shading program (install it into the pipeline):
            _glUseProgramObjectARB(_shadingProgramName);
        }
        else
        {
            // Un-install the program (use the fixed OpenGL pipeline):
            _glUseProgramObjectARB(0);
        }

        // Log the use of the shading program:
        _shadingProgramInUse = isShadingProgramUsed;
    }
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::useGeometryShader
// Description: Use / don't use the geometry shader (if exists).
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::useGeometryShader(bool isGeometryShaderUsed)
{
    // If the shading program or the geometry shader does not exist:
    if (!_shadingProgramExists || !_isGeometryShaderCompiled)
    {
        _geometryShaderInUse = false;
    }
    else
    {
        if (isGeometryShaderUsed)
        {
            // Attach the geometry shader to the program:
            _glAttachObjectARB(_shadingProgramName, _geometryShaderName);

        }
        else
        {
            // detach the geometry shader:
            _glDetachObjectARB(_shadingProgramName, _geometryShaderName);

        }

        // Log the use of the shading program:
        _geometryShaderInUse = isGeometryShaderUsed;

        // Link the program (with / without the geometry shader)
        linkProgramObject();
    }
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::setupVertexArrays
// Description: Set up the teapot model data using VBOs if available, or with
//              OpenGL 1.1 vertex arrays if not.
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::setupVertexArrays()
{
    bool loadedVBOs = false;

    // If we can use VBOs:
    if (_isOpenGL1_5Supported || _isGL_ARB_vertex_buffer_objectSupported)
    {
        // Create VBOs to hold our data:
        bool rc1 = createVertexBufferObjects();

        if (rc1)
        {
            // Load the data into the VBOs and bind the pointers inside them:
            bool rc2 = loadAndBindDataIntoVertexBufferObjects();

            if (rc2)
            {
                loadedVBOs = true;
            }
        }
    }

    // If we didn't or couldn't use VBOs:
    if (!loadedVBOs)
    {
        // Load the data as local (user) data:
        bindVertexArrayLocalData();
    }
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::createVertexBufferObjects
// Description: Create the VBOs to hold the teapot model data.
// ---------------------------------------------------------------------------
bool AMDTTeapotOGLCanvas::createVertexBufferObjects()
{
    bool retVal = false;

    GLuint vboNames[4] = {0, 0, 0, 0};

    // We prefer the OpenGL 1.5 versions of functions over the extension ones:
    if (_isOpenGL1_5Supported)
    {
        _glGenBuffers(4, vboNames);
    }
    else if (_isGL_ARB_vertex_buffer_objectSupported)
    {
        _glGenBuffersARB(4, vboNames);
    }

    // Set the buffer names we received into this class's members:
    _verticesVBOName = vboNames[0];
    _normalsVBOName = vboNames[1];
    _texCoordVBOName = vboNames[2];
    _indicesVBOName = vboNames[3];

    // Make sure we got all four buffers assigned:
    retVal = ((_verticesVBOName != 0) && (_normalsVBOName != 0) && (_texCoordVBOName != 0) && (_indicesVBOName != 0));

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::loadAndBindDataIntoVertexBufferObjects
// Description: Loads the data into the VBOs and sets the vertex array pointers
//              to point inside them.
// ---------------------------------------------------------------------------
bool AMDTTeapotOGLCanvas::loadAndBindDataIntoVertexBufferObjects()
{
    bool retVal = false;

    if (_isOpenGL1_5Supported)
    {
        // Vertex positions data:
        _glBindBuffer(GL_ARRAY_BUFFER, _verticesVBOName);
        GLsizeiptr verticesDataSize = sizeof(float) * TP_TEAPOT_VERTICES_AMOUNT * 3;
        _glBufferData(GL_ARRAY_BUFFER, verticesDataSize, (const GLvoid*)stat_teapotVertices, GL_STATIC_DRAW);
        glVertexPointer(3, GL_FLOAT, 0, 0);

        // Normal directions data:
        _glBindBuffer(GL_ARRAY_BUFFER, _normalsVBOName);
        GLsizeiptr normalsDataSize = sizeof(float) * TP_TEAPOT_VERTICES_AMOUNT * 3;
        _glBufferData(GL_ARRAY_BUFFER, normalsDataSize, (const GLvoid*)stat_teapotNormals, GL_STATIC_DRAW);
        glNormalPointer(GL_FLOAT, 0, 0);

        // Texture coordinates data:
        _glBindBuffer(GL_ARRAY_BUFFER, _texCoordVBOName);
        GLsizeiptr texCoordsDataSize = sizeof(float) * TP_TEAPOT_VERTICES_AMOUNT * 3;
        _glBufferData(GL_ARRAY_BUFFER, texCoordsDataSize, (const GLvoid*)stat_teapotTexCoords, GL_STATIC_DRAW);
        glTexCoordPointer(3, GL_FLOAT, 0, 0);

        // Unbind the array buffer:
        _glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Vertex indices (element array) data:
        _glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indicesVBOName);
        GLsizeiptr indicesDataSize = sizeof(unsigned short) * TP_TEAPOT_INDICES_AMOUNT;
        _glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesDataSize, (const GLvoid*)stat_teapotIndices, GL_STATIC_DRAW);

        // See if this code generated any errors:
        GLenum openGLError = glGetError();
        retVal = (openGLError == GL_NO_ERROR);

        // Check our success:
        if (retVal)
        {
            // Set the elements array offset:
            _indicesBasePointer = 0;
        }
        else
        {
            // If we failed, unbind the element array buffer:
            _glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }
    }
    else if (_isGL_ARB_vertex_buffer_objectSupported)
    {
        // Vertex positions data:
        _glBindBufferARB(GL_ARRAY_BUFFER_ARB, _verticesVBOName);
        GLsizeiptr verticesDataSize = sizeof(float) * TP_TEAPOT_VERTICES_AMOUNT * 3;
        _glBufferDataARB(GL_ARRAY_BUFFER_ARB, verticesDataSize, (const GLvoid*)stat_teapotVertices, GL_STATIC_DRAW_ARB);
        glVertexPointer(3, GL_FLOAT, 0, 0);

        // Normal directions data:
        _glBindBufferARB(GL_ARRAY_BUFFER_ARB, _normalsVBOName);
        GLsizeiptr normalsDataSize = sizeof(float) * TP_TEAPOT_VERTICES_AMOUNT * 3;
        _glBufferDataARB(GL_ARRAY_BUFFER_ARB, normalsDataSize, (const GLvoid*)stat_teapotNormals, GL_STATIC_DRAW_ARB);
        glNormalPointer(GL_FLOAT, 0, 0);

        // Texture coordinates data:
        _glBindBufferARB(GL_ARRAY_BUFFER_ARB, _texCoordVBOName);
        GLsizeiptr texCoordsDataSize = sizeof(float) * TP_TEAPOT_VERTICES_AMOUNT * 3;
        _glBufferDataARB(GL_ARRAY_BUFFER_ARB, texCoordsDataSize, (const GLvoid*)stat_teapotTexCoords, GL_STATIC_DRAW_ARB);
        glTexCoordPointer(3, GL_FLOAT, 0, 0);

        // Unbind the array buffer:
        _glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

        // Vertex indices (element array) data:
        _glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, _indicesVBOName);
        GLsizeiptr indicesDataSize = sizeof(unsigned short) * TP_TEAPOT_INDICES_AMOUNT;
        _glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, indicesDataSize, (const GLvoid*)stat_teapotIndices, GL_STATIC_DRAW_ARB);

        // See if this code generated any errors:
        GLenum openGLError = glGetError();
        retVal = (openGLError == GL_NO_ERROR);

        // Check our success:
        if (retVal)
        {
            // Set the elements array offset:
            _indicesBasePointer = 0;
        }
        else
        {
            // If we failed, unbind the element array buffer:
            _glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::bindVertexArrayLocalData
// Description: Binds the teapot data as local (user memory) arrays.
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::bindVertexArrayLocalData()
{
    // Vertex positions data:
    glVertexPointer(3, GL_FLOAT, 0, (const GLvoid*)stat_teapotVertices);

    // Normal directions data:
    glNormalPointer(GL_FLOAT, 0, (const GLvoid*)stat_teapotNormals);

    // Texture coordinates data:
    glTexCoordPointer(3, GL_FLOAT, 0, (const GLvoid*)stat_teapotTexCoords);

    // Vertex indices (element array) data:
    _indicesBasePointer = (const GLvoid*)stat_teapotIndices;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::bindVertexData
// Description: Bind VBOs (already loaded) or load using local data
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::bindVertexData()
{
    if (_isOpenGL1_5Supported)
    {
        _glBindBuffer(GL_ARRAY_BUFFER, _verticesVBOName);
        glVertexPointer(3, GL_FLOAT, 0, 0);

        // Normal directions data:
        _glBindBuffer(GL_ARRAY_BUFFER, _normalsVBOName);
        glNormalPointer(GL_FLOAT, 0, 0);

        // Texture coordinates data:
        _glBindBuffer(GL_ARRAY_BUFFER, _texCoordVBOName);
        glTexCoordPointer(3, GL_FLOAT, 0, 0);

        // Unbind the array buffer:
        _glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Vertex indices (element array) data:
        if (_indicesBasePointer == NULL)
        {
            _glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indicesVBOName);
        }
    }
    else if (_isGL_ARB_vertex_buffer_objectSupported)
    {
        // Vertex positions data:
        _glBindBufferARB(GL_ARRAY_BUFFER_ARB, _verticesVBOName);
        glVertexPointer(3, GL_FLOAT, 0, 0);

        // Normal directions data:
        _glBindBufferARB(GL_ARRAY_BUFFER_ARB, _normalsVBOName);
        glNormalPointer(GL_FLOAT, 0, 0);

        // Texture coordinates data:
        _glBindBufferARB(GL_ARRAY_BUFFER_ARB, _texCoordVBOName);
        glTexCoordPointer(3, GL_FLOAT, 0, 0);

        // Unbind the array buffer:
        _glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

        // Vertex indices (element array) data:
        if (_indicesBasePointer == NULL)
        {
            _glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, _indicesVBOName);
        }
    }
    else
    {
        bindVertexArrayLocalData();
    }
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::unbindVertexData
// Description: Unbind any VBOs
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::unbindVertexData()
{
    if (_isOpenGL1_5Supported)
    {
        // Vertex indices (element array) data:
        if (_indicesBasePointer == NULL)
        {
            _glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }
    }
    else if (_isGL_ARB_vertex_buffer_objectSupported)
    {
        // Vertex indices (element array) data:
        if (_indicesBasePointer == NULL)
        {
            _glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::performGeneralOpenGLInitializations
// Description: Performs general initializations.
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::performGeneralOpenGLInitializations()
{

    // If OpenGL initializations  were not performed yet:
    static bool oglInitialized = false;

    if (!oglInitialized)
    {
        oglInitialized = true;

        // Setup the OpenGL helper
        _ogl = AMDTOpenGLHelper::GetInstance();

        initializeExtensions();

        // Add a string marker:
        if (_glStringMarkerGREMEDY != NULL)
        {
            _glStringMarkerGREMEDY(0, TP_GENERAL_INIT_MARKER_STR);
        }

        // Set the projection transformation:
        Mat4 proj(true);
        proj.SetFrustum(-0.5F, 0.5F, -0.5F, 0.5F, 1.0F, 3.0F);
        _state.setProjectionMatrix(proj);

        // Set location of camera
        _state.setEye(Vec3(0.0, 0.0, 2.0), Vec3(0.0, 0.0, 0.0), Vec3(0.0, 1.0, 0.0));

        // Set light
        AMDTTeapotRenderState::Light light;
        light._ambientColor[0] = 0.2f;
        light._ambientColor[1] = 0.2f;
        light._ambientColor[2] = 0.2f;
        light._ambientColor[3] = 1.0f;
        light._position = Vec3(0.0f, 0.0f, 1.0f);
        light._direction = Vec3(0.0f, 0.0f, -1.0f);
        light._orientation = Vec3(0.0f, 1.0f, 0.0f);
        _state.setLight(light);

        // Apply the project/view and light changes
        _state.apply();

        // Enable depth test:
        glEnable(GL_DEPTH_TEST);

        // Normalize normals:
        glEnable(GL_NORMALIZE);

        // Load the teapot geometry:
        setupVertexArrays();

        // Generate the used texture:
        createAndLoadTexture();

        // Create the shading programs and shaders:
        _shadingProgramExists = setupProgramsAndShaders();

        // If the shading program was created successfully:
        if (_shadingProgramExists)
        {
            // Use the shading program (install it into the pipeline):
            useShadingProgram(true);
        }

        setupGridTransformation();

        if (_smokeSystem == NULL)
        {
            _smokeSystem = new AMDTTeapotOCLSmokeSystem(this);
            // Initialize the smoke system (via OpenCL)
            performSmokeSystemGLInitializations();
            performSmokeSystemCLInitializations();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::performSmokeSystemCLInitializations
// Description: Performs smoke system initializations.
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::performSmokeSystemCLInitializations()
{
    _smokeSystem->getDeviceInfo(&_OCLInfo, &_OCLSmokeSimDevice, &_OCLVolSliceDevice);
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::performSmokeSystemGLInitializations
// Description: Performs smoke system initializations.
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::performSmokeSystemGLInitializations()
{
    _smokeSystem->initialize();
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::performGeneralOpenGLCleanups
// Description: Performs general OpenGL cleanups (delete allocated objects, etc).
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::performGeneralOpenGLCleanups()
{
    // Delete the VBOs that contain the tea pot geometry:
    if ((_verticesVBOName != 0) || (_normalsVBOName != 0) || (_texCoordVBOName != 0) || (_indicesVBOName != 0))
    {
        GLuint vboNames[4] = {_verticesVBOName, _normalsVBOName, _texCoordVBOName, _indicesVBOName};

        if (_isOpenGL1_5Supported)
        {
            _glDeleteBuffers(4, vboNames);
        }
        else if (_isGL_ARB_vertex_buffer_objectSupported)
        {
            _glDeleteBuffersARB(4, vboNames);
        }
    }

    // Delete the teapot texture:
    if (_textureObjName != 0)
    {
        glDeleteTextures(1, &_textureObjName);
    }

    if (_shadingProgramExists && _isGL_ARB_shader_objectsSupported)
    {
        // Remove the used program (if any):
        _glUseProgramObjectARB(0);

        // Detach shaders from the shading program:
        _glDetachObjectARB(_shadingProgramName, _vertexShaderName);
        _glDetachObjectARB(_shadingProgramName, _geometryShaderName);
        _glDetachObjectARB(_shadingProgramName, _fragmentShaderName);

        // Delete shaders:
        _glDeleteObjectARB(_vertexShaderName);
        _glDeleteObjectARB(_geometryShaderName);
        _glDeleteObjectARB(_fragmentShaderName);

        // Delete the shading program:
        _glDeleteObjectARB(_shadingProgramName);
    }
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::toggleFragmentShaders
// Description: Cycles through fragment shaders.
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::toggleFragmentShaders()
{
    // Will get the fragment shader file name:
    const wchar_t* pShaderFileName = NULL;

    // We toggle the used fragment shader:
    if (_fragmentShaderIndex == 1)
    {
        pShaderFileName = TP_FRAGMENT_SHADER_CODE_FILE_PATH2;
    }
    else
    {
        pShaderFileName = TP_FRAGMENT_SHADER_CODE_FILE_PATH1;
    }

    int sourceCodeBuffLength = 16384;
    char* fragmentShaderCode = new char[sourceCodeBuffLength];

    if (fragmentShaderCode != NULL)
    {
        // Read the Fragment Shader source codes:
        bool fsSourceRead = readStringFromFile(pShaderFileName, sourceCodeBuffLength, fragmentShaderCode);

        if (fsSourceRead)
        {
            // Load the source code into the OpenGL fragment shader:
            _glShaderSourceARB(_fragmentShaderName, 1, (const char**)(&fragmentShaderCode), NULL);

            // Compile the shader:
            _glCompileShaderARB(_fragmentShaderName);

            // Verify successful compilation:
            GLint wasFSCompilationSuccessful = false;
            _glGetObjectParameterivARB(_fragmentShaderName, GL_OBJECT_COMPILE_STATUS_ARB, &wasFSCompilationSuccessful);

            if (wasFSCompilationSuccessful == 1)
            {
                // ReLink the program:
                bool rc1 = linkProgramObject();

                if (rc1)
                {
                    // Mark that we toggled the fragment shader:
                    if (_fragmentShaderIndex == 1)
                    {
                        _fragmentShaderIndex = 2;
                    }
                    else
                    {
                        _fragmentShaderIndex = 1;
                    }
                }
            }
        }

        // Clean up:
        delete[] fragmentShaderCode;
    }
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::processSmokeSystemCommand
// Description: Process Smoke menu selections. If the menu item is a grid
//              resize, update the grid model transformation to reflect the
//              new dimensions.
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::processSmokeSystemCommand(
    SmokeSystemCommand* cmd)
{
    if (NULL != _smokeSystem)
    {
        if (cmd->getType() == SSCMD_GRID_SIZE)
        {
            // When there is a resize command, we also need to resize the grid.
            // Note that GL grid scaling (_gridScale?) has y and z interchanged.
            SmokeSystemCommandGridSize* grid = static_cast<SmokeSystemCommandGridSize*>(cmd);
            _gridScaleX = (float)grid->_x * grid->_spacingMETERS / 1.0f;
            _gridScaleY = (float)grid->_z * grid->_spacingMETERS / 1.0f;
            _gridScaleZ = (float)grid->_y * grid->_spacingMETERS / 1.0f;
            setupGridTransformation();
        }

        _smokeSystem->processSmokeSystemCommand(cmd);
        _checkGLCLSharing = true;
    }
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::setProgress
// Description: Called by AMDTTeapotOCLSmokeSystem to set the current
//              status bar progress message/
// ---------------------------------------------------------------------------
void AMDTTeapotOGLCanvas::setProgress(
    const char* msg)
{
    _progressMessage = msg;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::getSmokeSystemLastError
// Description: Called when menu Smoke->Show errors is selected.
// ---------------------------------------------------------------------------
const char* AMDTTeapotOGLCanvas::getSmokeSystemLastError()
{
    const char* ret = NULL;

    if (NULL != _smokeSystem)
    {
        ret = _smokeSystem->getLastError();
    }

    return ret;
}


// ---------------------------------------------------------------------------
// Name:        AMDTTeapotOGLCanvas::getTeapotModelTrans
// Description: Generate model transformation matrix.
// ---------------------------------------------------------------------------
Mat4 AMDTTeapotOGLCanvas::getTeapotModelTrans()
{
    Mat4 modelTrans(true);
    modelTrans.SetTranslation(_objectXTran, _objectYTran, _objectZTran);
    modelTrans.RotateX(rMath::Deg2Rad(_objectXRot));
    modelTrans.RotateY(rMath::Deg2Rad(_objectYRot));
    modelTrans.RotateZ(rMath::Deg2Rad(_objectZRot));
    modelTrans.GetGLMatrix(_modelMatrix);
    return modelTrans;
}

