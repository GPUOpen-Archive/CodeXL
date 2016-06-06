//=====================================================================
// Copyright 2011 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/Common/Src/CElf/Src/CElfSection.cpp $
/// \version $Revision: #6 $
/// \brief  A set of classes to allow easy manipulation of ELF
///         binaries in memory.
//
//=====================================================================
// $Id: //devtools/main/Common/Src/CElf/Src/CElfSection.cpp#6 $
// Last checkin:   $DateTime: 2015/11/02 15:29:15 $
// Last edited by: $Author: callan $
// Change list:    $Change: 546423 $
//=====================================================================

#include <cstring>
#include "CElf.h"

using namespace std;

//
// Dtor
//
CElfSection::~CElfSection()
{
}

std::string
CElfSection::GetName() const
{
    return m_Name;
}

void
CElfSection::SetName(
    const std::string& name)
{
    m_Name = name;
}

Elf64_Word
CElfSection::GetType() const
{
    return m_Header.sh_type;
}

void
CElfSection::SetType(Elf64_Word type)
{
    m_Header.sh_type = type;
}

Elf64_Xword
CElfSection::GetFlags() const
{
    return m_Header.sh_flags;
}

void
CElfSection::SetFlags(Elf64_Xword flags)
{
    m_Header.sh_flags = flags;
}

Elf64_Addr
CElfSection::GetAddress() const
{
    return m_Header.sh_addr;
}

void
CElfSection::SetAddress(Elf64_Addr addr)
{
    m_Header.sh_addr = addr;
}

Elf64_Xword
CElfSection::GetAddrAlign() const
{
    return m_Header.sh_addralign;
}

bool
CElfSection::SetAddrAlign(
    Elf64_Xword align)
{
    // Only positive powers of 1 are valid.
    bool check = (align & (align - 1)) == 0;

    if (!check)
    {
        return false;
    }

    m_Header.sh_addralign = align;
    return true;
}

Elf64_Xword
CElfSection::GetEntrySize() const
{
    return m_Header.sh_entsize;
}

void
CElfSection::SetEntrySize(Elf64_Xword size)
{
    m_Header.sh_entsize = size;
}

const CElfSection*
CElfSection::GetLinkSection() const
{
    return m_Link;
}

CElfSection*
CElfSection::GetLinkSection()
{
    return m_Link;
}

bool
CElfSection::SetLinkSection(CElfSection* section)
{
    m_Link = section;
    return true;
}

const CElfSection*
CElfSection::GetInfoSection() const
{
    return m_Info;
}

CElfSection*
CElfSection::GetInfoSection()
{
    return m_Info;
}

bool
CElfSection::SetInfoSection(CElfSection* section)
{
    m_Info = section;
    return true;
}

Elf64_Word
CElfSection::GetInfo() const
{
    return m_Header.sh_info;
}

void
CElfSection::SetInfo(
    Elf64_Word info)
{
    m_Header.sh_info = info;
}

size_t
CElfSection::GetDataSize() const
{
    return m_Data.size();
}

const std::vector<char>&
CElfSection::GetData() const
{
    return m_Data;
}

std::vector<char>&
CElfSection::GetMutableData()
{
    return m_Data;
}

bool
CElfSection::SetData(
    const char* pData,
    size_t size)
{
    m_Data.resize(size);

    if (size != 0)
    {
        memcpy(&m_Data[0], pData, size);
    }

    return true;
}

bool
CElfSection::SetData(
    const std::string& data)
{
    size_t size = data.size();
    m_Data.resize(size);

    if (size != 0)
    {
        memcpy(&m_Data[0], data.data(), size);
    }

    return true;
}

bool
CElfSection::SetData(
    const std::vector<char>& data)
{
    size_t size = data.size();
    m_Data.resize(size);

    if (size != 0)
    {
        memcpy(&m_Data[0], &data[0], size);
    }

    return true;
}

