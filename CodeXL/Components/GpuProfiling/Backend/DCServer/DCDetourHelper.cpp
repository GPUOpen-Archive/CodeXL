//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief
//==============================================================================

#include "DCDetourHelper.h"
#include "DCVtableOffsets.h"
#include "DCFuncDefs.h"

using namespace GPULogger;


ptrdiff_t GetD3D11StaticOffset(void* pInstance, DWORD index)
{
    void* pFunc = (*reinterpret_cast< void*** >(pInstance))[index];
    uintptr_t uPtrFunc = reinterpret_cast<uintptr_t>(pFunc);

    return uPtrFunc;
}

ptrdiff_t* PatchVtable(ptrdiff_t** vtable, DWORD dwIndex, const ptrdiff_t* pMine_Func)
{
    ptrdiff_t* real = (vtable)[ dwIndex ];

    DWORD dwOld;
    BOOL res = VirtualProtect(&vtable[ dwIndex ], 4, PAGE_EXECUTE_READWRITE, &dwOld);
    SpAssert(res != FALSE);
    vtable[ dwIndex ] = const_cast<ptrdiff_t*>(pMine_Func);

    DWORD dwOld2;
    res = VirtualProtect(&vtable[ dwIndex ], 4, dwOld, &dwOld2);
    SpAssert(res != FALSE);
    return real;
}

ULONG getRefCount(IUnknown* pObj, ptrdiff_t* pReal_Release)
{
    IUnknown_Release_type pReal_Release_Func = (IUnknown_Release_type)pReal_Release;
    pObj->AddRef();
    return pReal_Release_Func(pObj);
}

VTableManager::VTableManager(char* Name, DWORD dwIndex, ptrdiff_t* pFnMine, unsigned long ulReleaseRef)
{
    m_InstanceCount++;

    m_dwIndex = dwIndex;
    m_pFnMine = (ptrdiff_t*)pFnMine;
    m_Name = Name;
    m_pOnReleaseCallBack = NULL;
    m_ulReleaseRef = ulReleaseRef;
    m_mtx = new AMDTMutex("VTableManager mutex");
}


VTableManager::~VTableManager()
{
#ifdef _DEBUG // only display RefLeak message boxes in debug builds

    if (m_RealTrampoline.empty() == false)
    {
        std::string title = StringUtils::FormatString("Ref leaks on %s", m_Name).c_str();

        std::string tmp = Report(false);

        //MessageBox( 0, tmp.c_str() , title.c_str(), 0 );
        Log(traceMESSAGE, "%s", tmp.c_str());
    }

#endif //_DEBUG
    m_InstanceCount--;

    if (m_mtx != NULL)
    {
        delete m_mtx;
    }
}

ULONG VTableManager::GetRefCount(IUnknown* pObj)
{
    VTABLE vtable = *(VTABLE*)pObj;

    VTableMap::iterator iterVTable = m_RealTrampoline.find(vtable);

    if (iterVTable == m_RealTrampoline.end())
    {
        // object is not in the database
        // so it should be safe to call its release function
        Log(logWARNING, "Virtual table not in database: 0x%p\n", vtable);
        pObj->AddRef();
        return pObj->Release();
    }

    // Get the real release
    ptrdiff_t* pfunc = iterVTable->second.m_pFunc;
    Release_type pReleaseFunc = (Release_type)pfunc;

    SpAssert(pReleaseFunc != NULL);

    // get the current reference count (note that we don't detour the AddRef function)
    pObj->AddRef();
    ULONG ulRef = (pReleaseFunc)(pObj);
    return ulRef;
}

std::vector<IUnknown*> VTableManager::GetInterfacesList()
{
    std::vector<IUnknown*> IfaceList;

    for (VTableMap::iterator iterVTable = m_RealTrampoline.begin(); iterVTable != m_RealTrampoline.end(); iterVTable++)
    {
        for (IFacesMap::iterator iterObj = iterVTable->second.m_Objects.begin(); iterObj != iterVTable->second.m_Objects.end(); iterObj++)
        {
            IfaceList.push_back(iterObj->first);
        }
    }

    return IfaceList;
}

