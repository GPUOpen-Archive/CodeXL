//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProjectHandler.cpp
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/src/CpuProjectHandler.cpp#148 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================

// Qt:
#include <QtCore>
#include <QtWidgets>

// Infra:
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apMonitoredObjectsTreeEvent.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTOSWrappers/Include/osMachine.h>
#include <AMDTOSWrappers/Include/osApplication.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afHTMLContent.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>

//SharedProfiling
#include <AMDTSharedProfiling/inc/SessionExplorerDefs.h>
#include <AMDTSharedProfiling/inc/ProfileApplicationTreeHandler.h>
#include <AMDTSharedProfiling/inc/StringConstants.h>
#include <AMDTSharedProfiling/inc/SharedProfileSettingPage.h>

//Backend
#include <AMDTCpuProfilingRawData/inc/CpuProfileReader.h>
#include <AMDTCpuProfilingRawData/inc/RunInfo.h>
#include <AMDTCpuPerfEventUtils/inc/EventEngine.h>
#include <AMDTOSWrappers/Include/osTime.h>

// Local:
#include <inc/AmdtCpuProfiling.h>
#include <inc/StringConstants.h>
#include <inc/CpuProjectHandler.h>
#include <inc/SessionViewCreator.h>
#include <inc/StdAfx.h>
#include <inc/CommandsHandler.h>
#include <inc/ProfileConfigs.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include <AMDTExecutableFormat/inc/PeFile.h>
    #include <Driver/Windows/CpuProf/inc/UserAccess/CpuProfDriver.h>
#endif


// Static members:
CpuProjectHandler* CpuProjectHandler::m_pMySingleInstance = nullptr;
const gtString CPUSessionTreeItemData::ms_CPU_PROFILE_OVRVIEW_DISPLAY_STR = L"Overview";
const gtString CPUSessionTreeItemData::ms_CPU_PROFILE_MODULES_DISPLAY_STR = L"Modules";
const gtString CPUSessionTreeItemData::ms_CPU_PROFILE_CALL_GRAPH_DISPLAY_STR = L"Call Graph";
const gtString CPUSessionTreeItemData::ms_CPU_PROFILE_FUNCTIONS_DISPLAY_STR = L"Functions";


#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define JAVA_EXE_NAME    L"java.exe"
#else
    #define JAVA_EXE_NAME    L"java"
#endif

CPUSessionTreeItemData::CPUSessionTreeItemData()
    : SessionTreeNodeData()
{
    Init();


}

CPUSessionTreeItemData::CPUSessionTreeItemData(const CPUSessionTreeItemData& other) : SessionTreeNodeData()
{
    // Set all values to default
    Init();

    CopyFrom(&other, true);
}
void CPUSessionTreeItemData::Init()
{
    m_cores = 1;
    m_name = "Session";
    m_startDelay = 0;
#pragma message ("TODO: handle more than 64-m_cores here")
    // The default affinity is all available m_cores
    int coreCount(0);
    osGetAmountOfLocalMachineCPUs(coreCount);
    m_startAffinity = 0;

    for (int core = 0; core < coreCount; core++)
    {
        m_startAffinity <<= 1;
        m_startAffinity |= 1;
    }

    m_collectCSS = true;
    m_cssInterval = CP_CSS_DEFAULT_UNWIND_INTERVAL;
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    m_cssScope = CP_CSS_SCOPE_USER;
#else
    m_cssScope = CP_CSS_SCOPE_UNKNOWN;
#endif
    m_isTimeBasedCssSupportFpo = true;
    m_isOtherCpuCssSupportFpo = false;
    m_timeBasedCssDepthLevel = CP_CSS_HIGH_UNWIND_DEPTH;
    m_otherCpuCssDepthLevel = CP_CSS_LOW_UNWIND_DEPTH;

    m_shouldSaveRawFiles = false;

    //Next version
    m_cacheLineUtilChecked = false;
    m_cacheLineUtilEnabled = false;
    m_ldstChecked = false;
    m_ldstEnabled = false;
    m_shoulApplyFilter = false;
    m_utilMask = 0;

    m_msInterval = 0;

    m_fetchSample = false;
    m_opSample = false;
    m_opCycleCount = false;
    m_fetchInterval = 250000;
    m_opInterval = 250000;

    m_cluSample = false;
    m_cluCycleCount = false;
    m_cluInterval = 250000;
}


CPUSessionTreeItemData::~CPUSessionTreeItemData()
{
    m_eventsVector.clear();
}

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
static bool IsExeClr(const gtString& exeFileName)
{
    PeFile exe(exeFileName.asCharArray());
    return exe.Open() ? exe.FindClrInfo() : false;
}
#endif // AMDT_WINDOWS_OS

bool CPUSessionTreeItemData::ShouldDisableCSS(const QString& exeFileName)
{
    bool result = false;

    gtString exeFullPath = acQStringToGTString(exeFileName);

    osFilePath filePath(exeFullPath);
    gtString fileName;
    filePath.getFileNameAndExtension(fileName);


    // if this is a Java application.
    if (fileName == JAVA_EXE_NAME)
    {
        result = true;
    }

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    // If this is a managed application.
    if (!result && !exeFullPath.isEmpty() && IsExeClr(exeFullPath))
    {
        result = true;
    }

#endif

    return result;
}
bool CPUSessionTreeItemData::IsFpoChecked() const
{
    bool retval = IsTimeBasedProfiling() ? m_isTimeBasedCssSupportFpo : m_isOtherCpuCssSupportFpo;
    return retval;
}

unsigned int CPUSessionTreeItemData::GetCssUnwindLevel() const
{
    unsigned int retval = IsTimeBasedProfiling() ? m_timeBasedCssDepthLevel : m_otherCpuCssDepthLevel;
    return retval;
}

bool CPUSessionTreeItemData::IsTimeBasedProfiling() const
{
    return m_profileTypeStr == PM_profileTypeTimeBased;
}

void CPUSessionTreeItemData::CopyFrom(const SessionTreeNodeData& other)
{
    SessionTreeNodeData::CopyFrom(&other);
}

void CPUSessionTreeItemData::CopyFrom(const CPUSessionTreeItemData* pOther, bool copySharedData)
{
    GT_IF_WITH_ASSERT(pOther != nullptr)
    {
        // Clear any previous events
        m_eventsVector.clear();

        if (copySharedData)
        {
            SessionTreeNodeData::CopyFrom(pOther);

            if (pOther->m_pParentData != nullptr)
            {
                m_pParentData = new afApplicationTreeItemData;
                m_pParentData->m_filePath = pOther->m_pParentData->m_filePath;
                m_pParentData->m_filePathLineNumber = pOther->m_pParentData->m_filePathLineNumber;
            }
        }

        m_cores = pOther->m_cores;
        m_envVariables = pOther->m_envVariables;
        m_startDelay = pOther->m_startDelay;
        m_startAffinity = pOther->m_startAffinity;
        m_collectCSS = pOther->m_collectCSS;
        m_cssInterval = pOther->m_cssInterval;
        m_cssScope = pOther->m_cssScope;
        m_isTimeBasedCssSupportFpo = pOther->m_isTimeBasedCssSupportFpo;
        m_isOtherCpuCssSupportFpo = pOther->m_isOtherCpuCssSupportFpo;
        m_timeBasedCssDepthLevel = pOther->m_timeBasedCssDepthLevel;
        m_otherCpuCssDepthLevel = pOther->m_otherCpuCssDepthLevel;

        m_shouldSaveRawFiles = pOther->m_shouldSaveRawFiles;
        m_cacheLineUtilChecked = pOther->m_cacheLineUtilChecked;
        m_cacheLineUtilEnabled = pOther->m_cacheLineUtilEnabled;
        m_ldstChecked = pOther->m_ldstChecked;
        m_ldstEnabled = pOther->m_ldstEnabled;
        m_shoulApplyFilter = pOther->m_shoulApplyFilter;
        m_utilMask = pOther->m_utilMask;
        m_msInterval = pOther->m_msInterval;

        m_eventsVector.clear();

        for (size_t i = 0; i < pOther->m_eventsVector.size(); i++)
        {
            DcEventConfig config;
            config.pmc = pOther->m_eventsVector[i].pmc;
            config.eventCount = pOther->m_eventsVector[i].eventCount;
            m_eventsVector.push_back(config);
        }

        m_fetchSample = pOther->m_fetchSample;
        m_opSample = pOther->m_opSample;
        m_opCycleCount = pOther->m_opCycleCount;
        m_fetchInterval = pOther->m_fetchInterval;
        m_opInterval = pOther->m_opInterval;

        m_cluSample = pOther->m_cluSample;
        m_cluCycleCount = pOther->m_cluCycleCount;
        m_cluInterval = pOther->m_cluInterval;
    }

}

