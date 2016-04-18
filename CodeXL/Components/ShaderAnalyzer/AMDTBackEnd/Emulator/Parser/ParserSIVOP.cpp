#include "ParserSIVOP.h"

ParserSI::kaStatus
ParserSIVOP::Parse(GDT_HW_GENERATION hwGen, Instruction::instruction32bit hexInstruction, Instruction*& instruction, bool, uint32_t, int iLabel /*=NO_LABEL*/ , int iGotoLabel /*=NO_LABEL*/)
{
    kaStatus retStatus =   ParserSI::Status_32BitInstructionNotSupported;
    VOPInstruction::Encoding encoding = GetInstructionType(hexInstruction);


    if ((hwGen == GDT_HW_GENERATION_SEAISLAND) || (hwGen == GDT_HW_GENERATION_SOUTHERNISLAND))
    {
        if (VOPInstruction::Encoding_VOP1 == encoding)
        {
            uint64_t hexInstTem = hexInstruction << 15;
            hexInstTem = hexInstTem >> 24;
            SIVOP1Instruction::VOP1_OP op1 = static_cast<SIVOP1Instruction::VOP1_OP>(hexInstTem);
            instruction = new SIVOP1Instruction(32, encoding, op1, iLabel, iGotoLabel);
            retStatus = ParserSI::Status_SUCCESS;
        }
        else if (VOPInstruction::Encoding_VOP2 == encoding)
        {
            uint64_t hexInstTem = hexInstruction << 15;
            hexInstTem = hexInstTem >> 24;
            SIVOP2Instruction::VOP2_OP op2 = static_cast<SIVOP2Instruction::VOP2_OP>(hexInstTem);
            instruction = new SIVOP2Instruction(32, encoding, op2, iLabel, iGotoLabel);
            retStatus = ParserSI::Status_SUCCESS;
        }

        else if (VOPInstruction::Encoding_VOPC == encoding)
        {
            uint64_t hexInstTem = hexInstruction << 15;
            hexInstTem = hexInstTem >> 24;
            SIVOPCInstruction::VOPC_OP opc = static_cast<SIVOPCInstruction::VOPC_OP>(hexInstTem);
            instruction = new SIVOPCInstruction(32, encoding, opc, iLabel, iGotoLabel);
            retStatus = ParserSI::Status_SUCCESS;
        }
    }
    else
    {
        if (VOPInstruction::Encoding_VOP1 == encoding)
        {
            uint64_t hexInstTem = hexInstruction << 15;
            hexInstTem = hexInstTem >> 24;
            VIVOP1Instruction::VOP1_OP op1 = static_cast<VIVOP1Instruction::VOP1_OP>(hexInstTem);
            instruction = new VIVOP1Instruction(32, encoding, op1, iLabel, iGotoLabel);
            retStatus = ParserSI::Status_SUCCESS;
        }
        else if (VOPInstruction::Encoding_VOP2 == encoding)
        {
            uint64_t hexInstTem = hexInstruction << 15;
            hexInstTem = hexInstTem >> 24;
            VIVOP2Instruction::VOP2_OP op2 = static_cast<VIVOP2Instruction::VOP2_OP>(hexInstTem);
            instruction = new VIVOP2Instruction(32, encoding, op2, iLabel, iGotoLabel);
            retStatus = ParserSI::Status_SUCCESS;
        }

        else if (VOPInstruction::Encoding_VOPC == encoding)
        {
            uint64_t hexInstTem = hexInstruction << 15;
            hexInstTem = hexInstTem >> 24;
            VIVOPCInstruction::VOPC_OP opc = static_cast<VIVOPCInstruction::VOPC_OP>(hexInstTem);
            instruction = new VIVOPCInstruction(32, encoding, opc, iLabel, iGotoLabel);
            retStatus = ParserSI::Status_SUCCESS;
        }
    }

    return retStatus;
}

ParserSI::kaStatus
ParserSIVOP::Parse(GDT_HW_GENERATION hwGen, Instruction::instruction64bit hexInstruction, Instruction*& instruction, int iLabel /*=NO_LABEL*/ , int iGotoLabel /*=NO_LABEL*/)
{
    (void)(hwGen); // Unreferenced parameter

    kaStatus retStatus =   ParserSI::Status_64BitInstructionNotSupported;
    VOPInstruction::Encoding encoding = GetInstructionType(hexInstruction);


    if (VOPInstruction::Encoding_VOP3 == encoding)
    {
        uint64_t hexInstTem = hexInstruction << 15;
        hexInstTem = hexInstTem >> 24;
        SIVOP3Instruction::VOP3_OP op3 = static_cast<SIVOP3Instruction::VOP3_OP>(hexInstTem);
        instruction = new SIVOP3Instruction(64, encoding, op3, iLabel, iGotoLabel);
        retStatus =  ParserSI::Status_SUCCESS;
    }

    return retStatus;
}

VOPInstruction::Encoding
ParserSIVOP::GetInstructionType(Instruction::instruction32bit hexInstruction)
{
    uint64_t hexInstTemp = hexInstruction >> 25;

    if (hexInstTemp == VOPInstruction::Encoding_VOP1)
    {
        return VOPInstruction::Encoding_VOP1;
    }
    else if (hexInstTemp == VOPInstruction::VOPMask_VOPC)
    {
        return VOPInstruction::Encoding_VOPC;
    }

    hexInstTemp = hexInstruction  >> 31;

    if (hexInstTemp == VOPInstruction::VOPMask_VOP2)
    {
        return VOPInstruction::Encoding_VOP2;
    }

    return VOPInstruction::Encoding_Illegal;
}

VOPInstruction::Encoding
ParserSIVOP::GetInstructionType(Instruction::instruction64bit hexInstruction)
{
    if ((hexInstruction && VOPInstruction::VOPMask_VOP3) >> 26 == VOPInstruction::Encoding_VOP3)
    {
        return VOPInstruction::Encoding_VOP3;
    }

    return VOPInstruction::Encoding_Illegal;
}
