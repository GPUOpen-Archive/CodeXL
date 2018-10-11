//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdDebuggingCommands.h
///
//==================================================================================

//------------------------------ gdDebuggingCommands.h ------------------------------

#ifndef __GDDEBUGGINGCOMMANDS_H
#define __GDDEBUGGINGCOMMANDS_H

// Forward declarations:
class gtString;

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:          GD_API gdDebuggingCommands
// General Description: A base class to allow different instances of the application
//                      (standalone vs. extension) to implement functionality differently
// Author:              Uri Shomroni
// Creation Date:       3/5/2012
// ----------------------------------------------------------------------------------
class GD_API gdDebuggingCommands
{
public:
    virtual ~gdDebuggingCommands() {};
    static gdDebuggingCommands* instance();

    virtual bool addWatchVariable(const gtString& watchVariable) = 0;

protected:
    gdDebuggingCommands() {};

    // Register my instance (this function is private, to make sure that only approved
    // classes access this function:
    static void registerInstance(gdDebuggingCommands* pDebuggingCommandsInstance);

private:
    // My single instance:
    static gdDebuggingCommands* m_pMySingleInstance;
};

#endif //__GDDEBUGGINGCOMMANDS_H