gtString CPUSessionTreeItemData::sessionDisplayTypeToString(afTreeItemType dispType)
{
    gtString retVal;

    switch (dispType)
    {
        case AF_TREE_ITEM_PROFILE_SESSION:
        case AF_TREE_ITEM_PROFILE_CPU_OVERVIEW:
        {
            retVal = CPUSessionTreeItemData::ms_CPU_PROFILE_OVRVIEW_DISPLAY_STR;
            break;
        }

        case AF_TREE_ITEM_PROFILE_CPU_MODULES:
        {
            retVal = CPUSessionTreeItemData::ms_CPU_PROFILE_MODULES_DISPLAY_STR;
            break;
        }

        case AF_TREE_ITEM_PROFILE_CPU_CALL_GRAPH:
        {
            retVal = CPUSessionTreeItemData::ms_CPU_PROFILE_CALL_GRAPH_DISPLAY_STR;
            break;
        }

        case AF_TREE_ITEM_PROFILE_CPU_FUNCTIONS:
        {
            retVal = CPUSessionTreeItemData::ms_CPU_PROFILE_FUNCTIONS_DISPLAY_STR;
            break;
        }

        case AF_TREE_ITEM_PROFILE_CPU_SOURCE_CODES:
        {
            retVal = SP_STR_SourceCodeNodeText;
            break;
        }

        default:
        {
            GT_ASSERT_EX(false, L"None CPU tree item type, should not get here")
            break;
        }
    }

    return retVal;

}

bool CPUSessionTreeItemData::ShouldCollectCSS(bool checkForLimitations) const
{
    bool retVal = m_collectCSS;

    if (retVal && checkForLimitations)
    {
        // If the user asked to collect CSS, check if this is possible:
        if (ShouldDisableCSS(m_exeFullPath))
        {
            retVal = false;
        }
        else
        {
            // If this is a CLU profile type, disable the CSS collection:
            bool isCLU = m_profileTypeStr.endsWith(PM_profileTypeCLU);

            if (isCLU)
            {
                retVal = false;
            }
        }
    }

    return retVal;
}

// ----------------------------------------------------------------------------------
// Name:        CpuProjectHandler::CpuProjectHandler
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        5/3/2012
// ----------------------------------------------------------------------------------
CpuProjectHandler::CpuProjectHandler() : QObject(), apIEventsObserver(), m_ProjectLoaded(false),
    m_pProfileTreeHandler(nullptr), m_pCurrentXMLSession(nullptr), m_caperfImported(false)
{
    // Register as an events observer:
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);

    onSharedSessionAvailable();

    m_currentSettings.m_pParentData = new afApplicationTreeItemData;
}

// ----------------------------------------------------------------------------------
// Name:        CpuProjectHandler::instance
// Description: Gets an instance
// Author:  AMD Developer Tools Team
// Date:        5/3/2012
// ----------------------------------------------------------------------------------
CpuProjectHandler& CpuProjectHandler::instance()
{
    // If this class single instance was not already created:
    if (nullptr == m_pMySingleInstance)
    {
        // Create it:
        m_pMySingleInstance = new CpuProjectHandler;
        GT_ASSERT(m_pMySingleInstance);
    }

    return *m_pMySingleInstance;
}

// ----------------------------------------------------------------------------------
// Name:        CpuProjectHandler::~CpuProjectHandler
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        5/3/2012
// ----------------------------------------------------------------------------------
CpuProjectHandler::~CpuProjectHandler()
{
    // Unregister as an events observer:
    apEventsHandler::instance().unregisterEventsObserver(*this);

    delete m_currentSettings.m_pParentData;

}

// ----------------------------------------------------------------------------------
// Name:        CpuProjectHandler::onEvent
// Description: Is called when a process event occurs.
// Arguments:   const apEvent& eve
//              bool& vetoEvent
// Author:  AMD Developer Tools Team
// Date:        5/3/2012
// ----------------------------------------------------------------------------------
void CpuProjectHandler::onEvent(const apEvent& eve, bool& vetoEvent)
{
    (void)(vetoEvent); // unused
    // Get the event type:
    apEvent::EventType eventType = eve.eventType();

    switch (eventType)
    {

        case apEvent::GD_MONITORED_OBJECT_ACTIVATED_EVENT:
        {
            onTreeActivationEvent((const apMonitoredObjectsTreeActivatedEvent&)eve);
        }

        default:
            break;
    }

}

// ----------------------------------------------------------------------------------
// Name:        CpuProjectHandler::isProjectLoaded
// Description: Returns whether a project is currently loaded
// Author:  AMD Developer Tools Team
// Date:        5/3/2012
// ----------------------------------------------------------------------------------
bool CpuProjectHandler::isProjectLoaded()
{
    return m_ProjectLoaded;
}

// ----------------------------------------------------------------------------------
// Name:        CpuProjectHandler::getProjectSettings
// Description: Returns the cpu profiling project settings
// Author:  AMD Developer Tools Team
// Date:        5/3/2012
// ----------------------------------------------------------------------------------
CPUSessionTreeItemData* CpuProjectHandler::getProjectSettings()
{
    return &m_currentSettings;
}

// ----------------------------------------------------------------------------------
// Name:        CpuProjectHandler::getProjectSettingsXML
// Description: Writes the settings to the project file
// Author:  AMD Developer Tools Team
// Date:        5/3/2012
// ----------------------------------------------------------------------------------
bool CpuProjectHandler::getProjectSettingsXML(gtString& projectAsXMLString)
{
    gtString numVal;
    projectAsXMLString.append(L"<");
    projectAsXMLString.append(CPU_STR_PROJECT_EXTENSION);
    projectAsXMLString.append(L">");

    writeSession(projectAsXMLString, m_currentSettings, L"Current");

    // Go through the sessions, and write each of it to the project file:
    GT_IF_WITH_ASSERT(m_pProfileTreeHandler != nullptr)
    {
        gtVector<ExplorerSessionId>::const_iterator iter = m_sessions.begin();
        gtVector<ExplorerSessionId>::const_iterator endIter = m_sessions.end();

        for (; iter != endIter; iter++)
        {
            // Get the current item data:
            SessionTreeNodeData* pSessionData = m_pProfileTreeHandler->GetSessionTreeNodeData(*iter);
            GT_IF_WITH_ASSERT(pSessionData != nullptr)
            {
                CPUSessionTreeItemData* pCPUSessionData = qobject_cast<CPUSessionTreeItemData*>(pSessionData);
                GT_IF_WITH_ASSERT(pCPUSessionData != nullptr)
                {
                    writeSession(projectAsXMLString, *pCPUSessionData, L"Profile");
                }
            }
        }
    }

    projectAsXMLString.append(L"</");
    projectAsXMLString.append(CPU_STR_PROJECT_EXTENSION);
    projectAsXMLString.append(L">");

    return true;
}

// ----------------------------------------------------------------------------------
// Name:        CpuProjectHandler::setProjectSettingsXML
// Description: Reads the properties and profiles from the project file
// Author:  AMD Developer Tools Team
// Date:        5/3/2012
// ----------------------------------------------------------------------------------
bool CpuProjectHandler::setProjectSettingsXML(const gtString& projectAsXMLString)
{
    bool retVal = false;

    //Clear out previous sessions
    m_sessions.clear();

    m_currentProject = afProjectManager::instance().currentProjectFilePath().asString();

    //If we're adding a session and it's not been initialized yet, try again
    if ((nullptr == m_pProfileTreeHandler) &&
        (nullptr != ProfileApplicationTreeHandler::instance()))
    {
        m_pProfileTreeHandler = ProfileApplicationTreeHandler::instance();
        onSharedSessionAvailable();
    }

    QXmlInputSource source;
    source.setData(acGTStringToQString(projectAsXMLString));
    QXmlSimpleReader reader ;
    // Connect this object's handler interface to the XML reader
    reader.setContentHandler(this) ;
    reader.setErrorHandler(this);

    retVal = reader.parse(source);

    return retVal;
}

