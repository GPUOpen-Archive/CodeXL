//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdSaveProjectCommand.cpp
///
//==================================================================================

//------------------------------ gdSaveProjectCommand.cpp ------------------------------

// Ignore warnings:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Qt:
#include <QtWidgets>




//TinyXml
#include <tinyxml.h>

// Infra
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osBundle.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTAPIClasses/Include/apAIDFunctions.h>
#include <AMDTAPIClasses/Include/apCounterInfo.h>
#include <AMDTAPIClasses/Include/apFrameTerminators.h>
#include <AMDTAPIClasses/Include/apGLDebugOutput.h>
#include <AMDTAPIClasses/Include/apKernelFunctionNameBreakpoint.h>
#include <AMDTAPIClasses/Include/apKernelSourceCodeBreakpoint.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionBreakPoint.h>
#include <AMDTAPIClasses/Include/apSourceCodeBreakpoint.h>
#include <AMDTAPIClasses/Include/apHostSourceBreakPoint.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/commands/gdSaveProjectCommand.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdStateVariablesView.h>

// ---------------------------------------------------------------------------
// Name:        gdSaveProjectCommand::gdSaveProjectCommand
// Description: Constructor.
// Arguments: fileName - The file into which the project will be saved.
// ---------------------------------------------------------------------------
gdSaveProjectCommand::gdSaveProjectCommand()
{
}

// ---------------------------------------------------------------------------
// Name:        gdSaveProjectCommand::~gdSaveProjectCommand
// Description: Destructor.
// Author:      Avi Shapira
// Date:        10/1/2007
// ---------------------------------------------------------------------------
gdSaveProjectCommand::~gdSaveProjectCommand()
{
}

// ---------------------------------------------------------------------------
// Name:        gdSaveProjectCommand::canExecuteSpecificCommand
// Description: Answers the question - can we save the project of the CodeXL application.
// Author:      Avi Shapira
// Date:        8/11/2003
// Implementation Notes:
//  Currently - the answer is always yes.
// ---------------------------------------------------------------------------
bool gdSaveProjectCommand::canExecuteSpecificCommand()
{
    return true;
}


