//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscGRApiFunctions.h
///
//==================================================================================

#ifndef vscGRApiFunctions_h__
#define vscGRApiFunctions_h__
#include "CodeXLVSPackageCoreDefs.h"

// Core interfaces:
#include <Include/Public/CoreInterfaces/IVscGRApiFunctionsOwner.h>

void vscGRApiFunctions_SetOwner(IVscGRApiFunctionsOwner* pOwner);

#endif // vscGRApiFunctions_h__