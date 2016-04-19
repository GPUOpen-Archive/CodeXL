//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file provides the CL Deprecated Function Analyzer
//==============================================================================

#ifndef _CL_DEPRECATED_FUNCTION_ANALYZER_H_
#define _CL_DEPRECATED_FUNCTION_ANALYZER_H_

#include <map>
#include <string>
#include <list>
#include "CLAPIAnalyzer.h"

//------------------------------------------------------------------------------------
/// OpenCL Deprecated Function tracker
//------------------------------------------------------------------------------------
class CLDeprecatedFunctionAnalyzer :
    public CLAPIAnalyzer
{
public:
    /// Constructor
    /// \param p CLAPIAnalyzerManager pointer
    CLDeprecatedFunctionAnalyzer(CLAPIAnalyzerManager* p);

    /// Destructor
    ~CLDeprecatedFunctionAnalyzer(void);

    /// Callback function for flattened APIs
    /// \param pAPIInfo APIInfo object
    void FlattenedAPIAnalyze(APIInfo* pAPIInfo);

    /// Analyze API
    /// \param pAPIInfo APIInfo object
    void Analyze(APIInfo* pAPIInfo);

    /// Generate APIAnalyzerMessage
    void EndAnalyze();

    /// Override clear
    void Clear();

private:
    /// Copy constructor
    /// \param obj object
    CLDeprecatedFunctionAnalyzer(const CLDeprecatedFunctionAnalyzer& obj);

    /// Assignment operator
    /// \param obj object
    /// \return ref to itself
    const CLDeprecatedFunctionAnalyzer& operator = (const CLDeprecatedFunctionAnalyzer& obj);

    std::map<int, std::string> m_deprecatedFunctionMap;  ///< map of deprecated functions (maps from enum value to message displayed with the warning)
};

#endif //_CL_DEPRECATED_FUNCTION_ANALYZER_H_
