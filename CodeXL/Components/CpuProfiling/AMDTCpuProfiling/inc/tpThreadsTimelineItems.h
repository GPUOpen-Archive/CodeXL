//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpThreadsTimelineItems.h
///
//==================================================================================

//------------------------------ tpThreadsTimelineItem.h ------------------------------

#ifndef __TPTHREADSTIMELINEITEM_H
#define __TPTHREADSTIMELINEITEM_H

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTApplicationComponents/Include/Timeline/acTimeline.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/views/afBaseView.h>

// Backend:
#include <AMDTThreadProfileApi.h>

/// Is used for creation of thread branches in the session timeline. The class will contain the thread ID
class tpThreadsTimelineBranch : public acTimelineBranch
{
    Q_OBJECT
public:

    /// Construct/Initialize a new instance of the qcTimelinebranch class.
    tpThreadsTimelineBranch(AMDTUInt32 threadID) : acTimelineBranch(), m_tid(threadID) {};

    /// Destroys instance of the the acTimelineBranch class.
    virtual ~tpThreadsTimelineBranch() {};

    AMDTUInt32 ThreadID() const { return m_tid; };

private:

    /// Contain the thread id of this branch:
    AMDTUInt32 m_tid;


};

/// qcAPITimelineItem descendant for API items that have an associated trace table item -- this allows for faster navigation between timeline items and trace table items.
class tpThreadsTimelineItem : public acTimelineItem
{
public:
    /// Initializes a new instance of the APITimelineItem class
    /// \param startTime the start time for this timeline item.
    /// \param endTime the end time for this timeline item.
    tpThreadsTimelineItem(quint64 startTime, quint64 endTime);

    /// Sets the details for this timeline item:
    /// \param tid the thread id for this sample
    /// \param coreId the core id for this sample
    /// \param threadState the thread state id for this sample
    /// \param waitReasonStr a string describing the wait reason for a waiting thread
    void SetSampleData(AMDTThreadId tid, AMDTUInt32 coreId, const QString& threadStateStr, const QString& waitReasonStr);

    /// Set the sample callstack:
    void SetSampleCallstack(const QStringList& callstack) { m_callstack = callstack; };

    /// Fill in a TimelineItemToolTip instance with a set of name/value pairs that will be displayed in the tooltip for this timeline item
    /// \param tooltip acTimelineItemToolTip instance that should get populated with name/value pairs
    virtual void tooltipItems(acTimelineItemToolTip& tooltip) const;

protected:

    /// The sample start time:
    quint64 m_sampleStartTime;

    /// The sample end time:
    quint64 m_sampleEndTime;

    /// The duration of this sample:
    quint64 m_sampleDuration;

    /// The thread ID for this sample:
    AMDTThreadId m_tid;

    /// The core id for this sample:
    AMDTUInt32 m_coreId;

    /// The thread state for this sample:
    QString m_threadStateStr;

    /// The wait reason for a waiting thread:
    QString m_waitReasonStr;

    /// List of strings describing the callstack:
    QStringList m_callstack;
};
#endif // __TPTHREADSTIMELINEITEM_H
