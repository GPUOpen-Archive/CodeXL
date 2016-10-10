//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Helpers for profile data CSV File Parsing and merging
//==============================================================================

#include "CSVFileMerger.h"
#include "../Common/FileUtils.h"
#include <sstream>

typedef std::vector<std::string> CounterNameList;
typedef std::pair<unsigned int, CounterNameList> CounterListFileIndexPair;
typedef std::vector<unsigned int> FileIndexList;
typedef std::pair<std::string, FileIndexList> CounterInFilesPair;
typedef std::vector<std::string> ThreadList;

const std::string KernelRowDataHelper::m_methodString = "Method";
const std::string KernelRowDataHelper::m_executionOrderString = "ExecutionOrder";
const std::string KernelRowDataHelper::m_threadIdString = "ThreadID";
const std::string KernelRowDataHelper::m_callIndexString = "CallIndex";
const std::string KernelRowDataHelper::m_globalWorkSizeString = "GlobalWorkSize";
const std::string KernelRowDataHelper::m_workGroupSizeString = "WorkGroupSize";
const std::string KernelRowDataHelper::m_localMemSizeString = "LocalMemSize";
const std::string KernelRowDataHelper::m_vgprsString = "VGPRs";
const std::string KernelRowDataHelper::m_sgprsString = "SGPRs";
const std::string KernelRowDataHelper::m_scratchRegsString = "ScratchRegs";

unsigned int KernelRowData::GetRowCount()
{
    return static_cast<unsigned int>(m_rows.size());
}

unsigned int KernelRowData::GetRowCountByThreadId(const std::string& threadId)
{
    return static_cast<unsigned int>(m_rowsByThreadId[threadId].size());
}

std::vector<std::string> KernelRowData::GetUniqueThreads()
{
    std::vector<std::string> threads;

    for (std::map<std::string, CSVRowList>::iterator kernelRowDataByThreadIterator = m_rowsByThreadId.begin();
         kernelRowDataByThreadIterator != m_rowsByThreadId.end();
         ++kernelRowDataByThreadIterator)
    {
        threads.push_back(kernelRowDataByThreadIterator->first);
    }

    return threads;
}

std::string KernelRowData::GetValueByThreadId(const std::string& threadId, const unsigned int& rowIndex, const std::string& columnName)
{
    std::string retValue;
    CSVRow* tempRow = GetRowByThreadId(threadId, rowIndex);

    if (nullptr != tempRow)
    {
        retValue = tempRow->operator[](columnName);
    }

    return retValue;
}

CSVRow* KernelRowData::GetRowByThreadId(const std::string& threadId, const unsigned int& rowIndex)
{
    CSVRow* row = nullptr;

    bool isThreadExist = m_rowsByThreadId.find(threadId) != m_rowsByThreadId.end();

    if (isThreadExist && rowIndex < m_rowsByThreadId[threadId].size())
    {
        row = m_rowsByThreadId[threadId].at(rowIndex);
    }

    return row;
}

void KernelRowData::OnParse(CSVRow* csvRow, bool& stopParsing)
{
    std::string threadIdString = "ThreadID";

    if (nullptr != csvRow)
    {
        m_rows.push_back(csvRow);
        std::string threadIdValueString = csvRow->GetRowData(threadIdString);
        bool succeed = m_rowsByThreadId.find(threadIdValueString) != m_rowsByThreadId.end();

        if (succeed)
        {
            m_rowsByThreadId[threadIdValueString].push_back(csvRow);
        }
        else
        {
            std::vector<CSVRow*> tempCSVRowVector;
            tempCSVRowVector.push_back(csvRow);
            m_rowsByThreadId.insert(std::pair<std::string, std::vector<CSVRow*>>(threadIdValueString, tempCSVRowVector));
        }
    }

    stopParsing = false;
}

