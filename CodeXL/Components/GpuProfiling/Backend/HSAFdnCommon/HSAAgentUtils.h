//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file contains utilities that are shared by HSA Agent libs
//==============================================================================
#ifndef _HSA_AGENT_UTILS_H_
#define _HSA_AGENT_UTILS_H_

#include <hsa.h>

/// Checks whether the hsa runtime tools lib failed to load (most likely due to version check failure)
/// \param runtimeVersion   the runtime version number (passed to an agent's OnLoad function)
/// \param failedToolCount  the number of tools which failed to load (passed to an agent's OnLoad function)
/// \param pFailedToolNames the list of tools which failed to load (passed to an agent's OnLoad function)
/// \return false if the runtime tools lib failed to load (or if an error condition is detected)
bool CheckRuntimeToolsLibLoaded(uint64_t runtimeVersion, uint64_t failedToolCount, const char* const* pFailedToolNames);

#endif // _HSA_AGENT_UTILS_H_
