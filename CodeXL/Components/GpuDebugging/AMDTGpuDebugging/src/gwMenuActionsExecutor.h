//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gwMenuActionsExecutor.h
///
//==================================================================================

//------------------------------ gwMenuActionsExecutor.h ------------------------------

#ifndef __GWMENUACTIONSCREATOR_H
#define __GWMENUACTIONSCREATOR_H

// AMDTGpuDebuggingComponents:
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>

// Infra:
#include <AMDTApplicationFramework/Include/afActionExecutorAbstract.h>

class gwQTActionsHandler;
class gdApplicationCommands;
class QPixmap;

class gwMenuActionsExecutor : public afActionExecutorAbstract
{
public:
    gwMenuActionsExecutor(void);
    ~gwMenuActionsExecutor(void);

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

    // Utilities (override this function is one of the actions has an icon):
    virtual void initActionIcons();
    QString FindStartDebugActionText();
protected:

    // Each derived class must populate the vector of supported command Ids
    virtual void populateSupportedCommandIds();

    // The application main command handler:
    gdApplicationCommands* _pApplicationCommandsHandler;

};

#endif //__gwMenuActionsCreator_H

