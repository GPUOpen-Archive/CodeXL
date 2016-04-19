//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdSaveStateChangeStatisticsCommand.h
///
//==================================================================================

//------------------------------ gdSaveStateChangeStatisticsCommand.h ------------------------------

#ifndef __GDSAVESTATECHANGESTATISTICSCOMMAND
#define __GDSAVESTATECHANGESTATISTICSCOMMAND

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afCommand.h>

// ----------------------------------------------------------------------------------
// Class Name:           gdSaveStateChangeStatisticsCommand : public afCommand
// General Description:
// Save Total Statistics Command - Saves the total statistics data
// into a .csv file.
// Author:               Sigal Algranaty
// Creation Date:        26/8/2008
// ----------------------------------------------------------------------------------
class GD_API gdSaveStateChangeStatisticsCommand : public afCommand
{

public:
    gdSaveStateChangeStatisticsCommand(const osFilePath& fileName);
    virtual ~gdSaveStateChangeStatisticsCommand();

    // Overrides afCommand:
    virtual bool canExecuteSpecificCommand();
    virtual bool executeSpecificCommand();

private:
    osFilePath _filePath;
    gtASCIIString _stateChangeStatisticsOutputString;


private:
    bool getStateChangeStatisticsData();
    bool writeStateChangeStatisticsDataToFile();

};


#endif  // __GDSAVESTATECHANGESTATISTICSCOMMAND
