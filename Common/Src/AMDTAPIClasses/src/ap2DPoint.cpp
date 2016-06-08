//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ap2DPoint.cpp
///
//==================================================================================

//------------------------------ ap2DPoint.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/ap2DPoint.h>


// ---------------------------------------------------------------------------
// Name:        ap2DPoint::ap2DPoint
// Description: Default constructor - initialize both coordinates to contain 0.
// Author:  AMD Developer Tools Team
// Date:        9/2/2005
// ---------------------------------------------------------------------------
ap2DPoint::ap2DPoint()
    : _xPos(0.0f), _yPos(0.0f)
{
}


// ---------------------------------------------------------------------------
// Name:        ap2DPoint::ap2DPoint
// Description: Constructor
// Arguments:   xPos - The point x coordinate position.
//              yPos - The point y coordinate position.
// Author:  AMD Developer Tools Team
// Date:        9/2/2005
// ---------------------------------------------------------------------------
ap2DPoint::ap2DPoint(float xPos, float yPos)
    : _xPos(xPos), _yPos(yPos)
{
}


// ---------------------------------------------------------------------------
// Name:        ap2DPoint::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        9/2/2005
// ---------------------------------------------------------------------------
osTransferableObjectType ap2DPoint::type() const
{
    return OS_TOBJ_ID_2D_POINT;
}


// ---------------------------------------------------------------------------
// Name:        ap2DPoint::writeSelfIntoChannel
// Description: Write self into an IPC channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        9/2/2005
// ---------------------------------------------------------------------------
bool ap2DPoint::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << _xPos;
    ipcChannel << _yPos;
    return true;
}



// ---------------------------------------------------------------------------
// Name:        ap2DPoint::readSelfFromChannel
// Description: Read self from an ipcChannel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        9/2/2005
// ---------------------------------------------------------------------------
bool ap2DPoint::readSelfFromChannel(osChannel& ipcChannel)
{
    ipcChannel >> _xPos;
    ipcChannel >> _yPos;
    return true;
}

