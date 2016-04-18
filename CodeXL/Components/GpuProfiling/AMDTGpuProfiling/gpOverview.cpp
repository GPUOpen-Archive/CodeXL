//------------------------------ gpOverview.cpp ------------------------------

#include <qtIgnoreCompilerWarnings.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acTreeCtrl.h>

// Framework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afCSSSettings.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/afTreeItemType.h>

#include <AMDTGraphicsServerInterface/Include/AMDTGraphicsServerInterface.h>

// Local:
#include <AMDTGpuProfiling/gpViewsCreator.h>
#include <AMDTGpuProfiling/gpCountersSelectionDialog.h>
#include <AMDTGpuProfiling/gpOverview.h>
#include <AMDTGpuProfiling/gpExecutionMode.h>
#include <AMDTGpuProfiling/ProfileManager.h>

#define GP_IMAGE_SECTION_WIDTH 512
// ---------------------------------------------------------------------------
// Name:        gpOverview
// Description: constructor
// Author:      Gilad Yarnitzky
// Date:        18/8/2013
// ---------------------------------------------------------------------------
gpOverview::gpOverview(QWidget* pParent) : gpBaseSessionView(pParent),
    m_pMainLayout(nullptr), m_pFrameDescriptionLabel(nullptr), m_pTimelineLabel(nullptr),
    m_pPerfCountersLabel(nullptr), m_pPredefinedCountersComboBox(nullptr), m_pCountersLabel(nullptr),
    m_pCountersTree(nullptr), m_pCounterDescLabel(nullptr), m_pAddRemoveButton(nullptr),
    m_pCaptionLabel(nullptr), m_pImageLabel(nullptr), m_pStatisticsHTMLLabel(nullptr), m_pRenderButton(nullptr)
{
    // Init the view layout
    InitLayout();

    // Set white background
    QPalette p = palette();
    p.setColor(backgroundRole(), Qt::white);
    p.setColor(QPalette::Base, Qt::white);
    setAutoFillBackground(true);
    setPalette(p);
}

// ---------------------------------------------------------------------------
// Name:        gpOverview::~gpOverview
// Description: destructor
// Author:      Gilad Yarnitzky
// Date:        18/8/2013
// ---------------------------------------------------------------------------
gpOverview::~gpOverview()
{
    // Remove me from the list of session windows in the session view creator:
    gpViewsCreator::Instance()->OnWindowClose(this);
}

