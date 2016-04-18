//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SessionViewCreator.h
/// \brief  Framework interface to create the MDI Session windows
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/inc/SessionViewCreator.h#40 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================
#ifndef _SessionViewCreator_H
#define _SessionViewCreator_H

//Qt:
#include <QtCore>
#include <QtWidgets>
#include <QtWidgets/QLabel>

// Infra:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afQtViewCreatorAbstract.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTreeItemData.h>
#include <AMDTBaseTools/Include/gtVector.h>

// Local:
#include <inc/DllExport.h>

//predefined:
class CpuSessionWindow;
class CommandsHandler;
class CPUSessionTreeItemData;

/// This class is responsible for interacting with the framework for the CA session windows
///
/// Note that a 'view' is a displayed window
/// \TODO Handle Properties signal when selection changed
class AMDT_CPU_PROF_API SessionViewCreator : public QObject, public afQtViewCreatorAbstract
{
    Q_OBJECT;
public:
    /// Constructor
    SessionViewCreator();

    /// Destructor
    ~SessionViewCreator();

    QWidget* openMdiWidget(QWidget* pQParent, const gtString& sessionPath);

    /// Creates the session window and opens the mdi
    void openSession(const gtString& sessionPath, const gtString& moduleFilePath, afTreeItemType cpuItemType);

    /// Closes an open session window, if applicable. This function is called before deleting the session:
    ///  Returns whether the session was closed
    bool closeSessionBeforeDeletion(const gtString& sessionPath);

    /// The title of the view
    virtual void titleString(int viewIndex, gtString& viewTitle, gtString& viewMenuCommand);

    /// Update the matching session window title with the new session name
    void updateTitleString(const CPUSessionTreeItemData* pSessionData);

    /// Get the associated toolbar string for the view
    virtual gtString associatedToolbar(int viewIndex);

    /// Get the type of the view
    virtual afViewType type(int viewIndex);

    /// Get the docking area of the view
    virtual int dockArea(int viewIndex);

    /// Whether there can be multiple dynamic instances of the views
    virtual bool isDynamic() { return true;};

    QPixmap* iconAsPixmap(int viewIndex);

    //// The MDI views id which is used for these views creation:
    virtual const gtString CreatedMDIType() const { return AF_STR_CPUProfileViewsCreatorID; };

    /// Get the docking features:
    virtual QDockWidget::DockWidgetFeatures dockWidgetFeatures(int viewIndex);

    /// Create the inner view:
    virtual bool createViewContent(int viewIndex, QWidget*& pContentQWidget, QWidget* pQParent);

    /// Display the view containing the content specified in the event
    virtual bool displayExistingView(const apMDIViewCreateEvent& mdiViewEvent);

    /// Get the initial size:
    virtual QSize initialSize(int viewIndex);

    /// Get the initial visibility of the view:
    virtual bool visibility(int viewIndex);

    /// Get number of types of views that are supported by this creator:
    virtual int amountOfViewTypes();

    /// Handle sub window close:
    virtual bool onMDISubWindowClose(afQMdiSubWindow* pMDISubWindow);

    /// Handle the action when it is triggered
    virtual void handleTrigger(int viewIndex, int actionIndex);

    /// handle UI update
    virtual void handleUiUpdate(int viewIndex, int actionIndex);

    // Get the file that is currently displayed:
    virtual bool getCurrentlyDisplayedFilePath(osFilePath& filePath);

    // Get the index for the view with matching file path:
    int viewIndexByPath(const osFilePath& filePath);

    /// The map of session path to session window
    const gtVector<CpuSessionWindow*>& openedSessionWindows() const {return m_sessionWindowsVector;}
    /// Get the currently opened session windows:
    const gtVector<CpuSessionWindow*>& currentlyOpenedSessionWindows() {return m_sessionWindowsVector;};

    // Show / hide information panel:
    bool showInfoPanel() const {return m_showInfoPanel;};
    void setShowInfoPanel(bool show) {m_showInfoPanel = show;};

    // Display existing session:
    bool displayOpenSession(const osFilePath& filePath, int lineNumber);

    /// Find the session window for the session represents by the item data
    /// \param pSessionItemData - the item data representing the session in the application tree:
    CpuSessionWindow* findSessionWindow(const afApplicationTreeItemData* pSessionItemData);

    /// Find the session window for the session in sessionPath
    /// \param sessionPath the requested session file path
    CpuSessionWindow* findSessionWindow(const osFilePath& sessionPath);

    /// Remove a session window that is about to be deleted
    /// \param pDeletedWindow - the session window which is about to be deleted
    void removeDeletedSessionWindow(CpuSessionWindow* pDeletedWindow);

    /// Remove the session window displaying the requested session file path:
    /// \param sessionFilePath the session file path which it's window should be removed
    /// \param deleteView should the view be also deleted after removal
    void removeSessionWindow(const osFilePath& sessionFilePath, bool deleteView = false);

    /// Get the currently active CPU session window. If the active window is not a CPU session, return null:
    CpuSessionWindow* GetCurrentActiveSessionWindow();

    /// Update the display settings for the session related to filePath
    /// \param filePath the file path of the session to update
    void UpdateView(const osFilePath& filePath);

    /// Creates a CPU session window (used from VS)
    QWidget* CreateSessionWindow(QWidget* pParent);

protected:

    /// Check the show hide information panel action properties:
    void showHideInformationPanel();
protected:
    /// Application command handler:
    CommandsHandler* m_pCommandsHandler;

    /// The map of session path to session window
    gtVector<CpuSessionWindow*> m_sessionWindowsVector;

    /// Show / Hide information panel:
    bool m_showInfoPanel;
};

#endif //_SessionViewCreator_H
