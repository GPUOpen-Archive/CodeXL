//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  A visitor to a CommandProcessor that extracts its full command string
//==============================================================================

#ifndef _COMMAND_VISITOR_H_
#define _COMMAND_VISITOR_H_

#include "ICommunication.h"

#include <stdio.h>
#include <string.h>

#include <vector>
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include "Logger.h"
#include <vector>

class CommandProcessor;
class CommandResponse;

/// A visitor to a CommandProcessor that extracts its full command string
///
/// Responsible for extracting the full command string at a Command leaf.
/// Part of a visitor pattern with CommandProcessor. The CommandVisitor is
/// passed around the command tree and collects the full path (made up from
/// the sub-paths of each node) until it reaches a Command. At each command
/// the visitor will make a copy of the full path plus the command string and
/// add it to the completed strings. When the visitor finishes its odyssey
/// around the command tree the completed command strings can be sent back to
/// the client.
class CommandVisitor
{
public:

    //--------------------------------------------------------------------------
    /// This gets called when the visitor encounters a CommandProcessor.
    //--------------------------------------------------------------------------
    void VisitCommandProcessor(CommandProcessor* pCommProc);

    //--------------------------------------------------------------------------
    /// This gets called when the visitor encounters a CommandResponse.
    //--------------------------------------------------------------------------
    void VisitCommandResponse(CommandResponse* pCommResp);

    //--------------------------------------------------------------------------
    /// Returns all of the command strings in one gtASCIIString.
    //--------------------------------------------------------------------------
    gtASCIIString GetCommandStrings();

    //--------------------------------------------------------------------------
    /// Destructor
    //--------------------------------------------------------------------------
    ~CommandVisitor();

private:

    //--------------------------------------------------------------------------
    /// Place the input string on the stack.
    //--------------------------------------------------------------------------
    bool PushString(const char* pStr);

    //--------------------------------------------------------------------------
    /// Pop the last string off the stack.
    //--------------------------------------------------------------------------
    void PopString();

    //--------------------------------------------------------------------------
    /// Copy the current full path and command into one string and store it.
    //--------------------------------------------------------------------------
    void RecordCommandString(CommandResponse* pCommResp);

    /// Used to store a node name in the command tree.
    std::vector < gtASCIIString > vCommandTreeStack ;

    /// Used to store the completed full path commands.
    std::vector < gtASCIIString > vFinalCommands ;

};

#endif //_COMMAND_VISITOR_H_