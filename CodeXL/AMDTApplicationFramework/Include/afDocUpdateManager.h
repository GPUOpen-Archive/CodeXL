//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afDocUpdateManager.h
///
//==================================================================================

#ifndef __AFDOCUPDATEMANAGER_H
#define __AFDOCUPDATEMANAGER_H

// Qt
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osTime.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>

class afIDocUpdateHandler;

class afDocUpdateData
{
public:
    // Mdi window that owns that document:
    QWidget* m_pOwnedWidget;

    // The file to be checked:
    osFile m_filePath;

    // last time updated:
    time_t m_lastUpdate;

    // Update Notifier:
    afIDocUpdateHandler* m_pUpdateNotifier;

    // notify user before updating:
    bool m_notifyUser;

    // force an update event even if the document is closed
    bool m_alwaysUpdate;
};

class AF_API afDocUpdateManager
{
public:

    /// Return my single instance:
    static afDocUpdateManager& instance();

    /// Destructor:
    ~afDocUpdateManager();

    /// Register a handler for a specific document. Need to supply a QWidget that is owned by a QMdiSubWindow that will be
    /// checked when that QMdiSubWindow is activated.
    /// "Should ask the user" flag opens a dialog to the user and asks him if he wans to update the document, if the flag is false
    /// The updater is called automatically.
    void RegisterDocument(QWidget* pOwnedWidget, const osFile& filePath, afIDocUpdateHandler* updateHandler, bool notifyUser);

    /// Register a document to be monitored not just to a specific widget to be activated.
    /// This will be monitored every second using the main timer of the application
    /// in this case there is no notification to the user in any case
    /// always update force update event even if the document is closed
    void RegisterDocumentActivate(const osFile& filePath, afIDocUpdateHandler* updateHandler, bool notifyUser, bool alwaysUpdate = false);

    /// Rename a file:
    /// \param oldFilePath the old file path
    /// \param newFilePath the new file path
    void RenameFile(const osFilePath& oldFilePath, const osFilePath& newFilePath);

    /// Remove a document with a specific view from the observed list.
    void UnregisterDocumentOfWidget(QWidget* pOwnedWidget);

    /// Remove a document with a specific path and handler from timer handler list
    void UnregisterDocumentOfActivate(const osFile& filePath, afIDocUpdateHandler* updateHandler);

    /// Activate View, notify the user and call the updated if needed:
    void ActivateView(QMdiSubWindow* pSubWindow, bool isTimerCheck = false);

    /// Check update documents by path
    void GetActivateDocumentList(osFilePath& docToActivate, gtVector<afDocUpdateData*>& docToUpdateList, bool isTimerCheck, bool& notifyUser);

    /// Remove view from observed activated QMdiSubWindow and that includes the relevant doc that goes with it:
    void RemoveSubWindow(QMdiSubWindow* pSubWindow);

    /// Update a document that was saved:
    void UpdateDocument(QWidget* pOwnedWidget);

    /// Update documents that have the always update flag set
    void ForceAlwasyUpdateDocuments();

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    /// Activate View from VS data, notify the user and call the updated if needed:
    void ActivateView(HWND subWindow, gtString& activeFilePath, bool isTimerCheck = false);

    /// Remove view from vs data from observed activated QMdiSubWindow and that includes the relevant doc that goes with it:
    void RemoveSubWindow(HWND subWindow);
#endif

protected:
    // Make the actually check of the widget once it is found from SA or VS:
    void CheckWidget(QWidget* pOwnedWidget, osFilePath& filePath, bool isTimerCheck);

    // My single instance:
    static afDocUpdateManager* m_pMySingleInstance;

    /// Get owned widget by subwindow that contains a document that needs update:
    QWidget* OwnedWidget(QMdiSubWindow* pSubWindow);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    /// Get owned widget by VS data:
    QWidget* OwnedWidget(HWND subWindow);
#endif

private:
    /// Only the instance method can create this class:

    afDocUpdateManager();
    // Only afSingletonsDelete can delete my instance:
    friend class afSingletonsDelete;

    // Map between the owning mdi window and the data:
    gtMap<QWidget*, afDocUpdateData*> m_widgetDocUpdateDataMap;

    // list of documents that are monitored by activation:
    gtList<afDocUpdateData*> m_activateDocUpdateDataList;
};

#endif  // __AFDOCUPDATEMANAGER_H
