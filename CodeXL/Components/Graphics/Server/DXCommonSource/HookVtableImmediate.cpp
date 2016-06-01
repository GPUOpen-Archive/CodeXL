//=============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  File contains the code for patching vtables
//=============================================================================

#include "HookVtableImmediate.h"
#include "../Common/Logger.h"

//-----------------------------------------------------------------------------
/// Constructor
//-----------------------------------------------------------------------------
HookVtableImmediate::HookVtableImmediate(char* Name, DWORD dwIndex, ptrdiff_t* pFnMine)
{
    m_dwReleaseFunctionIndex = dwIndex;
    m_pFnMineReleaseFunction = (ptrdiff_t*)pFnMine;
    m_Name = Name;
    m_pOnReleaseCallBack = nullptr;
}


//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
HookVtableImmediate::~HookVtableImmediate()
{
}


//-----------------------------------------------------------------------------
/// Return a ref count
//-----------------------------------------------------------------------------
ULONG HookVtableImmediate::GetRefCount(IUnknown* pObj)
{
    pObj->AddRef();

    VTABLE vtable = *(VTABLE*)pObj;

    VTableMap::iterator iterVTable = m_VTableMap.find(vtable);

    if (iterVTable == m_VTableMap.end())
    {
        // object is not in the database
        // so it should be safe to call its release function
        Log(logWARNING, "Virtual table not in database: 0x%p\n", vtable);
        return pObj->Release();
    }

    // Get the real release
    ptrdiff_t* pfunc = iterVTable->second.m_pFunc;
    Release_type pReleaseFunc = (Release_type)pfunc;

    return (pReleaseFunc)(pObj);
}


//-----------------------------------------------------------------------------
/// Return a vector of interfaces
//-----------------------------------------------------------------------------
std::vector<IUnknown*> HookVtableImmediate::GetInterfacesList()
{
    std::vector<IUnknown*> IfaceList;

    for (VTableMap::iterator iterVTable = m_VTableMap.begin(); iterVTable != m_VTableMap.end(); ++iterVTable)
    {
        for (IFacesMap::const_iterator iterObj = iterVTable->second.m_Objects.begin(); iterObj != iterVTable->second.m_Objects.end(); ++iterObj)
        {
            IfaceList.push_back(iterObj->first);
        }
    }

    return IfaceList;
}

/// Repeated instantiation of CDebugHelpers is expensive as it is loading and unloading the dbghelp library each time.
/// We declare it here as a global to avoid this.
#ifdef ENABLE_STACK_TRACE
    DebugHelpers::CDebugHelpers f_DebugHelpers;
#endif // _DEBUG


