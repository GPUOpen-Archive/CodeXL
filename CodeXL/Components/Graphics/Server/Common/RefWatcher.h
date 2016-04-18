//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Used to monitor the current ref count of an interface
//==============================================================================

#ifndef REFWATCHER_H
#define REFWATCHER_H

#include "Logger.h"
#include "unknwn.h"

/*
   ** About enabling/disabling the RefWatch **

   It's hard to tell when to enable or disable the RefWatcher, it has more to do with the game
   itself than with the DEBUG/RELEASE build. Also its bad to have it off all the time and having
   it on in some games can be annoying.

   So here is a rule of thumb, when you are working on the HUD on a simple app is good to have
   them on once you don’t have any leaks, you can turn them off.

   I'll add a macro anyway that by default will be off, but please turn it on if you are working
   on the HUD
*/


/// Description required
class RefWatcher
{
    /// interface to monitor
    IUnknown* m_pI;

    /// number of references the object had when we started to monitor it
    ULONG m_NumRef;

    /// number of references that we know already exist, but that need to be released
    /// before this object is destroyed
    ULONG m_ulPreExistingRefs;

    /// string to help identify the source file where the ref problem happened
    LPCTSTR m_File;

    /// string to help identify the function where the ref problem happened
    LPCTSTR m_Function;

    /// string to help identify the instance where the ref problem happened
    LPCTSTR m_Instance;

    /// string to help identify the line in the source code file where the ref problem happened
    DWORD m_Line;

public:

    /// constructor
    RefWatcher(IUnknown* pI, LPCTSTR pFile = NULL, LPCTSTR pFunction = NULL, DWORD dwLine = 0, LPCTSTR pInstance = NULL)
    {
        PsAssert(pI != NULL);
        m_pI = pI;

        if (m_pI)
        {
            m_NumRef = m_pI->AddRef();
        }
        else
        {
            m_NumRef = 0;
        }

        m_File = pFile;
        m_Function = pFunction;
        m_Line = dwLine;
        m_Instance = pInstance;
        m_ulPreExistingRefs = 0;
    }

    ///
    ~RefWatcher()
    {
        ULONG Ref = GetRefCount(m_pI);

        if (m_NumRef != Ref + m_ulPreExistingRefs)
        {
            // We want to use the log to capture RefWatcher warnings - however we don't want the
            // log to report the line below - so we use a hidden log function to force the file, function and line
            // captured by the ref watcher.
            _SetupLog(false, LOG_MODULE, m_File, m_Line, m_Function);

            _Log(logWARNING, "RefWatcher: %s, ptr %p, in: %i, out %i, function: %s\n", m_Instance, m_pI, m_NumRef, Ref + m_ulPreExistingRefs, m_Function);
        }

        if (m_pI)
        {
            m_pI->Release();
        }
    }

    /// Gets the reference count of the monitored interface
    ULONG GetRefCount(IUnknown* pI)
    {
        if (pI == NULL)
        {
            return 0;
        }

        pI->AddRef();
        return pI->Release();
    }

    /// Provides us a way to add references that we know already exist.
    /// There are several DX functions that add a ref when the object is created
    /// and we have to create the RefWatcher after the object exists, but we also
    /// need to do the release from the creation, so this causes the in / out refs
    /// to get out of sync.
    /// \param ulPreExistingRefs the number of references that we know already exist
    void AddKnownRefs(ULONG ulPreExistingRefs)
    {
        m_ulPreExistingRefs += ulPreExistingRefs;
    }
};

//
// (!) If you are going to disable the RefWatch please read "About enabling/disabling the RefWatch"
//
#ifdef _DEBUG
    /// Definition
    #define REFWATCHER(var, a) RefWatcher var(a, __FILE__, __FUNCTION__, __LINE__, #a);
    /// Definition
    #define ADDKNOWNREFS( var, count ) var.AddKnownRefs( count );
#else
    /// Definition
    #define REFWATCHER(var, a){}
    /// Definition
    #define ADDKNOWNREFS( var, count ){}
#endif



#endif // REFWATCHER_H
