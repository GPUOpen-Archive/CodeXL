//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief KerneProfileResult manager. This class consolidate profile results from different sources
///        and output in a single csv file.
//==============================================================================

#ifndef _KERNEL_PROFILE_RESULT_MANAGER_H_
#define _KERNEL_PROFILE_RESULT_MANAGER_H_

#include <string>
#include <vector>
#include <list>
#include <map>
#include <deque>

#include <AMDTOSWrappers/Include/osOSDefinitions.h>

#include "CSVFileParser.h"
#include "OSUtils.h"
#include "StringUtils.h"
#include "TSingleton.h"
#include "AMDTMutex.h"

typedef CSVRow KernelInfoRow;

#define RegisterProfileResultSource(source) \
    vector<string> source##items;   \
    source::GetResultColumns(source##items); \
    KernelProfileResultManager::Instance()->AddProfileResultItems(source##items)

//------------------------------------------------------------------------------------
/// Kernel profile result manager. It manages different profiler sources and consolidate
/// output results in a single file.
//------------------------------------------------------------------------------------
class KernelProfileResultManager : public TSingleton<KernelProfileResultManager>
{
public:
    /// Constructor
    KernelProfileResultManager(void);

    /// Destructor
    ~KernelProfileResultManager(void);

    /// Called during entering intercetped_clEnqueueNDRangeKernel
    /// The first call will cause the header to be written
    /// Nested call generates sub-kernel row.
    /// \return KernelInfoRow for convenient editing.
    KernelInfoRow* BeginKernelInfo();

    /// Called anywhere within intercetped_clEnqueueNDRangeKernel, it writes to most recent
    /// openned row.
    /// \param strColName Column name
    /// \param strValue value
    /// \return false if no row is open for editing
    bool WriteKernelInfo(const std::string& strColName, const std::string& strValue);

    /// Called anywhere within intercetped_clEnqueueNDRangeKernel, it writes to most recent
    /// openned row.
    /// \param strColName Column name
    /// \param val value
    /// \return false if no row is open for editing
    template <class T>
    bool WriteKernelInfo(const std::string& strColName, T val)
    {
        return WriteKernelInfo(strColName, StringUtils::ToString(val));
    }

    /// This Call ends editing for most recent row
    /// If no more row left for editing, all rows are flushed to disk
    void EndKernelInfo();

    /// Set output file
    /// \param fileName Output file
    void SetOutputFile(const std::string& fileName);

    /// Add result item - used as column of csv file header
    /// \param results A list of column names
    void AddProfileResultItem(const std::string result)
    {
        SpAssertRet(m_pWriter != NULL);
        m_pWriter->AddColumn(result);
    }

    /// Add header entry
    /// \param strHeader header string
    void AddHeader(const std::string& strHeader)
    {
        SpAssertRet(m_pWriter != NULL);
        m_pWriter->AddHeader(strHeader);
    }

    /// Get columns
    /// \param columnsOut Output columns
    void GetColumns(std::vector<std::string>& columnsOut) const
    {
        SpAssertRet(m_pWriter != NULL);
        columnsOut = m_pWriter->GetColumns();
    }

    /// Add result items - used as column of csv file header
    /// \param results A list of column names
    void AddProfileResultItems(std::vector<std::string>& results);

    /// Set list separator
    /// \param ch list separator
    void SetListSeparator(char ch)
    {
        SpAssertRet(m_pWriter != NULL);
        m_pWriter->SetListSeparator(ch);
    }

protected:
    /// Write csv file header
    void WriteHeader();

    CSVFileWriter* m_pWriter;                                       ///< CSV file writter
    std::map<osThreadId, std::deque<CSVRow*> > m_pCurrentKernelRow; ///< Per thread kernel info
    bool m_bWriteHeader;                                            ///< Flag indicating whether or not header has been written
    std::string m_strOutputFile;                                    ///< Output file
    AMDTMutex m_mtx;                                                ///< Mutex object
};

#endif //_KERNEL_PROFILE_RESULT_MANAGER_H_
