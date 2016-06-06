//=====================================================================
// Copyright 2011 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/Common/Src/CElf/Include/CElf.h $
/// \version $Revision: #15 $
/// \brief  A set of classes to allow easy unpacking, manipulation and writing of ELF objects.
//
//=====================================================================
// $Id: //devtools/main/Common/Src/CElf/Include/CElf.h#15 $
// Last checkin:   $DateTime: 2015/11/02 15:29:15 $
// Last edited by: $Author: callan $
// Change list:    $Change: 546423 $
//=====================================================================

// ELF: Executable and Linking Format (ELF)
// There is an overview that looks reasonable at:
//    http://elinux.org/Executable_and_Linkable_Format_%28ELF%29
// The ELF specification is available at a couple of places:
//    http://refspecs.linuxbase.org/elf/elf.pdf
//    http://docs.oracle.com/cd/E19082-01/819-0690/index.html
// The latter is Oracles linkers/loader guide.
//
// Apparently, there have been extensions from the reference (TIS) (Oracle being
// one of the implementors) to deal with the case of very large ELF files.
//
// For purposes of this implementation, assume the standard (May, 1995)
// specification to apply.

//
// The in-memory formats are 64 bit.
// Conversion to/from ELF32 happens in CElf::Read/Write.
//

#ifndef CELF_H
#define CELF_H

#include <cassert>
#include <fstream>
#include <string>
#include <vector>
#include <limits>

#ifdef _WIN32
    #include "compat.h"
    #include "elf32.h"
    #include "elf64.h"
#else
    #include <stdint.h>
    #include "elf32.h"
    #include "elf64.h"
#endif

class CElf;
class CElfProgramSegment;
class CElfSection;
class CElfStringTable;
class CElfSymbolTable;

const Elf64_Xword MAX_32BIT_VALUE = (Elf64_Xword)std::numeric_limits<Elf32_Word>::max();
#define FITS32_OR_RETURN_FALSE(unsigned64val) if ((unsigned64val) > MAX_32BIT_VALUE) return false;


/// Class to read, modify and write ELF formatted data.
/// \todo Fill in all of the other Doxygen comments.
class CElf
{
public:

    /// ctor
    /// \param filename Name of file to read.
    CElf(const std::string& filename);

    /// ctor
    /// \param pStream Pointer to input stream.
    CElf(std::istream* pStream);

    /// ctor
    /// \param pStream Pointer to input stream.
    CElf(const std::vector<char>& vBinary);

    /// Copy ctor.
    CElf(const CElf& other);

    /// Assignment operator.
    CElf& operator= (const CElf& other);

    /// dtor
    ~CElf();

    /// Write out an ELF file.
    /// \param filename Name of file to write.
    /// \returns success
    bool Store(const std::string& filename);

    /// Write out an ELF stream.
    /// \param pStream Stream into which we write bytes.
    /// \returns success
    bool Store(std::ostream* pStream);

    /// Write out an ELF byte vector.
    /// \param pvBinary Vector into which we write bytes.
    /// \returns success
    bool Store(std::vector<char>* pvBinary);

    /// Did the CElf object get loaded correctly?
    /// \returns Is OK.
    bool good() const;

    //
    // Ehdr accessor functions.
    //

    /// Check byte stream for ELF magic cookie.
    static bool IsElf(const char* p);

    /// Check byte stream for ELF64.
    /// assumes you have checked fot ELF already.
    static bool IsElf64(const char* p);

    /// Get the e_info[EI_CLASS] field from the file header.
    unsigned char GetClass() const;
    /// Set the e_info[EI_CLASS] field for the file header.
    /// \returns success
    bool          SetClass(unsigned char ehdrClass);

    /// Get the e_info[EI_DATA] field from the file header.
    unsigned char GetDataEncoding() const;
    /// Set the e_info[EI_DATA] field for the file header.
    /// \returns success
    bool          SetDataEncoding(unsigned char encoding);

