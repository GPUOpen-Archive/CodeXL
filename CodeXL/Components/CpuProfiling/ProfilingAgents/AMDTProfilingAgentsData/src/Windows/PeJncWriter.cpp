//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file PeJncWriter.cpp
/// \brief This file contains an interface to write JIT Native Code file in PE format.
///
//==================================================================================

#include "PeJncWriter.h"
#include <ctime>

PeJncWriter::PeJncWriter()
{
    m_jncFileName[0] = L'\0';
    m_jncClassName[0] = L'\0';
    m_jncFuncName[0] = L'\0';
    m_javaSrcName[0] = L'\0';
    m_clrModuleName[0] = L'\0';

    m_jitLoadAddr = 0;
    m_ilNativeMapEntryCount = 0;
    m_ilSize = 0;
    m_javaSrcEntryCnt = 0;
    m_codeSize = 0;
    m_dataSize = 0;

    m_sizeOfAlignedCodeSect = 0;
    m_sizeOfAlignedJavaSrcSect = 0;
    m_sizeOfAlignedILSect = 0;
    m_sizeOfAlignedOffSetMap = 0;
    m_methodOffsetOfImage = 0;
    m_majorVersion = MAJORIMAGEVERSION;
}

PeJncWriter::~PeJncWriter()
{
    Close();
}

void PeJncWriter::Close()
{
    if (m_fileStream.isOpened())
    {
        m_fileStream.close();
    }
}

void PeJncWriter::SetJNCFileName(const wchar_t* pFileName)
{
    if (NULL != pFileName)
    {
        wcsncpy_s(m_jncFileName, OS_MAX_FNAME, pFileName, _TRUNCATE);
    }
}

void PeJncWriter::SetJITFuncName(const wchar_t* pClassName, const wchar_t* pFuncName, const wchar_t* pJavaSourceFile)
{
    if (NULL != pClassName)
    {
        wcsncpy_s(m_jncClassName, OS_MAX_FNAME, pClassName, _TRUNCATE);
    }

    if (NULL != pFuncName)
    {
        wcsncpy_s(m_jncFuncName, OS_MAX_FNAME, pFuncName, _TRUNCATE);
    }

    if (NULL != pJavaSourceFile)
    {
        wcsncpy_s(m_javaSrcName, OS_MAX_FNAME, pJavaSourceFile, _TRUNCATE);
    }
}

void PeJncWriter::SetJITStartAddr(gtUInt64 addr)
{
    m_jitLoadAddr = addr;
}

void PeJncWriter::SetJITModuleName(wchar_t* pModName)
{
    if (NULL != pModName)
    {
        wcsncpy_s(m_clrModuleName, OS_MAX_FNAME, pModName, _TRUNCATE);
    }
}



void PeJncWriter::WriteMSDOSHeader()
{
    //  typedef struct _IMAGE_DOS_HEADER {      // DOS .EXE header
    //      WORD   e_magic;                     // Magic number
    //      WORD   e_cblp;                      // Bytes on last page of file
    //      WORD   e_cp;                        // Pages in file
    //      WORD   e_crlc;                      // Relocations
    //      WORD   e_cparhdr;                   // Size of header in paragraphs
    //      WORD   e_minalloc;                  // Minimum extra paragraphs needed
    //      WORD   e_maxalloc;                  // Maximum extra paragraphs needed
    //      WORD   e_ss;                        // Initial (relative) SS value
    //      WORD   e_sp;                        // Initial SP value
    //      WORD   e_csum;                      // Checksum
    //      WORD   e_ip;                        // Initial IP value
    //      WORD   e_cs;                        // Initial (relative) CS value
    //      WORD   e_lfarlc;                    // File address of relocation table
    //      WORD   e_ovno;                      // Overlay number
    //      WORD   e_res[4];                    // Reserved words
    //      WORD   e_oemid;                     // OEM identifier (for e_oeminfo)
    //      WORD   e_oeminfo;                   // OEM information; e_oemid specific
    //      WORD   e_res2[10];                  // Reserved words
    //      LONG   e_lfanew;                    // File address of new exe header
    //    } IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;

    const DWORD dosHeader [] =
    {
        0x00905a4d, 0x00000003, 0x00000004, 0x0000ffff, //  :  0 MZ~~~~~~~~~~~~~~
        0x000000b8, 0x00000000, 0x00000040, 0x00000000, //  : 10 ~~~~~~~~@~~~~~~~
        0x00000000, 0x00000000, 0x00000000, 0x00000000, //  : 20 ~~~~~~~~~~~~~~~~
        0x00000000, 0x00000000, 0x00000000, 0x000000c0, //  : 30 ~~~~~~~~~~~~~~~~
        // Note: here is end of IMAGE_DOS_HEADER, last DWORD (at 0x3c) 0x000000c0
        // is offset of NT header.

        0x0eba1f0e, 0xcd09b400, 0x4c01b821, 0x685421cd, //  : 40 ~~~~~~~~!~~L~!Th
        0x70207369, 0x72676f72, 0x63206d61, 0x6f6e6e61, //  : 50 is program canno
        0x65622074, 0x6e757220, 0x206e6920, 0x20534f44, //  : 60 t be run in DOS
        0x65646f6d, 0x0a0d0d2e, 0x00000024, 0x00000000, //  : 70 mode.~~~$~~~~~~~
        0xd87f9bf1, 0x8b11fab5, 0x8b11fab5, 0x8b11fab5, //  : 80 ~~~~~~~~~~~~~~~~
        0x8b7f8cdb, 0x8b11fab4, 0x8b6b8cdb, 0x8b11fab4, //  : 90 ~~~~~~~~~~k~~~~~
        0x8b698cdb, 0x8b11fab4, 0x68636952, 0x8b11fab5, //  : A0 ~~i~~~~~Rich~~~~
        0x00000000, 0x00000000, 0x00000000, 0x00000000  //  : B0 ~~~~~~~~~~~~~~~~
    };
    // Note: we may need to modify e_lfanew later.

    IMAGE_DOS_HEADER* pDos_Header = (IMAGE_DOS_HEADER*) dosHeader;
    DWORD totalSize = SIZEOFALLHEADERS + m_sizeOfAlignedCodeSect +
                      m_sizeOfAlignedILSect + m_sizeOfAlignedJavaSrcSect + m_sizeOfAlignedOffSetMap;

    pDos_Header->e_cblp = 0;
    pDos_Header->e_cp = (WORD)(totalSize / FILEPAGESIZE);
    pDos_Header->e_crlc = 0;
    pDos_Header->e_cparhdr = 0;

    // m_pFileStream already initialized
    m_fileStream.write(dosHeader);
}

