//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afGeneralActionsCreator.h
///
//==================================================================================

#ifndef __AFGENERALACTIONSCREATOR_H
#define __AFGENERALACTIONSCREATOR_H

// Infra:
#include <AMDTApplicationFramework/Include/afActionCreatorAbstract.h>


// ----------------------------------------------------------------------------------
// Class Name:          AF_API afGeneralActionsCreator : public afActionCreatorAbstract
// General Description:  This class is used for handling the creation and execution of
//                       all actions related to the framework views
// Author:              Sigal Algranaty
// Creation Date:       8/5/2012
// ----------------------------------------------------------------------------------
class AF_API afGeneralActionsCreator : public afActionCreatorAbstract
{

public:

    afGeneralActionsCreator();
    ~afGeneralActionsCreator();

    // Virtual functions that needs to be implemented:

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

protected:
    // Create a vector of command Ids that are supported by this actions creator object
    virtual void populateSupportedCommandIds();

};



#endif //__AFGENERALACTIONSCREATOR_H

