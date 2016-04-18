//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file implements the methods to read the structure of the executable
///        file header (COFF for Windows/ELF for Linux)
/// \brief
//==============================================================================

#ifndef _BIN_FILE_HEADER_H_
#define _BIN_FILE_HEADER_H_

#ifdef _WIN32
    #include <windows.h>
    #include <winnt.h>
#elif defined(_LINUX) || defined(LINUX)
    #include <limits.h>
    #if (__WORDSIZE == 64 )
        #define BUILD_64 1
    #endif
    #include <elf.h>
#endif

#include <AMDTBaseTools/Include/gtString.h>

#ifdef _WIN32
    const size_t COFF_HEADER_SIZE = 0x14;
    const DWORD  PE_SIGNATURE     = 0x00004550;
#endif

class HeaderBinFile
{
public:
    HeaderBinFile();
    ~HeaderBinFile();

    int ReadHeader(const gtString& strFile);
    int GetFileType(void) const;
    int GetFileNbrAddressBits(void) const;

protected:
#ifdef _WIN32
    IMAGE_DOS_HEADER hdrDOS;
    IMAGE_NT_HEADERS hdrBinFile;
#elif defined(_LINUX) || defined(LINUX)
#ifdef BUILD_64
    Elf64_Ehdr m_Header;
#else
    Elf32_Ehdr m_Header;
#endif
#endif
    void GetFileTypeInt();
    void GetFileNbrAddressBitsInt();
    int iFileType;
    int iFileNbrAddressBits;

};

namespace FileUtils
{
/// Get the type of binary file (executable, shared library)
/// \param strFileName file name
/// \return type of file ( greater or equal to zero); if return is negative, file error occurred
int GetBinaryFileType(const gtString& strFileName);

/// Get the number of bits of the binary file
/// \param strFileName file name
/// \return machine type (number of bits)
int GetBinaryNbrBits(const gtString& strFileName);
}

#endif // _BIN_FILE_HEADER_H_
