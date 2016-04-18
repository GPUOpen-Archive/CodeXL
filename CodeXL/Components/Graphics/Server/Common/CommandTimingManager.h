//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Debug code used to measure how much time is spent in the different
/// parts of the server as a command is being processed.
//==============================================================================

#ifndef COMMAND_TIMING_MANAGER
#define COMMAND_TIMING_MANAGER

#include "ICommunication.h"
#include "ICommunication_Impl.h"
#include "xml.h"

#include <AMDTBaseTools/Include/gtASCIIString.h>

class NetSocket;

// Uncomment this to activate the server comms performance code on the server.
//#define DEBUG_COMMS_PERFORMANCE

#ifdef DEBUG_COMMS_PERFORMANCE

///////////////////////////////////////////////////////////////////////////////////////
/// This class is responsible for storing an instance of a commands server
/// side timing data.
///////////////////////////////////////////////////////////////////////////////////////
class CommandTiming
{

private :

    /// The socket handle.
    NetSocket* m_Request_id;

    /// URL of the command.
    std::string m_strURL;

    /// The time the command reached the server.
    LARGE_INTEGER m_nWebServerRoundTripStart;

    /// The time the command left the server.
    LARGE_INTEGER m_nWebServerRoundTripEnd;

    /// The time spent in the server.
    LARGE_INTEGER m_nWebServerRoundTripTime;

    /// The time just after the command was sent across the shared memory to the application.
    LARGE_INTEGER m_nPostSharedMemorySend;

    /// The time just after the comand returns from the application.
    LARGE_INTEGER m_nPreSharedMemoryGet;

    /// The time spent ine th shared memory.
    LARGE_INTEGER m_nSharedMemoryTransferTime;

    /// The size of teh data response.
    unsigned long m_uResponseSize;

    /// The number of commands that are currently being processed in one pass of teh server.
    int m_nServerLoadingCount ;

public:

