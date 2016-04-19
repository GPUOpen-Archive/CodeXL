//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file MemoryMap.h
/// \brief Memory map utility class.
///
//==================================================================================

#ifndef _MEMORYMAP_H_
#define _MEMORYMAP_H_


#include <AMDTOSWrappers/Include/osOSDefinitions.h>

//
// Class MemoryMap
//  Interface for Memory Map
//

class MemoryMap
{
public:

    MemoryMap::MemoryMap() : m_fileHandle(INVALID_HANDLE_VALUE), m_fileMap(NULL), m_mapAddress(NULL)
    {
    }

    MemoryMap::~MemoryMap()
    {
        UnMap();
    }

    // Memory map the given file
    HRESULT Map(const wchar_t* fileName, DWORD hiOffset, DWORD lowOffset);
    HRESULT Map(const wchar_t* fileName) { return Map(fileName, 0, 0); }

    // Create a mapped view of the file for the given offset and length
    HRESULT CreateView(gtUInt64 offset, DWORD length);

    // Remove the view
    void DestroyView();

    // Memory unmap
    HRESULT UnMap();

    bool isMapped() const { return NULL != m_mapAddress; }

    void* GetMappedAddress() { return m_mapAddress; }

    void* operator*() { return m_mapAddress; }

private:
    HANDLE m_fileHandle;
    HANDLE m_fileMap;
    LPVOID m_mapAddress;
};

#endif // _MEMORYMAP_H_
