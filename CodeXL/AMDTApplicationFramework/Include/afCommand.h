//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afCommand.h
///
//==================================================================================

#ifndef __AFCOMMAND
#define __AFCOMMAND

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           afCommand
// General Description: Base class for all the CodeXL commands.
// Author:               Yaki Tebeka
// Creation Date:        1/11/2003
// ----------------------------------------------------------------------------------
class AF_API afCommand
{
public:
    afCommand() {};
    virtual ~afCommand();

    bool canExecute();
    bool execute();

    // Should be implemened by sub-classes:
    virtual bool canExecuteSpecificCommand() = 0;
    virtual bool executeSpecificCommand() = 0;
};

#endif  // __AFCOMMAND