    /// Get the e_info[EI_VERSION] field from the file header.
    unsigned char GetEIdentVersion() const;
    /// Set the e_info[EI_VERSION] field for the file header.
    bool          SetEIdentVersion(unsigned char version);

    /// Get the e_type field from the file header.
    Elf64_Half    GetType() const;
    /// Set the e_type field for the file header.
    bool          SetType(Elf64_Half type);

    /// Get the e_machine field from the file header.
    Elf64_Half    GetMachine() const;
    /// Set the e_machine field for the file header.
    bool          SetMachine(Elf64_Half machine);

    /// Get the e_version field from the file header.
    Elf64_Word    GetVersion() const;
    /// Set the e_version field for the file header.
    bool          SetVersion(Elf64_Word version);

    /// Get the e_entry field from the file header.
    Elf64_Addr    GetEntry() const;
    /// Set the e_entry field for the file header.
    bool          SetEntry(Elf64_Addr entry);

    /// Get the e_flags field from the file header.
    Elf64_Word    GetFlags() const;
    /// Set the e_flags field for the file header.
    bool          SetFlags(Elf64_Word flags);

    //
    // Section manipulators.
    //

    /// Typedefs for iterating through sections.
    typedef std::vector<CElfSection*>::iterator             SectionIterator;
    typedef std::vector<CElfSection*>::const_iterator const_SectionIterator;

    /// Get current number of sections.
    size_t GetNumSections() const;

    /// Get a pointer to a section.
    const CElfSection* GetSection(const std::string& name) const;
    CElfSection* GetSection(const std::string& name);

    /// Get an iterator to a section.
    const_SectionIterator GetSectionIterator(const std::string& name) const;
    SectionIterator GetSectionIterator(const std::string& name);

    /// Add a new section.
    CElfSection* AddSection(const std::string& name,
                            Elf64_Word         type,
                            Elf64_Xword        flags,
                            Elf64_Word         info,
                            Elf64_Xword        addrAlign,
                            Elf64_Xword        entrySize);

    /// Remove a section.  Deletes associated symbols first.
    /// \returns success
    bool RemoveSection(SectionIterator section);
    /// Remove a section.  Deletes associated symbols first.
    /// \returns success
    bool RemoveSection(const std::string& name);
    /// Remove a section.  Deletes associated symbols first.
    /// \returns success
    bool RemoveSection(const CElfSection* pSec);

    /// Iterators.
    const_SectionIterator SectionsBegin() const;
    const_SectionIterator SectionsEnd() const;
    SectionIterator SectionsBegin();
    SectionIterator SectionsEnd();

    /// Get the section that is the symbol table.
    const CElfSymbolTable* GetSymbolTable() const;
    CElfSymbolTable* GetSymbolTable();

private:
    /// File Header.
    Elf64_Ehdr   m_Header;

    /// Is the data read into this object?
    bool         m_Initialized;

    /// Is the ELF 64 bit.
    /// For now this is set in CElf::Read and does not ever change.
    /// Future extension to the code could make it more flexible.
    bool         m_IsElf64;

    /// The sections.
    std::vector<CElfSection*> m_Sections;

    /// The program headers.
    std::vector<CElfProgramSegment*> m_ProgramSegments;

    /// The section header string table.
    /// There can only be one.
    CElfStringTable* m_SectionHeaderStringTable;

    /// The symbol table.
    /// The ELF spec currently only allows 1 SHT_SYMTAB
    /// and 1 SHT_DYNSYM section per object/image.
    CElfSymbolTable* m_SymbolTable;

    /// default ctor.  Disabled by making it private.
    CElf();

    /// Worker to convert from ELF bits to this C++ object.
    /// \param vBinary place to get bits.
    /// \returns success
    bool Read(const std::vector<char>& vBinary);

    /// Worker to convert from this C++ object to ELF bits.
    /// \param vBinary place to get bits.
    /// \returns success
    bool Write(std::vector<char>& vBinary);

    /// Where we detect sections types which we don't expect
    /// and which we have not yet debugged.
    bool UnsupportedSectionType(Elf64_Word sh_type);

