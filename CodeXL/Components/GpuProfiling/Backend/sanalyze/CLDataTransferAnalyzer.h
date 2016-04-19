//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file provides the CL Data Transfer Analyzer
//==============================================================================

#ifndef _CL_DATA_TRANSFER_ANALYZER_H_
#define _CL_DATA_TRANSFER_ANALYZER_H_

#include <map>
#include <string>
#include <list>
#include "CLAPIAnalyzer.h"

typedef std::map<std::string, std::string> BufferFlagMap;

//------------------------------------------------------------------------------------
/// OpenCL Data transfer analyzer
//------------------------------------------------------------------------------------
class CLDataTransferAnalyzer :
    public CLAPIAnalyzer
{
public:
    /// Constructor
    /// \param p CLAPIAnalyzerManager pointer
    CLDataTransferAnalyzer(CLAPIAnalyzerManager* p);

    /// Destructor
    ~CLDataTransferAnalyzer(void);

    /// Analyze API
    /// \param pAPIInfo APIInfo object
    void Analyze(APIInfo* pAPIInfo);

    /// Callback function for flattened APIs
    /// \param pAPIInfo APIInfo object
    void FlattenedAPIAnalyze(APIInfo* pAPIInfo);

    /// Generate APIAnalyzerMessage
    void EndAnalyze();

    /// Override SetEnable, Enable kernel args setup analyzer if CLDataTransferAnalyzer is enabled
    void SetEnable(const AnalyzeOps& op);

private:
    /// Copy constructor
    /// \param obj object
    CLDataTransferAnalyzer(const CLDataTransferAnalyzer& obj);

    /// Assignment operator
    /// \param obj object
    /// \return ref to itself
    const CLDataTransferAnalyzer& operator = (const CLDataTransferAnalyzer& obj);

private:
    BufferFlagMap m_BufferFlagMap;  ///< Buffer Handle to mem_flag map
};

#endif //_CL_DATA_TRANSFER_ANALYZER_H_
