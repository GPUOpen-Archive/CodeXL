//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsAttribStackItem.h
///
//==================================================================================

//------------------------------ gsAttribStackItem.h ------------------------------

#ifndef __GSATTRIBSTACKITEM
#define __GSATTRIBSTACKITEM

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>


// ----------------------------------------------------------------------------------
// Struct Name:          gsAttribStackItem
// General Description:
//   An item in the gsAttribStack stack.
//   See gsAttribStack documentation for more details.
// Author:               Yaki Tebeka
// Creation Date:        21/6/2005
// ----------------------------------------------------------------------------------
struct gsAttribStackItem
{
    // OpenGL raster mode (front and back faces):
    GLenum _rasterModeFrontFace;
    GLenum _rasterModeBackFace;

    // The drawn buffer / buffers:
    GLenum _drawnBuffers;

    // The view port (x, y, width, height):
    GLint _viewPort[4];

    // Are modes forced in this stack item:
    bool _isPolygonRasterModeForced;
    bool _isDrawnBufferForced;
    bool _isSinglePixelViewPortForced;

public:
    gsAttribStackItem();
    gsAttribStackItem(const gsAttribStackItem& other);
    gsAttribStackItem& operator=(const gsAttribStackItem& other);
};

#endif  // __GSATTRIBSTACKITEM
