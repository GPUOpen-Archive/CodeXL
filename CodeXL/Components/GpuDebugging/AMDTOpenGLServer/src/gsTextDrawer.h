//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsTextDrawer.h
///
//==================================================================================

//------------------------------ gsTextDrawer.h ------------------------------

#ifndef __GSTEXTDRAWER
#define __GSTEXTDRAWER

... Yaki 13 / 12 / 2006 - This file is not used anymore(we replaced the FPS heads up
                                                        display drawing by a preformance counter)

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtString.h>


// ----------------------------------------------------------------------------------
// Class Name:           gsTextDrawer
// General Description:
//   Draws text into the active context.
// Author:               Yaki Tebeka
// Creation Date:        10/2/2005
// ----------------------------------------------------------------------------------
class gsTextDrawer
{
public:
    gsTextDrawer();
    void setFontTexture(GLuint fontTextureName, GLuint fontWidth, GLuint fontHeight);
    void setOpenGLStatesForTextDrawing();
    void restoreOpenGLStates();
    bool draw(const gtString& textToBeDrawn);

private:
    bool drawText(const gtString& textToBeDrawn);

private:
    // The name of the texture that holds our font:
    GLuint _fontTextureName;

    // The width and height of each font character:
    GLuint _fontWidth;
    GLuint _fontHeight;

    // Stored OpenGL modes:
    GLenum _textureEnvMode;
    GLfloat _textureCoordinates[4];
};

#endif  // __GSTEXTDRAWER
