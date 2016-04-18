//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdLoadProjectCommand.h
///
//==================================================================================

//------------------------------ gdLoadProjectCommand.h ------------------------------

#ifndef __GDLOADPROJECTCOMMAND
#define __GDLOADPROJECTCOMMAND

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTAPIClasses/Include/apGLDebugOutput.h>
#include <AMDTAPIClasses/Include/apCounterScope.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afCommand.h>

// ----------------------------------------------------------------------------------
// Class Name:           gdLoadProjectCommand : public afCommand
// General Description:
// Save project Command - Load the project from a .gdb file.
// Author:               Avi Shapira
// Creation Date:        10/11/2003
// ----------------------------------------------------------------------------------
class GD_API gdLoadProjectCommand : public afCommand
{
public:
    gdLoadProjectCommand(const gtString& xmlInput);
    virtual ~gdLoadProjectCommand();

    // Overrides afCommand:
    virtual bool canExecuteSpecificCommand();
    virtual bool executeSpecificCommand();

    // Get the settings:
    const apDebugProjectSettings& loadedProjectSettings() const {return m_debugProjectSettings;};

protected:

    bool SaveCurrentProject();
    bool ReadProjectSettingsFromString();
    bool ReadProjectDebuggingSettings(TiXmlHandle& docHandle);
    void handleFileVersionCompatibility(TiXmlHandle& docHandle);
    bool readBreakpointsSettings(TiXmlNode* pBreakpointsNode);
    bool readBreakpointProperty(TiXmlNode& breakpointPropertyNode, gtString& propertyToReadNodeName, gtString& dataFieldValue);
    bool readOpenGLStateVariables(TiXmlNode* pStateVariableNode);

protected:

    gtString m_xmlStringInput;

    // Holds the loaded project:
    apDebugProjectSettings m_debugProjectSettings;

};

#endif  // __GDLOADPROJECTCOMMAND

