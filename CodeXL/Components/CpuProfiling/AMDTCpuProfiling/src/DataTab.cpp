//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file DataTab.cpp
/// \brief  Implementation of the DataTab class.
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/src/DataTab.cpp#85 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtCore>
#include <QtWidgets>
#include <QApplication>
#include <QClipboard>
#include <QHeaderView>

// Infra:
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apMDIViewActivatedEvent.h>
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTApplicationComponents/Include/acToolBar.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/src/afUtils.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTree.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afQMdiSubWindow.h>

#include <AMDTCpuPerfEventUtils/inc/ViewConfig.h>
#include <AMDTCpuPerfEventUtils/inc/IbsEvents.h>

// AMDTSharedProfiling:
#include <ProfileApplicationTreeHandler.h>

// Local:
#include <inc/AmdtCpuProfiling.h>
#include <inc/CPUProfileDataTable.h>
#include <inc/CpuProfileTreeHandler.h>
#include <inc/CpuProjectHandler.h>
#include <inc/DataTab.h>
#include <inc/DisplayFilterDlg.h>
#include <inc/StringConstants.h>
#include <inc/SessionWindow.h>
#include <inc/SessionViewCreator.h>
#include <inc/SessionCallGraphView.h>


DataTab::DataTab(QWidget* pParent, CpuSessionWindow* pParentSessionWindow, const QString& sessionDir)
    : QMainWindow(pParent), m_pProfileReader(nullptr), m_pDisplayedSessionItemData(nullptr), m_exportString(""), m_pList(nullptr),
      m_indexOffset(0), m_pMenu(nullptr), m_pColumnMenu(nullptr), m_selectText(""),
      m_precision(0), m_sessionDir(""), m_shownTotal(0), m_pHintLabel(nullptr), m_pHintFrame(nullptr), m_pDisplaySettings(nullptr),
      m_pDisplaySettingsAction(nullptr), m_pTopToolbar(nullptr), m_pParentSessionWindow(pParentSessionWindow),
      m_enableOnlySystemDllInDisplaySettings(false), m_updateChangeType(UPDATE_TABLE_NONE), m_pLastFocusedWidget(nullptr), m_isUpdatingData(false)
{
    GT_IF_WITH_ASSERT(pParentSessionWindow != nullptr)
    {
        m_pProfDataRdr = pParentSessionWindow->profDbReader();
        m_pProfileReader = &pParentSessionWindow->profileReader();
        m_pProfileInfo = m_pProfileReader->getProfileInfo();
        m_pDisplayFilter = pParentSessionWindow->GetDisplayFilter();

        AMDTProfileCounterDescVec counterDesc;
        m_pProfDataRdr->GetSampledCountersList(counterDesc);
        int idx = 0;

        for (const auto& counter : counterDesc)
        {
            m_CounterIdxMap.emplace(counter.m_name, idx++);
        }
    }

    IntializeCLUNoteString();

    m_pDisplayedSessionItemData = nullptr;
    m_exportString = "&Export data...";
    m_pList = nullptr;
    m_pColumnMenu = nullptr;
    m_indexOffset = 0;
    m_sessionDir = sessionDir;
    m_sessionDir = m_sessionDir.replace('/', PATH_SLASH);

    m_pHintLabel = nullptr;

    m_pNoteWidget = nullptr;
    m_pNoteHeader = nullptr;
    m_isProfiledClu = false;
    GT_IF_WITH_ASSERT(m_pParentSessionWindow != nullptr)
    {
        m_pSessionDisplaySettings = m_pParentSessionWindow->sessionDisplaySettings();

        if ((m_pSessionDisplaySettings != nullptr) && (m_pSessionDisplaySettings->m_pProfileInfo != nullptr))
        {
            m_isProfiledClu = m_pSessionDisplaySettings->m_displayClu;
        }
    }

    // Connect to the application focus changed signal:
    bool rc = connect(qApp, SIGNAL(focusChanged(QWidget*, QWidget*)), this, SLOT(OnApplicationFocusChanged(QWidget*, QWidget*)));
    GT_ASSERT(rc);

    // Register as an events observer:
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);
};

