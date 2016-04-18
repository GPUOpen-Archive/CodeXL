//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file DisassemblerX86.cpp
///
//==================================================================================

#include "DisassemblerX86.h"

#if AMDT_CPP_COMPILER == AMDT_VISUAL_CPP_COMPILER
    #pragma warning(push)
    #pragma warning(disable : 4201) // nameless struct/union
#endif

union OpcodeInfo
{
    struct
    {
        gtUInt32 m_opcode      : 8;
        gtUInt32 m_size        : 3;
        gtUInt32 m_size2       : 3;
        gtUInt32 m_offsetModRm : 3;
    };

    gtUInt32 m_value;
};

union OperandInfo
{
    struct
    {
        gtUInt32 m_hasOperand         : 1;
        gtUInt32 m_shiftRegister      : 3;
        gtUInt32 m_offsetRegister     : 5;
        gtUInt32 m_sizeDisplacement   : 2;
        gtUInt32 m_shiftDisplacement  : 3;
        gtUInt32 m_offsetDisplacement : 5;
    };

    struct
    {
        gtUInt32                : 1;
        gtUInt32 m_dataRegister : 8;
    };

    gtUInt32 m_value;
};

#if AMDT_CPP_COMPILER == AMDT_VISUAL_CPP_COMPILER
    #pragma warning(pop)
#endif

