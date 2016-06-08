//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ElfFile.cpp
/// \brief This file contains a class for querying a ELF file information
///
//==================================================================================

#include <ElfFile.h>
#include "SymbolEngines/Formats/ElfSymbolEngine.h"
#include "SymbolEngines/Formats/DwarfSymbolEngine.h"
#include "SymbolEngines/Formats/StabsSymbolEngine.h"
#include "SymbolEngines/Generics/ProxySymbolEngine.h"
#include <gelf.h>
#include <fcntl.h>
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include <io.h>
#else
    #include <unistd.h>
#endif

#include <AMDTOSWrappers/Include/osDebugLog.h>

static Elf64_Addr ExtractImageBase(Elf* pElf);
static Elf_Scn* LookupSectionByName(Elf* pElf, const char* pName, GElf_Shdr& shdr);
static Elf_Scn* LookupSectionByAddr(Elf* pElf, Elf64_Addr addr, GElf_Shdr& shdr);


ElfFile::ElfFile(const wchar_t* pImageName) : ExecutableFile(pImageName),
    m_imageBase(0),
    m_pElf(NULL),
    m_fd(-1),
    m_signature(0),
    m_checksum(0),
    m_isDebugInfoAvailable(false)
{
}

ElfFile::~ElfFile()
{
    Close();
}

void ElfFile::Close()
{
    if (NULL != m_pSymbolEngine)
    {
        delete m_pSymbolEngine;
    }

    if (NULL != m_pElf)
    {
        elf_end(m_pElf);
        m_pElf = NULL;
    }

    if (m_fd >= 0)
    {
        close(m_fd);
        m_fd = -1;

        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Executable (ELF) %ls closed", m_modulePath);
    }
}

bool ElfFile::Open(gtVAddr loadAddress)
{
    bool ret = false;

    // Initialize libelf
    if (EV_NONE != elf_version(EV_CURRENT))
    {
#if AMDT_BUILD_TARGET != AMDT_WINDOWS_OS
        char modulePathMb[OS_MAX_PATH];

        // convert to char
        if (size_t(-1) != wcstombs(modulePathMb, m_modulePath, OS_MAX_PATH))
        {
            m_fd = open(modulePathMb, O_RDONLY);
#else
        {
            m_fd = _wopen(m_modulePath, O_RDONLY | O_BINARY);
#endif

            if (m_fd >= 0)
            {
                gtUInt32 fileSize = static_cast<gtUInt32>(static_cast<unsigned long>(lseek(m_fd, 0, SEEK_END)));
                lseek(m_fd, 0, SEEK_SET);

                m_pElf = elf_begin(m_fd, ELF_C_READ, NULL);

                if (NULL != m_pElf)
                {
                    if (ELF_K_ELF == elf_kind(m_pElf))
                    {
                        m_imageBase = ExtractImageBase(m_pElf);

                        if (GT_INVALID_VADDR == loadAddress)
                        {
                            m_loadAddress = m_imageBase;
                        }
                        else
                        {
                            m_loadAddress = loadAddress;
                        }

                        m_checksum = static_cast<gtUInt64>(gelf_checksum(m_pElf));

                        gtUInt32 checksum = static_cast<gtUInt32>(m_checksum);
                        checksum ^= (fileSize << 16);

                        m_signature = static_cast<gtUInt64>(GetImageSize());
                        m_signature |= static_cast<gtUInt64>(Is64Bit());
                        m_signature |= static_cast<gtUInt64>(checksum) << 32;

                        ret = true;
                    }
                }
                else
                {
                    close(m_fd);
                    m_fd = -1;
                }
            }
        }
    }

    if (ret)
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Executable (ELF) %ls opened", m_modulePath);
    }
    else
    {
        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_ERROR, L"Executable (ELF) %ls failed to open", m_modulePath);
    }

    return ret;
}

bool ElfFile::IsOpen() const
{
    return m_fd >= 0;
}

gtVAddr ElfFile::GetImageBase() const
{
    return m_imageBase;
}

