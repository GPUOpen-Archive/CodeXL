//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ElfJncWriter.cpp
/// \brief This file contains an interface to write JIT Native Code file in ELF format.
///
//==================================================================================

// System Headers
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ElfJncWriter.h"

ElfJncWriter::ElfJncWriter()
{
    m_fd    = -1;
    m_pElf  = NULL;
    m_pScn  = NULL;
    m_pData = NULL;
    m_pEhdr = NULL;
    m_pShdr = NULL;

    memset(m_strtab, 0, 260);
    m_strtabOffset = 1;
}


ElfJncWriter::~ElfJncWriter()
{
    if (NULL != m_pElf)
    {
        ElfUpdate();
    }
}

bool ElfJncWriter::ElfCreate(const char* pFileName)
{
    // initialize libelf
    if (EV_NONE == elf_version(EV_CURRENT))
    {
        return false;
    }

    m_fd = open(pFileName, O_WRONLY | O_CREAT, 0777);

    if (m_fd < 0)
    {
        return false;
    }

    m_pElf = elf_begin(m_fd, ELF_C_WRITE, NULL);

    if (NULL == m_pElf)
    {
        return false;
    }

    if (ELF_K_ELF != elf_kind(m_pElf))
    {
        return false;
    }

    // Write the Executable HEader
    m_pEhdr = (GElf_Ehdr*)gelf_newehdr(m_pElf, ELFCLASS64);

    if (NULL == m_pEhdr)
    {
        return false;
    }

    m_pEhdr->e_ident[EI_VERSION] = EV_CURRENT;
    m_pEhdr->e_ident[EI_CLASS]   = ELFCLASS64; // 64-bit arch
    m_pEhdr->e_ident[EI_DATA]    = ELFDATA2LSB; // 2's complement; little endian
    m_pEhdr->e_ident[EI_OSABI]   = ELFOSABI_NONE;

    m_pEhdr->e_machine           = EM_X86_64; // AMD 64-bit
    m_pEhdr->e_type              = ET_REL;    // relocatable object

    // set ".shstrtab" in shrstrtab
    char tmp[] = ".shstrtab";
    strncpy(&m_strtab[m_strtabOffset], tmp, strlen(tmp));
    m_strtabOffset += strlen(tmp);
    m_strtab[m_strtabOffset] = 0;
    m_strtabOffset ++;

    return true;
}


int ElfJncWriter::ElfAddCodeSection(const char* name, const void* pData, unsigned int size, gtUInt64 vaddr, bool isProgbits)
{
    // Create a new section and appends it to the
    // list of sections associated with ELF descriptor;
    m_pScn = elf_newscn(m_pElf);

    if (NULL == m_pScn)
    {
        return -1;
    }

    m_pData = elf_newdata(m_pScn);

    if (NULL == m_pData)
    {
        return -1;
    }

    m_pData->d_align   = 1;
    m_pData->d_buf     = (void*)pData;
    m_pData->d_off     = 0LL;
    m_pData->d_size    = size;
    m_pData->d_type    = ELF_T_BYTE;
    m_pData->d_version = EV_CURRENT;

    // Create a section header for this section
    m_pShdr = elf64_getshdr(m_pScn);

    if (NULL == m_pShdr)
    {
        return -1;
    }

    m_pShdr->sh_name = m_strtabOffset;
    m_pShdr->sh_type = SHT_PROGBITS;
    m_pShdr->sh_entsize = 0; // size of each entry in section

    if (true == isProgbits)
    {
        m_pShdr->sh_flags = SHF_WRITE | SHF_ALLOC | SHF_EXECINSTR;
        m_pShdr->sh_addr  = vaddr; // address in memory image
        m_pEhdr->e_entry  = vaddr;
    }
    else
    {
        m_pShdr->sh_flags = 0;
        m_pShdr->sh_addr  = 0;
    }

    // m_pShdr->sh_offset    = 0; // offset in file
    // m_pShdr->sh_link      = 0; // index of a related section ?
    // m_pShdr->sh_info      = 0;
    // m_pShdr->sh_addralign = 16; // alignment in bytes

    // copy the section name to strtab
    strncpy(&m_strtab[m_strtabOffset], name, strlen(name));
    m_strtabOffset += strlen(name);
    m_strtab[m_strtabOffset] = 0;
    m_strtabOffset ++;

    return 0;
}


int  ElfJncWriter::ElfAddStringSection()
{
    // Create a new section and appends it to the
    // list of sections associated with ELF descriptor;
    m_pScn = elf_newscn(m_pElf);

    if (NULL == m_pScn)
    {
        return -1;
    }

    m_pData = elf_newdata(m_pScn);

    if (NULL == m_pData)
    {
        return -1;
    }

    m_pData->d_align   = 1;
    m_pData->d_buf     = m_strtab;
    m_pData->d_off     = 0LL;
    m_pData->d_size    = m_strtabOffset;
    m_pData->d_type    = ELF_T_BYTE;
    m_pData->d_version = EV_CURRENT;

    // Create a section header for this section
    m_pShdr = elf64_getshdr(m_pScn);

    if (NULL == m_pShdr)
    {
        return -1;
    }

    m_pShdr->sh_name    = 1; // the section name was copied alread in string table
    m_pShdr->sh_type    = SHT_STRTAB;
    m_pShdr->sh_flags   = 0;
    m_pShdr->sh_entsize = 0;

    // Set this section as string table
    elf_setshstrndx(m_pElf, elf_ndxscn(m_pScn));

    return 0;
}

int ElfJncWriter::ElfUpdate()
{
    int ret;

    ret = elf_update(m_pElf, ELF_C_WRITE);

    if (ret < 0)
    {
        return ret;
    }

    (void) elf_end(m_pElf);
    (void) close(m_fd);

    m_pElf = NULL;
    m_fd = -1;

    return 0;
}


int ElfJncWriter::init(const char* jncFile, const char* pJittedCodeSymbolName, const void* pJittedCodeAddr, unsigned int jittedCodeSize)
{
    (void)(pJittedCodeSymbolName); // unused
    int  ret = -1;
    bool rv  = false;

    if (regit_map.end() == regit_map.find((gtUInt64)pJittedCodeAddr))
    {
        regit_map[(gtUInt64)pJittedCodeAddr] = 0;
    }

    rv = ElfCreate(jncFile);

    if (false == rv)
    {
        return -1;
    }

    // add the code section
    ret = addSection((const char*)".text",
                     pJittedCodeAddr,
                     jittedCodeSize,
                     (gtUInt64)pJittedCodeAddr,
                     true);

    ++regit_map[(gtUInt64)pJittedCodeAddr];

    return ret;
}


int ElfJncWriter::addSection(const char* name, const void* pData, unsigned int size, gtUInt64 vma, bool isProgbits)
{
    if ((NULL == name) || (NULL == pData) || (size == 0))
    {
        return -1;
    }

    int rv = ElfAddCodeSection(name, pData, size, vma, isProgbits);

    if (rv != 0)
    {
        return -1;
    }

    return 0;
}


int ElfJncWriter::write()
{
    // Add the string table
    int rv = ElfAddStringSection();

    if (rv != 0)
    {
        return -1;
    }

    rv = ElfUpdate();

    if (rv != 0)
    {
        return -1;
    }

    return 0;
}