    ///////////////////////////////////////////////////////////////////////////////////////
    /// Constructor
    ///////////////////////////////////////////////////////////////////////////////////////
    CommandTiming::CommandTiming()
    {
        m_Request_id = 0 ;
        m_strURL = '\0';
        m_nWebServerRoundTripStart.QuadPart = 0 ;
        m_nWebServerRoundTripEnd.QuadPart = 0 ;
        m_nWebServerRoundTripTime.QuadPart = 0 ;
        m_nPostSharedMemorySend.QuadPart = 0 ;
        m_nPreSharedMemoryGet.QuadPart = 0;
        m_nSharedMemoryTransferTime.QuadPart = 0;
        m_uResponseSize = 0 ;
        m_nServerLoadingCount = 0 ;
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    /// Gets the Request ID.
    /// \param SOCKET RequestID
    ///////////////////////////////////////////////////////////////////////////////////////
    NetSocket* GetRequestID()
    {
        return m_Request_id;
    }

    ///////////////////////////////////////////////////////////////////
    /// Sets the start time
    /// \param time Current time
    ///////////////////////////////////////////////////////////////////
    void SetWebServerRoundTripStart(LARGE_INTEGER time)
    {
        m_nWebServerRoundTripStart.QuadPart = time.QuadPart;
    }

    ///////////////////////////////////////////////////////////////////
    /// Sets the end time and calculates the round trip value.
    /// \param time Current time
    ///////////////////////////////////////////////////////////////////
    void SetWebServerRoundTripEnd(LARGE_INTEGER time)
    {
        m_nWebServerRoundTripEnd.QuadPart = time.QuadPart ;
        m_nWebServerRoundTripTime.QuadPart = m_nWebServerRoundTripEnd.QuadPart - m_nWebServerRoundTripStart.QuadPart;
    }

    ///////////////////////////////////////////////////////////////////
    /// Sets the  time just after the smPutLock was finished
    /// \param time Current time
    ///////////////////////////////////////////////////////////////////
    void SetPostSharedMemorySend(LARGE_INTEGER time)
    {
        m_nPostSharedMemorySend.QuadPart = time.QuadPart;
    }

    ///////////////////////////////////////////////////////////////////
    /// Sets the  time just before the shared memory get is called.
    /// \param time Current time
    ///////////////////////////////////////////////////////////////////
    void SetPreSharedMemoryGet(LARGE_INTEGER time)
    {
        m_nPreSharedMemoryGet.QuadPart = time.QuadPart;
        m_nSharedMemoryTransferTime.QuadPart =  m_nWebServerRoundTripTime.QuadPart - (m_nPreSharedMemoryGet.QuadPart - m_nPostSharedMemorySend.QuadPart);
    }

    ///////////////////////////////////////////////////////////////////
    /// Sets the response size value to the size of the returned data
    /// \param uResponseSize Cthe data size
    ///////////////////////////////////////////////////////////////////
    void SetResponseSize(unsigned long uResponseSize)
    {
        m_uResponseSize = uResponseSize;
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    /// Output an XML record of this timing object
    /// \param The performance counter frequency
    /// \return SuString of the XML
    ///////////////////////////////////////////////////////////////////////////////////////
    gtASCIIString ToXML(double fFreq)
    {
        gtASCIIString str;

        str += "<CommandTiming>" ;

        str += "<URL>" ;
        str += m_strURL.c_str() ;
        str += "</URL>" ;

        //gtASCIIString strRequestID;
        str += "<RequestID>" ;
        gtASCIIString strRequestID = FormatText("%d", m_Request_id);
        str += strRequestID;
        str += "</RequestID>" ;

        str += "<ServerRoundTripTime>" ;
        gtASCIIString  strTime = FormatText("%lf", (double)m_nWebServerRoundTripTime.QuadPart * fFreq * 1000.0);
        str += strTime;
        str += "</ServerRoundTripTime>" ;

        str += "<SharedMemoryTransferTime>" ;
        gtASCIIString strMemTime = FormatText("%lf", (double)m_nSharedMemoryTransferTime.QuadPart * fFreq * 1000.0);
        str += strMemTime;
        str += "</SharedMemoryTransferTime>" ;

        str += "<ResponseSize>" ;
        gtASCIIString strResponseSize = FormatText("%lu", m_uResponseSize);
        str += strResponseSize;
        str += "</ResponseSize>" ;

        str += "<ServerLoading>" ;
        gtASCIIString strServerLoading = FormatText("%d", m_nServerLoadingCount);
        str += strServerLoading;
        str += "</ServerLoading>" ;

        str += "</CommandTiming>" ;

        return str;
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    /// Set the request ID.
    /// \param nID the Request ID.
    ///////////////////////////////////////////////////////////////////////////////////////
    void SetRequestID(NetSocket* nID)
    {
        m_Request_id = nID;
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    /// Set the URL.
    /// \param nID the Request ID.
    ///////////////////////////////////////////////////////////////////////////////////////
    void SetURL(char* strURL)
    {
        m_strURL = strURL;

        size_t found = m_strURL.find('&');

        if (found != string::npos)
        {
            //cout << "Period found at: " << int(found) << endl;
            m_strURL.erase(found, m_strURL.length() - found);
        }

    }

    ///////////////////////////////////////////////////////////////////////////////////////
    /// Record the server loading count at the time the command was added to the shared memory
    /// \param nCount The current count.
    ///////////////////////////////////////////////////////////////////////////////////////
    void SetServerLoadingCount(int nCount)
    {
        m_nServerLoadingCount = nCount;
    }

};

///////////////////////////////////////////////////////////////////////////////////////
/// This class is responsible for managing the command timing data on the server.
///////////////////////////////////////////////////////////////////////////////////////
class CommandTimingManager
{

public:

    ///////////////////////////////////////////////////////////////////
    /// Singleton access to the instance pointer
    ///////////////////////////////////////////////////////////////////
    static CommandTimingManager* Instance()
    {
        if (s_pInstance == NULL)
        {
            s_pInstance = new CommandTimingManager();
        }

        return s_pInstance;
    }

    ///////////////////////////////////////////////////////////////////
    /// Add a timing object to the pending list
    ///////////////////////////////////////////////////////////////////
    void AddTimingToPendingList(CommandTiming* pTiming)
    {
        m_CommandTimingsPending.push_back(pTiming);
    }

    ///////////////////////////////////////////////////////////////////
    /// Get the timing object associated with input request ID.
    /// \param nID Input Request ID
    ///////////////////////////////////////////////////////////////////
    CommandTiming* GetTimingFromPendingList(NetSocket* nID)
    {
        CommandTimings::const_iterator itr = m_CommandTimingsPending.begin();

        for (itr = m_CommandTimingsPending.begin(); itr != m_CommandTimingsPending.end(); itr++)
        {
            CommandTiming* pTimer = *itr;

            if (pTimer->GetRequestID() == nID)
            {
                return pTimer ;
            }
        }

        return NULL;
    }

    ///////////////////////////////////////////////////////////////////
    /// Helper function that takes the command timing from the pending
    /// list to the returned list.
    ///////////////////////////////////////////////////////////////////
    CommandTiming* HandleResponse(NetSocket* nID)
    {
        // Locate the record in the pending list
        CommandTimings::const_iterator itr = m_CommandTimingsPending.begin();

        for (itr = m_CommandTimingsPending.begin(); itr != m_CommandTimingsPending.end(); itr++)
        {
            CommandTiming* pTimer = *itr;

            if (pTimer->GetRequestID() == nID)
            {
                // Add the timer to the sent list
                m_CommandTimingsReturned.push_back(pTimer);
                m_CommandTimingsPending.erase(itr);
                return pTimer ;
            }
        }

        return NULL;
    }

    ///////////////////////////////////////////////////////////////////
    /// Converts the returned commands list into XML data for the client
    /// \return The XML data
    ///////////////////////////////////////////////////////////////////
    gtASCIIString GetCommandsAsXML()
    {
        gtASCIIString strCommandsData;
        int nCount = 0 ;
        CommandTimings::const_iterator itr = m_CommandTimingsReturned.begin();

        for (itr = m_CommandTimingsReturned.begin(); itr != m_CommandTimingsReturned.end(); itr++)
        {
            CommandTiming* pTimer = *itr;

            if (pTimer != NULL)
            {
                strCommandsData += pTimer->ToXML(m_fPerfCountFreq);
                strCommandsData += "\n";
                nCount++;
            }
        }

        gtASCIIString strAttrib = FormatText("size='%u'", nCount);

        gtASCIIString strFinal = XMLAttrib("Timings", strAttrib.asCharArray(), strCommandsData.asCharArray());
        return strFinal;
    }

    ///////////////////////////////////////////////////////////////////
    /// Clear the command lists and free the data objects
    ///////////////////////////////////////////////////////////////////
    void ClearCommands()
    {
        CommandTimings::const_iterator itr = m_CommandTimingsPending.begin();

        for (itr = m_CommandTimingsPending.begin(); itr != m_CommandTimingsPending.end(); itr++)
        {
            CommandTiming* pTimer = *itr;

            if (pTimer != NULL)
            {
                delete(pTimer);
            }
        }

        itr = m_CommandTimingsReturned.begin();

        for (itr = m_CommandTimingsReturned.begin(); itr != m_CommandTimingsReturned.end(); itr++)
        {
            CommandTiming* pTimer = *itr;

            if (pTimer != NULL)
            {
                delete(pTimer);
            }
        }

        m_CommandTimingsPending.clear();
        m_CommandTimingsReturned.clear();
    }

    ///////////////////////////////////////////////////////////////////
    /// Set the frequency of the performance timer
    /// \param fFreq the frequency
    ///////////////////////////////////////////////////////////////////
    void SetPerformanceCounterFrequency(double fFreq)
    {
        m_fPerfCountFreq = fFreq;
    }

    ///////////////////////////////////////////////////////////////////
    /// Increment the server loading record
    /// \param nIncrement the amount to increment (can be a negative number).
    ///////////////////////////////////////////////////////////////////
    void IncrementServerLoadingCount(int nIncrement)
    {
        m_nServerLoadingCount += nIncrement;
    }

    ///////////////////////////////////////////////////////////////////
    /// Accessor for the server loaing record
    /// \return Integer count
    ///////////////////////////////////////////////////////////////////
    int GetServerLoadingCount()
    {
        return m_nServerLoadingCount;
    }

private:

    ///////////////////////////////////////////////////////////////////
    /// Hidden Constructor
    ///////////////////////////////////////////////////////////////////
    CommandTimingManager::CommandTimingManager()
    {
        m_fPerfCountFreq = 1.0f;
        m_nServerLoadingCount = 0 ;
    }

    ///////////////////////////////////////////////////////////////////
    /// Hidden Destructor
    ///////////////////////////////////////////////////////////////////
    CommandTimingManager::~CommandTimingManager()
    {
        ClearCommands();
    }

    /// Singleton instance pointer
    static CommandTimingManager* s_pInstance ;

    /// Typedef the vector
    typedef std::vector< CommandTiming* > CommandTimings;

    /// A command timing object exists in either the pending or returned list below.
    /// The list of command timings for incoming unprocessed commands
    CommandTimings m_CommandTimingsPending;
    /// The list of command timings for returned commands
    CommandTimings m_CommandTimingsReturned;

    double m_fPerfCountFreq ;

    int m_nServerLoadingCount;
};

#endif

#endif //COMMAND_TIMING_MANAGER