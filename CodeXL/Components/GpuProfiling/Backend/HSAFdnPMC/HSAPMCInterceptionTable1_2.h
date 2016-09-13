//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file defines the interceptor helpers for the 1.2 runtime
//==============================================================================

#ifndef _HSA_PMC_INTERCEPTION_TABLE_1_2_H_
#define _HSA_PMC_INTERCEPTION_TABLE_1_2_H_

#ifdef FUTURE_ROCR_VERSION
#include "HSAAPITable1_2.h"

/// Intercepts HSA APIs for this HSA Profiler agent
void InitHSAAPIInterceptPMC1_2(HsaApiTable1_2* pTable);
#endif

#endif // _HSA_PMC_INTERCEPTION_TABLE_1_2_H_