void  DataTab::IntializeCLUNoteString()
{
    m_CLUNoteStringList << CP_CLU_NOTE_SPAN;
    m_CLUNoteStringList << CP_CLU_NOTE_SPAN_OVER_THRESHOLD;
    m_CLUNoteStringList << CP_CLU_NOTE_LOW_UTILIZATION;
    m_CLUNoteStringList << CP_CLU_NOTE_MEDIUM_UTILIZATION;
    m_CLUNoteStringList << CP_CLU_NOTE_LOW_CLU_HIGH_ACCESS_RATE;
    m_CLUNoteStringList << CP_CLU_NOTE_MED_CLU_HIGH_ACCESS_RATE;
    m_CLUNoteStringList << CP_CLU_NOTE_COMPULSORY;
    m_CLUNoteStringList << CP_CLU_NOTE_BAD_DISASM;
}

DataTab::~DataTab()
{
    // Register as an events observer:
    apEventsHandler::instance().unregisterEventsObserver(*this);
};

void DataTab::ResizeColumnsToContents()
{
    for (int i = 0; i < m_pList->columnCount(); i++)
    {
        m_pList->resizeColumnToContents(i);
    }
}

void DataTab::onEditCopy()
{
    if (nullptr == m_pList)
    {
        return;
    }

    QClipboard* cb = QApplication::clipboard();

    if (!cb)
    {
        return;
    }

    QString tempStr;

    int columnNum = m_pList->columnCount();
    int columnCnt;

    unsigned int* pColumnLen = nullptr;
    pColumnLen = new unsigned int [columnNum];

    if (!pColumnLen)
    {
        return;
    }

    memset(pColumnLen, 0x00, sizeof(unsigned int) * columnNum);

    for (columnCnt = 0; columnCnt < columnNum; columnCnt++)
    {
        tempStr = m_pList->headerItem()->text(columnCnt);

        if (pColumnLen[columnCnt] < (unsigned int)(tempStr.length() + 1))
        {
            pColumnLen[columnCnt] = tempStr.length() + 1;
        }
    }

    QList<QTreeWidgetItem*> selList = m_pList->selectedItems();

    foreach (QTreeWidgetItem* pItem, selList)
    {
        for (columnCnt = 0; columnCnt < columnNum; ++columnCnt)
        {
            tempStr = pItem->text(columnCnt);

            if (pColumnLen[columnCnt] < (unsigned int)(tempStr.length() + 1))
            {
                pColumnLen[columnCnt] = tempStr.length() + 1;
            }
        }
    }

    QString textString;
    QString headerString;

    for (columnCnt = 0; columnCnt < columnNum; columnCnt++)
    {
        tempStr = m_pList->headerItem()->text(columnCnt);

        if (m_pList->columnWidth(columnCnt) > 0)
        {
            tempStr = tempStr.leftJustified(pColumnLen[columnCnt], ' ');
            headerString.append(tempStr);
            headerString.append("\t");
        }
    }

    headerString.append("\n");

    QTreeWidgetItemIterator it(m_pList, QTreeWidgetItemIterator::Selected);

    while (*it)
    {
        for (columnCnt = 0; columnCnt < columnNum; columnCnt++)
        {
            if (m_pList->columnWidth(columnCnt) > 0)
            {
                if (Qt::Checked != (*it)->checkState(columnCnt))
                {
                    tempStr = (*it)->text(columnCnt).remove(QChar('\0'));
                }
                else //checked items have a "1"
                {
                    tempStr = "1";
                }

                tempStr = tempStr.leftJustified(pColumnLen[columnCnt], ' ');
                textString.append(tempStr);
                textString.append("\t");
            }
        }

        textString.append("\n");
        ++it;
    }

    if (!textString.isEmpty())
    {
        cb->clear();
        cb->setText(headerString + textString + "\n" + m_selectText);
    }

    delete [] pColumnLen;
} //DataTab::onCopyText()

