//== == == == == == == == == == =
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
//
/// \file $File: $
/// \version $Revision: $
/// \brief Thread Profile Translate
//
//=====================================================================

// Project headers
#include <tpPerfTranslate.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

#include <linux/perf_event.h>


//
// Public Member functions
//

AMDTResult tpPerfTranslate::OpenThreadProfileData(bool isPassOne)
{
    AMDTResult retVal = AMDT_ERROR_INTERNAL;

    // TODO: what if the reader is already open: DO NOTHING
    //if (nullptr != m_pCaperfReader)
    //{
    //    m_pCaperfReader = NULL;
    //}

    if (nullptr == m_pCaperfReader)
    {
        m_pCaperfReader = new CaPerfDataReader();

        gtString path = m_logFile;

        retVal = m_pCaperfReader->init(path.asASCIICharArray());

        AddGenericData();
    }

    if (isPassOne)
    {
        // testing
        m_verbose = false;
        retVal = ProcessPerfRecordsPass1();
        m_verbose = false;

        if (AMDT_STATUS_OK == retVal)
        {
            m_isPassOneCompleted = true;
        }
    }

    return retVal;
} // OpenThreadProfileData


AMDTResult tpPerfTranslate::ProcessThreadProfileData()
{
    AMDTResult retVal = AMDT_ERROR_INTERNAL;

    if (nullptr != m_pCaperfReader)
    {
        // For testing purpose
#if 0
        gtString debugLogPath(L"CodeXLTP");
        osDebugLog::instance().initialize(debugLogPath, NULL, NULL);

        m_pCaperfReader->dumpHeaderSections();
        m_pCaperfReader->dumpData();
#endif //0

        if (m_isPassOneCompleted)
        {
            retVal = ProcessPerfRecordsPass2();

            if (AMDT_STATUS_OK == retVal)
            {
                m_allCSRecordsProcessed = true;
            }
        }
    }

    return retVal;
} // ProcessThreadProfileData


AMDTResult tpPerfTranslate::CloseThreadProfileData()
{
    AMDTResult retVal = AMDT_ERROR_INTERNAL;

    if (nullptr != m_pCaperfReader)
    {
        m_pCaperfReader->deinit();

        delete m_pCaperfReader;
        m_pCaperfReader = nullptr;

        retVal = AMDT_STATUS_OK;
    }

    return retVal;
} // CloseThreadProfileData


//
// Intenal helper functions
//


AMDTResult tpPerfTranslate::AddGenericData()
{
    AMDTResult retVal = AMDT_ERROR_INTERNAL;

    if (nullptr != m_pCaperfReader)
    {
        m_numberOfProcessors  = m_pCaperfReader->getNumCpus();

        retVal = AMDT_STATUS_OK;
    }

    return retVal;
}


AMDTResult tpPerfTranslate::ProcessPerfRecordsPass1()
{
    AMDTResult retVal = AMDT_ERROR_INTERNAL;

    if (nullptr == m_pCaperfReader)
    {
        return retVal;
    }

    // int retVal = S_OK;
    struct perf_event_header hdr;
    void* pData = nullptr;
    AMDTUInt32 recIndx = 0;
    AMDTUInt32 offset = 0;

    if (m_pCaperfReader->getFirstRecord(&hdr, (const void**)(&pData), &offset) == S_OK)
    {
        do
        {
            if (0 == hdr.size || nullptr == pData)
            {
                retVal = E_FAIL;
                break;
            }

            // Check for invalide type
            if (hdr.type >= PERF_RECORD_MAX)
            {
                fprintf(stdout, "Error: Unknown PERF record type (%x)\n", hdr.type);
                continue;
            }

            // fprintf(stdout, "PERF record type (%x)\n", hdr.type);
            switch (hdr.type)
            {
                case PERF_RECORD_MMAP:
                    ProcessMmapRecord(&hdr, pData, offset, recIndx);
                    break;

                case PERF_RECORD_COMM:
                    ProcessCommRecord(&hdr, pData, offset, recIndx);
                    break;

                case PERF_RECORD_EXIT:
                    ProcessExitRecord(&hdr, pData, offset, recIndx);
                    break;

                case PERF_RECORD_FORK:
                    ProcessForkRecord(&hdr, pData, offset, recIndx);
                    break;

                // this is just for testing purpose
                case PERF_RECORD_SAMPLE:
                    ProcessSampleRecord(&hdr, pData, offset, recIndx);
                    break;

                default:
                    break;
            }

            recIndx++;
        }
        while (m_pCaperfReader->getNextRecord(&hdr, (const void**)(&pData), &offset) == S_OK);

        retVal = AMDT_STATUS_OK;
    }

    // TODO:
    m_isPassOneComplete = true;
    return retVal;
} // ProcessPerfRecordsPass1


