//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Responsible for tracking commands that are currently in the shared memory
/// being processed by the graphics server
//==============================================================================

#include "RequestInFlight.h"
#include "../Common/NetSocket.h"

#ifndef REQUESTS_IN_FLIGHT_DATABASE_H_
#define REQUESTS_IN_FLIGHT_DATABASE_H_

/// Record the requests that are sent to the server
typedef std::map<NetSocket*, RequestInFlight*> RequestsInFlightDB;

/// Responsible for tracking commands that are currently in the shared memory
/// being processed by the graphics server
class RequestsInFlightDatabase : public TSingleton< RequestsInFlightDatabase >
{
    /// TSingleton needs to be able to use our constructor.
    friend class TSingleton<RequestsInFlightDatabase>;

protected:

    /// Constructor
    RequestsInFlightDatabase()
    {
    }

    /// Destructor
    ~RequestsInFlightDatabase()
    {
        m_RequestsInFlight.clear();
    }

public:

    /// Add a request to the DB
    /// \param pSocket Socket info
    /// \param pRequest Request info
    void Add(NetSocket* pSocket, RequestInFlight* pRequest)
    {
        ScopeLock sc(m_mutex);
        m_RequestsInFlight[pSocket] = pRequest;
    }

    /// Remove a tracked request from this DB
    /// \param pSocket Input socket
    void Remove(NetSocket* pSocket)
    {
        ScopeLock sc(m_mutex);
        m_RequestsInFlight.erase(pSocket);
    }

private:

    /// Requests DB
    RequestsInFlightDB m_RequestsInFlight;

    /// Mutex to control access from multiple worker threads
    mutex m_mutex;
};



#endif // REQUESTS_IN_FLIGHT_DATABASE_H_