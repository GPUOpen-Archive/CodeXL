//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLRenderContextInfo.cpp
///
//==================================================================================

//------------------------------ apGLRenderContextInfo.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apGLRenderContextInfo.h>


// ---------------------------------------------------------------------------
// Name:        apGLRenderContextInfo::apGLRenderContextInfo
// Description: Constructor.
// Author:  AMD Developer Tools Team
// Date:        17/6/2008
// ---------------------------------------------------------------------------
apGLRenderContextInfo::apGLRenderContextInfo()
    : apAllocatedObject(), _spyID(-1), _sharingContextID(-2), _openCLContextID(0)
{
}

// ---------------------------------------------------------------------------
// Name:        apGLRenderContextInfo::readSelfFromChannel
// Description: Writes this class data into ipcChannel.
// Author:  AMD Developer Tools Team
// Date:        17/6/2008
// ---------------------------------------------------------------------------
bool apGLRenderContextInfo::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    ipcChannel << (gtInt32)_spyID;
    ipcChannel << (gtInt32)_sharingContextID;
    ipcChannel << (gtInt32)_openCLContextID;

    apAllocatedObject::writeSelfIntoChannel(ipcChannel);

    return true;
}

// ---------------------------------------------------------------------------
// Name:        apGLRenderContextInfo::readSelfFromChannel
// Description: Reads this class data from ipcChannel.
// Author:  AMD Developer Tools Team
// Date:        17/6/2008
// ---------------------------------------------------------------------------
bool apGLRenderContextInfo::readSelfFromChannel(osChannel& ipcChannel)
{
    gtInt32 spyIDAsInt32 = 0;
    ipcChannel >> spyIDAsInt32;
    _spyID = (int)spyIDAsInt32;

    gtInt32 varAsInt32 = 0;
    ipcChannel >> varAsInt32;
    _sharingContextID = (int)varAsInt32;

    ipcChannel >> varAsInt32;
    _openCLContextID = (int)varAsInt32;

    apAllocatedObject::readSelfFromChannel(ipcChannel);

    return true;
}
