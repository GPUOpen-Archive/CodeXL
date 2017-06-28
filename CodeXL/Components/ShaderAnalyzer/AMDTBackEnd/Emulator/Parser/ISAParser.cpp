//=============================================================
// Copyright (c) 2013 Advanced Micro Devices, Inc.
//
/// \file   ISAParser.cpp
/// \author GPU Developer Tools
/// \version $Revision: $
/// \brief Description: Parser for the ISA
//
//=============================================================
// $Id: $
// Last checkin:   $DateTime: $
// Last edited by: $Author: $
// Change list:    $Change: $
//=============================================================

// C++.
#include <fstream>
#include <string>

// Boost.
#include <boost/regex.hpp>

#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local.
#include <Include/beStringConstants.h>
#include "ISAParser.h"
#include "ParserSISOP2.h"
#include "ParserSISOPK.h"
#include "ParserSISOP1.h"
#include "ParserSISOPP.h"
#include "ParserSISOPC.h"
#include "ParserSISMRD.h"
#include "ParserSIVINTRP.h"
#include "ParserSIDS.h"
#include "ParserSIMUBUF.h"
#include "ParserSIMTBUF.h"
#include "ParserSIMIMG.h"
#include "ParserSIEXP.h"
#include "ParserSIVOP.h"
#include "ParserFLAT.h"

// *** INTERNALLY-LINKED AUXILIARY FUNCTIONS - BEGIN ***

static bool ExtractRuntimeChangedNumOfGprs(const std::string& isaLine, unsigned int& numOfGPRs)
{
    bool ret = false;
    const std::string CHANGED_BY_RT_TOKEN = "modified by runtime to be ";
    size_t posBegin = isaLine.find(CHANGED_BY_RT_TOKEN);

    if (posBegin != std::string::npos)
    {
        // Handles the case where the number of SGPRs was changed by the runtime.
        posBegin += CHANGED_BY_RT_TOKEN.size();
        size_t posEnd = isaLine.find(";", posBegin);

        if (posEnd > posBegin)
        {
            const std::string numAsText = isaLine.substr(posBegin, posEnd - posBegin);
            numOfGPRs = std::stoul(numAsText);
            ret = true;
        }
    }

    return ret;
}

