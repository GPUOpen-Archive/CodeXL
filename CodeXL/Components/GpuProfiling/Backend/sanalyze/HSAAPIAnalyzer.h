//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file defines base HSAAPIAnalyzer class and HSAAPIAnalyzerManager class which is a IParserListener
//==============================================================================

#ifndef _HSA_API_ANALYZER_H_
#define _HSA_API_ANALYZER_H_

#include "APIAnalyzer.h"
#include "../HSAFdnTrace/HSAAPIInfo.h"

//------------------------------------------------------------------------------------
/// HSA API Analyzer base class
//------------------------------------------------------------------------------------
class HSAAPIAnalyzer : public APIAnalyzer<HSA_API_Type>
{
};

class HSAAPIAnalyzerManager : public APIAnalyzerManager <HSAAPIAnalyzer, HSA_API_Type>, public IParserListener<HSAAPIInfo>
{
public:
    /// Listener function
    /// \param pAPIInfo API Info object
    /// \param[out] stopParsing flag indicating if parsing should stop after this item
    void OnParse(HSAAPIInfo* pAPIInfo, bool& stopParsing);
};

#endif //_HSA_API_ANALYZER_H_
