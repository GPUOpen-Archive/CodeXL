//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdOptionsFileHandler.cpp
///
//==================================================================================

//------------------------------ gdOptionsFileHandler.cpp ------------------------------

// Qt
#include <QtGui>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTAPIClasses/Include/apParameters.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>

// Local:
#include <src/gdOptionsFileHandler.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
// #include <AMDTGpuDebuggingComponents/Include/gdPerformanceCountersTimer.h>
#include <AMDTGpuDebuggingComponents/Include/gdCodeXLGlobalVariablesManager.h>


// ---------------------------------------------------------------------------
// Name:        gdOptionsFileHandler::gdOptionsFileHandler
// Description: Constructor.
// Arguments: optionsFilePath - The options file path.
// Author:      Yaki Tebeka
// Date:        12/11/2007
// ---------------------------------------------------------------------------
gdOptionsFileHandler::gdOptionsFileHandler(const osFilePath& optionsFilePath)
    : osSettingsFileHandler(optionsFilePath)
{
}


// ---------------------------------------------------------------------------
// Name:        gdOptionsFileHandler::~gdOptionsFileHandler
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        11/11/2007
// ---------------------------------------------------------------------------
gdOptionsFileHandler::~gdOptionsFileHandler()
{
}


// ---------------------------------------------------------------------------
// Name:        gdOptionsFileHandler::updateXMLDocumentFromApplicationValues
// Description:
//  Updated this class XML document from the application's current values.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        12/11/2007
// ---------------------------------------------------------------------------
bool gdOptionsFileHandler::updateXMLDocumentFromApplicationValues()
{
    bool retVal = false;

    bool field1ReadOk = false;
    bool field2ReadOk = false;
    bool field3ReadOk = false;
    bool field4ReadOk = false;
    bool field5ReadOk = false;
    bool field6ReadOk = false;
    bool field7ReadOk = false;
    bool field8ReadOk = false;
    bool field9ReadOk = false;
    bool field10ReadOk = false;
    bool field13ReadOk = false;
    bool field14ReadOk = false;
    bool field15ReadOk = false;
    bool field16ReadOk = false;
    bool field17ReadOk = false;
    bool field18ReadOk = false;
    bool field19ReadOk = false;

    // Aid variables:
    bool xmlValueBool = false;
    gtString xmlValueString;

    // Get the global variables manager:
    gdCodeXLGlobalVariablesManager& theGlobalVariablesMgr = gdCodeXLGlobalVariablesManager::instance();

    // A document handle that will wrap our XML document:
    TiXmlHandle settingsXMLDocHandle(&_settingsFileAsXMLDocument);

    // Set the SearchInAdditionalDirectories
    TiXmlNode* pXmlNode = settingsXMLDocHandle.FirstChild(GD_STR_ToolsOptionsCodeXLNode).FirstChild(GD_STR_ToolsOptionsOptionsNode).FirstChild(GD_STR_ToolsOptionsCallStackNode).FirstChild(GD_STR_ToolsOptionsSearchInAdditionalDirectoriesNode).FirstChild().Node();
    GT_IF_WITH_ASSERT(pXmlNode != NULL)
    {
        xmlValueBool = theGlobalVariablesMgr.searchInAdditionalDirectoriesBool();
        bool rc1 = writeBoolToXmlNode(pXmlNode, xmlValueBool);
        GT_IF_WITH_ASSERT(rc1)
        {
            field1ReadOk = true;
        }
    }

    // Set the setSourceCodeRootLocation
    pXmlNode = settingsXMLDocHandle.FirstChild(GD_STR_ToolsOptionsCodeXLNode).FirstChild(GD_STR_ToolsOptionsOptionsNode).FirstChild(GD_STR_ToolsOptionsCallStackNode).FirstChild(GD_STR_ToolsOptionsSetSourceCodeRootLocationNode).FirstChild().Node();
    GT_IF_WITH_ASSERT(pXmlNode != NULL)
    {
        xmlValueBool = theGlobalVariablesMgr.setSourceCodeRootLocationBool();
        bool rc2 = writeBoolToXmlNode(pXmlNode, xmlValueBool);
        GT_IF_WITH_ASSERT(rc2)
        {
            field2ReadOk = true;
        }
    }

    // Set the sourceCodeRootLocation
    pXmlNode = settingsXMLDocHandle.FirstChild(GD_STR_ToolsOptionsCodeXLNode).FirstChild(GD_STR_ToolsOptionsOptionsNode).FirstChild(GD_STR_ToolsOptionsCallStackNode).FirstChild(GD_STR_ToolsOptionsSourceCodeRootLocationNode).FirstChild().Node();
    GT_IF_WITH_ASSERT(pXmlNode != NULL)
    {
        xmlValueString = theGlobalVariablesMgr.sourceCodeRootLocation();
        bool rc3 = writeStringToXmlNode(pXmlNode, xmlValueString);
        GT_IF_WITH_ASSERT(rc3)
        {
            field3ReadOk = true;
        }
    }

    // Set the additionalSourceCodeDirectories
    pXmlNode = settingsXMLDocHandle.FirstChild(GD_STR_ToolsOptionsCodeXLNode).FirstChild(GD_STR_ToolsOptionsOptionsNode).FirstChild(GD_STR_ToolsOptionsCallStackNode).FirstChild(GD_STR_ToolsOptionsAdditionalSourceCodeDirectoriesNode).Node();
    GT_IF_WITH_ASSERT(pXmlNode != NULL)
    {
        xmlValueString = theGlobalVariablesMgr.additionalSourceCodeDirectories();
        bool rc4 = writeAdditionalSourceCodeDirectoriesStringToXml(pXmlNode, xmlValueString);
        GT_IF_WITH_ASSERT(rc4)
        {
            field4ReadOk = true;
        }
    }

    // Set the collectAllocatedObjectsCreationCallsStacks
    pXmlNode = settingsXMLDocHandle.FirstChild(GD_STR_ToolsOptionsCodeXLNode).FirstChild(GD_STR_ToolsOptionsOptionsNode).FirstChild(GD_STR_ToolsOptionsCallStackNode).FirstChild(GD_STR_ToolsOptionsCollectAllocatedObjectsCreationCallsStacksNode).FirstChild().Node();
    GT_IF_WITH_ASSERT(pXmlNode != NULL)
    {
        gaGetCollectAllocatedObjectCreationCallsStacks(xmlValueBool);
        bool rc5 = writeBoolToXmlNode(pXmlNode, xmlValueBool);
        GT_IF_WITH_ASSERT(rc5)
        {
            field5ReadOk = true;
        }
    }

    // Set the enableTexturesLogging
    pXmlNode = settingsXMLDocHandle.FirstChild(GD_STR_ToolsOptionsCodeXLNode).FirstChild(GD_STR_ToolsOptionsOptionsNode).FirstChild(GD_STR_ToolsOptionsTexturesNode).FirstChild(GD_STR_ToolsOptionsEnableTexturesLoggingNode).FirstChild().Node();
    GT_IF_WITH_ASSERT(pXmlNode != NULL)
    {
        gaIsImagesDataLogged(xmlValueBool);
        bool rc6 = writeBoolToXmlNode(pXmlNode, xmlValueBool);
        GT_IF_WITH_ASSERT(rc6)
        {
            field6ReadOk = true;
        }
    }

    // Set the texturesFileFormat
    pXmlNode = settingsXMLDocHandle.FirstChild(GD_STR_ToolsOptionsCodeXLNode).FirstChild(GD_STR_ToolsOptionsOptionsNode).FirstChild(GD_STR_ToolsOptionsTexturesNode).FirstChild(GD_STR_ToolsOptionsTexturesFileFormatNode).FirstChild().Node();
    GT_IF_WITH_ASSERT(pXmlNode != NULL)
    {
        apFileTypeToFileExtensionString(theGlobalVariablesMgr.imagesFileFormat(), xmlValueString);
        bool rc7 = writeStringToXmlNode(pXmlNode, xmlValueString);
        GT_IF_WITH_ASSERT(rc7)
        {
            field7ReadOk = true;
        }
    }

    // Get the log file severity from the infra:
    osDebugLog& theDebugLog = osDebugLog::instance();
    osDebugLogSeverity loggedSeverity = theDebugLog.loggedSeverity();

    // Set the debug log level
    pXmlNode = settingsXMLDocHandle.FirstChild(GD_STR_ToolsOptionsCodeXLNode).FirstChild(GD_STR_ToolsOptionsOptionsNode).FirstChild(GD_STR_ToolsOptionsAdvancedNode).FirstChild(GD_STR_ToolsOptionsDebugLogNode).FirstChild().Node();
    GT_IF_WITH_ASSERT(pXmlNode != NULL)
    {
        xmlValueString = osDebugLogSeverityToString(loggedSeverity);
        bool rc8 = writeStringToXmlNode(pXmlNode, xmlValueString);
        GT_IF_WITH_ASSERT(rc8)
        {
            field8ReadOk = true;
        }
    }

    // Get the performance counters timer interval in mSec:
    // gdPerformanceCountersTimer& thePeformanceCountersTimer = gdPerformanceCountersTimer::instance();
    int performanceCountersTimerInterval = 200; //thePeformanceCountersTimer.timerInterval();

    // Set the performance counters timer interval
    pXmlNode = settingsXMLDocHandle.FirstChild(GD_STR_ToolsOptionsCodeXLNode).FirstChild(GD_STR_ToolsOptionsOptionsNode).FirstChild(GD_STR_ToolsOptionsAdvancedNode).FirstChild(GD_STR_ToolsOptionsPerformanceCounterSampleNode).FirstChild().Node();
    GT_IF_WITH_ASSERT(pXmlNode != NULL)
    {
        xmlValueString.makeEmpty();
        xmlValueString.appendFormattedString(L"%d", performanceCountersTimerInterval);
        bool rc9 = writeStringToXmlNode(pXmlNode, xmlValueString);
        GT_IF_WITH_ASSERT(rc9)
        {
            field9ReadOk = true;
        }
    }

    // Get the Floating Point Precision Display:
    unsigned int performanceCountersFloatingPointPrecision = 8;
    performanceCountersFloatingPointPrecision = apGetFloatParamsDisplayPrecision();
    // Set the Floating Point Precision Display
    pXmlNode = settingsXMLDocHandle.FirstChild(GD_STR_ToolsOptionsCodeXLNode).FirstChild(GD_STR_ToolsOptionsOptionsNode).FirstChild(GD_STR_ToolsOptionsAdvancedNode).FirstChild(GD_STR_ToolsOptionsFloatingPointPrecisionDisplayNode).FirstChild().Node();
    GT_IF_WITH_ASSERT(pXmlNode != NULL)
    {
        xmlValueString.makeEmpty();
        xmlValueString.appendFormattedString(L"%u", performanceCountersFloatingPointPrecision);
        bool rc10 = writeStringToXmlNode(pXmlNode, xmlValueString);
        GT_IF_WITH_ASSERT(rc10)
        {
            field10ReadOk = true;
        }
    }

    // Set the Survey number
    // (Not needed anymore)
    field13ReadOk = true;

    // Set the EulaA revision number
    pXmlNode = settingsXMLDocHandle.FirstChild(GD_STR_ToolsOptionsCodeXLNode).FirstChild(GD_STR_EulaAcceptedRevisionNumberNode).FirstChild().Node();

    // Backward compatibility - In case the xml doesn't have a revision number element node then add it.
    if (pXmlNode == NULL)
    {
        // Create the new element node
        TiXmlElement eulaElement(GD_STR_EulaAcceptedRevisionNumberNode);

        // Create the default revision number as Text node
        TiXmlText defaultRevisionNumber(GD_STR_InitialEULARevisionNumber);

        // Add the Text node into the breakpoint node (as child)
        eulaElement.InsertEndChild(defaultRevisionNumber);

        // Add the EULA revision number element node into the XML file
        pXmlNode = settingsXMLDocHandle.FirstChild(GD_STR_ToolsOptionsCodeXLNode).Node();
        GT_IF_WITH_ASSERT(pXmlNode != NULL)
        {
            // Inserting the new EULA element node and returning it
            pXmlNode->InsertEndChild(eulaElement);
            pXmlNode = settingsXMLDocHandle.FirstChild(GD_STR_ToolsOptionsCodeXLNode).FirstChild(GD_STR_EulaAcceptedRevisionNumberNode).FirstChild().Node();
        }
    }

    GT_IF_WITH_ASSERT(pXmlNode != NULL)
    {
        // Get the revision number from the application
        int currentApplicationRevision = afGlobalVariablesManager::instance().getEULRevisionNumber();

        // Format the revision number as string
        gtString eulaNumberAsString;
        eulaNumberAsString.appendFormattedString(L"%d", currentApplicationRevision);

        // Write the string to the xml
        bool rc13 = writeStringToXmlNode(pXmlNode, eulaNumberAsString);
        GT_IF_WITH_ASSERT(rc13)
        {
            field14ReadOk = true;
        }
    }

    // Set the using proxy and proxy server values
    bool isUsingProxy = false;
    osPortAddress proxyServer(AF_STR_Empty, 0);
    bool rcProx = afGlobalVariablesManager::instance().getProxyInformation(isUsingProxy, proxyServer);
    GT_IF_WITH_ASSERT(rcProx)
    {
        pXmlNode = settingsXMLDocHandle.FirstChild(GD_STR_ToolsOptionsCodeXLNode).FirstChild(GD_STR_ToolsOptionsOptionsNode).FirstChild(GD_STR_ToolsOptionsConnectionNode).FirstChild(GD_STR_ToolsOptionsUsingProxyNode).FirstChild().Node();
        GT_IF_WITH_ASSERT(pXmlNode != NULL)
        {
            xmlValueBool = isUsingProxy;
            bool rc14 = writeBoolToXmlNode(pXmlNode, xmlValueBool);
            GT_IF_WITH_ASSERT(rc14)
            {
                field15ReadOk = true;
            }
        }

        if (isUsingProxy)
        {
            pXmlNode = settingsXMLDocHandle.FirstChild(GD_STR_ToolsOptionsCodeXLNode).FirstChild(GD_STR_ToolsOptionsOptionsNode).FirstChild(GD_STR_ToolsOptionsConnectionNode).Node();
            GT_IF_WITH_ASSERT(pXmlNode != NULL)
            {
                proxyServer.toString(xmlValueString);
                GT_IF_WITH_ASSERT(!xmlValueString.isEmpty())
                {
                    TiXmlElement ProxyServerNode(GD_STR_ToolsOptionsProxyServerNode);
                    TiXmlText ProxyServerText(xmlValueString.asASCIICharArray());

                    ProxyServerNode.InsertEndChild(ProxyServerText);
                    pXmlNode = pXmlNode->InsertEndChild(ProxyServerNode)->FirstChild();

                    bool rc15 = writeStringToXmlNode(pXmlNode, xmlValueString);
                    GT_IF_WITH_ASSERT(rc15)
                    {
                        field16ReadOk = true;
                    }
                }
            }
        }
        else
        {
            field16ReadOk = true;
        }
    }

    // Set the AlwaysOnTop
    pXmlNode = settingsXMLDocHandle.FirstChild(GD_STR_ToolsOptionsCodeXLNode).FirstChild(GD_STR_AlwaysOnTopNode).FirstChild().Node();
    GT_IF_WITH_ASSERT(pXmlNode != NULL)
    {
        bool xmlValueBool = afGlobalVariablesManager::instance().getAlwaysOnTopStatus();
        bool rc16 = writeBoolToXmlNode(pXmlNode, xmlValueBool);
        GT_IF_WITH_ASSERT(rc16)
        {
            field17ReadOk = true;
        }
    }

    pXmlNode = settingsXMLDocHandle.FirstChild(GD_STR_ToolsOptionsCodeXLNode).FirstChild(GD_STR_ToolsOptionsOptionsNode).FirstChild(GD_STR_ToolsOptionsLoggingNode).Node();
    GT_IF_WITH_ASSERT(pXmlNode != NULL)
    {
        // Set the logging limits:
        bool field18_1ReadOk = false;
        bool field18_2ReadOk = false;
        bool field18_3ReadOk = false;
        unsigned int maxLoggedOpenGLCallsPerContext = AP_DEFAULT_OPENGL_CONTEXT_CALLS_LOG_MAX_SIZE;
        unsigned int maxLoggedOpenCLCalls = AP_DEFAULT_OPENCL_CONTEXT_CALLS_LOG_MAX_SIZE;
        unsigned int maxLoggedOpenCLCommandsPerQueue = AP_DEFAULT_OPENCL_QUEUE_COMMANDS_LOG_MAX_SIZE;
        theGlobalVariablesMgr.getLoggingLimits(maxLoggedOpenGLCallsPerContext, maxLoggedOpenCLCalls, maxLoggedOpenCLCommandsPerQueue);
        TiXmlNode* pXmlSubNode = pXmlNode->FirstChild(GD_STR_ToolsOptionsMaxOpenGLCallsPerContextNode);

        if (pXmlSubNode != NULL)
        {
            pXmlSubNode = pXmlSubNode->FirstChild();
        }

        GT_IF_WITH_ASSERT(pXmlSubNode != NULL)
        {
            xmlValueString.makeEmpty();
            xmlValueString.appendFormattedString(L"%u", maxLoggedOpenGLCallsPerContext);
            bool rc18 = writeStringToXmlNode(pXmlSubNode, xmlValueString);
            GT_IF_WITH_ASSERT(rc18)
            {
                field18_1ReadOk = true;
            }
        }
        pXmlSubNode = NULL;
        pXmlSubNode = pXmlNode->FirstChild(GD_STR_ToolsOptionsMaxOpenCLCallsPerContextNode);

        if (pXmlSubNode != NULL)
        {
            pXmlSubNode = pXmlSubNode->FirstChild();
        }

        GT_IF_WITH_ASSERT(pXmlSubNode != NULL)
        {
            xmlValueString.makeEmpty();
            xmlValueString.appendFormattedString(L"%u", maxLoggedOpenCLCalls);
            bool rc18 = writeStringToXmlNode(pXmlSubNode, xmlValueString);
            GT_IF_WITH_ASSERT(rc18)
            {
                field18_2ReadOk = true;
            }
        }
        pXmlSubNode = NULL;
        pXmlSubNode = pXmlNode->FirstChild(GD_STR_ToolsOptionsMaxOpenCLCommandsPerQueueNode);

        if (pXmlSubNode != NULL)
        {
            pXmlSubNode = pXmlSubNode->FirstChild();
        }

        GT_IF_WITH_ASSERT(pXmlSubNode != NULL)
        {
            xmlValueString.makeEmpty();
            xmlValueString.appendFormattedString(L"%u", maxLoggedOpenCLCommandsPerQueue);
            bool rc19 = writeStringToXmlNode(pXmlSubNode, xmlValueString);
            GT_IF_WITH_ASSERT(rc19)
            {
                field18_3ReadOk = true;
            }
        }

        field18ReadOk = field18_1ReadOk && field18_2ReadOk && field18_3ReadOk;

        xmlValueString = afGlobalVariablesManager::instance().logFilesDirectoryPath().asString();
        GT_IF_WITH_ASSERT(!xmlValueString.isEmpty())
        {
            TiXmlElement logFilesDirectoryNode(GD_STR_ToolsOptionsLogFilesDirectoryNode);
            TiXmlText logFilesDirectoryText(xmlValueString.asASCIICharArray());

            logFilesDirectoryNode.InsertEndChild(logFilesDirectoryText);
            pXmlNode = pXmlNode->InsertEndChild(logFilesDirectoryNode)->FirstChild();

            bool rc20 = writeStringToXmlNode(pXmlNode, xmlValueString);
            GT_IF_WITH_ASSERT(rc20)
            {
                field19ReadOk = true;
            }
        }
    }

    retVal = field1ReadOk && field2ReadOk && field3ReadOk && field4ReadOk && field5ReadOk && field6ReadOk &&
             field7ReadOk && field8ReadOk && field9ReadOk && field10ReadOk &&
             field13ReadOk && field14ReadOk && field15ReadOk && field16ReadOk && field17ReadOk && field18ReadOk && field19ReadOk;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdOptionsFileHandler::updateApplicationValuesFromXMLDocument
// Description:
//  Updated the application values from the current XML document values.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        12/11/2007
// ---------------------------------------------------------------------------
bool gdOptionsFileHandler::updateApplicationValuesFromXMLDocument()
{
    bool retVal = false;

    bool field1UpdatedOk = false;
    bool field2UpdatedOk = false;
    bool field3UpdatedOk = false;
    bool field4UpdatedOk = false;
    bool field5UpdatedOk = false;
    bool field6UpdatedOk = false;
    bool field7UpdatedOk = false;
    bool field8UpdatedOk = false;
    bool field9UpdatedOk = false;
    bool field10UpdatedOk = false;
    bool field11UpdatedOk = false;
    bool field13UpdatedOk = false;
    bool field15UpdatedOk = false;
    bool field16UpdatedOk = false;
    bool field17UpdatedOk = false;
    bool field18UpdatedOk = false;
    bool field19UpdatedOk = false;

    // Aid variables:
    gtString xmlValueString;

    // A document handle that will wrap our XML document:
    TiXmlHandle settingsXMLDocHandle(&_settingsFileAsXMLDocument);

    // Get the global variables manager:
    gdCodeXLGlobalVariablesManager& theGlobalVariablesMgr = gdCodeXLGlobalVariablesManager::instance();

    // Get the SearchInAdditionalDirectories
    TiXmlNode* pXmlNode = settingsXMLDocHandle.FirstChild(GD_STR_ToolsOptionsCodeXLNode).FirstChild(GD_STR_ToolsOptionsOptionsNode).FirstChild(GD_STR_ToolsOptionsCallStackNode).FirstChild(GD_STR_ToolsOptionsSearchInAdditionalDirectoriesNode).FirstChild().Node();
    GT_IF_WITH_ASSERT(pXmlNode != NULL)
    {
        xmlValueString.makeEmpty();
        bool rc1 = readStringFromXmlNode(pXmlNode, xmlValueString);
        GT_IF_WITH_ASSERT(rc1)
        {
            if (!xmlValueString.isEmpty())
            {
                if (xmlValueString == OS_STR_TrueXMLValueUnicode)
                {
                    theGlobalVariablesMgr.setSearchInAdditionalDirectoriesBool(true);
                }
                else if (xmlValueString == OS_STR_FalseXMLValueUnicode)
                {
                    theGlobalVariablesMgr.setSearchInAdditionalDirectoriesBool(false);
                }
            }

            field1UpdatedOk = true;
        }
    }

    // Get the setSourceCodeRootLocation
    pXmlNode = NULL;
    pXmlNode = settingsXMLDocHandle.FirstChild(GD_STR_ToolsOptionsCodeXLNode).FirstChild(GD_STR_ToolsOptionsOptionsNode).FirstChild(GD_STR_ToolsOptionsCallStackNode).FirstChild(GD_STR_ToolsOptionsSetSourceCodeRootLocationNode).FirstChild().Node();
    GT_IF_WITH_ASSERT(pXmlNode != NULL)
    {
        xmlValueString.makeEmpty();
        bool rc2 = readStringFromXmlNode(pXmlNode, xmlValueString);
        GT_IF_WITH_ASSERT(rc2)
        {
            if (xmlValueString == OS_STR_TrueXMLValueUnicode)
            {
                theGlobalVariablesMgr.setSetSourceCodeRootLocationBool(true);
            }
            else if (xmlValueString == OS_STR_FalseXMLValueUnicode)
            {
                theGlobalVariablesMgr.setSetSourceCodeRootLocationBool(false);
            }

            field2UpdatedOk = true;
        }
    }

    // Get the sourceCodeRootLocation
    pXmlNode = NULL;
    pXmlNode = settingsXMLDocHandle.FirstChild(GD_STR_ToolsOptionsCodeXLNode).FirstChild(GD_STR_ToolsOptionsOptionsNode).FirstChild(GD_STR_ToolsOptionsCallStackNode).FirstChild(GD_STR_ToolsOptionsSourceCodeRootLocationNode).FirstChild().Node();
    GT_IF_WITH_ASSERT(pXmlNode != NULL)
    {

        xmlValueString.makeEmpty();
        bool rc3 = readStringFromXmlNode(pXmlNode, xmlValueString);
        GT_IF_WITH_ASSERT(rc3)
        {
            if (xmlValueString == OS_STR_EmptyXMLStringUnicode)
            {
                theGlobalVariablesMgr.setSourceCodeRootLocation(AF_STR_Empty);
            }
            else
            {
                theGlobalVariablesMgr.setSourceCodeRootLocation(xmlValueString);
            }

            field3UpdatedOk = true;
        }
    }

    // Get the AdditionalSourceCodeDirectories
    pXmlNode = NULL;
    pXmlNode = settingsXMLDocHandle.FirstChild(GD_STR_ToolsOptionsCodeXLNode).FirstChild(GD_STR_ToolsOptionsOptionsNode).FirstChild(GD_STR_ToolsOptionsCallStackNode).FirstChild(GD_STR_ToolsOptionsAdditionalSourceCodeDirectoriesNode).Node();
    GT_IF_WITH_ASSERT(pXmlNode != NULL)
    {
        xmlValueString.makeEmpty();
        bool rc4 = readAdditionalDirectoriesFromXml(pXmlNode, xmlValueString);
        GT_IF_WITH_ASSERT(rc4)
        {
            if (!xmlValueString.isEmpty())
            {
                if (xmlValueString == OS_STR_EmptyXMLStringUnicode)
                {
                    theGlobalVariablesMgr.setAdditionalSourceCodeDirectories(AF_STR_Empty);
                }
                else
                {
                    theGlobalVariablesMgr.setAdditionalSourceCodeDirectories(xmlValueString);
                }
            }

            field4UpdatedOk = true;
        }
    }

    // Get the collectAllocatedObjectsCreationCallsStacks option
    pXmlNode = NULL;
    pXmlNode = settingsXMLDocHandle.FirstChild(GD_STR_ToolsOptionsCodeXLNode).FirstChild(GD_STR_ToolsOptionsOptionsNode).FirstChild(GD_STR_ToolsOptionsCallStackNode).FirstChild(GD_STR_ToolsOptionsCollectAllocatedObjectsCreationCallsStacksNode).FirstChild().Node();

    if (pXmlNode == NULL)
    {
        // This option did not exist in CodeXL versions prior to 4.5, so we allow it to not exist in the file
        field5UpdatedOk = true;
    }
    else
    {
        xmlValueString.makeEmpty();
        bool rc5 = readStringFromXmlNode(pXmlNode, xmlValueString);
        GT_IF_WITH_ASSERT(rc5)
        {
            if (xmlValueString == OS_STR_TrueXMLValueUnicode)
            {
                gaCollectAllocatedObjectsCreationCallsStacks(true);
            }
            else if (xmlValueString == OS_STR_FalseXMLValueUnicode)
            {
                gaCollectAllocatedObjectsCreationCallsStacks(false);
            }

            field5UpdatedOk = true;
        }
    }

    // Get the enableTexturesLogging
    pXmlNode = NULL;
    pXmlNode = settingsXMLDocHandle.FirstChild(GD_STR_ToolsOptionsCodeXLNode).FirstChild(GD_STR_ToolsOptionsOptionsNode).FirstChild(GD_STR_ToolsOptionsTexturesNode).FirstChild(GD_STR_ToolsOptionsEnableTexturesLoggingNode).FirstChild().Node();
    GT_IF_WITH_ASSERT(pXmlNode != NULL)
    {
        xmlValueString.makeEmpty();
        bool rc6 = readStringFromXmlNode(pXmlNode, xmlValueString);
        GT_IF_WITH_ASSERT(rc6)
        {
            if (xmlValueString == OS_STR_TrueXMLValueUnicode)
            {
                gaEnableImagesDataLogging(true);
            }
            else if (xmlValueString == OS_STR_FalseXMLValueUnicode)
            {
                gaEnableImagesDataLogging(false);
            }

            field6UpdatedOk = true;
        }
    }

    // Get the texturesFileFormat
    pXmlNode = NULL;
    pXmlNode = settingsXMLDocHandle.FirstChild(GD_STR_ToolsOptionsCodeXLNode).FirstChild(GD_STR_ToolsOptionsOptionsNode).FirstChild(GD_STR_ToolsOptionsTexturesNode).FirstChild(GD_STR_ToolsOptionsTexturesFileFormatNode).FirstChild().Node();
    GT_IF_WITH_ASSERT(pXmlNode != NULL)
    {
        xmlValueString.makeEmpty();
        bool rc7 = readStringFromXmlNode(pXmlNode, xmlValueString);

        if (rc7)
        {
            if (xmlValueString == OS_STR_EmptyXMLStringUnicode)
            {
                // do nothing
            }
            else
            {
                // Convert the string file format to apFileType
                apFileType texturesFileFormat;
                apFileExtensionStringToFileType(xmlValueString, texturesFileFormat);

                // Set it into the global vars
                theGlobalVariablesMgr.setImagesFileFormat(texturesFileFormat);
            }

            field7UpdatedOk = true;
        }
    }

    // Get the log file severity from the infra:
    pXmlNode = NULL;
    pXmlNode = settingsXMLDocHandle.FirstChild(GD_STR_ToolsOptionsCodeXLNode).FirstChild(GD_STR_ToolsOptionsOptionsNode).FirstChild(GD_STR_ToolsOptionsAdvancedNode).FirstChild(GD_STR_ToolsOptionsDebugLogNode).FirstChild().Node();
    GT_IF_WITH_ASSERT(pXmlNode != NULL)
    {
        xmlValueString.makeEmpty();
        bool rc8 = readStringFromXmlNode(pXmlNode, xmlValueString);
        GT_IF_WITH_ASSERT(rc8)
        {
            if (xmlValueString == OS_STR_EmptyXMLStringUnicode)
            {
                // do nothing
            }
            else
            {
                osDebugLogSeverity debugLogSeverity = osStringToDebugLogSeverity(xmlValueString.asCharArray());

                // Set the log file severity to the infra:
                osDebugLog& theDebugLog = osDebugLog::instance();
                theDebugLog.setLoggedSeverity(debugLogSeverity);
            }

            field8UpdatedOk = true;
        }
    }

    /*  // Get the performance counters timer interval in mSec:
        pXmlNode = NULL;
        pXmlNode = settingsXMLDocHandle.FirstChild(GD_STR_ToolsOptionsCodeXLNode).FirstChild(GD_STR_ToolsOptionsOptionsNode).FirstChild(GD_STR_ToolsOptionsAdvancedNode).FirstChild(GD_STR_ToolsOptionsPerformanceCounterSampleNode).FirstChild().Node();
        GT_IF_WITH_ASSERT(pXmlNode != NULL)
        {
            xmlValueString.makeEmpty();
            bool rc9 = readStringFromXmlNode(pXmlNode, xmlValueString);
            GT_IF_WITH_ASSERT(rc9)
            {
                if (xmlValueString == OS_STR_EmptyXMLStringUnicode)
                {
                    // do nothing
                }
                else
                {
                    GT_IF_WITH_ASSERT(xmlValueString.isIntegerNumber())
                    {
                        int performanceCountersTimerInterval = 200;
                        bool rc10 = xmlValueString.toIntNumber(performanceCountersTimerInterval);
                        GT_IF_WITH_ASSERT(rc10)
                        {
                            gdPerformanceCountersTimer& thePeformanceCountersTimer = gdPerformanceCountersTimer::instance();
                            thePeformanceCountersTimer.setTimerInterval(performanceCountersTimerInterval);

                            field9UpdatedOk = true;
                        }
                    }
                }
            }
        }*/

    // Get the Floating Point Precision Display:
    pXmlNode = NULL;
    pXmlNode = settingsXMLDocHandle.FirstChild(GD_STR_ToolsOptionsCodeXLNode).FirstChild(GD_STR_ToolsOptionsOptionsNode).FirstChild(GD_STR_ToolsOptionsAdvancedNode).FirstChild(GD_STR_ToolsOptionsFloatingPointPrecisionDisplayNode).FirstChild().Node();

    if (pXmlNode == NULL)
    {
        // Old Options files does not contains Floating Point Precision Display value:
        field10UpdatedOk = true;
    }
    else
    {
        xmlValueString.makeEmpty();
        bool rc11 = readStringFromXmlNode(pXmlNode, xmlValueString);
        GT_IF_WITH_ASSERT(rc11)
        {
            if (xmlValueString == OS_STR_EmptyXMLStringUnicode)
            {
                // do nothing
            }
            else
            {
                GT_IF_WITH_ASSERT(xmlValueString.isIntegerNumber())
                {
                    unsigned int performanceCountersFloatingPointPrecision = 8;
                    bool rc12 = xmlValueString.toUnsignedIntNumber(performanceCountersFloatingPointPrecision);
                    GT_IF_WITH_ASSERT(rc12)
                    {
                        // Set the Floating Point Precision Display value to the infra:
                        apSetFloatParamsDisplayPrecision(performanceCountersFloatingPointPrecision);
                        field10UpdatedOk = true;
                    }
                }
            }
        }
    }

    // Get the Eula revision number
    pXmlNode = NULL;
    pXmlNode = settingsXMLDocHandle.FirstChild(GD_STR_ToolsOptionsCodeXLNode).FirstChild(GD_STR_EulaAcceptedRevisionNumberNode).FirstChild().Node();

    if (pXmlNode == NULL)
    {
        // Backwards compatibility: In case the element node doesn't exist -
        // put a default value of -1 so that the EULA dialog would pop
        afGlobalVariablesManager::instance().setEULRevisionNumber(-1);
        field13UpdatedOk = true;
    }
    else
    {
        xmlValueString.makeEmpty();
        bool rc19 = readStringFromXmlNode(pXmlNode, xmlValueString);
        GT_IF_WITH_ASSERT(rc19)
        {
            int revisionNumber;
            bool rc20 = xmlValueString.toIntNumber(revisionNumber);
            GT_IF_WITH_ASSERT(rc20)
            {
                // Set the Variable manager's current revision number
                afGlobalVariablesManager::instance().setEULRevisionNumber(revisionNumber);
                field13UpdatedOk = true;
            }
        }
    }

    // Get the proxy settings:
    pXmlNode = settingsXMLDocHandle.FirstChild(GD_STR_ToolsOptionsCodeXLNode).FirstChild(GD_STR_ToolsOptionsOptionsNode).FirstChild(GD_STR_ToolsOptionsConnectionNode).FirstChild(GD_STR_ToolsOptionsUsingProxyNode).FirstChild().Node();

    if (pXmlNode != NULL)
    {
        xmlValueString.makeEmpty();
        bool rc21 = readStringFromXmlNode(pXmlNode, xmlValueString);
        GT_IF_WITH_ASSERT(rc21)
        {
            bool isUsingProxy = (xmlValueString == OS_STR_TrueXMLValueUnicode);
            osPortAddress proxyServer(AF_STR_Empty, 0);
            field15UpdatedOk = true;

            if (isUsingProxy)
            {
                pXmlNode = settingsXMLDocHandle.FirstChild(GD_STR_ToolsOptionsCodeXLNode).FirstChild(GD_STR_ToolsOptionsOptionsNode).FirstChild(GD_STR_ToolsOptionsConnectionNode).FirstChild(GD_STR_ToolsOptionsProxyServerNode).FirstChild().Node();
                GT_IF_WITH_ASSERT(pXmlNode != NULL)
                {
                    xmlValueString.makeEmpty();
                    bool rc22 = readStringFromXmlNode(pXmlNode, xmlValueString);
                    GT_IF_WITH_ASSERT(rc22)
                    {
                        bool rc23 = proxyServer.fromString(xmlValueString);
                        GT_IF_WITH_ASSERT(rc23)
                        {
                            field16UpdatedOk = true;
                        }
                    }
                }
            }
            else
            {
                field16UpdatedOk = true;
            }

            if (field16UpdatedOk)
            {
                afGlobalVariablesManager::instance().setProxyInformation(isUsingProxy, proxyServer);
            }
        }
    }
    else
    {
        // Older versions don't have connection settings:
        afGlobalVariablesManager::instance().setProxyInformation(false, osPortAddress(AF_STR_Empty, 0));
        field15UpdatedOk = true;
        field16UpdatedOk = true;
    }

    pXmlNode = settingsXMLDocHandle.FirstChild(GD_STR_ToolsOptionsCodeXLNode).FirstChild(GD_STR_AlwaysOnTopNode).FirstChild().Node();

    if (pXmlNode != NULL)
    {
        xmlValueString.makeEmpty();
        bool rc24 = readStringFromXmlNode(pXmlNode, xmlValueString);
        GT_IF_WITH_ASSERT(rc24)
        {
            if (xmlValueString == OS_STR_TrueXMLValueUnicode)
            {
                afGlobalVariablesManager::instance().setAlwaysOnTopStatus(true);
            }
            else if (xmlValueString == OS_STR_FalseXMLValueUnicode)
            {
                afGlobalVariablesManager::instance().setAlwaysOnTopStatus(false);
            }

            field17UpdatedOk = true;
        }
    }
    else
    {
        // In versions before Mac Beta 2.3 / Release 5.0, this field did not exist
        field17UpdatedOk = true;
    }

    pXmlNode = NULL;
    pXmlNode = settingsXMLDocHandle.FirstChild(GD_STR_ToolsOptionsCodeXLNode).FirstChild(GD_STR_ToolsOptionsOptionsNode).FirstChild(GD_STR_ToolsOptionsLoggingNode).Node();

    if (pXmlNode != NULL)
    {
        // Get the logging limits:
        bool field18_1updatedOk = false;
        bool field18_2updatedOk = false;
        bool field18_3updatedOk = false;
        unsigned int maxLoggedOpenGLCallsPerContext = AP_DEFAULT_OPENGL_CONTEXT_CALLS_LOG_MAX_SIZE;
        unsigned int maxLoggedOpenCLCallsPerContext = AP_DEFAULT_OPENCL_CONTEXT_CALLS_LOG_MAX_SIZE;
        unsigned int maxLoggedOpenCLCommandsPerQueue = AP_DEFAULT_OPENCL_QUEUE_COMMANDS_LOG_MAX_SIZE;
        TiXmlNode* pXmlSubNode = pXmlNode->FirstChild(GD_STR_ToolsOptionsMaxOpenGLCallsPerContextNode);

        if (pXmlSubNode != NULL)
        {
            pXmlSubNode = pXmlSubNode->FirstChild();
        }

        GT_IF_WITH_ASSERT(pXmlSubNode != NULL)
        {
            xmlValueString.makeEmpty();
            bool rc25 = readStringFromXmlNode(pXmlSubNode, xmlValueString);
            GT_IF_WITH_ASSERT(rc25)
            {
                // Convert the string file format to uint
                field18_1updatedOk = xmlValueString.toUnsignedIntNumber(maxLoggedOpenGLCallsPerContext);
                GT_ASSERT(field18_1updatedOk);
            }
        }
        pXmlSubNode = NULL;
        pXmlSubNode = pXmlNode->FirstChild(GD_STR_ToolsOptionsMaxOpenCLCallsPerContextNode);

        if (pXmlSubNode != NULL)
        {
            pXmlSubNode = pXmlSubNode->FirstChild();
        }

        GT_IF_WITH_ASSERT(pXmlSubNode != NULL)
        {
            xmlValueString.makeEmpty();
            bool rc26 = readStringFromXmlNode(pXmlSubNode, xmlValueString);
            GT_IF_WITH_ASSERT(rc26)
            {
                // Convert the string file format to uint
                field18_2updatedOk = xmlValueString.toUnsignedIntNumber(maxLoggedOpenCLCallsPerContext);
                GT_ASSERT(field18_2updatedOk);
            }
        }
        pXmlSubNode = NULL;
        pXmlSubNode = pXmlNode->FirstChild(GD_STR_ToolsOptionsMaxOpenCLCommandsPerQueueNode);

        if (pXmlSubNode != NULL)
        {
            pXmlSubNode = pXmlSubNode->FirstChild();
        }

        GT_IF_WITH_ASSERT(pXmlSubNode != NULL)
        {
            xmlValueString.makeEmpty();
            bool rc27 = readStringFromXmlNode(pXmlSubNode, xmlValueString);
            GT_IF_WITH_ASSERT(rc27)
            {
                // Convert the string file format to uint
                field18_3updatedOk = xmlValueString.toUnsignedIntNumber(maxLoggedOpenCLCommandsPerQueue);
                GT_ASSERT(field18_3updatedOk);
            }
        }

        field18UpdatedOk = field18_1updatedOk && field18_2updatedOk && field18_3updatedOk;
        GT_IF_WITH_ASSERT(field18UpdatedOk)
        {
            theGlobalVariablesMgr.setLoggingLimits(maxLoggedOpenGLCallsPerContext, maxLoggedOpenCLCallsPerContext, maxLoggedOpenCLCommandsPerQueue);
        }

        // Get the log files directory:
        pXmlSubNode = NULL;
        pXmlSubNode = pXmlNode->FirstChild(GD_STR_ToolsOptionsLogFilesDirectoryNode);

        if (pXmlSubNode != NULL)
        {
            pXmlSubNode = pXmlSubNode->FirstChild();
            GT_IF_WITH_ASSERT(pXmlSubNode != NULL)
            {
                xmlValueString.makeEmpty();
                bool rc28 = readStringFromXmlNode(pXmlSubNode, xmlValueString);
                GT_IF_WITH_ASSERT(rc28)
                {
                    xmlValueString.removeTrailing(osFilePath::osPathSeparator).append(osFilePath::osPathSeparator);
                    osFilePath logFilesDirectoryPath(xmlValueString);

                    // If the directory does not exist, create it:
                    if (!logFilesDirectoryPath.exists())
                    {
                        osDirectory logsDirectory(logFilesDirectoryPath);
                        bool rcCreate = logsDirectory.create();
                        GT_ASSERT(rcCreate);
                    }

                    // Make sure that the log files path is an existing directory:
                    GT_IF_WITH_ASSERT(logFilesDirectoryPath.isDirectory())
                    {
                        afGlobalVariablesManager::instance().setLogFilesDirectoryPath(logFilesDirectoryPath);
                        field19UpdatedOk = true;
                    }
                }
            }
        }
        else // pXmlSubNode == NULL
        {
            // In versions before 6.0, this field was saved per-project:
            field19UpdatedOk = true;
        }
    }
    else
    {
        // In versions before 5.5.1, this field did not exist:
        field18UpdatedOk = true;
        field19UpdatedOk = true;
    }

    retVal = field1UpdatedOk && field2UpdatedOk && field3UpdatedOk && field4UpdatedOk && field5UpdatedOk && field6UpdatedOk &&
             field7UpdatedOk && field8UpdatedOk && field9UpdatedOk && field10UpdatedOk && field13UpdatedOk &&
             field15UpdatedOk && field16UpdatedOk && field17UpdatedOk && field18UpdatedOk && field19UpdatedOk;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdOptionsFileHandler::initXMLDocToSettingsFileDefaultValues
// Description: Initialize the XML document to a default options file content.
// Arguments: XMLDocument - The XML document to be initialized.
// Author:      Yaki Tebeka
// Date:        11/11/2007
// ---------------------------------------------------------------------------
bool gdOptionsFileHandler::initXMLDocToSettingsFileDefaultValues(TiXmlDocument& XMLDocument)
{
    bool retVal = false;

    // The Options file default content:
    const char* pOptionsFileDefaultContent =
        "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
        "<"GD_STR_ToolsOptionsCodeXLNode">\n"
        "<"GD_STR_ToolsOptionsOptionsNode">\n"
        "<"GD_STR_ToolsOptionsCallStackNode">\n"
        "<"GD_STR_ToolsOptionsSearchInAdditionalDirectoriesNode">"OS_STR_FalseXMLValue"</"GD_STR_ToolsOptionsSearchInAdditionalDirectoriesNode">\n"
        "<"GD_STR_ToolsOptionsSetSourceCodeRootLocationNode">"OS_STR_FalseXMLValue"</"GD_STR_ToolsOptionsSetSourceCodeRootLocationNode">\n"
        "<"GD_STR_ToolsOptionsSourceCodeRootLocationNode">"OS_STR_EmptyXMLString"</"GD_STR_ToolsOptionsSourceCodeRootLocationNode">\n"
        "<"GD_STR_ToolsOptionsAdditionalSourceCodeDirectoriesNode">\n"
        //                      "<AdditionalDirectory>C:\work</AdditionalDirectory>\n" //**Will be added dynamically**//
        "</"GD_STR_ToolsOptionsAdditionalSourceCodeDirectoriesNode">\n"
        "<"GD_STR_ToolsOptionsCollectAllocatedObjectsCreationCallsStacksNode">"OS_STR_TrueXMLValue"</"GD_STR_ToolsOptionsCollectAllocatedObjectsCreationCallsStacksNode">\n"
        "</"GD_STR_ToolsOptionsCallStackNode">\n"
        "<"GD_STR_ToolsOptionsTexturesNode">\n"
        "<"GD_STR_ToolsOptionsEnableTexturesLoggingNode">"OS_STR_TrueXMLValue"</"GD_STR_ToolsOptionsEnableTexturesLoggingNode">\n"
        "<"GD_STR_ToolsOptionsTexturesFileFormatNode">"OS_STR_FalseXMLValue"</"GD_STR_ToolsOptionsTexturesFileFormatNode">\n"
        "</"GD_STR_ToolsOptionsTexturesNode">\n"
        "<"GD_STR_ToolsOptionsAdvancedNode">\n"
        "<"GD_STR_ToolsOptionsDebugLogNode">INFO</"GD_STR_ToolsOptionsDebugLogNode">\n"
        "<"GD_STR_ToolsOptionsPerformanceCounterSampleNode">200</"GD_STR_ToolsOptionsPerformanceCounterSampleNode">\n"
        "<"GD_STR_ToolsOptionsFloatingPointPrecisionDisplayNode">8</"GD_STR_ToolsOptionsFloatingPointPrecisionDisplayNode">\n"
        "</"GD_STR_ToolsOptionsAdvancedNode">\n"
        "<"GD_STR_ToolsOptionsConnectionNode">\n"
        "<"GD_STR_ToolsOptionsUsingProxyNode">"OS_STR_FalseXMLValue"</"GD_STR_ToolsOptionsUsingProxyNode">\n"
        "</"GD_STR_ToolsOptionsConnectionNode">\n"
        "<"GD_STR_ToolsOptionsLoggingNode">\n"
        "<"GD_STR_ToolsOptionsMaxOpenGLCallsPerContextNode">4000000</"GD_STR_ToolsOptionsMaxOpenGLCallsPerContextNode">"
        "<"GD_STR_ToolsOptionsMaxOpenCLCallsPerContextNode">50000</"GD_STR_ToolsOptionsMaxOpenCLCallsPerContextNode">"
        "<"GD_STR_ToolsOptionsMaxOpenCLCommandsPerQueueNode">2000</"GD_STR_ToolsOptionsMaxOpenCLCommandsPerQueueNode">"
        // "<"GD_STR_ToolsOptionsLogFilesDirectoryNode">" "C:\temp" "</"GD_STR_ToolsOptionsLogFilesDirectoryNode">\n" //**Will be added dynamically**//
        "</"GD_STR_ToolsOptionsLoggingNode">\n"
        "</"GD_STR_ToolsOptionsOptionsNode">\n"
        "<"GD_STR_EulaAcceptedRevisionNumberNode">\n"GD_STR_InitialEULARevisionNumber"</"GD_STR_EulaAcceptedRevisionNumberNode">\n"
        "<"GD_STR_AlwaysOnTopNode">"OS_STR_FalseXMLValue"</"GD_STR_AlwaysOnTopNode">\n"
        "</"GD_STR_ToolsOptionsCodeXLNode">\n";

    // Initialize the XML document to the above default context:
    XMLDocument.Parse(pOptionsFileDefaultContent);

    // If all went ok:
    if (!XMLDocument.Error())
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdOptionsFileHandler::writeAdditionalSourceCodeDirectoriesStringToXml
// Description: Write the Additional Directories into the XML file
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        24/1/2005
// ---------------------------------------------------------------------------
bool gdOptionsFileHandler::writeAdditionalSourceCodeDirectoriesStringToXml(TiXmlNode* pXmlNode, gtString dataFieldValue)
{
    // The Additional Directories are ';' separated:
    gtStringTokenizer strTokenizer(dataFieldValue, L";");
    gtString currentAdditionalDirectory;

    // Iterate over the Additional Directories strings, and push them into the xml file:
    while (strTokenizer.getNextToken(currentAdditionalDirectory))
    {
        // Get the AdditionalSourceCodeDirectories <AdditionalDirectory> node
        TiXmlElement AdditionalDirectory(GD_STR_ToolsOptionsAdditionalDirectoryNode);

        // Create the AdditionalDirectory name as Text node
        TiXmlText AdditionalDirectoryText(currentAdditionalDirectory.asASCIICharArray());

        // Add the Text node into the breakpoint node (as child)
        AdditionalDirectory.InsertEndChild(AdditionalDirectoryText);

        // Add the breakpoint node into the XML file
        pXmlNode->InsertEndChild(AdditionalDirectory);
    }

    return true;
}


// ---------------------------------------------------------------------------
// Name:        gdLoadBreakpointsCommand::readOpenGLStateVar
// Description: Read the OpenGL state variable from the file and add it into the app
// Arguments:   wxCommandEvent& event
// Author:      Avi Shapira
// Date:        24/1/2005
// ---------------------------------------------------------------------------
bool gdOptionsFileHandler::readAdditionalDirectoriesFromXml(const TiXmlNode* pXmlNode, gtString& string)
{
    bool retVal = false;
    gtString xmlNodeValue;

    if (pXmlNode)
    {
        // Walk on all the function breakpoints nodes
        const TiXmlNode* pNode = NULL;

        for (pNode = pXmlNode->FirstChild(); pNode != NULL; pNode = pNode->NextSibling())
        {
            if (pNode)
            {
                // Create the additional name as Text node
                const TiXmlText* pXMLNodeText = pNode->FirstChild()->ToText();

                // If the text is not empty - Add the additional directory name
                if (NULL != pXMLNodeText)
                {
                    gtString xmlValue;
                    xmlValue.fromASCIIString(pXMLNodeText->Value());
                    xmlNodeValue += xmlValue;
                    xmlNodeValue += L";";
                }
            }
        }

        retVal = true;
    }

    string = xmlNodeValue;
    return retVal;
}

