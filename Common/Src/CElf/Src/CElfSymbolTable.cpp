//=====================================================================
// Copyright 2011 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/Common/Src/CElf/Src/CElfSymbolTable.cpp $
/// \version $Revision: #6 $
/// \brief  A set of classes to allow easy manipulation of ELF
///         binaries in memory.
//
//=====================================================================
// $Id: //devtools/main/Common/Src/CElf/Src/CElfSymbolTable.cpp#6 $
// Last checkin:   $DateTime: 2015/11/02 15:29:15 $
// Last edited by: $Author: callan $
// Change list:    $Change: 546423 $
//=====================================================================

#include "CElf.h"

using namespace std;

CElfSymbolTable::~CElfSymbolTable()
{
}

bool
CElfSymbolTable::Load(const CElf* elf)
{
    assert(m_Header.sh_type == SHT_SYMTAB ||
           m_Header.sh_type == SHT_DYNSYM);
    assert((m_Header.sh_entsize == sizeof(Elf32_Sym) && !elf->m_IsElf64) ||
           (m_Header.sh_entsize == sizeof(Elf64_Sym) && elf->m_IsElf64));

    size_t numEntries                   = (size_t)m_Header.sh_size / (elf->m_IsElf64 ? sizeof(Elf64_Sym) : sizeof(Elf32_Sym));
    CElfStringTable* stringTableSection = dynamic_cast<CElfStringTable*>(GetLinkSection());
    vector<char> stringTableData        = stringTableSection->GetData();
    vector<char> extendedIndexData      = m_ExtendedIndexSection ?
                                          m_ExtendedIndexSection->GetData() : vector<char>();

    m_Table.resize(numEntries);

    for (size_t i = 0; i < numEntries; i++)
    {
        if (elf->m_IsElf64)
        {
            m_Table[i].symEntry = *reinterpret_cast<Elf64_Sym*>(&m_Data[i * sizeof(Elf64_Sym)]);
        }
        else
        {
            Elf32_Sym* stent32 = reinterpret_cast<Elf32_Sym*>(&m_Data[i * sizeof(Elf32_Sym)]);

            m_Table[i].symEntry.st_name  = stent32->st_name;
            m_Table[i].symEntry.st_info  = stent32->st_info;
            m_Table[i].symEntry.st_other = stent32->st_other;
            m_Table[i].symEntry.st_shndx = stent32->st_shndx;
            m_Table[i].symEntry.st_value = stent32->st_value;
            m_Table[i].symEntry.st_size  = stent32->st_size;
        }

        char* name             = &stringTableData[m_Table[i].symEntry.st_name];
        m_Table[i].symName     = string(name);
        size_t sectionIndex    = m_Table[i].symEntry.st_shndx;

        if (sectionIndex == SHN_XINDEX)
        {
            // SHN_XINDEX case not yet debugged
            assert(sectionIndex != SHN_XINDEX);
            sectionIndex = extendedIndexData[size_t(m_ExtendedIndexSection->GetEntrySize() * i)];
        }

        m_Table[i].symSection  = elf->GetSection(sectionIndex);
    }

    return true;
}

CElfSymbolTable::const_SymbolIterator
CElfSymbolTable::SymbolsBegin() const
{
    return m_Table.begin();
}

CElfSymbolTable::SymbolIterator
CElfSymbolTable::SymbolsBegin()
{
    return m_Table.begin();
}

CElfSymbolTable::const_SymbolIterator
CElfSymbolTable::SymbolsEnd() const
{
    return m_Table.end();
}

CElfSymbolTable::SymbolIterator
CElfSymbolTable::SymbolsEnd()
{
    return m_Table.end();
}

size_t
CElfSymbolTable::GetNumSymbols() const
{
    return m_Table.size();
}

bool
CElfSymbolTable::AddSymbol(
    const std::string& name,
    unsigned char      bind,
    unsigned char      type,
    unsigned char      other,
    CElfSection*       section,
    Elf64_Addr         value,
    Elf64_Xword        size)
{
    m_Table.resize(m_Table.size() + 1);
    TableEntry& newSymbol    = m_Table.back();
    newSymbol.symName           = name;
    newSymbol.symEntry.st_info  = ELF32_ST_INFO(bind, type);
    newSymbol.symEntry.st_other = other;
    newSymbol.symSection        = section;
    newSymbol.symEntry.st_value = value;
    newSymbol.symEntry.st_size  = size;
    return true;
}

CElfSymbolTable::const_SymbolIterator
CElfSymbolTable::GetSymbol(
    const std::string& name) const
{
    const_SymbolIterator it = SymbolsBegin();

    while (it != SymbolsEnd() && it->symName != name)
    {
        it++;
    }

    return it;
}

