//=====================================================================
// Copyright 2011 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/Common/Src/CElf/Src/CElf.cpp $
/// \version $Revision: #19 $
/// \brief  A set of classes to allow easy manipulation of ELF
///         binaries in memory.
//
//=====================================================================
// $Id: //devtools/main/Common/Src/CElf/Src/CElf.cpp#19 $
// Last checkin:   $DateTime: 2016/04/07 13:16:11 $
// Last edited by: $Author: callan $
// Change list:    $Change: 567789 $
//=====================================================================

#include <map>
#include <cstring>

#include "CElf.h"

#if !defined(PROGRAM_HEADER_SUPPORT)
    // The code for Program Sections merely reads the data structures.
    // I wanted this to debug the crazy containers that OpenGL creates.
    // In the future, we might find a reason to complete this code.
    #define PROGRAM_HEADER_SUPPORT 0
#endif

using namespace std;

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
// Ctors
//

CElf::CElf(
    const string& strFilename)
    : m_Initialized(false),
      m_IsElf64(false),
      m_SectionHeaderStringTable(NULL),
      m_SymbolTable(NULL)
{
    ifstream stream(strFilename.c_str(), ifstream::in | ifstream::binary);
    vector<char>vBinary;

    if (stream.good())
    {
        stream.seekg(0, ios::end);
        size_t length = size_t(stream.tellg());
        stream.seekg(0, ios::beg);

        vBinary.resize(length);

        if (length == 0)
        {
            return;
        }

        stream.read(&vBinary[0], length);
        stream.close();
        Read(vBinary);
    }
    else
    {
        stream.close();
    }
}


CElf::CElf(
    istream* stream)
    : m_Initialized(false),
      m_IsElf64(false),
      m_SectionHeaderStringTable(NULL),
      m_SymbolTable(NULL)
{
    stream->seekg(0, ios::end);
    size_t length = size_t(stream->tellg());
    stream->seekg(0, ios::beg);

    vector<char>vBinary;
    vBinary.resize(length);
    stream->read(&vBinary[0], length);

    Read(vBinary);
}


CElf::CElf(
    const vector<char>& vBinary)
    : m_Initialized(false),
      m_IsElf64(false),
      m_SectionHeaderStringTable(NULL),
      m_SymbolTable(NULL)
{
    Read(vBinary);
}


CElf::CElf(const CElf& celf)
    : m_Initialized(false),
      m_IsElf64(celf.m_IsElf64),
      m_SectionHeaderStringTable(NULL),
      m_SymbolTable(NULL)
{
    Copy(celf);
}


CElf& CElf::operator= (const CElf& celf)
{
    // check for self assignment.
    if (this == &celf)
    {
        return *this;
    }

    Deinitialize();
    Copy(celf);
    return *this;
}


//
// Dtor
//
CElf::~CElf()
{
    Deinitialize();
}


//
// Store
//

bool
CElf::Store(
    const string& strFilename)
{
    if (!good())
    {
        return false;
    }

    ofstream fileStream(strFilename.c_str(), ifstream::out | ifstream::binary);
    bool ret = false;

    if (fileStream.good())
    {
        ret = Store(&fileStream);
    }

    fileStream.close();
    return ret;
}

bool
CElf::Store(
    ostream* stream)
{
    if (!good())
    {
        return false;
    }

    vector<char> vBinary;
    bool ret = Write(vBinary);

    if (ret)
    {
        stream->write(&vBinary[0], vBinary.size());
    }

    return ret && stream->good();
}

bool
CElf::Store(
    vector<char>* pvBinary)
{
    if (!good())
    {
        return false;
    }

    return Write(*pvBinary);
}

bool
CElf::good() const
{
    return m_Initialized;
}


//
// Ehdr accessor functions.
//

bool
CElf::IsElf(const char* p)
{
    // this is the same 32/64.
    const Elf32_Ehdr* eh = reinterpret_cast<const Elf32_Ehdr*>(p);
    return (eh->e_ident[EI_MAG0] == ELFMAG0 &&
            eh->e_ident[EI_MAG1] == ELFMAG1 &&
            eh->e_ident[EI_MAG2] == ELFMAG2 &&
            eh->e_ident[EI_MAG3] == ELFMAG3);
}

bool
CElf::IsElf64(const char* p)
{
    // this is the same 32/64.
    const Elf32_Ehdr* eh = reinterpret_cast<const Elf32_Ehdr*>(p);
    return eh->e_ident[EI_CLASS] == ELFCLASS64;
}

unsigned char
CElf::GetClass() const
{
    return m_Header.e_ident[EI_CLASS];
}

bool
CElf::SetClass(unsigned char ehdrClass)
{
    switch (ehdrClass)
    {
        case ELFCLASS32:
            m_Header.e_ident[EI_CLASS] = ehdrClass;
            return true;

        case ELFCLASS64:

        // We don't support ELF64 yet.
        default:
            return false;
    }
}

unsigned char
CElf::GetDataEncoding() const
{
    return m_Header.e_ident[EI_DATA];
}