void DataTab::onEvent(const apEvent& eve, bool& vetoEvent)
{
    (void)(vetoEvent); // unused

    // Get the event type:
    apEvent::EventType eventType = eve.eventType();

    // When the sub window is activated, update the display settings:
    if (eventType == apEvent::AP_MDI_ACTIVATED_EVENT)
    {
        if (m_pParentSessionWindow != nullptr)
        {
            // Check if this window is about to be activated:
            const apMDIViewActivatedEvent& activateEvent = (const apMDIViewActivatedEvent&)eve;

            if (activateEvent.filePath() == m_pParentSessionWindow->displayedSessionFilePath())
            {
                // Update the display settings:
                UpdateTableDisplaySettings();
            }
        }
    }
}

QFrame* DataTab::createHintLabelFrame()
{
    m_pHintFrame = nullptr;

    m_pHintLabel = new QLabel;

    QLabel* pPixmapLabel = new QLabel;
    QPixmap infoIcon;
    acSetIconInPixmap(infoIcon, AC_ICON_WARNING_INFO);

    pPixmapLabel->setPixmap(infoIcon);

    QHBoxLayout* pBoxLayout = new QHBoxLayout;
    pBoxLayout->addWidget(pPixmapLabel);
    pBoxLayout->addWidget(m_pHintLabel, 1);
    m_pHintFrame = new QFrame;

    m_pHintFrame->setLayout(pBoxLayout);

    // Set the background color:
    QPalette p = m_pHintFrame->palette();
    p.setColor(m_pHintFrame->backgroundRole(), acYELLOW_INFO_COLOUR);
    p.setColor(QPalette::Base, acYELLOW_INFO_COLOUR);
    m_pHintFrame->setAutoFillBackground(true);
    m_pHintFrame->setPalette(p);
    m_pHintFrame->setMinimumHeight(40);
    m_pHintFrame->setFrameStyle(QFrame::Panel);
    m_pHintFrame->setFrameShape(QFrame::Panel);

    return m_pHintFrame;
}

void DataTab::createCLUNotesFrame(QLayout* pLayout)
{
    if (m_pSessionDisplaySettings->m_displayClu && (pLayout != nullptr))
    {
        m_pNoteWidget = new QTextEdit;

        m_pNoteWidget->setReadOnly(true);

        m_pNoteHeader = new QLabel(CP_CacheLineUtilizationNotes);


        pLayout->addWidget(m_pNoteHeader);
        pLayout->addWidget(m_pNoteWidget);
        m_pNoteWidget->clear();

        QColor bgColor = acGetSystemDefaultBackgroundColor();
        QPalette p = m_pNoteWidget->palette();
        p.setColor(QPalette::Background, bgColor);
        p.setColor(QPalette::Base, bgColor);
        p.setColor(m_pNoteWidget->backgroundRole(), bgColor);
        p.setColor(QPalette::Base, bgColor);
        m_pNoteWidget->setAutoFillBackground(true);
        m_pNoteWidget->setPalette(p);
        m_pNoteWidget->setMaximumHeight(60);
    }
}

void DataTab::selectTableItem(CPUProfileDataTable* pProfileDataTable, const QString& itemName, int column)
{
    GT_IF_WITH_ASSERT(pProfileDataTable != nullptr)
    {
        int itemRowIndex = -1;

        for (int i = 0; i < pProfileDataTable->rowCount(); i++)
        {
            QTableWidgetItem* pItem = pProfileDataTable->item(i, column);

            if (pItem != nullptr)
            {
                if (pItem->text() == itemName)
                {
                    itemRowIndex = i;
                    break;
                }
            }
        }

        // Select and ensure visible:
        GT_IF_WITH_ASSERT((itemRowIndex >= 0) && (itemRowIndex < pProfileDataTable->rowCount()))
        {
            pProfileDataTable->ensureRowVisible(itemRowIndex, true);
            pProfileDataTable->setFocus();
            setFocus();
            raise();
            activateWindow();
        }
    }
}

