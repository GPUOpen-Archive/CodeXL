//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afSourceCodeActionsCreator.h
///
//==================================================================================

#ifndef __AFSOURCECODEACTIONSCREATOR_H
#define __AFSOURCECODEACTIONSCREATOR_H


// Infra:
#include <AMDTApplicationFramework/Include/afActionCreatorAbstract.h>


// ----------------------------------------------------------------------------------
// Class Name:           afSourceCodeActionsCreator : public afActionCreatorAbstract
// General Description:  This class is used for handling the creation and execution of
//                       all actions related to the source code editing
// Author:               Sigal Algranaty
// Creation Date:        1/9/2011
// ----------------------------------------------------------------------------------
class AF_API afSourceCodeActionsCreator : public afActionCreatorAbstract
{

public:

    afSourceCodeActionsCreator();
    ~afSourceCodeActionsCreator();

    // Virtual functions that needs to be implemented:

    // Create a vector of command Ids that are supported by this actions creator object
    virtual void populateSupportedCommandIds();

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

};



#endif //__AFSOURCECODEACTIONSCREATOR_H

