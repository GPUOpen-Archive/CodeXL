//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file generate occupancy charts
//==============================================================================

#ifndef _OCCUPANCY_CHART_H_
#define _OCCUPANCY_CHART_H_
#include <string>
#include "OccupancyUtils.h"

bool GenerateOccupancyChart(const OccupancyUtils::OccupancyParams& params, const std::string& strFileName, std::string& strError);

#endif //_OCCUPANCY_CHART_H_
