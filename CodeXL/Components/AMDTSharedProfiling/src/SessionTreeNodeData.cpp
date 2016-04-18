//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SessionTreeNodeData.cpp
///
//==================================================================================

// TinyXml:
#include <tinyxml.h>

// Qt:
#include <QtCore>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/src/afUtils.h>

#include <inc/SharedProfileManager.h>
#include <inc/SessionTreeNodeData.h>
#include <inc/StringConstants.h>

SessionTreeNodeData::SessionTreeNodeData()
    : afTreeDataExtension(),
      m_name(""), m_displayName(""), m_sessionId(SESSION_ID_ERROR), m_profileTypeStr(""),
      m_isImported(false), m_projectName(""), m_commandArguments(""), m_workingDirectory(""), m_exeName(""), m_exeFullPath(""), m_envVariables(L""),
      m_startTime(""), m_endTime(""), m_profileScope(PM_PROFILE_SCOPE_SINGLE_EXE), m_shouldProfileEntireDuration(true), m_isProfilePaused(false),
      m_startDelay(-1), m_profileDuration(-1), m_terminateAfterDataCollectionIsDone(false), m_isSessionRunning(false)
{
    // Set the profile type to be the current profile type:
    m_profileTypeStr = acGTStringToQString(SharedProfileManager::instance().selectedSessionTypeName());

    // For power sessions, default scope will be PM_PROFILE_SCOPE_SYS_WIDE_FOCUS_ON_EXE:
    if (m_profileTypeStr == PM_profileTypePowerProfilePrefix)
    {
        m_profileScope = PM_PROFILE_SCOPE_SYS_WIDE_FOCUS_ON_EXE;
    }
}


SessionTreeNodeData::SessionTreeNodeData(ExplorerSessionId sessionId, const QString& strNodeSuffix, bool isImported)
    : afTreeDataExtension(),
      m_isImported(isImported)
{
    m_workingDirectory.clear();
    m_exeName.clear();
    m_sessionId = sessionId;
    m_envVariables.makeEmpty();

    if (strNodeSuffix.isEmpty())
    {
        // Set the profile type to be the current profile type:
        m_profileTypeStr = acGTStringToQString(SharedProfileManager::instance().selectedSessionTypeName());
    }
    else
    {
        m_profileTypeStr = QString(" - %1").arg(strNodeSuffix);
    }

    /// Profile scope (single application by default):
    m_profileScope = PM_PROFILE_SCOPE_SINGLE_EXE;

    if (m_profileTypeStr == PM_profileTypePowerProfile)
    {
        m_profileScope = PM_PROFILE_SCOPE_SYS_WIDE;
    }

    // For power sessions, default scope will be PM_PROFILE_SCOPE_SYS_WIDE_FOCUS_ON_EXE:
    if (m_profileTypeStr == PM_profileTypePowerProfilePrefix)
    {
        m_profileScope = PM_PROFILE_SCOPE_SYS_WIDE_FOCUS_ON_EXE;
    }

    /// Profile entire duration:
    m_shouldProfileEntireDuration = true;

    /// Start profiling with paused data collection:
    m_isProfilePaused = false;

    /// Start after (seconds):
    m_startDelay = -1;

    /// End after (seconds):
    m_profileDuration = -1;

    // Should terminate after data collection is done:
    m_terminateAfterDataCollectionIsDone = false;

    // By default the session is not running:
    m_isSessionRunning = false;
}

SessionTreeNodeData::SessionTreeNodeData(const SessionTreeNodeData& other)
{
    m_name = other.m_name;
    m_displayName = other.m_displayName;

    // NOTICE: Do not copy the session id! the session id is unique for the new session data
    // m_sessionId = other.m_sessionId;

    m_profileTypeStr = other.m_profileTypeStr;
    m_projectName = other.m_projectName;
    m_isImported = other.m_isImported;
    m_commandArguments = other.m_commandArguments;
    m_workingDirectory = other.m_workingDirectory;
    m_exeName = other.m_exeName;
    m_exeFullPath = other.m_exeFullPath;
    m_profileScope = other.m_profileScope;
    m_shouldProfileEntireDuration = other.m_shouldProfileEntireDuration;
    m_isProfilePaused = other.m_isProfilePaused;
    m_startDelay = other.m_startDelay;
    m_profileDuration = other.m_profileDuration;
    m_terminateAfterDataCollectionIsDone = other.m_terminateAfterDataCollectionIsDone;
    m_isSessionRunning = other.m_isSessionRunning;
    m_envVariables = other.m_envVariables;
    m_startTime = other.m_startTime;
    m_endTime = other.m_endTime;
}

