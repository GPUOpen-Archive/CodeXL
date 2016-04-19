//==============================================================================
// Copyright (c) 2014 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief The web server listens for messages from the client
//==============================================================================

#ifndef GPS_WEBSERVER_INCLUDED
#define GPS_WEBSERVER_INCLUDED

#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTOSWrappers/Include/osCriticalSection.h>
#include "../Common/NamedEvent.h"

class osThread;
class NetSocket;

/// The maximum count of putting response to SM
#define MAX_SEM_COUNT 100

//--------------------------------------------------------------
// WrapperInfo Struct
/// Contains all information necessary to identify a plugin which
/// wraps an API
//--------------------------------------------------------------
struct WrapperInfo
{
    gtASCIIString strPluginName;       ///< the name of the dll
    gtASCIIString strPluginPath;       ///< full path to the dll
    gtASCIIString strPluginVersion;    ///< version string of the plugin
    gtASCIIString strPluginShortDesc;  ///< single-word identifier of the plugin
    gtASCIIString strPluginLongDesc;   ///< one sentence description of the plugin's functionality
    gtASCIIString strWrappedDll;       ///< the dll that gets wrapped by this wrapper
};

extern NamedEvent g_shutdownEvent; ///< Shutdown event - triggered when the webserver is to be shutdown.

extern bool g_bAppSpecifiedAtCmdLine; ///< Bool to record if an application exe has been specified on the command line.

NetSocket* WebServerSetup(int port);

void WebServerCleanup(NetSocket* server_socket);

osThread* ForkAndWaitForClientRequests(NetSocket* server_socket);

DWORD GetWebServerPort();

#endif // GPS_WEBSERVER_INCLUDED
