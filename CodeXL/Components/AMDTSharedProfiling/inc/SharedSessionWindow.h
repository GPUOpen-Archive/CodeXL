//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SharedSessionWindow.h
///
//==================================================================================

#ifndef __SHAREDSESSIONWINDOW_H
#define __SHAREDSESSIONWINDOW_H


// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osFilePath.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afTreeItemType.h>

// Local:
#include "LibExport.h"

class SessionTreeNodeData;

/// -----------------------------------------------------------------------------------------------
/// \class Name: SharedSessionWindow : public QWidget
/// \brief Description:  Base class for the GPU and CPU session windows. Will be used for shared
///                      functionality. Currently will only be used for edit actions implementation
/// -----------------------------------------------------------------------------------------------
class AMDTSHAREDPROFILING_API SharedSessionWindow : public QWidget
{
    Q_OBJECT

public:
    SharedSessionWindow(QWidget* pParent);
    ~SharedSessionWindow();

    /// Display the session file. This function should be implemented for session views with multiple children
    /// \param sessionFilePath the file path for the requested session
    /// \param sessionInnerPage the item type describing the inner view to open, or AF_TREE_ITEM_ITEM_NONE when the root is supposed to open
    /// \param [out] errorMessage when the display fails, errorMessage should contain a message for the user
    virtual bool DisplaySession(const osFilePath& sessionFilePath, afTreeItemType sessionInnerPage, QString& errorMessage);

    /// Update the session folder after rename
    /// \param oldSessionFileName the session original file path
    /// \param newSessionDirectory the session original file path after rename
    virtual void UpdateRenamedSession(const osFilePath& oldSessionFileName, const osFilePath& newSessionFileName);

    /// Get the session file path
    /// \return the session file path
    const osFilePath& SessionFilePath() const { return m_sessionFilePath; }

    /// Set the session file path
    /// \param filePath the new session file path
    void SetSessionFilePath(const osFilePath& filePath) { m_sessionFilePath = filePath; }

    virtual void onUpdateEdit_Copy(bool& isEnabled);
    virtual void onUpdateEdit_SelectAll(bool& isEnabled);
    virtual void onUpdateEdit_Find(bool& isEnabled);
    virtual void onUpdateEdit_FindNext(bool& isEnabled);

    static QMap<QWidget*, bool> m_sessionsMap;

    /// Accessor to the session data
    /// \return the item data for the displayed session
    SessionTreeNodeData* SessionTreeData() const { return m_pSessionData; };

public slots:

    /// Copies any selected text to the clipboard:
    virtual void OnEditCopy();

    /// Select all:
    virtual void OnEditSelectAll();

    // Find in text
    virtual void onFindPrev();

    // Find in text
    virtual void onFindClick();

    // Find in text
    virtual void onFindNext();

    /// is session exist or already been deleted
    static bool IsSessionExistInMap(QWidget* wid);

protected:

    /// Contain the file path for the opened session
    osFilePath m_sessionFilePath;

    /// The currently display session data
    SessionTreeNodeData* m_pSessionData = nullptr;
};


#endif //__SHAREDSESSIONWINDOW_H

