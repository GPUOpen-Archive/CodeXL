//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osAtomic.h
///
//=====================================================================
#ifndef __OSATOMIC
#define __OSATOMIC

#include <AMDTBaseTools/Include/AMDTDefinitions.h>

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    #include "../src/win32/osAtomicImpl.h"
#elif (AMDT_BUILD_TARGET == AMDT_LINUX_OS)
    #if (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT)
        #include "../src/linux/osAtomicImpl.h"
    #else
        #       error Error: no atomic operations defined for build target!
    #endif
#endif

#endif // __OSATOMIC
