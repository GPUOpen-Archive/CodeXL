
#include <qtIgnoreCompilerWarnings.h>
#include <QtCore>
#include <QtWidgets>
#include <QDomDocument>
#include <QSize>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTApplicationComponents/Include/acItemDelegate.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acRibbonManager.h>
#include <AMDTApplicationComponents/Include/acTabWidget.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTApplicationComponents/Include/acNavigationChart.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afHTMLContent.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/ProfileApplicationTreeHandler.h>

// Local:
#include <AMDTGpuProfiling/gpExecutionMode.h>
#include <AMDTGpuProfiling/gpNavigationRibbon.h>
#include <AMDTGpuProfiling/gpViewsCreator.h>
#include <AMDTGpuProfiling/gpTreeHandler.h>
#include <AMDTGpuProfiling/gpUIManager.h>
#include <AMDTGpuProfiling/gpObjectView.h>
#include <AMDTGpuProfiling/gpTraceView.h>
#include <AMDTGpuProfiling/ProfileManager.h>
#include <AMDTGpuProfiling/SessionViewTabWidget.h>
#include <AMDTGpuProfiling/gpObjectModels.h>


gpObjectView::gpObjectView(QWidget* parent) : gpBaseSessionView(parent),
    m_pSummaryTableTabWidget(nullptr),
    m_pSessionDataContainer(nullptr),
    m_frameIndex(-1)
{
    // Set the background color to white
    QPalette p = palette();
    p.setColor(backgroundRole(), Qt::gray);
    p.setColor(QPalette::Base, Qt::white);
    setAutoFillBackground(true);
    setPalette(p);

    /// Object Inspector tree view
    m_pObjTreeView = new QTreeView;
    m_pObjDBaseTreeView = new QTreeView;

    m_pLblDeviceInfo = new QLabel;
    m_pLblTypeInfo = new QLabel;
    m_pLblTagDataInfo = new QLabel;

    pMainLayout = new QHBoxLayout;
    pDeviceLayout = new QHBoxLayout;
    pTypeLayout = new QHBoxLayout;
    pObjectItemLayout = new QHBoxLayout;
    pLLayout = new QVBoxLayout;
    pRLayout = new QVBoxLayout;
    pFilteringLayout = new QVBoxLayout;
    pLGroupBox = new QGroupBox;
    pRGroupBox = new QGroupBox;
    pGroupBoxFiltering = new QGroupBox;

    pChkIncludeDestroyed = new QCheckBox;
    pChkGroupDestroyed = new QCheckBox;

    plblDevice = new QLabel;
    plblType = new QLabel;

    pBtnFindUsage = new QPushButton;
}

gpObjectView::~gpObjectView()
{
    // Remove me from the list of session windows in the session view creator:
    gpViewsCreator::Instance()->OnWindowClose(this);
}

bool gpObjectView::DisplaySession(const osFilePath& sessionFilePath, afTreeItemType sessionInnerPage, QString& errorMessage)
{
    afApplicationCommands::instance()->StartPerformancePrintout("Loading Object");

    // Call the base class implementation
    bool retVal = SharedSessionWindow::DisplaySession(sessionFilePath, sessionInnerPage, errorMessage);

    GT_IF_WITH_ASSERT(retVal)
    {
        // Make sure that the file exists, and contain data from server, and parse the trace file
        retVal = PrepareObjectFile();
    }

    afApplicationCommands::instance()->EndPerformancePrintout("Loading Object");

    return retVal;
}

