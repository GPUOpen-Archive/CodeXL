//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afLoadProjectCommand.cpp
///
//==================================================================================

// gtDefinitions needs to be first because of INT16_MAX
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Qt:
#include <QtWidgets>

//TinyXml
#include <tinyxml.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTAPIClasses/Include/apAIDFunctions.h>
#include <AMDTAPIClasses/Include/apProjectSettings.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afLoadProjectCommand.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>

// ---------------------------------------------------------------------------
// Name:        afLoadProjectCommand::afLoadProjectCommand
// Description: Constructor
// Arguments:   const gtString& projectFilePath
// Author:      Sigal Algranaty
// Date:        8/4/2012
// ---------------------------------------------------------------------------
afLoadProjectCommand::afLoadProjectCommand(const gtString& projectFilePath)
    : m_filePath(projectFilePath)
{
}

// ---------------------------------------------------------------------------
// Name:        afLoadProjectCommand::~afLoadProjectCommand
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        8/4/2012
// ---------------------------------------------------------------------------
afLoadProjectCommand::~afLoadProjectCommand()
{
}

// ---------------------------------------------------------------------------
// Name:        afLoadProjectCommand::canExecuteSpecificCommand
// Description: Checks if the command can be executed
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        8/4/2012
// ---------------------------------------------------------------------------
bool afLoadProjectCommand::canExecuteSpecificCommand()
{
    return true;
}

