//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdProcessDebuggersManager.h
///
//==================================================================================

//------------------------------ pdProcessDebuggersManager.h ------------------------------

#ifndef __PDPROCESSDEBUGGERSMANAGER_H
#define __PDPROCESSDEBUGGERSMANAGER_H

// Forward Declarations:
class apDebugProjectSettings;
class pdProcessDebugger;

// Local:
#include <AMDTProcessDebugger/Include/ProcessDebuggerDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:          pdProcessDebuggerTypes
// General Description: An enumeration used as indices for the process debuggers vector
// Author:              Uri Shomroni
// Creation Date:       11/8/2009
// ----------------------------------------------------------------------------------
enum pdProcessDebuggerTypes
{
    PD_WINDOWS_PROCESS_DEBUGGER = 0,
    PD_LINUX_PROCESS_DEBUGGER,
    PD_REMOTE_PROCESS_DEBUGGER,
    PD_IPHONE_DEVICE_PROCESS_DEBUGGER,
    PD_VISUAL_STUDIO_PROCESS_DEBUGGER,
    PD_NUMBER_OF_PROCESS_DEBUGGER_TYPES
};

// ----------------------------------------------------------------------------------
// Class Name:          PD_API pdProcessDebuggersManager
// General Description: A manager holding pointers to all process debuggers and switching
//                      them as needed
// Author:              Uri Shomroni
// Creation Date:       11/8/2009
// ----------------------------------------------------------------------------------
class PD_API pdProcessDebuggersManager
{
public:
    pdProcessDebuggersManager();
    ~pdProcessDebuggersManager();

    void adjustProcessDebuggerToProcessCreationData(const apDebugProjectSettings& processCreationData);
    void createAndInstallPlatformDefaultProcessDebugger();

    static pdProcessDebuggersManager& instance();

public:
    // These two functions must only be used in the constructors and destructors of pdProcessDebugger subclasses:
    void setProcessDebuggerInSlot(pdProcessDebugger& rProcessDebugger, pdProcessDebuggerTypes processDebuggerType);
    void removeProcessDebuggerFromSlot(pdProcessDebugger& rProcessDebugger, pdProcessDebuggerTypes processDebuggerType);

private:
    pdProcessDebuggerTypes processDebuggerAppropriateForProcessCreationData(const apDebugProjectSettings& processCreationData);
    pdProcessDebugger* getOrCreateProcessDebuggerByType(pdProcessDebuggerTypes processDebuggerType);

private:
    // Friend classes used to create and delete the single instance:
    friend class pdRegisterProcessDebuggersManagerInstance;
    friend class pdSingletonsDelete;

private:
    // A vector holding all the process debuggers:
    pdProcessDebugger** _ppProcessDebuggers;

    // The single instance of this class:
    static pdProcessDebuggersManager* _pMySingleInstance;
};


#endif //__PDPROCESSDEBUGGERSMANAGER_H

