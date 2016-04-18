//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ExecutableReader.h
///
//==================================================================================

#ifndef _EXECUTABLEREADER_H_
#define _EXECUTABLEREADER_H_

#include <AMDTOSWrappers/Include/osOSDefinitions.h>

class ExecutableReader
{
public:
    ExecutableReader();
    ~ExecutableReader();

    void Close();
    bool Open(const wchar_t* pModulePath);
    bool IsOpen() const;

    gtRVAddr GetCodeBase() const;
    gtUInt32 GetCodeSize() const;
    gtVAddr GetImageBase() const;
    bool Is64Bit() const;

    unsigned GetSectionsCount() const;
    bool GetSectionRvaLimits(unsigned index, gtRVAddr& startRva, gtRVAddr& endRva) const;
    unsigned LookupSectionIndex(const char* pName) const;
    const gtUByte* GetSectionBytes(unsigned index) const;
    bool IsCodeSection(unsigned index) const;
    const char* GetSectionShortName(unsigned index) const;

private:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    bool LoadImage(const wchar_t* pImageName);
    bool InitializeNtHeader(DWORD fileSize);
    bool InitializeImageHeaders(DWORD fileSize);

    const IMAGE_SECTION_HEADER* GetSectionByIndex(unsigned index) const;
    const IMAGE_SECTION_HEADER* LookupSectionByName(const char* pName) const;

    gtVAddr m_imageBase;                ///< The image base virtual address

    HANDLE m_hFile;                     ///< The image file
    HANDLE m_hFileMapping;              ///< The image file mapping handle
    gtUByte* m_pFileBase;               ///< The pointer to the base of the mapped image file

    IMAGE_NT_HEADERS* m_pNtHeader;      ///< The image NT file header
    IMAGE_SECTION_HEADER* m_pSections;  ///< The image sections array

#else // AMDT_BUILD_TARGET != AMDT_WINDOWS_OS
#endif
};

#endif // _EXECUTABLEREADER_H_
