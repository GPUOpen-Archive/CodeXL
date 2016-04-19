//=============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Support class for vtable patching
//=============================================================================

#ifndef VTABLE_DATA
#define VTABLE_DATA

/// simplifies the type for a virtual table
typedef ptrdiff_t** VTABLE;

class VTableData;

/// map of a vtable to a map of objects which use that vtable
typedef std::map< VTABLE, VTableData > VTableMap;

/// map between function indices and function pointers
typedef std::map< DWORD, ptrdiff_t* > VtableIndexDatabase;

/// \addtogroup Patcher

//-----------------------------------------------------------------------------
/// Stores information about a virtual table
//-----------------------------------------------------------------------------
class VTableData
{
public:

    /// pointer to the release function of the IUnknown object this is a vtable for
    ptrdiff_t* m_pFunc;

    /// the nubmer of references on this virtual table
    DWORD m_dwRefCount;

    //-----------------------------------------------------------------------------
    /// Default constructor
    //-----------------------------------------------------------------------------
    VTableData()
    {
        m_dwRefCount = 0;
        m_pFunc = nullptr;
        m_bPatched = false;
    }

    /// tells if the vtable is patched or not
    bool m_bPatched;

    /// list of entries in the vtable to patch/patched (depending on the value of bPatched)
    VtableIndexDatabase m_VtableFunctionsToPatch;

    /// map of objects that use this interface
    IFacesMap m_Objects;

    //-----------------------------------------------------------------------------
    /// Adds the IUnknown object as an instance of this virtual table
    /// \param pObject pointer object to add
    /// \param strCallStack The call stack to store with the object
    /// \param bAppCreated true if the app created this object; false if PerfStudio created it
    /// \return true if the object is a new object; false if it already exists, or if parameters are invalid
    //-----------------------------------------------------------------------------
    bool AddInstance(IUnknown* pObject, std::string strCallStack, bool bAppCreated)
    {
        PsAssert(pObject != nullptr);

        if (pObject == nullptr)
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

    //-----------------------------------------------------------------------------
    /// Removes the IUnknown object as an instance of this virtual table
    /// \param pObject pointer object to remove
    /// \return current refcount of vtable if parameters are invalid or if object isn't in the map; decremented refcount if object is removed
    //-----------------------------------------------------------------------------
    DWORD RemoveInstance(IUnknown* pObject)
    {
        PsAssert(pObject != nullptr);

        if (pObject == nullptr)
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
};

#endif // VTABLE_DATA