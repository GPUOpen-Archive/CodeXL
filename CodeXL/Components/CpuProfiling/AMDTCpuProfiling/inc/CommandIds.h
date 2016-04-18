//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CommandIds.h
/// \brief  The CodeAnayst component command ids used to communicate with the framework
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/inc/CommandIds.h#10 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================
#ifndef _CACOMMANDIDS_H
#define _CACOMMANDIDS_H

/// \TODO Handle dynamic profiles
/// \TODO Handle export

/// Enums covering the inital static menu items
enum caMenuItemCommands
{
    //Profile menu:
    /// Opens the settings to the Profiling tab
    ID_CPU_SESSION_SETTINGS_DIALOG,

    caAmountOfMenuCommands
};

enum caCommands
{
    ID_SHOW_HIDE_INFO
};

#endif //_CACOMMANDIDS_H