bool
CElf::SetDataEncoding(unsigned char encoding)
{
    switch (encoding)
    {
        case ELFDATA2LSB:
            m_Header.e_ident[EI_DATA] = encoding;
            return true;

        case ELFDATA2MSB:

        // We don't support big endian yet.
        default:
            return false;
    }
}

unsigned char
CElf::GetEIdentVersion() const
{
    return m_Header.e_ident[EI_VERSION];
}

Elf64_Half
CElf::GetType() const
{
    return m_Header.e_type;
}

bool
CElf::SetType(Elf64_Half type)
{
    /// \todo No checking here.  Does any make sense?
    m_Header.e_type = type;
    return true;
}

Elf64_Half
CElf::GetMachine() const
{
    return m_Header.e_machine;
}

bool
CElf::SetMachine(Elf64_Half machine)
{
    /// \todo No checking here.  Does any make sense?
    m_Header.e_machine = machine;
    return true;
}

Elf64_Word
CElf::GetVersion() const
{
    return m_Header.e_version;
}

bool
CElf::SetVersion(Elf64_Word version)
{
    /// \todo No checking here.  Does any make sense?
    m_Header.e_version = version;
    return true;
}

// I don't think this gets used with OpenCL kernels in ELF.
Elf64_Addr
CElf::GetEntry() const
{
    return m_Header.e_entry;
}

bool
CElf::SetEntry(Elf64_Addr entry)
{
    /// \todo Might check that entry offset hits something mapped by the Image.
    m_Header.e_entry = entry;
    return true;
}

Elf64_Word
CElf::GetFlags() const
{
    return m_Header.e_flags;
}

bool
CElf::SetFlags(Elf64_Word flags)
{
    m_Header.e_flags = flags;
    return true;
}

//
// Section manipulators.
//

size_t
CElf::GetNumSections() const
{
    assert(m_Initialized);
    return m_Sections.size();
}

const CElfSection*
CElf::GetSection(const string& name) const
{
    assert(m_Initialized);

    for (size_t i = 0; i < m_Sections.size(); i++)
    {
        if (name == m_Sections[i]->GetName())
        {
            return m_Sections[i];
        }
    }

    return NULL;
}

CElfSection*
CElf::GetSection(const string& name)
{
    assert(m_Initialized);

    for (size_t i = 0; i < m_Sections.size(); i++)
    {
        if (name == m_Sections[i]->GetName())
        {
            return m_Sections[i];
        }
    }

    return NULL;
}

CElf::const_SectionIterator
CElf::GetSectionIterator(const string& name) const
{
    assert(m_Initialized);

    const_SectionIterator it = SectionsBegin();

    while (it != SectionsEnd() && (*it)->m_Name != name)
    {
        ++it;
    }

    return it;
}

CElf::SectionIterator
CElf::GetSectionIterator(const string& name)
{
    assert(m_Initialized);

    SectionIterator it = SectionsBegin();

    while (it != SectionsEnd() && (*it)->m_Name != name)
    {
        ++it;
    }

    return it;
}

CElfSection*
CElf::AddSection(
    const string& name,
    Elf64_Word    type,
    Elf64_Xword   flags,
    Elf64_Word    info,
    Elf64_Xword   addrAlign,
    Elf64_Xword   entrySize)
{
    assert(m_Initialized);

    // These need to be created with CElfStringTable & CElfSymbolTable.
    // Also, we are only prepared to deal with one SYMTAB per object.
    // And DYNSYM is unimplemented.
    // If this code ever gets used to create objects from scratch,
    // I'll need to adjust this part to mirror the code in CElf::Read().
    if (type == SHT_DYNSYM ||
        type == SHT_SYMTAB ||
        type == SHT_STRTAB)
    {
        return NULL;
    }

    if (UnsupportedSectionType(type))
    {
        return NULL;
    }

    CElfSection* section = new CElfSection();

    section->SetName(name);
    section->SetData("", 0);
    section->m_Header.sh_type      = type;
    section->m_Header.sh_flags     = flags;
    // Does this function need an addr argument?
    section->m_Header.sh_addr      = 0;
    // sh_offset is meaningless here.  We'll recompute before writing.
    // sh_size will be determined from m_Data.size().
    // sh_link gets determined from m_Link.
    // sh_info might get determined from m_Info (if non-null).
    // Otherwise it's the data here.
    section->m_Header.sh_info      = info;
    section->m_Header.sh_addralign = addrAlign;
    section->m_Header.sh_entsize   = entrySize;
    m_Sections.push_back(section);

    return section;
}

bool
CElf::RemoveSection(
    SectionIterator section)
{
    // More code will be needed if either of these are to be removed.
    // We will never do this (I hope).
    assert(*section != m_SymbolTable &&
           *section != m_SectionHeaderStringTable);

    // Remove any symbols first.
    m_SymbolTable->RemoveSymbolsInSection(*section);

    // Delete the section.
    delete *section;

    // Remove the pointer to the section.
    m_Sections.erase(section);
    return true;
}

bool
CElf::RemoveSection(
    const string& name)
{
    CElf::SectionIterator it = GetSectionIterator(name);

    if (it == SectionsEnd())
    {
        return false;
    }

    RemoveSection(it);
    return true;
}