void CpuProjectHandler::writeSession(gtString& projectAsXMLString, const CPUSessionTreeItemData& session, const gtString& type)
{
    gtString numVal;
    projectAsXMLString.append(L"<Session type=\"");
    projectAsXMLString.append(type);
    projectAsXMLString.append(L"\">");
    writeValue(projectAsXMLString, L"profileName", acQStringToGTString(session.m_displayName));
    writeValue(projectAsXMLString, L"profilePath", session.m_pParentData->m_filePath.asString());
    writeValue(projectAsXMLString, L"profileType", acQStringToGTString(session.m_profileTypeStr));
    numVal.makeEmpty();
    numVal.appendFormattedString(L"%d", session.m_isImported);
    writeValue(projectAsXMLString, L"isImported", numVal);
    writeValue(projectAsXMLString, L"launchTarget", acQStringToGTString(session.m_exeFullPath));
    writeValue(projectAsXMLString, L"commandArguments", acQStringToGTString(session.m_commandArguments));
    writeValue(projectAsXMLString, L"workingDir", acQStringToGTString(session.m_workingDirectory));
    writeValue(projectAsXMLString, L"envVariables", session.m_envVariables);
    writeValue(projectAsXMLString, L"startTime", acQStringToGTString(session.m_startTime));
    writeValue(projectAsXMLString, L"endTime", acQStringToGTString(session.m_endTime));
    numVal.makeEmpty();
    numVal.appendFormattedString(L"%llu", session.m_startAffinity);
    writeValue(projectAsXMLString, L"startAffinity", numVal);
    writeBool(projectAsXMLString, L"css", session.ShouldCollectCSS(false));
    numVal.makeEmpty();
    numVal.appendFormattedString(L"%d", session.m_cssInterval);
    writeValue(projectAsXMLString, L"cssInterval", numVal);

    if (CP_CSS_SCOPE_UNKNOWN != session.m_cssScope)
    {
        numVal.makeEmpty();
        numVal.appendFormattedString(L"0x%X", static_cast<unsigned int>(session.m_cssScope));
        writeValue(projectAsXMLString, L"cssScope", numVal);
    }

    writeBool(projectAsXMLString, L"cssFpoTimeBased", session.m_isTimeBasedCssSupportFpo);
    writeBool(projectAsXMLString, L"cssFpo", session.m_isOtherCpuCssSupportFpo);

    numVal.makeEmpty();
    numVal.appendFormattedString(L"%d", session.m_timeBasedCssDepthLevel);
    writeValue(projectAsXMLString, L"cssUnwindTimeBased", numVal);

    numVal.makeEmpty();
    numVal.appendFormattedString(L"%d", session.m_otherCpuCssDepthLevel);
    writeValue(projectAsXMLString, L"cssUnwind", numVal);

    writeBool(projectAsXMLString, L"saveRawFiles", session.m_shouldSaveRawFiles);

    numVal.makeEmpty();
    numVal.appendFormattedString(L"%d", (int)session.m_profileScope);
    writeValue(projectAsXMLString, L"profileScope", numVal);

    numVal.makeEmpty();
    numVal.appendFormattedString(L"%d", session.m_startDelay);
    writeValue(projectAsXMLString, L"startDelay", numVal);
    writeBool(projectAsXMLString, L"startPaused", session.m_isProfilePaused);
    numVal.makeEmpty();
    writeBool(projectAsXMLString, L"profileEntire", session.m_shouldProfileEntireDuration);
    numVal.makeEmpty();
    numVal.appendFormattedString(L"%d", session.m_profileDuration);
    writeValue(projectAsXMLString, L"duration", numVal);
    writeBool(projectAsXMLString, L"terminateProcess", session.m_terminateAfterDataCollectionIsDone);

    // TBP
    numVal.makeEmpty();
    numVal.appendFormattedString(L"%f", session.m_msInterval);
    writeValue(projectAsXMLString, L"msInterval", numVal);

    // Events
    numVal.makeEmpty();
    size_t eventsCount = session.m_eventsVector.size();
    numVal.appendFormattedString(L"%d", eventsCount);
    writeValue(projectAsXMLString, L"EventCount", numVal);

    for (size_t i = 0; i < eventsCount; i++)
    {
        projectAsXMLString.append(L"<");
        projectAsXMLString.append(L"PMC");
        projectAsXMLString.append(L">");

        numVal.makeEmpty();
        numVal.appendFormattedString(L"%llu", session.m_eventsVector[i].eventCount);
        writeValue(projectAsXMLString, L"PMCCount", numVal);
        numVal.makeEmpty();
        numVal.appendFormattedString(L"%llu", session.m_eventsVector[i].pmc.perf_ctl);
        writeValue(projectAsXMLString, L"PMCEvent", numVal);

        projectAsXMLString.append(L"</");
        projectAsXMLString.append(L"PMC");
        projectAsXMLString.append(L">");
    }

    //IBS
    writeBool(projectAsXMLString, L"fetchSample", session.m_fetchSample);
    writeBool(projectAsXMLString, L"opSample", session.m_opSample);
    writeBool(projectAsXMLString, L"opCycleCount", session.m_opCycleCount);
    numVal.makeEmpty();
    numVal.appendFormattedString(L"%ul", session.m_fetchInterval);
    writeValue(projectAsXMLString, L"fetchInterval", numVal);
    numVal.makeEmpty();
    numVal.appendFormattedString(L"%ul", session.m_opInterval);
    writeValue(projectAsXMLString, L"opInterval", numVal);

    //CLU
    writeBool(projectAsXMLString, L"cluSample", session.m_cluSample);
    writeBool(projectAsXMLString, L"cluCycleCount", session.m_cluCycleCount);
    numVal.makeEmpty();
    numVal.appendFormattedString(L"%ul", session.m_cluInterval);
    writeValue(projectAsXMLString, L"cluInterval", numVal);

    //Next version
    writeBool(projectAsXMLString, L"cacheLineUtilChecked", session.m_cacheLineUtilChecked);
    writeBool(projectAsXMLString, L"cacheLineUtilEnabled", session.m_cacheLineUtilEnabled);
    writeBool(projectAsXMLString, L"ldstChecked", session.m_ldstChecked);
    writeBool(projectAsXMLString, L"ldstEnabled", session.m_ldstEnabled);
    writeBool(projectAsXMLString, L"applyFilter", session.m_shoulApplyFilter);
    numVal.makeEmpty();
    numVal.appendFormattedString(L"%d", session.m_utilMask);
    writeValue(projectAsXMLString, L"utilMask", numVal);

    projectAsXMLString.append(L"</Session>");
}

void CpuProjectHandler::writeValue(gtString& projectAsXMLString, const gtString& key, const gtString& value)
{
    projectAsXMLString.appendFormattedString(L"<%ls>%ls</%ls>", key.asCharArray(), value.asCharArray(), key.asCharArray());
}

void CpuProjectHandler::writeBool(gtString& projectAsXMLString, const gtString& key, const bool value)
{
    gtString val;
    val = value ? L"T" : L"F";
    writeValue(projectAsXMLString, key, val);
}

bool CpuProjectHandler::startDocument()
{
    //Clear settings
    delete m_pCurrentXMLSession;
    m_pCurrentXMLSession = nullptr;

    m_pCurrentXMLSession = new CPUSessionTreeItemData;
    m_pCurrentXMLSession->m_pParentData = new afApplicationTreeItemData;

    m_xmlProfile = false;
    m_CpuProfileExtension = false;
    return true;
}

bool CpuProjectHandler::endDocument()
{
    return true;
}

bool CpuProjectHandler::startElement(const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts)
{
    (void)(namespaceURI); // unused
    (void)(localName); // unused

    if ((m_CpuProfileExtension) && (qName == "Session"))
    {
        m_xmlProfile = (atts.value("type") == "Profile");
    }
    else if (qName == QString::fromWCharArray(CPU_STR_PROJECT_EXTENSION))
    {
        m_CpuProfileExtension = true;
    }

    return true;
}

