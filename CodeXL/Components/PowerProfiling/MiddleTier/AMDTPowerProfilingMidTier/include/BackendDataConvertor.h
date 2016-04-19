//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file BackendDataConvertor.h
///
//==================================================================================

#ifndef BackendDataConvertor_h__
#define BackendDataConvertor_h__

// Local.
#include <AMDTPowerProfilingMidTier/include/AMDTPowerProfilingMidTier.h>
#include <AMDTPowerProfilingMidTier/include/IPowerProfilerBackendAdapter.h>

// For the middle tier data types.
#include <AMDTPowerProfilingMidTier/include/PowerProfilerDefs.h>

class AMDTPOWERPROFILINGMIDTIER_API BackendDataConvertor
{
public:

    // Creating a PPDevice while accessing the Power Profiling API directly.
    // This function is currently used by the CLI.
    static PPDevice* CreatePPDevice(const AMDTPwrDevice& bePwrDevice);

    // Creating a PPDevice while accessing the Power Profiling API via a Backend Adapter.
    // This function is used by front-end components (CodeXL).
    static PPDevice* CreatePPDevice(const AMDTPwrDevice& bePwrDevice, IPowerProfilerBackendAdapter* pBeAdapter);

    static bool AMDTPwrSampleToPPSample(const AMDTPwrSample* pSample, AMDTProfileTimelineSample& buffer);

    // Returns true if this counter needs to be forwarded to the front-end.
    // Otherwise, returns false.
    // This function is used to filter out unwanted counters.
    static bool IsCounterRequired(const AMDTPwrCounterDesc& pCounter);

    // Allocates an AMDTPwrCounterDesc structure and returns a pointer to the allocated structure
    // after safely copying the data from the given AMDTPwrCounterDesc parameter. Since AMDTPwrCounterDesc
    // is implemented as a "C" API, we need to provide a copy-CTOR behavior.
    static AMDTPwrCounterDesc* CopyPwrCounterDesc(const AMDTPwrCounterDesc& other);

    static AMDTPwrCounterDesc* ConvertToPwrCounterDesc(const AMDTProfileCounterDesc& counterDesc);
    static AMDTProfileCounterDesc* ConvertToProfileCounterDesc(const AMDTPwrCounterDesc& counterDesc);

    static bool ConvertToProfileCounterDescVec(gtList<AMDTPwrCounterDesc*>& pwrCounterDesc,
                                               gtVector<AMDTProfileCounterDesc*>& counterDesc);

    static bool ConvertToProfileDevice(const AMDTPwrDevice& bePwrDevice, gtVector<AMDTProfileDevice*>& profileDevice);
    static AMDTProfileDevice* CreateProfileDevice(const AMDTPwrDevice& bePwrDevice);

    static AMDTProfileDevice* CreateProfileDevice(const PPDevice& ppDevice);
    static bool ConvertToProfileDevice(const gtList<PPDevice*>& ppDeviceList, gtVector<AMDTProfileDevice*>& profileDevice);

    static bool AMDTDeviceTypeToString(AMDTDeviceType deviceType, gtString& deviceTypeStr);
    static bool AMDTPwrUnitToString(AMDTPwrUnit unit, gtString& unitAsStr);
    static bool AMDTPwrAggregationToString(AMDTPwrAggregation aggregationType, gtString& aggregationAsStr);
    static bool AMDTPwrCategoryToString(AMDTPwrCategory category, gtString& categoryAsStr);

    static bool GetPwrDeviceIdStringMap(gtMap<gtString, int>& deviceIdStrMap);
    static bool GetPwrAggregationIdStringMap(gtMap<gtString, int>& aggregationIdStrMap);
    static bool GetPwrUnitIdStringMap(gtMap<gtString, int>& unitIdStrMap);
    static bool GetPwrCategoryIdStringMap(gtMap<gtString, int>& categoryIdStrMap);
};

#endif // BackendDataConvertor_h__
