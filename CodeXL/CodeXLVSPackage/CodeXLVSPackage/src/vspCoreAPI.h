//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspCoreAPI.h
///
//==================================================================================
#ifndef __VSPCOREAPI_H
#define __VSPCOREAPI_H

// Infra:
#include <src/vscVsUtils.h>
#include <Include/Public/CoreInterfaces/IVscCoreAPI.h>

extern const IVscCoreAPI& gr_coreAPIPointers;
extern const bool& gr_coreAPIPointersInitialized;

bool vspInitializeCoreAPI(void* hCurrentModule);

// Helper for calling functions:
#define VSCORE(f) gr_coreAPIPointers.f

#endif // __VSPCOREAPI_H

