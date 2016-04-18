#ifndef _CPUPROF_CLIENT_HPP_
#define _CPUPROF_CLIENT_HPP_
#pragma once

#include "CpuProfConfiguration.hpp"
#include "CpuProfCssConfiguration.hpp"
#include "CpuProfPrdWriter.hpp"
#include "CpuProfTiWriter.hpp"
#include <WinDriverUtils\Include\PtrIterator.hpp>

class StackWalker;

namespace CpuProf {

class Client
{
private:
    struct MissedData : public NonPagedObject
    {
        UCHAR m_type;
        UCHAR m_resourceId;
        ULONG m_missedCount;
        ULONG64 m_controlValue;
    };

    PrdWriter m_prdWriter;

    // Handle to the open temporary task info data file
    TiWriter m_tiWriter;

    // Which user application registered this client
    AtomicCounter<PFILE_OBJECT> m_pUserFileObject;

    // The registration with Pcore.
    HANDLE m_pcoreReg;

    // This array is core * MAX_RESOURCE_TYPE * Device::GetMaxResourceCount() in size.
    // It will track the current weight of resources.
    UCHAR* m_pCoreResourceWeights;
    // This array is core * configCount in size.
    MissedData* m_pCoreMissedData;

    // The configurations list.
    Stack<Configuration> m_configs;

    // A count of the records of sample data taken during the current profile. Obtained via IOCTL_GET_RECORD_COUNT.
    AtomicCounter<ULONG> m_samplesCount;

    // The count of added event configurations.
    ULONG m_eventsCount;
    // The count of added counting event configurations.
    ULONG m_countingEventsCount;

    // A pointer to the given abort notification event that will be set in the case that a profile has to abort.
    KEVENT* m_pAbortUserEvent;
    // The profile can be aborted for two reasons: a file write failed or the client was unregistered.
    CPUPROF_ERROR_CODES m_lastErrorCode;
    // A mask of the profile's current states: see \ref CPUPROF_STATE.
    AtomicCounter<ULONG> m_profilerState;
    // The starting tick value, so all samples will have a delta tick.
    ULONG64 m_startTime;

    CssConfiguration m_cssConfig;

    // PID filter array.
    HANDLE m_pidList[MAX_PID_COUNT];
    // Count of PIDs in the \ref pidList array.
    volatile ULONG m_pidCount;
    GuardedMutex m_pidMutex;
    // Whether to add child processes of filtered PIDs to the list.
    bool m_addChildrenPidsToList;


    void ReadCountingEvents();

    //  ==========================================================================
    /// Adds the configurations to the applicable Pcore cores and resources.
    ///
    /// IRQL Level: PASSIVE_LEVEL
    // ===========================================================================
    NTSTATUS SubmitConfigurations();

    void ClearConfigurationsList();

    ULONG AggregateMissedData(ULONG configIndex, const Configuration& config) const;
    bool WriteMissedDataToPrdFile();
    bool WriteConfigurationsToPrdFile();


    Configuration* CreateIbsFetchConfiguration(const IBS_PROPERTIES& props, NTSTATUS& status);
    Configuration* CreateIbsOpConfiguration(const IBS_PROPERTIES& props, NTSTATUS& status);

    //  ==========================================================================
    /// The data sample callback from Pcore.  Stores the data in a buffer, if
    /// possible
    ///
    /// \param[in] pData Data passed from Pcore in regards to a configuration
    ///
    // ===========================================================================
    static VOID NTAPI SampleDataCallback(PCORE_DATA* pData);
    void SampleDataCallback(const PCORE_DATA& data);

public:
    Client();
    ~Client();

    //  ==========================================================================
    /// Clears the profile settings, except for abort reason, overhead data, and
    /// profile record count
    ///
    /// IRQL Level: PASSIVE_LEVEL
    // ===========================================================================
    void Clear();