// Trim from the start.
static std::string& trimStart(std::string& strToTrim)
{
    strToTrim.erase(strToTrim.begin(), std::find_if(strToTrim.begin(),
                                                    strToTrim.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return strToTrim;
}

// Trim from the end.
static std::string& trimEnd(std::string& strToTrim)
{
    strToTrim.erase(std::find_if(strToTrim.rbegin(), strToTrim.rend(),
                                 std::not1(std::ptr_fun<int, int>(std::isspace))).base(), strToTrim.end());
    return strToTrim;
}

// Trim from both sides.
static std::string trimStr(const std::string& strToTrim)
{
    std::string ret = strToTrim;
    return trimStart(trimEnd(ret));
}

// Split the given instruction into its building blocks: opcode, operands, binary representation and offset.
static bool ExtractBuildingBlocks(const std::string& isaInstruction, std::string& instrOpCode,
                                  std::string& params, std::string& binaryRepresentation, std::string& offset)
{
    bool ret = false;

    // Clear the white spaces.
    std::string trimmedInstr = trimStr(isaInstruction);

    // Find the first white space.
    size_t beginIndex = trimmedInstr.find(' ');

    if (beginIndex != std::string::npos)
    {
        // Extract the instruction.
        instrOpCode = trimmedInstr.substr(0, beginIndex);

        // Extract the parameters.
        size_t endIndex = trimmedInstr.find("//");

        if (endIndex != std::string::npos)
        {
            size_t substrLength = endIndex - beginIndex;

            if (substrLength > 0)
            {
                params = trimmedInstr.substr(beginIndex, substrLength);

                // Clear white spaces.
                trimStart(trimEnd(params));


                // Extract the offset.
                beginIndex = endIndex + 3;
                endIndex = trimmedInstr.find(':', endIndex);

                if (beginIndex != std::string::npos)
                {
                    substrLength = endIndex - beginIndex;

                    if (substrLength > 0)
                    {
                        offset = trimmedInstr.substr(beginIndex, substrLength);

                        // Extract the binary representation.
                        binaryRepresentation = trimmedInstr.substr(endIndex + 2);

                        // We are done.
                        ret = true;
                    }
                }

            }
        }
    }

    return ret;
}

// *** INTERNALLY-LINKED AUXILIARY FUNCTIONS - END ***


ParserISA::ParserISA(ParserSI::LoggingCallBackFuncP logFunc) : m_sgprs(0), m_vgprs(0), m_CodeLen(0)
{
    ParserSI::SetLog(logFunc);
    m_parsersSI[Instruction::InstructionSet_SOP2] = new ParserSISOP2();
    m_parsersSI[Instruction::InstructionSet_SOPK] = new ParserSISOPK();
    m_parsersSI[Instruction::InstructionSet_SOP1] = new ParserSISOP1();
    m_parsersSI[Instruction::InstructionSet_SOPC] = new ParserSISOPC();
    m_parsersSI[Instruction::InstructionSet_SOPP] = new ParserSISOPP();
    m_parsersSI[Instruction::InstructionSet_SMRD] = new ParserSISMRD();
    m_parsersSI[Instruction::InstructionSet_VINTRP] = new ParserSIVINTRP();
    m_parsersSI[Instruction::InstructionSet_DS] = new ParserSIDS();
    m_parsersSI[Instruction::InstructionSet_MUBUF] = new ParserSIMUBUF();
    m_parsersSI[Instruction::InstructionSet_MTBUF] = new ParserSIMTBUF();
    m_parsersSI[Instruction::InstructionSet_MIMG] = new ParserSIMIMG();
    m_parsersSI[Instruction::InstructionSet_EXP] = new ParserSIEXP();
    m_parsersSI[Instruction::InstructionSet_VOP] = new ParserSIVOP();
    m_parsersSI[Instruction::InstructionSet_FLAT] = new ParserFLAT();
}

ParserISA::~ParserISA()
{
    ParserSI::SetLog(NULL);
    delete m_parsersSI[Instruction::InstructionSet_SOP2];
    delete m_parsersSI[Instruction::InstructionSet_SOPK];
    delete m_parsersSI[Instruction::InstructionSet_SOP1];
    delete m_parsersSI[Instruction::InstructionSet_SOPC];
    delete m_parsersSI[Instruction::InstructionSet_SOPP];
    delete m_parsersSI[Instruction::InstructionSet_SMRD];
    delete m_parsersSI[Instruction::InstructionSet_VINTRP];
    delete m_parsersSI[Instruction::InstructionSet_DS];
    delete m_parsersSI[Instruction::InstructionSet_MUBUF];
    delete m_parsersSI[Instruction::InstructionSet_MTBUF];
    delete m_parsersSI[Instruction::InstructionSet_MIMG];
    delete m_parsersSI[Instruction::InstructionSet_EXP];
    delete m_parsersSI[Instruction::InstructionSet_VOP];
    delete m_parsersSI[Instruction::InstructionSet_FLAT];
}

bool ParserISA::Parse(const std::string& isaLine, GDT_HW_GENERATION asicGen,
                      Instruction::instruction32bit hexInstruction, bool isLiteral32b,
                      uint32_t literal32b, int iLabel /*=NO_LABEL*/,
                      int iGotoLabel /*=NO_LABEL*/, int iLineCount/* = 0*/)
{
    bool ret = false;
    Instruction* pInstruction = NULL;
    ParserSI::InstructionEncoding instructionEncoding = ParserSI::GetInstructionEncoding(hexInstruction);

    if (instructionEncoding == ParserSI::InstructionEncoding_SOP2)
    {
        m_parsersSI[Instruction::InstructionSet_SOP2]->Parse(asicGen, hexInstruction, pInstruction, isLiteral32b, literal32b, iLabel, iGotoLabel);
    }
    else if (instructionEncoding == ParserSI::InstructionEncoding_SOPK)
    {
        m_parsersSI[Instruction::InstructionSet_SOPK]->Parse(asicGen, hexInstruction, pInstruction, isLiteral32b, literal32b, iLabel, iGotoLabel);
    }
    else if (instructionEncoding == ParserSI::InstructionEncoding_SOP1)
    {
        m_parsersSI[Instruction::InstructionSet_SOP1]->Parse(asicGen, hexInstruction, pInstruction, isLiteral32b, literal32b, iLabel, iGotoLabel);
    }
    else if (instructionEncoding == ParserSI::InstructionEncoding_SOPC)
    {
        m_parsersSI[Instruction::InstructionSet_SOPC]->Parse(asicGen, hexInstruction, pInstruction, isLiteral32b, literal32b, iLabel, iGotoLabel);
    }
    else if (instructionEncoding == ParserSI::InstructionEncoding_SOPP)
    {
        m_parsersSI[Instruction::InstructionSet_SOPP]->Parse(asicGen, hexInstruction, pInstruction, isLiteral32b, literal32b, iLabel, iGotoLabel);
    }
    else if ((instructionEncoding == ParserSI::InstructionEncoding_SMRD) || (instructionEncoding == ParserSI::VIInstructionEncoding_SMEM))
    {
        m_parsersSI[Instruction::InstructionSet_SMRD]->Parse(asicGen, hexInstruction, pInstruction, isLiteral32b, literal32b, iLabel, iGotoLabel);
    }
    else if ((instructionEncoding == ParserSI::InstructionEncoding_VOP2) ||
             (instructionEncoding == ParserSI::InstructionEncoding_VOP1) ||
             (instructionEncoding == ParserSI::InstructionEncoding_VOPC))
    {
        m_parsersSI[Instruction::InstructionSet_VOP]->Parse(asicGen, hexInstruction, pInstruction, isLiteral32b, literal32b, iLabel, iGotoLabel);
    }
    else if ((instructionEncoding == ParserSI::InstructionEncoding_VINTRP) ||
             (instructionEncoding == ParserSI::VIInstructionEncoding_VINTRP))
    {
        m_parsersSI[Instruction::InstructionSet_VINTRP]->Parse(asicGen, hexInstruction, pInstruction, isLiteral32b, literal32b, iLabel, iGotoLabel);
    }

    if (nullptr == pInstruction)
    {
        gtString isaLineStr;
        isaLineStr.fromASCIIString(isaLine.c_str(), (int)isaLine.length());
        gtString logMsg(BE_STR_FAILED_TO_PARSE_ISA_LINE);
        logMsg += isaLineStr;
        OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_ERROR);

        // Push an instruction of an arbitrary type into the collection so the ISA view can display the text of this instruction.
        // The textual part that is displayed in the ISA view is added in the next if block below.
        pInstruction = new SIVOP1Instruction(32, VOPInstruction::Encoding_VOP1, SIVOP1Instruction::V_NOP, NO_LABEL, NO_LABEL);
    }

    if (pInstruction != NULL)
    {
        pInstruction->SetLineNumber(iLineCount);
        m_instructions.push_back(pInstruction);

        std::string opcode;
        std::string params;
        std::string binaryRepresentation;
        std::string offset;

        ret = ExtractBuildingBlocks(isaLine, opcode, params, binaryRepresentation, offset);

        if (ret)
        {
            // Set the ISA instruction's string representation.
            pInstruction->SetInstructionStringRepresentation(opcode, params, binaryRepresentation, offset);
        }
    }

    return ret;
}

bool ParserISA::Parse(const std::string& isaLine, GDT_HW_GENERATION asicGen,
                      Instruction::instruction64bit hexInstruction, int iLabel /*=NO_LABEL*/,
                      int iGotoLabel /*=NO_LABEL*/, int iLineCount /*= 0*/)
{
    Instruction* pInstruction = NULL;
    /// Instruction encoding appears only in low 32 bits of instruction
    ParserSI::InstructionEncoding instructionEncoding = ParserSI::GetInstructionEncoding(static_cast<Instruction::instruction32bit>(hexInstruction));

    switch (instructionEncoding)
    {
        case ParserSI::InstructionEncoding_DS:
            m_parsersSI[Instruction::InstructionSet_DS]->Parse(asicGen, hexInstruction, pInstruction, iLabel, iGotoLabel);
            break;

        case ParserSI::InstructionEncoding_MUBUF:
            m_parsersSI[Instruction::InstructionSet_MUBUF]->Parse(asicGen, hexInstruction, pInstruction, iLabel, iGotoLabel);
            break;

        case ParserSI::InstructionEncoding_MIMG:
            m_parsersSI[Instruction::InstructionSet_MIMG]->Parse(asicGen, hexInstruction, pInstruction, iLabel, iGotoLabel);
            break;

        case ParserSI::InstructionEncoding_MTBUF:
            m_parsersSI[Instruction::InstructionSet_MTBUF]->Parse(asicGen, hexInstruction, pInstruction, iLabel, iGotoLabel);
            break;

        case ParserSI::InstructionEncoding_EXP:
            m_parsersSI[Instruction::InstructionSet_EXP]->Parse(asicGen, hexInstruction, pInstruction, iLabel, iGotoLabel);
            break;

        case ParserSI::InstructionEncoding_VOP3:
            m_parsersSI[Instruction::InstructionSet_VOP]->Parse(asicGen, hexInstruction, pInstruction, iLabel, iGotoLabel);
            break;

        case ParserSI::VIInstructionEncoding_SMEM:
            m_parsersSI[Instruction::InstructionSet_SMRD]->Parse(asicGen, hexInstruction, pInstruction, iLabel, iGotoLabel);
            break;

        case ParserSI::VIInstructionEncoding_FLAT:
            m_parsersSI[Instruction::InstructionSet_FLAT]->Parse(asicGen, hexInstruction, pInstruction, iLabel, iGotoLabel);
            break;

        default:
            break;
    }

    if (nullptr == pInstruction)
    {
        gtString isaLineStr;
        isaLineStr.fromASCIIString(isaLine.c_str(), (int)isaLine.length());
        gtString logMsg(BE_STR_FAILED_TO_PARSE_ISA_LINE);
        logMsg += isaLineStr;
        OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_ERROR);

        // Push an instruction of an arbitrary type into the collection so the ISA view can display the text of this instruction.
        // The textual part that is displayed in the ISA view is added in the next if block below.
        pInstruction = new SIVOP1Instruction(32, VOPInstruction::Encoding_VOP1, SIVOP1Instruction::V_NOP, NO_LABEL, NO_LABEL);
    }

    if (pInstruction != NULL)
    {
        if (NO_LABEL == pInstruction->GetLabel())
        {
            pInstruction->SetLineNumber(iLineCount);
            m_instructions.push_back(pInstruction);

            // Set the ISA instruction's string representation.
            std::string opcode;
            std::string params;
            std::string binaryRepresentation;
            std::string offset;
            bool ret = ExtractBuildingBlocks(isaLine, opcode, params, binaryRepresentation, offset);

            if (ret)
            {
                pInstruction->SetInstructionStringRepresentation(opcode, params, binaryRepresentation, offset);
            }
        }
    }

    // *****************************************************************
    // TODO: HANDLE SCENARIOS WHERE THE INSTRUCTION IS NOT BEING PARSED.
    //       FOR NOW: ALWAYS RETURN 'TRUE'.
    // *****************************************************************
    return true;
}

