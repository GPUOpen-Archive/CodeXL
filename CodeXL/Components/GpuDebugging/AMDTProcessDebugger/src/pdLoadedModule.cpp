//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdLoadedModule.cpp
///
//==================================================================================

//------------------------------ pdLoadedModule.cpp ------------------------------

// Local:
#include <src/pdLoadedModule.h>


// ---------------------------------------------------------------------------
// Name:        pdLoadedModule::pdLoadedModule
// Description: Constructor.
// Author:      Yaki Tebeka
// Date:        12/7/2010
// ---------------------------------------------------------------------------
pdLoadedModule::pdLoadedModule()
    : _isDriverModule(false), _isSpyModule(false)
{
}


// ---------------------------------------------------------------------------
// Name:        pdLoadedModule::~pdLoadedModule
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        12/7/2010
// ---------------------------------------------------------------------------
pdLoadedModule::~pdLoadedModule()
{
}


// ---------------------------------------------------------------------------
// Name:        pdLoadedModule::getKey
// Description: Returns the loaded module start address.
// Author:      Yaki Tebeka
// Date:        12/7/2010
// ---------------------------------------------------------------------------
gtUInt64 pdLoadedModule::getKey() const
{
    return _loadedModuleData._pModuleStartAddress;
}

