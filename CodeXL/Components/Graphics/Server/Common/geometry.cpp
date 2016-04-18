//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Helper functions for drawing geometry to the HUD
//==============================================================================

#ifdef _LINUX
    #include "SafeCRT.h"
#endif
#include "geometry.h"


SCREEN_VERTEX::SCREEN_VERTEX(float x, float y, float z, float w, float tx, float ty)
{
    pos[0] = x;
    pos[1] = y;
    pos[2] = z;
    pos[3] = w;

    tex[0] = tx;
    tex[1] = ty;
}

//---------------------------------------------------------------------------------------------------------------
// Generates geometry for a Quad with 2d texture coordinates
//---------------------------------------------------------------------------------------------------------------
SCREEN_VERTEX* GetQuadGeometry(FRECT fRect)
{
    static SCREEN_VERTEX svQuad[4];

    float fZ = 0.5f;
    float fA = 1.0f;

    svQuad[0] = SCREEN_VERTEX(fRect.left,  fRect.top,    fZ, fA, 0, 0);
    svQuad[1] = SCREEN_VERTEX(fRect.right, fRect.top,    fZ, fA, 1, 0);
    svQuad[2] = SCREEN_VERTEX(fRect.left,  fRect.bottom, fZ, fA, 0, 1);
    svQuad[3] = SCREEN_VERTEX(fRect.right, fRect.bottom, fZ, fA, 1, 1);

    return svQuad;
}

//---------------------------------------------------------------------------------------------------------------
// Generates geometry for a Quad with 2d texture coordinates
//---------------------------------------------------------------------------------------------------------------
VERTEX_3D* GetBBoxGeometry(VERTEX_3D& bbmin, VERTEX_3D& bbmax)
{
    static VERTEX_3D bboxQuad[16];

    bboxQuad[0] = VERTEX_3D(bbmin.pos[0], bbmin.pos[1], bbmin.pos[2], 1.0f);
    bboxQuad[1] = VERTEX_3D(bbmax.pos[0], bbmin.pos[1], bbmin.pos[2], 1.0f);
    bboxQuad[2] = VERTEX_3D(bbmax.pos[0], bbmax.pos[1], bbmin.pos[2], 1.0f);
    bboxQuad[3] = VERTEX_3D(bbmin.pos[0], bbmax.pos[1], bbmin.pos[2], 1.0f);
    bboxQuad[4] = VERTEX_3D(bbmin.pos[0], bbmin.pos[1], bbmin.pos[2], 1.0f);

    bboxQuad[5] = VERTEX_3D(bbmin.pos[0], bbmin.pos[1], bbmax.pos[2], 1.0f);
    bboxQuad[6] = VERTEX_3D(bbmax.pos[0], bbmin.pos[1], bbmax.pos[2], 1.0f);
    bboxQuad[7] = VERTEX_3D(bbmax.pos[0], bbmax.pos[1], bbmax.pos[2], 1.0f);
    bboxQuad[8] = VERTEX_3D(bbmin.pos[0], bbmax.pos[1], bbmax.pos[2], 1.0f);
    bboxQuad[9] = VERTEX_3D(bbmin.pos[0], bbmin.pos[1], bbmax.pos[2], 1.0f);

    bboxQuad[10] = VERTEX_3D(bbmax.pos[0], bbmin.pos[1], bbmax.pos[2], 1.0f);
    bboxQuad[11] = VERTEX_3D(bbmax.pos[0], bbmin.pos[1], bbmin.pos[2], 1.0f);
    bboxQuad[12] = VERTEX_3D(bbmax.pos[0], bbmax.pos[1], bbmin.pos[2], 1.0f);
    bboxQuad[13] = VERTEX_3D(bbmax.pos[0], bbmax.pos[1], bbmax.pos[2], 1.0f);
    bboxQuad[14] = VERTEX_3D(bbmin.pos[0], bbmax.pos[1], bbmax.pos[2], 1.0f);
    bboxQuad[15] = VERTEX_3D(bbmin.pos[0], bbmax.pos[1], bbmin.pos[2], 1.0f);

    return bboxQuad;
}

SCREEN_VERTEX_CUBE::SCREEN_VERTEX_CUBE(float x, float y, float z, float w, float tx, float ty, float tz)
{
    pos[0] = x;
    pos[1] = y;
    pos[2] = z;
    pos[3] = w;

    tex[0] = tx;
    tex[1] = ty;
    tex[2] = tz;
}

void SCREEN_VERTEX_CUBE::SetPos(float x, float y, float z)
{
    pos[0] = x;
    pos[1] = y;
    pos[2] = z;
    pos[3] = 1.0f;
}

void SCREEN_VERTEX_CUBE::SetTextCoord(float tx, float ty, float tz)
{
    tex[0] = tx;
    tex[1] = ty;
    tex[2] = tz;
}

