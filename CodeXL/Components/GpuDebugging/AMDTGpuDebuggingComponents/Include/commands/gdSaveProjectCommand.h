//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdSaveProjectCommand.h
///
//==================================================================================

//------------------------------ gdSaveProjectCommand.h ------------------------------

#ifndef __GDSAVEPROJECTCOMMAND
#define __GDSAVEPROJECTCOMMAND

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTAPIClasses/Include/apCounterScope.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afCommand.h>


// ----------------------------------------------------------------------------------
// Class Name:           gdSaveProjectCommand : public afCommand
// General Description:
// Save project Command - Saves the current configuration into a .gdb file.
// Author:               Avi Shapira
// Creation Date:        6/11/2003
// ----------------------------------------------------------------------------------
class GD_API gdSaveProjectCommand : public afCommand
{
public:
    gdSaveProjectCommand();
    virtual ~gdSaveProjectCommand();

    // Overrides afCommand:
    virtual bool canExecuteSpecificCommand();
    virtual bool executeSpecificCommand();

    bool getXMLOutputString(gtString& xmlOutputString);

private:

    bool GetProjectSettings();
    bool initXMLDebuggerTextStructure(TiXmlDocument& xmlDoc);
    bool setDebuggerData(TiXmlHandle& docHandle, const gtString& dataFieldName, const gtString& dataFieldValue);
    bool setDebugOutputData(TiXmlHandle& docHandle, const gtString& dataFieldName, const gtString& dataFieldValue);
    bool saveBreakpointsData(TiXmlHandle& docHandle);
    bool saveStateVariablesData(TiXmlHandle& docHandle);
    bool writeStringToXml(TiXmlNode* pXmlNode, const gtString& dataFieldValue);
    bool saveDebugSettingsAsString();
    bool addBreakpointProperty(TiXmlNode* pBreakpointNode, const gtString& breakpointPropertyNode, const gtString& dataFieldValue);
    bool setBreakpointData(TiXmlNode* pBreakpointsXMLNode, gtString breakpointTypeStr, gtString breakpointStringValue, bool isBreakpointEnabled, int lineNumber = -1);
    bool setOpenGLStateVariableData(TiXmlNode* pStateVariablesNode, const gtString& functionName);

private:

    // The project settings for save:
    apDebugProjectSettings m_projectSettings;

    // OpenGL debug output:
    gtString _glDebugOutputLoggingEnabledString;
    gtString _glDebugOutputBreakOnReportsString;
    gtString _debugOutputMessagesMaskString;
    gtString _debugOutputSeverityString;

    // Contain the XML output as string:
    gtString m_xmlOutputString;


};

#endif  // __GDSAVEPROJECTCOMMAND