void gpOverview::InitLayout()
{
    m_pMainLayout = new QVBoxLayout(this);

    m_pCaptionLabel = new QLabel;
    m_pMainLayout->addWidget(m_pCaptionLabel);

    // layout to hold the two main vboxlayouts;
    QHBoxLayout* pHLayout = new QHBoxLayout;
    m_pMainLayout->addLayout(pHLayout);

    // The two vboxlayouts that hold all the information to the user
    QVBoxLayout* pInfoLayout = new QVBoxLayout;
    QVBoxLayout* pImageLayout = new QVBoxLayout;
    pHLayout->addLayout(pInfoLayout, 1);
    pHLayout->addLayout(pImageLayout, 1);

    // Create the frame analysis section:
    m_pFrameDescriptionLabel = new QLabel(GPU_STR_dashboard_FrameAnalysisCaption);
    m_pFrameDescriptionLabel->setStyleSheet(AF_STR_captionLabelStyleSheetMain);

    m_pTimelineLabel = new QLabel(GPU_STR_overviewTimelineLabel);
    m_pPerfCountersLabel = new QLabel("Profile");

    pInfoLayout->addWidget(m_pFrameDescriptionLabel);
    pInfoLayout->addWidget(m_pTimelineLabel);

#ifdef GP_OBJECT_VIEW_ENABLE
    m_pObjectLabel = new QLabel(GPU_STR_overviewObjectLabel);
    // TODO, add/enable object label
    // pInfoLayout->addWidget(m_pObjectLabel);
#endif
    pInfoLayout->addWidget(m_pPerfCountersLabel);

    // Create the PerfCounter section
    QLabel* pPerfCountersLabel = new QLabel(GPU_STR_overview_PerfCounterProfileCaption);
    pPerfCountersLabel->setStyleSheet(AF_STR_captionLabelStyleSheetMain);
    QLabel* pPreDefinedSetsLabel = new QLabel(GPU_STR_overview_PresetLabel);
    m_pPredefinedCountersComboBox = new QComboBox();

    m_pCountersLabel = new QLabel;
    m_pCountersTree = new acTreeCtrl(this);
    m_pCountersTree->setHeaderHidden(true);
    m_pCounterDescLabel = new QLabel();
    m_pAddRemoveButton = new QPushButton(GPU_STR_overview_AddRemoveButton);

    pInfoLayout->addWidget(pPerfCountersLabel);
    QHBoxLayout* pPreDefLayout = new QHBoxLayout;
    pPreDefLayout->addWidget(pPreDefinedSetsLabel);
    pPreDefLayout->addWidget(m_pPredefinedCountersComboBox);
    pInfoLayout->addLayout(pPreDefLayout);
    pInfoLayout->addWidget(m_pCountersLabel);
    pInfoLayout->addWidget(m_pCountersTree);
    pInfoLayout->addWidget(m_pCounterDescLabel);
    pInfoLayout->addWidget(m_pAddRemoveButton, 0, Qt::AlignRight);

    // Create the image section
    m_pImageLabel = new QLabel;
    m_pImageLabel->setMinimumHeight(256);
    m_pImageLabel->setMinimumWidth(256);

    m_pImageLabel->setMaximumWidth(GP_IMAGE_SECTION_WIDTH);

    pImageLayout->addWidget(m_pImageLabel);

    QLabel* pFrameStatLabel = new QLabel(GPU_STR_overview_FrameStatCaption);
    pFrameStatLabel->setStyleSheet(AF_STR_captionLabelStyleSheetMain);
    m_pStatisticsHTMLLabel = new QLabel;
    m_pRenderButton = new QPushButton(GPU_STR_overviewRenderAgain);

    pImageLayout->addWidget(pFrameStatLabel);
    pImageLayout->addWidget(m_pStatisticsHTMLLabel);
    pImageLayout->addWidget(m_pRenderButton, 0, Qt::AlignLeft);
    pImageLayout->addStretch(1);

    // Set the connections
    bool rc = connect(m_pRenderButton, SIGNAL(clicked()), this, SLOT(OnRenderButton()));
    GT_ASSERT(rc);

    rc = connect(m_pTimelineLabel, SIGNAL(linkActivated(const QString&)), this, SLOT(OnTimelineClick()));
    GT_ASSERT(rc);

    rc = connect(m_pPerfCountersLabel, SIGNAL(linkActivated(const QString&)), this, SLOT(OnProfileClick(const QString&)));
    GT_ASSERT(rc);

    rc = connect(m_pPredefinedCountersComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(OnUpdatePresetChanged(const QString&)));
    GT_ASSERT(rc);

    rc = connect(m_pCountersTree, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(OnItemClicked(QTreeWidgetItem*)));
    GT_ASSERT(rc);

    rc = connect(m_pAddRemoveButton, SIGNAL(pressed()), this, SLOT(OnAddRemoveCounters()));
    GT_ASSERT(rc);

#ifdef GP_OBJECT_VIEW_ENABLE
    // TODO add object link
    rc = connect(m_pObjectLabel, SIGNAL(linkActivated(const QString&)), this, SLOT(OnObjectClick()));
    GT_ASSERT(rc);
#endif

    setLayout(m_pMainLayout);
}