    /// Get a section by index.
    /// We use this in constructing the C++ representation.
    /// Otherwise, we should look things up by name or use iterators.
    /// \param index which section to fetch.
    CElfSection* GetSection(size_t index) const;

    /// Utility to copy another CElf to this CElf.
    /// Use the copy constructor or assignment operator -- not this.
    void Copy(const CElf& celf);

    /// Utility to free resources.
    void Deinitialize();

    friend class CElfSymbolTable;
};


/// Program Headers.
/// TODO: I have lots of questions about these & what is the right thing to do.
/// TODO: OpenGL is the only user right now.  I expect only PT_LOAD kinds.
/// TODO: I think I need to make this load stuff point to a section, so
///       that modifications of the section are reflected in this loading.
///       Also, if the section is deleted, the header then can be deleted.
///       And writing these things out looks interesting.
/// TODO: Program headers are optional when there is a section table.
///       Section tables are optional when there is a program header.
///       I don't think that we can deal with ELFs that don't have section tables.
/// TODO: Maybe the right thing to do, if there are no sections,
///       is to attach the segment data here.  If there are sections,
///       it could be that sections describe all or part of a section's data.
///       Or maybe they could describe the whole file (this seems less likely).
///       I can think of ways to make both of those work.
///       I cannot imagine any other way this would make sense with both shdr's and phdrs.
class CElfProgramSegment
{
public:

    /// Get the section type.  PT_...
    Elf64_Word   GetType() const;
    /// Set the section type.  PT_...
    void         SetType(Elf64_Word type);

    /// Get the section flags.  PF_...
    Elf64_Xword  GetFlags() const;
    /// Set the section flags.  PF_...
    void         SetFlags(Elf64_Xword flags);

    // p_offset
    // file offset of contents (maybe private?)
    // I think we want to figure out what CElfSection this is.

    // p_vaddr
    // Virtual address in memory image.
    // Dunno.  Do we care?

    // p_paddr
    // Ignored.

    // p_filesz
    // Size of contents in file.
    // Maybe we should check that this matches the length of the section data.

    // p_memsz
    // Size of contents in memory.
    // Should be greater or equal to p_filesz.
    // Does .bss have a length that we should check.

    /// Get the alignment requirements.
    Elf64_Xword  GetAlign() const;
    /// Set the alignment requirements.
    bool         SetAlign(Elf64_Xword align);

protected:
    // TODO: When the rest of this stuff is implemented,
    // move the ctor code to a CElfProgramSegment with the rest of the implementation.
    /// Ctor
    CElfProgramSegment() {}

    /// Copy
    CElfProgramSegment(const CElfProgramSegment& other);

    /// Assignment
    CElfProgramSegment& operator= (CElfProgramSegment const& other);

    /// Copy file format program header into this object.
    /// \param[in] vBinary The ELF binary container.
    /// \param[in] phdrOff Offset of the section header in vBinary.
    /// \param[in] is64    Are we unpacking an ELF64?
    /// \returns   success
    //
    // For now limited to PT_LOAD kinds.
    //
    bool Load(const std::vector<char>& vBinary, std::size_t phdrOff, bool is64);

private:

    /// Section Header.
    Elf64_Phdr        m_ProgramHeader;

    /// Section containing the referenced data.
    CElfSection*      m_ReferencedSection;

    friend class CElf;
};


class CElfSection
{
public:
    /// dtor.  virtual because we inherit from this class.
    virtual ~CElfSection();

    /// Get the section name.
    std::string  GetName() const;
    /// Set the section name.
    void         SetName(const std::string& name);

    /// Get the section type.  SHT_...
    Elf64_Word   GetType() const;
    /// Set the section type.  SHT_...
    void         SetType(Elf64_Word type);

    /// Get the section flags.  SHF_...
    Elf64_Xword  GetFlags() const;
    /// Set the section flags.  SHF_...
    void         SetFlags(Elf64_Xword flags);

