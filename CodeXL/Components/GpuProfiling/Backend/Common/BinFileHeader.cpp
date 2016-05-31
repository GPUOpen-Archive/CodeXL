//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file implements the methods to read the structure of the executable
///        file header (COFF for Windows/ELF for Linux)
//==============================================================================

#include <string>
#include <fstream>
#include <vector>
#include "BinFileHeader.h"
#include "FileUtils.h"
#include "FileUtilsDefs.h"
#include "StringUtils.h"

#include <AMDTOSWrappers/Include/osFile.h>

HeaderBinFile::HeaderBinFile(): iFileType(FileUtils::FTYPE_EXE), iFileNbrAddressBits(FileUtils::FILE_BITS_UNKNOWN)
{
}

HeaderBinFile::~HeaderBinFile()
{
}

#ifdef _WIN32
int HeaderBinFile::ReadHeader(const gtString& strFileName)
{
    osFile binaryFile(strFileName);

    if (!binaryFile.exists())
    {
        iFileType = FileUtils::FTYPE_UNKNOWN;
        return FileUtils::FILE_NOT_FOUND;
    }

    std::ifstream finput((const wchar_t*)strFileName.asCharArray(), std::ios::in | std::ios::binary);

    if (!finput.is_open())
    {
        iFileType = FileUtils::FTYPE_UNKNOWN;
        return FileUtils::FILE_NOT_OPENED;
    }

    size_t nDosHeaderSize = sizeof(IMAGE_DOS_HEADER);

    //get the file size
    finput.seekg(0, std::ios::end);
    size_t fileLength = (size_t)finput.tellg();

    if (fileLength < nDosHeaderSize)
    {
        iFileType = FileUtils::FTYPE_UNKNOWN;
        return FileUtils::FILE_HDR_NOT_KNOWN;
    }

    finput.seekg(0, std::ios::beg);

    std::vector<char> vHdrData;
    vHdrData.resize(fileLength);
    finput.read(&vHdrData[ 0 ], fileLength);

    hdrDOS.e_magic = (unsigned char)vHdrData[ 0 ] + ((unsigned char)vHdrData[ 1 ] << 8);

    if (hdrDOS.e_magic != IMAGE_DOS_SIGNATURE)
    {
        iFileType = FileUtils::FTYPE_NOT_WIN_BINARY;
        return FileUtils::FILE_HDR_NOT_KNOWN;
    }

    //offset field size is 4 bytes, so have to read out (in reverse order) 4 bytes
    //from end
    hdrDOS.e_lfanew = ((unsigned char)vHdrData[ nDosHeaderSize - 4 ]) +
                      ((unsigned char)vHdrData[ nDosHeaderSize - 3 ]   << 8) +
                      ((unsigned char)vHdrData[ nDosHeaderSize - 2 ]   << 16) +
                      ((unsigned char)vHdrData[ nDosHeaderSize - 1 ]   << 24) ;


    //check that this is a valid PE file format
    if ((size_t)hdrDOS.e_lfanew  > fileLength)
    {
        iFileType = FileUtils::FTYPE_NOT_WIN_BINARY;
        return FileUtils::FILE_HDR_NOT_KNOWN;
    }

    unsigned int COFFOffset = hdrDOS.e_lfanew;

    DWORD dwCoffSignature = (vHdrData[ COFFOffset + 3 ] << 24) +
                            (vHdrData[ COFFOffset + 2 ] << 16) +
                            (vHdrData[ COFFOffset + 1 ] << 8)  +
                            vHdrData[ COFFOffset ];
    hdrBinFile.Signature = dwCoffSignature;

    if (hdrBinFile.Signature != PE_SIGNATURE)
    {
        iFileType = FileUtils::FTYPE_NOT_WIN_BINARY;
        return FileUtils::FILE_HDR_NOT_KNOWN;
    }

    //get the COFF header
    COFFOffset = hdrDOS.e_lfanew + 4;

    if ((COFFOffset + COFF_HEADER_SIZE)  > fileLength)
    {
        iFileType = FileUtils::FTYPE_NOT_WIN_BINARY;
        return FileUtils::FILE_HDR_NOT_KNOWN;
    }

    hdrBinFile.FileHeader.Machine = vHdrData[ COFFOffset ] + (vHdrData[ COFFOffset + 1 ] << 8);
    hdrBinFile.FileHeader.NumberOfSections = vHdrData[ COFFOffset + 2 ] + (vHdrData[ COFFOffset + 3 ] << 8);
    hdrBinFile.FileHeader.TimeDateStamp =   vHdrData[ COFFOffset + 4 ]         +
                                            (vHdrData[ COFFOffset + 5 ] << 8) +
                                            (vHdrData[ COFFOffset + 6 ] << 16) +
                                            (vHdrData[ COFFOffset + 7 ] << 24);
    hdrBinFile.FileHeader.PointerToSymbolTable =   vHdrData[ COFFOffset + 8  ]         +
                                                   (vHdrData[ COFFOffset + 9  ] << 8) +
                                                   (vHdrData[ COFFOffset + 10 ] << 16) +
                                                   (vHdrData[ COFFOffset + 11 ] << 24);
    hdrBinFile.FileHeader.NumberOfSymbols =   vHdrData[ COFFOffset + 12 ] +
                                              (vHdrData[ COFFOffset + 13 ] << 8) +
                                              (vHdrData[ COFFOffset + 14 ] << 16) +
                                              (vHdrData[ COFFOffset + 15 ] << 24);
    hdrBinFile.FileHeader.SizeOfOptionalHeader =   vHdrData[ COFFOffset + 16 ] + (vHdrData[ COFFOffset + 17 ] << 8);
    hdrBinFile.FileHeader.Characteristics = vHdrData[COFFOffset + 18] + (vHdrData[COFFOffset + 19] << 8);


    //Fill in the attributes that the user may need
    GetFileTypeInt();
    GetFileNbrAddressBitsInt();

    return FileUtils::FILE_FOUND;
}

