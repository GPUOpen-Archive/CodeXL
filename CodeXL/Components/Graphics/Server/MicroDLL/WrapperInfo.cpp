//=====================================================================================
// Copyright (c) 2013 Advanced Micro Devices, Inc.
/// \author GPU Developer Tools
/// \file
/// \brief Functions that manage loading of plugins (graphics servers)
//=====================================================================================

#include "WrapperInfo.h"

#include "../Common/logger.h"
#include "../Common/SharedMemoryManager.h"
#include "../Common/ICommunication.h"
#include <vector>

static std::vector< WrapperInfo > g_wrapperArray; ///< Wrapper array

// Functions required by a plugin
static const char* (* GetPluginVersion)(); ///< Function pointer to GetPluginVersion
static const char* (* GetShortDescription)(); ///< Function pointer to GetShortDescription
static const char* (* GetLongDescription)(); ///< Function pointer to GetLongDescription
static bool (* ProcessRequest)(CommunicationID requestID); ///< Function pointer to ProcessRequest

// Functions required by a wrapper
static const char* (* GetWrappedDLLName)(); ///< Function pointer to GetWrappedDLLName
static bool (* UpdateHooks)(); ///< Function pointer to UpdateHooks

//--------------------------------------------------------------
/// Stores information about each of the available wrappers
/// in the wrapper array
//--------------------------------------------------------------
void CollectWrapperInfo()
{
    Log(logMESSAGE, "CollectWrapperInfo.\n");

    // open shared memory to read the wrapper info
    if (smOpen("GPS_TO_MDLL") == false)
    {
        Log(logERROR, "Unable to open shared memory to collect wrapper info.\n");
        return;
    }

    if (smLockGet("GPS_TO_MDLL") == false)
    {
        Log(logERROR, "Unable to lock shared memory to collect wrapper info.\n");
        return;
    }

    char strPath[ PS_MAX_PATH ];
    char strName[ PS_MAX_PATH ];
    char strDlls[ PS_MAX_PATH ];

    // while there is data to get from the shared memory
    while (smGet("GPS_TO_MDLL", NULL, 0) > 0)
    {
        WrapperInfo wi;

        memset(strPath, 0, PS_MAX_PATH);
        memset(strName, 0, PS_MAX_PATH);
        memset(strDlls, 0, PS_MAX_PATH);

        if (smGet("GPS_TO_MDLL", strPath, PS_MAX_PATH) > 0 &&
            smGet("GPS_TO_MDLL", strName, PS_MAX_PATH) > 0 &&
            smGet("GPS_TO_MDLL", strDlls, PS_MAX_PATH) > 0)
        {
            wi.strPluginPath = strPath;
            wi.strPluginName = strName;
            wi.strWrappedDll = strDlls;

            wi.hWrapperLoaded = NULL;

            Log(logMESSAGE, "Wrapper Info: %s, %s, %s\n", wi.strPluginPath.asCharArray(), wi.strPluginName.asCharArray(), wi.strWrappedDll.asCharArray());
            g_wrapperArray.push_back(wi);
        }
        else
        {
            Log(logERROR, "Failed to read all of the wrapper information from shared memory.\n");
        }
    }

    smUnlockGet("GPS_TO_MDLL");

    unsigned long ulNumBuffers = (unsigned long) g_wrapperArray.size() * 3;

    if (smLockPut("GPS_TO_MDLL", ulNumBuffers * PS_MAX_PATH, ulNumBuffers) == false)
    {
        Log(logWARNING, "Failed to lock shared memory prior to repeating wrapper info. Injecting into created processes may not work as expected.\n");
        return;
    }

    // now write the data back out so that if the MicroDLL is injected into a new process,
    // the new instance of MicroDLL can read the list of allowed wrappers
    for (unsigned int i = 0; i < g_wrapperArray.size(); i ++)
    {
        WrapperInfo wi = g_wrapperArray[i];

        memset(strPath, 0, PS_MAX_PATH);
        memset(strName, 0, PS_MAX_PATH);
        memset(strDlls, 0, PS_MAX_PATH);

        strcpy_s(strPath, PS_MAX_PATH, wi.strPluginPath.asCharArray());
        strcpy_s(strName, PS_MAX_PATH, wi.strPluginName.asCharArray());
        strcpy_s(strDlls, PS_MAX_PATH, wi.strWrappedDll.asCharArray());

        if (smPut("GPS_TO_MDLL", (void*) strPath, PS_MAX_PATH) == false ||
            smPut("GPS_TO_MDLL", (void*) strName, PS_MAX_PATH) == false ||
            smPut("GPS_TO_MDLL", (void*) strDlls, PS_MAX_PATH) == false)
        {
            Log(logWARNING, "Failed to repeat wrapper info into the Shared Memory. Injecting into created processes may not work as expected.\n");
        }
    }

    smUnlockPut("GPS_TO_MDLL");
}


//----------------------------------------------
/// Checks to see if a plugin is needed based
/// on the list of dlls wrapped by that plugin.
/// This is done by seeing if the process has a
/// handle to any of the dlls.
/// \param wrappedDLLs string containg a comma-
///    separated list of dll names
/// \return true if the process has a handle to
///    any of the dlls; false otherwise
//----------------------------------------------
static bool IsPluginNeeded(gtASCIIString wrappedDLLs)
{
    std::list< gtASCIIString > dllList;
    wrappedDLLs.Split(",", true, dllList);

    for (const gtASCIIString& iter : dllList)
    {
        HMODULE hMod = GetModuleHandle(iter.asCharArray());

        if (hMod != NULL)
        {
            // DLL wrapped by plugin is still in use
            return (true);
        }
    }

    return (false);
}

