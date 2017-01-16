//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpSessionData.cpp
///
//==================================================================================

//-------------------------- - tpSessionData.cpp------------------------------

#include <qtIgnoreCompilerWarnings.h>

// Backend:
#include <AMDTThreadProfileAPI/inc/AMDTThreadProfileApi.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Infra:
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include <inc/tpSessionData.h>
#include <inc/StringConstants.h>
#include <inc/tpAppController.h>

tpSessionData::tpSessionData(const osFilePath& filePath, tpSessionTreeNodeData* pSessionTreeData) : m_pSessionTreeData(pSessionTreeData)
{
    m_totalThreadsCount = 0;
    m_totalProcessesCount = 0;
    m_totalCoresCount = 0;

    // Initialize the data:
#pragma message ("TODO: TP : Handle init failure")
    Init(filePath);
}

tpSessionData::~tpSessionData()
{

}

QString tpSessionData::GetThreadNameById(AMDTThreadId threadId)
{
    QString retVal;

    GT_IF_WITH_ASSERT(m_pReaderHandle != nullptr)
    {
        AMDTThreadData threadData;
        // get thread data and get its name
        AMDTResult rc = AMDTGetThreadData(*m_pReaderHandle, threadId, &threadData);
        GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
        {
            if (threadData.m_pThreadName != nullptr)
            {
                retVal = threadData.m_pThreadName;
            }
        }
    }

    return retVal;
}

QString tpSessionData::GetProcessNameById(AMDTProcessId procId)
{
    QString retVal;

    GT_IF_WITH_ASSERT(m_pReaderHandle != nullptr)
    {
        AMDTProcessData procData;
        // get process data and get its name
        AMDTResult rc = AMDTGetProcessData(*m_pReaderHandle, procId, &procData);
        GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
        {
            if (procData.m_pCommand != nullptr)
            {
                retVal = procData.m_pCommand;
            }
        }
    }

    return retVal;
}

void tpSessionData::SetProcessesAndThreadsMap()
{
    AMDTUInt32 processCount = 0;

    GT_IF_WITH_ASSERT(m_pReaderHandle != nullptr)
    {
        // Get processes ids count:
        AMDTResult rc = AMDTGetProcessIds(*m_pReaderHandle, &processCount, 0, nullptr);
        GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
        {
            if (processCount > 0)
            {
                // Get the list of process Ids:
                AMDTProcessId* pProcessIds = new AMDTProcessId[processCount];
                rc = AMDTGetProcessIds(*m_pReaderHandle, nullptr, processCount, pProcessIds);
                GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
                {
                    // for every process - get its threads
                    for (AMDTUInt32 i = 0; i < processCount; i++)
                    {
                        AMDTProcessData processData;
                        QVector<AMDTThreadId> threadsList;
                        // get threads list
                        rc = AMDTGetProcessData(*m_pReaderHandle, pProcessIds[i], &processData);
                        GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
                        {
                            for (AMDTUInt32 j = 0; j < processData.m_nbrThreads; j++)
                            {
                                AMDTThreadId threadId = processData.m_pThreads[j];
                                threadsList.append(threadId);
                                m_totalThreadsCount++;
                            }
                        }

                        // update processes threads map
                        m_procsAndThreadsMap[pProcessIds[i]] = threadsList;
                        m_totalProcessesCount++;
                    }
                }

                delete [] pProcessIds;
            }
        }
    }
}

bool tpSessionData::GetThreadData(AMDTThreadId threadId, AMDTProcessId& processId, QString& processName, AMDTUInt64& threadExecTime)
{
    bool retVal = false;
    AMDTThreadData threadData;

    // get thread data
    bool rc = AMDTGetThreadData(*m_pReaderHandle, threadId, &threadData);
    GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
    {
        // get process name and insert to table
        AMDTProcessData procData;
        rc = AMDTGetProcessData(*m_pReaderHandle, threadData.m_processId, &procData);
        GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
        {
            processName = QString(procData.m_pCommand);
        }

        // insert process id to table
        processId = threadData.m_processId;
        threadExecTime = threadData.m_totalExeTime;

        retVal = true;
    }

    return retVal;
}

