#include "ISAProgramGraph.h"
#include "ParserSI.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>

const int DEFAULT_ITERATION_COUNT = 10;
const int DEFAULT_ITERATION_COUNT_HW_LOOPS = 64;

ISAProgramGraph::ISAProgramGraph()
{
    m_iNextLabel = NO_LABEL - 1 ;
    m_ISACodeBlock = NULL;
    m_iNumOfLoopIterations = DEFAULT_ITERATION_COUNT;
}


ISAProgramGraph::~ISAProgramGraph()
{

}

ISACodeBlock* ISAProgramGraph::CreateNewNode(int iLabel)
{
    LabelNodeSet newNodeSet;
    ISACodeBlock* pNewNodeISA = new ISACodeBlock();

    // case this is a chunk of instruction not under specific label
    if (iLabel == NO_LABEL)
    {
        pNewNodeISA->m_iLabel = m_iNextLabel;
        newNodeSet.iLabel = m_iNextLabel;
        m_iNextLabel--;
    }
    // there is a label parsed from the ISA
    else
    {
        newNodeSet.iLabel = iLabel;
        pNewNodeISA->m_iLabel = iLabel;
    }

    newNodeSet.pNode = pNewNodeISA;
    m_NodesSet.insert(newNodeSet);

    // check if this is the head of the graph- if so- save it!
    if (NULL == m_ISACodeBlock)
    {
        m_ISACodeBlock = pNewNodeISA;
    }

    return pNewNodeISA;
}

void ISAProgramGraph::DestroyISAProgramStructure()
{

    if (!m_NodesSet.empty())
    {
        std::set<LabelNodeSet, LabelNodeSetCompare>::const_iterator iterBegin = m_NodesSet.begin();

        for (; iterBegin != m_NodesSet.end(); ++iterBegin)
        {
            if (iterBegin->pNode)
            {
                delete iterBegin->pNode;
            }
        }

        m_NodesSet.clear();

        m_ISACodeBlock = NULL;
        m_iNextLabel = NO_LABEL - 1;
    }
}

ISACodeBlock* ISAProgramGraph::LabelSearcher(int iLabel)
{
    ISACodeBlock* pRet = NULL;
    LabelNodeSet newNodeSet;
    newNodeSet.iLabel = iLabel;
    std::set<LabelNodeSet, LabelNodeSetCompare>::const_iterator iterBegin = m_NodesSet.find(newNodeSet);

    if (iterBegin != m_NodesSet.end())
    {
        pRet = iterBegin->pNode;
    }

    return pRet;
}

bool ISAProgramGraph::BuildISAProgramStructure(std::vector<Instruction*>& Instructions)
{
    std::vector<Instruction*>::const_iterator iIterator = Instructions.begin();
    return BuildISAProgramStructureInternal(Instructions, iIterator, this->m_ISACodeBlock);
}

