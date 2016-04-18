//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief
//==============================================================================

#ifndef _DC_DETOUR_HELPER_H_
#define _DC_DETOUR_HELPER_H_

#include <windows.h>
#include <d3d11.h>
#include <map>
#include <vector>
#include "..\Common\Logger.h"
#include "AMDTMutex.h"
#include "..\Common\StringUtils.h"

using namespace GPULogger;

/// \defgroup DCVirtualTablePatching DCVirtualTablePatching
/// This module handles the virtual table patching of the DX device and device context objects.
///
/// \ingroup DCCodeInjection
// @{

typedef ptrdiff_t** VTABLE;

/// Get the index th member function's address pointer from instance's vtable
/// \param pInstance Pointer to the instance, 1st DWORD points to vtable
/// \param dwIdx Index to vtable
/// \return function pointer
ptrdiff_t GetD3D11StaticOffset(void* pInstance, DWORD dwIdx);

/// Patch VTable
/// \param vtable Pointer to Vtable
/// \param dwIndex member function index
/// \param pMine_Func mine function pointer
/// \return real function pointer
ptrdiff_t* PatchVtable(VTABLE vtable, DWORD dwIndex, const ptrdiff_t* pMine_Func);

/// Return reference counter
/// \param pObj COM obj
/// \return reference counter
ULONG getRefCount(IUnknown* pObj);

/// Typedef the OnRelease callback function
typedef void (*OnRelease_type)(IUnknown* pObj);

/// map between function indices and function pointers
typedef std::map< DWORD, const ptrdiff_t* > VtableIndexDatabase;

// typedef for the Release function in COM objects'
typedef ULONG(WINAPI* Release_type)(IUnknown* pObj);

class VTableData;
class VTableManager;
class IFaceData;

/// map interface pointer to data about the object
typedef std::map< IUnknown*, IFaceData > IFacesMap;

/// map of a vtable to a map of objects which use that vtable
typedef std::map< VTABLE, VTableData > VTableMap;


/// Stores information about an instances of an interface
class IFaceData
{
    friend class VTableData;
    friend class VTableManager;
public:
    /// Constructor
    /// \param strCallStack the callstack when the object was created
    /// \param bAppCreated true if the object was created by the app; false if created by perfstudio
    IFaceData(std::string strCallStack, bool bAppCreated)
    {
        AddCallStack(strCallStack);
        m_bAppCreated = bAppCreated;
    }

    /// Default Constructor
    /// no callstack is added and it is assumbed to be created by perfstudio
    IFaceData()
    {
        m_bAppCreated = false;
    }

    /// Destructor
    ~IFaceData()
    {
    }

    /// Adds the callstack and increments the number of times
    /// \param strCallStack string containing the callstack to add
    void AddCallStack(std::string strCallStack)
    {
        std::map< std::string, unsigned long >::iterator iCallStack = m_callStackMap.find(strCallStack);

        if (iCallStack == m_callStackMap.end())
        {
            // new call stack, add it and set value to 1
            m_callStackMap[ strCallStack ] = 1;
        }
        else
        {
            // increment count
            iCallStack->second++;
        }
    }

    /// Generates a string of the callstacks
    /// \return a string containing all the callstacks
    std::string GetCallStackString()
    {
        std::string strCS;

        unsigned long ulCount = 1;

        for (std::map< std::string, unsigned long >::iterator iCS = m_callStackMap.begin();
             iCS != m_callStackMap.end();
             iCS++)
        {
            strCS += StringUtils::FormatString("Call Stack %d: (Called %d times)\n%s\n\n", ulCount, iCS->second, iCS->first.c_str());
            ulCount++;
        }

        return strCS;
    }

    /// Assignment operator
    /// \param obj right
    IFaceData& operator = (const IFaceData& obj)
    {
        if (&obj != this)
        {
            m_bAppCreated = obj.m_bAppCreated;
            m_callStackMap.clear();

            for (std::map< std::string, unsigned long >::const_iterator it = obj.m_callStackMap.begin(); it != obj.m_callStackMap.end(); it++)
            {
                m_callStackMap.insert(std::pair< std::string, unsigned long >(it->first, it->second));
            }
        }

        return *this;
    }

