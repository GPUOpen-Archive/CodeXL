#include "..\inc\CpuProfDevice.hpp"
#include "..\inc\CpuProfPrdBuffer.hpp"
#pragma warning(push)
#pragma warning(disable:4201) // nameless struct/union
#include "..\inc\CpuProfHdMsr.h"
#pragma warning(pop)

namespace CpuProf
{

#define USER_CSS_COUNT_FLAG ((UCHAR)0x80)

template <typename RecTy>
inline void InitializeSampleDataRecord(RecTy& record, UCHAR recordType, const PCORE_DATA& data, ULONG64 startTick)
{
    record.m_RecordType = recordType;
    record.m_Core = data.core;
    record.m_ProcessHandle = reinterpret_cast<ULONG64>(data.contextId.m_processId);
    record.m_ThreadHandle  = reinterpret_cast<ULONG64>(data.contextId.m_threadId);
    record.m_TickStamp = data.timeStamp - startTick;
}


ULONG PrdDataBuffer::AppendSampleData(const PCORE_DATA& data, bool extendedData, ULONG64 startTick)
{
    ULONG recordsCount = 0UL;

    switch (data.type)
    {
        case APIC:
        {
            recordsCount = 1UL;
            PRD_APIC_DATA_RECORD* pTbpRec = static_cast<PRD_APIC_DATA_RECORD*>(AcquireNextRecord());
            InitializeSampleDataRecord(*pTbpRec, PROF_REC_TIMER, data, startTick);

#ifdef _AMD64_
            pTbpRec->m_StatusBits = 0;
            // The shift amounts put the values in their correct bit positions according to spec
            pTbpRec->m_StatusBits |= ((data.pTrapFrame->EFlags & EFLAGS_RX_BIT_MASK) >> 31);                       // bit0
            pTbpRec->m_StatusBits |= ((data.pTrapFrame->EFlags & EFLAGS_VM_BIT_MASK) >> 16);                       // bit1
            pTbpRec->m_StatusBits |= static_cast<USHORT>((data.pTrapFrame->SegCs  & DESCRIPTOR_L_BIT_MASK) >> 51); // bit2
            pTbpRec->m_StatusBits |= static_cast<USHORT>((data.pTrapFrame->SegCs  & DESCRIPTOR_D_BIT_MASK) >> 51); // bit3
#else
            // doesn't apply in 32-bit mode
            pTbpRec->m_StatusBits = 0xFFFF;
#endif
            pTbpRec->m_InstructionPointer = KxGetInstructionPointerFromTrapFrame(data.pTrapFrame);
        }
        break;

        case EVENT_CTR:
        case NB_CTR:
        case L2I_CTR:
        {
            recordsCount = 1UL;
            PRD_EVENT_CTR_DATA_RECORD* pEbpRec = static_cast<PRD_EVENT_CTR_DATA_RECORD*>(AcquireNextRecord());
            InitializeSampleDataRecord(*pEbpRec, PROF_REC_EVENT, data, startTick);

            pEbpRec->m_EventCounter = data.ResourceId;
            pEbpRec->m_EventSelectHigh = static_cast<UCHAR>(data.controlValue >> 32);

            if (data.type == L2I_CTR)
            {
                pEbpRec->m_EventSelectHigh += (FAKE_L2I_EVENT_ID_PREFIX << 4);
            }

            pEbpRec->m_EventCtl = static_cast<ULONG>(data.controlValue);
            pEbpRec->m_InstructionPointer = KxGetInstructionPointerFromTrapFrame(data.pTrapFrame);
        }
        break;

        case IBS_FETCH:
        {
            recordsCount = extendedData ? 2UL : 1UL;
            PRD_IBS_FETCH_DATA_BASIC_RECORD* pFetRec = static_cast<PRD_IBS_FETCH_DATA_BASIC_RECORD*>(AcquireNextRecord(recordsCount));
            InitializeSampleDataRecord(*pFetRec, PROF_REC_IBS_FETCH_BASIC, data, startTick);

            pFetRec->m_IbsFetchCtlHigh = static_cast<ULONG>(data.value[VALUE_DATA_CONTROL] >> 32);
            pFetRec->m_IbsFetchLinAd = data.value[VALUE_FETCH_LINEAR_ADDR];

            // The physical address will only be available if it was asked for in the configuration and it was valid.
            if (extendedData)
            {
                // Change the first record type to extended.
                pFetRec->m_RecordType = PROF_REC_IBS_FETCH_EXT;
                PRD_IBS_FETCH_DATA_EXT_RECORD* pExtRec = reinterpret_cast<PRD_IBS_FETCH_DATA_EXT_RECORD*>(pFetRec + 1);
                pExtRec->m_IbsFetchPhysAd = data.value[VALUE_FETCH_PHYSICAL_ADDR];
                pExtRec->m_IbsFetchCtlExtd = static_cast<USHORT>(data.value[VALUE_FETCH_CONTROL_EXTD] & FETCH_CTL_EXTD_MASK);
            }
        }
        break;

        case IBS_OP:
        {
            recordsCount = extendedData ? 2UL : 1UL;
            PRD_IBS_OP_DATA_BASIC_RECORD* pOpRec = static_cast<PRD_IBS_OP_DATA_BASIC_RECORD*>(AcquireNextRecord(recordsCount));
            InitializeSampleDataRecord(*pOpRec, PROF_REC_IBS_OP_BASIC, data, startTick);

            pOpRec->m_IbsOpDataHigh = static_cast<USHORT>(data.value[VALUE_OP_DATA_1] >> 32);
            pOpRec->m_IbsOpDataLow = static_cast<ULONG>(data.value[VALUE_OP_DATA_1]);
            pOpRec->m_IbsOpRip = data.value[VALUE_OP_LOGIC_ADDR];

            // If any of the extended data is available.
            if (extendedData)
            {
                // Change the first record type to extended.
                pOpRec->m_RecordType = PROF_REC_IBS_OP_EXT;
                PRD_IBS_OP_DATA_EXT_RECORD* pExtRec = reinterpret_cast<PRD_IBS_OP_DATA_EXT_RECORD*>(pOpRec + 1);

                // If any of the values weren't valid, they will be zeros.
                pExtRec->m_IbsOpData2  = static_cast<UCHAR>(data.value[VALUE_OP_DATA_2] & NB_DATA_MASK);
                pExtRec->m_IbsOpData3  = data.value[VALUE_OP_DATA_3];
                pExtRec->m_IbsDcLinAd  = data.value[VALUE_OP_DC_LINEAR_ADDR];
                pExtRec->m_IbsDcPhyAd  = data.value[VALUE_OP_DC_PHYSICAL_ADDR];
                pExtRec->m_IbsBrTarget = data.value[VALUE_OP_BRANCH_ADDR];
                pExtRec->m_IbsOpData4 = static_cast<UCHAR>(data.value[VALUE_OP_DATA_4] & OP_DATA_4_MASK);
            }
        }
        break;

        default:
            ASSERT(0);
            break;
    }

    return recordsCount;
}

#if 0
ULONG PrdDataBuffer::AppendCallStack(const ULONG_PTR* pKernelCallers, ULONG kernelCallersCount,
                                     const ULONG_PTR* pUserCallers, ULONG userCallersCount,
                                     ULONG maxDepth,
                                     ULONG core)
{
    ULONG recordsCount = 0UL;
    int index = 0;

    // This calculation defines the maximum number of records that will hold the maximum depth.
    const ULONG maxIndex = ((maxDepth - 1) / CSS_DATA_PER_RECORD + 1) * sizeof(PRD_CSS_DATA_RECORD);

    // Set the first record pointer at the end of the
    PRD_CSS_DATA_RECORD* pCurCSSRec = static_cast<PRD_CSS_DATA_RECORD*>(AcquireNextRecord());

    // Initialize the first call-stack record.
    pCurCSSRec->m_RecordType = PROF_REC_CSS;
    pCurCSSRec->m_Core = static_cast<UCHAR>(core);

    const ULONG_PTR* pTotalCallers[MaximumMode] = { pKernelCallers, pUserCallers };
    ULONG totalCallersCounts[MaximumMode] = { kernelCallersCount, userCallersCount };

    for (int mode = 0; mode < MaximumMode; ++mode)
    {
        const ULONG_PTR* pCallers = pTotalCallers[mode];

        for (const ULONG_PTR* pCallersEnd = pCallers + totalCallersCounts[mode]; pCallers != pCallersEnd; ++pCallers)
        {
            pCurCSSRec->m_CallStack[index++] = static_cast<ULONG64>(*pCallers);

            // If the CSS record is full.
            if (CSS_DATA_PER_RECORD == index)
            {
                // Move to next call-stack record.
                pCurCSSRec = static_cast<PRD_CSS_DATA_RECORD*>(AcquireNextRecord());
                recordsCount++;

                if (recordsCount >= maxIndex)
                {
                    // Don't overflow the
                    break;
                }

                // Initialize the next record.
                pCurCSSRec->m_RecordType = PROF_REC_CSS;
                pCurCSSRec->m_Core = static_cast<UCHAR>(core);
                index = 0;
            }
        }
    }

    // If the last record is partially filled.
    if (0 != index)
    {
        // Fill the remaining space of the current record with zeros.
        while (index < CSS_DATA_PER_RECORD)
        {
            pCurCSSRec->m_CallStack[index] = 0x0;
            index++;
        }

        recordsCount++;
    }

    return recordsCount;
}
#endif


ULONG PrdDataBuffer::GetKernelCallStackRecordsCount(ULONG callersCount)
{
#ifdef _AMD64_
    const ULONG VALUE_SIZE = sizeof(ULONG64);
#else
    const ULONG VALUE_SIZE = sizeof(ULONG32);
#endif
    ULONG totalBytesCount = FIELD_OFFSET(PRD_KERNEL_CSS_DATA_RECORD, m_CallStack32) + (callersCount * VALUE_SIZE);
    return (totalBytesCount - 1) / PRD_RECORD_SIZE + 1;
}


ULONG PrdDataBuffer::AppendKernelCallStack(const ULONG_PTR* pCallers, ULONG count)
{
    ULONG recordsCount = GetKernelCallStackRecordsCount(count);
    PRD_KERNEL_CSS_DATA_RECORD* pCssRecord = static_cast<PRD_KERNEL_CSS_DATA_RECORD*>(AcquireNextRecord(recordsCount));

    pCssRecord->m_RecordType = PROF_REC_KERNEL_CSS;
    pCssRecord->m_Depth = static_cast<USHORT>(count);
#ifdef _AMD64_
    pCssRecord->m_Is64Bit = TRUE;
#else
    pCssRecord->m_Is64Bit = FALSE;
#endif

    RtlCopyMemory(pCssRecord->m_CallStack32, pCallers, sizeof(ULONG_PTR) * count);

    return recordsCount;
}


ULONG PrdDataBuffer::GetUserCallStackRecordsCount(ULONG callersCount, bool is64Bit)
{
#ifdef _AMD64_
    const ULONG VALUE_SIZE = is64Bit ? sizeof(ULONG64) : sizeof(ULONG32);
#else
    UNREFERENCED_PARAMETER(is64Bit);
    ASSERT(FALSE == is64Bit);
    const ULONG VALUE_SIZE = sizeof(ULONG32);
#endif
    ULONG totalBytesCount = sizeof(PRD_USER_CSS_DATA_RECORD) +
                            FIELD_OFFSET(PRD_USER_CSS_DATA_EXT_RECORD, m_CallStack32) + (callersCount * VALUE_SIZE);
    return (totalBytesCount - 1) / PRD_RECORD_SIZE + 1;
}


ULONG PrdDataBuffer::AppendUserCallStack(const ULONG_PTR* pCallers, ULONG count,
                                         BOOLEAN is64Bit,
                                         HANDLE processId, HANDLE threadId,
                                         ULONG64 startTick, ULONG64 endTick)
{
    ULONG lastUserCssRecordOffset = GetLastUserCssRecordOffset();
    ULONG offset = GetRecordsCount();

    ULONG recordsCount = GetUserCallStackRecordsCount(count, FALSE != is64Bit);
    PRD_USER_CSS_DATA_RECORD* pCssRecord = static_cast<PRD_USER_CSS_DATA_RECORD*>(AcquireNextRecord(recordsCount));

    pCssRecord->m_RecordType = PROF_REC_USER_CSS;
    pCssRecord->m_Is64Bit = is64Bit;
    pCssRecord->m_Depth = static_cast<USHORT>(count);
    pCssRecord->m_ProcessHandle = reinterpret_cast<ULONG64>(processId);
    pCssRecord->m_ThreadHandle = reinterpret_cast<ULONG64>(threadId);
    pCssRecord->m_TickStampBegin = startTick;
    pCssRecord->m_TickStampEnd = endTick;

    PRD_USER_CSS_DATA_EXT_RECORD* pCssRecordExt = reinterpret_cast<PRD_USER_CSS_DATA_EXT_RECORD*>(pCssRecord + 1);
    pCssRecordExt->m_PrevUserCssRecordOffset = static_cast<ULONG64>(lastUserCssRecordOffset);

#ifdef _AMD64_
    const ULONG VALUE_SIZE = is64Bit ? sizeof(ULONG64) : sizeof(ULONG32);
#else
    ASSERT(FALSE == is64Bit);
    const ULONG VALUE_SIZE = sizeof(ULONG32);
#endif
    RtlCopyMemory(pCssRecordExt->m_CallStack32, pCallers, VALUE_SIZE * count);

    SetLastUserCssRecordOffset(offset);
    return recordsCount;
}


ULONG PrdDataBuffer::GetVirtualStackRecordsCount(ULONG valuesCount)
{
    ULONG totalBytesCount = FIELD_OFFSET(PRD_VIRTUAL_STACK_RECORD, m_Values) + valuesCount * (sizeof(ULONG32) + sizeof(USHORT));
    return (totalBytesCount - 1) / PRD_RECORD_SIZE + 1;
}


ULONG PrdDataBuffer::AppendVirtualStack(const ULONG32* pValues, const USHORT* pOffsets, ULONG count,
                                        ULONG_PTR stackPtr, ULONG_PTR framePtr)
{
    ULONG recordsCount = GetVirtualStackRecordsCount(count);
    PRD_VIRTUAL_STACK_RECORD* pVirtualStackRecord = static_cast<PRD_VIRTUAL_STACK_RECORD*>(AcquireNextRecord(recordsCount));

    pVirtualStackRecord->m_RecordType = PROF_REC_VIRTUAL_STACK;
    pVirtualStackRecord->m_ValuesCount = static_cast<USHORT>(count);

    pVirtualStackRecord->m_FramePointerOffset = static_cast<ULONG>(framePtr - stackPtr);
    pVirtualStackRecord->m_StackPointer = static_cast<ULONG64>(stackPtr);

    RtlCopyMemory(pVirtualStackRecord->m_Values        , pValues, sizeof(ULONG32) * count);
    RtlCopyMemory(pVirtualStackRecord->m_Values + count, pOffsets, sizeof(USHORT) * count);

    return recordsCount;
}


ULONG PrdDataBuffer::AppendResourceWeights(Client& client, ULONG core)
{
    PRD_WEIGHT_RECORD* pWeightRec = static_cast<PRD_WEIGHT_RECORD*>(AcquireNextRecord());
    pWeightRec->m_RecordType = PROF_REC_WEIGHT;
    pWeightRec->m_Core = static_cast<UCHAR>(core);

    UCHAR index = 0;

    for (Client::CoreResourceWeightsIterator it = client.GetCoreResourceWeightsBegin(core),
         itEnd = client.GetCoreResourceWeightsEnd(core); it != itEnd; ++it)
    {
        pWeightRec->m_indexes[it.GetType()] = index;

        for (UCHAR* pTypeWeight = it.GetTypeBegin(), *pTypeEnd = it.GetTypeEnd(); pTypeWeight != pTypeEnd; ++pTypeWeight)
        {
            pWeightRec->m_Weights[index] = *pTypeWeight;
            index++;
        }
    }

    return 1UL;
}


ULONG PrdDataBuffer::AppendProcessId(HANDLE processId)
{
    PRD_PID_CONFIG_RECORD* pPidRec = static_cast<PRD_PID_CONFIG_RECORD*>(AcquireNextRecord());
    pPidRec->m_RecordType = PROF_REC_PIDCFG;
    pPidRec->m_PID_Array[0] = reinterpret_cast<ULONG64>(processId);

    // The buffer must be initialized to zero.
    ASSERT(0ULL == pPidRec->m_PID_Array[1]);

    return 1UL;
}


ULONG PrdDataBuffer::Initialize(Client& client, ULONG core)
{
    ASSERT(0UL == GetRecordsCount());
    return AppendResourceWeights(client, core);
}


const void* PrdDataBuffer::Finalize(ULONG& length, size_t currentRecordOffset, size_t& prevUserCssRecordOffset)
{
    ULONG lastUserCssRecordOffset = static_cast<ULONG>(m_header.m_BufferRecordCount & (~USER_CSS_COUNT_FLAG));

    if (lastUserCssRecordOffset == GetRecordsCount())
    {
        lastUserCssRecordOffset = GetLastUserCssRecordOffset();
    }
    else
    {
        m_header.m_BufferRecordCount = static_cast<UCHAR>(MAX_RECORDS_COUNT);
    }

    if (0UL != lastUserCssRecordOffset)
    {
        PRD_USER_CSS_DATA_EXT_RECORD* pCssRecord = reinterpret_cast<PRD_USER_CSS_DATA_EXT_RECORD*>(&m_buffer[lastUserCssRecordOffset + 1]);
        ULONG userCssRecordOffset = static_cast<ULONG>(pCssRecord->m_PrevUserCssRecordOffset);

        while (0UL != userCssRecordOffset)
        {
            pCssRecord->m_PrevUserCssRecordOffset += static_cast<ULONG64>(currentRecordOffset);

            pCssRecord = reinterpret_cast<PRD_USER_CSS_DATA_EXT_RECORD*>(&m_buffer[userCssRecordOffset + 1]);
            userCssRecordOffset = static_cast<ULONG>(pCssRecord->m_PrevUserCssRecordOffset);
        }

        pCssRecord->m_PrevUserCssRecordOffset = static_cast<ULONG64>(prevUserCssRecordOffset);
        prevUserCssRecordOffset = currentRecordOffset + static_cast<size_t>(lastUserCssRecordOffset);
    }

    length = GetRecordsCount() * PRD_RECORD_SIZE;
    return m_buffer;
}


ULONG PrdDataBuffer::GetLastUserCssRecordOffset() const
{
    return static_cast<ULONG>(m_buffer[GetRecordsCount()].m_reserved[0]);
}


void PrdDataBuffer::SetLastUserCssRecordOffset(ULONG offset)
{
    ULONG recordsCount = GetRecordsCount();

    if (recordsCount < MAX_RECORDS_COUNT)
    {
        m_buffer[recordsCount].m_reserved[0] = static_cast<UCHAR>(offset);
    }
    else
    {
        m_header.m_BufferRecordCount = (static_cast<UCHAR>(offset) | USER_CSS_COUNT_FLAG);
    }
}


void* PrdDataBuffer::AcquireNextRecord(ULONG count)
{
    ULONG lastUserCssRecordOffset = GetLastUserCssRecordOffset();

    void* pRecord = &m_buffer[GetRecordsCount()];
    m_header.m_BufferRecordCount += static_cast<UCHAR>(count);
    ASSERT(GetRecordsCount() <= MAX_RECORDS_COUNT);

    SetLastUserCssRecordOffset(lastUserCssRecordOffset);
    return pRecord;
}


ULONG PrdDataBuffer::GetRecordsCount() const
{
    return static_cast<ULONG>(m_header.m_BufferRecordCount);
}


bool PrdDataBuffer::HasData() const
{
    return (1UL < GetRecordsCount());
}


bool PrdDataBuffer::HasEnoughSpace(ULONG recordsCount) const
{
    return (GetRecordsCount() + recordsCount) < static_cast<ULONG>(MAX_RECORDS_COUNT);
}


bool PrdDataBuffer::IsExtendedData(const PCORE_DATA& data)
{
    bool ret;

    // If the IBS config might need the extended record type.
    if (IBS_FETCH == data.type)
    {
        ret = ((0 != data.value[VALUE_FETCH_PHYSICAL_ADDR]) ||
               (0 != data.value[VALUE_FETCH_CONTROL_EXTD]));
    }
    else if (IBS_OP == data.type)
    {
        ret = ((0 != data.value[VALUE_OP_DATA_2          ]) ||
               (0 != data.value[VALUE_OP_DATA_3          ]) ||
               (0 != data.value[VALUE_OP_DATA_4          ]) ||
               (0 != data.value[VALUE_OP_DC_LINEAR_ADDR  ]) ||
               (0 != data.value[VALUE_OP_DC_PHYSICAL_ADDR]) ||
               (0 != data.value[VALUE_OP_BRANCH_ADDR     ]));
    }
    else
    {
        ret = false;
    }

    return ret;
}


PrdDataBufferPool::~PrdDataBufferPool()
{
    Clear();
}


void PrdDataBufferPool::Clear()
{
    PrdDataBuffer* pBuffer;

    while (NULL != (pBuffer = m_emptyBuffers.Dequeue()))
    {
        delete pBuffer;
    }
}


bool PrdDataBufferPool::Reserve(ULONG count)
{
    bool ret = true;

    Queue<PrdDataBuffer> newBuffers;
    PrdDataBuffer* pBuffer;

    for (ULONG i = 0UL; i < count; ++i)
    {
        pBuffer = new PrdDataBuffer();

        if (NULL == pBuffer)
        {
            // Free the partially-allocated buffers.
            while (NULL != (pBuffer = newBuffers.Dequeue()))
            {
                delete pBuffer;
            }

            ret = false;
            break;
        }

        RtlZeroMemory(pBuffer, sizeof(PrdDataBuffer));
        newBuffers.Enqueue(*pBuffer);
    }

    while (NULL != (pBuffer = newBuffers.Dequeue()))
    {
        m_emptyBuffers.Enqueue(*pBuffer);
    }

    return ret;
}


ULONG PrdDataBufferPool::Free(ULONG count)
{
    ULONG i = 0UL;

    for (; i < count; ++i)
    {
        PrdDataBuffer* pBuffer = m_emptyBuffers.Dequeue();

        if (NULL == pBuffer)
        {
            break;
        }

        delete pBuffer;
    }

    ASSERT(i == count);
    return i;
}


PrdDataBuffer* PrdDataBufferPool::AcquireBuffer()
{
    return m_emptyBuffers.Dequeue();
}


void PrdDataBufferPool::ReleaseBuffer(PrdDataBuffer& buffer)
{
    RtlZeroMemory(&buffer, sizeof(buffer));
    m_emptyBuffers.Enqueue(buffer);
}


PrdDataBufferPool& PrdDataBufferPool::GetInstance()
{
    return Device::GetInstance()->GetPrdDataBufferPool();
}

} // namespace CpuProf
