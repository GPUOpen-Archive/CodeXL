//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gwStatisticsActionsExecutor.h
///
//==================================================================================

//------------------------------ gwStatisticsActionsExecutor.h ------------------------------

#ifndef __GWSTATISTICSACTIONSEXECUTOR_H
#define __GWSTATISTICSACTIONSEXECUTOR_H

// Forward declaration:
class gdStatisticsView;

// Infra:
#include <AMDTApplicationFramework/Include/afActionExecutorAbstract.h>


// ----------------------------------------------------------------------------------
// Class Name:           gwStatisticsActionsExecutor : public afActionCreatorAbstract
// General Description:  Creates the menu items for the statistics commands
// Author:               Sigal Algranaty
// Creation Date:        20/9/2011
// ----------------------------------------------------------------------------------
class gwStatisticsActionsExecutor : public afActionExecutorAbstract
{
public:

    gwStatisticsActionsExecutor();
    ~gwStatisticsActionsExecutor();

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

protected:
    // Create a vector of command Ids that are supported by this actions creator object
    virtual void populateSupportedCommandIds();

    // Statistics view:
    gdStatisticsView* _pStatisticsView;
    gdStatisticsPanel* _pStatisticsPanel;

};


#endif //__GWSTATISTICSACTIONSEXECUTOR_H

