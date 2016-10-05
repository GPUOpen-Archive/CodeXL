//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Helpers for profile data CSV File Pasrsing and merging
//==============================================================================

#include "CSVFileMerger.h"
#include "../Common/FileUtils.h"
#include <sstream>

unsigned int KernelRowData::GetRowCount()
{
    return static_cast<unsigned int>(m_rows.size());
}

unsigned int KernelRowData::GetRowCountByThreadId(const std::string& threadId)
{
    return static_cast<unsigned int>(m_rowsbyThreadId[threadId].size());
}

std::vector<std::string> KernelRowData::GetUniqueThreads()
{
    std::vector<std::string> threads;

    for (std::map<std::string, std::vector<CSVRow*>>::iterator kernelRowDataByThreadIterator = m_rowsbyThreadId.begin();
         kernelRowDataByThreadIterator != m_rowsbyThreadId.end();
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

    bool isThreadExist = m_rowsbyThreadId.find(threadId) != m_rowsbyThreadId.end();

    if (isThreadExist && rowIndex < m_rowsbyThreadId[threadId].size())
    {
        row = m_rowsbyThreadId[threadId].at(rowIndex);
    }

    return row;
}

void KernelRowData::OnParse(CSVRow* csvRow, bool& stopParsing)
{
    std::string threadIdString = "ThreadID";

    if (!stopParsing && nullptr != csvRow)
    {
        m_rows.push_back(csvRow);
        std::string threadIdValueString = csvRow->GetRowData(threadIdString);
        bool succeed = m_rowsbyThreadId.find(threadIdValueString) != m_rowsbyThreadId.end();

        if (succeed)
        {
            m_rowsbyThreadId[threadIdValueString].push_back(csvRow);
        }
        else
        {
            std::vector<CSVRow*> tempCSVRowVector;
            tempCSVRowVector.push_back(csvRow);
            m_rowsbyThreadId.insert(std::pair<std::string, std::vector<CSVRow*>>(threadIdValueString, tempCSVRowVector));
        }
    }
}

std::string KernelRowDataHelper::RemoveLineFeed(const std::string& string)
{
    std::string tempString = string;

    char ch = tempString.c_str()[tempString.size() - 1];

    if (ch == '\r' || ch == '\n')
    {
        tempString.pop_back();
    }

    return tempString;
}

bool KernelRowDataHelper::IsThreadMappingRow(CSVRow* firstRow, CSVRow* secondRow)
{
    bool equal = true;
    std::string threadIdString = "ThreadID";
    std::string threadIdFirstRow = firstRow->operator[](threadIdString);
    std::string threadIdSecondRow = secondRow->operator[](threadIdString);

    if (nullptr != firstRow && nullptr != secondRow && firstRow != secondRow)
    {
        std::string methodString = "Method";
        std::string executionOrderString = "ExecutionOrder";
        std::string callIndexString = "CallIndex";
        std::string globalWorkSizeString = "GlobalWorkSize";
        std::string workGroupSizeString = "WorkGroupSize";
        std::string localMemSizeString = "LocalMemSize";
        std::string vgprsString = "VGPRs";
        std::string sgprsString = "SGPRs";
        std::string scratchRegsString = "ScratchRegs";

        equal &= RemoveLineFeed(firstRow->operator[](methodString)).compare(RemoveLineFeed(secondRow->operator[](methodString))) == 0;
        equal &= RemoveLineFeed(firstRow->operator[](executionOrderString)).compare(RemoveLineFeed(secondRow->operator[](executionOrderString))) == 0;
        equal &= RemoveLineFeed(firstRow->operator[](callIndexString)).compare(RemoveLineFeed(secondRow->operator[](callIndexString))) == 0;
        equal &= RemoveLineFeed(firstRow->operator[](globalWorkSizeString)).compare(RemoveLineFeed(secondRow->operator[](globalWorkSizeString))) == 0;
        equal &= RemoveLineFeed(firstRow->operator[](workGroupSizeString)).compare(RemoveLineFeed(secondRow->operator[](workGroupSizeString))) == 0;
        equal &= RemoveLineFeed(firstRow->operator[](localMemSizeString)).compare(RemoveLineFeed(secondRow->operator[](localMemSizeString))) == 0;
        equal &= RemoveLineFeed(firstRow->operator[](vgprsString)).compare(RemoveLineFeed(secondRow->operator[](vgprsString))) == 0;
        equal &= RemoveLineFeed(firstRow->operator[](sgprsString)).compare(RemoveLineFeed(secondRow->operator[](sgprsString))) == 0;
        equal &= RemoveLineFeed(firstRow->operator[](scratchRegsString)).compare(RemoveLineFeed(secondRow->operator[](scratchRegsString))) == 0;
    }
    else
    {
        equal = false;
    }

    return equal;
}


bool KernelRowDataHelper::IsCommonColumn(const std::string& columnName)
{
    std::string methodString = "Method";
    std::string executionOrderString = "ExecutionOrder";
    std::string callIndexString = "CallIndex";
    std::string globalWorkSizeString = "GlobalWorkSize";
    std::string workGroupSizeString = "WorkGroupSize";
    std::string localMemSizeString = "LocalMemSize";
    std::string vgprsString = "VGPRs";
    std::string sgprsString = "SGPRs";
    std::string scratchRegsString = "ScratchRegs";

    bool isCommon = false;

    isCommon |= columnName.compare(methodString) == 0;
    isCommon |= columnName.compare(executionOrderString) == 0;
    isCommon |= columnName.compare(callIndexString) == 0;
    isCommon |= columnName.compare(globalWorkSizeString) == 0;
    isCommon |= columnName.compare(workGroupSizeString) == 0;
    isCommon |= columnName.compare(localMemSizeString) == 0;
    isCommon |= columnName.compare(vgprsString) == 0;
    isCommon |= columnName.compare(sgprsString) == 0;
    isCommon |= columnName.compare(scratchRegsString) == 0;

    return isCommon;
}

std::vector<std::pair<std::string, std::pair<std::string, unsigned int>>> KernelRowDataHelper::CreateHeader(const std::vector<std::string>& counterFileList, bool includeTime)
{
    std::vector<std::pair<std::string, std::pair<std::string, unsigned int>>> headersWithActualHeaderNameAndFileIndex;
    std::vector<std::pair<unsigned int, std::vector<std::string>>> counterNamesByFileIndex;

    bool succeedReadingCounterFile = true;
    unsigned int baseFileIndex = 0;

    if (counterFileList.size() > 1)
    {
        unsigned int fileIndex = 0;

        for (std::vector<std::string>::const_iterator counterFileListIterator = counterFileList.begin(); counterFileListIterator != counterFileList.end(); ++counterFileListIterator)
        {
            std::vector<std::string> counternames;
            succeedReadingCounterFile &= FileUtils::ReadFile(*counterFileListIterator, counternames, true, true);

            if (succeedReadingCounterFile)
            {
                counterNamesByFileIndex.push_back(std::pair<unsigned int, std::vector<std::string>>(fileIndex, counternames));
            }

            fileIndex++;
        }

        // Common Columns
        std::string methodString = "Method";
        std::string executionOrderString = "ExecutionOrder";
        std::string threadIdString = "ThreadID";
        std::string callIndexString = "CallIndex";
        std::string globalWorkSizeString = "GlobalWorkSize";
        std::string workGroupSizeString = "WorkGroupSize";
        std::string timeString = "Time";
        std::string localMemSizeString = "LocalMemSize";
        std::string vgprsString = "VGPRs";
        std::string sgprsString = "SGPRs";
        std::string scratchRegsString = "ScratchRegs";
        std::string passString = "_pass_";

        headersWithActualHeaderNameAndFileIndex.push_back(std::pair<std::string, std::pair<std::string, unsigned int>>(methodString, std::pair<std::string, unsigned int>(methodString, baseFileIndex)));
        headersWithActualHeaderNameAndFileIndex.push_back(std::pair<std::string, std::pair<std::string, unsigned int>>(executionOrderString, std::pair<std::string, unsigned int>(executionOrderString, baseFileIndex)));

        for (unsigned int i = 0; i < counterFileList.size(); i++)
        {
            std::stringstream ss;
            ss << i;
            std::string threadIdWithPass = threadIdString + passString + ss.str();
            headersWithActualHeaderNameAndFileIndex.push_back(std::pair<std::string, std::pair<std::string, unsigned int>>(threadIdWithPass, std::pair<std::string, unsigned int>(threadIdString, i)));
        }

        headersWithActualHeaderNameAndFileIndex.push_back(std::pair<std::string, std::pair<std::string, unsigned int>>(callIndexString, std::pair<std::string, unsigned int>(callIndexString, baseFileIndex)));
        headersWithActualHeaderNameAndFileIndex.push_back(std::pair<std::string, std::pair<std::string, unsigned int>>(globalWorkSizeString, std::pair<std::string, unsigned int>(globalWorkSizeString, baseFileIndex)));
        headersWithActualHeaderNameAndFileIndex.push_back(std::pair<std::string, std::pair<std::string, unsigned int>>(workGroupSizeString, std::pair<std::string, unsigned int>(workGroupSizeString, baseFileIndex)));

        // noGputime flag is not set - need to include time in the file headers
        if (includeTime)
        {
            for (unsigned int i = 0; i < counterFileList.size(); i++)
            {
                std::stringstream ss;
                ss << i;
                std::string timeWithPass = timeString + passString + ss.str();
                headersWithActualHeaderNameAndFileIndex.push_back(std::pair<std::string, std::pair<std::string, unsigned int>>(timeWithPass, std::pair<std::string, unsigned int>(timeString, i)));
            }
        }

        headersWithActualHeaderNameAndFileIndex.push_back(std::pair<std::string, std::pair<std::string, unsigned int>>(localMemSizeString, std::pair<std::string, unsigned int>(localMemSizeString, baseFileIndex)));
        headersWithActualHeaderNameAndFileIndex.push_back(std::pair<std::string, std::pair<std::string, unsigned int>>(vgprsString, std::pair<std::string, unsigned int>(vgprsString, baseFileIndex)));
        headersWithActualHeaderNameAndFileIndex.push_back(std::pair<std::string, std::pair<std::string, unsigned int>>(sgprsString, std::pair<std::string, unsigned int>(sgprsString, baseFileIndex)));
        headersWithActualHeaderNameAndFileIndex.push_back(std::pair<std::string, std::pair<std::string, unsigned int>>(scratchRegsString, std::pair<std::string, unsigned int>(scratchRegsString, baseFileIndex)));

        std::vector<std::pair<std::string, std::vector<unsigned int>>> counterColumns;
        std::vector<std::pair<std::string, std::vector<unsigned int>>>::iterator counterColumnsIterator;

        for (std::vector<std::pair<unsigned int, std::vector<std::string>>>::iterator counterNamesByFileIndexIterator = counterNamesByFileIndex.begin();
             counterNamesByFileIndexIterator != counterNamesByFileIndex.end(); ++counterNamesByFileIndexIterator)
        {
            for (std::vector<std::string>::iterator counterNamesIterator = counterNamesByFileIndexIterator->second.begin();
                 counterNamesIterator != counterNamesByFileIndexIterator->second.end(); ++counterNamesIterator)
            {
                bool isCounterExistInColumn = false;

                for (std::vector<std::pair<std::string, std::vector<unsigned int>>>::iterator counterColumnsTempIterator = counterColumns.begin(); counterColumnsTempIterator != counterColumns.end(); ++counterColumnsTempIterator)
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
                    counterColumns.push_back(std::pair<std::string, std::vector<unsigned int>>(*counterNamesIterator, tempFileIndex));
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
                    headersWithActualHeaderNameAndFileIndex.push_back(std::pair<std::string, std::pair<std::string, unsigned int>>(columnNameWithPassString, std::pair<std::string, unsigned int>(counterColumnsIterator->first, i - 1)));
                }
            }
            else
            {
                headersWithActualHeaderNameAndFileIndex.push_back(std::pair<std::string, std::pair<std::string, unsigned int>>(counterColumnsIterator->first, std::pair<std::string, unsigned int>(counterColumnsIterator->first, counterColumnsIterator->second.front())));
            }
        }
    }

    return headersWithActualHeaderNameAndFileIndex;
}


