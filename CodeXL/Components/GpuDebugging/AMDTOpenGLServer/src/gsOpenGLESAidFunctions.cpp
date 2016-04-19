//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsOpenGLESAidFunctions.cpp
///
//==================================================================================

//------------------------------ gsOpenGLESAidFunctions.cpp ------------------------------

// Standard C:
#include <math.h>


// Infra:
#include <AMDTOSWrappers/osOpenGLIncludes.h>
#include <AMDTAPIClasses/Include/ap2DRectangle.h>

// Local:
#include <inc/gsMonitoredFunctionPointers.h>
#include <inc/gsWrappersCommon.h>
#include <inc/gsOpenGLMonitor.h>


// ---------------------------------------------------------------------------
// Name:        gsSwapFloats
// Description: Swaps two float values.
// Author:      Yaki Tebeka
// Date:        9/4/2006
// ---------------------------------------------------------------------------
void gsSwapFloats(GLfloat* floatA, GLfloat* floatB)
{
    GLfloat tempFloat = *floatA;
    *floatA = *floatB;
    *floatB = tempFloat;
}


// ---------------------------------------------------------------------------
// Name:        gsGetCurrent2DTextureCropRectangle
// Description: Retrieves the current 2D bind texture crop rectangle.
// Arguments: rectangle - The crop rectangle.
//            (u1, v1), (u2, v2) - The crop rectangle in U,V texture space.
//
// Return Val: bool - Success / Failure.
// Author:      Yaki Tebeka
// Date:        10/4/2006
// ---------------------------------------------------------------------------
bool gsGetCurrent2DTextureCropRectangle(ap2DRectangle& rectangle, float& u1, float& v1, float& u2, float& v2)
{
    bool retVal = false;

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();
    GT_IF_WITH_ASSERT(pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the current 2D texture crop rectangle:
        gsTexturesMonitor* texturesMon = pCurrentThreadRenderContextMonitor->texturesMonitor();

        GT_IF_WITH_ASSERT(texturesMon != NULL)
        {
            // Get the current texture unit bind 2D texture name:
            int activeTexUnitIndex = pCurrentThreadRenderContextMonitor->activeTextureUnitIndex();
            GLuint bind2DTextureName = pCurrentThreadRenderContextMonitor->bindTextureName(activeTexUnitIndex, AP_2D_TEXTURE);

            // If there is a bound 2D texture:
            if (bind2DTextureName != 0)
            {
                // Get an object that represents this texture:
                const apGLTexture* pTexParamObj = texturesMon->getTextureObjectDetails(bind2DTextureName);

                if (pTexParamObj)
                {
                    // Get its crop rectangle:
                    bool rc = pTexParamObj->getCropRectangle(rectangle);

                    if (rc)
                    {
                        // Get its size:
                        GLsizei texWidth, texHeight, texDepth, texBorderWidth;
                        pTexParamObj->getDimensions(texWidth, texHeight, texDepth, texBorderWidth);

                        // Calculate its getDimensions in UV space:
                        u1 = rectangle._xPos / texWidth;
                        v1 = rectangle._yPos / texHeight;
                        u2 = (float)(rectangle._xPos + fabs(rectangle._width)) / (float)texWidth;
                        v2 = (float)(rectangle._yPos + fabs(rectangle._height)) / (float)texHeight;

                        retVal = true;
                    }
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsCalculateQuadArraysForDrawTexfOES
// Description:
//   Calculates the vertex and texture coordinate arrays that draws
//   the quad, used by gsDrawTexfOES.
//
// Arguments: x, y, z - Rectangle origin, given in viewport coordinates.
//            width, height - Rectangle size, measured in screen rectangles.
//            verticesArray - Buffer, will get the quad vertices coordinates 4 * (x, y, z):
//            textureCoordsArray - Buffer, will get the quad texture coordinates 4 * (U,V):
//
// Return Val: bool - Success / Failure.
//
// Author:      Yaki Tebeka
// Date:        10/4/2006
// ---------------------------------------------------------------------------
bool gsCalculateQuadArraysForDrawTexfOES(GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height,
                                         GLfloat* verticesArray, GLfloat* textureCoordsArray)
{
    bool retVal = false;

    // Get the current 2D texture crop rectangle:
    ap2DRectangle cropRect;
    float cropRectU1 = 0;
    float cropRectV1 = 0;
    float cropRectU2 = 0;
    float cropRectV2 = 0;
    bool rc = gsGetCurrent2DTextureCropRectangle(cropRect, cropRectU1, cropRectV1, cropRectU2, cropRectV2);

    // Retrieve the current view-port size:
    GLint viewportDimensions[4];
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);
    gs_stat_realFunctionPointers.glGetIntegerv(GL_VIEWPORT, viewportDimensions);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);
    GLfloat viewPortWidth = (GLfloat)viewportDimensions[2];
    GLfloat viewPortHeight = (GLfloat)viewportDimensions[3];

    // Calculate the quad vertices positions:
    GLfloat rx1 = 2.0f * x / viewPortWidth - 1.0f;
    GLfloat ry1 = 2.0f * y / viewPortHeight - 1.0f;
    GLfloat rx2 = 2.0f * (x + width) / viewPortWidth - 1.0f;
    GLfloat ry2 = 2.0f * (y + height) / viewPortHeight - 1.0f;

    z = z * 2.0f - 1;

    if (z < -1.0) { z = -1.0; };

    if (z > +1.0) { z = +1.0; };

    // Initialize the buffer, containing the quad vertices coordinates (x, y, z):
    verticesArray[0] = rx1;

    verticesArray[1] = ry1;

    verticesArray[2] = z;

    verticesArray[3] = rx1;

    verticesArray[4] = ry2;

    verticesArray[5] = z;

    verticesArray[6] = rx2;

    verticesArray[7] = ry1;

    verticesArray[8] = z;

    verticesArray[9] = rx2;

    verticesArray[10] = ry2;

    verticesArray[11] = z;

    // Initialize the buffer, containing the quad texture coordinates (U,V):
    textureCoordsArray[0] = cropRectU1;

    textureCoordsArray[1] = cropRectV1;

    textureCoordsArray[2] = cropRectU1;

    textureCoordsArray[3] = cropRectV2;

    textureCoordsArray[4] = cropRectU2;

    textureCoordsArray[5] = cropRectV1;

    textureCoordsArray[6] = cropRectU2;

    textureCoordsArray[7] = cropRectV2;

    // Handle negative width and height: invert the image (see spec):
    if (cropRect._width < 0)
    {
        GLfloat ofs = (GLfloat)width / (GLfloat)viewPortWidth;
        verticesArray[0 * 3 + 0] -= ofs;
        verticesArray[1 * 3 + 0] -= ofs;
        verticesArray[2 * 3 + 0] -= ofs;
        verticesArray[3 * 3 + 0] -= ofs;
        gsSwapFloats(&textureCoordsArray[0 * 2 + 0], &textureCoordsArray[2 * 2 + 0]);
        gsSwapFloats(&textureCoordsArray[1 * 2 + 0], &textureCoordsArray[3 * 2 + 0]);
    }

    if (cropRect._height < 0)
    {
        GLfloat ofs = (GLfloat)height / (GLfloat)viewPortHeight;
        verticesArray[0 * 3 + 1] -= ofs;
        verticesArray[1 * 3 + 1] -= ofs;
        verticesArray[2 * 3 + 1] -= ofs;
        verticesArray[3 * 3 + 1] -= ofs;
        gsSwapFloats(&textureCoordsArray[0 * 2 + 1], &textureCoordsArray[1 * 2 + 1]);
        gsSwapFloats(&textureCoordsArray[2 * 2 + 1], &textureCoordsArray[3 * 2 + 1]);
    }

    retVal = rc;
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsDrawTexfOES
// Description:
//  Implements glDrawTexXXOES functions (OES_draw_texture extension).
//  Draws a texture rectangle to the screen.
//
// Arguments: x, y, z - Rectangle origin, given in viewport coordinates.
//            width, height - Rectangle size, measured in screen rectangles.
//
// Author:      Yaki Tebeka
// Date:        9/4/2006
//
// Implementation notes:
//   To immitate this extension function, we draw a quad in screen coordinates,
//   covered by the current texture rectangle image.
// ---------------------------------------------------------------------------
void gsDrawTexfOES(GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height)
{
    // Buffer, containing the quad vertices coordinates (x, y, z):
    GLfloat verticesArray[12];

    // Buffer, containing the quad texture coordinates (U,V):
    GLfloat textureCoordsArray[8];

    // Calculate the vertices and texture coordinates:
    bool rc = gsCalculateQuadArraysForDrawTexfOES(x, y, z, width, height, verticesArray, textureCoordsArray);

    if (rc)
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glDisable);
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glDrawArrays);
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glEnable);
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glEnableClientState);
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glIsEnabled);
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glLoadIdentity);
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glMatrixMode);
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glPushMatrix);
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glTexCoordPointer);
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glVertexPointer);

        // Stores current state variable values (to restore them later):
        GLboolean is2DTextureEnabled = gs_stat_realFunctionPointers.glIsEnabled(GL_TEXTURE_2D);
        GLboolean isLightningEnabled = gs_stat_realFunctionPointers.glIsEnabled(GL_LIGHTING);
        GLboolean isFaceCullingEnabled = gs_stat_realFunctionPointers.glIsEnabled(GL_CULL_FACE);
        GLboolean isVertexArrayEnabled = gs_stat_realFunctionPointers.glIsEnabled(GL_VERTEX_ARRAY);
        GLboolean isTextureCoordArrayEnabled = gs_stat_realFunctionPointers.glIsEnabled(GL_TEXTURE_COORD_ARRAY);

        GLint vertexArraySize = 0;
        GLint vertexArrayType = GL_FLOAT;
        GLint vertexArrayStride = 0;
        GLvoid* vetexArrayPointer = NULL;
        gs_stat_realFunctionPointers.glGetIntegerv(GL_VERTEX_ARRAY_SIZE, &vertexArraySize);
        gs_stat_realFunctionPointers.glGetIntegerv(GL_VERTEX_ARRAY_TYPE, &vertexArrayType);
        gs_stat_realFunctionPointers.glGetIntegerv(GL_VERTEX_ARRAY_STRIDE, &vertexArrayStride);
        gs_stat_realFunctionPointers.glGetPointerv(GL_VERTEX_ARRAY_POINTER, &vetexArrayPointer);

        GLint texCoordSize = 0;
        GLint texCoordType = GL_FLOAT;
        GLint texCoordStride = 0;
        GLvoid* texCoordArrayPointer = NULL;
        gs_stat_realFunctionPointers.glGetIntegerv(GL_TEXTURE_COORD_ARRAY_SIZE, &texCoordSize);
        gs_stat_realFunctionPointers.glGetIntegerv(GL_TEXTURE_COORD_ARRAY_TYPE, &texCoordType);
        gs_stat_realFunctionPointers.glGetIntegerv(GL_TEXTURE_COORD_ARRAY_STRIDE, &texCoordStride);
        gs_stat_realFunctionPointers.glGetPointerv(GL_TEXTURE_COORD_ARRAY_POINTER, &texCoordArrayPointer);

        // Enable / Disable states:
        if (!is2DTextureEnabled) { gs_stat_realFunctionPointers.glEnable(GL_TEXTURE_2D); };

        if (isLightningEnabled) { gs_stat_realFunctionPointers.glDisable(GL_LIGHTING); };

        if (isFaceCullingEnabled) { gs_stat_realFunctionPointers.glDisable(GL_CULL_FACE); };

        // Sets the matrices to identity:
        gs_stat_realFunctionPointers.glMatrixMode(GL_MODELVIEW);

        gs_stat_realFunctionPointers.glPushMatrix();

        gs_stat_realFunctionPointers.glLoadIdentity();

        gs_stat_realFunctionPointers.glMatrixMode(GL_PROJECTION);

        gs_stat_realFunctionPointers.glPushMatrix();

        gs_stat_realFunctionPointers.glLoadIdentity();

        // Draws the quad:
        gs_stat_realFunctionPointers.glEnableClientState(GL_VERTEX_ARRAY);

        gs_stat_realFunctionPointers.glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        gs_stat_realFunctionPointers.glVertexPointer(3, GL_FLOAT, sizeof(GLfloat) * 3, verticesArray);

        gs_stat_realFunctionPointers.glTexCoordPointer(2, GL_FLOAT, sizeof(GLfloat) * 2, textureCoordsArray);

        gs_stat_realFunctionPointers.glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Restore states to their initial values:
        gs_stat_realFunctionPointers.glVertexPointer(vertexArraySize, vertexArrayType, vertexArrayStride, vetexArrayPointer);

        gs_stat_realFunctionPointers.glTexCoordPointer(texCoordSize, texCoordType, texCoordStride, texCoordArrayPointer);

        if (!isVertexArrayEnabled) { gs_stat_realFunctionPointers.glDisableClientState(GL_VERTEX_ARRAY); };

        if (!isTextureCoordArrayEnabled) { gs_stat_realFunctionPointers.glDisableClientState(GL_TEXTURE_COORD_ARRAY); };

        gs_stat_realFunctionPointers.glMatrixMode(GL_PROJECTION);

        gs_stat_realFunctionPointers.glPopMatrix();

        gs_stat_realFunctionPointers.glMatrixMode(GL_MODELVIEW);

        gs_stat_realFunctionPointers.glPopMatrix();

        if (!is2DTextureEnabled) { gs_stat_realFunctionPointers.glDisable(GL_TEXTURE_2D); };

        if (isLightningEnabled) { gs_stat_realFunctionPointers.glEnable(GL_LIGHTING); };

        if (isFaceCullingEnabled) { gs_stat_realFunctionPointers.glEnable(GL_CULL_FACE); };

        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glDisable);

        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glDrawArrays);

        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glEnable);

        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glEnableClientState);

        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);

        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glIsEnabled);

        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glLoadIdentity);

        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glMatrixMode);

        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glPushMatrix);

        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glTexCoordPointer);

        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glVertexPointer);
    }
}