//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file contains the functions to detour LoadLibrary.
//==============================================================================

#ifndef _DETOURLOADLIBRARY_H_
#define _DETOURLOADLIBRARY_H_

#include "DLLMain.h"

/// \addtogroup MicroDLL
// @{

/// Start detouring LoadLibrary calls
/// \return true if successful, false otherwise
bool DetoursAttachLoadLibrary();

/// Stop detouring LoadLibrary calls
/// \return true if successful, false otherwise
bool DetoursDetachLoadLibrary();

// @}

#endif // _DETOURLOADLIBRARY_H_