gtRVAddr ElfFile::GetCodeBase() const
{
    GElf_Shdr shdr;
    Elf_Scn* pScn = LookupSectionByName(m_pElf, ".text", shdr);
    return (NULL != pScn) ? static_cast<gtRVAddr>(shdr.sh_addr - m_imageBase) : GT_INVALID_RVADDR;
}

gtUInt32 ElfFile::GetCodeSize() const
{
    GElf_Shdr shdr;
    Elf_Scn* pScn = LookupSectionByName(m_pElf, ".text", shdr);
    return (NULL != pScn) ? static_cast<gtUInt32>(shdr.sh_size) : 0;
}

gtUInt32 ElfFile::GetImageSize() const
{
    gtUInt32 imageSize = 0;

    Elf_Scn* scn = NULL;

    while (NULL != (scn = elf_nextscn(m_pElf, scn)))
    {
        GElf_Shdr shdr;

        if (NULL != gelf_getshdr(scn, &shdr))
        {
            Elf64_Xword size = shdr.sh_size;
            Elf64_Xword align =  shdr.sh_addralign;

            if ((align > 1) && !(align & (align - 1)))
            {
                size = (size + align - 1) & ~(align - 1);
            }

            imageSize += static_cast<gtUInt32>(size);
        }
    }

    return imageSize;
}

bool ElfFile::Is64Bit() const
{
    return ELFCLASS64 == gelf_getclass(m_pElf);
}

unsigned ElfFile::GetSectionsCount() const
{
    size_t numSections = 0;
    elf_getshdrnum(m_pElf, &numSections);
    return static_cast<unsigned>(numSections);
}

gtUInt64 ElfFile::GetSignature() const
{
    return m_signature;
}

gtUInt64 ElfFile::GetChecksum() const
{
    return m_checksum;
}

bool ElfFile::IsDebugInfoAvailable() const
{
    return m_isDebugInfoAvailable;
}

bool ElfFile::IsCodeSection(unsigned index) const
{
    bool ret = false;

    Elf_Scn* pScn = elf_getscn(m_pElf, index);

    if (NULL != pScn)
    {
        GElf_Shdr shdr;

        if (NULL != gelf_getshdr(pScn, &shdr) && 0 != (SHF_EXECINSTR & shdr.sh_flags))
        {
            ret = true;
        }
    }

    return ret;
}

unsigned ElfFile::LookupSectionIndex(const char* pName) const
{
    GElf_Shdr shdr;
    Elf_Scn* pScn = LookupSectionByName(m_pElf, pName, shdr);

    unsigned index;

    if (NULL != pScn)
    {
        index = static_cast<unsigned>(elf_ndxscn(pScn));
    }
    else
    {
        index = GetSectionsCount();
    }

    return index;
}

unsigned ElfFile::LookupSectionIndex(gtRVAddr rva) const
{
    Elf64_Addr addr = static_cast<Elf64_Addr>(rva) + m_imageBase;
    GElf_Shdr shdr;
    Elf_Scn* pScn = LookupSectionByAddr(m_pElf, addr, shdr);

    unsigned index;

    if (NULL != pScn)
    {
        index = static_cast<unsigned>(elf_ndxscn(pScn));
    }
    else
    {
        index = GetSectionsCount();
    }

    return index;
}

const gtUByte* ElfFile::GetSectionBytes(unsigned index) const
{
    const gtUByte* pBytes = NULL;

    Elf_Scn* pScn = elf_getscn(m_pElf, index);

    if (NULL != pScn)
    {
        Elf_Data* pData = elf_getdata(pScn, NULL);

        if (NULL != pData)
        {
            pBytes = static_cast<gtUByte*>(pData->d_buf);
        }
    }

    return pBytes;
}