// This is not yet implemented.
// CElf::RemoveSection(const CElfSection* pSec)
// I don't think it's needed and am considering removing it from the API.


CElf::const_SectionIterator
CElf::SectionsBegin() const
{
    return m_Sections.begin();
}

CElf::SectionIterator
CElf::SectionsBegin()
{
    return m_Sections.begin();
}

CElf::const_SectionIterator
CElf::SectionsEnd() const
{
    return m_Sections.end();
}

CElf::SectionIterator
CElf::SectionsEnd()
{
    return m_Sections.end();
}

const CElfSymbolTable*
CElf::GetSymbolTable() const
{
    return m_SymbolTable;
}

CElfSymbolTable*
CElf::GetSymbolTable()
{
    return m_SymbolTable;
}

//
// Read
//
bool
CElf::Read(
    const vector<char>& vBinary)
{
    if ((sizeof(Elf32_Ehdr) > vBinary.size()) ||
        (IsElf64(&vBinary[0]) && (sizeof(Elf64_Ehdr) > vBinary.size())))
    {
        // Binary wasn't big enough to have a header
        return false;
    }

    m_IsElf64 = IsElf64(&vBinary[0]);

    if (m_IsElf64)
    {
        m_Header = *reinterpret_cast<const Elf64_Ehdr*>(&vBinary[0]);
    }
    else
    {
        // convert header 32->64.
        const Elf32_Ehdr* hdr32 = reinterpret_cast<const Elf32_Ehdr*>(&vBinary[0]);
        memcpy(m_Header.e_ident, hdr32->e_ident, sizeof(m_Header.e_ident));
        m_Header.e_type      = hdr32->e_type;
        m_Header.e_machine   = hdr32->e_machine;
        m_Header.e_version   = hdr32->e_version;
        m_Header.e_entry     = hdr32->e_entry;
        m_Header.e_phoff     = hdr32->e_phoff;
        m_Header.e_shoff     = hdr32->e_shoff;
        m_Header.e_flags     = hdr32->e_flags;
        m_Header.e_ehsize    = hdr32->e_ehsize;
        m_Header.e_phentsize = hdr32->e_phentsize;
        m_Header.e_phnum     = hdr32->e_phnum;
        m_Header.e_shentsize = hdr32->e_shentsize;
        m_Header.e_shnum     = hdr32->e_shnum;
        m_Header.e_shstrndx  = hdr32->e_shstrndx;
    }

    // Check our assumptions here.
    if (m_Header.e_ident[EI_MAG0]         != ELFMAG0 ||
        m_Header.e_ident[EI_MAG1]       != ELFMAG1 ||
        m_Header.e_ident[EI_MAG2]       != ELFMAG2 ||
        m_Header.e_ident[EI_MAG3]       != ELFMAG3 ||
        !(m_Header.e_ident[EI_CLASS]    == ELFCLASS32 ||
          m_Header.e_ident[EI_CLASS]    == ELFCLASS64) ||
        m_Header.e_ident[EI_DATA]       != ELFDATA2LSB ||
        m_Header.e_ident[EI_VERSION]    != EV_CURRENT ||
        // We don't care what the values are.
        // m_Header.e_ident[EI_OSABI]      != ELFOSABI_SYSV ||
        // m_Header.e_ident[EI_ABIVERSION] != 0 ||
        // m_Header.e_type                 != ET_NONE ||
        // Program headers are optional.  For now we are ignoring them.
        // m_Header.e_phnum                != 0 ||
        m_Header.e_version              != 1)
    {
        // If the file format is not as we expect, bail out now.
        // We expect (ELF32 or ELF64) and Little Endian
        return false;
    }

    // Up front checks for malformed content
    if ((m_Header.e_shoff > vBinary.size()) ||
        (m_Header.e_phoff > vBinary.size()))
    {
        return false;
    }

    // First figure out how many sections we need.
    uint64_t numberSections = m_Header.e_shnum;

    if (numberSections == 0)
    {
        // This can happen for two reasons:
        // 1. There are more than SHN_LORESERVE sections.
        // 2. This is an Execution View ELF file.

        // We will deal with 1. when we encounter such an object.
        // e_shnum
        //     This member holds the number of entries in the section header
        //     table. Thus the product of e_shentsize and e_shnum gives the
        //     section header table's size in bytes. If a file has no section
        //     header table, e_shnum holds the value zero.
        //
        //     If the number of sections is greater than or equal to
        //     SHN_LORESERVE (0xff00), this member has the value zero and the
        //     actual number of section header table entries is contained in the
        //     sh_size field of the section header at index 0. (Otherwise, the
        //     sh_size member of the initial entry contains 0.)

        // We don't handle 2.
        // We require at least one Section.
        return false;
    }

    // Get the Section Headers String Table.
    Elf64_Word shstrtabIndex = m_Header.e_shstrndx;

    if (shstrtabIndex == SHN_XINDEX)
    {
        // This code is untested (and so is commented out).
        // When we encounter such an object, we will debug it.
        //
        // e_shstrndx
        //     This member holds the section header table index of the entry
        //     associated with the section name string table. If the file has no
        //     section name string table, this member holds the value
        //     SHN_UNDEF.
        //
        //     If the section name string table section index is greater than or
        //     equal to SHN_LORESERVE (0xff00), this member has the value
        //     SHN_XINDEX (0xffff) and the actual index of the section name
        //     string table section is contained in the sh_link field of the
        //     section header at index 0. (Otherwise, the sh_link member of the
        //     initial entry contains 0.)

        // const Elf32_Shdr* section0 = reinterpret_cast<const Elf32_Shdr*>(&vBinary[m_Header.e_shoff]);
        // const Elf64_Shdr* section0 = reinterpret_cast<const Elf64_Shdr*>(&vBinary[m_Header.e_shoff]);
        // shstrtabIndex = section0->sh_link;
        return false;
    }

    size_t strTabOffset = size_t((m_IsElf64 ? sizeof(Elf64_Shdr) : sizeof(Elf32_Shdr)) *
                                 shstrtabIndex + m_Header.e_shoff);

    if (strTabOffset > vBinary.size())
    {
        // Deformed - bail out
        return false;
    }

    Elf64_Off   shstrtabDataOffset;
    Elf64_Xword shstrtabDataSize;

    if (m_IsElf64)
    {
        const Elf64_Shdr* shstrtabHeader = reinterpret_cast<const Elf64_Shdr*>(&vBinary[strTabOffset]);
        shstrtabDataOffset = shstrtabHeader->sh_offset;
        shstrtabDataSize   = shstrtabHeader->sh_size;
    }
    else
    {
        const Elf32_Shdr* shstrtabHeader = reinterpret_cast<const Elf32_Shdr*>(&vBinary[strTabOffset]);
        shstrtabDataOffset = shstrtabHeader->sh_offset;
        shstrtabDataSize   = shstrtabHeader->sh_size;
    }

    // Size the sections vector.
    m_Sections.resize(size_t(numberSections), NULL);

    for (size_t i = 0; i < numberSections; i++)
    {
        size_t   thisHdrOffset = (m_IsElf64 ? sizeof(Elf64_Shdr) : sizeof(Elf32_Shdr)) * i + size_t(m_Header.e_shoff);

        if (thisHdrOffset > vBinary.size())
        {
            // Deformed - bail out
            return false;
        }

        Elf64_Word sh_type;

        if (m_IsElf64)
        {
            const Elf64_Shdr* header = reinterpret_cast<const Elf64_Shdr*>(&vBinary[thisHdrOffset]);
            sh_type = header->sh_type;
        }
        else
        {
            const Elf32_Shdr* header = reinterpret_cast<const Elf32_Shdr*>(&vBinary[thisHdrOffset]);
            sh_type = header->sh_type;
        }

        if (UnsupportedSectionType(sh_type))
        {
            // Presumably the user of the CElf will call the dtor.
            // That will clean up whatever gunk we have laying around.
            return false;
        }

        CElfSection* section;

        if (sh_type == SHT_SYMTAB)
        {
            // The ELF spec (currently) only lets there be one of these.
            // Check for malformed input.
            assert(m_SymbolTable == NULL);

            m_SymbolTable = new CElfSymbolTable();
            section = m_SymbolTable;
        }
        else if (i == shstrtabIndex)
        {
            m_SectionHeaderStringTable = new CElfStringTable();
            section = m_SectionHeaderStringTable;
        }
        else if (sh_type == SHT_STRTAB)
        {
            section = new CElfStringTable();
        }
        else
        {
            section = new CElfSection();
        }

        m_Sections[i] = section;

        section->Load(vBinary, (m_IsElf64 ? sizeof(Elf64_Shdr) : sizeof(Elf32_Shdr)) * i + size_t(m_Header.e_shoff), m_IsElf64);
        assert(section->m_Header.sh_name < shstrtabDataSize);

        if (section->m_Header.sh_name >= shstrtabDataSize)
        {
            return false;
        }

        section->SetName(&vBinary[size_t(shstrtabDataOffset + section->m_Header.sh_name)]);
    }

    // Now that we have all of our sections, set up the inter-section pointers.
    for (size_t i = 0; i < numberSections; i++)
    {
        CElfSection* section = m_Sections[i];

        // Make sh_info and sh_link fields into the appropriate pointers.
        // This code parallels that in CElf::Copy.
        switch (section->GetType())
        {
            // sh_link.
            //       SHT_DYNAMIC      String table used by entries in this section
            //       SHT_HASH         Symbol table to which the hash table applies
            //       SHT_REL          Section header index of the associated symbol table
            //       SHT_RELA         Section header index of the associated symbol table
            //       SHT_SYMTAB       Section header index of the associated string table
            //       SHT_DYNSYM       Section header index of the associated string table
            //       SHT_GROUP        Section header index of the associated symbol table
            //       SHT_SYMTAB_SHNDX Section header index of the associated symbol table

            // sh_info
            //       SHT_REL    Section index of section to which the relocations apply
            //       SHT_RELA   Section index of section to which the relocations apply
            //       SHT_SYMTAB Index of first non-local symbol (i.e., number of local symbols)
            //       SHT_DYNSYM Index of first non-local symbol (i.e., number of local symbols)
            //       SHT_GROUP  The symbol table index of an entry in the associated symbol table.
            //                  The name of the specified symbol table entry provides
            //                  a signature for the group section.

            // sh_info
            // This member holds extra information, whose interpretation
            // depends on the section type. A table below describes the values. If
            // the sh_flags field for this section header includes the attribute
            // SHF_INFO_LINK, then this member represents a section header table
            // index.

            case SHT_DYNAMIC:
                section->SetLinkSection(m_Sections[section->GetLink()]);
                break;

            case SHT_HASH:
                section->SetLinkSection(m_Sections[section->GetLink()]);
                break;

            case SHT_REL:
                section->SetLinkSection(m_Sections[section->GetLink()]);
                section->SetInfoSection(m_Sections[section->GetInfo()]);
                break;

            case SHT_RELA:
                section->SetLinkSection(m_Sections[section->GetLink()]);
                section->SetInfoSection(m_Sections[section->GetInfo()]);
                break;

            case SHT_SYMTAB:
                section->SetLinkSection(m_Sections[section->GetLink()]);
                break;

            case SHT_DYNSYM:
                section->SetLinkSection(m_Sections[section->GetLink()]);
                break;

            case SHT_GROUP:
                section->SetLinkSection(m_Sections[section->GetLink()]);
                section->SetInfoSection(m_Sections[section->GetInfo()]);
                break;

            case SHT_SYMTAB_SHNDX:
                section->SetLinkSection(m_Sections[section->GetLink()]);
                assert(m_Sections[section->GetLink()]->GetType() == SHT_SYMTAB ||
                       m_Sections[section->GetLink()]->GetType() == SHT_DYNSYM);
                // Tell the symbol table where its extended index is.
                dynamic_cast<CElfSymbolTable*>(
                    m_Sections[section->GetLink()])->SetExtendedIndexSection(section);
                break;
        }
    }

    // Set up the symbol table.
    // DX generates ELF objects without symbol tables.
    if (m_SymbolTable != NULL)
    {
        m_SymbolTable->Load(this);
    }

#if PROGRAM_HEADER_SUPPORT
    // TODO: Deal with optional program headers.
    // For now we are going to ignore them.
    m_ProgramSegments.resize(m_Header.e_phnum, NULL);

    for (Elf64_Half i = 0; i < m_Header.e_phnum; i++)
    {
        m_ProgramSegments[i] = new CElfProgramSegment;
        size_t headerOff = (size_t)(m_Header.e_phoff + i * m_Header.e_phentsize);

        if (m_IsElf64)
        {
            m_ProgramSegments[i]->m_ProgramHeader = *reinterpret_cast<const Elf64_Phdr*>(&vBinary[headerOff]);
        }
        else
        {
            // convert header 32->64.
            const Elf32_Phdr* phdr32 = reinterpret_cast<const Elf32_Phdr*>(&vBinary[headerOff]);
            m_ProgramSegments[i]->m_ProgramHeader.p_type   = phdr32->p_type;
            m_ProgramSegments[i]->m_ProgramHeader.p_flags  = phdr32->p_flags;
            m_ProgramSegments[i]->m_ProgramHeader.p_offset = phdr32->p_offset;
            m_ProgramSegments[i]->m_ProgramHeader.p_vaddr  = phdr32->p_vaddr;
            m_ProgramSegments[i]->m_ProgramHeader.p_paddr  = phdr32->p_paddr;
            m_ProgramSegments[i]->m_ProgramHeader.p_filesz = phdr32->p_filesz;
            m_ProgramSegments[i]->m_ProgramHeader.p_memsz  = phdr32->p_memsz;
            m_ProgramSegments[i]->m_ProgramHeader.p_align  = phdr32->p_align;
        }
    }

#endif

    m_Initialized = true;
    return true;
}