AMDTResult tpPerfTranslate::ProcessPerfRecordsPass2()
{
    AMDTResult retVal = AMDT_ERROR_INTERNAL;

    if (nullptr == m_pCaperfReader)
    {
        return retVal;
    }

    // int retVal = S_OK;
    struct perf_event_header hdr;
    void* pData = nullptr;
    AMDTUInt32 recIndx = 0;
    AMDTUInt32 offset = 0;

    if (m_pCaperfReader->getFirstRecord(&hdr, (const void**)(&pData), &offset) == S_OK)
    {
        do
        {
            if (0 == hdr.size || nullptr == pData)
            {
                retVal = E_FAIL;
                break;
            }

            // Check for invalid type
            if (hdr.type >= PERF_RECORD_MAX)
            {
                fprintf(stdout, "Error: Unknown PERF record type (%x)\n", hdr.type);
                continue;
            }

            switch (hdr.type)
            {
#if 0

                case PERF_RECORD_COMM:
                    ProcessCommRecord(&hdr, pData, offset, recIndx);
                    break;
#endif

                case PERF_RECORD_SAMPLE:
                    ProcessSampleRecord(&hdr, pData, offset, recIndx);
                    break;
#if 0

                case PERF_RECORD_EXIT:
                    ProcessExitRecord(&hdr, pData, offset, recIndx);
                    break;
#endif

                default:
                    break;
            }

            recIndx++;
        }
        while (m_pCaperfReader->getNextRecord(&hdr, (const void**)(&pData), &offset) == S_OK);

        retVal = AMDT_STATUS_OK;
    }

    return retVal;
} // ProcessPerfRecordsPass2


AMDTResult tpPerfTranslate::ParsePerfRecordSample(void* pData, perfRecordSample* pSample)
{
    uint64_t sampleType = m_pCaperfReader->getAttrSampleType();

    if (!pData || !pSample)
    {
        return E_FAIL;
    }

    char* pCur = (char*) pData;

    if (sampleType & PERF_SAMPLE_TID)
    {
        pSample->pid = *((u32*) pCur);
        pCur += 4;
        pSample->tid = *((u32*) pCur);
        pCur += 4;
    }

    if (sampleType & PERF_SAMPLE_TIME)
    {
        pSample->time = *((unsigned long long*) pCur);
        pCur += 8;
    }

    if (sampleType & PERF_SAMPLE_ADDR)
    {
        pSample->addr = *((u64*)pCur);
        pCur += 8;
    }

    if (sampleType & PERF_SAMPLE_ID)
    {
        pSample->id = *((u64*)pCur);
        pCur += 8;
    }

    if (sampleType & PERF_SAMPLE_CPU)
    {
        pSample->cpu = *((u32*)pCur);
        pCur += 4;
        pSample->res = *((u32*)pCur);
        pCur += 4;
    }

    // FIXME should this come before CPU?
    if (sampleType & PERF_SAMPLE_STREAM_ID)
    {
        pSample->streamId = *((u64*)pCur);
        pCur += 8;
    }

    pSample->pCallchain = nullptr;

    if (sampleType & PERF_SAMPLE_CALLCHAIN)
    {
        pSample->pCallchain = (perfIpCallchain*)pCur;

        if (nullptr != pSample->pCallchain)
        {
            if ((pSample->pCallchain->nbrStackFrames > 0)
                && (pSample->pCallchain->stackFrames[0] > PERF_CONTEXT_MAX))
            {
                pCur += sizeof(AMDTUInt64) * (pSample->pCallchain->nbrStackFrames + 1);
            }
        }
    }

    // TODO: other fields
    // PERF_SAMPLE_READ, PERF_SAMPLE_RAW, PERF_SAMPLE_BRANCH_STACK, PERF_SAMPLE_REGS_USER, PERF_SAMPLE_STACK_USER
    // PERF_SAMPLE_WEIGHT, PERF_SAMPLE_DATA_SRC, PERF_SAMPLE_TRANSACTION, PERF_SAMPLE_REGS_INTR

    return S_OK;
}


