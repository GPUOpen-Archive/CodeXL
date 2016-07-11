//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file defines the interceptor helpers for the 1.0 runtime
//==============================================================================

#ifndef _HSA_PMC_INTERCEPTION_TABLE_1_0_H_
#define _HSA_PMC_INTERCEPTION_TABLE_1_0_H_

#include "HSAAPITable1_0.h"

/// Intercepts HSA APIs for this HSA Profiler agent
void InitHSAAPIInterceptPMC1_0(ApiTable1_0* pTable);

#endif // _HSA_PMC_INTERCEPTION_TABLE_1_0_H_
