//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csDevicesMonitor.h
///
//==================================================================================

//------------------------------ csDevicesMonitor.h ------------------------------

#ifndef __CSDEVICESMONITOR_H
#define __CSDEVICESMONITOR_H

// Forward decelerations:
class gtASCIIString;
class osFilePath;

// Infra:
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>
#include <AMDTOSAPIWrappers/Include/oaOpenCLIncludes.h>
#include <AMDTAPIClasses/Include/apCLDevice.h>


// ----------------------------------------------------------------------------------
// Class Name:           csDevicesMonitor
// General Description: Monitors OpenCL devices.
// Author:               Sigal Algranaty
// Creation Date:        29/11/2009
// ----------------------------------------------------------------------------------
class csDevicesMonitor
{
public:
    csDevicesMonitor();
    virtual ~csDevicesMonitor();

    // Device object details:
    const apCLDevice* getDeviceObjectDetailsByIndex(int deviceAPIID) const;
    const apCLDevice* getDeviceObjectDetails(oaCLDeviceID deviceHandle) const;
    void onDeviceCreated(oaCLDeviceID deviceHandle);
    bool onDeviceMarkedForDeletion(oaCLDeviceID deviceHandle);
    int getDeviceObjectAPIID(oaCLDeviceID deviceHandle) const;
    bool isAMDDevice(oaCLDeviceID deviceHandle) const;
    int amountOfDevices() { return (int)_devices.size(); };

    // Platform object details:
    bool getPlatformAPIID(oaCLPlatformID platformId, int& platformAPIID);
    bool isAMDPlatform(oaCLPlatformID platform, gtASCIIString* pPlatformVer = nullptr) const;
    int amdRuntimeVersion(int* pMinor = nullptr) const;

private:
    // Do not allow use of the = operator for this class. Use reference or pointer transferral instead
    csDevicesMonitor& operator=(const csDevicesMonitor& otherMonitor);
    csDevicesMonitor(const csDevicesMonitor& otherMonitor);

    bool initialize();

private:
    // True iff the devices information is already updated:
    bool _isInitialized;

    // A vector containing devices objects:
    gtPtrVector<apCLDevice*> _devices;

    // Contain a mapping from platform OpenCL id, to platform APIId:
    gtMap <oaCLPlatformID, int> _platformIdToPlatformAPIID;

    // Holds the AMD OpenCL runtime version (parsed from the platform version string)
    int m_amdRuntimeVersion[2];
};


#endif //__CSDEVICESMONITOR_H

