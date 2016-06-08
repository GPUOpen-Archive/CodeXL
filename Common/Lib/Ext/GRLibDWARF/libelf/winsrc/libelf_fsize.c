/*-
 * Copyright (c) 2006,2008-2011 Joseph Koshy
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS `AS IS' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <libelf.h>

#include "_libelf.h"

ELFTC_VCSID("$Id: libelf_fsize.m4 2225 2011-11-26 18:55:54Z jkoshy $");

/* WARNING: GENERATED FROM stdin. */

/*
 * Create an array of file sizes from the elf_type definitions
 */



struct fsize {
	size_t fsz32;
	size_t fsz64;
};

#ifdef _MSC_VER
#define MEMBER_ASSIGNMENT(expr)
#else // defined (_MSC_VER)
#define MEMBER_ASSIGNMENT(expr) expr = 
#endif

static struct fsize fsize[ELF_T_NUM] = {

MEMBER_ASSIGNMENT([ELF_T_ADDR]) { MEMBER_ASSIGNMENT(.fsz32) sizeof(Elf32_Addr), MEMBER_ASSIGNMENT(.fsz64) sizeof(Elf64_Addr) },

MEMBER_ASSIGNMENT([ELF_T_BYTE]) { MEMBER_ASSIGNMENT(.fsz32) 1, MEMBER_ASSIGNMENT(.fsz64) 1 },

MEMBER_ASSIGNMENT([ELF_T_CAP]) { MEMBER_ASSIGNMENT(.fsz32) sizeof(Elf32_Word)+sizeof(Elf32_Word)+0, MEMBER_ASSIGNMENT(.fsz64) sizeof(Elf64_Xword)+sizeof(Elf64_Xword)+0 },

MEMBER_ASSIGNMENT([ELF_T_DYN]) { MEMBER_ASSIGNMENT(.fsz32) sizeof(Elf32_Sword)+sizeof(Elf32_Word)+0, MEMBER_ASSIGNMENT(.fsz64) sizeof(Elf64_Sxword)+sizeof(Elf64_Xword)+0 },

MEMBER_ASSIGNMENT([ELF_T_EHDR]) { MEMBER_ASSIGNMENT(.fsz32) EI_NIDENT+sizeof(Elf32_Half)+sizeof(Elf32_Half)+sizeof(Elf32_Word)+sizeof(Elf32_Addr)+sizeof(Elf32_Off)+sizeof(Elf32_Off)+sizeof(Elf32_Word)+sizeof(Elf32_Half)+sizeof(Elf32_Half)+sizeof(Elf32_Half)+sizeof(Elf32_Half)+sizeof(Elf32_Half)+sizeof(Elf32_Half)+0, MEMBER_ASSIGNMENT(.fsz64) EI_NIDENT+sizeof(Elf64_Half)+sizeof(Elf64_Half)+sizeof(Elf64_Word)+sizeof(Elf64_Addr)+sizeof(Elf64_Off)+sizeof(Elf64_Off)+sizeof(Elf64_Word)+sizeof(Elf64_Half)+sizeof(Elf64_Half)+sizeof(Elf64_Half)+sizeof(Elf64_Half)+sizeof(Elf64_Half)+sizeof(Elf64_Half)+0 },

MEMBER_ASSIGNMENT([ELF_T_HALF]) { MEMBER_ASSIGNMENT(.fsz32) sizeof(Elf32_Half), MEMBER_ASSIGNMENT(.fsz64) sizeof(Elf64_Half) },

MEMBER_ASSIGNMENT([ELF_T_LWORD]) { MEMBER_ASSIGNMENT(.fsz32) sizeof(Elf32_Lword), MEMBER_ASSIGNMENT(.fsz64) sizeof(Elf64_Lword) },

MEMBER_ASSIGNMENT([ELF_T_MOVE]) { MEMBER_ASSIGNMENT(.fsz32) sizeof(Elf32_Lword)+sizeof(Elf32_Word)+sizeof(Elf32_Word)+sizeof(Elf32_Half)+sizeof(Elf32_Half)+0, MEMBER_ASSIGNMENT(.fsz64) sizeof(Elf64_Lword)+sizeof(Elf64_Xword)+sizeof(Elf64_Xword)+sizeof(Elf64_Half)+sizeof(Elf64_Half)+0 },

MEMBER_ASSIGNMENT([ELF_T_MOVEP]) { MEMBER_ASSIGNMENT(.fsz32) 0, MEMBER_ASSIGNMENT(.fsz64) 0 },

MEMBER_ASSIGNMENT([ELF_T_NOTE]) { MEMBER_ASSIGNMENT(.fsz32) 1, MEMBER_ASSIGNMENT(.fsz64) 1 },

MEMBER_ASSIGNMENT([ELF_T_OFF]) { MEMBER_ASSIGNMENT(.fsz32) sizeof(Elf32_Off), MEMBER_ASSIGNMENT(.fsz64) sizeof(Elf64_Off) },

MEMBER_ASSIGNMENT([ELF_T_PHDR]) { MEMBER_ASSIGNMENT(.fsz32) sizeof(Elf32_Word)+sizeof(Elf32_Off)+sizeof(Elf32_Addr)+sizeof(Elf32_Addr)+sizeof(Elf32_Word)+sizeof(Elf32_Word)+sizeof(Elf32_Word)+sizeof(Elf32_Word)+0, MEMBER_ASSIGNMENT(.fsz64) sizeof(Elf64_Word)+sizeof(Elf64_Word)+sizeof(Elf64_Off)+sizeof(Elf64_Addr)+sizeof(Elf64_Addr)+sizeof(Elf64_Xword)+sizeof(Elf64_Xword)+sizeof(Elf64_Xword)+0 },

MEMBER_ASSIGNMENT([ELF_T_REL]) { MEMBER_ASSIGNMENT(.fsz32) sizeof(Elf32_Addr)+sizeof(Elf32_Word)+0, MEMBER_ASSIGNMENT(.fsz64) sizeof(Elf64_Addr)+sizeof(Elf64_Xword)+0 },

MEMBER_ASSIGNMENT([ELF_T_RELA]) { MEMBER_ASSIGNMENT(.fsz32) sizeof(Elf32_Addr)+sizeof(Elf32_Word)+sizeof(Elf32_Sword)+0, MEMBER_ASSIGNMENT(.fsz64) sizeof(Elf64_Addr)+sizeof(Elf64_Xword)+sizeof(Elf64_Sxword)+0 },

MEMBER_ASSIGNMENT([ELF_T_SHDR]) { MEMBER_ASSIGNMENT(.fsz32) sizeof(Elf32_Word)+sizeof(Elf32_Word)+sizeof(Elf32_Word)+sizeof(Elf32_Addr)+sizeof(Elf32_Off)+sizeof(Elf32_Word)+sizeof(Elf32_Word)+sizeof(Elf32_Word)+sizeof(Elf32_Word)+sizeof(Elf32_Word)+0, MEMBER_ASSIGNMENT(.fsz64) sizeof(Elf64_Word)+sizeof(Elf64_Word)+sizeof(Elf64_Xword)+sizeof(Elf64_Addr)+sizeof(Elf64_Off)+sizeof(Elf64_Xword)+sizeof(Elf64_Word)+sizeof(Elf64_Word)+sizeof(Elf64_Xword)+sizeof(Elf64_Xword)+0 },

MEMBER_ASSIGNMENT([ELF_T_SWORD]) { MEMBER_ASSIGNMENT(.fsz32) sizeof(Elf32_Sword), MEMBER_ASSIGNMENT(.fsz64) sizeof(Elf64_Sword) },

MEMBER_ASSIGNMENT([ELF_T_SXWORD]) { MEMBER_ASSIGNMENT(.fsz32) 0, MEMBER_ASSIGNMENT(.fsz64) sizeof(Elf64_Sxword) },

MEMBER_ASSIGNMENT([ELF_T_SYMINFO]) { MEMBER_ASSIGNMENT(.fsz32) sizeof(Elf32_Half)+sizeof(Elf32_Half)+0, MEMBER_ASSIGNMENT(.fsz64) sizeof(Elf64_Half)+sizeof(Elf64_Half)+0 },

MEMBER_ASSIGNMENT([ELF_T_SYM]) { MEMBER_ASSIGNMENT(.fsz32) sizeof(Elf32_Word)+sizeof(Elf32_Addr)+sizeof(Elf32_Word)+1+1+sizeof(Elf32_Half)+0, MEMBER_ASSIGNMENT(.fsz64) sizeof(Elf64_Word)+1+1+sizeof(Elf64_Half)+sizeof(Elf64_Addr)+sizeof(Elf64_Xword)+0 },

MEMBER_ASSIGNMENT([ELF_T_VDEF]) { MEMBER_ASSIGNMENT(.fsz32) 1, MEMBER_ASSIGNMENT(.fsz64) 1 },

MEMBER_ASSIGNMENT([ELF_T_VNEED]) { MEMBER_ASSIGNMENT(.fsz32) 1, MEMBER_ASSIGNMENT(.fsz64) 1 },

MEMBER_ASSIGNMENT([ELF_T_WORD]) { MEMBER_ASSIGNMENT(.fsz32) sizeof(Elf32_Word), MEMBER_ASSIGNMENT(.fsz64) sizeof(Elf64_Word) },

MEMBER_ASSIGNMENT([ELF_T_XWORD]) { MEMBER_ASSIGNMENT(.fsz32) 0, MEMBER_ASSIGNMENT(.fsz64) sizeof(Elf64_Xword) },

MEMBER_ASSIGNMENT([ELF_T_GNUHASH]) { MEMBER_ASSIGNMENT(.fsz32) 1, MEMBER_ASSIGNMENT(.fsz64) 1 },


/* "Magic numbers" version for windows, this should be used as a fallback if something goes wrong: */
/*
{4, 8}, {1, 1}, {0, 0}, {8, 16}, {52, 64},
{2, 2}, {0, 0}, {0, 0}, {0, 0}, {1, 1},
{4, 8}, {32, 56}, {8, 16}, {12, 24}, {40, 64},
{4, 4}, {0, 8}, {0, 0}, {16, 24}, {20, 20},
{16, 16}, {4, 4}, {0, 8}, {1, 1}
*/


};

size_t
_libelf_fsize(Elf_Type t, int ec, unsigned int v, size_t c)
{
	size_t sz;

	sz = 0;
	if (v != EV_CURRENT)
		LIBELF_SET_ERROR(VERSION, 0);
	else if ((int) t < ELF_T_FIRST || t > ELF_T_LAST)
		LIBELF_SET_ERROR(ARGUMENT, 0);
	else {
		sz = ec == ELFCLASS64 ? fsize[t].fsz64 : fsize[t].fsz32;
		if (sz == 0)
			LIBELF_SET_ERROR(UNIMPL, 0);
	}

	return (sz*c);
}
