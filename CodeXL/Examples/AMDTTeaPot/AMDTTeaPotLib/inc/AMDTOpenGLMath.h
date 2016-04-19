//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTOpenGLMath.h
///
//==================================================================================

//------------------------------ AMDTOpenGLMath.h ------------------------------

#ifndef __AMDTOPENGLMATH_H
#define __AMDTOPENGLMATH_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <assert.h>


/******************************************************************************
 *
 * rMath
 * -----
 * Class of static members that provides some basic math functionality:
 *
 *     - Convert degrees to radians and visa-versa.
 *     - clamp, max, min.
 *     - sqrt
 *     - random number seeding and generation.
 *
 ******************************************************************************/
#ifndef M_PI
    #define M_PI       3.14159265358979323846
#endif

#define INV_RAND_MAX (1.0f / RAND_MAX)

class rMath
{
public:
    static float Deg2Rad(float deg)
    {
        return (float)(deg * M_PI / 180.0f);
    }
    static float Rad2Deg(float rad)
    {
        return (float)(rad * 180.0f / M_PI);
    }

    static float Clamp(float a, float min, float max)
    {
        float retVal;

        if (a > max)
        {
            retVal = max;
        }
        else if (a < min)
        {
            retVal = min;
        }
        else
        {
            retVal = a;
        }

        return retVal;
    }

    static float Sqrt(float x)
    {
        assert(x >= 0);
        return sqrtf(x);
    }

    static float Max(float x, float y)
    {
        float retVal = x;

        if (x < y)
        {
            retVal = y;
        }

        return retVal;
    }

    static float Min(float x, float y)
    {
        float retVal = x;

        if (x > y)
        {
            retVal = y;
        }

        return retVal;
    }

    // seed the random number generator using current time.
    static void SeedRand()
    {
        srand((unsigned int)time(0));
    }

    // generate a random number in: [0,1]
    static float AbsRandf()
    {
        return (float)rand() * INV_RAND_MAX;
    }

    // generate a random number in: [-1,1]
    static float Randf()
    {
        return 2.0f * ((float)rand() * INV_RAND_MAX) - 1.0f;
    }

    // return a random integer [0, RAND-MAX]
    static int RandI()
    {
        return rand();
    }
};

/******************************************************************************
 *
 * Vec3
 * ----
 * Class for representing a 3-component vector and performing various
 * mathematical operations up and between vectors of the same type.
 *
 ******************************************************************************/
class Vec3
{
public:
    // static functions
    static Vec3     Mix(const Vec3& a, const Vec3& b, float alpha);
    static Vec3     FaceForward(const Vec3& V, const Vec3& I);
    static Vec3     Cross(const Vec3& u, const Vec3& v);
    static Vec3     Normalize(const Vec3& v);

    // calculate the determinant of a 3x3 matrix where the 3 column vectors are provided
    static float    Det(const Vec3& v1, const Vec3& v2, const Vec3& v3);

public:
    Vec3()  {}
    Vec3(float ix, float iy, float iz) : _x(ix), _y(iy), _z(iz) {}
    Vec3(const Vec3& v) : _x(v._x), _y(v._y), _z(v._z)  {}

    void operator= (const Vec3& v);

    // access by index
    float operator[](const int index) const;
    float& operator[](const int index);

    void Set(const float vx, const float vy, const float vz);

    void SetX(const float vx)
    {
        _x = vx;
    };
    void SetY(const float vy)
    {
        _y = vy;
    };
    void SetZ(const float vz)
    {
        _z = vz;
    };

    Vec3 Flip() const;
    Vec3 operator- () const;

    bool operator==(const Vec3& v);

    Vec3 operator+ (const Vec3& v) const;
    Vec3 operator- (const Vec3& v) const;

    float operator*(const Vec3& v) const;   // Dot product

    // scalar multiplication and division
    Vec3 operator*(const float d) const;
    Vec3 operator/ (const float d) const;

    void operator+= (const float d);
    void operator-= (const float d);
    void operator*= (const float d);
    void operator/= (const float d);

    void operator+= (const Vec3& v);
    void operator-= (const Vec3& v);

    void operator*= (const Vec3& v);
    void operator/= (const Vec3& v);