bool ParserISA::Parse(const std::string& isa)
{
    ResetInstsCounters();

    bool bRet = ParseToVector(isa);

    if (bRet)
    {
        bRet = m_pIsaTree.BuildISAProgramStructure(m_instructions);
    }

    return bRet;
}

bool ParserISA::ParseForSize(const std::string& isa)
{
    bool retVal = false;

    ResetInstsCounters();
    boost::regex codeLenInByteEx("([[:blank:]]*codeLenInByte[[:blank:]]*=[[:blank:]]*)([[:digit:]]*)");
    boost::regex codeLenInByteNI("([[:blank:]]*CodeLen[[:blank:]]*=[[:blank:]]*)([[:digit:]]*)");
    // v_add_u32     v1, s[2:3], v0, s0    // 000000000130: D1190201 00000100   <--- 64-bit instruction
    // v_mov_b32     v0, 0                 // 000000000138: 7E000280            <--- 32-bit instruction
    boost::regex instAnnotation("//[[:blank:]]*[[:xdigit:]]{12}:[[:blank:]]*([[:xdigit:]]{8})([[:blank:]]+[[:xdigit:]]{8}){0,1}");

    std::istringstream isaStream(isa);
    boost::smatch matchInst;
    std::string isaLine;
    int  isaSize = 0;

    while (getline(isaStream, isaLine))
    {
        if (boost::regex_search(isaLine, matchInst, codeLenInByteEx))
        {
            std::string codeLenText(matchInst[2].first, matchInst[2].second);
            m_CodeLen = atoi(codeLenText.c_str());
            retVal = true;
            break;
        }
        else if (boost::regex_search(isaLine, matchInst, codeLenInByteNI))
        {
            std::string codeLenText(matchInst[2].first, matchInst[2].second);
            m_CodeLen = atoi(codeLenText.c_str());
            retVal = true;
            break;
        }
        else if (boost::regex_search(isaLine, matchInst, instAnnotation))
        {
            // Count size of instructions "manually" if ISA size is not provided by disassembler.
            int  instSize = matchInst[(int) matchInst.size() - 1].matched ? 8 : 4;
            isaSize += instSize;
            retVal = true;
        }
    }

    if (retVal && isaSize != 0)
    {
        m_CodeLen = isaSize;
    }

    return retVal;
}

