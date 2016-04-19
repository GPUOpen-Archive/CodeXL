//------------------------------ suMacOSXInterception.cpp ------------------------------

//////////////////////////////////////////////////////////////////////////
// Mac OS X Interception method explained:
// ======================================
// Initialization:
// --------------
//  1. Obtain the real and wrapper addresses of the OpenGL function we
//      want to intercept.
//  2. Calculate the offset between the real function address + 5 bytes
//      and the wrapper function address.
//  2'. Create from that offset an assembler JMP command (which is 5 bytes
//      long on 32-bit OSes, and note it aside for later use.
//  3. Make the virtual memory page on which the real function resides
//      writable (if this fails, we leave the function and don't start up the
//      debugged process, as the spy process doesn't have appropriate privileges.
//  4. Note aside the first 5 bytes of instructions in the real function
//  5. Replace the first five bytes of the real function with the JMP command
//      we assembled.
//
//  Example:
//      1. assume glBegin is at address 0x12345678 and our wrapper for it is at 0x99999999.
//      2. The above difference is 09ABCDEF - (0x12345678 + 5) = 0x87654321 - 5 = 0x8765431C
//      2'. The instruction would be E9 1C 43 65 87 (Intel-based Macs are big endian), this
//          is noted in su_stat_functionInterceptionInfo[ap_glBegin]._interceptionInstructions
//      3. obvious
//      4. assume the original instructions are 12 34 56 78 9A, this is noted in
//          su_stat_functionInterceptionInfo[ap_glBegin]._originalInstructions
//      5. obvious
//
// 64-bit
// ------
// The process is similar, but with a few differences:
// 2. We use the absolute address instead of the offset
// 2'. We use an indirect absolute near jump (FF /4) instead of a standard (direct relative near) JMP
// 2''. To use the indirect jump with a 64-bit address, we need to feed it to a register first
//      (with a MOV instruction) then use the register to jump.
// 4-5. The above operation uses 10 bytes for the MOV instruction and 3 for the JMP instruction
//      so we replace 13 bytes instead of 5.
//
// Runtime:
// -------
//  1. The debugged application calls the real function
//  2. The first instruction in the function is a JMP command directing it to our wrapper
//  3. The wrapper restores the real 5 bytes of instruction (note that instructions vary in
//      length so these 5 bytes may contain more than one instruction or fragments of an instruction)
//  4. The wrapper calls the real function. The real instructions are restored in step 3, so the actual
//      real function's code is run.
//  5. The wrapper function restores the JMP instruction to the beginning of the real function
//  6. The wrapper function returns (to the debugged application, it seems as if it were the real function)
//
//  See also http://www.intel.com/products/processor/manuals/.
//////////////////////////////////////////////////////////////////////////

// OS X
#include <mach/vm_map.h>

// Infra
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local
#include <AMDTServerUtilities/Include/suGlobalVariables.h>
#include <AMDTServerUtilities/Include/suMacOSXInterception.h>
#include <AMDTServerUtilities/Include/suStringConstants.h>

// 32-bit instructions:
#define SU_I386_JMP_OPCODE 0xE9

// Armv6-specific instructions:
// See ARM assembly reference (http://www.arm.com/miscPDFs/14128.pdf),
// Sections A4.1.23 (LDR) and A5-2 (Addressing Mode 2 - Load and Store Word or Unsigned Byte)
// in pages A4-43 (193) and A5-18 (458):                // Possible range (which bits are relevant)
#define SU_ARMV6_AL_CONDITION 0xE0                      // & 0xF0
#define SU_ARMV6_LDR_OPCODE 0x05                        // & 0x0F
#define SU_ARMV6_LDR_SIGN_WIDTH_AND_DIRECTION_MODS 0x10 // & 0xF0
#define SU_ARMV6_LDR_BASE_REGISTER 0x0F                 // & 0x0F
#define SU_ARMV6_LDR_DEST_REGISTER 0xF0                 // & 0xF0
#define SU_ARMV6_LDR_REGISTER_OFFSET1 0x00              // & 0x0F
#define SU_ARMV6_LDR_REGISTER_OFFSET2 0x04              // & 0xFF

// Arm Thumb-2 Specific instructions
// See ARM and Thumb-2 instruction set quick refernce http://infocenter.arm.com/help/topic/com.arm.doc.qrc0001l/QRC0001_UAL.pdf
//  + ARM LDR and STR http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0204i/CIHGJHED.html
//  + Thumb-2 compiler discussion and output at http://sourceware.org/ml/binutils/2009-01/msg00360.html and http://sourceware.org/ml/binutils/2005-08/msg00389.html
//  + Experimaent with Linux and ARM Thumb-2 ISA http://www.linuxfordevices.com/files/article078/Experiment_with_Linux_and_ARM_Thumb-2_ISA.pdf (emph. pg. 8)
#define SU_ARM_THUMB2_LDR_OPCODE 0xF8                   // & 0xFE
#define SU_ARM_THUMB2_LDR_SIGN_EXTENSION_MODIFIER 0x00  // & 0x01
#define SU_ARM_THUMB2_LDR_OFFSET_SIGN_MODIFIER 0x80     // & 0x80 - (8 = +, 0 = -)
#define SU_ARM_THUMB2_LDR_SIZE_MODIFIER 0x40            // & 0x60 - Should not be 0x60 (4 = W, 2 = HW, 0 = B)
#define SU_ARM_THUMB2_LDR_STR_MODIFIER 0x10             // & 0x10 - (1 = LDR, 0 = STR)
#define SU_ARM_THUMB2_LDR_BASE_REGISTER 0x0F            // & 0x0F - Not sure if this is the function
#define SU_ARM_THUMB2_LDR_DEST_REGISTER 0xF0            // & 0xF0
#define SU_ARM_THUMB2_LDR_REGISTER_OFFSET1 0x00         // & 0x0F
#define SU_ARM_THUMB2_LDR_REGISTER_OFFSET2 0x00         // & 0xFF