    float Dot(const Vec3& o) const;
    Vec3 Cross(const Vec3& o) const;

    float Length(void) const;
    float LengthSqr(void) const;

    void Normalize(void);

    float MinComponent() const;
    float MaxComponent() const;
    float MinAbsComponent() const;
    float MaxAbsComponent() const;

public:
    float _x, _y, _z;
};

inline void Vec3::operator= (const Vec3& v)
{
    _x = v._x;
    _y = v._y;
    _z = v._z;
}

// access by index
inline float Vec3::operator[](const int index) const
{
    return (&_x)[ index ];
}
inline float& Vec3::operator[](const int index)
{
    return (&_x)[ index ];
}

inline void Vec3::Set(const float vx, const float vy, const float vz)
{
    _x = vx;
    _y = vy;
    _z = vz;
}

inline Vec3 Vec3::Flip() const
{
    return Vec3(-_x, -_y, -_z);
}

inline Vec3 Vec3::operator- () const
{
    return Vec3(-_x, -_y, -_z);
}

inline Vec3 Vec3::operator+ (const Vec3& v) const
{
    return Vec3(_x + v._x, _y + v._y, _z + v._z);
}

inline Vec3 Vec3::operator- (const Vec3& v) const
{
    return Vec3(_x - v._x, _y - v._y, _z - v._z);
}

inline bool Vec3::operator== (const Vec3& v)
{
    bool retVal = false;

    if (_x == v._x && _y == v._y && _z == v._z)
    {
        retVal = true;
    }

    return retVal;
}
// same as Dot product
inline float Vec3::operator*(const Vec3& v) const
{
    return (_x * v._x + _y * v._y + _z * v._z);
}

// scalar multiplication and division
inline Vec3 Vec3::operator*(const float d) const
{
    return Vec3(_x * d, _y * d, _z * d);
}

inline Vec3 Vec3::operator/ (const float d) const
{
    return Vec3(_x / d, _y / d, _z / d);
}

inline void Vec3::operator+= (const float d)
{
    _x += d;
    _y += d;
    _z += d;
}

inline void Vec3::operator-= (const float d)
{
    _x -= d;
    _y -= d;
    _z -= d;
}

inline void Vec3::operator*= (const float d)
{
    _x *= d;
    _y *= d;
    _z *= d;
}

inline void Vec3::operator/= (const float d)
{
    _x /= d;
    _y /= d;
    _z /= d;
}

inline void Vec3::operator+= (const Vec3& v)
{
    _x += v._x;
    _y += v._y;
    _z += v._z;
}

inline void Vec3::operator-= (const Vec3& v)
{
    _x -= v._x;
    _y -= v._y;
    _z -= v._z;
}

inline void Vec3::operator*= (const Vec3& v)
{
    _x *= v._x;
    _y *= v._y;
    _z *= v._z;
}

inline void Vec3::operator/= (const Vec3& v)
{
    _x /= v._x;
    _y /= v._y;
    _z /= v._z;
}

inline float Vec3::Dot(const Vec3& o) const
{
    return (_x * o._x + _y * o._y + _z * o._z);
}

inline Vec3 Vec3::Cross(const Vec3& o) const
{
    return Vec3(_y * o._z - _z * o._y,  _z * o._x - _x * o._z,  _x * o._y - _y * o._x);
}

inline float Vec3::Length(void) const
{
    return rMath::Sqrt(_x * _x + _y * _y + _z * _z);
}
inline float Vec3::LengthSqr(void) const
{
    return (_x * _x + _y * _y + _z * _z);
}

inline void Vec3::Normalize(void)
{
    float invLen = 1.0f / rMath::Sqrt(_x * _x + _y * _y + _z * _z);

    if (invLen)
    {
        _x *= invLen;
        _y *= invLen;
        _z *= invLen;
    }
}