void DataTab::showInformationPanel(bool show)
{
    // Sanity check:
    if (m_pHintFrame != nullptr)
    {
        m_pHintFrame->setVisible(show);
    }
}


// ---------------------------------------------------------------------------
// Name:        DataTab::displayFilterString
// Description: update the string to be displayed
// Arguments:   CpuSessionWindow* pSessionWindow
// Return Val:  QString
// Author:  AMD Developer Tools Team
// Date:        29/4/2013
// ---------------------------------------------------------------------------
QString DataTab::displayFilterString()
{
    QString retVal;

    SessionDisplaySettings* pSessionDisplaySettings = CurrentSessionDisplaySettings();
    GT_IF_WITH_ASSERT(pSessionDisplaySettings != nullptr)
    {
        if ((pSessionDisplaySettings != nullptr) && !m_enableOnlySystemDllInDisplaySettings)
        {
            // Add the view name to the filter string:
            retVal.append(pSessionDisplaySettings->m_displayFilterName);
        }

        // Add the system modules to the filter string:
        QString displaySys = CPUGlobalDisplayFilter::instance().m_displaySystemDLLs ? "All Modules" : "System Modules Hidden";

        if (!retVal.isEmpty())
        {
            retVal.append(", ");
        }

        retVal.append(displaySys);

        bool isProfilingCLU = false;

        if (pSessionDisplaySettings->m_pProfileInfo != nullptr)
        {
            isProfilingCLU = pSessionDisplaySettings->m_pProfileInfo->m_isProfilingCLU;
        }

        bool displayPercentageInColumn = CPUGlobalDisplayFilter::instance().m_displayPercentageInColumn && (!isProfilingCLU);

        if (displayPercentageInColumn && !m_enableOnlySystemDllInDisplaySettings)
        {
            if (!retVal.isEmpty())
            {
                retVal.append(", ");
            }

            retVal.append("Percentages");
        }

        if ((m_pParentSessionWindow != nullptr) && !m_enableOnlySystemDllInDisplaySettings)
        {
            // Get the CPU filter and separate options from session display settings:
            CoreFilter cpuFilter = pSessionDisplaySettings->m_cpuFilter;
            int separate = pSessionDisplaySettings->m_separateBy;

            if (!cpuFilter.isEmpty())
            {
                if (!retVal.isEmpty())
                {
                    retVal.append(", ");
                }

                int amountOfDisplayedCores = (int)m_pParentSessionWindow->profileReader().getProfileInfo()->m_numCpus - (int)cpuFilter.size();
                QString coresFilter;
                coresFilter.sprintf("%d Cores", amountOfDisplayedCores);
                retVal.append(coresFilter);
            }

            if (SEPARATE_BY_CORE == separate || SEPARATE_BY_NUMA == separate)
            {
                if (!retVal.isEmpty())
                {
                    retVal.append(", ");
                }

                if (SEPARATE_BY_CORE == separate)
                {
                    retVal.append(CP_strCPUProfilePerCPU);
                }
                else
                {
                    retVal.append(CP_strCPUProfilePerNuma);
                }
            }
        }
    }

    return retVal;
}

void DataTab::updateDisplaySettingsString()
{
    if ((m_pDisplaySettingsAction != nullptr) && (m_pParentSessionWindow != nullptr))
    {
        // Get the filter string:
        QString filterStr = displayFilterString();

        if (filterStr.isEmpty())
        {
            filterStr = CP_strCPUProfileToolbarBase;
        }

        // Set the link based on the string constructed:
        QString link = QString("<a href='open_display_filter'>%1</a>").arg(filterStr);
        m_pDisplaySettingsAction->UpdateText(link);
    }
}


