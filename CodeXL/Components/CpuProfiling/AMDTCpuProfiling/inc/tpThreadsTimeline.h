//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpThreadsTimeline.h
///
//==================================================================================

//------------------------------ tpThreadsTimeline.h ------------------------------

#ifndef __TPTHREADSTIMELINE_H
#define __TPTHREADSTIMELINE_H

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTApplicationComponents/Include/Timeline/acTimeline.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/views/afBaseView.h>

// Backend:
#include <AMDTThreadProfileAPI/inc/AMDTThreadProfileApi.h>

// Local:
#include <inc/tpDisplayInfo.h>

class tpSessionTreeNodeData;
class tpSessionData;
class acNavigationChart;
class tpThreadsTimelineItem;

// This struct is used to map a core running on a thread:
struct ThreadCoreID
{
    AMDTProcessId m_pid;
    AMDTThreadId m_tid;
    AMDTUInt32 m_coreId;

    bool operator< (const ThreadCoreID& other) const
    {
        bool retVal = false;

        if (m_pid < other.m_pid)
        {
            retVal = true;
        }
        else if (m_tid < other.m_tid)
        {
            retVal = true;
        }

        else if (m_coreId < other.m_coreId)
        {
            retVal = true;
        }

        return retVal;
    }
};
/// -----------------------------------------------------------------------
/// This class is used for the display of threads profile sessions timeline
/// -----------------------------------------------------------------------
class tpThreadsTimeline : public acTimeline
{
    Q_OBJECT

public:
    tpThreadsTimeline(QWidget* pParent, tpSessionData* pSessionData, tpSessionTreeNodeData* pTPData, acNavigationChart* pNavigationChart);
    virtual ~tpThreadsTimeline();

protected:

    /// Build a branch for each process displayed in the session:
    void BuildMultipleProcessesBranches();

    /// Build single process branch (the launched process):
    void BuildSingleProcessBranch();

    /// Add a branch for the process described in pProcessData:
    /// \param pProcessData a struct describing the process
    void AddProcessBranch(const AMDTProcessData* pProcessData);

    /// Add a branch for each thread related to the process
    /// \param pProcessData a struct describing the process
    /// \param pProcessBranch the branch of the process
    void AddThreadsTimelineToProcess(acTimelineBranch* pProcessBranch, const AMDTProcessData* pProcessData);

    /// Add the sample in pThreadSampleData to the timeline thread:
    /// \param sample the data of the sample of the thread
    void AddSampleToCore(const AMDTThreadSample& sample);

    /// Create a timeline item for the requested thread sample:
    /// \param sample the timeline sample extracted from the backed
    /// \return an acTimelineItem created with the properties set
    tpThreadsTimelineItem* CreateTimelineItem(const AMDTThreadSample& sample);

    /// Add all the samples for the thread
    /// \param pThreadBranch the thread timeline branch
    /// \param threadId the thread id
    void AddThreadTimeSamples(acTimelineBranch* pThreadBranch, AMDTThreadId threadId);

    /// Set the data on the navigation chart
    void SetNavigationChartData();

    /// A a sub branch for each of the core on this thread branch:
    /// \param pThreadBranch the timeline branch related to this thread
    /// \param pid the process id
    /// \param tid the thread id
    void AddThreadCoresSubBranches(acTimelineBranch* pThreadBranch, AMDTProcessId pid, AMDTThreadId tid);

    /// Display the requested threads count (the top execution time threads):
    /// \param amountOfThreads the amount of threads to display
    void DisplayTopThreads(int amountOfThreads);

protected slots:

    /// Is handling the change of the display settings:
    void OnSettingsChanged(const tpControlPanalData& data);

private:

    /// Backend data handle:
    tpSessionData* m_pSessionData;

    /// Session data related to this session:
    tpSessionTreeNodeData* m_pThreadProfileData;

    /// Session navigation chart:
    acNavigationChart* m_pNavigationChart;

    /// A map containing the timeline branch for each core on thread. The map is used for a quick
    /// access while building the session timeline:
    QMap<ThreadCoreID, acTimelineBranch*> m_coreBranchesMap;

    /// Vectors containing data that will be insterted into the navigation chart:
    QVector<double> m_navigationXData;
    QVector<double> m_navigationYData;

};

#endif // __TPTHREADSTIMELINE_H