SessionTreeNodeData& SessionTreeNodeData::operator=(const SessionTreeNodeData& other)
{
    m_name = other.m_name;
    m_displayName = other.m_displayName;

    // NOTICE: Do not copy the session id! the session id is unique for the new session data
    // m_sessionId = other.m_sessionId;

    m_profileTypeStr = other.m_profileTypeStr;
    m_projectName = other.m_projectName;
    m_isImported = other.m_isImported;
    m_commandArguments = other.m_commandArguments;
    m_workingDirectory = other.m_workingDirectory;
    m_exeName = other.m_exeName;
    m_exeFullPath = other.m_exeFullPath;
    m_profileScope = other.m_profileScope;
    m_shouldProfileEntireDuration = other.m_shouldProfileEntireDuration;
    m_isProfilePaused = other.m_isProfilePaused;
    m_startDelay = other.m_startDelay;
    m_profileDuration = other.m_profileDuration;
    m_terminateAfterDataCollectionIsDone = other.m_terminateAfterDataCollectionIsDone;
    m_isSessionRunning = other.m_isSessionRunning;
    m_envVariables = other.m_envVariables;
    m_startTime = other.m_startTime;
    m_endTime = other.m_endTime;

    return *this;
}

void SessionTreeNodeData::CopyFrom(const SessionTreeNodeData* pOther)
{
    GT_IF_WITH_ASSERT(pOther)
    {
        m_name = pOther->m_name;
        m_displayName = pOther->m_displayName;

        // NOTICE: Do not copy the session id! the session id is unique for the new session data
        // m_sessionId = pOther->m_sessionId;

        m_profileTypeStr = pOther->m_profileTypeStr;
        m_projectName = pOther->m_projectName;
        m_isImported = pOther->m_isImported;
        m_commandArguments = pOther->m_commandArguments;
        m_workingDirectory = pOther->m_workingDirectory;
        m_exeName = pOther->m_exeName;
        m_exeFullPath = pOther->m_exeFullPath;
        m_profileScope = pOther->m_profileScope;
        m_shouldProfileEntireDuration = pOther->m_shouldProfileEntireDuration;
        m_isProfilePaused = pOther->m_isProfilePaused;
        m_startDelay = pOther->m_startDelay;
        m_profileDuration = pOther->m_profileDuration;
        m_terminateAfterDataCollectionIsDone = pOther->m_terminateAfterDataCollectionIsDone;
        m_isSessionRunning = pOther->m_isSessionRunning;
        m_envVariables = pOther->m_envVariables;
        m_startTime = pOther->m_startTime;
        m_endTime = pOther->m_endTime;
    }
}


SessionTreeNodeData::~SessionTreeNodeData()
{
}


osDirectory SessionTreeNodeData::SessionDir() const
{
    osDirectory retVal;

    if (m_pParentData != nullptr)
    {
        if (!m_pParentData->m_filePath.getFileDirectory(retVal))
        {
            retVal = osDirectory();
        }
    }

    return retVal;
}

void SessionTreeNodeData::copyID(afTreeDataExtension*& pOtherItemData) const
{
    if (pOtherItemData == NULL)
    {
        pOtherItemData = new SessionTreeNodeData(m_sessionId, m_profileTypeStr, m_isImported);

    }

    GT_IF_WITH_ASSERT(pOtherItemData != NULL)
    {
        SessionTreeNodeData* pSessionData = qobject_cast<SessionTreeNodeData*>(pOtherItemData);
        GT_IF_WITH_ASSERT(pSessionData != NULL)
        {
            pSessionData->CopyFrom(this);
            pSessionData->m_sessionId = m_sessionId;
        }
    }
}

bool SessionTreeNodeData::isSameObject(afTreeDataExtension* pOtherItemData) const
{
    bool retVal = false;

    if (pOtherItemData != NULL)
    {
        SessionTreeNodeData* pSessionData = qobject_cast<SessionTreeNodeData*>(pOtherItemData);
        GT_IF_WITH_ASSERT(pSessionData != NULL)
        {
            retVal = true;
            retVal = retVal && (m_name == pSessionData->m_name);
            retVal = retVal && (m_sessionId == pSessionData->m_sessionId);
            retVal = retVal && (m_profileTypeStr == pSessionData->m_profileTypeStr);
            retVal = retVal && (m_projectName == pSessionData->m_projectName);
            retVal = retVal && (m_commandArguments == pSessionData->m_commandArguments);
            retVal = retVal && (m_isImported == pSessionData->m_isImported);
            retVal = retVal && (m_workingDirectory == pSessionData->m_workingDirectory);
            retVal = retVal && (m_exeName == pSessionData->m_exeName);
            retVal = retVal && (m_exeFullPath == pSessionData->m_exeFullPath);
            retVal = retVal && (m_envVariables == pSessionData->m_envVariables);
            retVal = retVal && (m_startTime == pSessionData->m_startTime);
            retVal = retVal && (m_endTime == pSessionData->m_endTime);
        }
    }

    return retVal;
}

