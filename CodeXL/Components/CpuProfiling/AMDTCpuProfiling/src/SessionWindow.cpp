//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SessionWindow.cpp
/// \brief  The implementation for the session window
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/src/SessionWindow.cpp#159 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================

// Qt:
#include <QtCore>
#include <QtWidgets>
#include <QApplication>
#include <QClipboard>
#include <QLabel>
#include <QLayout>
#include <QToolButton>
#include <QWidget>

// Infra:
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/src/afUtils.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/ProfileApplicationTreeHandler.h>
#include <AMDTCpuProfilingDataAccess/inc/AMDTCpuProfilingDataAccess.h>

//Local:
#include <inc/AmdtCpuProfiling.h>
#include <inc/Auxil.h>
#include <inc/SessionWindow.h>
#include <inc/CpuProfileTreeHandler.h>
#include <inc/CpuProjectHandler.h>
#include <inc/CpuProfilingOptions.h>
#include <inc/SessionFunctionView.h>
#include <inc/SessionModulesView.h>
#include <inc/SessionOverviewWindow.h>
#include <inc/SessionSourceCodeView.h>
#include <inc/SessionViewCreator.h>
#include <inc/StringConstants.h>
#include <inc/SessionCallGraphView.h>

CpuSessionWindow::CpuSessionWindow(const afApplicationTreeItemData* pSessionTreeItemData, QWidget* pParent) : SharedSessionWindow(pParent)
{
    QSizePolicy spolicy;
    spolicy.setVerticalPolicy(QSizePolicy::Ignored);
    spolicy.setHorizontalPolicy(QSizePolicy::Ignored);
    setSizePolicy(spolicy);

    m_isSessionBeingRenamed = false;
    m_pSessionTreeItemData = pSessionTreeItemData;
    m_pTabWidget = nullptr;
    m_pProfileInfo = nullptr;
    m_pOverviewWindow = nullptr;
    m_pSessionModulesView = nullptr;
    m_pSessionFunctionView = nullptr;
    m_pCallGraphTab = nullptr;
    m_firstActivation = true;

    CPUSessionTreeItemData* pCPUItemData = qobject_cast<CPUSessionTreeItemData*>(pSessionTreeItemData->extendedItemData());
    GT_IF_WITH_ASSERT(pCPUItemData != nullptr)
    {
        QString windowStr = CPU_PREFIX_A;
        windowStr += pCPUItemData->m_displayName;
        setWindowTitle(windowStr);
    }

    // Connect the profile tree handler rename signals:
    bool rc = connect(ProfileApplicationTreeHandler::instance(), SIGNAL(SessionRenamed(SessionTreeNodeData*, const osFilePath&, const osDirectory&)), this, SLOT(onSessionRename(SessionTreeNodeData*, const osFilePath&, const osDirectory&)));
    GT_ASSERT(rc);

    // Connect the profile tree handler rename signals:
    rc = connect(ProfileApplicationTreeHandler::instance(), SIGNAL(BeforeSessionRename(SessionTreeNodeData*, bool&, QString&)), this, SLOT(onBeforeSessionRename(SessionTreeNodeData*, bool&, QString&)));
    GT_ASSERT(rc);
}

CpuSessionWindow::~CpuSessionWindow()
{
    // Remove me from the list of session windows in the session view creator:
    SessionViewCreator* pSessionViewCreator = AmdtCpuProfiling::sessionViewCreator();
    GT_IF_WITH_ASSERT(pSessionViewCreator != nullptr)
    {
        pSessionViewCreator->removeDeletedSessionWindow(this);
    }

    if (nullptr != m_pTabWidget)
    {
        while (0 < m_pTabWidget->count())
        {
            onTabClose(0);
        }
    }

    delete m_pTabWidget;
    m_profileReader.close();
    m_pProfileInfo = nullptr;
}

bool CpuSessionWindow::initialize()
{
    bool retVal = false;
    QGridLayout* pLayout = new QGridLayout(this);
    RETURN_FALSE_IF_NULL(pLayout);
    pLayout->setContentsMargins(0, 0 , 0, 0);


    this->setLayout(pLayout);
    m_pTabWidget = new QTabWidget(this);
    RETURN_FALSE_IF_NULL(m_pTabWidget);

    m_pTabWidget->setTabsClosable(true);

    // Connect the tab widget to slot:
    bool rc = connect(m_pTabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(onTabClose(int)));
    GT_ASSERT(rc);
    rc = connect(m_pTabWidget, SIGNAL(currentChanged(int)), this, SLOT(onTabCurrentChange(int)));
    GT_ASSERT(rc);

    pLayout->addWidget(m_pTabWidget, 0, 0);
    setFocusProxy(m_pTabWidget);
    retVal = display();

    return retVal;

}

void CpuSessionWindow::OnBeforeDeletion()
{
    m_profileReader.close();
}