    bool SetOutputFile(const wchar_t* pPrdFilePath, ULONG prdLength, const wchar_t* pTiFilePath, ULONG tiLength);
    bool GetOutputFile(wchar_t* pPrdFilePath, ULONG lenPrdFilePathBuffer, wchar_t* pTiFilePath, ULONG lenTiFilePathBuffer);

    ULONG GetId() const;

    bool IsValid() const;
    bool IsRegistered(PFILE_OBJECT pUserFileObject) const;
    bool Register(PFILE_OBJECT pUserFileObject);
    bool Unregister();

    void SetLastErrorCode(CPUPROF_ERROR_CODES errorCode);
    CPUPROF_ERROR_CODES GetLastErrorCode() const { return m_lastErrorCode; }


    NTSTATUS StartProfiling(HANDLE hAbortUserEvent, KPROCESSOR_MODE accessMode);

    //  ==========================================================================
    /// Stops profiling, flushes any partial buffers to the files, and clears the
    /// previous profile's settings.
    ///
    /// \return Success of stopping the profile
    ///
    /// IRQL Level: PASSIVE_LEVEL or
    // ===========================================================================
    NTSTATUS StopProfiling();

    NTSTATUS ResumeProfiling();
    NTSTATUS PauseProfiling();


    NTSTATUS ReadCountingEvent(ULONG core, ULONG resourceIndex, ULONG64 controlValue, ULONG64& count);

    NTSTATUS AddEbpConfiguration(const EVENT_PROPERTIES& props);
    NTSTATUS SetTbpConfiguration(const TIMER_PROPERTIES& props);
    NTSTATUS SetIbsConfiguration(const IBS_PROPERTIES& props);

    NTSTATUS GetEbpConfigurations(EVENT_PROPERTIES* pProps, ULONG& length) const;
    NTSTATUS GetTbpConfiguration(TIMER_PROPERTIES& props) const;
    NTSTATUS GetIbsConfiguration(IBS_PROPERTIES& props) const;

    NTSTATUS SetCssConfiguration(const CSS_PROPERTIES& props);
    NTSTATUS GetCssConfiguration(CSS_PROPERTIES& props) const;


    const TiWriter& GetTiWriter() const { return m_tiWriter; }
          TiWriter& GetTiWriter()       { return m_tiWriter; }

    const PrdWriter& GetPrdWriter() const { return m_prdWriter; }
          PrdWriter& GetPrdWriter()       { return m_prdWriter; }

    const CssConfiguration& GetCssConfiguration() const { return m_cssConfig; }
          CssConfiguration& GetCssConfiguration()       { return m_cssConfig; }

    void SetAutoAttachToChildProcesses(bool attach);
    bool AttachToProcess(HANDLE processId, ULONG core = ~0UL);
    bool InitializeAttachedProcesses(ULONG64* pProcessIds);
    bool DetachFromProcess(HANDLE processId);
    bool IsAttachedToProcess(HANDLE processId) const;

    const HANDLE* GetAttachedProcesses(ULONG& count) const;
    bool IsAutoAttachToChildProcessesEnabled() const { return m_addChildrenPidsToList; }


    //  ==========================================================================
    /// Increments the count of missed data for the sample data provided.
    ///
    /// \param[in] data The interrupt sample data
    // ===========================================================================
    bool UpdateMissedData(const PCORE_DATA& data);

    ULONG GetSamplesCount() const { return m_samplesCount; }
    ULONG GetState() const { return m_profilerState; }

    bool IsConfigured() const { return m_profilerState != STATE_NOT_CONFIGURED; }
    bool IsOutputFileSet() const { return 0 != (m_profilerState & STATE_OUTPUT_FILE_SET); }
    bool IsSystemWide() const { return 0 == (m_profilerState & STATE_PID_FILTER_SET); }

    bool IsCssEnabled() const { return 0 != (m_profilerState & STATE_CSS_SET); }

    bool IsProfilingConfigured() const { return 0 != (m_profilerState & (STATE_EBP_SET | STATE_TBP_SET | STATE_IBS_SET)); }

