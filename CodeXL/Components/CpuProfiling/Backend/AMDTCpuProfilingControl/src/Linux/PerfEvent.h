//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PerfEvent.h
///
//==================================================================================

#ifndef _PERFEVENT_H_
#define _PERFEVENT_H_

// Standard headers
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <limits.h>
#include <wchar.h>

// C++ STL headers
#include <list>

#include <AMDTBaseTools/Include/AMDTDefinitions.h>

//
//  Macros
//

#define AMD_PMU_EVENTSEL_EVENT                     0x000000FFULL
#define AMD_PMU_EVENTSEL_UMASK                     0x0000FF00ULL
#define AMD_PMU_EVENT_AND_UMASK                    0x0000FFFFULL

struct PerfEventCountData;

//
//  typedefs
//
typedef std::list<PerfEventCountData> PerfEventCountDataList;
typedef std::list<PerfEventCountData> PerfEventDataList;


// class PerfEvent
//
// Profile Event Configuration
//   - Generic class for Event Configuration
//   -  User Visible
//
class PerfEvent
{
public:
    PerfEvent();

    // ctor for predefined events..
    PerfEvent(uint32_t type, uint32_t pmuFlags, uint32_t taskFlags);

    // ctor for raw profile events
    PerfEvent(uint32_t event, uint32_t unitmask, uint32_t pmuFlags, uint32_t taskFlags);

    // Copy constructor
    PerfEvent(const PerfEvent& event);

    // assignment op
    PerfEvent& operator= (const PerfEvent& event);

    ~PerfEvent();

    bool initialize(uint32_t type, uint32_t pmuFlags, uint32_t taskFlags);
    bool initialize(uint32_t event, uint32_t unitmask, uint32_t pmuFlags, uint32_t taskFlags);
    bool initializeIbs(uint32_t type, uint32_t pmuFlags, uint32_t taskFlags);

    int setEvent(uint32_t eventid, uint32_t unitmask = 0)
    {
        m_eventId  = eventid;
        m_unitMask = unitmask;

        // Set type and config
        m_type   = PERF_PROFILE_TYPE_RAW;
        m_config = (((unitmask & 0xff) << 8) | (eventid & 0xff)) & AMD_PMU_EVENT_AND_UMASK;

        return validateEvent();
    }

    bool setPmuFlags(uint32_t pmuFlags = (PERF_PMU_FLAG_EXCLUDE_KERNEL | PERF_PMU_FLAG_EXCLUDE_IDLE))
    {
        m_pmuFlags = pmuFlags;
        return validatePmuFlags();
    }

    bool setTaskFlags(uint32_t taskFlags = PERF_TASK_FLAG_INHERIT)
    {
        m_taskFlags = taskFlags;
        return validateTaskFlags();
    }

    bool setSkidFactor(uint8_t skidFactor)
    {
        m_skidFactor = skidFactor;
        return validateSkid();
    }

    // Set the PMU Event Counter to be used to program the PMU Event
    // PERF does NoT need this info
    void setPMUCounter(uint32_t eventCounter)
    {
        // This does not check
        //   - whether this is a valid PMC counter, or whether this
        //   - whether this counter can be used for this event
        //   - grouping of events etc..
        m_eventCounter = eventCounter;
    }

    bool validate();
    void clear();
    void print();

    uint32_t getType() const { return m_type; }
    uint64_t getConfig() const { return m_config; }
    uint32_t getEventId() const { return m_eventId; }
    uint32_t getUnitMask() const { return m_unitMask; }
    const char* getEventName() const { return m_pEventName; }
    const char* getEventAlias() const { return m_pEventAlias; }
    uint8_t  getSkidFactor() const { return m_skidFactor; }
    uint32_t getPmuFlags() const { return m_pmuFlags; }
    uint32_t getTaskFlags() const { return m_taskFlags; }
    bool     isInitialized() const { return m_initialized; }
    uint32_t getPMUCounter() const { return m_eventCounter; }

    enum ProfileTypes
    {
        // PERF - command hardware events
        //     type = PERF_TYPE_HARDWARE
        //     config = PERF_COUNT_HW_*
        PERF_PROFILE_TYPE_HW_CPU_CYCLES                 = 0,
        PERF_PROFILE_TYPE_HW_INSTRUCTIONS               = 1,
        PERF_PROFILE_TYPE_HW_CACHE_REFERENCES           = 2,
        PERF_PROFILE_TYPE_HW_CACHE_MISSES               = 3,
        PERF_PROFILE_TYPE_HW_BRANCH_INSTRUCTIONS        = 4,
        PERF_PROFILE_TYPE_HW_BRANCH_MISSES              = 5,
        PERF_PROFILE_TYPE_HW_BUS_CYCLES                 = 6,

