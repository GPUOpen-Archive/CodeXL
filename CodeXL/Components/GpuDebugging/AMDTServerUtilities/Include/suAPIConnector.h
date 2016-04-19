//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suAPIConnector.h
///
//==================================================================================

//------------------------------ suAPIConnector.h ------------------------------

#ifndef __SUAPICONNECTOR_H
#define __SUAPICONNECTOR_H

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTAPIClasses/Include/apAPIConnectionType.h>

// Local:
#include <AMDTServerUtilities/Include/suSpiesUtilitiesDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           SU_API suAPIConnector
// General Description: Enables connecting to API spy, from other spy
// Author:               Sigal Algranaty
// Creation Date:        21/7/2010
// ----------------------------------------------------------------------------------
class SU_API suAPIConnector
{
public:
    static suAPIConnector& instance();
    virtual ~suAPIConnector();

    // Get Spy procedure address:
    bool osGetSpyProcAddress(apAPIConnectionType connectionType, const char* procName, osProcedureAddress& procedureAddress);

    enum suSpyType
    {
        SU_SPY_UNKNOWN,
        SU_SPY,
        SU_SPY_IPHONE,
        SU_SPY_ES,
        SU_SPY_ES_LITE
    };
    // Utilities:
    bool getServerModuleHandle(suSpyType spyType, apAPIConnectionType conntectionType, osModuleHandle& thisModuleHandle);
    bool getServerModulePath(suSpyType spyType, apAPIConnectionType connectionType, osFilePath& oglModulePath);

    bool getServerModuleHandleUnderDotNetApp(apAPIConnectionType connectionType, osModuleHandle& thisModuleHandle);


private:

    // Load a spy handle if not loaded:
    osModuleHandle getLoadedSpyHandle(apAPIConnectionType connectionType);

    suAPIConnector();

private:

    // My single instance:
    static suAPIConnector* _pMySingleInstance;

    // Maps API type to module handle:
    gtMap<apAPIConnectionType, osModuleHandle> _apiTypeToModuleHandleMap;
};


#endif //__SUAPICONNECTOR_H