bool
CElf::Write(vector<char>& vBinary)
{
    //
    // General method:
    // 0. Check the number of sections.
    // 1. Make new string tables.
    // 2. Refresh the file format data structures.
    //    This will involve updating:
    //    a. Symbol table section numbers.
    //    b. Section link/info fields where those are section numbers.
    //    c. File offsets of section data.
    // 3. Write the section headers.
    // 4. Write the section data.
    // 5. Write the file header.
    //

    // Make sure there's no dreck in the vector.
    vBinary.resize(0);

    if (GetNumSections() >= SHN_LORESERVE)
    {
        // The SHN_XINDEX case is not yet implemented, not yet debugged.
        return false;
    }

    // Empty section header string table.
    m_SectionHeaderStringTable->EraseData();

    // Make a map from CElfSection* to index.
    map<CElfSection*, size_t> SectionMap;
    {
        size_t i = 0;

        for (SectionIterator it = SectionsBegin(); it != SectionsEnd(); i++, ++it)
        {
            SectionMap[*it] = i;
        }
    }

    // for each section
    //   1. add its name to the section header string table.
    //   2. update link/info fields
    for (SectionIterator it = SectionsBegin(); it != SectionsEnd(); ++it)
    {
        // If we add support for SHN_XINDEX, more code will be needed here.
        // Values above SHN_LORESERVE will set SHN_XINDEX.
        // Other code will need to set up the associated SHT_SYMTAB_SHNDX section.
        size_t name = m_SectionHeaderStringTable->AddString((*it)->GetName());
        (*it)->m_Header.sh_name = static_cast<Elf32_Word>(name);

        if ((*it)->GetLinkSection())
        {
            size_t link = SectionMap.find((*it)->GetLinkSection())->second;
            (*it)->SetLink(static_cast<Elf64_Word>(link));
        }

        if ((*it)->GetInfoSection())
        {
            size_t info = SectionMap.find((*it)->GetInfoSection())->second;
            (*it)->SetInfo(static_cast<Elf64_Word>(info));
        }
    }

    // Empty symbol table string table.
    if (nullptr != m_SymbolTable)
    {

        CElfStringTable* symbolStrings =
            dynamic_cast<CElfStringTable*>(m_SymbolTable->GetLinkSection());
        symbolStrings->EraseData();

        // Empty the symbol table data (CElfSection.m_Data).
        // The real data is kept in the m_Table TableEntries.
        m_SymbolTable->EraseData();

        // for each symbol
        //   1. add its name to the symbol table string table.
        //   2. update section number
        //   3. copy the record to the CElfSection.m_Data.
        for (CElfSymbolTable::SymbolIterator it = m_SymbolTable->SymbolsBegin();
             it != m_SymbolTable->SymbolsEnd();
             ++it)
        {
            string    name;
            unsigned char  bind;
            unsigned char  type;
            unsigned char  other;
            CElfSection*   section;
            Elf64_Addr     value;
            Elf64_Xword    size;

            m_SymbolTable->GetInfo(it, &name, &bind, &type, &other, &section, &value, &size);

            size_t nameIndex = symbolStrings->AddString(name);
            m_SymbolTable->SetNameIndex(it, static_cast<Elf64_Word>(nameIndex));

            // If we add support for SHN_XINDEX, more code will be needed here.
            // Values above SHN_LORESERVE will set SHN_XINDEX.
            // Other code will need to set up the associated SHT_SYMTAB_SHNDX section.
            m_SymbolTable->SetSectionIndex(it, SectionMap.find(section)->second);

            bool ok = m_SymbolTable->CopySymbolRecordToData(it, m_IsElf64);

            if (!ok)
            {
                return false;
            }
        }//for symbol table
    }


    // Update the file offsets of the data in the section headers.
    // Update the sizes in the file format section headers.
    // SHN_XINDEX support would need more code here.
    m_Header.e_shnum = static_cast<Elf64_Half>(GetNumSections());
    std::streamoff fileOffset = m_IsElf64 ? sizeof(Elf64_Ehdr) : sizeof(Elf32_Ehdr);

    for (SectionIterator it = SectionsBegin(); it != SectionsEnd(); ++it)
    {
        size_t size = (*it)->GetDataSize();
        (*it)->SetDataSize(static_cast<Elf64_Word>(size));
        Elf64_Xword align = (*it)->GetAddrAlign();

        // Round up to our required alignment.
        if (size != 0)
        {
            fileOffset = size_t((fileOffset + align - 1) & ~(align - 1));
            (*it)->SetDataOffset(static_cast<Elf64_Word>(fileOffset));
            fileOffset += size;
        }
        else
        {
            (*it)->SetDataOffset(0);
        }
    }

    // Make space for the section headers.
    // Section headers can go anywhere in the file after the Ehdr.
    // Align to an 8 byte boundary.
    fileOffset = (fileOffset + 7) & ~7;
    m_Header.e_shoff = static_cast<Elf64_Word>(fileOffset);
    fileOffset += m_Header.e_shnum * m_Header.e_shentsize;

    // We now know how big the output will be.
    // Allocate the space.
    vBinary.resize(size_t(fileOffset));

    // Update the file header.
    // This might have moved with add/delete sections.
    // Need to update its location.
    // Again, more code needed here for SHN_XINDEX.
    size_t stringTableIndex = SectionMap.find(m_SectionHeaderStringTable)->second;
    assert(stringTableIndex < SHN_LORESERVE);
    m_Header.e_shstrndx = static_cast<Elf64_Half>(stringTableIndex);

    // Write the header.
    if (m_IsElf64)
    {
        *reinterpret_cast<Elf64_Ehdr*>(&vBinary[0]) = m_Header;
    }
    else
    {
        Elf32_Ehdr* hdr32 = reinterpret_cast<Elf32_Ehdr*>(&vBinary[0]);

        FITS32_OR_RETURN_FALSE(m_Header.e_entry);
        FITS32_OR_RETURN_FALSE(m_Header.e_phoff);
        FITS32_OR_RETURN_FALSE(m_Header.e_shoff);

        memcpy(hdr32->e_ident, m_Header.e_ident, sizeof(m_Header.e_ident));
        hdr32->e_type      = (Elf32_Half) m_Header.e_type;
        hdr32->e_machine   = (Elf32_Half) m_Header.e_machine;
        hdr32->e_version   = (Elf32_Word) m_Header.e_version;
        hdr32->e_entry     = (Elf32_Addr) m_Header.e_entry;
        hdr32->e_phoff     = (Elf32_Off) m_Header.e_phoff;
        hdr32->e_shoff     = (Elf32_Off) m_Header.e_shoff;
        hdr32->e_flags     = (Elf32_Word) m_Header.e_flags;
        hdr32->e_ehsize    = (Elf32_Word) m_Header.e_ehsize;
        hdr32->e_phentsize = (Elf32_Word) m_Header.e_phentsize;
        hdr32->e_phnum     = (Elf32_Word) m_Header.e_phnum;
        hdr32->e_shentsize = (Elf32_Word) m_Header.e_shentsize;
        hdr32->e_shnum     = (Elf32_Word) m_Header.e_shnum;
        hdr32->e_shstrndx  = (Elf32_Word) m_Header.e_shstrndx;
    }

    // Write the section data.
    for (SectionIterator it = SectionsBegin(); it != SectionsEnd(); ++it)
    {
        size_t dest = size_t((*it)->GetDataOffset());
        size_t size = (*it)->GetDataSize();

        if (size != 0)
        {
            const vector<char>& sectionData = (*it)->GetData();
            memcpy(&vBinary[dest], &sectionData[0], size);
        }
    }

    // Write the section headers.
    {
        size_t i = 0;

        for (SectionIterator it = SectionsBegin(); it != SectionsEnd(); ++it, i++)
        {
            size_t offset = size_t(m_Header.e_shoff + i * m_Header.e_shentsize);

            // The placement of this code is asymetric with ::Read.
            // There the section headers get filled in within the sections ::Load method.
            // Such is life.
            if (m_IsElf64)
            {
                *reinterpret_cast<Elf64_Shdr*>(&vBinary[offset]) = (*it)->GetShdr();
            }
            else
            {
                const Elf64_Shdr& shdr64 = (*it)->GetShdr();
                Elf32_Shdr* shdr32 = reinterpret_cast<Elf32_Shdr*>(&vBinary[offset]);

                FITS32_OR_RETURN_FALSE(shdr64.sh_flags);
                FITS32_OR_RETURN_FALSE(shdr64.sh_addr);
                FITS32_OR_RETURN_FALSE(shdr64.sh_offset);
                FITS32_OR_RETURN_FALSE(shdr64.sh_size);
                FITS32_OR_RETURN_FALSE(shdr64.sh_addralign);
                FITS32_OR_RETURN_FALSE(shdr64.sh_entsize);

                shdr32->sh_name      = (Elf32_Word) shdr64.sh_name;
                shdr32->sh_type      = (Elf32_Word) shdr64.sh_type;
                shdr32->sh_flags     = (Elf32_Word) shdr64.sh_flags;
                shdr32->sh_addr      = (Elf32_Addr) shdr64.sh_addr;
                shdr32->sh_offset    = (Elf32_Off)  shdr64.sh_offset;
                shdr32->sh_size      = (Elf32_Word) shdr64.sh_size;
                shdr32->sh_link      = (Elf32_Word) shdr64.sh_link;
                shdr32->sh_info      = (Elf32_Word) shdr64.sh_info;
                shdr32->sh_addralign = (Elf32_Word) shdr64.sh_addralign;
                shdr32->sh_entsize   = (Elf32_Word) shdr64.sh_entsize;
            }
        }
    }

    return true;
}