void DataTab::OnDisplaySettingsClicked()
{
    QString profileString = acGTStringToQString(m_pParentSessionWindow->displayedSessionFilePath().asString());

    if (QDialog::Accepted == DisplayFilterDlg::instance().displayDialog(profileString, m_enableOnlySystemDllInDisplaySettings))
    {
        // Update the display and the views' filters:
        ProtectedUpdateTableDisplay(UPDATE_TABLE_COLUMNS_DATA);

        // Update the displayed string:
        updateDisplaySettingsString();

#if 0
        // Get the edited settings and the current session settings:
        const SessionDisplaySettings& changedSettings = DisplayFilterDlg::instance().SessionSettings();
        SessionDisplaySettings* pSessionSettings = m_pParentSessionWindow->sessionDisplaySettings();

        // Compare both settings:
        bool isGlobalFlagChanged = false;
        unsigned int settingsChangeType = pSessionSettings->CompareSettings(changedSettings);

        if (CPUGlobalDisplayFilter::instance().m_displaySystemDLLs != DisplayFilterDlg::instance().DisplaySystemDlls())
        {
            settingsChangeType = settingsChangeType | UPDATE_TABLE_REBUILD;
            isGlobalFlagChanged = true;
        }

        if (CPUGlobalDisplayFilter::instance().m_displayPercentageInColumn != DisplayFilterDlg::instance().ShowPercentage())
        {
            settingsChangeType = settingsChangeType | UPDATE_TABLE_COLUMNS_DATA;
            isGlobalFlagChanged = true;
        }

        if (settingsChangeType != UPDATE_TABLE_NONE)
        {
            // Set the new settings values:
            pSessionSettings->CopyFrom(changedSettings);
            CPUGlobalDisplayFilter::instance().m_displayPercentageInColumn = DisplayFilterDlg::instance().ShowPercentage();
            CPUGlobalDisplayFilter::instance().m_displaySystemDLLs = DisplayFilterDlg::instance().DisplaySystemDlls();

            // Update the display and the views' filters:
            ProtectedUpdateTableDisplay(settingsChangeType);

            // Update the displayed string:
            updateDisplaySettingsString();
        }

        // If one of the global flags had changed, then views should be updated:
        if (isGlobalFlagChanged)
        {
            gtVector<CpuSessionWindow*> openedWindowsVector = AmdtCpuProfiling::instance().sessionViewCreator()->currentlyOpenedSessionWindows();

            for (int i = 0 ; i < (int)openedWindowsVector.size(); i++)
            {
                CpuSessionWindow* pWindow = openedWindowsVector[i];
                GT_IF_WITH_ASSERT(pWindow != nullptr)
                {
                    // Update the window display filter (delayed update for non-active session windows):
                    bool isWindowActive = (pWindow == m_pParentSessionWindow);
                    pWindow->UpdateDisplaySettings(isWindowActive, settingsChangeType);
                }
            }
        }
        else
        {
            // Update only the session views:
            GT_IF_WITH_ASSERT(m_pParentSessionWindow != nullptr)
            {
                // Set a flag stating that the view should be updated:
                m_pParentSessionWindow->UpdateDisplaySettings(true, settingsChangeType);
            }
        }

#endif
    }
}

void DataTab::updateHint(const QString& hint)
{
    GT_IF_WITH_ASSERT(m_pHintLabel != nullptr)
    {
        m_pHintLabel->setText(hint);
    }
}

void DataTab::openCallGraphView()
{
    GT_IF_WITH_ASSERT(m_pDisplayedSessionItemData != nullptr)
    {
        // Get the modules item data for the current session:
        afApplicationTreeItemData* pSessionItemData = m_pDisplayedSessionItemData;

        if (pSessionItemData->m_itemType != AF_TREE_ITEM_PROFILE_SESSION)
        {
            pSessionItemData = ProfileApplicationTreeHandler::instance()->FindParentSessionItemData(m_pDisplayedSessionItemData);
        }

        afApplicationTreeItemData* pModulesItemData = CpuProfileTreeHandler::instance().findChildItemData(pSessionItemData, AF_TREE_ITEM_PROFILE_CPU_CALL_GRAPH);
        GT_IF_WITH_ASSERT((pModulesItemData != nullptr) && (pModulesItemData->m_pTreeWidgetItem != nullptr))
        {
            afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
            GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
            {
                afApplicationTree* pApplicationTree = pApplicationCommands->applicationTree();
                GT_IF_WITH_ASSERT(pApplicationTree != nullptr)
                {
                    pApplicationTree->selectItem(pModulesItemData, true);
                    pApplicationTree->expandItem(pModulesItemData->m_pTreeWidgetItem);
                }
            }
        }
    }
}


