//=============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  File contains support code for patching vtables
//=============================================================================

#ifndef HOOK_HELPERS_H
#define HOOK_HELPERS_H
#include <windows.h>
#include <ole2.h>
#include <vector>
#include <string>
#include <map>

#include "../Common/mymutex.h"
#include "../Common/misc.h"

ptrdiff_t GetD3D10StaticOffset(void* object, DWORD index);
ptrdiff_t GetD3D11StaticOffset(void* object, DWORD index);

#define HOOK( real, mine )                           \
    error = AMDT::HookAPICall( &( PVOID& )real, (PVOID)mine );  \
    PsAssert( error == NO_ERROR );

#define UNHOOK( real, mine )                           \
    error = AMDT::UnhookAPICall( &( PVOID& )real, (PVOID)mine );  \
    real = nullptr;                                            \
    PsAssert( error == NO_ERROR );

//-----------------------------------------------------------------------------
/// Holds a list of Real-Mine functions
/// Attachment happens by just calling Attach(), same thig for Detach().
/// Uses refcounting so the attachment only happens the first time you call Attach()
/// and detaching only happens when you have called Detach as many times as Attach()
/// It is thread safe
//-----------------------------------------------------------------------------
class HookBase
{
protected:
    /// name
    std::string m_Name;

    /// list of real functions
    std::vector<PVOID*> m_Real;

    /// list of Mine functions
    std::vector<PVOID > m_Mine;

    /// ref count
    DWORD m_dwAttached;

    /// mutex to avoid race conditions
    mutex m_mtx;
public:

    //-----------------------------------------------------------------------------
    /// constructor
    //-----------------------------------------------------------------------------
    HookBase(std::string Name);

    //-----------------------------------------------------------------------------
    /// Add a real-mine pair to the list
    //-----------------------------------------------------------------------------
    void Add(PVOID* real, PVOID mine);

    //-----------------------------------------------------------------------------
    /// erases the list
    //-----------------------------------------------------------------------------
    bool  Reset();

    //-----------------------------------------------------------------------------
    /// Attach all the "real" functions to its corresponding "mine" functions
    //-----------------------------------------------------------------------------
    bool Attach();

    //-----------------------------------------------------------------------------
    /// Detach all the "real" functions to its corresponding "mine" functions
    //-----------------------------------------------------------------------------
    bool Detach();

    //-----------------------------------------------------------------------------
    /// returns the refcount
    //-----------------------------------------------------------------------------
    DWORD AttachedRefCount();
};


#endif // HOOK_HELPERS_H
