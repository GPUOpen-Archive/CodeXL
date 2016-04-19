//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/AMDTGpuProfilerPlugin.h $
/// \version $Revision: #6 $
/// \brief  This file implements the main plugin for the GPU Profiler
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/AMDTGpuProfilerPlugin.h#6 $
// Last checkin:   $DateTime: 2016/03/22 10:41:56 $
// Last edited by: $Author: gyarnitz $
// Change list:    $Change: 565023 $
//=====================================================================

#ifndef _AMDTGPUPROFILERPLUGIN_H_
#define _AMDTGPUPROFILERPLUGIN_H_

#include <TSingleton.h>

#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>
#include <AMDTGpuProfiling/gpViewsCreator.h>

extern "C"
{
    // check validity of the plugin:
    int AMDT_GPU_PROF_API CheckValidity(gtString& errString);

    void AMDT_GPU_PROF_API initialize();
}


/// Singleton class that implements the GPU Profiler plugin
class AMDT_GPU_PROF_API GpuProfilerPlugin : public TSingleton<GpuProfilerPlugin>
{
public:
    /// Function called by the application framework to initialize this plugin
    void Initialize();

    int CheckValidity(gtString& errString);

    // marks if the prerequisite of this plugin were met
    static bool s_loadEnabled;
};

/// Class whose purpose is to delete all singleton instances created by the AMDTGpuProfiling plugin
class GpuProfilerSingletonsDelete
{
public:
    /// Empty constructor
    GpuProfilerSingletonsDelete() {}

    /// Destructor that deletes all singleton instances created by the AMDTGpuProfiling plugin
    virtual ~GpuProfilerSingletonsDelete();
};


#endif // _AMDTGPUPROFILERPLUGIN_H_