bool CpuProjectHandler::endElement(const QString& namespaceURI, const QString& localName, const QString& qName)
{
    (void)(namespaceURI); // unused
    (void)(localName); // unused
    bool retVal = true;

    if ((m_CpuProfileExtension) && (qName == "Session") && (m_pCurrentXMLSession != nullptr))
    {
        if (m_pCurrentXMLSession->m_pParentData->m_filePath.isEmpty())
        {
            m_pCurrentXMLSession->m_pParentData->m_filePath = getAlphaProfilePath(m_pCurrentXMLSession->m_name);
        }

        if (m_xmlProfile)
        {
            // Add the current profile session to the list of loaded sessions:
            CPUSessionTreeItemData* pCurrentLoadedSession = new CPUSessionTreeItemData;
            pCurrentLoadedSession->m_pParentData = new afApplicationTreeItemData;
            pCurrentLoadedSession->m_pParentData->m_filePath = m_pCurrentXMLSession->m_pParentData->m_filePath;
            pCurrentLoadedSession->m_pParentData->m_filePathLineNumber = (int)AF_TREE_ITEM_PROFILE_CPU_OVERVIEW;

            pCurrentLoadedSession->CopyFrom(m_pCurrentXMLSession, true);

            // Add the session to the tree:
            ExplorerSessionId newId = m_pProfileTreeHandler->AddSession(pCurrentLoadedSession, false);
            GT_IF_WITH_ASSERT(newId != SESSION_ID_ERROR)
            {
                m_sessions.push_back(newId);
            }
        }
        else
        {
            m_currentSettings.CopyFrom(m_pCurrentXMLSession, true);
        }

        //Restore defaults for next session
        CPUSessionTreeItemData empty;
        m_pCurrentXMLSession->CopyFrom(&empty, true);
    }
    else if ((m_CpuProfileExtension) && (qName == "profileName"))
    {
        m_pCurrentXMLSession->m_name = m_xmlContent;
        m_pCurrentXMLSession->m_displayName = m_xmlContent;
    }
    else if ((m_CpuProfileExtension) && (qName == "profilePath"))
    {
        gtString tmp = acQStringToGTString(m_xmlContent);

        if ((tmp[0] != ' ') && m_xmlProfile)
        {
            // Check if the file exists:
            osFilePath profileOutputPath(tmp);

            if (!profileOutputPath.exists())
            {
                osFilePath projectFilePath;

                // Get the path for the CXL files folder:
                afGetUserDataFolderPath(projectFilePath);
                tmp.prepend(osFilePath::osPathSeparator);
                tmp.prepend(projectFilePath.asString());

                profileOutputPath.setFullPathFromString(tmp);
                GT_ASSERT(profileOutputPath.exists());
            }

            m_pCurrentXMLSession->m_pParentData->m_filePath = profileOutputPath;
        }
    }
    else if ((m_CpuProfileExtension) && (qName == "profileType"))
    {
        m_pCurrentXMLSession->m_profileTypeStr = m_xmlContent;
    }
    else if ((m_CpuProfileExtension) && (qName == "isImported"))
    {
        m_pCurrentXMLSession->m_isImported = (bool)m_xmlContent.toUInt();
    }
    else if ((m_CpuProfileExtension) && (qName == "launchTarget"))
    {
        m_pCurrentXMLSession->m_exeFullPath = m_xmlContent;
    }
    else if ((m_CpuProfileExtension) && (qName == "commandArguments"))
    {
        m_pCurrentXMLSession->m_commandArguments = m_xmlContent;
    }
    else if ((m_CpuProfileExtension) && (qName == "workingDir"))
    {
        m_pCurrentXMLSession->m_workingDirectory = m_xmlContent;
    }
    else if ((m_CpuProfileExtension) && (qName == "envVariables"))
    {
        std::wstring     wTmp(m_xmlContent.toStdWString());
        m_pCurrentXMLSession->m_envVariables = wTmp.c_str();
    }
    else if ((m_CpuProfileExtension) && (qName == "startTime"))
    {
        m_pCurrentXMLSession->m_startTime = m_xmlContent;
    }
    else if ((m_CpuProfileExtension) && (qName == "endTime"))
    {
        m_pCurrentXMLSession->m_endTime = m_xmlContent;
    }
    else if ((m_CpuProfileExtension) && (qName == "startAffinity"))
    {
        m_pCurrentXMLSession->m_startAffinity = m_xmlContent.toULongLong();
    }
    else if ((m_CpuProfileExtension) && (qName == "css"))
    {
        m_pCurrentXMLSession->SetShouldCollectCSS(m_xmlContent == "T");
    }
    else if ((m_CpuProfileExtension) && (qName == "cssScope"))
    {
        m_pCurrentXMLSession->m_cssScope = static_cast<CpuProfileCssScope>(m_xmlContent.toUInt(nullptr, 16));
    }
    else if ((m_CpuProfileExtension) && (qName == "cssFpoTimeBased"))
    {
        m_pCurrentXMLSession->m_isTimeBasedCssSupportFpo = (m_xmlContent == "T");
    }
    else if ((m_CpuProfileExtension) && (qName == "cssFpo"))
    {
        m_pCurrentXMLSession->m_isOtherCpuCssSupportFpo = (m_xmlContent == "T");
    }
    else if ((m_CpuProfileExtension) && (qName == "cssUnwindTimeBased"))
    {
        m_pCurrentXMLSession->m_timeBasedCssDepthLevel = m_xmlContent.toUInt();
    }
    else if ((m_CpuProfileExtension) && (qName == "cssUnwind"))
    {
        m_pCurrentXMLSession->m_otherCpuCssDepthLevel = m_xmlContent.toUInt();
    }
    else if ((m_CpuProfileExtension) && (qName == "cssInterval"))
    {
        m_pCurrentXMLSession->m_cssInterval = m_xmlContent.toUInt();
    }
    else if ((m_CpuProfileExtension) && (qName == "saveRawFiles"))
    {
        m_pCurrentXMLSession->m_shouldSaveRawFiles = (m_xmlContent == "T");
    }
    else if ((m_CpuProfileExtension) && (qName == "msInterval"))
    {
        m_pCurrentXMLSession->m_msInterval = m_xmlContent.toFloat();
    }
    else if ((m_CpuProfileExtension) && (qName == "EventCount"))
    {
        int eventsCount = m_xmlContent.toUInt();

        for (int i = 0; i < eventsCount; i++)
        {
            DcEventConfig config;
            m_pCurrentXMLSession->m_eventsVector.push_back(config);
        }

        m_xmlPmcIndex = 0;
    }
    else if ((m_CpuProfileExtension) && (qName == "PMCCount"))
    {
        GT_IF_WITH_ASSERT(m_xmlPmcIndex < m_pCurrentXMLSession->m_eventsVector.size())
        {
            m_pCurrentXMLSession->m_eventsVector[m_xmlPmcIndex].eventCount = m_xmlContent.toULongLong();
        }
    }
    else if ((m_CpuProfileExtension) && (qName == "PMCEvent"))
    {
        m_pCurrentXMLSession->m_eventsVector[m_xmlPmcIndex].pmc.perf_ctl = m_xmlContent.toULongLong();
    }
    else if ((m_CpuProfileExtension) && (qName == "PMC"))
    {
        m_xmlPmcIndex++;
    }
    else if ((m_CpuProfileExtension) && (qName == "fetchSample"))
    {
        m_pCurrentXMLSession->m_fetchSample = (m_xmlContent == "T");
    }
    else if ((m_CpuProfileExtension) && (qName == "opSample"))
    {
        m_pCurrentXMLSession->m_opSample = (m_xmlContent == "T");
    }
    else if ((m_CpuProfileExtension) && (qName == "opCycleCount"))
    {
        m_pCurrentXMLSession->m_opCycleCount = (m_xmlContent == "T");
    }
    else if ((m_CpuProfileExtension) && (qName == "fetchInterval"))
    {
        m_pCurrentXMLSession->m_fetchInterval = m_xmlContent.toULong();
    }
    else if ((m_CpuProfileExtension) && (qName == "opInterval"))
    {
        m_pCurrentXMLSession->m_opInterval = m_xmlContent.toULong();
    }
    else if ((m_CpuProfileExtension) && (qName == "cluSample"))
    {
        m_pCurrentXMLSession->m_cluSample = (m_xmlContent == "T");
    }
    else if ((m_CpuProfileExtension) && (qName == "cluCycleCount"))
    {
        m_pCurrentXMLSession->m_cluCycleCount = (m_xmlContent == "T");
    }
    else if ((m_CpuProfileExtension) && (qName == "cluInterval"))
    {
        m_pCurrentXMLSession->m_cluInterval = m_xmlContent.toULong();
    }
    else if ((m_CpuProfileExtension) && (qName == "cacheLineUtilChecked"))
    {
        m_pCurrentXMLSession->m_cacheLineUtilChecked = (m_xmlContent == "T");
    }
    else if ((m_CpuProfileExtension) && (qName == "cacheLineUtilEnabled"))
    {
        m_pCurrentXMLSession->m_cacheLineUtilEnabled = (m_xmlContent == "T");
    }
    else if ((m_CpuProfileExtension) && (qName == "ldstChecked"))
    {
        m_pCurrentXMLSession->m_ldstChecked = (m_xmlContent == "T");
    }
    else if ((m_CpuProfileExtension) && (qName == "ldstEnabled"))
    {
        m_pCurrentXMLSession->m_ldstEnabled = (m_xmlContent == "T");
    }
    else if ((m_CpuProfileExtension) && (qName == "applyFilter"))
    {
        m_pCurrentXMLSession->m_shoulApplyFilter = (m_xmlContent == "T");
    }
    else if ((m_CpuProfileExtension) && (qName == "utilMask"))
    {
        m_pCurrentXMLSession->m_utilMask = m_xmlContent.toUInt();
    }
    else if (qName == QString::fromWCharArray(CPU_STR_PROJECT_EXTENSION))
    {
        m_CpuProfileExtension = false;
    }

    else if ((m_CpuProfileExtension) && (qName == "systemWide"))
    {
        // Backwards compatibility - this field was moved to shared profile extension:
        if (m_xmlContent == "T")
        {
            m_pCurrentXMLSession->m_profileScope = (m_xmlContent == "T") ? PM_PROFILE_SCOPE_SYS_WIDE : PM_PROFILE_SCOPE_SINGLE_EXE;
        }
    }
    else if ((m_CpuProfileExtension) && (qName == "profileScope"))
    {
        int scopeVal = m_xmlContent.toUInt();
        m_pCurrentXMLSession->m_profileScope = (ProfileSessionScope)scopeVal;
    }
    else if ((m_CpuProfileExtension) && (qName == "startDelay"))
    {
        // Backwards compatibility - this field was moved to shared profile extension:
        m_pCurrentXMLSession->m_startDelay = m_xmlContent.toUInt();
    }
    else if ((m_CpuProfileExtension) && (qName == "startPaused"))
    {
        // Backwards compatibility - this field was moved to shared profile extension:
        m_pCurrentXMLSession->m_isProfilePaused = (m_xmlContent == "T");
    }
    else if ((m_CpuProfileExtension) && (qName == "profileEntire"))
    {
        // Backwards compatibility - this field was moved to shared profile extension:
        m_pCurrentXMLSession->m_shouldProfileEntireDuration = (m_xmlContent == "T");
    }
    else if ((m_CpuProfileExtension) && (qName == "duration"))
    {
        // Backwards compatibility - this field was moved to shared profile extension:
        m_pCurrentXMLSession->m_profileDuration = m_xmlContent.toUInt();
    }
    else if ((m_CpuProfileExtension) && (qName == "terminateProcess"))
    {
        // Backwards compatibility - this field was moved to shared profile extension:
        m_pCurrentXMLSession->m_terminateAfterDataCollectionIsDone = (m_xmlContent == "T");
    }

    else
    {
        // Dump the unrecognized tag to the log file and to the Output View (only in debug build)
        QString msgErrorReadingProjectXML = "Error reading project file. Unrecognized tag: local name = ";
        msgErrorReadingProjectXML += localName;
        msgErrorReadingProjectXML += ", name = ";
        msgErrorReadingProjectXML += qName;
        msgErrorReadingProjectXML += "\n";
#if (AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD)
        afApplicationCommands::instance()->AddStringToInformationView(msgErrorReadingProjectXML);
#endif // (AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD)
        std::wstring msgToLog = msgErrorReadingProjectXML.toStdWString();
        OS_OUTPUT_DEBUG_LOG(msgToLog.c_str() , OS_DEBUG_LOG_ERROR);
    }

    return retVal;
}

