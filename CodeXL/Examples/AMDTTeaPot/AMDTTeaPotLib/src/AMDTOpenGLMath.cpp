//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTOpenGLMath.cpp
///
//==================================================================================

//------------------------------ AMDTOpenGLMath.cpp ----------------------------

#include <cmath>
#include <inc/AMDTOpenGLMath.h>

// static functions
Vec3 Vec3::Mix(
    const Vec3& a,
    const Vec3& b,
    float alpha)
{
    return (a * (1 - alpha) + b * alpha);
}

Vec3 Vec3::FaceForward(
    const Vec3& V,
    const Vec3& I)
{
    Vec3 rV;

    // if V and I point in the same direction, return -V
    if (V * I > 0)
    {
        rV -= V;
    }
    else
    {
        rV = V;
    }

    return rV;
}

Vec3 Vec3::Cross(
    const Vec3& u,
    const Vec3& v)
{
    return Vec3(u._y * v._z - u._z * v._y,  u._z * v._x - u._x * v._z,      u._x * v._y - u._y * v._x);
}

Vec3 Vec3::Normalize(
    const Vec3& v)
{
    Vec3 retVal;
    float invLen = 1.0f / sqrtf(v._x * v._x + v._y * v._y + v._z * v._z);

    if (invLen)
    {
        retVal.Set(v._x * invLen,   v._y * invLen,  v._z * invLen);
    }
    else
    {
        retVal.Set(0, 0, 0);
    }

    return retVal;
}

float Vec3::Det(
    const Vec3& v1,
    const Vec3& v2,
    const Vec3& v3)
{
    // calculate the determinant using Cramer's rule
    float d1 = v1._x * ((v2._y * v3._z) - (v3._y * v2._z));
    float d2 = v2._x * ((v3._y * v1._z) - (v1._y * v3._z));
    float d3 = v3._x * ((v1._y * v2._z) - (v2._y * v1._z));
    return (d1 + d2 + d3);
};

//-------------------------------------------------------
float Vec3::MinComponent() const
{
    float retVal;

    if (_x < _y && _x < _z)
    {
        retVal = _x;
    }
    else if (_y < _z)
    {
        retVal = _y;
    }
    else
    {
        retVal = _z;
    }

    return retVal;
};
float Vec3::MaxComponent() const
{
    float retVal;

    if (_x > _y && _x > _z)
    {
        retVal = _x;
    }
    else if (_y > _z)
    {
        retVal = _y;
    }
    else
    {
        retVal = _z;
    }

    return retVal;
};

float Vec3::MinAbsComponent() const
{
    float retVal;

    if (abs(_x) < abs(_y) && abs(_x) < abs(_z))
    {
        retVal = _x;
    }
    else if (abs(_y) < abs(_z))
    {
        retVal = _y;
    }
    else
    {
        retVal = _z;
    }

    return retVal;
};
float Vec3::MaxAbsComponent() const
{
    float retVal;

    if (abs(_x) > abs(_y) && abs(_x) > abs(_z))
    {
        retVal = _x;
    }
    else if (abs(_y) > abs(_z))
    {
        retVal = _y;
    }
    else
    {
        retVal = _z;
    }

    return retVal;
};


