//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file siOpenCLInformationCollector.h
///
//==================================================================================

//------------------------------ siCollectOpenCLPlatformsInformation.h ------------------------------

#ifndef __COLLECTOPENCLPLATFORMSINFORMATION
#define __COLLECTOPENCLPLATFORMSINFORMATION

// Infra:
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTOSAPIWrappers/Include/oaPixelFormat.h>
#include <AMDTOSAPIWrappers/Include/oaOpenCLIncludes.h>
#include <AMDTOSAPIWrappers/Include/oaOpenCLFunctionPointers.h>
#include <AMDTAPIClasses/Include/apCLDevice.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afCommand.h>


/// \brief duplicate of functions from afSystemInformationCommand

class siOpenCLInformationCollector
{
public:
    siOpenCLInformationCollector();
    virtual ~siOpenCLInformationCollector();

    bool CollectOpenCLDevicesInformation(gtList< gtList <gtString> >& infoData, bool fullAttributesList);
    bool CollectOpenCLPlatformIds();
    bool CollectOpenCLDevicesInformation(gtPtrVector<apCLDevice*>& devicesList, bool fullAttributesList);

    /// Additional functionality to what was copied from afSystemInformationCommand
    /// assemble data to return to code XL via client socket
    bool GenerateAndSendOpenCLDevicesInformation(gtString pipeName);

protected:

    bool InitOpenCLFunctionPointers();
    bool ReleaseOpenCLFunctionPointers();
    bool CollectOpenCLSinglePlatformInformation(int platformID, oaCLPlatformID platformHandle, gtList< gtList <gtString> >& infoData);
    bool CollectOpenCLPlatformsExtensions(gtList< gtList <gtString> >& infoData);
    bool CollectOpenCLDevicesExtensions(gtList< gtList <gtString> >& infoData, const gtPtrVector<apCLDevice*>& devicesList);
    bool GetOpenCLPlatformsExtensions(gtVector< gtVector <gtString> >& platformsExtensions);
    bool CollectOpenCLSingleDeviceInformation(gtList< gtList <gtString> >& infoData, const apCLDevice* pDevice, int deviceIndex, bool fullAttributesList);
    void BuildOpenCLPlatformsTitlesAndParameterNames(gtList< gtList <gtString> >& infoData);
    void BuildOpenCLDevicesTitlesAndParameterNames(gtList< gtList <gtString> >& infoData);
    bool PlatformParamAsString(oaCLPlatformID platformHandle, cl_platform_info paramName, gtString& paramValueAsStr);

protected:
    // The OpenCL module handle:
    osModuleHandle hOpenCLModule;

    // The OpenCL function pointers:
    PFNCLGETPLATFORMIDSPROC pclGetPlatformIDs;
    PFNCLGETPLATFORMINFOSPROC pclGetPlatformInfo;
    PFNCLGETDEVICEIDSPROC pclGetDeviceIDs;
    PFNCLGETDEVICEINFOPROC pclGetDeviceInfo;

    // Contain a mapping from OpenCL
    gtMap <oaCLPlatformID, int> _platformIdToName;
    gtVector<gtString> _systemModuleNames;

};

#endif  // __COLLECTOPENCLPLATFORMSINFORMATION