//----------------------------------------------
/// Calls UpdateHooks entrypoint of the supplied
/// module (dll handle) so they can handle the
/// load and unload of the dlls being to monitored
/// \param hLib handle to a dll that is loaded in the app
/// \return true if the call to UpdateHooks succeeded; false otherwise
//----------------------------------------------
bool CallUpdateHooks(HMODULE hLib)
{
    *((FARPROC*) &UpdateHooks) = ::GetProcAddress(hLib, "UpdateHooks");

    if ((UpdateHooks != NULL))
    {
        if (UpdateHooks() == true)
        {
            return true;
        }
    }

    //search for dll name
    for (unsigned int i = 0; i < g_wrapperArray.size(); i++)
    {
        if (hLib == g_wrapperArray[ i ].hWrapperLoaded)
        {
            if (UpdateHooks == NULL)
            {
                Log(logERROR, "Could not find UpdateHooks() entry point in %s\n", g_wrapperArray[ i ].strPluginName.asCharArray());
            }
            else
            {
                Log(logERROR, "Call to UpdateHooks() in %s failed\n", g_wrapperArray[ i ].strPluginName.asCharArray());
            }

            return false;
        }
    }

    Log(logERROR, "%s returned false\n", __FUNCTION__);
    return false;
}


//----------------------------------------------
/// Checks if the libraries needed by any of the available
/// plugins are active.  If so then loads the plugin and
/// calls UpdateHooks
//----------------------------------------------
void CheckWrapperOnLoadLibrary(void)
{
    // iterate over all available plugins
    for (unsigned int i = 0; i < g_wrapperArray.size(); i++)
    {
        // is this plugin already loaded
        if (g_wrapperArray[ i ].hWrapperLoaded == NULL)
        {
            // plugin isn't loaded - should it be?
            if (IsPluginNeeded(g_wrapperArray[i].strWrappedDll))
            {
                // yes - then load plugin if necessary and hook library
                // wrapper not loaded - so load it

                gtASCIIString strPluginName = g_wrapperArray[ i ].strPluginName;
                strPluginName += GDT_PROJECT_SUFFIX ".dll";
                //                Log(logMESSAGE, "Loading %s = 0x%p\n", strPluginName.asCharArray(), g_wrapperArray[ i ].hWrapperLoaded);

                gtASCIIString strPluginPath = g_wrapperArray[ i ].strPluginPath;
                strPluginPath += GDT_PROJECT_SUFFIX ".dll";
                g_wrapperArray[ i ].hWrapperLoaded = LoadLibrary(strPluginPath.asCharArray());

                if (g_wrapperArray[ i ].hWrapperLoaded == NULL)
                {
                    Log(logERROR, "Loading %s failed\n", strPluginName.asCharArray());
                }
            }
        }

        // if the plugin is loaded call UpdateHooks so they can update hooks if necessary
        if (g_wrapperArray[ i ].hWrapperLoaded != NULL)
        {
            // so plugin can update hooks
            CallUpdateHooks(g_wrapperArray[ i ].hWrapperLoaded);
        }
    }

    return;
}




//----------------------------------------------
/// Checks if any of the loaded plugins can be unloaded from memory
/// it does this by confirming if the DLL that has been hooked is still loaded. If not
/// then it unloads the plugin from memory.
//----------------------------------------------
void CheckWrapperOnFreeLibrary()
{
    // iterate over all available plugins
    for (unsigned int i = 0; i < g_wrapperArray.size(); i++)
    {
        // Is this plugin currently loaded and still required
        if (g_wrapperArray[ i ].hWrapperLoaded)
        {
            if (IsPluginNeeded(g_wrapperArray[ i ].strWrappedDll) == false)
            {
                // no longer needed - unload the plugin
                gtASCIIString strPluginName = g_wrapperArray[i].strPluginName;
                strPluginName += GDT_PROJECT_SUFFIX ".dll";
                Log(logMESSAGE, "CheckWrapperOnFreeLibrary: Unloading %s\n", strPluginName.asCharArray());
                FreeLibrary(g_wrapperArray[ i ].hWrapperLoaded);
                g_wrapperArray[ i ].hWrapperLoaded = NULL;
            }
        }

        // if the plugin is loaded call UpdateHooks so they can update hooks if necessary
        if (g_wrapperArray[ i ].hWrapperLoaded != NULL)
        {
            // so plugin can update hooks
            CallUpdateHooks(g_wrapperArray[ i ].hWrapperLoaded);
        }
    }
}

//--------------------------------------------------------------
/// Call UpdateHooks() for a replaced Dll (if it's loaded) when
/// the application calls LoadLibrary
//--------------------------------------------------------------
void UpdateHooksOnLoadLibrary()
{
    for (unsigned int i = 0; i < g_wrapperArray.size(); i++)
    {
        // get the list of wrapped dll's
        std::list< gtASCIIString > dllList;
        g_wrapperArray[i].strWrappedDll.Split(",", true, dllList);

        for (const gtASCIIString& iter : dllList)
        {
            // check to see if this wrapped dll has been loaded
            HMODULE hMod = GetModuleHandle(iter.asCharArray());

            if (hMod != NULL)
            {
                // required dll is loaded, so call UpdateHooks()
                CallUpdateHooks(hMod);
            }
        }
    }
}
