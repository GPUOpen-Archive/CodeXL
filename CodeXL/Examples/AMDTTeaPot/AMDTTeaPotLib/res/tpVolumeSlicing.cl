//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpVolumeSlicing.cl 
/// 
//==================================================================================

//------------------------------ tpVolumeSlicing.cl ------------------------------

// -----------------------------------------------------------------
//   © 2004 - 2015 Advanced Micro Devices, Inc. All rights reserved.
// -----------------------------------------------------------------
/**********************************************************************************
 *
 * tpVolumeSlicing
 * =================
 * In order to render the 3D smoke density OpenGL texture, we need to calculate
 * planes whose normal faces toward the viewer and that slice through the grid.
 * It can be shown that such a plane can intersect with 3 - 6 sides of the grid.
 * We can thus draw the plane intersection surface with 1 - 4 triangles. The
 * purpose of this kernel is to output the OpenGL triangle coordinates for a given
 * plane. If less than 4 triangles are needed, the remaining triangle vertices
 * are set to 0 (zero-area triangles are automatically culled).
 *
 * The kernel receives receives an array of eight vertices (8 corners of the grid)
 * and an array of vertex indices that defines the 12 sides of the grid. Then,
 * given the distance from the viewer and the direction of the normal, the
 * kernel calculates the intersection with each of the 12 sides and if the
 * intersection lies inside the grid, it outputs a triangle. Intersection of the
 * 12 sides is checked one after the other in the order given by the host.
 * The ordering is such that the points will always define a ccw polygon (or
 * triangle fan). In our case, we convert to discrete triangles so that this
 * same kernel can be used with OpenGL ES systems.
 *
 * In order to always output vertices in ccw, the host first computes the grid
 * vertex that is closest to the viewer. In this way, this kernel is able to use
 * a lookup table that provides the ordering of grid sides based on the closest
 * grid corner. Thus it will always output triangles with the correct ccw.
 *
 * The canonical grid corner vertices are expected to be -0.5 and 0.5. The kernel
 * expects to also receive the grid vertices transformed into model space and
 * the location of the eye in model space. In this way, the output triangle
 * vertices are in model space and only the view transformation is required to
 * draw the slices correctly.
 *
 * The kernel also outputs, for each vertex, the corresponding texture
 * coordinate, assuming that the texture is mapped to the entire grid.
 *
 * The output buffer can be a GL-CL shared VBO. Then the application simply needs
 * to call glDrawArrays.
 *
 **********************************************************************************/

// This structure is identical to the one used by the host. What about endianness?
// It is used to pass constants to the kernels. It is created and initialized once.
typedef struct _VolumeSlicingConstants
{
    // Vertex indices (into verts[] and tVerts[] in the VolumeSlicingParams struct
    // defined below). v1[12] and v2[12] define the 12 sides of the grid in OpenGL.
    // nSequence[64] give permutations of the vertex indices based on the nearest
    // vertex to the eye.
    int v1[12];
    int v2[12];
    int nSequence[64];
} VolumeSlicingConstants;

// This structure is identical to the one used by the host. What about endianness?
// It is passed as a memory object every simulation step.
typedef struct _VolumeSlicingParams
{
    // View vector pointing from the plane towards the eye position.
    float3  view;

    // Canonical vertices of the grid (-0.5 or 0.5)
    float3  verts[8];

    // Grid vertices after model transformation
    float3  tVerts[8];

    // Distance of the starting plane from the origin (model space)
    float   dPlaneStart;

    // Distance between each plane (in model space)
    float   dPlaneIncr;

    // The grid corner that is closest to the viewer.
    int     frontIdx;

    // The number of slices from closest corner to farthest corner.
    int     numSlices;
} VolumeSlicingParams;