#ifdef _GR_IPHONE_DEVICE_BUILD
// SU_ARMV6_LDR_PC_PC_MINUS_4_INSTRUCTION = 0xE51FF004
#define SU_ARMV6_LDR_PC_PC_MINUS_4_INSTRUCTION (((SU_ARMV6_AL_CONDITION | SU_ARMV6_LDR_OPCODE) << 24) | ((SU_ARMV6_LDR_SIGN_WIDTH_AND_DIRECTION_MODS | SU_ARMV6_LDR_BASE_REGISTER) << 16) | ((SU_ARMV6_LDR_DEST_REGISTER | SU_ARMV6_LDR_REGISTER_OFFSET1) << 8) | (SU_ARMV6_LDR_REGISTER_OFFSET2))
// SU_ARM_THUMB2_LDR_PC_PC_PLUS_0_INSTRUCTION = 0xF8DF 0xF000 = 0xF000F8DF
#define SU_ARM_THUMB2_LDR_PC_PC_PLUS_0_INSTRUCTION (((SU_ARM_THUMB2_LDR_OPCODE | SU_ARM_THUMB2_LDR_SIGN_EXTENSION_MODIFIER) << 8) | (SU_ARM_THUMB2_LDR_OFFSET_SIGN_MODIFIER | SU_ARM_THUMB2_LDR_SIZE_MODIFIER | SU_ARM_THUMB2_LDR_STR_MODIFIER | SU_ARM_THUMB2_LDR_BASE_REGISTER) | ((SU_ARM_THUMB2_LDR_DEST_REGISTER | SU_ARM_THUMB2_LDR_REGISTER_OFFSET1) << 24) | (SU_ARM_THUMB2_LDR_REGISTER_OFFSET2 << 16))
// This is actually MOV r8, r8 - which has no effect. See http://sourceware.org/ml/binutils/2007-06/msg00038.html.
#define SU_ARM_THUMB2_NOP_INSTRUCTION 0x46C0
// Used in the interception of -[EAGLContext presentRenderbuffer:] on iPhone OS 3.2:
#define SU_ARM_THUMB2_LDR_R3_PC_OFF_INSTRUCTION 0x4B00  // Set the lowest 8 bits to a value, which is later multiplied by 4 to determine the offset.
#define SU_ARM_THUMB2_ADD_R3_PC_INSTRUCTION 0x447B
// The original and replacement code in -[EAGLContext presentRenderbuffer:]. See gsFixiPhoneOS3_2InterceptionOfEAGLContext_presentRenderbuffer for more details:
#define SU_IPHONE_OS_3_2_PRESENT_RENDERBUFFER_ORIGINAL_INSTRUCTIONS_3_AND_4 ((SU_ARM_THUMB2_ADD_R3_PC_INSTRUCTION << 16) | (SU_ARM_THUMB2_LDR_R3_PC_OFF_INSTRUCTION | 0x11))    // LDR r3, [pc, #68 (= 0x11 * 4)] ; ADD r3, pc
#define SU_IPHONE_OS_3_2_PRESENT_RENDERBUFFER_INTERCEPTION_INSTRUCTIONS_3_AND_4 ((SU_ARM_THUMB2_NOP_INSTRUCTION << 16) | (SU_ARM_THUMB2_LDR_R3_PC_OFF_INSTRUCTION | 0x02))      // LDR r3, [pc, #8 (= 0x02 * 4) ; NOP

suARMv6InterceptionIsland::suARMv6InterceptionIsland()
{
    _instructions[0] = 0;
    _instructions[1] = 0;
    _instructions[2] = 0;
    _instructions[3] = 0;
    _instructions[4] = (SU_ARM_THUMB2_NOP_INSTRUCTION << 16) | SU_ARM_THUMB2_NOP_INSTRUCTION;
}

// See comment related to -[EAGLContext initWithAPI:] below
#define SU_LARGER_INTERCEPTION_ISLAND_SIZE 32

#endif

