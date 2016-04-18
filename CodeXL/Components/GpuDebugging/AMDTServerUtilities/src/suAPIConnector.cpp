//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suAPIConnector.cpp
///
//==================================================================================

//------------------------------ suAPIConnector.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osDirectory.h>

// Local:
#include <AMDTServerUtilities/Include/suAPIConnector.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>
#include <AMDTServerUtilities/Include/suInterceptionFunctions.h>


// Static members initializations:
suAPIConnector* suAPIConnector::_pMySingleInstance = NULL;


// ---------------------------------------------------------------------------
// Name:        suBreakpointsManager::instance
// Description: Returns the single instance of the suAPIConnector class
// Author:      Sigal Algranaty
// Date:        24/11/2009
// ---------------------------------------------------------------------------
suAPIConnector& suAPIConnector::instance()
{
    // If this class single instance was not already created:
    if (_pMySingleInstance == NULL)
    {
        // Create it:
        _pMySingleInstance = new suAPIConnector;
        GT_ASSERT(_pMySingleInstance);
    }

    return *_pMySingleInstance;
}


// ---------------------------------------------------------------------------
// Name:        suAPIConnector::suAPIConnector
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        24/11/2009
// ---------------------------------------------------------------------------
suAPIConnector::suAPIConnector()
{
}


// ---------------------------------------------------------------------------
// Name:        suAPIConnector::~suAPIConnector
// Description: Destructor.
// Author:      Sigal Algranaty
// Date:        24/11/2009
// ---------------------------------------------------------------------------
suAPIConnector::~suAPIConnector()
{
}

