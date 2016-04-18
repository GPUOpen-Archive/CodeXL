//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/gpViewsCreator.h $
/// \version $Revision: #13 $
/// \brief  This file contains the MDI views added by the GPU Profiler plugin
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/gpViewsCreator.h#13 $
// Last checkin:   $DateTime: 2016/03/17 05:20:00 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 564197 $
//=====================================================================

#ifndef _GPVIEWSCREATOR_H_
#define _GPVIEWSCREATOR_H_

#include <qtIgnoreCompilerWarnings.h>
#include <QtCore>
#include <QtWidgets>

#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afQtViewCreatorAbstract.h>
#include <AMDTApplicationFramework/Include/afQMdiSubWindow.h>
#include <AMDTApplicationFramework/Include/afTreeItemType.h>

#include <TSingleton.h>

// Local:
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>
#include <AMDTGpuProfiling/Util.h>

class SharedSessionWindow;
class SessionTreeNodeData;
class gpTraceDataModel;


// Forward declarations
class GPUSessionTreeItemData;
class GPUSessionWindow;
class TraceView;


enum TraceViewsUIType
{
    NEW_UI,
    OLD_UI,
    PROMPT_THE_USER
};


/// Represents the types of windows created by the GPU profiler
enum GPUWindowType
{
    /// Unknown or undefined window type
    GPUWindowTypeUnknown = 0,

    /// Performance counters window
    GPUWindowTypePerformanceCounters,

    /// API Trace window
    GPUWindowTypeAPITrace,

    /// Ojbect Inspector window
    GPUWindowTypeObjectInspector,

    /// Frame analysis dashboard
    GPUWindowTypeFrameAnalysisSession,

    /// Frame analysis frame view
    GPUWindowTypeFrameAnalysisFrameView,

    /// Frame analysis Object Inspector
    GPUWindowTypeFrameAnalysisObjectInspector,

    GPUWindowTypesCount = GPUWindowTypeFrameAnalysisObjectInspector
};

/// View Creator for the GPU Profiler GPUSessionTreeItemData views
class AMDT_GPU_PROF_API gpViewsCreator : public QObject, public afQtViewCreatorAbstract, public TSingleton<gpViewsCreator>
{
    Q_OBJECT

    // required so that TSingleton can access our constructor
    friend class TSingleton<gpViewsCreator>;

public:
    virtual void titleString(int viewIndex, gtString& viewTitle, gtString& viewMenuCommand);

    virtual gtString associatedToolbar(int viewIndex);

    virtual afViewType type(int viewIndex);

    virtual int dockArea(int viewIndex);

    virtual bool isDynamic() { return true; }

    // MDI icon:
    QPixmap* iconAsPixmap(int viewIndex);

    // For dynamic views: the event type which is used for these views creation:
    virtual const gtString CreatedMDIType() const { return AF_STR_GPUProfileViewsCreatorID; }

    virtual bool getCurrentlyDisplayedFilePath(osFilePath& filePath);

    virtual QDockWidget::DockWidgetFeatures dockWidgetFeatures(int viewIndex) { Q_UNUSED(viewIndex); return QDockWidget::NoDockWidgetFeatures; } // TODO: what is the correct value?

    virtual QWidget* createQtWidgetWrapping(int viewIndex) { GT_UNREFERENCED_PARAMETER(viewIndex); return nullptr; };
    virtual bool createQTWrapping(int viewIndex, QMainWindow* pMainWindow) { (void)(viewIndex); (void)(pMainWindow); return true; };

    virtual bool createViewContent(int viewIndex, QWidget*& pContentQWidget, QWidget* pQParent);

    virtual QSize initialSize(int viewIndex) { Q_UNUSED(viewIndex); return QSize(400, 300); }

    virtual bool visibility(int viewIndex) { Q_UNUSED(viewIndex); return true; }

    //virtual int amountOfCreatedViews();

    virtual int amountOfViewTypes();

    virtual void handleTrigger(int viewIndex, int actionIndex);

    virtual void handleUiUpdate(int viewIndex, int actionIndex);

    virtual bool displayExistingView(const apMDIViewCreateEvent& mdiViewEvent);

    /// Enable the creator to block the process of a file open.
    /// \param cannotOpenFileMessage the message displayed to the user in case that the file cannot be opened
    /// \return true iff the file can currently be opened
    virtual bool CanFileBeOpened(const osFilePath& filePath, gtString& cannotOpenFileMessage);;

    /// Try to find a window that is matching this file path:
    /// For sessions, look for the file path itself
    /// For temp pc file, look also for the mapped file
    /// For Frame analysis, for inner frame files (trace and profile files), look for the owning frame window
    /// \param filePath the file path
    /// \param windowType the window type extracted from the file path
    /// \return an existing window that can display the requested file path
    SharedSessionWindow* FindMatchingWindow(const osFilePath& filePath, osFilePath& sessionPath, GPUWindowType windowType);

    /// Show the main session window for the specified session
    /// \param pSession the session to show
    /// \param treeItemType - the session summary page item type
    void ShowSession(GPUSessionTreeItemData* pSession, afTreeItemType treeItemType);

    /// Create a temporary performace counter file, with a link to the csv file path
    /// \param pSession the session for which the temp pc file should be created
    void CreateTempPCFile(GPUSessionTreeItemData* pSession);

    /// Hides all windows for the specified session
    /// \param sessionPath the session to hide
    void HideSession(const osFilePath& sessionPath);

    /// Updates the MDI tab title for the specified session
    /// \param oldSessionFileName the session original file path
    /// \param newSessionDirectory the session original file path after rename
    void UpdateTitleString(const osFilePath& oldSessionFileName, const osFilePath& newSessionFileName);

