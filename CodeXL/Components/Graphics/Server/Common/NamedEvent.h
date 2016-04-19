//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  An interface to a named event class. The event is
///         global systemwide, so can be accessed across threads and processes
///         by its name. Each process will need its own copy of the event
///
///         The current implementation expected by perf studio is a manual-reset
///         event. Events have to be explicitly set to the non-signaled state.
///         This means that multiple waiting threads can respond to the signaled
///         state until it is reset
///         This system will have to change as it isn't supported by Linux, which
///         uses auto-reset, meaning that one waiting thread will be woken up
///         and the event will be set to non-signaled automatically.
//==============================================================================

#ifndef GPS_NAMEDEVENT_H
#define GPS_NAMEDEVENT_H

#include "misc.h"

/// forward declaration to implementation class
class NamedEventImpl;

/// NamedEvent wrapper class
class NamedEvent
{
public:
    NamedEvent();
    ~NamedEvent();

    //--------------------------------------------------------------------------
    /// Create a system-wide event
    /// \param eventName the name this event will be known by
    /// \param signaled if true, the event will be created in the signaled state
    ///
    /// \return true if Create succeeded, false if error
    //--------------------------------------------------------------------------
    bool  Create(const char* eventName, bool signaled = false);

    //--------------------------------------------------------------------------
    /// Open a previously created system-wide event
    /// \param eventName the name this event is be known by (the name it was
    /// given when created)
    /// \param inherit if true, processes created by this process will inherit
    /// the event
    ///
    /// \return true if Open succeeded, false if event doesn't exist
    //--------------------------------------------------------------------------
    bool  Open(const char* eventName, bool inherit = false);

    //--------------------------------------------------------------------------
    /// Wait for a signal/event to happen. This function won't return until it
    /// receives a signal
    ///
    /// \return true if successful, false if error
    //--------------------------------------------------------------------------
    bool  Wait();

    //--------------------------------------------------------------------------
    /// Set the event to the signaled state. Any other threads/processes waiting
    /// for the signal can now proceed.
    ///
    /// \return true if successful, false if error
    //--------------------------------------------------------------------------
    bool  Signal();

    //--------------------------------------------------------------------------
    /// Is the event in the Signaled state. Does not block or wait but returns
    /// immediately
    ///
    /// \return true if currently signaled, false if not
    //--------------------------------------------------------------------------
    bool  IsSignaled();

    //--------------------------------------------------------------------------
    /// reset the event to non-signaled
    //--------------------------------------------------------------------------
    void  Reset();

    //--------------------------------------------------------------------------
    /// close the event
    //--------------------------------------------------------------------------
    void  Close();

private:
    /// copy constructor made private; Prevent making copies of this object
    NamedEvent(const NamedEvent& rhs)
    {
        PS_UNREFERENCED_PARAMETER(rhs);
    }

    /// assignment operator made private; Prevent making copies of this object
    NamedEvent& operator= (const NamedEvent& rhs)
    {
        PS_UNREFERENCED_PARAMETER(rhs);
        return *this;
    }

    /// pointer to implementation. Dependent on platform
    NamedEventImpl*   m_pImpl;
};

#endif // GPS_NAMEDEVENT_H
