//=============================================================
// Copyright (c) 2013 Advanced Micro Devices, Inc.
//=============================================================

#ifndef __PARSESIVOP_H
#define __PARSESIVOP_H

#include "ParserSI.h"

/// -----------------------------------------------------------------------------------------------
/// \class Name: ParserSIVOP : public ParserSI { };
/// \brief Description:  Parser for the Southern Island [SI] VOP instructions
/// -----------------------------------------------------------------------------------------------
class ParserSIVOP : public ParserSI
{
public:
    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        Parse
    /// \brief Description: Parse the 32-bit VOP instruction (Suits VOP1,VOP2,VOPC)
    /// \param[in]          hexInstruction
    /// \param[in]          instruction
    /// \param[in]          isLiteral32b
    /// \param[in]          literal32b
    /// \return ParserSI::Status
    /// -----------------------------------------------------------------------------------------------
    ParserSI::kaStatus Parse(GDT_HW_GENERATION hwGen, Instruction::instruction32bit hexInstruction, Instruction*& instruction, bool isLiteral32b, uint32_t literal32b, int iLabel = NO_LABEL , int iGotoLabel = NO_LABEL);
    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        Parse
    /// \brief Description: Parse the 64-bit VOP instruction (Suits 2 kinds of VOP3)
    /// \param[in]          hexInstruction
    /// \param[in]          instruction
    /// \return ParserSI::Status
    /// -----------------------------------------------------------------------------------------------
    ParserSI::kaStatus Parse(GDT_HW_GENERATION hwGen, Instruction::instruction64bit hexInstruction, Instruction*& instruction, int iLabel = NO_LABEL , int iGotoLabel = NO_LABEL);
    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        ParserSIVOP
    /// \brief Description: ctor
    /// \return
    /// -----------------------------------------------------------------------------------------------
    ParserSIVOP() { }
    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        ~ParserSIVOP
    /// \brief Description: dtor
    /// \return
    /// -----------------------------------------------------------------------------------------------
    ~ParserSIVOP() { }
private:

    //
    // Private member functions
    //
    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        GetInstructionType
    /// \brief Description: Get VOP Instruction type
    /// \param[in]          hexInstruction
    /// \return VOPInstruction::InstructionType
    /// -----------------------------------------------------------------------------------------------
    static VOPInstruction::Encoding GetInstructionType(Instruction::instruction32bit hexInstruction);
    /// -----------------------------------------------------------------------------------------------
    /// \brief Name:        GetInstructionType
    /// \brief Description: Get VOP Instruction type
    /// \param[in]          hexInstruction
    /// \return VOPInstruction::InstructionType
    /// -----------------------------------------------------------------------------------------------
    static VOPInstruction::Encoding GetInstructionType(Instruction::instruction64bit hexInstruction);

};

#endif //__PARSESIVOP_H

