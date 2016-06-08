//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apLoadedModule.h
///
//==================================================================================

//------------------------------ apLoadedModule.h ------------------------------

#ifndef __APLOADEDMODULE_H
#define __APLOADEDMODULE_H

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apLoadedModule
// General Description:
//   Represents a module (.dll / .ext file) that was loaded into memory.
// Author:  AMD Developer Tools Team
// Creation Date:        9/10/2004
// ----------------------------------------------------------------------------------
struct AP_API apLoadedModule
{
    // The path of the module file:
    osFilePath _moduleFilePath;

    // The module loaded address:
    osInstructionPointer _pModuleStartAddress;

    // The module loaded size:
    size_t _pModuleLoadedSize;

public:
    apLoadedModule();
};


#endif //__APLOADEDMODULE_H