static const gtUByte s_rbModRm[0x100] =
{
    0x00, 0x00, 0x00, 0x00, 0x11, 0x04, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x11, 0x04, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x11, 0x04, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x11, 0x04, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x11, 0x04, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x11, 0x04, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x11, 0x04, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x11, 0x04, 0x00, 0x00,
    0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01,
    0x04, 0x04, 0x04, 0x04, 0x05, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x05, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x05, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x05, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x05, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x05, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x05, 0x04, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x05, 0x04, 0x04, 0x04,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const DisassemblerX86::OpcodeEntry DisassemblerX86::s_rceCopyTable0F[0x100] =
{
    {0x5200, &DisassemblerX86::CrackBytes, 0},
    {0x5201, &DisassemblerX86::CrackBytes, 0},
    {0x5202, &DisassemblerX86::CrackBytes, 0},
    {0x5203, &DisassemblerX86::CrackBytes, 0},
    {0x904, &DisassemblerX86::Invalid, 0},
    {0x905, &DisassemblerX86::Invalid, 0},
    {0x1206, &DisassemblerX86::CrackBytes, 0},
    {0x907, &DisassemblerX86::Invalid, 0},
    {0x1208, &DisassemblerX86::CrackBytes, 0},
    {0x1209, &DisassemblerX86::CrackBytes, 0},
    {0x90A, &DisassemblerX86::Invalid, 0},
    {0x120B, &DisassemblerX86::CrackBytes, 0},
    {0x90C, &DisassemblerX86::Invalid, 0},
    {0x90D, &DisassemblerX86::Invalid, 0},
    {0x90E, &DisassemblerX86::Invalid, 0},
    {0x90F, &DisassemblerX86::Invalid, 0},
    {0x910, &DisassemblerX86::Invalid, 0},
    {0x911, &DisassemblerX86::Invalid, 0},
    {0x912, &DisassemblerX86::Invalid, 0},
    {0x913, &DisassemblerX86::Invalid, 0},
    {0x914, &DisassemblerX86::Invalid, 0},
    {0x915, &DisassemblerX86::Invalid, 0},
    {0x916, &DisassemblerX86::Invalid, 0},
    {0x917, &DisassemblerX86::Invalid, 0},
    {0x918, &DisassemblerX86::Invalid, 0},
    {0x919, &DisassemblerX86::Invalid, 0},
    {0x91A, &DisassemblerX86::Invalid, 0},
    {0x91B, &DisassemblerX86::Invalid, 0},
    {0x91C, &DisassemblerX86::Invalid, 0},
    {0x91D, &DisassemblerX86::Invalid, 0},
    {0x91E, &DisassemblerX86::Invalid, 0},
    {0x91F, &DisassemblerX86::Invalid, 0},
    {0x5220, &DisassemblerX86::CrackBytes, 0},
    {0x5221, &DisassemblerX86::CrackBytes, 0},
    {0x5222, &DisassemblerX86::CrackBytes, 0},
    {0x5223, &DisassemblerX86::CrackBytes, 0},
    {0x924, &DisassemblerX86::Invalid, 0},
    {0x925, &DisassemblerX86::Invalid, 0},
    {0x926, &DisassemblerX86::Invalid, 0},
    {0x927, &DisassemblerX86::Invalid, 0},
    {0x928, &DisassemblerX86::Invalid, 0},
    {0x929, &DisassemblerX86::Invalid, 0},
    {0x92A, &DisassemblerX86::Invalid, 0},
    {0x92B, &DisassemblerX86::Invalid, 0},
    {0x92C, &DisassemblerX86::Invalid, 0},
    {0x92D, &DisassemblerX86::Invalid, 0},
    {0x92E, &DisassemblerX86::Invalid, 0},
    {0x92F, &DisassemblerX86::Invalid, 0},
    {0x1230, &DisassemblerX86::CrackBytes, 0},
    {0x1231, &DisassemblerX86::CrackBytes, 0},
    {0x1232, &DisassemblerX86::CrackBytes, 0},
    {0x1233, &DisassemblerX86::CrackBytes, 0},
    {0x1234, &DisassemblerX86::CrackBytes, 0},
    {0x1235, &DisassemblerX86::CrackBytes, 0},
    {0x936, &DisassemblerX86::Invalid, 0},
    {0x937, &DisassemblerX86::Invalid, 0},
    {0x938, &DisassemblerX86::Invalid, 0},
    {0x939, &DisassemblerX86::Invalid, 0},
    {0x93A, &DisassemblerX86::Invalid, 0},
    {0x93B, &DisassemblerX86::Invalid, 0},
    {0x93C, &DisassemblerX86::Invalid, 0},
    {0x93D, &DisassemblerX86::Invalid, 0},
    {0x93E, &DisassemblerX86::Invalid, 0},
    {0x93F, &DisassemblerX86::Invalid, 0},
    {0x5240, &DisassemblerX86::CrackBytes, 0},
    {0x5241, &DisassemblerX86::CrackBytes, 0},
    {0x5242, &DisassemblerX86::CrackBytes, 0},
    {0x5243, &DisassemblerX86::CrackBytes, 0},
    {0x5244, &DisassemblerX86::CrackBytes, 0},
    {0x5245, &DisassemblerX86::CrackBytes, 0},
    {0x5246, &DisassemblerX86::CrackBytes, 0},
    {0x5247, &DisassemblerX86::CrackBytes, 0},
    {0x5248, &DisassemblerX86::CrackBytes, 0},
    {0x5249, &DisassemblerX86::CrackBytes, 0},
    {0x524A, &DisassemblerX86::CrackBytes, 0},
    {0x524B, &DisassemblerX86::CrackBytes, 0},
    {0x524C, &DisassemblerX86::CrackBytes, 0},
    {0x524D, &DisassemblerX86::CrackBytes, 0},
    {0x524E, &DisassemblerX86::CrackBytes, 0},
    {0x524F, &DisassemblerX86::CrackBytes, 0},
    {0x950, &DisassemblerX86::Invalid, 0},
    {0x951, &DisassemblerX86::Invalid, 0},
    {0x952, &DisassemblerX86::Invalid, 0},
    {0x953, &DisassemblerX86::Invalid, 0},
    {0x954, &DisassemblerX86::Invalid, 0},
    {0x955, &DisassemblerX86::Invalid, 0},
    {0x956, &DisassemblerX86::Invalid, 0},
    {0x957, &DisassemblerX86::Invalid, 0},
    {0x958, &DisassemblerX86::Invalid, 0},
    {0x959, &DisassemblerX86::Invalid, 0},
    {0x95A, &DisassemblerX86::Invalid, 0},
    {0x95B, &DisassemblerX86::Invalid, 0},
    {0x95C, &DisassemblerX86::Invalid, 0},
    {0x95D, &DisassemblerX86::Invalid, 0},
    {0x95E, &DisassemblerX86::Invalid, 0},
    {0x95F, &DisassemblerX86::Invalid, 0},
    {0x5260, &DisassemblerX86::CrackBytes, 0},
    {0x961, &DisassemblerX86::Invalid, 0},
    {0x5262, &DisassemblerX86::CrackBytes, 0},
    {0x5263, &DisassemblerX86::CrackBytes, 0},
    {0x5264, &DisassemblerX86::CrackBytes, 0},
    {0x5265, &DisassemblerX86::CrackBytes, 0},
    {0x5266, &DisassemblerX86::CrackBytes, 0},
    {0x5267, &DisassemblerX86::CrackBytes, 0},
    {0x5268, &DisassemblerX86::CrackBytes, 0},
    {0x5269, &DisassemblerX86::CrackBytes, 0},
    {0x526A, &DisassemblerX86::CrackBytes, 0},
    {0x526B, &DisassemblerX86::CrackBytes, 0},
    {0x96C, &DisassemblerX86::Invalid, 0},
    {0x96D, &DisassemblerX86::Invalid, 0},
    {0x526E, &DisassemblerX86::CrackBytes, 0},
    {0x526F, &DisassemblerX86::CrackBytes, 0},
    {0x970, &DisassemblerX86::Invalid, 0},
    {0x5B71, &DisassemblerX86::CrackBytes, 0},
    {0x5B72, &DisassemblerX86::CrackBytes, 0},
    {0x5B73, &DisassemblerX86::CrackBytes, 0},
    {0x5274, &DisassemblerX86::CrackBytes, 0},
    {0x5275, &DisassemblerX86::CrackBytes, 0},
    {0x5276, &DisassemblerX86::CrackBytes, 0},
    {0x1277, &DisassemblerX86::CrackBytes, 0},
    {0x978, &DisassemblerX86::Invalid, 0},
    {0x979, &DisassemblerX86::Invalid, 0},
    {0x97A, &DisassemblerX86::Invalid, 0},
    {0x97B, &DisassemblerX86::Invalid, 0},
    {0x97C, &DisassemblerX86::Invalid, 0},
    {0x97D, &DisassemblerX86::Invalid, 0},
    {0x527E, &DisassemblerX86::CrackBytes, 0},
    {0x527F, &DisassemblerX86::CrackBytes, 0},
    {0x1D80, &DisassemblerX86::CrackBytes, 0},
    {0x1D81, &DisassemblerX86::CrackBytes, 0},
    {0x1D82, &DisassemblerX86::CrackBytes, 0},
    {0x1D83, &DisassemblerX86::CrackBytes, 0},
    {0x1D84, &DisassemblerX86::CrackBytes, 0},
    {0x1D85, &DisassemblerX86::CrackBytes, 0},
    {0x1D86, &DisassemblerX86::CrackBytes, 0},
    {0x1D87, &DisassemblerX86::CrackBytes, 0},
    {0x1D88, &DisassemblerX86::CrackBytes, 0},
    {0x1D89, &DisassemblerX86::CrackBytes, 0},
    {0x1D8A, &DisassemblerX86::CrackBytes, 0},
    {0x1D8B, &DisassemblerX86::CrackBytes, 0},
    {0x1D8C, &DisassemblerX86::CrackBytes, 0},
    {0x1D8D, &DisassemblerX86::CrackBytes, 0},
    {0x1D8E, &DisassemblerX86::CrackBytes, 0},
    {0x1D8F, &DisassemblerX86::CrackBytes, 0},
    {0x5290, &DisassemblerX86::CrackBytes, 0},
    {0x5291, &DisassemblerX86::CrackBytes, 0},
    {0x5292, &DisassemblerX86::CrackBytes, 0},
    {0x5293, &DisassemblerX86::CrackBytes, 0},
    {0x5294, &DisassemblerX86::CrackBytes, 0},
    {0x5295, &DisassemblerX86::CrackBytes, 0},
    {0x5296, &DisassemblerX86::CrackBytes, 0},
    {0x5297, &DisassemblerX86::CrackBytes, 0},
    {0x5298, &DisassemblerX86::CrackBytes, 0},
    {0x5299, &DisassemblerX86::CrackBytes, 0},
    {0x529A, &DisassemblerX86::CrackBytes, 0},
    {0x529B, &DisassemblerX86::CrackBytes, 0},
    {0x529C, &DisassemblerX86::CrackBytes, 0},
    {0x529D, &DisassemblerX86::CrackBytes, 0},
    {0x529E, &DisassemblerX86::CrackBytes, 0},
    {0x529F, &DisassemblerX86::CrackBytes, 0},
    {0x12A0, &DisassemblerX86::CrackBytes, 0},
    {0x12A1, &DisassemblerX86::CrackBytes, 0},
    {0x12A2, &DisassemblerX86::CrackBytes, 0},
    {0x52A3, &DisassemblerX86::CrackBytes, 0},
    {0x5BA4, &DisassemblerX86::CrackBytes, 0},
    {0x52A5, &DisassemblerX86::CrackBytes, 0},
    {0x9A6, &DisassemblerX86::Invalid, 0},
    {0x9A7, &DisassemblerX86::Invalid, 0},
    {0x12A8, &DisassemblerX86::CrackBytes, 0},
    {0x12A9, &DisassemblerX86::CrackBytes, 0},
    {0x12AA, &DisassemblerX86::CrackBytes, 0},
    {0x52AB, &DisassemblerX86::CrackBytes, 0},
    {0x5BAC, &DisassemblerX86::CrackBytes, 0},
    {0x52AD, &DisassemblerX86::CrackBytes, 0},
    {0x52AE, &DisassemblerX86::CrackBytes, 0},
    {0x52AF, &DisassemblerX86::CrackBytes, 0},
    {0x52B0, &DisassemblerX86::CrackBytes, 0},
    {0x52B1, &DisassemblerX86::CrackBytes, 0},
    {0x52B2, &DisassemblerX86::CrackBytes, 0},
    {0x52B3, &DisassemblerX86::CrackBytes, 0},
    {0x52B4, &DisassemblerX86::CrackBytes, 0},
    {0x52B5, &DisassemblerX86::CrackBytes, 0},
    {0x52B6, &DisassemblerX86::CrackBytes, 0},
    {0x52B7, &DisassemblerX86::CrackBytes, 0},
    {0x9B8, &DisassemblerX86::Invalid, 0},
    {0x9B9, &DisassemblerX86::Invalid, 0},
    {0x5BBA, &DisassemblerX86::CrackBytes, 0},
    {0x52BB, &DisassemblerX86::CrackBytes, 0},
    {0x52BC, &DisassemblerX86::CrackBytes, 0},
    {0x52BD, &DisassemblerX86::CrackBytes, 0},
    {0x52BE, &DisassemblerX86::CrackBytes, 0},
    {0x52BF, &DisassemblerX86::CrackBytes, 0},
    {0x52C0, &DisassemblerX86::CrackBytes, 0},
    {0x52C1, &DisassemblerX86::CrackBytes, 0},
    {0x9C2, &DisassemblerX86::Invalid, 0},
    {0x9C3, &DisassemblerX86::Invalid, 0},
    {0x9C4, &DisassemblerX86::Invalid, 0},
    {0x9C5, &DisassemblerX86::Invalid, 0},
    {0x9C6, &DisassemblerX86::Invalid, 0},
    {0x52C7, &DisassemblerX86::CrackBytes, 0},
    {0x12C8, &DisassemblerX86::CrackBytes, 0},
    {0x12C9, &DisassemblerX86::CrackBytes, 0},
    {0x12CA, &DisassemblerX86::CrackBytes, 0},
    {0x12CB, &DisassemblerX86::CrackBytes, 0},
    {0x12CC, &DisassemblerX86::CrackBytes, 0},
    {0x12CD, &DisassemblerX86::CrackBytes, 0},
    {0x12CE, &DisassemblerX86::CrackBytes, 0},
    {0x12CF, &DisassemblerX86::CrackBytes, 0},
    {0x9D0, &DisassemblerX86::Invalid, 0},
    {0x52D1, &DisassemblerX86::CrackBytes, 0},
    {0x52D2, &DisassemblerX86::CrackBytes, 0},
    {0x52D3, &DisassemblerX86::CrackBytes, 0},
    {0x9D4, &DisassemblerX86::Invalid, 0},
    {0x52D5, &DisassemblerX86::CrackBytes, 0},
    {0x9D6, &DisassemblerX86::Invalid, 0},
    {0x9D7, &DisassemblerX86::Invalid, 0},
    {0x52D8, &DisassemblerX86::CrackBytes, 0},
    {0x52D9, &DisassemblerX86::CrackBytes, 0},
    {0x9DA, &DisassemblerX86::Invalid, 0},
    {0x52DB, &DisassemblerX86::CrackBytes, 0},
    {0x52DC, &DisassemblerX86::CrackBytes, 0},
    {0x52DD, &DisassemblerX86::CrackBytes, 0},
    {0x9DE, &DisassemblerX86::Invalid, 0},
    {0x52DF, &DisassemblerX86::CrackBytes, 0},
    {0x9E0, &DisassemblerX86::Invalid, 0},
    {0x52E1, &DisassemblerX86::CrackBytes, 0},
    {0x52E2, &DisassemblerX86::CrackBytes, 0},
    {0x9E3, &DisassemblerX86::Invalid, 0},
    {0x9E4, &DisassemblerX86::Invalid, 0},
    {0x52E5, &DisassemblerX86::CrackBytes, 0},
    {0x9E6, &DisassemblerX86::Invalid, 0},
    {0x9E7, &DisassemblerX86::Invalid, 0},
    {0x52E8, &DisassemblerX86::CrackBytes, 0},
    {0x52E9, &DisassemblerX86::CrackBytes, 0},
    {0x9EA, &DisassemblerX86::Invalid, 0},
    {0x52EB, &DisassemblerX86::CrackBytes, 0},
    {0x52EC, &DisassemblerX86::CrackBytes, 0},
    {0x52ED, &DisassemblerX86::CrackBytes, 0},
    {0x9EE, &DisassemblerX86::Invalid, 0},
    {0x52EF, &DisassemblerX86::CrackBytes, 0},
    {0x9F0, &DisassemblerX86::Invalid, 0},
    {0x52F1, &DisassemblerX86::CrackBytes, 0},
    {0x52F2, &DisassemblerX86::CrackBytes, 0},
    {0x52F3, &DisassemblerX86::CrackBytes, 0},
    {0x9F4, &DisassemblerX86::Invalid, 0},
    {0x52F5, &DisassemblerX86::CrackBytes, 0},
    {0x9F6, &DisassemblerX86::Invalid, 0},
    {0x9F7, &DisassemblerX86::Invalid, 0},
    {0x52F8, &DisassemblerX86::CrackBytes, 0},
    {0x52F9, &DisassemblerX86::CrackBytes, 0},
    {0x52FA, &DisassemblerX86::CrackBytes, 0},
    {0x9FB, &DisassemblerX86::Invalid, 0},
    {0x52FC, &DisassemblerX86::CrackBytes, 0},
    {0x52FD, &DisassemblerX86::CrackBytes, 0},
    {0x52FE, &DisassemblerX86::CrackBytes, 0},
    {0x9FF, &DisassemblerX86::Invalid, 0}
};


const DisassemblerX86::OpcodeEntry DisassemblerX86::s_rceCopyTable[0x100] =
{
    {0x5200, &DisassemblerX86::CrackBytes, 0},
    {0x5201, &DisassemblerX86::CrackBytes, 0},
    {0x5202, &DisassemblerX86::CrackBytes, 0},
    {0x5203, &DisassemblerX86::CrackBytes, 0},
    {0x1204, &DisassemblerX86::CrackBytes, 0},
    {0x1D05, &DisassemblerX86::CrackBytes, 0},
    {0x906, &DisassemblerX86::CrackBytes, 0},
    {0x907, &DisassemblerX86::CrackBytes, 0},
    {0x5208, &DisassemblerX86::CrackBytes, 0},
    {0x5209, &DisassemblerX86::CrackBytes, 0},
    {0x520A, &DisassemblerX86::CrackBytes, 0},
    {0x520B, &DisassemblerX86::CrackBytes, 0},
    {0x120C, &DisassemblerX86::CrackBytes, 0},
    {0x1D0D, &DisassemblerX86::CrackBytes, 0},
    {0x90E, &DisassemblerX86::CrackBytes, 0},
    {0x90F, &DisassemblerX86::Crack0F, 0},
    {0x5210, &DisassemblerX86::CrackBytes, 0},
    {0x5211, &DisassemblerX86::CrackBytes, 0},
    {0x5212, &DisassemblerX86::CrackBytes, 0},
    {0x5213, &DisassemblerX86::CrackBytes, 0},
    {0x1214, &DisassemblerX86::CrackBytes, 0},
    {0x1D15, &DisassemblerX86::CrackBytes, 0},
    {0x916, &DisassemblerX86::CrackBytes, 0},
    {0x917, &DisassemblerX86::CrackBytes, 0},
    {0x5218, &DisassemblerX86::CrackBytes, 0},
    {0x5219, &DisassemblerX86::CrackBytes, 0},
    {0x521A, &DisassemblerX86::CrackBytes, 0},
    {0x521B, &DisassemblerX86::CrackBytes, 0},
    {0x121C, &DisassemblerX86::CrackBytes, 0},
    {0x1D1D, &DisassemblerX86::CrackBytes, 0},
    {0x91E, &DisassemblerX86::CrackBytes, 0},
    {0x91F, &DisassemblerX86::CrackBytes, 0},
    {0x5220, &DisassemblerX86::CrackBytes, 0},
    {0x5221, &DisassemblerX86::CrackBytes, 0},
    {0x5222, &DisassemblerX86::CrackBytes, 0},
    {0x5223, &DisassemblerX86::CrackBytes, 0},
    {0x1224, &DisassemblerX86::CrackBytes, 0},
    {0x1D25, &DisassemblerX86::CrackBytes, 0},
    {0x926, &DisassemblerX86::CrackBytesPrefix, 0},
    {0x927, &DisassemblerX86::CrackBytes, 0},
    {0x5228, &DisassemblerX86::CrackBytes, 0},
    {0x5229, &DisassemblerX86::CrackBytes, 0},
    {0x522A, &DisassemblerX86::CrackBytes, 0},
    {0x522B, &DisassemblerX86::CrackBytes, 0},
    {0x122C, &DisassemblerX86::CrackBytes, 0},
    {0x1D2D, &DisassemblerX86::CrackBytes, 0},
    {0x92E, &DisassemblerX86::CrackBytesPrefix, 0},
    {0x92F, &DisassemblerX86::CrackBytes, 0},
    {0x5230, &DisassemblerX86::CrackBytes, 0},
    {0x5231, &DisassemblerX86::CrackBytes, 0},
    {0x5232, &DisassemblerX86::CrackBytes, 0},
    {0x5233, &DisassemblerX86::CrackBytes, 0},
    {0x1234, &DisassemblerX86::CrackBytes, 0},
    {0x1D35, &DisassemblerX86::CrackBytes, 0},
    {0x936, &DisassemblerX86::CrackBytesPrefix, 0},
    {0x937, &DisassemblerX86::CrackBytes, 0},
    {0x5238, &DisassemblerX86::CrackBytes, 0},
    {0x5239, &DisassemblerX86::CrackBytes, 0},
    {0x523A, &DisassemblerX86::CrackBytes, 0},
    {0x523B, &DisassemblerX86::CrackBytes, 0},
    {0x123C, &DisassemblerX86::CrackBytes, 0},
    {0x1D3D, &DisassemblerX86::CrackBytes, 0},
    {0x93E, &DisassemblerX86::CrackBytesPrefix, 0},
    {0x93F, &DisassemblerX86::CrackBytes, 0},
    {0x940, &DisassemblerX86::CrackBytes, 0},
    {0x941, &DisassemblerX86::CrackBytes, 0},
    {0x942, &DisassemblerX86::CrackBytes, 0},
    {0x943, &DisassemblerX86::CrackBytes, 0},
    {0x944, &DisassemblerX86::CrackBytes, 0},
    {0x945, &DisassemblerX86::CrackBytes, 0},
    {0x946, &DisassemblerX86::CrackBytes, 0},
    {0x947, &DisassemblerX86::CrackBytes, 0},
    {0x948, &DisassemblerX86::CrackBytes, 0},
    {0x949, &DisassemblerX86::CrackBytes, 0},
    {0x94A, &DisassemblerX86::CrackBytes, 0},
    {0x94B, &DisassemblerX86::CrackBytes, 0},
    {0x94C, &DisassemblerX86::CrackBytes, 0},
    {0x94D, &DisassemblerX86::CrackBytes, 0},
    {0x94E, &DisassemblerX86::CrackBytes, 0},
    {0x94F, &DisassemblerX86::CrackBytes, 0},
    {0x950, &DisassemblerX86::CrackBytes, 1},
    {0x951, &DisassemblerX86::CrackBytes, 1},
    {0x952, &DisassemblerX86::CrackBytes, 1},
    {0x953, &DisassemblerX86::CrackBytes, 1},
    {0x954, &DisassemblerX86::CrackBytes, 1},
    {0x955, &DisassemblerX86::CrackBytes, 1},
    {0x956, &DisassemblerX86::CrackBytes, 1},
    {0x957, &DisassemblerX86::CrackBytes, 1},
    {0x958, &DisassemblerX86::CrackBytes, 1},
    {0x959, &DisassemblerX86::CrackBytes, 1},
    {0x95A, &DisassemblerX86::CrackBytes, 1},
    {0x95B, &DisassemblerX86::CrackBytes, 1},
    {0x95C, &DisassemblerX86::CrackBytes, 1},
    {0x95D, &DisassemblerX86::CrackBytes, 1},
    {0x95E, &DisassemblerX86::CrackBytes, 1},
    {0x95F, &DisassemblerX86::CrackBytes, 1},
    {0x960, &DisassemblerX86::CrackBytes, 0x1FF},
    {0x961, &DisassemblerX86::CrackBytes, 0x1FF},
    {0x5262, &DisassemblerX86::CrackBytes, 0},
    {0x5263, &DisassemblerX86::CrackBytes, 0},
    {0x964, &DisassemblerX86::CrackBytesPrefix, 0},
    {0x965, &DisassemblerX86::CrackBytesPrefix, 0},
    {0x966, &DisassemblerX86::Crack66, 0},
    {0x967, &DisassemblerX86::Crack67, 0},
    {0x1D68, &DisassemblerX86::CrackBytes, 0x47FF},
    {0x6669, &DisassemblerX86::CrackBytes, 0},
    {0x126A, &DisassemblerX86::CrackBytes, 0x43FF},
    {0x5B6B, &DisassemblerX86::CrackBytes, 0},
    {0x96C, &DisassemblerX86::CrackBytes, 0},
    {0x96D, &DisassemblerX86::CrackBytes, 0},
    {0x96E, &DisassemblerX86::CrackBytes, 0},
    {0x96F, &DisassemblerX86::CrackBytes, 0},
    {0x1270, &DisassemblerX86::CrackBytes, 0},
    {0x1271, &DisassemblerX86::CrackBytes, 0},
    {0x1272, &DisassemblerX86::CrackBytes, 0},
    {0x1273, &DisassemblerX86::CrackBytes, 0},
    {0x1274, &DisassemblerX86::CrackBytes, 0},
    {0x1275, &DisassemblerX86::CrackBytes, 0},
    {0x1276, &DisassemblerX86::CrackBytes, 0},
    {0x1277, &DisassemblerX86::CrackBytes, 0},
    {0x1278, &DisassemblerX86::CrackBytes, 0},
    {0x1279, &DisassemblerX86::CrackBytes, 0},
    {0x127A, &DisassemblerX86::CrackBytes, 0},
    {0x127B, &DisassemblerX86::CrackBytes, 0},
    {0x127C, &DisassemblerX86::CrackBytes, 0},
    {0x127D, &DisassemblerX86::CrackBytes, 0},
    {0x127E, &DisassemblerX86::CrackBytes, 0},
    {0x127F, &DisassemblerX86::CrackBytes, 0},
    {0x5B80, &DisassemblerX86::CrackBytes, 0x8211},
    {0x6681, &DisassemblerX86::CrackBytes, 0x8611},
    {0x1282, &DisassemblerX86::CrackBytes, 0x8211},
    {0x5B83, &DisassemblerX86::CrackBytes, 0x8211},
    {0x5284, &DisassemblerX86::CrackBytes, 0},
    {0x5285, &DisassemblerX86::CrackBytes, 0},
    {0x5286, &DisassemblerX86::CrackBytes, 0},
    {0x5287, &DisassemblerX86::CrackBytes, 0},
    {0x5288, &DisassemblerX86::CrackBytes, 0},
    {0x5289, &DisassemblerX86::CrackBytes, 0x11},
    {0x528A, &DisassemblerX86::CrackBytes, 0},
    {0x528B, &DisassemblerX86::CrackBytes, 0x17},
    {0x528C, &DisassemblerX86::CrackBytes, 0},
    {0x528D, &DisassemblerX86::CrackBytes, 0},
    {0x528E, &DisassemblerX86::CrackBytes, 0},
    {0x528F, &DisassemblerX86::CrackBytes, 0},
    {0x990, &DisassemblerX86::CrackBytes, 0},
    {0x991, &DisassemblerX86::CrackBytes, 0},
    {0x992, &DisassemblerX86::CrackBytes, 0},
    {0x993, &DisassemblerX86::CrackBytes, 0},
    {0x994, &DisassemblerX86::CrackBytes, 0},
    {0x995, &DisassemblerX86::CrackBytes, 0},
    {0x996, &DisassemblerX86::CrackBytes, 0},
    {0x997, &DisassemblerX86::CrackBytes, 0},
    {0x998, &DisassemblerX86::CrackBytes, 0},
    {0x999, &DisassemblerX86::CrackBytes, 0},
    {0x2F9A, &DisassemblerX86::CrackBytes, 0},
    {0x99B, &DisassemblerX86::CrackBytes, 0},
    {0x99C, &DisassemblerX86::CrackBytes, 0},
    {0x99D, &DisassemblerX86::CrackBytes, 0},
    {0x99E, &DisassemblerX86::CrackBytes, 0},
    {0x99F, &DisassemblerX86::CrackBytes, 0},
    {0x101DA0, &DisassemblerX86::CrackBytes, 0},
    {0x101DA1, &DisassemblerX86::CrackBytes, 0},
    {0x101DA2, &DisassemblerX86::CrackBytes, 0},
    {0x101DA3, &DisassemblerX86::CrackBytes, 0},
    {0x9A4, &DisassemblerX86::CrackBytes, 0},
    {0x9A5, &DisassemblerX86::CrackBytes, 0},
    {0x9A6, &DisassemblerX86::CrackBytes, 0},
    {0x9A7, &DisassemblerX86::CrackBytes, 0},
    {0x12A8, &DisassemblerX86::CrackBytes, 0},
    {0x1DA9, &DisassemblerX86::CrackBytes, 0},
    {0x9AA, &DisassemblerX86::CrackBytes, 0},
    {0x9AB, &DisassemblerX86::CrackBytes, 0},
    {0x9AC, &DisassemblerX86::CrackBytes, 0},
    {0x9AD, &DisassemblerX86::CrackBytes, 0},
    {0x9AE, &DisassemblerX86::CrackBytes, 0},
    {0x9AF, &DisassemblerX86::CrackBytes, 0},
    {0x12B0, &DisassemblerX86::CrackBytes, 0},
    {0x12B1, &DisassemblerX86::CrackBytes, 0},
    {0x12B2, &DisassemblerX86::CrackBytes, 0},
    {0x12B3, &DisassemblerX86::CrackBytes, 0},
    {0x12B4, &DisassemblerX86::CrackBytes, 0},
    {0x12B5, &DisassemblerX86::CrackBytes, 0},
    {0x12B6, &DisassemblerX86::CrackBytes, 0},
    {0x12B7, &DisassemblerX86::CrackBytes, 0},
    {0x1DB8, &DisassemblerX86::CrackBytes, 0},
    {0x1DB9, &DisassemblerX86::CrackBytes, 0},
    {0x1DBA, &DisassemblerX86::CrackBytes, 0},
    {0x1DBB, &DisassemblerX86::CrackBytes, 0},
    {0x1DBC, &DisassemblerX86::CrackBytes, 0},
    {0x1DBD, &DisassemblerX86::CrackBytes, 0},
    {0x1DBE, &DisassemblerX86::CrackBytes, 0},
    {0x1DBF, &DisassemblerX86::CrackBytes, 0},
    {0x5BC0, &DisassemblerX86::CrackBytes, 0},
    {0x5BC1, &DisassemblerX86::CrackBytes, 0},
    {0x1BC2, &DisassemblerX86::CrackBytes, 0},
    {0x9C3, &DisassemblerX86::CrackBytes, 0},
    {0x52C4, &DisassemblerX86::CrackBytes, 0},
    {0x52C5, &DisassemblerX86::CrackBytes, 0},
    {0x5BC6, &DisassemblerX86::CrackBytes, 0},
    {0x66C7, &DisassemblerX86::CrackBytes, 0},
    {0x24C8, &DisassemblerX86::CrackBytes, 0},
    {0x9C9, &DisassemblerX86::CrackBytes, 0},
    {0x1BCA, &DisassemblerX86::CrackBytes, 0},
    {0x9CB, &DisassemblerX86::CrackBytes, 0},
    {0x9CC, &DisassemblerX86::CrackBytes, 0},
    {0x12CD, &DisassemblerX86::CrackBytes, 0},
    {0x9CE, &DisassemblerX86::CrackBytes, 0},
    {0x9CF, &DisassemblerX86::CrackBytes, 0},
    {0x52D0, &DisassemblerX86::CrackBytes, 0},
    {0x52D1, &DisassemblerX86::CrackBytes, 0},
    {0x52D2, &DisassemblerX86::CrackBytes, 0},
    {0x52D3, &DisassemblerX86::CrackBytes, 0},
    {0x12D4, &DisassemblerX86::CrackBytes, 0},
    {0x12D5, &DisassemblerX86::CrackBytes, 0},
    {0x9D6, &DisassemblerX86::Invalid, 0},
    {0x9D7, &DisassemblerX86::CrackBytes, 0},
    {0x52D8, &DisassemblerX86::CrackBytes, 0},
    {0x52D9, &DisassemblerX86::CrackBytes, 0},
    {0x52DA, &DisassemblerX86::CrackBytes, 0},
    {0x52DB, &DisassemblerX86::CrackBytes, 0},
    {0x52DC, &DisassemblerX86::CrackBytes, 0},
    {0x52DD, &DisassemblerX86::CrackBytes, 0},
    {0x52DE, &DisassemblerX86::CrackBytes, 0},
    {0x52DF, &DisassemblerX86::CrackBytes, 0},
    {0x12E0, &DisassemblerX86::CrackBytes, 0},
    {0x12E1, &DisassemblerX86::CrackBytes, 0},
    {0x12E2, &DisassemblerX86::CrackBytes, 0},
    {0x12E3, &DisassemblerX86::CrackBytes, 0},
    {0x12E4, &DisassemblerX86::CrackBytes, 0},
    {0x12E5, &DisassemblerX86::CrackBytes, 0},
    {0x12E6, &DisassemblerX86::CrackBytes, 0},
    {0x12E7, &DisassemblerX86::CrackBytes, 0},
    {0x1DE8, &DisassemblerX86::CrackBytes, 0x47FF},
    {0x1DE9, &DisassemblerX86::CrackBytes, 0},
    {0x2FEA, &DisassemblerX86::CrackBytes, 0},
    {0x12EB, &DisassemblerX86::CrackBytes, 0},
    {0x9EC, &DisassemblerX86::CrackBytes, 0},
    {0x9ED, &DisassemblerX86::CrackBytes, 0},
    {0x9EE, &DisassemblerX86::CrackBytes, 0},
    {0x9EF, &DisassemblerX86::CrackBytes, 0},
    {0x9F0, &DisassemblerX86::CrackBytesPrefix, 0},
    {0x9F1, &DisassemblerX86::Invalid, 0},
    {0x9F2, &DisassemblerX86::CrackBytesPrefix, 0},
    {0x9F3, &DisassemblerX86::CrackBytesPrefix, 0},
    {0x9F4, &DisassemblerX86::CrackBytes, 0},
    {0x9F5, &DisassemblerX86::CrackBytes, 0},
    {0x0F6, &DisassemblerX86::CrackF6, 0},
    {0x0F7, &DisassemblerX86::CrackF7, 0},
    {0x9F8, &DisassemblerX86::CrackBytes, 0},
    {0x9F9, &DisassemblerX86::CrackBytes, 0},
    {0x9FA, &DisassemblerX86::CrackBytes, 0},
    {0x9FB, &DisassemblerX86::CrackBytes, 0},
    {0x9FC, &DisassemblerX86::CrackBytes, 0},
    {0x9FD, &DisassemblerX86::CrackBytes, 0},
    {0x52FE, &DisassemblerX86::CrackBytes, 0},
    {0x52FF, &DisassemblerX86::CrackBytes, 0}
};

DisassemblerX86::DisassemblerX86() : m_dw0(0), m_dw1(0) {}

const gtUByte* DisassemblerX86::CrackInstruction(const gtUByte* pBytes)
{
    if (NULL != pBytes)
    {
        m_hasOperands = false;
        m_opcode = OPCODE_INVALID;
        m_value = VALUE_INVALID;
        m_register = REGISTER_INVALID;

        const OpcodeEntry& entry = s_rceCopyTable[*pBytes];
        pBytes = (this->*(entry.m_pfnCrackBytes))(entry, pBytes);
    }

    return pBytes;
}

unsigned DisassemblerX86::CrackInstructionLength(const gtUByte* pBytes)
{
    return static_cast<unsigned>(DisassemblerX86().CrackInstruction(pBytes) - pBytes);
}

bool DisassemblerX86::CrackInstructionOperands(const gtUByte* pBytes, gtUInt32& value, gtUInt32& reg)
{
    reg = REGISTER_INVALID;
    value = VALUE_INVALID;

    const OpcodeEntry& entry = s_rceCopyTable[*pBytes];
    return CrackInstructionOperands(pBytes, value, reg, entry);
}

unsigned DisassemblerX86::CrackInstructionRegister(const gtUByte* pBytes)
{
    unsigned reg = REGISTER_INVALID;

    OperandInfo operandinfo;
    operandinfo.m_value = s_rceCopyTable[*pBytes].m_operandInfo;

    if (0 != operandinfo.m_hasOperand)
    {
        if (0xFF != operandinfo.m_dataRegister)
        {
            reg = (*reinterpret_cast<const gtUInt32*>(&pBytes[operandinfo.m_offsetRegister]) >> operandinfo.m_shiftRegister) & 0x7;
        }
    }

    return reg;
}

unsigned DisassemblerX86::CrackInstructionOperandValue(const gtUByte* pBytes)
{
    unsigned value = VALUE_INVALID;

    OperandInfo operandinfo;
    operandinfo.m_value = s_rceCopyTable[*pBytes].m_operandInfo;

    if (0 != operandinfo.m_hasOperand)
    {
        if (1 == operandinfo.m_sizeDisplacement)
        {
            value = (*reinterpret_cast<const gtUInt32*>(&pBytes[operandinfo.m_offsetDisplacement]) >> operandinfo.m_shiftDisplacement) & 0xFF;
        }
        else if (3 == operandinfo.m_sizeDisplacement)
        {
            value = *reinterpret_cast<const gtUInt32*>(&pBytes[operandinfo.m_offsetDisplacement]) >> operandinfo.m_shiftDisplacement;
        }
    }

    return value;
}

const gtUByte* DisassemblerX86::CrackBytes(const OpcodeEntry& entry, const gtUByte* pBytes)
{
    unsigned size;
    const OpcodeInfo& opcodeinfo = reinterpret_cast<const OpcodeInfo&>(entry.m_opcodeInfo);

    if ((0 != (entry.m_opcodeInfo & 0x200000) && 0 != m_dw1) || (0 == (entry.m_opcodeInfo & 0x200000) && 0 != m_dw0))
    {
        size = opcodeinfo.m_size2;
    }
    else
    {
        size = opcodeinfo.m_size;
    }

    m_opcode = opcodeinfo.m_opcode;


    unsigned offsetModRm = opcodeinfo.m_offsetModRm;

    if (0 != offsetModRm)
    {
        gtUByte modRm = pBytes[offsetModRm];
        gtUByte infoModRm = s_rbModRm[modRm];

        if (0 != (infoModRm & MODRM_INFO_SIB_NO_BASE_ALLOWED) && SIB_BASE_NONE == (pBytes[offsetModRm + 1] & SIB_BASE_MASK))
        {
            gtUByte mod = modRm & MODRM_MOD_MASK;

            if (MODRM_MOD_01 == mod)
            {
                size += 1; // disp8
            }
            else if (MODRM_MOD_00 == mod || MODRM_MOD_10 == mod)
            {
                size += 4; // disp32
            }
        }

        size += static_cast<unsigned>(infoModRm & MODRM_INFO_SIZE_MASK);
    }

    m_hasOperands = CrackInstructionOperands(pBytes, m_value, m_register, entry);

    return &pBytes[size];
}

bool DisassemblerX86::CrackInstructionOperands(const gtUByte* pBytes, gtUInt32& value, gtUInt32& reg, const OpcodeEntry& entry)
{
    OperandInfo operandinfo;
    operandinfo.m_value = entry.m_operandInfo;

    bool hasOperands = (0 != operandinfo.m_hasOperand);

    if (hasOperands)
    {
        if (0xFF != operandinfo.m_dataRegister)
        {
            reg = (*reinterpret_cast<const gtUInt32*>(&pBytes[operandinfo.m_offsetRegister]) >> operandinfo.m_shiftRegister) & 0x7;
        }

        if (1 == operandinfo.m_sizeDisplacement)
        {
            value = (*reinterpret_cast<const gtUInt32*>(&pBytes[operandinfo.m_offsetDisplacement]) >> operandinfo.m_shiftDisplacement) & 0xFF;
        }
        else if (3 == operandinfo.m_sizeDisplacement)
        {
            value = *reinterpret_cast<const gtUInt32*>(&pBytes[operandinfo.m_offsetDisplacement]) >> operandinfo.m_shiftDisplacement;
        }
    }

    return hasOperands;
}

const gtUByte* DisassemblerX86::Invalid(const OpcodeEntry& entry, const gtUByte* pBytes)
{
    GT_UNREFERENCED_PARAMETER(entry);
    return pBytes + 1;
}

const gtUByte* DisassemblerX86::Crack0F(const OpcodeEntry& entry, const gtUByte* pBytes)
{
    CrackBytes(entry, pBytes++);

    const OpcodeEntry& entryEx = s_rceCopyTable0F[*pBytes];
    return (this->*(entryEx.m_pfnCrackBytes))(entryEx, pBytes);
}

const gtUByte* DisassemblerX86::Crack66(const OpcodeEntry& entry, const gtUByte* pBytes)
{
    m_dw0 = 1;
    CrackBytes(entry, pBytes++);

    const OpcodeEntry& entryEx = s_rceCopyTable[*pBytes];
    return (this->*(entryEx.m_pfnCrackBytes))(entryEx, pBytes);
}

const gtUByte* DisassemblerX86::Crack67(const OpcodeEntry& entry, const gtUByte* pBytes)
{
    m_dw1 = 1;
    CrackBytes(entry, pBytes++);

    const OpcodeEntry& entryEx = s_rceCopyTable[*pBytes];
    return (this->*(entryEx.m_pfnCrackBytes))(entryEx, pBytes);
}

const gtUByte* DisassemblerX86::CrackF6(const OpcodeEntry& entry, const gtUByte* pBytes)
{
    GT_UNREFERENCED_PARAMETER(entry);

    const OpcodeEntry entryNew = {(0 != (pBytes[1] & 0x38)) ? 0x52F6U : 0x5BF6U, &DisassemblerX86::CrackBytes, 0};
    return CrackBytes(entryNew, pBytes);
}

const gtUByte* DisassemblerX86::CrackF7(const OpcodeEntry& entry, const gtUByte* pBytes)
{
    GT_UNREFERENCED_PARAMETER(entry);

    const OpcodeEntry entryNew = {(0 != (pBytes[1] & 0x38)) ? 0x52F7U : 0x66F7U, &DisassemblerX86::CrackBytes, 0};
    return CrackBytes(entryNew, pBytes);
}

const gtUByte* DisassemblerX86::CrackBytesPrefix(const OpcodeEntry& entry, const gtUByte* pBytes)
{
    CrackBytes(entry, pBytes++);

    const OpcodeEntry& entryEx = s_rceCopyTable[*pBytes];
    return (this->*(entryEx.m_pfnCrackBytes))(entryEx, pBytes);
}