void DataTab::openCallGraphViewForFunction(const QString& funcName, ProcessIdType pid)
{
    // Make sure that the function view is opened for this session:
    openCallGraphView();

    // Get the tree instance:
    afApplicationCommands* pCommands = afApplicationCommands::instance();
    SessionViewCreator* pSessionViewCreator = AmdtCpuProfiling::sessionViewCreator();

    GT_IF_WITH_ASSERT((pCommands != nullptr) && (pSessionViewCreator != nullptr) && (m_pDisplayedSessionItemData != nullptr))
    {
        CpuSessionWindow* pSessionWindow = nullptr;
        const gtVector<CpuSessionWindow*>& openedSessions = pSessionViewCreator->currentlyOpenedSessionWindows();

        for (int i = 0; i < (int)openedSessions.size(); i++)
        {
            if (openedSessions[i] != nullptr)
            {
                if (openedSessions[i]->displayedSessionFilePath() == m_pDisplayedSessionItemData->m_filePath)
                {
                    pSessionWindow = openedSessions[i];
                }
            }
        }

        GT_IF_WITH_ASSERT(pSessionWindow != nullptr)
        {
            pSessionWindow->onViewCallGraphView(pid);
            SessionCallGraphView* pCallGraphTab = pSessionWindow->sessionCallGraphTab();
            GT_IF_WITH_ASSERT((pCallGraphTab != nullptr) && (!funcName.isEmpty()))
            {
                pCallGraphTab->selectFunction(funcName, pid);
            }
        }
    }
}

void DataTab::UpdateTableDisplaySettings()
{
    if (!m_isUpdatingData)
    {
        m_isUpdatingData = true;

        if (m_updateChangeType != UPDATE_TABLE_NONE)
        {
            // Update the table:
            ProtectedUpdateTableDisplay(m_updateChangeType);

            // Update the displayed string:
            updateDisplaySettingsString();

            // Apply user display information:
            applyUserDisplayInformation();

            m_updateChangeType = UPDATE_TABLE_NONE;
        }

        m_isUpdatingData = false;
    }
}

SessionDisplaySettings* DataTab::CurrentSessionDisplaySettings()
{
    SessionDisplaySettings* pRetVal = nullptr;
    GT_IF_WITH_ASSERT(m_pParentSessionWindow != nullptr)
    {
        pRetVal = m_pParentSessionWindow->sessionDisplaySettings();
    }
    return pRetVal;
}

bool DataTab::ProcessNameToPID(const QString& processName, ProcessIdType& pid)
{
    bool retVal = false;

    QString number;
    int i = processName.lastIndexOf("(");

    if (i >= 0)
    {
        number = processName.mid(i + 1, processName.length() - i - 2);
    }
    else
    {
        // All string should be a number:
        number = processName;
    }

    pid = number.toUInt(&retVal);

    return retVal;

}

bool DataTab::PIDToProcessName(ProcessIdType pid, QString& processName)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pProfileReader != nullptr)
    {
        PidProcessMap::iterator iter = m_pProfileReader->getProcessMap()->find(pid);

        if (iter != m_pProfileReader->getProcessMap()->end())
        {
            retVal = true;
            QString processPath = acGTStringToQString(iter->second.getPath());

            QFileInfo fi(processPath);

            if (processPath.isEmpty())
            {
                processName = QString("%1").arg(pid);
            }
            else
            {
                processName = QString("%1(%2)").arg(fi.fileName()).arg(pid);
            }
        }
    }
    return retVal;
}