// 64-bit instructions:
#define SU_X86_64_REX_W_PREFIX_BYTE 0x48
#define SU_X86_64_REX_B_PREFIX_BYTE 0x41
#define SU_X86_64_MOV_OPCODE 0xB8
#define SU_X86_64_ABS_JMP_OPCODE 0xFF
#define SU_X86_64_ABS_JMP_TO_REGISTER_VALUE_MOD_RM_BYTE 0xE0
// These following two items decide which register to use to store the pointer.
// The first few registers might be overwritten by the application, so it is
// best to use one of the seldom used ones.
// In addition, there is a slight complication to using RBP and R13, please confer
// the Intel manual (linked above) before attempting to use them.
//
// Important note:
// --------------
// According to the table on page 21 of http://www.x86-64.org/documentation/abi.pdf
// AMD64 ABI manual, the scratch register in AMD64 calling convention is R11.
// Specificly, it is problematic to use RDI, RSI, RDX, RCX, R8, R9 (all used to pass
// function arguments) and RSP (used to store the stack pointer). Apparently,
// Objective-C also uses R12-R15 (which are callee-reserved registers), so
// overwriting them can also cause crashes.
// +------------------+---+---+---+---+---+---+---+---+
// |Register Number   | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
// +------------------+---+---+---+---+---+---+---+---+
// |Normal Register   |RAX|RCX|RDX|RBX|RSP|RBP|RSI|RDI|
// +------------------+---+---+---+---+---+---+---+---+
// |Extension Register|R8 |R9 |R10|R11|R12|R13|R14|R15|
// +------------------+---+---+---+---+---+---+---+---+
#define SU_X86_64_REGISTER_INDEX 3
#define SU_X86_64_USE_EXT_REGISTER true

#if SU_X86_64_USE_EXT_REGISTER
    #define SU_X86_64_REX_PREFIX_BYTE (SU_X86_64_REX_W_PREFIX_BYTE | SU_X86_64_REX_B_PREFIX_BYTE)
#else
    #define SU_X86_64_REX_PREFIX_BYTE SU_X86_64_REX_W_PREFIX_BYTE
#endif

// Encode the function names, since Unix shared libraries show all the function names (even ones that aren't exported);
#define gsGet32BitIntelInterceptionByte(param1, param2) gsIFEN1419(param1, param2)
#define gsGetARMv6IntercptionByte(param1, param2)       gsIFEN2986(param1, param2)
#define gsGetARMThumb2InterceptionByte(param1, param2)  gsIFEN8370(param1, param2)
#define gsGet64BitIntelInterceptionByte(param1, param2) gsIFEN0812(param1, param2)

// Fixes for specific problematic functions:
#define gsFixiPhoneOS3_2InterceptionOfEAGLContext_presentRenderbuffer(param1, param2) gsIFEN1002(param1, param2)

#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
// ---------------------------------------------------------------------------
// Name:        gsGet32BitIntelInterceptionByte
// Description: Gets the byteIndex-th byte for intercepting in intel 32-bit
// Author:      Uri Shomroni
// Date:        16/11/2009
// ---------------------------------------------------------------------------
gtUByte gsGet32BitIntelInterceptionByte(int byteIndex, int32_t relativeAddress)
{
    gtUByte retVal = 0x00;

    // The interception instruction is an assembly JMP command (and Intel-based OS X computers are big endian)
    // JMP (rel imm32)0xwwxxyyzz                            E9 zz yy xx ww
    // E9: JMP near direct relative
    // 4 bytes holding the relative address from the EIP after the instruction to the wrapper function.
    if (byteIndex > 0)
    {
        GT_ASSERT(byteIndex < SU_LENGTH_OF_JMP_COMMAND_IN_BYTES);
        retVal = (gtUByte)(relativeAddress >> (8 * (byteIndex - 1)));
    }
    else
    {
        retVal = SU_I386_JMP_OPCODE;
    }

    return retVal;
}

