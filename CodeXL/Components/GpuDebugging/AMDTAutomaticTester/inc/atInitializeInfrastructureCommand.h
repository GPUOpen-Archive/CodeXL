//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file atInitializeInfrastructureCommand.h
///
//==================================================================================

//------------------------------ atInitializeInfrastructureCommand.h ------------------------------

#ifndef __ATINITIALIZEINFRASTRUCTURECOMMAND_H
#define __ATINITIALIZEINFRASTRUCTURECOMMAND_H


// ----------------------------------------------------------------------------------
// Class Name:           atInitializeInfrastructureCommand
// General Description:
//   Initializes the infrastructure to be used by the automatic tester executable.
//
// Author:      Merav Zanany
// Date:        29/11/2011
// ----------------------------------------------------------------------------------
class atInitializeInfrastructureCommand
{
public:
    atInitializeInfrastructureCommand();
    ~atInitializeInfrastructureCommand();

    bool execute();

private:
    void initializeUnhandledExceptionHandler();
    bool initializeDebugLogFile();
    void nameMainThreadInDebugger();
};


#endif //__ATINITIALIZEINFRASTRUCTURECOMMAND_H