void PeJncWriter::WriteNTHeader64()
{
    // sizeof(IMAGE_NT_HEADER64) = 264
    DWORD ntHdrContent[] =
    {
        0x00004550, 0x00038664, 0x3f251feb, 0x00000000, //  : C0 PE~~d~~~~~%?~~~~
        0x00000000, 0x202e00f0, 0x0008020b, 0x00001000, //  : D0 ~~~~~~. ~~~~~~~~
        0x00000200, 0x00000000, 0x00003000, 0x00003BAD, //  : E0 ~~~~~~~~~~~~~~~~
        0x01230000, 0x00000000, 0x00001000, 0x00000200, //  : F0 ~~#~~~~~~~~~~~~~
        0x00000004, 0x00000000, 0x00020005, 0x00000000, //  :100 ~~~~~~~~~~~~~~~~
        0x00004000, 0x00000400, 0x00000000, 0x00000002, //  :110 ~@~~~~~~~~~~~~~~
        0x00100000, 0x00000000, 0x00001000, 0x00000000, //  :120 ~~~~~~~~~~~~~~~~
        0x00100000, 0x00000000, 0x00001000, 0x00000000, //  :130 ~~~~~~~~~~~~~~~~
        0x00000000, 0x00000010, 0x00002000, 0x00000042, //  :140 ~~~~~~~~ #~~B~~~
        0x00000000, 0x00000000, 0x00000000, 0x00000000, //  :150 ~~~~~~~~~~~~~~~~
        0x00000000, 0x00000000, 0x00000000, 0x00000000, //  :160 ~~~~~~~~~~~~~~~~
        0x00000000, 0x00000000, 0x00000000, 0x00000000, //  :170 ~0~~~~~~~~~~~~~~
        0x00000000, 0x00000000, 0x00000000, 0x00000000, //  :180 ~~~~~~~~~~~~~~~~
        0x00000000, 0x00000000, 0x00000000, 0x00000000, //  :190 ~~~~~~~~~~~~~~~~
        0x00000000, 0x00000000, 0x00000000, 0x00000000, //  :1A0 ~~~~~~~~~~~~~~~~
        0x00000000, 0x00000000, 0x00000000, 0x00000000, //  :1B0 ~~~~~~~~~~~~~~~~
        0x00000000, 0x00000000                          //  :1C0 ~~~~~~~~
    };

    //  typedef struct _IMAGE_FILE_HEADER {
    //      WORD    Machine;
    //      WORD    NumberOfSections;
    //      DWORD   TimeDateStamp;
    //      DWORD   PointerToSymbolTable;
    //      DWORD   NumberOfSymbols;
    //      WORD    SizeOfOptionalHeader;
    //      WORD    Characteristics;
    //  } IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

    //  typedef struct _IMAGE_OPTIONAL_HEADER64 {
    //      WORD        Magic;
    //      BYTE        MajorLinkerVersion;
    //      BYTE        MinorLinkerVersion;
    //      DWORD       SizeOfCode;
    //      DWORD       SizeOfInitializedData;
    //      DWORD       SizeOfUninitializedData;
    //      DWORD       AddressOfEntryPoint;
    //      DWORD       BaseOfCode;
    //      ULONGLONG   ImageBase;
    //      DWORD       SectionAlignment;
    //      DWORD       FileAlignment;
    //      WORD        MajorOperatingSystemVersion;
    //      WORD        MinorOperatingSystemVersion;
    //      WORD        MajorImageVersion;
    //      WORD        MinorImageVersion;
    //      WORD        MajorSubsystemVersion;
    //      WORD        MinorSubsystemVersion;
    //      DWORD       Win32VersionValue;
    //      DWORD       SizeOfImage;
    //      DWORD       SizeOfHeaders;
    //      DWORD       CheckSum;
    //      WORD        Subsystem;
    //      WORD        DllCharacteristics;
    //      ULONGLONG   SizeOfStackReserve;
    //      ULONGLONG   SizeOfStackCommit;
    //      ULONGLONG   SizeOfHeapReserve;
    //      ULONGLONG   SizeOfHeapCommit;
    //      DWORD       LoaderFlags;
    //      DWORD       NumberOfRvaAndSizes;
    //      IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
    //  } IMAGE_OPTIONAL_HEADER64, *PIMAGE_OPTIONAL_HEADER64;

    //  typedef struct _IMAGE_NT_HEADERS64 {
    //      DWORD Signature;
    //      IMAGE_FILE_HEADER FileHeader;
    //      IMAGE_OPTIONAL_HEADER64 OptionalHeader;
    //  } IMAGE_NT_HEADERS64, *PIMAGE_NT_HEADERS64;


    IMAGE_NT_HEADERS64* pNTHeader = (IMAGE_NT_HEADERS64*)(void*) & ntHdrContent;

    // Signature = PE\0\0
    pNTHeader->Signature = IMAGE_NT_SIGNATURE;

    time_t now;
    time(&now);

    // IMAGE_FILE_HEADER
    pNTHeader->FileHeader.Machine = IMAGE_FILE_MACHINE_AMD64;

    WORD numSections = 1;   // at least there is a .text section
    numSections += static_cast<WORD>(0 != m_javaSrcEntryCnt);           // .JVASRC section
    numSections += static_cast<WORD>(0 != m_ilSize);                // .CAIL section
    numSections += static_cast<WORD>(0 != m_ilNativeMapEntryCount); // .ILMAP
    pNTHeader->FileHeader.NumberOfSections = numSections;

    pNTHeader->FileHeader.TimeDateStamp = static_cast<DWORD>(now);
    pNTHeader->FileHeader.PointerToSymbolTable = 0x0;
    pNTHeader->FileHeader.NumberOfSymbols = 0;
    pNTHeader->FileHeader.SizeOfOptionalHeader = sizeof(IMAGE_OPTIONAL_HEADER64);
    pNTHeader->FileHeader.Characteristics = IMAGE_FILE_DLL | IMAGE_FILE_DEBUG_STRIPPED |
                                            IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP | IMAGE_FILE_NET_RUN_FROM_SWAP |
                                            IMAGE_FILE_LARGE_ADDRESS_AWARE;

    // IMAGE_OPTIONAL_HEADER64;
    pNTHeader->OptionalHeader.Magic = IMAGE_NT_OPTIONAL_HDR64_MAGIC;
    pNTHeader->OptionalHeader.MajorLinkerVersion = 0x08;
    pNTHeader->OptionalHeader.MinorLinkerVersion = 0x00;
    pNTHeader->OptionalHeader.SizeOfCode = m_sizeOfAlignedCodeSect;
    pNTHeader->OptionalHeader.SizeOfInitializedData = m_sizeOfAlignedILSect +
                                                      m_sizeOfAlignedJavaSrcSect + m_sizeOfAlignedOffSetMap;
    pNTHeader->OptionalHeader.SizeOfUninitializedData = 0x00;
    pNTHeader->OptionalHeader.AddressOfEntryPoint = 0x00;
    pNTHeader->OptionalHeader.BaseOfCode = SIZEOFALLHEADERS;
    pNTHeader->OptionalHeader.ImageBase = 0x10000000;
    pNTHeader->OptionalHeader.SectionAlignment = 0x00000100;
    pNTHeader->OptionalHeader.FileAlignment = 0x00000100;
    pNTHeader->OptionalHeader.MajorOperatingSystemVersion = 0x04;
    pNTHeader->OptionalHeader.MinorOperatingSystemVersion = 0x00;
    pNTHeader->OptionalHeader.MajorImageVersion = m_majorVersion;
    pNTHeader->OptionalHeader.MinorImageVersion = 0x0;
    pNTHeader->OptionalHeader.MajorSubsystemVersion = 0x05;
    pNTHeader->OptionalHeader.MinorSubsystemVersion = 0x02;
    pNTHeader->OptionalHeader.Win32VersionValue = 0x00;
    pNTHeader->OptionalHeader.SizeOfImage = SIZEOFALLHEADERS +
                                            m_sizeOfAlignedCodeSect + m_sizeOfAlignedILSect +
                                            m_sizeOfAlignedJavaSrcSect + m_sizeOfAlignedOffSetMap;

    pNTHeader->OptionalHeader.SizeOfHeaders = SIZEOFALLHEADERS;
    pNTHeader->OptionalHeader.CheckSum = 0;
    pNTHeader->OptionalHeader.Subsystem = IMAGE_SUBSYSTEM_NATIVE;
    pNTHeader->OptionalHeader.DllCharacteristics = 0;
    pNTHeader->OptionalHeader.SizeOfStackReserve = 0x100000;
    pNTHeader->OptionalHeader.SizeOfStackCommit = 0x1000;
    pNTHeader->OptionalHeader.SizeOfHeapReserve = 0x100000;
    pNTHeader->OptionalHeader.SizeOfHeapCommit = 0x1000;
    pNTHeader->OptionalHeader.LoaderFlags = 0;
    pNTHeader->OptionalHeader.NumberOfRvaAndSizes = 0x10;

    for (unsigned int i = 0; i < IMAGE_NUMBEROF_DIRECTORY_ENTRIES; i++)
    {
        pNTHeader->OptionalHeader.DataDirectory[i].VirtualAddress = 0x00;
        pNTHeader->OptionalHeader.DataDirectory[i].Size = 0x00;
    }

    m_fileStream.write(*pNTHeader);
}


