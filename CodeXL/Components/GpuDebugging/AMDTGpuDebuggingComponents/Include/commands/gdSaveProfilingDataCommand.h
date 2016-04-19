//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdSaveProfilingDataCommand.h
///
//==================================================================================

//------------------------------ gdSaveProfilingDataCommand.h ------------------------------

#ifndef __GDSAVEPROFILINGDATACOMMAND
#define __GDSAVEPROFILINGDATACOMMAND

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afCommand.h>

// ----------------------------------------------------------------------------------
// Class Name:           gdSaveProfilingDataCommand : public afCommand
// General Description:
// Save Profiling data Command - Saves the performance data into a .csv file.
// Author:               Avi Shapira
// Creation Date:        9/8/2005
// ----------------------------------------------------------------------------------
class GD_API gdSaveProfilingDataCommand : public afCommand
{
public:
    gdSaveProfilingDataCommand(const osFilePath& fileName);
    virtual ~gdSaveProfilingDataCommand();

    // Overrides afCommand:
    virtual bool canExecuteSpecificCommand();
    virtual bool executeSpecificCommand();

    static bool canExecuteSaveProfilingDataCommand();

private:
    const osFilePath& _filePath;
    gtString _profilingDataOutputString;


private:
    bool getProfilingData();
    bool writeProfilingDataToFile();

};


#endif  // __GDSAVEPROFILINGDATACOMMAND
