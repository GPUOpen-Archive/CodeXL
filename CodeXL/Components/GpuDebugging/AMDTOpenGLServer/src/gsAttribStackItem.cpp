//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsAttribStackItem.cpp
///
//==================================================================================

//------------------------------ gsAttribStackItem.cpp ------------------------------

// Local:
#include <src/gsAttribStackItem.h>


// ---------------------------------------------------------------------------
// Name:        gsAttribStackItem::gsAttribStackItem
// Description: Constructor
// Author:      Yaki Tebeka
// Date:        21/6/2005
// ---------------------------------------------------------------------------
gsAttribStackItem::gsAttribStackItem()
    : _rasterModeFrontFace(GL_FILL), _rasterModeBackFace(GL_FILL), _drawnBuffers(GL_BACK),
      _isPolygonRasterModeForced(false), _isDrawnBufferForced(false), _isSinglePixelViewPortForced(false)
{
    for (int i = 0; i < 4; i++)
    {
        _viewPort[i] = 0;
    }
}



// ---------------------------------------------------------------------------
// Name:        gsAttribStackItem::gsAttribStackItem
// Description: Copy constructor.
// Arguments:   other - The other item from which I am copied.
// Author:      Yaki Tebeka
// Date:        21/6/2005
// ---------------------------------------------------------------------------
gsAttribStackItem::gsAttribStackItem(const gsAttribStackItem& other)
{
    operator=(other);
}


// ---------------------------------------------------------------------------
// Name:        gsAttribStackItem::operator=
// Description: Assignment operator
// Arguments:   other - The other item from which I am copied.
// Return Val:  const gsAttribStackItem& - A reference to me.
// Author:      Yaki Tebeka
// Date:        21/6/2005
// ---------------------------------------------------------------------------
gsAttribStackItem& gsAttribStackItem::operator=(const gsAttribStackItem& other)
{
    // Copy the other stack item members:
    _rasterModeFrontFace = other._rasterModeFrontFace;
    _rasterModeBackFace = other._rasterModeBackFace;
    _drawnBuffers = other._drawnBuffers;

    for (int i = 0; i < 4; i++)
    {
        _viewPort[i] = other._viewPort[i];
    }

    _isPolygonRasterModeForced = other._isPolygonRasterModeForced;
    _isDrawnBufferForced = other._isDrawnBufferForced;
    _isSinglePixelViewPortForced = other._isSinglePixelViewPortForced;

    // Return a reference to myself:
    return *this;
}



