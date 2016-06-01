//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  An interface to a named semaphore class. The semaphore
///         is global systemwide, so can be accessed across threads and processes
///         by its name. Each process will need its own copy of the semaphore
//==============================================================================

#ifndef GPS_NAMEDSEMAPHORE_H
#define GPS_NAMEDSEMAPHORE_H

#include "misc.h"

/// forward declaration to implementation class
class NamedSemaphoreImpl;

/// Name Semaphore wrapper class
class NamedSemaphore
{
public:
    NamedSemaphore();
    ~NamedSemaphore();

    //--------------------------------------------------------------------------
    /// Create a system-wide semaphore
    /// \param semaphoreName the name this semaphore will be known by
    ///
    /// \return true if Create succeeded, false if error
    //--------------------------------------------------------------------------
    bool  Create(const char* semaphoreName);

    //--------------------------------------------------------------------------
    /// Open a previously created system-wide semaphore
    /// \param semaphoreName the name this semaphore is be known by (the name it was
    /// given when created)
    ///
    /// \return true if Open succeeded, false if semaphore doesn't exist
    //--------------------------------------------------------------------------
    bool  Open(const char* semaphoreName);

    //--------------------------------------------------------------------------
    /// Wait for a semaphore to be signaled. This function won't return until it
    /// receives a signal
    ///
    /// \return true if successful, false if error
    //--------------------------------------------------------------------------
    bool  Wait();

    //--------------------------------------------------------------------------
    /// Signal the semaphore. Adds 1 to its count. Any other threads/processes
    /// waiting for the signal can now proceed if the count is greater than 0.
    /// The task of waking waiting threads and allowing them to proceed is
    /// the responsibility of the OS
    ///
    /// \return true if successful, false if error
    //--------------------------------------------------------------------------
    bool  Signal();

    //--------------------------------------------------------------------------
    /// close the semaphore
    //--------------------------------------------------------------------------
    void  Close();

private:
    /// copy constructor made private; Prevent making copies of this object
    NamedSemaphore(const NamedSemaphore& rhs)
    {
        PS_UNREFERENCED_PARAMETER(rhs);
    }

    /// assignment operator made private; Prevent making copies of this object
    NamedSemaphore& operator= (const NamedSemaphore& rhs) = delete;

    /// pointer to implementation. Dependent on platform
    NamedSemaphoreImpl*   m_pImpl;
};

#endif // GPS_NAMEDSEMAPHORE_H
