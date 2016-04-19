//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Debug code used to measure how much time is spent in the different
/// parts of the server as a command is being processed.
//==============================================================================

#include "CommandTimingManager.h"

#ifdef DEBUG_COMMS_PERFORMANCE
CommandTimingManager* CommandTimingManager::s_pInstance = NULL;
#endif