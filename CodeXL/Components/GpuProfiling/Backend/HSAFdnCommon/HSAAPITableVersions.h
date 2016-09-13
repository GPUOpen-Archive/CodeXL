//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file contains definitions for the historical api table versions
//==============================================================================

#ifndef _HSA_API_TABLE_VERSIONS_H
#define _HSA_API_TABLE_VERSIONS_H

#include "hsa_api_trace.h"

#define ROCM_1_1_X_AND_EARLIER_ROOT_RUNTIME_VERSION 0

#define ROCM_1_2_ROOT_MAJOR_VERSION 1
#define ROCM_1_2_ROOT_MINOR_VERSION 48 
#define ROCM_1_2_ROOT_STEP_VERSION 0

#define ROCM_1_2_CORE_MAJOR_VERSION 1
#define ROCM_1_2_CORE_MINOR_VERSION 784
#define ROCM_1_2_CORE_STEP_VERSION 0

#define ROCM_1_2_AMDEXT_MAJOR_VERSION 1
#define ROCM_1_2_AMDEXT_MINOR_VERSION 224
#define ROCM_1_2_AMDEXT_STEP_VERSION 0

#define ROCM_1_2_IMAGEEXT_MAJOR_VERSION 1
#define ROCM_1_2_IMAGEEXT_MINOR_VERSION 96
#define ROCM_1_2_IMAGEEXT_STEP_VERSION 0

#define ROCM_1_2_FINALIZEREXT_MAJOR_VERSION 1
#define ROCM_1_2_FINALIZEREXT_MINOR_VERSION 64
#define ROCM_1_2_FINALIZEREXT_STEP_VERSION 0

/// Checks if the table represents the ROCm 1.2 table
/// \param pHsaTable the table to check
/// \return true if this table is the ROCm 1.2 table
bool IsROCm12(const HsaApiTable* pHsaTable);

#endif // _HSA_API_TABLE_VERSIONS_H
