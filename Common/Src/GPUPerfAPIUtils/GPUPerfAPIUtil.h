//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Defines the entrypoints to the GPUPerfAPIUtil library.
//==============================================================================

#ifndef _GPUPERFAPI_UTIL_H_
#define _GPUPERFAPI_UTIL_H_

#include "GPUPerfAPITypes.h"
#include "GPUPerfAPILoader.h"

/// Enable counters from file
/// \param pLoader GPA loader object
/// \param pFileName Input file
/// \param pCountersRead List of counter read
/// \param ppErrorText Error message
/// \param pActiveSectionLabel Active section label
/// \return GPA status
GPA_Status GPA_EnableCountersFromFile(GPUPerfAPILoader* pLoader, const char* pFileName, gpa_uint32* pCountersRead, const char** ppErrorText, const char* pActiveSectionLabel);

#endif // _GPUPERFAPI_UTIL_H_