    /// Gets the View title for the specified file
    /// \param fileName the file whose view title is needed
    /// \return the view title for the specified file
    gtString GetViewTitleForFile(const osFilePath& fileName);

    /// Is called when a session window is about to be closed
    /// \param pClosedSessionWindow the about to be closed session window
    void OnWindowClose(QWidget* pClosedSessionWindow);

    /// Gets the temp (.gpsession) file for a perf counter session
    /// \param projectName the project name
    /// \param sessionName the session name for the session whose temp file is needed
    /// \param[out] tempPCFile the filename
    /// \return true if successful, false otherwise
    static bool GetTempPCFile(const QString& projectName, const QString& sessionName, osFilePath& tempPCFile);

    /// Reads the real session file (.csv) from the .gpsession file
    /// \param tempPCFile the .gpsession file
    /// \param[out] sessionFile the name of the real session file
    /// \return true if the file could be read from the temp session file, false otherwise
    static bool GetSessionFileFromTempPCFile(const osFilePath& tempPCFile, QString& sessionFile);

    /// Is called when a session is about to be deleted:
    /// \param deletedSessionFilePath the file path for the deleted session
    void OnSessionDelete(const gtString& deletedSessionFilePath);


    /// Create an MDI widget with the requested parent:
    /// \param pParent the MDI parent
    /// \param sessionPath the session file path
    QWidget* CreateMDIWidget(QWidget* pParent, const osFilePath& sessionPath);

    // Events:
    virtual bool onMDISubWindowClose(afQMdiSubWindow* pMDISubWindow);

private:
    /// Initializes the GpuProfilerMDIViewsCreator singleton
    gpViewsCreator();

    /// Gets the appropriate view index from a file extension
    /// \param filePath the file path whose type is needed
    /// \return the appropriate view index from the file extension
    GPUWindowType GPUWindowTypeFromFilePath(const osFilePath& filePath);

    /// Gets the session from the temp (.gpsession) file
    /// \param tempPCFile the filename of the temp file
    /// \return the session that corresponds to the .gpsession file or NULL if no session found
    GPUSessionTreeItemData* GetSessionFromTempPCFile(const osFilePath& tempPCFile);

    /// Helper function called by OpenMdiWidget (VS) and createViewContent (SA) to unify the session view handling
    /// \param profileType the requested profile type for the created session
    /// \param pParent the parent widget
    /// \param sessionPath the path of the session file
    /// \param pWidget the session widget for the specified session
    /// \return true if widget is successfully created, false otherwise
    bool GetWidgetForFilePath(GPUWindowType profileType, QWidget* pParent, const osFilePath& sessionPath, QWidget*& pWidget);

    /// Creates frame analysis dashboard
    /// \param pParent the parent widget
    /// \return the created dashboard window
    SharedSessionWindow* CreateFrameAnalysisSessionView(QWidget* pParent, const osFilePath& sessionPath);

    /// Creates frame analysis overview
    /// \param pParent the parent widget
    /// \return the created overview window
    SharedSessionWindow* CreateFrameAnalysisFrameView(QWidget* pParent);

    /// Create an application trace session window:
    /// \param pParent the parent widget
    /// \return the created API trace window
    SharedSessionWindow* CreateAppTraceSessionWindow(QWidget* pParent);

    /// Create an application object inspector session window:
    /// \param pParent the parent widget
    /// \return the created object inspector window
    SharedSessionWindow* CreateFrameAnalysisObjectView(QWidget* pParent);

    /// Create an application object inspector session window:
    /// \param pParent the parent widget
    /// \return the created object inspector window
    SharedSessionWindow* CreateAppObjectInspectorSessionWindow(QWidget* pParent);

    /// Create a performance counters session window:
    /// \param pParent the parent widget
    /// \param sessionPath the path of the session file
    /// \param pathToDisplay[out] the path that the view should display
    /// \return the created performance counters window
    SharedSessionWindow* CreatePerfCountersSessionWindow(QWidget* pParent, const osFilePath& sessionPath, osFilePath& pathToDisplay);

    /// Load a session into a view
    /// \param pNewSessionWindow the session created window
    /// \param sessionFilePath the session file path
    /// \param displayItemInView the type of the item that should be displayed within the view
    /// \param [out] errorMessage when the display fails, errorMessage should contain a message for the user
    bool LoadFileToView(SharedSessionWindow* pNewSessionWindow, const osFilePath& sessionFilePath, afTreeItemType displayItemInView, QString& errorMessage);

    /// Load a DX session into a view
    /// \param pNewSessionWindow the session created window
    /// \param sessionFilePath the session file path
    /// \param displayItemInView the type of the item that should be displayed within the view
    bool LoadFrameAnalysisView(SharedSessionWindow* pNewSessionWindow, const osFilePath& sessionFilePath, afTreeItemType displayItemInView);

    /// Convert a session path to a session name
    /// \param sessionFilePath the session full file path
    /// \return a string with the session name
    static gtString SessionFilePathToSessionName(const osFilePath& sessionFilePath);

    /// Get the current active session window:
    SharedSessionWindow* GetCurrentActiveSessionWindow();

    /// Used in titleString -- there's probably a better way to do this
    gtString m_lastSessionFileOpened;

    /// Map from session file path to the session window:
    QMap<osFilePath, SharedSessionWindow*> m_filePathToSessionWindowsMap;

    /// The profile type for the last opened MDI
    GPUWindowType m_lastCreatedMDIType;

    /// Should open trace view with: Old /New UI, ask the user
    /// This member will be used to test the old UI in comparison to the new one. Until we have stable new UI,
    // then this member can be removed
    TraceViewsUIType m_traceUI;

    // object inspector UI, old vs new, can be removed, same condition as m_traceUI
    TraceViewsUIType m_objectInspectorUI;
};

#endif // _GPVIEWSCREATOR_H_