void PeJncWriter::WriteNTHeader32()
{
    //  typedef struct _IMAGE_FILE_HEADER {
    //      WORD    Machine;
    //      WORD    NumberOfSections;
    //      DWORD   TimeDateStamp;
    //      DWORD   PointerToSymbolTable;
    //      DWORD   NumberOfSymbols;
    //      WORD    SizeOfOptionalHeader;
    //      WORD    Characteristics;
    //  } IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;


    //  typedef struct _IMAGE_OPTIONAL_HEADER {
    //      //
    //      // Standard fields.
    //      //
    //
    //      WORD    Magic;
    //      BYTE    MajorLinkerVersion;
    //      BYTE    MinorLinkerVersion;
    //      DWORD   SizeOfCode;
    //      DWORD   SizeOfInitializedData;
    //      DWORD   SizeOfUninitializedData;
    //      DWORD   AddressOfEntryPoint;
    //      DWORD   BaseOfCode;
    //      DWORD   BaseOfData;
    //
    //      //
    //      // NT additional fields.
    //      //
    //
    //      DWORD   ImageBase;
    //      DWORD   SectionAlignment;
    //      DWORD   FileAlignment;
    //      WORD    MajorOperatingSystemVersion;
    //      WORD    MinorOperatingSystemVersion;
    //      WORD    MajorImageVersion;
    //      WORD    MinorImageVersion;
    //      WORD    MajorSubsystemVersion;
    //      WORD    MinorSubsystemVersion;
    //      DWORD   Win32VersionValue;
    //      DWORD   SizeOfImage;
    //      DWORD   SizeOfHeaders;
    //      DWORD   CheckSum;
    //      WORD    Subsystem;
    //      WORD    DllCharacteristics;
    //      DWORD   SizeOfStackReserve;
    //      DWORD   SizeOfStackCommit;
    //      DWORD   SizeOfHeapReserve;
    //      DWORD   SizeOfHeapCommit;
    //      DWORD   LoaderFlags;
    //      DWORD   NumberOfRvaAndSizes;
    //      IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
    //  } IMAGE_OPTIONAL_HEADER32, *PIMAGE_OPTIONAL_HEADER32;

    //  typedef struct _IMAGE_NT_HEADERS {
    //      DWORD Signature;
    //      IMAGE_FILE_HEADER FileHeader;
    //      IMAGE_OPTIONAL_HEADER32 OptionalHeader;
    //  } IMAGE_NT_HEADERS32, *PIMAGE_NT_HEADERS32;

    //IMAGE_NT_HEADERS32 * pNTHeader = (IMAGE_NT_HEADERS32 *)(void *) & ntHdrContent;
    IMAGE_NT_HEADERS32 NTHeader;

    // Signature = PE\0\0
    NTHeader.Signature = IMAGE_NT_SIGNATURE;

    time_t now;
    time(&now);

    // IMAGE_FILE_HEADER
    NTHeader.FileHeader.Machine = IMAGE_FILE_MACHINE_I386;

    WORD numSections = 1;   // at least there is a .text section
    numSections += static_cast<WORD>(0 != m_javaSrcEntryCnt);           // .JVASRC section
    numSections += static_cast<WORD>(0 != m_ilSize);                // .CAIL section
    numSections += static_cast<WORD>(0 != m_ilNativeMapEntryCount); // .ILMAP
    NTHeader.FileHeader.NumberOfSections = numSections;

    NTHeader.FileHeader.TimeDateStamp = static_cast<DWORD>(now);
    NTHeader.FileHeader.PointerToSymbolTable = 0x0;
    NTHeader.FileHeader.NumberOfSymbols = 0;
    NTHeader.FileHeader.SizeOfOptionalHeader = sizeof(IMAGE_OPTIONAL_HEADER32);
    NTHeader.FileHeader.Characteristics = IMAGE_FILE_DLL | IMAGE_FILE_DEBUG_STRIPPED |
                                          IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP | IMAGE_FILE_NET_RUN_FROM_SWAP |
                                          IMAGE_FILE_LARGE_ADDRESS_AWARE;

    // IMAGE_OPTIONAL_HEADER32;
    NTHeader.OptionalHeader.Magic = IMAGE_NT_OPTIONAL_HDR32_MAGIC;
    NTHeader.OptionalHeader.MajorLinkerVersion = 0x06;
    NTHeader.OptionalHeader.MinorLinkerVersion = 0x00;
    NTHeader.OptionalHeader.SizeOfCode = m_sizeOfAlignedCodeSect;
    NTHeader.OptionalHeader.SizeOfInitializedData = m_sizeOfAlignedILSect + m_sizeOfAlignedJavaSrcSect + m_sizeOfAlignedOffSetMap;
    NTHeader.OptionalHeader.SizeOfUninitializedData = 0x00;
    NTHeader.OptionalHeader.AddressOfEntryPoint = 0x00;
    NTHeader.OptionalHeader.BaseOfCode = 0x1000;
    NTHeader.OptionalHeader.ImageBase = 0x10000000;
    NTHeader.OptionalHeader.SectionAlignment = ADDRESSALIGNMENT;
    NTHeader.OptionalHeader.FileAlignment = ADDRESSALIGNMENT;
    NTHeader.OptionalHeader.MajorOperatingSystemVersion = 0x04;
    NTHeader.OptionalHeader.MinorOperatingSystemVersion = 0x00;
    NTHeader.OptionalHeader.MajorImageVersion = m_majorVersion;
    NTHeader.OptionalHeader.MinorImageVersion = 0x0;
    NTHeader.OptionalHeader.MajorSubsystemVersion = 0x05;
    NTHeader.OptionalHeader.MinorSubsystemVersion = 0x02;
    NTHeader.OptionalHeader.Win32VersionValue = 0x00;
    NTHeader.OptionalHeader.SizeOfImage = SIZEOFALLHEADERS + m_sizeOfAlignedCodeSect +
                                          m_sizeOfAlignedILSect + m_sizeOfAlignedJavaSrcSect + m_sizeOfAlignedOffSetMap;
    NTHeader.OptionalHeader.SizeOfHeaders = SIZEOFALLHEADERS;
    NTHeader.OptionalHeader.CheckSum = 0;
    NTHeader.OptionalHeader.Subsystem = 2;
    NTHeader.OptionalHeader.DllCharacteristics = 0;
    NTHeader.OptionalHeader.SizeOfStackReserve = 0x100000;
    NTHeader.OptionalHeader.SizeOfStackCommit = 0x1000;
    NTHeader.OptionalHeader.SizeOfHeapReserve = 0x100000;
    NTHeader.OptionalHeader.SizeOfHeapCommit = 0x1000;
    NTHeader.OptionalHeader.LoaderFlags = 0;
    NTHeader.OptionalHeader.NumberOfRvaAndSizes = 0x10;

    for (unsigned int i = 0; i < IMAGE_NUMBEROF_DIRECTORY_ENTRIES; i++)
    {
        NTHeader.OptionalHeader.DataDirectory[i].VirtualAddress = 0x00;
        NTHeader.OptionalHeader.DataDirectory[i].Size = 0x00;
    }

    m_fileStream.write(NTHeader);
}


