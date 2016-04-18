//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdLoadedModule.h
///
//==================================================================================

//------------------------------ pdLoadedModule.h ------------------------------

#ifndef __PDLOADEDMODULE_H
#define __PDLOADEDMODULE_H

// Infra:
#include <AMDTBaseTools/Include/gtRedBlackTree.h>
#include <AMDTAPIClasses/Include/apLoadedModule.h>


// ----------------------------------------------------------------------------------
// Class Name:           pdLoadedModule : public gtRedBlackTreeValue
// General Description:
//  Represents a module loaded into the debugged process address space.
//  Inherits gtRedBlackTreeValue to enable the usage of a red-black tree for
//  holding loaded modules data and querying loaded modules efficiently.
//
// Author:               Yaki Tebeka
// Creation Date:        12/7/2010
// ----------------------------------------------------------------------------------
struct pdLoadedModule : public gtRedBlackTreeValue
{
public:
    pdLoadedModule();
    virtual ~pdLoadedModule();

    // Overrides gtRedBlackTreeValue:
    virtual gtUInt64 getKey() const;

public:
    // The loaded module's data:
    apLoadedModule _loadedModuleData;

    // Contains true iff the represented module is a driver module:
    bool _isDriverModule;

    // Contains true iff the represented module is a "Spy" module:
    bool _isSpyModule;
};

#endif //__PDLOADEDMODULE_H