std::string VTableManager::Report(bool bPrintRefCount)
{
    std::string tmp;

    // by "object" here I really mean "virtual table"
    tmp += StringUtils::FormatString("There are %d virtual %s object(s) remaining:\n\n", m_RealTrampoline.size(), m_Name);

    unsigned int uElementCount = 1;

    for (VTableMap::iterator iterVTable = m_RealTrampoline.begin();
         iterVTable != m_RealTrampoline.end();
         iterVTable++)
    {
        VTABLE vtable = iterVTable->first;

        tmp += StringUtils::FormatString("Virtual table %d: 0x%p, Ref count %d, Objects %d\n\n", uElementCount, vtable, iterVTable->second.m_dwRefCount, iterVTable->second.m_Objects.size());

        for (IFacesMap::iterator iterObject = iterVTable->second.m_Objects.begin(); iterObject != iterVTable->second.m_Objects.end(); iterObject++)
        {
            IUnknown* pObj = iterObject->first;

            tmp += StringUtils::FormatString("0x%p",  pObj);

            if (bPrintRefCount)
            {
                tmp += StringUtils::FormatString(" : refs %5d", GetRefCount(pObj));
            }

            tmp += "\n\n";

            if (iterObject->second.m_bAppCreated)
            {
                tmp += "Initially created by: Application\n";
            }
            else
            {
                tmp += "Initially created by: GPU PerfStudio\n";
            }

            if (iterObject->second.m_callStackMap.size() > 0)
            {
                tmp.append("Created from the following callstacks:\n");

                tmp += iterObject->second.GetCallStackString();
            }

            // add extra newline
            tmp += "\n";
        }

        uElementCount++;
    }

    return tmp;
}

/// Repeated instantiation of CDebugHelpers is expensive as it is loading and unloading the dbghelp library each time.
/// We declare it here as a global to avoid this.
#ifdef ENABLE_STACK_TRACE
    DebugHelpers::CDebugHelpers f_DebugHelpers;
#endif // _DEBUG

//-----------------------------------------------------------------------------
bool VTableManager::AddAndDetourIfUnique(IUnknown* pObject, bool bAppCreated)
{
    AMDTScopeLock t(m_mtx);
    VTABLE vtable = *(VTABLE*) pObject;

    std::string strCallStack;

#ifdef ENABLE_STACK_TRACE
    // strInterface isn't actually used
    std::string strInterface;
    f_DebugHelpers.GetSymbolName((DWORD64) pObject, strInterface);

    // strFunc isn't actually used
    std::string strFunc;
    f_DebugHelpers.GetSymbolName((DWORD64) m_pFnMine, strFunc);

    f_DebugHelpers.GetCallStack(strCallStack);
#else // !_DEBUG
    strCallStack = StringUtils::FormatString("0x%p", m_pFnMine);
#endif // !_DEBUG

    VTableMap::iterator iterVTable = m_RealTrampoline.find(vtable);

    if (iterVTable == m_RealTrampoline.end())
    {
        // The object's virtual table is not in the database,
        // so add the virtual table (this is done using [])
        // and add an instance of this object
        if (m_RealTrampoline[ vtable ].AddInstance(pObject, strCallStack, bAppCreated))
        {
            // if we could add the object, then patch it's vtable
            m_RealTrampoline[ vtable ].m_pFunc = PatchVtable(vtable, m_dwIndex, m_pFnMine);
            Log(traceMESSAGE, "Patch vtable - %s(0x%8p) : Successful", m_Name, pObject);
            return true;
        }

        Log(logERROR, "Failed to add an instance of a new virtual table to the database.\n");
        m_RealTrampoline.erase(vtable);
        return false;
    }

    // the object's virtual table already exists
    // so just add an instance of this new object
    if (iterVTable->second.AddInstance(pObject, strCallStack, bAppCreated) == false)
    {
        // this isn't actually an error, but may help with some debugging
#ifdef _DEBUG
        //      Log( logWARNING, "Failed to add an instance to a pre-existing virtual table in the database. (object type: %s 0x%p)\n", TranslateIUnknown( pObject ), pObject );
        //      Log( logWARNING, "This instance was created in callstack:\n%s\n", strCallStack.c_str() );
#endif

    }

    // return false since the virtual table was already detoured
    return false;
}

