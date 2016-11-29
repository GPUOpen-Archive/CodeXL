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

// cpp includes
#include <memory>

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>

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
class afApplicationTreeItemData;
class SessionModulesView;
class SessionFunctionView;
class SessionTreeNodeData;
class CPUSessionTreeItemData;
class SessionCallGraphView;
class cxlProfileDataReader;


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
    virtual bool DisplaySession(const osFilePath& sessionFilePath, afTreeItemType sessionInnerPage, QString& errorMessage);

    SessionModulesView* sessionModulesView() const {return m_pSessionModulesView;};
    SessionFunctionView* sessionFunctionsView() const {return m_pSessionFunctionView;};
    SessionCallGraphView* sessionCallGraphTab() const {return m_pCallGraphTab;};
    SessionOverviewWindow* sessionOverviewWindow() const {return m_pOverviewWindow;};

    const afApplicationTreeItemData* displayedItemData() const {return m_pSessionTreeItemData;};
    const CPUSessionTreeItemData* displayedCPUSessionItemData() const;

    const osFilePath& displayedSessionFilePath() const {return m_sessionFile;}

    bool displaySessionSource();

    /// Show / Hide information panel:
    void showInformationPanel(bool show);

    /// Display filter update:
    /// \param isActive is the session window active
    /// \param changeType what change type should be performed (see SettingsDifference for definitions)
    void UpdateDisplaySettings(bool isActive, unsigned int changeType);


    std::shared_ptr<cxlProfileDataReader> profDbReader() { return m_pProfDataRd; }
    std::shared_ptr<DisplayFilter> GetDisplayFilter() { return m_pDisplayFilter; }
    bool IsProfilingTypeCLU();
    AMDTProfileType GetProfileType();

public slots:
    /// New GUI:
    bool displayOverviewWindow(const osFilePath& filePath);
    void onTabClose(int index);
    void onTabCurrentChange(int index);
    void onNavigateToDifferentTab(const QString& dest);
    bool onViewModulesView(SYSTEM_DATA_TAB_CONTENT aggregateBy);
    void onViewSourceView(gtVAddr Address, ProcessIdType pid, ThreadIdType tid, const CpuProfileModule* pModDetail);
    void onViewSourceViewSlot(std::tuple<AMDTFunctionId, const gtString&, AMDTUInt32, AMDTUInt32> funcModInfo);
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

protected:
    /// return true if data is present else provide error message and return false
    bool checkIfDataIsPresent();

private:
    bool OpenDataReader();

    /// Session tree item data:
    const afApplicationTreeItemData* m_pSessionTreeItemData;

    /// Project/Profile related data structures
    CpuProfileInfo*      m_pProfileInfo;
    osFilePath          m_sessionFile;

    /// Data tabs
    SessionOverviewWindow*          m_pOverviewWindow      = nullptr;
    SessionModulesView*             m_pSessionModulesView  = nullptr;
    SessionFunctionView*            m_pSessionFunctionView = nullptr;
    SessionCallGraphView*           m_pCallGraphTab        = nullptr;

    // database reader
    std::shared_ptr<cxlProfileDataReader> m_pProfDataRd    = nullptr;
    std::shared_ptr<DisplayFilter>        m_pDisplayFilter = nullptr;
    bool m_isCLU = false;

    /// Session window widgets
    QTabWidget*         m_pTabWidget;   // The widget that holds all the tabs

    /// True iff the session is in the process of rename:
    bool m_isSessionBeingRenamed;

    /// Ignore first activation signal:
    bool m_firstActivation;
};

#endif //_CACpuSessionWindow_H