/******************************************************************************
 *
 * Mat4
 * ----
 * Class for representing a 4x4 matrix. Supports matrix operations, conversion
 * to and from OpenGL matrices, creating OpenGL matrix transformations such
 * as scaling, rotation, translation and perspective. Provides matrix
 * mathematical operations, including operations on Vec3 vectors.
 *
 * The order of application is the same as OpenGL. Thus, assume that
 * we want to create a matrix that first applies a translation Ty, then a scale
 * Sz, then we would create it like this:
 *
 *     _m.Identity();
 *     _m.Translate(0, Ty, 0);
 *     _m.Scale(0, 0, Sz);
 *
 * Similarly, assume that we have a matrix for translation T and a matrix for
 * scaling S and we want to apply T before S, then the combined matrix would
 * be given by:
 *
 *     T * S
 *
 ******************************************************************************/
class Mat4
{
public:

    Mat4(void);
    Mat4(bool identity);   // 0=zero, 1=identity
    Mat4(const float r00, const float r01, const float r02, const float r03,
         const float r10, const float r11, const float r12, const float r13,
         const float r20, const float r21, const float r22, const float r23,
         const float r30, const float r31, const float r32, const float r33);

    // OPERATORS

    // Get an element from the matrix.
    float           operator()(const unsigned int i, const unsigned int j) const;

    // scalar multiplication, creating a new matrix instance
    Mat4            operator*(const float f) const;

    // vector 3 multiplication (assumes that w = 1), creating a new Vec3 instance
    Vec3            operator*(const Vec3& v) const;

    // matrix multiplcation, creating a new matrix instance
    Mat4            operator*(const Mat4& b) const;

    // scalar multiplication, modifying this instance
    Mat4&           operator*=(const float f);

    // matrix multiplication, modifying this instance.
    Mat4&           operator*=(const Mat4& b);

    // FUNCTIONS

    // Create a matrix instance from the GL matrix array of floats (as returned
    // by the glGetFloatv(GL_MODELVIEW_MATRIX)).
    static Mat4     FromGLMatrix(const float* mgl);

    // set all elements to zero.
    void            Zero(void);

    // create the identiy matrix.
    void            Identity(void);

    // return the transpose of this matrix.
    Mat4            Transpose(void) const;

    // Setup translation elements in the matrix, overriding the existing
    // translation.
    void            SetTranslation(const float x, const float y, const float z);

    // Setup a perspective matrix.
    void            SetPerspective(const float fov, const float aspect, const float zNear, const float zFar);

    // Setup a frustum matrix.
    void            SetFrustum(const float l, const float r, const float b, const float t, const float n, const float f);

    // Setup a look-at view matrix.
    void            LookAt(const Vec3& position, const Vec3& center, const Vec3& up);

    // Set the scale elements of the matrix.
    void            SetScale(const float sx, const float sy, const float sz);

    // Set all scale elements of the matrix to be the same scaling factor.
    void            SetScaleUniform(const float s);

    // Fill an 1D array of floats in col-major representation (i.e. can be
    // passed to glLoadMatrix and glMultMatrix).
    void            GetGLMatrix(float* mgl) const;

    // Fill a 1D array of floats in row-major representation.
    void            Get1DMatrix(float* mgl) const;

    // Apply matrix to Vec3, creating a new Vec3.
    Vec3            TransformVec(const Vec3& v) const;
    Vec3            TransformVec3(const Vec3& v) const;

    // Apply a translation to the existing matrix.
    void            Translate(const float x, const float y, const float z);

    // Apply scaling to the existing matrix.
    void            Scale(const float sx, const float sy, const float sz);

    // Apply scaling to the existing matrix.
    void            ScaleUniform(const float f);

    // Apply a rotation (angle in radians) about the X to the existing matrix.
    void            RotateX(float angle);

    // Apply a rotation (angle in radians) about the Y to the existing matrix.
    void            RotateY(float angle);

    // Apply a rotation (angle in radians) about the Z to the existing matrix.
    void            RotateZ(float angle);

    // Set all elements of the matrix to val.
    void            Set(const int i, const int j, const float val);

private:
    // The elements of the matrix.
    float _m[4][4];
};

inline Mat4::Mat4(void)
{
    // do nothing here, if we need a zero or identity matrix we shall use the functions
}

inline Mat4::Mat4(bool identity)
{
    if (identity)
    {
        Identity();
    }
    else
    {
        Zero();
    }
}

