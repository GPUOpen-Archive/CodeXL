//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProjectHandler.h
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/inc/CpuProjectHandler.h#58 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================
#ifndef _CPUPROJECTHANDLER_H
#define _CPUPROJECTHANDLER_H

// Qt:
#include <QtCore>
#include <QtWidgets>
#include <QXmlDefaultHandler>

//Shared Profiling
#include <SessionExplorerDefs.h>
#include <SessionTreeNodeData.h>

// Infra:
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>
#include <AMDTApplicationFramework/Include/afGlobalVariableChangedEvent.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTOSWrappers/Include/osFilePath.h>

#include <AMDTCpuPerfEventUtils/inc/DcConfig.h>
#include <AMDTCpuProfilingBackendUtils/Include/CpuProfileDefinitions.h>

// Predefine
class ProfileApplicationTreeHandler;
class apMonitoredObjectsTreeActivatedEvent;

class CPUSessionTreeItemData : public SessionTreeNodeData
{
    Q_OBJECT

public:

    // Tab captions strings:
    static const gtString ms_CPU_PROFILE_OVRVIEW_DISPLAY_STR;
    static const gtString ms_CPU_PROFILE_MODULES_DISPLAY_STR;
    static const gtString ms_CPU_PROFILE_CALL_GRAPH_DISPLAY_STR;
    static const gtString ms_CPU_PROFILE_FUNCTIONS_DISPLAY_STR;

    CPUSessionTreeItemData();
    ~CPUSessionTreeItemData();

    /// Copy constructor
    CPUSessionTreeItemData(const CPUSessionTreeItemData& other);

    /// Initializes all values to default
    void Init();

    /// Copy the data from the other cpu session data:
    /// \param pOther the data to copy from
    void CopyFrom(const CPUSessionTreeItemData* pOther, bool copySharedData);

    /// Copy the data from the other session data:
    /// \param other the data to copy from
    void CopyFrom(const SessionTreeNodeData& other);

    /// Static utility checking if CSS collection is disabled for the input the exe file name:
    static bool ShouldDisableCSS(const QString& exeFileName);

    /// Accessor to CSS collection flag:
    bool ShouldCollectCSS(bool checkForLimitations = true) const;
    void SetShouldCollectCSS(bool shouldCollectCSS) { m_collectCSS = shouldCollectCSS; };

    static gtString sessionDisplayTypeToString(afTreeItemType displayType);

    /// this bug returns the checked/unchecked value of FPO checkbox, depend on the profiling type
    /// returns - if time based profiling returns the value of m_isTimeBasedCssSupportFpo, else returns m_isOtherCpuCssSupportFpo
    bool IsFpoChecked() const;

    unsigned int GetCssUnwindLevel() const;

    bool IsTimeBasedProfiling() const;

    int m_cores;
    gtUInt64 m_startAffinity;

    unsigned int m_cssInterval;
    CpuProfileCssScope m_cssScope;
    bool m_isTimeBasedCssSupportFpo;
    bool m_isOtherCpuCssSupportFpo;
    unsigned int m_timeBasedCssDepthLevel;
    unsigned int m_otherCpuCssDepthLevel;

    // Save raw files
    bool m_shouldSaveRawFiles;

    // Next version
    bool m_cacheLineUtilChecked;
    bool m_cacheLineUtilEnabled;
    bool m_ldstChecked;
    bool m_ldstEnabled;
    bool m_shoulApplyFilter;
    unsigned int m_utilMask;

    // Timer
    float m_msInterval;

    // Event
    gtVector<DcEventConfig> m_eventsVector;

    // IBS
    bool m_fetchSample;
    bool m_opSample;
    bool m_opCycleCount;
    unsigned long m_fetchInterval;
    unsigned long m_opInterval;

    // CLU
    bool m_cluSample;
    bool m_cluCycleCount;
    unsigned long m_cluInterval;

    enum UTILMASK
    {
        CPU_UTIL = 1,
        MEM_UTIL = 2
    };

    bool getCLUOption() { return (m_cacheLineUtilEnabled && m_cacheLineUtilChecked); };
    bool getldstOption() { return (m_ldstEnabled && m_ldstChecked); };

