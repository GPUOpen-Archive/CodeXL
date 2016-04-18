//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This analyzer class analyzes opencl return codes; reports errors for failures
//==============================================================================

#ifndef _CL_RET_CODE_ANALYZER_H_
#define _CL_RET_CODE_ANALYZER_H_

#include <map>
#include <string>
#include <list>
#include "CLAPIAnalyzer.h"

//------------------------------------------------------------------------------------
/// OpenCL Return code analyzer
//------------------------------------------------------------------------------------
class CLRetCodeAnalyzer :
    public CLAPIAnalyzer
{
public:
    /// Constructor
    /// \param pManager CLAPIAnalyzerManager pointer
    CLRetCodeAnalyzer(CLAPIAnalyzerManager* pManager);

    /// Destructor
    ~CLRetCodeAnalyzer(void);

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
    CLRetCodeAnalyzer(const CLRetCodeAnalyzer& obj);

    /// Assignment operator
    /// \param obj object
    /// \return ref to itself
    const CLRetCodeAnalyzer& operator = (const CLRetCodeAnalyzer& obj);

    /// map from OCL API to the parameter index that contains the return code
    /// abscence from this map indicates an API returns its return code
    /// as the return value and not a parameter
    std::map<CL_FUNC_TYPE, unsigned int> m_retCodeArgMap;
};

#endif //_CL_RET_CODE_ANALYZER_H_
