//==================================================================================
// Copyright (c) 2014-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileDefinitions.h
///
//==================================================================================

#ifndef _CPUPROFILEDEFINITIONS_H_
#define _CPUPROFILEDEFINITIONS_H_

enum CpuProfileCssScope
{
    CP_CSS_SCOPE_UNKNOWN,
    CP_CSS_SCOPE_USER,
    CP_CSS_SCOPE_KERNEL,
    CP_CSS_SCOPE_ALL
};

#define CP_CSS_MIN_UNWIND_INTERVAL        1
#define CP_CSS_MAX_UNWIND_INTERVAL        100
#define CP_CSS_DEFAULT_UNWIND_INTERVAL    1

#define CP_CSS_MIN_UNWIND_DEPTH           2
#define CP_CSS_LOW_UNWIND_DEPTH           32
#define CP_CSS_MEDIUM_UNWIND_DEPTH        64
#define CP_CSS_HIGH_UNWIND_DEPTH          128
#define CP_CSS_MAX_UNWIND_DEPTH           392

#define CP_CSS_DEFAULT_UNWIND_DEPTH       128


#endif //_CPUPROFILEDEFINITIONS_H_
