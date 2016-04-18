//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdSaveVarStateCommand.h
///
//==================================================================================

//------------------------------ gdSaveVarStateCommand.h ------------------------------

#ifndef __GDSAVEVARSTATECOMMAND
#define __GDSAVEVARSTATECOMMAND

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTAPIClasses/Include/apCounterID.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afCommand.h>


// ----------------------------------------------------------------------------------
// Class Name:           gdSaveVarStateCommand : public afCommand
// General Description:
// Save project Command - Saves the state variables into a .var file.
// Author:               Avi Shapira
// Creation Date:        10/11/2003
// ----------------------------------------------------------------------------------
class GD_API gdSaveVarStateCommand : public afCommand
{
public:
    gdSaveVarStateCommand(const osFilePath& fileName);
    virtual ~gdSaveVarStateCommand();

    // Overrides afCommand:
    virtual bool canExecuteSpecificCommand();
    virtual bool executeSpecificCommand();

private:
    osFilePath _filePath;
    apContextID _chosenContextId;
    gtString _stateVarOutputString;


private:
    bool getStateVariables();
    bool writeStateVariablesToFile();

};


#endif  // __GDSAVEVARSTATECOMMAND
