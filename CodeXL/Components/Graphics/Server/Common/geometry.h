//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Helper functions for drawing geometry to the HUD
//==============================================================================

#ifndef GPS_GEOMETRY_H
#define GPS_GEOMETRY_H

#include <vector>
#include "frect.h"

//-----------------------------------------------------------------------------
/// Quad with 2d texture coordinates
//-----------------------------------------------------------------------------
struct SCREEN_VERTEX
{
    /// 4D position of the vertex
    float pos[4];

    /// 2D texture coordinates of the vertex
    float tex[2];

    /// Default Constructor
    SCREEN_VERTEX()
    {
        pos[0] = 0.0;
        pos[1] = 0.0;
        pos[2] = 0.0;
        pos[3] = 0.0;

        tex[0] = 0.0;
        tex[1] = 0.0;
    };

    /// Constructor which allows setting of all values
    SCREEN_VERTEX(float x, float y, float z, float w, float tx, float ty);
};

//---------------------------------------------------------------------------------------------------------------
/// Generates geometry for a Quad with 2d texture coordinates
/// \param fRect Input
/// \return Screen vertex struct
//---------------------------------------------------------------------------------------------------------------
SCREEN_VERTEX* GetQuadGeometry(FRECT fRect);

//-----------------------------------------------------------------------------
/// Quad with 3d texture coordinates
//-----------------------------------------------------------------------------
struct SCREEN_VERTEX_CUBE
{
    /// 4D position of the vertex
    float pos[4];

    /// 3D texture coordinates of the vertex
    float tex[3];

    /// Default Constructor
    SCREEN_VERTEX_CUBE()
    {
        pos[0] = 0.0;
        pos[1] = 0.0;
        pos[2] = 0.0;
        pos[3] = 0.0;

        tex[0] = 0.0;
        tex[1] = 0.0;
        tex[2] = 0.0;
    };

    /// Constructor which allows setting of all values
    SCREEN_VERTEX_CUBE(float x, float y, float z, float w, float tx, float ty, float tz);

    /// Sets the position
    void SetPos(float x, float y, float z);

    /// Sets the texture coordinates
    void SetTextCoord(float tx, float ty, float tz);
};

//-----------------------------------------------------------------------------
/// Vertex with 3 coordinates and a w component
//-----------------------------------------------------------------------------
struct VERTEX_3D
{
    /// Default constructor
    VERTEX_3D()
    {
        pos[0] = 0.0;
        pos[1] = 0.0;
        pos[2] = 0.0;
        pos[3] = 0.0;
    };

    /// Constructor which takes in 4 dimensions
    VERTEX_3D(float x, float y, float z, float w);

    /// Provides a means of setting the position of this vertex
    void SetPos(float x, float y, float z);

    /// Stores the position of this vertex
    float pos[4];
};

//---------------------------------------------------------------------------------------------------------------
/// Generates geometry for a Quad with 2d texture coordinates
/// \param bbmin Input
/// \param bbmax Input
/// \return 3D Vertex struct
//---------------------------------------------------------------------------------------------------------------
VERTEX_3D* GetBBoxGeometry(VERTEX_3D& bbmin, VERTEX_3D& bbmax);

//---------------------------------------------------------------------------------------------------------------
/// Creates the geometry and index buffers needed to render a CubeMap on the HUD
/// \param fRect Input
/// \param PrimitiveCount Input
/// \param IndexData Input
/// \param Vertices Input
//---------------------------------------------------------------------------------------------------------------
void CreateCrossGeometryAndIndex(FRECT fRect, unsigned int& PrimitiveCount, std::vector<int>& IndexData, std::vector<SCREEN_VERTEX_CUBE>& Vertices);

//---------------------------------------------------------------------------------------------------------------
/// Creates the geometry and index buffers needed to render a specific CubeMap face on the HUD
/// \param fRect Input frect
/// \param PrimitiveCount Input prim count
/// \param IndexData Input index data
/// \param Vertices Input vertices
/// \param nFace Input face index
//---------------------------------------------------------------------------------------------------------------
void CreateCubeFaceGeometryAndIndex(FRECT fRect, unsigned int& PrimitiveCount, std::vector<int>& IndexData, std::vector<SCREEN_VERTEX_CUBE>& Vertices, int nFace);

//-----------------------------------------------------------------------------
/// These definitions for face number are currently the same as those for DX9 and DX10
//-----------------------------------------------------------------------------
typedef enum _CUBEMAP_FACES
{
    CUBEMAP_FACE_POSITIVE_X     = 0,
    CUBEMAP_FACE_NEGATIVE_X     = 1,
    CUBEMAP_FACE_POSITIVE_Y     = 2,
    CUBEMAP_FACE_NEGATIVE_Y     = 3,
    CUBEMAP_FACE_POSITIVE_Z     = 4,
    CUBEMAP_FACE_NEGATIVE_Z     = 5,

    CUBEMAP_FACE_FORCE_DWORD    = 0x7fffffff
} CUBEMAP_FACES;

#endif // GPS_GEOMETRY_H