void PeJncWriter::WriteSectionHeaders()
{
    IMAGE_SECTION_HEADER textSectHeader, dataSectHeader;
    textSectHeader.Name[0] = 0x2e;  // .
    textSectHeader.Name[1] = 0x74;  // t
    textSectHeader.Name[2] = 0x65;  // e
    textSectHeader.Name[3] = 0x78;  // x
    textSectHeader.Name[4] = 0x74;  // t
    textSectHeader.Name[5] = 0x00;  //
    textSectHeader.Name[6] = 0x00;  //
    textSectHeader.Name[7] = 0x00;  //
    textSectHeader.Misc.VirtualSize = m_codeSize;
    textSectHeader.VirtualAddress = SIZEOFALLHEADERS;
    textSectHeader.SizeOfRawData = m_sizeOfAlignedCodeSect;
    textSectHeader.PointerToRawData = SIZEOFALLHEADERS;     // .text is the first section
    textSectHeader.PointerToRelocations = 0x00;
    textSectHeader.PointerToLinenumbers = 0x00;
    textSectHeader.NumberOfRelocations = 0x00;
    textSectHeader.NumberOfLinenumbers = 0x00;
    textSectHeader.Characteristics = IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ;

    m_fileStream.write(textSectHeader);

    DWORD virtualAddr = textSectHeader.VirtualAddress + textSectHeader.SizeOfRawData;

    if (0 != m_javaSrcEntryCnt)
    {
        // should be .JAVASRC
        memcpy(dataSectHeader.Name, ".JVASRC\0", IMAGE_SIZEOF_SHORT_NAME);

        if (m_majorVersion <= MAJORIMAGEVERSION)
        {
            // 4 DWORD to save 3 string length, and 1 entry count;
            // format:
            // 1 DWORD for length of java source name, following by source file name string
            // 1 DWORD for length of java class name, following by class name string
            // 1 DWORD for length of java function name, following by function name string
            // 1 QWORD for address. NOTE: even for 32-bit
            // 1 DWROD for entry count, then following by srouce line Info elements.
            // 1 BYTE for terminate char.
            dataSectHeader.Misc.VirtualSize = static_cast<gtUInt32>(wcslen(m_javaSrcName) * sizeof(wchar_t) +
                                                                    wcslen(m_jncClassName) * sizeof(wchar_t) +
                                                                    wcslen(m_jncFuncName) * sizeof(wchar_t) +
                                                                    4 * sizeof(DWORD) + sizeof(gtUInt64) +
                                                                    m_javaSrcEntryCnt * sizeof(JavaSrcLineInfo) + 1);
        }
        else
        {
            // version is 4 or higher
            dataSectHeader.Misc.VirtualSize = m_dataSize;
        }

        // since .data section follows .text section right away.
        dataSectHeader.VirtualAddress =  virtualAddr;
        dataSectHeader.SizeOfRawData = m_sizeOfAlignedJavaSrcSect;

        dataSectHeader.PointerToRawData = dataSectHeader.VirtualAddress;
        dataSectHeader.PointerToRelocations = 0x00;
        dataSectHeader.PointerToLinenumbers = 0x00;
        dataSectHeader.NumberOfRelocations = 0x00;
        dataSectHeader.NumberOfLinenumbers = 0x00;
        dataSectHeader.Characteristics = IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ;

        m_fileStream.write(dataSectHeader);

        // calculate virtual address for next section
        virtualAddr += dataSectHeader.SizeOfRawData;
    }

    if (0 != m_ilSize)
    {
        // should be .IL section
        memcpy(dataSectHeader.Name, ".CAIL\0\0\0", IMAGE_SIZEOF_SHORT_NAME);

        // 4 DWORD to save 3 string length, and 1 entry count;
        // format:
        // 1 DWORD for length of module name, following by module name string.
        // 1 DWORD for length of class name, following by class name string.
        // 1 DWORD for length of function name, following by function name string.
        //      Note: All names do not contain terminate character.
        // 1 QWORD for address. NOTE: even for 32-bit
        // 1 DWROD for entry count, then following by IL byte code
        dataSectHeader.Misc.VirtualSize = static_cast<DWORD>(wcslen(m_clrModuleName) * sizeof(wchar_t) +
                                                             wcslen(m_jncClassName) * sizeof(wchar_t) +
                                                             wcslen(m_jncFuncName) * sizeof(wchar_t) +
                                                             4 * sizeof(DWORD) + sizeof(gtUInt64) + m_ilSize);

        // since .data section follows .text section right away.
        dataSectHeader.VirtualAddress =  virtualAddr;
        dataSectHeader.SizeOfRawData = m_sizeOfAlignedILSect;

        dataSectHeader.PointerToRawData = dataSectHeader.VirtualAddress;
        dataSectHeader.PointerToRelocations = 0x00;
        dataSectHeader.PointerToLinenumbers = 0x00;
        dataSectHeader.NumberOfRelocations = 0x00;
        dataSectHeader.NumberOfLinenumbers = 0x00;
        dataSectHeader.Characteristics = IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ;

        m_fileStream.write(dataSectHeader);

        // calculate virtual address for next section
        virtualAddr += dataSectHeader.SizeOfRawData;
    }

    if (0 != m_ilNativeMapEntryCount)
    {
        memcpy(dataSectHeader.Name, ".ILMAP\0\0", IMAGE_SIZEOF_SHORT_NAME);

        // 4 DWORD to save 3 string length, and 1 entry count;
        // format:
        // 1 DWORD for length of module name, following by module name string.
        // 1 DWORD for length of class name, following by class name string.
        // 1 DWORD for length of function name, following by function name string.
        //      Note: All names do not contain terminate character.
        // 1 QWORD for address. NOTE: even for 32-bit
        // 1 DWROD for IL to Native code offset Mapping count,
        // then following by m_ILNativeMapEntryCount * COR_DEBUG_IL_TO_NATIVE_MAP
        //      Note: COR_DEBUG_IL_TO_NATIVE_MAP contains 3 gtUInt32. (see header for definition)
        dataSectHeader.Misc.VirtualSize = static_cast<DWORD>(wcslen(m_clrModuleName) * sizeof(wchar_t) +
                                                             wcslen(m_jncClassName) * sizeof(wchar_t) +
                                                             wcslen(m_jncFuncName) * sizeof(wchar_t) +
                                                             4 * sizeof(DWORD) + sizeof(gtUInt64) +
                                                             m_ilNativeMapEntryCount * sizeof(gtUInt32) * 3);

        // since .data section follows .text section right away.
        dataSectHeader.VirtualAddress =  virtualAddr;
        dataSectHeader.SizeOfRawData = m_sizeOfAlignedOffSetMap;

        dataSectHeader.PointerToRawData = dataSectHeader.VirtualAddress;
        dataSectHeader.PointerToRelocations = 0x00;
        dataSectHeader.PointerToLinenumbers = 0x00;
        dataSectHeader.NumberOfRelocations = 0x00;
        dataSectHeader.NumberOfLinenumbers = 0x00;
        dataSectHeader.Characteristics = IMAGE_SCN_CNT_INITIALIZED_DATA | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ;

        m_fileStream.write(dataSectHeader);
    }
}