bool CpuSessionWindow::display()
{
    bool retVal = false;
    GT_IF_WITH_ASSERT((nullptr != m_pTabWidget) && (nullptr != m_pSessionTreeItemData))
    {
        // Open the database file
        OpenDataReader();

        m_pDisplayFilter.reset(new DisplayFilter);
        m_pDisplayFilter->SetProfDataReader(m_pProfDataRd);
        retVal = m_pDisplayFilter->CreateConfigCounterMap();

        if (retVal == true)
        {
            // init with default configuration
            retVal = m_pDisplayFilter->InitToDefault();
        }

        m_sessionFile = m_pSessionTreeItemData->m_filePath;

        // Close the profile reader before opening it:
        m_profileReader.close();

        // If there is an available profile data file, open it
        bool rc = m_profileReader.open(m_sessionFile.asString().asCharArray());

        if (!rc)
        {
            gtString message;
            message.appendFormattedString(L"Failed to open profile file. File path: %ls", m_sessionFile.asString().asCharArray());
            OS_OUTPUT_DEBUG_LOG(message.asCharArray(), OS_DEBUG_LOG_ERROR);
        }

        m_pProfileInfo = m_profileReader.getProfileInfo();
        GT_IF_WITH_ASSERT(rc && (nullptr != m_pProfileInfo))
        {
            if (m_pProfileInfo->m_numSamples > 0)
            {
                m_sessionDisplayFilter.initialize(m_pProfileInfo);

                if (!displayOverviewWindow(m_sessionFile))
                {
                    if (m_pOverviewWindow != nullptr)
                    {
                        delete m_pOverviewWindow;
                        m_pOverviewWindow = nullptr;
                    }
                }
            }
            else
            {
                QMessageBox::information(this, "No Samples available", "The selected session does not have any data, please try again.");
            }
        }

        // Prepare the list of processes that has CSS collection for this session:
        BuildCSSProcessesList();
    }

    // Show / hide information panel:
    SessionViewCreator* pSessionViewCreator = AmdtCpuProfiling::sessionViewCreator();
    GT_IF_WITH_ASSERT(pSessionViewCreator != nullptr)
    {
        showInformationPanel(pSessionViewCreator->showInfoPanel());
    }

    return retVal;

}

void CpuSessionWindow::activate()
{
    if (nullptr == m_pTabWidget)
    {
        return;
    }

    if (!m_sessionFile.exists())
    {
        QMessageBox::information(this, "Session does not exist!",
                                 "The selected session does not exist, please try to view a different profile.");
        return;
    }

    QWidget* currentTab = m_pTabWidget->currentWidget();

    if (!displayOverviewWindow(m_sessionFile))
    {
        if (m_pOverviewWindow != nullptr)
        {
            delete m_pOverviewWindow;
            m_pOverviewWindow = nullptr;
        }
    }

    if (m_pOverviewWindow == nullptr)
    {
        onViewModulesView(AGGREGATE_BY_MODULES);
    }

    if (nullptr != currentTab)
    {
        m_pTabWidget->setCurrentWidget(currentTab);
    }
}

QWidget* CpuSessionWindow::FindTab(QString caption, QString filePath)
{
    (void)(filePath); // unused

    for (int i = 0; i < m_pTabWidget->count(); i++)
    {
        QWidget* pWidget = m_pTabWidget->widget(i);
        QString tips = m_pTabWidget->tabToolTip(i);

        if (tips == caption)
        {
            return pWidget;
        }
    }

    return nullptr;
}

void CpuSessionWindow::AddTabToNavigatorBar(QWidget* pMainTabWidget, const QString& tabCaption, QPixmap* pPixmap)
{
    // Do not respond to current change signal:
    m_pTabWidget->blockSignals(true);

    QWidget* pOldTab = FindTab(tabCaption);

    // Add the Data Tab if not already shown
    if (nullptr == pOldTab)
    {
        // Add Data Tab
        QString trunc;

        if (tabCaption.length() > 50)
        {
            trunc = QString("...") + tabCaption.right(50);
        }
        else
        {
            trunc = tabCaption;
        }

        QIcon icon;

        if (pPixmap != nullptr)
        {
            icon = *pPixmap;
        }

        int tabIndex = m_pTabWidget->addTab(pMainTabWidget, icon, trunc);
        // In case there are two files with different paths, but the same file name
        m_pTabWidget->setTabToolTip(tabIndex, tabCaption);
        m_pTabWidget->setCurrentWidget(pMainTabWidget);
    }
    else
    {
        m_pTabWidget->setCurrentWidget(pOldTab);
    }

    m_pTabWidget->blockSignals(false);

}

void CpuSessionWindow::onTabClose(int index)
{
    GT_IF_WITH_ASSERT(m_pTabWidget != nullptr)
    {
        m_pTabWidget->blockSignals(true);

        QWidget* pDataTab = m_pTabWidget->widget(index);

        if (m_pCallGraphTab == pDataTab)
        {
            m_pCallGraphTab = nullptr;
        }

        else if (m_pSessionModulesView == pDataTab)
        {
            m_pSessionModulesView = nullptr;
        }

        else if (m_pOverviewWindow == pDataTab)
        {
            m_pOverviewWindow = nullptr;
        }

        else if (m_pSessionFunctionView == pDataTab)
        {
            m_pSessionFunctionView = nullptr;
        }

        m_pTabWidget->removeTab(index);
        m_pTabWidget->blockSignals(false);
        delete pDataTab;

        if (index > 0)
        {
            // update the tab left as active
            emit m_pTabWidget->currentChanged(index - 1);
        }
    }
}

void CpuSessionWindow::onTabCurrentChange(int index)
{
    (void)(index); // unused

    // We want the tree to be aligned with the user selections, therefore we want the tree item to be activated once a tab is clicked:
    DataTab* pDataTab = qobject_cast<DataTab*>(m_pTabWidget->currentWidget());

    if (pDataTab != nullptr)
    {
        const afApplicationTreeItemData* pSelectedTabItemData = pDataTab->currentlyDisplayedItemData();

        if (pSelectedTabItemData != nullptr)
        {
            if (pDataTab == m_pOverviewWindow)
            {
                pSelectedTabItemData = ProfileApplicationTreeHandler::instance()->FindSessionChildItemData(pSelectedTabItemData, AF_TREE_ITEM_PROFILE_CPU_OVERVIEW);
            }

            afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
            GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
            {
                afApplicationTree* pTree = pApplicationCommands->applicationTree();
                GT_IF_WITH_ASSERT(pTree != nullptr)
                {
                    pTree->selectItem(pSelectedTabItemData, false);
                }
            }

            // If the displayed filter is not updated, update it:
            pDataTab->UpdateTableDisplaySettings();
        }
    }
}