bool CpuProjectHandler::characters(const QString& ch)
{
    m_xmlContent = ch;
    return true;
}

const osFilePath& CpuProjectHandler::getAlphaProfilePath(const QString& profileName)
{
    static osFilePath retPath;

    //Get the base directory of the project file
    osDirectory baseDir;
    afProjectManager::instance().currentProjectFilePath().getFileDirectory(baseDir);
    gtString buildPath;
    buildPath = baseDir.directoryPath().asString();

    if (!buildPath.isEmpty())
    {
        // Add the profile directory and session instance directory and file name
        buildPath.appendFormattedString(L"/ProfileOutput/%ls/%ls%ls", profileName.toStdWString().data(), profileName.toStdWString().data(), DATA_EXT_STR.toStdWString().data());
        retPath.setFullPathFromString(buildPath);
    }

    return retPath;
}


void CpuProjectHandler::addSession(CPUSessionTreeItemData* pSessionData, bool displaySession)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((pSessionData != nullptr) && (pSessionData->m_pParentData != nullptr))
    {
        //If we're adding a session and it's not been initialized yet, try again
        if ((nullptr == m_pProfileTreeHandler) && (nullptr != ProfileApplicationTreeHandler::instance()))
        {
            m_pProfileTreeHandler = ProfileApplicationTreeHandler::instance();
            onSharedSessionAvailable();
        }

        if (nullptr != m_pProfileTreeHandler)
        {
            //If the project name is empty, like VS, sort by the target executable
            if (pSessionData->m_projectName.isEmpty())
            {
                gtString exeName;
                afProjectManager::instance().currentProjectSettings().executablePath().getFileName(exeName);
                pSessionData->m_projectName = QString::fromWCharArray(exeName.asCharArray());
            }

            // Add this session to the tree:
            ExplorerSessionId newId = m_pProfileTreeHandler->AddSession(pSessionData, displaySession);
            GT_IF_WITH_ASSERT(newId != SESSION_ID_ERROR)
            {
                m_sessions.push_back(newId);
            }
        }
    }
}

void CpuProjectHandler::onSharedSessionAvailable()
{
    m_pProfileTreeHandler = ProfileApplicationTreeHandler::instance();

    bool rc = connect(m_pProfileTreeHandler, SIGNAL(SessionDeleted(ExplorerSessionId, SessionExplorerDeleteType, bool&)), SLOT(onRemoveSession(ExplorerSessionId, SessionExplorerDeleteType, bool&)));
    GT_ASSERT(rc);

    rc = connect(m_pProfileTreeHandler, SIGNAL(FileImported(const QString&, bool&)), SLOT(onImportSession(const QString&, bool&)));
    GT_ASSERT(rc);

    rc = connect(this, SIGNAL(FileImportedComplete()), m_pProfileTreeHandler, SLOT(onFileImportedComplete()));
    GT_ASSERT(rc);

    rc = connect(m_pProfileTreeHandler, SIGNAL(SessionRenamed(SessionTreeNodeData*, const osFilePath&, const osDirectory&)), SLOT(onSessionRename(SessionTreeNodeData*, const osFilePath&, const osDirectory&)));
    GT_ASSERT(rc);

    rc = connect(&afProjectManager::instance(), SIGNAL(OnClearCurrentProjectSettings()), SLOT(OnClearCurrentProjectSettings()));
    GT_ASSERT(rc);

    m_pProfileTreeHandler->AddImportFileFilter("CPU Profiles", "*.ebp", PM_STR_PROFILE_MODE);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    //Only import raw files in debug mode
    m_pProfileTreeHandler->AddImportFileFilter("Raw CPU Profiles", "*.prd", PM_STR_PROFILE_MODE);
#else
    //Only import raw files in debug mode
    m_pProfileTreeHandler->AddImportFileFilter("Raw CPU Profiles", "*.caperf", PM_STR_PROFILE_MODE);
#endif
}

