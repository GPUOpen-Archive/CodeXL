//=============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  File contains the code for patching vtables
//=============================================================================

#ifndef HOOK_VTABLE_IMMEDIATE
#define HOOK_VTABLE_IMMEDIATE

#include <windows.h>
#include <ole2.h>
#include <vector>
#include <string>
#include <map>

#include "../Common/misc.h"
#include "IFaceData.h"
#include "VTableData.h"

#include "../Common/mymutex.h"

/// Typedef the OnRelease callback
typedef void (*OnRelease_type)(IUnknown* pObj);

/// typedef for the Release function in COM objects'
typedef ULONG(WINAPI* Release_type)(IUnknown* pObj);

//-----------------------------------------------------------------------------
/// Helper to hook virtual tables of object's intances
//-----------------------------------------------------------------------------
class HookVtableImmediate
{
    /// Index to the release function in vtable
    DWORD m_dwReleaseFunctionIndex;

    /// Release function pointed to by m_dwIndex
    ptrdiff_t* m_pFnMineReleaseFunction;

    /// callback function when the object's last release is completed
    OnRelease_type m_pOnReleaseCallBack;

    /// Helper function
    /// \param vtable
    /// \param dwIndex
    /// \param pFunc
    /// \return
    static ptrdiff_t* PatchVtableFunction(ptrdiff_t** vtable, DWORD dwIndex, ptrdiff_t* pFunc)
    {
        ptrdiff_t* real = (vtable)[ dwIndex ];

        DWORD dwOld;
        VirtualProtect(&vtable[ dwIndex ], 4, PAGE_EXECUTE_READWRITE, &dwOld);

        vtable[ dwIndex ] = pFunc;

        DWORD dwOld2;
        VirtualProtect(&vtable[ dwIndex ], 4, dwOld, &dwOld2);

        return real;
    }

protected:

    /// name to display when there are ref leaks
    char* m_Name;

    /// map of virtual tables to original functions and referencing objects
    VTableMap m_VTableMap;

public:

    //-----------------------------------------------------------------------------
    /// Attaches to all elements that have been added for patching
    //-----------------------------------------------------------------------------
    virtual void Attach();

    //-----------------------------------------------------------------------------
    /// Detaches from all patched virtual table elements
    //-----------------------------------------------------------------------------
    virtual void Detach();

    //-----------------------------------------------------------------------------
    /// Adds a function to the list of those that should be patched
    /// \param pI interface of object that needs to be patched
    /// \param dwIndex of the vtable element to patch
    /// \param pFnMine pointer to function that should be called instead of original function
    /// \param logOriginalPtr Bool to control if additional logging is required (used when debugging)
    //-----------------------------------------------------------------------------
    virtual void AddVtableFunctionToPatch(IUnknown* pI, DWORD dwIndex, ptrdiff_t* pFnMine, bool logOriginalPtr = false);

    //-----------------------------------------------------------------------------
    /// constructor
    /// \param Name a string name to give to this object to aid in debugging
    /// \param dwIndex the index to the release function
    /// \param pFnMine callback function when the last release has been called
    //-----------------------------------------------------------------------------
    HookVtableImmediate(char* Name, DWORD dwReleaseFunctionIndex, ptrdiff_t* pFnMineReleaseFunction);

    //-----------------------------------------------------------------------------
    /// Use the real release function associated with the object (not our mine_ version). This avoids recursion into our own function.
    /// \param pObject The object to call release on.
    /// \return The current ref count.
    //-----------------------------------------------------------------------------
    virtual ULONG ObjectRealRelease(IUnknown* pObject);

    //-----------------------------------------------------------------------------
    /// Destructor, it complains if there are ref leaks
    //-----------------------------------------------------------------------------
    virtual ~HookVtableImmediate();

    //-----------------------------------------------------------------------------
    /// Gets the current reference count of the virtual table
    /// \param pI the IUnknown object to get the refcount of
    /// \return the refcount of an interface's instance
    //-----------------------------------------------------------------------------
    virtual ULONG GetRefCount(IUnknown* pI);

    //-----------------------------------------------------------------------------
    /// \return a list containing a list of all the instances held in the database
    //-----------------------------------------------------------------------------
    virtual std::vector<IUnknown*> GetInterfacesList();

    //-----------------------------------------------------------------------------
    /// Dumps a string with all the instances in the database, for debugging.
    //-----------------------------------------------------------------------------
    virtual std::string Report(bool bPrintRefCount)
    {
        PS_UNREFERENCED_PARAMETER(bPrintRefCount);
        return "";
    }

