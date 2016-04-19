//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csExtensionsManager.h
///
//==================================================================================

//------------------------------ csExtensionsManager.h ------------------------------

#ifndef __CSEXTENSIONSMANAGER_H
#define __CSEXTENSIONSMANAGER_H

// Forward declarations:
class gtString;
class osFilePath;
struct csMonitoredFunctionPointers;

// Infra:
#include <CL/cl.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionId.h>

// ----------------------------------------------------------------------------------
// Class Name:          csExtensionsManager
// General Description: Manages the supported OpenCL extension functions.
// Author:              Uri Shomroni
// Creation Date:       26/11/2009
// ----------------------------------------------------------------------------------
class csExtensionsManager
{
public:
    static csExtensionsManager& instance();
    ~csExtensionsManager();
    bool initialize();

    osProcedureAddress wrapperFunctionAddress(const gtString& functionName);
    osProcedureAddress wrapperFunctionAddress(cl_platform_id platformId, const gtString& functionName);
    osProcedureAddress spyImplementedExtensionAddress(const gtString& functionName) const;
    csMonitoredFunctionPointers* extensionsRealImplementationPointers();
    int functionIndexFromMonitoredFunctionId(apMonitoredFunctionId funcId) const;

private:
    bool initializeExtensionWrapperAddresses();
    bool getOCLServerModulePath(osFilePath& oclModulePath);
    bool getOCLServerModuleHandle(osModuleHandle& thisModuleHandle);
    bool getOCLServerModuleHandleUnderDotNetApp(osModuleHandle& thisModuleHandle);

    // Only my instance() method is allowed to create me:
    csExtensionsManager();

private:
    // Maps OpenCL function index (see functionIndexFromMonitoredFunctionId)
    // to our wrapper / implementation address:
    gtVector<osProcedureAddress> _functionIdToWrapperAddress;

private:
    // A pointer to this class single instance:
    static csExtensionsManager* _pMySingleInstance;

    // Allow csSingletonsDelete to delete my single instance:
    friend class csSingletonsDelete;
};

#endif //__CSEXTENSIONSMANAGER_H