CPUSessionTreeItemData* CpuProjectHandler::findSessionItemData(ExplorerSessionId sharedId)
{
    CPUSessionTreeItemData* pRetVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pProfileTreeHandler != nullptr)
    {
        SessionTreeNodeData* pData = m_pProfileTreeHandler->GetSessionTreeNodeData(sharedId);

        if (pData != nullptr)
        {
            pRetVal = qobject_cast<CPUSessionTreeItemData*>(pData);
        }
    }

    return pRetVal;
}

void CpuProjectHandler::onTreeActivationEvent(const apMonitoredObjectsTreeActivatedEvent& activationEvent)
{
    // Get the pItem data;
    afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)activationEvent.selectedItemData();

    if (pItemData != nullptr)
    {
        if (pItemData->m_itemType == AF_TREE_ITEM_PROFILE_CPU_SOURCE_CODE)
        {
            // See if the item type if of GPU session:
            CPUSessionTreeItemData* pCPUSessionData = qobject_cast<CPUSessionTreeItemData*>(pItemData->extendedItemData());

            // Sanity check:
            GT_IF_WITH_ASSERT((pCPUSessionData != nullptr) && (m_pProfileTreeHandler != nullptr))
            {
                // Find the related session file path
                afApplicationTreeItemData* pSessionItemData = m_pProfileTreeHandler->FindParentSessionItemData(pCPUSessionData->m_pParentData);
                // Sanity check:
                GT_IF_WITH_ASSERT(pSessionItemData != nullptr)
                {
                    gtString sessionFilePath = pSessionItemData->m_filePath.asString();
                    gtString moduleFilePath = acQStringToGTString(pCPUSessionData->m_exeFullPath);
                    AmdtCpuProfiling::instance().sessionViewCreator()->openSession(sessionFilePath, moduleFilePath, pItemData->m_itemType);
                }
            }
        }
    }
}

void CpuProjectHandler::onRemoveSession(ExplorerSessionId sharedId, SessionExplorerDeleteType deleteType, bool& sessionDeleted)
{
    (void)(sessionDeleted); // unused
    (void)(deleteType); // unused
    osFilePath oldSession;
    CPUSessionTreeItemData* pData = findSessionItemData(sharedId);

    // Ignore sessions that aren't mine:
    if ((pData != nullptr) && (pData->m_pParentData != nullptr))
    {
        oldSession.setFullPathFromString(pData->m_pParentData->m_filePath.asString());

        // Remove the profile session from the sessions vector:
        int index = -1;

        for (int i = 0; i < (int)m_sessions.size(); i++)
        {
            if (m_sessions[i] == sharedId)
            {
                index = i;
                break;
            }
        }

        if ((index >= 0) && (index < (int)m_sessions.size()))
        {
            m_sessions.removeItem(index);
        }

        //Close the session window, if it's open
        AmdtCpuProfiling::instance().sessionViewCreator()->closeSessionBeforeDeletion(oldSession.asString());
    }
}



bool CpuProjectHandler::isProfileNameValid(const QString& profileName, QString& invalidMessageStr)
{
    bool bRet = true;
    bRet = (!profileName.isEmpty());

    if (!bRet)
    {
        invalidMessageStr = "The session name cannot be empty.";
    }

    if (bRet)
    {
        //Check if the Session name contains bad characters for a path
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        QString pattern("[.*?/:<>%|\\]");
#else
        QString pattern("[/]");
#endif
        QRegExp rex(pattern);
        bRet = (rex.indexIn(profileName) == -1);

        if (!bRet)
        {
            invalidMessageStr = "The session name must not contain any of the following characters: ";
            //strip off reg ex brackets
            QString temp(pattern.mid(1, (pattern.size() - 2)));
            invalidMessageStr.append(temp);
        }
    }

    if (bRet)
    {
        //Is the session name too long for a path
        bRet = (OS_MAX_PATH > getAlphaProfilePath(profileName).asString().length());

        if (!bRet)
        {
            invalidMessageStr = "The session name is too long.";
        }
    }

    return bRet;
}

void CpuProjectHandler::renameFilesInDir(osDirectory& dir, const gtString& oldName, const gtString& newName)
{
    osFilePath newFileName;
    gtString matchRename(oldName);
    matchRename.append(OS_ALL_CONTAINED_FILES_SEARCH_STR);
    gtList<osFilePath> filePaths;

    //Rename all files starting with the session name
    if (dir.getContainedFilePaths(matchRename, osDirectory::SORT_BY_NAME_DESCENDING, filePaths))
    {
        gtList<osFilePath>::const_iterator it = filePaths.begin();
        gtList<osFilePath>::const_iterator endIt = filePaths.end();

        for (; it != endIt; it++)
        {
            osDirectory renameFile(*it);
            newFileName.setFromOtherPath(*it);

            //Just in case there are multiple extensions, like bob.ebp.osv, 'bob.ebp' is the FileName
            gtString filePart;
            newFileName.getFileName(filePart);
            filePart.replace(oldName, newName);

            newFileName.setFileName(filePart);
            renameFile.rename(newFileName.asString());
        }
    }
}

