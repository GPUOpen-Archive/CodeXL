//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpDisplayInfo.h
///
//==================================================================================

//------------------------------ tpDisplayInfo.h ------------------------------

#ifndef __TPDISPLAYINFO_H
#define __TPDISPLAYINFO_H

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTApplicationComponents/Include/acTreeCtrl.h>

// Backend:
#include <AMDTThreadProfileDataTypes.h>


struct tpControlPanalData
{
    /// selected cores list
    QVector<QString> m_selectedCoresList;

    /// selected process and threads map
    QMap<AMDTProcessId, QVector<AMDTThreadId> > m_selectedProcessThreadsMap;

    /// number of top displayed threads. 0 if the display top checkbox is not checked
    int m_displayTopThreadsNum;
};


class tpDisplayInfo
{
public:
    // singleton access:
    static tpDisplayInfo& Instance();

    virtual ~tpDisplayInfo();

    void GetThreadStateDisplayData(AMDTThreadState state, QColor& stateColor, QString& stateString);

    QColor GetColorForCore(int coreIndex, int amountOfCores);
    void GetIconForCore(QIcon& icon, int coreIndex, int amountOfCores);

    /// Get the time stamp as a string:
    /// timestamp time stamp
    /// timestampStr the time stamp as string
    void TimeStampToString(AMDTUInt64 timestamp, QString& timestampStr);

    /// Return the wait reason for a waiting thread as a string:
    /// \param reason the wait reason enum
    /// \return a string describing the wait reason
    QString WaitReasonAsString(AMDTThreadWaitReason reason);

protected:

    // Do not allow the use of my default constructor:
    tpDisplayInfo();

    /// singleton instance
    static tpDisplayInfo* m_spMySingleInstance;
};


#endif // ____TPDISPLAYINFO_H
