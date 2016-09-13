//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file contains definitions for the historical api table versions
//==============================================================================

#include "HSAAPITableVersions.h"

bool IsROCm12(const HsaApiTable* pHsaTable)
{
    bool retVal = false;

    if (ROCM_1_2_ROOT_MAJOR_VERSION == pHsaTable->version.major_id &&
        ROCM_1_2_ROOT_MINOR_VERSION == pHsaTable->version.minor_id &&
        ROCM_1_2_ROOT_STEP_VERSION == pHsaTable->version.step_id &&
        ROCM_1_2_CORE_MAJOR_VERSION == pHsaTable->core_->version.major_id &&
        ROCM_1_2_CORE_MINOR_VERSION == pHsaTable->core_->version.minor_id &&
        ROCM_1_2_CORE_STEP_VERSION == pHsaTable->core_->version.step_id &&
        ROCM_1_2_AMDEXT_MAJOR_VERSION == pHsaTable->amd_ext_->version.major_id &&
        ROCM_1_2_AMDEXT_MINOR_VERSION == pHsaTable->amd_ext_->version.minor_id &&
        ROCM_1_2_AMDEXT_STEP_VERSION == pHsaTable->amd_ext_->version.step_id &&
        ROCM_1_2_FINALIZEREXT_MAJOR_VERSION == pHsaTable->finalizer_ext_->version.major_id &&
        ROCM_1_2_FINALIZEREXT_MINOR_VERSION == pHsaTable->finalizer_ext_->version.minor_id &&
        ROCM_1_2_FINALIZEREXT_STEP_VERSION == pHsaTable->finalizer_ext_->version.step_id &&
        ROCM_1_2_IMAGEEXT_MAJOR_VERSION == pHsaTable->image_ext_->version.major_id &&
        ROCM_1_2_IMAGEEXT_MINOR_VERSION == pHsaTable->image_ext_->version.minor_id &&
        ROCM_1_2_IMAGEEXT_STEP_VERSION == pHsaTable->image_ext_->version.step_id)
    {
        retVal = true;
    }

    return retVal;
}