    //-----------------------------------------------------------------------------
    /// Adds an object to the virtual table database and hooks the virtual table if needed
    /// \param pObject The object that should be added or hooked
    /// \param bAppCreated indicates (true) if the app created this object, or (false) if it was created by PerfStudio
    /// \return true if this object has a new virtual table; false if the virtual table has already been hookedor if the object is already in the database
    //-----------------------------------------------------------------------------
    virtual bool AddAndHookIfUnique(IUnknown* pDev, bool bAppCreated);

    //-----------------------------------------------------------------------------
    /// Removes an instance vtable from the database if its not referenced by any other instances (in the databse) and unhooks it
    /// \param pObject instance to remove
    /// \param  str String based reason for the removal (used for debugging)
    /// \return ULONG with the remaining ref counts
    //-----------------------------------------------------------------------------
    bool TrueRemoveAndDetach(IUnknown* pObject, char* strLayer  = "UNSPECIFIED");

    //-----------------------------------------------------------------------------
    /// This function needs to be called on Release (it should be called 'OnRelease') to update the internal database
    /// it will removes an instance vtable from the database if its not referenced by any other instances (in the databse)
    /// and unhooks it
    /// \param pDev instance to remove
    /// \param  str String based reason for the removal (used for debugging)
    /// \return ULONG with the remaining ref counts
    //-----------------------------------------------------------------------------
    virtual ULONG RemoveAndDetach(IUnknown* pDev, char* str = "UNSPECIFIED");

    //-----------------------------------------------------------------------------
    /// Returns a pointer to the original virtual table function
    /// \param pDev an instance, note it should exist in the database, crash otherwise
    /// \param dwIndex index of the pointer in the vtable
    /// \return pointer to the original function
    //-----------------------------------------------------------------------------
    virtual void* GetRealFunction(IUnknown* pDev, DWORD dwIndex);

    //-----------------------------------------------------------------------------
    /// Sets a callback that well be called before releasing definitively the instance
    //-----------------------------------------------------------------------------
    void SetOnReleaseCallBack(OnRelease_type pFn)
    {
        m_pOnReleaseCallBack = pFn;
    }

    //-----------------------------------------------------------------------------
    /// Prints vtables of all managed objects
    //-----------------------------------------------------------------------------
    virtual void Debug();
};

//-----------------------------------------------------------------------------
/// Helper to hook virtual tables of object's intances. This class adds a mutex around calls to the base class's VTables.
//-----------------------------------------------------------------------------
class HookVtableImmediateWithMutex : public HookVtableImmediate
{

protected:

    //-----------------------------------------------------------------------------
    /// Protect the code (in particular the maps) from use by two threads at the same time
    // Removed to fix RE5 Bug
    //-----------------------------------------------------------------------------
    mutex m_mtxVTable;

public:

    //-----------------------------------------------------------------------------
    /// Attaches to all elements that have been added for patching
    //-----------------------------------------------------------------------------
    void Attach()
    {
        // Use mutex whenever VTableMap is being used.
        //StreamLog::Ref() << "Attach: m_mtxVTable\n";
        ScopeLock mutex(m_mtxVTable);
        HookVtableImmediate::Attach();
    }

    //-----------------------------------------------------------------------------
    /// Detaches from all patched virtual table elements
    //-----------------------------------------------------------------------------
    void Detach()
    {
        // Use mutex whenever VTableMap is being used.
        //StreamLog::Ref() << "Detach: m_mtxVTable\n";
        ScopeLock mutex(m_mtxVTable);
        HookVtableImmediate::Detach();
    }

    //-----------------------------------------------------------------------------
    /// Adds a function to the list of those that should be patched
    /// \param pI interface of object that needs to be patched
    /// \param dwIndex of the vtable element to patch
    /// \param pFnMine pointer to function that should be called instead of original function
    //-----------------------------------------------------------------------------
    void AddVtableFunctionToPatch(IUnknown* pI, DWORD dwIndex, ptrdiff_t* pFnMine)
    {
        // Use mutex whenever VTableMap is being used.
        //StreamLog::Ref() << "AddVtableFunctionToPatch: m_mtxVTable\n";
        ScopeLock mutex(m_mtxVTable);
        HookVtableImmediate::AddVtableFunctionToPatch(pI, dwIndex, pFnMine);
    }

