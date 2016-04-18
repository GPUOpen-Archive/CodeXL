#include "..\inc\CpuProfDevice.hpp"
#include "..\inc\CpuProfPrdWriter.hpp"

namespace CpuProf
{

bool PrdWriter::AsyncWriteProcessId(HANDLE processId, ULONG core)
{
    // Try to grab an empty buffer from the pool.
    PrdDataBuffer* pBuffer = AcquirePrdDataBuffer();
    bool ret = (NULL != pBuffer);

    if (ret)
    {
        //
        // Fill the whole buffer with one weight record and the new pid record.
        //

        pBuffer->Initialize(GetClient(), core);
        pBuffer->AppendProcessId(processId);
        m_fullBuffers.Enqueue(*pBuffer);
    }

    return ret;
}


void PrdWriter::ClearFullDataBuffersList()
{
    if (!m_fullBuffers.IsEmpty())
    {
        PrintWarning("There was data leftover from previous run!");

        // Ensure that no data is left over from previous run.
        PrdDataBuffer* pBuffer;

        // For each buffer in the client full-buffer list: remove the buffer from the full list.
        while (NULL != (pBuffer = m_fullBuffers.Dequeue()))
        {
            // Add the buffer to the empty buffer pool.
            ReleasePrdDataBuffer(*pBuffer);
        }
    }
}


bool PrdWriter::IsAsynchronousModeActive() const
{
    return (NULL != m_pPrdReapThread);
}


bool PrdWriter::ActivateAsynchronousMode()
{
    bool ret;

    if (CreateDataBuffers())
    {
        ret = CreateReaperThread();

        if (!ret)
        {
            DeactivateAsynchronousMode();
        }
    }
    else
    {
        ret = false;
    }

    return ret;
}


void PrdWriter::DeactivateAsynchronousMode()
{
    if (NULL != m_pCoreBuffers)
    {
        // Flush partial buffers.
        for (ULONG core = 0UL, coresCount = GetCoresCount(); core < coresCount; ++core)
        {
            if (NULL != m_pCoreBuffers[core])
            {
                // If there is data in the buffer, add the buffer to the full buffer list.
                if (NULL != m_pPrdReapThread && m_pCoreBuffers[core]->HasData())
                {
                    m_fullBuffers.Enqueue(*m_pCoreBuffers[core]);
                }
                else
                {
                    ReleasePrdDataBuffer(*m_pCoreBuffers[core]);
                }

                m_pCoreBuffers[core] = NULL;
            }
        }
    }

    // Flush remaining data.
    if (NULL != m_pPrdReapThread)
    {
        if (!PsIsThreadTerminating(m_pPrdReapThread))
        {
            PrintInfo("Signalling last reap.");
            m_reapNotifier.Notify();

            // Wait for the PRD Reaper thread to finish writing all the partial buffers,
            // notice the STATE_STOPPING state, and terminate.
            KeWaitForSingleObject(m_pPrdReapThread, Executive, KernelMode, FALSE, NULL);
        }
        else
        {
            PrintInfo("PRD Reaper thread terminated before last reap.");
        }

        ObDereferenceObject(m_pPrdReapThread);
        m_pPrdReapThread = NULL;

        // Just in case the synchronization worked out so that the reaper was running when the state changed to stopping and
        // it needs to run once more.
        FlushFullDataBuffers();
    }

    ClearFullDataBuffersList();

    // Free allocated buffers, if available.
    FreePrdDataBuffers(CLIENT_BUFFER_COUNT * GetCoresCount());

    if (NULL != m_pCoreBuffers)
    {
        ExFreePoolWithTag(m_pCoreBuffers, ALLOC_POOL_TAG);
        m_pCoreBuffers = NULL;
    }
}


PrdDataBuffer* PrdWriter::GetDataBuffer(ULONG core, ULONG callersCount, ULONG stackValuesCount, bool isUser, bool is64Bit)
{
    ULONG newRecordsCount = 0UL;

    if (0UL != callersCount)
    {
        if (isUser)
        {
            newRecordsCount += PrdDataBuffer::GetUserCallStackRecordsCount(callersCount, is64Bit);
        }
        else
        {
            newRecordsCount += PrdDataBuffer::GetKernelCallStackRecordsCount(callersCount);
        }
    }

    if (0UL != stackValuesCount)
    {
        newRecordsCount += PrdDataBuffer::GetVirtualStackRecordsCount(stackValuesCount);
    }

    return GetDataBuffer(core, newRecordsCount);
}


PrdDataBuffer* PrdWriter::GetDataBuffer(ULONG core, ULONG callersCount, ULONG stackValuesCount,
                                        bool isUser, bool is64Bit, bool& weightChanged, bool extendedData)
{
    ULONG newRecordsCount = 1UL + (weightChanged ? 1UL : 0UL) + (extendedData ? 1UL : 0UL);

    if (0UL != callersCount)
    {
        if (isUser)
        {
            newRecordsCount += PrdDataBuffer::GetUserCallStackRecordsCount(callersCount, is64Bit);
        }
        else
        {
            newRecordsCount += PrdDataBuffer::GetKernelCallStackRecordsCount(callersCount);
        }
    }

    if (0UL != stackValuesCount)
    {
        newRecordsCount += PrdDataBuffer::GetVirtualStackRecordsCount(stackValuesCount);
    }

    return GetDataBuffer(core, newRecordsCount, weightChanged);
}


PrdDataBuffer* PrdWriter::GetDataBuffer(ULONG core, ULONG recordsCount)
{
    bool weightChanged;
    return GetDataBuffer(core, recordsCount, weightChanged);
}


PrdDataBuffer* PrdWriter::GetDataBuffer(ULONG core, ULONG recordsCount, bool& weightChanged)
{
    PrdDataBuffer* pBuffer = m_pCoreBuffers[core];

    // If the buffer is full before writing data.
    if (NULL != pBuffer && !pBuffer->HasEnoughSpace(recordsCount))
    {
        // Add the buffer to the full buffer list for later transcription to the data file by the reaper.
        m_fullBuffers.Enqueue(*pBuffer);

        pBuffer = NULL;
    }

    // If there is not a current buffer for the core and client,
    if (NULL == pBuffer)
    {
        // Try to grab an empty buffer from the pool.
        m_pCoreBuffers[core] = pBuffer = AcquirePrdDataBuffer();

        if (NULL != pBuffer)
        {
            // Woot, available buffer is mine now!
            pBuffer->Initialize(GetClient(), core);

            // No need to update weight anymore, as it is already set in the new buffer.
            weightChanged = false;
        }
    }

    return pBuffer;
}


ULONG PrdWriter::FlushFullDataBuffers()
{
    ULONG count = 0UL;

    if (IsOpened())
    {
        PrdDataBuffer* pBuffer;

        // For each buffer in the client full-buffers list.
        while (NULL != (pBuffer = m_fullBuffers.Dequeue()))
        {
            WriteDataBuffer(*pBuffer);

            // Add the buffer to the empty buffer pool.
            ReleasePrdDataBuffer(*pBuffer);
            count++;
        }
    }

    return count;
}


bool PrdWriter::CreateDataBuffers()
{
    bool ret = false;

    ULONG coresCount = GetCoresCount();

    m_pCoreBuffers = static_cast<PrdDataBuffer**>(ExAllocatePoolWithTag(NonPagedPool, sizeof(PrdDataBuffer*) * coresCount, ALLOC_POOL_TAG));

    if (NULL != m_pCoreBuffers)
    {
        RtlZeroMemory(m_pCoreBuffers, sizeof(PrdDataBuffer*) * coresCount);

        // Allocating buffer pools.
        // For simplicity, we're allocating the entire pool here, potentially causing latency for non-local memory accesses.
        if (AllocatePrdDataBuffers(CLIENT_BUFFER_COUNT * coresCount))
        {
            ret = true;
        }
        else
        {
            ExFreePoolWithTag(m_pCoreBuffers, ALLOC_POOL_TAG);
            m_pCoreBuffers = NULL;
        }
    }

    return ret;
}


bool PrdWriter::CreateReaperThread()
{
    bool ret = false;

    ClearFullDataBuffersList();

    OBJECT_ATTRIBUTES objAttrs;
    HANDLE hThread;

    InitializeObjectAttributes(&objAttrs, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);

    if (STATUS_SUCCESS == PsCreateSystemThread(&hThread, THREAD_ALL_ACCESS, &objAttrs, NULL, NULL, ReaperThread, this))
    {
        if (STATUS_SUCCESS == ObReferenceObjectByHandle(hThread,
                                                        THREAD_ALL_ACCESS,
                                                        NULL,
                                                        KernelMode,
                                                        reinterpret_cast<PVOID*>(&m_pPrdReapThread),
                                                        NULL))
        {
            ret = true;
        }

        ZwClose(hThread);
    }

    return ret;
}


VOID PrdWriter::ReaperThread(PVOID startContext)
{
    PrdWriter* pPrdWriter = static_cast<PrdWriter*>(startContext);
    ASSERT(NULL != pPrdWriter);

    Client& client = pPrdWriter->GetClient();

#ifdef DBG
    ULONG64 debugFirstTick = ReadTimeStampCounter();
    ULONG64 debugSecondTick;

    pPrdWriter->ClearReaperOverhead();
#endif

    // While the profile is not stopping.
    while (!client.IsStopping() && client.IsStarted())
    {
        // Wait for the reaper to be signaled or check if the SampleDataCallback has filled at least one buffer to reap.
        if (!pPrdWriter->m_fullBuffers.IsEmpty() || pPrdWriter->m_reapNotifier.Wait(10LL))
        {
#ifdef DBG
            debugFirstTick = ReadTimeStampCounter();
#endif

            pPrdWriter->FlushFullDataBuffers();

            pPrdWriter->m_reapNotifier.Clear();

#ifdef DBG
            debugSecondTick = ReadTimeStampCounter();
            pPrdWriter->AddReaperOverhead(debugSecondTick - debugFirstTick);
#endif
        }
    }

    PsTerminateSystemThread(STATUS_SUCCESS);
}

} // namespace CpuProf
