//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csDevicesMonitor.cpp
///
//==================================================================================

//------------------------------ csDevicesMonitor.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionId.h>
#include <AMDTServerUtilities/Include/suInterceptionMacros.h>

// Local:
#include <src/csDevicesMonitor.h>
#include <src/csGlobalVariables.h>
#include <src/csMonitoredFunctionPointers.h>
#include <src/csOpenCLHandleMonitor.h>
#include <src/csOpenCLMonitor.h>
#include <src/csStringConstants.h>


// ---------------------------------------------------------------------------
// Name:        csDevicesMonitor::csDevicesMonitor
// Description: Constructor.
// Arguments:
// Author:      Sigal Algranaty
// Date:        29/11/2009
// ---------------------------------------------------------------------------
csDevicesMonitor::csDevicesMonitor(): _isInitialized(false)
{
    m_amdRuntimeVersion[0] = -1;
    m_amdRuntimeVersion[1] = -1;
}


// ---------------------------------------------------------------------------
// Name:        csDevicesMonitor::~csDevicesMonitor
// Description: Destructor.
// Author:      Sigal Algranaty
// Date:        29/11/2009
// ---------------------------------------------------------------------------
csDevicesMonitor::~csDevicesMonitor()
{
}

// ---------------------------------------------------------------------------
// Name:        csDevicesMonitor::initialize
// Description: Initializes the devices
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        29/11/2009
// ---------------------------------------------------------------------------
bool csDevicesMonitor::initialize()
{
    // If we were not already initialized:
    if (!_isInitialized)
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetPlatformIDs);
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetDeviceIDs);
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetDeviceInfo);

        // Get OpenCL platforms count:
        cl_uint amountOfPlatforms;
        cl_uint clRetVal = cs_stat_realFunctionPointers.clGetPlatformIDs(0, NULL, &amountOfPlatforms);
        GT_IF_WITH_ASSERT(clRetVal == CL_SUCCESS)
        {
            if (amountOfPlatforms > 0)
            {
                // if there's a platform or more, make space for ID's:
                cl_platform_id* pPlatformIds = new cl_platform_id[amountOfPlatforms];
                ::memset(pPlatformIds, 0, amountOfPlatforms * sizeof(cl_platform_id));

                // Get the platform ids:
                clRetVal = cs_stat_realFunctionPointers.clGetPlatformIDs(amountOfPlatforms, pPlatformIds, NULL);
                GT_IF_WITH_ASSERT(clRetVal == CL_SUCCESS)
                {
                    gtASCIIString amdPlatformVersion;

                    for (cl_uint i = 0; i < amountOfPlatforms; ++i)
                    {
                        // Get the current platform id:
                        cl_platform_id currentPlatformId = pPlatformIds[i];

                        // Add this platform to the mapping:
                        _platformIdToPlatformAPIID[(oaCLPlatformID)currentPlatformId] = i;

                        // Get devices for this platform:
                        cl_uint amountOfDevices;
                        clRetVal = cs_stat_realFunctionPointers.clGetDeviceIDs(currentPlatformId, CL_DEVICE_TYPE_ALL, 0, NULL, &amountOfDevices);
                        GT_IF_WITH_ASSERT((clRetVal == CL_SUCCESS) && (amountOfDevices > 0))
                        {
                            _isInitialized = true;

                            // Allocate a memory for the current platform device ids:
                            cl_device_id* pDevicesIds = new cl_device_id[amountOfDevices];
                            ::memset(pDevicesIds, 0, amountOfDevices * sizeof(cl_device_id));

                            // Get the device IDs:
                            clRetVal = cs_stat_realFunctionPointers.clGetDeviceIDs(currentPlatformId, CL_DEVICE_TYPE_ALL, amountOfDevices, pDevicesIds, NULL);
                            GT_IF_WITH_ASSERT(clRetVal == CL_SUCCESS)
                            {
                                // Iterate the platforms devices:
                                for (int j = 0; j < (int) amountOfDevices; j++)
                                {
                                    // The device is "created" now:
                                    onDeviceCreated((oaCLDeviceID)pDevicesIds[j]);
                                }
                            }

                            // Delete the device ids vector:
                            delete[] pDevicesIds;
                        }

                        // Check if this is the AMD platform and if so, what is its version:
                        if (isAMDPlatform((oaCLPlatformID)currentPlatformId, &amdPlatformVersion))
                        {
                            gtString logMsg;
                            logMsg.fromASCIIString(amdPlatformVersion.asCharArray());

                            // There should be only one AMD platform, and it should have a version string:
                            GT_IF_WITH_ASSERT(!amdPlatformVersion.isEmpty() && (-1 == m_amdRuntimeVersion[0]))
                            {
                                // The string format is "OpenCL x.y AMD-APP (a.b)". We want the "a.b" substring:
                                int firstBracket = amdPlatformVersion.find('(');
                                int closingBracket = amdPlatformVersion.find(')', firstBracket);
                                GT_IF_WITH_ASSERT((-1 < firstBracket) && (firstBracket + 1 < closingBracket))
                                {
                                    // Truncate the string:
                                    amdPlatformVersion.truncate(firstBracket + 1, closingBracket - 1);
                                    int firstDot = amdPlatformVersion.find('.');
                                    int verLen = amdPlatformVersion.length();

                                    if (firstDot < 0)
                                    {
                                        // No minor version:
                                        firstDot = verLen;
                                        m_amdRuntimeVersion[1] = 0;
                                    }
                                    else
                                    {
                                        // Get minor version:
                                        gtASCIIString amdPlatformMinorVersion;
                                        amdPlatformVersion.getSubString(firstDot + 1, verLen - 1, amdPlatformMinorVersion);

                                        bool rcMin = amdPlatformMinorVersion.toIntNumber(m_amdRuntimeVersion[1]);
                                        GT_ASSERT(rcMin)
                                    }

                                    // Get major version:
                                    gtASCIIString amdPlatformMajorVersion;
                                    amdPlatformVersion.getSubString(0, firstDot - 1, amdPlatformMajorVersion);

                                    bool rcMaj = amdPlatformMajorVersion.toIntNumber(m_amdRuntimeVersion[0]);
                                    GT_ASSERT(rcMaj);
                                }
                            }

                            // Print the result to the log:
                            logMsg.prependFormattedString(CS_STR_DebugLog_detectedAMDOpenCLPlatformVersion, m_amdRuntimeVersion[0], m_amdRuntimeVersion[1]);
                            OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
                        }
                    }
                }
                // Delete the platform ids vector:
                delete[] pPlatformIds;
            }
        }

        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetDeviceInfo);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetDeviceIDs);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetPlatformIDs);
    }

    return _isInitialized;
}


