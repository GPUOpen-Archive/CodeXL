//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTFluidGrid.h
///
//==================================================================================

//------------------------------ AMDTFluidGrid.h ------------------------------

#ifndef __AMDTFLUIDGRID_H
#define __AMDTFLUIDGRID_H

#include <inc/AMDTOpenGLMath.h>

class AMDTTeapotOCLSmokeSystem;

/******************************************************************************
 *
 * AMDTFluidGrid
 * -----------
 *
 * This class maintains the 3D fluid grid dimensions - number of cells in
 * _x, _y, _z directions, the cell spacing in meters.
 *
 *
 * The _x axis corresponds to the OpenGL _x access, however the _y-_z axes of the
 * the grid are exchanged. Thus the positive _z-axis of the grid is the up
 * direction (away from gravity).
 *
 * The OpenGL coordinates of the grid are -0.5 -> 0.5 on all sides. The _vertices
 * have the following index order:
 *
 *              4--1
 *             /|  |
 *            / 7--5
 *           /    /
 *          /    /
 *         3--0 /
 *         |  |/
 *         6--2
 *
 * This class also provides the tables defining the 12 sides of the grid as
 * defined in static members v1[12] and v2[12]. The ordering of the sides is
 * important to the volume slicing kernel which uses these vectors to determine
 * the plane intersection points along each side of the cube. The ordering of
 * the vectors in v1[12] to v2[12] is such that the intersection points will
 * produce a ccw polygon.
 *
 * This class also provides the premutation table used by the volume slicing
 * OpenCL kernel (tpVolumeslicing.cl). This table modifies the side ordering
 * based on which vertex is closest to the eye, thus ensuring that for any
 * orientation of the grid in the world space, the resulting order of the
 * intersection points will give a ccw polygon.
 *
 ******************************************************************************/
class AMDTFluidGrid
{
    friend class AMDTTeapotOCLSmokeSystem;
private:
    // _z is height (= _y in OpenGL coordinates)
    // Grid dimensions (_numCellsX = 1<<_shiftX)
    int _numCellsX;
    int _numCellsY;
    int _numCellsZ;
    int _shiftX;
    int _shiftY;
    int _shiftZ;
    // The stride to move in _y and _z plus corresponding shift
    int _strideY;
    int _strideShiftY;
    int _strideZ;
    int _strideShiftZ;
    // _totalNumCells = _numCellsX * _numCellsY * _numCellsZ
    int _totalNumCells;
    // The maximum number of volume slices to render
    int _maxSlices;
    // Spacing in meters between cells.
    float _spacingMETERS;
    float _invSpacingMETERS;

    // The eight OpenGL _vertices of the grid. These are the canonical
    // _vertices of a -0.5 -> 0.5 cube. These don't change.
    Vec3    _vertices[8];

    // The OpenGL model transformation matrix required to position the
    // grid on the model. This must not include the view transformatino.
    Mat4    _transf;

    // The grid _vertices transformed by the model transformation.
    Vec3    _tVertices[8];

    // When this flag is true, we need to recompute _tVertices (apply
    // model transformation to the canonical cube).
    bool    _dirtyVertices;

    // Apply model transformation to the canonical cube.
    void updateVertices()
    {
        for (int i = 7; i >= 0; --i)
        {
            _tVertices[i] = _transf.TransformVec(_vertices[i]);
        }

        _dirtyVertices = false;
    }

public:

    AMDTFluidGrid()
    {
        // generate canonical _vertices
        _transf.Identity();

        Vec3 d(0.5f, 0.5f, 0.5f);
        _vertices[0].Set(+d._x, +d._y, +d._z);
        _vertices[1].Set(+d._x, +d._y, -d._z);
        _vertices[2].Set(+d._x, -d._y, +d._z);
        _vertices[3].Set(-d._x, +d._y, +d._z);

        _vertices[4].Set(-d._x, +d._y, -d._z);
        _vertices[5].Set(+d._x, -d._y, -d._z);
        _vertices[6].Set(-d._x, -d._y, +d._z);
        _vertices[7].Set(-d._x, -d._y, -d._z);

        _dirtyVertices = true;
    }
    ~AMDTFluidGrid()
    {
    }

    // Change the grid dimensions and cell spacing.
    void    Setup(int numCellsX, int numCellsY, int numCellsZ, float spacingMETERS);

    // Get one of the grid _vertices (transformed to world space).
    Vec3    GetVertex(const int idx);

    // Get all the grid _vertices (transformed to world space).
    void    GetTransformedVertices(float* verts);

    // Set the new model transformation.
    void    SetModelTransformation(const Mat4& mat);

    // Draw the grid. The OpenGL modelview matrix should contain the
    // view transformation (not the model transformation).
    void    DrawOutline() const;

    // Given a point v in world space, find the closest grid vertex index
    // (see diagram above of _vertices).
    int     GetClosestIndex(const Vec3& v);

    // Get the canonical grid coordinates (-0.5 to 0.5 in each dimension).
    void    GetVertsf(float* verts) const;

    // Vertex indices (into _vertices[] and _tVertices[]). v1[12] and v2[12]
    // define the 12 sides of the grid in OpenGL. nSequence[64] gives
    // permutations of the vertex indices based on the nearest vertex to
    // the eye.
    static const int v1[12];
    static const int v2[12];
    static const int nSequence[64];

    // Farthest vertex index given a closest index [i]
    static const int farIdx[8];
};

// This is used to return one of the grid _vertices, transformed into
// world space.
inline Vec3 AMDTFluidGrid::GetVertex(const int idx)
{
    assert(idx >= 0 && idx < 8);

    if (_dirtyVertices)
    {
        updateVertices();
    }

    return _tVertices[idx];
}

#endif /* __AMDTFLUIDGRID_H */