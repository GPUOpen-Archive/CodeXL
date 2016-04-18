#include "..\inc\CpuProfTiWriter.hpp"

namespace CpuProf
{

TiWriter::TiWriter() : m_pBuffers(NULL), m_refCount(0L)
{
    m_pos.m_value = 0UL;
}


TiWriter::~TiWriter()
{
    Close();
}


bool TiWriter::Open(const wchar_t* pFilePath, ULONG length)
{
    Close();

    if (m_file.Open(pFilePath, length))
    {
        m_pBuffers = static_cast<UCHAR*>(ExAllocatePoolWithTag(PagedPool, NUM_BUFFERS * BUFFER_SIZE, ALLOC_POOL_TAG));

        if (NULL != m_pBuffers)
        {
            m_refCount++;
        }
        else
        {
            m_file.Close();
        }
    }

    return m_file.IsOpened();
}


bool TiWriter::Close()
{
    bool ret = true;

    if (0L != m_refCount)
    {
        m_refCount--;

        while (0L != m_refCount)
        {
            YieldProcessor();
        }

        if (m_file.IsOpened() && 0 != m_pos.m_recordsCount)
        {
            m_file.Write(&m_pBuffers[m_pos.m_index], sizeof(TASK_INFO_RECORD) * m_pos.m_recordsCount);
        }

        ExFreePoolWithTag(m_pBuffers, ALLOC_POOL_TAG);
        m_pBuffers = NULL;
        m_pos.m_value = 0UL;
    }

    ASSERT(0UL == m_pos.m_value && NULL == m_pBuffers);

    if (m_file.IsOpened())
    {
        m_file.Close();
    }

    return ret;
}


LONG TiWriter::AddRef()
{
    LONG refCount;

    do
    {
        refCount = m_refCount;

        if (0L >= refCount)
        {
            break;
        }
    }
    while (!m_refCount.CompareAndSwap(refCount, refCount + 1L));

    // The reference count should never reach a negative number.
    ASSERT(0L <= refCount);

    return refCount;
}


LONG TiWriter::Release()
{
    LONG refCount = --m_refCount;

    // The reference count should never reach a negative number.
    ASSERT(0L <= refCount);

    return refCount;
}


TASK_INFO_RECORD* TiWriter::AcquireRecord(UCHAR*& pPrevBuffer)
{
    pPrevBuffer = NULL;
    TASK_INFO_RECORD* pRecord = NULL;

    LONG refCount = AddRef();

    if (0L < refCount)
    {
        //
        // Acquire a reference to the buffer, by increasing its reference count.
        //
        // There are 3 possible scenarios:
        // 1. There is enough space to allocate a new record on the current buffer and the new record is actually allocated there.
        //    - Is is safe to play with the reference count however we want, as long as we keep it balanced.
        // 2. There is enough space but by the time we tried to allocate, the new record is actually on the next buffer.
        //    - We may interfere with the writer of the full buffer only until the current allocation is done.
        // 3. The buffer is full and we allocate on the next buffer.
        //    - We may interfere with the writer of the full buffer only until the current allocation is done.
        //

        AtomicCounter<LONG>& oldBufferRefCount = m_bufferRefCounts[m_pos.m_index];
        oldBufferRefCount++;
        pRecord = AllocateRecord(pPrevBuffer);
        oldBufferRefCount--;
    }

    return pRecord;
}


TASK_INFO_RECORD* TiWriter::AllocateRecord(UCHAR*& pPrevBuffer)
{
    //
    // We try to aggregate as many records on the same buffer (of size BUFFER_SIZE).
    // When the buffer is full, we increment the buffer index and allocate the record on the new buffer,
    // then pPrevBuffer is set to the previously already full buffer.
    //

    TiBufferPosition oldPos, newPos;

    do
    {
        oldPos.m_value = m_pos.m_value;

        // If we have not reached the capacity.
        if (MAX_RECORDS > oldPos.m_recordsCount)
        {
            newPos.m_recordsCount = oldPos.m_recordsCount + 1;
            newPos.m_index = oldPos.m_index;
        }
        else
        {
            //
            // We use the first record of the next buffer in the array.
            //
            // ************************************************************************************************************
            // This method ASSUMES that the next buffer must have already been released (its data already written to file).
            // ************************************************************************************************************
            //

            newPos.m_recordsCount = 1;
            newPos.m_index = (oldPos.m_index + 1) % NUM_BUFFERS;
        }
    }
    while (!AtomicCompareAndSwap(m_pos.m_value, oldPos.m_value, newPos.m_value));

    m_bufferRefCounts[newPos.m_index]++;

    UCHAR* pBuffer = m_pBuffers + (newPos.m_index * BUFFER_SIZE) + ((newPos.m_recordsCount - 1) * sizeof(TASK_INFO_RECORD));

    if (newPos.m_index != oldPos.m_index)
    {
        pPrevBuffer = m_pBuffers + (oldPos.m_index * BUFFER_SIZE);
    }

    return reinterpret_cast<TASK_INFO_RECORD*>(pBuffer);
}


void TiWriter::ReleaseRecord(TASK_INFO_RECORD* pRecord)
{
    ULONG bufferIndex = GetBufferIndex(pRecord);

    m_bufferRefCounts[bufferIndex]--;

    // Remove the reference to the buffer.
    Release();
}


ULONG TiWriter::GetBufferIndex(const void* ptr) const
{
    ULONG_PTR offset = reinterpret_cast<ULONG_PTR>(ptr) - reinterpret_cast<ULONG_PTR>(m_pBuffers);
    return static_cast<ULONG>(offset / BUFFER_SIZE);
}


bool TiWriter::WriteRecord(const TASK_INFO_RECORD& tiRecord)
{
    bool ret = true;

    UCHAR* pPrevBuffer;
    TASK_INFO_RECORD* pRecord = AcquireRecord(pPrevBuffer);

    // If there is a ti buffer to write to.
    if (NULL != pRecord)
    {
        // Place the task info data into the buffer.
        *pRecord = tiRecord;

        if (NULL != pPrevBuffer)
        {
            ULONG bufferIndex = GetBufferIndex(pPrevBuffer);

            AtomicCounter<LONG>& prevBufferRefCount = m_bufferRefCounts[bufferIndex];

            // Wait until the we are the only ones holding the reference.
            while (1L < prevBufferRefCount)
            {
                YieldProcessor();
            }

            ret = m_file.Write(pPrevBuffer, MAX_RECORDS * sizeof(TASK_INFO_RECORD));
        }

        ReleaseRecord(pRecord);
    }

    return ret;
}

} // namespace CpuProf