//-----------------------------------------------------------------------------
void VTableManager::AddVtableElementToPatch(IUnknown* pI, DWORD dwIndex, ptrdiff_t* pPatchFunc)
{
    AMDTScopeLock t(m_mtx);
    VTABLE vtable = *(VTABLE*)pI;

    VTableMap::iterator iterVtable = m_RealTrampoline.find(vtable);

    if (iterVtable != m_RealTrampoline.end())
    {
        iterVtable->second.VtableElementsToPatch[ dwIndex ] = pPatchFunc;
        iterVtable->second.m_MineFuncTable[ dwIndex ] = pPatchFunc;
    }
}

void VTableManager::Reattach()
{
    for (VTableMap::iterator iterVTable = m_RealTrampoline.begin(); iterVTable != m_RealTrampoline.end(); iterVTable++)
    {
        VtableIndexDatabase& rFuncTable = iterVTable->second.m_MineFuncTable;

        for (VtableIndexDatabase::iterator iterFuncIndex = rFuncTable.begin(); iterFuncIndex != rFuncTable.end(); iterFuncIndex++)
        {
            VTABLE vtable = iterVTable->first;
            DWORD dwIndex = iterFuncIndex->first;

            //patch!
            PatchVtable(vtable, dwIndex, iterFuncIndex->second);
        }

        iterVTable->second.bPatched = true;
    }
}

//-----------------------------------------------------------------------------
void VTableManager::Attach()
{
    AMDTScopeLock t(m_mtx);

    for (VTableMap::iterator iterVTable = m_RealTrampoline.begin(); iterVTable != m_RealTrampoline.end(); iterVTable++)
    {
        if (iterVTable->second.bPatched == true)
        {
            continue;
        }

        VtableIndexDatabase& rFuncTable = iterVTable->second.VtableElementsToPatch;

        for (VtableIndexDatabase::iterator iterFuncIndex = rFuncTable.begin(); iterFuncIndex != rFuncTable.end(); iterFuncIndex++)
        {
            VTABLE vtable = iterVTable->first;
            DWORD dwIndex = iterFuncIndex->first;

            //patch!
            rFuncTable[dwIndex] = PatchVtable(vtable, dwIndex, iterFuncIndex->second);
        }

        iterVTable->second.bPatched = true;
    }
}

//-----------------------------------------------------------------------------
void VTableManager::Detach()
{
    for (VTableMap::iterator iterVTable = m_RealTrampoline.begin(); iterVTable != m_RealTrampoline.end(); iterVTable++)
    {
        if (iterVTable->second.bPatched == false)
        {
            continue;
        }

        VTABLE vtable = iterVTable->first;

        /// Unpatch release
        ptrdiff_t* pfunc = iterVTable->second.m_pFunc;
        PatchVtable(vtable, m_dwIndex, pfunc);

        /// Unpatch the rest
        VtableIndexDatabase& rFuncTable = iterVTable->second.VtableElementsToPatch;

        for (VtableIndexDatabase::iterator iterFuncIndex = rFuncTable.begin(); iterFuncIndex != rFuncTable.end(); iterFuncIndex++)
        {
            DWORD dwIndex = iterFuncIndex->first;

            //unpatch!
            rFuncTable[dwIndex] = PatchVtable(vtable, dwIndex, iterFuncIndex->second);
        }

        Log(traceMESSAGE, "Unpatch vtable - %s : Successful", m_Name);
        iterVTable->second.bPatched = false;
    }
}

void VTableManager::Debug(unsigned int uiStart, unsigned int uiEnd)
{
    for (VTableMap::iterator Ivt = m_RealTrampoline.begin(); Ivt != m_RealTrampoline.end(); Ivt++)
    {
        ptrdiff_t** vtable = Ivt->first;

        std::string tmp = StringUtils::FormatString("%s(0x%p): ", m_Name, vtable);

        for (unsigned int i = uiStart; i <= uiEnd; i++)
        {
            tmp += StringUtils::FormatString("%d-0x%p,", i, vtable[i]);
        }

        tmp += "\n";

        Log(logMESSAGE, "%s", tmp.c_str());
    }
}

void VTableManager::UnpatchAndRemoveAll()
{
    for (VTableMap::iterator it = m_RealTrampoline.begin(); it != m_RealTrampoline.end(); it++)
    {
        ptrdiff_t* pfunc = it->second.m_pFunc;
        VTABLE vtable = it->first;
        //unpatch release function
        PatchVtable(vtable, m_dwIndex, pfunc);

        // if the vtable was patched and its the last instance then restore vtable
        if (it->second.bPatched)
        {
            VtableIndexDatabase& fd = it->second.VtableElementsToPatch;

            for (VtableIndexDatabase::iterator Ifd = fd.begin(); Ifd != fd.end(); Ifd++)
            {
                //unpatch!
                Ifd->second = PatchVtable(vtable, Ifd->first, Ifd->second);
            }
        }
    }

    m_RealTrampoline.clear();
}