inline Mat4::Mat4(const float r00, const float r01, const float r02, const float r03,
                  const float r10, const float r11, const float r12, const float r13,
                  const float r20, const float r21, const float r22, const float r23,
                  const float r30, const float r31, const float r32, const float r33)
{
    _m[0][0] = r00;
    _m[0][1] = r01;
    _m[0][2] = r02;
    _m[0][3] = r03;
    _m[1][0] = r10;
    _m[1][1] = r11;
    _m[1][2] = r12;
    _m[1][3] = r13;
    _m[2][0] = r20;
    _m[2][1] = r21;
    _m[2][2] = r22;
    _m[2][3] = r23;
    _m[3][0] = r30;
    _m[3][1] = r31;
    _m[3][2] = r32;
    _m[3][3] = r33;
}

inline void Mat4::Zero()
{
    _m[0][0] = 0;
    _m[0][1] = 0;
    _m[0][2] = 0;
    _m[0][3] = 0;
    _m[1][0] = 0;
    _m[1][1] = 0;
    _m[1][2] = 0;
    _m[1][3] = 0;
    _m[2][0] = 0;
    _m[2][1] = 0;
    _m[2][2] = 0;
    _m[2][3] = 0;
    _m[3][0] = 0;
    _m[3][1] = 0;
    _m[3][2] = 0;
    _m[3][3] = 0;
}

inline void Mat4::Identity()
{
    _m[0][0] = 1;
    _m[0][1] = 0;
    _m[0][2] = 0;
    _m[0][3] = 0;
    _m[1][0] = 0;
    _m[1][1] = 1;
    _m[1][2] = 0;
    _m[1][3] = 0;
    _m[2][0] = 0;
    _m[2][1] = 0;
    _m[2][2] = 1;
    _m[2][3] = 0;
    _m[3][0] = 0;
    _m[3][1] = 0;
    _m[3][2] = 0;
    _m[3][3] = 1;
}

inline float Mat4::operator()(const unsigned int i, const unsigned int j) const
{
    return _m[i][j];
}


inline  Mat4 Mat4::operator*(const float f) const
{
    Mat4 mf;

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            mf._m[i][j] = _m[i][j] * f;
        }
    }

    return mf;
}

inline  Mat4    Mat4::operator*(const Mat4& b) const
{
    Mat4 ab;
    ab.Zero();

    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            for (int i = 0; i < 4; i++)
            {
                ab._m[r][c] += _m[r][i] * b._m[i][c];
            }
        }
    }

    return ab;
}

inline  Mat4& Mat4::operator*=(const float f)
{
    *this = (*this) * f;
    return *this;
}

inline  Mat4&   Mat4::operator*=(const Mat4& b)
{
    *this = (*this) * b;
    return *this;
}

inline Vec3 Mat4::operator*(const Vec3& v) const
{
    Vec3 tv;
    tv._x = _m[0][0] * v._x + _m[0][1] * v._y + _m[0][2] * v._z + _m[0][3]; // assuming v.w = 1 (w)
    tv._y = _m[1][0] * v._x + _m[1][1] * v._y + _m[1][2] * v._z + _m[1][3];
    tv._z = _m[2][0] * v._x + _m[2][1] * v._y + _m[2][2] * v._z + _m[2][3];
    return tv;
}

inline  Mat4    Mat4::Transpose(void) const
{
    Mat4 t;

    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            t._m[r][c] = _m[c][r];
        }
    }

    return t;

}

inline void Mat4::SetTranslation(const float x, const float y, const float z)
{
    _m[0][3] = x;
    _m[1][3] = y;
    _m[2][3] = z;
}

inline void Mat4::SetScale(const float sx, const float sy, const float sz)
{
    // set the diagonal values
    _m[0][0] = sx;
    _m[1][1] = sy;
    _m[2][2] = sz;
}

inline void Mat4::Translate(const float x, const float y, const float z)
{
    _m[0][3] += x;
    _m[1][3] += y;
    _m[2][3] += z;
}

inline void Mat4::Scale(const float sx, const float sy, const float sz)
{
    Mat4 sm;
    sm.Zero();
    sm._m[0][0] = sx;
    sm._m[1][1] = sy;
    sm._m[2][2] = sz;
    sm._m[3][3] = 1.0f;

    (*this) = sm * (*this);
}