bool ElfFile::GetSectionRvaLimits(unsigned index, gtRVAddr& startRva, gtRVAddr& endRva) const
{
    bool ret = false;

    Elf_Scn* pScn = elf_getscn(m_pElf, index);

    if (NULL != pScn)
    {
        GElf_Shdr shdr;

        if (NULL != gelf_getshdr(pScn, &shdr))
        {
            Elf64_Addr startAddr = shdr.sh_addr - m_imageBase;
            startRva = static_cast<gtRVAddr>(startAddr);
            endRva   = static_cast<gtRVAddr>(startAddr + shdr.sh_size);

            ret = true;
        }
    }

    return ret;
}

gtUInt32 ElfFile::GetSectionSize(unsigned index) const
{
    gtUInt32 size = 0;

    Elf_Scn* pScn = elf_getscn(m_pElf, index);

    if (NULL != pScn)
    {
        Elf_Data* pData = elf_getdata(pScn, NULL);

        if (NULL != pData)
        {
            size = static_cast<gtUInt32>(pData->d_size);
        }
    }

    return size;
}

const char* ElfFile::GetSectionName(unsigned index, unsigned* pLength) const
{
    char* pName = NULL;

    size_t shstrndx;

    if (0 == elf_getshdrstrndx(m_pElf, &shstrndx))
    {
        Elf_Scn* pScn = elf_getscn(m_pElf, index);

        if (NULL != pScn)
        {
            GElf_Shdr shdr;

            if (NULL != gelf_getshdr(pScn, &shdr))
            {
                pName = elf_strptr(m_pElf, shstrndx, shdr.sh_name);

                if (NULL != pLength && NULL != pName)
                {
                    *pLength = static_cast<unsigned>(strlen(pName));//TODO buggi in x64 must use pLength as size_t
                }
            }
        }
    }

    return pName;
}

gtRVAddr ElfFile::GetEntryPoint() const
{
    GElf_Ehdr ehdr;
    return (NULL != gelf_getehdr(m_pElf, &ehdr)) ? static_cast<gtRVAddr>(ehdr.e_entry - m_imageBase) : GT_INVALID_RVADDR;
}

const gtUByte* ElfFile::GetMemoryBlock(gtRVAddr rva, gtUInt32& size) const
{
    const gtUByte* pBytes = NULL;

    Elf64_Addr addr = static_cast<Elf64_Addr>(rva) + m_imageBase;
    GElf_Shdr shdr;
    Elf_Scn* pScn = LookupSectionByAddr(m_pElf, addr, shdr);

    if (NULL != pScn)
    {
        Elf_Data* pData = elf_getdata(pScn, NULL);

        if (NULL != pData)
        {
            Elf64_Addr sectionOffset = addr - shdr.sh_addr;
            gtUInt32 remainingSize = static_cast<gtUInt32>(pData->d_size - sectionOffset);

            if (remainingSize < size)
            {
                size = remainingSize;
            }

            pBytes = static_cast<gtUByte*>(pData->d_buf) + sectionOffset;
        }
    }

    return pBytes;
}

