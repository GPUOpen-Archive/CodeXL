//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCounterNames.h
///
//==================================================================================

//------------------------------ apCounterNames.h ------------------------------

#ifndef __APCOUNTERNAMES_H
#define __APCOUNTERNAMES_H

#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtString.h>

// ----------------------------------------------------------------------------------
// General Description:
//  This file contains predefined performance counter names.
//
// Author:  AMD Developer Tools Team
// Creation Date:        13/3/2008
// ----------------------------------------------------------------------------------


// Predefined counter names and descriptions:
// =========================================

// Average CPUs global utilization:
#define AP_CPU_AVERAGE_UTILIZATION_COUNTER_NAME L"CPUs Average Utilization"
#define AP_CPU_AVERAGE_UTILIZATION_COUNTER_DESCRIPTION L"Percentage of average elapsed CPUs time spent executing non-idle threads. This counter is the primary indicator of processor activity"

// Average CPUs user mode utilization:
#define AP_CPU_AVERAGE_USER_MODE_UTILIZATION_COUNTER_NAME L"CPUs Average User Mode Utilization"
#define AP_CPU_AVERAGE_USER_MODE_UTILIZATION_COUNTER_DESCRIPTION L"Percentage of average elapsed time that the CPUs spent executing code in user mode. User mode is a restricted processing mode designed for applications"

// Average CPUs privileged / system (kernel) mode utilization:
// The terminology is:
// - Windows - Privileged mode
// - Linux / Mac - System Mode
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #define AP_CPU_AVERAGE_PRIVILEGED_MODE_UTILIZATION_COUNTER_NAME L"CPUs Average System Mode Utilization"
    #define AP_CPU_AVERAGE_PRIVILEGED_MODE_UTILIZATION_COUNTER_DESCRIPTION L"Percentage of average elapsed time that the CPUs spent executing code in system mode. System mode is a non-restricted processing mode designed for operating system components (native operating system components, drivers, etc) that allows direct access to hardware and memory"
#else
    #define AP_CPU_AVERAGE_PRIVILEGED_MODE_UTILIZATION_COUNTER_NAME L"CPUs Average Privileged Mode Utilization"
    #define AP_CPU_AVERAGE_PRIVILEGED_MODE_UTILIZATION_COUNTER_DESCRIPTION L"Percentage of average elapsed time that the CPUs spent executing code in privileged mode. Privileged mode is a non-restricted processing mode designed for operating system components (native operating system components, drivers, etc) that allows direct access to hardware and memory"
#endif

// CPU x global utilization:
#define AP_CPU_UTILIZATION_COUNTER_NAME L"CPU %d Utilization"
#define AP_CPU_UTILIZATION_COUNTER_DESCRIPTION L"Percentage of elapsed CPU %d time spent executing non-idle threads. This counter is the primary indicator of processor activity"

// CPU x user mode utilization:
#define AP_CPU_USER_MODE_UTILIZATION_COUNTER_NAME L"CPU %d User Mode Utilization"
#define AP_CPU_USER_MODE_UTILIZATION_COUNTER_DESCRIPTION L"Percentage of elapsed time that the CPU %d spent executing code in user mode. User mode is a restricted processing mode designed for applications"

// CPU x user privileged / system mode utilization:
// The terminology is:
// - Windows - Privileged mode
// - Linux / Mac - System Mode
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #define AP_CPU_PRIVILEGED_MODE_UTILIZATION_COUNTER_NAME L"CPU %d System Utilization"
    #define AP_CPU_PRIVILEGED_MODE_UTILIZATION_COUNTER_DESCRIPTION L"Percentage of elapsed time that CPU %d spent executing code in system (kernel) mode. System mode is a non-restricted processing mode designed for operating system components (native operating system components, drivers, etc) that allows direct access to hardware and memory"
#else
    #define AP_CPU_PRIVILEGED_MODE_UTILIZATION_COUNTER_NAME L"CPU %d Privileged Mode Utilization"
    #define AP_CPU_PRIVILEGED_MODE_UTILIZATION_COUNTER_DESCRIPTION L"Percentage of elapsed time that the CPU %d spent executing code in privileged mode. Privileged mode is a non-restricted processing mode designed for operating system components (native operating system components, drivers, etc) that allows direct access to hardware and memory"
#endif


#endif //__APCOUNTERNAMES_H L"


