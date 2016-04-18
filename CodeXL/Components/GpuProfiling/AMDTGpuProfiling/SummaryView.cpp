//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/SummaryView.cpp $
/// \version $Revision: #35 $
/// \brief :  This file contains SummaryView
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/SummaryView.cpp#35 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================

#include <qtIgnoreCompilerWarnings.h>
#include <QtCore>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTAPIClasses/Include/Events/apMonitoredObjectsTreeEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTree.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/ProfileApplicationTreeHandler.h>

#include <AMDTApplicationFramework/src/afUtils.h>

#include "SummaryView.h"


SummaryView::SummaryView(QWidget* parent) :
    QWidget(parent),
    m_pSummarizer(nullptr),
    m_newestPageIndex(-1),
    m_currentPageIndex(-1),
    m_loading(false),
    m_pContextMenu(nullptr),
    m_pCopyAction(nullptr),
    m_pSelectAllAction(nullptr)
{
    setupUi(this);

    // Set the zoom factor to fit the resolution. QWebView assumes 96 DPI.
    QWidget* pScreen = QApplication::desktop()->screen();

    if (pScreen != nullptr)
    {
        const int horizontalDpi = pScreen->logicalDpiX();
        webView->setZoomFactor(horizontalDpi / 96.0);
    }

    webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    webView->setContextMenuPolicy(Qt::NoContextMenu);
    connect(webView, SIGNAL(linkClicked(const QUrl&)), this, SLOT(LinkClickedHandler(const QUrl&)));

    m_pContextMenu = new QMenu(this);


    // Add the actions to the table:
    m_pCopyAction = m_pContextMenu->addAction(AF_STR_CopyA, this, SLOT(OnEditCopy()));
    m_pSelectAllAction = m_pContextMenu->addAction(AF_STR_SelectAllA, this, SLOT(OnEditSelectAll()));

    m_pContextMenu->addSeparator();
    m_pSelectAllAction = m_pContextMenu->addAction(AF_STR_ExportToCSV, this, SLOT(OnExportToCSV()));

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnContextMenu(const QPoint&)));
}

SummaryView::~SummaryView()
{
}


bool SummaryView::LoadSession(TraceSession* pSession, CLSummarizer* pSummarizer)
{
    bool retVal = false;

    m_pDisplayedTraceSession = pSession;
    GT_IF_WITH_ASSERT(m_pDisplayedTraceSession != nullptr && pSummarizer != nullptr)
    {
        Reset();

        m_pSummarizer = pSummarizer;
        m_pSummarizer->CreateSummaryPages();

        if (m_pSummarizer->GetSummaryPagesMap().count() != 0)
        {
            int i = 0;
            QString selPage;
            bool initialPageSelected = false;

            foreach (QString page, m_pSummarizer->GetSummaryPagesMap().keys())
            {
                if (selPage.isEmpty())
                {
                    selPage = page;
                }

                comboBoxPages->addItem(page);

                if (!initialPageSelected)
                {
                    if (GetHasErrorWarningPage())
                    {
                        if (page.endsWith(Util::ms_BESTPRACTICES)) // focus first best practices summary page
                        {
                            selPage = page;
                            initialPageSelected = true;
                        }
                    }
                    else if (page.endsWith(Util::ms_CTXSUM))  // focus first context summary page
                    {
                        selPage = page;
                        initialPageSelected = true;
                    }
                }

                i++;
            }

            if (!selPage.trimmed().isEmpty())
            {
                webView->setUrl(QUrl::fromLocalFile(m_pSummarizer->GetSummaryPagesMap()[selPage]));
            }

            connect(comboBoxPages, SIGNAL(currentIndexChanged(int)), this, SLOT(SelectedPageChanged()));
            m_loading = true;
            comboBoxPages->setCurrentText(selPage);
            m_loading = false;
            retVal = true;
        }
    }

    return retVal;
}

void SummaryView::Reset()
{
    m_pSummarizer = nullptr;
    comboBoxPages->clear();
}

bool SummaryView::GetHasErrorWarningPage()
{
    bool retVal = false;

    if (m_pSummarizer)
    {
        retVal = m_pSummarizer->GetHasErrorWarningPage();
    }

    return retVal;
}

void SummaryView::SelectedPageChanged()
{
    ProfileApplicationTreeHandler* pTreeHandler = ProfileApplicationTreeHandler::instance();
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT((m_pSummarizer != nullptr) && (m_pDisplayedTraceSession != nullptr) && (pTreeHandler != nullptr) && (pApplicationCommands != nullptr))
    {
        gtString message;
        message.appendFormattedString(L"m_loading = %d", m_loading);

        if (!m_loading)
        {
            // Get the item data for the session:
            afApplicationTreeItemData* pItemData = m_pDisplayedTraceSession->m_pParentData;
            GT_IF_WITH_ASSERT(pItemData != nullptr)
            {
                message.appendFormattedString(L". m_profileOutputFilePath: %ls", m_pDisplayedTraceSession->m_pParentData->m_filePath.asString().asCharArray());
                // Get the item data for the item representing the summary type in the tree:
                afTreeItemType treeItemType = Util::GetEnumTypeFromSumPageName(comboBoxPages->currentText());

                message.appendFormattedString(L". treeItemType: %d. CurrentText: %ls", (int)treeItemType, acQStringToGTString(comboBoxPages->currentText()).asCharArray());

                afApplicationTreeItemData* pItemTypeItemData = pTreeHandler->FindSessionChildItemData(pItemData, treeItemType);

                if (pItemTypeItemData != nullptr)
                {
                    message.append(L" Got it");
                    afApplicationTree* pApplicationTree = pApplicationCommands->applicationTree();
                    GT_IF_WITH_ASSERT(pApplicationTree != nullptr)
                    {
                        pApplicationTree->selectItem(pItemTypeItemData, true);
                        pApplicationTree->expandItem(pItemTypeItemData->m_pTreeWidgetItem);
                    }
                }
            }
        }

        QString key = comboBoxPages->currentText();
        QString path = m_pSummarizer->GetSummaryPagesMap()[key];

        webView->setUrl(QUrl::fromLocalFile(path));
    }
}

