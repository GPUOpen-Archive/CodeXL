#include "..\inc\CpuProfDevice.hpp"
#pragma warning(push)
#pragma warning(disable:4201) // nameless struct/union
#include "..\inc\CpuProfHdMsr.h"
#pragma warning(pop)
#include <winerror.h>

namespace CpuProf
{

Client::Client() : m_pcoreReg(NULL),
    m_pCoreResourceWeights(NULL),
    m_pCoreMissedData(NULL),
    m_samplesCount(0UL),
    m_eventsCount(0UL),
    m_countingEventsCount(0UL),
    m_pAbortUserEvent(NULL),
    m_lastErrorCode(PROF_SUCCESS),
    m_profilerState(STATE_NOT_CONFIGURED),
    m_startTime(0ULL),
    m_pidCount(0UL),
    m_addChildrenPidsToList(true)
{
    RtlZeroMemory(m_pidList, sizeof(m_pidList));
}


Client::~Client()
{
    Clear();
}


void Client::Clear()
{
    if (m_prdWriter.IsAsynchronousModeActive())
    {
        m_prdWriter.DeactivateAsynchronousMode();
    }

    if (m_prdWriter.IsOpened())
    {
        // Write missed data as the last records of the PRD file.
        WriteMissedDataToPrdFile();
        m_prdWriter.Close();
    }

    if (!m_tiWriter.Close())
    {
        PrintError("Failed to flush the TI file!");
        SetLastErrorCode(PROF_FILE_WRITE_ERROR);
    }

    ClearConfigurationsList();

    m_samplesCount = 0UL;
    m_eventsCount = 0UL;
    m_countingEventsCount = 0UL;
    m_cssConfig.Clear();

    if (IsCssEnabled())
    {
        Device::GetInstance()->GetStackTraceDispatcher().UnregisterClient(GetId());
    }

    if (NULL != m_pCoreMissedData)
    {
        delete [] m_pCoreMissedData;
        m_pCoreMissedData = NULL;
    }

    if (NULL != m_pCoreResourceWeights)
    {
        ExFreePoolWithTag(m_pCoreResourceWeights, ALLOC_POOL_TAG);
        m_pCoreResourceWeights = NULL;
    }

    // Remove filtered PIDs.
    if (0UL != m_pidCount)
    {
        RtlZeroMemory(m_pidList, m_pidCount * sizeof(*m_pidList));
        m_pidCount = 0UL;
    }

    if (NULL != m_pAbortUserEvent)
    {
        ObDereferenceObject(m_pAbortUserEvent);
        m_pAbortUserEvent = NULL;
    }

    m_profilerState = STATE_NOT_CONFIGURED;
}


bool Client::SetOutputFile(const wchar_t* pPrdFilePath, ULONG prdLength, const wchar_t* pTiFilePath, ULONG tiLength)
{
    ASSERT(NULL != pPrdFilePath && 0UL != prdLength && NULL != pTiFilePath && 0UL != tiLength);

    bool ret = false;

    LARGE_INTEGER timeFreq;
    LARGE_INTEGER startTime = KeQueryPerformanceCounter(&timeFreq);
    m_startTime = startTime.QuadPart;

    if (m_prdWriter.Open(pPrdFilePath, prdLength, startTime.QuadPart, timeFreq.QuadPart))
    {
        if (m_tiWriter.Open(pTiFilePath, tiLength))
        {
            m_profilerState |= STATE_OUTPUT_FILE_SET;
            ret = true;
        }
        else
        {
            m_prdWriter.Close();
            SetLastErrorCode(PROF_BUFFER_NOT_ALLOCATED);
        }
    }
    else
    {
        SetLastErrorCode(PROF_FILE_WRITE_ERROR);
    }

    return ret;
}


bool Client::GetOutputFile(wchar_t* pPrdFilePath, ULONG lenPrdFilePathBuffer, wchar_t* pTiFilePath, ULONG lenTiFilePathBuffer)
{
    bool ret;

    if (m_prdWriter.IsOpened() && m_tiWriter.IsOpened())
    {
        ret = 0UL != m_prdWriter.GetFilePath(pPrdFilePath, lenPrdFilePathBuffer) &&
              0UL != m_tiWriter.GetFilePath(pTiFilePath, lenTiFilePathBuffer);
    }
    else
    {
        ret = false;
    }

    return ret;
}


ULONG Client::GetId() const
{
    return static_cast<ULONG>(this - GetClient(0UL));
}


bool Client::IsValid() const
{
    return (((PFILE_OBJECT)NULL) != m_pUserFileObject);
}


bool Client::IsRegistered(PFILE_OBJECT pUserFileObject) const
{
    return (pUserFileObject == m_pUserFileObject);
}


bool Client::Register(PFILE_OBJECT pUserFileObject)
{
    bool ret = false;

    if (NULL != pUserFileObject && m_pUserFileObject.CompareAndSwap(NULL, pUserFileObject))
    {
        if (S_OK == PcoreRegister(&m_pcoreReg, NULL))
        {
            ret = true;
        }
        else
        {
            m_pUserFileObject = NULL;
        }
    }

    return ret;
}


bool Client::Unregister()
{
    bool ret = false;

    if (NULL != m_pUserFileObject)
    {
        if (NULL == m_pcoreReg || SUCCEEDED(PcoreUnregister(m_pcoreReg)))
        {
            m_pcoreReg = NULL;
            m_pUserFileObject = NULL;

            ret = true;
            PrintInfo("Unregistered client %d.", GetId());
        }
    }

    return ret;
}


void Client::SetLastErrorCode(CPUPROF_ERROR_CODES errorCode)
{
#ifdef DBG
    const char* pErrorDesc = "";

    switch (errorCode)
    {
        case PROF_ERROR:
            pErrorDesc = "Error";
            break;

        case PROF_SUCCESS:
            pErrorDesc = "Success";
            break;

        case PROF_INVALID_ARG:
            pErrorDesc = "Invalid Argument";
            break;

        case PROF_INVALID_FILENAME_FORMAT:
            pErrorDesc = "Invalid File Name";
            break;

        case PROF_FILE_WRITE_ERROR:
            pErrorDesc = "File Writing Error";
            break;

        case PROF_INVALID_OPERATION:
            pErrorDesc = "Invalid Operation";
            break;

        case PROF_BUFFER_NOT_ALLOCATED:
            pErrorDesc = "Insufficient Memory";
            break;

        case PROF_CRITICAL_ERROR:
            pErrorDesc = "Critical Error";
            break;
    }

    PrintInfo("Set client error code: 0x%X (%s).", errorCode, pErrorDesc);
#else
    PrintInfo("Set client error code: 0x%X.", errorCode);
#endif

    m_lastErrorCode = errorCode;

    if (NULL != m_pAbortUserEvent && PROF_SUCCESS != errorCode)
    {
        KeSetEvent(m_pAbortUserEvent, EVENT_INCREMENT, FALSE);
    }
}


void Client::ClearConfigurationsList()
{
    Stack<Configuration>::Iterator it = m_configs.Flush();

    while (it.IsValid())
    {
        Configuration* pConfig = &*it;

        // Get the next entry before we delete it.
        ++it;

        delete pConfig;
    }
}


bool Client::IsSamplingSuspended() const
{
    return Device::GetInstance()->IsSamplingSuspended(GetId());
}


bool Client::UpdateMissedData(const PCORE_DATA& data)
{
    bool ret = false;

    ULONG startIndex = static_cast<ULONG>(data.core) * m_configs.GetSize();
    ULONG endIndex = startIndex + m_configs.GetSize();

    for (ULONG i = startIndex; i < endIndex; ++i)
    {
        // Find config to keep the missed count in.
        if (data.type         == m_pCoreMissedData[i].m_type       &&
            data.ResourceId   == m_pCoreMissedData[i].m_resourceId &&
            data.controlValue == m_pCoreMissedData[i].m_controlValue)
        {
            // Increment the missed sample count.
            m_pCoreMissedData[i].m_missedCount++;
            ret = true;
            break;
        }
    }

    return ret;
}


ULONG Client::AggregateMissedData(ULONG configIndex, const Configuration& config) const
{
    ULONG totalMissedCount = 0UL;

    if (NULL != m_pCoreMissedData)
    {
        ULONG configsCount = m_configs.GetSize();

        // Aggregate the missed data for each core in the core mask.
        for (ULONG core = 0UL, coresCount = GetCoresCount(); core < coresCount; ++core)
        {
            if (config.IsValidCore(core))
            {
                totalMissedCount += m_pCoreMissedData[core * configsCount + configIndex].m_missedCount;
            }
        }
    }

    return totalMissedCount;
}


bool Client::WriteMissedDataToPrdFile()
{
    ASSERT(m_prdWriter.IsOpened());

    bool ret = true;

    ULONG core = KeGetCurrentProcessorNumberEx(NULL);
    ULONG configIndex = 0UL;

    for (Stack<Configuration>::ConstIterator it = m_configs.GetBegin(), itEnd = m_configs.GetEnd(); it != itEnd; ++it)
    {
        const Configuration* pConfig = &*it;

        const Configuration* pConfigIbsFeth = NULL;
        ULONG missedIbsFethCount = 0UL;

        //
        // Since we are checking in the same order that we added the configurations to the array,
        // we do not have to worry about finding the configuration.
        //

        ULONG missedCount = AggregateMissedData(configIndex, *pConfig);

        // Need to combine IBS_OP and IBS_FETCH configurations into one Missed Data record.
        if (IBS_FETCH == pConfig->GetType())
        {
            Stack<Configuration>::ConstIterator itNext = it;
            ++itNext;

            if (itNext != itEnd && IBS_OP == itNext->GetType())
            {
                it = itNext;
                pConfigIbsFeth = pConfig;
                missedIbsFethCount = missedCount;

                configIndex++;

                pConfig = &*itNext;
                missedCount = AggregateMissedData(configIndex, *pConfig);
            }
        }

        if (!m_prdWriter.WriteMissedData(*pConfig, missedCount, m_startTime, core, pConfigIbsFeth, missedIbsFethCount))
        {
            SetLastErrorCode(PROF_FILE_WRITE_ERROR);
            ret = false;
            break;
        }

        configIndex++;
    }

    return ret;
}


bool Client::WriteConfigurationsToPrdFile()
{
    bool ret = true;

    for (Stack<Configuration>::Iterator it = m_configs.GetBegin(), itEnd = m_configs.GetEnd(); it != itEnd; ++it)
    {
        if (!m_prdWriter.WriteConfiguration(*it, m_startTime))
        {
            SetLastErrorCode(PROF_FILE_WRITE_ERROR);
            ret = false;
            break;
        }
    }

    return ret;
}


Client::CoreResourceWeightsIterator Client::GetCoreResourceWeightsBegin(ULONG core, PCORERESOURCETYPES type)
{

    return Client::CoreResourceWeightsIterator(
               &m_pCoreResourceWeights[(core * MAX_RESOURCE_TYPE + type) * Device::GetInstance()->GetMaxResourceCount()], type);
}


Client::CoreResourceWeightsIterator Client::GetCoreResourceWeightsEnd(ULONG core, PCORERESOURCETYPES type)
{
    type = static_cast<PCORERESOURCETYPES>(static_cast<int>(type) + 1);
    return Client::CoreResourceWeightsIterator(
               &m_pCoreResourceWeights[(core * MAX_RESOURCE_TYPE + type) * Device::GetInstance()->GetMaxResourceCount()], type);
}


Client::CoreResourceWeightsIterator::CoreResourceWeightsIterator(UCHAR* pWeights, PCORERESOURCETYPES type) :
    PtrIterator(pWeights),
    m_type(type)
{
}


Client::CoreResourceWeightsIterator& Client::CoreResourceWeightsIterator::operator++()
{
    m_ptr += Device::GetInstance()->GetMaxResourceCount();
    m_type = static_cast<PCORERESOURCETYPES>(static_cast<int>(m_type) + 1);
    return *this;
}


UCHAR* Client::CoreResourceWeightsIterator::GetTypeBegin()
{
    return m_ptr;
}


UCHAR* Client::CoreResourceWeightsIterator::GetTypeEnd()
{
    return m_ptr + Device::GetInstance()->GetResourceCount(m_type);
}


NTSTATUS Client::SubmitConfigurations()
{
    NTSTATUS status = STATUS_SUCCESS;

    ClearSamplingOverhead();

    ULONG coresCount = GetCoresCount();
    ULONG configsCount = m_configs.GetSize();

    // Allocate the space to track configuration weights.
    size_t length = coresCount * MAX_RESOURCE_TYPE * Device::GetInstance()->GetMaxResourceCount();
    m_pCoreResourceWeights = static_cast<UCHAR*>(ExAllocatePoolWithTag(NonPagedPool, sizeof(UCHAR) * length, ALLOC_POOL_TAG));

    if (NULL != m_pCoreResourceWeights)
    {
        RtlZeroMemory(m_pCoreResourceWeights, sizeof(UCHAR)* length);

        length = coresCount * m_configs.GetSize();
        m_pCoreMissedData = new MissedData[length];

        if (NULL != m_pCoreMissedData)
        {
            RtlZeroMemory(m_pCoreMissedData, sizeof(MissedData)* length);

            PCORE_CONFIGURATION tmpPcoreConfig;
            ULONG configIndex = 0UL;

            for (Stack<Configuration>::Iterator it = m_configs.GetBegin(), itEnd = m_configs.GetEnd(); it != itEnd; ++it)
            {
                Configuration* pConfig = &*it;

                PCORE_CONFIGURATION* pPcoreConfig = &pConfig->GetPcoreConfiguration();

                if (L2I_CTR == pConfig->GetType())
                {
                    tmpPcoreConfig = pConfig->GetPcoreConfiguration();

                    // Remove the fake bits to get the actual MSR_CTL value.
                    tmpPcoreConfig.msrControlValue &= ~FAKE_L2I_MSR_CTL_BITS;

                    pPcoreConfig = &tmpPcoreConfig;
                }

                for (ULONG core = 0UL, coresCount = GetCoresCount(); core < coresCount; ++core)
                {
                    if (pConfig->IsValidCore(core))
                    {
                        ULONG globalIndex = core * configsCount + configIndex;

                        m_pCoreMissedData[globalIndex].m_type         = static_cast<UCHAR>(pConfig->GetType());
                        m_pCoreMissedData[globalIndex].m_resourceId   = static_cast<UCHAR>(pConfig->GetResourceId());
                        m_pCoreMissedData[globalIndex].m_missedCount  = 0UL;
                        m_pCoreMissedData[globalIndex].m_controlValue = pConfig->GetPcoreConfiguration().msrControlValue;

                        HRESULT hr = PcoreAddConfiguration(m_pcoreReg, core, pConfig->GetType(), pConfig->GetResourceId(), pPcoreConfig);

                        // If there was trouble adding any of the configurations, abort the entire profile!
                        if (S_OK != hr)
                        {
                            PcoreRemoveAllConfigurations(m_pcoreReg);
                            SetLastErrorCode(PROF_ERROR);

                            if (E_ACCESSDENIED != hr)
                            {
                                status = STATUS_INSUFFICIENT_RESOURCES;
                            }
                            else
                            {
                                status = STATUS_LOCK_NOT_GRANTED;
                            }

                            break;
                        }
                    }
                }

                configIndex++;
            }
        }
        else
        {
            ExFreePoolWithTag(m_pCoreResourceWeights, ALLOC_POOL_TAG);
            m_pCoreResourceWeights = NULL;

            SetLastErrorCode(PROF_BUFFER_NOT_ALLOCATED);
            status = STATUS_NO_MEMORY;
        }
    }
    else
    {
        SetLastErrorCode(PROF_BUFFER_NOT_ALLOCATED);
        status = STATUS_NO_MEMORY;
    }

    return status;
}


void Client::ReadCountingEvents()
{
    for (Stack<Configuration>::Iterator it = m_configs.GetBegin(), itEnd = m_configs.GetEnd(); it != itEnd; ++it)
    {
        Configuration* pConfig = &*it;

        for (ULONG core = 0UL, coresCount = GetCoresCount(); core < coresCount; ++core)
        {
            if (pConfig->IsValidCore(core))
            {
                if (pConfig->IsCountingEvent())
                {
                    ULONG64 saveCount = 0ULL;

                    // Save the count, like if we are pausing.
                    PcoreReadCount(m_pcoreReg, core, pConfig->GetType(), pConfig->GetResourceId(), &pConfig->GetPcoreConfiguration(), &saveCount);

                    // A counting event should only be one per config per core, so this count should not get overwritten.
                    if (0ULL != saveCount)
                    {
                        pConfig->SetEventCount(saveCount);
                    }
                }
            }
        }
    }
}


NTSTATUS Client::StartProfiling(HANDLE hAbortUserEvent, KPROCESSOR_MODE accessMode)
{
    ASSERT(IsValid());

    NTSTATUS status = STATUS_SUCCESS;
    SetLastErrorCode(PROF_SUCCESS);

    // Can not be started when already tracing/paused.
    if (!IsStarted())
    {
        // Can profile only if a configuration is set.
        if (IsProfilingConfigured())
        {
            // Output file is not needed or already set.
            if (m_configs.GetSize() == m_countingEventsCount || IsOutputFileSet())
            {
                if (NULL == hAbortUserEvent || STATUS_SUCCESS == ObReferenceObjectByHandle(hAbortUserEvent,
                        EVENT_MODIFY_STATE,
                        *ExEventObjectType,
                        accessMode,
                        reinterpret_cast<PVOID*>(&m_pAbortUserEvent),
                        NULL))
                {
                    // Initialize profile values.
                    m_profilerState |= STATE_PROFILING;

                    // If there is a sampling file.
                    if (IsOutputFileSet())
                    {
                        if (m_prdWriter.WritePidsList(m_pidList, m_pidCount))
                        {
                            if (WriteConfigurationsToPrdFile())
                            {
                                if (!m_prdWriter.ActivateAsynchronousMode())
                                {
                                    m_profilerState &= ~static_cast<ULONG>(STATE_PROFILING);
                                    SetLastErrorCode(PROF_BUFFER_NOT_ALLOCATED);
                                    status = STATUS_INSUFFICIENT_RESOURCES;
                                }
                            }
                            else
                            {
                                m_profilerState &= ~static_cast<ULONG>(STATE_PROFILING);
                                status = STATUS_FILE_INVALID;
                            }
                        }
                        else
                        {
                            m_profilerState &= ~static_cast<ULONG>(STATE_PROFILING);
                            SetLastErrorCode(PROF_FILE_WRITE_ERROR);
                            status = STATUS_FILE_INVALID;
                        }
                    }


                    if (STATUS_SUCCESS == status)
                    {
                        // Add the configurations to Pcore.
                        status = SubmitConfigurations();

                        if (STATUS_SUCCESS != status)
                        {
                            StopProfiling();
                        }
                    }
                }
                else
                {
                    SetLastErrorCode(PROF_ERROR);
                    status = STATUS_INVALID_PARAMETER;
                }
            }
            else
            {
                SetLastErrorCode(PROF_INVALID_OPERATION);
                status = STATUS_NO_SUCH_FILE;
            }
        }
        else
        {
            PrintError("Cannot start an unconfigured profiling!");
            SetLastErrorCode(PROF_INVALID_OPERATION);
            status = STATUS_RESOURCE_TYPE_NOT_FOUND;
        }
    }
    else
    {
        SetLastErrorCode(PROF_INVALID_OPERATION);
        status = STATUS_DEVICE_BUSY;
    }

    return status;
}


NTSTATUS Client::StopProfiling()
{
    ASSERT(IsValid());

    SetLastErrorCode(PROF_SUCCESS);

    if (!IsStopping())
    {
        if (IsProfiling())
        {
            m_profilerState |= STATE_STOPPING;

            // Terminate any stuff going on for us on that core
            PcoreRemoveAllConfigurations(m_pcoreReg);

            PrintInfo("Client %d has stopped profiling.", GetId());
        }
        else
        {
            PrintInfo("Client %d cannot stop profiling as it is not profiling.", GetId());
        }

        Clear();
    }
    else
    {
        PrintInfo("Client %d cannot stop profiling as it is already stopping.", GetId());
    }

    return STATUS_SUCCESS;
}


NTSTATUS Client::ResumeProfiling()
{
    ASSERT(IsValid());

    NTSTATUS status;

    if (IsProfiling())
    {
        if (IsPaused())
        {
            status = SubmitConfigurations();

            // Clear the paused state.
            m_profilerState &= ~static_cast<ULONG>(STATE_PAUSED);
        }
        else
        {
            PrintInfo("Ignoring Resume operation, as profiling already resumed!");
            status = STATUS_SUCCESS;
        }
    }
    else
    {
        PrintError("Ignoring Resume operation, as profiling is not active!");
        status = STATUS_UNSUCCESSFUL;
    }

    return status;
}


NTSTATUS Client::PauseProfiling()
{
    ASSERT(IsValid());

    NTSTATUS status;

    if (IsProfiling())
    {
        status = STATUS_SUCCESS;

        if (!IsPaused())
        {
            ReadCountingEvents();

            // Pull configurations out of Pcore, so the counters will not advance.
            PcoreRemoveAllConfigurations(m_pcoreReg);

            m_profilerState |= STATE_PAUSED;
        }
        else
        {
            PrintInfo("Ignoring Pause operation, as profiling already paused!");
        }
    }
    else
    {
        PrintError("Ignoring Pause operation, as profiling is not active!");
        status = STATUS_UNSUCCESSFUL;
    }

    return status;
}


NTSTATUS Client::ReadCountingEvent(ULONG core, ULONG resourceIndex, ULONG64 controlValue, ULONG64& count)
{
    ASSERT(IsValid());

    NTSTATUS status;

    if (IsProfiling())
    {
        status = STATUS_RESOURCE_TYPE_NOT_FOUND;

        for (Stack<Configuration>::Iterator it = m_configs.GetBegin(), itEnd = m_configs.GetEnd(); it != itEnd; ++it)
        {
            Configuration* pConfig = &*it;

            if (pConfig->IsEvent() &&
                controlValue == pConfig->GetPcoreConfiguration().msrControlValue &&
                resourceIndex == pConfig->GetResourceId() &&
                pConfig->IsValidCore(core))
            {
                if (IsPaused())
                {
                    count = pConfig->GetEventCount();
                    status = STATUS_SUCCESS;
                }
                // Save the count, like if we are pausing.
                else if (S_OK == PcoreReadCount(m_pcoreReg, core, pConfig->GetType(), resourceIndex, &pConfig->GetPcoreConfiguration(), &count))
                {
                    status = STATUS_SUCCESS;
                }
                else
                {
                    status = STATUS_DEVICE_NOT_READY;
                }

                break;
            }
        }
    }
    else
    {
        status = STATUS_UNSUCCESSFUL;
    }

    return status;
}


NTSTATUS Client::AddEbpConfiguration(const EVENT_PROPERTIES& props)
{
    ASSERT(IsValid());
    ASSERT(GetId() == props.ulClientId);

    NTSTATUS status;

    ULONG maxEventsCount = Device::GetInstance()->GetResourceCount(EVENT_CTR) * MAX_QUEUE_WEIGHT;

    if (maxEventsCount > m_eventsCount)
    {
        if (!IsStarted())
        {
            PERF_CTL perfCtl;

            // Reinterpret as the Hardware MSR representation.
            perfCtl.perf_ctl = props.ullEventCfg;

            if (0 != perfCtl.bitEnabled)
            {
                Configuration* pConfig = new Configuration;

                if (NULL != pConfig)
                {
                    if (pConfig->InitializeEvent(props, SampleDataCallback))
                    {
                        // If the configuration is counting.
                        if (pConfig->IsCountingEvent())
                        {
                            // keep track of the counting events.
                            m_countingEventsCount++;
                        }

                        // Add the configuration to the list to be added to Pcore at start.
                        m_configs.Push(*pConfig);
                        m_eventsCount++;

                        m_profilerState |= STATE_EBP_SET;
                        status = STATUS_SUCCESS;
                    }
                    else
                    {
                        delete pConfig;
                        status = STATUS_NO_MEMORY;
                    }
                }
                else
                {
                    status = STATUS_NO_MEMORY;
                }
            }
            else
            {
                status = STATUS_INVALID_PARAMETER;
            }
        }
        else
        {
            status = STATUS_DEVICE_BUSY;
        }
    }
    else
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return status;
}


NTSTATUS Client::SetTbpConfiguration(const TIMER_PROPERTIES& props)
{
    ASSERT(IsValid());
    ASSERT(GetId() == props.ulClientId);

    NTSTATUS status;

    if (0UL != Device::GetInstance()->GetResourceCount(APIC))
    {
        if (!IsStarted())
        {
            // There can be only 1 TBP configuration.
            if (!IsTbpConfigured())
            {
                const ULONG APIC_TIMER_MINIMUM = 1;

                // Validate granularity is not less than 1 (0.1 mS).
                if (props.ulGranularity >= APIC_TIMER_MINIMUM)
                {
                    Configuration* pConfig = new Configuration;

                    if (NULL != pConfig)
                    {
                        if (pConfig->InitializeTimer(props, SampleDataCallback))
                        {
                            // Add the configuration to the list to be added to Pcore at start.
                            m_configs.Push(*pConfig);

                            m_profilerState |= STATE_TBP_SET;
                            status = STATUS_SUCCESS;
                        }
                        else
                        {
                            delete pConfig;
                            status = STATUS_NO_MEMORY;
                        }
                    }
                    else
                    {
                        status = STATUS_NO_MEMORY;
                    }
                }
                else
                {
                    return STATUS_INVALID_PARAMETER;
                }
            }
            else
            {
                status = STATUS_ALREADY_COMMITTED;
            }
        }
        else
        {
            status = STATUS_DEVICE_BUSY;
        }
    }
    else
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return status;
}


NTSTATUS Client::SetIbsConfiguration(const IBS_PROPERTIES& props)
{
    ASSERT(IsValid());
    ASSERT(GetId() == props.ulClientId);

    NTSTATUS status;
    Device* pDevice = Device::GetInstance();

    if (0UL != pDevice->GetResourceCount(IBS_OP) && 0UL != pDevice->GetResourceCount(IBS_FETCH))
    {
        if (!IsStarted())
        {
            // There can be only 1 IBS configuration.
            if (!IsIbsConfigured())
            {
                if ((props.bProfileFetch && !props.bProfileOp) ||
                    // If the branch target is requested then check that is is valid.
                    (props.bProfileOp && (pDevice->HasIbsBrnTrgt() || 0 == (props.opDataMask & CXL_OP_BR_ADDR_MASK))))
                {
                    status = STATUS_SUCCESS;

                    Configuration* pIbsFetchConfig = NULL;
                    Configuration* pIbsOpConfig = NULL;

                    if (props.bProfileFetch)
                    {
                        pIbsFetchConfig = CreateIbsFetchConfiguration(props, status);
                    }

                    if (props.bProfileOp && STATUS_SUCCESS == status)
                    {
                        pIbsOpConfig = CreateIbsOpConfiguration(props, status);

                        if (NULL == pIbsOpConfig)
                        {
                            if (NULL != pIbsFetchConfig)
                            {
                                delete pIbsFetchConfig;
                                pIbsFetchConfig = NULL;
                            }
                        }
                    }

                    if (STATUS_SUCCESS == status)
                    {
                        m_profilerState |= STATE_IBS_SET;

                        if (NULL != pIbsOpConfig)
                        {
                            // Add the configuration to the list to be added to Pcore at start.
                            m_configs.Push(*pIbsOpConfig);
                        }

                        if (NULL != pIbsFetchConfig)
                        {
                            // Add the configuration to the list to be added to Pcore at start.
                            m_configs.Push(*pIbsFetchConfig);
                        }
                    }
                }
                else
                {
                    status = STATUS_INVALID_PARAMETER;
                }
            }
            else
            {
                status = STATUS_ALREADY_COMMITTED;
            }
        }
        else
        {
            status = STATUS_DEVICE_BUSY;
        }
    }
    else
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return status;
}


Configuration* Client::CreateIbsFetchConfiguration(const IBS_PROPERTIES& props, NTSTATUS& status)
{
    Configuration* pConfig;

    // Verify that the fetch count is not out of range.
    if (MIN_IBS_CYCLE_COUNT <= props.ulIbsFetchMaxCnt && props.ulIbsFetchMaxCnt <= MAX_IBS_CYCLE_COUNT)
    {
        pConfig = new Configuration;

        if (NULL != pConfig)
        {
            if (pConfig->InitializeIbsFetch(props, SampleDataCallback))
            {
                status = STATUS_SUCCESS;
            }
            else
            {
                delete pConfig;
                pConfig = NULL;
                status = STATUS_NO_MEMORY;
            }
        }
        else
        {
            status = STATUS_NO_MEMORY;
        }
    }
    else
    {
        pConfig = NULL;
        status = STATUS_INVALID_PARAMETER;
    }

    return pConfig;
}


Configuration* Client::CreateIbsOpConfiguration(const IBS_PROPERTIES& props, NTSTATUS& status)
{
    Configuration* pConfig;

    const ULONG maxIbsCount = Device::GetInstance()->IsIbsOpCntExt() ? MAX_IBS_EXT_COUNT : MAX_IBS_CYCLE_COUNT;

    // Verify that the count is within boundaries.
    if (MIN_IBS_CYCLE_COUNT <= props.ulIbsOpMaxCnt && props.ulIbsOpMaxCnt <= maxIbsCount)
    {
        pConfig = new Configuration;

        if (NULL != pConfig)
        {
            if (pConfig->InitializeIbsOp(props, SampleDataCallback))
            {
                status = STATUS_SUCCESS;
            }
            else
            {
                delete pConfig;
                pConfig = NULL;
                status = STATUS_NO_MEMORY;
            }
        }
        else
        {
            status = STATUS_NO_MEMORY;
        }
    }
    else
    {
        pConfig = NULL;
        status = STATUS_INVALID_PARAMETER;
    }

    return pConfig;
}


NTSTATUS Client::SetCssConfiguration(const CSS_PROPERTIES& props)
{
    ASSERT(IsValid());
    ASSERT(GetId() == props.ulClientId);

    NTSTATUS status;

    // If the specified call-stack is not too deep or too shallow.
    if (MAX_CSS_VALUES >= props.ulCSSDepth && 0 != props.ulCSSDepth)
    {
        // If the target CSS sampling mode is set.
        if (0 != (props.ucTargetSamplingMode & (CSS_USER_MODE | CSS_KERNEL_MODE)))
        {
            // If the interval is valid.
            if (0 != props.ulCSSInterval)
            {
                // If there are not too many initial modules given.
                if (props.ulNumCodeRange < (InitialCodeRangeBufferSize / sizeof(CSS_CodeRange)))
                {
                    ULONG clientId = GetId();
                    HANDLE processId = reinterpret_cast<HANDLE>(props.ulTargetPid);
                    StackTraceDispatcher& stackTraceDispatcher = Device::GetInstance()->GetStackTraceDispatcher();

                    // Store the PID of the target application.
                    if (NULL != stackTraceDispatcher.AcquireStackWalker(reinterpret_cast<HANDLE>(props.ulTargetPid),
                                                                        clientId,
                                                                        props.ulCSSDepth,
                                                                        props.aCodeRangeInfo,
                                                                        props.ulNumCodeRange))
                    {
                        m_cssConfig.Initialize(props);
                        stackTraceDispatcher.SetCaptureStackPotentialValues(clientId, m_cssConfig.IsCaptureStackPotentialValuesEnabled());
                        m_profilerState |= STATE_CSS_SET;

                        if (!IsAttachedToProcess(processId))
                        {
                            AttachToProcess(processId);
                        }

                        status = STATUS_SUCCESS;
                    }
                    else
                    {
                        status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }
                else
                {
                    status = STATUS_INVALID_PARAMETER;
                }
            }
            else
            {
                status = STATUS_INVALID_PARAMETER;
            }
        }
        else
        {
            status = STATUS_INVALID_PARAMETER;
        }
    }
    else
    {
        // Can not handle unwinding this deep, because of buffer size limitation
        status = STATUS_INVALID_PARAMETER;
    }

    return status;
}


NTSTATUS Client::GetCssConfiguration(CSS_PROPERTIES& props) const
{
    ASSERT(IsValid());
    ASSERT(GetId() == props.ulClientId);

    NTSTATUS status;

    if (IsCssEnabled())
    {
        props.ulTargetPid = 0ULL; //TODO: Do we need it? It will require a lot of work to maintain this information. In the meantime it is not used.
        props.ucTargetSamplingMode = m_cssConfig.GetMode();
        props.bCaptureVirtualStack = m_cssConfig.IsCaptureStackPotentialValuesEnabled() ? TRUE : FALSE;
        props.ulCSSInterval = m_cssConfig.GetSampleInterval();
        props.ulCSSDepth = m_cssConfig.GetMaxDepth();

        status = STATUS_SUCCESS;
    }
    else
    {
        status = STATUS_DEVICE_NOT_READY;
    }

    return status;
}


NTSTATUS Client::GetEbpConfigurations(EVENT_PROPERTIES* pProps, ULONG& length) const
{
    ASSERT(IsValid());
    ASSERT(0UL == length || NULL != pProps);

    NTSTATUS status;

    if (m_eventsCount <= length)
    {
        status = STATUS_RESOURCE_TYPE_NOT_FOUND;

        if (IsEbpConfigured() && 0UL != m_eventsCount)
        {
            ASSERT(GetId() == pProps->ulClientId);

            ULONG count = m_eventsCount;

            for (Stack<Configuration>::ConstIterator it = m_configs.GetBegin(), itEnd = m_configs.GetEnd(); it != itEnd; ++it)
            {
                const Configuration* pConfig = &*it;

                if (pConfig->IsEvent())
                {
                    pConfig->GetEventConfiguration(*pProps);
                    ASSERT(GetId() == pProps->ulClientId);

                    count--;

                    if (0UL == count)
                    {
                        status = STATUS_SUCCESS;
                        break;
                    }

                    pProps++;
                }
            }
        }
    }
    else
    {
        status = STATUS_BUFFER_TOO_SMALL;
    }

    length = m_eventsCount;

    return status;
}


NTSTATUS Client::GetTbpConfiguration(TIMER_PROPERTIES& props) const
{
    ASSERT(IsValid());
    ASSERT(GetId() == props.ulClientId);

    NTSTATUS status = STATUS_RESOURCE_TYPE_NOT_FOUND;

    if (IsTbpConfigured())
    {
        for (Stack<Configuration>::ConstIterator it = m_configs.GetBegin(), itEnd = m_configs.GetEnd(); it != itEnd; ++it)
        {
            const Configuration* pConfig = &*it;

            // There can be only 1 TBP configuration.
            if (APIC == pConfig->GetType())
            {
                pConfig->GetTimerConfiguration(props);
                status = STATUS_SUCCESS;
                break;
            }
        }
    }

    return status;
}


NTSTATUS Client::GetIbsConfiguration(IBS_PROPERTIES& props) const
{
    ASSERT(IsValid());
    ASSERT(GetId() == props.ulClientId);

    NTSTATUS status = STATUS_RESOURCE_TYPE_NOT_FOUND;

    if (IsIbsConfigured())
    {
        RtlZeroMemory(&props, sizeof(props));
        props.ulClientId = GetId();

        // There can be only 1 IBS-Fetch and 1 IBS-Op configurations.
        ULONG configsFlag = (1UL << IBS_FETCH) | (1UL << IBS_OP);

        for (Stack<Configuration>::ConstIterator it = m_configs.GetBegin(), itEnd = m_configs.GetEnd(); it != itEnd; ++it)
        {
            const Configuration* pConfig = &*it;

            if (IBS_FETCH == pConfig->GetType())
            {
                pConfig->GetIbsFetchConfiguration(props);

                status = STATUS_SUCCESS;

                // Remove the IBS-Fetch flag.
                configsFlag &= ~(1UL << IBS_FETCH);

                if (0UL == configsFlag)
                {
                    break;
                }
            }
            else if (IBS_OP == pConfig->GetType())
            {
                pConfig->GetIbsOpConfiguration(props);

                status = STATUS_SUCCESS;

                // Remove the IBS-Op flag.
                configsFlag &= ~(1UL << IBS_OP);

                if (0UL == configsFlag)
                {
                    break;
                }
            }
        }
    }

    return status;
}


void Client::SetAutoAttachToChildProcesses(bool attach)
{
    m_addChildrenPidsToList = attach;
}


bool Client::AttachToProcess(HANDLE processId, ULONG core)
{
    bool ret = false;

    if (MAX_PID_COUNT > m_pidCount)
    {
        ScopedLock lock(m_pidMutex);

        if (MAX_PID_COUNT > m_pidCount)
        {
            m_pidList[m_pidCount] = processId;
            _ReadWriteBarrier();
            m_pidCount++;
            ret = true;
        }
    }

    if (ret && !IsSystemWide())
    {
        m_prdWriter.AsyncWriteProcessId(processId, core);
    }

    return ret;
}


bool Client::InitializeAttachedProcesses(ULONG64* pProcessIds)
{
    ULONG i = 0UL;

    for (; i < MAX_PID_COUNT && NULL != pProcessIds[i]; ++i)
    {
        m_pidList[i] = reinterpret_cast<HANDLE>(pProcessIds[i]);
    }

    bool ret = (0UL != i);

    if (ret)
    {
        m_pidCount = i;
        m_profilerState |= STATE_PID_FILTER_SET;
    }

    return ret;
}


bool Client::DetachFromProcess(HANDLE processId)
{
    bool ret = false;

    ScopedLock lock(m_pidMutex);

    ULONG i = m_pidCount;

    // Processes destruction is usually in reversed order.
    while (0UL != i)
    {
        i--;

        if (processId == m_pidList[i])
        {
            m_pidCount--;
            _ReadWriteBarrier();
            m_pidList[i] = m_pidList[m_pidCount];
            m_pidList[m_pidCount] = NULL;
            ret = true;
            break;
        }
    }

    return ret;
}


bool Client::IsAttachedToProcess(HANDLE processId) const
{
    bool ret = false;

    for (ULONG i = 0, count = m_pidCount; i < count; ++i)
    {
        if (processId == m_pidList[i])
        {
            ret = true;
            break;
        }
    }

    return ret;
}


const HANDLE* Client::GetAttachedProcesses(ULONG& count) const
{
    count = m_pidCount;
    return m_pidList;
}


void Client::ProcessCreatedCallback(HANDLE parentId, HANDLE processId, ULONG core, StackWalker*& pStackWalker, bool& paramStackWalker)
{
    bool isCssEnabled = IsCssEnabled();

    if (isCssEnabled || !IsSystemWide())
    {
        if (IsAutoAttachToChildProcessesEnabled())
        {
            if (IsAttachedToProcess(parentId))
            {
                if (AttachToProcess(processId, core))
                {
                    if (isCssEnabled)
                    {
                        if (!paramStackWalker)
                        {
                            paramStackWalker = true;
                            pStackWalker = Device::GetInstance()->GetStackTraceDispatcher().AcquireStackWalker(processId,
                                           GetId(),
                                           m_cssConfig.GetMaxDepth());

                            if (NULL == pStackWalker)
                            {
                                PrintError("Failed to add new process to CSS monitoring list!");
                            }
                        }
                        else if (NULL != pStackWalker)
                        {
                            pStackWalker->AddRef();
                            pStackWalker->RegisterClient(GetId(), m_cssConfig.GetMaxDepth());
                        }
                    }
                }
            }
        }
    }
}


void Client::ProcessDestroyedCallback(HANDLE processId, StackWalker*& pStackWalker, bool& paramStackWalker)
{
    bool isCssEnabled = IsCssEnabled();

    if (isCssEnabled || !IsSystemWide())
    {
        if (DetachFromProcess(processId))
        {
            if (isCssEnabled)
            {
                if (!paramStackWalker)
                {
                    paramStackWalker = true;
                    pStackWalker = Device::GetInstance()->GetStackTraceDispatcher().FindStackWalker(processId);

                    if (NULL != pStackWalker)
                    {
                        pStackWalker->Release();
                    }
                }

                if (NULL != pStackWalker && pStackWalker->UnregisterClient(GetId()))
                {
                    pStackWalker->Release();
                }
            }
        }
    }
}


void Client::SampleDataCallback(const PCORE_DATA& data)
{
    bool isAttachedToProcess = IsAttachedToProcess(data.contextId.m_processId);

    if (IsSystemWide() || isAttachedToProcess)
    {
        StackTraceDispatcher& stackTraceDispatcher = Device::GetInstance()->GetStackTraceDispatcher();

        ULONG kernelCallersCount = 0UL;
        ULONG_PTR* pKernelCallers = NULL;

        bool isCssEnabled = IsCssEnabled() && isAttachedToProcess;
        bool collectKernelCss = (isCssEnabled && m_cssConfig.IsKernelMode() && IS_KERNEL_MODE(data.pTrapFrame->SegCs));

        if (collectKernelCss)
        {
            isCssEnabled = m_cssConfig.UpdateInterval(data.core);

            if (isCssEnabled)
            {
                kernelCallersCount = stackTraceDispatcher.CaptureKernelStackBackTrace(GetId(),
                                                                                      data.timeStamp,
                                                                                      *data.pTrapFrame,
                                                                                      pKernelCallers,
                                                                                      m_cssConfig.GetMaxDepth());
            }
        }

        ASSERT(GetCoreResourceWeightsBegin(data.core, static_cast<PCORERESOURCETYPES>(data.type)) !=
               GetCoreResourceWeightsEnd(data.core, static_cast<PCORERESOURCETYPES>(data.type)));

        // If the weight has changed.
        UCHAR* pResourceWeight = &GetCoreResourceWeightsBegin(data.core, static_cast<PCORERESOURCETYPES>(data.type)).GetTypeBegin()[data.ResourceId];
        bool weightChanged = (data.weightOfResource != *pResourceWeight);

        bool extendedData = PrdDataBuffer::IsExtendedData(data);

#ifdef _X86_
        const bool is64Bit = false;
#else
        const bool is64Bit = true;
#endif
        PrdDataBuffer* pBuffer = m_prdWriter.GetDataBuffer(data.core, kernelCallersCount, 0UL, false, is64Bit, weightChanged, extendedData);

        // If there is a current buffer for the core and client.
        if (NULL != pBuffer)
        {
            // If the weight has changed, add the weight change record.
            if (weightChanged)
            {
                // Save new weight.
                *pResourceWeight = static_cast<UCHAR>(data.weightOfResource);
                m_samplesCount += pBuffer->AppendResourceWeights(*this, data.core);
            }

            // Add the data to the buffer.
            m_samplesCount += pBuffer->AppendSampleData(data, extendedData, m_startTime);

            // If kernel CSS is present.
            if (0UL != kernelCallersCount)
            {
                m_samplesCount += pBuffer->AppendKernelCallStack(pKernelCallers, kernelCallersCount);
            }

            if (isCssEnabled && m_cssConfig.IsUserMode())
            {
                // If we have already collected kernel mode CSS, then we have also updated the interval.
                if (collectKernelCss || (IS_USER_MODE(data.pTrapFrame->SegCs) && m_cssConfig.UpdateInterval(data.core)))
                {
                    stackTraceDispatcher.EnqueueUserStackBackTrace(GetId(), data.timeStamp);
                }
            }
        }
        else
        {
            UpdateMissedData(data);
        }
    }
}


VOID NTAPI Client::SampleDataCallback(PCORE_DATA* pData)
{
    PKTHREAD pThread = KeGetCurrentThread();

    if (!StackTraceDispatcher::IsUserStackWalkActive(pThread))
    {
#ifdef DBG
        ULONG64 debugFirstTick = ReadTimeStampCounter();
        ULONG64 debugSecondTick;
#endif

        // The callback argument informs us of which client set the configuration on Pcore.
        ULONG clientId = reinterpret_cast<ULONG>(pData->callbackArgument);
        Client* pClient = GetClient(clientId);
        ASSERT(NULL != pClient);

        // If the client shared memory and the profile key were not paused.
        if (pClient->IsActive() && !pClient->IsSamplingSuspended())
        {
            pClient->SampleDataCallback(*pData);
        }

#ifdef DBG
        debugSecondTick = ReadTimeStampCounter();
        pClient->AddSamplingOverhead(debugSecondTick - debugFirstTick);
#endif
    }
}


void Client::UserStackBackTraceCompleteCallback(ULONG clientId,
                                                const ULONG_PTR pCallers[], ULONG callersCount,
                                                const ULONG32 pValues[], const USHORT pOffsets[], ULONG stackValuesCount,
                                                ULONG_PTR stackPtr, ULONG_PTR framePtr,
                                                BOOLEAN is64Bit,
                                                HANDLE processId, HANDLE threadId,
                                                ULONG64 startTime, ULONG64 endTime)
{
#ifdef DBG
    ULONG64 debugFirstTick = ReadTimeStampCounter();
    ULONG64 debugSecondTick;
#endif

    ASSERT(0UL != callersCount);

    Client* pClient = GetClient(clientId);
    ASSERT(NULL != pClient);

    if (pClient->IsValid() && pClient->m_prdWriter.IsAsynchronousModeActive())
    {
        ULONG core = KeGetCurrentProcessorNumberEx(NULL);

        PrdDataBuffer* pBuffer = pClient->m_prdWriter.GetDataBuffer(core, callersCount, stackValuesCount, true, TRUE == is64Bit);

        // If there is a current buffer for the core and client.
        if (NULL != pBuffer)
        {
            pClient->m_samplesCount += pBuffer->AppendUserCallStack(pCallers, callersCount,
                                                                    is64Bit,
                                                                    processId, threadId,
                                                                    startTime - pClient->m_startTime,
                                                                    endTime - pClient->m_startTime);

            if (0UL != stackValuesCount)
            {
                pClient->m_samplesCount += pBuffer->AppendVirtualStack(pValues, pOffsets, stackValuesCount, stackPtr, framePtr);
            }
        }
    }

#ifdef DBG
    debugSecondTick = ReadTimeStampCounter();
    pClient->AddSamplingOverhead(debugSecondTick - debugFirstTick);
#endif
}

} // namespace CpuProf
