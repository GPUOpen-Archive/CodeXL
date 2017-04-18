//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/OccupancyInfo.h $
/// \version $Revision: #6 $
/// \brief :  This file contains OccupancyInfo class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/OccupancyInfo.h#6 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================
#ifndef _OCCUPANCYINFO_H_
#define _OCCUPANCYINFO_H_

#include <IOccupancyFileInfoDataHandler.h>

typedef QMap<osThreadId, QList<const IOccupancyInfoDataHandler*>> OccupancyTable;

#endif // _OCCUPANCYINFO_H_