void HeaderBinFile::GetFileTypeInt()
{
    //TODO refactor :  below condition is always true!!!!, see https://msdn.microsoft.com/en-us/library/70wx36ch.aspx
    if ((iFileType != FileUtils::FTYPE_NOT_WIN_BINARY) || (iFileType != FileUtils::FTYPE_UNKNOWN))
    {
        if ((hdrBinFile.FileHeader.Characteristics & IMAGE_FILE_DLL) != 0)
        {
            iFileType = FileUtils::FTYPE_DLL;
        }
        else
        {
            iFileType = FileUtils::FTYPE_EXE;
        }
    }
}

void HeaderBinFile::GetFileNbrAddressBitsInt()
{
    if (hdrBinFile.FileHeader.Machine == IMAGE_FILE_MACHINE_I386)
    {
        iFileNbrAddressBits = FileUtils::FILE_X86;
    }
    else if (hdrBinFile.FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64)
    {
        iFileNbrAddressBits = FileUtils::FILE_X64;
    }
    else
    {
        iFileNbrAddressBits = FileUtils::FILE_BITS_UNKNOWN;
    }
}

#elif defined (_LINUX) || defined (LINUX)
int HeaderBinFile::ReadHeader(const gtString& strFileName)
{
    std::string convertedName;
    StringUtils::WideStringToUtf8String(strFileName.asCharArray(), convertedName);

    std::ifstream finput(convertedName.c_str(), std::ifstream::in | std::ifstream::binary);
    std::vector<char> vFile;

    if (!finput.bad())
    {
        finput.seekg(0, std::ios::end);
        size_t file_length = size_t(finput.tellg());

#ifndef BUILD_64

        //check that the file is large enough to contain at least the whole ELF header
        if (file_length < sizeof(Elf32_Ehdr))
        {
            iFileType = FileUtils::FTYPE_UNKNOWN;
            return FileUtils::FILE_HDR_NOT_KNOWN;
        }

#else

        if (file_length < sizeof(Elf64_Ehdr))
        {
            iFileType = FileUtils::FTYPE_UNKNOWN;
            return FileUtils::FILE_HDR_NOT_KNOWN;
        }

#endif

        finput.seekg(0, std::ios::beg);
#ifndef BUILD_64
        vFile.resize(sizeof(Elf32_Ehdr));
        finput.read(&vFile[0], sizeof(Elf32_Ehdr));
#else
        vFile.resize(sizeof(Elf64_Ehdr));
        finput.read(&vFile[0], sizeof(Elf64_Ehdr));
#endif
        finput.close();

#ifndef BUILD_64
        m_Header = *reinterpret_cast<const Elf32_Ehdr*>(&vFile[0]);
#else
        m_Header = *reinterpret_cast<const Elf64_Ehdr*>(&vFile[0]);
#endif


        //Read the user-requested attributes
        GetFileTypeInt();
        GetFileNbrAddressBitsInt();

        return FileUtils::FILE_FOUND;
    }

    return FileUtils::FILE_NOT_OPENED;
}

