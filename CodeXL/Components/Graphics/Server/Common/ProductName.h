//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Components from PerfStudio are used in both CodeXL and PerfStudio
/// This file defines a few macros that ensure the correct product name
/// is used based on where the components are used
//==============================================================================

#ifndef GPS_PRODUCTNAME_INCLUDED
#define GPS_PRODUCTNAME_INCLUDED

#ifdef CODEXL_GRAPHICS
    #define PRODUCTNAME "CodeXL" ///< Definition
#else // CODEXL_GRAPHICS
    #define PRODUCTNAME "GPU PerfStudio" ///< Definition
#endif // CODEXL_GRAPHICS


#endif // GPS_MICRODLLNAME_INCLUDED