bool gpOverview::DisplaySession(const osFilePath& sessionFilePath, afTreeItemType sessionInnerPage)
{
    GT_UNREFERENCED_PARAMETER(sessionInnerPage);

    //
    bool retVal = false;

    // Set the session file path
    m_sessionFilePath = sessionFilePath;

    // Extract the frame index from the file path
    gtString fileName;
    m_sessionFilePath.getFileName(fileName);
    int pos = fileName.findLastOf(L"_");

    GT_IF_WITH_ASSERT(pos >= 0)
    {
        gtString frameIndexStr;
        fileName.getSubString(pos + 1, fileName.length(), frameIndexStr);
        int frameIndex = -1;
        retVal = frameIndexStr.toIntNumber(frameIndex);
        GT_IF_WITH_ASSERT(retVal)
        {
            // Get the frame info and thumb data
            QString overviewFilePath = acGTStringToQString(m_sessionFilePath.asString());
            retVal = gpUIManager::Instance()->GetFrameImageAndInfo(overviewFilePath, m_frameInfo);

            GT_IF_WITH_ASSERT(retVal && (m_pImageLabel != nullptr))
            {
                // Set the frame image
                QPixmap thumbnailPixmap;
                thumbnailPixmap.loadFromData((const uchar*)m_frameInfo.m_pImageBuffer, m_frameInfo.m_imageSize, "bmp");
                m_pImageLabel->setPixmap(thumbnailPixmap);

                // Update the frame information on the relevant widgets
                DisplayFrameInfo();
            }
        }
    }

    return retVal;
}

void gpOverview::SetProfileDataModel(gpTraceDataModel* pSessionDataModel)
{
    gpBaseSessionView::SetProfileDataModel(pSessionDataModel);

    // Sanity check
    gpExecutionMode* pFrameAnalysisManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();
    GT_IF_WITH_ASSERT((pFrameAnalysisManager != nullptr) && (m_pPredefinedCountersComboBox != nullptr))
    {
        // Add the sets to the combobox
        std::map<QString, QStringList> presetCountersLists = pFrameAnalysisManager->ProjectSettings().PresetCountersLists();
        std::map<QString, QStringList>::iterator presetIt = presetCountersLists.begin();

        while (presetIt != presetCountersLists.end())
        {
            m_pPredefinedCountersComboBox->addItem((*presetIt).first);
            presetIt++;
        }

        if (m_pPredefinedCountersComboBox->count() > 0)
        {
            m_pPredefinedCountersComboBox->setCurrentText(m_pPredefinedCountersComboBox->itemText(0));
        }
    }
}

void gpOverview::OnRenderButton()
{
    GT_ASSERT_EX(false, L"Implement");
    acMessageBox::instance().information("CodeXL", "Not implemented yet");
}

void gpOverview::OnTimelineClick()
{
    // Open the frame view, with AF_TREE_ITEM_GP_FRAME_TIMELINE as inner item
    bool rc = afApplicationCommands::instance()->OpenFileAtLine(m_sessionFilePath, AF_TREE_ITEM_GP_FRAME_TIMELINE, 0);
    GT_ASSERT(rc);
}

void gpOverview::OnObjectClick()
{
#ifdef GP_OBJECT_VIEW_ENABLE
    // Open the frame view, with AF_TREE_ITEM_GP_FRAME_OBJECTINSPCTOR as inner item
    bool rc = afApplicationCommands::instance()->OpenFileAtLine(m_sessionFilePath, AF_TREE_ITEM_GP_FRAME_OBJECTINSPCTOR, 0);
    GT_ASSERT(rc);
#endif
}

void gpOverview::OnProfileClick(const QString& link)
{
    GT_UNREFERENCED_PARAMETER(link);

}

