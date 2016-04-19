//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gwImagesAndBuffersActionsCreator.h
///
//==================================================================================

//------------------------------ gwImagesAndBuffersActionsCreator.h -------------------------

#ifndef __GWIMAGESANDBUFFERSACTIONSCREATOR_H
#define __GWIMAGESANDBUFFERSACTIONSCREATOR_H

// AMDTGpuDebuggingComponents:
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>

// Infra:
#include <AMDTApplicationFramework/Include/afActionCreatorAbstract.h>

class gwQTActionsHandler;
class gdApplicationCommands;
class QPixmap;

class gwImagesAndBuffersActionsCreator : public afActionCreatorAbstract
{
public:

    gwImagesAndBuffersActionsCreator();
    ~gwImagesAndBuffersActionsCreator();

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

    // Get the group command ids for the action:
    virtual void groupAction(int actionIndex);

    // Utilities (override this function is one of the actions has an icon):
    virtual void initActionIcons();

    // Toolbar separator position: returns -1 if before action index, 1 after, 0 - no separator
    virtual int separatorPosition(int actionIndex);

protected:
    // Each derived class must populate the vector of supported command Ids
    virtual void populateSupportedCommandIds();
};

#endif //__GWIMAGESANDBUFFERSACTIONSCREATOR_H