// ---------------------------------------------------------------------------
// Name:        csDevicesMonitor::getDeviceObjectDetailsByIndex
// Description: Inputs an OpenCL device API ID and returns a class that represents it.
// Author:      Yaki Tebeka
// Date:        21/1/2010
// ---------------------------------------------------------------------------
const apCLDevice* csDevicesMonitor::getDeviceObjectDetailsByIndex(int deviceAPIID) const
{
    const apCLDevice* retVal = NULL;

    // ID range test:
    int devicesAmount = int(_devices.size());
    GT_IF_WITH_ASSERT((0 <= deviceAPIID) && (deviceAPIID < devicesAmount))
    {
        retVal = _devices[deviceAPIID];
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csDevicesMonitor::getDeviceObjectDetails
// Description: Gets and OpenCL device by its cl_device_id handle.
// Author:      Uri Shomroni
// Date:        21/1/2010
// ---------------------------------------------------------------------------
const apCLDevice* csDevicesMonitor::getDeviceObjectDetails(oaCLDeviceID deviceHandle) const
{
    const apCLDevice* retVal = NULL;

    // If this class was not initialized:
    if (!_isInitialized)
    {
        // Initialize it:
        ((csDevicesMonitor*)this)->initialize();
    }

    // Sanity check:
    GT_IF_WITH_ASSERT(_isInitialized)
    {
        // Look for the input device id in the available devices list:
        // (We assume very small amount of devices, therefore using a simple for loop)
        int numberOfDevices = (int)_devices.size();

        for (int i = 0; i < numberOfDevices; i++)
        {
            const apCLDevice* pCurrentDevice = _devices[i];

            if (pCurrentDevice != NULL)
            {
                if (pCurrentDevice->deviceHandle() == deviceHandle)
                {
                    retVal = pCurrentDevice;
                    break;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csDevicesMonitor::getDeviceObjectAPIID
// Description: Inputs an OpenCL device handle and returns it's API ID,
//              or -1 if it does not exist.
// Author:      Sigal Algranaty
// Date:        29/11/2009
// ---------------------------------------------------------------------------
int csDevicesMonitor::getDeviceObjectAPIID(oaCLDeviceID deviceHandle) const
{
    int retVal = -1;

    // If this class was not initialized:
    if (!_isInitialized)
    {
        // Initialize it:
        ((csDevicesMonitor*)this)->initialize();
    }

    // Sanity check:
    GT_IF_WITH_ASSERT(_isInitialized)
    {
        // Look for the input device id in the available devices list:
        // (We assume very small amount of devices, therefore using a simple for loop)
        int numberOfDevices = (int)_devices.size();

        for (int i = 0; i < numberOfDevices; i++)
        {
            const apCLDevice* pCurrentDevice = _devices[i];

            if (pCurrentDevice != NULL)
            {
                if (pCurrentDevice->deviceHandle() == deviceHandle)
                {
                    retVal = i;
                    break;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csDevicesMonitor::isAMDPlatform
// Description: Returns true iff the device ID is valid and points to the AMD platform.
// Author:      Uri Shomroni
// Date:        29/7/2015
// ---------------------------------------------------------------------------
bool csDevicesMonitor::isAMDDevice(oaCLDeviceID deviceHandle) const
{
    bool retVal = false;

    if (OA_CL_NULL_HANDLE != deviceHandle)
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetDeviceInfo);
        cl_platform_id hPlatform = nullptr;
        cl_int rc = cs_stat_realFunctionPointers.clGetDeviceInfo((cl_device_id)deviceHandle, CL_DEVICE_PLATFORM, sizeof(cl_platform_id), &hPlatform, nullptr);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetDeviceInfo);
        GT_IF_WITH_ASSERT((CL_SUCCESS == rc) && (nullptr != hPlatform))
        {
            retVal = isAMDPlatform((oaCLPlatformID)hPlatform);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csDevicesMonitor::getPlatformAPIID
// Description: Return an OpenCL platform API ID, by the platform OpenCL ID
// Arguments:   int platformId
//            int &platformAPIID
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        11/4/2010
// ---------------------------------------------------------------------------
bool csDevicesMonitor::getPlatformAPIID(oaCLPlatformID platformId, int& platformAPIID)
{
    bool retVal = false;

    // If this class was not initialized:
    if (!_isInitialized)
    {
        // Initialize it:
        ((csDevicesMonitor*)this)->initialize();
    }


    // Sanity check:
    GT_IF_WITH_ASSERT(_isInitialized)
    {
        // Find the platform id in the map:
        gtMap <oaCLPlatformID, int>::const_iterator iter = _platformIdToPlatformAPIID.find(platformId);

        if (iter != _platformIdToPlatformAPIID.end())
        {
            platformAPIID = (*iter).second;
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csDevicesMonitor::isAMDPlatform
// Description: Returns true iff the platform ID is valid and points to the AMD platform.
// Author:      Uri Shomroni
// Date:        29/7/2015
// ---------------------------------------------------------------------------
bool csDevicesMonitor::isAMDPlatform(oaCLPlatformID platform, gtASCIIString* pPlatformVer) const
{
    bool retVal = false;

    // The null platform handle is not valid:
    if (OA_CL_NULL_HANDLE != platform)
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_clGetPlatformInfo);
        cl_platform_id hPlatform = (cl_platform_id)platform;

        // Get the platform vendor name:
        size_t vendorNameLength = 0;
        cl_int rc = cs_stat_realFunctionPointers.clGetPlatformInfo(hPlatform, CL_PLATFORM_VENDOR, 0, nullptr, &vendorNameLength);
        GT_IF_WITH_ASSERT((CL_SUCCESS == rc) && (0 < vendorNameLength))
        {
            char* vendorName = new char[vendorNameLength + 1];
            rc = cs_stat_realFunctionPointers.clGetPlatformInfo(hPlatform, CL_PLATFORM_VENDOR, vendorNameLength, vendorName, nullptr);
            GT_IF_WITH_ASSERT(CL_SUCCESS == rc)
            {
                vendorName[vendorNameLength] = '\0';
                gtASCIIString vendorNameStr = vendorName;
                retVal = ("Advanced Micro Devices, Inc." == vendorNameStr) || ("AuthenticAMD" == vendorNameStr);
            }

            delete[] vendorName;
        }

        // If requested, get the AMD platform version:
        if (retVal && (nullptr != pPlatformVer))
        {
            size_t platformVersionLength = 0;
            rc = cs_stat_realFunctionPointers.clGetPlatformInfo(hPlatform, CL_PLATFORM_VERSION, 0, nullptr, &platformVersionLength);
            GT_IF_WITH_ASSERT((CL_SUCCESS == rc) && (0 < platformVersionLength))
            {
                pPlatformVer->resize(platformVersionLength + 1);
                rc = cs_stat_realFunctionPointers.clGetPlatformInfo(hPlatform, CL_PLATFORM_VERSION, platformVersionLength, &((*pPlatformVer)[0]), nullptr);
                GT_ASSERT(CL_SUCCESS == rc);
            }
        }

        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_clGetPlatformInfo);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csDevicesMonitor::amdRuntimeVersion
// Description: Returns the AMD runtime version, if present, or -1 otherwise.
// Author:      Uri Shomroni
// Date:        10/4/2016
// ---------------------------------------------------------------------------
int csDevicesMonitor::amdRuntimeVersion(int* pMinor) const
{
    // If this class was not initialized:
    if (!_isInitialized)
    {
        // Initialize it:
        ((csDevicesMonitor*)this)->initialize();
    }

    if (nullptr != pMinor)
    {
        *pMinor = m_amdRuntimeVersion[1];
    }

    return m_amdRuntimeVersion[0];
}


// ---------------------------------------------------------------------------
// Name:        csDevicesMonitor::onDeviceCreated
// Description: Called when a device is first seen by our server
// Author:      Uri Shomroni
// Date:        27/8/2013
// ---------------------------------------------------------------------------
void csDevicesMonitor::onDeviceCreated(oaCLDeviceID deviceHandle)
{
    csOpenCLHandleMonitor& handlesMonitor = cs_stat_openCLMonitorInstance.openCLHandleMonitor();

    bool createDevice = true;
    bool newIndex = false;
    gtInt32 deviceAPIID = getDeviceObjectAPIID(deviceHandle);

    if (-1 == deviceAPIID)
    {
        deviceAPIID = (gtInt32)_devices.size();
        newIndex = true;
    }
    else // -1 != deviceAPIID
    {
        // This is a device we've seen before, make sure it's not a handle that's being re-used:
        apCLDevice*& prOldDevice = _devices[deviceAPIID];

        if (NULL != prOldDevice)
        {
            if (prOldDevice->wasMarkedForDeletion())
            {
                // The device ID was re-used, we can destroy the old device object:
                delete prOldDevice;
                prOldDevice = NULL;

                // Remove the deleted device from the handles monitor:
                handlesMonitor.registerOpenCLHandle((oaCLHandle)deviceHandle, 0, -1, OS_TOBJ_ID_CL_DEVICE);
            }
        }

        // If the device was NULL or was deleted:
        if (NULL == prOldDevice)
        {
            // This is a new device, but it can have the old ID:
            createDevice = true;
        }
        else // NULL != prOldDevice
        {
            // This is just a second call for this device, as clCreateSubDevices might re "create" devices if the same
            // paritition is used:
            createDevice = false;
        }

        // This is a pre-existing index anyway:
        newIndex = false;
    }

    // If this is not an already living device:
    if (createDevice)
    {
        // Create a data object for the current device:
        apCLDevice* pNewDevice = new apCLDevice(deviceAPIID, deviceHandle);

        // Log the device's details:
        pNewDevice->initialize(cs_stat_realFunctionPointers.clGetDeviceInfo, true);

        if (newIndex)
        {
            _devices.push_back(pNewDevice);
        }
        else // !newIndex
        {
            _devices[deviceAPIID] = pNewDevice;
        }

        // Register the device:
        handlesMonitor.registerOpenCLHandle((oaCLHandle)deviceHandle, 0, (int)deviceAPIID, OS_TOBJ_ID_CL_DEVICE);
    }
}

// ---------------------------------------------------------------------------
// Name:        csDevicesMonitor::onDeviceMarkedForDeletion
// Description: Mark a device for deletion
// Arguments:   oaCLDeviceID deviceHandle
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/1/2012
// ---------------------------------------------------------------------------
bool csDevicesMonitor::onDeviceMarkedForDeletion(oaCLDeviceID deviceHandle)
{
    bool retVal = false;

    // Get the device index:
    apCLDevice* pDevice = NULL;
    int deviceIndex = getDeviceObjectAPIID((oaCLDeviceID)deviceHandle);

    if ((deviceIndex >= 0) && (deviceIndex < (int)_devices.size()))
    {
        // Get the device object:
        pDevice = _devices[deviceIndex];

        if (pDevice != NULL)
        {
            // Mark the device for deletion:
            pDevice->onDeviceMarkedForDeletion();
            retVal = true;
        }
    }

    return retVal;
}

