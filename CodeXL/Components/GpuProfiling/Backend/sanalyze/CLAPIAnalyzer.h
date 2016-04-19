//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file defines the CLAPIAnalyzer and CLAPIAnalyzerManager specialization classes
//==============================================================================

#ifndef _CL_API_ANALYZER_H_
#define _CL_API_ANALYZER_H_

#include "APIAnalyzer.h"
#include "../CLTraceAgent/CLAPIInfo.h"

#define NUM_ARG_CL_SET_KERNEL_ARG 4
#define NUM_ARG_CL_CREATE_KERNELS_IN_PROGRAM 4
#define NUM_ARG_CL_CREATE_SUB_DEVICES_EXT 5
#define NUM_ARG_CL_WAIT_FOR_EVENTS 2

class CLAPIAnalyzerManager;

typedef std::map<unsigned int, std::string> CLKernelArgsSetup;

//------------------------------------------------------------------------------------
/// CL Kernel Info
//------------------------------------------------------------------------------------
struct CLKernelInfo
{
    unsigned int uiRefCount;         ///< Kernel handle reference count
    CLKernelArgsSetup kernelArgsSetup; ///< Kernel arguments setup table
};

typedef std::map<std::string, CLKernelInfo> CLKernelArgsSetupMap;

//------------------------------------------------------------------------------------
/// OpenCL API Analyzer base class
//------------------------------------------------------------------------------------
class CLAPIAnalyzer : public APIAnalyzer<CL_FUNC_TYPE>
{
public:
    /// Constructor
    /// \param pManager CLAPIAnalyzerManager object
    CLAPIAnalyzer(CLAPIAnalyzerManager* pManager);

protected:
    CLAPIAnalyzerManager* m_pParent; ///< CLAPIAnalyzerManager pointer
};

class CLAPIAnalyzerManager : public APIAnalyzerManager <CLAPIAnalyzer, CL_FUNC_TYPE>, public IParserListener<CLAPIInfo>
{
public:
    /// Constructor
    CLAPIAnalyzerManager();

    /// KernelArgsSetupMap getter
    /// \return reference to CLKernelArgsSetupMap
    const CLKernelArgsSetupMap& GetKernelArgsSetupMap() const;

    /// Enable kernel args setup analyzer
    /// It's used by DataTransferAnalyzer. If DataTransferAnalyzer is disabled,
    /// We can disable kernel args setup as well.
    void EnableKernelArgsSetupAnalyzer();

    /// Analyze kernel arg setup
    /// \param pAPIInfo API Info object
    void AnalyzeKernelArgSetup(CLAPIInfo* pAPIInfo);

    /// Listener function
    /// \param pAPIInfo API Info object
    /// \param[out] stopParsing flag indicating if parsing should stop after this item
    void OnParse(CLAPIInfo* pAPIInfo, bool& stopParsing);

protected:
    /// Overridden virtual fnuction see base class for description
    virtual bool DoEnableAnalyzer();

    /// Overridden virtual fnuction see base class for description
    virtual bool DoEndAnalyze(APIInfo* pAPIInfo);

private:
    CLKernelArgsSetupMap m_KernelArgsSetupMap;        ///< Kernel args setup map
    bool                 m_bEnableKernelArgsAnalyzer; ///< A flag indicating whether or not kernel args setup are parsed.
};

#endif //_CL_API_ANALYZER_H_