//-----------------------------------------------------------------------------
/// Add a new hook
//-----------------------------------------------------------------------------
bool HookVtableImmediate::AddAndHookIfUnique(IUnknown* pObject, bool bAppCreated)
{
    VTABLE vtable = *(VTABLE*) pObject;

#ifdef USE_STREAMLOG
    StreamLog::Ref() << "AddAndHookIfUnique: m_mtxVTable\n";
    StreamLog::Ref() << "-- AddAndHookIfUnique:pObject:" << pObject << "VTABLE:" << vtable << "\n";
#endif

#ifdef USE_GLOBAL_TRACKER
    GlobalObjectTracker::Ref().AddObject(pObject);
#endif

    std::string strCallStack;

#ifdef ENABLE_STACK_TRACE
    // strInterface isn't actually used
    std::string strInterface;
    f_DebugHelpers.GetSymbolName((DWORD64) pObject, strInterface);

    // strFunc isn't actually used
    std::string strFunc;
    f_DebugHelpers.GetSymbolName((DWORD64) m_pFnMineReleaseFunction, strFunc);

    f_DebugHelpers.GetCallStack(strCallStack);
#else // !_DEBUG
    strCallStack = FormatText("0x%p", m_pFnMineReleaseFunction).asCharArray();
#endif // !_DEBUG

    VTableMap::iterator iterVTable = m_VTableMap.find(vtable);

    if (iterVTable == m_VTableMap.end())
    {
        // The object's virtual table is not in the database,
        // so add the virtual table (this is done using [])
        // and add an instance of this object
        if (m_VTableMap[ vtable ].AddInstance(pObject, strCallStack, bAppCreated))
        {
            //StreamLog::Ref() << "VTable not in map - adding it." << "Count:" << m_VTableMap.size() << "\n";
            // if we could add the object, then patch the release function
            m_VTableMap[ vtable ].m_pFunc = PatchVtableFunction(vtable, m_dwReleaseFunctionIndex, m_pFnMineReleaseFunction);
            return true;
        }

        //StreamLog::Ref() << "Failed to add an instance of a new virtual table to the database." << "\n";
        Log(logERROR, "Failed to add an instance of a new virtual table to the database.\n");
        m_VTableMap.erase(vtable);
        return false;
    }

#ifdef _DEBUG
    else
    {
        // TODO:  Look into why this happens so much when playing back a frame capture
        //Log(logERROR, "The Object's VTable has already been patched: 0x%p\n", pObject);
    }

#endif

    // the object's virtual table already exists
    // so just add an instance of this new object
    if (iterVTable->second.AddInstance(pObject, strCallStack, bAppCreated) == false)
    {
        // this isn't actually an error, but may help with some debugging
#ifdef _DEBUG
        // TODO: look into these logs and find out why an Instance cannot be added
        //Log(logWARNING, "Failed to add an instance to a pre-existing virtual table in the database. (object type: %s 0x%p)\n", TranslateGUID( pObject ), pObject );
        //Log(logERROR, "Failed to add an instance to a pre-existing virtual table in the database. (object: 0x%p)\n", pObject);
        //Log(logERROR, "This instance was created in callstack:\n%s\n", strCallStack.c_str());
#endif
        return false;
    }

    // return false since the virtual table was already hooked
    return false;
}

//-----------------------------------------------------------------------------
/// Add a new vtable function
//-----------------------------------------------------------------------------
void HookVtableImmediate::AddVtableFunctionToPatch(IUnknown* pI, DWORD dwIndex, ptrdiff_t* pPatchFunc, bool logOriginalPtr)
{
    logOriginalPtr;
    VTABLE vtable = *(VTABLE*)pI;

    VTableMap::iterator iterVtable = m_VTableMap.find(vtable);

    if (iterVtable != m_VTableMap.end())
    {
        //if (logOriginalPtr == true)
        //{
        //    void * ptr = vtable[dwIndex];
        //    Log(logDEBUG, "AddVtableFunctionToPatch: Original function pointer (0x%p) Mine (0x%p)\n", ptr, pPatchFunc);
        //}

        iterVtable->second.m_VtableFunctionsToPatch[ dwIndex ] = pPatchFunc;
    }
}

//-----------------------------------------------------------------------------
/// Attach a function
//-----------------------------------------------------------------------------
void HookVtableImmediate::Attach()
{

    for (VTableMap::iterator iterVTable = m_VTableMap.begin(); iterVTable != m_VTableMap.end(); ++iterVTable)
    {
        if (iterVTable->second.m_bPatched == true)
        {
            continue;
        }

        VtableIndexDatabase& rFuncTable = iterVTable->second.m_VtableFunctionsToPatch;

        for (VtableIndexDatabase::const_iterator iterFuncIndex = rFuncTable.begin(); iterFuncIndex != rFuncTable.end(); ++iterFuncIndex)
        {
            VTABLE vtable = iterVTable->first;
            DWORD dwIndex = iterFuncIndex->first;

            //patch!
            rFuncTable[dwIndex] = PatchVtableFunction(vtable, dwIndex, iterFuncIndex->second);
        }

        iterVTable->second.m_bPatched = true;
    }
}