AMDTResult tpPerfTranslate::ProcessMmapRecord(struct perf_event_header* pHdr,
                                              const void* pData,
                                              AMDTUInt32 offset,
                                              AMDTUInt32 index)
{
    GT_UNREFERENCED_PARAMETER(offset);
    GT_UNREFERENCED_PARAMETER(index);

    AMDTResult retVal = AMDT_STATUS_OK;

    if (nullptr != pHdr && nullptr != pData)
    {
        perfRecordMmap* pRec = (perfRecordMmap*) pData;

        char* pCur = (char*)pData;

        // Skip filename.
        int len = strlen(pRec->filename) + 1;
        int tmp = len % 8;

        len += (tmp != 0) ? (8 - tmp) : 0;
        // Skip pid + tid + addr + len + pgoff
        len += 4 + 4 + 8 + 8 + 8;

        pCur += len;
        perfRecordSample sample; // = { 0 };
        memset(&sample, 0, sizeof(sample));

        // FIXME: check is sample_id_all is set to 1
        if ((len + sizeof(perf_event_header)) < pHdr->size)
        {
            ParsePerfRecordSample(pCur, &sample);
        }

        if (pRec->filename[0] != '\0')
        {
            ThreadProfileEventImage imageRec;

            imageRec.m_processId       = pRec->pid;
            imageRec.m_imageBase       = pRec->addr;
            imageRec.m_imageSize       = pRec->len;
            imageRec.m_imageCheckSum   = 0;
            imageRec.m_timeDateStamp   = sample.time;  // TODO.. this is not correct
            imageRec.m_defaultBase     = pRec->pgoff;  // TODO
            imageRec.m_fileName.fromASCIIString(pRec->filename);

            AddImageLoadEvent(imageRec);

            if (m_verbose)
            {
                fprintf(stdout, "rec#:%5u, MMAP: hdr.size:%d, hdr.misc:%d, time:%016lu, addr:0x%016lx, cpu:%d, pid:%d, tid:%d, "
                        "len:0x%016lx, pgoff:0x%016lx, filename:%s\n",
                        index, pHdr->size, pHdr->misc, sample.time, pRec->addr, sample.cpu, pRec->pid, pRec->tid,
                        pRec->len, pRec->pgoff, pRec->filename);
            }
        }
    }

    return retVal;
}

AMDTResult tpPerfTranslate::ProcessCommRecord(struct perf_event_header* pHdr,
                                              const void* pData,
                                              AMDTUInt32 offset,
                                              AMDTUInt32 index)
{
    GT_UNREFERENCED_PARAMETER(offset);
    GT_UNREFERENCED_PARAMETER(index);

    AMDTResult retVal = AMDT_STATUS_OK;

    if (nullptr != pHdr && nullptr != pData)
    {
        perfRecordComm* pRec = (perfRecordComm*) pData;
        char* pCur = (char*)pData;

        // Skip command string
        int len = strlen(pRec->commandStr) + 1;
        int tmp = len % 8;

        len += (tmp != 0) ? (8 - tmp) : 0;
        // Skip pid + tid
        len += 4 + 4;

        pCur += len;

        perfRecordSample sample;
        memset(&sample, 0, sizeof(sample));

        if (pHdr->size > (len + sizeof(perf_event_header)))
        {
            ParsePerfRecordSample(pCur, &sample);
        }

        if (!m_isPassOneComplete)
        {
            if (m_verbose)
            {
                fprintf(stdout, "rec#:%5u, COMM: hdr.size:%d hdr.misc:%d, time:%016lu, cpu:%d, pid:%d, tid:%d, comm:%s\n",
                        index, pHdr->size, pHdr->misc, sample.time, sample.cpu, pRec->pid, pRec->tid, pRec->commandStr);
            }

            ThreadProfileEventProcess processRec;

            // TODO: how to get the commandline
            processRec.m_timeStamp   = sample.time;
            processRec.m_processorId = sample.cpu;
            processRec.m_processId   = pRec->pid;
            processRec.m_exitStatus  = 0; // STILL ALIVE
            processRec.m_imageFileName.fromASCIIString(pRec->commandStr);

            // update the process map
            AddProcessStartEvent(processRec);
        }
        else
        {
            AddCSRecord(sample);
        }
    }

    return retVal;
}

