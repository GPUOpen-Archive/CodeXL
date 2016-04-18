//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Simple stream-based logger. Use for directed debugging only.
///         Don't use in any release code.
//==============================================================================

#ifndef STREAM_LOG_H
#define STREAM_LOG_H

#include <fstream>
#include <iostream>
#include <sstream>
#include <map>

#if defined (_WIN32)
    #include "Windows.h"
#endif

using namespace std;

// Uncomment if you want to use stream logging
#if defined (_WIN32)
    #define USE_STREAMLOG
#else
    //#define USE_STREAMLOG
#endif

#ifdef USE_STREAMLOG

/// A structure to store items specific to logging for a specific thread
struct ThreadLogger
{
    string name;        /// Name of the log file
    long threadID;      /// The ID of the thread
    fstream* logFile;   /// file pointer
};

/// Map of loggers based in thread ID
typedef std::map<long, ThreadLogger*> ThreadLoggersMap;

/// Iterator for logger map
typedef std::map<long, ThreadLogger*>::iterator ThreadLoggersMapIter;

///////////////////////////////////////////////////////////////////////////////////////////
/// Simple stream-based logger. Use for directed debugging only.
/// Don't use in any release code.
///////////////////////////////////////////////////////////////////////////////////////////
class StreamLog
{
private:

    /// Map of loggers - one for each active thread that writes a log enrty
    ThreadLoggersMap m_threadLoggers;

    /// Base name of the log file
    string m_baseNamePath;

public:

    ///////////////////////////////////////////////////////////////////////////////////////////
    /// Constructor
    ///////////////////////////////////////////////////////////////////////////////////////////
    StreamLog()
    {
        /// Default location fo the log. Can be changed by using SetBaseNamePath()
        m_baseNamePath = "C:\\Temp\\GPS";
    }

    ///////////////////////////////////////////////////////////////////////////////////////////
    /// Destructor
    ///////////////////////////////////////////////////////////////////////////////////////////
    ~StreamLog()
    {
        ThreadLoggersMapIter iter = m_threadLoggers.begin();

        while (iter != m_threadLoggers.end())
        {
            ThreadLogger* pLogger = iter->second;

            if (pLogger != NULL)
            {
                if (pLogger->logFile != NULL)
                {
                    pLogger->logFile->close();
                }

                delete pLogger;
            }

            ++iter;
        }

        m_threadLoggers.clear();
    }

    /// Sets the base name to where the log files will be output to.
    /// \param path The path and prefix name of the log files.
    void SetBaseNamePath(const char* path)
    {
        m_baseNamePath = path;
    }

    /// Log a message
    /// \param msg the message to log
    void LogMsg(char* msg)
    {
        *this << msg;
    }

    /// Log a message
    /// \param msg the message to log
    void LogMsg(const char* msg)
    {
        *this << msg;
    }

    /// Log a message
    /// \param msg the message to log
    void LogMsg(std::stringstream& msg)
    {
        *this << msg.str().c_str();
    }

public:

    ///////////////////////////////////////////////////////////////////////////////////////////
    /// Templatized overloading of the << operator
    ///////////////////////////////////////////////////////////////////////////////////////////
    template <class T>
    StreamLog& operator<<(T msg)
    {
        LARGE_INTEGER nTimeNow;
        QueryPerformanceCounter(&nTimeNow);

        DWORD threadId = ::GetCurrentThreadId();

        ThreadLogger* pLogger = GetThreadLogger(threadId);

        if (pLogger != NULL)
        {
            *(pLogger->logFile) << nTimeNow.QuadPart << " " << threadId << " " << msg << "\n";
            pLogger->logFile->flush();
        }

        return *this;
    }

    /// Gets a specific logger based on thread id.
    /// \param threadID The thread ID
    ThreadLogger* GetThreadLogger(long threadID)
    {
        ThreadLogger* pLogger = NULL;

        ThreadLoggersMapIter iter = m_threadLoggers.find(threadID);

        if (iter == m_threadLoggers.end())
        {
            pLogger = new ThreadLogger();

            // assign the threadID
            pLogger->threadID = threadID;
            // Create a unique name based on the basename and thread id

            stringstream tt;
            tt << threadID;
            string tidStr = tt.str();

            string fileName = m_baseNamePath + "-" + tidStr + ".txt";
            pLogger->name = fileName;
            pLogger->logFile = new fstream(fileName, fstream::out);

            // Add new item to the map
            m_threadLoggers[threadID] = pLogger;
        }
        else
        {
            pLogger = iter->second;
        }

        return pLogger;
    }
};

#endif //USE_STREAMLOG

#endif //STREAM_LOG_H