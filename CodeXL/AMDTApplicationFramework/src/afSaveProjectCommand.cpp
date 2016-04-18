//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afSaveProjectCommand.cpp
///
//==================================================================================


// Ignore warnings:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Qt:
#include <QtWidgets>

//TinyXml
#include <tinyxml.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/apProjectSettings.h>
#include <AMDTAPIClasses/Include/apAIDFunctions.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afSaveProjectCommand.h>

// ---------------------------------------------------------------------------
// Name:        afSaveProjectCommand::afSaveProjectCommand
// Description: Constructor
// Arguments:   const osFilePath& fileName
// Author:      Sigal Algranaty
// Date:        4/4/2012
// ---------------------------------------------------------------------------
afSaveProjectCommand::afSaveProjectCommand(const osFilePath& fileName) : _filePath(fileName), m_pProjectSettings(nullptr)
{
}


// ---------------------------------------------------------------------------
// Name:        afSaveProjectCommand::~afSaveProjectCommand
// Description: Destructor.
// Author:      Avi Shapira
// Date:        10/1/2007
// ---------------------------------------------------------------------------
afSaveProjectCommand::~afSaveProjectCommand()
{
}


// ---------------------------------------------------------------------------
// Name:        afSaveProjectCommand::canExecuteSpecificCommand
// Description: Answers the question - can we save the project of the G-Debugger application.
// Author:      Avi Shapira
// Date:        8/11/2003
// Implementation Notes:
//  Currently - the answer is always yes.
// ---------------------------------------------------------------------------
bool afSaveProjectCommand::canExecuteSpecificCommand()
{
    return true;
}