void gpOverview::OnUpdatePresetChanged(const QString& presetSelected)
{
    // Clear the tree and add all counters that are in the set if they are available
    if (m_pCountersTree != nullptr)
    {
        m_pCountersTree->clear();

        // Sanity check:
        gpExecutionMode* pFrameAnalysisManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();
        GT_IF_WITH_ASSERT(pFrameAnalysisManager != nullptr)
        {
            std::map<QString, QStringList>& presetCountersLists = pFrameAnalysisManager->ProjectSettings().PresetCountersLists();
            const std::map<QString, gpPerfCounter*>& countersMap = pFrameAnalysisManager->ProjectSettings().CountersMap();
            GT_IF_WITH_ASSERT(presetCountersLists.count(presetSelected) == 1)
            {

                QStringList selectedPresetCountersList = presetCountersLists.at(presetSelected);

                int numCounters = selectedPresetCountersList.size();

                for (int nCounter = 0; nCounter < numCounters; nCounter++)
                {
                    // check if the counter is available for the specific device that we got the image from
                    if (countersMap.count(selectedPresetCountersList[nCounter]) == 1)
                    {
                        // Get the parent and check if it is already in the tree. if it is add this as his child
                        // if not add it as a node and then add it as his child
                        // in both cases put the counter data as a data of the tree node for usage
                        QString counter = selectedPresetCountersList.at(nCounter);
                        gpPerfCounter* pCounterData = countersMap.at(counter);

                        GT_IF_WITH_ASSERT(nullptr != pCounterData)
                        {
                            QTreeWidgetItem* pParent = nullptr;
                            QList<QTreeWidgetItem*> itemsFound = m_pCountersTree->findItems(pCounterData->m_parent, Qt::MatchCaseSensitive | Qt::MatchFixedString);

                            if (itemsFound.size() == 1)
                            {
                                pParent = itemsFound[0];
                            }
                            else if (itemsFound.size() == 0)
                            {
                                pParent = new QTreeWidgetItem(QStringList(pCounterData->m_parent));
                                m_pCountersTree->addTopLevelItem(pParent);
                            }
                            else
                            {
                                // There can't be more than one parent with the same name
                                GT_ASSERT(false);
                            }

                            GT_IF_WITH_ASSERT(pParent != nullptr)
                            {
                                QTreeWidgetItem* pChild = new QTreeWidgetItem(QStringList(pCounterData->m_name));
                                pParent->addChild(pChild);

                                QVariant dataAsVaraiant = qVariantFromValue((void*)pCounterData);
                                pChild->setData(0, Qt::UserRole, dataAsVaraiant);
                            }
                        }
                    }
                }
            }

            // clear the description
            if (m_pCounterDescLabel != nullptr)
            {
                m_pCounterDescLabel->setText("");
            }

            // Expand the tree
            m_pCountersTree->expandAll();

            // Send the selected counters to the server
            SendCountersSelectedToServer();

            // Update the view labels with the current selected performance counters
            UpdateSelectedPerformanceCountersTexts();
        }
    }
}


void gpOverview::OnItemClicked(QTreeWidgetItem* pItemSelected)
{
    // get the item data and see if it has any dat. if not it is a parent node
    if (pItemSelected != nullptr)
    {
        QVariant itemData = pItemSelected->data(0, Qt::UserRole);
        gpPerfCounter* pItemData = (gpPerfCounter*)itemData.value<void*>();

        if (pItemData != nullptr && m_pCounterDescLabel != nullptr)
        {
            m_pCounterDescLabel->setText(pItemData->m_description);
        }
    }
}

void gpOverview::SendCountersSelectedToServer()
{
    // Sanity check:
    gpExecutionMode* pFrameAnalysisManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();
    GT_IF_WITH_ASSERT(pFrameAnalysisManager != nullptr)
    {
        for (int i = 0; i < m_pCountersTree->topLevelItemCount(); i++)
        {
            QTreeWidgetItem* rootItem = m_pCountersTree->topLevelItem(i);

            for (int j = 0; j < rootItem->childCount(); j++)
            {
                QTreeWidgetItem* pChild = rootItem->child(j);
                QVariant itemData = pChild->data(0, Qt::UserRole);
                gpPerfCounter* pCounterData = (gpPerfCounter*)itemData.value<void*>();

                if (pCounterData != nullptr)
                {
                    pFrameAnalysisManager->ProjectSettings().AddSelectedCounter(pCounterData->m_id);
                }
            }
        }

        if (pFrameAnalysisManager->ProjectSettings().SessionSelectedCounters().size())
        {
            GraphicsServerCommunication* pServerComm = pFrameAnalysisManager->GetGraphicsServerComminucation();
            GT_IF_WITH_ASSERT(pServerComm != nullptr)
            {
                gtASCIIString srvReply;
                bool rc = pServerComm->SetCounters(pFrameAnalysisManager->ProjectSettings().SessionSelectedCounters(), srvReply);
                GT_ASSERT(rc);
            }
        }
    }
}

