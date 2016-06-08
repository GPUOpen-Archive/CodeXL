//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ap2DPoint.h
///
//==================================================================================

//------------------------------ ap2DPoint.h ------------------------------

#ifndef __AP2DPOINT
#define __AP2DPOINT

// Infra:
#include <AMDTOSWrappers/Include/osTransferableObject.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>


// ----------------------------------------------------------------------------------
// Struct Name:          AP_API ap2DPoint : public osTransferableObject
// General Description:
//   Represents a point in 2D space.
//
// Author:  AMD Developer Tools Team
// Creation Date:        8/4/2004
// ----------------------------------------------------------------------------------
struct AP_API ap2DPoint : public osTransferableObject
{
public:
    ap2DPoint();
    ap2DPoint(float xPos, float yPos);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

public:
    // The point coordinates:
    float _xPos;
    float _yPos;
};


#endif  // __AP2DPOINT