void ParserISA::GetNumOfInstructionsInCategory(ISAProgramGraph::NumOfInstructionsInCategory NumOfInstructionsInCategory[ISAProgramGraph::CALC_NUM_OF_PATHES], std::string sDumpGraph)
{
    m_pIsaTree.GetNumOfInstructionsInCategory(NumOfInstructionsInCategory, sDumpGraph);
}

bool ParserISA::ParseToVector(const std::string& isa)
{
    int iLineCount = 0;
    Instruction::instruction32bit inst32;
    Instruction::instruction64bit inst64;

    std::istringstream isaStream(isa);
    std::string isaLine;
    bool isaCodeProc = false, parseOK = true, gprProc = false, vgprFound = false, sgprFound = false, codeLenFound = false;
    int iLabel = NO_LABEL, iGotoLabel = NO_LABEL;

    boost::smatch matchInst;

    boost::regex inst32_48Ex("([[:print:]]*// [[:print:]]{12}: )([[:print:]]{8})");
    boost::regex inst64_48Ex("([[:print:]]*// [[:print:]]{12}: )([[:print:]]{8})( )([[:print:]]{8})");

    boost::regex inst32Ex("([[:print:]]*// [[:print:]]{8}: )([[:print:]]{8})");
    boost::regex inst64Ex("([[:print:]]*// [[:print:]]{8}: )([[:print:]]{8})( )([[:print:]]{8})");
    boost::regex vgprsEx("([[:blank:]]*NumVgprs[[:blank:]]*=[[:blank:]]*)([[:digit:]]*)");
    boost::regex sgprsEx("([[:blank:]]*NumSgprs[[:blank:]]*=[[:blank:]]*)([[:digit:]]*)");
    boost::regex codeLenInByteEx("([[:blank:]]*codeLenInByte[[:blank:]]*=[[:blank:]]*)([[:digit:]]*)");
    boost::regex codeLenInByteNI("([[:blank:]]*CodeLen[[:blank:]]*=[[:blank:]]*)([[:digit:]]*)");

    std::string isaStart;
    std::string isaEnd;

    if (isa.find("Disassembly --------------------") != std::string::npos)
    {
        // OCL or non DX
        /// ISA SI + "starts" from "; -------- Disassembly --------------------"
        /// ISA NI "starts" from "; --------  Disassembly --------------------" notice extra space
        isaStart = "Disassembly --------------------";

        /// ISA "ends" with "; ----------------- CS Data ------------------------"
        isaEnd = "; ----------------- CS Data ------------------------";

        // For Vulkan, we don't have the CS Data section.
        if (isa.find(isaEnd) == std::string::npos)
        {
            isaEnd = "end";
        }
    }
    else if (isa.find("&__OpenCL_") != std::string::npos)
    {
        // Shader entry point in HSAIL disassembly.
        isaStart = "Disassembly for ";

        // End of shader token in HSAIL disassembly.
        isaEnd = "end";
    }
    else
    {
        // beginning of shader
        isaStart = "shader ";

        // parse to the end
        isaEnd = "end";
    }

    GDT_HW_GENERATION asicGen = GDT_HW_GENERATION_NONE;
    /// Asic generation is in "asci("
    const std::string asicGenStr("asic(");

    while (getline(isaStream, isaLine))
    {
        iLineCount++;

        if (!isaCodeProc && !gprProc && strstr(isaLine.c_str(), isaStart.c_str()) == NULL)
        {
            continue;
        }
        else if (!isaCodeProc && !gprProc)
        {
            isaCodeProc = true;
        }
        else if (isaCodeProc && strstr(isaLine.c_str(), isaEnd.c_str()) != NULL
                 && isaLine.find("//") == std::string::npos)
        {
            // at least one line of valid code detected
            gprProc = true;
            isaCodeProc = false;
        }

        else if (isaCodeProc)
        {
            /// check generation first
            size_t iPos = isaLine.find(asicGenStr);

            if (iPos != std::string::npos)
            {
                std::string sTemp = isaLine.substr(iPos + 5, 2);

                if (sTemp == "SI")
                {
                    asicGen = GDT_HW_GENERATION_SOUTHERNISLAND;
                }
                else if (sTemp == "CI")
                {
                    asicGen = GDT_HW_GENERATION_SEAISLAND;    // we consider SI and CI to be the same when parsing
                }
                else if (sTemp == "VI")
                {
                    asicGen = GDT_HW_GENERATION_VOLCANICISLAND;
                }
            }

            std::stringstream instStream;
            std::string::const_iterator isaLineStart = isaLine.begin();
            std::string::const_iterator isaLineEnd   = isaLine.end();
            bool instParseOK = true;

            if (iLabel == NO_LABEL)
            {
                iLabel = GetLabel(isaLine);
            }

            iGotoLabel = GetGotoLabel(isaLine);

            if ((boost::regex_search(isaLineStart, isaLineEnd, matchInst, inst64Ex)) ||
                (boost::regex_search(isaLineStart, isaLineEnd, matchInst, inst64_48Ex)))
            {
                // This is either inst64Ex or inst64_48Ex.
                std::string inst32TextLow(matchInst[2].first, matchInst[2].second);
                std::string inst32TextHigh(matchInst[4].first, matchInst[4].second);
                instStream << std::hex << inst32TextHigh << inst32TextLow;
                instStream >> inst64;
                instParseOK = Parse(isaLine, asicGen, inst64, iLabel, iGotoLabel, iLineCount);
                iLabel = iGotoLabel = NO_LABEL;

                if (!instParseOK)
                {
                    uint32_t literal32b;

                    instStream.clear();
                    instStream << std::hex << inst32TextLow;
                    instStream >> inst32;

                    instStream.clear();
                    instStream << std::hex << inst32TextHigh;
                    instStream >> literal32b;

                    instParseOK = Parse(isaLine, asicGen, inst32, true, literal32b, iLabel, iGotoLabel, iLineCount);
                    iLabel = iGotoLabel = NO_LABEL;

                }
            }
            else if (instParseOK && (boost::regex_search(isaLineStart, isaLineEnd, matchInst, inst32Ex) ||
                                     boost::regex_search(isaLineStart, isaLineEnd, matchInst, inst32_48Ex)))
            {
                // This is either inst32Ex or inst32_48Ex.
                std::string inst32Text(matchInst[2].first, matchInst[2].second);

                instStream << std::hex << inst32Text;
                instStream >> inst32;
                instParseOK = Parse(isaLine, asicGen, inst32 , false, 0, iLabel, iGotoLabel, iLineCount);
                iLabel = iGotoLabel = NO_LABEL;
            }
            else if (iLabel != NO_LABEL)
            {
                Instruction* pInstruction = nullptr;
                std::string trimmedIsaLine = trimStr(isaLine);
                pInstruction = new Instruction(trimmedIsaLine);
                m_instructions.push_back(pInstruction);
                iLabel = iGotoLabel = NO_LABEL;
            }

            parseOK &= instParseOK;
        }
        else if (sgprFound && vgprFound && codeLenFound)
        {
            break;
        }

        // Danana- do we want it here? this is part of the statistics, not analysis.
        // but a decision was made that we want it as part of the analysis as well, so I keep it until asked by the backend managerr
        else if (gprProc)
        {
            std::string::const_iterator isaLineStart = isaLine.begin();
            std::string::const_iterator isaLineEnd   = isaLine.end();

            if (boost::regex_search(isaLineStart, isaLineEnd, matchInst, vgprsEx))
            {
                // Mark the VGPR section as found.
                vgprFound = true;
                m_vgprs = 0;

                // Check if the number of VGPRs was changed by the runtime.
                bool isChangedByRuntime = ExtractRuntimeChangedNumOfGprs(isaLine, m_vgprs);

                if (!isChangedByRuntime)
                {
                    // If the value was not changed, extract the original value.
                    std::string vgprsText(matchInst[2].first, matchInst[2].second);
                    m_vgprs = atoi(vgprsText.c_str());
                }
            }
            else if (boost::regex_search(isaLineStart, isaLineEnd, matchInst, sgprsEx))
            {
                // Mark the SGPR section as found.
                sgprFound = true;
                m_sgprs = 0;

                // Check if the number of SGPRs was changed by the runtime.
                bool isChangedByRuntime = ExtractRuntimeChangedNumOfGprs(isaLine, m_sgprs);

                if (!isChangedByRuntime)
                {
                    // If the value was not changed, extract the original value.
                    std::string sgprsText(matchInst[2].first, matchInst[2].second);
                    m_sgprs = atoi(sgprsText.c_str());
                }
            }
            else if (boost::regex_search(isaLineStart, isaLineEnd, matchInst, codeLenInByteEx))
            {
                codeLenFound = true;
                std::string codeLenText(matchInst[2].first, matchInst[2].second);
                m_CodeLen = atoi(codeLenText.c_str());
            }
            else if (boost::regex_search(isaLineStart, isaLineEnd, matchInst, codeLenInByteNI))
            {
                codeLenFound = true;
                std::string codeLenText(matchInst[2].first, matchInst[2].second);
                m_CodeLen = atoi(codeLenText.c_str());
            }
        }
    }

    return parseOK;
}