    /// Copy constructor
    /// \param obj source obj
    IFaceData(const IFaceData& obj)
    {
        m_bAppCreated = obj.m_bAppCreated;
        m_callStackMap.clear();

        for (std::map< std::string, unsigned long >::const_iterator it = obj.m_callStackMap.begin(); it != obj.m_callStackMap.end(); it++)
        {
            m_callStackMap.insert(std::pair< std::string, unsigned long >(it->first, it->second));
        }
    }

private:
    std::map< std::string, unsigned long > m_callStackMap;   ///< stores unique callstacks when an object is created, maps callstack to the number of times the object was created from that callstack
    bool m_bAppCreated;                                      ///< indicates whether the app (true) or perfstudio (false) created the object
};

/// Stores information about a virtual table
class VTableData
{
    friend class VTableManager;
public:
    /// Default constructor
    VTableData()
    {
        m_dwRefCount = 0;
        m_pFunc = NULL;
        bPatched = false;
    }

    /// Adds the IUnknown object as an instance of this virtual table
    /// \param pObject pointer object to add
    /// \param strCallStack The call stack to store with the object
    /// \param bAppCreated true if the app created this object; false if PerfStudio created it
    /// \return true if the object is a new object; false if it already exists, or if parameters are invalid
    bool AddInstance(IUnknown* pObject, std::string strCallStack, bool bAppCreated)
    {
        SpAssert(pObject != NULL);

        if (pObject == NULL)
        {
            return false;
        }

        IFacesMap::iterator iObject = m_Objects.find(pObject);

        if (iObject != m_Objects.end())
        {
            // since the object already exists, probably two identical objects were created
            // and the runtime returned the address of a pre-existing object

            // add the call stack so we can track it
            iObject->second.AddCallStack(strCallStack);

            // return false because this isn't a new object
            return false;
        }

        // object doesn't already exist, so add it
        m_Objects[ pObject ] = IFaceData(strCallStack, bAppCreated);

        // increase the vtable refcount
        m_dwRefCount++;

        // return true because this was a new object
        return true;
    }

    /// Removes the IUnknown object as an instance of this virtual table
    /// \param pObject pointer object to remove
    /// \return current refcount of vtable if parameters are invalid or if object isn't in the map; decremented refcount if object is removed
    DWORD RemoveInstance(IUnknown* pObject)
    {
        SpAssert(pObject != NULL);

        if (pObject == NULL)
        {
            return m_dwRefCount;
        }

        IFacesMap::iterator iObject = m_Objects.find(pObject);

        if (iObject == m_Objects.end())
        {
            Log(logWARNING, "Object is not in the database (0x%p).\n", pObject);
            return m_dwRefCount;
        }

        // decrease the vtable refcount
        m_dwRefCount--;

        // object exists, so remove it
        m_Objects.erase(iObject);

        return m_dwRefCount;
    }

    /// Assignment operator
    /// \param obj right
    VTableData& operator = (const VTableData& obj)
    {
        if (&obj != this)
        {
            m_pFunc = obj.m_pFunc;
            m_dwRefCount = obj.m_dwRefCount;
            bPatched = obj.bPatched;

            VtableElementsToPatch.clear();
            m_MineFuncTable.clear();

            for (VtableIndexDatabase::const_iterator it = obj.VtableElementsToPatch.begin(); it  != obj.VtableElementsToPatch.end(); it++)
            {
                VtableElementsToPatch.insert(std::pair<DWORD, const ptrdiff_t*>(it->first, it->second));
            }

            for (VtableIndexDatabase::const_iterator it = obj.m_MineFuncTable.begin(); it  != obj.m_MineFuncTable.end(); it++)
            {
                m_MineFuncTable.insert(std::pair<DWORD, const ptrdiff_t*>(it->first, it->second));
            }

            m_Objects = obj.m_Objects;
        }

        return *this;
    }