//-----------------------------------------------------------------------------
/// Detach a function
//-----------------------------------------------------------------------------
void HookVtableImmediate::Detach()
{

    for (VTableMap::iterator iterVTable = m_VTableMap.begin(); iterVTable != m_VTableMap.end(); ++iterVTable)
    {
        if (iterVTable->second.m_bPatched == false)
        {
            continue;
        }

        VtableIndexDatabase& rFuncTable = iterVTable->second.m_VtableFunctionsToPatch;

        for (VtableIndexDatabase::const_iterator iterFuncIndex = rFuncTable.begin(); iterFuncIndex != rFuncTable.end(); ++iterFuncIndex)
        {
            VTABLE vtable = iterVTable->first;
            DWORD dwIndex = iterFuncIndex->first;

            //unpatch!
            rFuncTable[dwIndex] = PatchVtableFunction(vtable, dwIndex, iterFuncIndex->second);
        }

        iterVTable->second.m_bPatched = false;
    }
}

//-----------------------------------------------------------------------------
/// Debugging helper
//-----------------------------------------------------------------------------
void HookVtableImmediate::Debug()
{
    for (VTableMap::iterator Ivt = m_VTableMap.begin(); Ivt != m_VTableMap.end(); Ivt++)
    {
        ptrdiff_t** vtable = Ivt->first;

        std::string tmp = FormatText("%s(0x%p): ", m_Name, vtable).asCharArray();

        for (int i = 0; i < 10; i++)
        {
            tmp += FormatText("0x%p,", vtable[i]).asCharArray();
        }

        tmp += "\n";

        Log(logMESSAGE, "%s", tmp.c_str());
    }
}

//-----------------------------------------------------------------------------
/// Remove and detach
//-----------------------------------------------------------------------------
bool HookVtableImmediate::TrueRemoveAndDetach(IUnknown* pObject, char* strLayer)
{
    UNREFERENCED_PARAMETER(strLayer);
    PsAssert(pObject != nullptr);
    VTABLE vtable = *(VTABLE*) pObject;

#ifdef USE_STREAMLOG
    DWORD  id  = GetCurrentThreadId();
    StreamLog::Ref() << strLayer << " Thread ID: " << id << " RemoveAndDetach: pObject:" << pObject << "VTABLE:" << vtable << "\n";
#endif

#ifdef USE_GLOBAL_TRACKER

    if (GlobalObjectTracker::Ref().IsBeingTracked(pObject) == false)
    {
#ifdef USE_STREAMLOG
        StreamLog::Ref() << "RemoveAndDetach: Object is not being tracked:" << pObject << "\n";
#endif
        // We have not seen this object before - it is not coming from the application. We suspect this release
        // is comming from the runtime so ignore it.
        return false;
    }
    else
    {
        GlobalObjectTracker::Ref().RemoveObjectRef(pObject);
    }

#endif

#ifdef USE_STREAMLOG
    StreamLog::Ref() << "RemoveAndDetach: For Object:" << pObject << "\n";
#endif
//TODO refactor this :  if can't be true since see above line : VTABLE vtable = *(VTABLE*) pObject;
    if (pObject == nullptr)
    {
        Log(logERROR, "HookVtableImmediate::RemoveAndDetach pObj is 0x%p\n", pObject);
        return false;
    }

    VTableMap::iterator iterVTable = m_VTableMap.find(vtable);

    if (iterVTable == m_VTableMap.end())
    {
        // object is not in the database
        Log(logWARNING, "Virtual table not in database: 0x%p\n", vtable);
        return false;
    }

    // the object's virtual table is in the database
    IFacesMap::iterator iterObject = iterVTable->second.m_Objects.find(pObject);

    if (iterObject == iterVTable->second.m_Objects.end())
    {
        // the object was not found in the database,
        return false;
    }


    // remove the object from the database
    if (iterVTable->second.RemoveInstance(pObject) == 0)
    {
        // there are no more references to this virtual table

        //unpatch release function
        PatchVtableFunction(vtable, m_dwReleaseFunctionIndex, iterVTable->second.m_pFunc);

        // if the vtable was patched and its the last instance then restore vtable
        if (iterVTable->second.m_bPatched)
        {
            VtableIndexDatabase fd = iterVTable->second.m_VtableFunctionsToPatch;

            for (VtableIndexDatabase::iterator Ifd = fd.begin(); Ifd != fd.end(); ++Ifd)
            {
                //unpatch!
                Ifd->second = PatchVtableFunction(vtable, Ifd->first, Ifd->second);
            }
        }

        // we've detached from the virtual table, now remove it from the list
        m_VTableMap.erase(iterVTable);
    }

    return true;
}

