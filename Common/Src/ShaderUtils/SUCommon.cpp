//=====================================================================
// Copyright (c) 2010 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/Common/Src/ShaderUtils/SUCommon.cpp $
/// \version $Revision: #5 $
/// \brief  This file defines enums and structs used in ShaderDebugger
///         and APP Profiler.
//
//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/SUCommon.cpp#5 $
// Last checkin:   $DateTime: 2016/04/14 04:43:34 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569084 $
//=====================================================================

#include "SUCommon.h"

using namespace ShaderUtils;

const ThreadID ShaderUtils::g_tID_Error = ThreadID(TID_2D, -1, -1, -1, -1);

bool ShaderUtils::IsValid(const ThreadID& threadID)
{
    switch (threadID.type)
    {
        case TID_1D: return (threadID.x >= 0);

        case TID_2D: return (threadID.x >= 0 && threadID.y >= 0);

        case TID_3D: return (threadID.x >= 0 && threadID.y >= 0 && threadID.z >= 0);

        default:     break;
    }

    return false;
}
