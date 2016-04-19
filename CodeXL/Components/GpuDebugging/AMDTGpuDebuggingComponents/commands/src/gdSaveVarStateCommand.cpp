//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdSaveVarStateCommand.cpp
///
//==================================================================================

//------------------------------ gdSaveVarStateCommand.cpp ------------------------------

// Qt
#include <QtWidgets>

// Infra
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTAPIClasses/Include/apOpenGLAPIType.h>
#include <AMDTAPIClasses/Include/apParameters.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/dialogs/gdStateVariablesDialog.h>
#include <AMDTGpuDebuggingComponents/Include/commands/gdSaveVarStateCommand.h>

gdSaveVarStateCommand::gdSaveVarStateCommand(const osFilePath& fileName)
    : _filePath(fileName)
{
}

// ---------------------------------------------------------------------------
// Name:        gdSaveVarStateCommand::~gdSaveVarStateCommand
// Description: Destructor.
// Author:      Avi Shapira
// Date:        10/1/2007
// ---------------------------------------------------------------------------
gdSaveVarStateCommand::~gdSaveVarStateCommand()
{
}

// ---------------------------------------------------------------------------
// Name:        gdSaveVarStateCommand::canExecuteSpecificCommand
// Description: Answers the question - can we save the state varibles of the G-Debugger application.
// Author:      Avi Shapira
// Date:        10/11/2003
// Implementation Notes:
// Currently - the answer is always yes.
// ---------------------------------------------------------------------------
bool gdSaveVarStateCommand::canExecuteSpecificCommand()
{
    return true;
}

// ---------------------------------------------------------------------------
// Name:        gdSaveVarStateCommand::executeSpecificCommand
// Description: Save the state variables of the G-Debugger application.
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        10/11/2003
// ---------------------------------------------------------------------------
bool gdSaveVarStateCommand::executeSpecificCommand()
{
    bool rc = false;

    rc = getStateVariables();
    rc = rc && writeStateVariablesToFile();

    return rc;
}

// ---------------------------------------------------------------------------
// Name:        gdSaveVarStateCommand::getStateVariables
// Description: Get the OpenGL state variables and their value into _stateVarOutputString
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        2/8/2004
// ---------------------------------------------------------------------------
bool gdSaveVarStateCommand::getStateVariables()
{
    _chosenContextId._contextId = 0;
    _chosenContextId._contextType = AP_OPENGL_CONTEXT;
    bool retVal = true;
    gtString stateVariableName;
    gtAutoPtr<apParameter> aptrStateVariableValue;
    gtString StateVariableValue = AF_STR_Empty;
    int amountOfStateVariables = 0;
    _stateVarOutputString = AF_STR_Empty;

    // Get a mask of valid state variable types for the current project and platform:
    unsigned int validStateVariablesMask = gdStateVariablesDialog::getValidStateVariableTypesMask();

    // Get the selected context id:
    gdGDebuggerGlobalVariablesManager& globalVarsManager = gdGDebuggerGlobalVariablesManager::instance();
    _chosenContextId = globalVarsManager.chosenContext();

    // Get the amount of monitored state variables:
    retVal = gaGetAmountOfOpenGLStateVariables(amountOfStateVariables);

    if (retVal)
    {
        // Iterate the monitored state variables:
        for (int stateVariableId = 0; stateVariableId < amountOfStateVariables; stateVariableId++)
        {
            // Get the state variable type (OpenGL / OpenGL ES):
            unsigned int stateVariableGlobalType = 0;
            bool rc1 = gaGetOpenGLStateVariableGlobalType(stateVariableId, stateVariableGlobalType);

            // If the current state variable's type is valid:
            if (rc1 && (stateVariableGlobalType & validStateVariablesMask))
            {
                // Get the current state variable name
                bool rc = gaGetOpenGLStateVariableName(stateVariableId, stateVariableName);

                if (rc)
                {
                    // Get the variable value
                    rc = rc && gaGetOpenGLStateVariableValue(_chosenContextId._contextId, stateVariableId, aptrStateVariableValue);

                    if (rc)
                    {
                        gtString tempStateVariableValue;
                        aptrStateVariableValue->valueAsString(tempStateVariableValue);
                        StateVariableValue = tempStateVariableValue.asCharArray();
                    }
                    else
                    {
                        StateVariableValue = AF_STR_NotAvailable;
                    }
                }
                else
                {
                    stateVariableName = GD_STR_StateVariablesUnknownVar;
                }

                // Remove the AF_STR_NewLine from the variable value
                // we would like to have the matrix's value in the output file as one line...
                StateVariableValue.replace(AF_STR_NewLine, AF_STR_Empty, true);

                _stateVarOutputString.append(stateVariableName);
                _stateVarOutputString.append(L"=");
                _stateVarOutputString.append(StateVariableValue);
                _stateVarOutputString.append(AF_STR_NewLine);

            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdSaveVarStateCommand::writeStateVariablesToFile
// Description: Write the state variables into a file
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        2/8/2004
// ---------------------------------------------------------------------------
bool gdSaveVarStateCommand::writeStateVariablesToFile()
{
    bool retVal = true;
    gtString fileHeader;
    osTime fileSavedDateAndTime;
    gtString fileSavedDate;
    gtString fileSavedTime;
    gtString chosenContextIdString;

    chosenContextIdString.appendFormattedString(L"%d", _chosenContextId._contextId);

    osFilePath filePath(_filePath);
    osFile stateVarFile;
    retVal = stateVarFile.open(filePath, osChannel::OS_UNICODE_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);

    if (retVal)
    {
        fileSavedDateAndTime.setFromCurrentTime();

        fileSavedDateAndTime.dateAsString(fileSavedDate, osTime::WINDOWS_STYLE, osTime::LOCAL);
        fileSavedDateAndTime.timeAsString(fileSavedTime, osTime::WINDOWS_STYLE, osTime::LOCAL);

        // Get the current project name:
        gtString projectFileName = afProjectManager::instance().currentProjectSettings().projectName();

        fileHeader.append(L"////////////////////////////////////////////////////////////");
        fileHeader.append(AF_STR_NewLine);
        fileHeader.append(L"// This File contain a snapshot of OpenGL state variables\n");
        fileHeader.append(L"// Project name: ") += projectFileName += AF_STR_NewLine;
        fileHeader.append(L"// Generation date: ") += fileSavedDate += AF_STR_NewLine;
        fileHeader.append(L"// Generation time: ") += fileSavedTime += AF_STR_NewLine;
        fileHeader.append(L"// Context id: ") += chosenContextIdString += AF_STR_NewLine;
        fileHeader.append(L"//\n");
        fileHeader.append(L"// Generated by CodeXL - an OpenCL and OpenGL Debugger\n");
        fileHeader.append(L"// http://gpuopen.com/\n");
        fileHeader.append(L"////////////////////////////////////////////////////////////\n\n");

        stateVarFile << fileHeader;
        stateVarFile << _stateVarOutputString;
        stateVarFile.close();
    }

    return retVal;
}