    bool IsEbpConfigured() const { return 0 != (m_profilerState & STATE_EBP_SET); }
    bool IsTbpConfigured() const { return 0 != (m_profilerState & STATE_TBP_SET); }
    bool IsIbsConfigured() const { return 0 != (m_profilerState & STATE_IBS_SET); }

    bool IsStarted() const { return 0 != (m_profilerState & (STATE_PROFILING | STATE_PAUSED | STATE_STOPPING)); }

    bool IsActive() const { return STATE_PROFILING == (m_profilerState & (STATE_PROFILING | STATE_PAUSED | STATE_STOPPING)); }
    
    bool IsProfiling() const
    {
        return (m_profilerState & (STATE_PROFILING | STATE_STOPPING)) == STATE_PROFILING;
    }

    bool IsPaused() const
    {
        return (m_profilerState & (STATE_PROFILING | STATE_PAUSED | STATE_STOPPING)) == (STATE_PROFILING | STATE_PAUSED);
    }

    bool IsStopping() const
    {
        return (m_profilerState & (STATE_PROFILING | STATE_STOPPING)) == (STATE_PROFILING | STATE_STOPPING);
    }

    bool IsSamplingSuspended() const;


    class CoreResourceWeightsIterator : public PtrIterator<UCHAR>
    {
    private:
        PCORERESOURCETYPES m_type;

    public:
        CoreResourceWeightsIterator(UCHAR* pWeights, PCORERESOURCETYPES type);
        CoreResourceWeightsIterator& operator++();
        PCORERESOURCETYPES GetType() const { return m_type; }

        UCHAR* GetTypeBegin();
        UCHAR* GetTypeEnd();
    };

    CoreResourceWeightsIterator GetCoreResourceWeightsBegin(ULONG core, PCORERESOURCETYPES type = APIC);
    CoreResourceWeightsIterator GetCoreResourceWeightsEnd(ULONG core, PCORERESOURCETYPES type = L2I_CTR);

    void ProcessCreatedCallback(HANDLE parentId, HANDLE processId, ULONG core, StackWalker*& pStackWalker, bool& paramStackWalker);
    void ProcessDestroyedCallback(HANDLE processId, StackWalker*& pStackWalker, bool& paramStackWalker);


    static void WriteCreateProcessInfo(Client* pClient, ULONG64 time, ULONG core, HANDLE hProcessId, BOOLEAN is32Bit, BOOLEAN bCreate);
    static void WriteCreateThreadInfo(Client* pClient, ULONG64 time, ULONG core, HANDLE hProcessId, HANDLE hThreadId, BOOLEAN bCreate);
    static void WriteLoadImageInfo(Client* pClient, TASK_INFO_RECORD tiRecord);


    static void UserStackBackTraceCompleteCallback(ULONG clientId,
                                                   const ULONG_PTR pCallers[], ULONG callersCount,
                                                   const ULONG32 pValues[], const USHORT pOffsets[], ULONG stackValuesCount,
                                                   ULONG_PTR stackPtr, ULONG_PTR framePtr,
                                                   BOOLEAN is64Bit,
                                                   HANDLE processId, HANDLE threadId,
                                                   ULONG64 startTime, ULONG64 endTime);

#ifdef DBG

private:
    // The aggregated debug count of TSC diffs during \ref SampleDataCallback. Obtained via IOCTL_GET_OVERHEAD.
    ULONG64 m_debugSamplingTsc;

public:
    void ClearSamplingOverhead() { m_debugSamplingTsc = 0ULL; }
    void AddSamplingOverhead(ULONG64 time) { m_debugSamplingTsc += time; }
    ULONG64 GetSamplingOverhead() const { return m_debugSamplingTsc; }

#else

public:
    void ClearSamplingOverhead() { }
    void AddSamplingOverhead(ULONG64 time) { UNREFERENCED_PARAMETER(time); }
    ULONG64 GetSamplingOverhead() const { return 0ULL; }

#endif
};

} // namespace CpuProf

#endif // _CPUPROF_CLIENT_HPP_