AMDTResult tpPerfTranslate::ProcessForkRecord(struct perf_event_header* pHdr,
                                              const void* pData,
                                              AMDTUInt32 offset,
                                              AMDTUInt32 index)
{
    GT_UNREFERENCED_PARAMETER(offset);
    GT_UNREFERENCED_PARAMETER(index);

    AMDTResult retVal = AMDT_STATUS_OK;

    if (nullptr != pHdr && nullptr != pData)
    {
        perfRecordFork* pRec = (perfRecordFork*) pData;
        char* pCur = (char*)pData;

        // Skip perfRecordFork
        int len = sizeof(perfRecordFork);

        pCur += len;

        perfRecordSample sample;
        memset(&sample, 0, sizeof(sample));

        if (pHdr->size > (len + sizeof(perf_event_header)))
        {
            ParsePerfRecordSample(pCur, &sample);
        }

        if (m_verbose)
        {
            fprintf(stdout, "rec#:%5u, FORK: hdr.size:%d hdr.misc:%d, time:%016lu, time:%016lu, pid:%d, ppid:%d, tid:%d, ptid:%d\n",
                    index, pHdr->size, pHdr->misc, pRec->time, sample.time,
                    pRec->pid, pRec->ppid, pRec->tid, pRec->ptid);
        }

        // If this is process
        if (pRec->pid == pRec->tid)
        {
            ThreadProfileEventProcess processRec;
            processRec.m_timeStamp   = pRec->time; // TODO: sample.rec ?
            processRec.m_processorId = sample.cpu;
            processRec.m_processId   = pRec->pid;
            processRec.m_parentId    = pRec->ppid;
            processRec.m_exitStatus  = 0;
            processRec.m_isActive    = true;

            // If the parent pid is 0, add an entry for PID - 0, since there is no
            // seperate FORK event for PID 0
            if (!m_addedPid0 && (0 == processRec.m_parentId))
            {
                processRec.m_processId = 0;
                AddProcessCreateEvent(processRec);
                m_addedPid0 = true;
            }

            // Add to process map
            processRec.m_processId   = pRec->pid;
            AddProcessCreateEvent(processRec);
        }

        // Add to thread map
        ThreadProfileEventThread threadRec;
        threadRec.m_timeStamp   = pRec->time; // TODO: sample.time?
        threadRec.m_processorId = sample.cpu;
        threadRec.m_processId   = pRec->pid;
        threadRec.m_threadId    = pRec->tid;
        threadRec.m_affinity    = 0xF; // TODO: how to get these data?

        AddThreadStartEvent(threadRec);
    }

    return retVal;
}

AMDTResult tpPerfTranslate::ProcessExitRecord(struct perf_event_header* pHdr,
                                              const void* pData,
                                              AMDTUInt32 offset,
                                              AMDTUInt32 index)
{
    GT_UNREFERENCED_PARAMETER(offset);
    GT_UNREFERENCED_PARAMETER(index);

    AMDTResult retVal = AMDT_STATUS_OK;

    if (nullptr != pHdr && nullptr != pData)
    {
        perfRecordExit* pRec = (perfRecordExit*) pData;
        char* pCur = (char*)pData;

        // Skip perfRecordExit
        int len = sizeof(perfRecordExit);

        pCur += len;

        perfRecordSample sample;
        memset(&sample, 0, sizeof(sample));

        if (pHdr->size > (len + sizeof(perf_event_header)))
        {
            ParsePerfRecordSample(pCur, &sample);
        }

        if (!m_isPassOneComplete)
        {
            // Thread EXIT event
            ThreadProfileEventThread threadRec;
            threadRec.m_timeStamp   = pRec->time; // TODO: sample.time?
            threadRec.m_processorId = sample.cpu;
            threadRec.m_processId   = pRec->pid;
            threadRec.m_threadId    = pRec->tid;

            AddThreadStopEvent(threadRec);

            // Process EXIT event
            if (pRec->pid == pRec->tid)
            {
                ThreadProfileEventProcess processRec;
                processRec.m_timeStamp   = pRec->time; // TODO: sample.rec ?
                processRec.m_processorId = sample.cpu;
                processRec.m_processId   = pRec->pid;
                processRec.m_parentId    = pRec->ppid;
                processRec.m_exitStatus  = 0;
                processRec.m_isActive    = false;

                AddProcessStopEvent(processRec);
            }

            if (m_verbose)
            {
                fprintf(stdout, "rec#:%5u, EXIT: hdr.size:%d hdr.misc:%d, time:%016lu, time:%016lu, pid:%d, ppid:%d, tid:%d, ptid:%d\n",
                        index, pHdr->size, pHdr->misc, pRec->time, sample.time,
                        pRec->pid, pRec->ppid, pRec->tid, pRec->ptid);
            }
        }
        else
        {
            // pass 2
            AddCSRecord(sample);
        }

    }

    return retVal;
}

