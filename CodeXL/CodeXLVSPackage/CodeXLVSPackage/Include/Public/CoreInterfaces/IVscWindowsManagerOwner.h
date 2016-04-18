//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file IVscWindowsManagerOwner.h
///
//==================================================================================

#ifndef IVscWindowsManager_h__
#define IVscWindowsManager_h__

#include "CodeXLVSPackageCoreDefs.h"

// This enum windows management mode in runtime. Notice that this is  a different mechanism than
// the preprocessor directives (VSP_VS11BUILD, VSP_VS12BUILD, ...) which we use in design time.
// The reason that prevents us from using the aforementioned macros over here, is that the
// code which should consume these macros is located in the CodeXLVSPackageCore project which is
// always being built with our latest MSVC tool-set (currently: MSVC-2013), and therefore does not
// allow us to differentiate between different VS versions.
enum VsWindowsManagementMode
{
    VS_WMM_VS10,
    VS_WMM_VS11,
    VS_WMM_VS12,
    VS_WMM_VS14,
    VS_WMM_UNKNOWN
};

class IVscWindowsManagerOwner
{
public:
    virtual ~IVscWindowsManagerOwner() {}

    virtual void SaveChangedFiles(bool saveSolutionAndProject) const = 0;

    virtual bool OpenSolution(const wchar_t* solutionName) const = 0;

    virtual bool GetVsWindowsManagementMode(VsWindowsManagementMode& buffer) const = 0;

    // Renamed with a prefix to avoid name clashes.
    virtual bool ivwmoVerifyBaseViewsCreated() = 0;

};

#endif // IVscWindowsManager_h__
