//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Utilities header file for MDO.
///         Home of the MdoUtil namespace, containing utility functions used
///         throughout MDO. Here we also find reusable macros, includes, etc.
//==============================================================================

#ifndef __MDO_UTIL_H__
#define __MDO_UTIL_H__

#include <windows.h>
#include <process.h>

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <sstream>
#include <fstream>

#ifdef _DEBUG
    #define MDO_ASSERT(__expr__) if (!(__expr__)) __debugbreak(); ///< Definition
    #define MDO_ASSERT_ALWAYS() __debugbreak(); ///< Definition
    #define MDO_ASSERT_NOT_IMPLEMENTED() __debugbreak(); ///< Definition
#else
    #define MDO_ASSERT(__expr__) ///< Definition
    #define MDO_ASSERT_ALWAYS() ///< Definition
    #define MDO_ASSERT_NOT_IMPLEMENTED() ///< Definition
#endif

#define MDO_SAFE_DELETE(__expr__) if (__expr__) delete __expr__; __expr__ = nullptr; ///< Definition
#define MDO_SAFE_DELETE_ARRAY(__expr__) if (__expr__) delete [] __expr__; __expr__ = nullptr; ///< Definition

/// MDO delta storage enumerations
enum MdoDeltaStorage
{
    MDO_DISABLED,
    MDO_DELTA_STORAGE_PER_MAP,
    MDO_DELTA_STORAGE_PER_BYTE,
};

/// MDO stat enumerations
enum MdoState
{
    MDO_STATE_CAPTURE_START,
    MDO_STATE_CAPTURE,
    MDO_STATE_PLAYBACK,
};

class MdoResource;

/// Config options for MDO
struct MdoConfig
{
    MdoDeltaStorage deltaStorage; ///< Delta storage
    bool            bypassExceptionFiltering; ///< Exception filtering

    // For debugging
    bool            dbgMirrorAppMaps; ///< Mirrors
    bool            dbgMdoSpaceUsage; ///< Space usage
    bool            dbgMdoTimeUsage; ///< Data field
};

/// Stores the map information for a resource
struct MdoResMapInfo
{
    UINT64 resHandle; ///<  Handle
    UINT32 subResource; ///< Subresource
    UINT32 mapType; ///< Map type
    UINT32 mapFlags; ///< Map flags
    void*  pDriverMem; ///< Pointer to  memory
};

/// Stores a mapId to data relationship
struct SlotHistory
{
    UINT32        mapId; ///< MapID data field
    unsigned char data; ///< Data field
};

/// List of slot histories
typedef std::vector<SlotHistory> SlotHistories;

/// Reflected buffer data
struct ReflectionData
{
    unsigned char* pReferenceData; ///< Original data pointer
    unsigned char* pNewData; ///< Copy of the original data
    SlotHistories* pHistories; ///< List of slots
};

/// Resource creation info
struct MdoResourceCreateInfo
{
    UINT64    resHandle; ///< Handle to the resource
    void*     pDevice; ///< Pointer to the creating device
    MdoConfig mdoConfig; ///< Configuration of the MDO
};

/// Records a chunk of data
struct DataChunk
{
    UINT32 offset; ///< Offset into the data
    UINT32 size; ///< Size of the chunk
    void*  pData; ///< Pointer to the data
};

typedef std::vector<DataChunk> DataChunks; ///< Datat chunk store

/// Stores the relationship between dirtypages, buffer deltas and mapinfo data
struct MapEvent
{
    DataChunks     dirtyPages; ///< Dirty pages
    DataChunks     deltas; ///< Deltas
    MdoResMapInfo  mapInfo; ///< Map info

    // For debugging
    unsigned char* pAppMapMirror; ///< App map mirror
};

typedef std::unordered_map<int, MapEvent> MapEvents; ///< Map of events
typedef std::map<int, DataChunks> OrderedAccumDeltas; ///< Map of deltas
typedef std::unordered_map<UINT64, MdoResource*> ResourceMap; ///< Map of resources

