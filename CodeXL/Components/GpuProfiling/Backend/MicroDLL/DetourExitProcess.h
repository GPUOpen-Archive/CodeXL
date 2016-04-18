//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file contains the functions to detour ExitProcess.
//==============================================================================

#ifndef _DETOUR_EXIT_PROCESS_H_
#define _DETOUR_EXIT_PROCESS_H_

/// \addtogroup MicroDLL
// @{

/// Start detouring ExitProcess calls
/// \return true if successful, false otherwise
bool DetoursAttachExitProcess();

/// Stop detouring ExitProcess calls
/// \return true if successful, false otherwise
bool DetoursDetachExitProcess();

// @}

#endif // _DETOUR_EXIT_PROCESS_H_