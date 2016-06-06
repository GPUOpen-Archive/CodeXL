//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apOpenCLQueuePerformanceCounters.h
///
//==================================================================================

//------------------------------ apOpenCLQueuePerformanceCounters.h ------------------------------

#ifndef __APOPENCLQUEUEPERFORMANCECOUNTERS_H
#define __APOPENCLQUEUEPERFORMANCECOUNTERS_H


// Enumerates the available OpenCL Command Queue performance counters:
enum apOpenCLQueuePerformanceCounters
{
    AP_QUEUE_FPS_COUNTER,                               // Frames/Sec
    AP_KERNEL_COMMANDS_QUEUE_UTILIZATION_COUNTER,       // Percent of time the queue was executing Kernel commands.
    AP_WRITE_COMMANDS_QUEUE_UTILIZATION_COUNTER,        // Percent of time the queue was executing Write commands.
    AP_COPY_COMMANDS_QUEUE_UTILIZATION_COUNTER,         // Percent of time the queue was executing Copy commands.
    AP_READ_COMMANDS_QUEUE_UTILIZATION_COUNTER,         // Percent of time the queue was executing Read commands.
    AP_OTHER_COMMANDS_QUEUE_UTILIZATION_COUNTER,        // Percent of time the queue was executing other type of commands.
    AP_QUEUE_BUSY_COUNTER,                              // Percent of time the queue was executing commands (sum of all of the above)
    AP_QUEUE_IDLE_COUNTER,                              // Percent of time the queue was idle.

    AP_QUEUE_WORK_ITEM_SIZE_PER_SECOND,                 // Work item size per second
    AP_QUEUE_READ_BYTES_PER_SECOND,                     // Buffer / Image data read in bytes per second.
    AP_QUEUE_WRITE_BYTES_PER_SECOND,                    // Buffer / Image data write in bytes per second.
    AP_QUEUE_COPY_BYTES_PER_SECOND,                     // Buffer / Image data copy in bytes per second.

    AP_COMMAND_QUEUE_COUNTERS_AMOUNT                    // The amount of command queue performance counters.
};


#endif //__APOPENCLQUEUEPERFORMANCECOUNTERS_H

