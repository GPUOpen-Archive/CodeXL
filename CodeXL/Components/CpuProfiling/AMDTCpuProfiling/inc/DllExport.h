//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file DllExport.h
/// \brief  The export description for the CodeAnalyst component
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/inc/DllExport.h#5 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================
#ifndef _DLLEXPORT_H
#define _DLLEXPORT_H

#if defined(_WIN32)

    #ifdef AMDT_CPU_PROFILING_EXPORTS
        #define AMDT_CPU_PROF_API __declspec(dllexport)
    #else
        #define AMDT_CPU_PROF_API __declspec(dllimport)
    #endif

#else
    #define AMDT_CPU_PROF_API
#endif

#endif //_DLLEXPORT_H
