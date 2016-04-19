//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file MemoryMap.cpp
/// \brief Memory map utility class.
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/Backend/AMDTCpuProfilingTranslation/src/Windows/MemoryMap.cpp#6 $
// Last checkin:   $DateTime: 2016/04/14 02:42:23 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569060 $
//=====================================================================

#include "MemoryMap.h"
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Memory map the given file
HRESULT MemoryMap::Map(const wchar_t* fileName, DWORD hiOffset, DWORD lowOffset)
{
    if (NULL == fileName)
    {
        return E_INVALIDARG;
    }

    HRESULT ret = E_FAIL;

    // - Create a file handle
    // - Create an unnamed file mapping
    // - Map the file
    m_fileHandle = CreateFile(fileName,
                              GENERIC_READ,
                              FILE_SHARE_READ,
                              NULL,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL);

    if (INVALID_HANDLE_VALUE == m_fileHandle)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Could not open file(%ls), error(0x%0x)", fileName, GetLastError());
    }
    else
    {
        // Create a file mapping
        m_fileMap = CreateFileMapping(m_fileHandle, // file handle
                                      NULL,         // default security
                                      PAGE_READONLY,
                                      hiOffset,     // size of mapping object, high
                                      lowOffset,    // size of mapping object, low
                                      NULL);        // name of the mapping object

        if (NULL == m_fileMap)
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Could not memory map the file(%ls), error(0x%0x)", fileName, GetLastError());
            UnMap();
        }
        else
        {
            ret = S_OK;

            if (GetLastError() == ERROR_ALREADY_EXISTS)
            {
                OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Memory Map for file(%ls) already exists", fileName);
                // Should this be treated as error ? - No.
            }

            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Memory Mapped file(%ls)", fileName);
        }
    }

    return ret;
}

HRESULT MemoryMap::CreateView(gtUInt64 offset, DWORD length)
{
    HRESULT hr;

    // Create a Mapped view of the file
    m_mapAddress = MapViewOfFile(m_fileMap,         // handle to mapping object
                                 FILE_MAP_READ,      // read access
                                 reinterpret_cast<ULARGE_INTEGER&>(offset).HighPart, // high-order 32 bits of file offset
                                 reinterpret_cast<ULARGE_INTEGER&>(offset).LowPart,  // low-order  32-bits of file offset
                                 length);            // number of bytes to map

    if (NULL != m_mapAddress)
    {
        hr = S_OK;
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"m_mapAddess : 0x%p", m_mapAddress);
    }
    else
    {
        DWORD errCode = GetLastError();

        if (ERROR_NOT_ENOUGH_MEMORY == errCode)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            hr = E_FAIL;
        }

        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Could not create view of the mapped file, error 0x%0x", errCode);
    }

    return hr;
}

// Unmap the view of the file
void MemoryMap::DestroyView()
{

    if (NULL != m_mapAddress)
    {
        if (! UnmapViewOfFile(m_mapAddress))
        {
            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"UnmapViewOfFile failed with error(0x%0x)", GetLastError());
        }

        m_mapAddress = NULL;
    }
}


// Memory unmap function
HRESULT MemoryMap::UnMap()
{
    DestroyView();

    if (NULL != m_fileMap)
    {
        CloseHandle(m_fileMap);
        m_fileMap = NULL;
    }

    if (INVALID_HANDLE_VALUE != m_fileHandle)
    {
        CloseHandle(m_fileHandle);
        m_fileHandle = INVALID_HANDLE_VALUE;
    }

    OS_OUTPUT_DEBUG_LOG(L"Closed mapping handles", OS_DEBUG_LOG_DEBUG);

    return S_OK;
}
