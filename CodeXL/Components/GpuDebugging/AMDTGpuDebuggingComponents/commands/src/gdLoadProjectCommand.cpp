//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdLoadProjectCommand.cpp
///
//==================================================================================

//------------------------------ gdLoadProjectCommand.cpp ------------------------------
// TinyXml:
#include <tinyxml.h>

// Qt:
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osBundle.h>
#include <AMDTAPIClasses/Include/apAIDFunctions.h>
#include <AMDTAPIClasses/Include/apFrameTerminators.h>
#include <AMDTAPIClasses/Include/apKernelFunctionNameBreakpoint.h>
#include <AMDTAPIClasses/Include/apKernelSourceCodeBreakpoint.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionBreakPoint.h>
#include <AMDTAPIClasses/Include/apSourceCodeBreakpoint.h>
#include <AMDTAPIClasses/Include/apHostSourceBreakPoint.h>
#include <AMDTAPIClasses/Include/Events/apBreakpointsUpdatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTOSWrappers/Include/osProductVersion.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/commands/gdLoadProjectCommand.h>
#include <AMDTGpuDebuggingComponents/Include/commands/gdSaveProjectCommand.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdStateVariablesView.h>


//TinyXml
#include <tinyxml.h>

// ---------------------------------------------------------------------------
// Name:        gdLoadProjectCommand::gdLoadProjectCommand
// Description: Constructor.
// Arguments: projectFilePath - The project file path.
// ---------------------------------------------------------------------------
gdLoadProjectCommand::gdLoadProjectCommand(const gtString& xmlInput)
    : m_xmlStringInput(xmlInput)
{
    // Initialize the project name:
    m_debugProjectSettings.setProjectName(afProjectManager::instance().currentProjectSettings().projectName());

    unsigned int maxGLCalls, maxCLCalls, maxCommandQueues;
    gdGDebuggerGlobalVariablesManager::instance().getLoggingLimits(maxGLCalls, maxCLCalls, maxCommandQueues);
    m_debugProjectSettings.setMaxLoggedOpenGLCallsPerContext(maxGLCalls);
    m_debugProjectSettings.setMaxLoggedOpenCLCallsPerContext(maxCLCalls);
    m_debugProjectSettings.setMaxLoggedOpenCLCommandsPerQueue(maxCommandQueues);
}


// ---------------------------------------------------------------------------
// Name:        gdLoadProjectCommand::~gdLoadProjectCommand
// Description: Destructor.
// Author:      Avi Shapira
// Date:        10/1/2007
// ---------------------------------------------------------------------------
gdLoadProjectCommand::~gdLoadProjectCommand()
{
}

// ---------------------------------------------------------------------------
// Name:        gdLoadProjectCommand::canExecuteSpecificCommand
// Description: Answers the question - can we load the project of the G-Debugger application.
// Author:      Avi Shapira
// Date:        10/11/2003
// Implementation Notes:
// Currently - the answer is always yes.
// ---------------------------------------------------------------------------
bool gdLoadProjectCommand::canExecuteSpecificCommand()
{
    return true;
}


