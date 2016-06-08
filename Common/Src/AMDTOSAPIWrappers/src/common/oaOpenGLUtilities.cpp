//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file oaOpenGLUtilities.cpp
///
//=====================================================================

//------------------------------ oaOpenGLUtilities.cpp ------------------------------

// Local:
#include <AMDTOSAPIWrappers/Include/oaOpenGLUtilities.h>


// ---------------------------------------------------------------------------
// Name:        oaMultMatricesd
// Description: Multiplies two 4x4 matrices
//
// Arguments: a - Input matrix, given in OpenGL style.
//            b - Input matrix, given in OpenGL style.
//            r - output matrix, in OpenGL style.
//
// Author:      AMD Developer Tools Team
// Date:        24/6/2009
// ---------------------------------------------------------------------------
void oaMultMatricesd(const GLdouble a[16], const GLdouble b[16], GLdouble r[16])
{
    int i, j;

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            r[i * 4 + j] =
                a[i * 4 + 0] * b[0 * 4 + j] +
                a[i * 4 + 1] * b[1 * 4 + j] +
                a[i * 4 + 2] * b[2 * 4 + j] +
                a[i * 4 + 3] * b[3 * 4 + j];
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        oaInvertMatrixd
// Description: Inverts a 4x4 matrix.
// Arguments: m - The matrix to be inverted, , given in OpenGL style.
//            invOut - Will get the inverted matrix, in OpenGL style.
// Return Val: int - GL_TRUE / GL_FALSE for success / failure.
// Author:      AMD Developer Tools Team
// Date:        24/6/2009
// ---------------------------------------------------------------------------
int oaInvertMatrixd(const GLdouble m[16], GLdouble invOut[16])
{
    double inv[16], det;
    int i;

    // *INDENT-OFF*
    inv[0] =    m[5] * m[10] * m[15] - m[5]  * m[11] * m[14] - m[9]  * m[6] * m[15]
              + m[9] * m[7]  * m[14] + m[13] * m[6]  * m[11] - m[13] * m[7] * m[10];
    inv[4] =  - m[4] * m[10] * m[15] + m[4]  * m[11] * m[14] + m[8]  * m[6] * m[15]
              - m[8] * m[7]  * m[14] - m[12] * m[6]  * m[11] + m[12] * m[7] * m[10];
    inv[8] =    m[4] * m[9]  * m[15] - m[4]  * m[11] * m[13] - m[8]  * m[5] * m[15]
              + m[8] * m[7]  * m[13] + m[12] * m[5]  * m[11] - m[12] * m[7] * m[9];
    inv[12] = - m[4] * m[9]  * m[14] + m[4]  * m[10] * m[13] + m[8]  * m[5] * m[14]
              - m[8] * m[6]  * m[13] - m[12] * m[5]  * m[10] + m[12] * m[6] * m[9];
    inv[1] =  - m[1] * m[10] * m[15] + m[1]  * m[11] * m[14] + m[9]  * m[2] * m[15]
              - m[9] * m[3]  * m[14] - m[13] * m[2]  * m[11] + m[13] * m[3] * m[10];
    inv[5] =    m[0] * m[10] * m[15] - m[0]  * m[11] * m[14] - m[8]  * m[2] * m[15]
              + m[8] * m[3]  * m[14] + m[12] * m[2]  * m[11] - m[12] * m[3] * m[10];
    inv[9] =  - m[0] * m[9]  * m[15] + m[0]  * m[11] * m[13] + m[8]  * m[1] * m[15]
              - m[8] * m[3]  * m[13] - m[12] * m[1]  * m[11] + m[12] * m[3] * m[9];
    inv[13] =   m[0] * m[9]  * m[14] - m[0]  * m[10] * m[13] - m[8]  * m[1] * m[14]
              + m[8] * m[2]  * m[13] + m[12] * m[1]  * m[10] - m[12] * m[2] * m[9];
    inv[2] =    m[1] * m[6]  * m[15] - m[1]  * m[7]  * m[14] - m[5]  * m[2] * m[15]
              + m[5] * m[3]  * m[14] + m[13] * m[2]  * m[7]  - m[13] * m[3] * m[6];
    inv[6] =  - m[0] * m[6]  * m[15] + m[0]  * m[7]  * m[14] + m[4]  * m[2] * m[15]
              - m[4] * m[3]  * m[14] - m[12] * m[2]  * m[7]  + m[12] * m[3] * m[6];
    inv[10] =   m[0] * m[5]  * m[15] - m[0]  * m[7]  * m[13] - m[4]  * m[1] * m[15]
              + m[4] * m[3]  * m[13] + m[12] * m[1]  * m[7]  - m[12] * m[3] * m[5];
    inv[14] = - m[0] * m[5]  * m[14] + m[0]  * m[6]  * m[13] + m[4]  * m[1] * m[14]
              - m[4] * m[2]  * m[13] - m[12] * m[1]  * m[6]  + m[12] * m[2] * m[5];
    inv[3] =  - m[1] * m[6]  * m[11] + m[1]  * m[7]  * m[10] + m[5]  * m[2] * m[11]
              - m[5] * m[3]  * m[10] - m[9]  * m[2]  * m[7]  + m[9]  * m[3] * m[6];
    inv[7] =    m[0] * m[6]  * m[11] - m[0]  * m[7]  * m[10] - m[4]  * m[2] * m[11]
              + m[4] * m[3]  * m[10] + m[8]  * m[2]  * m[7]  - m[8]  * m[3] * m[6];
    inv[11] = - m[0] * m[5]  * m[11] + m[0]  * m[7]  * m[9]  + m[4]  * m[1] * m[11]
              - m[4] * m[3]  * m[9]  - m[8]  * m[1]  * m[7]  + m[8]  * m[3] * m[5];
    inv[15] =   m[0] * m[5]  * m[10] - m[0]  * m[6]  * m[9]  - m[4]  * m[1] * m[10]
              + m[4] * m[2]  * m[9]  + m[8]  * m[1]  * m[6]  - m[8]  * m[2] * m[5];
    // *INDENT-ON*

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0)
    {
        return GL_FALSE;
    }

    det = 1.0 / det;

    for (i = 0; i < 16; i++)
    {
        invOut[i] = inv[i] * det;
    }

    return GL_TRUE;
}


