//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afGlobalVariablesManager.cpp
///
//==================================================================================

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSAPIWrappers/Include/oaDriver.h>
#include <AMDTAPIClasses/Include/apCLDevice.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTApplicationComponents/Include/acItemDelegate.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTAPIClasses/Include/apDebugProjectSettings.h>

// Local:
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/commands/afSystemInformationCommand.h>
#include <AMDTApplicationFramework/src/afUtils.h>

afGlobalVariablesManager* afGlobalVariablesManager::m_pMySingleInstance = nullptr;
gtString  afGlobalVariablesManager::m_sProductName = L"";
QString  afGlobalVariablesManager::m_sProductNameA = "";
char* afGlobalVariablesManager::m_sProductNameCharArray = nullptr;
acIconId afGlobalVariablesManager::m_sProductIconId = AC_ICON_EMPTY;
// ---------------------------------------------------------------------------
// Name:        afGlobalVariablesManager::afGlobalVariablesManager
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        19/4/2012
// ---------------------------------------------------------------------------
afGlobalVariablesManager::afGlobalVariablesManager(const osFilePath& settingsFilePath)
    : osSettingsFileHandler(settingsFilePath), _isUsingProxy(false), _proxyServer("", 0),
      _eulaAcceptedRevisionNumber(-1), m_spyPort(AP_REMOTE_TARGET_CONNECTION_DEFAULT_SPY_API_PORT), m_spyEventsPort(AP_REMOTE_TARGET_CONNECTION_DEFAULT_SPY_EVENTS_PORT),
      m_rdsPort(AP_REMOTE_TARGET_CONNECTION_DEFAULT_CONNECTION_PORT), m_rdsEventsPort(AP_REMOTE_TARGET_CONNECTION_DEFAULT_EVENTS_PORT),
      m_fpPrecision(8), m_percentagePrecision(2), m_shouldAlertMissingSourceFile(false), m_versionCaption(L""), m_installedAMDComponentsBitmask(0), m_missingInstalledAMDComponentsMessage(L"")
{
    // Set the temp folder as default log files folder:
    m_logFilesDirectoryPath.setPath(osFilePath::OS_TEMP_DIRECTORY);

    // Register myself to listen to debugged process events:
    apEventsHandler::instance().registerEventsObserver(*this, AP_GLOBAL_VARIABLES_MANAGER_EVENTS_HANDLING_PRIORITY);

    // Construct the version caption:
#if AMDT_BUILD_ACCESS == AMDT_PUBLIC_ACCESS
    // No string for public version
#elif AMDT_BUILD_ACCESS == AMDT_NDA_ACCESS
    m_versionCaption = AF_STR_NDAVersion;
#elif AMDT_BUILD_ACCESS == AMDT_INTERNAL_ACCESS
    m_versionCaption = AF_STR_InternalVersion;
#else
#error Unknown build access
#endif

    // Append "Debug" when in debug mode:
#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD

    if (!m_versionCaption.isEmpty())
    {
        m_versionCaption.append('-');
    }

    m_versionCaption.append(AF_STR_Debug);
#endif

    // Get the installed AMD components:
    FindInstalledAMDComponents();
}


// ---------------------------------------------------------------------------
// Name:        afGlobalVariablesManager::~afGlobalVariablesManager
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        19/4/2012
// ---------------------------------------------------------------------------
afGlobalVariablesManager::~afGlobalVariablesManager()
{
    // Unregister myself from listening to debugged process events:
    apEventsHandler::instance().unregisterEventsObserver(*this);

    // There is an issue with deleting the settings pages, since they are also Qt windows.
    // Instead, just clear the vector:
    m_globalSettingsPages.clear();
}

// ---------------------------------------------------------------------------
// Name:        gdGDebuggerGlobalVariablesManager::instance
// Description: Returns the single instance of this class.
//              (If it does not exist - create it)
// Author:      Sigal Algranaty
// Date:        19/4/2012
// ---------------------------------------------------------------------------
afGlobalVariablesManager& afGlobalVariablesManager::instance()
{
    if (m_pMySingleInstance == nullptr)
    {
        osFilePath globalSettingsFilePath;
        afGetUserDataFolderPath(globalSettingsFilePath);
        globalSettingsFilePath.setFileName(AF_STR_globalSettingsFileName);
        globalSettingsFilePath.setFileExtension(AF_STR_globalSettingsFileExtension);

        m_pMySingleInstance = new afGlobalVariablesManager(globalSettingsFilePath);
        GT_ASSERT(m_pMySingleInstance);
    }

    return *m_pMySingleInstance;
}


// ---------------------------------------------------------------------------
// Name:        afGlobalVariablesManager::onEvent
// Description:
// Arguments:    const apEvent& eve
//              bool& vetoEvent
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        22/4/2012
// ---------------------------------------------------------------------------
void afGlobalVariablesManager::onEvent(const apEvent& eve, bool& vetoEvent)
{
    GT_UNREFERENCED_PARAMETER(eve);
    GT_UNREFERENCED_PARAMETER(vetoEvent);
    // Currently do nothing:
}

// ---------------------------------------------------------------------------
// Name:        afGlobalVariablesManager::setEULRevisionNumber
// Description: Updates the revision number in case of a newer version, else do nothing.
// Arguments:   int newRevisionNumber
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        22/4/2012
// ---------------------------------------------------------------------------
void afGlobalVariablesManager::setEULRevisionNumber(int newRevisionNumber)
{
    if (newRevisionNumber > _eulaAcceptedRevisionNumber)
    {
        _eulaAcceptedRevisionNumber = newRevisionNumber;
    }
}


// ---------------------------------------------------------------------------
// Name:        afGlobalVariablesManager::setNotToShowAgain
// Description: set the dialog not be shown again
// Author:      Gilad Yarnitzky
// Date:        31/12/2012
// ---------------------------------------------------------------------------
void afGlobalVariablesManager::setNotToShowAgain(QString& title)
{
    // Make sure the dialog is not already in the list:
    if (!isSetNotToShowAgain(title))
    {
        // Add it to the list of dialogs not to show:
        m_doNotShowAgainDialogs.push_back(title);
    }
}


