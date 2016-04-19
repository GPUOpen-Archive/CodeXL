//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PPDevice.h
///
//==================================================================================

#ifndef _PPDEVICE_H_
#define _PPDEVICE_H_

// Local.
#include <AMDTPowerProfilingMidTier/include/AMDTPowerProfilingMidTier.h>

// Infra.
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTCommonHeaders/AMDTDefinitions.h>

// Backend.
#include <AMDTPowerProfileAPI/inc/AMDTPowerProfileDataTypes.h>

struct AMDTPOWERPROFILINGMIDTIER_API PPDevice
{
    // Default CTOR.
    PPDevice() {}

    ~PPDevice();

    PPDevice(int deviceId,
             AMDTDeviceType deviceType,
             const gtString& deviceName,
             const gtString& deviceDescription,
             const gtList<PPDevice*>& subDevices,
             const gtList<AMDTPwrCounterDesc*>& supportedCounters);

    // Copy CTOR.
    PPDevice(const PPDevice& other);

    // Device id.
    int m_deviceId;

    // Device type.
    AMDTDeviceType m_deviceType;

    // Device name.
    gtString m_deviceName;

    // Device description.
    gtString m_deviceDescription;

    // Device's sub-devices.
    gtList<PPDevice*> m_subDevices;

    // A list of counters which are supported by this device.
    gtList<AMDTPwrCounterDesc*> m_supportedCounters;
};

#endif // _PPDEVICE_H_