bool ElfFile::InitializeSymbolEngine(const wchar_t* pSearchPath,
                                     const wchar_t* pServerList,
                                     const wchar_t* pCachePath)
{
    (void)pServerList; // Unused
    (void)pCachePath;  // Unused

    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Executable (ELF) %ls started Symbol Engine initialization", m_modulePath);

    if (NULL != m_pSymbolEngine)
    {
        delete m_pSymbolEngine;
        m_pSymbolEngine = NULL;
    }

    // Set whether the debug info pertaining to inlined symbols to be processed or not
    // This will be consumed by symbol engines
    SetProcessInlineInfo(IsSystemExecutable() ? false : true);

    ElfSymbolEngine* pElfEngine = new ElfSymbolEngine();

    if (pElfEngine->Initialize(*this))
    {
        m_pSymbolEngine = pElfEngine;

        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Executable (ELF) %ls initialized Symbol Engine: ELF", m_modulePath);
    }
    else
    {
        delete pElfEngine;
        pElfEngine = NULL;

        OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Executable (ELF) %ls failed to initialize Symbol Engine: ELF", m_modulePath);
    }

    unsigned sectionsCount = GetSectionsCount();

    GElf_Shdr shdr;

    if (NULL != LookupSectionByName(m_pElf, ".debug_abbrev", shdr) && NULL != LookupSectionByName(m_pElf, ".debug_info", shdr))
    {
        DwarfSymbolEngine* pDwarfEngine = new DwarfSymbolEngine();

        if (pDwarfEngine->Initialize(*this, pElfEngine, m_pElf))
        {
            m_pSymbolEngine = pDwarfEngine;
            m_isDebugInfoAvailable = true;

            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Executable (ELF) %ls initialized Symbol Engine: DWARF", m_modulePath);
        }
        else
        {
            delete pDwarfEngine;

            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Executable (ELF) %ls failed to initialize Symbol Engine: DWAF", m_modulePath);
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

                if (pStabsEngine->Initialize(*this, stabIndex, stabstrIndex, pElfEngine))
                {
                    m_pSymbolEngine = pStabsEngine;
                    m_isDebugInfoAvailable = true;

                    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Executable (ELF) %ls initialized Symbol Engine: STABS", m_modulePath);
                }
                else
                {
                    delete pStabsEngine;

                    OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Executable (ELF) %ls failed to initialize Symbol Engine: STABS", m_modulePath);
                }
            }
        }
    }

    unsigned debuglinkIndex = LookupSectionIndex(".gnu_debuglink");

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

                    // Some unfortunate .gnu_debuglink points to same executable file instead of pointing
                    // to an actual debug file, which would lead to file loading again and again.
                    // Stop such scenarios by comparing the file names.

                    // Find the filename in modulepath
                    int i = wcslen(m_modulePath) - 1;

                    while (i >= 0 && m_modulePath[i] != osFilePath::osPathSeparator)
                    {
                        --i;
                    }

                    wchar_t* moduleFileName = &m_modulePath[i + 1];

                    if (0 != wcsncmp(fileName, moduleFileName, OS_MAX_PATH))
                    {
                        wchar_t imagePath[OS_MAX_PATH];

                        if (0U != SymbolEngine::FindFile(pSearchPath, fileName, imagePath))
                        {
                            ProxySymbolEngine<ElfFile>* pProxyEngine = new ProxySymbolEngine<ElfFile>(imagePath);

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

    return NULL != m_pSymbolEngine;
}


static Elf64_Addr ExtractImageBase(Elf* pElf)
{
    Elf64_Addr imageBase = 0;

    size_t n = 0;

    if (0 == elf_getphdrnum(pElf, &n) && 0 != n)
    {
        // we need the first Program header
        GElf_Phdr elfphdr;

        if (NULL != gelf_getphdr(pElf, 0, &elfphdr))
        {
            if (PT_PHDR == elfphdr.p_type || PT_LOAD == elfphdr.p_type)
            {
                imageBase = elfphdr.p_vaddr - elfphdr.p_offset;
            }
        }
    }

    return imageBase;
}

static Elf_Scn* LookupSectionByName(Elf* pElf, const char* pName, GElf_Shdr& shdr)
{
    Elf_Scn* pScn = NULL;

    size_t shstrndx;

    if (0 == elf_getshdrstrndx(pElf, &shstrndx))
    {
        while (NULL != (pScn = elf_nextscn(pElf, pScn)))
        {
            if (NULL != gelf_getshdr(pScn, &shdr))
            {
                char* pCandidateName = elf_strptr(pElf, shstrndx, shdr.sh_name);

                if (NULL != pCandidateName)
                {
                    if (0 == strcmp(pName, pCandidateName))
                    {
                        break;
                    }
                }
            }
        }
    }

    return pScn;
}

static Elf_Scn* LookupSectionByAddr(Elf* pElf, Elf64_Addr addr, GElf_Shdr& shdr)
{
    Elf_Scn* pScn = NULL;

    while (NULL != (pScn = elf_nextscn(pElf, pScn)))
    {
        if (NULL != gelf_getshdr(pScn, &shdr))
        {
            if (shdr.sh_addr <= addr && addr < (shdr.sh_addr + shdr.sh_size))
            {
                break;
            }
        }
    }

    return pScn;
}