void gpOverview::UpdateSelectedPerformanceCountersTexts()
{
    // Sanity check:
    gpExecutionMode* pFrameAnalysisManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();

    if ((pFrameAnalysisManager != nullptr) && (m_pPerfCountersLabel != nullptr) &&
        (m_pCountersLabel != nullptr) && (pFrameAnalysisManager->ProjectSettings().SessionSelectedCounters().size() > 0))
    {
#pragma message ("TODO: FA: Calculate the real data")
        GT_ASSERT_EX(false, L"calculate the real data");
        int requiredPasses = qrand() % 8;
        int amountOfCounters = pFrameAnalysisManager->ProjectSettings().SessionSelectedCounters().size();
        int availableCount = qrand() % amountOfCounters;

        QString selectedCountersPassesText = QString(GPU_STR_overview_PerfCounterCheckBox).arg(amountOfCounters).arg(requiredPasses);
        m_pPerfCountersLabel->setText(selectedCountersPassesText);

        QString countersTitle = QString(GPU_STR_overview_CountersSelectedLabel).arg(amountOfCounters).arg(availableCount).arg(requiredPasses);
        m_pCountersLabel->setText(countersTitle);
    }
}

void gpOverview::DisplayFrameInfo()
{
    GT_IF_WITH_ASSERT((m_pFrameDescriptionLabel != nullptr) && (m_pStatisticsHTMLLabel != nullptr))
    {
        // Update the frame description label
        QString frameDesc = QString(GPU_STR_overview_FrameAnalysisDescriptionCaption).arg(m_frameInfo.m_frameIndex).arg(m_frameInfo.m_frameDuration);
        m_pFrameDescriptionLabel->setText(frameDesc);

        afHTMLContent htmlContent;

        gtString htmlStr, cpuTime, gpuTime, gpuTimePercentage, gpuTimeBusy, cpuTimeDraw, apiCalls, drawCalls;
        apiCalls.appendFormattedString(L"%d", m_frameInfo.m_apiCalls);
        drawCalls.appendFormattedString(L"%d", m_frameInfo.m_drawCalls);
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_BOLD_LINE, GPU_STR_overview_HTMLCPUTime, cpuTime);
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_BOLD_LINE, GPU_STR_overview_HTMLGPUTime, gpuTime);
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_BOLD_LINE, GPU_STR_overview_HTMLGPUTimeBusy, gpuTimePercentage);
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_BOLD_LINE, GPU_STR_overview_HTMLCPUTimeDrawCalls, cpuTimeDraw);
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_BOLD_LINE, GPU_STR_overview_HTMLAPICallsCount, apiCalls);
        htmlContent.addHTMLItem(afHTMLContent::AP_HTML_BOLD_LINE, GPU_STR_overview_HTMLDrawCallsCount, drawCalls);
        htmlContent.toString(htmlStr);

        // Set the statistics in the widget
        m_pStatisticsHTMLLabel->setText(acGTStringToQString(htmlStr));

    }
}

void gpOverview::OnAddRemoveCounters()
{
    QString selectedPreset = m_pPredefinedCountersComboBox->currentText();

    gpCountersSelectionDialog countersSelectionDialog(selectedPreset);

    // Sanity check:
    gpExecutionMode* pFrameAnalysisManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();
    GT_IF_WITH_ASSERT(pFrameAnalysisManager != nullptr)
    {

        int rc = afApplicationCommands::instance()->showModal(&countersSelectionDialog);

        if (QDialog::Accepted == rc)
        {
            // update the preset combo, select the new preset and save the presets
            m_pPredefinedCountersComboBox->addItem(countersSelectionDialog.NewPresetName());
            m_pPredefinedCountersComboBox->setCurrentText(countersSelectionDialog.NewPresetName());

            pFrameAnalysisManager->ProjectSettings().SavePresetCountersLists();
        }
    }
}
