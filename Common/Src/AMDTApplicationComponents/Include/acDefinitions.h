//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acDefinitions.h
///
//==================================================================================

//------------------------------ acDefinitions.h ------------------------------

#ifndef __ACDEFINITIONS
#define __ACDEFINITIONS

#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// The default text char height and average width, measured in pixels:
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #define AC_DEFAULT_TEXT_CHAR_HEIGHT 14
    #define AC_DEFAULT_TEXT_AVERAGE_CHAR_WIDTH 7
#else
    #define AC_DEFAULT_TEXT_CHAR_HEIGHT 12
    #define AC_DEFAULT_TEXT_AVERAGE_CHAR_WIDTH 5
#endif

// A sash (of a splitter) width, measured in pixels:
#define AC_SASH_WIDTH 6

// The maximal number of filters that can be applied to an image or data view:
#define AC_MAX_RAW_FILE_FILTER_HANDLERS 2


#endif  // __ACDEFINITIONS