    /// Get the address for this section to load into memory.
    Elf64_Addr   GetAddress() const;
    /// Set the address for this section to load into memory.
    void         SetAddress(Elf64_Addr addr);

    /// Get the alignment requirements for this section.
    Elf64_Xword  GetAddrAlign() const;
    /// Set the alignment requirements for this section.
    bool         SetAddrAlign(Elf64_Xword align);

    /// Get the size of tabular entries in the data for this section.
    /// This applies to SHT_*SYM*, SHT_GROUP, SHT_REL* and others.
    Elf64_Xword  GetEntrySize() const;
    /// Set the size of tabular entries in the data for this section.
    void         SetEntrySize(Elf64_Xword size);

    /// Get the info value.
    /// Caution! If the section type SHT_ implies a section, don't use this.
    Elf64_Word         GetInfo() const;

    /// Get link associated section.
    const CElfSection* GetLinkSection() const;
    CElfSection* GetLinkSection();
    /// Set link associated section.
    bool         SetLinkSection(CElfSection* section);

    /// Get info associated section.
    const CElfSection* GetInfoSection() const;
    CElfSection* GetInfoSection();
    /// Set info associated section.
    bool         SetInfoSection(CElfSection* section);

    /// Get the size of the section's data.
    size_t       GetDataSize() const;

    /// Get the section's data.
    const std::vector<char>& GetData() const;
    std::vector<char>& GetMutableData();

    /// Set the section's data.
    bool SetData(const char* pData, size_t size);
    bool SetData(const std::string& data);
    bool SetData(const std::vector<char>& data);

    /// Append to the end of the section's data.
    bool AppendData(const char* pData, size_t size);
    bool AppendData(const std::string& data);
    bool AppendData(std::vector<char>& data);

    /// Empty the section's data area.
    void EraseData();

protected:

    /// Section Header.
    Elf64_Shdr        m_Header;

    /// If sh_type implies that sh_link is a section index, this gets used.
    CElfSection*      m_Link;

    /// If sh_type implies that sh_info is a section index, this gets used.
    CElfSection*      m_Info;

    /// Name of section.
    std::string       m_Name;

    /// The bits the section contains (if any).
    std::vector<char> m_Data;

    /// Ctor -- use CElf::AddSection.
    CElfSection();

private:

    /// Copy
    CElfSection(const CElfSection& other);

    /// Assignment
    CElfSection& operator= (CElfSection const& other);

    /// Copy file format section header and data into this object.
    /// \param[in] vBinary The ELF binary container.
    /// \param[in] shdrOff Offset of the section header in vBinary.
    /// \param[in] is64    Are we unpacking an ELF64?
    /// \returns   success
    bool Load(const std::vector<char>& vBinary, std::size_t shdrOff, bool is64);

    // These are for CElf which knows how to play with fire.
    // The fire is largely needed to write object.

    /// Get the section's file format link field.
    /// An index into the section table.
    Elf64_Word         GetLink() const;
    /// Set the section's file format link field.
    /// An index into the section table.
    void               SetLink(Elf64_Word link);

    /// Set the info value.
    /// Caution! If the section type SHT_ implies a section, this is only for CElf::Write.
    void               SetInfo(Elf64_Word info);

    /// Get the file offset of the section's data.
    Elf64_Off          GetDataOffset() const;
    /// Set the file offset of the section's data.
    void               SetDataOffset(Elf64_Off offset);

    /// Set the data size in the file format section header.
    void               SetDataSize(Elf64_Xword size);

    /// Get the file formated section header.
    const Elf64_Shdr&  GetShdr() const;

    friend class CElf;
};


class CElfStringTable : public CElfSection
{
public:
    /// dtor.
    virtual ~CElfStringTable();

    /// Add a string to the string table.
    /// \returns its offset within the table.
    size_t AddString(const std::string& str);

private:
    /// ctor.
    CElfStringTable();
    /// copy.
    CElfStringTable(const CElfStringTable& other);
    /// assign.
    CElfStringTable& operator= (const CElfStringTable& other);