void PeJncWriter::WriteHeaders()
{
    WriteMSDOSHeader();

#if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
    WriteNTHeader64();
#else
    WriteNTHeader32();
#endif

    WriteSectionHeaders();
    WriteAlignmentPadding();
}


void PeJncWriter::CalculateSectionSize(unsigned int additionalSize)
{
    unsigned int alignPages = m_codeSize / ADDRESSALIGNMENT;
    unsigned int padBytes = m_codeSize % ADDRESSALIGNMENT;
    m_sizeOfAlignedCodeSect = (padBytes) ? (alignPages + 1) * ADDRESSALIGNMENT : alignPages * ADDRESSALIGNMENT;

    if (m_majorVersion <= MAJORIMAGEVERSION)
    {
        if (0 != m_javaSrcEntryCnt)
        {
            // source file name, DWORD for source file string length,
            m_sizeOfAlignedJavaSrcSect = static_cast<unsigned int>(wcslen(m_javaSrcName)) * sizeof(wchar_t) + sizeof(DWORD);

            // class name, DWORD for length
            m_sizeOfAlignedJavaSrcSect += static_cast<unsigned int>(wcslen(m_jncClassName)) * sizeof(wchar_t) + sizeof(DWORD);

            // function name, DWORD for length
            m_sizeOfAlignedJavaSrcSect += static_cast<unsigned int>(wcslen(m_jncFuncName)) * sizeof(wchar_t) + sizeof(DWORD);

            // load address, gtUInt64 always
            m_sizeOfAlignedJavaSrcSect += sizeof(gtUInt64);

            // source entry count;
            m_sizeOfAlignedJavaSrcSect += sizeof(DWORD);

            // JavaSrcLineInfo entries
            m_sizeOfAlignedJavaSrcSect += sizeof(JavaSrcLineInfo) * m_javaSrcEntryCnt;

            // calculate padding
            alignPages = m_sizeOfAlignedJavaSrcSect / ADDRESSALIGNMENT;
            padBytes = m_sizeOfAlignedJavaSrcSect % ADDRESSALIGNMENT;
            m_sizeOfAlignedJavaSrcSect = (padBytes) ? (alignPages + 1) * ADDRESSALIGNMENT : alignPages * ADDRESSALIGNMENT;
        }
    }
    else
    {
        // With inline info; m_majorVersion is >= 4
        m_sizeOfAlignedJavaSrcSect = sizeof(m_jitLoadAddr); // jit load address
        m_sizeOfAlignedJavaSrcSect += sizeof(DWORD);//numnerOfSubSections variable
        m_sizeOfAlignedJavaSrcSect += sizeof(DWORD) * JAVASRC_SECTION_COUNT; // for holding size of each section
        m_sizeOfAlignedJavaSrcSect += sizeof(DWORD) * JAVASRC_SECTION_COUNT; // for holding type of each section
        m_sizeOfAlignedJavaSrcSect += additionalSize; // 3 SubSections body size

        m_dataSize = m_sizeOfAlignedJavaSrcSect;

        // calculate padding
        alignPages = m_sizeOfAlignedJavaSrcSect / ADDRESSALIGNMENT;
        padBytes = m_sizeOfAlignedJavaSrcSect % ADDRESSALIGNMENT;
        m_sizeOfAlignedJavaSrcSect = (padBytes) ? (alignPages + 1) * ADDRESSALIGNMENT : alignPages * ADDRESSALIGNMENT;
    }

    if (0 != m_ilSize)
    {
        // .CAIL section;
        // module length and name
        m_sizeOfAlignedILSect = sizeof(DWORD) + static_cast<unsigned int>(wcslen(m_clrModuleName)) * sizeof(wchar_t);

        // class length and name
        m_sizeOfAlignedILSect += sizeof(DWORD) + static_cast<unsigned int>(wcslen(m_jncClassName)) * sizeof(wchar_t);

        //function length and name
        m_sizeOfAlignedILSect += sizeof(DWORD) + static_cast<unsigned int>(wcslen(m_jncFuncName)) * sizeof(wchar_t);

        //jit load address, always gtUInt64
        m_sizeOfAlignedILSect += sizeof(gtUInt64);

        // Method IL offset relative to module imagebase;
        m_sizeOfAlignedILSect += sizeof(unsigned int);

        // IL size and IL bytes
        m_sizeOfAlignedILSect += sizeof(DWORD) + sizeof(unsigned char) * m_ilSize;

        // padding
        alignPages = m_sizeOfAlignedILSect / ADDRESSALIGNMENT;
        padBytes = m_sizeOfAlignedILSect % ADDRESSALIGNMENT;
        m_sizeOfAlignedILSect = (padBytes) ? (alignPages + 1) * ADDRESSALIGNMENT : alignPages * ADDRESSALIGNMENT;
    }

    if (0 != m_ilNativeMapEntryCount)
    {
        // write .ILMap section;

        // module length and name
        m_sizeOfAlignedOffSetMap = sizeof(DWORD) + static_cast<unsigned int>(wcslen(m_clrModuleName)) * sizeof(wchar_t);

        // class length and name
        m_sizeOfAlignedOffSetMap += sizeof(DWORD) + static_cast<unsigned int>(wcslen(m_jncClassName)) * sizeof(wchar_t);

        //function length and name
        m_sizeOfAlignedOffSetMap += sizeof(DWORD) + static_cast<unsigned int>(wcslen(m_jncFuncName)) * sizeof(wchar_t);

        //jit load address, always gtUInt64
        m_sizeOfAlignedOffSetMap += sizeof(gtUInt64);

        // Write counts for IL to Native Mapping
        m_sizeOfAlignedOffSetMap += sizeof(DWORD) + sizeof(gtUInt32) * 3 * m_ilNativeMapEntryCount;

        // write padding
        alignPages = m_sizeOfAlignedOffSetMap / ADDRESSALIGNMENT;
        padBytes = m_sizeOfAlignedOffSetMap % ADDRESSALIGNMENT;
        m_sizeOfAlignedOffSetMap = (padBytes) ? (alignPages + 1) * ADDRESSALIGNMENT : alignPages * ADDRESSALIGNMENT;
    }
}


