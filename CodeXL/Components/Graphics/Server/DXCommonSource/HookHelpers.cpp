//=============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  File contains support code for patching vtables
//=============================================================================

#include <windows.h>
#include "HookHelpers.h"
#include "../Common/misc.h"
#include "Interceptor.h"
#include "../Common/Logger.h"

#ifdef ENABLE_STACK_TRACE
    #include "DebugHelpers/DebugHelpers.h"
#endif

/// typedef for the Release function in COM objects'
typedef ULONG(WINAPI* Release_type)(IUnknown* pObj);

//-----------------------------------------------------------------------------
/// Retuns a pointer to the index-th virtual function of an instance
/// \param object a pointer to an instance
/// \param index index to a function in the instances virtual table
/// \return a pointer to that function
//-----------------------------------------------------------------------------
ptrdiff_t GetD3D10StaticOffset(void* pInstance, DWORD index)
{
    void* pFunc = (*reinterpret_cast< void*** >(pInstance))[index];

    uintptr_t uPtrFunc = reinterpret_cast<uintptr_t>(pFunc);

    return uPtrFunc;
}

//-----------------------------------------------------------------------------
/// Retuns a pointer to the index-th virtual function of an instance
/// \param object a pointer to an instance
/// \param index index to a function in the instances virtual table
/// \return a pointer to that function
//-----------------------------------------------------------------------------
ptrdiff_t GetD3D11StaticOffset(void* pInstance, DWORD index)
{
    void* pFunc = (*reinterpret_cast< void*** >(pInstance))[index];

    uintptr_t uPtrFunc = reinterpret_cast<uintptr_t>(pFunc);

    return uPtrFunc;
}

//-----------------------------------------------------------------------------
/// Constructor
/// \param Name A name for the class, it will be displayed if any assert gets triggered
//-----------------------------------------------------------------------------
HookBase::HookBase(std::string Name) : m_Name(Name)
{
    m_dwAttached = 0;
}

//-----------------------------------------------------------------------------
/// Add a pair real-mine functions, when you call Attach later the "mine" function will get called instead of the "real"
/// \param real the real function, when Attach gets called the "*real" will have a pointer to the original function
/// \param mine the mine function
//-----------------------------------------------------------------------------
void HookBase::Add(PVOID* real, PVOID mine)
{
    ScopeLock t(&m_mtx);

    m_Real.push_back(real);
    m_Mine.push_back(mine);
}

//-----------------------------------------------------------------------------
/// Clears the database if the ref cont is 0
/// \return true if the database was cleared
//-----------------------------------------------------------------------------
bool HookBase::Reset()
{
    ScopeLock t(&m_mtx);

    if (AttachedRefCount() == 0)
    {
        m_Real.clear();
        m_Mine.clear();
        return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
/// Calls Hook on all the real-mine function pairs
/// If this function succeeds, the *real parameters that were passed in the Add function will point
/// to the original functions
/// \return the ref count
//-----------------------------------------------------------------------------
bool HookBase::Attach()
{
    ScopeLock t(&m_mtx);

    if (m_dwAttached == 0)
    {
        LONG error;
        AMDT::BeginHook();

        for (DWORD i = 0; i < m_Real.size(); i++)
        {
            HOOK(*m_Real[i], m_Mine[i]);
        }

        error = AMDT::EndHook();

        if (error != NO_ERROR)
        {
            Log(logERROR, "Attaching to %s failed\n", m_Name.c_str());
            return false;
        }
        else
        {
            m_dwAttached++;
        }
    }
    else
    {
        m_dwAttached++;
    }

    return true;
}

//-----------------------------------------------------------------------------
/// Calls Unhook on all the real-mine function pairs
/// \return the ref count
//-----------------------------------------------------------------------------
bool HookBase::Detach()
{
    ScopeLock t(&m_mtx);

    if (m_dwAttached == 1)
    {
        LONG error;
        AMDT::BeginHook();

        for (DWORD i = 0; i < m_Real.size(); i++)
        {
            UNHOOK(*m_Real[i], m_Mine[i]);
        }

        error = AMDT::EndHook();

        if (error != NO_ERROR)
        {
            Log(logERROR, "Detaching to %s failed\n", m_Name.c_str());
            return false;
        }
        else
        {
            m_dwAttached--;
        }
    }
    else if (m_dwAttached > 1)
    {
        m_dwAttached--;
    }
    else // m_dwAttached is less than 0
    {
        Log(logERROR, "Detaching %s refcount < 0 !!\n", m_Name.c_str());
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
/// Returns the refcount
/// \return the refcount
//-----------------------------------------------------------------------------
DWORD HookBase::AttachedRefCount()
{
    return m_dwAttached;
}

