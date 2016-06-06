//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGPUInfo.h
///
//==================================================================================

//------------------------------ apGPUInfo.h ------------------------------

#ifndef __APGPUINFO_H
#define __APGPUINFO_H

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           apGPUInfo
// General Description: Holds a GPU (graphic processing unit) information.
// Author:  AMD Developer Tools Team
// Creation Date:        3/3/2009
// ----------------------------------------------------------------------------------
struct AP_API apGPUInfo
{
    // The GPU name:
    gtString _name;

    // The GPU's driver name:
    gtString _driverName;

public:
    apGPUInfo();
};


#endif //__APGPUINFO_H