ULONG VTableManager::CallRealRelease(IUnknown* pObj)
{
    VTABLE vtable = *(VTABLE*)pObj;

    VTableMap::iterator iterVTable = m_RealTrampoline.find(vtable);

    if (iterVTable == m_RealTrampoline.end())
    {
        // object is not in the database
        // so it should be safe to call its release function
        Log(logWARNING, "Unpatch vtable - %s : Virtual table not in database: 0x%p\n", m_Name, vtable);
        return pObj->Release();
    }

    // Get the real release
    ptrdiff_t* pfunc = iterVTable->second.m_pFunc;
    Release_type pReleaseFunc = (Release_type)pfunc;

    SpAssert(pReleaseFunc != NULL);
    ULONG ulRef = (pReleaseFunc)(pObj);
    return ulRef;
}

ULONG VTableManager::RemoveAndDetach(IUnknown* pObj)
{
    VTABLE vtable = *(VTABLE*)pObj;

    VTableMap::iterator iterVTable = m_RealTrampoline.find(vtable);

    if (iterVTable == m_RealTrampoline.end())
    {
        // object is not in the database
        // so it should be safe to call its release function
        Log(logWARNING, "Unpatch vtable - %s : Virtual table not in database: 0x%p\n", m_Name, vtable);
        return pObj->Release();
    }

    // Get the real release
    ptrdiff_t* pfunc = iterVTable->second.m_pFunc;
    Release_type pReleaseFunc = (Release_type)pfunc;

    SpAssert(pReleaseFunc != NULL);

    // the object's virtual table is in the database
    IFacesMap::iterator iterObject = iterVTable->second.m_Objects.find(pObj);

    if (iterObject == iterVTable->second.m_Objects.end())
    {
        // the object was not found in the database,
        // this can happen if an object is created within the runtime.
        // Since many objects share the same IUnkown virtual table, the release
        // is the same for all of them.
        return (pReleaseFunc)(pObj);
    }

    // the object itself is in the database

    // get the current reference count (note that we don't detour the AddRef function)
    pObj->AddRef();
    ULONG ulRef = (pReleaseFunc)(pObj);

    if (ulRef <= m_ulReleaseRef)
    {
        // remove the object from the database
        if (iterVTable->second.RemoveInstance(pObj) == 0)
        {
            // there are no more references to this virtual table

            //unpatch release function
            PatchVtable(vtable, m_dwIndex, pfunc);

            // if the vtable was patched and its the last instance then restore vtable
            if (iterVTable->second.bPatched)
            {
                VtableIndexDatabase& fd = iterVTable->second.VtableElementsToPatch;

                for (VtableIndexDatabase::iterator Ifd = fd.begin(); Ifd != fd.end(); Ifd++)
                {
                    //unpatch!
                    Ifd->second = PatchVtable(vtable, Ifd->first, Ifd->second);
                }
            }

            // we've detached from the virtual table, now remove it from the list
            m_RealTrampoline.erase(iterVTable);

            Log(traceMESSAGE, "Unpatch vtable - %s(0x%8p) : Successful", m_Name, pObj);
        }

        //we are about to release, call callback
        if (m_pOnReleaseCallBack != NULL)
        {
            m_pOnReleaseCallBack(pObj);
        }

    }

    //real release
    ulRef = (pReleaseFunc)(pObj);

    return ulRef;
}

const ptrdiff_t* VTableManager::CallReal(const IUnknown* pObj, DWORD dwIndex)
{
    VTABLE vtable = *(VTABLE*)pObj;

    VTableMap::iterator It = m_RealTrampoline.find(vtable);

    if (It != m_RealTrampoline.end())
    {
        VtableIndexDatabase::iterator Iid = It->second.VtableElementsToPatch.find(dwIndex);

        if (Iid != It->second.VtableElementsToPatch.end())
        {
            return Iid->second;
        }
    }
    else
    {
        Log(logASSERT, "Instance's vtable not managed!\n");
    }

    return NULL;
}
