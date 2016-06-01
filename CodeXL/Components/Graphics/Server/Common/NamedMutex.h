//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  An interface to a named mutex class. The mutex is
///         global systemwide, so can be accessed across threads and processes
///         by its name. Each process will need its own copy of the mutex
//==============================================================================

#ifndef GPS_NAMEDMUTEX_H
#define GPS_NAMEDMUTEX_H

#include "misc.h"

/// forward declaration to implementation class
class NamedMutexImpl;

/// NamedMutex wrapper class
class NamedMutex
{
public:
    NamedMutex();
    ~NamedMutex();

    //--------------------------------------------------------------------------
    /// Open an existing systemwide global mutex if it exists already, or create
    /// a new one if it doesn't exist
    /// \param mutexName the name this mutex will be known by
    /// \param initialOwner if true, the calling process will own this mutex, causing
    /// it to be started in the locked state
    /// \param global if true, the mutex is shared between 32 and 64 bit versions. If
    /// false, the 32 and 64 bit versions will have independent mutexes
    ///
    /// \return true if Create succeeded, false if error
    //--------------------------------------------------------------------------
    bool  OpenOrCreate(const char* mutexName, bool initialOwner = false, bool global = false);

    //--------------------------------------------------------------------------
    /// Open a previously created systemwide global mutex
    /// \param mutexName the name this mutex is be known by
    /// \param inherit if true, processes created by this process will inherit
    /// the mutex
    /// \param global if true, the mutex is shared between 32 and 64 bit versions. If
    /// false, the 32 and 64 bit versions will have independent mutexes
    ///
    /// \return true if Create succeeded, false if mutex doesn't exist
    //--------------------------------------------------------------------------
    bool  Open(const char* mutexName, bool inherit = false, bool global = false);

    //--------------------------------------------------------------------------
    /// Attempt to lock the mutex. The mutex implementation is recursive, meaning
    /// that calling Lock on a Locked mutex will not block. However, Unlock()
    /// must be called for each Lock()
    ///
    /// \return true if the Lock succeeded, false if not
    //--------------------------------------------------------------------------
    bool  Lock();

    //--------------------------------------------------------------------------
    /// Unlock a previously locked mutex
    //--------------------------------------------------------------------------
    void  Unlock();

    //--------------------------------------------------------------------------
    /// Close the mutex
    //--------------------------------------------------------------------------
    void  Close();

private:
    /// copy constructor made private; Prevent making copies of this object
    NamedMutex(const NamedMutex& rhs)
    {
        PS_UNREFERENCED_PARAMETER(rhs);
    }

    /// assignment operator made private; Prevent making copies of this object
    NamedMutex& operator= (const NamedMutex& rhs) = delete;

    /// pointer to implementation. Dependent on platform
    NamedMutexImpl*   m_pImpl;
};

#endif   // GPS_NAMEDMUTEX_H
