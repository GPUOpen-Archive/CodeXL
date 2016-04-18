#include "..\inc\CpuProfDevice.hpp"
#include "..\inc\CpuProfPrdBuffer.hpp"
#include <WinDriverUtils\Include\Dpc.hpp>
#include <WinDriverUtils\Include\Cpuid.h>

namespace CpuProf
{

static VOID Deferred_ReadTopology(PKDPC pDpc, PVOID context, PVOID synchEvent, PVOID arg2);
static ULONG GetCpuFrequency();


const Client& PrdWriter::GetClient() const
{
    return *reinterpret_cast<const Client*>(
               reinterpret_cast<const UCHAR*>(this) - reinterpret_cast<ULONG_PTR>(&static_cast<const Client*>(0)->GetPrdWriter()));
}


Client& PrdWriter::GetClient()
{
    return const_cast<Client&>(const_cast<const PrdWriter*>(this)->GetClient());
}


PrdWriter::PrdWriter() : m_pPrdReapThread(NULL), m_lastUserCssRecordOffset(0), m_recordsCount(0), m_pCoreBuffers(NULL)
{
}


PrdWriter::~PrdWriter()
{
    Close();
}


bool PrdWriter::Open(const wchar_t* pFilePath, ULONG length, ULONG64 startTime, ULONG64 timeFreq)
{
    Close();

    if (m_file.Open(pFilePath, length))
    {
        if (!WriteHeader(startTime, timeFreq))
        {
            m_file.Close();
        }
    }

    return m_file.IsOpened();
}


void PrdWriter::Close()
{
    if (IsAsynchronousModeActive())
    {
        DeactivateAsynchronousMode();
    }

    if (m_file.IsOpened())
    {
        WriteLastUserCssRecordOffset();
        m_file.Close();
    }

    m_lastUserCssRecordOffset = 0;
    m_recordsCount = 0;
}


bool PrdWriter::WriteLastUserCssRecordOffset()
{
    bool ret;

    if (0 != m_lastUserCssRecordOffset)
    {
        ULONG64 lastUserCssRecordOffset = static_cast<ULONG64>(m_lastUserCssRecordOffset);
        const ULONG64 offset = sizeof(PRD_FILE_HEADER_V3) + FIELD_OFFSET(PRD_HEADER_EXT_RECORD, m_LastUserCssRecordOffset);

        ret = m_file.Write(&lastUserCssRecordOffset, sizeof(ULONG64), offset);
    }
    else
    {
        ret = true;
    }

    return ret;
}


bool PrdWriter::WriteConfiguration(const Configuration& config, ULONG64 startTime)
{
    union
    {
        PRD_APIC_CONFIG_RECORD apic;
        PRD_IBS_CONFIG_RECORD ibs;
        PRD_EVENT_CTR_CONFIG_RECORD eventCtr;
    } configRec;

    RtlZeroMemory(&configRec, sizeof(configRec));

    LARGE_INTEGER time = KeQueryPerformanceCounter(NULL);
    const PCORE_CONFIGURATION& pcoreConfig = config.GetPcoreConfiguration();

    switch (config.GetType())
    {
        case APIC:
            configRec.apic.m_CoreMaskCount = static_cast<USHORT>(config.GetCoresMaskCount());

            if (NULL != config.GetCoresMask())
            {
                // Save the first 64 cores mask in the record.
                configRec.apic.m_CoreMask = config.GetCoresMask()[0];
            }

            configRec.apic.m_RecordType = PROF_REC_TIMERCFG;
            configRec.apic.m_TimerGranularity = pcoreConfig.count;
            configRec.apic.m_TickStamp = static_cast<ULONG64>(time.QuadPart) - startTime;
            break;

        case EVENT_CTR:
        case NB_CTR:
        case L2I_CTR:
            configRec.eventCtr.m_CoreMaskCount = static_cast<USHORT>(config.GetCoresMaskCount());

            if (NULL != config.GetCoresMask())
            {
                // Save the first 64 cores mask in the record.
                configRec.eventCtr.m_CoreMask = config.GetCoresMask()[0];
            }

            configRec.eventCtr.m_RecordType = PROF_REC_EVTCFG;
            configRec.eventCtr.m_TickStamp = static_cast<ULONG64>(time.QuadPart) - startTime;
            configRec.eventCtr.m_EventCount = TwosComplement(pcoreConfig.count);
            configRec.eventCtr.m_EventCounter = static_cast<UCHAR>(config.GetResourceId());
            configRec.eventCtr.m_EventSelReg = pcoreConfig.msrControlValue;
            configRec.eventCtr.m_ModeFlags = 2;
            break;

        case IBS_FETCH:
            configRec.ibs.m_CoreMaskCount = static_cast<USHORT>(config.GetCoresMaskCount());

            if (NULL != config.GetCoresMask())
            {
                // Save the first 64 cores mask in the record.
                configRec.ibs.m_CoreMask = config.GetCoresMask()[0];
            }

            configRec.ibs.m_RecordType = PROF_REC_IBSCFG;
            configRec.ibs.m_TickStamp = static_cast<ULONG64>(time.QuadPart) - startTime;
            configRec.ibs.m_IbsFetchCtl = pcoreConfig.msrControlValue;
            break;

        case IBS_OP:
            configRec.ibs.m_CoreMaskCount = static_cast<USHORT>(config.GetCoresMaskCount());

            if (NULL != config.GetCoresMask())
            {
                // Save the first 64 cores mask in the record.
                configRec.ibs.m_CoreMask = config.GetCoresMask()[0];
            }

            configRec.ibs.m_RecordType = PROF_REC_IBSCFG;
            configRec.ibs.m_TickStamp = static_cast<ULONG64>(time.QuadPart) - startTime;
            configRec.ibs.m_IbsOpCtl = pcoreConfig.msrControlValue;
            break;
    }


    bool ret = m_file.Write(configRec);

    if (ret)
    {
        m_recordsCount++;

        //If we need more than the 1st 64-cores masked, write extended records
        if (64 < config.GetCoresMaskCount())
        {
            ULONG core = 64 + 1;

            PRD_CORE_MASK_RECORD coreMaskRec;

            while (core < config.GetCoresMaskCount())
            {
                RtlZeroMemory(&coreMaskRec, sizeof(PRD_CORE_MASK_RECORD));
                coreMaskRec.m_RecordType = PROF_REC_CORE_MASK;
                coreMaskRec.m_StartingIndex = static_cast<USHORT>(core / 64);

                for (int i = 0; i < 4; ++i)
                {
                    coreMaskRec.m_CoreMasks[i] = config.GetCoresMask()[core / 64];
                    core += 64;
                }

                if (m_file.Write(coreMaskRec))
                {
                    m_recordsCount++;
                }
                else
                {
                    ret = false;
                    break;
                }
            }
        }
    }

    return ret;
}


bool PrdWriter::WritePidsList(const HANDLE* pPidList, ULONG count)
{
    ASSERT(0UL == count || NULL != pPidList);

    bool ret = true;

    if (0UL != count)
    {
        PRD_PID_CONFIG_RECORD pidConfigRecord;
        RtlZeroMemory(&pidConfigRecord, sizeof(pidConfigRecord));
        pidConfigRecord.m_RecordType = PROF_REC_PIDCFG;

        ULONG recordsCount = count / PID_DATA_PER_RECORD;

        // For each pid in the given list.
        for (ULONG rec = 0UL; rec < recordsCount; ++rec)
        {
            for (ULONG i = 0UL; i < PID_DATA_PER_RECORD; ++i)
            {
                pidConfigRecord.m_PID_Array[i] = reinterpret_cast<ULONG64>(*pPidList);
                pPidList++;
            }

            if (m_file.Write(pidConfigRecord))
            {
                m_recordsCount++;
            }
            else
            {
                ret = false;
                break;
            }
        }


        if (ret)
        {
            count %= PID_DATA_PER_RECORD;

            if (0UL != count)
            {
                RtlZeroMemory(pidConfigRecord.m_PID_Array, sizeof(pidConfigRecord.m_PID_Array));

                for (ULONG i = 0UL; i < count; ++i)
                {
                    pidConfigRecord.m_PID_Array[i] = reinterpret_cast<ULONG64>(*pPidList);
                    pPidList++;
                }

                // Write partial record.
                if (m_file.Write(pidConfigRecord))
                {
                    m_recordsCount++;
                }
                else
                {
                    ret = false;
                }
            }
        }
    }

    return ret;
}


bool PrdWriter::WriteDataBuffer(PrdDataBuffer& buffer)
{
    ULONG length;
    const void* pData = buffer.Finalize(length, m_recordsCount, m_lastUserCssRecordOffset);
    ASSERT(0UL != length);

    bool ret = m_file.Write(pData, length);

    if (ret)
    {
        m_recordsCount += buffer.GetRecordsCount();
    }

    return ret;
}


bool PrdWriter::WriteMissedData(const Configuration& config, ULONG missedCount, ULONG64 startTime, ULONG core,
                                const Configuration* pConfigIbsFeth, ULONG missedIbsFethCount)
{
    PRD_MISSED_DATA_RECORD missedDataRecord;
    RtlZeroMemory(&missedDataRecord, sizeof(PRD_MISSED_DATA_RECORD));

    LARGE_INTEGER time = KeQueryPerformanceCounter(NULL);
    missedDataRecord.m_RecordType = PROF_REC_MISSED;
    missedDataRecord.m_TickStamp = static_cast<ULONG64>(time.QuadPart) - startTime;
    missedDataRecord.m_Core = static_cast<UCHAR>(core);

    switch (config.GetType())
    {
        case APIC:
            missedDataRecord.m_MissedType = PROF_REC_TIMERCFG;
            missedDataRecord.m_MissedCount = missedCount;
            missedDataRecord.m_ConfigCtl = config.GetPcoreConfiguration().count;
            break;

        case EVENT_CTR:
        case NB_CTR:
        case L2I_CTR:
            missedDataRecord.m_MissedType = PROF_REC_EVTCFG;
            missedDataRecord.m_MissedCount = missedCount;
            missedDataRecord.m_ConfigCtl = config.GetPcoreConfiguration().msrControlValue;
            break;

        case IBS_FETCH:
            missedDataRecord.m_MissedType = PROF_REC_IBSCFG;
            missedDataRecord.m_MissedFetchCount = missedCount;
            missedDataRecord.m_IbsFetchMaxCnt = config.GetPcoreConfiguration().msrControlValue;
            break;

        case IBS_OP:
            missedDataRecord.m_MissedType = PROF_REC_IBSCFG;
            missedDataRecord.m_MissedCount = missedCount;
            missedDataRecord.m_ConfigCtl = config.GetPcoreConfiguration().msrControlValue;

            if (NULL != pConfigIbsFeth)
            {
                missedDataRecord.m_MissedFetchCount = missedIbsFethCount;
                missedDataRecord.m_IbsFetchMaxCnt = pConfigIbsFeth->GetPcoreConfiguration().msrControlValue;
            }

            break;
    }

    bool ret = m_file.Write(missedDataRecord);

    if (ret)
    {
        m_recordsCount++;
    }

    return ret;
}


bool PrdWriter::WriteHeader(ULONG64 startTime, ULONG64 timeFreq)
{
    bool ret = false;

    int aCPUInfo[NUM_CPUID_OFFSETS] = { -1 };

    ULONG coresCount = GetCoresCount();

    PRD_FILE_HEADER_V3 headerRecord;
    RtlZeroMemory(&headerRecord, sizeof(headerRecord));

    LARGE_INTEGER fileTime;
    KeQuerySystemTime(&fileTime);

    headerRecord.m_TickStamp = startTime;
    headerRecord.m_StartMark.dwLowDateTime = fileTime.LowPart;
    headerRecord.m_StartMark.dwHighDateTime = fileTime.HighPart;
    headerRecord.m_Signature = PRD_HDR_SIGNATURE;
    headerRecord.m_Version = CXL_HDR_VERSION;

#ifdef _AMD64_
    headerRecord.m_Source = V1_HDR_SRC_WIN64;
#else
    headerRecord.m_Source = V1_HDR_SRC_WIN32;
#endif

    headerRecord.m_coreCount = static_cast<USHORT>(coresCount);
    headerRecord.m_speed = GetCpuFrequency();

    __cpuid(aCPUInfo, CPUID_FnVendorIdentification);
    bool cpuidAvailable = (aCPUInfo[EAX_OFFSET] >= CPUID_FnBasicFeatures);

    if (cpuidAvailable)
    {
        __cpuid(aCPUInfo, CPUID_FnBasicFeatures);

        // The number of sockets is the total cores / the number of cores per socket.
        ULONG coresPerSocket = static_cast<unsigned int>(aCPUInfo[EBX_OFFSET]) & CPUID_FnBasicFeatures_EBX_LogicalProcessorCount;
        coresPerSocket >>= CPUID_FnBasicFeatures_EBX_LogicalProcessorCount_OFFSET;

        if (1UL >= coresPerSocket)
        {
            headerRecord.m_socketCount = static_cast<UCHAR>(coresCount);
        }
        else
        {
            headerRecord.m_socketCount = static_cast<UCHAR>(coresCount / coresPerSocket);
        }
    }

    if (m_file.Write(headerRecord))
    {
        m_recordsCount++;

        // Write extended header.
        PRD_HEADER_EXT_RECORD headerExtRecord;
        RtlZeroMemory(&headerExtRecord, sizeof(PRD_HEADER_EXT_RECORD));
        headerExtRecord.m_RecordType = PROF_REC_EXT_HEADER;
        headerExtRecord.m_HrFrequency = timeFreq;

        if (m_file.Write(headerExtRecord))
        {
            m_recordsCount++;

            // Write out CPUID records.
            if (cpuidAvailable)
            {
                ret = WriteCpuInfo();
            }
            else
            {
                ret = true;
            }
        }
    }

    return ret;
}


bool PrdWriter::WriteCpuInfo()
{
    bool ret = false;

    PRD_CPUINFO_RECORD cpuInfoRecord;
    RtlZeroMemory(&cpuInfoRecord, sizeof(cpuInfoRecord));
    cpuInfoRecord.m_RecordType = PROF_REC_CPUID;

    cpuInfoRecord.m_CpuId_function = CPUID_FnBasicFeatures;
    __cpuid(cpuInfoRecord.aRegisterInfo, cpuInfoRecord.m_CpuId_function);

    // This has to be written after the header!
    if (m_file.Write(cpuInfoRecord))
    {
        m_recordsCount++;

        int aCPUInfo[NUM_CPUID_OFFSETS] = { -1 };

        __cpuid(aCPUInfo, CPUID_FnAmdExtendedFeatures);

        bool continueWrite;

        if (0 != (aCPUInfo[ECX_OFFSET] & CPUID_FnAmdExtendedFeatures_ECX_IBS))
        {
            cpuInfoRecord.m_CpuId_function = CPUID_FnIbsIdentifiers;
            // Note that only EAX is really valid.
            RtlZeroMemory(cpuInfoRecord.aRegisterInfo, sizeof(cpuInfoRecord.aRegisterInfo));
            __cpuid(cpuInfoRecord.aRegisterInfo, cpuInfoRecord.m_CpuId_function);

            continueWrite = m_file.Write(cpuInfoRecord);

            if (continueWrite)
            {
                m_recordsCount++;
            }
        }
        else
        {
            continueWrite = true;
        }

        if (continueWrite)
        {
            cpuInfoRecord.m_CpuId_function = CPUID_FnL1CacheIdentifiers;
            RtlZeroMemory(cpuInfoRecord.aRegisterInfo, sizeof(cpuInfoRecord.aRegisterInfo));
            __cpuid(cpuInfoRecord.aRegisterInfo, cpuInfoRecord.m_CpuId_function);

            if (m_file.Write(cpuInfoRecord))
            {
                m_recordsCount++;

                cpuInfoRecord.m_CpuId_function = CPUID_FnL2L3CacheIdentifiers;
                RtlZeroMemory(cpuInfoRecord.aRegisterInfo, sizeof(cpuInfoRecord.aRegisterInfo));
                __cpuid(cpuInfoRecord.aRegisterInfo, cpuInfoRecord.m_CpuId_function);

                if (m_file.Write(cpuInfoRecord))
                {
                    m_recordsCount++;

                    cpuInfoRecord.m_CpuId_function = CPUID_FnProcessorCapabilities;
                    RtlZeroMemory(cpuInfoRecord.aRegisterInfo, sizeof(cpuInfoRecord.aRegisterInfo));
                    __cpuid(cpuInfoRecord.aRegisterInfo, cpuInfoRecord.m_CpuId_function);

                    if (m_file.Write(cpuInfoRecord))
                    {
                        m_recordsCount++;

                        cpuInfoRecord.m_CpuId_function = CPUID_FnTLB1GIdentifiers;
                        RtlZeroMemory(cpuInfoRecord.aRegisterInfo, sizeof(cpuInfoRecord.aRegisterInfo));
                        __cpuid(cpuInfoRecord.aRegisterInfo, cpuInfoRecord.m_CpuId_function);

                        if (m_file.Write(cpuInfoRecord))
                        {
                            m_recordsCount++;

                            ret = WriteCpuTopology();
                        }
                    }
                }
            }
        }
    }

    return ret;
}


struct DeferredReadTopologyData
{
    PRD_TOPOLOGY_RECORD m_topologyRecord;
    unsigned int m_largestFuncExt;
};

bool PrdWriter::WriteCpuTopology()
{
    bool ret = true;

    DeferredReadTopologyData data;
    PRD_TOPOLOGY_RECORD& topologyRecord = data.m_topologyRecord;
    RtlZeroMemory(&topologyRecord, sizeof(PRD_TOPOLOGY_RECORD));

    int aCPUInfo[NUM_CPUID_OFFSETS] = { -1 };
    __cpuid(aCPUInfo, CPUID_FnExtendedVendorIdentification);
    data.m_largestFuncExt = static_cast<unsigned int>(aCPUInfo[EAX_OFFSET]);

#ifdef DBG
    PrintInfo("Max CPUID Function 0x%08x.", data.m_largestFuncExt);
#endif

    const ULONG lastCore = GetCoresCount() - 1UL;
    KDPC dpcs[TOPOLOGY_DATA_PER_RECORD];
    EventNotifier synchEvents[TOPOLOGY_DATA_PER_RECORD];

    for (ULONG i = 0UL; i < TOPOLOGY_DATA_PER_RECORD; ++i)
    {
        KeInitializeDpc(dpcs + i, Deferred_ReadTopology, &data);
        KeSetImportanceDpc(dpcs + i, HighImportance);
    }

    ULONG index = 0UL;

    for (ULONG core = 0UL; core <= lastCore; ++core)
    {
        if (SetTargetCoreDpc(dpcs[index], core))
        {
            if (FALSE == KeInsertQueueDpc(dpcs + index, synchEvents + index, reinterpret_cast<PVOID>(index)))
            {
                PrintError("Failed to enqueue DPC!");
                ret = false;

                for (ULONG i = 0UL; i < index; ++i)
                {
                    synchEvents[i].Wait();
                }

                break;
            }
        }

        ++index;

        if (core == lastCore)
        {
            topologyRecord.m_RecordType = PROF_REC_TOPOLOGY;

            for (ULONG i = 0UL, dataCore = core - (index - 1UL); i < index; ++i, ++dataCore)
            {
                topologyRecord.m_Core[i] = static_cast<UCHAR>(dataCore);
                synchEvents[i].Wait();
            }

            if (m_file.Write(topologyRecord))
            {
                m_recordsCount++;
            }
            else
            {
                ret = false;
                break;
            }

            RtlZeroMemory(&topologyRecord, sizeof(PRD_TOPOLOGY_RECORD));
        }
        else if (index == TOPOLOGY_DATA_PER_RECORD)
        {
            topologyRecord.m_RecordType = PROF_REC_TOPOLOGY;

            for (ULONG i = 0UL, dataCore = core - (TOPOLOGY_DATA_PER_RECORD - 1UL); i < TOPOLOGY_DATA_PER_RECORD; ++i, ++dataCore)
            {
                topologyRecord.m_Core[i] = static_cast<UCHAR>(dataCore);
                synchEvents[i].Wait();
                synchEvents[i].Clear();
            }

            if (m_file.Write(topologyRecord))
            {
                m_recordsCount++;
            }
            else
            {
                ret = false;
                break;
            }

            RtlZeroMemory(&topologyRecord, sizeof(PRD_TOPOLOGY_RECORD));
            index = 0UL;
        }
    }

    return ret;
}


// Read the CPUID topology data from the hardware, has to be a DPC so it can be done on each core.
static VOID Deferred_ReadTopology(PKDPC pDpc, PVOID context, PVOID synchEvent, PVOID arg2)
{
    UNREFERENCED_PARAMETER(pDpc);

    DeferredReadTopologyData* pData = static_cast<DeferredReadTopologyData*>(context);
    ULONG index = reinterpret_cast<ULONG>(arg2);
    int aCPUInfo[NUM_CPUID_OFFSETS] = { -1 };

    __cpuid(aCPUInfo, CPUID_FnBasicFeatures);

    unsigned int localApicId = static_cast<unsigned int>(aCPUInfo[EBX_OFFSET]) & CPUID_FnBasicFeatures_EBX_LocalApicId;
    localApicId >>= CPUID_FnBasicFeatures_EBX_LocalApicId_OFFSET;

    if (CPUID_FnSizeIdentifiers <= pData->m_largestFuncExt)
    {
        __cpuid(aCPUInfo, CPUID_FnSizeIdentifiers);

        unsigned int apicIdCoreIdSize = static_cast<unsigned int>(aCPUInfo[ECX_OFFSET]) & CPUID_FnSizeIdentifiers_ECX_ApicIdCoreIdSize;
        apicIdCoreIdSize >>= CPUID_FnSizeIdentifiers_ECX_ApicIdCoreIdSize_OFFSET;

        unsigned int numbits;
        unsigned int mask = 1;

        if (apicIdCoreIdSize)
        {
            numbits = apicIdCoreIdSize;

            for (unsigned int j = apicIdCoreIdSize; 1 < j; --j)
            {
                mask = (mask << 1) + 1;
            }
        }
        else
        {
            numbits = 1;
        }

        pData->m_topologyRecord.m_Processor[index] = static_cast<UCHAR>((localApicId & ~mask) >> numbits);
    }


    if (CPUID_FnIdentifiers <= pData->m_largestFuncExt)
    {
        __cpuid(aCPUInfo, CPUID_FnIdentifiers);
        pData->m_topologyRecord.m_Node[index] = aCPUInfo[ECX_OFFSET] & CPUID_FnIdentifiers_ECX_NodeId;
    }
    else
    {
        pData->m_topologyRecord.m_Node[index] = pData->m_topologyRecord.m_Processor[index];
    }

    static_cast<EventNotifier*>(synchEvent)->Notify();
}


// Calculate the CPU frequency in MHz.
static ULONG GetCpuFrequency()
{
    static ULONG s_freqMHz = 0UL;

    if (0UL == s_freqMHz)
    {
        NTSTATUS status;
        RTL_QUERY_REGISTRY_TABLE queryTable[2];
        RtlZeroMemory(&queryTable, sizeof(queryTable));

        queryTable[0].Name = L"~MHz";
        queryTable[0].Flags = RTL_QUERY_REGISTRY_REQUIRED | RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_NOEXPAND;
        queryTable[0].EntryContext = &s_freqMHz;
        queryTable[0].DefaultType = REG_DWORD;
        status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
                                        L"\\Registry\\MACHINE\\HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                                        queryTable, NULL, NULL);
    }

    return s_freqMHz;
}

} // namespace CpuProf
