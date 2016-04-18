//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file defines Utilities that work with HSA_API_Type defined in HSAFunctionDefs.h
//==============================================================================

#ifndef _HSA_FUNCTION_DEFS_UTILS_H_
#define _HSA_FUNCTION_DEFS_UTILS_H_

#include <string>
#include <unordered_map>

#include <TSingleton.h>
#include "HSAFunctionDefs.h"

class HSAFunctionDefsUtils : public TSingleton<HSAFunctionDefsUtils>
{
    friend class TSingleton<HSAFunctionDefsUtils>;

protected:
    /// Constructor
    HSAFunctionDefsUtils();

public:
    /// Convert HSA API name string to enum
    /// \param strName API name string
    /// \return enum representation of HSA API
    HSA_API_Type ToHSAAPIType(const std::string& strName);

private:
    std::unordered_map<std::string, HSA_API_Type> m_hsaAPIMap; ///< map from apiName to api type
};

#endif // _HSA_FUNCTION_DEFS_UTILS_H_
