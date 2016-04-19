//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Interface for the Plugins to implement which process requests. It is
///         indended that the implementor is the consumer of the ICommunication.h file
//==============================================================================

#ifndef GPS_PROCESSREQUEST_INTERFACE
#define GPS_PROCESSREQUEST_INTERFACE

// in order to be able to process requests, the code must be able to
// initiallize the communication channel
#include "ICommunication.h"

#ifdef GPS_PLUGIN_EXPORTS
    #define GPS_PLUGIN_API extern "C" __declspec( dllexport ) ///< DLL Export Definition
#elif defined GPS_PLUGIN_STATIC
    #define GPS_PLUGIN_API extern "C" __attribute__ ((visibility ("default"))) ///< Export Definition
#else
    #define GPS_PLUGIN_API extern "C" __declspec( dllimport ) ///< DLL Import Definition
#endif

//=============================================================================
//
// EntryPoints for Receiving Requests
//
//=============================================================================

//-----------------------------------------------------------------------------
/// ProcessRequests
///
/// This function must be implemented by server-side plugins so that they can
/// receive requests from the client. Global plugins will have this function
/// called directly by the framework, while it will be used as a callback after
/// wrapper plugins call GetPendingRequests(...) (see ICommunication.h)
///
/// \param requestID The ID of a request that is in waiting to be processed.
///  This ID can be used to query the contents of an incoming request.
///
/// \return true if the request could be processed; false otherwise
//-----------------------------------------------------------------------------
GPS_PLUGIN_API bool ProcessRequest(CommunicationID requestID);

#endif // GPS_PROCESSREQUEST_INTERFACE
