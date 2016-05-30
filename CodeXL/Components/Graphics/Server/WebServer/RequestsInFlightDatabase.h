//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Responsible for tracking commands that are currently in the shared memory
/// being processed by the graphics server
//==============================================================================

#include "RequestInFlight.h"
#include "../Common/NetSocket.h"
#include "ProcessTracker.h"

#ifndef REQUESTS_IN_FLIGHT_DATABASE_H_
#define REQUESTS_IN_FLIGHT_DATABASE_H_

/// Record the requests that are sent to the server
typedef std::map<NetSocket*, RequestInFlight*> RequestsInFlightDB;
typedef std::map<NetSocket*, RequestInFlight*>::iterator RequestsInFlightDBIter;

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

        // Used for debugging
        Log(logMESSAGE, "Adding request socket 0x%p\n", pSocket);
    }

    /// Remove a tracked request from this DB
    /// \param pSocket Input socket
    void Remove(NetSocket* pSocket)
    {
        ScopeLock sc(m_mutex);
        m_RequestsInFlight.erase(pSocket);

        // Used for debugging
        Log(logMESSAGE, "Removing request socket 0x%p\n", pSocket);
    }

    /// Get a count of the number of requests in flight
    /// \return Number of requests in flight
    int InFlightCount()
    {
        return (int)m_RequestsInFlight.size();
    }

    /// Sends the stalled state to all commands in flight
    void SendServerStalledStateToAll()
    {
        ScopeLock sc(m_mutex);
        RequestsInFlightDBIter iter = m_RequestsInFlight.begin();
        while (iter != m_RequestsInFlight.end())
        {
            RequestInFlight* pRequest = iter->second;

            ProcessTracker::Instance()->HandleServerStatusResponse(GRAPHICS_SERVER_STATE_PROCESS_NOT_RUNNING, pRequest->m_pRequestHeader, pRequest->m_pClientSocket);

            ++iter;
        }
    }

    /// Checks to see if the process that a request was sent to is still running.
    void CheckProcessesAreRunning()
    {
        ScopeLock sc(m_mutex);
        RequestsInFlightDBIter iter = m_RequestsInFlight.begin();
        while (iter != m_RequestsInFlight.end())
        {
            NetSocket* pSock = iter->first;
            RequestInFlight* pRequest = iter->second;

            unsigned long pid = pRequest->m_processID;
            bool isAlive = true;
            osIsProcessAlive((DWORD)pid, isAlive);

            Log(logMESSAGE, "CheckProcessesAreRunning:: Checking for process %ld\n", pid);

            if (isAlive == false)
            {
                Log(logMESSAGE, "CheckProcessesAreRunning::Process %ld is not running: URL = %s\n", pid, pRequest->m_pRequestHeader->GetUrl());
                // Need to return the correct error data for process not running
                ProcessTracker::Instance()->HandleServerStatusResponse(GRAPHICS_SERVER_STATE_PROCESS_NOT_RUNNING, pRequest->m_pRequestHeader, pRequest->m_pClientSocket);
                // Clear the request
                m_RequestsInFlight.erase(pSock);
            }
            else
            {
                Log(logMESSAGE, "CheckProcessesAreRunning:: Process %ld is running\n", pid);
            }

            ++iter;
        }
    }

private:

    /// Requests DB
    RequestsInFlightDB m_RequestsInFlight;

    /// Mutex to control access from multiple worker threads
    mutex m_mutex;
};



#endif // REQUESTS_IN_FLIGHT_DATABASE_H_