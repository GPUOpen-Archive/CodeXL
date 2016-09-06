//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A visitor to a CommandProcessor that extracts its full command string
//==============================================================================

#include "Logger.h"
#include "CommandVisitor.h"
#include "CommandProcessor.h"

using namespace std;

//--------------------------------------------------------------------------
/// Destructor - Make sure to clear the vectors.
//--------------------------------------------------------------------------
CommandVisitor::~CommandVisitor()
{
    vCommandTreeStack.clear() ;
    vFinalCommands.clear() ;
}

//--------------------------------------------------------------------------
/// Called when we encounter a CommandProcessor. Extract any commands and
/// store them and pass the visitor on to any child CommandProcessors. This
/// is a recursive algorithm.
/// \param pCommProc The input CommandProcessor.
//--------------------------------------------------------------------------
void CommandVisitor::VisitCommandProcessor(CommandProcessor* pCommProc)
{
    PsAssert(NULL != pCommProc);

    // Remember if we added or not
    bool bAddedID = PushString(pCommProc->GetID());

    // Handle the commands in pCommProc
    size_t nCommCount = pCommProc->CommandCount();

    for (size_t i = 0 ; i < nCommCount ; i++)
    {
        CommandResponse* pObj =  pCommProc->GetCommandResponse(i) ;
        PsAssert(pObj);
        // Pass the visitor on to the child CommandResponse.
        pObj->Accept(*this);
    }

    // Pass the visitor on to pCommProc's processors
    size_t nProcCount = pCommProc->ProcessorCount();

    for (size_t i = 0 ; i < nProcCount ; i++)
    {
        CommandProcessor* pProc =  pCommProc->GetCommandProcessor(i) ;
        PsAssert(pProc);
        // Pass the visitor on to the child CommandProcessor.
        pProc->Accept(*this);
    }

    // Only remove the string if we added it earlier.
    if (bAddedID)
    {
        PopString();
    }
}

//--------------------------------------------------------------------------
/// Called wen we encounter a CommandResponse. Pushes the command onto the
/// path tree stack then makes a copy of the full path & command.
/// \param pCommResp
//--------------------------------------------------------------------------
void CommandVisitor::VisitCommandResponse(CommandResponse* pCommResp)
{
    if (PushString(pCommResp->GetURL()))
    {
        RecordCommandString(pCommResp);
        PopString();
    }
}

//--------------------------------------------------------------------------
/// Pushes a path item or command string onto a stack. Basically keeps track
/// of where we are in the command tree.
/// \param pStr
/// \return True if success, False if fail.
//--------------------------------------------------------------------------
bool CommandVisitor::PushString(const char* pStr)
{
    if (NULL == pStr)
    {
        return false;
    }

    if (strlen(pStr) < 1)
    {
        return false;
    }

    gtASCIIString strCmd = pStr ;
    vCommandTreeStack.push_back(strCmd);
    return true;
}

//--------------------------------------------------------------------------
/// Remove the last added item from the stack.
//--------------------------------------------------------------------------
void CommandVisitor::PopString()
{
    vCommandTreeStack.pop_back();
}

//--------------------------------------------------------------------------
/// Concatenates all the sub strings in the tree stack into a single string
/// and saves that string in another stack.
/// \param pCommResp The input Command.
//--------------------------------------------------------------------------
void CommandVisitor::RecordCommandString(CommandResponse* pCommResp)
{
    gtASCIIString strFinal;

    for (vector< gtASCIIString >::iterator iter = vCommandTreeStack.begin();
         iter != vCommandTreeStack.end();
         iter ++)
    {
        strFinal += "/";
        strFinal += *iter;
    }

    switch (pCommResp->GetEditableContent())
    {
        case EDITABLE_TEXT :
        {
            strFinal += "=[\"text\"]";
            break;
        }

        case EDITABLE_BOOL :
        {
            strFinal += "=[True,False]";
            break;
        }

        case EDITABLE_INT :
        {
            strFinal += "=[integer]";
            break;
        }

        case EDITABLE_FLOAT :
        {
            strFinal += "=[float]";
            break;
        }

        case EDITABLE_ULONG :
        {
            strFinal += "=[unsigned long integer]";
            break;
        }

        default:
            break;
    }

    strFinal += "\n" ;

    vFinalCommands.push_back(strFinal) ;
}

//--------------------------------------------------------------------------
/// Returns all of the command strings found during the tree traversal.
/// \return All of the full path command strings that the visitor found.
//--------------------------------------------------------------------------
gtASCIIString CommandVisitor::GetCommandStrings()
{
    gtASCIIString strFinal;

    for (vector< gtASCIIString >::iterator iter = vFinalCommands.begin();
         iter != vFinalCommands.end();
         iter ++)
    {
        strFinal += *iter;
        strFinal += "\n";
    }

    return strFinal;
}