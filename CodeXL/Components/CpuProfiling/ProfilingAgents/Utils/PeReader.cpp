//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PeReader.cpp
///
//==================================================================================

#include "ExecutableReader.h"

ExecutableReader::ExecutableReader() : m_imageBase(0),
    m_hFile(INVALID_HANDLE_VALUE),
    m_hFileMapping(NULL),
    m_pFileBase(NULL),
    m_pNtHeader(NULL),
    m_pSections(NULL)
{
}

ExecutableReader::~ExecutableReader()
{
    Close();
}

void ExecutableReader::Close()
{
    if (NULL != m_pFileBase)
    {
        UnmapViewOfFile(m_pFileBase);
        m_pFileBase = NULL;
    }

    if (NULL != m_hFileMapping)
    {
        CloseHandle(m_hFileMapping);
        m_hFileMapping = NULL;
    }

    if (INVALID_HANDLE_VALUE != m_hFile)
    {
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
    }
}

bool ExecutableReader::Open(const wchar_t* pModulePath)
{
    bool ret = false;

    if (IsOpen())
    {
        Close();
    }

    if (LoadImage(pModulePath))
    {
        DWORD fileSize = GetFileSize(m_hFile, NULL);

        if (INVALID_FILE_SIZE != fileSize && InitializeImageHeaders(fileSize))
        {
            if (m_pNtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
            {
                m_imageBase = reinterpret_cast<const IMAGE_OPTIONAL_HEADER32&>(m_pNtHeader->OptionalHeader).ImageBase;
            }
            else
            {
                m_imageBase = reinterpret_cast<const IMAGE_OPTIONAL_HEADER64&>(m_pNtHeader->OptionalHeader).ImageBase;
            }

            ret = true;
        }
        else
        {
            UnmapViewOfFile(m_pFileBase);
            m_pFileBase = NULL;
            CloseHandle(m_hFileMapping);
            m_hFileMapping = NULL;
            CloseHandle(m_hFile);
            m_hFile = INVALID_HANDLE_VALUE;
        }
    }

    return ret;
}

bool ExecutableReader::IsOpen() const
{
    return INVALID_HANDLE_VALUE != m_hFile;
}

bool ExecutableReader::LoadImage(const wchar_t* pImageName)
{
    PVOID oldRedirectValue = nullptr;
    BOOL doRedirect = FALSE;
    IsWow64Process(GetCurrentProcess(), &doRedirect);

    if (doRedirect)
    {
        doRedirect = Wow64DisableWow64FsRedirection(&oldRedirectValue);
    }

    m_hFile = CreateFileW(pImageName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

    if (INVALID_HANDLE_VALUE != m_hFile)
    {
        m_hFileMapping = CreateFileMappingW(m_hFile, NULL, PAGE_READONLY, 0, 0, NULL);

        if (NULL != m_hFileMapping)
        {
            m_pFileBase = static_cast<gtUByte*>(MapViewOfFile(m_hFileMapping, FILE_MAP_READ, 0, 0, 0));

            if (NULL == m_pFileBase)
            {
                CloseHandle(m_hFileMapping);
                m_hFileMapping = NULL;
                CloseHandle(m_hFile);
                m_hFile = INVALID_HANDLE_VALUE;
            }
        }
        else
        {
            CloseHandle(m_hFile);
            m_hFile = INVALID_HANDLE_VALUE;
        }
    }

    if (doRedirect)
    {
        Wow64RevertWow64FsRedirection(oldRedirectValue);
    }

    return NULL != m_pFileBase;
}

bool ExecutableReader::InitializeNtHeader(DWORD fileSize)
{
    if (sizeof(IMAGE_NT_HEADERS32) <= fileSize)
    {
        IMAGE_DOS_HEADER* pDosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(m_pFileBase);

        if (IMAGE_DOS_SIGNATURE == pDosHeader->e_magic)
        {
            if (sizeof(IMAGE_DOS_HEADER) <= pDosHeader->e_lfanew)
            {
                const DWORD pos = static_cast<DWORD>(pDosHeader->e_lfanew);

                if (pos < fileSize && sizeof(IMAGE_NT_HEADERS32) <= (fileSize - pos))
                {
                    m_pNtHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(m_pFileBase + pDosHeader->e_lfanew);
                }
            }
        }
        else if (IMAGE_NT_SIGNATURE == pDosHeader->e_magic)
        {
            m_pNtHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(pDosHeader);
        }
    }

    return NULL != m_pNtHeader;
}

bool ExecutableReader::InitializeImageHeaders(DWORD fileSize)
{
    if (InitializeNtHeader(fileSize))
    {
        if (IMAGE_NT_SIGNATURE == m_pNtHeader->Signature && 0 != m_pNtHeader->FileHeader.SizeOfOptionalHeader)
        {
            const IMAGE_OPTIONAL_HEADER& optional = m_pNtHeader->OptionalHeader;

            if (IMAGE_NT_OPTIONAL_HDR32_MAGIC == optional.Magic)
            {
                if (optional.MajorLinkerVersion < 3 && optional.MinorLinkerVersion < 5)
                {
                    m_pNtHeader = NULL;
                }
            }
            else if (IMAGE_NT_OPTIONAL_HDR64_MAGIC != optional.Magic)
            {
                m_pNtHeader = NULL;
            }

            if (NULL != m_pNtHeader)
            {
                m_pSections = IMAGE_FIRST_SECTION(m_pNtHeader);

                if (0 == m_pNtHeader->FileHeader.NumberOfSections ||
                    reinterpret_cast<gtUByte*>(m_pSections + m_pNtHeader->FileHeader.NumberOfSections) > (m_pFileBase + fileSize))
                {
                    m_pSections = NULL;
                    m_pNtHeader = NULL;
                }
            }
        }
        else
        {
            m_pNtHeader = NULL;
        }
    }

    return NULL != m_pSections;
}

gtRVAddr ExecutableReader::GetCodeBase() const
{
    return (NULL != m_pNtHeader) ? m_pNtHeader->OptionalHeader.BaseOfCode : GT_INVALID_RVADDR;
}

gtUInt32 ExecutableReader::GetCodeSize() const
{
    return (NULL != m_pNtHeader) ? m_pNtHeader->OptionalHeader.SizeOfCode : 0;
}

gtVAddr ExecutableReader::GetImageBase() const
{
    return m_imageBase;
}

bool ExecutableReader::Is64Bit() const
{
    return NULL != m_pNtHeader && m_pNtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC;
}

unsigned ExecutableReader::GetSectionsCount() const
{
    unsigned numSections = 0;

    if (NULL != m_pNtHeader)
    {
        numSections = m_pNtHeader->FileHeader.NumberOfSections;
    }

    return numSections;
}

bool ExecutableReader::GetSectionRvaLimits(unsigned index, gtRVAddr& startRva, gtRVAddr& endRva) const
{
    const IMAGE_SECTION_HEADER* pSection = GetSectionByIndex(index);

    bool ret;

    if (NULL != pSection)
    {
        startRva = pSection->VirtualAddress;
        endRva = startRva + pSection->Misc.VirtualSize;
        ret = true;
    }
    else
    {
        ret = false;
    }

    return ret;
}

unsigned ExecutableReader::LookupSectionIndex(const char* pName) const
{
    const IMAGE_SECTION_HEADER* pSection = LookupSectionByName(pName);

    unsigned index;

    if (NULL != pSection)
    {
        index = static_cast<unsigned>(pSection - m_pSections);
    }
    else
    {
        index = m_pNtHeader->FileHeader.NumberOfSections;
    }

    return index;
}

const gtUByte* ExecutableReader::GetSectionBytes(unsigned index) const
{
    const IMAGE_SECTION_HEADER* pSection = GetSectionByIndex(index);

    const gtUByte* pBytes;

    if (NULL != pSection)
    {
        pBytes = m_pFileBase + pSection->PointerToRawData;
    }
    else
    {
        pBytes = NULL;
    }

    return pBytes;
}

bool ExecutableReader::IsCodeSection(unsigned index) const
{
    const IMAGE_SECTION_HEADER* pSection = GetSectionByIndex(index);
    return NULL != pSection && 0 != (pSection->Characteristics & IMAGE_SCN_CNT_CODE);
}

const char* ExecutableReader::GetSectionShortName(unsigned index) const
{
    const IMAGE_SECTION_HEADER* pSection = GetSectionByIndex(index);
    return (NULL != pSection) ? reinterpret_cast<const char*>(pSection->Name) : "";
}

const IMAGE_SECTION_HEADER* ExecutableReader::GetSectionByIndex(unsigned index) const
{
    const IMAGE_SECTION_HEADER* pSection = NULL;

    if (NULL != m_pSections)
    {
        if (index < m_pNtHeader->FileHeader.NumberOfSections)
        {
            pSection = m_pSections + index;
        }
    }

    return pSection;
}

const IMAGE_SECTION_HEADER* ExecutableReader::LookupSectionByName(const char* pName) const
{
    const IMAGE_SECTION_HEADER* pFoundSection = NULL;

    if (NULL != m_pSections && NULL != pName)
    {
        const IMAGE_SECTION_HEADER* pSection = m_pSections;

        size_t lenName = strlen(pName);

        if (IMAGE_SIZEOF_SHORT_NAME < lenName)
        {
            const char* pStringTable = reinterpret_cast<const char*>(m_pFileBase +
                                                                     m_pNtHeader->FileHeader.PointerToSymbolTable + m_pNtHeader->FileHeader.NumberOfSymbols * IMAGE_SIZEOF_SYMBOL);

            for (unsigned i = m_pNtHeader->FileHeader.NumberOfSections; 0 != i; --i)
            {
                if ('/' == pSection->Name[0] &&
                    0 == strcmp(&pStringTable[atoi(reinterpret_cast<const char*>(pSection->Name) + 1)], pName))
                {
                    pFoundSection = pSection;
                    break;
                }

                ++pSection;
            }
        }
        else
        {
            char fullName[IMAGE_SIZEOF_SHORT_NAME] = {0};
            memcpy(fullName, pName, lenName);

            for (unsigned i = m_pNtHeader->FileHeader.NumberOfSections; 0 != i; --i)
            {
                if (0 == memcmp(pSection->Name, fullName, IMAGE_SIZEOF_SHORT_NAME))
                {
                    pFoundSection = pSection;
                    break;
                }

                ++pSection;
            }
        }
    }

    return pFoundSection;
}
