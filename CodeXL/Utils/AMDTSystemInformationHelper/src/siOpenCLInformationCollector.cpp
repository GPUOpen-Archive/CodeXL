//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file siOpenCLInformationCollector.cpp
///
//==================================================================================

//------------------------------ siCollectOpenCLPlatformsInformation.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTOSWrappers/Include/osMachine.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osCpuid.h>
#include <AMDTOSWrappers/Include/osPipeSocketClient.h>
#include <AMDTOSAPIWrappers/Include/oaDriver.h>
#include <AMDTOSAPIWrappers/Include/oaHiddenWindow.h>
#include <AMDTOSAPIWrappers/Include/oaDeviceContext.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLRenderContext.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include "../inc/siOpenCLInformationCollector.h"

// standard C
#include <string.h>
#include <stdio.h>

// ---------------------------------------------------------------------------
siOpenCLInformationCollector::siOpenCLInformationCollector()
    : hOpenCLModule(0), pclGetPlatformIDs(NULL), pclGetPlatformInfo(NULL), pclGetDeviceIDs(NULL), pclGetDeviceInfo(NULL)
{
}

// ---------------------------------------------------------------------------
siOpenCLInformationCollector::~siOpenCLInformationCollector()
{
}


// ---------------------------------------------------------------------------
bool siOpenCLInformationCollector::CollectOpenCLSinglePlatformInformation(int platformID, oaCLPlatformID platformHandle, gtList< gtList <gtString> >& infoData)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(pclGetPlatformInfo != NULL)
    {
        gtList< gtList <gtString> >::iterator iter = infoData.begin();
        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& titleLinesList = (*iter);

            // Push the platform id:
            gtString currPlatformID = AF_STR_Platform;
            currPlatformID.append(AF_STR_Space);
            currPlatformID.appendFormattedString(AF_STR_IntFormat, platformID + 1);
            titleLinesList.push_back(currPlatformID);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            // Get the platform's vendor:
            gtString platformVendor;
            bool rcVendor = PlatformParamAsString(platformHandle, CL_PLATFORM_VENDOR, platformVendor);
            GT_ASSERT(rcVendor);

            gtList<gtString>& vendorsList = (*iter);
            vendorsList.push_back(platformVendor);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            // Get the platform's name:
            gtString platformName;
            bool rcName = PlatformParamAsString(platformHandle, CL_PLATFORM_NAME, platformName);
            GT_ASSERT(rcName);

            gtList<gtString>& namessLinesList = (*iter);
            namessLinesList.push_back(platformName);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            // Get the platform's profile:
            gtString platformProfile;
            bool rcProfile = PlatformParamAsString(platformHandle, CL_PLATFORM_PROFILE, platformProfile);
            GT_ASSERT(rcProfile);

            gtList<gtString>& profilesLinesList = (*iter);
            profilesLinesList.push_back(platformProfile);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            // Get the platform's version:
            gtString platformVersion;
            bool rcVersion = PlatformParamAsString(platformHandle, CL_PLATFORM_VERSION, platformVersion);
            GT_ASSERT(rcVersion);

            gtList<gtString>& versionsLinesList = (*iter);
            versionsLinesList.push_back(platformVersion);
            iter++;
        }


        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
bool siOpenCLInformationCollector::CollectOpenCLPlatformsExtensions(gtList< gtList <gtString> >& infoData)
{
    bool retVal = false;

    // Get the platforms extensions list:
    gtVector< gtVector <gtString> > platformsExtensions;
    bool rcGetExt = GetOpenCLPlatformsExtensions(platformsExtensions);
    GT_IF_WITH_ASSERT(rcGetExt)
    {
        retVal = true;

        // Calculate the longest extensions list:
        size_t longestExtesnionsListSize = 0;
        int platformsAmount = (int)_platformIdToName.size();

        for (int k = 0; k < platformsAmount; k++)
        {
            size_t currExtensionListSize = platformsExtensions[k].size();

            if (longestExtesnionsListSize < currExtensionListSize)
            {
                longestExtesnionsListSize = currExtensionListSize;
            }
        }

        // Iterate the extensions lists by extensions order:
        for (unsigned int i = 0; i < longestExtesnionsListSize; i++)
        {
            // Will hold the current horizontal line of the OpenCL extensions:
            gtList <gtString> extensionsHorizontalLine;

            // Contains first column content:
            gtString firstColumnContent;

            if (i == 0)
            {
                firstColumnContent = AF_STR_SystemInformationCommandExtensions;
            }

            extensionsHorizontalLine.push_back(firstColumnContent);

            // Iterate the platforms:
            for (int j = 0; j < platformsAmount; j++)
            {
                // Will contain the current extension name:
                gtString currExtensionName;

                const gtVector <gtString>& currPlatformExtensions = platformsExtensions[j];

                if (i < currPlatformExtensions.size())
                {
                    currExtensionName = currPlatformExtensions[i];
                }

                extensionsHorizontalLine.push_back(currExtensionName);
            }

            infoData.push_back(extensionsHorizontalLine);
        }
    }


    return retVal;
}

