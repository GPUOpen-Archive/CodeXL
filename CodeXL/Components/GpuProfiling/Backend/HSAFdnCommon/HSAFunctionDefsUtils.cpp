//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file defines Utilities that work with HSA_API_Type defined in HSAFunctionDefs.h
//==============================================================================

#include <unordered_map>

#include "Logger.h"
#include "HSAFunctionDefsUtils.h"

using namespace GPULogger;

HSAFunctionDefsUtils::HSAFunctionDefsUtils()
{
#define X(SYM) m_hsaAPIMap.insert(std::pair<std::string, HSA_API_Type>(std::string("hsa_" #SYM), HSA_API_Type_hsa_##SYM));
    HSA_RUNTIME_API_TABLE
    HSA_EXT_FINALIZE_API_TABLE
    HSA_EXT_IMAGE_API_TABLE
    //HSA_EXT_AMD_API_TABLE // pending discussion with RT team whether these should be added to interception table
#undef X
}

HSA_API_Type HSAFunctionDefsUtils::ToHSAAPIType(const std::string& strName)
{
    std::unordered_map<std::string, HSA_API_Type>::iterator it = m_hsaAPIMap.find(strName);
    SpAssert(it != m_hsaAPIMap.end());

    if (it != m_hsaAPIMap.end())
    {
        return it->second;
    }
    else
    {
        return HSA_API_Type_UNKNOWN;
    }
}
