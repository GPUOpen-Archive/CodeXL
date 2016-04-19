//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class manages the CL enqueue APIs for API tracing.
//==============================================================================

#ifndef _CL_API_INFO_H_
#define _CL_API_INFO_H_

#include "../Common/APIInfo.h"
#include "../CLCommon/CLFunctionEnumDefs.h"

//------------------------------------------------------------------------------------
/// CL API Base class
//------------------------------------------------------------------------------------
class CLAPIInfo : public APIInfo
{
public:
    /// Virtual destructor
    virtual ~CLAPIInfo() {}
    CLAPIType m_Type = CL_UNKNOWN_API;  ///< CL API Type
    unsigned int m_uiAPIID = 0;         ///< CL API ID, defined in ../Common/CLFunctionEnumDefs.h
    std::string m_strComment;           ///< Comment
};

//------------------------------------------------------------------------------------
/// CL Enqueue API
//------------------------------------------------------------------------------------
class CLEnqueueAPI : public CLAPIInfo
{
public:
    /// Virtual destructor
    virtual ~CLEnqueueAPI() {}
    bool           m_bInfoMissing = false;  ///< Indicating whether or not Enqueue API information is missing
    std::string    m_strCMDType;            ///< Command type
    std::string    m_strDevice;             ///< Device name
    std::string    m_strCntxHandle;         ///< Context handle
    std::string    m_strCmdQHandle;         ///< Command queue handle
    std::string    m_strEventHandle;        ///< Event handle
    ULONGLONG      m_ullQueue = 0;          ///< Queue timestamp
    ULONGLONG      m_ullSubmit = 0;         ///< Submit timestamp
    ULONGLONG      m_ullRunning = 0;        ///< Running timestamp
    ULONGLONG      m_ullComplete = 0;       ///< Complete timestamp
    unsigned int   m_uiCMDType = 0;         ///< Command type enum
    unsigned int   m_uiQueueID = 0;         ///< Queue ID
    unsigned int   m_uiContextID = 0;       ///< Context ID
};

//------------------------------------------------------------------------------------
/// CL Memory operation API
//------------------------------------------------------------------------------------
class CLMemAPIInfo : public CLEnqueueAPI
{
public:
    unsigned int m_uiTransferSize;   ///< Transfer size
};


//------------------------------------------------------------------------------------
/// CL other enqueue operation API
//------------------------------------------------------------------------------------
class CLOtherEnqueueAPIInfo : public CLEnqueueAPI
{
};


//------------------------------------------------------------------------------------
/// CL enqueue operation with data
//------------------------------------------------------------------------------------
class CLDataEnqueueAPIInfo : public CLOtherEnqueueAPIInfo
{
public:
    unsigned int m_uiDataSize = 0;   ///< Transfer size
};

//------------------------------------------------------------------------------------
/// CL Kernel dispatch API
//------------------------------------------------------------------------------------
class CLKernelAPIInfo : public CLEnqueueAPI
{
public:
    std::string m_strKernelHandle;   ///< Kernel handle
    std::string m_strKernelName;     ///< Kernel name
    std::string m_strGroupWorkSize;  ///< Work-group size
    std::string m_strGlobalWorkSize; ///< Global work size
};

#endif //_CL_API_INFO_H_
