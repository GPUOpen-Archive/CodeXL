//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Declaration of the TraceAnalyzer class which should
///         be utilized by all plugins.
//==============================================================================

#ifndef TRACEANALYZER__H
#define TRACEANALYZER__H

#include "ILayer.h"
#include "CommandProcessor.h"
#include "HTTPLogger.h"
#include <map>
#include <string>
#include "defines.h"
#include "mymutex.h"
#include "TimingLog.h"
#include <AMDTOSWrappers/Include/osThread.h>

/// Stores an id and reference count for a dictionary entry
struct DictEntry
{
    unsigned long m_dwId;      ///< ID of the entry
    unsigned long m_dwCount;   ///< number of times the entry is referenced

    /// Constructor
    /// \param dwId the id of this dictionary entry
    DictEntry(unsigned long dwId = 0)
    {
        m_dwId = dwId;
        m_dwCount = 0;
    }
};


/// Holds keys,value pairs and returns its contents XMLified
class DictKeyUsage
{
    /// map to store the usage of a dictionary entry
    std::map<std::string, DictEntry > Calls;
public:

    /// Adds a dictionary entry
    /// \param str the string to enter into the dictionary
    unsigned long Add(std::string str);

    /// Clears the current dicionary entry
    void Clear();

    /// Accessor to the underlying map
    /// \return the map containing the dictionary entries and their usage
    std::map<std::string, DictEntry>& GetMap();

    /// Accessor the map as XML
    /// \return string containing the dictionary in XML format
    std::string GetData();
};

//=============================================================================
/// Collects api trace
//=============================================================================
class TraceAnalyzer: public ILayer, public CommandProcessor
{
private:
    mutex m_mutex;      ///< mutex to ensure only one API call is added at a time

    GPS_TIMESTAMP m_startTime; ///< the time the call is added

public:

    /// Default constructor
    TraceAnalyzer();

    /// Destructor
    virtual ~TraceAnalyzer() {};

    /// BeforeAPICall - set up anything that needs initializing before capturing an API call
    virtual void BeforeAPICall()
    {
        if (m_bCollectingTimingLog)
        {
            m_startTime = m_apiCallTimer.GetRaw();
        }
    }

    //-----------------------------------------------------------------------------
    /// Adds XML for the supplied API call to the APITrace log.
    /// This should only be called if the apiTraceXML command is active.
    /// \param pstrDevice the device that the call was made on
    /// \param pstrInterface the interface that the call is related to
    /// \param pstrFunction the name of the API entrypoint
    /// \param pstrParameters the parameters supplied to the function call
    /// \param pstrReturnValue the return value of the function call
    //-----------------------------------------------------------------------------
    void AddAPICall(const char* pstrDevice,
                    const char* pstrInterface,
                    const char* pstrFunction,
                    const char* pstrParameters,
                    const char* pstrReturnValue);

    //-----------------------------------------------------------------------------
    /// Indicates that the TraceAnalyzer is collecing API calls
    /// \return true if API calls should be added; false otherwise
    //-----------------------------------------------------------------------------
    bool IsCollectingAPICalls()
    {
        return m_apiTraceXML.IsActive() ||
               m_apiTraceTXT.IsActive() ||
               m_cmdTimingLog.IsActive();
    }

    //-----------------------------------------------------------------------------
    /// Signals the trace analyzer that a frame is beginning and it should
    /// initialize based on any received commands
    //-----------------------------------------------------------------------------
    virtual void BeginFrame();

    //-----------------------------------------------------------------------------
    /// Signals the trace analyzer that the frame is ended and any collected data should be sent
    //-----------------------------------------------------------------------------
    virtual void EndFrame();

    //-----------------------------------------------------------------------------
    /// Adds Debug messages to the API trace.
    /// \param str The string message to add to the API trace before the next API call.
    //-----------------------------------------------------------------------------
    void AddDebugString(std::string str)
    {
        if (m_bCollectingTimingLog)
        {
            m_apiCallTimer.Add(osGetCurrentThreadId(), m_startTime);
        }

        m_strDebug.push_back(str);
    }

    //-----------------------------------------------------------
    /// Clears the current API trace string
    //-----------------------------------------------------------
    void ClearOutputDebugString()
    {
        m_strDebug.clear();
    }

    //-----------------------------------------------------------
    /// Gets the API Trace's mutex which should be used to ensure
    /// that multiple threads do not call into the API entrypoints
    /// at the same time.
    //-----------------------------------------------------------
    mutex& GetMutex() { return m_APITraceMutex; }

protected:

    //-----------------------------------------------------------------------------
    /// Gets the syntax color for the specified function
    /// \param pStr The API call
    /// \return A long int color value
    //-----------------------------------------------------------------------------
    virtual unsigned long GetSyntaxColour(const char* pStr);

    //-----------------------------------------------------------------------------
    /// Gets the currently active device.
    /// \return A pointer to the currently active device.
    //-----------------------------------------------------------------------------
    virtual void* GetActiveDevice() = 0;

    //--------------------------------------------------------------------------
    /// Clears out old trace data. Implement a custom clear in subclasses.
    //--------------------------------------------------------------------------
    virtual void Clear();

    /// Command to collect API calls in XML and send back to the client
    CommandResponse m_apiTraceXML;

    /// Command to collect API calls in text and send back to the client.
    /// This also causes the timing log data to be collected, but not sent to client.
    CommandResponse m_apiTraceTXT;

    /// Command to request the timing log to be sent back to the client.
    /// If the data was not already collected, it will be collected, then sent.
    CommandResponse m_cmdTimingLog;

    /// Indicates whether the timing log should be collected.
    bool m_bCollectingTimingLog;

private:

    /// mutex which should be used to ensure that multiple threads do not call
    /// into the API entrypoints at the same time.
    mutex m_APITraceMutex;

    /// The current Output Debug String.
    std::vector<std::string> m_strDebug;

    /// Dictionary to store the API Calls that are collected for XML response
    DictKeyUsage m_Calls;

    /// Dictionary to store the parameters to the API Calls that are collected for XML response
    DictKeyUsage m_Params;

    /// Stores indices for a pair of API call and parameters for each line of the XML API Trace
    std::vector < std::pair < unsigned long, unsigned long > > m_Lines;

    /// A string version of the API Trace.
    std::string m_apiTraceString;

    /// Logs the thread and time that calls are made
    TimingLog m_apiCallTimer;

    /// Generates and returns the APITrace in an XML format
    /// \return an XML encoding of the API Trace
    std::string GetAPITrace();
};



#endif //TRACEANALYZER__H
