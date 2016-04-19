//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afSystemInformationCommand.h
///
//==================================================================================

#ifndef __GDSYSTEMINFORMATIONCOMMAND
#define __GDSYSTEMINFORMATIONCOMMAND

// Forward declarations:
class apCLDevice;

// Infra:
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTOSAPIWrappers/Include/oaPixelFormat.h>
#include <AMDTOSAPIWrappers/Include/oaOpenCLIncludes.h>
#include <AMDTOSAPIWrappers/Include/oaOpenCLFunctionPointers.h>
#include <AMDTOSWrappers/Include/osThread.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afCommand.h>
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>

enum afApplicationType
{
    af32BitApp,
    af64BitApp
};

class afSystemInformationCommandThread : public osThread
{
    friend class afInitializeApplicationCommand;
public:
    ~afSystemInformationCommandThread() {};
    /// Overrides osThread's entryPoint -- the thread function
    /// \return zero
    virtual int entryPoint();

    /// return the status of gathering data
    bool& isGatheringData() { return m_isGatheringData; }

    /// The system information string
    gtString m_systemInformationStr;

private:
    /// Initializes a new thread to handle gathering of data
    afSystemInformationCommandThread();

    /// flag to indicate the thread is gathering data
    bool m_isGatheringData;
};

// ----------------------------------------------------------------------------------
// Class Name:           afSystemInformationCommand : public afCommand
// General Description:
//                       Get the system information for the gdSystemInformationDialog.
// Author:               Avi Shapira
// Creation Date:        13/11/2003
// ----------------------------------------------------------------------------------
class AF_API afSystemInformationCommand : public afCommand
{
public:
    afSystemInformationCommand();
    virtual ~afSystemInformationCommand();

    // Overrides afCommand:
    virtual bool canExecuteSpecificCommand();
    virtual bool executeSpecificCommand();

    bool collectSystemInformation(gtList< gtList <gtString> >& infoData);
    bool collectDisplayInformation(gtList< gtList <gtString> >& infoData);
    bool collectGraphicCardInformation();
    bool collectPixelFormatInformation();
    bool collectOpenGLExtensionsInformation();
    bool collectOpenCLPlatformsInformation(gtList< gtList <gtString> >& infoData, bool fullAttributesList);
    /// Collects info regarding open CL devices
    bool CollectOpenCLDevicesInformation(gtList< gtList <gtString> >& infoData, bool fullAttributesList);
    /// Collects info regarding open CL devices by launching an 32 or 64 bit executable that will call the above CollectOpenCLDevicesInformation
    bool CollectOpenCLDevicesInformation(gtList< gtList <gtString> >& infoData, afApplicationType appType);
    /// Collects open CL devices info for both 32 & 64 bit and merge results into infoData
    bool CollectAllOpenCLDevicesInformation(gtList< gtList <gtString> >& infoData);
    bool collectOpenCLPlatformIds();
    bool getSystemInformationDataAsString(gtString& systemInformationStr);
    bool getSystemModulesVersionAsString(gtString& systemModulesVersionStr);
    bool getGraphicCardDetails(gtString& vendorString, gtString& rendererString, gtString& versionString, gtString& shadingLangVersionString, gtString& rendererType);
    bool CollectOpenCLDevicesInformation(gtPtrVector<apCLDevice*>& devicesList, bool fullAttributesList);
    bool getNumberOfOpenCLDevices(int& numberOfDevices);

    // Get cached information
    // Get graphic card information
    gtList< gtList <gtString> >& GetGraphicCardData() { return m_sGraphicCardInfoList; }

    // Get graphic card information
    gtList< gtList <gtString> >& GetPixelData() { return m_sPixelInfoList; }

    // Get graphic card information
    gtList< gtList <gtString> >& GetOpenGLExtData() { return m_sOpenGLExtInfoList; }

protected:

    bool getExtensionsList(gtList <gtString>& extensionsStringList);
    oaPixelFormat::HardwareSupport calculateMaximalHadrwareSupport() const;
    bool initOpenCLFunctionPointers();
    bool releaseOpenCLFunctionPointers();
    bool collectOpenCLSinglePlatformInformation(int platformID, oaCLPlatformID platformHandle, gtList< gtList <gtString> >& infoData);
    bool collectOpenCLPlatformsExtensions(gtList< gtList <gtString> >& infoData);
    bool collectOpenCLDevicesExtensions(gtList< gtList <gtString> >& infoData, const gtPtrVector<apCLDevice*>& devicesList);
    bool getOpenCLPlatformsExtensions(gtVector< gtVector <gtString> >& platformsExtensions);
    bool collectOpenCLSingleDeviceInformation(gtList< gtList <gtString> >& infoData, const apCLDevice* pDevice, int deviceIndex, bool fullAttributesList);
    void buildOpenCLPlatformsTitlesAndParameterNames(gtList< gtList <gtString> >& infoData);
    void buildOpenCLDevicesTitlesAndParameterNames(gtList< gtList <gtString> >& infoData);
    bool platformParamAsString(oaCLPlatformID platformHandle, cl_platform_info paramName, gtString& paramValueAsStr);
    void appendFlattenedListOfStrings(gtList<gtList<gtString> >& infoList, gtString& stringToAppnedTo);
    void GenerateUniquePipeName(gtString& pipeName);

protected:
    // The OpenCL module handle:
    osModuleHandle _hOpenCLModule;

    // The OpenCL function pointers:
    PFNCLGETPLATFORMIDSPROC _pclGetPlatformIDs;
    PFNCLGETPLATFORMINFOSPROC _pclGetPlatformInfo;
    PFNCLGETDEVICEIDSPROC _pclGetDeviceIDs;
    PFNCLGETDEVICEINFOPROC _pclGetDeviceInfo;

    // Contain a mapping from OpenCL
    gtMap <oaCLPlatformID, int> _platformIdToName;

    gtVector<gtString> _systemModuleNames;

    // collected graphic card info
    static gtList< gtList <gtString> > m_sGraphicCardInfoList;

    // collected pixel info
    static gtList< gtList <gtString> > m_sPixelInfoList;

    // collected OpenGLExt info
    static gtList< gtList <gtString> > m_sOpenGLExtInfoList;
};

#endif  // __GDSYSTEMINFORMATIONCOMMAND