void CpuSessionWindow::onNavigateToDifferentTab(const QString& dest)
{
    QString tabType = dest.section(":", 0, 0);
    QString param = dest.section(":", 1);

    if (tabType == "proc")
    {
        onViewModulesView(AGGREGATE_BY_PROCESSES);
    }
    else if (tabType == "sys")
    {
        onViewModulesView(AGGREGATE_BY_MODULES);
    }
    else if (tabType == "css")
    {
        unsigned int pid = param.section(".", 0, 0).toUInt();
        onViewCallGraphView(pid);
    }

    else
    {
        QMessageBox::warning(this, "CodeXL Error", "Link Error: " + dest + ".");
    }
}

bool CpuSessionWindow::displayOverviewWindow(const osFilePath& filePath)
{
    m_pTabWidget->blockSignals(true);

    bool retVal = false;

    if (nullptr != m_pOverviewWindow)
    {
        m_pTabWidget->setCurrentWidget(m_pOverviewWindow);
        m_pOverviewWindow->setFocus();
        retVal = true;
    }
    else
    {
        GT_IF_WITH_ASSERT(ProfileApplicationTreeHandler::instance() != nullptr)
        {
            // Find the item data that is related to the item with this profile file:
            afApplicationTreeItemData* pItemData = ProfileApplicationTreeHandler::instance()->FindItemByProfileFilePath(filePath);

            m_pOverviewWindow = new SessionOverviewWindow(m_pTabWidget, this);


            bool rc = connect(m_pOverviewWindow, SIGNAL(opensourceCodeViewSig(std::tuple<AMDTFunctionId, const gtString&, AMDTUInt32, AMDTUInt32>)), this,
                              SLOT(onViewSourceViewSlot(std::tuple<AMDTFunctionId, const gtString&, AMDTUInt32, AMDTUInt32>)));
            GT_ASSERT(rc);

            rc = m_pOverviewWindow->display(pItemData);
            GT_IF_WITH_ASSERT(rc)
            {
                m_pTabWidget->setTabToolTip(0, CP_STR_OverviewTabTitle);

                QIcon icon;
                QPixmap* pPixmap = ProfileApplicationTreeHandler::instance()->TreeItemTypeToPixmap(AF_TREE_ITEM_PROFILE_CPU_OVERVIEW);

                if (pPixmap != nullptr)
                {
                    icon = *pPixmap;
                }

                m_pTabWidget->insertTab(0, m_pOverviewWindow, icon, CP_STR_OverviewTabTitle);

                // Don't allow to close "Profile Overview" tab
                QTabBar* tabBar =  m_pTabWidget->findChild<QTabBar*>();

                if (nullptr != tabBar)
                {
                    tabBar->setTabButton(0, QTabBar::RightSide, 0);
                }

                retVal = true;
            }
        }
    }

    m_pTabWidget->blockSignals(false);

    return retVal;
}

bool CpuSessionWindow::onViewModulesView(SYSTEM_DATA_TAB_CONTENT aggregateBy)
{
    (void)(aggregateBy); // unused
    bool ret = checkIfDataIsPresent();

    if (ret)
    {
        if (m_pSessionModulesView == nullptr)
        {
            m_pSessionModulesView = new SessionModulesView(m_pTabWidget, this);


            // Get the modules item data:
            afApplicationTreeItemData* pModuleItemData = CpuProfileTreeHandler::instance().findChildItemData(m_pSessionTreeItemData, AF_TREE_ITEM_PROFILE_CPU_MODULES);
            GT_IF_WITH_ASSERT(pModuleItemData != nullptr)
            {
                m_pSessionModulesView->display(pModuleItemData);
            }

            // Look for the icon for this tab:
            QPixmap* pPixmap = ProfileApplicationTreeHandler::instance()->TreeItemTypeToPixmap(AF_TREE_ITEM_PROFILE_CPU_MODULES);

            // Add the modules tab to the navigator:
            AddTabToNavigatorBar(m_pSessionModulesView, CP_STR_ModulesTabTitle, pPixmap);
        }

        m_pTabWidget->setCurrentWidget(m_pSessionModulesView);
        m_pSessionModulesView->setFocus();

        // BUG421904: Set Module view m_CLUNoteShown to current clu flag
        CPUProfileDataTable::m_CLUNoteShown = m_pSessionModulesView->m_CLUNoteShown;
    }

    return ret;
}

void CpuSessionWindow::onViewSourceViewSlot(std::tuple<AMDTFunctionId, const gtString&, AMDTUInt32, AMDTUInt32> funcModInfo)
{
	QString modName = acGTStringToQString(std::get<1>(funcModInfo));
	AMDTUInt32 pid = std::get<3>(funcModInfo);
	QString caption = modName + " - Source/Disassembly";
	QWidget* pOldTab = FindTab(caption);
	SessionSourceCodeView* pSourceCodeView = (SessionSourceCodeView*)pOldTab;

	bool createdNewView = false;

	if (nullptr == pSourceCodeView)
	{
		// Create new source tab:
		osDirectory sessionDir;
		m_sessionFile.getFileDirectory(sessionDir);
		QString sessionFileStr = acGTStringToQString(sessionDir.directoryPath().asString());
		pSourceCodeView = new SessionSourceCodeView(m_pTabWidget, this, sessionFileStr);

		createdNewView = true;
		pSourceCodeView->setDisplayedItemData((afApplicationTreeItemData*)m_pSessionTreeItemData);

		if (!pSourceCodeView->DisplayViewModule(funcModInfo))
		{
			QMessageBox::information(this, "Source/Disassembly View Error", "Failed to initialize Source/Disassembly tab for module :\n" + modName);
			delete pSourceCodeView;
			return;
		}

		pSourceCodeView->setWindowTitle(caption);

		connect(&(CpuProfilingOptions::instance()), SIGNAL(settingsUpdated()), pSourceCodeView, SLOT(onViewChanged()));

		// Look for the icon for this tab:
		QPixmap* pPixmap = ProfileApplicationTreeHandler::instance()->TreeItemTypeToPixmap(AF_TREE_ITEM_PROFILE_CPU_SOURCE_CODE);

		AddTabToNavigatorBar(pSourceCodeView, caption, pPixmap);
	}

	if (createdNewView)
	{
		// Do not redisplay address for alredy displayed source code view:
		pSourceCodeView->DisplayAddress(0, pid, SHOW_ALL_TIDS);
	}

	m_pTabWidget->setCurrentWidget(pSourceCodeView);
}

