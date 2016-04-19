//=============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Functions to convert DXGI_FORMAT enums into strings.
//=============================================================================

#ifndef STRINGIFYDXGIFORMATENUMS_H
#define STRINGIFYDXGIFORMATENUMS_H

#include "dxgiformat.h"
#include "AMDTBaseTools/Include/gtASCIIString.h"

gtASCIIString Stringify_DXGI_FORMAT(DXGI_FORMAT var);

#endif // STRINGIFYDXGIFORMATENUMS_H