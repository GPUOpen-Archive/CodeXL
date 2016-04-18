//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Base class used in all derived API specific capture objects
//==============================================================================

#ifndef CAPTURE_H
#define CAPTURE_H
#include <string>
#include <list>
#include <AMDTOSWrappers/Include/osThread.h>
#if defined (_LINUX)
    #include <sys/time.h>
#endif // _LINUX

#include "CaptureClassTypes.h"

class Capture;

/// List of captures
typedef std::list<Capture*> CaptureList;

/// This class is responsible for providing override behaviour for Capture objects
class CaptureOverride
{

public:

    /// The Do function plays the capture by default but can be overriden to provide different functionality.
    /// \param pCap The capture object
    /// \return True if success, False if fail.
    virtual bool Do(Capture* pCap);
};


//=============================================================================
/// Every captured call and its parameters must be put into a class that derives from this one
/// The function parameters have to be set with a function called OnCreate( <params> )
//=============================================================================
class Capture
{
    /// The Timestamp
    LARGE_INTEGER TimeStamp;

    /// The thread ID
    DWORD dwThreadID;

public:

    /// Constructor, captures by default the threadID and the time stamp
    Capture()
    {
        dwThreadID = osGetCurrentThreadId();
#if defined (_WIN32)
        QueryPerformanceCounter(&TimeStamp);
#elif defined (_LINUX)
        struct timeval tv;
        gettimeofday(&tv, NULL);
        TimeStamp = (GPS_TIMESTAMP)tv.tv_sec * 1000000 + (GPS_TIMESTAMP)tv.tv_usec;
#endif
    }

    /// Get the thread ID
    /// \return the thread ID
    DWORD GetThreadID()
    {
        return dwThreadID;
    }

    /// Get the timestamp
    /// \return the time stamp
    LARGE_INTEGER GetTimeStamp()
    {
        return TimeStamp;
    }

    /// place where to release all that was allocated on OnCreate
    virtual ~Capture() {};

    /// to be overriden, the derived class must execute the captured call
    virtual bool Play() = 0;

    /// Allows to apply an override to the current class
    /// \param pCO
    virtual bool PlayOverride(CaptureOverride* pCO)
    {
        return pCO->Do(this);
    }

    /// to be overriden, here the derived class must print the function call and its parameters,
    /// this string is what will be displayed in the API trace window
    virtual std::string Print() = 0;
};



#endif