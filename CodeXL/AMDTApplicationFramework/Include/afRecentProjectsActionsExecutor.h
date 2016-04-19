//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afRecentProjectsActionsExecutor.h
///
//==================================================================================

#ifndef __AFRECENTPROJECTSACTIONSEXECUTOR_H
#define __AFRECENTPROJECTSACTIONSEXECUTOR_H

// Forward declaration:
class afApplicationCommands;

// Infra:
#include <AMDTApplicationFramework/Include/afActionExecutorAbstract.h>
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           afRecentProjectsActionsExecutor : public afActionCreatorAbstract
// General Description:  Creates the menu items for the recently used project commands
// Author:               Sigal Algranaty
// Creation Date:        14/8/2011
// ----------------------------------------------------------------------------------
class AF_API afRecentProjectsActionsExecutor : public afActionExecutorAbstract
{
public:

    afRecentProjectsActionsExecutor();
    ~afRecentProjectsActionsExecutor();

    // Virtual functions that needs to be implemented:

    // Get the caption of the action item:
    virtual bool actionText(int actionIndex, gtString& caption, gtString& tooltip, gtString& keyboardShortcut);

    // Menu position.
    // Each hierarchy on the ,emu include name/priority.
    // If separator is needed after the item then 's' after the priority is needed
    // in case of a sub menu if one item is marked with an 's' it is enough to mark a separator after it:
    virtual gtString menuPosition(int actionIndex, afActionPositionData& positionData);

    // Toolbar position. separators are defined by "/":
    // Position is defined as in menus:
    virtual gtString toolbarPosition(int actionIndex);

    // Handle the action when it is triggered:
    virtual void handleTrigger(int actionIndex);

    // Handle UI update:
    virtual void handleUiUpdate(int actionIndex);

    // Get the group command ids for the action:
    virtual void groupAction(int actionIndex);

    bool UpdateRecentlyUsedProjects();

protected:
    // Each derived class must populate the vector of supported command Ids
    virtual void populateSupportedCommandIds();

    // The application main command handler:
    afApplicationCommands* m_pApplicationCommandsHandler;

    // Contain the names of the recently used projects names:
    gtVector<gtString> m_recentlyUsedProjectsNames;

};


#endif //__AFRECENTPROJECTSACTIONSEXECUTOR_H

