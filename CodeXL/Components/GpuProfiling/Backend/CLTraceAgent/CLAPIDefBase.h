//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file defines the CLAPI abstract base class.
//==============================================================================

#ifndef _CL_API_DEF_BASE_H_
#define _CL_API_DEF_BASE_H_

/// \ingroup CLAPIDefs
// @{

#include <memory>
#include <string>
#include <vector>
#include <CL/opencl.h>
#include "CLStringUtils.h"
#include "../Common/OSUtils.h"
#include "../Common/Defs.h"
#include "../Common/StackTracer.h"
#include "../Common/APITraceUtils.h"
#include "../CLCommon/CLFunctionEnumDefs.h"
#include "APIInfoManagerBase.h"

struct CLEvent;

typedef std::shared_ptr<CLEvent> CLEventPtr;

//------------------------------------------------------------------------------------
/// CLAPI base abstract class
//------------------------------------------------------------------------------------
class CLAPIBase : public APIBase
{
public:
    /// Constructor
    CLAPIBase() :
        APIBase(),
        m_type(CL_FUNC_TYPE_Unknown),
        m_apiType(CL_API)
    {
    }

    /// Virtual destructor
    virtual ~CLAPIBase()
    {
    }

    /// Write API entry
    /// \param sout output stream
    void WriteAPIEntry(std::ostream& sout)
    {
        m_strName = CLStringUtils::GetCLAPINameString(m_type);
        APIBase::WriteAPIEntry(sout);
    }

    /// Search for OpenCL API call stack frame
    void CreateStackEntry();

    /// Write stack entry
    /// \param sout output stream
    void WriteStackEntry(std::ostream& sout)
    {
        if (m_pStackEntry == NULL)
        {
            // Search for OpenCL API call stack frame lazily
            CreateStackEntry();
        }

        APIBase::WriteStackEntry(sout);
    }

    /// Write timestamp entry
    /// \param sout output stream
    /// \param bTimeout a flag indicating output mode
    /// \return True if timestamps are ready
    virtual bool WriteTimestampEntry(std::ostream& sout, bool bTimeout);

    CL_FUNC_TYPE m_type;                   ///< api type enum
    CLAPIType m_apiType;                   ///< api type
#ifdef AMDT_INTERNAL
    std::vector<ULONGLONG> m_PrePMCs;      ///< Pre API call PMC values
    std::vector<ULONGLONG> m_PostPMCs;     ///< Post API call PMC values
#endif

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLAPIBase(const CLAPIBase& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    CLAPIBase& operator=(const CLAPIBase& obj);

#ifdef WIN32
    /// Helper function to determine if a stack frame is the one that should be used in the symbol file
    /// \param strSymName the symbol name for the stack frame
    /// \param strModName the module name for the stack frame
    /// \return true if this is a stack frame to use
    bool IsValidStackEntry(const std::string& strSymName, const std::string& strModName);
#endif
};

//------------------------------------------------------------------------------------
/// CLEnqueueAPI base abstract class
//------------------------------------------------------------------------------------
class CLEnqueueAPIBase : public CLAPIBase
{
    friend class CLAPIInfoManager;
public:
    //Constructor
    CLEnqueueAPIBase() : CLAPIBase()
    {
        m_apiType = CL_ENQUEUE_BASE_API;
    }

    /// virtual destructor
    virtual ~CLEnqueueAPIBase() {}

    /// Pure virtual function - Return whether the API succeeded
    /// \return true if the API succeeded (CL_SUCCESS), false otherwise
    virtual bool GetAPISucceeded() const = 0;

    /// Is this enqueue command finished
    /// \return true if it's finished
    bool IsReady();

    /// Write timestamp entry
    /// \param sout output stream
    /// \param bTimeout a flag indicating output mode
    /// \return True if timestamps are ready
    virtual bool WriteTimestampEntry(std::ostream& sout, bool bTimeout);

    /// Get CLEvent object
    /// \return const ptr to CLEvent object
    const CLEvent* GetEvent() const
    {
        return m_pEvent.get();
    }

protected:
    /// Query context pointer, context id, queue id from queue pointer
    void GetContextInfo();

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLEnqueueAPIBase(const CLEnqueueAPIBase& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    CLEnqueueAPIBase& operator=(const CLEnqueueAPIBase& obj);

protected:
    const cl_event* m_event_wait_list;           ///< Wait list passed to the API
    cl_uint  m_num_events_in_wait_list;          ///< Number of events in wait list
    std::vector<cl_event> m_vecEvent_wait_list;  ///< Wait list
    CLEventPtr m_pEvent;                         ///< Event wrapper object
    cl_command_queue m_command_queue;            ///< cmd queue
    cl_context m_context;                        ///< context objcet
    cl_uint m_uiContextID;                       ///< context id
    cl_uint m_uiQueueID;                         ///< queue id
    std::string m_strDeviceName;                 ///< device name
};

//------------------------------------------------------------------------------------
/// CLEnqueueDataTransfer
//------------------------------------------------------------------------------------
class CLEnqueueDataTransfer : public CLEnqueueAPIBase
{
public:
    /// Constructor
    CLEnqueueDataTransfer() : CLEnqueueAPIBase()
    {
        m_apiType = CL_ENQUEUE_MEM;
    }

    /// virtual destructor
    virtual ~CLEnqueueDataTransfer() {}

    /// Pure virtual function - Get data transfer size in byte
    /// \return data transfer size in byte
    virtual size_t GetDataSize() const = 0;

    /// Write timestamp entry
    /// \param sout output stream
    /// \param bTimeout a flag indicating output mode
    /// \return True if timestamps are ready
    virtual bool WriteTimestampEntry(std::ostream& sout, bool bTimeout);

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLEnqueueDataTransfer(const CLEnqueueDataTransfer& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    CLEnqueueDataTransfer& operator=(const CLEnqueueDataTransfer& obj);
};


//------------------------------------------------------------------------------------
/// CLEnqueueOther base abstract class
//------------------------------------------------------------------------------------
class CLEnqueueOther : public CLEnqueueAPIBase
{
public:
    /// Constructor
    CLEnqueueOther() : CLEnqueueAPIBase()
    {
        m_apiType = CL_ENQUEUE_OTHER_OPERATIONS;
    }

    /// virtual destructor
    virtual ~CLEnqueueOther() {}

    /// Write timestamp entry
    /// \param sout output stream
    /// \param bTimeout a flag indicating output mode
    /// \return True if timestamps are ready
    virtual bool WriteTimestampEntry(std::ostream& sout, bool bTimeout);

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLEnqueueOther(const CLEnqueueOther& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    CLEnqueueOther& operator=(const CLEnqueueOther& obj);
};


//------------------------------------------------------------------------------------
/// CLEnqueueData
//------------------------------------------------------------------------------------
class CLEnqueueData : public CLEnqueueOther
{
public:
    /// Constructor
    CLEnqueueData() : CLEnqueueOther()
    {
        m_apiType = CL_ENQUEUE_DATA_OPERATIONS;
    }

    /// virtual destructor
    virtual ~CLEnqueueData() {}

    /// Pure virtual function - Get data size in byte
    /// \return data size in byte
    virtual size_t GetDataSize() const = 0;

    /// Write timestamp entry
    /// \param sout output stream
    /// \param bTimeout a flag indicating output mode
    /// \return True if timestamps are ready
    virtual bool WriteTimestampEntry(std::ostream& sout, bool bTimeout);

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    CLEnqueueData(const CLEnqueueData& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    CLEnqueueData& operator=(const CLEnqueueData& obj);
};

// @}

#endif //_CL_API_DEF_BASE_H_