// ---------------------------------------------------------------------------
// Name:        gdSaveProjectCommand::executeSpecificCommand
// Description: Save the G-Debugger application project.
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        8/11/2003
// ---------------------------------------------------------------------------
bool gdSaveProjectCommand::executeSpecificCommand()
{
    bool retVal = false;

    // Get the project settings:
    retVal = GetProjectSettings();

    if (retVal)
    {
        retVal = saveDebugSettingsAsString();
    }


    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdSaveProjectCommand::GetProjectSettings
// Description: Get the CodeXL project settings from the application
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        18/5/2004
// ---------------------------------------------------------------------------
bool gdSaveProjectCommand::GetProjectSettings()
{
    // Get the gdGDebuggerGlobalVariablesManager instance
    gdGDebuggerGlobalVariablesManager& theStateManager = gdGDebuggerGlobalVariablesManager::instance();

    // Get the process creation data:
    m_projectSettings = theStateManager.currentDebugProjectSettings();

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    {
        // Debug output settings:

        // Get the category mask:
        gtUInt64 categoryMask = 0;
        bool rc4 = gaGetGLDebugOutputKindMask(categoryMask);

        // Get the "Should Break On" debug output messages flag:
        bool breakOnDebugOutputReports = false;
        bool doesExist = false, isEnabled = false;
        bool rc5 = gaGetGenericBreakpointStatus(AP_BREAK_ON_DEBUG_OUTPUT, doesExist, isEnabled);
        breakOnDebugOutputReports = doesExist && isEnabled;

        // Check if the debug output Logging is enabled:
        bool rc6 = gaGetGLDebugOutputLoggingEnabledStatus(isEnabled);

        // Get the debug output messages severity:
        bool severities[AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES] = {0};
        bool rc7 = true;

        for (int i = 0; i < AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES; i++)
        {
            rc7 = gaGetGLDebugOutputSeverityEnabled((apGLDebugOutputSeverity)i, severities[i]) && rc7;
        }

        bool rcGLDebugOutput = rc4 && rc5 && rc6 && rc7;
        GT_ASSERT(rcGLDebugOutput);

        // Build the strings:
        _debugOutputMessagesMaskString.appendFormattedString(L"%llu", categoryMask);
        apGLDebugOutputSeveritiesAsString(severities, _debugOutputSeverityString);
        _glDebugOutputLoggingEnabledString = (isEnabled) ?  OS_STR_TrueXMLValueUnicode : OS_STR_FalseXMLValueUnicode;
        _glDebugOutputBreakOnReportsString = (breakOnDebugOutputReports) ?  OS_STR_TrueXMLValueUnicode : OS_STR_FalseXMLValueUnicode;

    }
#endif

    return true;
}

// ---------------------------------------------------------------------------
// Name:        gdSaveProjectCommand::saveBreakpointsData
// Description: Save the breakpoints into the XML
// Arguments:   TiXmlHandle& docHandle
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        7/8/2012
// ---------------------------------------------------------------------------
bool gdSaveProjectCommand::saveBreakpointsData(TiXmlHandle& docHandle)
{
    bool retVal = true;

    // Get the breakpoints node from the XML:
    // Get the data field
    TiXmlNode* pBreakpointsXMLNode = docHandle.FirstChild(GD_STR_projectSettingsExtensionNameASCII).FirstChild(GD_STR_loadProjectDebuggerBreakpoints).FirstChild(GD_STR_loadProjectBreakpoints).Node();
    GT_IF_WITH_ASSERT(pBreakpointsXMLNode != NULL)
    {
        int amountOfBreakpoints;

        // Iterate on the active breakpoints
        gaGetAmountOfBreakpoints(amountOfBreakpoints);

        for (int i = 0; i < amountOfBreakpoints; i++)
        {
            gtAutoPtr<apBreakPoint> aptrBreakpoint;
            // Get the current breakpoint
            gaGetBreakpoint(i, aptrBreakpoint);

            // Get the breakpoint type
            osTransferableObjectType curentBreakpointType = aptrBreakpoint->type();

            if (curentBreakpointType == OS_TOBJ_ID_MONITORED_FUNC_BREAKPOINT)
            {
                // Down cast it to apMonitoredFunctionBreakPoint:
                apMonitoredFunctionBreakPoint* pFunctionBreakpoint = (apMonitoredFunctionBreakPoint*)(aptrBreakpoint.pointedObject());

                // get the breakpoint function id
                apMonitoredFunctionId funcId = pFunctionBreakpoint->monitoredFunctionId();

                // get the breakpoint function name
                gtString functionName;
                gaGetMonitoredFunctionName(funcId, functionName);

                // get the breakpoint enable / disable status
                bool isBreakpointEnabled;
                isBreakpointEnabled = pFunctionBreakpoint->isEnabled();

                // Add the breakpoint to the XML file:
                retVal = setBreakpointData(pBreakpointsXMLNode, GD_STR_loadProjectBreakpointTypeFunction, functionName, isBreakpointEnabled) && retVal;
            }

            else if (curentBreakpointType == OS_TOBJ_ID_KERNEL_FUNC_NAME_BREAKPOINT)
            {
                // Down cast it to apKernelFunctionNameBreakpoint:
                apKernelFunctionNameBreakpoint* pFunctionBreakpoint = (apKernelFunctionNameBreakpoint*)(aptrBreakpoint.pointedObject());
                GT_IF_WITH_ASSERT(pFunctionBreakpoint != NULL)
                {
                    // get the breakpoint enable / disable status
                    bool isBreakpointEnabled;
                    isBreakpointEnabled = pFunctionBreakpoint->isEnabled();

                    // Add the breakpoint to the XML file:
                    retVal = setBreakpointData(pBreakpointsXMLNode, GD_STR_loadProjectBreakpointTypeKernel, pFunctionBreakpoint->kernelFunctionName(), isBreakpointEnabled) && retVal;
                }
            }

            else if (curentBreakpointType == OS_TOBJ_ID_GENERIC_BREAKPOINT)
            {
                // Down cast it to apGenericBreakpoint:
                apGenericBreakpoint* pGenericBreakpoint = (apGenericBreakpoint*)(aptrBreakpoint.pointedObject());
                GT_IF_WITH_ASSERT(pGenericBreakpoint != NULL)
                {
                    // get the breakpoint enable / disable status:
                    bool isBreakpointEnabled;
                    isBreakpointEnabled = pGenericBreakpoint->isEnabled();

                    // Add the breakpoint to the XML file:
                    gtString genericBreakpointName;
                    bool rc = apGenericBreakpoint::breakpointTypeToString(pGenericBreakpoint->breakpointType(), genericBreakpointName);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        // Add the breakpoint to the XML file:
                        retVal = setBreakpointData(pBreakpointsXMLNode, GD_STR_loadProjectBreakpointTypeGeneric, genericBreakpointName, isBreakpointEnabled) && retVal;
                    }
                }
            }

            else if (curentBreakpointType == OS_TOBJ_ID_SOURCE_CODE_BREAKPOINT)
            {
                // Down cast it to apSourceCodeBreakpoint:
                apSourceCodeBreakpoint* pSourceCodeBreakpoint = (apSourceCodeBreakpoint*)(aptrBreakpoint.pointedObject());
                GT_IF_WITH_ASSERT(pSourceCodeBreakpoint != NULL)
                {
                    // get the breakpoint enable / disable status:
                    bool isBreakpointEnabled;
                    isBreakpointEnabled = pSourceCodeBreakpoint->isEnabled();

                    // Add the breakpoint source code file path to the XML file:
                    int lineNumber = pSourceCodeBreakpoint->lineNumber();
                    retVal = setBreakpointData(pBreakpointsXMLNode, GD_STR_loadProjectBreakpointTypeSourceCode, pSourceCodeBreakpoint->filePath().asString(), isBreakpointEnabled, lineNumber) && retVal;
                }
            }

            else if (curentBreakpointType == OS_TOBJ_ID_HOST_SOURCE_CODE_BREAKPOINT)
            {
                // Down cast it to apHostSourceCodeBreakpoint:
                apHostSourceCodeBreakpoint* pSourceCodeBreakpoint = (apHostSourceCodeBreakpoint*)(aptrBreakpoint.pointedObject());
                GT_IF_WITH_ASSERT(pSourceCodeBreakpoint != NULL)
                {
                    // get the breakpoint enable / disable status:
                    bool isBreakpointEnabled;
                    isBreakpointEnabled = pSourceCodeBreakpoint->isEnabled();

                    // Add the breakpoint source code file path to the XML file:
                    int lineNumber = pSourceCodeBreakpoint->lineNumber();
                    retVal = setBreakpointData(pBreakpointsXMLNode, GD_STR_loadProjectBreakpointTypeHostSourceCode, pSourceCodeBreakpoint->filePath().asString(), isBreakpointEnabled, lineNumber) && retVal;
                }
            }

            else if (curentBreakpointType == OS_TOBJ_ID_KERNEL_SOURCE_CODE_BREAKPOINT)
            {
                // Down cast it to apSourceCodeBreakpoint:
                apKernelSourceCodeBreakpoint* pKernelSourceCodeBreakpoint = (apKernelSourceCodeBreakpoint*)(aptrBreakpoint.pointedObject());
                GT_IF_WITH_ASSERT(pKernelSourceCodeBreakpoint != NULL)
                {
                    // get the breakpoint enable / disable status:
                    bool isBreakpointEnabled;
                    isBreakpointEnabled = pKernelSourceCodeBreakpoint->isEnabled();

                    // Add the breakpoint source code file path to the XML file:
                    int lineNumber = pKernelSourceCodeBreakpoint->lineNumber();
                    retVal = setBreakpointData(pBreakpointsXMLNode, GD_STR_loadProjectBreakpointTypeKernelSourceCode, pKernelSourceCodeBreakpoint->unresolvedPath().asString(), isBreakpointEnabled, lineNumber) && retVal;
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdSaveProjectCommand::saveStateVariablesData
// Description: Save the selected state variables into the XML
// Arguments:   TiXmlHandle& docHandle
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        7/8/2012
// ---------------------------------------------------------------------------
bool gdSaveProjectCommand::saveStateVariablesData(TiXmlHandle& docHandle)
{
    bool retVal = true;

    // Get the state variables node from the XML:
    TiXmlNode* pStateVarsXMLNode = docHandle.FirstChild(GD_STR_projectSettingsExtensionNameASCII).FirstChild(GD_STR_loadProjectOpenGLStateVarNode).FirstChild(GD_STR_loadProjectStateVarNode).Node();
    GT_IF_WITH_ASSERT(pStateVarsXMLNode != NULL)
    {
        int amountOfSelectedVariables = 0;

        // Get the application command instance:
        gdApplicationCommands* pApplicationCommands = gdApplicationCommands::gdInstance();
        GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
        {
            // Get the state variables view:
            gdStateVariablesView* pStateVariableView = pApplicationCommands->stateVariablesView();
            GT_IF_WITH_ASSERT(pStateVariableView != NULL)
            {
                amountOfSelectedVariables = pStateVariableView->rowCount();

                if (amountOfSelectedVariables > 1)
                {
                    // Iterate on the selected OpenGL State Variables
                    // We are decreasing the amountOfSelectedVariables by 1 because the "Add State variable..." item

                    // Add the variables to the view:
                    for (int i = amountOfSelectedVariables - 1; i > 0; i--)
                    {
                        // Get the Selected Variable name
                        QString varName;
                        bool rc = pStateVariableView->getItemText(i - 1, 0, varName);

                        if (rc)
                        {
                            // Add the state variable to the XML file:
                            rc = setOpenGLStateVariableData(pStateVarsXMLNode, acQStringToGTString(varName));
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdSaveProjectCommand::initXMLDebuggerTextStructure
// Description: Create a string for the debugger settings as XML
// Arguments:   TiXmlDocument& xmlDoc
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        5/4/2012
// ---------------------------------------------------------------------------
bool gdSaveProjectCommand::initXMLDebuggerTextStructure(TiXmlDocument& xmlDoc)
{
    bool rc = true;

    // The project file format - we will use this file every time
    const char* pDebuggerSettingsXMLString =
        "<" GD_STR_projectSettingsExtensionNameASCII">\n"
        "<" GD_STR_loadProjectInterceptionMethodNodeASCII">0</" GD_STR_loadProjectInterceptionMethodNodeASCII">\n"
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
#ifdef GD_ALLOW_HSA_DEBUGGING
        "<" GD_STR_loadProjectDebugHSAKernelsNodeASCII">false</" GD_STR_loadProjectDebugHSAKernelsNodeASCII">\n"
#endif // GD_ALLOW_HSA_DEBUGGING
#endif // AMDT_BUILD_TARGET == AMDT_LINUX_OS
        "<" GD_STR_loadProjectDebuggerBreakpoints">\n"
        "<" GD_STR_loadProjectBreakpoints">\n"
        "</" GD_STR_loadProjectBreakpoints">\n"
        "</" GD_STR_loadProjectDebuggerBreakpoints">\n"
        "<" GD_STR_loadProjectOpenGLStateVarNode">\n"
        "<" GD_STR_loadProjectStateVarNode">\n"
        "</" GD_STR_loadProjectStateVarNode">\n"
        "</" GD_STR_loadProjectOpenGLStateVarNode">\n"
        "<" GD_STR_loadProjectFrameTerminatorsNodeASCII">4</" GD_STR_loadProjectFrameTerminatorsNodeASCII">\n"
        "<" GD_STR_loadProjectGLDebugOutputNode">\n"
        "<" GD_STR_loadProjectGLDebugOutputLoggingEnabledNodeASCII">true</" GD_STR_loadProjectGLDebugOutputLoggingEnabledNodeASCII">\n"
        "<" GD_STR_loadProjectGLDebugOutputBreakOnReportsNodeASCII">false</" GD_STR_loadProjectGLDebugOutputBreakOnReportsNodeASCII">\n"
        "<" GD_STR_loadProjectGLDebugOutputMessagesMaskNodeASCII">0</" GD_STR_loadProjectGLDebugOutputMessagesMaskNodeASCII">\n"
        "<" GD_STR_loadProjectGLDebugOutputMessagesSeverityNodeASCII">0</" GD_STR_loadProjectGLDebugOutputMessagesSeverityNodeASCII">\n"
        "</" GD_STR_loadProjectGLDebugOutputNode">\n"
        "</" GD_STR_projectSettingsExtensionNameASCII">\n";

    xmlDoc.Parse(pDebuggerSettingsXMLString);

    if (xmlDoc.Error())
    {
        rc = false;

        //      gtString errorMsg = "Error in";
        //      errorMsg.appendFormattedString(L"%d", doc.Value());
        //      errorMsg.appendFormattedString(L"%d", doc.ErrorDesc());
        //      acMessageBox(errorMsg.asCharArray());
    }

    return rc;
}

// ---------------------------------------------------------------------------
// Name:        gdSaveProjectCommand::setDebuggerData
// Description: Set a debugger settings data
// Arguments:   TiXmlHandle& docHandle
//              const gtString& dataFieldName
//              const gtString& dataFieldValue
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        5/4/2012
// ---------------------------------------------------------------------------
bool gdSaveProjectCommand::setDebuggerData(TiXmlHandle& docHandle, const gtString& dataFieldName, const gtString& dataFieldValue)
{
    bool rc = false;

    // TinyXML does not support wide strings but it does supports UTF8 so we convert the strings to UTF8
    std::string utf8FieldValue;
    dataFieldValue.asUtf8(utf8FieldValue);

    // Get the data field
    TiXmlNode* pDataField = docHandle.FirstChild(GD_STR_projectSettingsExtensionNameASCII).FirstChild(dataFieldName.asASCIICharArray()).FirstChild().Node();

    if (pDataField)
    {
        // Get the field text
        TiXmlText* pDataFieldText = pDataField->ToText();

        if (pDataFieldText)
        {
            // Set the filed text
            if (dataFieldValue.isEmpty())
            {
                pDataFieldText->SetValue(OS_STR_EmptyXMLString);
            }
            else
            {
                pDataFieldText->SetValue(utf8FieldValue.c_str());
            }

            rc = true;
        }
    }

    GT_ASSERT(rc);
    return rc;
}

// ---------------------------------------------------------------------------
// Name:        gdSaveProjectCommand::setDebugOutputData
// Description: Set the debug output data into the XML file
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        13/6/2010
// ---------------------------------------------------------------------------
bool gdSaveProjectCommand::setDebugOutputData(TiXmlHandle& docHandle, const gtString& dataFieldName, const gtString& dataFieldValue)
{
    bool rc = false;

    // TinyXML does not support wide strings but it does supports UTF8 so we convert the strings to UTF8
    std::string utf8FieldValue;
    dataFieldValue.asUtf8(utf8FieldValue);

    // Get the data field
    TiXmlNode* pDataField = docHandle.FirstChild(GD_STR_projectSettingsExtensionNameASCII).FirstChild(GD_STR_loadProjectGLDebugOutputNode).FirstChild(dataFieldName.asASCIICharArray()).FirstChild().Node();

    if (pDataField)
    {
        // Get the field text
        TiXmlText* pDataFieldText = pDataField->ToText();

        if (pDataFieldText)
        {
            // Set the filed text
            pDataFieldText->SetValue(dataFieldValue.isEmpty() ? OS_STR_EmptyXMLString : utf8FieldValue.c_str());
            rc = true;
        }
    }

    GT_ASSERT(rc);
    return rc;
}

// ---------------------------------------------------------------------------
// Name:        gdSaveProjectCommand::writeStringToXml
// Description: Write a string into the XML file
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        23/7/2006
// ---------------------------------------------------------------------------
bool gdSaveProjectCommand::writeStringToXml(TiXmlNode* pXmlNode, const gtString& dataFieldValue)
{
    bool rc = false;

    // TinyXML does not support wide strings but it does supports UTF8 so we convert the strings to UTF8
    std::string utf8FieldValue;
    dataFieldValue.asUtf8(utf8FieldValue);

    if (pXmlNode)
    {
        // Get the field text
        TiXmlText* pXmlNodeText = pXmlNode->ToText();

        if (pXmlNodeText)
        {
            if (dataFieldValue.isEmpty())
            {
                pXmlNodeText->SetValue(OS_STR_EmptyXMLString);
            }
            else
            {
                pXmlNodeText->SetValue(utf8FieldValue.c_str());
            }

            rc = true;
        }
    }

    GT_ASSERT(rc);
    return rc;
}

// ---------------------------------------------------------------------------
// Name:        gdSaveProjectCommand::getXMLOutputString
// Description: Return the XML as string
// Arguments:   gtString& xmlOutputString
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        5/4/2012
// ---------------------------------------------------------------------------
bool gdSaveProjectCommand::getXMLOutputString(gtString& xmlOutputString)
{
    bool retVal = false;

    // Output string should only exist when we output as string:
    retVal = !m_xmlOutputString.isEmpty();
    xmlOutputString = m_xmlOutputString;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdSaveProjectCommand::saveDebugSettingsAsString
// Description: Save the debugger project settings as string
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        5/4/2012
// ---------------------------------------------------------------------------
bool gdSaveProjectCommand::saveDebugSettingsAsString()
{
    bool retVal = false;

    // Create the XML document:
    TiXmlDocument doc;

    // Initialize the file structure:
    retVal = initXMLDebuggerTextStructure(doc);

    if (retVal)
    {
        // Create XML document handle:
        TiXmlHandle docHandle(&doc);

        // Save breakpoints only in standalone:
        if (!afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
        {
            // Save the breakpoints:
            retVal = retVal && saveBreakpointsData(docHandle);

            // Save the selected state variables:
            retVal = retVal && saveStateVariablesData(docHandle);
        }

        // Set the Debugged Application DLL Interception Method file:
        gtString interceptionMethodString;
        interceptionMethodString.appendFormattedString(L"%d", m_projectSettings.interceptionMethod());
        retVal = retVal && setDebuggerData(docHandle, GD_STR_loadProjectInterceptionMethodNode, interceptionMethodString);

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
#ifdef GD_ALLOW_HSA_DEBUGGING
        gtString enableHSAString = m_projectSettings.shouldDebugHSAKernels() ? OS_STR_TrueXMLValueUnicode : OS_STR_FalseXMLValueUnicode;
        retVal = retVal && setDebuggerData(docHandle, GD_STR_loadProjectDebugHSAKernelsNode, enableHSAString);
#endif // GD_ALLOW_HSA_DEBUGGING
#endif // AMDT_BUILD_TARGET == AMDT_LINUX_OS

        gtString frameTerminatorsString;
        frameTerminatorsString.appendFormattedString(L"%d", m_projectSettings.frameTerminatorsMask());
        retVal = retVal && setDebuggerData(docHandle, GD_STR_loadProjectFrameTerminatorsNode, frameTerminatorsString);

        // Set the debug output GLDebugOutputLoggingEnabled:
        retVal = retVal && setDebugOutputData(docHandle, GD_STR_loadProjectGLDebugOutputLoggingEnabledNode, _glDebugOutputLoggingEnabledString.asCharArray());

        // Set the debug output GLDebugOutputBreakOnReports:
        retVal = retVal && setDebugOutputData(docHandle, GD_STR_loadProjectGLDebugOutputBreakOnReportsNode, _glDebugOutputBreakOnReportsString.asCharArray());

        // Set the debug output GLDebugOutputMessagesMask:
        retVal = retVal && setDebugOutputData(docHandle, GD_STR_loadProjectGLDebugOutputMessagesMaskNode, _debugOutputMessagesMaskString.asCharArray());

        // Set the debug output GLDebugOutputSeverity:
        retVal = retVal && setDebugOutputData(docHandle, GD_STR_loadProjectGLDebugOutputMessagesSeverityNode, _debugOutputSeverityString.asCharArray());

        TiXmlPrinter printer;
        printer.SetIndent("    ");

        doc.Accept(&printer);
        m_xmlOutputString = acQStringToGTString(printer.CStr());
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdSaveProjectCommand::setBreakpointData
// Description: Set the function breakpoint into the XML file
// Arguments:   TiXmlHandle& docHandle - the XML handle
//              breakpointTypeStr - the breakpoint type as string
//              breakpointStringValue - the breakpoint value as string
//              isBreakpointEnabled - is the breakpoint enabled
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        10/7/2011
// ---------------------------------------------------------------------------
bool gdSaveProjectCommand::setBreakpointData(TiXmlNode* pBreakpointsXMLNode, gtString breakpointTypeStr, gtString breakpointStringValue, bool isBreakpointEnabled, int lineNumber)
{
    bool rc = true;

    // Sanity check:
    GT_IF_WITH_ASSERT(pBreakpointsXMLNode != NULL)
    {
        // Fill empty strings with CodeXL empty string:
        if (breakpointTypeStr.isEmpty())
        {
            breakpointTypeStr = OS_STR_EmptyXMLStringUnicode;
        }

        if (breakpointStringValue.isEmpty())
        {
            breakpointStringValue = OS_STR_EmptyXMLStringUnicode;
        }

        // Add the node of a single function
        TiXmlElement breakpoint(GD_STR_loadProjectBreakpointNode);

        TiXmlNode* pBreakpointNode = NULL;
        // Insert a Breakpoint node
        pBreakpointNode = pBreakpointsXMLNode->InsertEndChild(breakpoint);

        if (pBreakpointNode)
        {
            // Insert the function's name element:
            rc = rc && addBreakpointProperty(pBreakpointNode, GD_STR_loadProjectFunctionBreakpointNameNode, breakpointStringValue);
            GT_ASSERT(rc);

            // Get the boolean status to gtString:
            gtString enabledStatusString = isBreakpointEnabled ? OS_STR_TrueXMLValueUnicode : OS_STR_FalseXMLValueUnicode;

            // Insert the function's enabled status element:
            rc = rc && addBreakpointProperty(pBreakpointNode, GD_STR_loadProjectFunctionBreakpointIsEnabledNode, enabledStatusString);
            GT_ASSERT(rc);

            // Insert the breakpoint's type element:
            rc = rc && addBreakpointProperty(pBreakpointNode, GD_STR_loadProjectBreakpointType, breakpointTypeStr);
            GT_ASSERT(rc);

            // Insert the breakpoint's line number element:
            gtString lineNumberStr;
            lineNumberStr.appendFormattedString(L"%d", lineNumber);
            rc = rc && addBreakpointProperty(pBreakpointNode, GD_STR_loadProjectSourceCodeLineNumberNode, lineNumberStr);
            GT_ASSERT(rc);
        }
    }

    return true;
}

// ---------------------------------------------------------------------------
// Name:        gdSaveProjectCommand::addBreakpointProperty
// Description: Add breakpoint function property to the XML string
// Arguments:   TiXmlNode* pBreakpointNode
//              const gtString& breakpointPropertyNode
//              const gtString& dataFieldValue
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        7/8/2012
// ---------------------------------------------------------------------------
bool gdSaveProjectCommand::addBreakpointProperty(TiXmlNode* pBreakpointNode, const gtString& breakpointPropertyNode, const gtString& dataFieldValue)
{
    // Get the Breakpoint <functionName> node
    TiXmlElement propertyName(breakpointPropertyNode.asASCIICharArray());

    // Create the function name as Text node
    std::string utf8FieldValue;
    dataFieldValue.asUtf8(utf8FieldValue);
    TiXmlText propertyNameValue(utf8FieldValue.c_str());

    // Add the Text node into the breakpoint node (as child)
    propertyName.InsertEndChild(propertyNameValue);

    // Add the breakpoint node into the XML file
    pBreakpointNode->InsertEndChild(propertyName);

    return true;
}

// ---------------------------------------------------------------------------
// Name:        gdSaveProjectCommand::setOpenGLStateVariableData
// Description: Set the OpenGL state variables into the XML string
// Arguments:   TiXmlNode* pStateVariablesNode
//              const gtString& functionName
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        7/8/2012
// ---------------------------------------------------------------------------
bool gdSaveProjectCommand::setOpenGLStateVariableData(TiXmlNode* pStateVariablesNode, const gtString& functionName)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(pStateVariablesNode != NULL)
    {

        // Get the StateVariable <VariableName> node
        TiXmlElement variableName(GD_STR_loadProjectStateVariableNameNode);

        // Create the variable name as Text node
        TiXmlText variableNameText(functionName.asASCIICharArray());

        // Add the Text node into the breakpoint node (as child)
        variableName.InsertEndChild(variableNameText);

        // Add the breakpoint node into the XML file
        pStateVariablesNode->InsertEndChild(variableName);
    }

    return retVal;
}