// Load the data, Draw the UI
void gpObjectView::SetProfileObjectDataModel(gpObjectDataModel* pDataModel)
{
    // Call the base class
    gpBaseSessionView::SetProfileObjectDataModel(pDataModel);

    // Build the view layout according to the content of the session
    // Sanity check:
    GT_IF_WITH_ASSERT(pDataModel != nullptr)
    {
        m_pSessionDataContainer = pDataModel->ObjectDataContainer();

        m_pTxtSelectedObject = new QTextEdit;
        m_pTxtSelectedObject->setReadOnly(true);

        pMainLayout->addWidget(pLGroupBox, 0);
        pMainLayout->addWidget(pRGroupBox, 1);

        pLGroupBox->setLayout(pLLayout);
        pRGroupBox->setLayout(pRLayout);
        pGroupBoxFiltering->setLayout(pFilteringLayout);

        pLGroupBox->setTitle("Object Instances");
        pRGroupBox->setTitle("Object Properties");
        pGroupBoxFiltering->setTitle("Filtering");

        // setup left layout
        pChkIncludeDestroyed->setText("Include Destroyed");
        pChkGroupDestroyed->setText("Group Destroyed");

        pChkIncludeDestroyed->setChecked(m_iIncludeDestroyed);
        pChkGroupDestroyed->setChecked(m_iGroupDestroyed);

        pGroupBoxFiltering->layout()->addWidget(pChkIncludeDestroyed);
        pGroupBoxFiltering->layout()->addWidget(pChkGroupDestroyed);
        pLLayout->addWidget(pGroupBoxFiltering);
        pLLayout->addWidget(m_pObjTreeView);

        // setup right layout
        plblDevice->setText("Device:");
        m_pLblDeviceInfo->setText("Device Text HOLDER");
        plblType->setText("Type:");
        m_pLblTypeInfo->setText("Text HOLDER ID3D12Resource");
        m_pTxtSelectedObject->setText("Text HOLDER ID3D12CommittedResource");
        m_pTxtSelectedObject->setMaximumSize(300, 30);
        pBtnFindUsage->setText("Find Usages");
        pBtnFindUsage->setMaximumSize(150, 150);

        pRLayout->addLayout(pDeviceLayout);
        pRLayout->addLayout(pTypeLayout);
        pRLayout->addLayout(pObjectItemLayout);

        pDeviceLayout->addWidget(plblDevice);
        pDeviceLayout->addWidget(m_pLblDeviceInfo);
        pTypeLayout->addWidget(plblType);
        pTypeLayout->addWidget(m_pLblTypeInfo);
        pObjectItemLayout->addWidget(m_pTxtSelectedObject);
        pObjectItemLayout->addWidget(pBtnFindUsage);
        pRLayout->addWidget(m_pObjDBaseTreeView);

        // push controls to the left
        pDeviceLayout->addStretch(1);
        pTypeLayout->addStretch(2);
        pObjectItemLayout->addStretch(3);

        // open object tree file
        QDomDocument treeDOM("treeDOM");
        gtString strTreeFilePath = L"";
        gtString strTreeFileName;

        // Assign object tree file name
        strTreeFilePath.append(m_sessionFilePath.asString().asCharArray());

        QFile treeXmlFile(strTreeFilePath.asASCIICharArray());
        bool rc = treeXmlFile.open(QIODevice::ReadOnly | QIODevice::Text);

        if (!rc || !treeDOM.setContent(&treeXmlFile))
        {
            // Failed to load object tree data
            m_pLblDeviceInfo->setText("FAILED TO LOAD OBJECT TREE");
        }

        treeXmlFile.close();

        if ((nullptr != m_oTreeModel) && (m_oTreeModel->hasChildren()))
        {
            // clear / reload
            m_oTreeModel->clear();
        }
        else if (nullptr == m_oTreeModel)
        {
            m_oTreeModel = new ObjTreeModel(treeDOM, this);
        }

        m_pObjTreeView->setModel(m_oTreeModel);

        // expand select 1st item
        expandSelectTree(m_pObjTreeView);

        // load full object Database
        QDomDocument oDBaseDOM("oDBaseDOM");
        gtString strObjDBFilePath = L"";
        gtString strObjDBFileName;

        // create database file name
        strObjDBFilePath.append(m_sessionFilePath.fileDirectoryAsString().asCharArray());
        strObjDBFilePath.append(GPU_STR_FullObjectDatabase);

        QFile oDBaseXmlFile(strObjDBFilePath.asASCIICharArray());
        rc = oDBaseXmlFile.open(QIODevice::ReadOnly | QIODevice::Text);

        if (rc && !oDBaseDOM.setContent(&oDBaseXmlFile))
        {
            //        cout << "ERROR" << endl;
            oDBaseXmlFile.close();
        }

        oDBaseXmlFile.close();

        // clear model 1st
        if ((nullptr != m_oDBaseModel) && (m_oDBaseModel->hasChildren()))
        {
            m_oDBaseModel->clear();
            delete(m_oDBaseModel);
        }

        m_oDBaseModel = new ObjDatabaseModel(oDBaseDOM, this);

        m_pObjDBaseTreeView->setModel(m_oDBaseModel);
        m_pObjDBaseTreeView->expandAll();
        m_pObjDBaseTreeView->setColumnWidth(0, 250);
        m_pObjDBaseTreeView->show();

        //selection changes connect to ObjectView handler
        QItemSelectionModel* selectionModel = m_pObjTreeView->selectionModel();
        connect(selectionModel, SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(OnTreeObjSelected(const QItemSelection&, const QItemSelection&)));
        connect(this, SIGNAL(viewObjSelected(QString)), m_oDBaseModel, SLOT(dBaseObjSelected(const QString)));
        connect(pChkIncludeDestroyed, SIGNAL(stateChanged(int)), this, SLOT(OnIncludeDestroyedChanged(int)));
        connect(pChkGroupDestroyed, SIGNAL(stateChanged(int)), this, SLOT(OnGroupDestroyedChanged(int)));

        pMainLayout->setContentsMargins(1, 1, 1, 1);
        setLayout(pMainLayout);
    }
}

void gpObjectView::OnSummaryItemClicked(ProfileSessionDataItem* pItem)
{
    GT_UNREFERENCED_PARAMETER(pItem);
}

void gpObjectView::expandSelectTree(QTreeView* treeViewSelect)
{
    QModelIndex idxFirstChild;

    if (nullptr != treeViewSelect)
    {
        treeViewSelect->expandAll();
        treeViewSelect->show();
        treeViewSelect->setCurrentIndex(treeViewSelect->model()->index(0, 0));
    }
}