void ParserISA::ResetInstsCounters()
{
    std::vector<Instruction*>::const_iterator instBegin = m_instructions.begin();

    for (; instBegin != m_instructions.end(); ++instBegin)
    {
        if (NULL != (*instBegin))
        {
            delete *instBegin;
        }
    }

    m_instructions.clear();
    m_sgprs = 0;
    m_vgprs = 0;

    m_pIsaTree.DestroyISAProgramStructure();

}

int ParserISA::GetLabel(const std::string& sISALine)
{
    int iRet = NO_LABEL;
    int iLocation = (int)sISALine.find("label");
    const int HSAIL_ISA_OFFSET = 2;
    int offset = 0;

    if (iLocation == HSAIL_ISA_OFFSET)
    {
        offset = HSAIL_ISA_OFFSET;
    }

    if (iLocation == 0 || iLocation == HSAIL_ISA_OFFSET)
    {
        std::string labelText(sISALine.substr(6 + offset, sISALine.length() - 7));
        std::stringstream instStream;
        instStream << std::hex << labelText;
        instStream >> iRet;
    }

    return iRet;
}

int ParserISA::GetGotoLabel(const std::string& sISALine)
{
    int iRet = NO_LABEL;
    size_t iLocation = (int)sISALine.find("label_");

    if (iLocation != std::string::npos)
    {
        std::string labelText(sISALine.substr(iLocation + 6, 4));
        std::stringstream instStream;
        instStream << std::hex << labelText;
        instStream >> iRet;
    }

    return iRet;
}

