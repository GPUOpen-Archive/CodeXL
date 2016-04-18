//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PerfRecName.c
///
//==================================================================================

#include <linux/perf_event.h>

// NOTE: This comes from the linux/perf/util/event.c
const char* _ca_perfRecNames[] =
{
    [0]                                     = "TOTAL",
    [PERF_RECORD_MMAP]                      = "MMAP",
    [PERF_RECORD_LOST]                      = "LOST",
    [PERF_RECORD_COMM]                      = "COMM",
    [PERF_RECORD_EXIT]                      = "EXIT",
    [PERF_RECORD_THROTTLE]                  = "THROTTLE",
    [PERF_RECORD_UNTHROTTLE]                = "UNTHROTTLE",
    [PERF_RECORD_FORK]                      = "FORK",
    [PERF_RECORD_READ]                      = "READ",
    [PERF_RECORD_SAMPLE]                    = "SAMPLE",
    //  [PERF_RECORD_HEADER_ATTR]               = "ATTR",
    //  [PERF_RECORD_HEADER_EVENT_TYPE]         = "EVENT_TYPE",
    //  [PERF_RECORD_HEADER_TRACING_DATA]       = "TRACING_DATA",
    //  [PERF_RECORD_HEADER_BUILD_ID]           = "BUILD_ID",
    //  [PERF_RECORD_FINISHED_ROUND]            = "FINISHED_ROUND",
};