bool ISAProgramGraph::BuildISAProgramStructureInternal(std::vector<Instruction*>& Instructions, std::vector<Instruction*>::const_iterator instIterator, ISACodeBlock* pHeadIsaNode)
{
    if (instIterator == Instructions.end())
    {
        return false;
    }

    std::vector<Instruction*>::const_iterator iIterator = instIterator;
    Instruction* pCurrentInstruction = *instIterator;

    // set the label
    if (NULL == pHeadIsaNode)
    {
        pHeadIsaNode = CreateNewNode(pCurrentInstruction->GetLabel());
    }

    // iterate through the instructions and push them to the node
    for (; iIterator != Instructions.end(); ++iIterator)
    {
        // first- add it to the instructions vector of the node
        pHeadIsaNode->m_vInstructions.push_back(*iIterator);

        // A branch
        if ((*iIterator)->GetGotoLabel() != NO_LABEL)
        {
            // this is the info we want to gather in order to decide how to build the graph
            bool bNodeExist = false;
            bool bLoop = false;
            bool bIsSBranch = false;

            std::vector<Instruction*>::const_iterator instBegin = Instructions.begin();
            //case 1: the label is already in the graph: search for the label in the IsaTree if found- set the true pointer if false call with the label
            ISACodeBlock* pLoop = LabelSearcher((*iIterator)->GetGotoLabel());

            if (pLoop) // the node is already in the graph
            {
                // this check if we have a real loop, and if so, update the num of iterations.
                bLoop = UpdateNumOfIteration(Instructions, pLoop, iIterator);
                bNodeExist = true;
            }
            else // need to create a new node
            {
                instBegin = Instructions.begin();

                for (; instBegin != Instructions.end(); ++instBegin)
                {
                    if (((*instBegin)->GetLabel() != NO_LABEL) && ((*instBegin)->GetLabel() == (*iIterator)->GetGotoLabel()))
                    {
                        break;
                    }
                }

                if (instBegin != Instructions.end())
                {
                    pLoop = CreateNewNode((*iIterator)->GetGotoLabel());

                }

            }

            // now see if we are facing an S_Branch  or S_CBranch (unconditional branch vs. conditional)
            pCurrentInstruction = *iIterator;

            if (pCurrentInstruction->GetInstructionCategory() == Instruction::ScalarALU)
            {
                if (pCurrentInstruction->GetInstructionFormat() == Instruction::InstructionSet_SOPP)
                {
                    SISOPPInstruction* siInstSOPP = dynamic_cast<SISOPPInstruction*>(pCurrentInstruction);

                    if (siInstSOPP != NULL)
                    {
                        SISOPPInstruction::OP opSOPP = siInstSOPP->GetOp();

                        if (SISOPPInstruction::S_BRANCH == opSOPP)
                        {
                            bIsSBranch = true;
                        }
                    }
                    else
                    {
                        VISOPPInstruction* viInstSOPP = dynamic_cast<VISOPPInstruction*>(pCurrentInstruction);

                        if (viInstSOPP != NULL)
                        {
                            VISOPPInstruction::OP opSOPP = viInstSOPP->GetOp();

                            if (VISOPPInstruction::s_branch == opSOPP)
                            {
                                bIsSBranch = true;
                            }
                        }
                    }
                }
            }

            // now we have all the information we need, just need to set the right sequential and call function again
            if (bIsSBranch)
            {
                if (!bLoop) // this is a case where we will mark the next because this is unconditional branch and we do it always
                {
                    pHeadIsaNode->SetNext(pLoop);
                }
                else // we shouldn't get it, but just in case
                {
                    pHeadIsaNode->SetNext(NULL);
                    pHeadIsaNode->SetFalse(NULL);
                    pHeadIsaNode->SetTrue(NULL);
                }

                if (!bNodeExist)
                {
                    BuildISAProgramStructureInternal(Instructions, instBegin, pLoop);
                }

                // we have an unconditional branch we break here because there is no false.
                break;
            }
            else
            {
                if (bLoop)
                {
                    // in case of a loop, we mark the TRUE=NULL because we already know it is a loop and we don't want loops in the graph
                    pHeadIsaNode->m_pTrue = NULL; //pLoop;
                }
                else
                {
                    // this is the case where we have a branch that points to a node which exist in the graph and it is not a loop.
                    pHeadIsaNode->SetTrue(pLoop);
                }

                if (!bNodeExist)
                {
                    BuildISAProgramStructureInternal(Instructions, instBegin, pLoop);
                }

                // do the false
                std::vector<Instruction*>::const_iterator pNextInstruction = (iIterator + 1);

                if (pNextInstruction != Instructions.end())
                {
                    ISACodeBlock* pFalseIsaNode = CreateNewNode((*pNextInstruction)->GetLabel());
                    pHeadIsaNode->SetFalse(pFalseIsaNode);
                    BuildISAProgramStructureInternal(Instructions, pNextInstruction, pFalseIsaNode);
                }

                break;
            }
        }
        else // not a branch- is next instruction is with a label?
        {
            std::vector<Instruction*>::const_iterator pNextInstruction = (iIterator + 1);

            if ((pNextInstruction != Instructions.end()) && ((*pNextInstruction)->GetLabel() != NO_LABEL))
            {
                ISACodeBlock* pNext = LabelSearcher((*pNextInstruction)->GetLabel());

                if (pNext)
                {
                    pHeadIsaNode->SetNext(pNext);
                    return true;
                }
                else
                {
                    std::vector<Instruction*>::const_iterator instBegin = Instructions.begin();

                    for (; instBegin != Instructions.end(); ++instBegin)
                    {
                        if (((*instBegin)->GetLabel() != NO_LABEL) && ((*instBegin)->GetLabel() == (*pNextInstruction)->GetLabel()))
                        {
                            break;
                        }
                    }

                    if (instBegin != Instructions.end())
                    {
                        ISACodeBlock* pNextIsaNode = CreateNewNode((*instBegin)->GetLabel());
                        pHeadIsaNode->SetNext(pNextIsaNode);
                        BuildISAProgramStructureInternal(Instructions, instBegin, pNextIsaNode);
                        break;
                    }
                }
            }
        }
    }

    return true;
}


