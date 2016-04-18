//==============================================================================
// Copyright (c) 2014 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief GPUPerfServer main program loop. Starts the GPUPerfStudio server,
///        reads command line arguments and starts the target application
//==============================================================================

#ifndef GPS_OSDEPENDENT_INCLUDED
#define GPS_OSDEPENDENT_INCLUDED

#include <map>
#include <vector>

#include "../Common/ICommunication.h"
#include "WebServer.h"

/// define a simplified type for accessing wrapper info based on the wrapper's short description
typedef std::map< std::string, WrapperInfo > WrapperMap;

/// collection of the wrappers allowed to be injected into an application
extern WrapperMap g_allowedWrappersMap;

/// collection of wrappers that are actively being used in an application
extern WrapperMap g_activeWrappersMap;

/// Gets the map of available plugins
WrapperMap GetWrapperMap();

/// Adds quotations around the string if it contains spaces
/// \param arg String to process
/// \return the input string with quotations if needed
gtASCIIString AddQuotesIfStringHasSpaces(const char* arg);

/// Generates a list of wrappers which were injected into a specified process
/// \param uPID the process ID to check for wrappers
/// \param rWrapperArray if functions returns true, then contains the wrappers that are in the specified process
/// \return true if a list of wrappers could be obtained; false otherwise
bool GetListOfInjectedWrappers(unsigned long uPID, std::vector< WrapperInfo >& rWrapperArray);

/// Performs OS dependent initialization
bool OSDependentModulesInitialization();

/// Sends XML describing the currently available wrappers
/// \param requestID the ID of the incoming request for wrapper information
/// \param pClientSocket the socket used to send the response
void DoWrappersCommand(CommunicationID requestID, NetSocket* pClientSocket);

// "Attach on the fly" is when the application is running and we connect the server to it
// in the middle of the app execution. We have decided to disable this because there is a lot
// of information we miss by not being attached from the beginning and we were often having
// trouble either attaching or detaching properly.
#ifdef GPS_ENABLE_ATTACH_ON_THE_FLY
    void DoAttachCommand(CommunicationID requestID, char** sCmd);
    void DoDetachCommand(CommunicationID requestID, char** sCmd);
#endif

/// Sends the requested file (relative to the root web directory) to the requestor
/// \param requestID the ID of the incoming request
/// \param file the relative path to a requested file
/// \param pClientSocket the socket used to send the data
void ReturnFileFromWebRoot(CommunicationID requestID, const char* file, NetSocket* pClientSocket);

/// Sends the requested file (relative to the server directory) to the requestor
/// \param requestID the ID of the incoming request
/// \param file the relative path to a requested file
/// \param pClientSocket the socket used to send the data
void ReturnFileFromRoot(CommunicationID requestID, const char* file, NetSocket* pClientSocket);

#endif // GPS_OSDEPENDENT_INCLUDED