// ---------------------------------------------------------------------------
// Name:        suAPIConnector::osGetSpyProcAddress
// Description: Get spy procedure address
// Arguments:   apAPIConnectionType connectionType
//              const char* procName - we use char* since GetProcAddress uses char*, and we
//              don't want to waste time on conversions.
//              osProcedureAddress& procedureAddress
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        21/7/2010
// ---------------------------------------------------------------------------
bool suAPIConnector::osGetSpyProcAddress(apAPIConnectionType connectionType, const char* procName, osProcedureAddress& procedureAddress)
{
    bool retVal = false;

    // Get the requested API loaded module:
    osModuleHandle spyModuleHandle = getLoadedSpyHandle(connectionType);
    GT_IF_WITH_ASSERT(spyModuleHandle != NULL)
    {
        // Get the procedure address:
        retVal = osGetProcedureAddress(spyModuleHandle, procName, procedureAddress);
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        suAPIConnector::getLoadedSpyHandle
// Description: Loads a spy handle if not loaded:
// Arguments:   apAPIConnectionType connectionType
// Return Val:  osModuleHandle
// Author:      Sigal Algranaty
// Date:        21/7/2010
// ---------------------------------------------------------------------------
osModuleHandle suAPIConnector::getLoadedSpyHandle(apAPIConnectionType connectionType)
{
    osModuleHandle pRetVal = NULL;

    // Check if the module is already loaded:
    gtMap<apAPIConnectionType, osModuleHandle>::const_iterator findIter = _apiTypeToModuleHandleMap.find(connectionType);

    if (findIter == _apiTypeToModuleHandleMap.end())
    {
        // Get the module handle:
        getServerModuleHandle(SU_SPY , connectionType, pRetVal);
    }
    else
    {
        // The module is already loaded:
        pRetVal = findIter->second;
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        suAPIConnector::getServerModuleHandle
// Description: Returns a handle to the requested spy server module.
// Arguments:   suSpyType spyType
//            apAPIConnectionType conntectionType
//            osModuleHandle& thisModuleHandle
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        21/7/2010
// Implementation notes:
//   We must not load another instance of this module (osLoadModule). Instead,
//   we use osGetLoadedModuleHandle that retrieves a handle to this loaded module.
// ---------------------------------------------------------------------------
bool suAPIConnector::getServerModuleHandle(suSpyType spyType, apAPIConnectionType conntectionType, osModuleHandle& thisModuleHandle)
{
    bool retVal = false;

    // Get the server module path:
    osFilePath serverModulePath;
    bool rc1 = getServerModulePath(spyType, conntectionType, serverModulePath);
    GT_IF_WITH_ASSERT(rc1)
    {
        // Get the loaded OpenGL / ES module handle:
        bool gotServerModuleHandle = osGetLoadedModuleHandle(serverModulePath, thisModuleHandle);

        // Under Windows, it might be that we are running under a .NET application.
        // (See getServerModuleHandleUnderDotNetApp documentation for more details)
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        {
            if (!gotServerModuleHandle)
            {
                gotServerModuleHandle = getServerModuleHandleUnderDotNetApp(conntectionType, thisModuleHandle);
            }
        }
#endif

        // If we managed to the the server module handle:
        GT_IF_WITH_ASSERT(gotServerModuleHandle)
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        suAPIConnector::getServerModuleHandleUnderDotNetApp
// Description:
//   When debugging .NET application, we ask the user to copy the spy into the debugged
//   application directory. In this case, the osGetLoadedModuleHandle() call that resides in
//   getOGLServerModuleHandle() will fail since the loaded OpenGL / OpenCL server now resides under the
//   debugged application directory and not under the spies directory.
//   This function assumes that the OpenGL server resides under the debugged application
//   directory and tried to get a handle to it.
//
// Arguments:  apAPIConnectionType connectionType - the requested server API tpye
//             thisModuleHandle - Will get the OpenGL server module handle.
// Return Val: bool  - Success / failure.
//
// Author:      Yaki Tebeka
// Date:        16/5/2007
// ---------------------------------------------------------------------------
bool suAPIConnector::getServerModuleHandleUnderDotNetApp(apAPIConnectionType connectionType, osModuleHandle& thisModuleHandle)
{
    bool retVal = false;

    // Get the current application path:
    osFilePath currentApplicationPath;
    bool rc2 = osGetCurrentApplicationPath(currentApplicationPath);
    GT_IF_WITH_ASSERT(rc2)
    {
        // Get the current application directory:
        osDirectory thisAppDir;
        bool rc3 = currentApplicationPath.getFileDirectory(thisAppDir);
        GT_IF_WITH_ASSERT(rc3)
        {
            gtString serverFileName = OS_GREMEDY_OPENGL_SERVER_MODULE_NAME;

            // Get the server file name:
            if (connectionType == AP_OPENCL_API_CONNECTION)
            {
                serverFileName = OS_GREMEDY_OPENCL_SERVER_MODULE_NAME;
            }

            // Build the server module path:
            gtString serverModuleFullPathAsStr = thisAppDir.directoryPath().asString();
            serverModuleFullPathAsStr += osFilePath::osPathSeparator;
            serverModuleFullPathAsStr += serverFileName;

            osFilePath serverModuleFullPath(serverModuleFullPathAsStr);

            // Try to get a handle to the module server:
            bool gotServerModuleHandle = osGetLoadedModuleHandle(serverModuleFullPath, thisModuleHandle);
            GT_IF_WITH_ASSERT(gotServerModuleHandle)
            {
                retVal = true;
            }
        }
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        suAPIConnector::getServerModulePath
// Description: Returns the path of the requested server module.
// Arguments:   oglModulePath - Will get the OpenGL server module.
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        21/7/2010
// ---------------------------------------------------------------------------
bool suAPIConnector::getServerModulePath(suSpyType spyType, apAPIConnectionType connectionType, osFilePath& serverModulePath)
{
    bool retVal = true;

    // Get this module name + file extension according to the spy type:
    gtString thisModuleFileName;

    if (connectionType == AP_OPENCL_API_CONNECTION)
    {
        // Get the OpenCL spy file name:
        thisModuleFileName = OS_GREMEDY_OPENCL_SERVER_MODULE_NAME;
    }

    else if (connectionType == AP_OPENGL_API_CONNECTION)
    {
        // Get the OpenGL spy file name:
        switch (spyType)
        {
            case SU_SPY:
            {
                thisModuleFileName = OS_GREMEDY_OPENGL_SERVER_MODULE_NAME;
                break;
            }

            case SU_SPY_ES:
            {
                thisModuleFileName = OS_OPENGL_ES_COMMON_DLL_NAME;
                break;
            }

            case SU_SPY_IPHONE:
            {
                thisModuleFileName = OS_OPENGL_ES_DEVICE_COMMON_DLL_NAME;
                break;
            }

            case SU_SPY_ES_LITE:
            {
                thisModuleFileName = OS_OPENGL_ES_COMMON_LITE_DLL_NAME;
                break;
            }

            default:
            {
                GT_ASSERT_EX(false, L"Unknown spy type");
                retVal = false;
                break;
            }
        }
    }
    else
    {
        retVal = false;
        GT_ASSERT_EX(false, L"Unknown API type");
    }

    GT_IF_WITH_ASSERT(retVal)
    {
        // If we are compiling the server:
        if ((spyType != SU_SPY_ES_LITE) && (spyType != SU_SPY_ES))
        {
            gtString spiesDirectory;
            bool isRunningInStandaloneMode = suIsRunningInStandaloneMode();
            bool rc1 = suGetSpiesDirectory(spiesDirectory, isRunningInStandaloneMode);
            GT_IF_WITH_ASSERT(rc1)
            {
                // Calculate this module full path:
                gtString thisModuleFullPathAsStr = spiesDirectory;
                thisModuleFullPathAsStr += osFilePath::osPathSeparator;
                thisModuleFullPathAsStr += thisModuleFileName;

                serverModulePath.setFullPathFromString(thisModuleFullPathAsStr);
                retVal = true;
            }
        }
        else
        {
            // We are compiling one of the OpenGL ES servers:
            serverModulePath.setFullPathFromString(thisModuleFileName);
            retVal = true;
        }

    }

    return retVal;
}

