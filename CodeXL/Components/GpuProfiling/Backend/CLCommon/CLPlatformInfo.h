//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file defines the structure used to store platform info.
//==============================================================================

#ifndef _CL_PLATFORM_INFO_H_
#define _CL_PLATFORM_INFO_H_

#include <CL/opencl.h>
#include <string>
#include <set>

namespace CLPlatformInfo
{

/// Structure containing the CL platform information
typedef struct platform_info
{
    std::string strPlatformVendor;  ///< Compute platform vendor
    std::string strPlatformName;    ///< Compute platform name
    std::string strDeviceName;      ///< Compute device name
    std::string strPlatformVersion; ///< Compute platform version string
    std::string strDriverVersion;   ///< Compute Abstraction Layer version information}
    std::string strCLRuntime;       ///< CL runtime version
    unsigned int uiNbrAddressBits;  ///< Number of address bits used by application (application 'bitness')
} platform_info_struct;

struct CLPlatformInfoCompare
{
    bool operator()(const CLPlatformInfo::platform_info p1, const CLPlatformInfo::platform_info p2) const;
};

}

typedef std::set<CLPlatformInfo::platform_info, CLPlatformInfo::CLPlatformInfoCompare> CLPlatformSet;

#endif // _CL_PLATFORM_INFO_H_