void DataTab::UpdateNoteWindowContent(gtVector<float>& cluDataVector)
{
    if (m_pSessionDisplaySettings->m_displayClu)
    {
        int countSeperateBy = 1;
        QString strSeperateBy;
        int cluDataCount = IBS_CLU_OFFSET(IBS_CLU_END) + 1;
        GT_IF_WITH_ASSERT(m_pSessionDisplaySettings != nullptr)
        {
            if (m_pSessionDisplaySettings->m_separateBy != SEPARATE_BY_NONE)
            {
                countSeperateBy = cluDataVector.size() / cluDataCount;

                if (m_pSessionDisplaySettings->m_separateBy == SEPARATE_BY_CORE)
                {
                    strSeperateBy = "Core ";
                }
                else if (m_pSessionDisplaySettings->m_separateBy == SEPARATE_BY_NUMA)
                {
                    strSeperateBy = "Numa ";
                }
            }
        }

        m_pNoteWidget->clear();

        gtVector<float> cluDataSeperateBy;
        gtVector<float>::iterator it = cluDataVector.begin();
        QString str;

        for (int i = 0; i < countSeperateBy; i++)
        {
            cluDataSeperateBy.clear();
            cluDataSeperateBy.insert(cluDataSeperateBy.begin(),
                                     it + (i * cluDataCount),
                                     it + ((i * cluDataCount) + cluDataCount));

            if (m_pSessionDisplaySettings->m_separateBy != SEPARATE_BY_NONE)
            {
                str += strSeperateBy + QString::number(i) + ":\n";
            }

            str += GetNoteString(cluDataSeperateBy);
            str.append("\n");
        }

        if (m_pNoteWidget != nullptr)
        {
            m_pNoteWidget->setText(str);
        }
    }
}

QString DataTab::GetNoteString(gtVector<float>& cluDataVector)
{
    QString str;
    QList<CLU_NOTE> noteList;

    bool iscluDataAvailable = false;

    for (int id = 0; id < (int)cluDataVector.size(); ++id)
    {
        if (cluDataVector[id] != 0)
        {
            iscluDataAvailable = true;
            break;
        }
    }

    if (iscluDataAvailable)
    {
        GetNoteList(cluDataVector, noteList);

        if (0 == noteList.size())
        {
            str = "No specific cache line utilization notes for this selection\n";
        }
        else
        {
            for (int i = 0; i < noteList.size(); ++i)
            {
                str += m_CLUNoteStringList.at(noteList.at(i));
                str.append("\n");
            }

        }
    }
    else
    {
        str = "No cache line utilization data available for this selection\n";
    }

    return str;
}


void DataTab::GetNoteList(gtVector<float>& cluDataVector, QList<CLU_NOTE>& noteList)
{
    noteList.clear();
    float utilization = cluDataVector[IBS_CLU_OFFSET(DE_IBS_CLU_PERCENTAGE)];
    float numSpans = cluDataVector[IBS_CLU_OFFSET(DE_IBS_CLU_SPANNING)];
    float totalEvict = cluDataVector[IBS_CLU_OFFSET(DE_IBS_CLU_EVICT_COUNT)];
    float numAccesses = cluDataVector[IBS_CLU_OFFSET(DE_IBS_CLU_ACCESS_COUNT)];

    int note = 0;

    if ((totalEvict > 1) && (utilization < THRESHOLD_CLU_UTIL_MED))
    {
        note = ((utilization <= THRESHOLD_CLU_UTIL_LOW) ? CLU_NOTE_LOW_UTILIZATION : CLU_NOTE_MEDIUM_UTILIZATION);
        noteList.append((CLU_NOTE)note);

        if (totalEvict < (0.1 * numAccesses))
        {
            if (CLU_NOTE_MEDIUM_UTILIZATION == note)
            {
                note = CLU_NOTE_MED_CLU_HIGH_ACCESS_RATE;
            }
            else
            {
                note = CLU_NOTE_LOW_CLU_HIGH_ACCESS_RATE;
            }

            noteList.append((CLU_NOTE)note);
        }
    }

    if (numSpans > 0)
    {
        note = (numSpans > THRESHOLD_CLU_SPAN) ? CLU_NOTE_SPAN_OVER_THRESHOLD : CLU_NOTE_SPAN;
        noteList.append((CLU_NOTE)note);
    }

    if (totalEvict == 1)
    {
        if (utilization <= THRESHOLD_CLU_UTIL_MED)
        {
            note = CLU_NOTE_COMPULSORY;
            noteList.append((CLU_NOTE)note);
        }
    }
}

