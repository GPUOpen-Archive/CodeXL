//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file defines internal functions
//==============================================================================


#include "CLInternalFunctionDefs.h"
#include "CLFunctionDefs.h"
#include "CLUtils.h"
#include "Defs.h"

clExtAMDDispatchTable clExtAMDDispatchTable::m_instance;

clExtAMDDispatchTable* clExtAMDDispatchTable::Instance()
{
    INIT_CL_EXT_FCN_PTR(GetKernelInfoAMD);
    return &m_instance;
}

clExtAMDDispatchTable::clExtAMDDispatchTable()
{
    GetKernelInfoAMD = NULL;
}