QString SessionTreeNodeData::GetNameWithImportSuffix(const QString& name) const
{
    QString retVal = name;

    if (m_isImported)
    {
        QString namePostfix;
        osFilePath sessionExePath(acQStringToGTString(m_exeFullPath));

        if (afProjectManager::instance().currentProjectSettings().executablePath() == sessionExePath)
        {
            // Same executable:
            namePostfix = PM_STR_ImportedSessionPostfix;
        }
        else
        {
            // Other executable:
            gtString fileName;
            sessionExePath.getFileName(fileName);
            QString qfileName = acGTStringToQString(fileName);
            namePostfix.sprintf(" (%s - %s)", PM_STR_ImportedSessionPostfix, qfileName.toLatin1().data());
        }

        retVal.append(namePostfix);
    }

    return retVal;
}

void SessionTreeNodeData::SetDisplayName(const QString& displayName)
{
    m_displayName = displayName;
}


bool SessionTreeNodeData::DeleteSessionFilesFromDisk(QString& report)
{
    bool retVal = false;

    osDirectory sessionDir = SessionDir();
    GT_IF_WITH_ASSERT(!sessionDir.directoryPath().asString().isEmpty() && (sessionDir.exists()))
    {
        retVal = sessionDir.deleteRecursively();

        if (!retVal)
        {
            // Make a list of the files that could not be deleted:
            gtList<osFilePath> filesStillExist;
            sessionDir.getContainedFilePaths(L"*", osDirectory::SORT_BY_NAME_ASCENDING, filesStillExist);
            gtList<osFilePath>::iterator iter = filesStillExist.begin();
            gtList<osFilePath>::iterator iterEnd = filesStillExist.end();

            for (; iter != iterEnd; iter++)
            {
                report.append((*iter).asString().asASCIICharArray());
                report.append("\n");;
            }
        }
    }

    return retVal;
}

void SessionTreeNodeData::BuildSessionHTML(afHTMLContent& htmlContent) const
{
    gtString strContent;

    StartSessionHTML(htmlContent);

    // Session Name:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_BOLD_LINE, PM_Str_HTMLSessionName, acQStringToGTString(m_displayName));

    // Executable Path:
    strContent = PM_Str_HTMLExePathTitle;
    strContent.append(acQStringToGTString(m_exeFullPath));
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, strContent);

    if (!m_commandArguments.trimmed().isEmpty())
    {
        strContent = PM_Str_HTMLArgumentsTitle;
        strContent.append(acQStringToGTString(m_commandArguments));
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, strContent);
    }

    if (!m_workingDirectory.trimmed().isEmpty())
    {
        strContent = PM_Str_HTMLWorkDirTitle;
        strContent.append(acQStringToGTString(m_workingDirectory));
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, strContent);
    }

    QString envVarsQStr = acGTStringToQString(m_envVariables);

    if (!envVarsQStr.trimmed().isEmpty())
    {
        gtString content = PM_Str_HTMLEnvVarTitle;
        content.append(m_envVariables);
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, content);
    }

    if (m_profileTypeStr != PM_profileTypeFrameAnalysis)
    {
        gtString scopeStr = PM_STR_ProfileScopeSystemWide;

        if (m_profileScope == PM_PROFILE_SCOPE_SYS_WIDE_FOCUS_ON_EXE)
        {
            scopeStr = PM_STR_ProfileScopeSystemWideWithFocus;
        }
        else if (m_profileScope == PM_PROFILE_SCOPE_SINGLE_EXE)
        {
            scopeStr = PM_STR_ProfileScopeSingleApplication;
        }

        gtString scopeTitleStr = PM_Str_HTMLProfileScope;
        scopeTitleStr.append(AF_STR_ColonW);
        scopeTitleStr.append(AF_STR_Space);
        strContent = scopeTitleStr;
        strContent.append(scopeStr);
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, strContent);
    }

    if ((!m_startTime.isEmpty()) && (!m_endTime.isEmpty()))
    {
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, PM_Str_HTMLProfileStartTime, acQStringToGTString(m_startTime));
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_LINE, PM_Str_HTMLProfileEndTime, acQStringToGTString(m_endTime));
    }
}

