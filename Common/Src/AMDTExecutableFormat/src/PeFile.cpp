//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PeFile.cpp
/// \brief This file contains a class for querying a Portable Executable (PE) file information
///
//==================================================================================

#include <PeFile.h>
#include "SymbolEngines/Formats/CoffSymbolEngine.h"
#include "SymbolEngines/Formats/PdbSymbolEngine.h"
#include "SymbolEngines/Formats/DwarfSymbolEngine.h"
#include "SymbolEngines/Formats/StabsSymbolEngine.h"
#include "SymbolEngines/Generics/ProxySymbolEngine.h"

#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osProcess.h>

static const RUNTIME_FUNCTION* FindFirstValidFunctionEntry(const RUNTIME_FUNCTION* pFirst, const RUNTIME_FUNCTION* pLast);
static const RUNTIME_FUNCTION* RFindFirstValidFunctionEntry(const RUNTIME_FUNCTION* pFrom, const RUNTIME_FUNCTION* pFirst);
static const RUNTIME_FUNCTION* BinarySearchFunctionEntry(const RUNTIME_FUNCTION* pFirst, const RUNTIME_FUNCTION* pLast, DWORD val);


PeFile::PeFile(const wchar_t* pImageName) : ExecutableFile(pImageName),
    m_hFile(INVALID_HANDLE_VALUE),
    m_hFileMapping(NULL),
    m_pFileBase(NULL),
    m_pNtHeader(NULL),
    m_pSections(NULL),
    m_isDebugInfoAvailable(false)
{
}

PeFile::~PeFile()
{
    Close();
}

void PeFile::Close()
{
    if (NULL != m_pSymbolEngine)
    {
        delete m_pSymbolEngine;
    }

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

        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Executable (PE) %ls closed", m_modulePath);
    }
}

