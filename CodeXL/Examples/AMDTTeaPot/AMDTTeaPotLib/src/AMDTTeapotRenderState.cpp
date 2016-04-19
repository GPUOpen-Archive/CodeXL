//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTTeapotRenderState.cpp
///
//==================================================================================

//------------------------------ AMDTTeapotRenderState.cpp ------------------------------

#include <inc/AMDTTeapotRenderState.h>

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotRenderState::AMDTTeapotRenderState
// Description: Constructor. Setup the default render state - eye position,
//              light data, viewport.
// ---------------------------------------------------------------------------
AMDTTeapotRenderState::AMDTTeapotRenderState() :
    _dirtyFlags(0)
{
    _eyePosition = Vec3(0.0f, 0.0f, 2.0f);
    _eyeLookAt = Vec3(0.0f, 0.0f, 0.0f);
    _eyeOrientation = Vec3(0.0f, 1.0f, 0.0f);
    _dirtyFlags |= DIRTY_VIEWMAT;

    _light._position = Vec3(0.0f, 0.0f, 1.0);
    _light._direction = Vec3(0.0f, 0.0f, -1.0f);
    _light._orientation = Vec3(0.0f, 1.0f, 0.0f);
    _dirtyFlags |= DIRTY_LIGHT;

    _projectionMatrix.Identity();
    _dirtyFlags |= DIRTY_PROJMAT;

    _windowWidth = 400;
    _windowHeight = 400;
    _viewportX = 0;
    _viewportY = 0;
    _viewportWidth = 400;
    _viewportHeight = 400;
    _dirtyFlags |= DIRTY_VIEWPORT;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotRenderState::~AMDTTeapotRenderState
// Description: Deconstructor.
// ---------------------------------------------------------------------------
AMDTTeapotRenderState::~AMDTTeapotRenderState()
{
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotRenderState::setProjectionMatrix
// Description: Store the current projection matrix.
// ---------------------------------------------------------------------------
void AMDTTeapotRenderState::setProjectionMatrix(
    const Mat4& mat)
{
    _projectionMatrix = mat;
    _dirtyFlags |= DIRTY_PROJMAT;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotRenderState::setEye
// Description: Store eye position, direction and orientation.
// ---------------------------------------------------------------------------
void AMDTTeapotRenderState::setEye(
    const Vec3& position,
    const Vec3& lookAt,
    const Vec3& orientation)
{
    _eyePosition = position;
    _eyeLookAt = lookAt;
    _eyeOrientation = orientation;
    _dirtyFlags |= DIRTY_VIEWMAT;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotRenderState::setLight
// Description: Set ambient light data: position, direction, orientation, color.
// ---------------------------------------------------------------------------
void AMDTTeapotRenderState::setLight(
    const Light& light)
{
    _light = light;
    _dirtyFlags |= DIRTY_LIGHT;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotRenderState::apply
// Description: Make OpenGL calls to change state based on changes to state as
//              registered by this clase.
// ---------------------------------------------------------------------------
void AMDTTeapotRenderState::apply()
{
    if (_dirtyFlags & DIRTY_PROJMAT)
    {
        _projectionMatrix.GetGLMatrix(_glProjectionMatrix);
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(_glProjectionMatrix);
        glMatrixMode(GL_MODELVIEW);
        _dirtyFlags &= (~DIRTY_PROJMAT);
    }

    if (_dirtyFlags & DIRTY_VIEWMAT)
    {
        _viewMatrix.Identity();
        _viewMatrix.LookAt(_eyePosition, _eyeLookAt, _eyeOrientation);
        _viewMatrix.GetGLMatrix(_glViewMatrix);
        glLoadMatrixf(_glViewMatrix);
        _dirtyFlags &= (~DIRTY_VIEWMAT);
    }

    if (_dirtyFlags & DIRTY_VIEWPORT)
    {
        glViewport(_viewportX, _viewportY, _viewportWidth, _viewportHeight);
        _dirtyFlags &= (~DIRTY_VIEWPORT);
    }

    if (_dirtyFlags & DIRTY_LIGHT)
    {
        // When we set the light position, OpenGL applies the current model view transformation.
        // Just in case, let's set it up correctly now to be the view matrix.
        glPushMatrix();
        glLoadMatrixf(_glViewMatrix);

        GLfloat position[4] = {_light._position[0], _light._position[1], _light._position[2], 1.0f};

        glLightfv(GL_LIGHT0, GL_AMBIENT, _light._ambientColor);
        glLightfv(GL_LIGHT0, GL_POSITION, position);

        _dirtyFlags &= (~DIRTY_LIGHT);

        glPopMatrix();
    }
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotRenderState::setWindowDimensions
// Description: Keep track of changes to the window size.
// ---------------------------------------------------------------------------
void AMDTTeapotRenderState::setWindowDimensions(
    int width,
    int height)
{
    _windowWidth = width;
    _windowHeight = height;
}

// ---------------------------------------------------------------------------
// Name:        AMDTTeapotRenderState::setViewport
// Description: Keep track of viewport as set by the application.
// ---------------------------------------------------------------------------
void AMDTTeapotRenderState::setViewport(
    int x,
    int y,
    int width,
    int height)
{
    _viewportX = x;
    _viewportY = y;
    _viewportWidth = width;
    _viewportHeight = height;
    _dirtyFlags |= DIRTY_VIEWPORT;
}
