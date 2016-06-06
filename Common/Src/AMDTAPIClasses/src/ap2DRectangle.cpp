//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ap2DRectangle.cpp
///
//==================================================================================

//------------------------------ ap2DRectangle.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/ap2DRectangle.h>


// ---------------------------------------------------------------------------
// Name:        ap2DRectangle::ap2DRectangle
// Description: Default constructor - initialize both coordinates and sizes
//              to contain 0.
// Author:  AMD Developer Tools Team
// Date:        10/4/2006
// ---------------------------------------------------------------------------
ap2DRectangle::ap2DRectangle()
    : _xPos(0.0f), _yPos(0.0f), _width(0.0f), _height(0.0f)
{
}


// ---------------------------------------------------------------------------
// Name:        ap2DRectangle::ap2DRectangle
// Description: Copy constructor
// Author:  AMD Developer Tools Team
// Date:        10/4/2006
// ---------------------------------------------------------------------------
ap2DRectangle::ap2DRectangle(const ap2DRectangle& other)
    : _xPos(other._xPos), _yPos(other._yPos), _width(other._width), _height(other._height)
{
}


// ---------------------------------------------------------------------------
// Name:        ap2DRectangle::ap2DRectangle
// Description: Constructor
// Arguments:   xPos, yPos - Rectangle position.
//              width, height - Rectangle size.
// Author:  AMD Developer Tools Team
// Date:        10/4/2006
// ---------------------------------------------------------------------------
ap2DRectangle::ap2DRectangle(float xPos, float yPos, float width, float height)
    : _xPos(xPos), _yPos(yPos), _width(width), _height(height)
{
}


// ---------------------------------------------------------------------------
// Name:        ap2DRectangle::ap2DRectangle
// Description: Constructor. Copies the rectangle values from an array of 4
//              floats (x, y, width, height)
// Author:  AMD Developer Tools Team
// Date:        10/4/2006
// ---------------------------------------------------------------------------
ap2DRectangle::ap2DRectangle(const float* rectangleAsFloatArray)
    : _xPos(rectangleAsFloatArray[0]), _yPos(rectangleAsFloatArray[1]),
      _width(rectangleAsFloatArray[2]), _height(rectangleAsFloatArray[3])
{
}


// ---------------------------------------------------------------------------
// Name:        ap2DRectangle::operator=
// Description: Assignment operator.
// Arguments: other - The other rectangle from which my values are assigned.
// Return Val: ap2DRectangle& - A reference to me.
// Author:  AMD Developer Tools Team
// Date:        10/4/2006
// ---------------------------------------------------------------------------
ap2DRectangle& ap2DRectangle::operator=(const ap2DRectangle& other)
{
    _xPos = other._xPos;
    _yPos = other._yPos;
    _width = other._width;
    _height = other._height;

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        ap2DRectangle::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        10/4/2006
// ---------------------------------------------------------------------------
osTransferableObjectType ap2DRectangle::type() const
{
    return OS_TOBJ_ID_2D_RECTANGLE;
}


// ---------------------------------------------------------------------------
// Name:        ap2DRectangle::writeSelfIntoChannel
// Description: Write self into an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        10/4/2006
// ---------------------------------------------------------------------------
bool ap2DRectangle::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _xPos;
    ipcChannel << _yPos;
    ipcChannel << _width;
    ipcChannel << _height;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        ap2DRectangle::readSelfFromChannel
// Description: Read self from an ipcChannel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        10/4/2006
// ---------------------------------------------------------------------------
bool ap2DRectangle::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _xPos;
    ipcChannel >> _yPos;
    ipcChannel >> _width;
    ipcChannel >> _height;
    return true;
}