void tpSessionData::GetTopExecTimeThreads(QVector<AMDTThreadId>& topThreadsVector, const int amountOfThreads)
{
    int threadsCount = 0;

    // Go reverse on the map, and take the amountOfThreads last items:
    QMapIterator<AMDTUInt64, AMDTThreadId> iter(m_exeTimeToThreadIDMap);
    iter.toBack();

    while (iter.hasPrevious() && (threadsCount < amountOfThreads))
    {
        iter.previous();
        topThreadsVector << iter.value();
        threadsCount++;
    }
}

void tpSessionData::ReplaceMinValueInMAp(QMap<AMDTThreadId, AMDTUInt64>& map, const AMDTUInt64 minVal, const AMDTThreadId newId, const AMDTUInt64 newVal)
{
    QMap<AMDTThreadId, AMDTUInt64>::iterator it = map.begin();

    for (; it != map.end(); it++)
    {
        if (it.value() == minVal)
        {
            // remove old min val and add the new item
            map.remove(it.key());
            map[newId] = newVal;
            break;
        }
    }
}

bool tpSessionData::GetMapMinVal(const QMap<AMDTThreadId, AMDTUInt64>& map, AMDTUInt64& newMinVal)
{
    QMap<AMDTThreadId, AMDTUInt64>::const_iterator it = map.constBegin();

    newMinVal = 0;

    // go over all map and get the min value
    for (; it != map.constEnd(); it++)
    {
        newMinVal = newMinVal < it.value() ? newMinVal : it.value();
    }

    return !map.isEmpty();
}


void tpSessionData::Init(const osFilePath& filePath)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSessionTreeData != nullptr)
    {
        m_pReaderHandle = new AMDTThreadProfileDataHandle;

        // Open the thread profile file:
        QString filePathStr = acGTStringToQString(filePath.asString());
        AMDTResult rc = AMDTOpenThreadProfile(filePathStr.toStdString().c_str(), m_pReaderHandle);

        GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK && (m_pReaderHandle != nullptr))
        {
            if (m_pSessionTreeData->m_pid == 0)
            {
                // If the pid is not initialized, it means that the session is opened not after the profile stop.
                // In this case, we should get the run info data, and extract the PID from it:
                tpAppController::Instance().ReadRunInfo(acGTStringToQString(filePath.asString()), m_pSessionTreeData);
            }

            // Set the filter processes:
            AMDTSetFilterProcesses(*m_pReaderHandle, 1, &m_pSessionTreeData->m_pid);

            rc = AMDTProcessThreadProfileData(*m_pReaderHandle);
            GT_ASSERT(rc == AMDT_STATUS_OK);

            // Get the numbers of cores:
            rc = AMDTGetNumOfProcessors(*m_pReaderHandle, &m_totalCoresCount);
            GT_ASSERT(rc == AMDT_STATUS_OK);

            for (int coreIndex = 0; coreIndex < m_totalProcessesCount; coreIndex++)
            {
                m_coresList << QString(CP_STR_ThreadsTimelineCoreSubBranch).arg(coreIndex);
            }
        }

        // Build the processes and threads map:
        SetProcessesAndThreadsMap();
    }
}

void tpSessionData::CloseThreadProfile()
{
    if (m_pReaderHandle != nullptr)
    {
        // Close the thread profile handle:
        AMDTCloseThreadProfile(*m_pReaderHandle);
    }
}

void tpSessionData::AnalyzeThreadsData()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pReaderHandle != nullptr) && (m_pSessionTreeData != nullptr))
    {
        // Get the amount of thread for this process:
        AMDTUInt32 threadsCount = 0;
        AMDTResult rc = AMDTGetThreadIds(*m_pReaderHandle, m_pSessionTreeData->m_pid, &threadsCount, 0, nullptr);
        GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
        {
            AMDTThreadId* pThreadIds = new AMDTThreadId[threadsCount];
            rc = AMDTGetThreadIds(*m_pReaderHandle, m_pSessionTreeData->m_pid, nullptr, threadsCount, pThreadIds);
            GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
            {
                for (AMDTUInt32 i = 0; i < threadsCount; i++)
                {
                    AMDTThreadData threadData;
                    rc = AMDTGetThreadData(*m_pReaderHandle, pThreadIds[i], &threadData);
                    GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
                    {
                        m_threadIDToExeTimeMap[threadData.m_threadId] = threadData.m_totalExeTime;
                        m_exeTimeToThreadIDMap[threadData.m_totalExeTime] = threadData.m_threadId;
                    }
                }
            }
        }
    }
}