bool
CElf::UnsupportedSectionType(
    Elf32_Word sh_type)
{
    // Some of these would be trivial to support.
    // But I want to debug cases with sample input
    // before I claim that they work.
    // Some are harder:
    // SHT_GROUP has ordering requirements in the section header table.
    // A newly created SYMTAB_SHNDX section makes bookkeeping harder.
    // SHT_HASH is only used in image files with program headers (not objects with sections).
    switch (sh_type)
    {
        //case SHT_REL:
        case SHT_RELA:
        case SHT_HASH:
        case SHT_DYNAMIC:
        case SHT_SHLIB:
        case SHT_GROUP:
        case SHT_SYMTAB_SHNDX:
            return true;
    }

    return false;
}

CElfSection*
CElf::GetSection(size_t index) const
{
    return m_Sections[index];
}

void
CElf::Copy(const CElf& celf)
{
    if (!celf.m_Initialized)
    {
        return;
    }

    // This is a utility with below...

    // Copy the image header to get the ones that we care about.
    m_Header = celf.m_Header;
    size_t numberSections = celf.m_Sections.size();
    m_Sections.resize(numberSections);

    // Iterate over celf's sections.
    // Copy, but don't populate the string tables.
    // Copy ordinary sections.
    // Make a map from celf's sections to their index.
    map<const CElfSection*, size_t> sectionMap;

    for (size_t i = 0; i < numberSections; ++i)
    {
        sectionMap[celf.m_Sections[i]] = i;

        CElfSection* section;

        switch (celf.m_Sections[i]->GetType())
        {
            case SHT_SYMTAB:
                m_SymbolTable = new CElfSymbolTable();
                section = m_SymbolTable;
                break;

            case SHT_STRTAB:
                if (celf.m_Sections[i] == celf.m_SectionHeaderStringTable)
                {
                    m_SectionHeaderStringTable = new CElfStringTable();
                    section = m_SectionHeaderStringTable;
                }
                else
                {
                    section = new CElfStringTable();
                }

                break;

            default:
                section = new CElfSection();
                const vector<char>& data = celf.m_Sections[i]->GetData();
                section->SetData(data);
                break;
        }

        m_Sections[i] = section;

        // I could just copy celf.m_Sections[i]->m_Header, but that's ugly.
        // Copy just the fields that are maintained through the section's lifetime.
        section->SetType(celf.m_Sections[i]->GetType());
        section->SetName(celf.m_Sections[i]->GetName());
        section->SetFlags(celf.m_Sections[i]->GetFlags());
        section->SetAddrAlign(celf.m_Sections[i]->GetAddrAlign());
        section->SetEntrySize(celf.m_Sections[i]->GetEntrySize());
        // Default sh_info fields here.
        // Some will get overriden with SetInfoSection below, but we need the rest.
        section->SetInfo(celf.m_Sections[i]->GetInfo());
    }

    // Deal with link and info fields.
    // This code parallels that in CElf::Read.
    for (size_t i = 0; i < numberSections; i++)
    {
        CElfSection* section = m_Sections[i];

        switch (section->GetType())
        {
            case SHT_DYNAMIC:
                section->SetLinkSection(m_Sections[sectionMap[celf.m_Sections[i]->GetLinkSection()]]);
                break;

            case SHT_HASH:
                section->SetLinkSection(m_Sections[sectionMap[celf.m_Sections[i]->GetLinkSection()]]);
                break;

            case SHT_REL:
                section->SetLinkSection(m_Sections[sectionMap[celf.m_Sections[i]->GetLinkSection()]]);
                section->SetInfoSection(m_Sections[sectionMap[celf.m_Sections[i]->GetInfoSection()]]);
                break;

            case SHT_RELA:
                section->SetLinkSection(m_Sections[sectionMap[celf.m_Sections[i]->GetLinkSection()]]);
                section->SetInfoSection(m_Sections[sectionMap[celf.m_Sections[i]->GetInfoSection()]]);
                break;

            case SHT_SYMTAB:
                section->SetLinkSection(m_Sections[sectionMap[celf.m_Sections[i]->GetLinkSection()]]);
                break;

            case SHT_DYNSYM:
                section->SetLinkSection(m_Sections[sectionMap[celf.m_Sections[i]->GetLinkSection()]]);
                break;

            case SHT_GROUP:
                section->SetLinkSection(m_Sections[sectionMap[celf.m_Sections[i]->GetLinkSection()]]);
                section->SetInfoSection(m_Sections[sectionMap[celf.m_Sections[i]->GetInfoSection()]]);
                break;

            case SHT_SYMTAB_SHNDX:
                section->SetLinkSection(m_Sections[sectionMap[celf.m_Sections[i]->GetLinkSection()]]);
                assert(m_Sections[sectionMap[celf.m_Sections[i]->GetLinkSection()]]->GetType() == SHT_SYMTAB ||
                       m_Sections[sectionMap[celf.m_Sections[i]->GetLinkSection()]]->GetType() == SHT_DYNSYM);
                // Tell the symbol table where its extended index is.
                dynamic_cast<CElfSymbolTable*>(
                    m_Sections[sectionMap[celf.m_Sections[i]->GetLinkSection()]])->SetExtendedIndexSection(section);
                break;
        }
    }

    // Copy and insert symbols into the symbol table.
    for (CElfSymbolTable::const_SymbolIterator it = celf.m_SymbolTable->SymbolsBegin();
         it != celf.m_SymbolTable->SymbolsEnd();
         ++it)
    {
        std::string   name;
        unsigned char bind;
        unsigned char type;
        unsigned char other;
        CElfSection*  celfSection;
        Elf64_Addr    value;
        Elf64_Xword   size;

        celf.m_SymbolTable->GetInfo(it, &name, &bind, &type, &other, &celfSection, &value, &size);
        m_SymbolTable->AddSymbol(name, bind, type, other, m_Sections[sectionMap[celfSection]], value, size);
    }

    m_Initialized = true;
}


void CElf::Deinitialize()
{
    // Iterate over the sections calling dtors.
    for (size_t i = 0; i < m_Sections.size();  i++)
    {
        delete m_Sections[i];
    }

    m_Sections.clear();
#if PROGRAM_HEADER_SUPPORT

    for (size_t i = 0; i < m_ProgramSegments.size();  i++)
    {
        delete m_ProgramSegments[i];
    }

    m_ProgramSegments.clear();
#endif
    m_SectionHeaderStringTable = NULL;
    m_SymbolTable = NULL;
    m_Initialized = false;
    m_IsElf64 = false;
}