bool ISAProgramGraph::UpdateNumOfIteration(std::vector<Instruction*>& Instructions, ISACodeBlock* pLoop, const std::vector<Instruction*>::const_iterator iIterator)
{
    std::vector<Instruction*>::const_iterator instBegin = Instructions.begin();
    bool bFound = false;

    for (; (instBegin != Instructions.end()) && (instBegin < iIterator); ++instBegin)
    {
        if (((*instBegin)->GetLabel() != NO_LABEL) && ((*instBegin)->GetLabel() == pLoop->m_iLabel))
        {
            bFound = true;
            pLoop->SetIterationCount(m_iNumOfLoopIterations);
            break;
        }
    }


    // it is a loop, check if this is HW loop or regular and update num of iteration
    if (bFound)
    {
        instBegin = pLoop->m_vInstructions.begin();

        for (; instBegin < pLoop->m_vInstructions.end(); ++instBegin)
        {
            SIVOP1Instruction* pCurrentInst = dynamic_cast<SIVOP1Instruction*>(*instBegin);

            if (pCurrentInst != NULL)
            {
                if ((pCurrentInst->GetInstructionType() == SIVOP1Instruction::Encoding_VOP1) &&
                    ((pCurrentInst->GetOp() == SIVOP1Instruction::V_READFIRSTLANE_B32) || (pCurrentInst->GetOp() == SIVOP1Instruction::V_MOVRELD_B32)))
                {
                    pLoop->SetIterationCount(DEFAULT_ITERATION_COUNT_HW_LOOPS);
                    break;
                }
            }
            else
            {
                VIVOP1Instruction* pCurrentInstVivo = dynamic_cast<VIVOP1Instruction*>(*instBegin);

                if (pCurrentInstVivo != NULL)
                {
                    if ((pCurrentInstVivo->GetInstructionType() == VIVOP1Instruction::Encoding_VOP1) && (pCurrentInstVivo->GetOp() == VIVOP1Instruction::v_readfirstlane_b32)) // || (pCurrentInstVivo->GetOp() == SIVOP1Instruction::V_MOVRELD_B32)))
                    {
                        pLoop->SetIterationCount(DEFAULT_ITERATION_COUNT_HW_LOOPS);
                        break;
                    }
                }
            }
        }
    }

    return bFound;
}

void ISAProgramGraph::GetInstructionsOfProgramPath(std::set<LabelNodeSet, LabelNodeSetCompare>& PathInstructionsSet, int iPath)
{
    GetInstructionsOfProgramPathInternal(PathInstructionsSet, this->m_ISACodeBlock, iPath, 1);
}


