//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SessionTreeNodeData.h
///
//==================================================================================

#ifndef _SESSION_TREE_NODE_H_
#define _SESSION_TREE_NODE_H_

// Ignore warnings:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Qt:
#include <QtCore>
#include <QtWidgets>
#include <QString>

// Infra:
#include <AMDTOSWrappers/Include/osDirectory.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afHTMLContent.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTreeItemData.h>

#include "SessionExplorerDefs.h"
#include "LibExport.h"

/// QTreeWidgetItem descendant used for profiler sessions in the session explorer
class AMDTSHAREDPROFILING_API SessionTreeNodeData : public afTreeDataExtension
{
    Q_OBJECT

public:

    /// Default constructor
    SessionTreeNodeData();

    /// -----------------------------------------------------------------------------------------------
    /// Initializes a new instance of the SessionTreeNodeData class.
    /// \brief Name:        SessionTreeNodeData
    /// \brief Description:
    /// \param[in]          sessionId
    /// \param[in]          strNodeSuffix
    /// \param[in]          isNewlyAdded flag indicating if this session node represents a session that was added during the current session (as opposed to being loaded/imported from a file)
    /// \return
    /// -----------------------------------------------------------------------------------------------
    SessionTreeNodeData(ExplorerSessionId sessionId, const QString& strNodeSuffix, bool isNewlyAdded);

    /// Copy constructor
    /// \param other the other data to copy from
    SessionTreeNodeData(const SessionTreeNodeData& other);

    /// Copy constructor
    /// \param other the other data to copy from
    SessionTreeNodeData& operator=(const SessionTreeNodeData& other);

    /// Copy from the requested other data
    void CopyFrom(const SessionTreeNodeData* pOther);

    /// Destructor
    virtual ~SessionTreeNodeData();

    virtual osDirectory SessionDir() const;

    /// afTreeDataExtension override compares 2 items
    virtual bool isSameObject(afTreeDataExtension* pOtherItemData) const;

    /// afTreeDataExtension copy one item data to other
    virtual void copyID(afTreeDataExtension*& pOtherItemData) const;

    /// Add the suffix in the name if missing
    /// \param name original name
    /// \return name with suffix if it was missing
    QString GetNameWithImportSuffix(const QString& name) const;

    /// Sets the session display name
    /// \param displayName the new display name
    virtual void SetDisplayName(const QString& displayName);

    /// Delete session files
    /// \param report Report of deletion, one line for each item
    /// \return true if could delete
    virtual bool DeleteSessionFilesFromDisk(QString& report);

    /// Appends HTML properties to an HTML content object:
    void BuildSessionHTML(afHTMLContent& content) const;

    /// Initializes the values from an XML strings:
    /// \param xmlString the settings as an XML string
    bool InitFromXML(const gtString& xmlString);

    /// Initializes the values from an XML strings:
    /// \param[out] xmlString the settings as an XML string
    bool ToXMLString(gtString& projectAsXMLString);

    /// Returns the prefix for the requested profile type (CPU / GPU / Power):
    /// \return the profile type prefix
    gtString ProfileTypePrefix() const;

    /// HTML content title (override for non-profile items)
    /// htmlContent the HTML content
    virtual void StartSessionHTML(afHTMLContent& htmlContent) const;

public:

    ///< Node name
    QString m_name;

    ///< Node display name
    QString m_displayName;

    ///< the session id in CodeXL explorer for this node
    ExplorerSessionId m_sessionId;

    ///< Node suffix string
    QString m_profileTypeStr;

    ///< Flag indicating if this session is a new profile session (i.e. not imported, not loaded from disk)
    bool m_isImported;

    ///< Project Name
    QString m_projectName;

    ///< Arguments passed to the exe
    QString m_commandArguments;

    ///< Working directory for the session
    QString m_workingDirectory;

    ///< Exe file name
    QString m_exeName;

    ///< Exe file full path
    QString m_exeFullPath;

    ///< Session environment variables list (separated by "\n")
    gtString m_envVariables;

    ///< Session start time:
    QString m_startTime;

    ///< Session end time:
    QString m_endTime;

    ///< Profile entire duration:
    bool m_shouldProfileEntireDuration;

    ///< Start profiling with paused data collection:
    bool m_isProfilePaused;

    ///< Start after (seconds):
    int m_startDelay;

    ///< End after (seconds):
    int m_profileDuration;


};

#endif // _SESSION_TREE_NODE_H_