//-----------------------------------------------------------------------------
/// Release an object
//-----------------------------------------------------------------------------
ULONG HookVtableImmediate::ObjectRealRelease(IUnknown* pObject)
{
    PsAssert(pObject != nullptr);
    VTABLE vtable = *(VTABLE*) pObject;

#ifdef USE_STREAMLOG
    DWORD  id  = GetCurrentThreadId();
    StreamLog::Ref() << strLayer << " Thread ID: " << id << " RemoveAndDetach: pObject:" << pObject << "VTABLE:" << vtable << "\n";
#endif

#ifdef USE_GLOBAL_TRACKER

    if (GlobalObjectTracker::Ref().IsBeingTracked(pObject) == false)
    {
#ifdef USE_STREAMLOG
        StreamLog::Ref() << "RemoveAndDetach: Object is not being tracked:" << pObject << "\n";
#endif
        // We have not seen this object before - it is not coming from the application. We suspect this release
        // is comming from the runtime so ignore it.
        return 0;
    }
    else
    {
        GlobalObjectTracker::Ref().RemoveObjectRef(pObject);
    }

#endif

#ifdef USE_STREAMLOG
    StreamLog::Ref() << "RemoveAndDetach: For Object:" << pObject << "\n";
#endif
//TODO refactor this :  if can't be true since see above line : VTABLE vtable = *(VTABLE*) pObject;
    if (pObject == nullptr)
    {
        Log(logERROR, "HookVtableImmediate::RemoveAndDetach pObj is 0x%p\n", pObject);
        return 0;
    }

    VTableMap::iterator iterVTable = m_VTableMap.find(vtable);

    if (iterVTable == m_VTableMap.end())
    {
        // object is not in the database
        // so it should be safe to call its release function
        Log(logWARNING, "Virtual table not in database: 0x%p\n", vtable);
        return pObject->Release();
    }

    // Get the real release
    ptrdiff_t* pfunc = iterVTable->second.m_pFunc;
    Release_type pReleaseFunc = (Release_type)pfunc;

    PsAssert(pReleaseFunc != nullptr);

    if (pReleaseFunc == nullptr)
    {
        Log(logERROR, "HookVtableImmediate::RemoveAndDetach pReleaseFunc is nullptr\n");
        return 0;
    }

    // the object's virtual table is in the database
    IFacesMap::iterator iterObject = iterVTable->second.m_Objects.find(pObject);

    if (iterObject == iterVTable->second.m_Objects.end())
    {
        // the object was not found in the database,
        // this can happen if an object is created within the runtime.
        // Since many objects share the same IUnkown virtual table, the release
        // is the same for all of them.
        return (pReleaseFunc)(pObject);
    }

    // the object itself is in the database
    //real release
    ULONG ulRef = (pReleaseFunc)(pObject);

    return ulRef;
}

