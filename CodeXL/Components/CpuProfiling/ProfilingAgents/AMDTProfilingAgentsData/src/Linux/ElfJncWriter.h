//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ElfJncWriter.h
/// \brief This file contains an interface to write JIT Native Code file in ELF format.
///
//==================================================================================

#ifndef _ELFJNCWRITER_H_
#define _ELFJNCWRITER_H_

#include "gelf.h"
#include <map>
#include <vector>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

class ElfJncWriter
{
public:
    ElfJncWriter();

    ~ElfJncWriter();

    int init(const char* jncFile, const char* pJittedCodeSymbolName, const void* pJittedCodeAddr, unsigned int jittedCodeSize);

    int addSection(const char* name, const void* data, unsigned int size, gtUInt64 vma, bool isProgbits);

    int write();

private:
    bool ElfCreate(const char*  pFileName);
    int ElfAddCodeSection(const char* name, const void* pData, unsigned int size, gtUInt64 vaddr, bool isProgbits);
    int ElfAddStringSection();
    int ElfUpdate();

    int          m_fd;
    Elf*         m_pElf;
    Elf_Scn*     m_pScn;
    Elf_Data*    m_pData;
    GElf_Ehdr*   m_pEhdr;
    Elf64_Shdr*  m_pShdr;

    char m_strtab[260];
    int m_strtabOffset;

    std::map<gtUInt64, unsigned int>  regit_map;
};

#endif // _ELFJNCWRITER_H_