bool
CElfSymbolTable::GetInfo(
    const_SymbolIterator sym,
    std::string*         name,
    unsigned char*       bind,
    unsigned char*       type,
    unsigned char*       other,
    CElfSection**        section,
    Elf64_Addr*          value,
    Elf64_Xword*         size) const
{
    if (name == NULL || bind == NULL || type == NULL || other == NULL ||
        section == NULL || value == NULL || size == NULL)
    {
        return false;
    }

    *name = sym->symName;
    unsigned char info = sym->symEntry.st_info;
    *bind    = ELF32_ST_BIND(info);
    *type    = ELF32_ST_TYPE(info);
    *other   = sym->symEntry.st_other;
    *section = sym->symSection;
    *value   = sym->symEntry.st_value;
    *size    = sym->symEntry.st_size;

    return true;
}

bool
CElfSymbolTable::SetInfo(
    SymbolIterator     sym,
    const std::string& name,
    unsigned char      bind,
    unsigned char      type,
    unsigned char      other,
    CElfSection*       section,
    Elf64_Addr         value,
    Elf64_Xword        size)
{
    if (sym == SymbolsEnd())
    {
        return false;
    }

    sym->symName           = name;
    sym->symEntry.st_info  = ELF64_ST_INFO(bind, type);
    sym->symEntry.st_other = other;
    sym->symSection        = section;
    sym->symEntry.st_value = value;
    sym->symEntry.st_size  = size;
    return true;
}

CElfSymbolTable::SymbolIterator
CElfSymbolTable::RemoveSymbol(
    SymbolIterator sym)
{
    // If we are deleting a STB_LOCAL, adjust the global boundary.
    if (sym < (SymbolsBegin() + m_Header.sh_info))
    {
        m_Header.sh_info--;
    }

    return m_Table.erase(sym);
}

CElfSymbolTable::SymbolIterator
CElfSymbolTable::GetSymbol(
    const std::string& name)
{
    SymbolIterator it = SymbolsBegin();

    while (it != SymbolsEnd() && it->symName != name)
    {
        ++it;
    }

    return it;
}

// This is not implemented.
// New code should use SetInfo with the iterator interface.
//  bool CElfSymbolTable::SetSymbol(
//     const std::string& name,
//     unsigned char      bind,
//     unsigned char      type,
//     unsigned char      other,
//     CElfSection*       section,
//     Elf32_Addr         value,
//     Elf32_Word         size);

bool
CElfSymbolTable::RemoveSymbol(
    const std::string& name)
{
    SymbolIterator it = GetSymbol(name);

    if (it == SymbolsEnd())
    {
        return false;
    }

    RemoveSymbol(it);
    return true;
}

void
CElfSymbolTable::RemoveSymbolsInSection(
    CElfSection* section)
{
    SymbolIterator it = SymbolsBegin();

    while (it != SymbolsEnd())
    {
        if (it->symSection == section)
        {
            it = RemoveSymbol(it);
        }
        else
        {
            ++it;
        }
    }
}

void
CElfSymbolTable::SetExtendedIndexSection(
    CElfSection* section)
{
    m_ExtendedIndexSection = section;
}

CElfSymbolTable::CElfSymbolTable()
    : m_Table(),
      m_ExtendedIndexSection(NULL)
{
    // We do most of the work in the Load method.
    // We need all of the sections to be created before calling that.
}

void
CElfSymbolTable::SetNameIndex(
    SymbolIterator sym,
    Elf32_Word     name)
{
    sym->symEntry.st_name = name;
}

void
CElfSymbolTable::SetSectionIndex(
    SymbolIterator sym,
    size_t         index)
{
    // More code needed here if we wish to support SHN_XINDEX.
    assert(index < SHN_LORESERVE);
    sym->symEntry.st_shndx = static_cast<Elf32_Half>(index);
}

bool
CElfSymbolTable::CopySymbolRecordToData(
    const_SymbolIterator sym,
    bool                 isElf64)
{
    if (isElf64)
    {
        AppendData(reinterpret_cast<const char*>(&sym->symEntry), sizeof(Elf64_Sym));
    }
    else
    {
        FITS32_OR_RETURN_FALSE(sym->symEntry.st_name);
        FITS32_OR_RETURN_FALSE(sym->symEntry.st_value);
        FITS32_OR_RETURN_FALSE(sym->symEntry.st_size);

        Elf32_Sym sym32;
        sym32.st_name  = (Elf32_Word) sym->symEntry.st_name;
        sym32.st_value = (Elf32_Addr) sym->symEntry.st_value;
        sym32.st_size  = (Elf32_Word) sym->symEntry.st_size;
        sym32.st_info  =              sym->symEntry.st_info;
        sym32.st_other =              sym->symEntry.st_other;
        sym32.st_shndx = (Elf32_Half) sym->symEntry.st_shndx;

        AppendData(reinterpret_cast<const char*>(&sym32), sizeof(Elf32_Sym));
    }

    return true;
}
