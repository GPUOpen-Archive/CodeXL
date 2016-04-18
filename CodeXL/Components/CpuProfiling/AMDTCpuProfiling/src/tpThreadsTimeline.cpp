//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpThreadsTimeline.cpp
///
//==================================================================================

//------------------------------ tpThreadsView.cpp ------------------------------

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>
#include <qcustomplot.h>


// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTApplicationComponents/Include/acNavigationChart.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimeline.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>

// Local:
#include <inc/StringConstants.h>
#include <inc/tpDisplayInfo.h>
#include <inc/tpSessionData.h>
#include <inc/tpThreadsTimelineItems.h>
#include <inc/tpTreeHandler.h>
#include <inc/tpThreadsTimeline.h>

tpThreadsTimeline::tpThreadsTimeline(QWidget* pParent, tpSessionData* pSessionData, tpSessionTreeNodeData* pTPData, acNavigationChart* pNavigationChart) :
    acTimeline(pParent), m_pSessionData(pSessionData), m_pThreadProfileData(pTPData), m_pNavigationChart(pNavigationChart)
{
    m_navigationXData.clear();
    m_navigationYData.clear();

    // Build the requested process branch:
    BuildSingleProcessBranch();

    // During the extraction of the samples, a x and y data vectors are built. Update the navigation chart with this data:
    SetNavigationChartData();
}

tpThreadsTimeline::~tpThreadsTimeline()
{

}

void tpThreadsTimeline::BuildMultipleProcessesBranches()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pSessionData != nullptr) && (m_pSessionData->ReaderHandler() != nullptr))
    {
        AMDTUInt32 processCount = 0;
        AMDTResult rc = AMDTGetProcessIds(*m_pSessionData->ReaderHandler(), &processCount, 0, nullptr);
        GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
        {
            AMDTProcessId* pProcessIds = new AMDTProcessId[processCount];
            rc = AMDTGetProcessIds(*m_pSessionData->ReaderHandler(), nullptr, processCount, pProcessIds);
            GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
            {
                // Initialize progress count and string:
                QString progressStr = QString(CP_STR_ThreadsTimelineProcessesProgress).arg(processCount);
                afProgressBarWrapper::instance().ShowProgressDialog(acQStringToGTString(progressStr), processCount);

                // Add each of the processes branches to the timeline:
                for (AMDTUInt32 i = 0; i < processCount; i++)
                {
                    AMDTProcessData processData;
                    rc = AMDTGetProcessData(*m_pSessionData->ReaderHandler(), pProcessIds[i], &processData);
                    GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
                    {
                        QString progStr = QString(CP_STR_ThreadsTimelineSingleProcessProgress).arg(processData.m_pCommand);
                        afProgressBarWrapper::instance().setProgressText(acQStringToGTString(progStr));

                        AddProcessBranch(&processData);

                        afProgressBarWrapper::instance().incrementProgressBar();
                    }
                }
            }

            afProgressBarWrapper::instance().hideProgressBar();
        }
    }
}

void tpThreadsTimeline::BuildSingleProcessBranch()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pSessionData != nullptr) && (m_pSessionData->ReaderHandler() != nullptr) && (m_pThreadProfileData != nullptr))
    {
        AMDTProcessData processData;
        AMDTResult rc = AMDTGetProcessData(*m_pSessionData->ReaderHandler(), m_pThreadProfileData->m_pid, &processData);
        GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
        {
            QString progressStr = QString(CP_STR_ThreadsTimelineSingleProcessProgress).arg(processData.m_pCommand);
            afProgressBarWrapper::instance().ShowProgressDialog(acQStringToGTString(progressStr), 0);

            // Add the branch for the launched process:
            AddProcessBranch(&processData);

            afProgressBarWrapper::instance().hideProgressBar();
        }
    }
}