    friend class CElf;
};


class CElfSymbolTable : public CElfSection
{
private:
    /// Where we keep the symbols.
    /// Tim tells me that there's a simple way to wrap these for
    /// efficient lookup.  We'll do that when we need to.
    /// For now we have very few symbols in each object.
    struct TableEntry
    {
        Elf64_Sym    symEntry;
        std::string  symName;
        CElfSection* symSection;
    };

public:
    virtual ~CElfSymbolTable();

    /// Process the table information.
    bool Load(const CElf* elf);

    /// Iterator type.
    typedef std::vector<TableEntry>::iterator             SymbolIterator;
    typedef std::vector<TableEntry>::const_iterator const_SymbolIterator;

    /// Iterator.
    const_SymbolIterator SymbolsBegin() const;
    SymbolIterator SymbolsBegin();

    /// Iterator.
    const_SymbolIterator SymbolsEnd() const;
    SymbolIterator SymbolsEnd();

    /// Get count of symbols in the table.
    size_t GetNumSymbols() const;

    /// Add a new symbol.
    bool AddSymbol(const std::string& name,
                   unsigned char      bind,
                   unsigned char      type,
                   unsigned char      other,
                   CElfSection*       section,
                   Elf64_Addr         value,
                   Elf64_Xword        size);


    // Tidy new interface.

    /// Get a handle for a symbol.
    /// \param name Symbol name.
    /// \returns Iterator or if symbol not found, SymbolIterator.end().
    const_SymbolIterator GetSymbol(const std::string& name) const;
    SymbolIterator GetSymbol(const std::string& name);

    bool GetInfo(const_SymbolIterator sym,
                 std::string*         name,
                 unsigned char*       bind,
                 unsigned char*       type,
                 unsigned char*       other,
                 CElfSection**        section,
                 Elf64_Addr*          value,
                 Elf64_Xword*         size) const;

    bool SetInfo(SymbolIterator     sym,
                 const std::string& name,
                 unsigned char      bind,
                 unsigned char      type,
                 unsigned char      other,
                 CElfSection*       section,
                 Elf64_Addr         value,
                 Elf64_Xword        size);

    SymbolIterator RemoveSymbol(SymbolIterator sym);



    // Interface more closely matching the exiting code.

    bool GetSymbol(const std::string& name,
                   unsigned char*     bind,
                   unsigned char*     type,
                   unsigned char*     other,
                   CElfSection**      section,
                   Elf64_Addr*        value,
                   Elf64_Word*        size) const;

    bool SetSymbol(const std::string& name,
                   unsigned char      bind,
                   unsigned char      type,
                   unsigned char      other,
                   CElfSection*       section,
                   Elf64_Addr         value,
                   Elf64_Word         size);

    bool RemoveSymbol(const std::string& name);



    /// Remove all of the symbols referencing this section.
    /// It's OK if there are no symbols.
    /// \param section Section to filter symbol table with.
    void RemoveSymbolsInSection(CElfSection* section);

    void SetExtendedIndexSection(CElfSection* section);

private:
    CElfSymbolTable();
    CElfSymbolTable(const CElfSymbolTable& other);
    CElfSymbolTable& operator= (const CElfSymbolTable& other);

    //
    // Data Members
    //

    std::vector<TableEntry> m_Table;
    CElfSection* m_ExtendedIndexSection;

    /// Set the st_name.
    /// This is only useful as part of Writing output.
    void SetNameIndex(SymbolIterator sym,
                      Elf64_Word     name);

    /// Set the st_shndx.
    /// This is only useful as part of Writing output.
    void SetSectionIndex(SymbolIterator sym,
                         size_t         index);

    /// Append the Elf64_Sym record into the section's data.
    /// This is only useful as part of Writing output.
    /// \param sym     The symbol to process.
    /// \param isElf64 Is the record to be processed in 64 bit format?
    /// \returns success
    bool CopySymbolRecordToData(const_SymbolIterator sym, bool isElf64);

    friend class CElf;
};

#endif // CELF_H
