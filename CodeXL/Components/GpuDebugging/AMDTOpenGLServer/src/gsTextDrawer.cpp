//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsTextDrawer.cpp
///
//==================================================================================

//------------------------------ gsTextDrawer.cpp ------------------------------

... Yaki 13 / 12 / 2006 - This file is not used anymore(we replaced the FPS heads up
                                                        display drawing by a preformance counter)

// OpenGL includes:
#include <AMDTOSWrappers/osOpenGLIncludes.h>

// Local:
#include <inc/gsOpenGLMonitor.h>
#include <inc/gsOpenGLWrappers.h>
#include <inc/gsTextDrawer.h>


// ---------------------------------------------------------------------------
// Name:        gsTextDrawer::gsTextDrawer
// Description: Constructor
// Author:      Yaki Tebeka
// Date:        13/2/2005
// ---------------------------------------------------------------------------
gsTextDrawer::gsTextDrawer()
    : _fontTextureName(0), _fontWidth(0), _fontHeight(0), _textureEnvMode(GL_REPLACE)
{
}


// ---------------------------------------------------------------------------
// Name:        gsTextDrawer::setFontTexture
// Description: Sets the name and parameters of the texture that contains the
//              text font.
// Arguments:   fontTextureName - The OpenGL name of the texture that contains
//                                the font.
//              fontWidth - Font char width.
//              fontHeight - Font char hight.
// Author:      Yaki Tebeka
// Date:        1/3/2005
// ---------------------------------------------------------------------------
void gsTextDrawer::setFontTexture(GLuint fontTextureName, GLuint fontWidth, GLuint fontHeight)
{
    _fontTextureName = fontTextureName;
    _fontWidth = fontWidth;
    _fontHeight = fontHeight;
}


// ---------------------------------------------------------------------------
// Name:        gsTextDrawer::draw
// Description: Draw text into the current context.
// Arguments:   textToBeDrawn - The text to be drawn.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        13/2/2005
// ---------------------------------------------------------------------------
bool gsTextDrawer::draw(const gtString& textToBeDrawn)
{
    bool retVal = false;

    // Verify that we have a font texture:
    if (_fontTextureName != 0)
    {
        setOpenGLStatesForTextDrawing();

        // Draw the text:
        retVal = drawText(textToBeDrawn);

        restoreOpenGLStates();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsTextDrawer::setOpenGLStatesForTextDrawing
// Description: Set OpenGL states for text drawing.
// Author:      Yaki Tebeka
// Date:        3/3/2005
// ---------------------------------------------------------------------------
void gsTextDrawer::setOpenGLStatesForTextDrawing()
{
    // TO_DO: Replace the glGetXX calls with using the context wrapper.

    // Store OpenGL modes:
    gs_stat_realFunctionPointers.glGetTexEnviv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, (int*)(&_textureEnvMode));
    gs_stat_realFunctionPointers.glGetFloatv(GL_CURRENT_TEXTURE_COORDS, _textureCoordinates);

    // Bind the font texture to the 2D texture bind target:
    gs_stat_realFunctionPointers.glBindTexture(GL_TEXTURE_2D, _fontTextureName);

    // Make the texture color replace the fragment color:
    gs_stat_realFunctionPointers.glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}


// ---------------------------------------------------------------------------
// Name:        gsTextDrawer::restoreOpenGLStates
// Description: Restore the values of states changed by setOpenGLStatesForTextDrawing()
// Author:      Yaki Tebeka
// Date:        3/3/2005
// ---------------------------------------------------------------------------
void gsTextDrawer::restoreOpenGLStates()
{
    gs_stat_realFunctionPointers.glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, _textureEnvMode);
    gs_stat_realFunctionPointers.glTexCoord4fv(_textureCoordinates);
}


// ---------------------------------------------------------------------------
// Name:        gsTextDrawer::drawText
// Description: Draws a text into
// Arguments:   const gtString& textToBeDrawn
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        14/2/2005
// Usage Sample:
// Implementation Notes:
// History:
// ---------------------------------------------------------------------------
bool gsTextDrawer::drawText(const gtString& textToBeDrawn)
{
    // TO_DO: We need to move the width, height and screen position to be inputs
    //        of this function:

    // Width and height of the text in a screen [0,1]:
    float s_width = 1.0f / 30.0f;
    float s_height = 1.0f / 15.0f;
    float offset = 0.2f / 8.0f;

    // The location in which the text will be drawn:
    float x = 0.0;
    float y = 1.0f - (s_height * 1.6f);

    unsigned char* pCharArray = (unsigned char*)textToBeDrawn.asCharArray();
    int row, c;
    float pos = x;

    while (*pCharArray)
    {
        if (*pCharArray > 127)
        {
            pCharArray++;
            continue;
        }

        if (*pCharArray == '\n')
        {
            y -= s_height;
            x = pos;
            pCharArray++;
            continue;
        }

        row = 16 - (*pCharArray / 8) - 1;
        c = *pCharArray % 8;

        gs_stat_realFunctionPointers.glBegin(GL_QUADS);

        //bottom left corner

        gs_stat_realFunctionPointers.glTexCoord2f(c / 8.0f + offset, row / 16.0f);
        gs_stat_realFunctionPointers.glVertex2f(x, y);

        //bottom right corner
        gs_stat_realFunctionPointers.glTexCoord2f((c + 1) / 8.0f - offset, row / 16.0f);
        gs_stat_realFunctionPointers.glVertex2f(x + s_width, y);

        //top right corner
        gs_stat_realFunctionPointers.glTexCoord2f((c + 1) / 8.0f - offset, (row + 1) / 16.0f);
        gs_stat_realFunctionPointers.glVertex2f(x + s_width, y + s_height);

        //top left corner
        gs_stat_realFunctionPointers.glTexCoord2f(c / 8.0f + offset, (row + 1) / 16.0f);
        gs_stat_realFunctionPointers.glVertex2f(x, y + s_height);

        gs_stat_realFunctionPointers.glEnd();

        pCharArray++;
        x += s_width;
    }

    return true;
}

