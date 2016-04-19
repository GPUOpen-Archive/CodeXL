//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afInitializeApplicationCommand.h
///
//==================================================================================

#ifndef __AFINITIALIZEAPPLICATIONCOMMAND_H
#define __AFINITIALIZEAPPLICATIONCOMMAND_H

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>
#include <AMDTApplicationFramework/Include/afCommand.h>

// Forward declarations:
class afSystemInformationCommandThread;

// ----------------------------------------------------------------------------------
// Class Name:           afInitializeApplicationCommand : public afCommand
// General Description:
//   Performs required application initializations.
//   Should be called once when the application starts running.
//
// Author:               Yaki Tebeka
// Creation Date:        20/8/2007
// ----------------------------------------------------------------------------------
class AF_API afInitializeApplicationCommand : public afCommand
{
public:
    afInitializeApplicationCommand(const gtString& productName, const gtString& productDescriptionString);
    virtual ~afInitializeApplicationCommand();

    // Overrides gdCommand:
    virtual bool canExecuteSpecificCommand();
    virtual bool executeSpecificCommand();

    void EndSysCommandThread();

private:
    bool verifyOSVersion();
    void initializeUnhandledExceptionHandler();
    bool initializeDebugLogFile();
    bool initializeSystemInformationData();
    void loadSplashScreen();
    void loadOptionsFile();
    void nameMainThreadInDebugger();

    // Contain the system information string:
    gtString m_systemInformationStr;

    // Contain the product name:
    gtString m_productName;

    // Contain the product information string:
    gtString m_productDescriptionString;

    // sys info thread to gether data
    static afSystemInformationCommandThread* m_spSysCommandThread;
};


#endif //__AFINITIALIZEAPPLICATIONCOMMAND_H

