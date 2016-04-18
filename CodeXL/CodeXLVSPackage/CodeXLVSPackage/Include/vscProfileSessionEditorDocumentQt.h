//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscProfileSessionEditorDocumentQt.h
///
//==================================================================================

#ifndef vscProfileSessionEditorDocumentQt_h__
#define vscProfileSessionEditorDocumentQt_h__

// Windows:
#include <Windows.h>

// Qt:
#include <QtWidgets>

// VS
#include <src/vspQTWindowPaneImpl.h>
#include <Include/vscStringConstants.h>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// GPU Debugging:
#include <AMDTGpuDebuggingComponents/Include/gdHTMLProperties.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIds.h>

// Shared Profiling
#include <ProfileApplicationTreeHandler.h>

// CPU Profiling:
#include <AMDTCpuProfiling/Inc/AmdtCpuProfiling.h>
#include <AMDTCpuProfiling/inc/SessionViewCreator.h>

// GPU Profiling:
#include <AMDTGpuProfiling/gpViewsCreator.h>

// Power Profiling:
#include <AMDTPowerProfiling/src/ppAppController.h>
#include <AMDTPowerProfiling/src/ppSessionView.h>

// C++:
#include <string>
#include <algorithm>

class vscProfileSessionEditorDocument;

class ProfileMDIWindowHelper : public QObject
{
    Q_OBJECT

public:
    static ProfileMDIWindowHelper& ProfileMDIWindowHelper::instance()
    {
        if (NULL == m_spMySingleInstance)
        {
            m_spMySingleInstance = new ProfileMDIWindowHelper;
            m_spMySingleInstance->initialize();
        }

        return *m_spMySingleInstance;
    }

    // Holds pointers to objects of type: vscProfileSessionEditorDocument
    // The type left as void* to align with the pre-refactoring implementation
    // (assuming that this was implemented this way for a reason). The refactoring
    // process which relocated this code did not reach that low level. If in the
    // future it seems more sensible to replace the void* with vscProfileSessionEditorDocument* -
    // feel free to do this change.
    gtVector<void*> m_documents;

    /// Load the content for the power profile session nested in pDocument:
    void LoadPowerProfileSessionContent(QWidget* pSessionWidget, const osFilePath& sessionFilePath, int viewIndex = AF_TREE_ITEM_PP_TIMELINE);

    /// Is called after the tree is filled with power profile sessions. The function is loading the content
    /// for all the sessions loaded when the vcxproj file was opened:
    void LoadOpenedPowerProfileSessions();

protected slots:

    /// A slot handling the load of sessions to CodeXL Explorer.
    /// After a session is loaded, we iterate the not yet loaded documents, and try to reload it:
    void OnSessionAddedToTree();

    /// Handle the session renamed signal:
    void OnAfterSessionRenamed(SessionTreeNodeData* pRenamedSessionData, const osFilePath& oldSessionFilePath, const osDirectory& oldSessionDir);

    /// Handle the before session renamed signal. Requesting the user permission to rename the session (the session window will later be re-opened):
    void OnBeforeSessionRenamed(SessionTreeNodeData* pAboutToRenameSessionData, bool& isRenameEnabled, QString& renameDisableMessage);

    /// Handle power profile session deletion:
    /// \param deletedSessionId the session tree id
    /// \param deleteType remove / do not remove from disk
    /// \param canDelete& [output] can the session be deleted?
    void OnSessionDelete(ExplorerSessionId deletedSessionId, SessionExplorerDeleteType deleteType, bool& canDelete);

private:

    static ProfileMDIWindowHelper* m_spMySingleInstance;
    ProfileMDIWindowHelper();

    void initialize();
};



#endif // vscProfileSessionEditorDocumentQt_h__