bool PeFile::Open(gtVAddr loadAddress)
{
    bool ret = false;

    if (IsOpen())
    {
        Close();
    }

    if (LoadImage(m_modulePath))
    {
        DWORD fileSize = GetFileSize(m_hFile, NULL);

        if (INVALID_FILE_SIZE != fileSize && InitializeImageHeaders(fileSize))
        {
            if (GT_INVALID_VADDR == loadAddress)
            {
                m_loadAddress = GetImageBase();
            }
            else
            {
                m_loadAddress = loadAddress;
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

    if (ret)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Executable (PE) %ls opened", m_modulePath);
    }
    else
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Executable (PE) %ls failed to open", m_modulePath);
    }

    return ret;
}

bool PeFile::IsOpen() const
{
    return INVALID_HANDLE_VALUE != m_hFile;
}

bool PeFile::LoadImage(const wchar_t* pImageName)
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

bool PeFile::InitializeNtHeader(DWORD fileSize)
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

bool PeFile::InitializeImageHeaders(DWORD fileSize)
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

gtVAddr PeFile::GetImageBase() const
{
    gtVAddr imageBase;

    if (m_pNtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
    {
        imageBase = reinterpret_cast<const IMAGE_OPTIONAL_HEADER32&>(m_pNtHeader->OptionalHeader).ImageBase;
    }
    else
    {
        imageBase = reinterpret_cast<const IMAGE_OPTIONAL_HEADER64&>(m_pNtHeader->OptionalHeader).ImageBase;
    }

    return imageBase;
}

gtRVAddr PeFile::GetCodeBase() const
{
    return (NULL != m_pNtHeader) ? m_pNtHeader->OptionalHeader.BaseOfCode : GT_INVALID_RVADDR;
}

gtUInt32 PeFile::GetCodeSize() const
{
    return (NULL != m_pNtHeader) ? m_pNtHeader->OptionalHeader.SizeOfCode : 0;
}

gtUInt32 PeFile::GetImageSize() const
{
    gtUInt32 imageSize = 0;

    if (NULL != m_pNtHeader)
    {
        if (m_pNtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
        {
            imageSize = reinterpret_cast<const IMAGE_OPTIONAL_HEADER32&>(m_pNtHeader->OptionalHeader).SizeOfImage;
        }
        else
        {
            imageSize = reinterpret_cast<const IMAGE_OPTIONAL_HEADER64&>(m_pNtHeader->OptionalHeader).SizeOfImage;
        }
    }

    return imageSize;
}

gtUInt32 PeFile::GetImageVersion() const
{
    gtUInt32 version = 0;

    if (NULL != m_pNtHeader)
    {
        if (m_pNtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
        {
            const IMAGE_OPTIONAL_HEADER32& optional = reinterpret_cast<const IMAGE_OPTIONAL_HEADER32&>(m_pNtHeader->OptionalHeader);
            version = (static_cast<gtUInt32>(optional.MajorImageVersion) << 16) | static_cast<gtUInt32>(optional.MinorImageVersion);
        }
        else
        {
            const IMAGE_OPTIONAL_HEADER64& optional = reinterpret_cast<const IMAGE_OPTIONAL_HEADER64&>(m_pNtHeader->OptionalHeader);
            version = (static_cast<gtUInt32>(optional.MajorImageVersion) << 16) | static_cast<gtUInt32>(optional.MinorImageVersion);
        }
    }

    return version;
}

bool PeFile::Is64Bit() const
{
    return NULL != m_pNtHeader && m_pNtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC;
}

unsigned PeFile::GetSectionsCount() const
{
    unsigned numSections = 0;

    if (NULL != m_pNtHeader)
    {
        numSections = m_pNtHeader->FileHeader.NumberOfSections;
    }

    return numSections;
}

gtUInt64 PeFile::GetSignature() const
{
    ULARGE_INTEGER signature;

    if (NULL != m_pNtHeader)
    {
        signature.HighPart = m_pNtHeader->FileHeader.TimeDateStamp;
        signature.LowPart = PeFile::GetImageSize();
        signature.LowPart |= static_cast<unsigned>(PeFile::Is64Bit());
    }
    else
    {
        signature.QuadPart = 0;
    }

    return signature.QuadPart;
}

gtUInt64 PeFile::GetChecksum() const
{
    gtUInt64 checksum = 0;

    if (m_pNtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
    {
        checksum = reinterpret_cast<const IMAGE_OPTIONAL_HEADER32&>(m_pNtHeader->OptionalHeader).CheckSum;
    }
    else
    {
        checksum = reinterpret_cast<const IMAGE_OPTIONAL_HEADER64&>(m_pNtHeader->OptionalHeader).CheckSum;
    }

    return checksum;
}

bool PeFile::IsDebugInfoAvailable() const
{
    return m_isDebugInfoAvailable;
}

const IMAGE_SECTION_HEADER* PeFile::LookupSectionByRva(DWORD rva) const // GetSectionHeader
{
    const IMAGE_SECTION_HEADER* pFoundSection = NULL;

    if (NULL != m_pSections)
    {
        const IMAGE_SECTION_HEADER* pSection = m_pSections;

        for (unsigned i = m_pNtHeader->FileHeader.NumberOfSections; 0 != i; --i)
        {
            if (pSection->VirtualAddress <= rva && rva < (pSection->VirtualAddress + pSection->SizeOfRawData))
            {
                pFoundSection = pSection;
                break;
            }

            ++pSection;
        }
    }

    return pFoundSection;
}

const IMAGE_SECTION_HEADER* PeFile::GetSectionByIndex(unsigned index) const
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

const IMAGE_SECTION_HEADER* PeFile::LookupSectionByName(const char* pName) const
{
    const IMAGE_SECTION_HEADER* pFoundSection = NULL;

    if (NULL != m_pSections && NULL != pName)
    {
        const IMAGE_SECTION_HEADER* pSection = m_pSections;

        size_t lenName = strlen(pName);

        if (IMAGE_SIZEOF_SHORT_NAME < lenName)
        {
            const char* pStringTable = reinterpret_cast<const char*>(m_pFileBase +
                                                                     m_pNtHeader->FileHeader.PointerToSymbolTable +
                                                                     m_pNtHeader->FileHeader.NumberOfSymbols * IMAGE_SIZEOF_SYMBOL);

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

const void* PeFile::TranslateRvaToPtr(DWORD rva) const // GetPtrFromRVA
{
    void* pBytes;
    const IMAGE_SECTION_HEADER* pSection = LookupSectionByRva(rva);

    if (NULL != pSection)
    {
        pBytes = m_pFileBase + (rva - pSection->VirtualAddress) + pSection->PointerToRawData;
    }
    else
    {
        pBytes = NULL;
    }

    return pBytes;
}

const IMAGE_DATA_DIRECTORY* PeFile::GetDirectoryEntry(DWORD directoryEntry) const
{
    const IMAGE_DATA_DIRECTORY* pDir = NULL;

    if (NULL != m_pNtHeader)
    {
        if (m_pNtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
        {
            const IMAGE_OPTIONAL_HEADER32& optional = reinterpret_cast<const IMAGE_OPTIONAL_HEADER32&>(m_pNtHeader->OptionalHeader);

            if (directoryEntry < optional.NumberOfRvaAndSizes && 0 != optional.DataDirectory[directoryEntry].VirtualAddress)
            {
                pDir = optional.DataDirectory + directoryEntry;
            }
        }
        else
        {
            const IMAGE_OPTIONAL_HEADER64& optional = reinterpret_cast<const IMAGE_OPTIONAL_HEADER64&>(m_pNtHeader->OptionalHeader);

            if (directoryEntry < optional.NumberOfRvaAndSizes && 0 != optional.DataDirectory[directoryEntry].VirtualAddress)
            {
                pDir = optional.DataDirectory + directoryEntry;
            }
        }
    }

    return pDir;
}

const void* PeFile::DirectoryEntryToData(DWORD directoryEntry) const
{
    const IMAGE_DATA_DIRECTORY* pDir = GetDirectoryEntry(directoryEntry);
    return NULL != pDir ? TranslateRvaToPtr(pDir->VirtualAddress) : NULL;
}


bool PeFile::IsCodeSection(unsigned index) const
{
    const IMAGE_SECTION_HEADER* pSection = GetSectionByIndex(index);
    return NULL != pSection && 0 != (pSection->Characteristics & IMAGE_SCN_CNT_CODE);
}

unsigned PeFile::LookupSectionIndex(const char* pName) const
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

unsigned PeFile::LookupSectionIndex(gtRVAddr rva) const
{
    const IMAGE_SECTION_HEADER* pSection = LookupSectionByRva(rva);

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

const gtUByte* PeFile::GetSectionBytes(unsigned index) const
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

bool PeFile::GetSectionRvaLimits(unsigned index, gtRVAddr& startRva, gtRVAddr& endRva) const
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

gtUInt32 PeFile::GetSectionSize(unsigned index) const
{
    const IMAGE_SECTION_HEADER* pSection = GetSectionByIndex(index);

    gtUInt32 size;

    if (NULL != pSection)
    {
        size = pSection->SizeOfRawData;
    }
    else
    {
        size = 0;
    }

    return size;
}

const char* PeFile::GetSectionName(unsigned index, unsigned* pLength) const
{
    const IMAGE_SECTION_HEADER* pSection = GetSectionByIndex(index);

    const char* pName = NULL;

    if (NULL != pSection)
    {
        if ('/' == pSection->Name[0])
        {
            const char* pStringTable = reinterpret_cast<const char*>(m_pFileBase +
                                                                     m_pNtHeader->FileHeader.PointerToSymbolTable +
                                                                     m_pNtHeader->FileHeader.NumberOfSymbols * IMAGE_SIZEOF_SYMBOL);
            pName = &pStringTable[atoi(reinterpret_cast<const char*>(pSection->Name) + 1)];

            if (NULL != pLength)
            {
                *pLength = static_cast<unsigned>(strlen(pName));
            }
        }
        else
        {
            pName = reinterpret_cast<const char*>(pSection->Name);

            if (NULL != pLength)
            {
                unsigned len = IMAGE_SIZEOF_SHORT_NAME;

                while (0 != len && '\0' == pName[len - 1U])
                {
                    --len;
                }

                *pLength = len;
            }
        }
    }

    return pName;
}

gtRVAddr PeFile::GetEntryPoint() const
{
    return NULL != m_pNtHeader ? m_pNtHeader->OptionalHeader.AddressOfEntryPoint : GT_INVALID_RVADDR;
}

const RUNTIME_FUNCTION* PeFile::LookupFunctionEntry64(gtRVAddr rva) const
{
    const RUNTIME_FUNCTION* pFuncs = NULL;

    const IMAGE_DATA_DIRECTORY* pDir = GetDirectoryEntry(IMAGE_DIRECTORY_ENTRY_EXCEPTION);

    if (NULL != pDir && 0 != pDir->Size)
    {
        pFuncs = static_cast<const RUNTIME_FUNCTION*>(TranslateRvaToPtr(pDir->VirtualAddress));

        if (NULL != pFuncs)
        {
            DWORD size = pDir->Size / sizeof(RUNTIME_FUNCTION);
            pFuncs = BinarySearchFunctionEntry(pFuncs, pFuncs + size, rva);
        }
    }

    return pFuncs;
}

bool PeFile::InitializeSymbolEngine(const wchar_t* pSearchPath,
                                    const wchar_t* pServerList,
                                    const wchar_t* pCachePath)
{
    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Executable (PE) %ls started Symbol Engine initialization", m_modulePath);

    if (NULL != m_pSymbolEngine)
    {
        delete m_pSymbolEngine;
        m_pSymbolEngine = NULL;
    }

    // Set whether the debug info pertaining to inlined symbols to be processed or not
    // This will be consumed by symbol engines

    // Baskar: Turn off inline processing if env var AMDT_CPUPROFILE_PROCESS_INLINE is set to "NO"
    gtString processInlineEnv;
    bool processInline = IsSystemExecutable() ? false : true;

    if (osGetCurrentProcessEnvVariableValue(L"AMDT_CPUPROFILE_PROCESS_INLINE", processInlineEnv))
    {
        processInline = !processInlineEnv.isEqualNoCase(L"NO");
    }

    SetProcessInlineInfo(processInline);

    PVOID oldRedirectValue = nullptr;
    BOOL doRedirect = FALSE;
    IsWow64Process(GetCurrentProcess(), &doRedirect);

    if (doRedirect)
    {
        doRedirect = Wow64DisableWow64FsRedirection(&oldRedirectValue);
    }

    wchar_t* (*pfnDemangleName)(const char*);

    const IMAGE_DATA_DIRECTORY* pImageDataDir = GetDirectoryEntry(IMAGE_DIRECTORY_ENTRY_DEBUG);
    const IMAGE_DEBUG_DIRECTORY* pDebugDir =
        static_cast<const IMAGE_DEBUG_DIRECTORY*>(NULL != pImageDataDir ? TranslateRvaToPtr(pImageDataDir->VirtualAddress) : NULL);

    if (NULL != pDebugDir)
    {
        PdbSymbolEngine* pPdbEngine = new PdbSymbolEngine();

        if (pPdbEngine->Initialize(*this,
                                   pDebugDir, (pImageDataDir->Size / sizeof(IMAGE_DEBUG_DIRECTORY)),
                                   pSearchPath, pServerList, pCachePath))
        {
            m_pSymbolEngine = pPdbEngine;
            m_isDebugInfoAvailable = true;

            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Executable (PE) %ls initialized Symbol Engine: PDB", m_modulePath);
        }
        else
        {
            delete pPdbEngine;

            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Executable (PE) %ls failed to initialize Symbol Engine: PDB", m_modulePath);
        }

        pfnDemangleName = SymbolEngine::DemangleNameVS;
    }
    else
    {
        pfnDemangleName = SymbolEngine::DemangleNameIA;
    }

    if (NULL == m_pSymbolEngine)
    {
        CoffSymbolEngine* pCoffEngine = new CoffSymbolEngine();

        if (pCoffEngine->Initialize(*this, pfnDemangleName))
        {
            m_pSymbolEngine = pCoffEngine;
            //m_isDebugInfoAvailable = true;

            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Executable (PE) %ls initialized Symbol Engine: COFF", m_modulePath);
        }
        else
        {
            delete pCoffEngine;
            pCoffEngine = NULL;

            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Executable (PE) %ls failed to initialize Symbol Engine: COFF", m_modulePath);
        }

        unsigned sectionsCount = GetSectionsCount();

        if (NULL != LookupSectionByName(".debug_abbrev") && NULL != LookupSectionByName(".debug_info"))
        {
            DwarfSymbolEngine* pDwarfEngine = new DwarfSymbolEngine();

            if (pDwarfEngine->Initialize(*this, pCoffEngine))
            {
                m_pSymbolEngine = pDwarfEngine;
                m_isDebugInfoAvailable = true;

                OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Executable (PE) %ls initialized Symbol Engine: DWARF", m_modulePath);
            }
            else
            {
                delete pDwarfEngine;

                OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Executable (PE) %ls failed to initialize Symbol Engine: DWARF", m_modulePath);
            }
        }
        else
        {
            unsigned stabIndex = LookupSectionIndex(".stab");

            if (stabIndex < sectionsCount)
            {
                unsigned stabstrIndex = LookupSectionIndex(".stabstr");

                if (stabstrIndex < sectionsCount)
                {
                    StabsSymbolEngine* pStabsEngine = new StabsSymbolEngine();

                    if (pStabsEngine->Initialize(*this, stabIndex, stabstrIndex, pCoffEngine))
                    {
                        m_pSymbolEngine = pStabsEngine;
                        m_isDebugInfoAvailable = true;

                        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Executable (PE) %ls initialized Symbol Engine: STABS", m_modulePath);
                    }
                    else
                    {
                        delete pStabsEngine;

                        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Executable (PE) %ls failed to initialize Symbol Engine: STABS", m_modulePath);
                    }
                }
            }
        }

        unsigned debuglinkIndex = LookupSectionIndex(".gnu_debuglink");

        if (debuglinkIndex >= sectionsCount)
        {
            debuglinkIndex = LookupSectionIndex(".gnu_deb");
        }

        if (debuglinkIndex < sectionsCount)
        {
            const gtUByte* pDebuglink = GetSectionBytes(debuglinkIndex);

            if (NULL != pDebuglink)
            {
                gtUInt32 szDebuglink = GetSectionSize(debuglinkIndex);

                if (NULL != memchr(pDebuglink, '\0', szDebuglink))
                {
                    wchar_t fileName[OS_MAX_PATH];

                    if ((size_t)(-1) != mbstowcs(fileName, reinterpret_cast<const char*>(pDebuglink), OS_MAX_PATH))
                    {
                        fileName[OS_MAX_PATH - 1] = L'\0';

                        wchar_t imagePath[OS_MAX_PATH];

                        if (0U != SymbolEngine::FindFile(pSearchPath, fileName, imagePath))
                        {
                            ProxySymbolEngine<PeFile>* pProxyEngine = new ProxySymbolEngine<PeFile>(imagePath);

                            if (pProxyEngine->Initialize(m_loadAddress))
                            {
                                if (NULL != m_pSymbolEngine)
                                {
                                    static_cast<ModularSymbolEngine*>(m_pSymbolEngine)->Splice(*pProxyEngine->GetInternalEngine());
                                    delete m_pSymbolEngine;
                                }

                                m_pSymbolEngine = pProxyEngine;
                                m_isDebugInfoAvailable = true;
                            }
                            else
                            {
                                delete pProxyEngine;
                            }
                        }
                    }
                }
            }
        }
    }

    if (doRedirect)
    {
        Wow64RevertWow64FsRedirection(oldRedirectValue);
    }

    return NULL != m_pSymbolEngine;
}

bool PeFile::FindClrInfo() const
{
    return NULL != DirectoryEntryToData(IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR);
}

bool PeFile::FindClrInfo(gtUInt32& flags) const
{
    gtUInt16 version;
    return FindClrInfo(flags, version, version);
}

bool PeFile::FindClrInfo(gtUInt32& flags, gtUInt16& major, gtUInt16& minor) const
{
    const IMAGE_COR20_HEADER* pComDesc =
        static_cast<const IMAGE_COR20_HEADER*>(DirectoryEntryToData(IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR));
    bool ret = NULL != pComDesc;

    if (ret)
    {
        flags = pComDesc->Flags;
        major = pComDesc->MajorRuntimeVersion;
        minor = pComDesc->MinorRuntimeVersion;
    }

    return ret;
}

ClrPlatform PeFile::FindClrPlatform() const
{
    ClrPlatform platform = CLR_PLATFORM_UNKNOWN;
    gtUInt32 flags;

    if (FindClrInfo(flags))
    {
        if (Is64Bit())
        {
            platform = CLR_PLATFORM_X64;
        }
        else
        {
            if (0 != (flags & COMIMAGE_FLAGS_32BITREQUIRED))
            {
                platform = CLR_PLATFORM_X86;
            }
            else
            {
                platform = CLR_PLATFORM_ANYCPU;
            }
        }
    }

    return platform;
}

const gtUByte* PeFile::GetMemoryBlock(gtRVAddr rva, gtUInt32& size) const
{
    const gtUByte* pBytes = NULL;
    const IMAGE_SECTION_HEADER* pSection = LookupSectionByRva(rva);

    if (NULL != pSection)
    {
        DWORD sectionOffset = rva - pSection->VirtualAddress;
        DWORD remainingSize = pSection->SizeOfRawData - sectionOffset;

        if (remainingSize < size)
        {
            size = remainingSize;
        }

        pBytes = m_pFileBase + sectionOffset + pSection->PointerToRawData;
    }

    return pBytes;
}


static const RUNTIME_FUNCTION* FindFirstValidFunctionEntry(const RUNTIME_FUNCTION* pFirst, const RUNTIME_FUNCTION* pLast)
{
    while (pFirst != pLast && 0 == pFirst->BeginAddress)
    {
        ++pFirst;
    }

    return pFirst;
}

static const RUNTIME_FUNCTION* RFindFirstValidFunctionEntry(const RUNTIME_FUNCTION* pFrom, const RUNTIME_FUNCTION* pFirst)
{
    while (pFrom >= pFirst && 0 == pFrom->BeginAddress)
    {
        --pFrom;
    }

    return pFrom;
}

static const RUNTIME_FUNCTION* BinarySearchFunctionEntry(const RUNTIME_FUNCTION* pFirst, const RUNTIME_FUNCTION* pLast, DWORD val)
{
    DWORD count = static_cast<DWORD>(pLast - pFirst);

    while (0 < count)
    {
        // Divide and conquer
        DWORD count2 = count / 2;
        const RUNTIME_FUNCTION* pMid = pFirst + count2;

        if (0 == pMid->BeginAddress)
        {
            const RUNTIME_FUNCTION* pNewMid;
            pNewMid = RFindFirstValidFunctionEntry(pMid - 1, pFirst);

            if (pNewMid >= pFirst)
            {
                if (pNewMid->EndAddress > val)
                {
                    count = static_cast<DWORD>(pNewMid - pFirst);
                    continue;
                }
            }

            pNewMid = FindFirstValidFunctionEntry(pMid + 1, pLast);

            if (pNewMid != pLast)
            {
                if (pNewMid->EndAddress <= val)
                {
                    ++pNewMid;
                    count -= static_cast<DWORD>(pNewMid - pFirst);
                    pFirst = pNewMid;
                    continue;
                }

                if (pNewMid->BeginAddress <= val)
                {
                    return pNewMid;
                }
            }

            return NULL;
        }

        if (pMid->EndAddress <= val)
        {
            // Try top half
            pFirst = pMid + 1;
            count -= count2 + 1;
        }
        else
        {
            count = count2;
        }
    }

    if (pFirst != pLast && pFirst->BeginAddress <= val)
    {
        return pFirst;
    }

    return NULL;
}