void tpThreadsTimeline::AddProcessBranch(const AMDTProcessData* pProcessData)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pProcessData != nullptr)
    {
        // Create a branch for the process:
        acTimelineBranch* pBranch = new acTimelineBranch;

        // Add the branch to the grid:
        addBranch(pBranch);

        // Set the command as text:
        pBranch->setText(pProcessData->m_pCommand);

        // Add the threads related to this process:
        AddThreadsTimelineToProcess(pBranch, pProcessData);
    }
}

void tpThreadsTimeline::AddThreadsTimelineToProcess(acTimelineBranch* pProcessBranch, const AMDTProcessData* pProcessData)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((pProcessBranch != nullptr) && (pProcessData != nullptr) && (m_pSessionData != nullptr) && (m_pSessionData->ReaderHandler() != nullptr))
    {
        // Get the amount of thread for this process:
        AMDTUInt32 threadsCount = 0;
        AMDTResult rc = AMDTGetThreadIds(*m_pSessionData->ReaderHandler(), pProcessData->m_pid, &threadsCount, 0, nullptr);
        GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
        {
            AMDTThreadId* pThreadIds = new AMDTThreadId[threadsCount];
            rc = AMDTGetThreadIds(*m_pSessionData->ReaderHandler(), pProcessData->m_pid, nullptr, threadsCount, pThreadIds);
            GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
            {
                for (AMDTUInt32 i = 0; i < threadsCount; i++)
                {
                    // Create the thread branch:
                    tpThreadsTimelineBranch* pThreadBranch = new tpThreadsTimelineBranch(pThreadIds[i]);

                    pThreadBranch->setText(QString(CP_STR_ThreadsTimelineThreadSubBranch).arg(pThreadIds[i]));

                    pProcessBranch->addSubBranch(pThreadBranch);

                    // Add the cores sub branches for this thread:
                    AddThreadCoresSubBranches(pThreadBranch, pProcessData->m_pid, pThreadIds[i]);

                    // Add the samples of this thread to the thread timeline, and the cores branches:
                    AddThreadTimeSamples(pThreadBranch, pThreadIds[i]);
                }
            }
        }
    }
}

void tpThreadsTimeline::AddSampleToCore(const AMDTThreadSample& sample)
{
    // Sanity check:

    // First look for the core branch (in the map that is built before):
    ThreadCoreID id;
    id.m_coreId = sample.m_coreId;
    id.m_tid = sample.m_threadId;
    id.m_pid = sample.m_processId;
    acTimelineBranch* pCoreBranch = m_coreBranchesMap[id];

    // Sanity check:
    GT_IF_WITH_ASSERT(pCoreBranch != nullptr)
    {
        // Create a timeline item for this sample:
        tpThreadsTimelineItem* pNewTimelineItem = CreateTimelineItem(sample);

        // Sanity check:
        GT_IF_WITH_ASSERT(pNewTimelineItem != NULL)
        {
            // Add the timeline item to the core branch:
            pCoreBranch->addTimelineItem(pNewTimelineItem);
        }
    }
}
void tpThreadsTimeline::AddThreadTimeSamples(acTimelineBranch* pThreadBranch, AMDTThreadId threadId)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pSessionData != nullptr) && (m_pSessionData->ReaderHandler() != nullptr))
    {
        AMDTThreadData threadData;
        AMDTResult rc = AMDTGetThreadData(*m_pSessionData->ReaderHandler(), threadId, &threadData);
        GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
        {
            AMDTUInt32 samplesCount = 0;
            rc = AMDTGetThreadSampleData(*m_pSessionData->ReaderHandler(), threadId, &samplesCount, nullptr);
            GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
            {
                if (samplesCount > 0)
                {
                    // Initialize the samples count for this thread in the progress bar:
                    QString progressText = QString(CP_STR_ThreadsTimelineThreadProgress).arg(threadId);
                    afProgressBarWrapper::instance().setProgressDetails(acQStringToGTString(progressText), samplesCount);

                    AMDTThreadSample* pSamples = new AMDTThreadSample[samplesCount];
                    rc = AMDTGetThreadSampleData(*m_pSessionData->ReaderHandler(), threadId, nullptr, &pSamples);
                    GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
                    {
                        for (AMDTUInt32 i = 0; i < samplesCount; i++)
                        {
                            AMDTThreadSample sample = pSamples[i];

                            // Create a timeline item for this sample:
                            tpThreadsTimelineItem* pThreadSampleTime = CreateTimelineItem(sample);

                            GT_IF_WITH_ASSERT(pThreadSampleTime != nullptr)
                            {
                                // Add the timeline item:
                                pThreadBranch->addTimelineItem(pThreadSampleTime);
                            }

                            // Add this sample to the branch related to the sample core:
                            AddSampleToCore(sample);

                            // Increment the progress bar:
                            afProgressBarWrapper::instance().incrementProgressBar();
                        }
                    }

                    afProgressBarWrapper::instance().hideProgressBar();
                }
            }
        }
    }
}