// ---------------------------------------------------------------------------
// Name:        afGlobalVariablesManager::isSetNotToShowAgain
// Description: ask if a dialog was marked not to show again
// Author:      Gilad Yarnitzky
// Date:        31/12/2012
// ---------------------------------------------------------------------------
bool afGlobalVariablesManager::isSetNotToShowAgain(QString& title)
{
    bool retVal = false;

    // Look in the list for the dialog title:
    int numItems = m_doNotShowAgainDialogs.size();

    for (int nString = 0 ; nString < numItems ; nString++)
    {
        if (m_doNotShowAgainDialogs.at(nString) == title)
        {
            retVal = true;
            break;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afGlobalVariablesManager::registerGlobalSettingsPage
// Description: Adds a global settings page
// Author:      Uri Shomroni
// Date:        22/4/2012
// ---------------------------------------------------------------------------
void afGlobalVariablesManager::registerGlobalSettingsPage(afGlobalSettingsPage* pGlobalSettingsPage)
{
    GT_IF_WITH_ASSERT(nullptr != pGlobalSettingsPage)
    {
        m_globalSettingsPages.push_back(pGlobalSettingsPage);
        pGlobalSettingsPage->initialize();

        if (!_settingsFileAsXMLDocument.Error())
        {
            // If the global settings are already initialized, load them into this page:
            loadGlobalSettingsIntoPage(pGlobalSettingsPage);

            // Save the page settings:
            pGlobalSettingsPage->saveCurrentSettings();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afGlobalVariablesManager::getGlobalSettingPage
// Description: Gets the global settings page
// Author:      Uri Shomroni
// Date:        22/4/2012
// ---------------------------------------------------------------------------
afGlobalSettingsPage* afGlobalVariablesManager::getGlobalSettingPage(int pageIndex, gtString* pageTitle)
{
    afGlobalSettingsPage* retVal = nullptr;

    if ((0 <= pageIndex) && ((int)m_globalSettingsPages.size() > pageIndex))
    {
        retVal = m_globalSettingsPages[pageIndex];
        GT_IF_WITH_ASSERT(nullptr != retVal)
        {
            if (nullptr != pageTitle)
            {
                *pageTitle = retVal->pageTitle();
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afGlobalVariablesManager::initXMLDocToSettingsFileDefaultValues
// Description: Initializes the XML document to the defualt settings
// Author:      Uri Shomroni
// Date:        9/5/2012
// ---------------------------------------------------------------------------
bool afGlobalVariablesManager::initXMLDocToSettingsFileDefaultValues(TiXmlDocument& XMLDocument)
{
    bool retVal = true;

#define AF_Str_GlobalSettingsStr         "<%1>"\
    "<" AF_globalSettingsGeneralHeader ">"\
    "<" AF_globalSettingsGeneralLogFilesNode ">" OS_STR_EmptyXMLString "</" AF_globalSettingsGeneralLogFilesNode ">"\
    "<" AF_globalSettingsGeneralLogLevelNode ">INFO</" AF_globalSettingsGeneralLogLevelNode ">"\
    "<" AF_globalSettingsProxyEnabledNode ">" OS_STR_FalseXMLValue "</" AF_globalSettingsProxyEnabledNode ">"\
    "</" AF_globalSettingsGeneralHeader ">"\
    "</%2>"

    QString str = QString(AF_Str_GlobalSettingsStr).arg(afGlobalVariablesManager::ProductNameCharArray()).arg(afGlobalVariablesManager::ProductNameCharArray());
    XMLDocument.Parse(str.toLocal8Bit().data());

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afGlobalVariablesManager::loadGlobalSettingsFromXMLFile
// Description: Loads the global settings from the XML file
// Author:      Uri Shomroni
// Date:        9/5/2012
// ---------------------------------------------------------------------------
void afGlobalVariablesManager::loadGlobalSettingsFromXMLFile()
{
    int numberOfSettingsPages = (int)m_globalSettingsPages.size();

    if (_settingsFilePath.isRegularFile() && loadSettingsFile())
    {
        // Load the general settings:
        loadGeneralSettingsFromLoadedXML();

        // Now that the settings are read, apply them to the pages:
        // Restore the default settings for all pages:
        for (int i = 0; i < numberOfSettingsPages; i++)
        {
            afGlobalSettingsPage* pCurrentPage = m_globalSettingsPages[i];
            loadGlobalSettingsIntoPage(pCurrentPage);
        }
    }
    else
    {
        // Restore the default settings for all pages:
        for (int i = 0; i < numberOfSettingsPages; i++)
        {
            afGlobalSettingsPage* pCurrentPage = m_globalSettingsPages[i];
            GT_IF_WITH_ASSERT(nullptr != pCurrentPage)
            {
                pCurrentPage->restoreDefaultSettings();
            }
        }
    }

    // Apply the loaded / default settings:
    for (int i = 0; i < numberOfSettingsPages; i++)
    {
        afGlobalSettingsPage* pCurrentPage = m_globalSettingsPages[i];
        GT_IF_WITH_ASSERT(nullptr != pCurrentPage)
        {
            pCurrentPage->saveCurrentSettings();
        }
    }

    // Read the versions XML:
    bool rc = readCodeXLVersionXML();
    GT_ASSERT(rc);
}

// ---------------------------------------------------------------------------
// Name:        afGlobalVariablesManager::loadGlobalSettingsIntoPage
// Description: Loads the opened settings page into the pointed extension settings page
// Author:      Uri Shomroni
// Date:        9/5/2012
// ---------------------------------------------------------------------------
void afGlobalVariablesManager::loadGlobalSettingsIntoPage(afGlobalSettingsPage* pGlobalSettingsPage)
{
    GT_IF_WITH_ASSERT(nullptr != pGlobalSettingsPage)
    {
        TiXmlHandle globalSettingsXMLDocHandle(&_settingsFileAsXMLDocument);
        gtString currentPageXMLHeader = pGlobalSettingsPage->xmlSectionTitle();
        TiXmlNode* pCurrentNode = globalSettingsXMLDocHandle.FirstChild(afGlobalVariablesManager::ProductNameCharArray()).FirstChild(currentPageXMLHeader.asASCIICharArray()).ToNode();
        bool successfullyRead = false;

        if (nullptr != pCurrentNode)
        {
            // Get the XML as text:
            TiXmlPrinter printer;
            printer.SetIndent("    ");
            pCurrentNode->Accept(&printer);
            gtString xmlContent = acQStringToGTString(printer.CStr());

            if (!xmlContent.isEmpty())
            {
                successfullyRead = pGlobalSettingsPage->setSettingsFromXMLString(xmlContent);
            }
        }

        if (!successfullyRead)
        {
            // This section is missing, load the defaults:
            pGlobalSettingsPage->restoreDefaultSettings();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afGlobalVariablesManager::saveGlobalSettingsToXMLFile
// Description: Saves the global settings to the xml file
// Author:      Uri Shomroni
// Date:        9/5/2012
// ---------------------------------------------------------------------------
void afGlobalVariablesManager::saveGlobalSettingsToXMLFile()
{
    _settingsFileAsXMLDocument.Clear();
    TiXmlHandle globalSettingsXMLDocHandle(&_settingsFileAsXMLDocument);
    TiXmlNode* pGlobalSettingsNode = globalSettingsXMLDocHandle.FirstChild(afGlobalVariablesManager::ProductNameCharArray()).ToNode();

    if (nullptr == pGlobalSettingsNode)
    {
        pGlobalSettingsNode = globalSettingsXMLDocHandle.ToNode()->InsertEndChild(TiXmlElement(afGlobalVariablesManager::ProductNameCharArray()));
    }

    // Save the general settings:
    saveGeneralSettingToLoadedXML();

    int numberOfSettingPages = (int)m_globalSettingsPages.size();

    for (int i = 0; i < numberOfSettingPages; i++)
    {
        afGlobalSettingsPage* pCurrentPage = m_globalSettingsPages[i];
        GT_IF_WITH_ASSERT(nullptr != pCurrentPage)
        {
            gtString currentPageData;
            bool rcXML = pCurrentPage->getXMLSettingsString(currentPageData);
            GT_IF_WITH_ASSERT(rcXML)
            {
                gtString currentPageXmlSectionTitle = pCurrentPage->xmlSectionTitle();
                TiXmlNode* pCurrentPageSection = pGlobalSettingsNode->FirstChild(currentPageXmlSectionTitle.asASCIICharArray());

                if (nullptr == pCurrentPageSection)
                {
                    pCurrentPageSection = pGlobalSettingsNode->InsertEndChild(TiXmlElement(currentPageXmlSectionTitle.asASCIICharArray()));
                }

                // TinyXML does not support wide strings but it does supports UTF8 so we convert the strings to UTF8
                std::string utf8PageData;
                currentPageData.asUtf8(utf8PageData);
                pCurrentPageSection->Parse(utf8PageData.c_str(), 0 , TIXML_ENCODING_UTF8);
            }
        }
    }

    // Save the settings file:
    saveSettingsFile();
}

// ---------------------------------------------------------------------------
// Name:        afGlobalVariablesManager::loadGeneralSettingsFromLoadedXML
// Description: Loads the general settings from the loaded _settingsFileAsXMLDocument member
// Author:      Uri Shomroni
// Date:        9/5/2012
// ---------------------------------------------------------------------------
void afGlobalVariablesManager::loadGeneralSettingsFromLoadedXML()
{
    TiXmlHandle globalSettingsXMLDocHandle(&_settingsFileAsXMLDocument);
    TiXmlNode* pGeneralNode = globalSettingsXMLDocHandle.FirstChild(afGlobalVariablesManager::ProductNameCharArray()).FirstChild(AF_globalSettingsGeneralHeader).ToNode();
    GT_IF_WITH_ASSERT(nullptr != pGeneralNode)
    {
        // Get the log files directory:
        TiXmlNode* pLogFilesNode = pGeneralNode->FirstChild(AF_globalSettingsGeneralLogFilesNode);
        GT_IF_WITH_ASSERT(nullptr != pLogFilesNode)
        {
            gtString logFilesPath;
            readStringFromXmlNode(pLogFilesNode->FirstChild(), logFilesPath);
            GT_IF_WITH_ASSERT(!logFilesPath.isEmpty())
            {
                osFilePath logOsFilePath;
                logOsFilePath.setFileDirectory(logFilesPath);
                setLogFilesDirectoryPath(logOsFilePath);
                // Initialize the log file:
                osDebugLog& theDebugLog = osDebugLog::instance();
                bool retVal = osDebugLog::instance().initialize(ProductName(), theDebugLog.productDescriptionString().asCharArray(), theDebugLog.osDescriptionString().asCharArray(), logFilesDirectoryPath());
                GT_ASSERT(retVal);
            }
        }

        // Get the log level:
        TiXmlNode* pLogLevelNode = pGeneralNode->FirstChild(AF_globalSettingsGeneralLogLevelNode);
        GT_IF_WITH_ASSERT(nullptr != pLogLevelNode)
        {
            gtString logLevelAsString;
            readStringFromXmlNode(pLogLevelNode->FirstChild(), logLevelAsString);
            GT_IF_WITH_ASSERT(!logLevelAsString.isEmpty())
            {
                osDebugLogSeverity logSeverity = osStringToDebugLogSeverity(logLevelAsString.asCharArray());
                osDebugLog::instance().setLoggedSeverity(logSeverity);
            }
        }

        // Get the proxy settings:
        bool isProxySet = false;
        osPortAddress proxyAddress;
        TiXmlNode* pIsProxySetNode = pGeneralNode->FirstChild(AF_globalSettingsProxyEnabledNode);
        GT_IF_WITH_ASSERT(nullptr != pIsProxySetNode)
        {
            readBoolFromXMLNode(pIsProxySetNode->FirstChild(), isProxySet);
        }

        if (isProxySet)
        {
            TiXmlNode* pProxyAddressNode = pGeneralNode->FirstChild(AF_globalSettingsProxyAddressNode);
            GT_IF_WITH_ASSERT(nullptr != pProxyAddressNode)
            {
                gtString proxyAddressAsString;
                readStringFromXmlNode(pProxyAddressNode->FirstChild(), proxyAddressAsString);
                proxyAddress.fromString(proxyAddressAsString);
            }
        }

        setProxyInformation(isProxySet, proxyAddress);

        // Get the remote debugging ports.
        TiXmlNode* pRdsApiPortNode = pGeneralNode->FirstChild(AF_globalSettingsRdsApiPortNode);
        GT_IF_WITH_ASSERT(nullptr != pRdsApiPortNode)
        {
            gtString rdsApiPort;
            readStringFromXmlNode(pRdsApiPortNode->FirstChild(), rdsApiPort);
            unsigned int val = 0;

            if (rdsApiPort.toUnsignedIntNumber(val) && val != 0 && val > 0)
            {
                m_rdsPort = static_cast<unsigned short>(val);
            }
        }
        TiXmlNode* pRdsEventsPortNode = pGeneralNode->FirstChild(AF_globalSettingsRdsEventsPortNode);
        GT_IF_WITH_ASSERT(nullptr != pRdsEventsPortNode)
        {
            gtString rdsEventsPort;
            readStringFromXmlNode(pRdsEventsPortNode->FirstChild(), rdsEventsPort);
            unsigned int val = 0;

            if (rdsEventsPort.toUnsignedIntNumber(val) && val != 0 && val > 0)
            {
                m_rdsEventsPort = static_cast<unsigned short>(val);
            }
        }
        TiXmlNode* pSpyApiPortNode = pGeneralNode->FirstChild(AF_globalSettingsSpyApiPortNode);
        GT_IF_WITH_ASSERT(nullptr != pSpyApiPortNode)
        {
            gtString spyApiPort;
            readStringFromXmlNode(pSpyApiPortNode->FirstChild(), spyApiPort);
            unsigned int val = 0;

            if (spyApiPort.toUnsignedIntNumber(val) && val != 0 && val > 0)
            {
                m_spyPort = static_cast<unsigned short>(val);
            }
        }
        TiXmlNode* pSpyEventsPortNode = pGeneralNode->FirstChild(AF_globalSettingsSpyEventsPortNode);
        GT_IF_WITH_ASSERT(nullptr != pSpyEventsPortNode)
        {
            gtString spyEventsPort;
            readStringFromXmlNode(pSpyEventsPortNode->FirstChild(), spyEventsPort);
            unsigned int val = 0;

            if (spyEventsPort.toUnsignedIntNumber(val) && val != 0 && val > 0)
            {
                m_spyEventsPort = static_cast<unsigned short>(val);
            }
        }

        // Get source path browser directory
        gtString pathAsString;
        TiXmlNode* pPathNode = pGeneralNode->FirstChild(AF_globalSettingsEulaVersionAcceptedNode);
        GT_IF_WITH_ASSERT(nullptr != pPathNode)
        {
            gtString eulaVersionAsStr;
            readStringFromXmlNode(pPathNode->FirstChild(), eulaVersionAsStr);
            int eulaVersion;
            bool rc = eulaVersionAsStr.toIntNumber(eulaVersion);
            GT_IF_WITH_ASSERT(rc)
            {
                setEULRevisionNumber(eulaVersion);
            }
        }

        // Get the floating point precision:
        pPathNode = pGeneralNode->FirstChild(AF_globalSettingsFloatingPointPrecisionNode);
        GT_IF_WITH_ASSERT(nullptr != pPathNode)
        {
            gtString fpStr;
            readStringFromXmlNode(pPathNode->FirstChild(), fpStr);
            int fpPrecision = 8;
            bool rc = fpStr.toIntNumber(fpPrecision);
            GT_IF_WITH_ASSERT(rc)
            {
                m_fpPrecision = fpPrecision;

                // Set the precision for the item responsible for numbering format:
                acNumberDelegateItem::SetFloatingPointPercision(m_fpPrecision);
            }
        }

        // Get the floating point precision:
        pPathNode = pGeneralNode->FirstChild(AF_globalSettingsAlertMissingSourceNode);
        GT_IF_WITH_ASSERT(nullptr != pPathNode)
        {
            gtString alertStr;
            readStringFromXmlNode(pPathNode->FirstChild(), alertStr);
            int test = 0;
            bool rc = alertStr.toIntNumber(test);
            GT_IF_WITH_ASSERT(rc)
            {
                m_shouldAlertMissingSourceFile = (test == 1);
            }
        }

        // Load dialogs not to show:
        pPathNode = pGeneralNode->FirstChild(AF_globalSettingsDialogsNotToShow);
        GT_IF_WITH_ASSERT(nullptr != pPathNode)
        {
            TiXmlNode* pPathDialog = pPathNode->FirstChild(AF_globalSettingsDialogsName);

            while (pPathDialog != nullptr)
            {
                gtString dialogName;
                readStringFromXmlNode(pPathDialog->FirstChild(), dialogName);
                m_doNotShowAgainDialogs.push_back(QString::fromStdWString(dialogName.asCharArray()));
                pPathDialog = pPathDialog->NextSibling();
            }
        }

        // Read the widgets history list from XML:
        ReadHistoryListFromXML(pGeneralNode);

    }
}

// ---------------------------------------------------------------------------
// Name:        afGlobalVariablesManager::loadGeneralSettingsFromLoadedXML
// Description: Saves the general settings to the loaded _settingsFileAsXMLDocument member
// Author:      Uri Shomroni
// Date:        9/5/2012
// ---------------------------------------------------------------------------
void afGlobalVariablesManager::saveGeneralSettingToLoadedXML()
{
    TiXmlHandle globalSettingsXMLDocHandle(&_settingsFileAsXMLDocument);
    TiXmlNode* pGlobalSettingsNode = globalSettingsXMLDocHandle.FirstChild(afGlobalVariablesManager::ProductNameCharArray()).ToNode();
    TiXmlNode* pGeneralNode = pGlobalSettingsNode->FirstChild(AF_globalSettingsGeneralHeader);

    if (nullptr == pGeneralNode)
    {
        pGeneralNode = pGlobalSettingsNode->InsertEndChild(TiXmlElement(AF_globalSettingsGeneralHeader));
    }

    GT_IF_WITH_ASSERT(nullptr != pGeneralNode)
    {
        // Get the log files directory:
        TiXmlNode* pLogFilesNode = pGeneralNode->FirstChild(AF_globalSettingsGeneralLogFilesNode);

        if (nullptr == pLogFilesNode)
        {
            pLogFilesNode = pGeneralNode->InsertEndChild(TiXmlElement(AF_globalSettingsGeneralLogFilesNode));
        }

        GT_IF_WITH_ASSERT(nullptr != pLogFilesNode)
        {
            TiXmlNode* pLogFilesTextNode = pLogFilesNode->FirstChild();

            if (nullptr == pLogFilesTextNode)
            {
                pLogFilesTextNode = pLogFilesNode->InsertEndChild(TiXmlText(OS_STR_EmptyXMLString));
            }

            gtString logFilesPath = logFilesDirectoryPath().asString();
            bool rcWr = writeStringToXmlNode(pLogFilesTextNode, logFilesPath);
            GT_ASSERT(rcWr);
        }

        // Get the log level:
        TiXmlNode* pLogLevelNode = pGeneralNode->FirstChild(AF_globalSettingsGeneralLogLevelNode);

        if (nullptr == pLogLevelNode)
        {
            pLogLevelNode = pGeneralNode->InsertEndChild(TiXmlElement(AF_globalSettingsGeneralLogLevelNode));
        }

        GT_IF_WITH_ASSERT(nullptr != pLogLevelNode)
        {
            TiXmlNode* pLogLevelTextNode = pLogLevelNode->FirstChild();

            if (nullptr == pLogLevelTextNode)
            {
                pLogLevelTextNode = pLogLevelNode->InsertEndChild(TiXmlText(OS_STR_EmptyXMLString));
            }

            gtString logLevelAsString = osDebugLogSeverityToString(osDebugLog::instance().loggedSeverity());
            bool rcWr = writeStringToXmlNode(pLogLevelTextNode, logLevelAsString);
            GT_ASSERT(rcWr);
        }

        // Get the proxy settings:
        bool isProxySet = false;
        osPortAddress proxyAddress;
        getProxyInformation(isProxySet, proxyAddress);
        TiXmlNode* pIsProxySetNode = pGeneralNode->FirstChild(AF_globalSettingsProxyEnabledNode);

        if (nullptr == pIsProxySetNode)
        {
            pIsProxySetNode = pGeneralNode->InsertEndChild(TiXmlElement(AF_globalSettingsProxyEnabledNode));
        }

        GT_IF_WITH_ASSERT(nullptr != pIsProxySetNode)
        {
            TiXmlNode* pIsProxySetTextNode = pIsProxySetNode->FirstChild();

            if (nullptr == pIsProxySetTextNode)
            {
                pIsProxySetTextNode = pIsProxySetNode->InsertEndChild(TiXmlText(OS_STR_EmptyXMLString));
            }

            writeBoolToXmlNode(pIsProxySetTextNode, isProxySet);
        }

        if (isProxySet)
        {
            TiXmlNode* pProxyAddressNode = pGeneralNode->FirstChild(AF_globalSettingsProxyAddressNode);

            if (nullptr == pProxyAddressNode)
            {
                pProxyAddressNode = pGeneralNode->InsertEndChild(TiXmlElement(AF_globalSettingsProxyAddressNode));
            }

            GT_IF_WITH_ASSERT(nullptr != pProxyAddressNode)
            {
                TiXmlNode* pProxyAddressTextNode = pProxyAddressNode->FirstChild();

                if (nullptr == pProxyAddressTextNode)
                {
                    pProxyAddressTextNode = pProxyAddressNode->InsertEndChild(TiXmlText(OS_STR_EmptyXMLString));
                }

                gtString proxyAddressAsString;
                proxyAddress.toString(proxyAddressAsString);
                writeStringToXmlNode(pProxyAddressTextNode, proxyAddressAsString);
            }
        }

        // Get (or create) the remote debugging ports:
        TiXmlNode* pRdsApiPort = pGeneralNode->FirstChild(AF_globalSettingsRdsApiPortNode);

        if (nullptr == pRdsApiPort)
        {
            pRdsApiPort = pGeneralNode->InsertEndChild(TiXmlElement(AF_globalSettingsRdsApiPortNode));
        }

        TiXmlNode* pRdsEventsPort = pGeneralNode->FirstChild(AF_globalSettingsRdsEventsPortNode);

        if (nullptr == pRdsEventsPort)
        {
            pRdsEventsPort = pGeneralNode->InsertEndChild(TiXmlElement(AF_globalSettingsRdsEventsPortNode));
        }

        TiXmlNode* pSpyApiPort = pGeneralNode->FirstChild(AF_globalSettingsSpyApiPortNode);

        if (nullptr == pSpyApiPort)
        {
            pSpyApiPort = pGeneralNode->InsertEndChild(TiXmlElement(AF_globalSettingsSpyApiPortNode));
        }

        TiXmlNode* pSpyEventsPort = pGeneralNode->FirstChild(AF_globalSettingsSpyEventsPortNode);

        if (nullptr == pSpyEventsPort)
        {
            pSpyEventsPort = pGeneralNode->InsertEndChild(TiXmlElement(AF_globalSettingsSpyEventsPortNode));
        }

        // We can now treat all 4 port nodes as one group.
        GT_IF_WITH_ASSERT(nullptr != pRdsApiPort && nullptr != pRdsEventsPort &&
                          nullptr != pSpyApiPort && nullptr != pSpyEventsPort)
        {
            TiXmlNode* pRdsApiPortTextNode = pRdsApiPort->FirstChild();
            TiXmlNode* pRdsEventsPortTextNode = pRdsEventsPort->FirstChild();
            TiXmlNode* pSpyApiPortTextNode = pSpyApiPort->FirstChild();
            TiXmlNode* pSpyEventsPortTextNode = pSpyEventsPort->FirstChild();

            if (nullptr == pRdsApiPortTextNode)
            {
                pRdsApiPortTextNode = pRdsApiPort->InsertEndChild(TiXmlText(OS_STR_EmptyXMLString));
            }

            if (nullptr == pRdsEventsPortTextNode)
            {
                pRdsEventsPortTextNode = pRdsEventsPort->InsertEndChild(TiXmlText(OS_STR_EmptyXMLString));
            }

            if (nullptr == pSpyApiPortTextNode)
            {
                pSpyApiPortTextNode = pSpyApiPort->InsertEndChild(TiXmlText(OS_STR_EmptyXMLString));
            }

            if (nullptr == pSpyEventsPortTextNode)
            {
                pSpyEventsPortTextNode = pSpyEventsPort->InsertEndChild(TiXmlText(OS_STR_EmptyXMLString));
            }

            // Now write the values.
            gtString txtRdsApiPort;
            gtString txtRdsEventsPort;
            gtString txtSpyApiPort;
            gtString txtSpyEventsPort;
            txtRdsApiPort.appendUnsignedIntNumber(m_rdsPort);
            txtRdsEventsPort.appendUnsignedIntNumber(m_rdsEventsPort);
            txtSpyApiPort.appendUnsignedIntNumber(m_spyPort);
            txtSpyEventsPort.appendUnsignedIntNumber(m_spyEventsPort);
            writeStringToXmlNode(pRdsApiPortTextNode, txtRdsApiPort);
            writeStringToXmlNode(pRdsEventsPortTextNode, txtRdsEventsPort);
            writeStringToXmlNode(pSpyApiPortTextNode, txtSpyApiPort);
            writeStringToXmlNode(pSpyEventsPortTextNode, txtSpyEventsPort);
        }

        // Get last kernel source path browser directory:
        TiXmlNode* pPathNode = pGeneralNode->FirstChild(AF_globalSettingsEulaVersionAcceptedNode);

        if (nullptr == pPathNode)
        {
            pPathNode = pGeneralNode->InsertEndChild(TiXmlElement(AF_globalSettingsEulaVersionAcceptedNode));
        }

        GT_IF_WITH_ASSERT(nullptr != pPathNode)
        {
            TiXmlNode* pPathTextNode = pPathNode->FirstChild();

            if (nullptr == pPathTextNode)
            {
                pPathTextNode = pPathNode->InsertEndChild(TiXmlText(OS_STR_EmptyXMLString));
            }

            gtString eulaVersionAsStr;
            eulaVersionAsStr.appendFormattedString(L"%d", getEULRevisionNumber());

            bool rcWr = writeStringToXmlNode(pPathTextNode, eulaVersionAsStr);
            GT_ASSERT(rcWr);
        }

        // Floating point precision:
        pPathNode = pGeneralNode->FirstChild(AF_globalSettingsFloatingPointPrecisionNode);

        if (nullptr == pPathNode)
        {
            pPathNode = pGeneralNode->InsertEndChild(TiXmlElement(AF_globalSettingsFloatingPointPrecisionNode));
        }

        GT_IF_WITH_ASSERT(nullptr != pPathNode)
        {
            TiXmlNode* pPathTextNode = pPathNode->FirstChild();

            if (nullptr == pPathTextNode)
            {
                pPathTextNode = pPathNode->InsertEndChild(TiXmlText(OS_STR_EmptyXMLString));
            }

            gtString fpStr;
            fpStr.appendFormattedString(L"%d", m_fpPrecision);

            bool rcWr = writeStringToXmlNode(pPathTextNode, fpStr);
            GT_ASSERT(rcWr);
        }

        // Source files alert:
        pPathNode = pGeneralNode->FirstChild(AF_globalSettingsAlertMissingSourceNode);

        if (nullptr == pPathNode)
        {
            pPathNode = pGeneralNode->InsertEndChild(TiXmlElement(AF_globalSettingsAlertMissingSourceNode));
        }

        GT_IF_WITH_ASSERT(nullptr != pPathNode)
        {
            TiXmlNode* pPathTextNode = pPathNode->FirstChild();

            if (nullptr == pPathTextNode)
            {
                pPathTextNode = pPathNode->InsertEndChild(TiXmlText(OS_STR_EmptyXMLString));
            }

            gtString alertStr;
            int test = m_shouldAlertMissingSourceFile ? 1 : 0;
            alertStr.appendFormattedString(L"%d", test);

            bool rcWr = writeStringToXmlNode(pPathTextNode, alertStr);
            GT_ASSERT(rcWr);
        }

        // Save dialogs not to show
        pPathNode = pGeneralNode->FirstChild(AF_globalSettingsDialogsNotToShow);

        if (nullptr == pPathNode)
        {
            pPathNode = pGeneralNode->InsertEndChild(TiXmlElement(AF_globalSettingsDialogsNotToShow));
        }

        GT_IF_WITH_ASSERT(nullptr != pPathNode)
        {
            int numDialogs = m_doNotShowAgainDialogs.size();

            for (int nDialog = 0 ; nDialog < numDialogs ; nDialog++)
            {
                TiXmlNode* pPathDialogNode = pPathNode->InsertEndChild(TiXmlElement(AF_globalSettingsDialogsName));
                TiXmlNode* pPathTextNode = pPathDialogNode->FirstChild();

                if (nullptr == pPathTextNode)
                {
                    pPathTextNode = pPathDialogNode->InsertEndChild(TiXmlText(OS_STR_EmptyXMLString));
                }

                gtString dialogName(m_doNotShowAgainDialogs.at(nDialog).toStdWString().c_str());

                bool rcWr = writeStringToXmlNode(pPathTextNode, dialogName);
                GT_ASSERT(rcWr);
            }
        }

        // Write the history list:
        WriteHistoryListToXML(pGeneralNode);
    }
}

// ---------------------------------------------------------------------------
// Name:        afGlobalVariablesManager::setProxyInformation
// Description: Sets the program's proxy settings
// Author:      Uri Shomroni
// Date:        3/9/2008
// ---------------------------------------------------------------------------
void afGlobalVariablesManager::setProxyInformation(bool isUsingProxy, const osPortAddress& proxyServer)
{
    _isUsingProxy = isUsingProxy;

    if (isUsingProxy)
    {
        _proxyServer = proxyServer;
    }
    else
    {
        _proxyServer.setAsRemotePortAddress(AF_STR_Empty, 0);
    }
}

// ---------------------------------------------------------------------------
// Name:        afGlobalVariablesManager::getProxyInformation
// Description: Retrieves the program's proxy settings
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        3/9/2008
// ---------------------------------------------------------------------------
bool afGlobalVariablesManager::getProxyInformation(bool& isUsingProxy, osPortAddress& proxyServer) const
{
    bool retVal = true;

    isUsingProxy = _isUsingProxy;

    if (_isUsingProxy)
    {
        proxyServer = _proxyServer;
    }
    else
    {
        proxyServer.setAsRemotePortAddress(AF_STR_Empty, 0);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afGlobalVariablesManager::askRestoreLogSeverity
// Description: Recomend to restore the log severity level
// Return Val:  void.
// Author:      Bhattacharyya Koushik
// Date:        8/16/2012
// ---------------------------------------------------------------------------
int afGlobalVariablesManager::askRestoreLogSeverity() const
{
    int reply = QMessageBox::Ok;
    // Get the log file severity from the infra:
    osDebugLog& theDebugLog = osDebugLog::instance();
    osDebugLogSeverity loggedSeverity = theDebugLog.loggedSeverity();

    if ((loggedSeverity >= OS_DEBUG_LOG_DEBUG) && theDebugLog.loggedSeverityChangedToHigh())
    {
        // Warn the user if the debug log level is set to Debug or extensive:
        gtString debugLogLevelWarning;
        const wchar_t* debugLevelAsString = osDebugLog::loggedSeverityAsString(loggedSeverity);
        debugLogLevelWarning.appendFormattedString(AF_STR_DebugLogLevelWarning, debugLevelAsString);
        reply = acMessageBox::instance().warning(AF_STR_DebugLogLevelDebugWarningTitle, acGTStringToQString(debugLogLevelWarning), QMessageBox::Ok | QMessageBox::Cancel);

        if (QMessageBox::Ok == reply)
        {
            theDebugLog.setLoggedSeverityChangedToHigh(false);
        }
    }

    return reply;
}

bool afGlobalVariablesManager::readCodeXLVersionXML()
{
    bool retVal = false;

    osFilePath versionXMLPath;
    retVal = versionXMLPath.SetInstallRelatedPath(osFilePath::OS_CODEXL_DATA_PATH);
    GT_IF_WITH_ASSERT(retVal)
    {
        versionXMLPath.setFileName(AF_STR_CodeXLVersionXMLFileName);
        versionXMLPath.setFileExtension(AF_STR_globalSettingsFileExtension);

        GT_IF_WITH_ASSERT(versionXMLPath.exists())
        {

            // Define an XML document:
            gtString filePathStr = versionXMLPath.asString();
            std::string utf8FilePath;
            filePathStr.asUtf8(utf8FilePath);
            TiXmlDocument doc(utf8FilePath.c_str());

            // Load the file:
            retVal = doc.LoadFile();

            if (retVal)
            {
                TiXmlHandle docHandle(&doc);
                TiXmlNode* pVersionCaptionNode = docHandle.FirstChild(AF_STR_CodeXLVersionXMLVersionSettingsNodeName).FirstChild(AF_STR_CodeXLVersionXMLTitleCaptionNodeName).FirstChild().ToNode();

                if (pVersionCaptionNode != nullptr)
                {
                    gtString versionCaptionStr;
                    readStringFromXmlNode(pVersionCaptionNode, versionCaptionStr);
                    versionCaptionStr.prepend(L"Version caption from XML file:");
                    OS_OUTPUT_DEBUG_LOG(versionCaptionStr.asCharArray(), OS_DEBUG_LOG_DEBUG);
                }

                TiXmlNode* pUpdaterXMLPathNode = docHandle.FirstChild(AF_STR_CodeXLVersionXMLVersionSettingsNodeName).FirstChild(AF_STR_CodeXLVersionXMLCheckForNewVersionURLNodeName).FirstChild().ToNode();
                GT_IF_WITH_ASSERT(pUpdaterXMLPathNode != nullptr)
                {
                    readStringFromXmlNode(pUpdaterXMLPathNode, m_updaterXML);
                }
            }
        }
    }

    return retVal;
}

void afGlobalVariablesManager::setRemoteDebuggingPorts(unsigned short spyPort, unsigned short spyEventsPort, unsigned short rdsPort, unsigned short rdsEventsPort)
{
    m_spyPort = spyPort;
    m_spyEventsPort = spyEventsPort;
    m_rdsPort = rdsPort;
    m_rdsEventsPort = rdsEventsPort;
}

void afGlobalVariablesManager::setFloatingPointPrecision(int precision)
{
    m_fpPrecision = precision;

    // Set the precision for the item responsible for numbering format:
    acNumberDelegateItem::SetFloatingPointPercision(m_fpPrecision);
}

void afGlobalVariablesManager::FindInstalledAMDComponents()
{
    // No components Loaded:
    m_installedAMDComponentsBitmask = 0;

    // Assume we have some GPU devices installed.
    // TODO: this should be replaced by some detailed GPU device analysis.
    m_installedAMDComponentsBitmask |= AF_AMD_GPU_COMPONENT;

    // check CPU not using catalyst in case catalyst was not installed:
    unsigned regs[4];

    // Get vendor
    unsigned vendor[8];
    GetCPUID(0, regs);

    vendor[0] = regs[1]; // EBX
    vendor[1] = regs[3]; // EDX
    vendor[2] = regs[2]; // ECX

    std::string cpuVendorOption1 = std::string((char*)vendor, 12);
    std::string cpuVendorOption2 = std::string((char*)vendor, 28);

    if ("Advanced Micro Devices, Inc." == cpuVendorOption2 || ("AuthenticAMD" == cpuVendorOption1))
    {
        m_installedAMDComponentsBitmask |= AF_AMD_CPU_COMPONENT;
    }

    int driverError = OA_DRIVER_UNKNOWN;
    gtString driverVersion = oaGetDriverVersion(driverError);

    if (driverError != OA_DRIVER_NOT_FOUND)
    {
        m_installedAMDComponentsBitmask |= AF_AMD_CATALYST_COMPONENT;
    }

    // Check for HSA driver (on Linux):
    if (oaIsHSADriver())
    {
        // If this is an HSA machine, we should enable GPU profiling:
        AddInstalledAMDComponents(AF_AMD_GPU_COMPONENT);
        AddInstalledAMDComponents(AF_AMD_HSA_COMPONENT);
    }

    // Build message string
    m_missingInstalledAMDComponentsMessage = L"";

    if (!(m_installedAMDComponentsBitmask & AF_AMD_GPU_COMPONENT))
    {
        m_missingInstalledAMDComponentsMessage += AF_STR_AMD_GPU_COMPONENT_NOT_INSTALLED;
        OS_OUTPUT_DEBUG_LOG(AF_STR_AMD_GPU_COMPONENT_NOT_INSTALLED, OS_DEBUG_LOG_INFO);
    }

    if (!(m_installedAMDComponentsBitmask & AF_AMD_CATALYST_COMPONENT))
    {
        m_missingInstalledAMDComponentsMessage += AF_STR_AMD_CATALYST_COMPONENT_NOT_INSTALLED;
        OS_OUTPUT_DEBUG_LOG(AF_STR_AMD_CATALYST_COMPONENT_NOT_INSTALLED, OS_DEBUG_LOG_INFO);
    }

    gtString strLog;
    strLog.appendFormattedString(L"Installed AMD Components bitmask: %d", m_installedAMDComponentsBitmask);
    OS_OUTPUT_DEBUG_LOG(strLog.asCharArray(), OS_DEBUG_LOG_DEBUG);
}


void afGlobalVariablesManager::GetCPUID(unsigned i, unsigned regs[4])
{
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    __cpuid((int*)regs, (int)i);

#else
    asm volatile
    ("cpuid" : "=a"(regs[0]), "=b"(regs[1]), "=c"(regs[2]), "=d"(regs[3])
     : "a"(i), "c"(0));
    // ECX is set to zero for CPUID function 4
#endif
}

void afGlobalVariablesManager::GetHistoryList(const QString& widgetID, QStringList& historyList)
{
    GT_IF_WITH_ASSERT(!widgetID.isEmpty())
    {
        historyList = m_widgetsHistoryMap[widgetID];
        historyList.removeDuplicates();
    }
}

void afGlobalVariablesManager::SetHistoryList(const QString& widgetID, const QStringList& historyList)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(!widgetID.isEmpty())
    {
        m_widgetsHistoryMap[widgetID] = historyList;
    }
}

void afGlobalVariablesManager::ReadHistoryListFromXML(TiXmlNode* pGeneralNode)
{
    // Get history list:
    TiXmlNode* pHistoryListNode = pGeneralNode->FirstChild(AF_globalSettingsHistoryList);

    if (nullptr != pHistoryListNode)
    {
        // Get the first widget history list node:
        TiXmlNode* pWidgetHistoryListNode = pHistoryListNode->FirstChild(AF_globalSettingsWidgetHistoryList);

        // Iterate the widget history list nodes:
        while (pWidgetHistoryListNode != nullptr)
        {
            // Get the widget name:
            gtString widgetName;
            ReadAttributeFromXMLNode(pWidgetHistoryListNode, AF_globalSettingsWidgetNameAttribute, widgetName);

            // Get the widget name as QString:
            QString widgetNameQt = acGTStringToQString(widgetName);

            // Get the first widget history list node:
            TiXmlNode* pWidgetCompleteListItem = pWidgetHistoryListNode->FirstChild(AF_globalSettingsWidgetHistoryListItem);

            // Iterate the widget history list item nodes:
            while (pWidgetCompleteListItem != nullptr)
            {
                // Get the widget name:
                gtString widgetItemString;
                readStringFromXmlNode(pWidgetCompleteListItem->FirstChild(), widgetItemString);
                pWidgetCompleteListItem = pWidgetCompleteListItem->NextSibling();

                m_widgetsHistoryMap[widgetNameQt] << acGTStringToQString(widgetItemString);
            }

            pWidgetHistoryListNode = pWidgetHistoryListNode->NextSibling();
        }
    }
}

void afGlobalVariablesManager::WriteHistoryListToXML(TiXmlNode* pGeneralNode)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pGeneralNode != nullptr)
    {
        // Get the history list node:
        TiXmlNode* pHistoryListNode = pGeneralNode->FirstChild(AF_globalSettingsHistoryList);

        if (pHistoryListNode == nullptr)
        {
            pHistoryListNode = pGeneralNode->InsertEndChild(TiXmlElement(AF_globalSettingsHistoryList));
        }

        GT_IF_WITH_ASSERT(pHistoryListNode != nullptr)
        {
            QMap<QString, QStringList>::Iterator iter = m_widgetsHistoryMap.begin();

            for (; iter != m_widgetsHistoryMap.end(); iter++)
            {
                TiXmlNode* pCurrentWidgetNode = pHistoryListNode->InsertEndChild(TiXmlElement(AF_globalSettingsWidgetHistoryList));
                GT_IF_WITH_ASSERT(pCurrentWidgetNode != nullptr)
                {
                    gtString widgetName = acQStringToGTString(iter.key());
                    WriteAttributeToXMLNode(pCurrentWidgetNode, AF_globalSettingsWidgetNameAttribute, widgetName);
                    QStringList list = iter.value();

                    foreach (QString str, list)
                    {
                        TiXmlNode* pCurrentWidgetValueNode = pCurrentWidgetNode->InsertEndChild(TiXmlElement(AF_globalSettingsWidgetHistoryListItem));
                        GT_IF_WITH_ASSERT(pCurrentWidgetValueNode != nullptr)
                        {
                            TiXmlNode* pCurrentWidgetValueNodeText = pCurrentWidgetValueNode->InsertEndChild(TiXmlText(OS_STR_EmptyXMLString));
                            GT_IF_WITH_ASSERT(pCurrentWidgetValueNodeText != nullptr)
                            {
                                gtString widgetStr = acQStringToGTString(str);
                                writeStringToXmlNode(pCurrentWidgetValueNodeText, widgetStr);
                            }
                        }
                    }
                }
            }
        }
    }
}

void afGlobalVariablesManager::AddStringToHistoryList(const QString& widgetID, const QString& historyStr)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(!widgetID.isEmpty())
    {
        m_widgetsHistoryMap[widgetID] << historyStr;
    }
}

QString afGlobalVariablesManager::GetLastBrowseLocation(const QString& browseLocationFieldID)
{
    QString retVal;

    if (m_widgetsHistoryMap.contains(browseLocationFieldID))
    {
        // Make sure that for these fields we only have one value in the array:
        GT_ASSERT(m_widgetsHistoryMap[browseLocationFieldID].size() == 1);
        retVal = m_widgetsHistoryMap[browseLocationFieldID].first();
    }

    return retVal;
}

void afGlobalVariablesManager::SetLastBrowseLocation(const QString& browseLocationFieldID, const QString& browseLocation)
{
    if (m_widgetsHistoryMap.contains(browseLocationFieldID))
    {
        // Make sure that for these fields we only have one value in the array:
        m_widgetsHistoryMap[browseLocationFieldID].clear();
    }

    // Add the browse location as first element in the list:
    m_widgetsHistoryMap[browseLocationFieldID] << browseLocation;
}

void afGlobalVariablesManager::SetProductName(const gtString& productName)
{
    m_sProductName = productName;
    m_sProductNameA = acGTStringToQString(m_sProductName);
    int charsAmount = m_sProductNameA.size() + 1;
    m_sProductNameCharArray = new char[charsAmount];
    strcpy(m_sProductNameCharArray, m_sProductNameA.toLatin1().data());
}

void afGlobalVariablesManager::AppendUnsupportedMessage(const gtString& message)
{
    if (!m_missingInstalledAMDComponentsMessage.isEmpty())
    {
        m_missingInstalledAMDComponentsMessage.append(AF_STR_NewLine);
    }

    m_missingInstalledAMDComponentsMessage.append(message);
}

void afGlobalVariablesManager::AddInstalledAMDComponents(afInstalledAMDComponents installedComponent)
{
    if (!(m_installedAMDComponentsBitmask & installedComponent))
    {
        // Add this installed component:
        m_installedAMDComponentsBitmask |= installedComponent;

        if (installedComponent == AF_AMD_GPU_COMPONENT)
        {
            m_missingInstalledAMDComponentsMessage.replace(AF_STR_AMD_GPU_COMPONENT_NOT_INSTALLED, L"");
        }

        if (installedComponent == AF_AMD_CATALYST_COMPONENT)
        {
            m_missingInstalledAMDComponentsMessage.replace(AF_STR_AMD_CATALYST_COMPONENT_NOT_INSTALLED, L"");
        }
    }
}

bool afGlobalVariablesManager::isRunningInsideVisualStudio() const
{
    bool isRunningInsideVS = false;

    if (GetExecutedApplicationType() == OS_VISUAL_STUDIO_PLUGIN_TYPE)
    {
        isRunningInsideVS = true;
    }

    return isRunningInsideVS;
}