// ---------------------------------------------------------------------------
// Name:        kernel: computeIntersection
// Description: Compute triangles that represent one plane of intersection
//              with the fluid grid. There are a maximum of four triangles
//              depending on the orientation of the fluid grid and the slice.
//
//              NOTE: This kernel is not optimized for GPU on account of
//                    branching. It is expected that this kernel will be
//                    run on the CPU while the GPU is performing one cycle
//                    of the smoke simulation.
//
//              Outputs 3 x (3-vertex/3-texture coordinates) for each
//              triangle into the memory object glVertices.
//
// ---------------------------------------------------------------------------
__kernel void computeIntersection(
    __global float* glVertices,
    __constant VolumeSlicingParams* p,
    __constant VolumeSlicingConstants* c)
{
    // We need to accumulate 2 vertices before we can start outputing
    // triangle vertices. position[] and cononPosition[] are where
    // we accumulate the vertices.
    float3 position[3];
    float3 canonPosition[3];

    // Calculate where we are storing the triangles for this slice in the
    // output buffer. (72 = 4 triangles/slice * 3 vertices/triangle * 6 floats/vertex).
    int outIndex = get_global_id(0) * 72;

    // Calculate the slice number (0 = closest slice). Here we want
    // to output slice from back to front since we will be using the
    // OpenGL under blending operator.
    int slice = p->numSlices - 1 - get_global_id(0);

    // Get the index in the nSequence table that gives us the correct permutation
    // of grid sides based on the closest corner to the view.
    int seqIndex = p->frontIdx << 3;

    // Calculate the distance from the model origin to the slice we want to
    // work on in this work-item.
    float dPlaneDist = p->dPlaneStart + (((float)(slice)) * p->dPlaneIncr);

    int pos = 0;
    int totalTriangles = 0;

    // Run over the 12 grid sides and compute intersection point. End when
    // we have outputed 4 triangles.
    for (int sideIndex = 0; (sideIndex < 12) && (totalTriangles < 4); ++sideIndex)
    {
        // Get the two vertices for this side, taking into account the
        // correct permutation of sides based on closest edge.

        int nSeqIdxi = seqIndex + c->v1[ sideIndex ];
        int vertsIdxi = c->nSequence[ nSeqIdxi ];

        int nSeqIdxj = seqIndex + c->v2[ sideIndex ];
        int vertsIdxj = c->nSequence[ nSeqIdxj ];

        // for Edge(i->j)
        float3 Vi = p->tVerts[ vertsIdxi ];
        float3 Vj = p->tVerts[ vertsIdxj ];
        float3 Vjtoi = Vj - Vi;

        float denom = dot(p->view, Vjtoi);

        // denom == 0 => the plane is coplaner with the edge in this case
        // we ignore the intersection.
        if (denom != 0.0f)
        {
            float lambda = (dPlaneDist - dot(p->view, Vi)) / denom;

            // now check intersection (true if lambda=[0,1]
            // Vertex[sideIndex] intersected with this Edge(i->j)
            if (lambda >= 0.0f && lambda <= 1.0f)
            {
                position[pos] = Vi + Vjtoi * lambda;
                canonPosition[pos] = p->verts[ vertsIdxi ] + (p->verts[ vertsIdxj ] - p->verts[ vertsIdxi ]) * lambda + 0.5f;

                if (pos == 2)
                {
                    // Have enough vertices to output a triangle.
                    for (int i = 0; i < 3; ++i)
                    {
                        glVertices[outIndex++] = position[i].x;
                        glVertices[outIndex++] = position[i].y;
                        glVertices[outIndex++] = position[i].z;
                        glVertices[outIndex++] = canonPosition[i].x;
                        glVertices[outIndex++] = canonPosition[i].z;
                        glVertices[outIndex++] = canonPosition[i].y;
                    }

                    // This is a triangle fan, so move the last vertex
                    // to the middle vertex. Then the next intersection
                    // will make the next triangle.
                    position[1] = position[2];
                    canonPosition[1] = canonPosition[2];
                    ++totalTriangles;
                }
                else
                {
                    ++pos;
                }
            }
        }
    }

    // If we outputed less than 4 triangles, fill the rest with zero-area
    // triangles so that they will not be drawn.
    while (totalTriangles++ < 4)
    {
        for (int i = 0; i < 3; ++i)
        {
            glVertices[outIndex++] = 0.0f;
            glVertices[outIndex++] = 0.0f;
            glVertices[outIndex++] = 0.0f;
            glVertices[outIndex++] = 0.0f;
            glVertices[outIndex++] = 0.0f;
            glVertices[outIndex++] = 0.0f;
        }
    }
}