        // Software events provided by PERF
        //     type = PERF_TYPE_SOFTWARE
        //     config = PERF_COUNT_SW_*
        PERF_PROFILE_TYPE_SW_CPU_CLOCK                  = 7,
        PERF_PROFILE_TYPE_SW_TASK_CLOCK                 = 8,
        PERF_PROFILE_TYPE_SW_PAGE_FAULTS                = 9,
        PERF_PROFILE_TYPE_SW_CONTEXT_SWITCHES           = 10,
        PERF_PROFILE_TYPE_SW_CPU_MIGRATIONS             = 11,
        PERF_PROFILE_TYPE_SW_PAGE_FAULTS_MIN            = 12,
        PERF_PROFILE_TYPE_SW_PAGE_FAULTS_MAJ            = 13,
        PERF_PROFILE_TYPE_SW_ALIGNMENT_FAULTS           = 14,
        PERF_PROFILE_TYPE_SW_EMULATION_FAULTS           = 15,

        // other types
        PERF_PROFILE_TYPE_TRACEPOINT                    = 16,
        PERF_PROFILE_TYPE_BREAKPOINT                    = 17,
        PERF_PROFILE_TYPE_RAW                           = 18,

        // Generalized hardware cache events:
        // ( L1D, L1-I, LLC, ITLB, DTLB, BPI) x
        // ( read, write, prefetch ) x
        // ( accesses, misses )
        //    type = PERF_TYPE_HW_CACHE
        //    config =    PERF_COUNT_HW_CACHE_L1D  <<  0
        //             | (PERF_COUNT_HW_CACHE_OP_READ <<  8)
        //             | (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16)
        PERF_PROFILE_TYPE_HW_CACHE_L1D_LOADS            = 19,
        PERF_PROFILE_TYPE_HW_CACHE_L1D_LOAD_MISSES      = 20,
        PERF_PROFILE_TYPE_HW_CACHE_L1D_STORES           = 21,
        PERF_PROFILE_TYPE_HW_CACHE_L1D_STORE_MISSES     = 22,
        PERF_PROFILE_TYPE_HW_CACHE_L1D_PREFETCHES       = 23,
        PERF_PROFILE_TYPE_HW_CACHE_L1D_PREFETCH_MISSES  = 24,

        PERF_PROFILE_TYPE_HW_CACHE_L1T_LOADS            = 25,
        PERF_PROFILE_TYPE_HW_CACHE_L1T_LOAD_MISSES      = 26,
        PERF_PROFILE_TYPE_HW_CACHE_L1T_PREFETCHES       = 27,
        PERF_PROFILE_TYPE_HW_CACHE_L1T_PREFETCH_MISSES  = 28,

        PERF_PROFILE_TYPE_HW_CACHE_LLC_LOADS            = 29,
        PERF_PROFILE_TYPE_HW_CACHE_LLC_LOAD_MISSES      = 30,
        PERF_PROFILE_TYPE_HW_CACHE_LLC_STORES           = 31,
        PERF_PROFILE_TYPE_HW_CACHE_LLC_STORE_MISSES     = 32,
        PERF_PROFILE_TYPE_HW_CACHE_LLC_PREFETCHES       = 33,
        PERF_PROFILE_TYPE_HW_CACHE_LLC_PREFETCH_MISSES  = 34,

        PERF_PROFILE_TYPE_HW_CACHE_DTLB_LOADS           = 35,
        PERF_PROFILE_TYPE_HW_CACHE_DTLB_LOAD_MISSES     = 36,
        PERF_PROFILE_TYPE_HW_CACHE_DTLB_STORES          = 37,
        PERF_PROFILE_TYPE_HW_CACHE_DTLB_STORE_MISSES    = 38,
        PERF_PROFILE_TYPE_HW_CACHE_DTLB_PREFETCHES      = 39,
        PERF_PROFILE_TYPE_HW_CACHE_DTLB_PREFETCH_MISSES = 40,

        PERF_PROFILE_TYPE_HW_CACHE_ITLB_LOADS           = 41,
        PERF_PROFILE_TYPE_HW_CACHE_ITLB_LOAD_MISSES     = 42,

        PERF_PROFILE_TYPE_HW_CACHE_BRANCH_LOADS         = 43,
        PERF_PROFILE_TYPE_HW_CACHE_BRANCH_LOAD_MISSES   = 44,

        PERF_PROFILE_TYPE_IBS_FETCH                     = 45,
        PERF_PROFILE_TYPE_IBS_OP                        = 46,

        // Should be the last entry
        PERF_PROFILE_TYPE_MAX                           = 47

                                                          // Misc
                                                          // PERF_PROFILE_TYPE_OPROFILE_RAW = 47,
                                                          // PERF_PROFILE_TYPE_WINDOWS_RAW  = 48,
                                                          // PERF_PROFILE_TYPE_OPENCL       = 49,
    };

