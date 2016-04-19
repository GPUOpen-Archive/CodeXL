//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file contains utilities that are shared by HSA Agent libs
//==============================================================================

#include <iostream>

#include "Logger.h"
#include "Defs.h"

#include "HSAAgentUtils.h"

bool CheckRuntimeToolsLibLoaded(uint64_t runtimeVersion, uint64_t failedToolCount, const char* const* pFailedToolNames)
{
    bool retVal = true;

    if (failedToolCount > 0 && runtimeVersion > 0)
    {
        if (nullptr == pFailedToolNames)
        {
            Log(logERROR, "Error loading HSA Profiler. Unknown tool library failed to load\n");
            retVal = false;
        }
        else
        {
            for (uint64_t i = 0; i < failedToolCount && retVal; i++)
            {
                if (nullptr == pFailedToolNames[i])
                {
                    Log(logERROR, "Error loading HSA Profiler. Unknown tool library failed to load\n");
                    retVal = false;
                }
                else
                {
                    std::string failedToolName = std::string(pFailedToolNames[i]);

                    if (std::string::npos != failedToolName.find_last_of(HSA_RUNTIME_TOOLS_LIB))
                    {
                        Log(logERROR, "Error loading HSA Profiler. %s could not be loaded due to version mismatch with the HSA runtime\n", HSA_RUNTIME_TOOLS_LIB);
                        retVal = false;
                    }
                }
            }
        }
    }

    return retVal;
}