VERTEX_3D::VERTEX_3D(float x, float y, float z, float w)
{
    pos[0] = x;
    pos[1] = y;
    pos[2] = z;
    pos[3] = w;
}

void VERTEX_3D::SetPos(float x, float y, float z)
{
    pos[0] = x;
    pos[1] = y;
    pos[2] = z;
    pos[3] = 1.0f;
}

//---------------------------------------------------------------------
/// Generates cube map 3D texture coords for rendring a quad showing a cube map face.
/// The texture coordinates represent a vector.
///
/// \param nFace  The face number
/// \param pVertices  the array of vertices to be modified
//---------------------------------------------------------------------
void SetCubemapTextureCoordinates(int nFace, SCREEN_VERTEX_CUBE* pVertices)
{
    switch (nFace)
    {
        case CUBEMAP_FACE_NEGATIVE_X:
            pVertices[0].SetTextCoord(-1.0f,  1.0f, -1.0f);
            pVertices[1].SetTextCoord(-1.0f,  1.0f,  1.0f);
            pVertices[2].SetTextCoord(-1.0f, -1.0f, -1.0f);
            pVertices[3].SetTextCoord(-1.0f, -1.0f,  1.0f);
            break;

        case CUBEMAP_FACE_POSITIVE_Z:
            pVertices[0].SetTextCoord(-1.0f,  1.0f, 1.0f);
            pVertices[1].SetTextCoord(1.0f,  1.0f, 1.0f);
            pVertices[2].SetTextCoord(-1.0f, -1.0f, 1.0f);
            pVertices[3].SetTextCoord(1.0f, -1.0f, 1.0f);
            break;

        case CUBEMAP_FACE_POSITIVE_X:
            pVertices[0].SetTextCoord(1.0f,  1.0f,  1.0f);
            pVertices[1].SetTextCoord(1.0f,  1.0f, -1.0f);
            pVertices[2].SetTextCoord(1.0f, -1.0f,  1.0f);
            pVertices[3].SetTextCoord(1.0f, -1.0f, -1.0f);
            break;

        case CUBEMAP_FACE_NEGATIVE_Z:
            pVertices[0].SetTextCoord(1.0f,  1.0f, -1.0f);
            pVertices[1].SetTextCoord(-1.0f,  1.0f, -1.0f);
            pVertices[2].SetTextCoord(1.0f, -1.0f, -1.0f);
            pVertices[3].SetTextCoord(-1.0f, -1.0f, -1.0f);
            break;

        case CUBEMAP_FACE_POSITIVE_Y:
            pVertices[0].SetTextCoord(-1.0f, 1.0f, -1.0f);
            pVertices[1].SetTextCoord(1.0f, 1.0f, -1.0f);
            pVertices[2].SetTextCoord(-1.0f, 1.0f,  1.0f);
            pVertices[3].SetTextCoord(1.0f, 1.0f,  1.0f);
            break;

        case CUBEMAP_FACE_NEGATIVE_Y:
            pVertices[0].SetTextCoord(-1.0f, -1.0f,  1.0f);
            pVertices[1].SetTextCoord(1.0f, -1.0f,  1.0f);
            pVertices[2].SetTextCoord(-1.0f, -1.0f, -1.0f);
            pVertices[3].SetTextCoord(1.0f, -1.0f, -1.0f);
            break;
    } // End of switch
}

void CreateCubeFaceGeometryAndIndex(FRECT fRect, unsigned int& PrimitiveCount,
                                    std::vector<int>& IndexData, std::vector<SCREEN_VERTEX_CUBE>& Vertices, int nFace)
{
    PrimitiveCount = 2u;
    Vertices.resize(4);
    IndexData.resize(6);

    // Define the vertices of a screen space quad
    Vertices[0].SetPos(fRect.left,                 fRect.top,                  0.5f);
    Vertices[1].SetPos(fRect.left + fRect.Width(), fRect.top,                  0.5f);
    Vertices[2].SetPos(fRect.left,                 fRect.top + fRect.Height(), 0.5f);
    Vertices[3].SetPos(fRect.left + fRect.Width(), fRect.top + fRect.Height(), 0.5f);

    SetCubemapTextureCoordinates(nFace, &Vertices[0]);

    // Define the two triangles in a quad
    int pIndexData[] = { 0, 1, 2, 1, 3, 2 };
    memcpy_s(&IndexData[0], sizeof(pIndexData) / sizeof(int), pIndexData, sizeof(pIndexData));
}