void DataTab::OnApplicationFocusChanged(QWidget* pOld, QWidget* pNew)
{
    Q_UNUSED(pOld);

    if (m_editActionsWidgetsList.contains(pNew))
    {
        m_pLastFocusedWidget = pNew;
    }
}

const QComboBox* DataTab::TopToolbarComboBox(acWidgetAction* pComboAction)
{
    const QComboBox* pRetVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT((pComboAction != nullptr) && (m_pTopToolbar != nullptr))
    {
        pRetVal = qobject_cast<QComboBox*>(m_pTopToolbar->widgetForAction(pComboAction));
    }

    return pRetVal;
}


const QLabel* DataTab::TopToolbarLabel(acWidgetAction* pLabelAction)
{
    QLabel* pRetVal = nullptr;

    // Sanity check:
    if ((pLabelAction != nullptr) && (m_pTopToolbar != nullptr))
    {
        pRetVal = qobject_cast<QLabel*>(m_pTopToolbar->widgetForAction(pLabelAction));
    }

    return pRetVal;
}

void DataTab::SetUpdateType(unsigned int changeType)
{
    // Accumulate the change type needs to be performed:
    m_updateChangeType = m_updateChangeType | changeType;
}

void DataTab::ProtectedUpdateTableDisplay(unsigned int changeType)
{
    // get the mdi tab if in SA and block its closing while updating (bug-1644 should be removed when cancel will be available)
    afQMdiSubWindow* pMdiWindow = nullptr;
    bool previousCloseMode = true;

    if (!afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
    {
        afMainAppWindow* pMainWindow = afMainAppWindow::instance();

        if (pMainWindow != nullptr && m_pParentSessionWindow != nullptr)
        {
            pMdiWindow = pMainWindow->findMDISubWindow(m_pParentSessionWindow->displayedSessionFilePath());

            if (pMdiWindow != nullptr)
            {
                previousCloseMode = pMdiWindow->IsAllowedToBeClosed();
                pMdiWindow->SetAllowedToBeClosed(false);
            }
        }
    }

    UpdateTableDisplay(changeType);

    // after update is done allow closing
    if (pMdiWindow != nullptr)
    {
        pMdiWindow->SetAllowedToBeClosed(previousCloseMode);
    }
}
ProfileViewDisplayInformation::ProfileViewDisplayInformation() : m_selectedTopLevelItemIndex(-1),
    m_selectedTopLevelChildItemIndex(-1),
    m_sortByColumn(-1),
    m_sortOrder(Qt::AscendingOrder),
    m_shouldExpandSelected(false),
    m_maxVerticalScrollPosition(0),
    m_verticalScrollPosition(-1),
    m_selectedHotSpot("") {}

void ProfileViewDisplayInformation::clear()
{
    m_selectedTopLevelItemIndex = -1;
    m_selectedTopLevelChildItemIndex = -1;
    m_sortByColumn = -1;
    m_sortOrder = Qt::AscendingOrder;
    m_shouldExpandSelected = false;
    m_maxVerticalScrollPosition = 0;
    m_verticalScrollPosition = -1;
    m_expandedTreeItems.clear();
    m_selectedHotSpot = "";
}