void HeaderBinFile::GetFileTypeInt()
{
    //Get the file type
    if (m_Header.e_type == ET_EXEC)
    {
        iFileType = FileUtils::FTYPE_EXE;
    }
    else if (m_Header.e_type == ET_DYN)
    {
        iFileType = FileUtils::FTYPE_DLL;
    }
    else if (m_Header.e_type == ET_REL)
    {
        iFileType = FileUtils::FTYPE_LIB;
    }
    else
    {
        iFileType = FileUtils::FTYPE_UNKNOWN;
    }
}

void HeaderBinFile::GetFileNbrAddressBitsInt()
{
    //Get the file format (x86 or x64)
    if (m_Header.e_ident[EI_CLASS] == ELFCLASS32)
    {
        iFileNbrAddressBits = FileUtils::FILE_X86;
    }
    else if (m_Header.e_ident[EI_CLASS] == ELFCLASS64)
    {
        iFileNbrAddressBits = FileUtils::FILE_X64;
    }
    else
    {
        iFileNbrAddressBits = FileUtils::FILE_BITS_UNKNOWN;
    }
}

#endif


int HeaderBinFile::GetFileType() const
{
    return iFileType;
}

int HeaderBinFile::GetFileNbrAddressBits() const
{
    return iFileNbrAddressBits;
}

int FileUtils::GetBinaryFileType(const gtString& strFileName)
{
    HeaderBinFile hdrBinFile;
    int nFileType = FileUtils::FTYPE_UNKNOWN;

    int nStatus = hdrBinFile.ReadHeader(strFileName);

    if ((nStatus == FileUtils::FILE_NOT_FOUND) || (nStatus == FileUtils::FILE_NOT_OPENED) ||
        (nStatus == FileUtils::FILE_HDR_NOT_KNOWN))
    {
        return FileUtils::FTYPE_UNKNOWN;
    }

    nFileType = hdrBinFile.GetFileType();

    return nFileType;
}

int FileUtils::GetBinaryNbrBits(const gtString& strFileName)
{
    HeaderBinFile hdrBinFile;
    int nFileNbrBits = FileUtils::FILE_BITS_UNKNOWN;

    int nStatus = hdrBinFile.ReadHeader(strFileName);

    if ((nStatus == FileUtils::FILE_NOT_FOUND) || (nStatus == FileUtils::FILE_NOT_OPENED) ||
        (nStatus == FileUtils::FILE_HDR_NOT_KNOWN))
    {
        return FileUtils::FILE_BITS_UNKNOWN;
    }

    nFileNbrBits = hdrBinFile.GetFileNbrAddressBits();

    return nFileNbrBits;
}