void CpuSessionWindow::onViewSourceView(gtVAddr Address, ProcessIdType pid, ThreadIdType tid, const CpuProfileModule* pModDetail)
{

    QString modName = acGTStringToQString(pModDetail->getPath());
    QString caption = modName + " - Source/Disassembly";
    QWidget* pOldTab = FindTab(caption);
    SessionSourceCodeView* pSourceCodeView = (SessionSourceCodeView*)pOldTab;

    bool createdNewView = false;

    if (nullptr == pSourceCodeView)
    {
        // Create new source tab:
        osDirectory sessionDir;
        m_sessionFile.getFileDirectory(sessionDir);
        QString sessionFileStr = acGTStringToQString(sessionDir.directoryPath().asString());
        pSourceCodeView = new SessionSourceCodeView(m_pTabWidget, this, sessionFileStr);


        createdNewView = true;
        pSourceCodeView->setDisplayedItemData((afApplicationTreeItemData*)m_pSessionTreeItemData);

#if 0
        if (!pSourceCodeView->DisplayModule(pModDetail))
        {
            QMessageBox::information(this, "Source/Disassembly View Error" , "Failed to initialize Source/Disassembly tab for module :\n" + modName);
            delete pSourceCodeView;
            return;
        }
#endif
        pSourceCodeView->setWindowTitle(caption);

        connect(&(CpuProfilingOptions::instance()), SIGNAL(settingsUpdated()), pSourceCodeView, SLOT(onViewChanged()));

        // Look for the icon for this tab:
        QPixmap* pPixmap = ProfileApplicationTreeHandler::instance()->TreeItemTypeToPixmap(AF_TREE_ITEM_PROFILE_CPU_SOURCE_CODE);

        AddTabToNavigatorBar(pSourceCodeView, caption, pPixmap);
    }

    if (createdNewView || Address > 0)
    {
        // Do not redisplay address for alredy displayed source code view:
        pSourceCodeView->DisplayAddress(Address, pid, tid);
    }

    m_pTabWidget->setCurrentWidget(pSourceCodeView);
}


void CpuSessionWindow::onViewCallGraphView(unsigned long pid)
{
    if (checkIfDataIsPresent())
    {
        //This tab is not allocated until it's needed
        if (nullptr == m_pCallGraphTab)
        {
            // Get the call graph item data:
            afApplicationTreeItemData* pCallGraphItemData = CpuProfileTreeHandler::instance().findChildItemData(m_pSessionTreeItemData, AF_TREE_ITEM_PROFILE_CPU_CALL_GRAPH);
            GT_ASSERT(pCallGraphItemData != nullptr);

            // Create a new call graph tab:
            m_pCallGraphTab = new SessionCallGraphView(m_pTabWidget, this, pCallGraphItemData);


            bool rc = connect(m_pCallGraphTab, SIGNAL(opensourceCodeViewSig(std::tuple<AMDTFunctionId, const gtString&, AMDTUInt32, AMDTUInt32>)),
                              this,
                              SLOT(onViewSourceViewSlot(std::tuple<AMDTFunctionId, const gtString&, AMDTUInt32, AMDTUInt32>)));
            GT_ASSERT(rc);

            QString sessionFileStr = acGTStringToQString(m_sessionFile.asString());

            if (!m_pCallGraphTab->Display(sessionFileStr, pid))
            {
                delete m_pCallGraphTab;
                m_pCallGraphTab = nullptr;
                return;
            }

            // Look for the icon for this tab:
            QPixmap* pPixmap = ProfileApplicationTreeHandler::instance()->TreeItemTypeToPixmap(AF_TREE_ITEM_PROFILE_CPU_CALL_GRAPH);
            AddTabToNavigatorBar(m_pCallGraphTab, CP_STR_CallGraphTabTitle, pPixmap);
        }

        //m_pCallGraphTab->showPid(pid); // removed 2012/09/12 because it causes uneeded re-read of data
        m_pCallGraphTab->setFocus();
        m_pTabWidget->setCurrentWidget(m_pCallGraphTab);
    }
}

void CpuSessionWindow::onViewFunctionTab(unsigned long pid)
{
    (void)(pid); // unused

    if (checkIfDataIsPresent())
    {
        if (nullptr == m_pSessionFunctionView)
        {
            m_pSessionFunctionView =  new SessionFunctionView(m_pTabWidget, this);


            connect(m_pSessionFunctionView, SIGNAL(opensourceCodeViewSig(std::tuple<AMDTFunctionId, const gtString&, AMDTUInt32, AMDTUInt32>)),
                    this,
                    SLOT(onViewSourceViewSlot(std::tuple<AMDTFunctionId, const gtString&, AMDTUInt32, AMDTUInt32>)));

            // Get the modules item data:
            afApplicationTreeItemData* pFunctionsItemData = CpuProfileTreeHandler::instance().findChildItemData(m_pSessionTreeItemData, AF_TREE_ITEM_PROFILE_CPU_FUNCTIONS);
            GT_IF_WITH_ASSERT(pFunctionsItemData != nullptr)
            {
                m_pSessionFunctionView->display(pFunctionsItemData);
            }

            // Look for the icon for this tab:
            QPixmap* pPixmap = ProfileApplicationTreeHandler::instance()->TreeItemTypeToPixmap(AF_TREE_ITEM_PROFILE_CPU_FUNCTIONS);
            AddTabToNavigatorBar(m_pSessionFunctionView, CP_STR_FunctionsTabTitle, pPixmap);
        }

        m_pTabWidget->setCurrentWidget(m_pSessionFunctionView);
        m_pSessionFunctionView->setFocus();

        // BUG421904: Set function view m_CLUNoteShown to current clu flag
        CPUProfileDataTable::m_CLUNoteShown = m_pSessionFunctionView->m_CLUNoteShown;
    }
}

