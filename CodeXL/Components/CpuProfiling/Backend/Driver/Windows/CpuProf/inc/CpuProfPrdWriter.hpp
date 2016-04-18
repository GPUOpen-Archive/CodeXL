#ifndef _CPUPROF_PRDWRITER_HPP_
#define _CPUPROF_PRDWRITER_HPP_
#pragma once

#include "CpuProfFileWriter.hpp"
#include "CpuProfPrdBuffer.hpp"
#include "CpuProfInternal.h"

namespace CpuProf {

class Client;

class PrdWriter : public ExplicitObject
{
private:
    FileWriter m_file;

    // The event used to synchronize when the reaper runs
    EventNotifier m_reapNotifier;
    // The reaper system thread
    PKTHREAD m_pPrdReapThread;

    size_t m_lastUserCssRecordOffset;
    size_t m_recordsCount;

    // Head of FIFO full-buffer list, inserted by \ref SampleDataCallback and removed by \ref ReaperThread.
    AtomicQueue<PrdDataBuffer> m_fullBuffers;

    // An array of data buffers, one per core, for a sampling profile.
    PrdDataBuffer** m_pCoreBuffers;

    bool WriteLastUserCssRecordOffset();
    bool WriteHeader(ULONG64 startTime, ULONG64 timeFreq);
    bool WriteCpuInfo();
    bool WriteCpuTopology();

    const Client& GetClient() const;
          Client& GetClient();


    ULONG FlushFullDataBuffers();
    void ClearFullDataBuffersList();

    bool CreateDataBuffers();
    bool CreateReaperThread();

    //  ==========================================================================
    /// Waits for a signal, then writes the buffers full of sampling data to the
    /// PRD data file and returns the buffers to the empty buffer pool.
    ///
    /// \note this could have been done with work items, like the TI, but would
    /// involve a lot more overhead with the allocations.
    ///
    /// \param[in] startContext The client for the profile
    ///
    /// IRQL Level: PASSIVE_LEVEL
    // ===========================================================================
    static VOID ReaperThread(PVOID startContext);

public:
    PrdWriter();
    ~PrdWriter();

    bool Open(const wchar_t* pFilePath, ULONG length, ULONG64 startTime, ULONG64 timeFreq);
    void Close();

    bool IsOpened() const { return m_file.IsOpened(); }

    ULONG GetFilePath(wchar_t* pBuffer, ULONG length) const { return m_file.GetPath(pBuffer, length); }

    bool WriteMissedData(const Configuration& config, ULONG missedCount, ULONG64 startTime, ULONG core,
                         const Configuration* pConfigIbsFeth = NULL, ULONG missedIbsFethCount = 0UL);

    bool WritePidsList(const HANDLE* pPidList, ULONG count);

    bool WriteConfiguration(const Configuration& config, ULONG64 startTime);

    bool WriteDataBuffer(PrdDataBuffer& buffer);



    bool ActivateAsynchronousMode();
    bool IsAsynchronousModeActive() const;


    //  ==========================================================================
    /// If applicable, flushes the partial buffers to the reaper for writing, and
    /// waits for the profile data reaper to finish, then finishes the prd file.
    ///
    /// IRQL Level: PASSIVE_LEVEL
    // ===========================================================================
    void DeactivateAsynchronousMode();


    bool AsyncWriteProcessId(HANDLE processId, ULONG core);

    PrdDataBuffer* GetDataBuffer(ULONG core, ULONG callersCount, ULONG stackValuesCount, bool isUser, bool is64Bit);
    PrdDataBuffer* GetDataBuffer(ULONG core, ULONG callersCount, ULONG stackValuesCount, bool isUser, bool is64Bit, bool& weightChanged, bool extendedData);
    PrdDataBuffer* GetDataBuffer(ULONG core, ULONG recordsCount, bool& weightChanged);
    PrdDataBuffer* GetDataBuffer(ULONG core, ULONG recordsCount);


#ifdef DBG

private:
    // The aggregated debug count of TSC diffs during the reaper routine (\ref ReaperThread). Obtained via IOCTL_GET_OVERHEAD.
    ULONG64 m_debugReaperTsc;

public:
    void ClearReaperOverhead() { m_debugReaperTsc = 0ULL; }
    void AddReaperOverhead(ULONG64 time) { m_debugReaperTsc += time; }
    ULONG64 GetReaperOverhead() const { return m_debugReaperTsc; }

#else

public:
    void ClearReaperOverhead() { }
    void AddReaperOverhead(ULONG64 time) { UNREFERENCED_PARAMETER(time); }
    ULONG64 GetReaperOverhead() const { return 0ULL; }

#endif
};

} // namespace CpuProf

#endif // _CPUPROF_PRDWRITER_HPP_