bool KernelRowDataHelper::IsThreadMappingRow(CSVRow* firstRow, CSVRow* secondRow)
{
    bool equal = true;

    if (nullptr != firstRow && nullptr != secondRow && firstRow != secondRow)
    {
        equal &= StringUtils::Trim(firstRow->operator[](m_methodString)).compare(StringUtils::Trim(secondRow->operator[](m_methodString))) == 0;
        equal &= StringUtils::Trim(firstRow->operator[](m_executionOrderString)).compare(StringUtils::Trim(secondRow->operator[](m_executionOrderString))) == 0;
        equal &= StringUtils::Trim(firstRow->operator[](m_callIndexString)).compare(StringUtils::Trim(secondRow->operator[](m_callIndexString))) == 0;
        equal &= StringUtils::Trim(firstRow->operator[](m_globalWorkSizeString)).compare(StringUtils::Trim(secondRow->operator[](m_globalWorkSizeString))) == 0;
        equal &= StringUtils::Trim(firstRow->operator[](m_workGroupSizeString)).compare(StringUtils::Trim(secondRow->operator[](m_workGroupSizeString))) == 0;
        equal &= StringUtils::Trim(firstRow->operator[](m_localMemSizeString)).compare(StringUtils::Trim(secondRow->operator[](m_localMemSizeString))) == 0;
        equal &= StringUtils::Trim(firstRow->operator[](m_vgprsString)).compare(StringUtils::Trim(secondRow->operator[](m_vgprsString))) == 0;
        equal &= StringUtils::Trim(firstRow->operator[](m_sgprsString)).compare(StringUtils::Trim(secondRow->operator[](m_sgprsString))) == 0;
        equal &= StringUtils::Trim(firstRow->operator[](m_scratchRegsString)).compare(StringUtils::Trim(secondRow->operator[](m_scratchRegsString))) == 0;
    }
    else
    {
        equal = false;
    }

    return equal;
}

bool KernelRowDataHelper::IsCommonColumn(const std::string& columnName)
{
    bool isCommon = false;

    isCommon |= columnName.compare(m_methodString) == 0;
    isCommon |= columnName.compare(m_executionOrderString) == 0;
    isCommon |= columnName.compare(m_callIndexString) == 0;
    isCommon |= columnName.compare(m_globalWorkSizeString) == 0;
    isCommon |= columnName.compare(m_workGroupSizeString) == 0;
    isCommon |= columnName.compare(m_localMemSizeString) == 0;
    isCommon |= columnName.compare(m_vgprsString) == 0;
    isCommon |= columnName.compare(m_sgprsString) == 0;
    isCommon |= columnName.compare(m_scratchRegsString) == 0;

    return isCommon;
}

