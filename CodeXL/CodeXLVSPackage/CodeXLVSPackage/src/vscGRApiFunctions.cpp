//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscGRApiFunctions.cpp
///
//==================================================================================

#include "stdafx.h"
#include <Include/Public/vscGRApiFunctions.h>
#include <src/vspGRApiFunctions.h>

void vscGRApiFunctions_SetOwner(IVscGRApiFunctionsOwner* pOwner)
{
    vspGRApiFunctions::setOwner(pOwner);
}
