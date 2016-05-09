//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afGlobalVariablesManager.h
///
//==================================================================================

#ifndef __AFGLOBALVARIABLESMANAGER_H
#define __AFGLOBALVARIABLESMANAGER_H

//Qt
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osPortAddress.h>
#include <AMDTOSWrappers/Include/osSettingsFileHandler.h>
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afGlobalSettingsPage.h>

// ----------------------------------------------------------------------------------
// Class Name:          afGlobalVariablesManager : apIEventsObserver
// General Description: Stores and manages the CodeXL global settings
// Author:              Sigal Algranaty
// Creation Date:       19/4/2012
// ----------------------------------------------------------------------------------
class AF_API afGlobalVariablesManager : public apIEventsObserver, public osSettingsFileHandler
{
public:

    static afGlobalVariablesManager& instance();
    virtual ~afGlobalVariablesManager();

    // Standalone / VS:
    bool isRunningInsideVisualStudio() const;

    void setLogFilesDirectoryPath(const osFilePath& logFilesDirectoryPath) { m_logFilesDirectoryPath = logFilesDirectoryPath; };
    const osFilePath& logFilesDirectoryPath() const { return m_logFilesDirectoryPath; };

    // Overrides apIEventsObserver:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);
    virtual const wchar_t* eventObserverName() const { return L"afGlobalVariablesManager"; };

    // Proxy Settings:
    void setProxyInformation(bool isUsingProxy, const osPortAddress& proxyServer);
    bool getProxyInformation(bool& isUsingProxy, osPortAddress& proxyServer) const;

    // Remote Debugging ports:
    void setRemoteDebuggingPorts(unsigned short spyPort, unsigned short spyEventsPort,
                                 unsigned short rdsPort, unsigned short rdsEventsPort);
    unsigned short getSpyPort() const { return m_spyPort; }
    unsigned short getSpyEventsPort() const { return m_spyEventsPort; }
    unsigned short getRdsPort() const { return m_rdsPort; }
    unsigned short getRdsEventsPort() const { return m_rdsEventsPort; }

    // EULA
    void setEULRevisionNumber(int newRevisionNumber);
    int getEULRevisionNumber() const {return _eulaAcceptedRevisionNumber;};

    /// This function should be called once, before the application initialization:
    /// \param productName The name of the product. For instance, for CodeXL.exe, productName will be CodeXL
    static void SetProductName(const gtString& productName);

    /// Return the product name:
    static gtString ProductName() { return m_sProductName; };

    /// Return the product name as QString:
    static QString ProductNameA() { return m_sProductNameA; };

    /// Return the product name as QString:
    static char* ProductNameCharArray() { return m_sProductNameCharArray; };

    /// Global product icon ID:
    static void SetProductIconID(acIconId iconId) { m_sProductIconId = iconId; }
    static acIconId ProductIconID() { return m_sProductIconId; }

    // dialogs not to show again
    void setNotToShowAgain(QString& title);
    bool isSetNotToShowAgain(QString& title);

    // Global settings pages:
    void registerGlobalSettingsPage(afGlobalSettingsPage* pGlobalSettingsPage);
    int amountOfGlobalSettingPages() {return (int)m_globalSettingsPages.size();};
    afGlobalSettingsPage* getGlobalSettingPage(int pageIndex, gtString* pageTitle = nullptr);

    // Global settings XML file:
    virtual bool initXMLDocToSettingsFileDefaultValues(TiXmlDocument& XMLDocument);
    void loadGlobalSettingsFromXMLFile();
    void loadGlobalSettingsIntoPage(afGlobalSettingsPage* pGlobalSettingsPage);
    void applyXMLSettingsToGlobalSettingsPages();
    void saveGlobalSettingsToXMLFile();

    void loadGeneralSettingsFromLoadedXML();

    /// Read the history list from the XML. Expecting the following structure:
    /// <AF_globalSettingsWidgetHistoryList>
    ///         <AF_globalSettingsWidgetHistoryList widgetName="widget1">
    ///                 <AF_globalSettingsWidgetHistoryListItem>item1ForWidget1</AF_globalSettingsWidgetHistoryListItem>
    ///                 <AF_globalSettingsWidgetHistoryListItem>item2ForWidget1</AF_globalSettingsWidgetHistoryListItem>
    ///                 <AF_globalSettingsWidgetHistoryListItem>item3ForWidget1</AF_globalSettingsWidgetHistoryListItem>
    ///         </AF_globalSettingsWidgetHistoryList>
    void ReadHistoryListFromXML(TiXmlNode* pGeneralNode);

    /// Write the history list to the XML. Build the following structure:
    /// <AF_globalSettingsWidgetHistoryList>
    ///         <AF_globalSettingsWidgetHistoryList widgetName="widget1">
    ///                 <AF_globalSettingsWidgetHistoryListItem>item1ForWidget1</AF_globalSettingsWidgetHistoryListItem>
    ///                 <AF_globalSettingsWidgetHistoryListItem>item2ForWidget1</AF_globalSettingsWidgetHistoryListItem>
    ///                 <AF_globalSettingsWidgetHistoryListItem>item3ForWidget1</AF_globalSettingsWidgetHistoryListItem>
    ///         </AF_globalSettingsWidgetHistoryList>
    void WriteHistoryListToXML(TiXmlNode* pGeneralNode);

