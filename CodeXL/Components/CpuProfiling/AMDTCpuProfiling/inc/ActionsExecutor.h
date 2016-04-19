//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ActionsExecutor.h
///
//==================================================================================

#ifndef __ACTIONSEXECUTOR_H
#define __ACTIONSEXECUTOR_H

// Infra:
#include <AMDTApplicationFramework/Include/afActionExecutorAbstract.h>

class ActionsExecutor : public afActionExecutorAbstract
{
public:
    ActionsExecutor();
    ~ActionsExecutor();

    // Overrides afActionExecutorAbstract:

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
    virtual void groupAction(int actionIndex) { (void)(actionIndex); };

protected:

    // Each derived class must populate the vector of supported command Ids
    virtual void populateSupportedCommandIds();
};

#endif //__ACTIONSEXECUTOR_H