void CpuSessionWindow::OnEditCopy()
{
    GT_IF_WITH_ASSERT(m_pTabWidget != nullptr)
    {
        DataTab* pCurrentTab = qobject_cast<DataTab*>(m_pTabWidget->currentWidget());

        if (pCurrentTab != nullptr)
        {
            pCurrentTab->onEditCopy();
        }
    }
}

void CpuSessionWindow::OnEditSelectAll()
{
    GT_IF_WITH_ASSERT(m_pTabWidget != nullptr)
    {
        DataTab* pCurrentTab = qobject_cast<DataTab*>(m_pTabWidget->currentWidget());

        if (pCurrentTab != nullptr)
        {
            pCurrentTab->onEditSelectAll();
        }
    }
}

void CpuSessionWindow::onFindClick()
{
    GT_IF_WITH_ASSERT(m_pTabWidget != nullptr)
    {
        DataTab* pCurrentTab = qobject_cast<DataTab*>(m_pTabWidget->currentWidget());

        if (pCurrentTab != nullptr)
        {
            pCurrentTab->onFindClick();
        }
    }
}

void CpuSessionWindow::onFindNext()
{
    GT_IF_WITH_ASSERT(m_pTabWidget != nullptr)
    {
        DataTab* pCurrentTab = qobject_cast<DataTab*>(m_pTabWidget->currentWidget());

        if (pCurrentTab != nullptr)
        {
            pCurrentTab->onFindNext();
        }
    }
}

bool CpuSessionWindow::DisplaySession(const osFilePath& filePath, afTreeItemType itemType, QString& errorMessage)
{
    GT_UNREFERENCED_PARAMETER(filePath);
    GT_UNREFERENCED_PARAMETER(errorMessage);

    m_pTabWidget->blockSignals(true);

    if (afMainAppWindow::instance() != nullptr)
    {
        if (afMainAppWindow::instance()->mdiArea() != nullptr)
        {
            afMainAppWindow::instance()->mdiArea()->blockSignals(true);
        }
    }

    bool retVal = true;

    switch (itemType)
    {
        case AF_TREE_ITEM_ITEM_NONE:
        case AF_TREE_ITEM_PROFILE_SESSION:
        case AF_TREE_ITEM_PROFILE_CPU_OVERVIEW:
        {
            displayOverviewWindow(osFilePath());
            break;
        }

        case AF_TREE_ITEM_PROFILE_CPU_MODULES:
        {
            retVal = true;
            onViewModulesView(AGGREGATE_BY_PROCESSES);
            break;
        }

        case AF_TREE_ITEM_PROFILE_CPU_FUNCTIONS:
        {
            retVal = true;
            onViewFunctionTab(0U);// 0U for all the processes
            break;
        }

        case AF_TREE_ITEM_PROFILE_CPU_CALL_GRAPH:
        {
            retVal = true;
            onViewCallGraphView(0U);// 0U for all the processes
            break;
        }

        case AF_TREE_ITEM_PROFILE_CPU_SOURCE_CODE:
        {
            retVal = displaySessionSource();
            break;
        }

        default:
        {
            GT_ASSERT_EX(false, L"Implement me: display the CPU profile item type");
            retVal = false;
        }
    }

    m_pTabWidget->blockSignals(false);

    if (afMainAppWindow::instance() != nullptr)
    {
        if (afMainAppWindow::instance()->mdiArea() != nullptr)
        {
            afMainAppWindow::instance()->mdiArea()->blockSignals(false);
        }
    }

    // Show / hide information panel:
    SessionViewCreator* pSessionViewCreator = AmdtCpuProfiling::sessionViewCreator();
    GT_IF_WITH_ASSERT(pSessionViewCreator != nullptr)
    {
        showInformationPanel(pSessionViewCreator->showInfoPanel());
    }

    return retVal;
}

const CPUSessionTreeItemData* CpuSessionWindow::displayedCPUSessionItemData() const
{
    const CPUSessionTreeItemData* pRetVal = nullptr;
    GT_IF_WITH_ASSERT(m_pSessionTreeItemData != nullptr)
    {
        pRetVal = qobject_cast<CPUSessionTreeItemData*>(m_pSessionTreeItemData->extendedItemData());
    }
    return pRetVal;
}

void CpuSessionWindow::onSessionRename(SessionTreeNodeData* pRenamedSessionData, const osFilePath& oldSessionFilePath, const osDirectory& oldSessionDirectory)
{
    (void)(oldSessionDirectory); // unused
    (void)(oldSessionFilePath); // unused

    CPUSessionTreeItemData* pCpuData = qobject_cast<CPUSessionTreeItemData*>(pRenamedSessionData);

    // If this is a CPU session data:
    if (pCpuData != nullptr)
    {
        // If the session renamed is mine:
        if (m_isSessionBeingRenamed)
        {
            // DisplayModule the session again (open the profile reader):
            display();

            // The session rename is done:
            m_isSessionBeingRenamed = false;
        }
    }
}

