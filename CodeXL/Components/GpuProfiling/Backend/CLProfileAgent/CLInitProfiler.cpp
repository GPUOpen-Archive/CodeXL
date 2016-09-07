//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file contains the function to initialize the profiler
//==============================================================================

#include <fstream>
#include <iostream>
#include <string.h>
#include "CLInitProfiler.h"
#include "CLGPAProfiler.h"
#include "CLInternalFunctionDefs.h"
#include "../Common/GlobalSettings.h"
#include "../Common/Logger.h"
#include "../Common/Defs.h"
#include "../Common/StringUtils.h"
#include <AMDTOSWrappers/Include/osProcess.h>

#ifndef _WIN32
    #include <cstdlib> //getenv
#endif

using std::cout;
using std::cin;
using std::endl;
using std::string;
using std::ifstream;

extern CLGPAProfiler g_Profiler;

bool InitProfiler()
{
    if (g_Profiler.Loaded())
    {
        // DLL has been loaded
        return true;
    }

    std::string strLogFile = FileUtils::GetDefaultOutputPath() + "clprofileagent.log";
    LogFileInitialize(strLogFile.c_str());

    // if it is not loaded, let's load the dll
#ifdef USE_DEBUG_GPA
    string strDll(LIB_PREFIX "GPUPerfAPICL" BITNESS "-d" GDT_BUILD_SUFFIX LIB_SUFFIX);
#else
    string strDll(LIB_PREFIX "GPUPerfAPICL" BITNESS GDT_BUILD_SUFFIX LIB_SUFFIX);
#endif

    // Pass params between processes through file
    // CodeXLGpuProfiler generate a text file in current dir
    Parameters params;
    FileUtils::GetParametersFromFile(params);
    FileUtils::ReadKernelListFile(params);

    GlobalSettings::GetInstance()->m_bVerbose = params.m_bVerbose;
    GlobalSettings::GetInstance()->m_params = params;

    std::string strError;

    if (!g_Profiler.Init(params, strError))
    {
        cout << "Error loading " << strDll << ": " << strError << endl;
        Log(logERROR, "Error loading %s. Error: %s\n", strDll.c_str(), strError.c_str());
        return false;
    }

    return true;
}