#ifdef _GR_IPHONE_DEVICE_BUILD
// ---------------------------------------------------------------------------
// Name:        gsGetARMv6IntercptionByte
// Description: Gets the byteIndex-th byte for intercepting in ARMv6
// Author:      Uri Shomroni
// Date:        16/11/2009
// ---------------------------------------------------------------------------
gtUByte gsGetARMv6IntercptionByte(int byteIndex, const osProcedureAddress& pWrapperFunction)
{
    gtUByte retVal = 0x00;

    // The interception instruction is an ARM assembly ldr command, loading from pc - 4 into pc, followed by the target address:
    // ldr pc, [pc, #-4]        04 F0 1F E5
    // #0xwwxxyyzz              zz yy xx ww
    // The Load register instruction is E5 1F F0 04 in a big-endian order (so 04 F0 1F E5):
    // E = 1110: Condition Code (COND bits) for AL (always perform)
    // 5 = 0101: Opcode for LDR and STR
    // 1 = 0001:
    //     UB0L - U = 0 for negative offset (-4 and not +4)
    //            B = 0 for 4-byte value (32-bit pointer instead of 8-bit)
    //            L = 1 for load (LDR and not STR)
    // F = 1111: Rn = r15 = pc as base register (read from PC address)
    // F = 1111: Rd = r15 = pc as destination register (write into PC)
    // 004 = 12-bit offset from PC (of -4) to read. Note that the value in PC is this instruction + 8 since ARM processors
    //  have a 2-instruction readahead.
    //
    // The first instruction effectively loads the second "instruction" into the program counter, supplying a jump.
    switch (byteIndex)
    {
        // The order of the first 4 cases is reversed intentionally, to maintain readability of the instruction.
        case 3:
        {
            retVal = SU_ARMV6_AL_CONDITION | SU_ARMV6_LDR_OPCODE;
        }
        break;

        case 2:
        {
            retVal = SU_ARMV6_LDR_SIGN_WIDTH_AND_DIRECTION_MODS | SU_ARMV6_LDR_BASE_REGISTER;
        }
        break;

        case 1:
        {
            retVal = SU_ARMV6_LDR_DEST_REGISTER | SU_ARMV6_LDR_REGISTER_OFFSET1;
        }
        break;

        case 0:
        {
            retVal = SU_ARMV6_LDR_REGISTER_OFFSET2;
        }
        break;

        case 4:
        case 5:
        case 6:
        case 7:
        {
            retVal = (gtUByte)((gtUInt32)pWrapperFunction >> (8 * (byteIndex - 4)));
        }
        break;

        default:
        {
            // We shouldn't get here:
            GT_ASSERT(false);
        }
        break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsGetARMThumb2InterceptionByte
// Description: Gets the byteIndex-th byte for intercepting in ARM Thumb-2
// Author:      Uri Shomroni
// Date:        16/11/2009
// ---------------------------------------------------------------------------
gtUByte gsGetARMThumb2InterceptionByte(int byteIndex, const osProcedureAddress& pWrapperFunction)
{
    gtUByte retVal = 0x00;

    // The interception instruction is an ARM Thumb-2 assembly ldr command, loading from pc - 4 into pc, followed by the target address:
    // ldr pc, [pc, #0]         df f8 00 f0
    // #0xwwxxyyzz              zz yy xx ww
    // The load register instruction is two big-endian halfwords (F8DF and F000), one being the function and the second being operands:
    //  F   8   D   F
    //  1111100011011111
    //  111                 Thumb opcode for BL[X], used to start Thumb-2 instructions
    //     1100             STR / LDR (11 is Non-00, Differentiates Thumb BL[X] from Thumb-2)
    //         0            Sign-extend operand (1 = signed, 0 = unsigned) - only used for halfword / byte operands
    //          1           Offset sign (1 = +, 0 = -) Add or substract from Base register value
    //           10         Operand size (10 = W, 01 = H, 00 = B, 11 = Unknown)
    //             1        Load / Store (1 = LDR, 0 = STR)
    //              1111    (Presumably) Base register (r15 = PC)
    //
    //  F000
    //  F = Target register (r15 = PC)
    //  000 = Immediate 12-bit offset. Note that PC value = This instruction + 2 Thumb instructions readahead,which is 4 bytes. Since our data is 4 bytes away, we add nothing.
    //
    // The first instruction effectively loads the second "instruction" into the program counter, supplying a jump.
    switch (byteIndex)
    {
        // The order of the first 4 cases is halfword-reversed intentionally, to maintain readability of the instruction.
        case 1:
        {
            retVal = SU_ARM_THUMB2_LDR_OPCODE | SU_ARM_THUMB2_LDR_SIGN_EXTENSION_MODIFIER;
        }
        break;

        case 0:
        {
            retVal = SU_ARM_THUMB2_LDR_OFFSET_SIGN_MODIFIER | SU_ARM_THUMB2_LDR_SIZE_MODIFIER | SU_ARM_THUMB2_LDR_STR_MODIFIER | SU_ARM_THUMB2_LDR_BASE_REGISTER;
        }
        break;

        case 3:
        {
            retVal = SU_ARM_THUMB2_LDR_DEST_REGISTER | SU_ARM_THUMB2_LDR_REGISTER_OFFSET1;
        }
        break;

        case 2:
        {
            retVal = SU_ARM_THUMB2_LDR_REGISTER_OFFSET2;
        }
        break;

        case 4:
        case 5:
        case 6:
        case 7:
        {
            retVal = (gtUByte)((gtUInt32)pWrapperFunction >> (8 * (byteIndex - 4)));
        }
        break;

        default:
        {
            // We shouldn't get here:
            GT_ASSERT(false);
        }
        break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsFixiPhoneOS3_2InterceptionOfEAGLContext_presentRenderbuffer
// Description: Fixes the Interception of -[EAGLContext presentRenderbuffer:]
//              on iPhone OS 3.2 (iPad).
// Author:      Uri Shomroni
// Date:        27/4/2010
// ---------------------------------------------------------------------------
void gsFixiPhoneOS3_2InterceptionOfEAGLContext_presentRenderbuffer(suARMv6InterceptionIsland& functionInterceptionIsland, gtUInt32 actualCodeAddress)
{
    //////////////////////////////////////////////////////////////////////////
    // In iPhone OS 3.2, the first four (THUMB-2) commands in -[EAGLContext presentRenderbuffer:]
    // are as follows:
    //  push {r4, r7, lr}   0xb590  // See PUSH A7.1.50 (A7-84)
    //  add r7, sp, #4      0xaf01  // See ADD(6) A7.1.8 (A7-11)
    //  ldr r3 [pc, #68]    0x4b11  // See LDR(3) A7.1.30 (A7-51)   *((uint32*)([presentRenderbuffer:] + 76))
    //  add r3, pc          0x447b  // See ADD(4) A7.1.6 (A7-8)     (uint32)[presentRenderbuffer:] + 10
    // So the first word is 0xaf01b590 and the second is 0x447b4b11.
    //
    // Since the pc while running in our interception island is not what it is in the real code, the last two commands
    // would cause problems. The first one would require us to copy a larger interception island (see the exception for
    // -[EAGLContext initWithAPI:]) to get the constant stored ahead, but the second cannot be solved without changing
    // the content of the memory to compensate for the move. However, unlike a compiler, for us the input value in
    // "add r3, pc" (i.e. the value of the program counter) is already constant, so we can simply calculate it in advance
    // and merge both commands into a single load command.
    //
    // Also note that this function does not have an ARM (4-byte) command being cut off at the end of the first 8 bytes, so
    // the 5th word in the interception island is unused. Therefore, what we do is as follows:
    //
    // 1. Access the 32-bit integer stored at a 76-byte offset from the functions start, and add it to the program counter
    //      address at the call to add r3, pc. This will form the value we expect to be stored in r3.
    // 2. Assign that value in the fifth word of the interception island.
    // 3. Replace the two commands into a new, ldr single command, followed by a nop command:
    //  ldr r3, [pc, #8]    0x4b02  // See LDR(3) A7.1.30 (A7-51)   *((int32*)(interceptionisland + 16))
    //  nop                 0x46c0
    //////////////////////////////////////////////////////////////////////////

    // First, check if the command we wish to replace is the correct one (note that this also verifies
    // that the 5th word is unused):
    if (functionInterceptionIsland._instructions[1] == SU_IPHONE_OS_3_2_PRESENT_RENDERBUFFER_ORIGINAL_INSTRUCTIONS_3_AND_4)
    {
        // Calculate the expected value of R3 after the operation:
        gtUInt32 valueOfR3 = (*((gtUInt32*)(actualCodeAddress + 76))) + actualCodeAddress + 10;

        // Set the value into the open place:
        functionInterceptionIsland._instructions[4] = valueOfR3;

        // Set the modified LDR command (and following NOP) instead of the reomved commands:
        functionInterceptionIsland._instructions[1] = SU_IPHONE_OS_3_2_PRESENT_RENDERBUFFER_INTERCEPTION_INSTRUCTIONS_3_AND_4;
    }
}

#endif // _GR_IPHONE_DEVICE_BUILD

#elif AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
// ---------------------------------------------------------------------------
// Name:        gsGet64BitIntelInterceptionByte
// Description: Gets the byteIndex-th byte for intercepting in intel 64-bit
// Author:      Uri Shomroni
// Date:        16/11/2009
// ---------------------------------------------------------------------------
gtUByte gsGet64BitIntelInterceptionByte(int byteIndex, const osProcedureAddress& pWrapperFunction)
{
    gtUByte retVal = 0x00;

    // Generate the new instructions:
    // Instruction 1: MOV %rXX,(imm64)0xssttuuvvwwxxyyzz    4m Bn zz yy xx ww vv uu tt ss
    // 48 / 49: REX prefix byte: 0100WRXB
    //      W = 1 for 64-bit operation, B = 1 if using an extension register
    // B8 - BF: MOV: 10111rrr
    //      rrr = the target register
    // 8 bytes holding the absolute address to the wrapper function.
    //
    // Instruction 2: JMP *%rXX                             4m FF En
    // 48 / 49: REX prefix, see above
    // FF: JMP, the "FF" part of the FF /4 instruction:
    // E0 - E8: MOD R/M Byte: 11100rrr
    //      11 = MOD bits for using a register's content (instead of the data pointed by the content)
    //      100 = the "/4" part of the FF /4 instruction
    //      rrr = r/m bits used to point to the register we want to use
    switch (byteIndex)
    {
        case 0:
            retVal = SU_X86_64_REX_PREFIX_BYTE;
            break;

        case 1:
            retVal = SU_X86_64_MOV_OPCODE | SU_X86_64_REGISTER_INDEX;
            break;

        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
            retVal = (gtUByte)((gtUInt64)pWrapperFunction >> (8 * (byteIndex - 2)));
            break;

        case 10:
            retVal = SU_X86_64_REX_PREFIX_BYTE;
            break;

        case 11:
            retVal = SU_X86_64_ABS_JMP_OPCODE;
            break;

        case 12:
            retVal = SU_X86_64_ABS_JMP_TO_REGISTER_VALUE_MOD_RM_BYTE | SU_X86_64_REGISTER_INDEX;
            break;

        default:
            // Unexpected value!
            GT_ASSERT(false);
            break;
    }

    return retVal;
}
#else // AMDT_ADDRESS_SPACE_TYPE
#error Unknown address space size
#endif // AMDT_ADDRESS_SPACE_TYPE


// ---------------------------------------------------------------------------
// Name:        suInitializeMacOSXInterception
// Description: Replaces a specific function's instructions
// Arguments: apMonitoredFunctionId funcId
//            osProcedureAddress pRealFunction
//            osProcedureAddress pWrapperFunction
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        21/5/2009
// ---------------------------------------------------------------------------
bool suInitializeMacOSXInterception(apMonitoredFunctionId funcId, const gtString& funcName, osProcedureAddress pRealFunction, osProcedureAddress pWrapperFunction, osProcedureAddress& pNewRealFunction)
{
    bool retVal = false;

    // Make sure this function exists in the real OpenGL library as well as ours:
    if ((pRealFunction != NULL) && (pWrapperFunction != NULL))
    {
        // Calculate the offset:
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
#ifdef _GR_IPHONE_DEVICE_BUILD
        suARMv6InterceptionIsland* pInterceptionIsland = &(su_stat_armv6InterceptionIslands[funcId]);

        gtUInt32 islandAddress = (gtUInt32)pInterceptionIsland;
#else
        // On 32 bit OSs - we add 5 to the real function pointer since that's the length of a JMP command:
        int32_t relativeAddress = (int32_t)pWrapperFunction - ((int32_t)pRealFunction + SU_LENGTH_OF_JMP_COMMAND_IN_BYTES);
#endif
#elif AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
        // The JMP command we use here is an absolute one, so there is no need to calculate the difference.
#else
#error Error unknown address space size!
#endif

        // Get the actual memory pointer where the code resides:
#ifdef _GR_IPHONE_DEVICE_BUILD
        // In ARMv7 and ARMv6 with Thumb-2 support, function calls and jumps can be made with a
        // change from ARM mode to Thumb mode. This is encoded by having the LSb of the address
        // be 1 for Thumb and 0 for ARM.
        // In addition, ARM code is word-aligned and Thumb code is halfword-aligned, so the possible
        // values for the last two bits are 00 (ARM) and 01 or 11 (Thumb).
        gtUInt32 actualCodeAddress = (gtUInt32)pRealFunction;
        gtUByte addressLastTwoBits = (gtUInt32)pRealFunction & 0x3;
        bool isThumbCode = false;

        if (addressLastTwoBits == 0)
        {
            // The intercepted function is ARM code, do nothing.
        }
        else if ((addressLastTwoBits == 1) || (addressLastTwoBits == 3))
        {
            // The intercepted function is Thumb code, note it and subtract 1 to get the real address:
            isThumbCode = true;
            actualCodeAddress--;
        }
        else
        {
            // The intercepted function is supposedly ARM, but isn't word-aligned, which cannot be:
            GT_ASSERT(false);
        }

        //////////////////////////////////////////////////////////////////////////
        // Uri, 17/12/09:
        //  This is required since one of the commands in the first two words of
        //  code in the iPhone / iPod 3GS version of -[EAGLContext initWithAPI:]
        //  contain a THUMB-2 instruction for "ldr r1, [pc, #12]".
        //  To make sure we don't redundantly do this on other iPhones, we verify
        //  that this is THUMB code by comparing actualCodeAddress to pRealFunction
        //
        //  This reads information from ahead in the code (somewhere after the end
        //  of our interception island (and into the next one). So insread we need
        //  to copy more of the code and give this function a bigger area to work.
        //
        //  Furthermore, if assigning a new, separate island for this and making it
        //  executable we cause some of the heap (depending on if this is local or
        //  global) to be executable and thus not writable in iPhone OS. This causes
        //  access violations to spring left and right.
        //
        //  Instead, we use the "dead" area reserved for the CGL functions, which are
        //  obviously not intercepted on the iPhone.
        //  To use a separate island we would need to use the below commented-out
        //  code, followed by making the separate island executable with vm_protect
        //  once we are finished writing to it.
        //////////////////////////////////////////////////////////////////////////
        if ((funcId == ap_EAGLContext_initWithAPI) && (actualCodeAddress != (gtUInt32)pRealFunction))
        {
            // Create a separate island:
            // gtUByte* largerInterceptionIsland = new gtUByte[SU_LARGER_INTERCEPTION_ISLAND_SIZE];
            // pInterceptionIsland = (suARMv6InterceptionIsland*)largerInterceptionIsland;

            // Use the islands for the first two CGL functions instead of the function's
            // real island:
            pInterceptionIsland = &(su_stat_armv6InterceptionIslands[ap_CGLChoosePixelFormat]);
            islandAddress = (gtUInt32)pInterceptionIsland;

            // Copy the bytes from the real code location:
            gtUByte* pLargerInterceptionIsland = (gtUByte*)pInterceptionIsland;

            for (int i = 0; i < SU_LARGER_INTERCEPTION_ISLAND_SIZE; i++)
            {
                pLargerInterceptionIsland[i] = ((gtUByte*)actualCodeAddress)[i];
            }
        }

#else // ndef _GR_IPHONE_DEVICE_BUILD
        // In Intel architectures, the code is stored at the function address.
        osProcedureAddress& actualCodeAddress = pRealFunction;
#endif // _GR_IPHONE_DEVICE_BUILD

        if (pRealFunction != pWrapperFunction)
        {
            // Set the virtual memory page where the real function resides to be readable, writable and executable:
            int err = vm_protect(mach_task_self(), (vm_address_t)actualCodeAddress, SU_LENGTH_OF_JMP_COMMAND_IN_BYTES, false, (VM_PROT_ALL | VM_PROT_COPY));

            if (err != 0)
            {
                // We failed, let's try making it just readable and writable:
                err = vm_protect(mach_task_self(), (vm_address_t)actualCodeAddress, SU_LENGTH_OF_JMP_COMMAND_IN_BYTES, false, (VM_PROT_DEFAULT | VM_PROT_COPY));
            }

            GT_IF_WITH_ASSERT_EX((!err), SU_STR_DebugLog_couldNotChangeVMAddressRWPermissions)
            {
                su_stat_functionInterceptionInfo[funcId]._isCurrentlyInsideWrapper = false;

                for (unsigned int i = 0; i < SU_LENGTH_OF_JMP_COMMAND_IN_BYTES; i++)
                {
                    // Read the original instructions:
                    su_stat_functionInterceptionInfo[funcId]._originalInstructions[i] = ((gtUByte*)actualCodeAddress)[i];

#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
#ifdef _GR_IPHONE_DEVICE_BUILD

                    if (isThumbCode)
                    {
                        su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[i] = gsGetARMThumb2InterceptionByte(i, pWrapperFunction);
                    }
                    else
                    {
                        su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[i] = gsGetARMv6IntercptionByte(i, pWrapperFunction);
                    }

#else
                    su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[i] = gsGet32BitIntelInterceptionByte(i, relativeAddress);
#endif
#elif AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
                    su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[i] = gsGet64BitIntelInterceptionByte(i , pWrapperFunction);
#else
#error Error unknown address space size!
#endif
                }

                // Output a message about changing the pointer.
                // This message is slightly unreadable to have users not see how our interception works
                // See the comments next to the string constants to see which field means what.
                if (osDebugLog::instance().loggedSeverity() == OS_DEBUG_LOG_DEBUG)
                {
                    gtString replacingStr = SU_STR_DebugLog_replacingFunctionInstructions1;
                    replacingStr.append(funcName).appendFormattedString(SU_STR_DebugLog_replacingFunctionInstructions2, pRealFunction);
                    replacingStr.appendFormattedString(SU_STR_DebugLog_replacingFunctionInstructions3, pWrapperFunction);

                    replacingStr.append(SU_STR_DebugLog_replacingFunctionInstructions4);

                    for (int i = 0; i < SU_LENGTH_OF_JMP_COMMAND_IN_BYTES; i++)
                    {
                        replacingStr.appendFormattedString(L" %02X", su_stat_functionInterceptionInfo[funcId]._originalInstructions[i]);
                    }

                    replacingStr.append(SU_STR_DebugLog_replacingFunctionInstructions5);

                    for (int i = 0; i < SU_LENGTH_OF_JMP_COMMAND_IN_BYTES; i++)
                    {
                        replacingStr.appendFormattedString(L" %02X", su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[i]);
                    }

                    OS_OUTPUT_DEBUG_LOG(replacingStr.asCharArray(), OS_DEBUG_LOG_DEBUG);
                }

                // Replace the command so that our function is initially called:
                for (int i = 0; i < SU_LENGTH_OF_JMP_COMMAND_IN_BYTES; i++)
                {
                    ((gtUByte*)actualCodeAddress)[i] = su_stat_functionInterceptionInfo[funcId]._interceptionInstructions[i];
                }

                // Note our success:
                retVal = true;
            }

#ifdef _GR_IPHONE_DEVICE_BUILD
            // Set the island code. This is made up of the first two instructions of the original function, followed
            // by a command to jump 8 bytes (SU_LENGTH_OF_JMP_COMMAND_IN_BYTES) ahead of the real function pointer.
            // If the original function was
            // INSTR1/INSTR1+2      INSTR2/INSTR3+4     INSTR3/INSTR5+6     ...
            // our interception made it
            // ldr pc, [pc, #-4/0]  pWrapperFunction    INSTR3/INSTR5+6     ...
            // So to run the real function code, we run this:
            // INSTR1/INSTR1+2      INSTR2/INSTR3+4     ldr pc, [pc, #-4/0] pRealFunction + 8
            // Which causes the code to continue from where we took over.
            //
            // Also note that if the real function is Thumb, the first four instructions will be Thumb, so when we call
            // the island as a function, we need it to be called as Thumb code.
            pInterceptionIsland->_instructions[0] = ((gtUInt32*)su_stat_functionInterceptionInfo[funcId]._originalInstructions)[0];
            pInterceptionIsland->_instructions[1] = ((gtUInt32*)su_stat_functionInterceptionInfo[funcId]._originalInstructions)[1];

            if (isThumbCode)
            {
                // To tell the processor to run the island as Thumb code, we add 1 to the island address:
                islandAddress++;

                // In which index to insert the LDR instruction:
                int jmpPos = 2;

                // If the last halfword begins with 0b111, it is a BL[X] command or a Thumb-2 command. Either way, it is a 4-byte
                // instruction, so we need to copy two more bytes:
                if ((pInterceptionIsland->_instructions[1] & 0xE0000000) == 0xE0000000)
                {
                    // We add another word:
                    jmpPos++;

                    // Add the other half of the function, followed by a NOP for word-alignment.
                    gtUByte* pRealFunctionBytes = ((gtUByte*)actualCodeAddress);
                    pInterceptionIsland->_instructions[2] = pRealFunctionBytes[SU_LENGTH_OF_JMP_COMMAND_IN_BYTES] | (pRealFunctionBytes[SU_LENGTH_OF_JMP_COMMAND_IN_BYTES + 1] << 8) | (SU_ARM_THUMB2_NOP_INSTRUCTION << 16);

                    // Set the address to be right after our interception code and the additional 2 bytes::
                    pInterceptionIsland->_instructions[4] = (((gtUInt32)pRealFunction) + SU_LENGTH_OF_JMP_COMMAND_IN_BYTES + 2);
                }
                else // (pInterceptionIsland->_instructions[1] & 0xE0000000) != 0xE0000000
                {
                    // Set the address to be right after our interception code:
                    pInterceptionIsland->_instructions[3] = (((gtUInt32)pRealFunction) + SU_LENGTH_OF_JMP_COMMAND_IN_BYTES);
                }

                // Set a Thumb-2 jump:
                pInterceptionIsland->_instructions[jmpPos] = SU_ARM_THUMB2_LDR_PC_PC_PLUS_0_INSTRUCTION;
            }
            else // !isThumbCode
            {
                // Set an ARMv6 jump:
                pInterceptionIsland->_instructions[2] = SU_ARMV6_LDR_PC_PC_MINUS_4_INSTRUCTION;
                pInterceptionIsland->_instructions[3] = (((gtUInt32)pRealFunction) + SU_LENGTH_OF_JMP_COMMAND_IN_BYTES);
            }

            // On iPhone OS 3.2, there is a problem with this interception for the function -[EAGLContext presentRenderbuffer:].
            // So, we need to treat this case specificly:
            if (funcId == ap_EAGLContext_presentRenderbuffer)
            {
                gsFixiPhoneOS3_2InterceptionOfEAGLContext_presentRenderbuffer(*pInterceptionIsland, actualCodeAddress);
            }

            // Set the "real" address to our island address - which effectively executes the real function code:
            pNewRealFunction = (void*)islandAddress;

            // On iPhone OS 2.0, a memory page can not be writable and executable at the same time. So we need to restore it after each change:
            err = vm_protect(mach_task_self(), (vm_address_t)pRealFunction, SU_LENGTH_OF_JMP_COMMAND_IN_BYTES, false, (VM_PROT_READ | VM_PROT_EXECUTE | VM_PROT_COPY));
            GT_ASSERT_EX((err == 0), SU_STR_DebugLog_couldNotChangeVMAddressRWPermissions)
#endif
        }
        else // (pRealFunction == pWrapperFunction)
        {
            // If the address we got for "both" functions is the same, something is not working properly, so we don't change anything
            gtString errMsg = funcName;
            errMsg.prepend(SU_STR_DebugLog_functionWrapperAndImplAreSame1).appendFormattedString(SU_STR_DebugLog_functionWrapperAndImplAreSame2, pRealFunction);
            OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
        }
    }
    else // ((pRealFunction == NULL) || (pWrapperFunction == NULL))
    {
        // Assemble a log message: "Could not find [function address [and] wrapper address] for function glFoo"
        gtString logMsg;

        if (pRealFunction == NULL)
        {
            logMsg.append("src");

            // We consider an non-existing real function not to be a failure, as it might mean the function is simply
            // not supported. The real check in this matter is performed when the function wrappers are initialized.
            retVal = true;
        }

        if (pWrapperFunction == NULL)
        {
            if (!logMsg.isEmpty())
            {
                logMsg.append(" and ");
            }

            logMsg.append("dst");
        }

        logMsg.prepend(L"Could not find ").append(" for function ").append(funcName);
        OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        suBeforeInitializingMacOSXInterception
// Description: Called before the OpenGL / OpenCL spies initialize the interception
//              for their proper functions
// Author:      Uri Shomroni
// Date:        24/11/2009
// ---------------------------------------------------------------------------
void suBeforeInitializingMacOSXInterception()
{
    // Assign the vectors needed for interception, if they were not yet assigned:
    if (su_stat_functionInterceptionInfo == NULL)
    {
        su_stat_functionInterceptionInfo = new suFunctionInterceptionInformation[apMonitoredFunctionsAmount];

    }

#ifdef _GR_IPHONE_DEVICE_BUILD

    // Create the interception islands array:
    if (su_stat_armv6InterceptionIslands == NULL)
    {
        su_stat_armv6InterceptionIslands = new suARMv6InterceptionIsland[apMonitoredFunctionsAmount];

    }

    // Make the interception islands writeable (even if we didn't just create them):
    int err = vm_protect(mach_task_self(), (vm_address_t)su_stat_armv6InterceptionIslands, apMonitoredFunctionsAmount * sizeof(suARMv6InterceptionIsland), false, (VM_PROT_READ | VM_PROT_WRITE));
    GT_ASSERT(err == 0);
#endif
}

// ---------------------------------------------------------------------------
// Name:        suAfterInitializingMacOSXInterception
// Description: Called after the OpenGL / OpenCL spies initialize the interception
//              for their proper functions
// Author:      Uri Shomroni
// Date:        24/11/2009
// ---------------------------------------------------------------------------
void suAfterInitializingMacOSXInterception()
{
#ifdef _GR_IPHONE_DEVICE_BUILD
    // Make the interception islands executable:
    int err = vm_protect(mach_task_self(), (vm_address_t)su_stat_armv6InterceptionIslands, apMonitoredFunctionsAmount * sizeof(suARMv6InterceptionIsland), false, (VM_PROT_READ | VM_PROT_EXECUTE));
    GT_ASSERT(err == 0);
#endif
}

// ---------------------------------------------------------------------------
// Name:        suFunctionInterceptionInformation::suFunctionInterceptionInformation
// Description: Constructor for suFunctionInterceptionInformation struct
// Author:      Uri Shomroni
// Date:        22/1/2009
// ---------------------------------------------------------------------------
suFunctionInterceptionInformation::suFunctionInterceptionInformation()
{
    for (int i = 0; i < SU_LENGTH_OF_JMP_COMMAND_IN_BYTES; i++)
    {
        _interceptionInstructions[i] = 0x00;
        _originalInstructions[i] = 0x00;
    }
}



