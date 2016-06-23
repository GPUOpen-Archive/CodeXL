//=============================================================
// Copyright (c) 2013 Advanced Micro Devices, Inc.
//=============================================================

#ifndef __PPSESSIONACTIONSCREATOR_H
#define __PPSESSIONACTIONSCREATOR_H

// Infra:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <TSingleton.h>
#include <AMDTApplicationFramework/Include/afActionCreatorAbstract.h>

/// Local:
#include <AMDTPowerProfiling/Include/ppAMDTPowerProfilingDLLBuild.h>

class PP_API ppSessionActionsCreator : public afActionCreatorAbstract, public TSingleton<ppSessionActionsCreator>
{
    // required so that TSingleton can access our constructor
    friend class TSingleton<ppSessionActionsCreator>;

public:

    ppSessionActionsCreator() = default;
    virtual ~ppSessionActionsCreator() = default;

    // Virtual functions that needs to be implemented:

    // Menu position.
    // Each hierarchy on the ,emu include name/priority.
    // If separator is needed after the item then 's' after the priority is needed
    // in case of a sub menu if one item is marked with an 's' it is enough to mark a separator after it:
    virtual gtString menuPosition(int actionIndex, afActionPositionData& positionData) override;

    // Toolbar position. separators are defined by "/":
    // Position is defined as in menus:
    virtual gtString toolbarPosition(int actionIndex) override;

    // Get the group command ids for the action:
    virtual void groupAction(int actionIndex) override;

protected:

    // Each derived class must populate the vector of supported command Ids
    virtual void populateSupportedCommandIds() override;
};

#endif //__PPSESSIONACTIONSCREATOR_H

