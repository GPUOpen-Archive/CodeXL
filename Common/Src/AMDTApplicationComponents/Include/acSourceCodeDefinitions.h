//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acSourceCodeDefinitions.h
///
//==================================================================================

//------------------------------ acSourceCodeDefinitions.h ------------------------------

#ifndef __ACSOURCECODEDEFINITIONS_H
#define __ACSOURCECODEDEFINITIONS_H

// Used for colors and size definitions for the source code view:

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define AC_SOURCE_CODE_EDITOR_DEFAULT_FONT_FAMILY "Consolas"
#else
    #define AC_SOURCE_CODE_EDITOR_DEFAULT_FONT_FAMILY "Monospace"
#endif
#define AC_SOURCE_CODE_EDITOR_DEFAULT_FONT_SIZE 10

#define AC_SOURCE_CODE_EDITOR_WHITE_SPACE_DEFAULT_COLOR QColor(43, 145, 175)
#define AC_SOURCE_CODE_EDITOR_MARGIN_BG_COLOR QColor(240, 240, 240, 255)
#define AC_SOURCE_CODE_EDITOR_MARGIN_FORE_COLOR QColor(43, 145, 175)
#define AC_SOURCE_CODE_EDITOR_GUIDES_BG_COLOR QColor(255,0,0)
#define AC_SOURCE_CODE_EDITOR_GUIDES_FORE_COLOR QColor(255,0,255)
#define AC_SOURCE_CODE_EDITOR_SELECTION_BG_COLOR QColor(51,153,255)

#endif //__ACSOURCECODEDEFINITIONS_H

