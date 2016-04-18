//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppMenuActionsExecutor.h
///
//==================================================================================

//------------------------------ ppMenuActionsExecutor.h ------------------------------

#ifndef __PPMENUACTIONSCREATOR_H
#define __PPMENUACTIONSCREATOR_H

// Infra:
#include <AMDTApplicationFramework/Include/afCommandIds.h>
#include <AMDTApplicationFramework/Include/afActionExecutorAbstract.h>

// Local:
#include <AMDTPowerProfiling/Include/ppAMDTPowerProfilingDLLBuild.h>

class afApplicationCommands;

/// power profiling main menu commands
enum ppMainMenuCommands
{
    COMMAND_ID_COUNTERS_SELECTION = POWER_PROFILER_FIRST_COMMAND_ID
};


class PP_API ppMenuActionsExecutor : public afActionExecutorAbstract
{

public:
    ppMenuActionsExecutor();
    ~ppMenuActionsExecutor();

    /// Get the caption of the action item
    /// @param[in]  actionIndex
    /// @param[out] caption
    /// @param[out] tooltip
    /// @param[out] keyboardShortcut
    virtual bool actionText(int actionIndex, gtString& caption, gtString& tooltip, gtString& keyboardShortcut);

    /// Menu position.
    /// Each hierarchy on the ,emu include name/priority.
    /// If separator is needed after the item then 's' after the priority is needed
    /// in case of a sub menu if one item is marked with an 's' it is enough to mark a separator after it
    /// @param[in]  actionIndex
    /// @param[out] positionData  menu item position
    virtual gtString menuPosition(int actionIndex, afActionPositionData& positionData);

    /// Toolbar position. separators are defined by "/":
    /// Position is defined as in menus
    /// @param[in]  actionIndex
    virtual gtString toolbarPosition(int actionIndex);

    /// Handle the action when it is triggered
    /// @param[in]  actionIndex
    virtual void handleTrigger(int actionIndex);

    /// Handle UI update
    /// @param[in]  actionIndex
    virtual void handleUiUpdate(int actionIndex);

    /// Get the group command ids for the action
    /// @param[in]  actionIndex
    virtual void groupAction(int actionIndex);

protected:
    // Each derived class must populate the vector of supported command Ids
    virtual void populateSupportedCommandIds();

    /// The application main command handler:
    afApplicationCommands* m_pApplicationCommandsHandler;
};

#endif//__ppMenuActionsCreator_H
