#ifndef _CPUPROF_PRDBUFFER_HPP_
#define _CPUPROF_PRDBUFFER_HPP_
#pragma once

#pragma warning(push)
#pragma warning(disable:4201) // nameless struct/union

#include "CpuProfCommon.hpp"
#include "UserAccess\PrdRecords.h"

namespace CpuProf {

class Client;

class PrdDataBuffer : public NonPagedObject
{
public:
    /// The size of the buffer if we want the whole structure on one 4096 byte page
    /// 4K - sizeof(QueueEntry) [16 bytes on 64-bit]
    enum
    {
        MAX_RECORDS_COUNT = (PAGE_SIZE - sizeof(::QueueEntry)) / PRD_RECORD_SIZE
    };

    QueueEntry m_linkField;

private:

    union
    {
        // The array of data records
        struct
        {
            UCHAR m_reserved[PRD_RECORD_SIZE];
        } m_buffer[MAX_RECORDS_COUNT];

        PRD_WEIGHT_RECORD m_header;
    };

    ULONG GetLastUserCssRecordOffset() const;
    void SetLastUserCssRecordOffset(ULONG offset);

    void* AcquireNextRecord(ULONG count = 1UL);

public:
    //  ==========================================================================
    /// Appends the interrupt sample data to the buffer. The extended data records
    /// are only written if the data is available.
    ///
    /// \note The extended IBS data must be requested for it to be read and
    /// available from Pcore in the first place.
    /// \note Assumes there are enough free records in the buffer for the
    /// sample data
    ///
    /// \param[in] pData The interrupt sample data
    /// \param[in] startTick The starting tick to calculate the offset
    ///
    /// \return the number of records appended to the buffer
    ///
    /// IRQL Level: IRQL LEVEL
    // ===========================================================================
    ULONG AppendSampleData(const PCORE_DATA& data, bool extendedData, ULONG64 startTick);

#if 0
    ULONG AppendCallStack(const ULONG_PTR* pKernelCallers, ULONG kernelCallersCount,
                          const ULONG_PTR* pUserCallers, ULONG userCallersCount,
                          ULONG maxDepth,
                          ULONG core);
#endif

    ULONG AppendKernelCallStack(const ULONG_PTR* pCallers, ULONG count);
    ULONG AppendUserCallStack(const ULONG_PTR* pCallers, ULONG count,
                              BOOLEAN is64Bit,
                              HANDLE processId, HANDLE threadId,
                              ULONG64 startTick, ULONG64 endTick);

    ULONG AppendVirtualStack(const ULONG32* pValues, const USHORT* pOffsets, ULONG count, ULONG_PTR stackPtr, ULONG_PTR framePtr);

    ULONG AppendResourceWeights(Client& client, ULONG core);

    ULONG AppendProcessId(HANDLE processId);

    ULONG Initialize(Client& client, ULONG core);
    const void* Finalize(ULONG& length, size_t currentRecordOffset, size_t& prevUserCssRecordOffset);

    ULONG GetRecordsCount() const;

    bool HasData() const;
    bool HasEnoughSpace(ULONG recordsCount) const;


    static ULONG GetKernelCallStackRecordsCount(ULONG callersCount);
    static ULONG GetUserCallStackRecordsCount(ULONG callersCount, bool is64Bit);
    static ULONG GetVirtualStackRecordsCount(ULONG valuesCount);
    static bool IsExtendedData(const PCORE_DATA& data);
};


class PrdDataBufferPool : public ExplicitObject
{
private:
    // The communal pool of empty buffers available to all clients
    AtomicQueue<PrdDataBuffer> m_emptyBuffers;

public:
    ~PrdDataBufferPool();

    void Clear();

    bool Reserve(ULONG count);
    ULONG Free(ULONG count);

    PrdDataBuffer* AcquireBuffer();
    void ReleaseBuffer(PrdDataBuffer& buffer);

    static PrdDataBufferPool& GetInstance();
};

inline bool AllocatePrdDataBuffers(ULONG count) { return PrdDataBufferPool::GetInstance().Reserve(count); }
inline ULONG FreePrdDataBuffers(ULONG count) { return PrdDataBufferPool::GetInstance().Free(count); }
inline PrdDataBuffer* AcquirePrdDataBuffer() { return PrdDataBufferPool::GetInstance().AcquireBuffer(); }
inline void ReleasePrdDataBuffer(PrdDataBuffer& buffer) { PrdDataBufferPool::GetInstance().ReleaseBuffer(buffer); }

} // namespace CpuProf

#pragma warning(pop)

#endif // _CPUPROF_PRDBUFFER_HPP_