void PeJncWriter::WriteTextSection(const gtUByte* pJITNativeCode)
{
    // Note: pJITNativeCode had been checked from caller side;

    // write .text section;
    m_fileStream.write(pJITNativeCode, m_codeSize);
    // write padding
    WriteAlignmentPadding();
}

bool PeJncWriter::WriteJITNativeCode(const gtUByte* pJITNativeCode, unsigned int size, JavaSrcLineInfo* pSrcInfo, unsigned int srcEntryCnt)
{
    bool ret = false;
    m_majorVersion = MAJORIMAGEVERSION;

    if (NULL != pJITNativeCode)
    {
        if (m_fileStream.open(m_jncFileName, FMODE_TEXT("w+b")))
        {
            m_codeSize = size;
            m_javaSrcEntryCnt = srcEntryCnt;

            CalculateSectionSize();
            WriteHeaders();

            WriteTextSection(pJITNativeCode);

            if (0U != m_javaSrcEntryCnt)
            {
                // write .JVASRC section;
                WriteString(m_javaSrcName);
                WriteString(m_jncClassName);
                WriteString(m_jncFuncName);

                m_fileStream.write(m_jitLoadAddr);

                m_fileStream.write(m_javaSrcEntryCnt);
                m_fileStream.write(pSrcInfo, sizeof(JavaSrcLineInfo) * m_javaSrcEntryCnt);

                // write padding
                WriteAlignmentPadding();
            }

            ret = true;
        }
    }

    return ret;
}

