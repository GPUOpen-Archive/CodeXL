//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SessionWindow.h
/// \brief  The interface for the session window
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/inc/SessionWindow.h#83 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================

#ifndef _CACpuSessionWindow_H
#define _CACpuSessionWindow_H

// Qt:
#include <QtCore>
#include <QtWidgets>
#include <QWidget>
#include <QFileInfo>

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTExecutableFormat/inc/ExecutableFile.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/views/afApplicationTreeItemData.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/SharedSessionWindow.h>

// Local:
#include <inc/DataTab.h>

// need to undef Bool after all includes so the moc will compile in Linux
#undef Bool

// Pre-declarations
class SystemDataTab;
class SessionOverviewWindow;
class CssTab;
class afApplicationTreeItemData;
class SessionModulesView;
class SessionFunctionView;
class SessionTreeNodeData;
class CPUSessionTreeItemData;
class SessionCallGraphView;

class CpuSessionWindow : public SharedSessionWindow
{
    Q_OBJECT
public:
    CpuSessionWindow(const afApplicationTreeItemData* pSessionTreeItemData, QWidget* pParent = nullptr);
    ~CpuSessionWindow();

    bool initialize();
    void OnBeforeDeletion();

    QWidget* FindTab(QString caption, QString filePath = QString::null);
    void AddTabToNavigatorBar(QWidget* pWidget, const QString& tabCaption, QPixmap* pPixmap = nullptr);

    bool display();
    void activate();

    /// Display the session file. This function should be implemented for session views with multiple children
    /// \param sessionFilePath the file path for the requested session
    /// \param sessionInnerPage the item type describing the inner view to open, or AF_TREE_ITEM_ITEM_NONE when the root is supposed to open
    /// \param [out] errorMessage when the display fails, errorMessage should contain a message for the user
    virtual bool DisplaySession(const osFilePath& sessionFilePath, const osFilePath& moduleFilePath,afTreeItemType sessionInnerPage, QString& errorMessage);

    SessionModulesView* sessionModulesView() const {return m_pSessionModulesView;};
    SessionFunctionView* sessionFunctionsView() const {return m_pSessionFunctionView;};
    SessionCallGraphView* sessionCallGraphTab() const {return m_pCallGraphTab;};
    SessionOverviewWindow* sessionOverviewWindow() const {return m_pOverviewWindow;};

    const afApplicationTreeItemData* displayedItemData() const {return m_pSessionTreeItemData;};
    const CPUSessionTreeItemData* displayedCPUSessionItemData() const;

    const osFilePath& displayedSessionFilePath() const {return m_sessionFile;}

    /// Expose information:
    CpuProfileReader& profileReader() { return m_profileReader; }

    bool displaySessionSource(const osFilePath& moduleFilePath);
    SessionDisplaySettings* sessionDisplaySettings() {return &m_sessionDisplayFilter;};

    /// Show / Hide information panel:
    void showInformationPanel(bool show);

    /// Display filter update:
    /// \param isActive is the session window active
    /// \param changeType what change type should be performed (see SettingsDifference for definitions)
    void UpdateDisplaySettings(bool isActive, unsigned int changeType);

    CpuProfileModule* getModuleDetail(const QString& modulePath, QWidget* pParent = nullptr, ExecutableFile** ppExe = nullptr);

public slots:

    /// New GUI:
    bool displayOverviewWindow(const osFilePath& filePath);
    void onTabClose(int index);
    void onTabCurrentChange(int index);
    void onNavigateToDifferentTab(const QString& dest);
    bool onViewModulesView(SYSTEM_DATA_TAB_CONTENT aggregateBy);
    void onViewSourceView(gtVAddr Address, ProcessIdType pid, ThreadIdType tid, const CpuProfileModule* pModDetail);
    void onViewCallGraphView(unsigned long pid);
    void onViewFunctionTab(unsigned long pid);

    //! Copies any selected text to the clipboard.
    //!
    //! \sa copyAvailable(), cut(), paste()
    virtual void OnEditCopy();

    //! Select all
    virtual void OnEditSelectAll();

    /// Find
    virtual void onFindClick();

    /// Find Next
    virtual void onFindNext();

    /// Rename slots:
    void onSessionRename(SessionTreeNodeData* pRenamedSessionData, const osFilePath& oldSessionFilePath, const osDirectory&);

    /// \param pAboutToRenameSessionData the item data for the object which is about to be renamed
    /// \param[out] isRenameEnabled is rename enabled?
    /// \param[out] renameDisableMessage the message for the user describing that currently the rename operation is disabled
    void onBeforeSessionRename(SessionTreeNodeData* pAboutToRenameSessionData, bool& isRenameEnabled, QString& renameDisableMessage);

    /// Window activation:
    void onAboutToActivate();

    /// Returns the map of file paths to process id of CSS collected processes:
    const QMap<ProcessIdType, QString>& CollectedProcessesMap() const { return m_CSSCollectedProcessesFilePathsMap; }

protected:

    /// Builds a list of details for the processes that had CSS collection. This data is used in several inner views,
    /// and we will need to re-use it:
    void BuildCSSProcessesList();

    /// return true if data is present else provide error message and return false
    bool checkIfDataIsPresent();
    bool syncWithSymbolEngine(CpuProfileModule& module, const QString& exePath, ExecutableFile** ppExe = nullptr);

private:

    /// Session tree item data:
    const afApplicationTreeItemData* m_pSessionTreeItemData;

    /// Project/Profile related data structures
    CpuProfileReader     m_profileReader;
    CpuProfileInfo*      m_pProfileInfo;
    osFilePath          m_sessionFile;

    SessionDisplaySettings m_sessionDisplayFilter;

    /// Data tabs
    SessionOverviewWindow*  m_pOverviewWindow;
    SessionModulesView*     m_pSessionModulesView;
    SessionFunctionView*    m_pSessionFunctionView;
    SessionCallGraphView*           m_pCallGraphTab;

    /// Session window widgets
    QTabWidget*         m_pTabWidget;   // The widget that holds all the tabs

    /// True iff the session is in the process of rename:
    bool m_isSessionBeingRenamed;

    /// Ignore first activation signal:
    bool m_firstActivation;

    /// Contain the list of file paths for the processes with CSS collection:
    QMap<ProcessIdType, QString> m_CSSCollectedProcessesFilePathsMap;

};

#endif //_CACpuSessionWindow_H