void ISAProgramGraph::GetInstructionsOfProgramPathInternal(std::set<LabelNodeSet, LabelNodeSetCompare>& PathInstructionsSet, ISACodeBlock* pHeadISACodeBlock, int iPath, int iNumOfIterations)
{
    if (NULL == pHeadISACodeBlock)
    {
        return;
    }

    // if the node is in the set- return. we visit only once!
    LabelNodeSet tempLabelNodeSet;
    tempLabelNodeSet.iLabel = pHeadISACodeBlock->GetLabel();

    if (PathInstructionsSet.find(tempLabelNodeSet) != PathInstructionsSet.end())
    {
        return;
    }

    // we got here with the node- put it in the set
    LabelNodeSet newLabelNodeSet;
    newLabelNodeSet.iLabel = pHeadISACodeBlock->GetLabel();
    newLabelNodeSet.iNumOfIteration = iNumOfIterations * (pHeadISACodeBlock->GetIterationCount());
    newLabelNodeSet.pNode = pHeadISACodeBlock;
    PathInstructionsSet.insert(newLabelNodeSet);

    //where we go next?
    int iTempNumOfIterations = newLabelNodeSet.iNumOfIteration;

    if (pHeadISACodeBlock->GetTrue())
    {
        // if this is true, we probably going out of a loop- reduce the iteration count.
        if (pHeadISACodeBlock->GetFalse() && (pHeadISACodeBlock->GetIterationCount() > 1))
        {
            if (iTempNumOfIterations > 1)
            {
                iTempNumOfIterations /= m_iNumOfLoopIterations;
            }

            GetInstructionsOfProgramPathInternal(PathInstructionsSet, pHeadISACodeBlock->GetTrue(), iPath, iTempNumOfIterations);
        }

        // we do the true unless we are in the false path
        if (iPath != 2)
        {
            GetInstructionsOfProgramPathInternal(PathInstructionsSet, pHeadISACodeBlock->GetTrue(), iPath, newLabelNodeSet.iNumOfIteration);
        }
    }

    if (pHeadISACodeBlock->GetFalse())
    {
        iTempNumOfIterations = newLabelNodeSet.iNumOfIteration;

        // we do the false always if the true is null because this is probably an end of a loop
        if (!pHeadISACodeBlock->GetTrue())
        {
            if ((pHeadISACodeBlock->GetIterationCount() > 1) && (iTempNumOfIterations > 1))
            {
                iTempNumOfIterations /= m_iNumOfLoopIterations;
            }

            GetInstructionsOfProgramPathInternal(PathInstructionsSet, pHeadISACodeBlock->GetFalse(), iPath, iTempNumOfIterations);
        }
        else if (iPath != 1)
        {
            GetInstructionsOfProgramPathInternal(PathInstructionsSet, pHeadISACodeBlock->GetFalse(), iPath, iTempNumOfIterations);
        }

    }


    if (pHeadISACodeBlock->GetNext())
    {
        // do it always
        GetInstructionsOfProgramPathInternal(PathInstructionsSet, pHeadISACodeBlock->GetNext(), iPath, newLabelNodeSet.iNumOfIteration);
    }

}

void ISAProgramGraph::DumpGraph(std::set<ISAProgramGraph::LabelNodeSet, ISAProgramGraph::LabelNodeSetCompare> PathInstructionsSet, std::string sFileName)
{
    // open the file:
    std::ofstream ofs;
    ofs.open(sFileName.c_str(), std::ofstream::out);
    ofs << "digraph G {\n";


    std::set<LabelNodeSet, LabelNodeSetCompare>::iterator iter = PathInstructionsSet.begin();

    for (; iter != PathInstructionsSet.end(); ++iter)
    {
        //loops gets double circle shape, also the arrows are red and mark the number of times it will be done
        if ((*iter).iNumOfIteration > 1)
        {
            ofs << (*iter).iLabel << " " << "[shape=doublecircle,style=filled,color=\".7 .3 1.0\", label=\" " << (*iter).iLabel << " X " << (*iter).iNumOfIteration << "\"];\n";
        }
        else
        {
            ofs << (*iter).iLabel << " " << "[shape=box];\n";
        }
    }

    for (iter = PathInstructionsSet.begin(); iter != PathInstructionsSet.end(); ++iter)
    {
        if ((*iter).pNode->GetTrue())
        {
            LabelNodeSet temp;
            temp.iLabel = (*iter).pNode->GetTrue()->GetLabel();

            if (PathInstructionsSet.find(temp) != PathInstructionsSet.end())
            {
                ofs << (*iter).iLabel << " -> " << (*iter).pNode->GetTrue()->GetLabel() << "[label=\"T\"]" << ";\n ";
            }
        }

        if ((*iter).pNode->GetFalse())
        {
            LabelNodeSet temp;
            temp.iLabel = (*iter).pNode->GetFalse()->GetLabel();

            if (PathInstructionsSet.find(temp) != PathInstructionsSet.end())
            {
                ofs << (*iter).iLabel << " -> " << (*iter).pNode->GetFalse()->GetLabel() <<  "[label=\"F\"]" << ";\n ";
            }
        }

        if ((*iter).pNode->GetNext())
        {
            LabelNodeSet temp;
            temp.iLabel = (*iter).pNode->GetNext()->GetLabel();

            if (PathInstructionsSet.find(temp) != PathInstructionsSet.end())
            {
                ofs << (*iter).iLabel << " -> " << (*iter).pNode->GetNext()->GetLabel() << "[label=\"N\"]" << ";\n ";
            }
        }
    }

    ofs << "}";
    ofs.close();
}

