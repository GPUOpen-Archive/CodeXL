//=====================================================================
// Copyright (c) 2010 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/Common/Src/ShaderUtils/SUDeviceBuffer.cpp $
/// \version $Revision: #5 $
/// \brief  This file defines enums and structs used in ShaderDebugger
///         and APP Profiler.
//
//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/SUDeviceBuffer.cpp#5 $
// Last checkin:   $DateTime: 2016/04/14 04:43:34 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569084 $
//=====================================================================

#include <cstring>
#include "SUDeviceBuffer.h"

using namespace std;
using namespace ShaderUtils;

SUDeviceBuffer::SUDeviceBuffer()
{
    memset(&m_Desc, 0, sizeof(m_Desc));
}


SUDeviceBuffer::~SUDeviceBuffer()
{
}

void SUDeviceBuffer::GetDeviceBufferDesc(BufferDesc* pDesc) const
{
    if (NULL != pDesc)
    {
        memcpy(pDesc, &m_Desc, sizeof(BufferDesc));
    }
}