void CpuSessionWindow::onBeforeSessionRename(SessionTreeNodeData* pAboutToRenameSessionData, bool& isRenameEnabled, QString& renameDisableMessage)
{

    CPUSessionTreeItemData* pCpuData = qobject_cast<CPUSessionTreeItemData*>(pAboutToRenameSessionData);

    // If this is a CPU session data:
    if ((pCpuData != nullptr) && (pCpuData->m_pParentData != nullptr))
    {
        // If the rename is for the same session I am displaying:
        if (pCpuData->m_pParentData->m_filePath == m_sessionFile)
        {
            m_isSessionBeingRenamed = true;

            // Sanity check:
            GT_IF_WITH_ASSERT(m_pTabWidget != nullptr)
            {
                for (int i = 0 ; i < m_pTabWidget->count(); i++)
                {
                    SessionSourceCodeView* pSourceView = qobject_cast<SessionSourceCodeView*>(m_pTabWidget->widget(i));

                    if (pSourceView != nullptr)
                    {
                        renameDisableMessage = CP_sessionCannotBeRenamed;
                        isRenameEnabled = false;
                        break;
                    }
                }
            }

            // Close the profile reader (in order to release the file handler, to allow rename of the file):
            m_profileReader.close();
        }
    }
}

void CpuSessionWindow::showInformationPanel(bool show)
{
    if (m_pOverviewWindow != nullptr)
    {
        m_pOverviewWindow->showInformationPanel(show);
    }

    if (m_pSessionModulesView != nullptr)
    {
        m_pSessionModulesView->showInformationPanel(show);
    }

    if (m_pSessionFunctionView != nullptr)
    {
        m_pSessionFunctionView->showInformationPanel(show);
    }

    if (m_pCallGraphTab != nullptr)
    {
        m_pCallGraphTab->showInformationPanel(show);
    }

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pTabWidget != nullptr)
    {
        // Go through each of the source code views, and show / hide the information panel:
        for (int i = 0 ; i < m_pTabWidget->count(); i++)
        {
            SessionSourceCodeView* pSourceView = qobject_cast<SessionSourceCodeView*>(m_pTabWidget->widget(i));

            if (pSourceView != nullptr)
            {
                pSourceView->showInformationPanel(show);
            }
        }
    }
}

void CpuSessionWindow::UpdateDisplaySettings(bool isActive, unsigned int changeType)
{
    // For non active windows, only mark the update flag:
    if (!isActive)
    {
        if (m_pSessionModulesView != nullptr)
        {
            m_pSessionModulesView->SetUpdateType(changeType);
        }

        if (m_pOverviewWindow != nullptr)
        {
            m_pOverviewWindow->SetUpdateType(changeType);
        }

        if (m_pSessionFunctionView != nullptr)
        {
            m_pSessionFunctionView->SetUpdateType(changeType);
        }

        if (m_pCallGraphTab != nullptr)
        {
            m_pCallGraphTab->SetUpdateType(changeType);
        }

        // Sanity check:
        GT_IF_WITH_ASSERT(m_pTabWidget != nullptr)
        {
            // Go through each of the source code views, and show / hide the information panel:
            for (int i = 0 ; i < m_pTabWidget->count(); i++)
            {
                SessionSourceCodeView* pSourceView = qobject_cast<SessionSourceCodeView*>(m_pTabWidget->widget(i));

                if (pSourceView != nullptr)
                {
                    pSourceView->SetUpdateType(changeType);
                }
            }
        }
    }
    else
    {
        if (m_pSessionFunctionView != nullptr)
        {
            if (m_pSessionFunctionView == m_pTabWidget->currentWidget())
            {
                m_pSessionFunctionView->SetUpdateType(changeType);
                m_pSessionFunctionView->UpdateTableDisplaySettings();
            }
            else
            {
                m_pSessionFunctionView->SetUpdateType(changeType);
            }
        }

        if (m_pSessionModulesView != nullptr)
        {
            // The modules view update is not performance expensive, so it can be done here:
            m_pSessionModulesView->SetUpdateType(changeType);
            m_pSessionModulesView->UpdateTableDisplaySettings();
        }

        if (m_pOverviewWindow != nullptr)
        {
            if (m_pOverviewWindow == m_pTabWidget->currentWidget())
            {
                m_pOverviewWindow->SetUpdateType(changeType);
                m_pOverviewWindow->UpdateTableDisplaySettings();
            }
            else
            {
                m_pOverviewWindow->SetUpdateType(changeType);
            }
        }

        if (m_pCallGraphTab != nullptr)
        {
            if (m_pCallGraphTab == m_pTabWidget->currentWidget())
            {
                m_pCallGraphTab->SetUpdateType(changeType);
                m_pCallGraphTab->UpdateTableDisplaySettings();
            }
            else
            {
                m_pCallGraphTab->SetUpdateType(changeType);
            }
        }

        // Sanity check:
        GT_IF_WITH_ASSERT(m_pTabWidget != nullptr)
        {
            // Go through each of the source code views, and show / hide the information panel:
            for (int i = 0 ; i < m_pTabWidget->count(); i++)
            {
                SessionSourceCodeView* pSourceView = qobject_cast<SessionSourceCodeView*>(m_pTabWidget->widget(i));

                if (pSourceView != nullptr)
                {
                    if (pSourceView == m_pTabWidget->currentWidget())
                    {
                        pSourceView->SetUpdateType(changeType);
                        pSourceView->UpdateTableDisplaySettings();
                    }
                    else
                    {
                        pSourceView->SetUpdateType(changeType);
                    }
                }
            }
        }
    }
}

bool CpuSessionWindow::displaySessionSource()
{
    bool retVal = false;
    // Overview window should be already opened:
    GT_IF_WITH_ASSERT(m_pOverviewWindow != nullptr)
    {
        // If the display settings were updated, make sure that the overview is updated
        // For example, to contain also the system modules:
        m_pOverviewWindow->UpdateTableDisplaySettings();

        const CPUSessionTreeItemData* pItemData = displayedCPUSessionItemData();
        GT_IF_WITH_ASSERT(pItemData != nullptr)
        {
            osFilePath moduleFilePath = acQStringToGTString(pItemData->m_exeFullPath);

            // Get the module hander for this file path:
            const CpuProfileModule* pModule = m_pOverviewWindow->findModuleHandler(moduleFilePath);
            GT_IF_WITH_ASSERT(pModule != nullptr)
            {
                //TODO : aalok
                onViewSourceView(0, 0, 0, pModule);
                retVal = true;
            }
        }
    }

    return retVal;
}