// here is the algorithm for the cycles:
//  Cycles Estimation Algorithm
//  We consider 7 categories of instructions:
//  •   Export / GDS
//      •   Vector memory read/write
//      •   Vector ALU                                                 - include LDS-direct read and parameter interpolation
//      •   LDS indexed/atomic
//      •   Scalar ALU or Scalar memory read                    - includes set_pc
//      •   Branch or Message out                                         - s_branch and s_cbranch_<cond> only, not s_cbranch_fork/join
//      •   Internal : sleep, wait, set-prio
//  Each instruction type has an associated number of cycles it takes to execute this instruction.
//  We store a counter per category of instructions.
//
//  Issues the algorithm considers:
//  •   Different number of cycles for execution of different types of instructions.
//      •   Barriers.
//  Issues the algorithm ignores:
//  •   Maximum 5 types of instructions performed each cycle.
//      Algorithm heuristics:
//      1.  Iterate over the instructions in the kernel, using the selected flow control path (All True, All False, Mixed.
//      2.  For each instruction:
//          a.  If the current instruction is a barrier (NOP)
//              Increase the counter for all(?) other instruction categories until the object of the barrier is achieved. The counters should be set to max(value of counter, value of counter for barrier category)
//          b.  Else
//              Increase the counter of the instruction category that matches the current instruction. The increased amount is the num of cycles of this specific instruction.
//      3.  The estimated cycles number will be the counter containing the largest value.
void ISAProgramGraph::CountInstructions(std::set<ISAProgramGraph::LabelNodeSet, ISAProgramGraph::LabelNodeSetCompare> PathInstructionsSet, ISAProgramGraph::NumOfInstructionsInCategory& NumOfInstructionsInCategory)
{
    (void)(PathInstructionsSet);
    (void)(NumOfInstructionsInCategory);

    // This code is intentionally commented out. We need to update this function to pass the device name when calculating the number of cycles that an instruction costs.
    // This functionality of ISAProgramGraph is no longer being used anyway since the Analysis features which are provided by this class were found to be inaccurate in not useful.
    // When we will get back to this implementation, this function's body should be adjusted accordingly.

    /* int iScalarALU_ScalarMemoryRead_Write, iVectorMemoryRead_Write, iVectorALU, iLDS_Atomics, iGDS_Export;
     iScalarALU_ScalarMemoryRead_Write = 0; iVectorMemoryRead_Write = 0, iVectorALU = 0; iLDS_Atomics = 0; iGDS_Export = 0;
     std::set<ISAProgramGraph::LabelNodeSet, ISAProgramGraph::LabelNodeSetCompare>::iterator iterSet = PathInstructionsSet.begin();

     for (; iterSet != PathInstructionsSet.end(); iterSet++)
     {
         if ((*iterSet).pNode != NULL)
         {
             std::vector<Instruction*> InstructionVec = (*iterSet).pNode->GetIsaCodeBlockInstructions();
             std::vector<Instruction*>::iterator  iterInstructions = InstructionVec.begin();

             for (; iterInstructions != InstructionVec.end(); iterInstructions++)
             {
                 Instruction* pInstruction = *iterInstructions;
                 Instruction::InstructionFormatKind instructionFormatKind = (pInstruction)->GetInstructionFormatKind();

                 switch (instructionFormatKind)
                 {
                     case Instruction::ScalarMemoryRead:
                         iScalarALU_ScalarMemoryRead_Write += (*iterSet).iNumOfIteration * pInstruction->GetInstructionClockCount();
                         NumOfInstructionsInCategory.m_scalarMemoryReadInstCount += (*iterSet).iNumOfIteration;
                         break;

                     case Instruction::ScalarMemoryWrite:
                         iScalarALU_ScalarMemoryRead_Write += (*iterSet).iNumOfIteration * pInstruction->GetInstructionClockCount();
                         NumOfInstructionsInCategory.m_scalarMemoryWriteInstCount += (*iterSet).iNumOfIteration;
                         break;

                     case Instruction::VectorMemoryRead:
                         iVectorMemoryRead_Write += (*iterSet).iNumOfIteration * pInstruction->GetInstructionClockCount();
                         NumOfInstructionsInCategory.m_vectorMemoryReadInstCount += (*iterSet).iNumOfIteration;
                         break;

                     case Instruction::VectorMemoryWrite:
                         iVectorMemoryRead_Write += (*iterSet).iNumOfIteration * pInstruction->GetInstructionClockCount();
                         NumOfInstructionsInCategory.m_vectorMemoryWriteInstCount += (*iterSet).iNumOfIteration;
                         break;

                     case Instruction::VectorALU:
                         if (pInstruction->GetInstructionFormat() == Instruction::InstructionSet_VOP)
                         {
                             SIVOP1Instruction* instVOP = (SIVOP1Instruction*)pInstruction;
                             SIVOP3Instruction* instVOP3 = (SIVOP3Instruction*)pInstruction;

                             if ((((instVOP->GetInstructionType() == SIVOP1Instruction::Encoding_VOP1) && (SIVOP1Instruction::V_NOP == instVOP->GetOp()))) ||
                                     ((instVOP->GetInstructionType() == SIVOP1Instruction::Encoding_VOP3)  && (SIVOP3Instruction::V3_NOP == instVOP3->GetOp())))
                             {
                                 iScalarALU_ScalarMemoryRead_Write = iVectorMemoryRead_Write = iVectorALU = iLDS_Atomics = iGDS_Export = GetBiggest(iScalarALU_ScalarMemoryRead_Write, iVectorMemoryRead_Write, iVectorALU, iLDS_Atomics, iGDS_Export);
                             }
                             else
                             {
                                 iVectorALU += (*iterSet).iNumOfIteration * pInstruction->GetInstructionClockCount();
                             }
                         }

                         NumOfInstructionsInCategory.m_vectorALUInstCount += (*iterSet).iNumOfIteration;
                         break;

                     case Instruction::LDS:
                         iLDS_Atomics += (*iterSet).iNumOfIteration * pInstruction->GetInstructionClockCount();
                         NumOfInstructionsInCategory.m_LDSInstCount += (*iterSet).iNumOfIteration;
                         break;

                     case Instruction::GDS:
                         iGDS_Export += (*iterSet).iNumOfIteration * pInstruction->GetInstructionClockCount();
                         NumOfInstructionsInCategory.m_GDSInstCount += (*iterSet).iNumOfIteration;
                         break;

                     case Instruction::Export:
                         iGDS_Export += (*iterSet).iNumOfIteration * pInstruction->GetInstructionClockCount();
                         NumOfInstructionsInCategory.m_exportInstCount += (*iterSet).iNumOfIteration;
                         break;

                     case Instruction::Atomics:
                         iLDS_Atomics += (*iterSet).iNumOfIteration * pInstruction->GetInstructionClockCount();
                         NumOfInstructionsInCategory.m_atomicsInstCount += (*iterSet).iNumOfIteration;
                         break;

                     case Instruction::ScalarALU:
                         if (pInstruction->GetInstructionFormat() == Instruction::InstructionSet_SOPP)
                         {
                         SISOPPInstruction::OP siopSopp = SISOPPInstruction::S_ILLEGAL;
                         VISOPPInstruction::OP viopSopp = VISOPPInstruction::S_ILLEGAL;
                             SISOPPInstruction* siInstSOPP = dynamic_cast<SISOPPInstruction*>(pInstruction);
                         if (siInstSOPP != NULL)
                         {
                             siopSopp = siInstSOPP->GetOp();
                         }
                         else
                         {
                             VISOPPInstruction* viInstSOPP = dynamic_cast<VISOPPInstruction*>(pInstruction);
                             if (viInstSOPP != NULL)
                             {
                                 viopSopp = viInstSOPP->GetOp();
                             }
                         }


                             if ((SISOPPInstruction::S_NOP == siopSopp) || (VISOPPInstruction::s_nop == viopSopp))
                             {
                                 iScalarALU_ScalarMemoryRead_Write = iVectorMemoryRead_Write = iVectorALU = iLDS_Atomics = iGDS_Export = GetBiggest(iScalarALU_ScalarMemoryRead_Write, iVectorMemoryRead_Write, iVectorALU, iLDS_Atomics, iGDS_Export);
                             }
                             else
                             {
                                 iScalarALU_ScalarMemoryRead_Write += (*iterSet).iNumOfIteration * pInstruction->GetInstructionClockCount();
                             }
                         }

                         NumOfInstructionsInCategory.m_scalarALUInstCount += (*iterSet).iNumOfIteration;
                         break;

                     default:
                         break;
                 }

             }
         }
     }

     NumOfInstructionsInCategory.m_CalculatedCycles = GetBiggest(iScalarALU_ScalarMemoryRead_Write, iVectorMemoryRead_Write, iVectorALU, iLDS_Atomics, iGDS_Export);*/
}