// ---------------------------------------------------------------------------
// Name:        oaMultMatrixVecd
// Description: Implementation gluMultMatrixVecd - multiply 2 4x4 matrices
// Arguments: const GLdouble matrix[16]
//            const GLdouble in[4]
//            GLdouble out[4]
// Return Val: void
// Author:      AMD Developer Tools Team
// Date:        24/6/2009
// ---------------------------------------------------------------------------
void oaMultMatrixVecd(const GLdouble matrix[16], const GLdouble in[4], GLdouble out[4])
{
    int i;

    for (i = 0; i < 4; i++)
    {
        out[i] =
            in[0] * matrix[0 * 4 + i] +
            in[1] * matrix[1 * 4 + i] +
            in[2] * matrix[2 * 4 + i] +
            in[3] * matrix[3 * 4 + i];
    }
}


// ---------------------------------------------------------------------------
// Name:        oaUnProjectPoint
// Description: Maps a specified window coordinates into object coordinates using modelMatrix, projectionMatrix, and viewport.
// Arguments:
//   winX - The window's x axis coordinate to be mapped.
//   winY - The window's y axis coordinate to be mapped.
//   winZ - The window's z axis coordinate to be mapped.
//   modelMatrix - The modelview matrix (as from a glGetDoublev call).
//   projectionMatrix - The projection matrix (as from a glGetDoublev call).
//   viewport - The viewport (as from a glGetIntegerv call).
//   objX - The computed object's x axis coordinate.
//   objY - The computed object's y axis coordinate.
//   objZ - The computed object's z axis coordinate.
//
// Return Val: GLint  - A return value of GL_TRUE indicates success; a return value of GL_FALSE indicates failure.
//
// Author:      AMD Developer Tools Team
// Date:        24/6/2009
// ---------------------------------------------------------------------------
GLint oaUnProjectPoint(GLdouble winx, GLdouble winy, GLdouble winz, const GLdouble modelMatrix[16], const GLdouble projMatrix[16], const GLint viewport[4], GLdouble* objx, GLdouble* objy, GLdouble* objz)
{
    double finalMatrix[16];
    double in[4];
    double out[4];

    oaMultMatricesd(modelMatrix, projMatrix, finalMatrix);

    if (!oaInvertMatrixd(finalMatrix, finalMatrix)) { return (GL_FALSE); }

    in[0] = winx;
    in[1] = winy;
    in[2] = winz;
    in[3] = 1.0;

    /* Map x and y from window coordinates */
    in[0] = (in[0] - viewport[0]) / viewport[2];
    in[1] = (in[1] - viewport[1]) / viewport[3];

    /* Map to range -1 to 1 */
    in[0] = in[0] * 2 - 1;
    in[1] = in[1] * 2 - 1;
    in[2] = in[2] * 2 - 1;

    oaMultMatrixVecd(finalMatrix, in, out);

    if (out[3] == 0.0) { return (GL_FALSE); }

    out[0] /= out[3];
    out[1] /= out[3];
    out[2] /= out[3];
    *objx = out[0];
    *objy = out[1];
    *objz = out[2];
    return (GL_TRUE);
}

