//=====================================================================
// Copyright 2012-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file 
/// 
//=====================================================================
#ifndef _SC_LOADER_H_
#define _SC_LOADER_H_

#include "v0_7/api_defs.h"


//! Structure that holds the required functions 
// that sc exports for the SCDLL infrastructure.
typedef struct _sc_loader_rec_0_7 {
  size_t struct_size;
  bool isBuiltinSC;
  uint32_t /*SC_UINT32*/ sc_interface_version;
  void*  /**SC_EXPORT_FUNCTIONS**/ scef;
  /* Any version specific fields go here. */
} SCLoader_0_7;


//! The function SetupSCLoader() creats a SCLoader object
// and correctly initializes it. The caller of this 
// function must also call TeardownSCLoader() so any 
// dynamically allocated memory is released.
SCLoader_0_7* AOC_INTERFACE  SetupSCLoader_0_7() AOC_API_0_7;

//! The function TeardownSCLoader() uninitializes any memory
// that was allocated by the setup function and if needed
// unloads the DLL.
void AOC_INTERFACE  TeardownSCLoader_0_7(SCLoader_0_7** loader) AOC_API_0_7;


#endif // _SC_LOADER_H_
