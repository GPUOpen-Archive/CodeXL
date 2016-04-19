//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gwWXViewsActionsCreator.h
///
//==================================================================================

//------------------------------ gwWXViewsActionsCreator.h ------------------------------

#ifndef __GWWXVIEWSACTIONSCREATOR_H
#define __GWWXVIEWSACTIONSCREATOR_H

// AMDTGpuDebuggingComponents:
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>

// Infra:
#include <AMDTApplicationFramework/Include/afActionCreatorAbstract.h>


// ----------------------------------------------------------------------------------
// Class Name:           gwWXViewsActionsCreator : public afActionCreatorAbstract
// General Description:  This class is used for handling the creation and execution of
//                       all actions related to the views imported from WX
// Author:               Sigal Algranaty
// Creation Date:        5/9/2011
// ----------------------------------------------------------------------------------
class gwWXViewsActionsCreator : public afActionCreatorAbstract
{

public:

    gwWXViewsActionsCreator();
    ~gwWXViewsActionsCreator();

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

    // Get number of actions created:
    virtual int numberActions();

    // Get the group command ids for the action:
    virtual void groupAction(int actionIndex);

protected:

    // Vector containing the ids of the edit commands supported:
    gtVector<int> _supportedCommandIds;

};

#endif //__GWWXVIEWSACTIONSCREATOR_H