//---------------------------------------------------------------------------------------------------------------
/// Creates the geometry and index buffers needed to render a CubeMap on the HUD
//---------------------------------------------------------------------------------------------------------------
void CreateCrossGeometryAndIndex(FRECT fRect,
                                 unsigned int& PrimitiveCount,
                                 std::vector<int>& IndexData,
                                 std::vector<SCREEN_VERTEX_CUBE>& Vertices)
{
    PrimitiveCount = 12u;
    Vertices.resize(24);
    IndexData.resize(36);

    //         ---------
    //         |       |
    //         |  +Y   |
    //         |       |
    // ---------------------------------
    // |       |       |       |       |
    // |  -X   |  +Z   |  +X   |  -Z   |
    // |       |       |       |       |
    // ---------------------------------
    //         |       |
    //         |  -Y   |
    //         |       |
    //         ---------

    // precompute some variables
    float c1y3 = fRect.Height() / 3.0f;
    float c2y3 = 2.0f * c1y3;

    float c1x4 = fRect.Width() / 4.0f;
    float c2x4 = 2.0f * c1x4;
    float c3x4 = 3.0f * c1x4;

    // -X
    Vertices[ 0].SetPos(fRect.left,        fRect.top + c1y3, 0.5f);
    Vertices[ 1].SetPos(fRect.left + c1x4, fRect.top + c1y3, 0.5f);
    Vertices[ 2].SetPos(fRect.left,        fRect.top + c2y3, 0.5f);
    Vertices[ 3].SetPos(fRect.left + c1x4, fRect.top + c2y3, 0.5f);

    SetCubemapTextureCoordinates(CUBEMAP_FACE_NEGATIVE_X, &Vertices[0]);

    // +Z
    Vertices[ 4].SetPos(fRect.left + c1x4, fRect.top + c1y3, 0.5f);
    Vertices[ 5].SetPos(fRect.left + c2x4, fRect.top + c1y3, 0.5f);
    Vertices[ 6].SetPos(fRect.left + c1x4, fRect.top + c2y3, 0.5f);
    Vertices[ 7].SetPos(fRect.left + c2x4, fRect.top + c2y3, 0.5f);

    SetCubemapTextureCoordinates(CUBEMAP_FACE_POSITIVE_Z, &Vertices[4]);

    // +X
    Vertices[ 8].SetPos(fRect.left + c2x4, fRect.top + c1y3, 0.5f);
    Vertices[ 9].SetPos(fRect.left + c3x4, fRect.top + c1y3, 0.5f);
    Vertices[10].SetPos(fRect.left + c2x4, fRect.top + c2y3, 0.5f);
    Vertices[11].SetPos(fRect.left + c3x4, fRect.top + c2y3, 0.5f);

    SetCubemapTextureCoordinates(CUBEMAP_FACE_POSITIVE_X, &Vertices[8]);

    // -Z
    Vertices[12].SetPos(fRect.left + c3x4,  fRect.top + c1y3, 0.5f);
    Vertices[13].SetPos(fRect.right,        fRect.top + c1y3, 0.5f);
    Vertices[14].SetPos(fRect.left + c3x4,  fRect.top + c2y3, 0.5f);
    Vertices[15].SetPos(fRect.right,        fRect.top + c2y3, 0.5f);

    SetCubemapTextureCoordinates(CUBEMAP_FACE_NEGATIVE_Z, &Vertices[12]);

    // +Y
    Vertices[16].SetPos(fRect.left + c1x4, fRect.top,        0.5f);
    Vertices[17].SetPos(fRect.left + c2x4, fRect.top,        0.5f);
    Vertices[18].SetPos(fRect.left + c1x4, fRect.top + c1y3, 0.5f);
    Vertices[19].SetPos(fRect.left + c2x4, fRect.top + c1y3, 0.5f);

    SetCubemapTextureCoordinates(CUBEMAP_FACE_POSITIVE_Y, &Vertices[16]);

    // -Y
    Vertices[20].SetPos(fRect.left + c1x4, fRect.top + c2y3, 0.5f);
    Vertices[21].SetPos(fRect.left + c2x4, fRect.top + c2y3, 0.5f);
    Vertices[22].SetPos(fRect.left + c1x4, fRect.bottom,        0.5f);
    Vertices[23].SetPos(fRect.left + c2x4, fRect.bottom,        0.5f);

    SetCubemapTextureCoordinates(CUBEMAP_FACE_NEGATIVE_Y, &Vertices[20]);

    int pIndexData[] = { /* -X */ 0,  1,  2,  1,  3,  2, /* +Z */  4,  5,  6,  5,  7,  6, /* +X */  8,  9, 10,  9, 11, 10,
                                  /* -Z */12, 13, 14, 13, 15, 14, /* +Y */ 16, 17, 18, 17, 19, 18, /* -Y */ 20, 21, 22, 21, 23, 22
                       };

    memcpy_s(&IndexData[0],
             IndexData.size() * sizeof(int),   // 36 * 4 = 144
             pIndexData,
             sizeof(pIndexData)   // 36 * 4 = 144
            );

}