bool CpuSessionWindow::checkIfDataIsPresent()
{
    bool ret = true;

    if (m_pProfileInfo->m_numSamples == 0)
    {
        ret = false;
        QMessageBox::information(this, "No Samples available", "The selected session does not have any data, please try again.");
    }

    return ret;
}

void CpuSessionWindow::onAboutToActivate()
{
    if (!m_firstActivation && (m_pSessionTreeItemData != nullptr))
    {
        // Get my cpu session data:
        CPUSessionTreeItemData* pSessionData = qobject_cast<CPUSessionTreeItemData*>(m_pSessionTreeItemData->extendedItemData());

        if (pSessionData != nullptr)
        {
            afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
            GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
            {
                afApplicationTree* pTree = pApplicationCommands->applicationTree();
                GT_IF_WITH_ASSERT(pTree != nullptr)
                {
                    const afApplicationTreeItemData* pCurrentItemData = pTree->getCurrentlySelectedItemData();
                    bool isSameSession = false;

                    if (pCurrentItemData != nullptr)
                    {
                        isSameSession = (pCurrentItemData->m_filePath == m_pSessionTreeItemData->m_filePath);
                    }

                    if (!isSameSession)
                    {
                        if (m_pSessionTreeItemData->m_pTreeWidgetItem != nullptr)
                        {
                            pTree->selectItem(m_pSessionTreeItemData, true);
                        }
                    }
                }
            }
        }
    }

    m_firstActivation = false;
}

CpuProfileModule* CpuSessionWindow::getModuleDetail(const QString& modulePath, QWidget* pParent, ExecutableFile** ppExe)
{
    CpuProfileModule* pModule = nullptr;

    if (nullptr != ppExe)
    {
        *ppExe = nullptr;
    }

    NameModuleMap* pModuleMap = m_profileReader.getModuleMap();

    if (nullptr != pModuleMap && !pModuleMap->empty())
    {
        gtString modulePathGt = acQStringToGTString(modulePath);

        NameModuleMap::iterator mit = pModuleMap->find(modulePathGt);

        if (pModuleMap->end() != mit)
        {
            if (mit->second.m_isImdRead)
            {
                pModule = &mit->second;

                if (nullptr != ppExe)
                {
                    // Get an executable handler for this process:
                    QString exePath;

                    CPUSessionTreeItemData* pSessionData =
                        qobject_cast<CPUSessionTreeItemData*>(m_pSessionTreeItemData->extendedItemData());

                    if (AuxGetExecutablePath(exePath,
                                             m_profileReader,
                                             acGTStringToQString(pSessionData->SessionDir().directoryPath().asString()),
                                             modulePath,
                                             pParent,
                                             pModule))
                    {
                        // Get an executable handler for this process:
                        *ppExe = ExecutableFile::Open(exePath.toStdWString().c_str(), pModule->getBaseAddr());

                        if (nullptr != *ppExe)
                        {
                            // Initialize executable symbol engine:
                            AuxInitializeSymbolEngine(*ppExe);
                        }
                    }
                }
            }
            else
            {
                pModule = m_profileReader.getModuleDetail(modulePathGt);

                if (nullptr != pModule)
                {
                    pModule->setSystemModule(AuxIsSystemModule(pModule->getPath()));

                    CPUSessionTreeItemData* pSessionData =
                        qobject_cast<CPUSessionTreeItemData*>(m_pSessionTreeItemData->extendedItemData());

                    // Get an executable handler for this process:
                    QString exePath;

                    if (AuxGetExecutablePath(exePath,
                                             m_profileReader,
                                             acGTStringToQString(pSessionData->SessionDir().directoryPath().asString()),
                                             modulePath,
                                             pParent,
                                             pModule))
                    {
                        pModule->m_symbolsLoaded = syncWithSymbolEngine(*pModule, exePath, ppExe);

                        if (nullptr != ppExe && nullptr == *ppExe)
                        {
                            // Get an executable handler for this process:
                            *ppExe = ExecutableFile::Open(exePath.toStdWString().c_str(), pModule->getBaseAddr());

                            if (nullptr != *ppExe)
                            {
                                // Initialize executable symbol engine:
                                AuxInitializeSymbolEngine(*ppExe);
                            }
                        }
                    }
                }
            }
        }
    }

    return pModule;
}

