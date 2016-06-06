//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ElfFile.h
/// \brief This file contains a class for querying a ELF file information
///
//==================================================================================

#ifndef _ELFFILE_H_
#define _ELFFILE_H_

#include "ExecutableFile.h"

/// -----------------------------------------------------------------------------------------------
/// \class ElfFile
/// \brief A class for querying an Executable and Linkable Format (ELF) file information.
/// -----------------------------------------------------------------------------------------------
class EXE_API ElfFile : public ExecutableFile
{
public:
    /// -----------------------------------------------------------------------------------------------
    /// \brief A constructor for the class.
    ///
    /// The constructor only initializes the class' fields. In order to use the class' methods, a call
    /// to Open() must be made first.
    ///
    /// \param[in] pImageName The full name of the executable file.
    ///
    /// \sa Open()
    /// -----------------------------------------------------------------------------------------------
    ElfFile(const wchar_t* pImageName);

    /// -----------------------------------------------------------------------------------------------
    /// \brief A destructor for the class.
    ///
    /// The destructor will free all resources, and will safely call to Close().
    ///
    /// \sa Close()
    /// -----------------------------------------------------------------------------------------------
    virtual ~ElfFile();

    /// -----------------------------------------------------------------------------------------------
    /// \brief Opens the PE file for use with the whole class' interface.
    ///
    /// \param[in] loadAddress The load virtual address of the image.
    ///
    /// \return A boolean value indicating success or failure of opening the file.
    /// -----------------------------------------------------------------------------------------------
    virtual bool Open(gtVAddr loadAddress = GT_INVALID_VADDR);

    /// -----------------------------------------------------------------------------------------------
    /// \brief Closes the PE file and free system resources.
    /// -----------------------------------------------------------------------------------------------
    virtual void Close();

    /// -----------------------------------------------------------------------------------------------
    /// \brief Checks whether the image file has been successfully opened.
    ///
    /// \return A boolean value indicating whether the image file has been opened.
    /// -----------------------------------------------------------------------------------------------
    virtual bool IsOpen() const;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Retrieves the image size.
    ///
    /// \return The image size.
    /// -----------------------------------------------------------------------------------------------
    virtual gtUInt32 GetImageSize() const;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Checks whether this is a 64-bit image, or not.
    ///
    /// \return A boolean value indicating whether this is a 64-bit image, or not.
    /// -----------------------------------------------------------------------------------------------
    virtual bool Is64Bit() const;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Retrieves the number of sections in the image.
    ///
    /// \return The number of sections in the image.
    /// -----------------------------------------------------------------------------------------------
    virtual unsigned GetSectionsCount() const;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Retrieves the unique signature of the file.
    ///
    /// \return The unique signature of the file.
    /// -----------------------------------------------------------------------------------------------
    virtual gtUInt64 GetSignature() const;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Retrieves the checksum of the file.
    ///
    /// \return The checksum of the file.
    /// -----------------------------------------------------------------------------------------------
    virtual gtUInt64 GetChecksum() const;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Checks whether the debug info (DWARF/STAB  debug info) is found for this file.
    ///
    /// Note: This will be known only after calling InitializeSymbolEngine()
    ///
    /// \return A boolean value indicating whether debug info is found or not.
    /// -----------------------------------------------------------------------------------------------
    virtual bool IsDebugInfoAvailable() const;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Checks whether the image section at \a index contains code.
    ///
    /// \param[in] index The index of the image section to check.
    ///
    /// \return A boolean value indicating whether the image section contains code.
    /// -----------------------------------------------------------------------------------------------
    virtual bool IsCodeSection(unsigned index) const;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Searches for the index of the image section named \a pName.
    ///
    /// \param[in] pName The name of the image section to search.
    ///
    /// \return On success, the index of the image section. On failure, the number of image sections.
    /// -----------------------------------------------------------------------------------------------
    virtual unsigned LookupSectionIndex(const char* pName) const;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Searches for the index of the image section containing the RVA.
    ///
    /// \param[in] rva The image relative virtual address.
    ///
    /// \return On success, the index of the image section. The number of image sections, on failure.
    /// -----------------------------------------------------------------------------------------------
    virtual unsigned LookupSectionIndex(gtRVAddr rva) const;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Retrieves a pointer to the file's memory of the image section at \a index.
    ///
    /// \param[in] index The index of the image section.
    ///
    /// \return A pointer to the file's memory of the the image section. NULL on failure.
    /// -----------------------------------------------------------------------------------------------
    virtual const gtUByte* GetSectionBytes(unsigned index) const;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Retrieves the limits RVAs of the image section at \a index.
    ///
    /// \param[in] index The index of the image section.
    /// \param[out] startRva The RVA in which the section starts.
    /// \param[out] endRva The RVA in which the section ends (this value is not inside the section).
    ///
    /// \return A boolean value indicating success or failure in retrieving the values.
    /// -----------------------------------------------------------------------------------------------
    virtual bool GetSectionRvaLimits(unsigned index, gtRVAddr& startRva, gtRVAddr& endRva) const;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Retrieves the size of the image section at \a index.
    ///
    /// \param[in] index The index of the image section.
    ///
    /// \return The size of the the image section in bytes.
    /// -----------------------------------------------------------------------------------------------
    virtual gtUInt32 GetSectionSize(unsigned index) const;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Retrieves the name of the image section at \a index.
    ///
    /// \param[in] index The index of the image section.
    ///
    /// \return The ANSI name of the the image section.
    /// -----------------------------------------------------------------------------------------------
    virtual const char* GetSectionName(unsigned index, unsigned* pLength = NULL) const;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Retrieves the entry point to the image.
    ///
    /// \return The RVA entry point to the image. GT_INVALID_RVADDR on failure.
    /// -----------------------------------------------------------------------------------------------
    virtual gtRVAddr GetEntryPoint() const;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Initializes the functions information of the image file.
    ///
    /// \param[in] pSearchPath A list of semicolon separated directories to search for the symbols file.
    /// \param[in] pServerList A list of semicolon separated servers for downloading the symbol file.
    /// \param[in] pCachePath The directory where to download the symbol file to.
    ///
    /// \return A boolean value indicating success or failure of the initialization.
    /// -----------------------------------------------------------------------------------------------
    virtual bool InitializeSymbolEngine(const wchar_t* pSearchPath = NULL,
                                        const wchar_t* pServerList = NULL,
                                        const wchar_t* pCachePath = NULL);

    /// -----------------------------------------------------------------------------------------------
    /// \brief Retrieves a pointer to the file's memory corresponding to a RVA.
    ///
    /// \param[in] rva The image relative virtual address.
    /// \param[in,out] size The size in bytes of the memory block requested. Truncated if the block is smaller.
    ///
    /// \return A pointer to the file's memory of the the image section. NULL on failure.
    /// -----------------------------------------------------------------------------------------------
    virtual const gtUByte* GetMemoryBlock(gtRVAddr rva, gtUInt32& size) const;

    virtual gtVAddr GetImageBase() const;

    virtual gtRVAddr GetCodeBase() const;
    virtual gtUInt32 GetCodeSize() const;

private:
    // ElfSymbolEngine is a friend class, as it is actually an integral part of the ELF format.
    friend class ElfSymbolEngine;

    gtUInt64 m_imageBase;                ///< The image base address

    struct _Elf* m_pElf;
    int m_fd;

    gtUInt64 m_signature;
    gtUInt64 m_checksum;
    bool     m_isDebugInfoAvailable;
};

#endif // _ELFFILE_H_