// ---------------------------------------------------------------------------
bool siOpenCLInformationCollector::CollectOpenCLDevicesExtensions(gtList< gtList <gtString> >& infoData, const gtPtrVector<apCLDevice*>& devicesList)
{
    bool retVal = true;

    // Check what is the longest device extensions list:
    size_t longestExtesnionsListSize = 0;
    int devicesAmount = (int)devicesList.size();

    for (int k = 0; k < devicesAmount; k++)
    {
        if (devicesList[k] != NULL)
        {
            // Get the current device extensions list:
            size_t currExtensionListSize = devicesList[k]->extensions().size();

            if (longestExtesnionsListSize < currExtensionListSize)
            {
                longestExtesnionsListSize = currExtensionListSize;
            }
        }
    }

    // Iterate the extensions lists by extensions order:
    for (unsigned int i = 0; i < longestExtesnionsListSize; i++)
    {
        // Will hold the current horizontal line of the OpenCL extensions:
        gtList <gtString> extensionsHorizontalLine;

        // Contains first column content:
        gtString firstColumnContent;

        if (i == 0)
        {
            firstColumnContent = AF_STR_SystemInformationCommandExtensions;
        }

        extensionsHorizontalLine.push_back(firstColumnContent);

        // Iterate the devices:
        for (int j = 0; j < devicesAmount; j++)
        {
            // Will contain the current extension name:
            gtString currExtensionName;

            if (devicesList[j] != NULL)
            {
                // Get the current device extensions:
                const gtVector <gtString>& currDeviceExtensions = devicesList[j]->extensions();

                if (i < currDeviceExtensions.size())
                {
                    currExtensionName = currDeviceExtensions[i];
                }

                extensionsHorizontalLine.push_back(currExtensionName);
            }
        }

        infoData.push_back(extensionsHorizontalLine);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
bool siOpenCLInformationCollector::GetOpenCLPlatformsExtensions(gtVector< gtVector <gtString> >& platformsExtensions)
{
    bool retVal = true;

    gtMap <oaCLPlatformID, int>::const_iterator iter = _platformIdToName.begin();

    while (iter != _platformIdToName.end())
    {
        // Collect the platform information:
        int platformId = (*iter).second;
        GT_ASSERT(platformId > -1);

        oaCLPlatformID platformHandle = (*iter).first;

        // Will get the current platforms extensions list:
        gtVector <gtString> currPlatformExtensions;

        // Get the platform's extensions:
        gtString currPlatformExtensionsString;
        bool rcExtensions = PlatformParamAsString(platformHandle, CL_PLATFORM_EXTENSIONS, currPlatformExtensionsString);
        GT_IF_WITH_ASSERT(rcExtensions)
        {
            // Parse the extensions string, which are space separated:
            gtString currentExtension;
            gtStringTokenizer strTokenizer(currPlatformExtensionsString, AF_STR_Space);

            while (strTokenizer.getNextToken(currentExtension))
            {
                currPlatformExtensions.push_back(currentExtension);
            }
        }

        platformsExtensions.push_back(currPlatformExtensions);
        iter++;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
bool siOpenCLInformationCollector::CollectOpenCLDevicesInformation(gtList< gtList <gtString> >& infoData, bool fullAttributesList)
{
    bool retVal = false;

    // Get the local machine OpenCL devices list:
    gtPtrVector<apCLDevice*> devicesList;
    bool rcGetDevices = CollectOpenCLDevicesInformation(devicesList, fullAttributesList);

    if (rcGetDevices)
    {
        retVal = true;

        // Build the device titles and para
        BuildOpenCLDevicesTitlesAndParameterNames(infoData);

        // Get the platform ids:
        bool rcGetPlatformIds = CollectOpenCLPlatformIds();
        GT_ASSERT(rcGetPlatformIds);

        // Iterate the devices and fill the parameters:
        size_t devicesAmount = devicesList.size();

        for (unsigned int i = 0; i < devicesAmount; i++)
        {
            // Get the current device:
            const apCLDevice* pCurrentDevice = devicesList[i];
            GT_IF_WITH_ASSERT(pCurrentDevice != NULL)
            {
                bool rc = CollectOpenCLSingleDeviceInformation(infoData, pCurrentDevice, i, fullAttributesList);
                GT_ASSERT(rc);
            }
        }

        retVal = true;
    }

    if (fullAttributesList)
    {
        // Collect the devices extensions:
        bool rcExtensios = CollectOpenCLDevicesExtensions(infoData, devicesList);
        retVal = retVal && rcExtensios;
    }

    // Clean up:
    devicesList.deleteElementsAndClear();

    return retVal;
}

// ---------------------------------------------------------------------------
bool siOpenCLInformationCollector::CollectOpenCLDevicesInformation(gtPtrVector<apCLDevice*>& devicesList, bool fullAttributesList)
{
    bool retVal = false;

    bool rcOpenCLFunctions = InitOpenCLFunctionPointers();

    if (rcOpenCLFunctions)
    {
        // Get OpenCL platforms count:
        cl_uint amountOfPlatforms = 0;
        cl_uint clRetVal = pclGetPlatformIDs(0, NULL, &amountOfPlatforms);
        GT_IF_WITH_ASSERT(clRetVal == CL_SUCCESS)
        {
            retVal = true;

            if (amountOfPlatforms > 0)
            {
                // if there's a platform or more, make space for ID's:
                cl_platform_id* pPlatformIds = new cl_platform_id[amountOfPlatforms];

                // Get the platform ids:
                clRetVal = pclGetPlatformIDs(amountOfPlatforms, pPlatformIds, NULL);
                GT_IF_WITH_ASSERT(clRetVal == CL_SUCCESS)
                {
                    for (cl_uint i = 0; i < amountOfPlatforms; ++i)
                    {
                        // Get the current platform id:
                        cl_platform_id currentPlatformId = pPlatformIds[i];

                        // Get devices for this platform:
                        cl_uint amountOfDevices;
                        clRetVal = pclGetDeviceIDs(currentPlatformId, CL_DEVICE_TYPE_ALL, 0, NULL, &amountOfDevices);
                        GT_IF_WITH_ASSERT(clRetVal == CL_SUCCESS)
                        {
                            // Allocate a memory for the current platform device ids:
                            cl_device_id* pDevicesIds = new cl_device_id[amountOfDevices];
                            clRetVal = pclGetDeviceIDs(currentPlatformId, CL_DEVICE_TYPE_ALL, amountOfDevices, pDevicesIds, NULL);
                            GT_IF_WITH_ASSERT(clRetVal == CL_SUCCESS)
                            {
                                // Iterate the platforms devices:
                                for (int j = 0; j < (int)amountOfDevices; j++)
                                {
                                    // Create a data object for the current device:
                                    gtInt32 deviceAPIID = (gtInt32)devicesList.size();
                                    apCLDevice* pNewDevice = new apCLDevice(deviceAPIID, (oaCLDeviceID)(pDevicesIds[j]));

                                    // Log the device's details:
                                    pNewDevice->initialize(pclGetDeviceInfo, fullAttributesList);
                                    devicesList.push_back(pNewDevice);
                                }
                            }

                            // Delete the device ids vector:
                            delete[] pDevicesIds;
                        }
                    }
                }
                // Delete the platform ids vector:
                delete[] pPlatformIds;
            }
        }

        // Clean up:
        ReleaseOpenCLFunctionPointers();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
bool siOpenCLInformationCollector::InitOpenCLFunctionPointers()
{
    bool retVal = false;

    // Get the system OpenCL module path:
    gtVector<osFilePath> systemOCLModulePaths;
    bool rcOCLModule = osGetSystemOpenCLModulePath(systemOCLModulePaths);

    if (rcOCLModule)
    {
        // Load the OpenCL module:
        bool rcOCLModuleLoad = false;
        hOpenCLModule = OS_NO_MODULE_HANDLE;
        int numberOfPaths = (int)systemOCLModulePaths.size();

        for (int i = 0; (i < numberOfPaths) && (!rcOCLModuleLoad); i++)
        {
            rcOCLModuleLoad = osLoadModule(systemOCLModulePaths[i], hOpenCLModule, NULL, false);
        }

        if (rcOCLModuleLoad)
        {
            osProcedureAddress pFunctionHandler = NULL;
            bool rcGetPlatformIDs = osGetProcedureAddress(hOpenCLModule, "clGetPlatformIDs", pFunctionHandler);
            GT_IF_WITH_ASSERT(rcGetPlatformIDs)
            {
                pclGetPlatformIDs = (PFNCLGETPLATFORMIDSPROC)pFunctionHandler;
            }

            bool rcGetPlatformInfo = osGetProcedureAddress(hOpenCLModule, "clGetPlatformInfo", pFunctionHandler);
            GT_IF_WITH_ASSERT(rcGetPlatformInfo)
            {
                pclGetPlatformInfo = (PFNCLGETPLATFORMINFOSPROC)pFunctionHandler;
            }

            bool rcGetDeviceIds = osGetProcedureAddress(hOpenCLModule, "clGetDeviceIDs", pFunctionHandler);
            GT_IF_WITH_ASSERT(rcGetDeviceIds)
            {
                pclGetDeviceIDs = (PFNCLGETDEVICEIDSPROC)pFunctionHandler;
            }

            bool rcGetDeviceInfo = osGetProcedureAddress(hOpenCLModule, "clGetDeviceInfo", pFunctionHandler);
            GT_IF_WITH_ASSERT(rcGetDeviceInfo)
            {
                pclGetDeviceInfo = (PFNCLGETDEVICEINFOPROC)pFunctionHandler;
            }

            retVal = ((pclGetPlatformIDs != NULL) && (pclGetDeviceIDs != NULL) && (pclGetDeviceInfo != NULL));
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
bool siOpenCLInformationCollector::ReleaseOpenCLFunctionPointers()
{
    bool retVal = true;

    if ((pclGetPlatformIDs != NULL) || (pclGetPlatformInfo != NULL)
        || (pclGetDeviceIDs != NULL) || (pclGetDeviceInfo != NULL))
    {
        retVal = false;

        // Terminate the OpenCL function's pointers:
        pclGetPlatformIDs = NULL;
        pclGetPlatformInfo = NULL;
        pclGetDeviceIDs = NULL;
        pclGetDeviceInfo = NULL;

        // Release the OpenCL Module:
        bool rcReleaseModule = osReleaseModule(hOpenCLModule);
        GT_IF_WITH_ASSERT(rcReleaseModule)
        {
            retVal = true;
        }

    }

    hOpenCLModule = 0;

    return retVal;
}

// ---------------------------------------------------------------------------
bool siOpenCLInformationCollector::CollectOpenCLSingleDeviceInformation(gtList< gtList <gtString> >& infoData, const apCLDevice* pDevice, int deviceIndex, bool fullAttributesList)
{
    bool retVal = true;
    gtList< gtList <gtString> >::iterator iter = infoData.begin();

    GT_IF_WITH_ASSERT(iter != infoData.end())
    {
        gtList<gtString>& currentList = (*iter);
        gtString currDeviceId = AF_STR_Device;
        currDeviceId.append(AF_STR_Space);
        currDeviceId.appendFormattedString(AF_STR_IntFormat, deviceIndex + 1);

        // Add the device type to the name:
        gtString currDeviceTypeAsStr;
        apCLDeviceTypeAsString(pDevice->deviceType(), currDeviceTypeAsStr);
        currDeviceId.appendFormattedString(L" (%ls)", currDeviceTypeAsStr.asCharArray());

        currentList.push_back(currDeviceId);
        iter++;
    }

    GT_IF_WITH_ASSERT(iter != infoData.end())
    {
        gtList<gtString>& currentList = (*iter);
        gtString attrStr;
        int platformName = 0;

        // Try to find this platform in the map:
        gtMap<oaCLPlatformID, int>::const_iterator iterFind = _platformIdToName.find(pDevice->platformID());
        GT_IF_WITH_ASSERT(iterFind != _platformIdToName.end())
        {
            platformName = (*iterFind).second;
        }
        attrStr.appendFormattedString(AF_STR_IntFormat, platformName + 1);
        currentList.push_back(attrStr);
        iter++;
    }

    GT_IF_WITH_ASSERT(iter != infoData.end())
    {
        gtList<gtString>& currentList = (*iter);
        gtString currDeviceTypeAsStr;
        apCLDeviceTypeAsString(pDevice->deviceType(), currDeviceTypeAsStr);
        currentList.push_back(currDeviceTypeAsStr);
        iter++;
    }

    GT_IF_WITH_ASSERT(iter != infoData.end())
    {
        gtList<gtString>& currentList = (*iter);
        const gtString& currDeviceName = pDevice->deviceNameForDisplay();
        currentList.push_back(currDeviceName);
        iter++;
    }

    GT_IF_WITH_ASSERT(iter != infoData.end())
    {
        gtList<gtString>& currentList = (*iter);
        const gtString& currDeviceVendor = pDevice->deviceVendor();
        currentList.push_back(currDeviceVendor);
        iter++;
    }

    if (fullAttributesList)
    {
        // Collect the device extended information:
        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            cl_command_queue_properties queueProperties = pDevice->deviceQueueProperties();
            gtString queuePropertiesAsStr;
            apCLDeviceQueuePropertiesAsString(queueProperties, queuePropertiesAsStr);
            currentList.push_back(queuePropertiesAsStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtUInt64 maxGlobVarSize = pDevice->deviceMaxGlobalVariableSize();
            gtString maxGlobVarSizeAsString;
            maxGlobVarSizeAsString.fromMemorySize(maxGlobVarSize);
            currentList.push_back(maxGlobVarSizeAsString);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtUInt64 globVarPrefTotSz = pDevice->deviceGlobalVariablePreferredTotalSize();
            gtString globVarPrefTotSzAsString;
            globVarPrefTotSzAsString.fromMemorySize((gtUInt64)globVarPrefTotSz);
            currentList.push_back(globVarPrefTotSzAsString);
            iter++;
        }
        // Collect the device extended information:
        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            cl_command_queue_properties queueOnDevProperties = pDevice->queueOnDeviceProperties();
            gtString queueOnDevPropertiesAsStr;
            apCLDeviceQueuePropertiesAsString(queueOnDevProperties, queueOnDevPropertiesAsStr);
            currentList.push_back(queueOnDevPropertiesAsStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtUInt32 qodPrefSize = pDevice->queueOnDevicePreferredSize();
            gtString qodPrefSizeAsString;
            qodPrefSizeAsString.fromMemorySize((gtUInt64)qodPrefSize);
            currentList.push_back(qodPrefSizeAsString);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtUInt32 qodMaxSize = pDevice->queueOnDeviceMaxSize();
            gtString qodMaxSizeAsString;
            qodMaxSizeAsString.fromMemorySize((gtUInt64)qodMaxSize);
            currentList.push_back(qodMaxSizeAsString);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString maxQODsAsString;
            maxQODsAsString.appendFormattedString(AF_STR_UIntFormat, pDevice->deviceMaxOnDeviceQueues());
            currentList.push_back(maxQODsAsString);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString maxQODEventsAsString;
            maxQODEventsAsString.appendFormattedString(AF_STR_UIntFormat, pDevice->deviceMaxOnDeviceEvents());
            currentList.push_back(maxQODEventsAsString);
            iter++;
        }


        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            cl_device_svm_capabilities svmCapabilities = pDevice->deviceSVMCapabilities();
            gtString svmCapabilitiesAsStr;
            apCLDeviceSVMCapabilitiesAsString(svmCapabilities, svmCapabilitiesAsStr);
            currentList.push_back(svmCapabilitiesAsStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString maxPipeArgsAsString;
            maxPipeArgsAsString.appendFormattedString(AF_STR_UIntFormat, pDevice->deviceMaxPipeArgs());
            currentList.push_back(maxPipeArgsAsString);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString maxPipeActiveReservAsString;
            maxPipeActiveReservAsString.appendFormattedString(AF_STR_UIntFormat, pDevice->devicePipeMaxActiveReservations());
            currentList.push_back(maxPipeActiveReservAsString);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtUInt32 maxPipePacketSz = pDevice->devicePipeMaxPacketSize();
            gtString maxPipePacketSzAsString;
            maxPipePacketSzAsString.fromMemorySize((gtUInt64)maxPipePacketSz);
            currentList.push_back(maxPipePacketSzAsString);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString prefPlatAtomicAlignAsString;
            prefPlatAtomicAlignAsString.appendFormattedString(AF_STR_UIntFormat, pDevice->devicePreferredPlatformAtomicAlignment());
            currentList.push_back(prefPlatAtomicAlignAsString);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString prefGlobAtomicAlignAsString;
            prefGlobAtomicAlignAsString.appendFormattedString(AF_STR_UIntFormat, pDevice->devicePreferredGlobalAtomicAlignment());
            currentList.push_back(prefGlobAtomicAlignAsString);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString prefLocAtomicAlignAsString;
            prefLocAtomicAlignAsString.appendFormattedString(AF_STR_UIntFormat, pDevice->devicePreferredLocalAtomicAlignment());
            currentList.push_back(prefLocAtomicAlignAsString);
            iter++;
        }

        // Gilad: Hidden information until we solve bug 7159
        //GT_IF_WITH_ASSERT (iter != infoData.end())
        //{
        //  gtList<gtString>& currentList = (*iter);
        //  gtString bitsAsStr;
        //  bitsAsStr.appendFormattedString(AF_STR_IntFormat, pDevice->addressBits());
        //  currentList.push_back(bitsAsStr);
        //  iter++;
        //  retVal = true;
        //}

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString availbleStr = (pDevice->isAvailable()) ? AF_STR_Yes : AF_STR_No;
            currentList.push_back(availbleStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString availbleStr = (pDevice->isCompilerAvailable()) ? AF_STR_Yes : AF_STR_No;
            currentList.push_back(availbleStr);
            iter++;
        }

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString deviceFPConfigAsStr;
            apCLFPConfigAsString(pDevice->deviceSingleFPConfig(), deviceFPConfigAsStr);
            currentList.push_back(deviceFPConfigAsStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString deviceFPConfigAsStr;
            apCLFPConfigAsString(pDevice->deviceDoubleFPConfig(), deviceFPConfigAsStr);
            currentList.push_back(deviceFPConfigAsStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString deviceFPConfigAsStr;
            apCLFPConfigAsString(pDevice->deviceHalfFPConfig(), deviceFPConfigAsStr);
            currentList.push_back(deviceFPConfigAsStr);
            iter++;
        }
#endif

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString propertyStr = (pDevice->isEndianLittle()) ? AF_STR_Yes : AF_STR_No;
            currentList.push_back(propertyStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString propertyStr = (pDevice->isDeviceErrorCorrectionSupport()) ? AF_STR_Yes : AF_STR_No;
            currentList.push_back(propertyStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrAsStr;
            apCLExecutionCapabilitiesAsString(pDevice->deviceExecutionCapabilities(), attrAsStr);
            currentList.push_back(attrAsStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.fromMemorySize(pDevice->globalMemCacheSize());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrAsStr;
            apCLMemoryCacheTypeAsString(pDevice->memoryCacheType(), attrAsStr);
            currentList.push_back(attrAsStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.fromMemorySize((gtUInt64)pDevice->globalMemCacheLineSize());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.fromMemorySize(pDevice->globalMemSize());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString propertyStr = AF_STR_NotAvailable;

            if (pDevice->clMinorVersion() >= 1)
            {
                propertyStr = (pDevice->isHostUnifiedMemory()) ? AF_STR_Yes : AF_STR_No;
            }

            currentList.push_back(propertyStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString propertyStr = (pDevice->isImageSupport()) ? AF_STR_Yes : AF_STR_No;
            currentList.push_back(propertyStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtUInt32 image2Ddims[2];
            pDevice->maxImage2DDimension(image2Ddims[0], image2Ddims[1]);

            gtString attrStr;
            attrStr.appendFormattedString(L"(%uw, %uh)", image2Ddims[0], image2Ddims[1]);
            currentList.push_back(attrStr);

            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtUInt32 image3Ddims[3];
            pDevice->maxImage3DDimension(image3Ddims[0], image3Ddims[1], image3Ddims[2]);

            gtString attrStr;
            attrStr.appendFormattedString(L"(%uw, %uh, %ud)", image3Ddims[0], image3Ddims[1], image3Ddims[2]);
            currentList.push_back(attrStr);

            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.fromMemorySize(pDevice->localMemSize());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrAsStr;
            apCLLocalMemTypeAsString(pDevice->localMemType(), attrAsStr);
            currentList.push_back(attrAsStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->maxClockFrequency());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->maxComputeUnits());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->maxConstantArgs());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.fromMemorySize(pDevice->maxConstantBufferSize());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.fromMemorySize(pDevice->maxMemAllocSize());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.fromMemorySize((gtUInt64)pDevice->maxParamSize());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->maxReadImageArgs());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->maxSamplers());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.appendFormattedString(AF_STR_UInt64Format, (gtUInt64)pDevice->maxWorkgroupSize());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->maxWorkItemDimensions());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.append(AF_STR_LeftParenthesis);
            gtUInt32 amountOfDims = pDevice->maxWorkItemDimensions();
            gtUInt32* pMaxWorkItems = pDevice->maxWorkItemSizes();

            for (gtUInt32 i = 0; i < amountOfDims; i++)
            {
                gtUInt32 maxWorkItemSize = pMaxWorkItems[i];
                attrStr.appendFormattedString(AF_STR_UIntFormat, maxWorkItemSize);

                if (i < (amountOfDims - 1))
                {
                    attrStr.appendFormattedString(AF_STR_Comma);
                }
            }

            attrStr.append(AF_STR_RightParenthesis);
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            gtUInt32 maxImages = pDevice->maxWriteImageArgs();

            if (0 < maxImages)
            {
                attrStr.appendFormattedString(AF_STR_UIntFormat, maxImages);
            }
            else
            {
                attrStr = AF_STR_NotAvailable;
            }

            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            gtUInt32 maxImages = pDevice->maxReadWriteImageArgs();

            if (0 < maxImages)
            {
                attrStr.appendFormattedString(AF_STR_UIntFormat, maxImages);
            }
            else
            {
                attrStr = AF_STR_NotAvailable;
            }

            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->memBaseAddrAlign());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.fromMemorySize((gtUInt64)pDevice->minDataTypeAlignSize());
            currentList.push_back(attrStr);
            iter++;
        }

        // Device OpenCL C version:
        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString propertyStr = AF_STR_NotAvailable;

            if (pDevice->clMinorVersion() >= 1)
            {
                propertyStr = pDevice->deviceOpenCLCVersion();
            }

            currentList.push_back(propertyStr);
            iter++;
        }

        // Native vector width size for built-in scalar types:
        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr = AF_STR_NotAvailable;

            if (pDevice->clMinorVersion() >= 1)
            {
                attrStr.makeEmpty();
                attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->nativeVecWidthChar());
            }

            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr = AF_STR_NotAvailable;

            if (pDevice->clMinorVersion() >= 1)
            {
                attrStr.makeEmpty();
                attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->nativeVecWidthShort());
            }

            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr = AF_STR_NotAvailable;

            if (pDevice->clMinorVersion() >= 1)
            {
                attrStr.makeEmpty();
                attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->nativeVecWidthInt());
            }

            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr = AF_STR_NotAvailable;

            if (pDevice->clMinorVersion() >= 1)
            {
                attrStr.makeEmpty();
                attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->nativeVecWidthLong());
            }

            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr = AF_STR_NotAvailable;

            if (pDevice->clMinorVersion() >= 1)
            {
                attrStr.makeEmpty();
                attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->nativeVecWidthFloat());
            }

            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr = AF_STR_NotAvailable;

            if (pDevice->clMinorVersion() >= 1)
            {
                attrStr.makeEmpty();
                attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->nativeVecWidthDouble());
            }

            currentList.push_back(attrStr);
            iter++;
        }

        // Preferred native vector width size for built-in scalar types:
        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr = AF_STR_NotAvailable;

            if (pDevice->clMinorVersion() >= 1)
            {
                attrStr.makeEmpty();
                attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->nativeVecWidthHalf());
            }

            currentList.push_back(attrStr);
            iter++;
        }

        // Preferred native vector width size for built-in scalar types:
        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->preferredVecWidthChar());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->preferredVecWidthShort());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->preferredVecWidthInt());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->preferredVecWidthLong());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->preferredVecWidthFloat());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->preferredVecWidthDouble());
            currentList.push_back(attrStr);
            iter++;
        }

        // Preferred native vector width size for built-in scalar types:
        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr = AF_STR_NotAvailable;

            if (pDevice->clMinorVersion() >= 1)
            {
                attrStr.makeEmpty();
                attrStr.appendFormattedString(AF_STR_UIntFormat, pDevice->preferredVecWidthHalf());
            }

            currentList.push_back(attrStr);
            iter++;
        }

        // Device profile:
        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            currentList.push_back(pDevice->profileStr());
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            gtString attrStr;
            attrStr.appendFormattedString(AF_STR_UInt64Format, (gtUInt64)pDevice->profilingTimerResolution());
            currentList.push_back(attrStr);
            iter++;
        }

        GT_IF_WITH_ASSERT(iter != infoData.end())
        {
            gtList<gtString>& currentList = (*iter);
            currentList.push_back(pDevice->deviceVersion());
            iter++;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
void siOpenCLInformationCollector::BuildOpenCLPlatformsTitlesAndParameterNames(gtList< gtList <gtString> >& infoData)
{
    // Will contain the devices parameters:
    gtList <gtString> titleLine;
    titleLine.push_back(AF_STR_SystemInformationCommandParameter);
    infoData.push_back(titleLine);

    gtList <gtString> devicesVendors;
    devicesVendors.push_back(AF_STR_SystemInformationCommandVendor);
    infoData.push_back(devicesVendors);

    gtList <gtString> devicesNames;
    devicesNames.push_back(AF_STR_Name);
    infoData.push_back(devicesNames);

    gtList <gtString> devicesProfiles;
    devicesProfiles.push_back(AF_STR_SystemInformationCommandProfile);
    infoData.push_back(devicesProfiles);

    gtList <gtString> devicesVersion;
    devicesVersion.push_back(AF_STR_SystemInformationCommandVersion);
    infoData.push_back(devicesVersion);
}

// ---------------------------------------------------------------------------
void siOpenCLInformationCollector::BuildOpenCLDevicesTitlesAndParameterNames(gtList< gtList <gtString> >& infoData)
{
    // Clear the list:
    infoData.clear();

    // Will contain the devices parameters:
    gtList <gtString> titleLine;
    titleLine.push_back(AF_STR_SystemInformationCommandParameter);
    infoData.push_back(titleLine);

    gtList <gtString> platformIDList;
    platformIDList.push_back(AF_STR_SystemInformationCommandPlatformID);
    infoData.push_back(platformIDList);

    gtList <gtString> devicesTypes;
    devicesTypes.push_back(AF_STR_SystemInformationCommandDeviceType);
    infoData.push_back(devicesTypes);

    gtList <gtString> devicesNames;
    devicesNames.push_back(AF_STR_SystemInformationCommandDeviceName);
    infoData.push_back(devicesNames);

    gtList <gtString> devicesVendors;
    devicesVendors.push_back(AF_STR_SystemInformationCommandVendor);
    infoData.push_back(devicesVendors);

    gtList <gtString> devicesCommandQueueProperties;
    devicesCommandQueueProperties.push_back(AF_STR_SystemInformationCommandCommandQueueProperties);
    infoData.push_back(devicesCommandQueueProperties);

    gtList <gtString> devicesMaxGlobalVariableSize;
    devicesMaxGlobalVariableSize.push_back(AF_STR_SystemInformationCommandMaxGlobalVariableSize);
    infoData.push_back(devicesMaxGlobalVariableSize);

    gtList <gtString> devicesGlobalVariablePreferredTotalSize;
    devicesGlobalVariablePreferredTotalSize.push_back(AF_STR_SystemInformationCommandGlobalVariablePreferredTotalSize);
    infoData.push_back(devicesGlobalVariablePreferredTotalSize);

    gtList <gtString> devicesQueueOnDeviceProperties;
    devicesQueueOnDeviceProperties.push_back(AF_STR_SystemInformationCommandQueueOnDeviceProperties);
    infoData.push_back(devicesQueueOnDeviceProperties);

    gtList <gtString> devicesQueueOnDevicePreferredSize;
    devicesQueueOnDevicePreferredSize.push_back(AF_STR_SystemInformationCommandQueueOnDevicePreferredSize);
    infoData.push_back(devicesQueueOnDevicePreferredSize);

    gtList <gtString> devicesQueueOnDeviceMaxSize;
    devicesQueueOnDeviceMaxSize.push_back(AF_STR_SystemInformationCommandQueueOnDeviceMaxSize);
    infoData.push_back(devicesQueueOnDeviceMaxSize);

    gtList <gtString> devicesMaxOnDeviceQueues;
    devicesMaxOnDeviceQueues.push_back(AF_STR_SystemInformationCommandMaxOnDeviceQueues);
    infoData.push_back(devicesMaxOnDeviceQueues);

    gtList <gtString> devicesMaxOnDeviceEvents;
    devicesMaxOnDeviceEvents.push_back(AF_STR_SystemInformationCommandMaxOnDeviceEvents);
    infoData.push_back(devicesMaxOnDeviceEvents);

    gtList <gtString> devicesSVMCapabilities;
    devicesSVMCapabilities.push_back(AF_STR_SystemInformationCommandSVMCapabilities);
    infoData.push_back(devicesSVMCapabilities);

    gtList <gtString> devicesMaxPipeArgs;
    devicesMaxPipeArgs.push_back(AF_STR_SystemInformationCommandMaxPipeArgs);
    infoData.push_back(devicesMaxPipeArgs);

    gtList <gtString> devicesPipeMaxActiveReservations;
    devicesPipeMaxActiveReservations.push_back(AF_STR_SystemInformationCommandPipeMaxActiveReservations);
    infoData.push_back(devicesPipeMaxActiveReservations);

    gtList <gtString> devicesPipeMaxPacketSize;
    devicesPipeMaxPacketSize.push_back(AF_STR_SystemInformationCommandPipeMaxPacketSize);
    infoData.push_back(devicesPipeMaxPacketSize);

    gtList <gtString> devicesPreferredPlatformAtomicAlignment;
    devicesPreferredPlatformAtomicAlignment.push_back(AF_STR_SystemInformationCommandPreferredPlatformAtomicAlignment);
    infoData.push_back(devicesPreferredPlatformAtomicAlignment);

    gtList <gtString> devicesPreferredGlobalAtomicAlignment;
    devicesPreferredGlobalAtomicAlignment.push_back(AF_STR_SystemInformationCommandPreferredGlobalAtomicAlignment);
    infoData.push_back(devicesPreferredGlobalAtomicAlignment);

    gtList <gtString> devicesPreferredLocalAtomicAlignment;
    devicesPreferredLocalAtomicAlignment.push_back(AF_STR_SystemInformationCommandPreferredLocalAtomicAlignment);
    infoData.push_back(devicesPreferredLocalAtomicAlignment);


    // Gilad: Hide information until bug 7159 is solved
    //  gtList <gtString> devicesAddress;
    //  devicesAddress.push_back(AF_STR_SystemInformationCommandAddressBits);
    //  infoData.push_back(devicesAddress);

    gtList <gtString> devicesIsAvailableBits;
    devicesIsAvailableBits.push_back(AF_STR_SystemInformationCommandIsAvailable);
    infoData.push_back(devicesIsAvailableBits);

    gtList <gtString> devicesIsCompilerAvailable;
    devicesIsCompilerAvailable.push_back(AF_STR_SystemInformationCommandIsCompilerAvailable);
    infoData.push_back(devicesIsCompilerAvailable);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    gtList <gtString> devicesSingleFPConfig;
    devicesSingleFPConfig.push_back(AF_STR_SystemInformationCommandSingleFPConfig);
    infoData.push_back(devicesSingleFPConfig);

    gtList <gtString> devicesDoubleFPConfig;
    devicesDoubleFPConfig.push_back(AF_STR_SystemInformationCommandDoubleFPConfig);
    infoData.push_back(devicesDoubleFPConfig);

    gtList <gtString> devicesHalfFPConfig;
    devicesHalfFPConfig.push_back(AF_STR_SystemInformationCommandHalfFPConfig);
    infoData.push_back(devicesHalfFPConfig);
#endif

    gtList <gtString> devicesIsLittleEndian;
    devicesIsLittleEndian.push_back(AF_STR_SystemInformationCommandIsLittleEndian);
    infoData.push_back(devicesIsLittleEndian);

    gtList <gtString> devicesIsErrorCorrectionSupport;
    devicesIsErrorCorrectionSupport.push_back(AF_STR_SystemInformationCommandErrorCorrectionSupport);
    infoData.push_back(devicesIsErrorCorrectionSupport);

    gtList <gtString> deviceExecutionCapabilitiesList;
    deviceExecutionCapabilitiesList.push_back(AF_STR_SystemInformationCommandExecutionCapabilities);
    infoData.push_back(deviceExecutionCapabilitiesList);

    gtList <gtString> deviceGlobalMemCacheSizeList;
    deviceGlobalMemCacheSizeList.push_back(AF_STR_SystemInformationCommandGlobalMemoryCacheSize);
    infoData.push_back(deviceGlobalMemCacheSizeList);

    gtList <gtString> deviceMemCacheTypeList;
    deviceMemCacheTypeList.push_back(AF_STR_SystemInformationCommandMemoryCacheType);
    infoData.push_back(deviceMemCacheTypeList);

    gtList <gtString> deviceGlobalMemCacheLineSizeList;
    deviceGlobalMemCacheLineSizeList.push_back(AF_STR_SystemInformationCommandGlobalMemoryCacheLineSize);
    infoData.push_back(deviceGlobalMemCacheLineSizeList);

    gtList <gtString> deviceGlobalMemSizeList;
    deviceGlobalMemSizeList.push_back(AF_STR_SystemInformationCommandGlobalMemorySize);
    infoData.push_back(deviceGlobalMemSizeList);

    gtList <gtString> deviceisHostUnifiedMemoryList;
    deviceisHostUnifiedMemoryList.push_back(AF_STR_SystemInformationCommandHostUnifiedMemory);
    infoData.push_back(deviceisHostUnifiedMemoryList);

    gtList <gtString> deviceIsImageSupportList;
    deviceIsImageSupportList.push_back(AF_STR_SystemInformationCommandAreImageSupported);
    infoData.push_back(deviceIsImageSupportList);

    gtList <gtString> maxImage2DDimensionList;
    maxImage2DDimensionList.push_back(AF_STR_SystemInformationCommandMaxImage2DDimensions);
    infoData.push_back(maxImage2DDimensionList);

    gtList <gtString> maxImage3DDimensionList;
    maxImage3DDimensionList.push_back(AF_STR_SystemInformationCommandMaxImage3DDimensions);
    infoData.push_back(maxImage3DDimensionList);

    gtList <gtString> deviceLocalMemSizeList;
    deviceLocalMemSizeList.push_back(AF_STR_SystemInformationCommandLocalMemorySize);
    infoData.push_back(deviceLocalMemSizeList);

    gtList <gtString> deviceLocalMemTypeList;
    deviceLocalMemTypeList.push_back(AF_STR_SystemInformationCommandLocalMemoryType);
    infoData.push_back(deviceLocalMemTypeList);

    gtList <gtString> maxClockFrequencyList;
    maxClockFrequencyList.push_back(AF_STR_SystemInformationCommandMaxClockFrequency);
    infoData.push_back(maxClockFrequencyList);

    gtList <gtString> maxComputeUnitsList;
    maxComputeUnitsList.push_back(AF_STR_SystemInformationCommandMaxComputeUnits);
    infoData.push_back(maxComputeUnitsList);

    gtList <gtString> maxConstantArgsList;
    maxConstantArgsList.push_back(AF_STR_SystemInformationCommandMaxConstantArguments);
    infoData.push_back(maxConstantArgsList);

    gtList <gtString> maxConstantBufferSizeList;
    maxConstantBufferSizeList.push_back(AF_STR_SystemInformationCommandMaxConstantBufferSize);
    infoData.push_back(maxConstantBufferSizeList);

    gtList <gtString> maxMemAllocSizeList;
    maxMemAllocSizeList.push_back(AF_STR_SystemInformationCommandMaxMemoryAllocationSize);
    infoData.push_back(maxMemAllocSizeList);

    gtList <gtString> maxParamSizeList;
    maxParamSizeList.push_back(AF_STR_SystemInformationCommandMaxParameterSize);
    infoData.push_back(maxParamSizeList);

    gtList <gtString> maxReadImageArgsList;
    maxReadImageArgsList.push_back(AF_STR_SystemInformationCommandReadImageArguments);
    infoData.push_back(maxReadImageArgsList);

    gtList <gtString> maxSamplersList;
    maxSamplersList.push_back(AF_STR_SystemInformationCommandMaxSamplers);
    infoData.push_back(maxSamplersList);

    gtList <gtString> maxWorkgroupSizeList;
    maxWorkgroupSizeList.push_back(AF_STR_SystemInformationCommandMaxWorkgroupSize);
    infoData.push_back(maxWorkgroupSizeList);

    gtList <gtString> maxWorkItemDimensionsList;
    maxWorkItemDimensionsList.push_back(AF_STR_SystemInformationCommandMaxWorkItemDimensions);
    infoData.push_back(maxWorkItemDimensionsList);

    gtList <gtString> maxWorkItemSizesList;
    maxWorkItemSizesList.push_back(AF_STR_SystemInformationCommandMaxWorkItemSizes);
    infoData.push_back(maxWorkItemSizesList);

    gtList <gtString> maxWriteImageArgsList;
    maxWriteImageArgsList.push_back(AF_STR_SystemInformationCommandMaxWriteImageArguments);
    infoData.push_back(maxWriteImageArgsList);

    gtList <gtString> maxReadWriteImageArgsList;
    maxReadWriteImageArgsList.push_back(AF_STR_SystemInformationCommandMaxReadWriteImageArguments);
    infoData.push_back(maxReadWriteImageArgsList);

    gtList <gtString> memBaseAddrAlignList;
    memBaseAddrAlignList.push_back(AF_STR_SystemInformationCommandMemoryBaseAddressAlignment);
    infoData.push_back(memBaseAddrAlignList);

    gtList <gtString> minDataTypeAlignSizeList;
    minDataTypeAlignSizeList.push_back(AF_STR_SystemInformationCommandMinimalDataTypeAlignmentSize);
    infoData.push_back(minDataTypeAlignSizeList);

    gtList <gtString> openCLCVersionList;
    openCLCVersionList.push_back(AF_STR_SystemInformationCommandOpenCLCVersion);
    infoData.push_back(openCLCVersionList);

    gtList <gtString> nativeVecWidthCharList;
    nativeVecWidthCharList.push_back(AF_STR_SystemInformationCommandNativeCharVectorWidth);
    infoData.push_back(nativeVecWidthCharList);

    gtList <gtString> nativeVecWidthShortList;
    nativeVecWidthShortList.push_back(AF_STR_SystemInformationCommandNativeShortVectorWidth);
    infoData.push_back(nativeVecWidthShortList);

    gtList <gtString> nativeVecWidthIntList;
    nativeVecWidthIntList.push_back(AF_STR_SystemInformationCommandNativeIntVectorWidth);
    infoData.push_back(nativeVecWidthIntList);

    gtList <gtString> nativeVecWidthLongList;
    nativeVecWidthLongList.push_back(AF_STR_SystemInformationCommandNativeLongVectorWidth);
    infoData.push_back(nativeVecWidthLongList);

    gtList <gtString> nativeVecWidthFloatList;
    nativeVecWidthFloatList.push_back(AF_STR_SystemInformationCommandNativeFloatVectorWidth);
    infoData.push_back(nativeVecWidthFloatList);

    gtList <gtString> nativeVecWidthDoubleList;
    nativeVecWidthDoubleList.push_back(AF_STR_SystemInformationCommandNativeDoubleVectorWidth);
    infoData.push_back(nativeVecWidthDoubleList);

    gtList <gtString> nativeVecWidthHalfList;
    nativeVecWidthHalfList.push_back(AF_STR_SystemInformationCommandNativeHalfVectorWidth);
    infoData.push_back(nativeVecWidthHalfList);

    gtList <gtString> preferredVecWidthCharList;
    preferredVecWidthCharList.push_back(AF_STR_SystemInformationCommandPreferredCharVectorWidth);
    infoData.push_back(preferredVecWidthCharList);

    gtList <gtString> preferredVecWidthShortList;
    preferredVecWidthShortList.push_back(AF_STR_SystemInformationCommandPreferredShortVectorWidth);
    infoData.push_back(preferredVecWidthShortList);

    gtList <gtString> preferredVecWidthIntList;
    preferredVecWidthIntList.push_back(AF_STR_SystemInformationCommandPreferredIntVectorWidth);
    infoData.push_back(preferredVecWidthIntList);

    gtList <gtString> preferredVecWidthLongList;
    preferredVecWidthLongList.push_back(AF_STR_SystemInformationCommandPreferredLongVectorWidth);
    infoData.push_back(preferredVecWidthLongList);

    gtList <gtString> preferredVecWidthFloatList;
    preferredVecWidthFloatList.push_back(AF_STR_SystemInformationCommandPreferredFloatVectorWidth);
    infoData.push_back(preferredVecWidthFloatList);

    gtList <gtString> preferredVecWidthDoubleList;
    preferredVecWidthDoubleList.push_back(AF_STR_SystemInformationCommandPreferredDoubleVectorWidth);
    infoData.push_back(preferredVecWidthDoubleList);

    gtList <gtString> preferredVecWidthHalfList;
    preferredVecWidthHalfList.push_back(AF_STR_SystemInformationCommandPreferredHalfVectorWidth);
    infoData.push_back(preferredVecWidthHalfList);

    gtList <gtString> profileStrList;
    profileStrList.push_back(AF_STR_SystemInformationCommandProfile);
    infoData.push_back(profileStrList);

    gtList <gtString> profilingTimerResolutionList;
    profilingTimerResolutionList.push_back(AF_STR_SystemInformationCommandProfilingTimerResolution);
    infoData.push_back(profilingTimerResolutionList);


    gtList <gtString> deviceVendorIDList;
    deviceVendorIDList.push_back(AF_STR_SystemInformationCommandVendorID);
    infoData.push_back(deviceVendorIDList);

}

// ---------------------------------------------------------------------------
bool siOpenCLInformationCollector::PlatformParamAsString(oaCLPlatformID platformHandle, cl_platform_info paramName, gtString& paramValueAsStr)
{
    bool retVal = false;

    // Get the parameter value's length:
    gtSizeType stringLen = 0;
    cl_platform_id platformId = (cl_platform_id)platformHandle;
    cl_int clRetVal = pclGetPlatformInfo(platformId, paramName, 0, NULL, &stringLen);
    GT_IF_WITH_ASSERT((clRetVal == CL_SUCCESS) && (stringLen > 0))
    {
        // Allocate space for the parameter's value:
        char* pParamValue = new char[stringLen + 1];

        // Get the parameter's value:
        clRetVal = pclGetPlatformInfo((cl_platform_id)platformHandle, paramName, stringLen + 1, pParamValue, NULL);
        GT_IF_WITH_ASSERT(clRetVal == CL_SUCCESS)
        {
            paramValueAsStr.fromASCIIString(pParamValue);
            retVal = true;
        }

        // Clean up:
        delete[] pParamValue;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
bool siOpenCLInformationCollector::CollectOpenCLPlatformIds()
{
    bool retVal = false;
    // Initialize the OpenCL platform function pointers:
    bool rcOpenCLFunctions = InitOpenCLFunctionPointers();

    if (rcOpenCLFunctions)
    {
        // Get OpenCL platforms count:
        cl_uint amountOfPlatforms = 0;
        cl_uint clRetVal = pclGetPlatformIDs(0, NULL, &amountOfPlatforms);
        GT_IF_WITH_ASSERT(clRetVal == CL_SUCCESS)
        {
            retVal = true;

            // If there are OpenCL platforms registered in the OpenCL ICD:
            if (0 < amountOfPlatforms)
            {
                // Allocate space for the platform IDs:
                cl_platform_id* pPlatformIds = new cl_platform_id[amountOfPlatforms];

                // Get the platform ids:
                clRetVal = pclGetPlatformIDs(amountOfPlatforms, pPlatformIds, NULL);
                GT_IF_WITH_ASSERT(clRetVal == CL_SUCCESS)
                {
                    // Iterate the platforms:
                    for (cl_uint i = 0; i < amountOfPlatforms; ++i)
                    {
                        // Get the current platform id:
                        cl_platform_id currentPlatformId = pPlatformIds[i];

                        // Add this platform to the mapping:
                        oaCLPlatformID platformId = oaCLPlatformID(currentPlatformId);
                        _platformIdToName[platformId] = i;
                    }
                }

                // Clean up:
                delete[] pPlatformIds;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
bool siOpenCLInformationCollector::GenerateAndSendOpenCLDevicesInformation(gtString pipeName)
{
    bool retVal = false;

    gtList< gtList <gtString> > infoData;
    bool rcCollectInfo = CollectOpenCLDevicesInformation(infoData, true);

    GT_IF_WITH_ASSERT(rcCollectInfo)
    {
        osPipeSocketClient* pClient = new osPipeSocketClient(pipeName, L"System Information Connection Socket");
        pClient->setWriteOperationTimeOut(OS_CHANNEL_INFINIT_TIME_OUT);
        bool rcOpenClient = pClient->open();

        GT_IF_WITH_ASSERT(rcOpenClient)
        {
            gtString size;
            size.appendUnsignedIntNumber((unsigned int)infoData.size());
            *pClient << size;

            while (infoData.size() > 0)
            {
                gtList<gtString> curList = infoData.front();
                size = L"";
                size.appendUnsignedIntNumber((unsigned int)curList.size());
                *pClient << size;

                while (curList.size() > 0)
                {
                    gtString curStr = curList.front();
                    *pClient << curStr;
                    curList.pop_front();
                }

                infoData.pop_front();
            }

            retVal = infoData.empty();
        }

        pClient->close();
        delete pClient;
    }
    return retVal;
}