void CpuProjectHandler::onImportSession(const QString& strFileName, bool& imported)
{
    // If the import was canceled, ignore it:
    if (strFileName.isEmpty())
    {
        emit FileImportedComplete();
    }
    else
    {
        // Find the imported file path:
        osFilePath importedSessionFilePath(acQStringToGTString(strFileName));
        gtString importedFileExtension;
        importedSessionFilePath.getFileExtension(importedFileExtension);
        gtString caperfString(L"caperf");

        if (0 == importedFileExtension.compareNoCase(caperfString))
        {
            m_caperfImported = true;
        }

        gtString importProfile;
        importedSessionFilePath.getFileName(importProfile);
        osDirectory projectPath;
        afProjectManager::instance().currentProjectFilePath().getFileDirectory(projectPath);
        gtString projName = afProjectManager::instance().currentProjectSettings().projectName();
        gtString comparison(DATA_EXT);
        comparison.removeChar(L'.');

        bool isDataExt = (importedFileExtension == comparison);

        if (!isDataExt)
        {
            handleNonEBPImport(importedSessionFilePath);
        }
        else
        {
            // Allocate a new session data:

            CPUSessionTreeItemData* pImportSessionData = new CPUSessionTreeItemData;
            pImportSessionData->m_pParentData = new afApplicationTreeItemData;

            pImportSessionData->m_displayName = acGTStringToQString(importProfile);
            const QString suffixImport(" import");

            if (m_caperfImported && (!pImportSessionData->m_displayName.endsWith(suffixImport)))
            {
                QString str = pImportSessionData->m_displayName;
                int lastIndex = str.lastIndexOf(suffixImport);
                pImportSessionData->m_displayName = str.left(lastIndex);
            }

            gtString profileFileName;
            profileFileName.fromASCIIString(pImportSessionData->m_displayName.toLatin1().data());
            osDirectory baseDir;
            ProfileApplicationTreeHandler::instance()->GetNextSessionNameAndDir(projName, projectPath, profileFileName, baseDir);

            // Make it relative to the project:
            pImportSessionData->m_pParentData->m_filePath.setFileExtension(DATA_EXT);

            osDirectory oldDir;
            importedSessionFilePath.getFileDirectory(oldDir);

            if (oldDir.directoryPath() != baseDir.directoryPath())
            {
                gtList<gtString> noFilter;
                oldDir.copyFilesToDirectory(baseDir.directoryPath().asString(), noFilter);

                // Update the imported files to the imported session name:
                renameFilesInDir(baseDir, importProfile, profileFileName);

                // Update the path for the rename:
                pImportSessionData->m_pParentData->m_filePath = baseDir.directoryPath();

                // Make it relative to the project:
                pImportSessionData->m_pParentData->m_filePath.setFileExtension(L"ebp");
                pImportSessionData->m_pParentData->m_filePath.setFileName(profileFileName);

                // Read the current session cache:
                CacheFileMap sessionFileCache;
                QString newSessionDir = acGTStringToQString(baseDir.directoryPath().asString());
                QString oldSessionDir = acGTStringToQString(oldDir.directoryPath().asString());
                bool rc = ReadSessionCacheFileMap(newSessionDir, sessionFileCache);
                GT_IF_WITH_ASSERT(rc)
                {
                    // Go through the mapped files and fix their paths with the new session folder:
                    CacheFileMap::Iterator iter;

                    for (iter = sessionFileCache.begin(); iter != sessionFileCache.end(); ++iter)
                    {
                        QString cachedFilePath = iter.value();

                        // When we import a session from the local machine, then we expect to find the source folder in each of the mapped cached files:
                        if (cachedFilePath.startsWith(oldSessionDir))
                        {
                            cachedFilePath.replace(oldSessionDir, newSessionDir);
                            sessionFileCache[iter.key()] = cachedFilePath;
                        }
                        else
                        {
                            // The file is copied from other machine and imported:
                            osFilePath oldFilePath(acQStringToGTString(cachedFilePath));
                            oldFilePath.setFileDirectory(acQStringToGTString(newSessionDir));
                            oldFilePath.appendSubDirectory(L"cache");
                            sessionFileCache[iter.key()] = acGTStringToQString(oldFilePath.asString());
                        }
                    }

                    // Write the updated cache file map:
                    rc = WriteSessionCacheFileMap(newSessionDir, sessionFileCache);
                    GT_ASSERT(rc);
                }
            }

            CpuProfileReader profileReader;

            if (profileReader.open(pImportSessionData->m_pParentData->m_filePath.asString().asCharArray()))
            {
                // Get the profile session data from the profile reader:
                pImportSessionData->m_profileTypeStr = acGTStringToQString(profileReader.getProfileInfo()->m_profType);

                if (pImportSessionData->m_profileTypeStr.isEmpty())
                {
                    gtString profileTypeStr = ProfileConfigs::instance().getProfileTypeByEventConfigs(profileReader.getProfileInfo()->m_eventVec);
                    pImportSessionData->m_profileTypeStr = acGTStringToQString(profileTypeStr);
                }

                pImportSessionData->m_commandArguments = acGTStringToQString(profileReader.getProfileInfo()->m_cmdArguments);
                pImportSessionData->m_envVariables = profileReader.getProfileInfo()->m_envVariables;
                pImportSessionData->m_exeFullPath = acGTStringToQString(profileReader.getProfileInfo()->m_targetPath);
                pImportSessionData->m_workingDirectory = acGTStringToQString(profileReader.getProfileInfo()->m_wrkDirectory);

                gtString fileName;
                osFilePath importedTargetPath(profileReader.getProfileInfo()->m_targetPath);

                if (importedTargetPath.isEmpty())
                {
                    // Use the profile read ebp path:
                    importedTargetPath = pImportSessionData->m_pParentData->m_filePath;
                }

                importedTargetPath.getFileName(fileName);
                pImportSessionData->m_projectName = acGTStringToQString(fileName);

                // Fill the imported session name:
                pImportSessionData->m_name = pImportSessionData->m_displayName;

                if (pImportSessionData->m_name.isEmpty())
                {
                    pImportSessionData->m_name = acGTStringToQString(profileReader.getProfileInfo()->m_timeStamp);
                }

                QString namePostfix;

                if (fileName == afProjectManager::instance().currentProjectSettings().projectName())
                {
                    namePostfix.sprintf(" (%s)", PM_STR_ImportedSessionPostfix);

                }
                else if (m_caperfImported)
                {
                    namePostfix.sprintf(" (%s)", PM_STR_ImportedSessionPostfix);
                }
                else
                {
                    namePostfix.sprintf(" (%s - %s)", PM_STR_ImportedSessionPostfix, acGTStringToQString(fileName).toLatin1().data());
                }

                // For an imported Session read the affinity data from the RI file
                osFilePath riFilePath;
                riFilePath.setFromOtherPath(pImportSessionData->m_pParentData->m_filePath);
                riFilePath.clearFileExtension();
                riFilePath.setFileExtension(L"ri");
                RunInfo runInfo;
                HRESULT hr = fnReadRIFile(riFilePath.asString().asCharArray(), &runInfo);

                if (hr == S_OK)
                {
                    pImportSessionData->m_startAffinity = runInfo.m_cpuAffinity;
                }

                // Initialize the session start + end time:
                pImportSessionData->m_startTime = acGTStringToQString(runInfo.m_profStartTime);
                pImportSessionData->m_endTime = acGTStringToQString(runInfo.m_profEndTime);

                pImportSessionData->m_name.append(namePostfix);
                pImportSessionData->m_displayName = pImportSessionData->m_name;
                pImportSessionData->m_cores = profileReader.getProfileInfo()->m_numCpus;
                pImportSessionData->SetShouldCollectCSS(profileReader.getProfileInfo()->m_isCSSEnabled);

                if (pImportSessionData->IsTimeBasedProfiling())
                {
                    pImportSessionData->m_timeBasedCssDepthLevel = profileReader.getProfileInfo()->m_cssUnwindDepth;
                    pImportSessionData->m_isTimeBasedCssSupportFpo = profileReader.getProfileInfo()->m_isCssSupportFpo;
                }
                else
                {
                    pImportSessionData->m_otherCpuCssDepthLevel = profileReader.getProfileInfo()->m_cssUnwindDepth;
                    pImportSessionData->m_isOtherCpuCssSupportFpo = profileReader.getProfileInfo()->m_isCssSupportFpo;
                }

                osTime startTime, endTime;
                startTime.setFromDateTimeString(osTime::LOCAL, profileReader.getProfileInfo()->m_profStartTime, osTime::NAME_SCHEME_FILE);
                endTime.setFromDateTimeString(osTime::LOCAL, profileReader.getProfileInfo()->m_profEndTime, osTime::NAME_SCHEME_FILE);
                pImportSessionData->m_profileDuration = endTime.secondsFrom1970() - startTime.secondsFrom1970();

                pImportSessionData->m_isImported = true;
                // Add the imported session to the tree:
                addSession(pImportSessionData, true);
                m_caperfImported = false;
                //Save the session list to the project
                afApplicationCommands::instance()->OnFileSaveProject();

                imported = true;
            }
            else
            {
                profileReader.close();

                //Remove the copied directory
                if (baseDir.exists())
                {
                    baseDir.deleteRecursively();
                }

                gtString profileName;
                importedSessionFilePath.getFileName(profileName);
                QString qprofileName = acGTStringToQString(profileName);
                pImportSessionData->m_name = qprofileName;
                pImportSessionData->m_displayName = qprofileName;
                QString msg = QString("The file selected for import is not valid: %1").arg(acGTStringToQString(importProfile));

                //Warn the user the import was rejected
                acMessageBox::instance().warning(CPU_PROF_MESSAGE, msg, QMessageBox::Ok);

                //Add the attempted import to the log file
                msg.append(": ");
                msg.append(acGTStringToQString(importedSessionFilePath.asString()));
                OS_OUTPUT_DEBUG_LOG(acQStringToGTString(msg).asCharArray(), OS_DEBUG_LOG_ERROR);
                emit FileImportedComplete();
            }
        }
    }
}

bool CpuProjectHandler::profileExists(const QString& profileName, QString& invalidMessage)
{
    bool retVal = false;

    gtVector<ExplorerSessionId>::iterator it = m_sessions.begin();
    gtVector<ExplorerSessionId>::iterator itEnd = m_sessions.end();

    for (; it != itEnd; it++)
    {
        CPUSessionTreeItemData* pData = findSessionItemData(*it);

        if (pData != nullptr)
        {
            if (pData->m_name == profileName)
            {
                invalidMessage.clear();
                invalidMessage.sprintf("Profile '%s' already exists", profileName.toLatin1().data());
                retVal = true;
                break;
            }
        }
    }

    return retVal;
}