    /// Copy constructor
    /// \param obj source obj
    VTableData(const VTableData& obj)
    {
        m_pFunc = obj.m_pFunc;
        m_dwRefCount = obj.m_dwRefCount;
        bPatched = obj.bPatched;

        VtableElementsToPatch.clear();
        m_MineFuncTable.clear();

        for (VtableIndexDatabase::const_iterator it = obj.VtableElementsToPatch.begin(); it  != obj.VtableElementsToPatch.end(); it++)
        {
            VtableElementsToPatch.insert(std::pair<DWORD, const ptrdiff_t*>(it->first, it->second));
        }

        for (VtableIndexDatabase::const_iterator it = obj.m_MineFuncTable.begin(); it  != obj.m_MineFuncTable.end(); it++)
        {
            m_MineFuncTable.insert(std::pair<DWORD, const ptrdiff_t*>(it->first, it->second));
        }

        m_Objects = obj.m_Objects;
    }

private:
    bool bPatched;                               ///< tells if the vtable is patched or not
    ptrdiff_t* m_pFunc;                          ///< pointer to the release function of the IUnknown object this is a vtable for
    DWORD m_dwRefCount;                          ///< the number of references on this virtual table
    VtableIndexDatabase VtableElementsToPatch;   ///< list of entries in the vtable to patch/parched (depending on the value of bPatched)
    VtableIndexDatabase m_MineFuncTable;         ///< list of mine functions
    IFacesMap m_Objects;                         /// map of objects that use this interface
};


/// Helper to hook virtual tables of object's intances
class VTableManager
{
public:
    /// constructor
    /// \param Name a string name to give to this object to aid in debugging
    /// \param dwIndex the index to the release function
    /// \param pFnMine callback function when the last release has been called
    /// \param ulReleaseRef The threshold for RemoveAndDetach() to unpatch vtable and delete VTableMap from m_RealTrampoline
    VTableManager(char* Name, DWORD dwIndex, ptrdiff_t* pFnMine, unsigned long ulReleaseRef = 1);

    /// Attaches to all elements that have been added for patching
    void Attach();

    /// Attach mine functions again in case anything happen that caused vtable to restore to original
    void Reattach();

    /// Detaches from all patched virtual table elements
    void Detach();

    /// Adds a function to the list of those that should be patched
    /// \param pI interface of object that needs to be patched
    /// \param dwIndex of the vtable element to patch
    /// \param pFnMine pointer to function that should be called instead of original function
    void AddVtableElementToPatch(IUnknown* pI, DWORD dwIndex, ptrdiff_t* pFnMine);

    /// destructor, it complains if there are ref leaks
    virtual ~VTableManager();

    /// Gets the ref count of an interface
    ULONG GetRefCount(IUnknown* pI);

    /// \return a list containing a list of all the instances held in the database
    std::vector<IUnknown*> GetInterfacesList();

    /// Dumps a string with all the instances in the database, for debugging
    virtual std::string Report(bool bPrintRefCount);

    /// Adds an object to the virtual table database and detours the virtual table if needed
    /// \param pObject The object that should be added or detoured
    /// \param bAppCreated indicates (true) if the app created this object, or (false) if it was created by PerfStudio
    /// \return true if this object has a new virtual table; false if the virtual table has already been detoured or if the object is already in the database
    bool AddAndDetourIfUnique(IUnknown* pDev, bool bAppCreated);

    /// removes an instance from the database
    ULONG RemoveAndDetach(IUnknown* pDev);

    /// returns a pointer to the original function in the vtable
    const ptrdiff_t* CallReal(const IUnknown* pDev, DWORD dwIndex);

    /// sets a callback that well be called before releasing definitively the instance
    void SetOnReleaseCallBack(OnRelease_type pFn) { m_pOnReleaseCallBack = pFn; }

    /// Print out vtable entry
    /// \param uiStart start index of vtable
    /// \param uiEnd end index of vtable
    void Debug(unsigned int uiStart, unsigned int uiEnd);

    /// Unpatch all vtables managed and remove them
    void UnpatchAndRemoveAll();

    /// Call real release
    /// \param pObj obj
    /// \return the reference counter
    ULONG CallRealRelease(IUnknown* pObj);

public:
    int m_InstanceCount;      ///< number of instance managed

private:
    /// Disable assignement operator
    VTableManager& operator = (const VTableManager& obj);
    /// Disable copy constructor
    VTableManager(const VTableManager& obj);

private:
    DWORD m_dwIndex;                       ///< index to the function in vtable
    VTableMap m_RealTrampoline;            ///< map of virtual tables to original functions and referencing objects
    ptrdiff_t* m_pFnMine;                  ///< function to be called by hook
    OnRelease_type m_pOnReleaseCallBack;   ///< callback function when the object's last release is completed
    unsigned long m_ulReleaseRef;          ///< Indicates the minimum number of reference to unpatch the object
    AMDTMutex* m_mtx;                      ///< Mutex

protected:
    char* m_Name;                          ///< name to display when there are ref leaks
};

#define DISABLE_PROFILING 0

// @}

#endif // _DC_DETOUR_HELPER_H_