AMDTResult tpPerfTranslate::ProcessSampleRecord(struct perf_event_header* pHdr,
                                                const void* pData,
                                                AMDTUInt32 offset,
                                                AMDTUInt32 index)
{
    GT_UNREFERENCED_PARAMETER(offset);
    GT_UNREFERENCED_PARAMETER(index);

    AMDTResult retVal = AMDT_STATUS_OK;

    if (nullptr != pHdr && nullptr != pData)
    {
        perfRecordSample sample;

        uint64_t sampleType = m_pCaperfReader->getAttrSampleType();
        char* pCur = (char*) pData;

        if (sampleType & PERF_SAMPLE_IP)
        {
            sample.ip = *((u64*) pCur);
            pCur += 8;
        }

        ParsePerfRecordSample(pCur, &sample);

        if (m_isPassOneComplete)
        {
            AddCSRecord(sample);
        }
        else
        {
            if (m_verbose)
            {
                fprintf(stdout, "rec#:%5u, IP: hdr.size:%d hdr.misc:%d, time:%016lu, id:%lu, cpu:%d, ip:0x%lx pid:%d, tid:%d\n",
                        index, pHdr->size, pHdr->misc, sample.time, sample.id, sample.cpu,
                        sample.ip, sample.pid, sample.tid);
            }
        }
    }

    return retVal;
}

AMDTResult tpPerfTranslate::AddCSRecord(perfRecordSample& sample)
{
    ThreadProfileEventCSwitch csRec;
    memset(&csRec, 0, sizeof(csRec));

    csRec.m_processorId = sample.cpu;

    if ((m_prevTimestamp[sample.cpu] != 0) && m_prevThreadId[sample.cpu] != CXL_TP_ANY_TID)
    {
        csRec.m_newThreadId         = sample.tid;
        csRec.m_oldThreadId         = m_prevThreadId[sample.cpu];
        csRec.m_timeStamp           = m_prevTimestamp[sample.cpu];
        csRec.m_oldThreadState      = AMDT_THREAD_STATE_WAITING;
        csRec.m_oldThreadWaitReason = AMDT_THREAD_WAIT_REASON_SUSPENDED; // FIXME this is not correct.. how to find?
        csRec.m_oldThreadWaitMode   = AMDT_THREAD_WAIT_MODE_USER; // how to find out the correct reason?

        AddCSwitchEvent(csRec);

        ThreadProfileEventStack stackRec;
        stackRec.m_timeStamp    = m_prevTimestamp[sample.cpu];
        stackRec.m_processorId  = sample.cpu;
        stackRec.m_stackThread  = sample.tid;
        stackRec.m_stackProcess = sample.pid;

        stackRec.m_nbrFrames    = 0;

        if ((nullptr != sample.pCallchain) && (sample.pCallchain->nbrStackFrames > 0))
        {
            int j = 0;

            for (AMDTUInt32 i = 0; i < sample.pCallchain->nbrStackFrames; i++)
            {
                AMDTUInt64 ip = sample.pCallchain->stackFrames[i];

                if (ip >= PERF_CONTEXT_MAX)
                {
                    // ignore the context markers... this is not the correct approach - FIXME
                    if (PERF_CONTEXT_KERNEL == ip)
                    {
                        // if it is kernel context, ignore the next ip - which is perf_event_task_sched_out()
                        i++;
                    }

                    continue;
                }

                stackRec.m_stacks[j] = ip;
                j++;
            }

            stackRec.m_nbrFrames = j;

            AddStackEvent(stackRec);
        }
    }

    m_prevTimestamp[sample.cpu] = sample.time;
    m_prevThreadId[sample.cpu]  = sample.tid;

    return AMDT_STATUS_OK;
}
