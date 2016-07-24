//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdProcessDebuggersManager.cpp
///
//==================================================================================

//------------------------------ pdProcessDebuggersManager.cpp ------------------------------

// Infra
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/apDebugProjectSettings.h>

// Local
#include <src/pdStringConstants.h>
#include <AMDTProcessDebugger/Include/pdProcessDebuggersManager.h>
#include <AMDTProcessDebugger/Include/pdProcessDebugger.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include <src/pdWin32ProcessDebugger.h>

    // The remote process debugger doesn't exist on 64-bit currently:
    #if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
        #include <src/pdRemoteProcessDebugger.h>
    #endif
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #include <src/pdLinuxProcessDebugger.h>
    #include <src/pdRemoteProcessDebugger.h>
    #if AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT
        #include <src/pdIPhoneDeviceProcessDebugger.h>
    #endif
#else
    #error Unknown build target!
#endif

// Static member initializations:
pdProcessDebuggersManager* pdProcessDebuggersManager::_pMySingleInstance = NULL;

// ---------------------------------------------------------------------------
// Name:        pdProcessDebuggersManager::pdProcessDebuggersManager
// Description: Constructor
// Author:      Uri Shomroni
// Date:        11/8/2009
// ---------------------------------------------------------------------------
pdProcessDebuggersManager::pdProcessDebuggersManager()
{
    // Create and initialize the process debuggers vector:
    _ppProcessDebuggers = new pdProcessDebugger*[PD_NUMBER_OF_PROCESS_DEBUGGER_TYPES];

    for (int i = 0; i < PD_NUMBER_OF_PROCESS_DEBUGGER_TYPES; i++)
    {
        _ppProcessDebuggers[i] = NULL;
    }

    // Create and register the default process debugger for this platform:
    createAndInstallPlatformDefaultProcessDebugger();
}

// ---------------------------------------------------------------------------
// Name:        pdProcessDebuggersManager::~pdProcessDebuggersManager
// Description: Destructor
// Author:      Uri Shomroni
// Date:        11/8/2009
// ---------------------------------------------------------------------------
pdProcessDebuggersManager::~pdProcessDebuggersManager()
{
    // Delete all process debuggers that we created:
    for (int i = 0; i < PD_NUMBER_OF_PROCESS_DEBUGGER_TYPES; i++)
    {
        delete _ppProcessDebuggers[i];
        _ppProcessDebuggers[i] = NULL;
    }

    // Delete the vector:
    delete[] _ppProcessDebuggers;
    _ppProcessDebuggers = NULL;
}