HeaderList KernelRowDataHelper::CreateHeader(const std::vector<std::string>& counterFileList, bool includeTime)
{
    HeaderList headersWithActualHeaderNameAndFileIndex;
    std::vector<CounterListFileIndexPair> counterNamesByFileIndex;

    bool succeedReadingCounterFile = true;
    unsigned int baseFileIndex = 0;

    if (counterFileList.size() > 1)
    {
        unsigned int fileIndex = 0;

        for (std::vector<std::string>::const_iterator counterFileListIterator = counterFileList.begin(); counterFileListIterator != counterFileList.end(); ++counterFileListIterator)
        {
            CounterNameList counternameList;
            succeedReadingCounterFile &= FileUtils::ReadFile(*counterFileListIterator, counternameList, true, true);

            if (succeedReadingCounterFile)
            {
                counterNamesByFileIndex.push_back(CounterListFileIndexPair(fileIndex, counternameList));
            }

            fileIndex++;
        }

        std::string timeString = "Time";
        std::string passString = "_pass_";

        headersWithActualHeaderNameAndFileIndex.push_back(HeaderPair(m_methodString, MethodFileIndexPair(m_methodString, baseFileIndex)));
        headersWithActualHeaderNameAndFileIndex.push_back(HeaderPair(m_executionOrderString, MethodFileIndexPair(m_executionOrderString, baseFileIndex)));

        for (unsigned int i = 0; i < counterFileList.size(); i++)
        {
            std::stringstream ss;
            ss << i;
            std::string threadIdWithPass = m_threadIdString + passString + ss.str();
            headersWithActualHeaderNameAndFileIndex.push_back(HeaderPair(threadIdWithPass, MethodFileIndexPair(m_threadIdString, i)));
        }

        headersWithActualHeaderNameAndFileIndex.push_back(HeaderPair(m_callIndexString, MethodFileIndexPair(m_callIndexString, baseFileIndex)));
        headersWithActualHeaderNameAndFileIndex.push_back(HeaderPair(m_globalWorkSizeString, MethodFileIndexPair(m_globalWorkSizeString, baseFileIndex)));
        headersWithActualHeaderNameAndFileIndex.push_back(HeaderPair(m_workGroupSizeString, MethodFileIndexPair(m_workGroupSizeString, baseFileIndex)));

        // noGputime flag is not set - need to include time in the file headers
        if (includeTime)
        {
            for (unsigned int i = 0; i < counterFileList.size(); i++)
            {
                std::stringstream ss;
                ss << i;
                std::string timeWithPass = timeString + passString + ss.str();
                headersWithActualHeaderNameAndFileIndex.push_back(HeaderPair(timeWithPass, MethodFileIndexPair(timeString, i)));
            }
        }

        headersWithActualHeaderNameAndFileIndex.push_back(HeaderPair(m_localMemSizeString, MethodFileIndexPair(m_localMemSizeString, baseFileIndex)));
        headersWithActualHeaderNameAndFileIndex.push_back(HeaderPair(m_vgprsString, MethodFileIndexPair(m_vgprsString, baseFileIndex)));
        headersWithActualHeaderNameAndFileIndex.push_back(HeaderPair(m_sgprsString, MethodFileIndexPair(m_sgprsString, baseFileIndex)));
        headersWithActualHeaderNameAndFileIndex.push_back(HeaderPair(m_scratchRegsString, MethodFileIndexPair(m_scratchRegsString, baseFileIndex)));

        std::vector<CounterInFilesPair> counterColumns;
        std::vector<CounterInFilesPair>::iterator counterColumnsIterator;

        for (std::vector<CounterListFileIndexPair>::iterator counterNamesByFileIndexIterator = counterNamesByFileIndex.begin();
             counterNamesByFileIndexIterator != counterNamesByFileIndex.end(); ++counterNamesByFileIndexIterator)
        {
            for (CounterNameList::iterator counterNamesIterator = counterNamesByFileIndexIterator->second.begin();
                 counterNamesIterator != counterNamesByFileIndexIterator->second.end(); ++counterNamesIterator)
            {
                bool isCounterExistInColumn = false;

                for (std::vector<CounterInFilesPair>::iterator counterColumnsTempIterator = counterColumns.begin(); counterColumnsTempIterator != counterColumns.end(); ++counterColumnsTempIterator)
                {
                    if (!isCounterExistInColumn)
                    {
                        isCounterExistInColumn |= counterColumnsTempIterator->first.compare(*counterNamesIterator) == 0;
                        counterColumnsIterator = counterColumnsTempIterator;
                    }
                }

                if (isCounterExistInColumn)
                {
                    counterColumnsIterator->second.push_back(counterNamesByFileIndexIterator->first);
                }
                else
                {
                    std::vector<unsigned int> tempFileIndex;
                    tempFileIndex.push_back(counterNamesByFileIndexIterator->first);
                    counterColumns.push_back(CounterInFilesPair(*counterNamesIterator, tempFileIndex));
                }
            }
        }


        for (counterColumnsIterator = counterColumns.begin(); counterColumnsIterator != counterColumns.end(); ++counterColumnsIterator)
        {
            if (counterColumnsIterator->second.size() > 1)
            {
                for (unsigned int i = 1; i <= counterColumnsIterator->second.size(); i++)
                {
                    std::stringstream stringStream;
                    stringStream << counterColumnsIterator->second.at(i - 1);
                    std::string columnNameWithPassString = counterColumnsIterator->first + passString + stringStream.str();
                    headersWithActualHeaderNameAndFileIndex.push_back(HeaderPair(columnNameWithPassString, MethodFileIndexPair(counterColumnsIterator->first, i - 1)));
                }
            }
            else
            {
                headersWithActualHeaderNameAndFileIndex.push_back(HeaderPair(counterColumnsIterator->first, MethodFileIndexPair(counterColumnsIterator->first, counterColumnsIterator->second.front())));
            }
        }
    }

    return headersWithActualHeaderNameAndFileIndex;
}