inline void Mat4::ScaleUniform(const float f)
{
    Mat4 sm;
    sm.Zero();
    sm._m[0][0] = f;
    sm._m[1][1] = f;
    sm._m[2][2] = f;
    sm._m[3][3] = 1.0f;

    (*this) = sm * (*this);
}

inline void Mat4::SetScaleUniform(const float s)
{
    // set the diagonal values
    _m[0][0] = s;
    _m[1][1] = s;
    _m[2][2] = s;
}

inline void Mat4::RotateX(float angle)
{
    // angle is in radians
    Mat4 rot;
    rot.Identity();

    float cosT = static_cast<float>(cos(angle));
    float sinT = static_cast<float>(sin(angle));

    rot._m[1][1] =  cosT;
    rot._m[1][2] = -sinT;
    rot._m[2][1] =  sinT;
    rot._m[2][2] =  cosT;

    (*this) *= rot;
}

inline void Mat4::RotateY(float angle)
{
    // angle is in radians
    Mat4 rot;
    rot.Identity();

    float cosT = static_cast<float>(cos(angle));
    float sinT = static_cast<float>(sin(angle));

    rot._m[0][0] =  cosT;
    rot._m[0][2] =  sinT;
    rot._m[2][0] = -sinT;
    rot._m[2][2] =  cosT;

    (*this) *= rot;
}

inline void Mat4::RotateZ(float angle)
{
    // angle is in radians
    Mat4 rot;
    rot.Identity();

    float cosT = static_cast<float>(cos(angle));
    float sinT = static_cast<float>(sin(angle));

    rot._m[0][0] =  cosT;
    rot._m[0][1] = -sinT;
    rot._m[1][0] =  sinT;
    rot._m[1][1] =  cosT;

    (*this) *= rot;
}

inline void Mat4::SetPerspective(const float fov, const float aspect,
                                 const float zNear, const float zFar)
{
    // using double for a little more accuracy
    double f = 1.0 / (tan((double)(fov * (M_PI / 180.0f)) / 2));
    _m[0][0] = (float)(f / (double)aspect);
    _m[1][1] = (float)f;
    _m[2][2] = (float)(((double)zFar + (double)zNear) / -((double)zNear - (double)zFar));
    _m[2][3] = (float)((2 * (double)zFar * (double)zNear) / -((double)zNear - (double)zFar));
    _m[3][2] = -1.0f;
}

inline void Mat4::SetFrustum(
    const float l,
    const float r,
    const float b,
    const float t,
    const float n,
    const float f)
{
    double inv_width = (r - l != 0.0f) ? 1.0 / (double)(r - l) : 0;
    double inv_height = (t - b != 0.0f) ? 1.0 / (double)(t - b) : 0;
    double inv_depth = (f - n != 0.0f) ? 1.0 / (double)(f - n) : 0;

    double two_n = ((double)n) * 2.0;

    _m[0][0] = (float)(two_n * inv_width);
    _m[0][2] = (float)((double)r + ((double)l) * inv_width);

    _m[1][1] = (float)(two_n * inv_height);
    _m[1][2] = (float)((double)t + ((double)b) * inv_height);

    _m[2][2] = (float)((double)(-f - n) * inv_depth);
    _m[2][3] = (float)((-two_n * inv_depth) * ((double)f));

    _m[3][2] = -1.0;
    _m[3][3] = 0.0;
}

inline void Mat4::LookAt(const Vec3& eyePosition, const Vec3& center, const Vec3& up)
{
    Vec3 forward = center - eyePosition;
    forward.Normalize();

    //Side = forward _x up
    Vec3 side = Vec3::Cross(forward, up);
    side.Normalize();

    //Recompute up as: up = side _x forward
    Vec3 upVector = Vec3::Cross(side, forward);

    Mat4 lm;

    lm._m[0][0] = side[0];
    lm._m[0][1] = side[1];
    lm._m[0][2] = side[2];
    lm._m[0][3] = 0.0f;

    lm._m[1][0] = upVector[0];
    lm._m[1][1] = upVector[1];
    lm._m[1][2] = upVector[2];
    lm._m[1][3] = 0.0f;

    lm._m[2][0] = -forward[0];
    lm._m[2][1] = -forward[1];
    lm._m[2][2] = -forward[2];
    lm._m[2][3] = 0.0f;

    lm._m[3][0] = 0.0f;
    lm._m[3][1] = 0.0f;
    lm._m[3][2] = 0.0f;
    lm._m[3][3] = 1.0f;

    (*this) *= lm;

    _m[0][3] -= eyePosition[0];
    _m[1][3] -= eyePosition[1];
    _m[2][3] -= eyePosition[2];
}