bool PeJncWriter::WriteJITNativeCode(const gtUByte* pJITNativeCode,
                                     unsigned int   size,
                                     const void*    pc2bc_blob,
                                     unsigned int   pc2bc_blob_size,
                                     void*          methodtableblob,
                                     unsigned int   methodtableblobsize,
                                     void*          stringtableblob,
                                     unsigned int   stringtableblobsize)
{
    m_majorVersion = MAJORIMAGEVERSION_WITH_INLINE; // inline version
    bool ret = false;

    if (NULL != pJITNativeCode)
    {
        if (m_fileStream.open(m_jncFileName, FMODE_TEXT("w+b")))
        {
            m_codeSize = size;

            // This is dummy value; Not used in inline case
            m_javaSrcEntryCnt = 1;

            unsigned int additionalSize = 0;
            additionalSize += pc2bc_blob_size;
            additionalSize += methodtableblobsize;
            additionalSize += stringtableblobsize;
            CalculateSectionSize(additionalSize);
            WriteHeaders();

            WriteTextSection(pJITNativeCode);

            // write .JVASRC section;
            m_fileStream.write(m_jitLoadAddr);
            m_fileStream.write((DWORD)JAVASRC_SECTION_COUNT); //subsection count

            m_fileStream.write((DWORD)(pc2bc_blob_size + sizeof(DWORD)));   // data size + type size
            m_fileStream.write((DWORD)PC2BC);//subsection type
            m_fileStream.write(pc2bc_blob, pc2bc_blob_size);  //subsection data

            m_fileStream.write((DWORD)(methodtableblobsize + sizeof(DWORD)));   // data size + type size
            m_fileStream.write((DWORD)METHOD_TABLE);//subsection type
            m_fileStream.write(methodtableblob, methodtableblobsize);  //subsection data

            m_fileStream.write((DWORD)(stringtableblobsize + sizeof(DWORD)));  // data size + type size
            m_fileStream.write((DWORD)STRING_TABLE);//subsection type
            m_fileStream.write(stringtableblob, stringtableblobsize);  //subsection data

            // write padding
            WriteAlignmentPadding();

            ret = true;
        }
    }

    return ret;
}

