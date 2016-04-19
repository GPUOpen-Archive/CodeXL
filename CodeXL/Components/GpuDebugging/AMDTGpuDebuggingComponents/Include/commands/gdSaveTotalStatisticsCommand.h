//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdSaveTotalStatisticsCommand.h
///
//==================================================================================

//------------------------------ gdSaveTotalStatisticsCommand.h ------------------------------

#ifndef __GDSAVETOTALSTATISTICSCOMMAND
#define __GDSAVETOTALSTATISTICSCOMMAND

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afCommand.h>

// ----------------------------------------------------------------------------------
// Class Name:           gdSaveTotalStatisticsCommand : public afCommand
// General Description:
// Save Total Statistics Command - Saves the total statistics data
// into a .csv file.
// Author:               Sigal Algranaty
// Creation Date:        26/8/2008
// ----------------------------------------------------------------------------------
class GD_API gdSaveTotalStatisticsCommand : public afCommand
{

public:
    gdSaveTotalStatisticsCommand(const osFilePath& fileName);
    virtual ~gdSaveTotalStatisticsCommand();

    // Overrides afCommand:
    virtual bool canExecuteSpecificCommand();
    virtual bool executeSpecificCommand();

private:
    osFilePath _filePath;
    gtASCIIString _totalStatisticsOutputString;


private:
    bool getTotalStatisticsData();
    bool writeTotalStatisticsDataToFile();

};


#endif  // __gdSaveTotalStatisticsCommand
