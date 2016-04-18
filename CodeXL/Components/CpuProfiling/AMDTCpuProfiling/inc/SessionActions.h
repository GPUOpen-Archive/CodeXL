//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SessionActions.h
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/inc/SessionActions.h#7 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================
#ifndef _SESSIONACTIONS_H
#define _SESSIONACTIONS_H

// Infra:
#include <AMDTApplicationFramework/Include/afActionCreatorAbstract.h>

//Local:
class CommandsHandler;

class SessionActions : public afActionCreatorAbstract
{
public:

    SessionActions();
    ~SessionActions();

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

    /// Application command handler:
    CommandsHandler* m_pCommandsHandler;
};

#endif //_SESSIONACTIONS_H