std::vector<std::set<std::pair<std::string, unsigned int>>> KernelRowDataHelper::SortMappedThreadByExecutionOrder(std::map<unsigned int, KernelRowData*> allFilesRowData, std::vector<std::set<std::pair<std::string, unsigned int>>> mappedThreads)
{
    std::vector<std::set<std::pair<std::string, unsigned int>>> sortedMappedThreadByExecutionOrder;
    std::vector<std::set<std::pair<std::string, unsigned int>>>::iterator mappedThreadIterator;
    std::vector<std::pair<std::string, unsigned int>> executionOrders;

    std::string executionOrderString = "ExecutionOrder";
    unsigned int firstRow = 0;
    std::map<std::string, unsigned int> executionOrderAndIndex;

    unsigned int index = 0;

    for (mappedThreadIterator = mappedThreads.begin(); mappedThreadIterator != mappedThreads.end(); ++mappedThreadIterator)
    {
        std::string executionOrder = allFilesRowData[mappedThreadIterator->begin()->second]->GetValueByThreadId(mappedThreadIterator->begin()->first, firstRow, executionOrderString);
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


std::vector<std::set<std::pair<std::string, unsigned int>>> KernelRowDataHelper::GetMappedThreads(std::map<unsigned int, KernelRowData*> allFilesRowData)
{
    std::vector<std::set<std::pair<std::string, unsigned int>>> mappedThreads;
    std::vector<std::set<std::pair<std::string, unsigned int>>>::iterator mappedThreadsIterator;

    if (allFilesRowData.size() > 1)
    {
        KernelRowData* baseRowData = allFilesRowData.begin()->second;
        std::vector<std::string> baseRowDataThreads = baseRowData->GetUniqueThreads();

        // Iterate for all threads in base files
        for (std::vector<std::string>::iterator baseRowDataThreadsIter = baseRowDataThreads.begin();
             baseRowDataThreadsIter != baseRowDataThreads.end(); ++baseRowDataThreadsIter)
        {
            std::pair <std::string, unsigned int> baseFileCurrentThreadPair(*baseRowDataThreadsIter, 0);
            std::set<std::pair<std::string, unsigned int>> mappedThreadSet;
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

                    std::vector<std::string> currentFileUniqueThreads = rowPerFileIterator->second->GetUniqueThreads();
                    unsigned int baseFileRowDataCountByThread = baseRowData->GetRowCountByThreadId(*baseRowDataThreadsIter);

                    for (std::vector<std::string>::iterator currentFileThreadIter = currentFileUniqueThreads.begin();
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
                            std::pair<std::string, unsigned int> currentThreadPair(*currentFileThreadIter, fileIndex);
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
                std::vector<std::string> currentFileUniqueThreads = rowPerFileIterator->second->GetUniqueThreads();

                for (std::vector<std::string>::iterator currentFileThreadIter = currentFileUniqueThreads.begin();
                     currentFileThreadIter != currentFileUniqueThreads.end(); currentFileThreadIter++)
                {
                    bool isThreadfound = false;

                    for (mappedThreadsIterator = mappedThreads.begin(); mappedThreadsIterator != mappedThreads.end(); ++mappedThreadsIterator)
                    {
                        for (std::set<std::pair<std::string, unsigned int>>::iterator currentSetIter = mappedThreadsIterator->begin();
                             currentSetIter != mappedThreadsIterator->end(); ++currentSetIter)
                        {
                            isThreadfound |= (currentSetIter->first.compare(*currentFileThreadIter) == 0) &&
                                             (currentSetIter->second == fileIndex);
                        }
                    }

                    if (!isThreadfound)
                    {
                        std::set<std::pair<std::string, unsigned int>> unMappedThreadSet;
                        std::pair<std::string, unsigned int> unMappedThreadPair(*currentFileThreadIter, fileIndex);
                        unMappedThreadSet.insert(unMappedThreadPair);
                        mappedThreads.push_back(unMappedThreadSet);
                    }
                }
            }
        }
    }

    return SortMappedThreadByExecutionOrder(allFilesRowData, mappedThreads);
}