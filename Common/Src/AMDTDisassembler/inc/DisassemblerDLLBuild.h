//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file DisassemblerDLLBuild.h
///
//==================================================================================

//------------------------------ DisassemblerDLLBuild.h ------------------------------

#ifndef _DISASSEMBLERDLLBUILD_H_
#define _DISASSEMBLERDLLBUILD_H_

// Under Win32 builds - define: DASM_API to be:
// - When building AMDTDisassembler.dll: __declspec(dllexport).
// - When building other projects:       __declspec(dllimport).

#if defined(_WIN32)
    #if defined(AMDT_DISASSEMBLER_EXPORTS)
        #define DASM_API __declspec(dllexport)
    #else
        #define DASM_API __declspec(dllimport)
    #endif
#else
    #define DASM_API
#endif


#endif  // _DISASSEMBLERDLLBUILD_H_
