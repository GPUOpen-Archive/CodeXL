//=====================================================================
// Copyright 2012-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file 
/// 
//=====================================================================
// This is a compatibility header file. Either define the version
// of the compiler library that is to be used or include the
// header file for that version directly.
#ifndef ACL_TYPES_H_
#define ACL_TYPES_H_
#if WITH_VERSION_0_8
#include "v0_8/aclTypes.h"
#elif WITH_VERSION_0_9
#include "v0_9/aclTypes.h"
#else
#error "The compiler library version was not defined."
#include "v0_8/aclTypes.h"
#endif
#endif // ACL_TYPES_H_