    //-----------------------------------------------------------------------------
    /// detaches from an object and stops tracking it
    /// \param pI interface of object that needs to be patched
    //-----------------------------------------------------------------------------
    bool TrueRemoveAndDetach(IUnknown* pI)
    {
        // Use mutex whenever VTableMap is being used.
        //StreamLog::Ref() << "AddVtableFunctionToPatch: m_mtxVTable\n";
        ScopeLock mutex(m_mtxVTable);
        return HookVtableImmediate::TrueRemoveAndDetach(pI);
    }

    //-----------------------------------------------------------------------------
    /// constructor
    /// \param Name a string name to give to this object to aid in debugging
    /// \param dwIndex the index to the release function
    /// \param pFnMine callback function when the last release has been called
    //-----------------------------------------------------------------------------
    HookVtableImmediateWithMutex(char* Name, DWORD dwReleaseFunctionIndex, ptrdiff_t* pFnMineReleaseFunction)
        : HookVtableImmediate(Name, dwReleaseFunctionIndex, pFnMineReleaseFunction)
    {
    }

    //-----------------------------------------------------------------------------
    /// destructor, it complains if there are ref leaks
    //-----------------------------------------------------------------------------
    virtual ~HookVtableImmediateWithMutex()
    {
    }

    //-----------------------------------------------------------------------------
    /// Gets the ref count of an interface
    //-----------------------------------------------------------------------------
    ULONG GetRefCount(IUnknown* pI)
    {
        // Use mutex whenever VTableMap is being used.
        //StreamLog::Ref() << "GetRefCount: m_mtxVTable\n";
        ScopeLock mutex(m_mtxVTable);
        return HookVtableImmediate::GetRefCount(pI);
    }

    //-----------------------------------------------------------------------------
    /// \return a list containing a list of all the instances held in the database
    //-----------------------------------------------------------------------------
    std::vector<IUnknown*> GetInterfacesList()
    {
        // Use mutex whenever VTableMap is being used.
        //StreamLog::Ref() << "GetInterfacesList: m_mtxVTable\n";
        ScopeLock mutex(m_mtxVTable);
        return HookVtableImmediate::GetInterfacesList();
    }

    //-----------------------------------------------------------------------------
    /// Dumps a string with all the instances in the database, for debugging
    //-----------------------------------------------------------------------------
    virtual std::string Report(bool bPrintRefCount)
    {
        PS_UNREFERENCED_PARAMETER(bPrintRefCount);
        return "";
    }

    //-----------------------------------------------------------------------------
    /// Adds an object to the virtual table database and hooks the virtual table if needed
    /// \param pObject The object that should be added or hooked
    /// \param bAppCreated indicates (true) if the app created this object, or (false) if it was created by PerfStudio
    /// \return true if this object has a new virtual table; false if the virtual table has already been hookedor if the object is already in the database
    //-----------------------------------------------------------------------------
    bool AddAndHookIfUnique(IUnknown* pDev, bool bAppCreated)
    {
        // Use mutex whenever VTableMap is being used.
        //StreamLog::Ref() << "AddAndHookIfUnique: m_mtxVTable\n";
        ScopeLock mutex(m_mtxVTable);
        return HookVtableImmediate::AddAndHookIfUnique(pDev, bAppCreated);
    }

    //-----------------------------------------------------------------------------
    /// removes an instance from the database  <- ****this function should be called OnRelease***
    //-----------------------------------------------------------------------------
    ULONG RemoveAndDetach(IUnknown* pDev, char* str = "UNSPECIFIED")
    {
        // Use mutex whenever VTableMap is being used.
        //StreamLog::Ref() << "RemoveAndDetach: m_mtxVTable\n";
        ScopeLock mutex(m_mtxVTable);
        return HookVtableImmediate::RemoveAndDetach(pDev, str);
    }

    //-----------------------------------------------------------------------------
    /// returns a pointer to the original function in the vtable
    //-----------------------------------------------------------------------------
    void* GetRealFunction(IUnknown* pDev, DWORD dwIndex)
    {
        // Use mutex whenever VTableMap is being used.
        //StreamLog::Ref() << "GetRealFunction: m_mtxVTable\n";
        ScopeLock mutex(m_mtxVTable);
        return HookVtableImmediate::GetRealFunction(pDev, dwIndex);
    }

    //-----------------------------------------------------------------------------
    /// Help debug the tables
    //-----------------------------------------------------------------------------
    void Debug()
    {
        // Use mutex whenever VTableMap is being used.
        //StreamLog::Ref() << "GetRealFunction: m_mtxVTable\n";
        ScopeLock mutex(m_mtxVTable);
        HookVtableImmediate::Debug();
    }
};

#endif // HOOK_VTABLE_IMMEDIATE