// ---------------------------------------------------------------------------
// Name:        afLoadProjectCommand::executeSpecificCommand
// Description: Perform the project load
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        8/4/2012
// ---------------------------------------------------------------------------
bool afLoadProjectCommand::executeSpecificCommand()
{
    bool retVal = false;

    // Read the generic project settings from the project file
    retVal = ReadProjectGenericSettings();
    GT_IF_WITH_ASSERT(retVal)
    {
        // Set the manager current settings:
        afProjectManager::instance().setCurrentProject(m_projectSettings);

        if (afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
        {
            // Set the correct project path
            afProjectManager::instance().setCurrentProjectFilePath(m_filePath);
        }

        // Set the original project path
        osFilePath originalPathOfProject(m_filePath);
        afProjectManager::instance().setOriginalProjectFilePath(originalPathOfProject.fileDirectoryAsString());

        //If the original path is not the current path, we need to grab profile files from the original
        if (originalPathOfProject != afProjectManager::instance().currentProjectFilePath())
        {
            //Build the profile files directory path
            gtString projectProfileOutputPath(afProjectManager::instance().currentProjectSettings().projectName());
            projectProfileOutputPath.prepend(osFilePath::osPathSeparator);
            projectProfileOutputPath.append(AF_STR_ProfileDirExtension);

            //Build the path to the original profile files
            gtString baseOutputPath(originalPathOfProject.fileDirectoryAsString());
            baseOutputPath.append(projectProfileOutputPath);

            //If the profile files directory exists
            osDirectory baseOutputDir(baseOutputPath);

            if (baseOutputDir.exists())
            {
                //Copy the profile files directory to the current project file path
                gtList<gtString> filter;
                filter.push_back(OS_ALL_CONTAINED_FILES_SEARCH_STR);
                gtString destPath(afProjectManager::instance().currentProjectFilePath().fileDirectoryAsString());
                destPath.append(projectProfileOutputPath);
                bool rc = baseOutputDir.copyFilesToDirectory(destPath, filter);
                GT_ASSERT(rc);
            }
        }

        // NOTICE: Only after that the general options are set, load the extensions specific XML strings:
        retVal = setExtensionXMLData();

        if (!afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
        {
            // Update the application with the recently used projects names:
            afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
            GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
            {
                bool rc = pApplicationCommands->UpdateRecentlyUsedProjects();
                GT_ASSERT(rc);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afLoadProjectCommand::ReadProjectGenericSettings
// Description: Read the project settings from the project XML file
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        8/4/2012
// ---------------------------------------------------------------------------
bool afLoadProjectCommand::ReadProjectGenericSettings()
{
    bool retVal = false;

    // Before loading a new project, restore the default settings for all extensions:
    afProjectManager::instance().restoreDefaultExtensionsProjectSettings();

    // Define an XML document:
    std::string utf8FilePath;
    m_filePath.asUtf8(utf8FilePath);
    TiXmlDocument doc(utf8FilePath.c_str());

    // Load the file:
    retVal = doc.LoadFile();

    if (retVal)
    {
        TiXmlHandle docHandle(&doc);

        if (!afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
        {
            // Get the project executable path:
            TiXmlNode* pProjectNamePath = getProjectRootChildNode(docHandle, AF_STR_loadProjectProjectNameNode);

            if (pProjectNamePath)
            {
                TiXmlText* pProjectNamePathText = pProjectNamePath->ToText();

                if (pProjectNamePathText)
                {
                    gtString projectNameStr;
                    readXmlFieldValue(*pProjectNamePathText, projectNameStr);
                    m_projectSettings.setProjectName(projectNameStr);
                }
            }

            // Check if we are in a remote session.
            bool isRemoteSession = false;

            // Get daemon configurations: are we in a remote session.
            TiXmlNode* pIsRemoteSessionPath = getProjectRootChildNode(docHandle, AF_STR_loadProjectIsRemoteSession);

            if (pIsRemoteSessionPath)
            {
                TiXmlText* pIsRemoteSesionText = pIsRemoteSessionPath->ToText();
                GT_IF_WITH_ASSERT(pIsRemoteSesionText)
                {
                    gtString isRemoteValue;
                    readXmlFieldValue(*pIsRemoteSesionText, isRemoteValue);
                    isRemoteSession = (0 == isRemoteValue.compareNoCase(L"yes"));
                }
            }

            // Check if this is a windows store app executable, or a application path
            bool isStoreApp = false;
            TiXmlNode* pIsStoreAppNode = getProjectRootChildNode(docHandle, AF_STR_loadProjectIsStoreApp);

            if (pIsStoreAppNode)
            {
                TiXmlText* pIsStoreAppNodeText = pIsStoreAppNode->ToText();
                GT_IF_WITH_ASSERT(pIsStoreAppNodeText)
                {
                    gtString isStoreAppValue;
                    readXmlFieldValue(*pIsStoreAppNodeText, isStoreAppValue);
                    isStoreApp = (0 == isStoreAppValue.compareNoCase(L"yes"));
                }
            }

            // Get the project executable path:
            TiXmlNode* pApplicationPath = getProjectRootChildNode(docHandle, AF_STR_loadProjectgApplicationPathNode);

            if (pApplicationPath)
            {
                TiXmlText* pApplicationPathText = pApplicationPath->ToText();

                if (pApplicationPathText)
                {
                    gtString application;
                    readXmlFieldValue(*pApplicationPathText, application);
                    osFilePath exePath(application);
                    static const gtString emptyStr;

                    // Do not require the executable to exist if:
                    // * This file represents a remote target
                    // * This machine does not support Windows Store apps (which can exist here)
                    //
                    // Note: If and when we decide to remote sessions on Windows store apps, this will have to be cleaned up
                    if (!isStoreApp)
                    {
                        m_projectSettings.setExecutablePath(exePath);
                        m_projectSettings.setWindowsStoreAppUserModelID(emptyStr);
                    }
                    else
                    {
                        m_projectSettings.setExecutablePath(emptyStr);
                        m_projectSettings.setWindowsStoreAppUserModelID(application);
                    }
                }
            }

            // Get daemon configurations: remote host ip, remote host port number.
            TiXmlNode* pIsRemoteHostNamePath = getProjectRootChildNode(docHandle, AF_STR_loadProjectRemoteDaemonHostName);
            TiXmlNode* pIsRemoteHostPortNumberPath = getProjectRootChildNode(docHandle, AF_STR_loadProjectRemoteDaemonPortNumber);
            GT_IF_WITH_ASSERT(pIsRemoteHostNamePath && pIsRemoteHostPortNumberPath)
            {
                TiXmlText* pIsRemoteHostNameText = pIsRemoteHostNamePath->ToText();
                TiXmlText* pIsRemoteHostPortNumberText = pIsRemoteHostPortNumberPath->ToText();
                GT_IF_WITH_ASSERT(pIsRemoteHostNameText && pIsRemoteHostPortNumberText)
                {
                    gtString remoteHostNameIp;
                    gtString remoteHostPortNumberStr;
                    readXmlFieldValue(*pIsRemoteHostNameText, remoteHostNameIp);
                    readXmlFieldValue(*pIsRemoteHostPortNumberText, remoteHostPortNumberStr);
                    int portNumber = 0;

                    if (!(remoteHostPortNumberStr.toIntNumber(portNumber) && portNumber > 0 &&  portNumber <= GT_INT16_MAX))
                    {
                        portNumber = 0;
                    }

                    if (isRemoteSession)
                    {
                        m_projectSettings.setRemoteDebugging(remoteHostNameIp, static_cast<gtUInt16>(portNumber));
                    }
                    else
                    {
                        // Indicate that we are in a local session.
                        m_projectSettings.setLocalDebugging();

                        // Remember the remote host IP and port.
                        m_projectSettings.SetRemoteTargetHostname(remoteHostNameIp);
                        m_projectSettings.setRemoteTargetDaemonPort(static_cast<gtUInt16>(portNumber));
                    }
                }
            }

            // Get the recently used host IPs:
            TiXmlNode* pRecentlyUsedHosts = getProjectRootChildNode(docHandle, AF_STR_loadProjectRecentlyUsedRemoteIPAddresses);

            if (pRecentlyUsedHosts)
            {
                TiXmlText* pRecentlyUsedHostsText = pRecentlyUsedHosts->ToText();

                if (pRecentlyUsedHostsText)
                {
                    gtString recentlyUsedHostsStr;
                    readXmlFieldValue(*pRecentlyUsedHostsText, recentlyUsedHostsStr);
                    m_projectSettings.setWorkDirectoryFromString(recentlyUsedHostsStr);
                }
            }

            // Get the application working directory:
            TiXmlNode* pApplicationWorkDir = getProjectRootChildNode(docHandle, AF_STR_loadProjectWorkDirNode);

            if (pApplicationWorkDir)
            {
                TiXmlText* pApplicationWorkDirText = pApplicationWorkDir->ToText();

                if (pApplicationWorkDirText)
                {
                    gtString appWorkingDirPath;
                    readXmlFieldValue(*pApplicationWorkDirText, appWorkingDirPath);
                    m_projectSettings.setWorkDirectoryFromString(appWorkingDirPath);
                }
            }

            // Get the source code root directory:
            gtString sourceCodeRootDir;
            TiXmlNode* pSourceCodeRootDir = getProjectRootChildNode(docHandle, AF_STR_loadProjectSourceCodeRootDirectoryNode);

            if (pSourceCodeRootDir != nullptr)
            {
                TiXmlText* pSourceCodeRootDirText = pSourceCodeRootDir->ToText();

                if (pSourceCodeRootDirText != nullptr)
                {
                    readXmlFieldValue(*pSourceCodeRootDirText, sourceCodeRootDir);
                }
            }

            // Set the source code root directory:
            m_projectSettings.SetSourceCodeRootLocation(sourceCodeRootDir);

            // Get the Debugged Application args
            TiXmlNode* pApplicationArgs = getProjectRootChildNode(docHandle, AF_STR_loadProjectAppArgsNode);

            if (pApplicationArgs)
            {
                TiXmlText* pApplicationArgsText = pApplicationArgs->ToText();

                if (pApplicationArgsText)
                {
                    gtString args;
                    readXmlFieldValue(*pApplicationArgsText, args);
                    m_projectSettings.setCommandLineArguments(args);
                }
            }

            // Get the file version node:
            TiXmlNode* pFileVersion = docHandle.FirstChild(AF_STR_loadProjectFileVersionNode).Node();
            gtString fileVer;

            if (pFileVersion != nullptr)
            {
                if (pFileVersion->FirstChild() != nullptr)
                {
                    std::string utf8Value(pFileVersion->FirstChild()->ToText()->Value());
                    fileVer.fromUtf8String(utf8Value);
                }
            }
        }
        else
        {
            // In VS get the project name from the cxlvs file name:
            osFilePath vsProjectFilePath(m_filePath);
            gtString projectName;
            vsProjectFilePath.getFileName(projectName);
            m_projectSettings.setProjectName(projectName);
        }

        // Get the source files directories:
        gtString sourceFilesDir;
        TiXmlNode* pSourceFilesDir = getProjectRootChildNode(docHandle, AF_STR_loadProjectSourceFilesDirectoryNode);

        if (pSourceFilesDir != nullptr)
        {
            TiXmlText* pSourceFilesDirText = pSourceFilesDir->ToText();

            if (pSourceFilesDirText != nullptr)
            {
                readXmlFieldValue(*pSourceFilesDirText, sourceFilesDir);
            }
        }

        if (sourceFilesDir.isEmpty())
        {
            // By default kernel are located in the application path:
            sourceFilesDir = m_projectSettings.workDirectory().asString();
        }

        // Set the source files directory:
        m_projectSettings.SetSourceFilesDirectories(sourceFilesDir);

        // Get the last active mode:
        TiXmlNode* pLastActiveMode = getProjectRootChildNode(docHandle, AF_STR_loadProjectLastActiveModeNode);

        if (pLastActiveMode)
        {
            TiXmlText* pLastActiveModeText = pLastActiveMode->ToText();

            if (pLastActiveModeText)
            {
                gtString lastActiveModeText;
                lastActiveModeText.fromASCIIString(pLastActiveModeText->Value());

                if (lastActiveModeText == OS_STR_EmptyXMLStringUnicode)
                {
                    lastActiveModeText.makeEmpty();
                }

                m_projectSettings.setLastActiveMode(lastActiveModeText);
            }
        }

        gtString lastActiveSessionTypeStr;
        TiXmlNode* pLastActiveSessionType = getProjectRootChildNode(docHandle, AF_STR_loadProjectLastSessionTypeNode);

        if (pLastActiveSessionType)
        {
            TiXmlText* pLastActiveSessionTypeText = pLastActiveSessionType->ToText();

            if (pLastActiveSessionTypeText)
            {
                QString lastActiveSessionTypeText = pLastActiveSessionTypeText->Value();

                if (lastActiveSessionTypeText != OS_STR_EmptyXMLString)
                {
                    lastActiveSessionTypeStr = acQStringToGTString(lastActiveSessionTypeText);
                }

                m_projectSettings.setLastActiveSessionType(lastActiveSessionTypeStr);
            }
        }

        // Read the project environment variable:
        retVal = retVal && ReadProjectEnvironmentVariable(docHandle);

        // Read the extensions from the XML:
        retVal = retVal && ReadProjectExtenstionsData(docHandle);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afLoadProjectCommand::ReadProjectExtenstionsData
// Description: Read the data for each extension from the XML
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        8/4/2012
// ---------------------------------------------------------------------------
bool afLoadProjectCommand::ReadProjectExtenstionsData(TiXmlHandle& docHandle)
{
    bool retVal = false;

    // Get the extension data node:
    TiXmlNode* pExtensionsNode = getProjectRootChildNode(docHandle, AF_STR_loadProjectProductExtensionsNode, "", false);

    GT_IF_WITH_ASSERT(pExtensionsNode != nullptr)
    {
        retVal = true;

        // Walk on all the function breakpoints nodes
        TiXmlNode* pNode = nullptr;

        for (pNode = pExtensionsNode->FirstChild(); pNode != nullptr; pNode = pNode->NextSibling())
        {
            if (pNode != nullptr)
            {
                // Get the extension name:
                gtString extensionName;
                extensionName.fromASCIIString(pNode->Value());
                GT_IF_WITH_ASSERT(!extensionName.isEmpty())
                {
                    // Ignore tokens for the empty project:
                    static const gtString emptyToken = OS_STR_EmptyXMLStringUnicode;

                    if (emptyToken != extensionName)
                    {
                        // Get the XML as text:
                        TiXmlPrinter printer;
                        printer.SetIndent("    ");
                        pNode->Accept(&printer);

                        gtString xmlContent;
                        std::string utf8Value(printer.CStr());
                        xmlContent.fromUtf8String(utf8Value);

                        // Add the extension XML string to the map:
                        m_extensionXMLStringsMap[extensionName] = xmlContent;

                        // Make sure that the XML string is not empty:
                        retVal = retVal && !xmlContent.isEmpty();
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afLoadProjectCommand::setExtensionXMLData
// Description: Set the extension XML data
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        10/4/2012
// ---------------------------------------------------------------------------
bool afLoadProjectCommand::setExtensionXMLData()
{
    bool retVal = true;

    // Iterate the map of XML string and set each extension data:
    gtMap<gtString, gtString>::const_iterator iter = m_extensionXMLStringsMap.begin();
    gtMap<gtString, gtString>::const_iterator iterEnd = m_extensionXMLStringsMap.end();

    for (/* iter */; iter != iterEnd; iter++)
    {
        // Set the current extension string from the XML:
        bool wasExtesnionFound;
        bool rc = afProjectManager::instance().setCurrentProjectDataFromXMLString((*iter).first, (*iter).second, wasExtesnionFound);

        if (!rc)
        {
            // If the extension was not found, do not fail the project load:
            if (!wasExtesnionFound)
            {
                gtString msg;
                msg.appendFormattedString(L"Could not load project settings for %ls plug-in. The plug-in is not loaded.", (*iter).first.asCharArray());
                GT_ASSERT_EX(false, msg.asCharArray());
            }
            else
            {
                retVal = false;
            }
        }

    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afLoadProjectCommand::getProjectChildNode
// Description: This function makes sure that the old project root name is supported
// Return Val:  TiXmlNode*
// Author:      Sigal Algranaty
// Date:        19/7/2012
// ---------------------------------------------------------------------------
TiXmlNode* afLoadProjectCommand::getProjectRootChildNode(TiXmlHandle& docHandle, const QString& nodeName, const QString& childNodeName, bool getFirstChild)
{
    TiXmlNode* pRetVal = nullptr;

    if (getFirstChild)
    {
        // Get the new project name node:
        if (childNodeName.isEmpty())
        {
            pRetVal = docHandle.FirstChild(afGlobalVariablesManager::ProductNameCharArray()).FirstChild(nodeName.toUtf8().data()).FirstChild().Node();
        }
        else
        {
            pRetVal = docHandle.FirstChild(afGlobalVariablesManager::ProductNameCharArray()).FirstChild(nodeName.toUtf8().data()).FirstChild(childNodeName.toUtf8().data()).Node();
        }

        if (pRetVal == nullptr)
        {
            pRetVal = docHandle.FirstChild(AF_STR_loadProjectRootNodeBackwardsCompatibility).FirstChild(nodeName.toUtf8().data()).FirstChild().Node();
        }
    }
    else
    {
        // Get the new project name node:
        pRetVal = docHandle.FirstChild(afGlobalVariablesManager::ProductNameCharArray()).FirstChild(nodeName.toUtf8().data()).Node();

        if (pRetVal == nullptr)
        {
            pRetVal = docHandle.FirstChild(AF_STR_loadProjectRootNodeBackwardsCompatibility).FirstChild(nodeName.toUtf8().data()).Node();
        }
    }

    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        afLoadProjectCommand::ReadProjectEnvironmentVariable
// Description: Read the project environment variables
// Arguments:   TiXmlHandle& docHandle
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        23/7/2012
// ---------------------------------------------------------------------------
bool afLoadProjectCommand::ReadProjectEnvironmentVariable(TiXmlHandle& docHandle)
{
    bool retVal = true;

    // If this is a new file, treat the environment variables as nodes
    TiXmlNode* pNode = getProjectRootChildNode(docHandle, AF_STR_loadProjectAppEnvVarsNode, AF_STR_loadProjectAppEnvVarsVarNode);

    TiXmlNode* pDummyNode = nullptr;
    TiXmlText* currName;
    TiXmlText* currValue;
    osEnvironmentVariable currEnvVar;
    QString nodeValueAsASCIIString;

    for (; pNode != nullptr; pNode = pNode->NextSibling())
    {
        currEnvVar._value.makeEmpty();
        currEnvVar._name.makeEmpty();

        pDummyNode = pNode->FirstChild(AF_STR_loadProjectAppEnvVarsNameNode)->FirstChild();
        currName = pDummyNode->ToText();

        if (currName != nullptr)
        {
            nodeValueAsASCIIString = currName->Value();

            if (nodeValueAsASCIIString == OS_STR_EmptyXMLString)
            {
                nodeValueAsASCIIString.clear();
            }

            currEnvVar._name = acQStringToGTString(nodeValueAsASCIIString);
        }

        pDummyNode = pNode->FirstChild(AF_STR_loadProjectAppEnvVarsValueNode)->FirstChild();

        if (pDummyNode != nullptr)
        {
            currValue = pDummyNode->ToText();

            if (currValue != nullptr)
            {
                nodeValueAsASCIIString = currValue->Value();

                if (nodeValueAsASCIIString == OS_STR_EmptyXMLString)
                {
                    nodeValueAsASCIIString.clear();
                }

                currEnvVar._value = acQStringToGTString(nodeValueAsASCIIString);
            }
        }

        apHandleXMLEscaping(currEnvVar._name, false);
        apHandleXMLEscaping(currEnvVar._value, false);

        m_projectSettings.addEnvironmentVariable(currEnvVar);

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afLoadProjectCommand::readXmlFieldValue
// Description: Read a string value from an XML node
// Arguments:   TiXmlText & xmlText [INPUT]
//              gtString& fieldValue [OUTPUT]
// Return Val:  na
// Author:      Doron Ofek
// Date:        Oct-23, 2012
// ---------------------------------------------------------------------------
void afLoadProjectCommand::readXmlFieldValue(TiXmlText& xmlText, gtString& fieldValue)
{
    // Decode UTF8 values
    std::string utf8Value(xmlText.Value());

    if (utf8Value == OS_STR_EmptyXMLString)
    {
        fieldValue.makeEmpty();
    }
    else
    {
        fieldValue.fromUtf8String(utf8Value);
    }
}

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS

// The way it is checked if the library exists is by using the "ldconfig -p" function and checking if the return pipe is empty.
// if it is not empty we assume the library is installed is some format (dev or none dev)
bool afLoadProjectCommand::IsTeapotNeededLibraryInstalled()
{
    bool retVal = true;

    FILE* pPipe = popen("ldconfig -p | grep gtkglext", "r");

    if (pPipe != nullptr)
    {
        int bufferSize = 0;
        char* pBuffer = new char[1024];
        int c;

        // read the pipe
        while ((c = fgetc(pPipe)) != EOF && bufferSize < 1024)
        {
            pBuffer[bufferSize++] = (char) c;
        }

        if (0 == bufferSize)
        {
            retVal = false;
        }

        delete [] pBuffer;
    }

    return retVal;
}
#endif