    void saveGeneralSettingToLoadedXML();

    int askRestoreLogSeverity() const;

    bool readCodeXLVersionXML();

    // CodeXL version:
    const gtString& versionCaption() const { return m_versionCaption; };
    const gtString& updaterXML() const { return m_updaterXML; };

    void setFloatingPointPrecision(int precision);
    int floatingPointPrecision() const { return m_fpPrecision; };

    int percentagePointPrecision() const { return m_percentagePrecision; };

    bool ShouldAlertMissingSourceFile() const { return m_shouldAlertMissingSourceFile; };
    void SetShouldAlertMissingSourceFile(bool alert) { m_shouldAlertMissingSourceFile = alert; };

    /// Installed AMD components:
    unsigned int InstalledAMDComponentsBitmask() const { return m_installedAMDComponentsBitmask; };
    gtString InstalledAMDComponentsErrorMessage() const { return m_missingInstalledAMDComponentsMessage; };

    /// Append an unsupported message to the current message:
    void AppendUnsupportedMessage(const gtString& message);

    /// Get a history list for the widget with the requested ID:
    /// \param browseLocationFieldID the ID for which the last browse location is needed
    /// \return the last browsed location for this field
    QString GetLastBrowseLocation(const QString& browseLocationFieldID);

    /// Set a history list for the widget with the requested ID:
    /// \param browseLocationFieldID the ID for which the last browse location is needed
    /// \param browseLocation the path in which the user last browsed for this field the list of strings typed in the requested widget
    void SetLastBrowseLocation(const QString& browseLocationFieldID, const QString& browseLocation);

    /// Get a history list for the widget with the requested ID:
    /// \param widgetID the widget ID (used object name for this ID, to avoid multiplication of strings):
    /// \param historyList[out] list of strings containing the history of the requested widgets edit
    void GetHistoryList(const QString& widgetID, QStringList& historyList);

    /// Set a history list for the widget with the requested ID:
    /// \param widgetID the widget ID (used object name for this ID, to avoid multiplication of strings)
    /// \param historyList the list of strings typed in the requested widget
    void SetHistoryList(const QString& widgetID, const QStringList& historyList);

    /// Set a history list for the widget with the requested ID:
    /// \param widgetID the widget ID (used object name for this ID, to avoid multiplication of strings)
    /// \param historyStr append the string to the list strings typed in the requested widget
    void AddStringToHistoryList(const QString& widgetID, const QString& historyStr);

protected:

    // Only my instance() function can create me:
    afGlobalVariablesManager();
    afGlobalVariablesManager(const osFilePath& settingsFilePath);
    // Only gdSingletonsDelete should delete me:
    friend class afSingletonsDelete;

    // Find the installed AMD components and save the error message for later components use:
    void FindInstalledAMDComponents();

    /// Update an installed component:
    void AddInstalledAMDComponents(afInstalledAMDComponents installedComponent);

    void GetCPUID(unsigned i, unsigned regs[4]);

protected:

    // The single instance of this class:
    static afGlobalVariablesManager* m_pMySingleInstance;

    // Holds the global settings pages:
    gtPtrVector<afGlobalSettingsPage*> m_globalSettingsPages;

    // Proxy settings:
    bool _isUsingProxy;
    osPortAddress _proxyServer;

    // EULA:
    int _eulaAcceptedRevisionNumber;

    // Remote debugging ports:
    unsigned short m_spyPort;
    unsigned short m_spyEventsPort;
    unsigned short m_rdsPort;
    unsigned short m_rdsEventsPort;

    // Log files folder:
    osFilePath m_logFilesDirectoryPath;

    // map of dialogs that were closed by the user not to be shown again
    gtVector<QString> m_doNotShowAgainDialogs;

    // Contain the floating point precision:
    int m_fpPrecision;

    // The precision to display percentage values
    int m_percentagePrecision;

    // Alert missing source file:
    bool m_shouldAlertMissingSourceFile;

    // The CodeXL version caption (NDA, Internal, or empty string for public):
    gtString m_versionCaption;
    gtString m_updaterXML;

    /// Contain the list of installed AMD components:
    unsigned int m_installedAMDComponentsBitmask;
    gtString m_missingInstalledAMDComponentsMessage;

    /// Widget history map - a map containing the users used string for each of the widgets.
    /// this map is used to store and load user edit history for some of our widgets:
    QMap<QString, QStringList> m_widgetsHistoryMap;

    /// Static members that contain the product name
    static gtString m_sProductName;
    static QString m_sProductNameA;
    static char* m_sProductNameCharArray;
    /// Static member that contain the product icon id:
    static acIconId m_sProductIconId;


};


#endif //__AFGLOBALVARIABLESMANAGER_H