void ISAProgramGraph::GetNumOfInstructionsInCategory(ISAProgramGraph::NumOfInstructionsInCategory NumOfInstructionsInCategory[CALC_NUM_OF_PATHES], std::string sDumpGraph)
{
    std::set<ISAProgramGraph::LabelNodeSet, ISAProgramGraph::LabelNodeSetCompare> PathInstructionsSetALL;
    GetInstructionsOfProgramPath(PathInstructionsSetALL, CALC_ALL); // ALL
    CountInstructions(PathInstructionsSetALL, NumOfInstructionsInCategory[CALC_ALL]);

    std::set<ISAProgramGraph::LabelNodeSet, ISAProgramGraph::LabelNodeSetCompare> PathInstructionsSetTRUE;
    GetInstructionsOfProgramPath(PathInstructionsSetTRUE, CALC_TRUE); // TRUE
    CountInstructions(PathInstructionsSetTRUE, NumOfInstructionsInCategory[CALC_TRUE]);

    std::set<ISAProgramGraph::LabelNodeSet, ISAProgramGraph::LabelNodeSetCompare> PathInstructionsSetFALSE;
    GetInstructionsOfProgramPath(PathInstructionsSetFALSE, CALC_FALSE); // FALSE
    CountInstructions(PathInstructionsSetFALSE, NumOfInstructionsInCategory[CALC_FALSE]);

    if (sDumpGraph.length() > 0)
    {
        size_t pos = sDumpGraph.rfind(".");

        if (pos == std::string::npos)
        {
            pos  = sDumpGraph.length() + 1;
        }

        std::string fileALL = sDumpGraph.substr(0, pos - 1) + "ALL.txt";
        std::string fileTrue = sDumpGraph.substr(0, pos - 1) + "TRUE.txt";
        std::string fileFalse = sDumpGraph.substr(0, pos - 1) + "FALSE.txt";
        DumpGraph(PathInstructionsSetALL, fileALL);
        DumpGraph(PathInstructionsSetTRUE, fileTrue);
        DumpGraph(PathInstructionsSetFALSE, fileFalse);
    }
}

int ISAProgramGraph::GetBiggest(int iScalarALU_ScalarMemoryRead_Write, int iVectorMemoryRead_Write, int iVectorALU, int iLDS_Atomics, int iGDS_Export)
{
    int iArr[5];
    iArr[0] = iScalarALU_ScalarMemoryRead_Write; iArr[1] = iVectorMemoryRead_Write; iArr[2] = iVectorALU; iArr[3] = iLDS_Atomics; iArr[4] = iGDS_Export;

    // standard bubble sort
    for (int i = 0 ; i < 4; i++)
    {
        if (iArr[i] > iArr[i + 1])
        {
            int iTemp = iArr[i];
            iArr[i] = iArr[i + 1];
            iArr[i + 1] = iTemp;
        }
    }

    return iArr[4];

}

/// this is for the analysis
void ISAProgramGraph::SetNumOfLoopIteration(int iNumOfLoopIteration)
{
    if (iNumOfLoopIteration > 0)
    {
        m_iNumOfLoopIterations = iNumOfLoopIteration;
    }
}

int ISAProgramGraph::GetNumOfLoopIteration()
{
    return m_iNumOfLoopIterations;
}
