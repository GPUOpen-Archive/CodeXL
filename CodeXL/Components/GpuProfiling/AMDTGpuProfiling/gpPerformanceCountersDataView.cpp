//------------------------------ gpPerformanceCountersDataView.cpp ------------------------------

#include <qtIgnoreCompilerWarnings.h>

// Qt:
#include <QDomDocument>

// Infra:
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>

// Local:
#include <AMDTGpuProfiling/gpCountersSelectionDialog.h>
#include <AMDTGpuProfiling/gpExecutionMode.h>
#include <AMDTGpuProfiling/gpPerformanceCountersDataView.h>
#include <AMDTGpuProfiling/gpUIManager.h>
#include <AMDTGpuProfiling/gpViewsCreator.h>
#include <AMDTGpuProfiling/Session.h>
#include <AMDTGpuProfiling/ProfileManager.h>


gpPerformanceCountersDataView::gpPerformanceCountersDataView(QWidget* parent) : gpBaseSessionView(parent),
    m_pPerformanceCountersTree(nullptr),
    m_pSummaryWidget(nullptr),
    m_pCountersDescriptionLabel(nullptr),
    m_pSummaryLabel(nullptr),
    m_pDisplaySettingsLabel(nullptr)
{
    // Set the erase back ground to mode to correctly draw the background when needed
    setAutoFillBackground(true);

    QVBoxLayout* pMainLayout = new QVBoxLayout;
    m_pSummaryWidget = new QWidget;

    QGridLayout* pSummaryLayout = new QGridLayout;

    m_pCountersDescriptionLabel = new QLabel("Preset collection: Assess Performance. 13 Counters <a href='select_counters'>selected</a>.");
    m_pCountersDescriptionLabel->setTextFormat(Qt::RichText);

    m_pSummaryLabel = new QLabel();
    m_pSummaryLabel->setTextFormat(Qt::RichText);
    m_pSummaryLabel->setText("");

    // Set the current summary data in m_pSummaryLabel
    BuildSummaryData();

    m_pDisplaySettingsLabel = new QLabel("<a href='display_options'>Display options</a>.");
    m_pDisplaySettingsLabel->setTextFormat(Qt::RichText);

    bool rc = connect(m_pCountersDescriptionLabel, SIGNAL(linkActivated(const QString&)), this, SLOT(OnCountersSelectionClick()));
    GT_ASSERT(rc);
    rc = connect(m_pDisplaySettingsLabel, SIGNAL(linkActivated(const QString&)), this, SLOT(OnDisplaySettingsClick()));
    GT_ASSERT(rc);

    pSummaryLayout->addWidget(m_pCountersDescriptionLabel, 0, 0, 1, 2);
    pSummaryLayout->addWidget(m_pSummaryLabel, 1, 0, 1, 1);
    pSummaryLayout->addWidget(m_pDisplaySettingsLabel, 1, 1, 1, 1);

    m_pSummaryWidget->setLayout(pSummaryLayout);

    QStringList headerCaptions;
    headerCaptions << GPU_STR_perfCountersTreeStateBucket;
    headerCaptions << GPU_STR_perfCountersTreeDrawCallIndex;


    m_pPerformanceCountersTree = new QTreeWidget;

    pMainLayout->addWidget(m_pSummaryWidget);
    pMainLayout->addWidget(m_pPerformanceCountersTree, 1);
    setLayout(pMainLayout);
}

gpPerformanceCountersDataView::~gpPerformanceCountersDataView()
{
    // Remove me from the list of session windows in the session view creator:
    gpViewsCreator::Instance()->OnWindowClose(this);
}

bool gpPerformanceCountersDataView::DisplaySession(const osFilePath& sessionFilePath, afTreeItemType sessionInnerPage)
{
    GT_UNREFERENCED_PARAMETER(sessionFilePath);
    GT_UNREFERENCED_PARAMETER(sessionInnerPage);

    m_sessionFilePath = sessionFilePath;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pPerformanceCountersTree != nullptr)
    {
        // Read the file into a string
        QFile perfFileForRead(acGTStringToQString(sessionFilePath.asString()));
        QString countersDataStr;
        bool rc = perfFileForRead.open(QIODevice::ReadOnly | QIODevice::Text);
        GT_IF_WITH_ASSERT(rc)
        {
            QTextStream in(&perfFileForRead);
            countersDataStr = in.readAll();

            QDomDocument countersDataDoc;
            countersDataDoc.setContent(countersDataStr);

            QDomNodeList drawCallsList = countersDataDoc.elementsByTagName(GPU_STR_perfCountersXMLDrawCallNode);
            QDomNodeList drawCallsDescsList = countersDataDoc.elementsByTagName(GPU_STR_perfCountersXMLDrawCallNodeSmall);

            int count = drawCallsList.size();
            int count2 = drawCallsDescsList.size();

            // Sanity check:
            GT_IF_WITH_ASSERT(count == count2)
            {
                for (int i = 0; i < count; i++)
                {
                    QDomNode drawCallNode = drawCallsList.at(i);
                    QDomNode drawCallDescriptionNode = drawCallsDescsList.at(i);

                    QDomElement indexElem1 = drawCallNode.firstChildElement(GPU_STR_perfCountersXMLIndexNode);
                    QString drawCallIndex = indexElem1.text();

                    QDomElement indexElem2 = drawCallDescriptionNode.firstChildElement(GPU_STR_perfCountersXMLIndexNodeSmall);
                    GT_ASSERT(indexElem2.text() == drawCallIndex);

                    // Find the name of the function
                    QDomElement callElem2 = drawCallDescriptionNode.firstChildElement(GPU_STR_perfCountersXMLCallNodeSmall);
                    QString callName = callElem2.text();

                    if (i == 0)
                    {
                        QStringList headerCaptions;
                        QString counterValue;

                        headerCaptions << GPU_STR_perfCountersTreeStateBucket;

                        // Go over the counters and add each of them to the tree
                        QDomElement counterElem = drawCallNode.firstChildElement();

                        // Go over the counters and get the column count and captions
                        for (; !counterElem.isNull() ; counterElem = counterElem.nextSiblingElement())
                        {
                            // Skip the index element
                            if (counterElem.tagName() == GPU_STR_perfCountersTreeDrawCallIndex)
                            {
                                headerCaptions << GPU_STR_perfCountersTreeDrawCallIndex;
                            }
                            else
                            {
                                headerCaptions << counterElem.tagName();
                            }
                        }

                        m_pPerformanceCountersTree->setColumnCount(headerCaptions.size());
                        m_pPerformanceCountersTree->setHeaderLabels(headerCaptions);

                        m_pPerformanceCountersTree->setSortingEnabled(true);
                    }

                    QStringList callValuesStrings;
                    callValuesStrings << callName;

                    QDomElement counterElem = drawCallNode.firstChildElement();

                    for (; !counterElem.isNull(); counterElem = counterElem.nextSiblingElement())
                    {
                        callValuesStrings << counterElem.text();
                    }

                    // Add this call to the state bucket tree node
                    AddDrawCallToStateBucket(callValuesStrings);

                    for (int col = 0; col < m_pPerformanceCountersTree->columnCount(); col++)
                    {
                        m_pPerformanceCountersTree->resizeColumnToContents(col);
                    }

                }
            }
        }
    }

    // Call the base class implementation
    bool retVal = true;

    return retVal;
}

