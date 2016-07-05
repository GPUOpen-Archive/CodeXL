//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PowerProfilerMidTierUtil.h
///
//==================================================================================

#ifndef PowerProfilerMidTierUtil_H_
#define PowerProfilerMidTierUtil_H_


// Local.
#include <AMDTPowerProfilingMidTier/include/PowerProfilerDefs.h>

/// Util class for Power Profiler Mid Tier. All functions are static.
class PowerProfilerMidTierUtil
{
public:
    static bool isNonFailureResult(PPResult result);
    static const gtString& CodeDescription(PPResult result);
};

#endif // PowerProfilerMidTierUtil_H_