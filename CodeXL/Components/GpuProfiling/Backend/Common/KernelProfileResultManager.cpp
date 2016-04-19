//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief KerneProfileResult manager. This class consolidate profile results from different sources
///        and output in a single csv file.
//==============================================================================

#include <AMDTOSWrappers/Include/osThread.h>

#include "KernelProfileResultManager.h"
#include "Logger.h"

using namespace std;

KernelProfileResultManager::KernelProfileResultManager(void) : m_pWriter(NULL), m_bWriteHeader(false)
{
    m_strOutputFile.clear();
}

KernelProfileResultManager::~KernelProfileResultManager(void)
{
}

KernelInfoRow* KernelProfileResultManager::BeginKernelInfo()
{
    SpAssertRet(m_pWriter != NULL) NULL;

    WriteHeader();

    AMDTScopeLock lock(&m_mtx);

    KernelInfoRow* pRow = m_pWriter->AddRow();

    osThreadId tid = osGetUniqueCurrentThreadId();
    m_pCurrentKernelRow[tid].push_back(pRow);
    return pRow;
}

bool KernelProfileResultManager::WriteKernelInfo(const std::string& strColName, const std::string& strValue)
{
    AMDTScopeLock lock(&m_mtx);
    osThreadId tid = osGetUniqueCurrentThreadId();

    deque<CSVRow*>& rowStack = m_pCurrentKernelRow[tid];

    if (!rowStack.empty())
    {
        CSVRow& row = *rowStack.front();
        row[strColName] = strValue;
        return true;
    }
    else
    {
        SpBreak("Stack empty: No KernelInfoRow for editing");
        return false;
    }
}

void KernelProfileResultManager::EndKernelInfo()
{
    AMDTScopeLock lock(&m_mtx);
    osThreadId tid = osGetUniqueCurrentThreadId();

    deque<CSVRow*>& rowStack = m_pCurrentKernelRow[tid];

    if (!rowStack.empty())
    {
        rowStack.pop_back();

        if (rowStack.empty())
        {
            // Only flush data when no row is in editing mode.
            SpAssertRet(m_pWriter != NULL);
            m_pWriter->Flush();
        }
    }
    else
    {
        SpBreak("Stack empty: No KernelInfoRow for editing");
    }
}

void KernelProfileResultManager::WriteHeader()
{
    if (m_bWriteHeader)
    {
        return;
    }

    SpAssertRet(m_pWriter != NULL);

    m_pWriter->Flush();
    m_bWriteHeader = true;
}

void KernelProfileResultManager::AddProfileResultItems(std::vector<std::string>& results)
{
    SpAssertRet(m_pWriter != NULL);
    m_pWriter->AddColumns(results);
}

void KernelProfileResultManager::SetOutputFile(const std::string& fileName)
{
    if (m_strOutputFile != fileName)
    {
        SAFE_DELETE(m_pWriter);
    }

    m_strOutputFile = fileName;

    if (m_pWriter == NULL)
    {
        m_pWriter = new(nothrow) CSVFileWriter(m_strOutputFile);
    }
}
