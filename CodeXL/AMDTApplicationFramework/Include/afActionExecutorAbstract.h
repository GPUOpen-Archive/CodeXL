//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afActionExecutorAbstract.h
///
//==================================================================================

#ifndef __AFACTIONEXECUTORABSTRACT_H
#define __AFACTIONEXECUTORABSTRACT_H


// Infra:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>
#include <AMDTApplicationFramework/Include/afActionCreatorAbstract.h>


// ----------------------------------------------------------------------------------
// Class Name:          afActionExecutorAbstract
// General Description: Abstract implementation for the action executor. This class
//                      represents an action creator that both trigger actions and handle
//                      UI update
// Author:              Sigal Algranaty
// Creation Date:       1/9/2011
// ----------------------------------------------------------------------------------
class AF_API afActionExecutorAbstract : public afActionCreatorAbstract
{
public:
    afActionExecutorAbstract();
    virtual ~afActionExecutorAbstract();

    // Handle the action when it is triggered
    virtual void handleTrigger(int actionIndex) = 0;

    // handle UI update
    virtual void handleUiUpdate(int actionIndex) = 0;

};


#endif //__AFACTIONEXECUTORABSTRACT_H