void SessionTreeNodeData::StartSessionHTML(afHTMLContent& htmlContent) const
{
    // Show the session properties:
    gtString title = ProfileTypePrefix();
    title += AF_STR_ColonW;
    title += AF_STR_Space;
    title += PM_Str_HTMLProfileSessionCaption;
    htmlContent.setTitle(title);

    // Profile Type:
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_BOLD_LINE, PM_Str_HTMLProfileType, acQStringToGTString(m_profileTypeStr));
}

bool SessionTreeNodeData::InitFromXML(const gtString& xmlString)
{
    bool retVal = false;

    TiXmlNode* pProfileNode = new TiXmlElement(PM_STR_SharedProfileExtensionNameA);


    pProfileNode->Parse(xmlString.asASCIICharArray(), 0, TIXML_DEFAULT_ENCODING);
    gtString profileNodeTitle;
    profileNodeTitle.fromASCIIString(pProfileNode->Value());

    if (profileNodeTitle == PM_STR_SharedProfileExtensionName)
    {
        retVal = true;

        int numVal = 0;
        afUtils::getFieldFromXML(*pProfileNode, PM_STR_xmlProfileScope, numVal);
        m_profileScope = (ProfileSessionScope)numVal;
        afUtils::getFieldFromXML(*pProfileNode, PM_STR_xmlProfileEntireDuration, m_shouldProfileEntireDuration);
        afUtils::getFieldFromXML(*pProfileNode, PM_STR_xmlProfilePaused, m_isProfilePaused);
        afUtils::getFieldFromXML(*pProfileNode, PM_STR_xmlProfileStartDelay, m_startDelay);
        afUtils::getFieldFromXML(*pProfileNode, PM_STR_xmlProfileEndAfter, m_profileDuration);
        afUtils::getFieldFromXML(*pProfileNode, PM_STR_xmlProfileTerminateAfter, m_terminateAfterDataCollectionIsDone);

    }

    return retVal;
}

bool SessionTreeNodeData::ToXMLString(gtString& projectAsXMLString)
{
    projectAsXMLString.appendFormattedString(L"<%ls>", PM_STR_SharedProfileExtensionName);

    afUtils::addFieldToXML(projectAsXMLString, PM_STR_xmlProfileScope, m_profileScope);
    afUtils::addFieldToXML(projectAsXMLString, PM_STR_xmlProfileEntireDuration, m_shouldProfileEntireDuration);
    afUtils::addFieldToXML(projectAsXMLString, PM_STR_xmlProfilePaused, m_isProfilePaused);
    afUtils::addFieldToXML(projectAsXMLString, PM_STR_xmlProfileStartDelay, m_startDelay);
    afUtils::addFieldToXML(projectAsXMLString, PM_STR_xmlProfileEndAfter, m_profileDuration);
    afUtils::addFieldToXML(projectAsXMLString, PM_STR_xmlProfileTerminateAfter, m_terminateAfterDataCollectionIsDone);

    projectAsXMLString.appendFormattedString(L"</%ls>", PM_STR_SharedProfileExtensionName);

    return true;
}

gtString SessionTreeNodeData::ProfileTypePrefix() const
{
    gtString retVal;

    if ((m_profileTypeStr == PM_profileTypeTimeBased) ||
        (m_profileTypeStr == PM_profileTypeCustomProfile) ||
        (m_profileTypeStr == PM_profileTypeCLU) ||
        (m_profileTypeStr == PM_profileTypeAssesPerformance) ||
        (m_profileTypeStr == PM_profileTypeInstructionBasedSampling) ||
        (m_profileTypeStr == PM_profileTypeInvestigateBranching) ||
        (m_profileTypeStr == PM_profileTypeInvestigateDataAccess) ||
        (m_profileTypeStr == PM_profileTypeInvestigateInstructionAccess) ||
        (m_profileTypeStr == PM_profileTypeInvestigateInstructionL2CacheAccess))
    {
        retVal = PM_Str_HTMLProfileCPUPrefix;
    }
    else if ((m_profileTypeStr == PM_profileTypePerformanceCounters) ||
             (m_profileTypeStr == PM_profileTypeApplicationTrace))
    {
        retVal = PM_Str_HTMLProfileGPUPrefix;
    }
    else if (m_profileTypeStr == PM_profileTypePowerProfile)
    {
        retVal = PM_Str_HTMLProfilePowerPrefix;
    }
    else if (m_profileTypeStr == PM_profileTypeThreadProfile)
    {
        retVal = PM_Str_HTMLProfileThreadPrefix;
    }
    else if (m_profileTypeStr == PM_profileTypeFrameAnalysis)
    {
        retVal = PM_Str_HTMLProfileFrameAnalysisPrefix;
    }
    else
    {
        GT_ASSERT_EX(false, PM_STR_UnsupportedProfileTypeError);
    }

    return retVal;
}
