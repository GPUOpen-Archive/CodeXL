//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCounterType.h
///
//==================================================================================

//------------------------------ apCounterType.h ------------------------------

#ifndef __APCOUNTERTYPE
#define __APCOUNTERTYPE

// Defines the counter type:
enum apCounterType
{
    // Counter supported by the native OS:
    AP_OS_NATIVE_COUNTER,

    // Counter supported by the native OS, and added on runtime:
    AP_OS_NATIVE_ADDITIONAL_COUNTER,

    // Counter supported by the iPhone OS:
    AP_IPHONE_OS_COUNTER,

    // Counters supported by CodeXL OpenGL implementation:
    AP_CodeXL_COUNTER,

    // Counter supported by ATI OpenGL profiling metrics:
    AP_ATI_OPENGL_COUNTER,

    // Counter supported by ATI OpenCL profiling metrics:
    AP_ATI_STREAM_OPENCL_COUNTER,

    // Graphic driver and GPU counters supported by Mac OS:
    AP_MAC_OPENGL_DRIVER_COUNTER,

    // Graphic driver and GPU counters supported by Mac OS:
    AP_IPHONE_GPU_COUNTER,

    // OpenCL Command Queue performance counters:
    AP_CodeXL_CL_QUEUE_COUNTER,

    // The amount of counter types:
    AP_COUNTER_TYPES_AMOUNT,

    // Unknown counter type
    AP_COUNTER_TYPE_UNKNOWN
};


#endif  // __APCOUNTERTYPE
