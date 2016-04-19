//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Class used to track a specific request being processed byu the server.
//==============================================================================

#ifndef REQUEST_IN_FLIGHT_H_
#define REQUEST_IN_FLIGHT_H_

#include "../Common/HTTPRequest.h"

/// Struct used to record which commands are in flight.
class RequestInFlight
{

public:

    /// Constructor
    /// \param pRequestHeader Input request
    /// \param pClientSocket Client socket
    RequestInFlight(HTTPRequestHeader* pRequestHeader, NetSocket* pClientSocket)
    {
        m_pRequestHeader = pRequestHeader;
        m_pClientSocket = pClientSocket;
    }

private:

    HTTPRequestHeader* m_pRequestHeader; ///< Stores the request in flight

    NetSocket* m_pClientSocket; ///< STores teh client socket

};

/// Record the requests that are sent to the server
typedef std::map<NetSocket*, RequestInFlight*> RequestsInFlightDB;

#endif // REQUEST_IN_FLIGHT_H_