void tpThreadsTimeline::AddThreadCoresSubBranches(acTimelineBranch* pThreadBranch, AMDTProcessId pid, AMDTThreadId tid)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((pThreadBranch != nullptr) && (m_pSessionData != nullptr) && (m_pSessionData->ReaderHandler() != nullptr))
    {
        AMDTUInt32 coresCount = 0;
        AMDTResult rc = AMDTGetNumOfProcessors(*m_pSessionData->ReaderHandler(), &coresCount);
        GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
        {
            for (AMDTUInt32 coreIndex = 0; coreIndex < coresCount; coreIndex++)
            {
                QString coreBranchText = QString(CP_STR_ThreadsTimelineCoreSubBranch).arg(coreIndex);

                // Create the core sub branch and set its properties:
                acTimelineBranch* pCoreBranch = new acTimelineBranch;

                pCoreBranch->setFolded(true);
                pCoreBranch->setText(coreBranchText);

                // Add the sub-branch to the thread branch:
                pThreadBranch->addSubBranch(pCoreBranch);

                // Map this branch for later use.
                // When the samples will be added, we will need this map to quickly access the branch of a thread running on specific core:
                ThreadCoreID id;
                id.m_coreId = coreIndex;
                id.m_pid = pid;
                id.m_tid = tid;
                m_coreBranchesMap[id] = pCoreBranch;
            }
        }
    }
}

void tpThreadsTimeline::DisplayTopThreads(int amountOfThreads)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSessionData != NULL)
    {
        // Get top exec time threads vector:
        QVector<AMDTThreadId> topThreadsVector;
        m_pSessionData->GetTopExecTimeThreads(topThreadsVector, amountOfThreads);
        int numBranches = m_subBranches.size();

        for (int i = 0; i < numBranches; i++)
        {
            acTimelineBranch* pBranch = m_subBranches[i];

            for (int j = 0; j < pBranch->subBranchCount(); j++)
            {
                tpThreadsTimelineBranch* pThreadBranch = qobject_cast<tpThreadsTimelineBranch*>(pBranch->getSubBranch(j));
                GT_IF_WITH_ASSERT(pThreadBranch != nullptr)
                {
                    AMDTUInt32 threadID = pThreadBranch->ThreadID();
                    bool isVisible = topThreadsVector.contains(threadID);

                    if (isVisible)
                    {
                        pBranch->getSubBranch(j)->setText(QString("Thread %1").arg(threadID));
                    }
                    else
                    {
                        pBranch->getSubBranch(j)->setText(QString("Thread %1 (Hidden)").arg(threadID));
                    }

                    // pBranch->getSubBranch(j)->SetVisibility(isVisible);
                }
            }
        }
    }

    update();
}

