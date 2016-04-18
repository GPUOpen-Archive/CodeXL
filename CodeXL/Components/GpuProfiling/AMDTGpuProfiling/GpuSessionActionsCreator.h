//=============================================================
// Copyright (c) 2013 Advanced Micro Devices, Inc.
//=============================================================

#ifndef __GPUSESSIONACTIONSCREATOR_H
#define __GPUSESSIONACTIONSCREATOR_H

// Infra:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

#include <TSingleton.h>
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>

// Infra:
#include <AMDTApplicationFramework/Include/afActionCreatorAbstract.h>

class AMDT_GPU_PROF_API GpuSessionActionsCreator : public afActionCreatorAbstract, public TSingleton<GpuSessionActionsCreator>
{
    // required so that TSingleton can access our constructor
    friend class TSingleton<GpuSessionActionsCreator>;

public:

    GpuSessionActionsCreator();
    ~GpuSessionActionsCreator();

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

#endif //__GPUSESSIONACTIONSCREATOR_H