// ---------------------------------------------------------------------------
// Name:        pdProcessDebuggersManager::adjustProcessDebuggerToProcessCreationData
// Description: Finds out which process debugger type fits processCreationData
//              and makes sure it exists and is registered as the process debugger instance.
// Author:      Uri Shomroni
// Date:        11/8/2009
// ---------------------------------------------------------------------------
void pdProcessDebuggersManager::adjustProcessDebuggerToProcessCreationData(const apDebugProjectSettings& processCreationData)
{
    // Get the process debugger that matches this creation data:
    pdProcessDebuggerTypes neededProcessDebuggerType = processDebuggerAppropriateForProcessCreationData(processCreationData);

    // Make sure we got a vaild result:
    GT_IF_WITH_ASSERT(neededProcessDebuggerType < PD_NUMBER_OF_PROCESS_DEBUGGER_TYPES)
    {
        pdProcessDebugger* pAppropriateProcessDebugger = getOrCreateProcessDebuggerByType(neededProcessDebuggerType);

        // Unregister all process debuggers from listening to events:
        apEventsHandler& theEventsHandler = apEventsHandler::instance();

        for (int i = 0; i < PD_NUMBER_OF_PROCESS_DEBUGGER_TYPES; i++)
        {
            pdProcessDebugger* pCurrentProcessDebugger = _ppProcessDebuggers[i];

            if (pCurrentProcessDebugger != NULL)
            {
                theEventsHandler.unregisterEventsObserver(*pCurrentProcessDebugger, false);

                // Process debuggers also listen to events registration:
                theEventsHandler.unregisterEventsRegistrationObserver(*pCurrentProcessDebugger);
            }
        }

        GT_IF_WITH_ASSERT(pAppropriateProcessDebugger != NULL)
        {
            pdProcessDebugger::registerInstance(pAppropriateProcessDebugger);

            // Register the process debugger to listen to events as well:
            theEventsHandler.registerEventsObserver(*pAppropriateProcessDebugger, AP_PROCESS_DEBUGGER_EVENTS_HANDLING_PRIORITY);

            // Process debuggers also listen to events registration:
            theEventsHandler.registerEventsRegistrationObserver(*pAppropriateProcessDebugger);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        pdProcessDebuggersManager::createAndInstallPlatformDefaultProcessDebugger
// Description: Creates the platform's default process debugger and sets it as the
//              pdProcessDebugger::instance();
// Author:      Uri Shomroni
// Date:        12/8/2009
// ---------------------------------------------------------------------------
void pdProcessDebuggersManager::createAndInstallPlatformDefaultProcessDebugger()
{
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    pdProcessDebuggerTypes neededProcessDebuggerType = PD_WINDOWS_PROCESS_DEBUGGER;
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
    pdProcessDebuggerTypes neededProcessDebuggerType = PD_LINUX_PROCESS_DEBUGGER;
#else
#error Unknown Build Target!
#endif

    GT_IF_WITH_ASSERT(neededProcessDebuggerType < PD_NUMBER_OF_PROCESS_DEBUGGER_TYPES)
    {
        pdProcessDebugger* pAppropriateProcessDebugger = getOrCreateProcessDebuggerByType(neededProcessDebuggerType);

        GT_IF_WITH_ASSERT(pAppropriateProcessDebugger != NULL)
        {
            pdProcessDebugger::registerInstance(pAppropriateProcessDebugger);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        pdProcessDebuggersManager::instance
// Description: Returns the instance of this class
// Author:      Uri Shomroni
// Date:        11/8/2009
// ---------------------------------------------------------------------------
pdProcessDebuggersManager& pdProcessDebuggersManager::instance()
{

    return *_pMySingleInstance;
}

// ---------------------------------------------------------------------------
// Name:        pdProcessDebuggersManager::setProcessDebuggerInSlot
// Description: Only to be used in the constructor of a pdProcessDebugger subclass
//              which cannot be created in getOrCreateProcessDebuggerByType (i.e.
//              a class which is defined in a higher-level module)
// Author:      Uri Shomroni
// Date:        19/12/2011
// ---------------------------------------------------------------------------
void pdProcessDebuggersManager::setProcessDebuggerInSlot(pdProcessDebugger& rProcessDebugger, pdProcessDebuggerTypes processDebuggerType)
{
    // Validate everything is as required:
    GT_IF_WITH_ASSERT(_ppProcessDebuggers[processDebuggerType] == NULL)
    {
        _ppProcessDebuggers[processDebuggerType] = &rProcessDebugger;
    }
}

// ---------------------------------------------------------------------------
// Name:        pdProcessDebuggersManager::removeProcessDebuggerFromSlot
// Description: Only to be used in the destructor of a pdProcessDebugger subclass
//              which cannot be created in getOrCreateProcessDebuggerByType (i.e.
//              a class which is defined in a higher-level module)
// Author:      Uri Shomroni
// Date:        19/12/2011
// ---------------------------------------------------------------------------
void pdProcessDebuggersManager::removeProcessDebuggerFromSlot(pdProcessDebugger& rProcessDebugger, pdProcessDebuggerTypes processDebuggerType)
{
    GT_IF_WITH_ASSERT(_ppProcessDebuggers != NULL)
    {
        GT_IF_WITH_ASSERT(_ppProcessDebuggers[processDebuggerType] != NULL)
        {
            GT_IF_WITH_ASSERT(_ppProcessDebuggers[processDebuggerType] == &rProcessDebugger)
            {
                _ppProcessDebuggers[processDebuggerType] = NULL;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        pdProcessDebuggersManager::processDebuggerAppropriateForProcessCreationData
// Description: Return the process debugger type matching the process creation data
//              and the build type:
// Arguments: const apDebugProjectSettings& processCreationData
// Return Val: pdProcessDebuggerTypes
// Author:      Uri Shomroni
// Date:        11/8/2009
// ---------------------------------------------------------------------------
pdProcessDebuggerTypes pdProcessDebuggersManager::processDebuggerAppropriateForProcessCreationData(const apDebugProjectSettings& processCreationData)
{
    pdProcessDebuggerTypes retVal = PD_NUMBER_OF_PROCESS_DEBUGGER_TYPES;

    bool isRemote = processCreationData.isRemoteTarget();

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    // If the Visual Studio Process Debugger is registered, use it. Otherwise, use our implementation:
    if (_ppProcessDebuggers[PD_VISUAL_STUDIO_PROCESS_DEBUGGER] != NULL && (!processCreationData.shouldDisableVSDebugEngine()))
    {
        retVal = PD_VISUAL_STUDIO_PROCESS_DEBUGGER;
    }
    else if (!isRemote)
    {
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
        // 32-bit Windows needs to run a different process debugger depending on if this is a 32-bit debugged app or a 64-bit one:
        bool is64Bit = false;
        bool rc64Bit = osIs64BitModule(processCreationData.executablePath(), is64Bit);
        GT_IF_WITH_ASSERT(rc64Bit)
        {
            int bits = 32;

            if (is64Bit)
            {
                // This is a 64-bit binary. Use the Remote process debugger:
                retVal = PD_REMOTE_PROCESS_DEBUGGER;

                bits = 64;
            }
            else
            {
                // This is a 32-bit binary. Use the Windows process debugger:
                retVal = PD_WINDOWS_PROCESS_DEBUGGER;

                bits = 32;
            }

            // Print the address space size to the log:
            gtString logMsg;
            logMsg.appendFormattedString(PD_STR_debuggingAppAddressSpaceSize, processCreationData.executablePath().asString().asCharArray(), bits);
            OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_INFO);
        }
#elif AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
        // Windows 64 only needs the normal windows process debugger:
        retVal = PD_WINDOWS_PROCESS_DEBUGGER;
#else
#error Unknown address space size
#endif
    }

#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
    // Always use the Linux process debugger:
    retVal = PD_LINUX_PROCESS_DEBUGGER;
#if AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT

    if (processCreationData.projectExecutionTarget() == AP_IPHONE_DEVICE_EXECUTION_TARGET)
    {
        retVal = PD_IPHONE_DEVICE_PROCESS_DEBUGGER;
    }

#endif
#else
#error Unknown build target!
#endif

    // The remote process debugger handles all remote connections:
    if ((PD_VISUAL_STUDIO_PROCESS_DEBUGGER != retVal) && isRemote)
    {
        retVal = PD_REMOTE_PROCESS_DEBUGGER;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdProcessDebuggersManager::getOrCreateProcessDebuggerByType
// Description: Gets the instance of the process debugger type mentioned by processDebuggerType.
//              If we haven't asked for it before, create it
// Author:      Uri Shomroni
// Date:        11/8/2009
// ---------------------------------------------------------------------------
pdProcessDebugger* pdProcessDebuggersManager::getOrCreateProcessDebuggerByType(pdProcessDebuggerTypes processDebuggerType)
{
    pdProcessDebugger* retVal = _ppProcessDebuggers[processDebuggerType];

    // The requested process debugger does not exist yet, create it:
    if (retVal == NULL)
    {
        switch (processDebuggerType)
        {
            case PD_WINDOWS_PROCESS_DEBUGGER:
            {
                // The windows process debugger is available on Windows 32 and 64:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                retVal = new pdWin32ProcessDebugger;
#else
                // We should not get here:
                GT_ASSERT(false);
#endif
            }
            break;

            case PD_REMOTE_PROCESS_DEBUGGER:
            {
                // The windows process debugger is currently available only on Windows 32 and Linux:
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) && (AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE)) || (AMDT_BUILD_TARGET == AMDT_LINUX_OS)
                retVal = new pdRemoteProcessDebugger;
#else
                // We should not get here:
                GT_ASSERT(false);
#endif
            }
            break;

            case PD_LINUX_PROCESS_DEBUGGER:
            {
                // The Linux process debugger is available on Mac and Linux (32 and 64):
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
                retVal = new pdLinuxProcessDebugger();
#else
                // We should not get here:
                GT_ASSERT(false);
#endif
            }
            break;

            case PD_IPHONE_DEVICE_PROCESS_DEBUGGER:
            {
                // The iPhone Device process debugger is available on Mac:
#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
                retVal = new pdIPhoneDeviceProcessDebugger();
#else
                // We should not get here:
                GT_ASSERT(false);
#endif
            }
            break;

            case PD_VISUAL_STUDIO_PROCESS_DEBUGGER:
            {
                // We should not get this process debugger unless it was registered, and only on Windows:
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
                GT_ASSERT(false);
                retVal = getOrCreateProcessDebuggerByType(PD_WINDOWS_PROCESS_DEBUGGER);
#else
                GT_ASSERT(false);
#endif
            }
            break;

            default:
            {
                // Unsupported process debugger type!
                GT_ASSERT(false);
            }
            break;
        }

        // Register the process debugger we just created:
        _ppProcessDebuggers[processDebuggerType] = retVal;
    }


    return retVal;
}