// ---------------------------------------------------------------------------
// Name:        afSaveProjectCommand::executeSpecificCommand
// Description: Save the project information
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        4/4/2012
// ---------------------------------------------------------------------------
bool afSaveProjectCommand::executeSpecificCommand()
{
    bool retVal = false;

    // Prepare the data to be saved:
    if (m_pProjectSettings == nullptr)
    {
        m_pProjectSettings = &afProjectManager::instance().currentProjectSettings();

        m_extensionSettingsStrings.clear();

        // Get the current extensions settings as strings:
        int extensionsNumber = afProjectManager::instance().amountOfProjectExtensions();

        for (int i = 0; i < extensionsNumber; i++)
        {
            gtString currentExtensionXMLString;
            bool rc = afProjectManager::instance().currentProjectDataAsXMLString(i, currentExtensionXMLString);
            GT_IF_WITH_ASSERT(rc)
            {
                // Add the string to the vector:
                m_extensionSettingsStrings.push_back(currentExtensionXMLString);
            }
        }
    }

    // Save the general project settings:
    retVal = SaveGeneralProjectSettings();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afSaveProjectCommand::SaveGeneralProjectSettings
// Description:
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        4/4/2012
// ---------------------------------------------------------------------------
bool afSaveProjectCommand::SaveGeneralProjectSettings()
{
    bool rc = true;

    // Create the directory:
    rc = createAppDataDir();

    if (rc && (m_pProjectSettings != nullptr))
    {
        // Create the XML document:
        std::string utf8FilePath;
        _filePath.asString().asUtf8(utf8FilePath);
        TiXmlDocument doc(utf8FilePath.c_str());

        // Initialize the XML file structure:
        rc = initXMLFileStructure(doc);

        // Create XML document handle:
        TiXmlHandle docHandle(&doc);
        gtString fieldName;

        if (!afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
        {
            // Set the Debugged Application path:
            rc = rc && setDebuggedApplicationData(docHandle, fieldName.fromASCIIString(AF_STR_loadProjectProjectNameNode), m_pProjectSettings->projectName());

            // Set the Debugged Application path:
            if (rc)
            {
                const gtString& app = m_pProjectSettings->windowsStoreAppUserModelID().isEmpty() ? m_pProjectSettings->executablePath().asString() :
                                      m_pProjectSettings->windowsStoreAppUserModelID();
                rc = setDebuggedApplicationData(docHandle, fieldName.fromASCIIString(AF_STR_loadProjectgApplicationPathNode), app);

                bool isStoreApp = !m_pProjectSettings->windowsStoreAppUserModelID().isEmpty();
                gtString isStoreAppString = isStoreApp ? L"yes" : L"no";

                rc = setDebuggedApplicationData(docHandle, fieldName.fromASCIIString(AF_STR_loadProjectIsStoreApp), isStoreAppString);
            }

            // Set the Debugged Application working directory:
            rc = rc && setDebuggedApplicationData(docHandle, fieldName.fromASCIIString(AF_STR_loadProjectWorkDirNode), m_pProjectSettings->workDirectory().asString());

            // Set the Debugged Application source code root directory:
            rc = rc && setDebuggedApplicationData(docHandle, fieldName.fromASCIIString(AF_STR_loadProjectSourceCodeRootDirectoryNode), m_pProjectSettings->SourceCodeRootLocation());

            // Set the Debugged Application args:
            rc = rc && setDebuggedApplicationData(docHandle, fieldName.fromASCIIString(AF_STR_loadProjectAppArgsNode), m_pProjectSettings->commandLineArguments());

            // Set the Debugged Application args:
            rc = rc && setDebuggedApplicationEnvironmentVariables(docHandle);

            // Set daemon configurations.
            rc = true;
            gtString isRemoteSession = m_pProjectSettings->isRemoteTarget() ? L"yes" : L"no";
            gtString remoteDaemonPort = L"";
            remoteDaemonPort.appendFormattedString(L"%d", m_pProjectSettings->remoteTargetDaemonConnectionPort());
            rc = rc && setDebuggedApplicationData(docHandle, fieldName.fromASCIIString(AF_STR_loadProjectIsRemoteSession), isRemoteSession);
            rc = rc && setDebuggedApplicationData(docHandle, fieldName.fromASCIIString(AF_STR_loadProjectRemoteDaemonHostName), m_pProjectSettings->remoteTargetName());
            rc = rc && setDebuggedApplicationData(docHandle, fieldName.fromASCIIString(AF_STR_loadProjectRemoteDaemonPortNumber), remoteDaemonPort);

            // Set the recently used host IP addresses:
            gtString recentHostsStr = m_pProjectSettings->GetRecentlyUsedRemoteIPAddressesAsString();
            rc = rc && setDebuggedApplicationData(docHandle, fieldName.fromASCIIString(AF_STR_loadProjectRecentlyUsedRemoteIPAddresses), recentHostsStr);


        }

        // Set the Debugged Application source files directory:
        rc = rc && setDebuggedApplicationData(docHandle, fieldName.fromASCIIString(AF_STR_loadProjectSourceFilesDirectoryNode), m_pProjectSettings->SourceFilesDirectories());

        // Set the last active mode:
        rc = rc && setDebuggedApplicationData(docHandle, fieldName.fromASCIIString(AF_STR_loadProjectLastActiveModeNode), m_pProjectSettings->lastActiveMode());

        // Set the last session type:
        rc = rc && setDebuggedApplicationData(docHandle, fieldName.fromASCIIString(AF_STR_loadProjectLastSessionTypeNode), m_pProjectSettings->lastActiveSessionType());

        // Save the extensions project settings:
        rc = rc && SaveExtensionsProjectSettings(docHandle);

        // Save the file into the disk
        rc = rc && doc.SaveFile();
    }

    return rc;
}

// ---------------------------------------------------------------------------
// Name:        afSaveProjectCommand::initXMLFileStructure
// Description: Initialize the XML file structure. The data would be stored lately
//              within the XML.
// Arguments:   TiXmlDocument& xmlDoc
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        5/4/2012
// ---------------------------------------------------------------------------
bool afSaveProjectCommand::initXMLFileStructure(TiXmlDocument& xmlDoc)
{
    bool rc = true;

    // The project file format - we will use this file every time
    const char* pProjectFileSA =
        "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
        "<" AF_STR_loadProjectFileVersionNode">1</" AF_STR_loadProjectFileVersionNode">"
        "<%1>\n"
        "<" AF_STR_loadProjectProjectNameNode">" OS_STR_EmptyXMLString"</" AF_STR_loadProjectProjectNameNode">\n"
        "<" AF_STR_loadProjectgApplicationPathNode">" OS_STR_EmptyXMLString"</" AF_STR_loadProjectgApplicationPathNode">\n"
        "<" AF_STR_loadProjectIsStoreApp">" OS_STR_EmptyXMLString"</" AF_STR_loadProjectIsStoreApp">\n"
        "<" AF_STR_loadProjectWorkDirNode">" OS_STR_EmptyXMLString"</" AF_STR_loadProjectWorkDirNode">\n"
        "<" AF_STR_loadProjectSourceFilesDirectoryNode">" OS_STR_EmptyXMLString"</" AF_STR_loadProjectSourceFilesDirectoryNode">\n"
        "<" AF_STR_loadProjectSourceCodeRootDirectoryNode">" OS_STR_EmptyXMLString"</" AF_STR_loadProjectSourceCodeRootDirectoryNode">\n"
        "<" AF_STR_loadProjectAppArgsNode">" OS_STR_EmptyXMLString"</" AF_STR_loadProjectAppArgsNode">\n"
        "<" AF_STR_loadProjectLastActiveModeNode">" OS_STR_EmptyXMLString"</" AF_STR_loadProjectLastActiveModeNode">\n"
        "<" AF_STR_loadProjectIsRemoteSession">" OS_STR_EmptyXMLString"</" AF_STR_loadProjectIsRemoteSession">\n"
        "<" AF_STR_loadProjectRemoteDaemonHostName">" OS_STR_EmptyXMLString"</" AF_STR_loadProjectRemoteDaemonHostName">\n"
        "<" AF_STR_loadProjectRemoteDaemonPortNumber">" OS_STR_EmptyXMLString"</" AF_STR_loadProjectRemoteDaemonPortNumber">\n"
        "<" AF_STR_loadProjectRecentlyUsedRemoteIPAddresses">" OS_STR_EmptyXMLString"</" AF_STR_loadProjectRecentlyUsedRemoteIPAddresses">\n"
        "<" AF_STR_loadProjectLastSessionTypeNode">" OS_STR_EmptyXMLString"</" AF_STR_loadProjectLastSessionTypeNode">\n"
        "<" AF_STR_loadProjectProfileNameNode">" OS_STR_EmptyXMLString"</" AF_STR_loadProjectProfileNameNode">\n"
        "<" AF_STR_loadProjectAppEnvVarsNode">" OS_STR_EmptyXMLString"</" AF_STR_loadProjectAppEnvVarsNode">\n"
        "<" AF_STR_loadProjectProductExtensionsNode">" OS_STR_EmptyXMLString"</" AF_STR_loadProjectProductExtensionsNode">\n"
        "</%2>\n";

    const char* pProjectFileVS =
        "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
        "<" AF_STR_loadProjectFileVersionNode">1</" AF_STR_loadProjectFileVersionNode">"
        "<%1>\n"
        "<" AF_STR_loadProjectSourceFilesDirectoryNode">" OS_STR_EmptyXMLString"</" AF_STR_loadProjectSourceFilesDirectoryNode">\n"
        "<" AF_STR_loadProjectLastActiveModeNode">" OS_STR_EmptyXMLString"</" AF_STR_loadProjectLastActiveModeNode">\n"
        "<" AF_STR_loadProjectLastSessionTypeNode">" OS_STR_EmptyXMLString"</" AF_STR_loadProjectLastSessionTypeNode">\n"
        "<" AF_STR_loadProjectProfileNameNode">" OS_STR_EmptyXMLString"</" AF_STR_loadProjectProfileNameNode">\n"
        "<" AF_STR_loadProjectProductExtensionsNode">" OS_STR_EmptyXMLString"</" AF_STR_loadProjectProductExtensionsNode">\n"
        "</%2>\n";

    QString projectSettingsStr;

    if (afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
    {
        projectSettingsStr = QString(pProjectFileVS).arg(afGlobalVariablesManager::ProductNameA()).arg(afGlobalVariablesManager::ProductNameA());
    }
    else
    {
        projectSettingsStr = QString(pProjectFileSA).arg(afGlobalVariablesManager::ProductNameA()).arg(afGlobalVariablesManager::ProductNameA());
    }

    xmlDoc.Parse(projectSettingsStr.toLocal8Bit().data());


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
// Name:        afSaveProjectCommand::createAppDataDir
// Description: Checks if the application Data directory exist.
//              If not - create it
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        1/6/2004
// ---------------------------------------------------------------------------
bool afSaveProjectCommand::createAppDataDir()
{
    bool rc = true;

    // Check if the Application Data directory exist:
    osDirectory destinationDir;
    _filePath.getFileDirectory(destinationDir);

    if (!(destinationDir.exists()))
    {
        // Create the destination Dir
        rc = destinationDir.create();
    }

    GT_ASSERT(rc);
    return rc;
}


// ---------------------------------------------------------------------------
// Name:        afSaveProjectCommand::setDebuggedApplicationData
// Description: Set the debugged application data into the XML file
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        1/6/2004
// ---------------------------------------------------------------------------
bool afSaveProjectCommand::setDebuggedApplicationData(TiXmlHandle& docHandle, const gtString& dataFieldName, const gtString& dataFieldValue)
{
    bool rc = false;

    // TinyXML does not support wide strings but it does supports UTF8 so we convert the strings to UTF8
    std::string utf8FieldValue;
    dataFieldValue.asUtf8(utf8FieldValue);

    // Get the data field:
    TiXmlNode* pDataField = docHandle.FirstChild(afGlobalVariablesManager::ProductNameCharArray()).FirstChild(dataFieldName.asASCIICharArray()).FirstChild().Node();

    if (pDataField)
    {
        // Get the field text
        TiXmlText* pDataFieldText = pDataField->ToText();

        if (pDataFieldText)
        {
            // Set the filed text
            if (utf8FieldValue.empty())
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
// Name:        afSaveProjectCommand::setDebuggedApplicationEnvironmentVariables
// Description: Parses and sets the environment variables from the member
//              list into the XML file represented by docHandle.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        1/6/2008
// ---------------------------------------------------------------------------
bool afSaveProjectCommand::setDebuggedApplicationEnvironmentVariables(TiXmlHandle& docHandle)
{
    bool retVal = false;

    TiXmlNode* pEnvironmentVarsNode = docHandle.FirstChild(afGlobalVariablesManager::ProductNameCharArray()).FirstChild(AF_STR_loadProjectAppEnvVarsNode).Node();

    if ((pEnvironmentVarsNode != nullptr) && (m_pProjectSettings != nullptr))
    {
        retVal = true;

        // Get the environment variables:
        gtList<osEnvironmentVariable> applicationEnvironmentVars = m_pProjectSettings->environmentVariables();

        // Empty the node's value
        pEnvironmentVarsNode->FirstChild()->ToText()->SetValue("");

        gtList<osEnvironmentVariable>::const_iterator iter = applicationEnvironmentVars.begin();
        gtList<osEnvironmentVariable>::const_iterator endIter = applicationEnvironmentVars.end();

        while (iter != endIter)
        {
            osEnvironmentVariable currentEnvVar = *iter;

            if (currentEnvVar._name.isEmpty())
            {
                currentEnvVar._name.fromASCIIString(OS_STR_EmptyXMLString);
            }

            if (currentEnvVar._value.isEmpty())
            {
                currentEnvVar._value.fromASCIIString(OS_STR_EmptyXMLString);
            }

            // Escape all triangle brackets for writing to an XML
            apHandleXMLEscaping(currentEnvVar._name, true);
            apHandleXMLEscaping(currentEnvVar._value, true);

            // Create an environment variable element
            TiXmlElement environmentVar(AF_STR_loadProjectAppEnvVarsVarNode);
            TiXmlElement envVarName(AF_STR_loadProjectAppEnvVarsNameNode);
            TiXmlElement envVarValue(AF_STR_loadProjectAppEnvVarsValueNode);

            TiXmlText nameText(iter->_name.asASCIICharArray());
            TiXmlText valueText(iter->_value.asASCIICharArray());
            envVarName.InsertEndChild(nameText);
            envVarValue.InsertEndChild(valueText);
            environmentVar.InsertEndChild(envVarName);
            environmentVar.InsertEndChild(envVarValue);

            // Add the breakpoint node into the XML file
            pEnvironmentVarsNode->InsertEndChild(environmentVar);

            iter++;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afSaveProjectCommand::SaveExtensionsProjectSettings
// Description: Add an XML node for the extensions
// Arguments:   TiXmlHandle& docHandle
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        5/4/2012
// ---------------------------------------------------------------------------
bool afSaveProjectCommand::SaveExtensionsProjectSettings(TiXmlHandle& docHandle)
{
    bool retVal = false;

    // Get the extension data node:
    TiXmlNode* pExtensionsNode = docHandle.FirstChild(afGlobalVariablesManager::ProductNameCharArray()).FirstChild(AF_STR_loadProjectProductExtensionsNode).Node();

    GT_IF_WITH_ASSERT(pExtensionsNode != nullptr)
    {
        retVal = true;

        // Get the project settings manager:
        int extensionsNumber = m_extensionSettingsStrings.size();

        for (int i = 0; i < extensionsNumber; i++)
        {
            gtString currentExtensionXMLString = m_extensionSettingsStrings[i];
            GT_IF_WITH_ASSERT_EX(!currentExtensionXMLString.isEmpty(), L"The XML string for the extension cannot be empty")
            {
                // Build the current extension settings as XML node:
                TiXmlDocument tempDocument;
                QString xmlString = acGTStringToQString(currentExtensionXMLString);
                tempDocument.Parse(xmlString.toUtf8().data());

                // Set an empty text for the extensions node:
                pExtensionsNode->FirstChild()->ToText()->SetValue("");

                // Create XML document handle:
                TiXmlHandle docHandleInner(&tempDocument);
                TiXmlNode* pNode = docHandleInner.FirstChild().Node();
                GT_IF_WITH_ASSERT_EX((pNode != nullptr), L"The xml is not valid")
                {
                    pExtensionsNode->InsertEndChild(*pNode);
                }
            }
        }
    }

    return retVal;
}

void afSaveProjectCommand::SetProjectSettings(const apProjectSettings* pProjectSettings, const gtVector<gtString>& extensionSettingsAsString)
{
    m_pProjectSettings = pProjectSettings;
    m_extensionSettingsStrings = extensionSettingsAsString;
}

