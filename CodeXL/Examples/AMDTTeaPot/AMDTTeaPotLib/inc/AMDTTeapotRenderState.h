//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTTeapotRenderState.h
///
//==================================================================================

//------------------------------ AMDTTeapotRenderState.h ------------------------------

#ifndef __AMDTTEAPOTRENDERSTATE_H
#define __AMDTTEAPOTRENDERSTATE_H

#include <inc/AMDTOpenGLMath.h>
#include <inc/AMDTOpenGLHelper.h>

/******************************************************************************
 *
 * AMDTTeapotRenderState
 * -------------------
 *
 * This class is used to keep track of and set the following OpenGL state:
 *
 *     - Eye location, direction and orientation.
 *     - Light location, direction and color.
 *     - Window size.
 *     - Viewport.
 *     - Projection matrix.
 *
 * It is used as a way to pass state information to other classes. For example,
 * AMDTTeapotOGLCanvas sets the state and calls apply() to have changes applied
 * in OpenGL. It passes this class over to AMDTTeapotOCLSmokeSystem so that the
 * latter can use this information.
 *
 ******************************************************************************/
class AMDTTeapotRenderState
{
public:
    AMDTTeapotRenderState();
    virtual ~AMDTTeapotRenderState();

    struct Light
    {
        GLfloat     _ambientColor[4];
        Vec3        _position;
        Vec3        _direction;
        Vec3        _orientation;

        Light()
        {
            _ambientColor[0] = 1.0f;
            _ambientColor[1] = 1.0f;
            _ambientColor[2] = 1.0f;
            _ambientColor[3] = 1.0f;
            _position = Vec3(0.0f, 0.0f, 1.0f);
            _direction = Vec3(0.0f, 0.0f, -1.0f);
            _orientation = Vec3(0.0f, 1.0f, 0.0f);
        }

    };

    // Set the window dimensions.
    void setWindowDimensions(int width, int height);

    // Set the viewport size.
    void setViewport(int x, int y, int width, int height);

    // Set the project matrix.
    void setProjectionMatrix(const Mat4& mat);

    // Set the location, direction and orientation of the eye/camera.
    void setEye(const Vec3& position, const Vec3& lookAt, const Vec3& orientation);

    // Set the current ambient light data.
    void setLight(const Light& light);

    // Apply the current state in OpenGL.
    void apply();

    // Getter functions for eye and light information.
    const Vec3& getEyePosition() const
    {
        return _eyePosition;
    }
    const Vec3& getEyeLookAt() const
    {
        return _eyeLookAt;
    }

    const Vec3& getLightPosition() const
    {
        return _light._position;
    }
    const Vec3& getLightDirection() const
    {
        return _light._direction;
    }

private:
    // Keep track of what has changed so that apply() knows what OpenGL render
    // state needs to be updated.
    enum DirtyFlags
    {
        DIRTY_PROJMAT       = (1 << 0),
        DIRTY_VIEWMAT       = (1 << 1),
        DIRTY_LIGHT         = (1 << 2),
        DIRTY_VIEWPORT      = (1 << 3)
    };
    unsigned int _dirtyFlags;

    // Observer location
    Vec3            _eyePosition;
    Vec3            _eyeLookAt;
    Vec3            _eyeOrientation;

    // Ambient light
    Light           _light;

    Mat4            _projectionMatrix;  // Current projection matrix
    GLfloat         _glProjectionMatrix[16];
    Mat4            _viewMatrix;        // Current view matrix
    GLfloat         _glViewMatrix[16];

    int             _windowWidth;
    int             _windowHeight;
    int             _viewportX;
    int             _viewportY;
    int             _viewportWidth;
    int             _viewportHeight;
};

#endif //__AMDTTEAPOTRENDERSTATE_H