void SummaryView::LinkClickedHandler(const QUrl& url)
{
    QString format = url.fragment();

    // QUrlQuery does not handle "?" like QUrl old function did.
    // in case the format starts with ? remove it
    if (format.startsWith("?"))
    {
        format.remove(0, 1);
    }

    QUrlQuery fragUrl(format);

    bool hasThread = fragUrl.hasQueryItem(gs_THREAD_ID_TAG);

    QString strQueryTid = fragUrl.queryItemValue(gs_THREAD_ID_TAG);
    QString strQuerySeqid = fragUrl.queryItemValue(gs_SEQUENCE_ID_TAG);
    QString strQueryView = fragUrl.queryItemValue(gs_VIEW_TAG);

    bool ok = false;
    unsigned int threadId = 0;

    if (hasThread)
    {
        threadId = strQueryTid.toUInt(&ok);

        if (!ok)
        {
            return;
        }
    }

    unsigned int callIndex = strQuerySeqid.toUInt(&ok);

    if (!ok)
    {
        return;
    }

    AnalyzerHTMLViewType viewType = StringToAnalyzerHTMLType(strQueryView.toStdString());

    // navigate to the block in the timeline/trace
    emit LinkClicked(comboBoxPages->currentText(), threadId, callIndex, viewType);
}

int SummaryView::GetComboIndexByPageName(const QString& name)
{
    int index = -1;

    for (int i = 0; i < comboBoxPages->count(); i++)
    {
        if (comboBoxPages->itemText(i).endsWith(name))
        {
            index = i;
            break;
        }
    }

    return index;
}

void SummaryView::DisplaySummaryPageType(int type)
{
    m_loading = true;

    GT_IF_WITH_ASSERT(comboBoxPages != nullptr && comboBoxPages->count() > 0)
    {
        gtString message;
        message.appendFormattedString(L"type = %d, ", type);
        QString page = acGTStringToQString(Util::SummaryTypeToGTString(static_cast<afTreeItemType>(type)));

        message.appendFormattedString(L"page = %ls, ", acQStringToGTString(page).asCharArray());
        // get the combo item string - not by findText - because on remote sessions the pages name has prefix
        int index = GetComboIndexByPageName(page);

        message.appendFormattedString(L"index = %d, ", index);

        // in case of bad index - put default
        if (index < 0)
        {
            message.appendFormattedString(L" Index < 0");

            // set the index default to be first Warning(s)/Error(s)
            index = GetComboIndexByPageName(Util::ms_BESTPRACTICES);

            // if not exist set it to be Context Summary
            if (index < 0)
            {
                message.appendFormattedString(L" Index < 0");

                index = GetComboIndexByPageName(Util::ms_CTXSUM);
            }

            // if not exist set to first in combo
            if (index < 0)
            {
                message.appendFormattedString(L" Index < 0");

                index = 0;

                GT_ASSERT_EX(false, L"Should not get here. ");

            }
        }

        comboBoxPages->setCurrentIndex(index);
    }

    m_loading = false;
}

void SummaryView::OnEditCopy()
{
    webView->triggerPageAction(QWebPage::Copy);
}

void SummaryView::OnEditSelectAll()
{
    webView->triggerPageAction(QWebPage::SelectAll);
}

void SummaryView::OnContextMenu(const QPoint& point)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pContextMenu != nullptr)
    {
        m_pContextMenu->exec(acMapToGlobal(this, point));
    }
}

void SummaryView::OnExportToCSV()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(comboBoxPages != nullptr)
    {
        // The file path for the saved CSV file:
        QString csvFilePathStr;

        // Get the output file name:
        QString fileNamePostfix = Util::GetShortFileNameFromSumPageName(comboBoxPages->currentText());

        // Build the CSV default file name:
        QString fileName = QString(GPU_CSV_FileNameFormat).arg(m_pDisplayedTraceSession->m_displayName).arg(fileNamePostfix);
        bool rc = afApplicationCommands::instance()->ShowQTSaveCSVFileDialog(csvFilePathStr, fileName, this);
        GT_IF_WITH_ASSERT(rc)
        {
            // Export the web view table to a CSV file:
            rc = acExportHTMLTableToCSV(csvFilePathStr, webView);
            GT_ASSERT(rc);
        }
    }
}