inline void Mat4::GetGLMatrix(float* mgl) const   // col-major
{
    mgl[ 0] = _m[0][0];
    mgl[ 1] = _m[1][0];
    mgl[ 2] = _m[2][0];
    mgl[ 3] = _m[3][0];

    mgl[ 4] = _m[0][1];
    mgl[ 5] = _m[1][1];
    mgl[ 6] = _m[2][1];
    mgl[ 7] = _m[3][1];

    mgl[ 8] = _m[0][2];
    mgl[ 9] = _m[1][2];
    mgl[10] = _m[2][2];
    mgl[11] = _m[3][2];

    mgl[12] = _m[0][3];
    mgl[13] = _m[1][3];
    mgl[14] = _m[2][3];
    mgl[15] = _m[3][3];
}

inline void Mat4::Get1DMatrix(float* mgl) const   // row-major
{
    // row 0
    mgl[ 0] = _m[0][0];
    mgl[ 1] = _m[0][1];
    mgl[ 2] = _m[0][2];
    mgl[ 3] = _m[0][3];

    // row 1
    mgl[ 4] = _m[1][0];
    mgl[ 5] = _m[1][1];
    mgl[ 6] = _m[1][2];
    mgl[ 7] = _m[1][3];

    // row 2
    mgl[ 8] = _m[2][0];
    mgl[ 9] = _m[2][1];
    mgl[10] = _m[2][2];
    mgl[11] = _m[2][3];

    // row 3
    mgl[12] = _m[3][0];
    mgl[13] = _m[3][1];
    mgl[14] = _m[3][2];
    mgl[15] = _m[3][3];
}

// create a matrix from a gl formatted matrix with 16 elements (4x4 column-major)
inline Mat4 Mat4::FromGLMatrix(const float* mgl)
{
    Mat4 mat;
    // col 0
    mat._m[0][0] = mgl[ 0];
    mat._m[1][0] = mgl[ 1];
    mat._m[2][0] = mgl[ 2];
    mat._m[3][0] = mgl[ 3];

    // col 1
    mat._m[0][1] = mgl[ 4];
    mat._m[1][1] = mgl[ 5];
    mat._m[2][1] = mgl[ 6];
    mat._m[3][1] = mgl[ 7];

    // col 2
    mat._m[0][2] = mgl[ 8];
    mat._m[1][2] = mgl[ 9];
    mat._m[2][2] = mgl[10];
    mat._m[3][2] = mgl[11];

    // col 3
    mat._m[0][3] = mgl[12];
    mat._m[1][3] = mgl[13];
    mat._m[2][3] = mgl[14];
    mat._m[3][3] = mgl[15];

    return mat;
}

inline Vec3 Mat4::TransformVec(const Vec3& v) const
{
    Vec3 tv;
    tv._x = _m[0][0] * v._x + _m[0][1] * v._y + _m[0][2] * v._z + _m[0][3]; // assuming v.w = 1 (w)
    tv._y = _m[1][0] * v._x + _m[1][1] * v._y + _m[1][2] * v._z + _m[1][3];
    tv._z = _m[2][0] * v._x + _m[2][1] * v._y + _m[2][2] * v._z + _m[2][3];
    return tv;
}

inline Vec3 Mat4::TransformVec3(const Vec3& v) const
{
    Vec3 tv;
    tv._x = _m[0][0] * v._x + _m[0][1] * v._y + _m[0][2] * v._z;
    tv._y = _m[1][0] * v._x + _m[1][1] * v._y + _m[1][2] * v._z;
    tv._z = _m[2][0] * v._x + _m[2][1] * v._y + _m[2][2] * v._z;
    return tv;
}

inline void Mat4::Set(const int i, const int j, const float val)
{
    assert(i < 4);
    assert(j < 4);
    _m[i][j] = val;
}


#endif /* __AMDTOPENGLMATH_H */