void tpThreadsTimeline::OnSettingsChanged(const tpControlPanalData& cpData)
{

    if (cpData.m_displayTopThreadsNum != 0)
    {
        // Display the requested amount of top execution time threads:
        DisplayTopThreads(cpData.m_displayTopThreadsNum);
    }
    else
    {
        // Show all branches:
        for (int i = 0; i < m_subBranches.size(); i++)
        {
            acTimelineBranch* pBranch = m_subBranches[i];

            for (int j = 0; j < pBranch->subBranchCount(); j++)
            {
#pragma message ("TODO: TP : implement")
                // pBranch->getSubBranch(j)->SetVisibility(true);

                QString text = pBranch->getSubBranch(j)->text();
                text.remove(" (Hidden)");
                pBranch->getSubBranch(j)->setText(text);
            }
        }

        update();
    }
}

tpThreadsTimelineItem* tpThreadsTimeline::CreateTimelineItem(const AMDTThreadSample& sample)
{
    // Create a timeline item for this sample:
    tpThreadsTimelineItem* pRetVal = new tpThreadsTimelineItem(sample.m_startTS, sample.m_endTS);

    // Find the right display settings for this sample:
    QColor sampleColor;
    QString stateStr, waitReasonStr;
    tpDisplayInfo::Instance().GetThreadStateDisplayData(sample.m_threadState, sampleColor, stateStr);

    if (sample.m_threadState != AMDT_THREAD_STATE_RUNNING)
    {
        // pRetVal->SetItemShape(acTimelineItem::AC_TIMELINE_DOT);
    }

    // Get the wait reason for a waiting thread:
    if (sample.m_threadState == AMDT_THREAD_STATE_WAITING)
    {
        waitReasonStr = tpDisplayInfo::Instance().WaitReasonAsString(sample.m_waitReason);
    }

    pRetVal->SetSampleData(sample.m_threadId, sample.m_coreId, stateStr, waitReasonStr);

    if (sample.m_nbrStackFrames > 0)
    {
        QStringList callstack;

        for (unsigned int i = 0; i < sample.m_nbrStackFrames; i++)
        {
            char* pFuncName = nullptr;
            AMDTGetFunctionName(*m_pSessionData->ReaderHandler(), sample.m_processId, sample.m_pStackFrames[i], &pFuncName);
            callstack << QString(pFuncName);
        }

        pRetVal->SetSampleCallstack(callstack);
    }

    pRetVal->setBackgroundColor(sampleColor);
    pRetVal->setForegroundColor(Qt::black);
    pRetVal->setText(stateStr);

    return pRetVal;
}

void tpThreadsTimeline::SetNavigationChartData()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pSessionData != nullptr) && (m_pSessionData->SessionTreeData() != nullptr))
    {
        AMDTProcessData procData;
        AMDTResult rc = AMDTGetProcessData(*m_pSessionData->ReaderHandler(), m_pSessionData->SessionTreeData()->m_pid, &procData);
        GT_IF_WITH_ASSERT(rc == AMDT_STATUS_OK)
        {
            QCPRange returnRange(0, 0);

            FILETIME ft;
            LARGE_INTEGER ts;
            SYSTEMTIME st;

            ts.QuadPart = procData.m_processCreateTS;
            ft.dwHighDateTime = ts.HighPart;
            ft.dwLowDateTime = ts.LowPart;
            AMDTUInt64 startNanosec = (ts.QuadPart % 10000000) * 100;
            FileTimeToSystemTime(&ft, &st);
            startNanosec = (ts.QuadPart % 10000000) * 100;

            ts.QuadPart = procData.m_processTerminateTS;
            ft.dwHighDateTime = ts.HighPart;
            ft.dwLowDateTime = ts.LowPart;
            AMDTUInt64 endNanosec = (ts.QuadPart % 10000000) * 100;
            FileTimeToSystemTime(&ft, &st);
            endNanosec = (ts.QuadPart % 10000000) * 100;

            GT_IF_WITH_ASSERT(endNanosec > startNanosec)
            {
                // Now build the vectors with data:
                for (AMDTUInt64 i = startNanosec; i < endNanosec; i += 100)
                {
                    m_navigationXData << i;
                    m_navigationYData << 1;
                }
            }

            // Set the vectors on the navigation chart:
            m_pNavigationChart->SetOfflineData(m_navigationXData, m_navigationYData);
        }
    }
}