// ---------------------------------------------------------------------------
// Name:        gdLoadProjectCommand::executeSpecificCommand
// Description: Open the G-Debugger application project.
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        10/11/2003
// ---------------------------------------------------------------------------
bool gdLoadProjectCommand::executeSpecificCommand()
{
    bool retVal = false;

    // Save the current project into a file:
    bool rc3 = SaveCurrentProject();
    GT_ASSERT(rc3);

    retVal = ReadProjectSettingsFromString();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdLoadProjectCommand::ReadProjectSettingsFromString
// Description: Read the CodeXL project settings from the an XML string
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        8/4/2012
// ---------------------------------------------------------------------------
bool gdLoadProjectCommand::ReadProjectSettingsFromString()
{
    bool retVal = false;

    TiXmlDocument doc;
    retVal = doc.Parse(m_xmlStringInput.asASCIICharArray());

    if (retVal)
    {
        // Create an XML document:
        TiXmlHandle docHandle(&doc);

        // Read the settings from the XML:
        retVal = ReadProjectDebuggingSettings(docHandle);

        // Trigger a breakpoints updated event:
        apBreakpointsUpdatedEvent eve(-1);
        apEventsHandler::instance().registerPendingDebugEvent(eve);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdLoadProjectCommand::SaveCurrentProject
// Description: Saves the current project into a file.
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        7/10/2004
// ---------------------------------------------------------------------------
bool gdLoadProjectCommand::SaveCurrentProject()
{
    bool rc = true;

    // Get the application commands instance:
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(nullptr != pApplicationCommands)
    {
        pApplicationCommands->OnFileSaveProject();
        rc = true;
    }

    return rc;
}

// ---------------------------------------------------------------------------
// Name:        gdLoadProjectCommand::ReadProjectDebuggingSettings
// Description: Read the settings for the project debugging
// Arguments:   TiXmlHandle &docHandle
// Author:      Sigal Algranaty
// Date:        9/4/2012
// ---------------------------------------------------------------------------
bool gdLoadProjectCommand::ReadProjectDebuggingSettings(TiXmlHandle& docHandle)
{
    bool retVal = true;

    // Get the breakpoints node from the XML string:
    TiXmlNode* pBreakpointsNode = docHandle.FirstChild(GD_STR_projectSettingsExtensionNameASCII).FirstChild(GD_STR_loadProjectDebuggerBreakpoints).FirstChild(GD_STR_loadProjectBreakpoints).Node();

    if (nullptr != pBreakpointsNode)
    {
        retVal = readBreakpointsSettings(pBreakpointsNode) && retVal;
    }

    // Get the breakpoints node from the XML string:
    TiXmlNode* pStateVariablesNode = docHandle.FirstChild(GD_STR_projectSettingsExtensionNameASCII).FirstChild(GD_STR_loadProjectOpenGLStateVarNode).FirstChild(GD_STR_loadProjectStateVarNode).Node();

    if (nullptr != pBreakpointsNode)
    {
        retVal = readOpenGLStateVariables(pStateVariablesNode) && retVal;
    }

    // Get the Debugged Application DLL Interception Method file
    TiXmlNode* pInterceptionMethod = docHandle.FirstChild(GD_STR_projectSettingsExtensionNameASCII).FirstChild(GD_STR_loadProjectInterceptionMethodNodeASCII).FirstChild().Node();

    if (nullptr != pInterceptionMethod)
    {
        TiXmlText* pInterceptionMethodText = pInterceptionMethod->ToText();

        if (nullptr != pInterceptionMethodText)
        {
            // Set the value of the text field into gtString to enable comparison
            gtString interceptionMethodTextValue;
            interceptionMethodTextValue.fromASCIIString(pInterceptionMethodText->Value());

            if (interceptionMethodTextValue.isIntegerNumber())
            {
                int pInterceptionMethodAsInt = 0;
                interceptionMethodTextValue.toIntNumber(pInterceptionMethodAsInt);

                m_debugProjectSettings.setInterceptionMethod((apInterceptionMethod)pInterceptionMethodAsInt);
            }
        }
    }

#if GD_ALLOW_HSA_DEBUGGING
    // Get the HSA kernel debugging flag:
    bool enableHSA = false;
    TiXmlNode* pEnableHSA = docHandle.FirstChild(GD_STR_projectSettingsExtensionNameASCII).FirstChild(GD_STR_loadProjectDebugHSAKernelsNodeASCII).FirstChild().Node();

    if (nullptr != pEnableHSA)
    {
        TiXmlText* pEnableHSAText = pEnableHSA->ToText();

        if (nullptr != pEnableHSAText)
        {
            // Set the value of the text field into gtString to enable comparison
            gtString enableHSATextValue;
            enableHSATextValue.fromASCIIString(pEnableHSAText->Value());

            if (OS_STR_TrueXMLValueUnicode == enableHSATextValue)
            {
                enableHSA = true;
            }
            else
            {
                enableHSA = false;
                GT_ASSERT((OS_STR_EmptyXMLStringUnicode == enableHSATextValue) || (OS_STR_FalseXMLValueUnicode == enableHSATextValue));
            }
        }
    }

    m_debugProjectSettings.setShouldDebugHSAKernels(enableHSA);
#endif // GD_ALLOW_HSA_DEBUGGING

    // Set the frame terminators data:
    unsigned int frameTerminators = 0;
    TiXmlNode* pFrameTeraminatorsNode = docHandle.FirstChild(GD_STR_projectSettingsExtensionNameASCII).FirstChild(GD_STR_loadProjectFrameTerminatorsNodeASCII).FirstChild().Node();

    if (nullptr != pFrameTeraminatorsNode)
    {
        TiXmlText* pFrameTeraminatorsNodeText = pFrameTeraminatorsNode->ToText();

        if (nullptr != pFrameTeraminatorsNodeText)
        {
            // Set the value of the text field into gtString to enable comparison
            gtString frameTerminatorsTextValue;
            frameTerminatorsTextValue.fromASCIIString(pFrameTeraminatorsNodeText->Value());

            if (frameTerminatorsTextValue.isIntegerNumber())
            {
                frameTerminatorsTextValue.toUnsignedIntNumber(frameTerminators);
            }
        }
    }

    bool isValidFrameTerminators = (0 != (frameTerminators & AP_ALL_GL_FRAME_TERMINATORS));

    if (!isValidFrameTerminators)
    {
        // Add default frame terminator for GL:
        frameTerminators |= AP_DEFAULT_FRAME_TERMINATORS;
    }

    m_debugProjectSettings.setFrameTerminators(frameTerminators);

    // OpenGL Debug output:

    // Get the OpenGL Debug Output:
    bool glDebugOutputLoggingEnabled = false;
    TiXmlNode* pGLDebugOutputLoggingEnabled = docHandle.FirstChild(GD_STR_projectSettingsExtensionNameASCII).FirstChild(GD_STR_loadProjectGLDebugOutputNode).FirstChild(GD_STR_loadProjectGLDebugOutputLoggingEnabledNodeASCII).FirstChild().Node();

    if (nullptr != pGLDebugOutputLoggingEnabled)
    {
        TiXmlText* pGLDebugOutputLoggingEnabledText = pGLDebugOutputLoggingEnabled->ToText();

        if (nullptr != pGLDebugOutputLoggingEnabledText)
        {
            // Set the value of the text field into gtString to enable comparison:
            gtString glDebugOutputLoggingEnabledValue;
            glDebugOutputLoggingEnabledValue.fromASCIIString(pGLDebugOutputLoggingEnabledText->Value());

            if (OS_STR_TrueXMLValueUnicode == glDebugOutputLoggingEnabledValue)
            {
                glDebugOutputLoggingEnabled = true;
            }
            else
            {
                glDebugOutputLoggingEnabled = false;
                GT_ASSERT((OS_STR_EmptyXMLStringUnicode == glDebugOutputLoggingEnabledValue) || (OS_STR_FalseXMLValueUnicode == glDebugOutputLoggingEnabledValue));
            }
        }
    }

    // Get the OpenGL Debug Output - break on debug output reports
    TiXmlNode* pGLDebugOutputBreakOnReports = docHandle.FirstChild(GD_STR_projectSettingsExtensionNameASCII).FirstChild(GD_STR_loadProjectGLDebugOutputNode).FirstChild(GD_STR_loadProjectGLDebugOutputBreakOnReportsNodeASCII).FirstChild().Node();

    if (nullptr != pGLDebugOutputBreakOnReports)
    {
        TiXmlText* pGLDebugOutputBreakOnReportsText = pGLDebugOutputBreakOnReports->ToText();

        if (nullptr != pGLDebugOutputBreakOnReportsText)
        {
            // Set the value of the text field into gtString to enable comparison:
            gtString glDebugOutputBreakOnReportsValue;
            glDebugOutputBreakOnReportsValue.fromASCIIString(pGLDebugOutputBreakOnReportsText->Value());

            if (OS_STR_TrueXMLValueUnicode != glDebugOutputBreakOnReportsValue)
            {
                GT_ASSERT((OS_STR_EmptyXMLStringUnicode == glDebugOutputBreakOnReportsValue) || (OS_STR_FalseXMLValueUnicode == glDebugOutputBreakOnReportsValue));
            }
        }
    }

    // Get the debug output message mask:
    gtUInt64 debugOutputMessagesMask = 0;
    TiXmlNode* pDebugOutputMessagesMask = docHandle.FirstChild(GD_STR_projectSettingsExtensionNameASCII).FirstChild(GD_STR_loadProjectGLDebugOutputNode).FirstChild(GD_STR_loadProjectGLDebugOutputMessagesMaskNodeASCII).FirstChild().Node();

    if (nullptr != pDebugOutputMessagesMask)
    {
        TiXmlText* pDebugOutputMessagesMaskText = pDebugOutputMessagesMask->ToText();

        if (nullptr != pDebugOutputMessagesMaskText)
        {
            gtString debugOutputMessagesMaskString;
            debugOutputMessagesMaskString.fromASCIIString(pDebugOutputMessagesMaskText->Value());
            bool rc = debugOutputMessagesMaskString.toUnsignedInt64Number(debugOutputMessagesMask);

            if (!rc)
            {
                debugOutputMessagesMask = 0;
                GT_ASSERT(OS_STR_EmptyXMLStringUnicode == debugOutputMessagesMaskString);
            }
        }
    }

    // Get the debug output messages severity:
    bool debugOutputSeverities[AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES] = {0};
    TiXmlNode* pDebugOutputMessagesSeverity = docHandle.FirstChild(GD_STR_projectSettingsExtensionNameASCII).FirstChild(GD_STR_loadProjectGLDebugOutputNode).FirstChild(GD_STR_loadProjectGLDebugOutputMessagesSeverityNodeASCII).FirstChild().Node();

    if (nullptr != pDebugOutputMessagesSeverity)
    {
        TiXmlText* pDebugOutputMessagesSeverityText = pDebugOutputMessagesSeverity->ToText();

        if (nullptr != pDebugOutputMessagesSeverityText)
        {
            gtString debugOutputMessagesSeverityString;
            debugOutputMessagesSeverityString.fromASCIIString(pDebugOutputMessagesSeverityText->Value());
            bool rcSev = apGLDebugOutputSeveritiesFromString(debugOutputMessagesSeverityString, debugOutputSeverities);

            if (!rcSev)
            {
                for (int i = 0; i < AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES; i++)
                {
                    debugOutputSeverities[i] = false;
                }
            }
        }
    }

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    {
        // OpenGL Debug output:
        bool doesExist = false, isEnabled = false;
        bool rcBreakOnDebugOutput = gaGetGenericBreakpointStatus(AP_BREAK_ON_DEBUG_OUTPUT, doesExist, isEnabled);
        bool rcEnableDebugOutput = gaEnableGLDebugOutputLogging(glDebugOutputLoggingEnabled);
        bool rcSetDebugOutputMask = gaSetGLDebugOutputKindMask(debugOutputMessagesMask);
        bool rcSetDebugOutputSeverity = true;

        for (int i = 0; i < AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES; i++)
        {
            rcSetDebugOutputSeverity = gaSetGLDebugOutputSeverityEnabled((apGLDebugOutputSeverity)i, debugOutputSeverities[i]) && rcSetDebugOutputSeverity;
        }

        bool rcDebugOutput = rcEnableDebugOutput && rcBreakOnDebugOutput && rcSetDebugOutputMask && rcSetDebugOutputSeverity;
        GT_ASSERT(rcDebugOutput);
    }
#endif

    retVal = true || glDebugOutputLoggingEnabled; // dummy use for glDebugOutputLoggingEnabled for gcc compiler on linux

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdLoadProjectCommand::handleFileVersionCompatibility
// Description: Reads file old versions
// Arguments:   TiXmlHandle &docHandle
// Author:      Sigal Algranaty
// Date:        9/4/2012
// ---------------------------------------------------------------------------
void gdLoadProjectCommand::handleFileVersionCompatibility(TiXmlHandle& docHandle)
{
    // Get the Debugged Application environment variables
    TiXmlNode* pFileVersion = docHandle.FirstChild(GD_STR_loadProjectFileVersionNode).Node();
    gtString fileVer;

    if (nullptr != pFileVersion)
    {
        if (nullptr != pFileVersion->FirstChild())
        {
            fileVer.fromASCIIString(pFileVersion->FirstChild()->ToText()->Value());
        }
    }

    if (L"1" == fileVer)
    {
        // If this is a new file, treat the environment variables as nodes
        TiXmlNode* pNode = docHandle.FirstChild(GD_STR_projectSettingsExtensionNameASCII).FirstChild(AF_STR_loadProjectAppEnvVarsNode).FirstChild(AF_STR_loadProjectAppEnvVarsVarNode).Node();
        TiXmlNode* pDummyNode = nullptr;
        TiXmlText* currName;
        TiXmlText* currValue;
        osEnvironmentVariable currEnvVar;
        gtString nodeValueStr;

        for (; pNode != nullptr; pNode = pNode->NextSibling())
        {
            pDummyNode = pNode->FirstChild(AF_STR_loadProjectAppEnvVarsNameNode)->FirstChild();
            currName = pDummyNode->ToText();

            if (nullptr != currName)
            {
                nodeValueStr.fromASCIIString(currName->Value());

                if (OS_STR_EmptyXMLStringUnicode == nodeValueStr)
                {
                    nodeValueStr.makeEmpty();
                }

                currEnvVar._name = nodeValueStr;
            }
            else
            {
                currEnvVar._name.makeEmpty();
            }

            pDummyNode = pNode->FirstChild(AF_STR_loadProjectAppEnvVarsValueNode)->FirstChild();
            currValue = pDummyNode->ToText();

            if (nullptr != currValue)
            {
                nodeValueStr.fromASCIIString(currValue->Value());

                if (OS_STR_EmptyXMLStringUnicode == nodeValueStr)
                {
                    nodeValueStr.makeEmpty();
                }

                currEnvVar._value = nodeValueStr;
            }
            else
            {
                currEnvVar._value.makeEmpty();
            }


            apHandleXMLEscaping(currEnvVar._name, false);
            apHandleXMLEscaping(currEnvVar._value, false);

            m_debugProjectSettings.addEnvironmentVariable(currEnvVar);

        }
    }
    else // (fileVer != "1")
    {
        // We are using the old file version, so handle the environment variables as a string
        TiXmlNode* pApplicationEnvVars = docHandle.FirstChild(GD_STR_projectSettingsExtensionNameASCII).FirstChild(AF_STR_loadProjectAppEnvVarsNode).FirstChild().Node();

        if (nullptr != pApplicationEnvVars)
        {
            TiXmlText* pApplicationEnvVarsText = pApplicationEnvVars->ToText();

            if (nullptr != pApplicationEnvVarsText)
            {
                gtString applicationEnvVarsAsString;
                applicationEnvVarsAsString.fromASCIIString(pApplicationEnvVarsText->Value());

                // We perform a fake XML escaping so as to avoid problems when reading this into the program
                apHandleXMLEscaping(applicationEnvVarsAsString, true);

                if (OS_STR_EmptyXMLStringUnicode == applicationEnvVarsAsString)
                {
                    m_debugProjectSettings.clearEnvironmentVariables();
                }
                else
                {
                    // Check if the string is not empty:
                    if (!applicationEnvVarsAsString.isEmpty())
                    {
                        gtStringTokenizer tokenizer(applicationEnvVarsAsString, L";");

                        gtString currentToken;

                        // Get the next token (variable name and value):
                        while (tokenizer.getNextToken(currentToken))
                        {
                            // find the separator:
                            int equalLocation = currentToken.find(L"=");

                            // If we found the separator:
                            if (-1 != equalLocation)
                            {
                                // Get the var name and value:
                                gtString curVarName;
                                gtString curVarValue;
                                currentToken.getSubString(0, equalLocation - 1, curVarName);
                                currentToken.getSubString(equalLocation + 1, 0, curVarValue);

                                osEnvironmentVariable currentVariable;
                                currentVariable._name = curVarName;
                                currentVariable._value = curVarValue;

                                apHandleXMLEscaping(currentVariable._name, false);
                                apHandleXMLEscaping(currentVariable._value, false);

                                m_debugProjectSettings.addEnvironmentVariable(currentVariable);
                            }
                        }
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdLoadProjectCommand::readBreakpointsSettings
// Description: Read the breakpoint settings from the breakpoints XML node
// Arguments:   TiXmlNode* pBreakpointsNode
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        7/8/2012
// ---------------------------------------------------------------------------
bool gdLoadProjectCommand::readBreakpointsSettings(TiXmlNode* pBreakpointsNode)
{
    bool retVal = false;

    // Sanity check
    GT_IF_WITH_ASSERT(nullptr != pBreakpointsNode)
    {
        TiXmlNode* pNode = nullptr;

        // Removing all the breakpoints from the debugger
        retVal = gaRemoveAllBreakpoints();

        GT_IF_WITH_ASSERT(retVal)
        {
            // Define a breakpoint pointer, and allocate it later according to the type:
            apBreakPoint* pBreakpoint = nullptr;

            // Adding the breakpoints from the bpt file to the global variable manager
            if (nullptr != pBreakpointsNode)
            {
                // Walk on all the function breakpoints nodes
                for (pNode = pBreakpointsNode->FirstChild(); pNode != nullptr; pNode = pNode->NextSibling())
                {
                    if (nullptr != pNode)
                    {
                        gtString breakpointStringProperty = OS_STR_EmptyXMLStringUnicode, isEnabledString = OS_STR_TrueXMLValueUnicode, breakpointTypeString = GD_STR_loadProjectBreakpointTypeFunction, lineNumberString;
                        gtString currentNodeValue;
                        currentNodeValue.fromASCIIString(pNode->Value());
                        gtString propertyName;

                        // Check what format is this node - 3.2 or older:
                        if (currentNodeValue.isEqual(GD_STR_loadProjectBreakpointNode))
                        {
                            // New format:
                            TiXmlNode* pSingleBreakpointNode = pNode->FirstChild();

                            if (nullptr != pSingleBreakpointNode)
                            {
                                // Read the breakpoint string parameter:
                                propertyName = GD_STR_loadProjectFunctionBreakpointNameNode;
                                readBreakpointProperty(*pSingleBreakpointNode, propertyName, breakpointStringProperty);
                                pSingleBreakpointNode = pSingleBreakpointNode->NextSibling();

                                if (nullptr != pSingleBreakpointNode)
                                {
                                    // Read the "Is Enabled" parameter:
                                    propertyName = GD_STR_loadProjectFunctionBreakpointIsEnabledNode;
                                    readBreakpointProperty(*pSingleBreakpointNode, propertyName, isEnabledString);

                                    pSingleBreakpointNode = pSingleBreakpointNode->NextSibling();

                                    if (nullptr != pSingleBreakpointNode)
                                    {
                                        // Read the "Breakpoint Type" parameter:
                                        propertyName = GD_STR_loadProjectBreakpointType;
                                        readBreakpointProperty(*pSingleBreakpointNode, propertyName, breakpointTypeString);
                                        pSingleBreakpointNode = pSingleBreakpointNode->NextSibling();

                                        if (nullptr != pSingleBreakpointNode)
                                        {
                                            // Read the "Line Number" parameter:
                                            propertyName = GD_STR_loadProjectSourceCodeLineNumberNode;
                                            readBreakpointProperty(*pSingleBreakpointNode, propertyName, lineNumberString);
                                        }
                                    }
                                }
                            }
                        }
                        else if (GD_STR_loadProjectFunctionBreakpointNameNode == currentNodeValue)
                        {
                            // Old format
                            propertyName = GD_STR_loadProjectFunctionBreakpointNameNode;

                            if (nullptr != pNode)
                            {
                                readBreakpointProperty(*pNode, propertyName, breakpointStringProperty);
                            }
                        }

                        // Allocate the breakpoint and fill its details:
                        if (GD_STR_loadProjectBreakpointTypeFunction == breakpointTypeString)
                        {
                            // Allocate monitored function breakpoint:
                            pBreakpoint = new apMonitoredFunctionBreakPoint;

                            // Fill the breakpoint monitored function id:
                            apMonitoredFunctionId functionId = apMonitoredFunctionsAmount;
                            bool rc = gaGetMonitoredFunctionId(breakpointStringProperty, functionId);
                            GT_IF_WITH_ASSERT(rc)
                            {
                                // Down cast the breakpoint to a monitored function breakpoint:
                                apMonitoredFunctionBreakPoint* pMonitoredFunctionBreakpoint = (apMonitoredFunctionBreakPoint*)pBreakpoint;
                                GT_IF_WITH_ASSERT(nullptr != pMonitoredFunctionBreakpoint)
                                {
                                    // Set the function id:
                                    pMonitoredFunctionBreakpoint->setMonitoredFunctionId(functionId);
                                }
                            }
                        }
                        else if (GD_STR_loadProjectBreakpointTypeFunction == breakpointTypeString)
                        {
                            // Allocate kernel function breakpoint:
                            pBreakpoint = new apKernelFunctionNameBreakpoint;

                            // Down cast the breakpoint to a monitored function breakpoint:
                            apKernelFunctionNameBreakpoint* pKernelFunctionBreakpoint = (apKernelFunctionNameBreakpoint*)pBreakpoint;
                            GT_IF_WITH_ASSERT(nullptr != pKernelFunctionBreakpoint)
                            {
                                // Set the kernel function name:
                                pKernelFunctionBreakpoint->setKernelFunctionName(breakpointStringProperty);
                            }
                        }
                        else if (GD_STR_loadProjectBreakpointTypeGeneric == breakpointTypeString)
                        {
                            // Allocate generic breakpoint:
                            pBreakpoint = new apGenericBreakpoint;

                            // Down cast the breakpoint to a generic breakpoint:
                            apGenericBreakpoint* pGenericBreakpoint = (apGenericBreakpoint*)pBreakpoint;
                            GT_IF_WITH_ASSERT(nullptr != pGenericBreakpoint)
                            {
                                // Get the breakpoint type from the XML string:
                                apGenericBreakpointType breakpointType = AP_BREAK_TYPE_UNKNOWN;
                                bool rc = apGenericBreakpoint::breakpointTypeFromString(breakpointStringProperty, breakpointType);
                                GT_IF_WITH_ASSERT(rc)
                                {
                                    // Set the generic breakpoint type:
                                    pGenericBreakpoint->setBreakpointType(breakpointType);
                                }
                            }
                        }

                        else if (GD_STR_loadProjectBreakpointTypeKernel == breakpointTypeString)
                        {
                            // Allocate kernel function name breakpoint:
                            pBreakpoint = new apKernelFunctionNameBreakpoint;

                            // Down cast the breakpoint to a kernel function breakpoint:
                            apKernelFunctionNameBreakpoint* pKernelFunctionBreakpoint = (apKernelFunctionNameBreakpoint*)pBreakpoint;
                            GT_IF_WITH_ASSERT(nullptr != pKernelFunctionBreakpoint)
                            {
                                // Get the breakpoint type from the XML string:
                                pKernelFunctionBreakpoint->setKernelFunctionName(breakpointStringProperty);
                            }
                        }

                        else if (GD_STR_loadProjectBreakpointTypeSourceCode == breakpointTypeString)
                        {
                            // Allocate source code breakpoint:
                            pBreakpoint = new apSourceCodeBreakpoint;

                            // Down cast the breakpoint to a source code breakpoint:
                            apSourceCodeBreakpoint* pSourceCodeBreakpoint = (apSourceCodeBreakpoint*)pBreakpoint;
                            GT_IF_WITH_ASSERT(nullptr != pSourceCodeBreakpoint)
                            {
                                // Get the breakpoint type from the XML string:
                                osFilePath breakpointFilePath(breakpointStringProperty);
                                pSourceCodeBreakpoint->setFilePath(breakpointFilePath);
                                int lineNumber = -1;
                                bool rc = lineNumberString.toIntNumber(lineNumber);
                                GT_IF_WITH_ASSERT(rc)
                                {
                                    pSourceCodeBreakpoint->setLineNumber(lineNumber);
                                }
                            }
                        }

                        else if (GD_STR_loadProjectBreakpointTypeHostSourceCode == breakpointTypeString)
                        {
                            // Allocate source code breakpoint:
                            pBreakpoint = new apHostSourceCodeBreakpoint;

                            // Down cast the breakpoint to a source code breakpoint:
                            apSourceCodeBreakpoint* pSourceCodeBreakpoint = (apSourceCodeBreakpoint*)pBreakpoint;
                            GT_IF_WITH_ASSERT(nullptr != pSourceCodeBreakpoint)
                            {
                                // Get the breakpoint type from the XML string:
                                osFilePath breakpointFilePath(breakpointStringProperty);
                                pSourceCodeBreakpoint->setFilePath(breakpointFilePath);
                                int lineNumber = -1;
                                bool rc = lineNumberString.toIntNumber(lineNumber);
                                GT_IF_WITH_ASSERT(rc)
                                {
                                    pSourceCodeBreakpoint->setLineNumber(lineNumber);
                                }
                            }
                        }

                        else if (GD_STR_loadProjectBreakpointTypeKernelSourceCode == breakpointTypeString)
                        {
                            // Allocate source code breakpoint:
                            pBreakpoint = new apKernelSourceCodeBreakpoint;

                            // Down cast the breakpoint to a source code breakpoint:
                            apKernelSourceCodeBreakpoint* pSourceCodeBreakpoint = (apKernelSourceCodeBreakpoint*)pBreakpoint;
                            GT_IF_WITH_ASSERT(nullptr != pSourceCodeBreakpoint)
                            {
                                // Get the breakpoint type from the XML string:
                                osFilePath breakpointFilePath(breakpointStringProperty);
                                pSourceCodeBreakpoint->setUnresolvedPath(breakpointFilePath);
                                int lineNumber = -1;
                                bool rc = lineNumberString.toIntNumber(lineNumber);
                                GT_IF_WITH_ASSERT(rc)
                                {
                                    pSourceCodeBreakpoint->setLineNumber(lineNumber);
                                }
                            }
                        }


                        // Set the is enabled flag:
                        bool isEnabled = true;

                        if (OS_STR_TrueXMLValueUnicode != isEnabledString)
                        {
                            isEnabled = false;
                        }

                        bool rcSetBreakpoint  = false;
                        GT_IF_WITH_ASSERT(nullptr != pBreakpoint)
                        {
                            // Set the breakpoint enable status:
                            pBreakpoint->setEnableStatus(isEnabled);

                            // Set the breakpoint:
                            rcSetBreakpoint = gaSetBreakpoint(*pBreakpoint);
                        }

                        retVal = retVal && rcSetBreakpoint;
                    }
                }
            }
        }

    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdLoadProjectCommand::readOpenGLStateVariables
// Description: Read the OpenGL state variable from the file and add it into the app
// Arguments:   TiXmlHandle& docHandle
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        7/8/2012
// ---------------------------------------------------------------------------
bool gdLoadProjectCommand::readOpenGLStateVariables(TiXmlNode* pStateVariableNode)
{
    bool retVal = false;

    // Get the application command instance:
    gdApplicationCommands* pApplicationCommands = gdApplicationCommands::gdInstance();
    GT_IF_WITH_ASSERT((nullptr != pApplicationCommands) && (nullptr != pStateVariableNode))
    {
        // Get the state variables view:
        gdStateVariablesView* pStateVariableView = pApplicationCommands->stateVariablesView();

        if (nullptr != pStateVariableView)
        {
            // Remove all the state variables from the list (even if we don't find the node)
            pStateVariableView->deleteAllStateVariables();

            retVal = true;

            // Walk on all the function breakpoints nodes
            TiXmlNode* pNode = nullptr;

            for (pNode = pStateVariableNode->FirstChild(); pNode != nullptr; pNode = pNode->NextSibling())
            {
                if (nullptr != pNode)
                {
                    if (nullptr != pNode->FirstChild())
                    {
                        // Create the function name as Text node
                        TiXmlText* pStateVariableNameText = pNode->FirstChild()->ToText();

                        if (nullptr != pStateVariableNameText)
                        {
                            // get the breakpoint function name
                            gtString variableName;
                            variableName.fromASCIIString(pStateVariableNameText->Value());

                            // Adding the OpenGL State Variables from the bpt file to the list
                            pStateVariableView->AddStateVariable(variableName);
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdLoadProjectCommand::readBreakpointProperty
// Description: Reads a text property from an xml element and store the text in dataFieldValue.
//              In case the property does not match propertyToReadNodeName return false and do nothing.
// Arguments:   TiXmlNode& breakpointPropertyNode
//              gtString& propertyToReadNodeName
//              gtString& dataFieldValue
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        7/8/2012
// ---------------------------------------------------------------------------
bool gdLoadProjectCommand::readBreakpointProperty(TiXmlNode& breakpointPropertyNode, gtString& propertyToReadNodeName, gtString& dataFieldValue)
{
    bool retVal = false;

    // Check if the value is the requested property
    if (propertyToReadNodeName.isEqual(breakpointPropertyNode.Value()))
    {
        // Get the text value
        dataFieldValue.fromASCIIString(breakpointPropertyNode.FirstChild()->Value());
        retVal = true;
    }
    else
    {
        retVal = true;
    }

    return retVal;
}