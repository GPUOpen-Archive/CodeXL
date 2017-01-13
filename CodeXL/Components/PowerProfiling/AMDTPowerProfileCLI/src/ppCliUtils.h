//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppCliUtils.h
///
//==================================================================================

#ifndef _PPCLIUTILS_H_
#define _PPCLIUTILS_H_
#pragma once

// Project:
#include <PowerProfileCLI.h>
#include <AMDTCommonHeaders/AMDTCommonProfileDataTypes.h>

class ppCliUtils
{
public:
    static bool GetCategoryString(AMDTPwrCategory category, gtString& categoryStr);

    static bool GetUnitString(AMDTPwrUnit unitType, gtString& unitStr);

    static bool GetAggregationString(AMDTPwrAggregation aggregationTypes, gtString& aggregationtStr);

    static bool GetDeviceTypeString(AMDTDeviceType deviceType, gtString& deviceTypeStr);

    // TODO: try using reference instead of pointer
    static AMDTProfileDevice* ConvertToProfileDevice(AMDTPwrDevice& counterDesc);

    static AMDTProfileCounterDesc* ConvertToProfileCounterDesc(AMDTPwrCounterDesc& counterDesc);

    static AMDTProfileTimelineSample* ConvertPwrSampleToTimelineSample(AMDTPwrSample& aBeSample);
};

#endif // #ifndef _PPCLIUTILS_H_