void gpPerformanceCountersDataView::AddDrawCallToStateBucket(const QStringList& callValuesStrings)
{
#pragma message ("TODO: FA: add more state buckets")
    QTreeWidgetItem* pStateBucketItem = GetStateBucketItem(0);
    // Sanity check:
    GT_IF_WITH_ASSERT(pStateBucketItem != nullptr)
    {
        QTreeWidgetItem* pNewCallItem = new QTreeWidgetItem(callValuesStrings);
        pStateBucketItem->addChild(pNewCallItem);
        pNewCallItem->setExpanded(true);
    }
}

void gpPerformanceCountersDataView::BuildSummaryData()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSummaryLabel != nullptr)
    {
#pragma message ("TODO: FA: get real data")
        FrameSummaryData randData;
        randData.m_apiCallsCount = qrand();
        randData.m_drawCallsCount = qrand();
        randData.m_cpuTimeMS = qrand() / 20000;
        randData.m_gpuTimeMS = qrand() / 20000;
        randData.m_cpuTimeInDrawPercentage = qrand() % 100;
        randData.m_gpuBusyPercentage = qrand() % 100;

        QString text;
        text.append(QString(GP_Str_CountersDataSummaryAPICalls).arg(randData.m_apiCallsCount));
        text.append(AF_STR_NewLineA);
        text.append(QString(GP_Str_CountersDataSummaryDrawCalls).arg(randData.m_drawCallsCount));
        text.append(AF_STR_NewLineA);
        text.append(QString(GP_Str_CountersDataSummaryCpuTime).arg(randData.m_cpuTimeMS));
        text.append(AF_STR_NewLineA);
        text.append(QString(GP_Str_CountersDataSummaryGpuTime).arg(randData.m_gpuTimeMS));
        text.append(AF_STR_NewLineA);
        text.append(QString(GP_Str_CountersDataSummaryCpuTimePercentageDrawCalls).arg(randData.m_cpuTimeInDrawPercentage));
        text.append(AF_STR_NewLineA);
        text.append(QString(GP_Str_CountersDataSummaryGpuBusy).arg(randData.m_gpuBusyPercentage));
        text.append(AF_STR_NewLineA);

        m_pSummaryLabel->setText(text);

    }

}

void gpPerformanceCountersDataView::OnCountersSelectionClick()
{

    gpSessionTreeNodeData* pCurrentlyRunningSessionData = gpUIManager::Instance()->CurrentlyRunningSessionData();

    // Sanity check:
    GT_IF_WITH_ASSERT(pCurrentlyRunningSessionData != nullptr)
    {
#pragma message ("TODO: FA: implement")
    }
}

void gpPerformanceCountersDataView::OnDisplaySettingsClick()
{

}

QTreeWidgetItem* gpPerformanceCountersDataView::GetStateBucketItem(int stateBucketIndex)
{
    QTreeWidgetItem* pRetVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pPerformanceCountersTree != nullptr)
    {
        QString childName = QString("State Bucket %1").arg(stateBucketIndex);
        QList<QTreeWidgetItem*> matchingItems = m_pPerformanceCountersTree->findItems(childName, Qt::MatchExactly);

        if (!matchingItems.isEmpty())
        {
            GT_IF_WITH_ASSERT(matchingItems.size() == 1)
            {
                pRetVal = matchingItems[0];
            }
        }

        if (pRetVal == nullptr)
        {
            QStringList strings;
            strings << childName;
            strings << AF_STR_EmptyA;
            pRetVal = new QTreeWidgetItem(strings);
            m_pPerformanceCountersTree->addTopLevelItem(pRetVal);
            pRetVal->setExpanded(true);
        }
    }
    return pRetVal;
}
