//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class manages pointers to each saved API object for
///        API tracing.
//==============================================================================

#ifndef _API_INFO_MANAGER_BASE_H_
#define _API_INFO_MANAGER_BASE_H_

/// \defgroup APIInfoManager APIInfoManager
/// This module maintains pointers to each saved API object.
///
/// \ingroup CLTraceAgent
// @{
#include <ostream>
#include "TraceInfoManager.h"

#define RECORD_STACK_TRACE_FOR_API(p)  if (GlobalSettings::GetInstance()->m_params.m_bStackTrace && p->m_pStackEntry == NULL) \
    { \
        StackTracer::Instance()->GetStackTrace(p->m_stack, false); \
    }

typedef void (*TimerFunc)(void* param);

//------------------------------------------------------------------------------------
/// API trace entry base class
//------------------------------------------------------------------------------------
class APIBase : public ITraceEntry
{
public:
    /// Constructor
    APIBase() : m_ullStart(0), m_ullEnd(0), m_pStackEntry(NULL)
    {
        m_strName.clear();
    }

    /// Virtual destructor
    virtual ~APIBase()
    {
        SAFE_DELETE(m_pStackEntry);
    }

    /// Pure virtual to string
    /// \return string representation of the API
    virtual std::string ToString() = 0;

    /// Pure virtual function get return value string
    /// \return string representation of the return value;
    virtual std::string GetRetString() = 0;

    /// Write API entry
    /// \param sout output stream
    virtual void WriteAPIEntry(std::ostream& sout);

    /// Write timestamp entry
    /// \param sout output stream
    /// \param bTimeout a flag indicating output mode
    /// \return True if timestamps are ready
    virtual bool WriteTimestampEntry(std::ostream& sout, bool bTimeout);

    /// Write stack trace entry
    /// \param sout output stream
    virtual void WriteStackEntry(std::ostream& sout);

public:
    ULONGLONG m_ullStart;            ///< api start timestamp
    ULONGLONG m_ullEnd;              ///< api end timestamp
    StackEntry* m_pStackEntry;       ///< Stack entry
    std::string m_strName;           ///< API name
    std::vector<StackEntry> m_stack; ///< stack trace

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    APIBase(const APIBase& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    APIBase& operator=(const APIBase& obj);
};

//------------------------------------------------------------------------------------
/// APIInfoManagerBase manages captured api calls
//------------------------------------------------------------------------------------
class APIInfoManagerBase : public TraceInfoManager
{
public:

    /// Constructor
    APIInfoManagerBase();

    /// Destructor
    virtual ~APIInfoManagerBase();

    /// Set output file
    /// Set m_strTimestampFile = strFileName
    /// Set m_strAPIFile = strFileName without ext + ".apitrace" + ext
    /// \param strFileName output file name
    virtual void SetOutputFile(const std::string& strFileName);

    /// Save trace data to tmp file
    /// \param bForceFlush Force to write all data out no matter it's ready or not - used in Detach() only
    virtual void FlushTraceData(bool bForceFlush = false);

    /// Save to Atp File
    void SaveToOutputFile();

    /// Load API filter file if specified
    /// \param strFileName API filter file
    void LoadAPIFilterFile(const std::string& strFileName);

protected:
    /// Disable copy constructor
    /// \param obj obj
    APIInfoManagerBase(const APIInfoManagerBase& obj);

    /// Disable assignment operator
    /// \param obj obj
    /// \return lhs
    APIInfoManagerBase& operator = (const APIInfoManagerBase& obj);

    /// write captured api to stream
    /// \param sout output stream
    void WriteTimestampToStream(std::ostream& sout);

    /// write api trace data to stream
    /// \param sout output stream
    void WriteAPITraceDataToStream(std::ostream& sout);

    /// write stack entries to stream
    /// \param sout output stream
    void WriteStackTraceDataToStream(std::ostream& sout);

    /// Write non-API timing data to stream
    /// \param sout output stream
    virtual void FlushNonAPITimestampData(std::ostream& sout);

    /// Add the specified api to the list of APIs to filter
    /// \param strAPIName the name of the API to add to the filter
    virtual void AddAPIToFilter(const std::string& strAPIName);

    /// Gets a temp file name based on process id, thread id and extension
    /// \param pid the process id
    /// \param tid the thread id
    /// \param strExtension the extension
    /// \return the full path of the temp file name
    std::string GetTempFileName(osProcessId pid, osThreadId tid, std::string strExtension);

protected:
    ULONGLONG     m_ullStart;            ///< first time stamp of the whole program
    ULONGLONG     m_ullEnd;              ///< end time stamp of the whole program
    std::string   m_strOutputFile;       ///< Output file
    std::string   m_strTraceModuleName;  ///< Trace module name, this is used to identify trace module when doing merging in CodeXLGpuProfiler
};

// @}

#endif //_API_INFO_MANAGER_BASE_H_