/**
**************************************************************************************************
* @brief Util and debug functions for MDO.
**************************************************************************************************
*/
namespace MdoUtil
{
/// Alignment support function
/// \param val the address
/// \param alignment the alignment size
/// \return the output address
inline UINT64 Pow2Align(UINT64 val, UINT64 alignment)
{
    return ((val + alignment - 1) & ~(alignment - 1));
}

/// For logging/debugging
std::string FormatWithCommas(UINT32 value);

/// For logging/debugging
void Print(const char* pStr);

/// For logging/debugging
void PrintLn(const char* pStr);

/// For logging/debugging
bool DumpMemBufferToDiskArray(
    const std::string& name,
    const std::string& path,
    UINT32             bytesPerRow,
    const char*        pBuf,
    UINT32             byteCount);

/// For logging/debugging
void DumpFileToDiskArray(
    const std::string& name,
    const std::string& path,
    const std::string& ext,
    UINT32             bytesPerRow);
}

/// Used control the use of older timestamp counter timing method
#define USE_RDTSC 0

/**
**************************************************************************************************
* @brief A CPU timer class.
**************************************************************************************************
*/
class MdoTimer
{
public:
    MdoTimer() : m_freq(0.0), m_startTime(0)
    {
#ifdef _WIN32

#if USE_RDTSC
        static const GR_DOUBLE CalibrationTime = 1.0;
        UINT64 t0 = rdtsc_time();
        Delay(CalibrationTime);
        UINT64 t1 = rdtsc_time();
        m_freq = static_cast<GR_DOUBLE>(t1 - t0) / CalibrationTime;
#else
        LARGE_INTEGER freq = { 0 };
        QueryPerformanceFrequency(&freq);
        m_freq = static_cast<double>(freq.QuadPart);
#endif

#endif
    }

    /// Starts the timer
    void Start()
    {
#ifdef _WIN32

#if USE_RDTSC
        m_startTime = rdtsc_time();
#else
        LARGE_INTEGER t = { 0 };
        QueryPerformanceCounter(&t);
        m_startTime = t.QuadPart;
#endif

#endif
    }

    /// Stop the timer.
    double Stop()
    {
        UINT64 endTime = m_startTime;

#ifdef _WIN32

#if USE_RDTSC
        endTime = rdtsc_time();
#else
        LARGE_INTEGER t = { 0 };
        QueryPerformanceCounter(&t);
        endTime = t.QuadPart;
#endif

#endif

        return static_cast<double>(endTime - m_startTime) / m_freq;
    }

private:

    /// Spin loops on the input delay time.
    /// \param delay Time delta to wait for
    void Delay(double delay)
    {
#ifdef _WIN32

        LARGE_INTEGER st = { 0 };
        LARGE_INTEGER et = { 0 };
        QueryPerformanceCounter(&st);

        bool calibrating = false;

        while (calibrating)
        {
            QueryPerformanceCounter(&et);

            double t = static_cast<double>(et.QuadPart - st.QuadPart) / m_freq;

            if (t >= delay)
            {
                calibrating = false;
            }
        }

#endif
    }

    double m_freq; ///< timer frequency
    UINT64 m_startTime; ///< timer start time
};

/**
**************************************************************************************************
* @brief A wrapper class to simplify using the OS's CRITICAL_SECTION
**************************************************************************************************
*/
class MdoMutex
{
public:
    /// Constructor
    ///
    /// Initializes the critical section so that it can be used
    MdoMutex()
    {
        InitializeCriticalSection(&m_cs);
    }

    /// Destructor
    ///
    /// Deletes the critical section
    ~MdoMutex()
    {
        DeleteCriticalSection(&m_cs);
    }

    /// Locks the critical section
    void Lock()
    {
        EnterCriticalSection(&m_cs);
    }

    /// Unlocks the critical section
    void Unlock()
    {
        LeaveCriticalSection(&m_cs);
    }

private:
    /// The underlying critical section object
    CRITICAL_SECTION m_cs;
};

/// Helper for Mutex class which Enters a critical section in the constructor and Leaves that section in the destructor
class ScopedLock
{
public:
    /// enters a critical section
    /// \param m pointer to the mutex to use
    ScopedLock(MdoMutex* m)
    {
        m_pMtx = m;
        m_pMtx->Lock();
    }

    /// enters a critical section
    /// \param m reference to the mutex to use
    ScopedLock(MdoMutex& m)
    {
        m_pMtx = &m;
        m_pMtx->Lock();
    }

    /// Leaves the critical section
    ~ScopedLock()
    {
        m_pMtx->Unlock();
    }

private:
    /// Underlying mutex class
    MdoMutex* m_pMtx;
};


#endif