// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file defines APIInfo descendants used when tracing
//==============================================================================

#ifndef _HSA_API_INFO_H_
#define _HSA_API_INFO_H_

// avoid conflict between Qt, which #defines "signals" and hsa_ext_amd.h which uses "signals" as a parameter name for a function
#if defined(signals)
    #pragma push_macro("signals")
    #undef signals
    #define NEED_TO_POP_SIGNALS_MACRO
#endif
#include "../HSAFdnCommon/HSAFunctionDefs.h"
#if defined (NEED_TO_POP_SIGNALS_MACRO)
    #pragma pop_macro("signals")
#endif

#include "../HSAFdnCommon/HSAFunctionDefsUtils.h"

#include <sstream>

#include <AMDTOSWrappers/Include/osOSDefinitions.h>

#include "../Common/APIInfo.h"
#include "../Common/StringUtils.h"

//------------------------------------------------------------------------------------
/// HSA API info
//------------------------------------------------------------------------------------
class HSAAPIInfo : public APIInfo
{
public:
    /// Constructor
    HSAAPIInfo() : APIInfo(), m_apiID(HSA_API_Type_UNKNOWN), m_bIsAPI(true)
    {
    }

    /// Virtual destructor
    virtual ~HSAAPIInfo() {}

    /// Parse the argument list -- empty impl in base class
    virtual void ParseArgList() {}

    HSA_API_Type m_apiID;  ///< HSA API ID, defined in HSAFdnCommon/HSAFunctionDefs.h
    bool         m_bIsAPI; ///< flag indicating if this item corresponds to an actual API
};

//------------------------------------------------------------------------------------
/// HSAAPIInfo descendant for memory APIs
//------------------------------------------------------------------------------------
class HSAMemoryAPIInfo : public HSAAPIInfo
{
public:
    /// Constructor
    HSAMemoryAPIInfo() : HSAAPIInfo(), m_size(0), m_shouldShowBandwidth(false)
    {
    }

    /// Virtual destructor
    virtual ~HSAMemoryAPIInfo() {}

    /// Parse the argument list
    virtual void ParseArgList()
    {
        unsigned int sizeArgPos = 0;
        bool doesAPIHaveSizeArg = false;

        if (HSA_API_Type_UNKNOWN == m_apiID)
        {
            m_apiID = HSAFunctionDefsUtils::Instance()->ToHSAAPIType(m_strName);
        }

        if (m_apiID == HSA_API_Type_hsa_memory_allocate || m_apiID == HSA_API_Type_hsa_memory_register ||
            m_apiID == HSA_API_Type_hsa_memory_deregister)
        {
            doesAPIHaveSizeArg = true;
            sizeArgPos = 1;
        }
        else if (m_apiID == HSA_API_Type_hsa_memory_copy)
        {
            doesAPIHaveSizeArg = true;
            sizeArgPos = 2;
            m_shouldShowBandwidth = true;
        }

        if (doesAPIHaveSizeArg)
        {
            std::vector<std::string> args;
            StringUtils::Split(args, m_ArgList, ";");

            if (sizeArgPos < args.size())
            {
                std::istringstream ss(args[sizeArgPos]);
                ss >> m_size;
            }
        }
    }

    size_t m_size;                ///< size of memory operation
    bool   m_shouldShowBandwidth; ///< flag indicating whether or not we should show bandwidth for this api
};

//------------------------------------------------------------------------------------
/// HSA Dispatch API info
//------------------------------------------------------------------------------------
class HSADispatchInfo : public HSAAPIInfo
{
public:
    /// Constructor
    HSADispatchInfo() :
        HSAAPIInfo(),
        m_uDim(0)
    {
        m_bIsAPI = false;
        m_apiID = HSA_API_Type_Non_API_Dispatch;
        m_localGroup[0] = m_localGroup[1] = m_localGroup[2] = 0;
        m_globalGroup[0] = m_globalGroup[1] = m_globalGroup[2] = 0;
    }

    std::string  m_strKernelName;      ///< Kernel name
    ULONGLONG    m_uiKernelHandle;     ///< Kernel handle
    std::string  m_strDeviceName;      ///< Device name
    std::string  m_strDeviceHandle;    ///< Device handle
    unsigned int m_queueIndex;         ///< Queue index
    std::string  m_strQueueHandle;     ///< Queue handle

    std::string m_strGroupWorkSize;    ///< Work-group size
    std::string m_strGlobalWorkSize;   ///< Global work size
#define HSA_MAX_DIM 3
    size_t m_localGroup[HSA_MAX_DIM];  ///< local group size
    size_t m_globalGroup[HSA_MAX_DIM]; ///< global group size
    size_t m_uDim;                     ///< group dimention
};

typedef std::map<osThreadId, std::vector<HSAAPIInfo*> > HSAAPIInfoMap;
typedef std::pair<osThreadId, std::vector<HSAAPIInfo*> > HSAAPIInfoMapPair;
typedef std::vector<HSADispatchInfo*> HSADispatchInfoList;

#endif // _HSA_API_INFO_H_
