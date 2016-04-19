//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpSessionData.h
///
//==================================================================================

//------------------------------ tpSessionData.h ------------------------------

#ifndef __TPSESSIONDATA_H
#define __TPSESSIONDATA_H

// Local:
#include <inc/tpTreeHandler.h>

class tpSessionData
{
public:

    /// constructor
    tpSessionData(const osFilePath& filePath, tpSessionTreeNodeData* pSessionTreeData);

    ~tpSessionData();

    /// returns processes and threads map
    /// \returns the map reference
    QMap<AMDTProcessId, QVector<AMDTThreadId> >& ProcessesAndThreadsMap() { return m_procsAndThreadsMap; }

    /// get thread name by id
    /// \param id id the thread id
    //// \returns the name of the thread
    QString GetThreadNameById(AMDTThreadId id);

    /// get process name by id
    /// \param id id the thread id
    //// \returns the name of the thread
    QString GetProcessNameById(AMDTProcessId id);

    /// gets the number of all processes - top level items in processes tree
    /// \returns the number of all processes
    int TotalProcessesCount() const { return m_totalProcessesCount; }

    /// gets the number of all threads - child level items in processes tree
    /// \returns the number of all threads
    int TotalThreadsCount() const { return m_totalThreadsCount; }

    /// gets the number of all cores monitored on the target machine
    /// \returns the number of cores
    AMDTUInt32 TotalCoresCount() const { return m_totalCoresCount; }

    /// returns the cores list
    const QStringList& CoresList() { return m_coresList; }

    /// returns thread related data
    /// \param (in) threadId - is the thread id
    /// \param (out) processId - is the process id of current thread
    /// \param (out) processName - is the process name of current thread
    /// \param (out) threadExecTime - is the execution time of current thread
    /// \returns true if succeeded
    bool GetThreadData(AMDTThreadId threadId, AMDTProcessId& processId,
                       QString& processName, AMDTUInt64& threadExecTime);

    /// gets the top execution time threads
    /// \param threadsMap is the map of top treads, threadId-total exec time
    /// \param amountOfThreads is the number of top threads requested
    void GetTopExecTimeThreads(QVector<AMDTThreadId>& topThreadsVector, const int amountOfThreads);

    /// returns the backend reader handler
    AMDTThreadProfileDataHandle* ReaderHandler() { return m_pReaderHandle; }

    /// closing the thread profiling by using backend API
    void CloseThreadProfile();

    /// Analyzes the data of all the running threads in the session, for future use.
    /// Notice: this function should be called after all the samples of the threads are extracted
    void AnalyzeThreadsData();

    /// Return the session tree item data;
    tpSessionTreeNodeData* SessionTreeData() const { return m_pSessionTreeData; };

private:
    /// sets processes and threads map from backend
    void SetProcessesAndThreadsMap();

    /// gets the minimum value in the map
    /// \param reference to the map
    /// \returns the minimum value in the map
    bool GetMapMinVal(const QMap<AMDTThreadId, AMDTUInt64>& map, AMDTUInt64& newMinVal);

    /// removes the minimum value item in the map and replacing it with the new item key-value
    /// \param map is a reference to the threadId-value(execTime) map
    /// \param minVal is the current min value in the map
    /// \param newId is the new id to be added to the map
    /// \param newVal is the value of to be set to the new item
    void ReplaceMinValueInMAp(QMap<AMDTThreadId, AMDTUInt64>& map, const AMDTUInt64 minVal, const AMDTThreadId newId, const AMDTUInt64 newVal);

    /// initializing the tpSessionData
    /// \param filePath is the session file path
    void Init(const osFilePath& filePath);

    /// Cores list:
    QStringList m_coresList;

    /// contain the processes count in the current session. This number is being cached for further UI operations:
    int m_totalProcessesCount;

    /// contain the threads count in the current session. This number is being cached for further UI operations:
    int m_totalThreadsCount;

    /// Total count of cores:
    AMDTUInt32 m_totalCoresCount;

    /// Backend data handle:
    AMDTThreadProfileDataHandle* m_pReaderHandle;

    // processes-threads map
    QMap<AMDTProcessId, QVector<AMDTThreadId> > m_procsAndThreadsMap;

    /// The session tree data for the monitored session:
    tpSessionTreeNodeData* m_pSessionTreeData;

    /// A map containing the threads execution times:
    QMap<AMDTThreadId, AMDTUInt64> m_threadIDToExeTimeMap;

    /// A map containing the threads execution times:
    QMap<AMDTUInt64, AMDTThreadId> m_exeTimeToThreadIDMap;
};

#endif //__TPSESSIONDATA_H