    //
    // flags that controls whether PMU is enabled or disabled during -
    // kernel mode execution, hypervisor mode execution, idle etc.
    // This is knid of measurement flags
    //
    enum PmuFlags
    {
        PERF_PMU_FLAG_ALWAYS_ON_PMU         = 1U << 0,
        PERF_PMU_FLAG_EXCLUSIVE_PMU         = 1U << 1,
        PERF_PMU_FLAG_EXCLUDE_USER          = 1U << 2,
        PERF_PMU_FLAG_EXCLUDE_KERNEL        = 1U << 3,
        PERF_PMU_FLAG_EXCLUDE_IDLE          = 1U << 4,
        PERF_PMU_FLAG_EXCLUDE_HYPERVISOR    = 1U << 5,
        PERF_PMU_FLAG_INCLUDE_MMAP_DATA     = 1U << 6, // This is not PMU stuff ?
        PERF_PMU_FLAG_INCLUDE_COMM_DATA     = 1U << 7, // This is not PMU stuff ?
        PERF_PMU_FLAG_IBS_RAND_INST_FETCH_TAGGING = 1U << 8, // IBS fetch-ctr randomized
        PERF_PMU_FLAG_IBS_COUNT_DISPATCHED_OPS    = 1U << 9, // IBS-OP, dispatch-count
        PERF_PMU_FLAG_IBS_COUNT_CLOCK_CYCLES      = 1U << 10, // IBS-OP, cycle-count

        PERF_PMU_FLAGS_MAX       = 1U << 11
    };

    //
    // Task flags to set whether
    //  - the children should inherit profile settings
    //  - to enable the profiling on exec
    //  - to trace forks and execs
    //
    enum TaskFlags
    {
        PERF_TASK_FLAG_INHERIT         = 1U << 0,  // children inherit it
        PERF_TASK_FLAG_INHERIT_STAT    = 1U << 1,  // per task counts ??
        PERF_TASK_FLAG_ENABLE_ON_EXEC  = 1U << 2,  // next exec enables
        PERF_TASK_FLAG_TRACE_TASK      = 1U << 3,  // trace fork and exec

        PERF_TASK_FLAGS_MAX  = 1U << 4
    };

    //
    // Flags to control the SKID during sample data collection
    //
    enum SampleSkidFactor
    {
        PERF_SAMPLE_ARBITRARY_SKID    = 0,
        PERF_SAMPLE_CONSTANT_SKID     = 1,
        PERF_SAMPLE_REQUEST_NO_SKID   = 2,
        PERF_SAMPLE_MUST_NO_SKID      = 3,

        PERF_SAMPLE_SKID_MAX          = 4
    };

private:
    uint32_t  m_type;     // HW/SW/TRACE..
    uint64_t  m_config;   // type specific configuration
    // in case of raw event, evensel+unitmask.
    // m_eventId & m_unitMask are redundant ?
    uint32_t  m_eventId;
    uint32_t  m_unitMask;
    char*     m_pEventName;
    char*     m_pEventAlias;

    // Event Flags include.. should i use flags are separate bool fields..
    uint32_t  m_pmuFlags; // should this be msmt flags ?
    uint32_t  m_taskFlags;

    uint8_t   m_skidFactor;

    // Event counter to be used to program the PMU Event
    // PERF does NoT need this info
    uint32_t  m_eventCounter;
    bool      m_initialized;

    void init(uint32_t type, uint32_t eventid, uint32_t unitmask, uint32_t pmuFlags, uint32_t taskFlags);

    void copy(const PerfEvent& evt);

    bool validateEvent() const;

    bool validatePmuFlags() const
    {
        return m_pmuFlags >= PERF_PMU_FLAG_ALWAYS_ON_PMU && m_pmuFlags <= PERF_PMU_FLAG_INCLUDE_COMM_DATA;
    }

    bool validateTaskFlags() const
    {
        return m_taskFlags >= PERF_TASK_FLAG_INHERIT && m_taskFlags <= PERF_TASK_FLAGS_MAX;
    }

    bool validateSkid() const
    {
        return /*m_skidFactor >= PERF_SAMPLE_ARBITRARY_SKID &&*/ m_skidFactor <= PERF_SAMPLE_SKID_MAX;
    }
};


// Maximum number of counter values - 4
//   - counter value
//   - time enabled; Only valid for PERF
//   - time running; Only valid for PERF
//   - sample id; Only valid for PERF in sampling mode
//
#define PERF_MAX_NBR_VALUES    4

struct PerfEventCountData
{
    uint32_t m_type;
    uint64_t m_config;
    int      m_pid;
    int      m_cpu;
    int      m_fd;
    uint64_t m_sampleId;
    bool     m_hasValidCountData;
    int      m_nbrValues;
    uint64_t m_values[PERF_MAX_NBR_VALUES];

    PerfEventCountData();

    void print();
};

#endif // _PERFEVENT_H_
