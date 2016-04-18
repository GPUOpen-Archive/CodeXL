#ifndef _CPUPROF_TIWRITER_HPP_
#define _CPUPROF_TIWRITER_HPP_
#pragma once

#include "CpuProfFileWriter.hpp"

#pragma warning(push)
#pragma warning(disable:4201) // nameless struct/union

namespace CpuProf {

class TiWriter : ExplicitObject
{
private:
    enum
    {
        // The size of one buffer of task info data.
        BUFFER_SIZE = 256 * PAGE_SIZE,
        // The task info buffer is split into 2 parts, so one part can be written while data is recorded in the second.
        NUM_BUFFERS = 2,

        MAX_RECORDS = (BUFFER_SIZE / sizeof(TASK_INFO_RECORD))
    };

    struct TiBufferPosition
    {
        union
        {
            struct
            {
                USHORT m_recordsCount;
                USHORT m_index;
            };

            ULONG m_value;
        };
    };

    FileWriter m_file;

    UCHAR* m_pBuffers;
    volatile TiBufferPosition m_pos;
    AtomicCounter<LONG> m_refCount;
    AtomicCounter<LONG> m_bufferRefCounts[NUM_BUFFERS];

    TASK_INFO_RECORD* AllocateRecord(UCHAR*& pPrevBuffer);
    TASK_INFO_RECORD* AcquireRecord(UCHAR*& pPrevBuffer);
    void ReleaseRecord(TASK_INFO_RECORD* pRecord);

    ULONG GetBufferIndex(const void* ptr) const;

public:
    TiWriter();
    ~TiWriter();

    bool Open(const wchar_t* pFilePath, ULONG length);
    bool Close();

    bool IsOpened() const { return m_file.IsOpened(); }

    bool WriteRecord(const TASK_INFO_RECORD& tiRecord);

    ULONG GetFilePath(wchar_t* pBuffer, ULONG length) const { return m_file.GetPath(pBuffer, length); }

    LONG AddRef();
    LONG Release();
};

} // namespace CpuProf

#pragma warning(pop)

#endif // _CPUPROF_TIWRITER_HPP_
