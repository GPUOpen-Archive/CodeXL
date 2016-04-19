//==============================================================================
// Copyright (c) 2014 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Process web commands received from the client
//==============================================================================

// from Common
#include "../Common/parser.h"
#include "../Common/Logger.h"
#include "../Common/CommandTimingManager.h"
#include "../Common/PerfStudioServer_Version.h"
#include "../Common/HTTPRequest.h"
#include "../Common/SharedGlobal.h"

#include "OSDependent.h"
#include "ProcessTracker.h"
#include "Registry.h"

#include "Commands.h"

/// A map of wrappers (identified by pid and wrapper name) that have been injected into processes
WrapperMap g_activeWrappersMap;

/// get any newly active wrappers from the shared memory and add them to the list
static void GetActiveWrappers()
{
    if (smLockGet("ActivePlugins") == false)
    {
        return;
    }

    // get any newly active wrappers from the shared memory and add them to the list
    while (smGet("ActivePlugins", NULL, 0) > 0)
    {
        char strPidAndPluginShortDesc[ PS_MAX_PATH ];

        if (smGet("ActivePlugins", strPidAndPluginShortDesc, PS_MAX_PATH) > 0)
        {
            // make sure that this wrapper isn't already in the injected list
            if (g_activeWrappersMap.find(strPidAndPluginShortDesc) != g_activeWrappersMap.end())
            {
                Log(logERROR, "Plugin '%s' is already active, cannot make another instance of it active\n", strPidAndPluginShortDesc);
                continue;
            }

            // split the PID and the plugin name
            unsigned long pid = 0;
            char pluginShortDesc[ PS_MAX_PATH ];
            sscanf_s(strPidAndPluginShortDesc, "%lu/%s", &pid, pluginShortDesc, int(sizeof(pluginShortDesc)));

            WrapperMap::iterator iterWrapper = g_allowedWrappersMap.find(pluginShortDesc);

            if (iterWrapper != g_allowedWrappersMap.end())
            {
                // create a shared memory to talk to the plugin
#ifdef WIN32
                char* memoryName = strPidAndPluginShortDesc;
#else
                // the '/' character can't be used as a filename in Linux, so just use the plugin name as the shared memory name
                // (ignore the process ID)
                char memoryName[PS_MAX_PATH];
                sprintf_s(memoryName, PS_MAX_PATH, "%lu %s", pid, pluginShortDesc);
#endif

                if (smCreate(memoryName, 100, sizeof(HTTPRequestHeader)) == false)
                {
                    LogConsole(logERROR, "Can't create shared memory for active plugin '%s', commands cannot be sent to it\n", memoryName);
                }
                else
                {
                    // only add the plugin if we were able to create a shared memory
                    // for it, otherwise we won't be able to communicate with the plugin
                    g_activeWrappersMap[strPidAndPluginShortDesc] = iterWrapper->second;
                }

                continue;
            }
        }
    }

    smUnlockGet("ActivePlugins");
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Handle the GET messages
/// \param pRequestHeader The request header to handle
/// \param pClientSocket the socket used to send the response
/// \param renderLoopStalled A bool that lets you know if the render loop has stalled.
////////////////////////////////////////////////////////////////////////////////////////////
void ProcessGetMethod(HTTPRequestHeader* pRequestHeader, NetSocket* pClientSocket, bool renderLoopStalled)
{
    char* ptr = pRequestHeader->GetUrl();
    char* sCmd = &ptr[1];
    size_t CommandLength = strlen(sCmd);
    bool removeRequest = true;

    GetActiveWrappers();

    // make a call into the communication interface to open the response
    CommunicationID requestID = CreateRequest(pRequestHeader, true);

    // keep a backup of the request ID; Calling send*() resets requestID to 0
    CommunicationID removeRequestID = requestID;

    if (IsToken(&sCmd, "log.txt"))
    {
        SendFileResponse(requestID, GetLogFilename(), pClientSocket);
    }
    else if (IsToken(&sCmd, "help.html"))
    {
        SendRedirectResponse(requestID, "html/index.html", pClientSocket);
    }
    else if (IsToken(&sCmd, "process.xml"))
    {
        SendXMLResponse(requestID, ProcessTracker::Instance()->GetProcessesXML().asCharArray(), pClientSocket);
    }
    else if (IsToken(&sCmd, "wrappers.xml"))
    {
        DoWrappersCommand(requestID, pClientSocket);
    }
    else if (IsToken(&sCmd, "favicon.ico"))
    {
        ReturnFileFromWebRoot(requestID, "favicon.ico", pClientSocket);
    }
    else if (IsToken(&sCmd, "page/"))
    {
        // remove parameters

        char* pch = strchr(sCmd, '?');

        if (pch)
        {
            *pch = 0;
            ReturnFileFromWebRoot(requestID, sCmd, pClientSocket);
        }
        else
        {
            ReturnFileFromWebRoot(requestID, sCmd, pClientSocket);
        }
    }
    else if (IsToken(&sCmd, "version"))
    {
        SendTextResponse(requestID, PERFSTUDIOSERVER_VERSION_STRING, pClientSocket);
    }
    else if (IsToken(&sCmd, "Shutdown"))
    {
        ProcessTracker::Instance()->KillAllProcesses();
        SendTextResponse(requestID, "OK", pClientSocket);
        g_shutdownEvent.Signal();
    }
    else if (IsToken(&sCmd, "CommandTree.xml"))
    {
        gtASCIIString strXML;

        // the server can be shutdown, so add that command
        strXML += XMLAttrib("shutdown", "name='Shutdown Server' url='Shutdown'");

        // no add commands for all of the attached processes
        strXML += ProcessTracker::Instance()->GetCommandTree().c_str();

        SendXMLResponse(requestID, XML("CommandTree", strXML.asCharArray()).asCharArray(), pClientSocket);
    }
    else if (CommandLength == 0)
    {
        SendRedirectResponse(requestID, "html/index.html", pClientSocket);
    }
    else if (IsToken(&sCmd, "GetWebCommandTimings"))
    {
#ifdef DEBUG_COMMS_PERFORMANCE
        gtASCIIString CommandsData = CommandTimingManager::Instance()->GetCommandsAsXML();
        SendXMLResponse(requestID, CommandsData.asCharArray(), pClientSocket);
#else
        SendTextResponse(requestID, "Feature not enabled", pClientSocket);
#endif
    }
    else if (IsToken(&sCmd, "ClearWebCommandTimings"))
    {
#ifdef DEBUG_COMMS_PERFORMANCE
        CommandTimingManager::Instance()->ClearCommands();
        SendTextResponse(requestID, "OK", pClientSocket);
#else
        SendTextResponse(requestID, "Feature not enabled", pClientSocket);
#endif
    }
    else if (ProcessTracker::Instance()->HandleRequest(pRequestHeader, requestID, pClientSocket, renderLoopStalled))
    {
        // ProcessTracker does all the work, don't need to do anything.
        // Don't remove the request yet since the request is still being used by the plugin. It will be
        // removed after the plugin has responded.
        removeRequest = false;
    }
    else
    {
        // Since it hasn't matched any of the expected commands,
        // treat the command the way any webserver would - assume it is a request for a file, and return the file.
        Log(logWARNING, "Requesting file from server root directory: %s.\n", sCmd);
        ReturnFileFromRoot(requestID, sCmd, pClientSocket);
    }

    if (removeRequest)
    {
        ProcessTracker::Instance()->RemoveSocketFromMap(removeRequestID);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Handle the POST messages
/// \param pRequestHeader The request header to handle
/// \param pClientSocket the socket used to send the response
/// \param renderLoopStalled A bool that lets you know if the render loop has stalled.
////////////////////////////////////////////////////////////////////////////////////////////
void ProcessPostMethod(HTTPRequestHeader* pRequestHeader, NetSocket* pClientSocket, bool renderLoopStalled)
{
    char* ptr = pRequestHeader->GetUrl();
    char* sCmd = &ptr[1];

    GetActiveWrappers();

    // make a call into the communication interface to open the response
    CommunicationID requestID = CreateRequest(pRequestHeader, true);

    if (ProcessTracker::Instance()->HandleRequest(pRequestHeader, requestID, pClientSocket, renderLoopStalled))
    {
        // ProcessTracker does all the work, don't need to do anything.
    }
    else
    {
        // Since it hasn't matched any of the expected commands,
        // treat the command the way any webserver would - assume it is a request for a file, and return the file.
        Log(logWARNING, "Requesting file from server root directory: %s.\n", sCmd);
        ReturnFileFromRoot(requestID, sCmd, pClientSocket);
    }
}
