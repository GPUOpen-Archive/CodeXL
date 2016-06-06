//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apMonitoredFunctionsManager.h
///
//==================================================================================

//------------------------------ apMonitoredFunctionsManager.h ------------------------------

#ifndef __APMONITOREDFUNCTIONSMANAGER
#define __APMONITOREDFUNCTIONSMANAGER

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionId.h>
#include <AMDTAPIClasses/Include/apAPIVersion.h>


// ----------------------------------------------------------------------------------
// Class Name:           apMonitoredFunctionsManager
// General Description:
//   Manages the functions that we are able to monitor.
//   (OpenGL functions, OpenGL extensions, WGL functions, etc)
//
// Author:  AMD Developer Tools Team
// Creation Date:        22/4/2004
// ----------------------------------------------------------------------------------
class AP_API apMonitoredFunctionsManager
{
public:
    static apMonitoredFunctionsManager& instance();

public:
    ~apMonitoredFunctionsManager();

    int amountOfMonitoredFunctions() const;
    const wchar_t* monitoredFunctionName(apMonitoredFunctionId functionId) const;
    apMonitoredFunctionId monitoredFunctionId(const wchar_t* functionName) const;
    unsigned int monitoredFunctionType(apMonitoredFunctionId functionId) const;
    unsigned int monitoredFunctionAPIType(apMonitoredFunctionId functionId) const;
    void getMonitoredFunctionDeprecationVersions(apMonitoredFunctionId functionId, apAPIVersion& deprectedAtVersion, apAPIVersion& removedAtVersion)const;

private:
    void initializeMonitoredFunctionsData();
    void initializeCoreOpenGLFunctionsData();
    void initializeCoreOpenGLESFunctionsData();
    void initializeOpenGLESExtensionsFunctionsData();
    void initializeEGLFunctionsData();
    void initializeEGLExtensionsFunctionsData();
    void initializeOpenGL12FunctionsData();
    void initializeOpenGL13FunctionsData();
    void initializeOpenGL14FunctionsData();
    void initializeOpenGL15FunctionsData();
    void initializeOpenGL20FunctionsData();
    void initializeOpenGL21FunctionsData();
    void initializeOpenGL30FunctionsData();
    void initializeOpenGL31FunctionsData();
    void initializeOpenGL32FunctionsData();
    void initializeOpenGL33FunctionsData();
    void initializeOpenGL40FunctionsData();
    void initializeOpenGL41FunctionsData();
    void initializeOpenGL42FunctionsData();
    void initializeOpenGL43FunctionsData();
    void initializeOpenGL44FunctionsData();
    void initializeOpenGL45FunctionsData();
    void initializeWGLFunctionsData();
    void initializeGLXFunctionsData();
    void initializeCGLFunctionsData();
    void initializeOpenGLExtensionFunctionsData();
    void initializeOpenCL10FunctionsData();
    void initializeOpenCL11FunctionsData();
    void initializeOpenCL12FunctionsData();
    void initializeOpenCL20FunctionsData();
    void initializeOpenCLExtensionFunctionsData();
    void initializeMonitoredFunctionNameToIdMap();

    // Only my instance() method should be able to create me:
    apMonitoredFunctionsManager();

    // Only apSingeltonsDelete should delete my single instance:
    friend class apSingeltonsDelete;

private:
    // Contains a monitored function data:
    struct AP_API MonitoredFunctionData
    {
        // Constructors:
        MonitoredFunctionData();
        MonitoredFunctionData(const wchar_t* name, unsigned int apiType, unsigned int functionType);
        MonitoredFunctionData(const wchar_t* name, unsigned int apiType, unsigned int functionType, apAPIVersion deprecatedAtVersion, apAPIVersion removedAtVersion);

        // The function name:
        const wchar_t* _name;

        // The function type (as a bit mask of apAPIType):
        unsigned int _apiType;

        // The function type (as a bit mask of apFunctionType):
        unsigned int _functionType;

        // The OpenGL version in which this function was declared as deprecated:
        //(or AP_GL_VERSION_NONE if the function was not declared as deprecated)
        apAPIVersion _deprecatedAtVersion;

        // The OpenGL version in which this function was removed:
        //(or AP_GL_VERSION_NONE if the function was not removed)
        apAPIVersion _removedAtVersion;
    };

    // Contains the monitored functions data:
    MonitoredFunctionData _monitoredFunctionsData[apMonitoredFunctionsAmount];

    // Maps monitored function name to id:
    gtMap<gtString, apMonitoredFunctionId> _monitoredFunctionNameToId;

    // This class single instance:
    static apMonitoredFunctionsManager* _pMySingleInstance;
};


#endif  // __APMONITOREDFUNCTIONSMANAGER
