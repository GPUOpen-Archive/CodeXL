//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gwDebugActionsCreator.h
///
//==================================================================================

//------------------------------ gwDebugActionsCreator.h ------------------------------

#ifndef __GWDEBUGACTIONSCREATOR_H
#define __GWDEBUGACTIONSCREATOR_H

// AMDTGpuDebuggingComponents:
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>

// Infra:
#include <AMDTApplicationFramework/Include/afActionCreatorAbstract.h>


// ----------------------------------------------------------------------------------
// Class Name:           gwDebugActionsCreator : public afActionCreatorAbstract
// General Description:  This class is used for handling the creation and execution of
//                       all actions related to the source code editing
// Author:               Sigal Algranaty
// Date:                 16/9/2011
// ----------------------------------------------------------------------------------
class gwDebugActionsCreator : public afActionCreatorAbstract
{

public:

    gwDebugActionsCreator();
    ~gwDebugActionsCreator();

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

    // Each derived class must populate the vector of supported command Ids
    virtual void populateSupportedCommandIds();

};

#endif //__GWDEBUGACTIONSCREATOR_H