bool CpuSessionWindow::syncWithSymbolEngine(CpuProfileModule& module, const QString& exePath, ExecutableFile** ppExe)
{
    bool ret = true;

    if (nullptr != ppExe)
    {
        *ppExe = nullptr;
    }

    if (CpuProfileModule::UNMANAGEDPE == module.getModType())
    {
        gtVAddr modLoadVAddr = module.getBaseAddr();

        ExecutableFile* pExecutable = nullptr;
        SymbolEngine* pSymbolEngine = nullptr;

        CpuProfileFunction* const pUnchartedFunc = module.getUnchartedFunction();

        AddrFunctionMultMap& funcMap = module.getFunctionMap();
        AddrFunctionMultMap::iterator f = funcMap.begin(), fEnd = funcMap.end();

        while (f != fEnd)
        {
            CpuProfileFunction& function = f->second;

            // If the function's name is empty then we need to try and discover the matching name.
            // If the function's size is zero then it has only partial symbol information, and we need to try and get the full info.
            // If the function is actually the "uncharted function" then we obviously need to process it.
            if (!function.getFuncName().isEmpty() && function.getSize() != 0 && pUnchartedFunc != &function)
            {
                ++f;
                continue;
            }

            if (nullptr == pSymbolEngine)
            {
                // Get an executable handler for this process:
                pExecutable = ExecutableFile::Open(exePath.toStdWString().c_str(), module.getBaseAddr());

                if (nullptr != pExecutable)
                {
                    // Initialize executable symbol engine:
                    if (AuxInitializeSymbolEngine(pExecutable))
                    {
                        pSymbolEngine = pExecutable->GetSymbolEngine();
                    }
                }

                if (nullptr == pSymbolEngine)
                {
                    ret = false;
                    break;
                }

                ret = pSymbolEngine->IsComplete();
            }

            AptAggregatedSampleMap::iterator s = function.getBeginSample(), sEnd = function.getEndSample();

            while (s != sEnd)
            {
                gtVAddr sampleVAddr = s->first.m_addr + function.getBaseAddr();
                gtRVAddr sampleRva = static_cast<gtRVAddr>(sampleVAddr - modLoadVAddr);
                gtRVAddr funcRvaEnd = GT_INVALID_RVADDR;

                const FunctionSymbolInfo* pFuncSymbol = pSymbolEngine->LookupFunction(sampleRva, &funcRvaEnd, true);

                if (nullptr == pFuncSymbol)
                {
                    ++s;
                    continue;
                }

                gtVAddr funcVAddr = static_cast<gtVAddr>(pFuncSymbol->m_rva) + modLoadVAddr;

                AddrFunctionMultMap::iterator fParent = funcMap.find(funcVAddr);

                if (fParent == funcMap.end())
                {
                    gtString srcFileName;
                    SourceLineInfo sourceLine;

                    if (pSymbolEngine->FindSourceLine(pFuncSymbol->m_rva, sourceLine))
                    {
                        int srcFileNameLen = static_cast<int>(wcslen(sourceLine.m_filePath));

                        if (!osFilePath::ConvertCygwinPath(sourceLine.m_filePath, srcFileNameLen, srcFileName))
                        {
                            srcFileName.assign(sourceLine.m_filePath, srcFileNameLen);
                        }
                    }
                    else
                    {
                        sourceLine.m_line = 0U;
                    }

                    gtUInt32 funcSize = pFuncSymbol->m_size;

                    if (0 == funcSize)
                    {
                        if (GT_INVALID_RVADDR != funcRvaEnd)
                        {
                            funcSize = funcRvaEnd - pFuncSymbol->m_rva;
                        }
                        else
                        {
                            funcSize = gtUInt32(-1);
                        }
                    }

                    CpuProfileFunction funcNew((nullptr != pFuncSymbol->m_pName) ? pFuncSymbol->m_pName : L"",
                                               funcVAddr,
                                               funcSize,
                                               gtString(),
                                               srcFileName,
                                               sourceLine.m_line);
                    fParent = funcMap.insert(AddrFunctionMultMap::value_type(funcVAddr, funcNew));
                }
                else if (fParent == f)
                {
                    ++s;
                    continue;
                }

                AptKey key(sampleVAddr - funcVAddr, s->first.m_pid, s->first.m_tid);

                fParent->second.addSample(key, s->second);

                function.removeSample(s++);
                sEnd = function.getEndSample();
            }

            if (0ULL == function.getTotal())
            {
                funcMap.erase(f++);
                fEnd = funcMap.end();
            }
            else
            {
                ++f;
            }
        }

        if (nullptr != pExecutable)
        {
            if (nullptr != ppExe)
            {
                *ppExe = pExecutable;
            }
            else
            {
                delete pExecutable;
            }
        }
    }

    return ret;
}

void CpuSessionWindow::BuildCSSProcessesList()
{
    GT_IF_WITH_ASSERT(m_profileReader.getProcessMap() != nullptr)
    {
        PidProcessMap::iterator processIter = m_profileReader.getProcessMap()->begin();
        PidProcessMap::iterator processIterEnd = m_profileReader.getProcessMap()->end();

        for (; processIter != processIterEnd; ++processIter)
        {
            // Get the session id in the map:
            ProcessIdType processID = processIter->first;
            osFilePath cssFilePath = m_sessionFile;
            gtString processIDStr;
            processIDStr.appendFormattedString(L"%u", processID);
            cssFilePath.setFileName(processIDStr);
            cssFilePath.setFileExtension(L"css");

            if (cssFilePath.exists())
            {
                // Keep this process file path:
                QString filePath = acGTStringToQString(processIter->second.getPath());

                m_CSSCollectedProcessesFilePathsMap[processID] = filePath;
            }
        }
    }
}

bool CpuSessionWindow::OpenDataReader()
{
    bool result = false;
    GT_IF_WITH_ASSERT((nullptr != m_pTabWidget) && (nullptr != m_pSessionTreeItemData))
    {
        // TODO:  to get the file path
        m_sessionFile = m_pSessionTreeItemData->m_filePath;

        // get Session directory Path
        gtString fileDir = m_sessionFile.fileDirectoryAsString();

        // set directory path for Db file
        osFilePath dbFilePath;
        dbFilePath.setFileDirectory(fileDir);

        // set file path
        gtString fileName;
        result = m_sessionFile.getFileName(fileName);

        if (result)
        {
            dbFilePath.setFileName(fileName);
            dbFilePath.setFileExtension(L"cxldb");
        }

        // validate if file exists
        if (!dbFilePath.isEmpty())
        {
            shared_ptr<cxlProfileDataReader> reader(new cxlProfileDataReader);
            result = reader->OpenProfileData(dbFilePath.asString());

            if (true == result)
            {
                m_pProfDataRd = reader;
                // TODO: debug code need to be removed
                AMDTProfileSessionInfo sessionInfo;

                result = m_pProfDataRd->GetProfileSessionInfo(sessionInfo);
            }
        }
    }
    return result;

}