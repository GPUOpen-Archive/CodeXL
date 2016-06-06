//=====================================================================
// Copyright 2012-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file 
/// 
//=====================================================================
#ifndef _API_GPUISA_H_
#define _API_GPUISA_H_
#include "v0_7/api_defs.h"
#include "v0_7/clTypes.h"
#include "v0_7/clCompiler.h"
#ifdef __cplusplus
extern "C" {
#endif
//! The function oclisaDisassembleISA disassembles the ISA for the specific kernel and inserts it back into the oclElf file. How this exists in the oclElf binary is specified in the BIF doc. If a callback is specified, what is inserted into the oclElf file is also passed into the callback. If the kernel does not exist or does not have ISA, whether recompilation occurs from a higher level to ISA is implementation defined. If ISA disassembling succeeds, then the results are stored in the oclElf and the results are passed to the disasm_callback, otherwise nothing occurs.
void AOC_INTERFACE 
oclisaDisassemble(
    oclCompiler* cl, 
    oclElf* bin, 
    const char* kernel, 
    oclLogFunction disasm_callback
    ) AOC_API_0_7;

//! The function oclisaGetBinaryBlob is used to get the ISA Binary Blob that
// needs to be passed down to the hardware in order to execute the kernel.
// How this exists in the oclElf binary is specified in the BIF document. The
// binary blob is defined by SC and will be a version of the SC_HW_SHADER 
// based on the generation and what the input language is.
// When compiled from OpenCL source, the following shader types will be 
// returned:
// R6XX devices will return a SC_R600PSHWSHADER.(If this path is supported)
// R7XX devices will return a SC_R600CSHWSHADER.(This needs to be verified)
// Evergreen and NI devices will return a SC_R800CSHWSHADER.
// SI devices will return a SC_SI_HWSHADER_CS.
// These returning binaries can be modified directly with SC APIs.
const void* AOC_INTERFACE 
oclisaGetBinaryBlob(
    oclCompiler* cl,
    const oclElf* bin, 
    const char* kernel, 
    size_t* size
    ) AOC_API_0_7;
#ifdef __cplusplus
}
#endif
#endif // _API_GPUISA_H_
