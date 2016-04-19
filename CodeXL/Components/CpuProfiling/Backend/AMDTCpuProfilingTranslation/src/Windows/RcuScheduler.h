//==================================================================================
// Copyright (c) 2014-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file RcuScheduler.h
///
//==================================================================================

#ifndef _RCUSCHEDULER_H_
#define _RCUSCHEDULER_H_

#include <AMDTOSWrappers/Include/osThread.h>
#include "RcuHandler.h"

///
/// Generic implementation of a derivative of the Read-Copy-Update Design Pattern
/// See http://en.wikipedia.org/wiki/Read-copy-update
///
class RcuScheduler
{
private:
    class WorkerThread : public osThread
    {
    public:
        WorkerThread(RcuScheduler& scheduler, RcuHandler& handler, RcuData& updateData, int id = -1);
        virtual ~WorkerThread();
        WorkerThread& operator=(const WorkerThread&) = delete;

    protected:
        virtual int entryPoint();

    private:
        RcuScheduler& m_scheduler;
        RcuHandler& m_handler;
        RcuData* m_pUpdateData;

#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
        int m_id;
#endif

        unsigned int m_readRecordsCount;
        unsigned int m_copyRecordsCount;
        unsigned int m_updateRecordsCount;

        friend class RcuScheduler;
    };

public:
    RcuScheduler(RcuHandlerAbstractFactory& handlerFactory, unsigned int dataSize = 4096, unsigned int threadsCount = 0U);
    virtual ~RcuScheduler();

    int Start(bool block = true);
    int WaitForCompletion(unsigned int milliseconds = INFINITE);

    unsigned int GetThreadsCount() const { return m_threadsCount; }

    // Calculate how much of the size that is passed as argument is free for use as a data buffer.
    // The calculation subtracts the size of the header that is embedded in the RcuData struct.
    static unsigned int GetBufferSize(unsigned int dataSize = 4096);

protected:
    bool TryUpdateAll(WorkerThread& thread);

private:
    void WorkerThreadEntry(WorkerThread& thread);

    /// There are 3 containers: Empty Blocks, Raw Blocks, and Copied Blocks. Each container holds data blocks read
    /// from the PRD file.
    /// The 3 containers are implemented as lock-free stacks using Microsoft's SLIST container, which is written
    /// in C, and does not use Object Oriented Programming.
    /// These containers are used as follows:
    /// 1. The class constructor initializes the Empty Blocks container with a list of empty blocks.
    /// 2. The Read operation pops a block from the Empty container, fills it with data from the PRD file and
    ///    pushes it into the Raw container.
    /// 3. The Copy operation pops a block from the Raw container, performs the copy operation and pushes them
    ///    into the Copied container.
    /// 4. The Update operation pops a block from the Copied container, performs the Update operation and pushes
    ///    the block into the Empty container.
    SLIST_HEADER m_emptyDataStackHeader;
    SLIST_HEADER m_rawDataStackHeader;
    SLIST_HEADER m_copiedDataStackHeader;
    volatile gtInt32 m_readLock;
    volatile gtInt32 m_updateLock;
    gtUInt16 m_maxConcurrentBuffers;

    unsigned int m_threadsCount;
    WorkerThread* m_pThreads;
};

#endif // _RCUSCHEDULER_H_
