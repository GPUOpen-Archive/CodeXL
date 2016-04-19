//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTFluidGrid.cpp
///
//==================================================================================

//------------------------------ AMDTFluidGrid.cpp ------------------------------

#include <inc/AMDTFluidGrid.h>
#include <inc/AMDTOpenGLHelper.h>

// Vertex indices (into _vertices[] and _tVertices[]). v1[12] and v2[12]
// define the 12 sides of the grid in OpenGL. nSequence[64] gives
// permutations of the vertex indices based on the nearest vertex to
// the eye. See AMDTFluidGrid.h for details.
const int AMDTFluidGrid::v1[12] = { 3, 0, 3, 6, 2, 0, 2, 5, 1, 0, 1, 4 };
const int AMDTFluidGrid::v2[12] = { 4, 3, 6, 7, 6, 2, 5, 7, 5, 1, 4, 7 };
const int AMDTFluidGrid::nSequence[64] =
{
    0, 1, 2, 3, 4, 5, 6, 7,
    1, 4, 5, 0, 3, 7, 2, 6,
    2, 0, 5, 6, 3, 1, 7, 4,
    3, 0, 6, 4, 1, 2, 7, 5,
    4, 3, 7, 1, 0, 6, 5, 2,
    5, 1, 7, 2, 0, 4, 6, 3,
    6, 3, 2, 7, 4, 0, 5, 1,
    7, 4, 6, 5, 1, 3, 2, 0
};

// Farthest vertex index given a closest index [i]
const int AMDTFluidGrid::farIdx[8] = { 7, 6, 4, 5, 2, 3, 1, 0 };

// ---------------------------------------------------------------------------
// Name:        closestPow2
// Description: For a given grid size, get the closest power of two size that
//              is equal to or greater than.
// ---------------------------------------------------------------------------
inline void closestPow2(
    int dim,
    int& outDim,
    int& outShift)
{
    outDim = 32;
    outShift = 5;

    while (outDim < dim)
    {
        outDim <<= 1;
        ++outShift;
    }
}

// ---------------------------------------------------------------------------
// Name:        AMDTFluidGrid::Setup
// Description: Give requested grid size and spacing, deduce power of 2
//              dimensions and calculate othe grid parameters.
// ---------------------------------------------------------------------------
void AMDTFluidGrid::Setup(
    int numCellsX,
    int numCellsY,
    int numCellsZ,
    float spacingMETERS)
{
    closestPow2(numCellsX, _numCellsX, _shiftX);
    closestPow2(numCellsY, _numCellsY, _shiftY);
    closestPow2(numCellsZ, _numCellsZ, _shiftZ);

    _strideShiftY = _shiftX;
    _strideShiftZ = _shiftX + _shiftY;

    _strideY = (1 << _strideShiftY);
    _strideZ = (1 << _strideShiftZ);

    _totalNumCells =
        _numCellsX
        * _numCellsY
        * _numCellsZ;

    _spacingMETERS = spacingMETERS;
    _invSpacingMETERS = 1.0f / _spacingMETERS;

    _maxSlices =
        (int)floor(sqrt((double)(
                            _numCellsX * _numCellsX
                            + _numCellsY * _numCellsY
                            + _numCellsZ * _numCellsZ)) + 1.0f);
}

// ---------------------------------------------------------------------------
// Name:        AMDTFluidGrid::SetModelTransformation
// Description: Set the new grid transformation that positions it in the model.
//              The transformation should not include the view transformation.
// ---------------------------------------------------------------------------
void AMDTFluidGrid::SetModelTransformation(
    const Mat4& mat)
{
    _transf = mat;

    _dirtyVertices = true;
}

// ---------------------------------------------------------------------------
// Name:        AMDTFluidGrid::DrawOutline
// Description: Draw the grid sides using OpenGL. It is assumed that the view
//              transformation has already been applied (but not the model
//              transformation).
// ---------------------------------------------------------------------------
void AMDTFluidGrid::DrawOutline() const
{
    glColor3f(0, 0, 0.8f);

    // 12 Edges
    GLubyte indices[24] = { 0, 1, 1, 4, 4, 7, 1, 5, 0, 3, 3, 6, 6, 7, 3, 4, 0, 2, 2, 5, 5, 7, 2, 6 };

    glPushMatrix();
    {
        GLfloat tMat[16];
        _transf.GetGLMatrix(tMat);

        glMultMatrixf(tMat);

        glEnableClientState(GL_VERTEX_ARRAY);
        {
            glVertexPointer(3, GL_FLOAT, 0, _vertices);
            glDrawElements(GL_LINES, 24, GL_UNSIGNED_BYTE, indices);

            GLfloat base[4 * 3] =
            {
                (_vertices[6]._x + _vertices[2]._x) * 0.5f, _vertices[6]._y, _vertices[6]._z,
                (_vertices[7]._x + _vertices[5]._x) * 0.5f, _vertices[5]._y, _vertices[5]._z,
                _vertices[6]._x, _vertices[6]._y, (_vertices[6]._z + _vertices[7]._z) * 0.5f,
                _vertices[2]._x, _vertices[2]._y, (_vertices[2]._z + _vertices[5]._z) * 0.5f
            };
            glVertexPointer(3, GL_FLOAT, 0, base);
            glDrawArrays(GL_LINES, 0, 4);
        }
        glDisableClientState(GL_VERTEX_ARRAY);
    }
    glPopMatrix();
}

// ---------------------------------------------------------------------------
// Name:        AMDTFluidGrid::GetClosestIndex
// Description: Return the index of the grid vertex that is closest to point
//              v in world space (not eye space).
// ---------------------------------------------------------------------------
int AMDTFluidGrid::GetClosestIndex(
    const Vec3& v)
{
    if (_dirtyVertices)
    {
        updateVertices();
    }

    int closestIdx = 0;
    float closestLen = (_tVertices[0] - v).LengthSqr();
    float tLen;

    for (int i = 1; i < 8; ++i)
    {
        tLen = (_tVertices[i] - v).LengthSqr();

        if (tLen < closestLen)
        {
            closestLen = tLen;
            closestIdx = i;
        }
    }

    return closestIdx;
}

// ---------------------------------------------------------------------------
// Name:        AMDTFluidGrid::GetVertsf
// Description: Copy the eight canonical _vertices into an array.
// ---------------------------------------------------------------------------
void AMDTFluidGrid::GetVertsf(
    float* verts) const
{
    for (int i = 0; i < 8; ++i)
    {
        verts[i * 3 + 0] = _vertices[i]._x;
        verts[i * 3 + 1] = _vertices[i]._y;
        verts[i * 3 + 2] = _vertices[i]._z;
    }
}

// ---------------------------------------------------------------------------
// Name:        AMDTFluidGrid::GetTransformedVertices
// Description: Copy the eight transformed _vertices into an array.
// ---------------------------------------------------------------------------
void AMDTFluidGrid::GetTransformedVertices(
    float* verts)
{
    if (_dirtyVertices)
    {
        updateVertices();
    }

    for (int i = 0; i < 8; ++i)
    {
        *verts++ = _tVertices[i]._x;
        *verts++ = _tVertices[i]._y;
        *verts++ = _tVertices[i]._z;
    }
}