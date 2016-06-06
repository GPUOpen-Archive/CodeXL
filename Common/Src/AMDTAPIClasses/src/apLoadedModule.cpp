//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apLoadedModule.cpp
///
//==================================================================================

//------------------------------ apLoadedModule.cpp ------------------------------

// Local:
#include <AMDTAPIClasses/Include/apLoadedModule.h>


// ---------------------------------------------------------------------------
// Name:        apLoadedModule::apLoadedModule
// Description: Constructor.
// Author:  AMD Developer Tools Team
// Date:        9/10/2005
// ---------------------------------------------------------------------------
apLoadedModule::apLoadedModule()
    : _pModuleStartAddress(0), _pModuleLoadedSize(0)
{
}
