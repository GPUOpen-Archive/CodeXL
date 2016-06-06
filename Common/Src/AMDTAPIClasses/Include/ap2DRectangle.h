//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ap2DRectangle.h
///
//==================================================================================

//------------------------------ ap2DRectangle.h ------------------------------

#ifndef __AP2DRECTANGLE_H
#define __AP2DRECTANGLE_H

// Infra:
#include <AMDTOSWrappers/Include/osTransferableObject.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>


// ----------------------------------------------------------------------------------
// Struct Name:          AP_API ap2DRectangle : public osTransferableObject
// General Description:
//   Represents a rectangle in 2D space.
//
// Author:  AMD Developer Tools Team
// Creation Date:        10/4/2006
// ----------------------------------------------------------------------------------
struct AP_API ap2DRectangle : public osTransferableObject
{
public:
    ap2DRectangle();
    ap2DRectangle(const ap2DRectangle& other);
    ap2DRectangle(float xPos, float yPos, float width, float height);
    ap2DRectangle(const float* rectangleAsFloatArray);
    ap2DRectangle& operator=(const ap2DRectangle& other);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

public:
    // The rectangle position and size:
    float _xPos;
    float _yPos;
    float _width;
    float _height;
};


#endif //__AP2DRECTANGLE_H