bool
CElfSection::AppendData(
    const char* pData,
    size_t size)
{
    size_t startingSize = m_Data.size();
    m_Data.resize(startingSize + size);

    if (size != 0)
    {
        memcpy(&m_Data[startingSize], pData, size);
    }

    return true;
}

bool
CElfSection::AppendData(
    const std::string& data)
{
    size_t startingSize = m_Data.size();
    size_t copySize     = data.size();
    m_Data.resize(startingSize + copySize);

    if (copySize != 0)
    {
        memcpy(&m_Data[startingSize], data.data(), copySize);
    }

    return true;
}

bool
CElfSection::AppendData(
    std::vector<char>& data)
{
    size_t startingSize = m_Data.size();
    size_t copySize     = data.size();
    m_Data.resize(startingSize + copySize);

    if (copySize != 0)
    {
        memcpy(&m_Data[startingSize], &data[0], copySize);
    }

    return true;
}

void
CElfSection::EraseData()
{
    m_Data.resize(0);
}

CElfSection::CElfSection()
    : m_Header(),
      m_Link(NULL),
      m_Info(NULL),
      m_Name(),
      m_Data()
{
}

CElfSection::CElfSection(
    const CElfSection& other)
    : m_Header(other.m_Header),
      m_Link(other.m_Link),
      m_Info(other.m_Info),
      m_Name(other.m_Name),
      m_Data(other.m_Data)
{
}

CElfSection&
CElfSection::operator=(
    CElfSection const& other)
{
    m_Header = other.m_Header;
    m_Link   = other.m_Link;
    m_Info   = other.m_Info;
    m_Name   = other.m_Name;
    m_Data   = other.m_Data;
    return *this;
}

bool
CElfSection::Load(
    const std::vector<char>& vBinary,
    std::size_t shdrOff,
    bool is64)
{
    // Read the section header (in the simplest way possible).
    // More code is needed to deal with cross endian & ELF64 objects.
    size_t fileSize = vBinary.size();

    if (shdrOff + sizeof(Elf32_Shdr) > fileSize)
    {
        return false;
    }

    if (is64)
    {
        m_Header = *reinterpret_cast<const Elf64_Shdr*>(&vBinary[shdrOff]);
    }
    else
    {
        const Elf32_Shdr* shdr32 = reinterpret_cast<const Elf32_Shdr*>(&vBinary[shdrOff]);

        m_Header.sh_name      = shdr32->sh_name;
        m_Header.sh_type      = shdr32->sh_type;
        m_Header.sh_flags     = shdr32->sh_flags;
        m_Header.sh_addr      = shdr32->sh_addr;
        m_Header.sh_offset    = shdr32->sh_offset;
        m_Header.sh_size      = shdr32->sh_size;
        m_Header.sh_link      = shdr32->sh_link;
        m_Header.sh_info      = shdr32->sh_info;
        m_Header.sh_addralign = shdr32->sh_addralign;
        m_Header.sh_entsize   = shdr32->sh_entsize;
    }

    if (m_Header.sh_offset + m_Header.sh_size > fileSize)
    {
        return false;
    }

    // Copy the data
    m_Data.resize((size_t)m_Header.sh_size);

    if (m_Header.sh_size != 0)
    {
        memcpy(&m_Data[0], &vBinary[(size_t)m_Header.sh_offset], (size_t)m_Header.sh_size);
    }

    return true;
}

Elf64_Word
CElfSection::GetLink() const
{
    return m_Header.sh_link;
}

void
CElfSection::SetLink(
    Elf64_Word link)
{
    m_Header.sh_link = link;
}

Elf64_Off
CElfSection::GetDataOffset() const
{
    return m_Header.sh_offset;
}

void
CElfSection::SetDataOffset(
    Elf64_Off offset)
{
    m_Header.sh_offset = offset;
}

void
CElfSection::SetDataSize(
    Elf64_Xword size)
{
    m_Header.sh_size = size;
}

const Elf64_Shdr&
CElfSection::GetShdr() const
{
    return m_Header;
}