MappedThreadSetList KernelRowDataHelper::SortMappedThreadByExecutionOrder(std::map<unsigned int, KernelRowData*> allFilesRowData, MappedThreadSetList mappedThreads)
{
    MappedThreadSetList sortedMappedThreadByExecutionOrder;
    MappedThreadSetList::iterator mappedThreadIterator;
    std::vector<std::pair<std::string, unsigned int>> executionOrders;

    unsigned int firstRow = 0;
    std::map<std::string, unsigned int> executionOrderAndIndex;

    unsigned int index = 0;

    for (mappedThreadIterator = mappedThreads.begin(); mappedThreadIterator != mappedThreads.end(); ++mappedThreadIterator)
    {
        std::string executionOrder = allFilesRowData[mappedThreadIterator->begin()->second]->GetValueByThreadId(mappedThreadIterator->begin()->first, firstRow, m_executionOrderString);
        executionOrders.push_back(std::pair<std::string, unsigned int>(executionOrder, index));
        index++;
    }

    unsigned int outerIndex = 0;

    for (std::vector<std::pair<std::string, unsigned int>>::iterator outerIter = executionOrders.begin(); outerIter != executionOrders.end(); ++outerIter)
    {
        std::string exectionOrder = outerIter->first;
        unsigned int indexToSwap = outerIndex;
        unsigned int innerIndex = outerIndex + 1;

        for (std::vector<std::pair<std::string, unsigned int>>::iterator innerIter = outerIter + 1; innerIter != executionOrders.end(); ++innerIter)
        {
            if (exectionOrder.compare(innerIter->first) > 0)
            {
                exectionOrder = innerIter->first;
                indexToSwap = innerIndex;
            }

            innerIndex++;
        }

        if (indexToSwap != outerIndex)
        {
            std::pair<std::string, unsigned int> tempPair = executionOrders[indexToSwap];
            executionOrders[indexToSwap] = executionOrders.at(outerIndex);
            executionOrders[outerIndex] = tempPair;
        }

        outerIndex++;
    }

    for (std::vector<std::pair<std::string, unsigned int>>::iterator outerIter = executionOrders.begin(); outerIter != executionOrders.end(); ++outerIter)
    {
        sortedMappedThreadByExecutionOrder.push_back(mappedThreads[outerIter->second]);
    }

    return sortedMappedThreadByExecutionOrder;
}


