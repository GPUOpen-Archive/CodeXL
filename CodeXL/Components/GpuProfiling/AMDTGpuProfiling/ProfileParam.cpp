//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/ProfileParam.cpp $
/// \version $Revision: #3 $
/// \brief :  This file contains ProfileParam class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/ProfileParam.cpp#3 $
// Last checkin:   $DateTime: 2015/08/31 01:56:31 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 538740 $
//=====================================================================
#include "ProfileParam.h"


ProfileParam::ProfileParam()
{
    m_currentProfileData = new ProfileSettingData();
}

ProfileParam::~ProfileParam()
{
    SAFE_DELETE(m_currentProfileData);
}