//-----------------------------------------------------------------------------
/// Remove and detach an object
//-----------------------------------------------------------------------------
ULONG HookVtableImmediate::RemoveAndDetach(IUnknown* pObject, char* strLayer)
{
    UNREFERENCED_PARAMETER(strLayer);
    PsAssert(pObject != nullptr);
    VTABLE vtable = *(VTABLE*) pObject;

#ifdef USE_STREAMLOG
    DWORD  id  = GetCurrentThreadId();
    StreamLog::Ref() << strLayer << " Thread ID: " << id << " RemoveAndDetach: pObject:" << pObject << "VTABLE:" << vtable << "\n";
#endif

#ifdef USE_GLOBAL_TRACKER

    if (GlobalObjectTracker::Ref().IsBeingTracked(pObject) == false)
    {
#ifdef USE_STREAMLOG
        StreamLog::Ref() << "RemoveAndDetach: Object is not being tracked:" << pObject << "\n";
#endif
        // We have not seen this object before - it is not coming from the application. We suspect this release
        // is comming from the runtime so ignore it.
        return 0;
    }
    else
    {
        GlobalObjectTracker::Ref().RemoveObjectRef(pObject);
    }

#endif

#ifdef USE_STREAMLOG
    StreamLog::Ref() << "RemoveAndDetach: For Object:" << pObject << "\n";
#endif
//TODO refactor this :  if can't be true since see above line : VTABLE vtable = *(VTABLE*) pObject;
    if (pObject == nullptr)
    {
        Log(logERROR, "HookVtableImmediate::RemoveAndDetach pObj is 0x%p\n", pObject);
        return 0;
    }

    VTableMap::iterator iterVTable = m_VTableMap.find(vtable);

    if (iterVTable == m_VTableMap.end())
    {
        // object is not in the database
        // so it should be safe to call its release function
        Log(logWARNING, "Virtual table not in database: 0x%p\n", vtable);
        return pObject->Release();
    }

    // Get the real release
    ptrdiff_t* pfunc = iterVTable->second.m_pFunc;
    Release_type pReleaseFunc = (Release_type)pfunc;

    PsAssert(pReleaseFunc != nullptr);

    if (pReleaseFunc == nullptr)
    {
        Log(logERROR, "HookVtableImmediate::RemoveAndDetach pReleaseFunc is nullptr\n");
        return 0;
    }

    // the object's virtual table is in the database
    IFacesMap::iterator iterObject = iterVTable->second.m_Objects.find(pObject);

    if (iterObject == iterVTable->second.m_Objects.end())
    {
        // the object was not found in the database,
        // this can happen if an object is created within the runtime.
        // Since many objects share the same IUnkown virtual table, the release
        // is the same for all of them.
        return (pReleaseFunc)(pObject);
    }

    // the object itself is in the database

    // get the current reference count (note that we don't hook the AddRef function)
    pObject->AddRef();
    ULONG ulRef = (pReleaseFunc)(pObject);

    // if there is only one left, that means we should detach from the vtable
    if (ulRef == 1)
    {
        // remove the object from the database
        if (iterVTable->second.RemoveInstance(pObject) == 0)
        {
            // there are no more references to this virtual table

            //unpatch release function
            PatchVtableFunction(vtable, m_dwReleaseFunctionIndex, pfunc);

            // if the vtable was patched and its the last instance then restore vtable
            if (iterVTable->second.m_bPatched)
            {
                VtableIndexDatabase fd = iterVTable->second.m_VtableFunctionsToPatch;

                for (VtableIndexDatabase::iterator Ifd = fd.begin(); Ifd != fd.end(); ++Ifd)
                {
                    //unpatch!
                    Ifd->second = PatchVtableFunction(vtable, Ifd->first, Ifd->second);
                }
            }

            // we've detached from the virtual table, now remove it from the list
            m_VTableMap.erase(iterVTable);
        }

        //we are about to release, call callback
        if (m_pOnReleaseCallBack != nullptr)
        {
            m_pOnReleaseCallBack(pObject);
        }
    }

    //real release
    ulRef = (pReleaseFunc)(pObject);

    return ulRef;
}

//-----------------------------------------------------------------------------
/// Get a pointer to function
//-----------------------------------------------------------------------------
void* HookVtableImmediate::GetRealFunction(IUnknown* pObj, DWORD dwIndex)
{
    VTABLE vtable = *(VTABLE*)pObj;

    VTableMap::iterator It = m_VTableMap.find(vtable);

    if (It != m_VTableMap.end())
    {
        VtableIndexDatabase::iterator Iid = It->second.m_VtableFunctionsToPatch.find(dwIndex);

        if (Iid != It->second.m_VtableFunctionsToPatch.end())
        {
            if (It->second.m_bPatched)
            {
                return Iid->second;
            }
            else
            {
                //if not attached return the vtable values
                return vtable[ dwIndex ];
            }
        }
    }
    else
    {
        Log(logASSERT, "Instance's vtable not managed! 0x%p\n", pObj);
    }

    return nullptr;
}