    // Check if we are doing the CLU profiling only (either by CLU or by Custom profile)
    bool IsProfilingCluOnly()
    {
        return (m_cluSample &&              // CLU is enabled
                (0.0f >= m_msInterval) &&   // No TBP
                (0 >= m_eventsVector.size()) &&     // No EBP
                (!m_fetchSample) &&         // No IBS fetch
                (!m_opSample));             // No IBS op

    }

private:

    /// Call stack collection: do not allow an access to this parameter. The css collection should take in consideration
    /// the exe type and profile type:
    bool m_collectCSS;
};

class CpuProjectHandler : public QObject, public apIEventsObserver, public QXmlDefaultHandler
{
    Q_OBJECT

public:

    /// Get the singleton instance
    static CpuProjectHandler& instance();
    virtual ~CpuProjectHandler();

    // Overrides apIEventsObserver:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);
    virtual const wchar_t* eventObserverName() const { return L"CpuProjectHandler"; };

    bool isProjectLoaded();
    CPUSessionTreeItemData* getProjectSettings();

    // Koushik:BUG367296
    CPUSessionTreeItemData* getSavedProjectSettings() { return &m_savedCurrentSettings; }

    const gtString& getProfileName(const gtString& profilePath);

    bool isProfileNameValid(const QString& profileName, QString& invalidMessageStr);
    bool profileExists(const QString& profileName, QString& invalidMessage);

    bool getProjectSettingsXML(gtString& projectAsXMLString);
    bool setProjectSettingsXML(const gtString& projectAsXMLString);

    void addSession(CPUSessionTreeItemData* pCPUSessionData, bool displaySession);

    // Override QXmlDefaultHandler
    bool startDocument() ;
    bool endDocument() ;
    bool startElement(const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts) ;
    bool endElement(const QString& namespaceURI, const QString& localName, const QString& qName) ;
    bool characters(const QString& ch) ;
    void emitFileImportedComplete();

public slots:
    void onSharedSessionAvailable();
    void onRemoveSession(ExplorerSessionId sharedId, SessionExplorerDeleteType deleteType, bool& sessionDeleted);
    void onImportSession(const QString& strFileName, bool& imported);

    void handleRawDataFileImport(const osFilePath& importProfileFilePath);
    void handleEBPFileImport(const osFilePath& importProfileFilePath);
    void handleDataFileImport(const osFilePath& importProfileFilePath);

    void onSessionRename(SessionTreeNodeData* pRenamedSessionData, const osFilePath& oldSessionFilePath, const osDirectory& oldSessionDirectory);
    void OnClearCurrentProjectSettings();
protected:
    void writeSession(gtString& projectAsXMLString, const CPUSessionTreeItemData& session, const gtString& type);
    void writeValue(gtString& projectAsXMLString, const gtString& key, const gtString& value);
    void writeBool(gtString& projectAsXMLString, const gtString& key, const bool value);
    CPUSessionTreeItemData* findSessionItemData(ExplorerSessionId sharedId);

    void onTreeActivationEvent(const apMonitoredObjectsTreeActivatedEvent& activationEvent);

    void renameFilesInDir(osDirectory& dir, const gtString& oldName, const gtString& newName);

    const osFilePath& getAlphaProfilePath(const QString& profileName);

    /// Protected creator
    CpuProjectHandler();
    /// The singleton instance
    static CpuProjectHandler* m_pMySingleInstance;

    //Project
    bool m_ProjectLoaded;
    gtString m_currentProject;
    CPUSessionTreeItemData m_currentSettings;

    CPUSessionTreeItemData m_savedCurrentSettings;

    gtVector<ExplorerSessionId> m_sessions;
    ProfileApplicationTreeHandler* m_pProfileTreeHandler;

    //XML reader variables
    bool m_CpuProfileExtension;
    bool m_xmlProfile;
    CPUSessionTreeItemData* m_pCurrentXMLSession;

    QString m_xmlContent;
    unsigned int m_xmlPmcIndex;
    bool m_caperfImported;

signals:

    //emited when file import is complete with success or failure
    void FileImportedComplete();

};
#endif //_CPUPROJECTHANDLER_H
