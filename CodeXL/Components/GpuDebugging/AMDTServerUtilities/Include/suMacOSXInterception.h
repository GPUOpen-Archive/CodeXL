//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suMacOSXInterception.h
///
//==================================================================================
//------------------------------ suMacOSXInterception.h ------------------------------

#ifndef __SUMACOSXINTERCEPTION_H
#define __SUMACOSXINTERCEPTION_H

// Mac OS X
#include <mach-o/dyld.h>

// Infra
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionId.h>

// Local:
#include <GRSpiesUtilities/suSpiesUtilitiesDLLBuild.h>

// Forward Declarations:
class gtString;

// Initializes the Mach-O interception:
bool SU_API suInitializeMacOSXInterception(apMonitoredFunctionId funcId, const gtString& funcName, osProcedureAddress pRealFunction, osProcedureAddress pWrapperFunction, osProcedureAddress& pNewRealFunction);
void SU_API suBeforeInitializingMacOSXInterception();
void SU_API suAfterInitializingMacOSXInterception();

// An assembler JMP command is made of the JMP code (one byte, 0xE9) and a relative address
// pointing how far to jump (which is as large as a pointer in the OS memory address space -
// either 32 bits = 4 bytes. Thus the length of a JMP command in bytes is 1 + (32 / 8) = 5
//
// In 64-bit mode, we use a MOVQ instruction (10 bytes) and a JMPQ ptr instruction (3 bytes),
// totaling in 13 bytes for the entire command.
//
// If changing these values, make sure to make the appropriate adjustments in gsWrappersCommon.h
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
    #ifdef _GR_IPHONE_DEVICE_BUILD
        #define SU_LENGTH_OF_JMP_COMMAND_IN_BYTES 8
    #else
        #define SU_LENGTH_OF_JMP_COMMAND_IN_BYTES 5
    #endif
#elif AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
    #define SU_LENGTH_OF_JMP_COMMAND_IN_BYTES 13
#else
    #error Error unknown address space size!
#endif


// This struct is held for each function for which we replaced the instructions for this
// interception method (see the cpp file for more details)
struct SU_API suFunctionInterceptionInformation
{
    // The original SU_LENGTH_OF_JMP_COMMAND_IN_BYTES of commands in the function.
    // Note that this might be more than one command or part of a larger command,
    // depending on what assembly commands are there (but we cannot assume this is
    // a single instruction).
    gtUByte _originalInstructions[SU_LENGTH_OF_JMP_COMMAND_IN_BYTES];

    // These are the new instruction bytes we write into the memory when initializing
    // the interception (the assembly JMP command). We keep them aside to save the time
    // of calculating the offsets on each function call.
    gtUByte _interceptionInstructions[SU_LENGTH_OF_JMP_COMMAND_IN_BYTES];

    // This flag should ONLY be set during the start / end function wrapper and we use it to check whether
    // we are trying to call SU_BEFORE_EXECUTING_REAL_FUNCTION / SU_AFTER_EXECUTING_REAL_FUNCTION during the
    // function usage. An example for this is that when we break on CGLFlushDrawable in Interactive mode, the
    // following commands are issued:                           in
    // -----------------------------                            --
    // SU_START_FUNCTION_WRAPPER(ap_CGLChoosePixelFormat);      CGLWrappers
    // SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_CGLFlushDrawable)   gsRenderContextMonitor
    // gs_stat_realFunctionPointers.CGLFlushDrawable()          gsRenderContextMonitor
    // SU_AFTER_EXECUTING_REAL_FUNCTION(ap_CGLFlushDrawable)    gsRenderContextMonitor
    // gs_stat_realFunctionPointers.CGLFlushDrawable()          CGLWrappers
    // SU_END_FUNCTION_WRAPPER(ap_CGLChoosePixelFormat);        CGLWrappers
    // In this example, the second realFunctionPointers call would call our function wrapper, which is not good
    bool _isCurrentlyInsideWrapper;

    suFunctionInterceptionInformation();
};

#ifdef _GR_IPHONE_DEVICE_BUILD
// This struct is used as pieces of executable code, which run the first two instructions
// of the OpenGL function, then jump to the continuation of the real function. effectively,
// after interception is initialized, this is the "real" function pointer - as calling it
// runs the real function.
struct SU_API suARMv6InterceptionIsland
{
    suARMv6InterceptionIsland();

    gtUInt32 _instructions[5];
};
#endif


#endif //__SUMACOSXINTERCEPTION_H