MappedThreadSetList KernelRowDataHelper::GetMappedThreads(std::map<unsigned int, KernelRowData*> allFilesRowData)
{
    MappedThreadSetList mappedThreads;
    MappedThreadSetList::iterator mappedThreadsIterator;

    if (allFilesRowData.size() > 1)
    {
        KernelRowData* baseRowData = allFilesRowData.begin()->second;
        ThreadList baseRowDataThreads = baseRowData->GetUniqueThreads();

        // Iterate for all threads in base files
        for (ThreadList::iterator baseRowDataThreadsIter = baseRowDataThreads.begin();
             baseRowDataThreadsIter != baseRowDataThreads.end(); ++baseRowDataThreadsIter)
        {
            ThreadFileIndexPair baseFileCurrentThreadPair(*baseRowDataThreadsIter, 0);
            MappedThreadSet mappedThreadSet;
            mappedThreadSet.insert(baseFileCurrentThreadPair);

            unsigned int fileIndex = 0;

            // Itreate for all files
            for (std::map<unsigned int, KernelRowData*>::iterator rowPerFileIterator = allFilesRowData.begin();
                 rowPerFileIterator != allFilesRowData.end(); ++rowPerFileIterator)
            {
                // Do except for base files
                if (baseRowData != rowPerFileIterator->second)
                {
                    fileIndex++;

                    ThreadList currentFileUniqueThreads = rowPerFileIterator->second->GetUniqueThreads();
                    unsigned int baseFileRowDataCountByThread = baseRowData->GetRowCountByThreadId(*baseRowDataThreadsIter);

                    for (ThreadList::iterator currentFileThreadIter = currentFileUniqueThreads.begin();
                         currentFileThreadIter != currentFileUniqueThreads.end(); currentFileThreadIter++)
                    {
                        unsigned int currentFileRowDataCountForCurrentThread = rowPerFileIterator->second->GetRowCountByThreadId(*currentFileThreadIter);
                        unsigned int rowsToCheck = baseFileRowDataCountByThread < currentFileRowDataCountForCurrentThread ? baseFileRowDataCountByThread : currentFileRowDataCountForCurrentThread;
                        bool isMapped = true;

                        for (unsigned int rowIterForCurrentThread = 0; rowIterForCurrentThread < rowsToCheck; rowIterForCurrentThread++)
                        {
                            isMapped &= IsThreadMappingRow(baseRowData->GetRowByThreadId(*baseRowDataThreadsIter, rowIterForCurrentThread),
                                                           rowPerFileIterator->second->GetRowByThreadId(*currentFileThreadIter, rowIterForCurrentThread));
                        }

                        if (isMapped)
                        {
                            ThreadFileIndexPair currentThreadPair(*currentFileThreadIter, fileIndex);
                            mappedThreadSet.insert(currentThreadPair);
                            break;
                        }
                    }
                }
            }

            mappedThreads.push_back(mappedThreadSet);
        }

        // Reinitialize
        unsigned int fileIndex = 0;
        baseRowData = allFilesRowData.begin()->second;

        // Itreate for all files
        for (std::map<unsigned int, KernelRowData*>::iterator rowPerFileIterator = allFilesRowData.begin();
             rowPerFileIterator != allFilesRowData.end(); ++rowPerFileIterator)
        {
            std::string executionOrder;

            // Do except for base files
            if (baseRowData != rowPerFileIterator->second)
            {
                fileIndex++;
                ThreadList currentFileUniqueThreads = rowPerFileIterator->second->GetUniqueThreads();

                for (ThreadList::iterator currentFileThreadIter = currentFileUniqueThreads.begin();
                     currentFileThreadIter != currentFileUniqueThreads.end(); currentFileThreadIter++)
                {
                    bool isThreadfound = false;

                    for (mappedThreadsIterator = mappedThreads.begin(); mappedThreadsIterator != mappedThreads.end(); ++mappedThreadsIterator)
                    {
                        for (MappedThreadSet::iterator currentSetIter = mappedThreadsIterator->begin();
                             currentSetIter != mappedThreadsIterator->end(); ++currentSetIter)
                        {
                            isThreadfound |= (currentSetIter->first.compare(*currentFileThreadIter) == 0) &&
                                             (currentSetIter->second == fileIndex);
                        }
                    }

                    if (!isThreadfound)
                    {
                        MappedThreadSet unMappedThreadSet;
                        ThreadFileIndexPair unMappedThreadPair(*currentFileThreadIter, fileIndex);
                        unMappedThreadSet.insert(unMappedThreadPair);
                        mappedThreads.push_back(unMappedThreadSet);
                    }
                }
            }
        }
    }

    return SortMappedThreadByExecutionOrder(allFilesRowData, mappedThreads);
}