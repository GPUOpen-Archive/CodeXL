//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ExecutableFile.h
///
//==================================================================================

#ifndef _EXECUTABLEFILE_H_
#define _EXECUTABLEFILE_H_

// Infra:
#include <AMDTBaseTools/Include/gtList.h>

// Local:
#include "SymbolEngine.h"

class EXE_API ExecutableFile
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
    ExecutableFile(const wchar_t* pImageName);

    /// -----------------------------------------------------------------------------------------------
    /// \brief A destructor for the class.
    ///
    /// The destructor will free all resources, and will safely call to Close().
    ///
    /// \sa Close()
    /// -----------------------------------------------------------------------------------------------
    virtual ~ExecutableFile();

    /// -----------------------------------------------------------------------------------------------
    /// \brief Opens the PE file for use with the whole class' interface.
    ///
    /// \param[in] loadAddress The load virtual address of the image.
    ///
    /// \return A boolean value indicating success or failure of opening the file.
    /// -----------------------------------------------------------------------------------------------
    virtual bool Open(gtVAddr loadAddress = GT_INVALID_VADDR) = 0;

    void Reset(const wchar_t* pImageName);

    /// -----------------------------------------------------------------------------------------------
    /// \brief Closes the PE file and free system resources.
    /// -----------------------------------------------------------------------------------------------
    virtual void Close() = 0;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Checks whether the image file has been successfully opened.
    ///
    /// \return A boolean value indicating whether the image file has been opened.
    /// -----------------------------------------------------------------------------------------------
    virtual bool IsOpen() const = 0;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Retrieves the image load virtual address.
    ///
    /// \return The image load virtual address.
    /// -----------------------------------------------------------------------------------------------
    gtVAddr GetLoadAddress() const;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Retrieves the image size.
    ///
    /// \return The image size.
    /// -----------------------------------------------------------------------------------------------
    virtual gtUInt32 GetImageSize() const = 0;

    const wchar_t* GetFilePath() const;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Translates a Virtual Address (VA) in the image to a Relative Virtual Address (RVA).
    ///
    /// \param[in] va The image virtual address.
    ///
    /// \return The corresponding image RVA.
    /// -----------------------------------------------------------------------------------------------
    gtRVAddr VaToRva(gtVAddr va) const;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Translates a Relative Virtual Address (RVA) in the image to a Virtual Address (VA).
    ///
    /// \param[in] rva The image relative virtual address.
    ///
    /// \return The corresponding image VA.
    /// -----------------------------------------------------------------------------------------------
    gtVAddr RvaToVa(gtRVAddr rva) const;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Checks whether this is a 64-bit image, or not.
    ///
    /// \return A boolean value indicating whether this is a 64-bit image, or not.
    /// -----------------------------------------------------------------------------------------------
    virtual bool Is64Bit() const = 0;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Retrieves the number of sections in the image.
    ///
    /// \return The number of sections in the image.
    /// -----------------------------------------------------------------------------------------------
    virtual unsigned GetSectionsCount() const = 0;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Retrieves the unique signature of the file.
    ///
    /// \return The unique signature of the file.
    /// -----------------------------------------------------------------------------------------------
    virtual gtUInt64 GetSignature() const = 0;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Retrieves the checksum of the file.
    ///
    /// \return The checksum of the file.
    /// -----------------------------------------------------------------------------------------------
    virtual gtUInt64 GetChecksum() const = 0;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Checks whether the debug info (PDB/DWARF/STAB debug info) is found for this file.
    ///
    /// \return A boolean value indicating whether debug info is found or not.
    /// -----------------------------------------------------------------------------------------------
    virtual bool IsDebugInfoAvailable() const = 0;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Checks whether the image section at \a index contains code.
    ///
    /// \param[in] index The index of the image section to check.
    ///
    /// \return A boolean value indicating whether the image section contains code.
    /// -----------------------------------------------------------------------------------------------
    virtual bool IsCodeSection(unsigned index) const = 0;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Searches for the index of the image section named \a pName.
    ///
    /// \param[in] pName The name of the image section to search.
    ///
    /// \return On success, the index of the image section. On failure, the number of image sections.
    /// -----------------------------------------------------------------------------------------------
    virtual unsigned LookupSectionIndex(const char* pName) const = 0;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Searches for the index of the image section containing the RVA.
    ///
    /// \param[in] rva The image relative virtual address.
    ///
    /// \return On success, the index of the image section. The number of image sections, on failure.
    /// -----------------------------------------------------------------------------------------------
    virtual unsigned LookupSectionIndex(gtRVAddr rva) const = 0;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Retrieves a pointer to the file's memory of the image section at \a index.
    ///
    /// \param[in] index The index of the image section.
    ///
    /// \return A pointer to the file's memory of the the image section. NULL on failure.
    /// -----------------------------------------------------------------------------------------------
    virtual const gtUByte* GetSectionBytes(unsigned index) const = 0;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Retrieves the limits RVAs of the image section at \a index.
    ///
    /// \param[in] index The index of the image section.
    /// \param[out] startRva The RVA in which the section starts.
    /// \param[out] endRva The RVA in which the section ends (this value is not inside the section).
    ///
    /// \return A boolean value indicating success or failure in retrieving the values.
    /// -----------------------------------------------------------------------------------------------
    virtual bool GetSectionRvaLimits(unsigned index, gtRVAddr& startRva, gtRVAddr& endRva) const = 0;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Retrieves the size of the image section at \a index.
    ///
    /// \param[in] index The index of the image section.
    ///
    /// \return The size of the the image section in bytes.
    /// -----------------------------------------------------------------------------------------------
    virtual gtUInt32 GetSectionSize(unsigned index) const = 0;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Retrieves the name of the image section at \a index.
    ///
    /// \param[in] index The index of the image section.
    ///
    /// \return The ANSI name of the the image section.
    /// -----------------------------------------------------------------------------------------------
    virtual const char* GetSectionName(unsigned index, unsigned* pLength = NULL) const = 0;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Retrieves the entry point to the image.
    ///
    /// \return The RVA entry point to the image. GT_INVALID_RVADDR on failure.
    /// -----------------------------------------------------------------------------------------------
    virtual gtRVAddr GetEntryPoint() const = 0;

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
                                        const wchar_t* pCachePath = NULL) = 0;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Retrieves a pointer to the file's memory corresponding to a RVA.
    ///
    /// \param[in] rva The image relative virtual address.
    /// \param[in,out] size The size in bytes of the memory block requested. Truncated if the block is smaller.
    ///
    /// \return A pointer to the file's memory of the the image section. NULL on failure.
    /// -----------------------------------------------------------------------------------------------
    virtual const gtUByte* GetMemoryBlock(gtRVAddr rva, gtUInt32& size) const = 0;

    /// -----------------------------------------------------------------------------------------------
    /// \brief Retrieves the associated Symbol Engine.
    ///
    /// \return A pointer to the symbol engine.
    /// -----------------------------------------------------------------------------------------------
    SymbolEngine* GetSymbolEngine() const;

    virtual gtVAddr GetImageBase() const = 0;

    virtual gtRVAddr GetCodeBase() const = 0;
    virtual gtUInt32 GetCodeSize() const = 0;

    bool IsSystemExecutable() const;

    void SetProcessInlineInfo(bool processInlineInfo, bool aggrInlineInstances = true);
    bool IsProcessInlineInfo() const;
    bool IsAggregateInlinedInstances() const;

    static ExecutableFile* Open(const wchar_t* pImageName, gtVAddr loadAddress = GT_INVALID_VADDR);

protected:
    gtVAddr m_loadAddress;                 ///< The image load virtual address

    wchar_t m_modulePath[OS_MAX_PATH];     ///< The full name of the executable file

    SymbolEngine* m_pSymbolEngine;

    bool m_processInlineInfo;
    bool m_aggrInlinedInstances;
};

#endif  // _EXECUTABLEFILE_H_