const gtString& CpuProjectHandler::getProfileName(const gtString& profilePath)
{
    static gtString retName;
    osFilePath target(profilePath);

    gtVector<ExplorerSessionId>::iterator it = m_sessions.begin();
    gtVector<ExplorerSessionId>::iterator itEnd = m_sessions.end();

    for (; it != itEnd; it++)
    {
        CPUSessionTreeItemData* pData = findSessionItemData(*it);

        if ((pData != nullptr) && (pData->m_pParentData != nullptr))
        {
            if (osFilePath(pData->m_pParentData->m_filePath.asString()) == target)
            {
                retName.fromASCIIString(pData->m_name.toLatin1().data());
                break;
            }
        }
    }

    return retName;
}

void CpuProjectHandler::handleNonEBPImport(const osFilePath& importedFilePath)
{
    gtString importProfileName;
    importedFilePath.getFileName(importProfileName);
    gtString importedFileExtension;
    importedFilePath.getFileExtension(importedFileExtension);
    gtString projName = afProjectManager::instance().currentProjectSettings().projectName();

    gtString win_comparison = PRD_EXT;
    gtString lin_comparison = CAPERF_EXT;
    win_comparison.removeChar(L'.');
    lin_comparison.removeChar(L'.');

    // Check to see if it's a Cpu raw data file:
    if (importedFileExtension != win_comparison && importedFileExtension != lin_comparison)
    {
        emit FileImportedComplete();
        return;
    }
    else
    {
        // Handle raw import:
        ReaderHandle* pHandle = nullptr;

        HRESULT hRet = fnOpenProfile(importedFilePath.asString().asCharArray(), &pHandle);

        if (!SUCCEEDED(hRet))
        {
            gtString msg;
            msg.appendFormattedString(CP_strFailedToOpenRawProfileFile, importedFilePath.asString().asCharArray(), gtGetErrorString(hRet), hRet);
            OS_OUTPUT_DEBUG_LOG(msg.asCharArray(), OS_DEBUG_LOG_ERROR);
            acMessageBox::instance().critical(CPU_PROF_MESSAGE, QString::fromWCharArray(msg.asCharArray()), QMessageBox::Ok);
            emit FileImportedComplete();
        }

        if (SUCCEEDED(hRet))
        {
            //Launch another thread to monitor the data translation

            gtString profileFileName = importProfileName;
            osDirectory baseDir;
            osDirectory projectPath;
            afProjectManager::instance().currentProjectFilePath().getFileDirectory(projectPath);
            ProfileApplicationTreeHandler::instance()->GetNextSessionNameAndDir(projName, projectPath, importProfileName, baseDir);

            gtString translatedFile = baseDir.directoryPath().asString();
            translatedFile.appendFormattedString(L"/%ls%ls", profileFileName.asCharArray(), DATA_EXT);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
            //Windows needs the path including the data file
            CommandsHandler::instance()->startTranslating(pHandle, translatedFile, true);
#else
            gtString sessionDir = osFilePath(translatedFile).fileDirectoryAsString();
            CommandsHandler::instance()->startTranslating(pHandle, sessionDir, true);
#endif
        }
    }
}

void CpuProjectHandler::onSessionRename(SessionTreeNodeData* pRenamedSessionData, const osFilePath& oldSessionFilePath, const osDirectory& oldSessionDirectory)
{
    (void)(oldSessionFilePath); // unused
    CPUSessionTreeItemData* pRenamedCPUSessionData = qobject_cast<CPUSessionTreeItemData*>(pRenamedSessionData);

    // Update the title of the window if it is opened:
    if ((pRenamedCPUSessionData != nullptr) && (pRenamedCPUSessionData->m_pParentData != nullptr))
    {
        AmdtCpuProfiling::instance().sessionViewCreator()->updateTitleString(pRenamedCPUSessionData);

        // Update the cache map file with the new paths:

        // Read the current session cache:
        CacheFileMap sessionFileCache;
        QString newSessionDir = acGTStringToQString(pRenamedSessionData->SessionDir().directoryPath().asString());
        QString oldSessionDir = acGTStringToQString(oldSessionDirectory.directoryPath().asString());
        bool rc = ReadSessionCacheFileMap(newSessionDir, sessionFileCache);
        GT_IF_WITH_ASSERT(rc)
        {
            // Go through the mapped files and fix their paths with the new session folder:
            CacheFileMap::Iterator iter;

            for (iter = sessionFileCache.begin(); iter != sessionFileCache.end(); ++iter)
            {
                QString cachedFilePath = iter.value();
                GT_IF_WITH_ASSERT(cachedFilePath.startsWith(oldSessionDir))
                {
                    cachedFilePath.replace(oldSessionDir, newSessionDir);
                    sessionFileCache[iter.key()] = cachedFilePath;
                }
            }

            // Write the updated cache file map:
            rc = WriteSessionCacheFileMap(newSessionDir, sessionFileCache);
            GT_ASSERT(rc);
        }

        // Rename all open source code views with the new session name:

        // Find the session item data:
        afApplicationTreeItemData* pSessionItemData = ProfileApplicationTreeHandler::instance()->FindItemByProfileDisplayName(pRenamedSessionData->m_displayName);
        GT_IF_WITH_ASSERT(pSessionItemData != nullptr)
        {
            // Get the source codes item data:
            afApplicationTreeItemData* pSourceCodesNodeItemData = ProfileApplicationTreeHandler::instance()->FindSessionChildItemData(pSessionItemData, AF_TREE_ITEM_PROFILE_CPU_SOURCE_CODES);

            if (pSourceCodesNodeItemData != nullptr)
            {
                // If there are source code windows opened:
                if (pSourceCodesNodeItemData->m_pTreeWidgetItem != nullptr)
                {
                    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
                    GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
                    {
                        afApplicationTree* pTree = pApplicationCommands->applicationTree();
                        GT_IF_WITH_ASSERT(pTree != nullptr)
                        {
                            int sourceCodeWindowsCount = pSourceCodesNodeItemData->m_pTreeWidgetItem->childCount();

                            for (int i = 0 ; i < sourceCodeWindowsCount; i++)
                            {
                                QTreeWidgetItem* pChild = pSourceCodesNodeItemData->m_pTreeWidgetItem->child(i);
                                afApplicationTreeItemData* pSourceCodeItemData = pTree->getTreeItemData(pChild);

                                if (pSourceCodeItemData != nullptr)
                                {
                                    SessionTreeNodeData* pSessionData = qobject_cast<SessionTreeNodeData*>(pSourceCodeItemData->extendedItemData());

                                    if (pSessionData != nullptr)
                                    {
                                        // Set the new name on the item data:
                                        pSessionData->m_displayName = pRenamedCPUSessionData->m_displayName;
                                        pSourceCodeItemData->m_filePath = pRenamedCPUSessionData->m_pParentData->m_filePath;
                                        pSessionData->m_name = pRenamedCPUSessionData->m_name;

                                        // Rename the exe full path (if this is a cached exe):
                                        osFilePath oldExePath(acQStringToGTString(pSessionData->m_exeFullPath));
                                        gtString exeOldDir = oldExePath.fileDirectoryAsString();

                                        if (exeOldDir.startsWith(oldSessionDirectory.directoryPath().asString()))
                                        {
                                            exeOldDir.replace(exeOldDir, pRenamedSessionData->SessionDir().directoryPath().asString());
                                            oldExePath.setFileDirectory(exeOldDir);
                                            pSessionData->m_exeFullPath = acGTStringToQString(oldExePath.asString());
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Rename all children's file path
            for (int i = AF_TREE_ITEM_PROFILE_CPU_OVERVIEW; i < AF_TREE_ITEM_PROFILE_CPU_FUNCTIONS; i++)
            {
                afApplicationTreeItemData* pChildData = ProfileApplicationTreeHandler::instance()->FindSessionChildItemData(pSessionItemData, (afTreeItemType)i);

                if (pChildData != nullptr)
                {
                    pChildData->m_filePath = pRenamedSessionData->m_pParentData->m_filePath;
                    pChildData->m_filePathLineNumber = i;
                }
            }

        }
    }
}

void CpuProjectHandler::emitFileImportedComplete()
{
    emit FileImportedComplete();
}

void CpuProjectHandler::OnClearCurrentProjectSettings()
{
    // Clear all loaded sessions:
    m_xmlProfile = false;
    delete m_pCurrentXMLSession;
    m_pCurrentXMLSession = nullptr;
    m_sessions.clear();
}