bool PeJncWriter::WriteCLRJITNativeCode(const gtUByte* pJITNativeCode, unsigned int size,
                                        const gtUByte* pILCode, unsigned int ilSize, unsigned int ilOffsetToImageBase,
                                        gtUInt32* pMap, gtUInt32 mapEntries)
{
    bool ret = false;

    if (NULL != pJITNativeCode && NULL != pILCode)
    {
        if (m_fileStream.open(m_jncFileName, FMODE_TEXT("w+b")))
        {
            m_codeSize = size;
            m_ilSize = ilSize;
            m_methodOffsetOfImage = ilOffsetToImageBase;
            m_ilNativeMapEntryCount = mapEntries;

            if (NULL == pMap)
            {
                m_ilNativeMapEntryCount = 0;
            }

            CalculateSectionSize();
            WriteHeaders();

            WriteTextSection(pJITNativeCode);

            if (0U != m_ilSize)
            {
                // write .CAIL section;

                // 1) module length and name
                WriteString(m_clrModuleName);

                // 2) class length and name
                WriteString(m_jncClassName);

                // 3) function length and name
                WriteString(m_jncFuncName);

                // 4) jit load address
                m_fileStream.write(m_jitLoadAddr);

                // 5) Method IL offset relative to module imagebase
                m_fileStream.write(m_methodOffsetOfImage);

                // 6) Write IL
                m_fileStream.write(m_ilSize);
                m_fileStream.write(pILCode, m_ilSize);

                // write padding
                WriteAlignmentPadding();
            }

            if (0 != m_ilNativeMapEntryCount)
            {
                // write .ILMap section;

                // 1) module length and name
                WriteString(m_clrModuleName);

                // 2) class length and name
                WriteString(m_jncClassName);

                // 3) function length and name
                WriteString(m_jncFuncName);

                // 4) jit load address
                m_fileStream.write(m_jitLoadAddr);

                // 5) Write counts for IL to Native Mapping
                //
                // Here is the IL Native Mapping (copied form corprof.h):
                // typedef struct COR_DEBUG_IL_TO_NATIVE_MAP
                // {
                //     ULONG32 ilOffset;
                //     ULONG32 nativeStartOffset;
                //     ULONG32 nativeEndOffset;
                // } COR_DEBUG_IL_TO_NATIVE_MAP;
                //
                m_fileStream.write(m_ilNativeMapEntryCount);
                m_fileStream.write(pMap, (sizeof(gtUInt32) * 3) * m_ilNativeMapEntryCount);

                // write padding
                WriteAlignmentPadding();
            }

            ret = true;
        }
    }

    return ret;
}

void PeJncWriter::WriteString(const wchar_t* pString)
{
    gtUInt32 strLength = static_cast<gtUInt32>(wcslen(pString) * sizeof(wchar_t));
    m_fileStream.write(strLength);
    m_fileStream.write(pString, strLength);
}

void PeJncWriter::WriteAlignmentPadding()
{
    gtByte padByte = 0;
    long writtenBytes = 0;

    if (m_fileStream.currentPosition(writtenBytes))
    {
        unsigned padBytes = ADDRESSALIGNMENT - (writtenBytes % ADDRESSALIGNMENT);

        for (unsigned cnt = 0; cnt < padBytes; cnt++)
        {
            m_fileStream.write(padByte);
        }
    }
}