/// this is for the analysis
void ParserISA::SetNumOfLoopIteration(int iNumOfLoopIteration)
{
    m_pIsaTree.SetNumOfLoopIteration(iNumOfLoopIteration);
}

int ParserISA::GetNumOfLoopIteration()
{
    return m_pIsaTree.GetNumOfLoopIteration();
}

const ISACodeBlock* ParserISA::GetGraphHead()
{
    return m_pIsaTree.GetISAProgramGraph();
}

bool ParserISA::SplitIsaLine(const std::string& isaInstruction, std::string& instrOpCode,
                             std::string& params, std::string& binaryRepresentation, std::string& offset) const
{
    return ExtractBuildingBlocks(isaInstruction, instrOpCode, params, binaryRepresentation, offset);
}

bool ParserISA::ParseHsailStatistics(const std::string& hsailIsa, beKA::AnalysisData& stats)
{
    // Constants that are specific to this function's logic.
    const std::string USED_SGPRS_TOKEN = " wavefront_sgpr_count";
    const std::string USED_VGPRS_TOKEN = " workitem_vgpr_count";
    const std::string WAVEFRONT_SIZE_TOKEN = " wavefront_size";
    const std::string LDS_USED_BYTES_TOKEN = " workgroup_group_segment_byte_size";

    // Reset the output buffer.
    stats.numSGPRsUsed = 0;
    stats.numVGPRsUsed = 0;
    stats.wavefrontSize = 0;
    stats.LDSSizeUsed = 0;

    // Extract the values.
    ExtractHsailIsaNumericValue(hsailIsa, USED_SGPRS_TOKEN, stats.numSGPRsUsed);
    ExtractHsailIsaNumericValue(hsailIsa, USED_VGPRS_TOKEN, stats.numVGPRsUsed);
    ExtractHsailIsaNumericValue(hsailIsa, WAVEFRONT_SIZE_TOKEN, stats.wavefrontSize);
    ExtractHsailIsaNumericValue(hsailIsa, LDS_USED_BYTES_TOKEN, stats.LDSSizeUsed);

    // Return true if we managed to successfully extract at least one value.
    bool ret = (stats.wavefrontSize != 0 || stats.numSGPRsUsed != 0 || stats.numVGPRsUsed != 0 || stats.LDSSizeUsed != 0);
    return ret;
}

void ParserISA::ExtractHsailIsaNumericValue(const std::string& hsailIsa, const std::string valueToken, CALuint64& valueBuffer)
{
    const std::string EQUALS_TOKEN = "= ";
    size_t posBegin = hsailIsa.find(valueToken);

    if (posBegin != std::string::npos)
    {
        posBegin = hsailIsa.find(EQUALS_TOKEN, posBegin);

        if (posBegin != std::string::npos && posBegin < (hsailIsa.size() - 2))
        {
            // Advance the position to the actual value's begin index.
            posBegin += 2;

            // Extract the value as a string.
            std::stringstream valueStream;

            while (std::isdigit(hsailIsa[posBegin]))
            {
                valueStream << hsailIsa[posBegin++];
            }

            // Convert the string to a numerical integer value.
            const std::string& valueStr = valueStream.str();

            if (!valueStr.empty())
            {
                valueBuffer = std::stoi(valueStr);
            }
        }
    }
}