// slot
void gpObjectView::OnTreeObjSelected(const QItemSelection& newSelection, const QItemSelection& oldSelection)
{
    GT_UNREFERENCED_PARAMETER(newSelection);
    GT_UNREFERENCED_PARAMETER(oldSelection);

    //get the text of the selected item
    const QModelIndex index = m_pObjTreeView->selectionModel()->currentIndex();
    QString selectedText = index.data(Qt::DisplayRole).toString();

    //find out the hierarchy level of the selected item
    int iTreeLevel = 1;
    QModelIndex seekRoot = index;

    while (seekRoot.parent() != QModelIndex())
    {
        seekRoot = seekRoot.parent();
        iTreeLevel++;
    }

    // use hierarchy level to determine whether it's Device, Object Type or Object item
    if (1 == iTreeLevel)
    {
        // Device item clicked, make sure Object properties showing obj from current device, if not, pick the 1st object data.
        m_pLblDeviceInfo->setText(index.data(Qt::DisplayRole).toString());
    }
    else if (2 == iTreeLevel)
    {
        // Object type clicked, make sure Object properties showing obj from current type, if not, pick the 1st object data from this type.
        m_pLblTypeInfo->setText(index.data(Qt::DisplayRole).toString());

        QModelIndex idxChild = index.child(0, 0);

        if (idxChild.isValid())
        {
            emit(viewObjSelected(idxChild.data(Qt::DisplayRole).toString()));
        }
    }
    else if (3 == iTreeLevel)
    {
        // Object type clicked, make sure Object properties showing obj from current type, if not, pick the 1st object data from this type.
        seekRoot = index;
        m_pLblTypeInfo->setText(seekRoot.parent().data(Qt::DisplayRole).toString());
        // set label box text
        m_pLblTagDataInfo->setText(index.data(Qt::DisplayRole).toString());

        emit(viewObjSelected(index.data(Qt::DisplayRole).toString()));
    }

    //    else, no change, should not happen

    m_pObjDBaseTreeView->expandAll();
    m_pObjDBaseTreeView->show();

    return;
}

bool gpObjectView::PrepareObjectFile()
{
    bool retVal = false;

#ifdef GP_OBJECT_VIEW_ENABLE

    if (m_frameIndex < 0)
    {
        // Extract the frame index from the file path
        gtString fileName;
        m_sessionFilePath.getFileName(fileName);
        int pos = fileName.findLastOf(L"_");

        GT_IF_WITH_ASSERT(pos >= 0)
        {
            gtString frameIndexStr;
            fileName.getSubString(pos + 1, fileName.length(), frameIndexStr);
            retVal = frameIndexStr.toIntNumber(m_frameIndex);
            GT_ASSERT(retVal);
        }
    }

    GT_IF_WITH_ASSERT(m_frameIndex > 0)
    {
        // Make sure that the frame objects are written to the object file (.aor)
        gpExecutionMode* pModeManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();
        GT_IF_WITH_ASSERT(pModeManager != nullptr)
        {
            // TODO, update this to the object UI and data files

            // We should look for the frame owner session file path (needed to calculate the frame file paths)
            afApplicationTreeItemData* pSessionData = ProfileApplicationTreeHandler::instance()->FindParentSessionItemData(m_pSessionData->m_pParentData);
            GT_IF_WITH_ASSERT(pSessionData != nullptr)
            {
                osFilePath sessionFilePath = pSessionData->m_filePath;
                gtString projectName = afProjectManager::instance().currentProjectSettings().projectName();
                gtString sessionName = acQStringToGTString(m_pSessionData->m_displayName);

                gtASCIIString objectsAsText;
                // Make sure that the frame objects is written to the object file
                gpExecutionMode* pModeManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();
                GT_IF_WITH_ASSERT(pModeManager != nullptr)
                {
                    osFilePath objectsFilePath = gpTreeHandler::Instance().GetFrameChildFilePath(m_sessionFilePath, m_frameIndex, AF_TREE_ITEM_GP_FRAME_OBJECTINSPCTOR);

                    retVal = pModeManager->GetFrameObject(sessionFilePath, m_frameIndex, objectsFilePath);
                    GT_IF_WITH_ASSERT(retVal)
                    {
                        gpUIManager::Instance()->PrepareObjectData(m_sessionFilePath, this, qobject_cast<GPUSessionTreeItemData*>(m_pSessionData));
                    }
                }
            }
        }
    }
#endif

    return retVal;
}

// slot
void gpObjectView::OnIncludeDestroyedChanged(int iCheckState)
{
    if (nullptr != m_oTreeModel)
    {
        m_oTreeModel->setIncludeDestroyed(iCheckState);
        m_pObjTreeView->expandAll();
        m_pObjTreeView->show();
    }
}

// slot
void gpObjectView::OnGroupDestroyedChanged(int iCheckState)
{
    if (nullptr != m_oTreeModel)
    {
        m_oTreeModel->setGroupDestroyed(iCheckState);
        m_pObjTreeView->expandAll();
        m_pObjTreeView->show();
    }
}

// slot
void gpObjectView::OnObjDisplayed(QString strObjDisplayed)
{
    if (nullptr != m_pTxtSelectedObject)
    {
        m_pTxtSelectedObject->setText(strObjDisplayed);
    }
}
