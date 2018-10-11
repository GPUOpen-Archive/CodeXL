//------------------------------ kaOverviewView.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTApplicationComponents/Include/acListCtrl.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acQHTMLWindow.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Framework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afDocUpdateManager.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>

// Local:
#include <AMDTKernelAnalyzer/src/kaApplicationTreeHandler.h>
#include <AMDTKernelAnalyzer/src/kaDataAnalyzerFunctions.h>
#include <AMDTKernelAnalyzer/src/kaOverviewView.h>
#include <AMDTKernelAnalyzer/src/kaProjectDataManager.h>
#include <AMDTKernelAnalyzer/src/kaTreeDataExtension.h>
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>

#define KA_KERNEL_LINE_HEIGHT 20

// ---------------------------------------------------------------------------
// Name:        kaOverviewView::kaOverviewView
// Description: constructor
// Author:      Gilad Yarnitzky
// Date:        11/8/2013
// ---------------------------------------------------------------------------
kaOverviewView::kaOverviewView(QWidget* pParent, const osFilePath& filePath): QWidget(pParent), afBaseView(&afProgressBarWrapper::instance()), m_doNotCheckVality(false)
{
    // the file path hold the path to the file which hold the real file path:
    osFile overViewFile;
    overViewFile.open(filePath, osChannel::OS_UNICODE_TEXT_CHANNEL, osFile::OS_OPEN_TO_READ);
    gtString dataFileAsString;
    overViewFile.readIntoString(dataFileAsString);
    overViewFile.close();
    m_filePath.setFullPathFromString(dataFileAsString);

    // Register to the monitor the document for this view:
    afDocUpdateManager::instance().RegisterDocument(this, overViewFile, this, false);
    // Create the layout and the controls in it:
    m_pAnalyzeInputLayout = new QVBoxLayout(this);

    m_pTopInformationCaption = new acQHTMLWindow(NULL);
    m_pTopInformationCaption->setFrameStyle(QFrame::NoFrame);
    m_pTopInformationCaption->viewport()->setAutoFillBackground(false);

    m_pAnalyzeInputLayout->addWidget(m_pTopInformationCaption);

    // add Emulation Dimensions only for cl files
    gtString fileExtension;
    m_filePath.getFileExtension(fileExtension);

    if (fileExtension == KA_STR_kernelFileExtension)
    {
        m_pTableInformationGroupBox = new QGroupBox(KA_STR_overviewTableCaption);
        QVBoxLayout* pTableVLayout = new QVBoxLayout;
        QLabel* pInfoLabel = new QLabel(KA_STR_overviewTableInformation, m_pTableInformationGroupBox);
        m_pAnalyzeInputTable = new acListCtrl(m_pTableInformationGroupBox);
        m_pUpdateTableButton = new QPushButton(KA_STR_overviewRefreshButton, m_pTableInformationGroupBox);

        pTableVLayout->addWidget(pInfoLabel);
        pTableVLayout->addWidget(m_pAnalyzeInputTable);
        pTableVLayout->addWidget(m_pUpdateTableButton, 0, Qt::AlignRight);

        m_pTableInformationGroupBox->setLayout(pTableVLayout);
        m_pAnalyzeInputLayout->addWidget(m_pTableInformationGroupBox);

        // Add the table headers:
        QStringList headerNamesVector;
        headerNamesVector << KA_STR_tableKernelNameColumn << KA_STR_tableGlobalXColumn << KA_STR_tableGlobalYColumn << KA_STR_tableGlobalZColumn
                          << KA_STR_tableLocalXColumn << KA_STR_tableLocalYColumn << KA_STR_tableLocalZColumn << KA_STR_tableLoopsColumn;
        m_pAnalyzeInputTable->initHeaders(headerNamesVector, false);

        m_pAnalyzeInputTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);

        // Add the items to the table:
        insertKernelsToTable();

        // put the data on the overview node
        // Make connection:
        bool rc = connect(m_pAnalyzeInputTable, SIGNAL(cellChanged(int, int)), this, SLOT(onCellChanged(int, int)));
        GT_ASSERT(rc);

        // make the connection to the update button:
        rc = connect(m_pUpdateTableButton, SIGNAL(clicked()), this, SLOT(updateTable()));
        GT_ASSERT(rc);
    }

    // Add the general information of the overview:
    kaApplicationTreeHandler* pTreeHandler = kaApplicationTreeHandler::instance();

    if (NULL != pTreeHandler)
    {
        afHTMLContent htmlContent;
        GT_IF_WITH_ASSERT(pTreeHandler->getOverviewHtmlInfo(m_filePath, htmlContent))
        {
            gtString htmlStr;
            htmlContent.toString(htmlStr);
            m_pTopInformationCaption->setHtml(acGTStringToQString(htmlStr));
        }
    }

    setLayout(m_pAnalyzeInputLayout);
}


// ---------------------------------------------------------------------------
// Name:        kaOverviewView::~kaOverviewView
// Description: destructor
// Author:      Gilad Yarnitzky
// Date:        11/8/2013
// ---------------------------------------------------------------------------
kaOverviewView::~kaOverviewView()
{
    afDocUpdateManager::instance().UnregisterDocumentOfWidget(this);
}


// ---------------------------------------------------------------------------
// Name:        kaOverviewView::insertKernelsToTable
// Description: insert the kernels from the project data manager
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        12/8/2013
// ---------------------------------------------------------------------------
void kaOverviewView::insertKernelsToTable()
{
    kaSourceFile* pFileInfo = KA_PROJECT_DATA_MGR_INSTANCE.dataFileByPath(m_filePath);

    GT_IF_WITH_ASSERT(NULL != m_pAnalyzeInputTable)
    {
        // The pFileInfo can be NULL when VS is reloading
        if (pFileInfo)
        {
            int numKernels = pFileInfo->analyzeVector().size();

            for (int nKernel = 0; nKernel < numKernels; nKernel++)
            {
                kaProjectDataManagerAnaylzeData& currentKernelData = pFileInfo->analyzeVector()[nKernel];

                // Build kernel string list:
                QStringList kernelStringList;

                kernelStringList << currentKernelData.m_kernelName;

                for (int i = 0; i < 3; i++)
                {
                    kernelStringList << QString::number(currentKernelData.m_globalWorkSize[i]);
                }

                for (int i = 0; i < 3; i++)
                {
                    kernelStringList << QString::number(currentKernelData.m_localWorkSize[i]);
                }

                kernelStringList << QString::number(currentKernelData.m_loopIterations);
                m_doNotCheckVality = true;
                m_pAnalyzeInputTable->addRow(kernelStringList, NULL);

                // make sure all items are editable since it is not by default:
                m_pAnalyzeInputTable->enableRowEditing(nKernel, true);

                // Disable editing of the kernel name:
                QTableWidgetItem* pCurrentItem = m_pAnalyzeInputTable->item(nKernel, 0);
                Qt::ItemFlags itemFlags = pCurrentItem->flags();
                itemFlags &= !(Qt::ItemIsEditable != 0);
                pCurrentItem->setFlags(itemFlags);
                m_doNotCheckVality = false;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        kaAnalysisSettingsPage::onCellChanged
// Description: Handle cell changed event to validate user data
// Author:      Gilad Yarnitzky
// Date:        30/7/2013
// ---------------------------------------------------------------------------
void kaOverviewView::onCellChanged(int row, int column)
{
    // check that the data is valid textual wise: no letters, value is larger then 0
    static bool inCellChange = false;

    if (!inCellChange && !m_doNotCheckVality)
    {
        kaSourceFile* pfileInfo = KA_PROJECT_DATA_MGR_INSTANCE.dataFileByPath(m_filePath);

        QString itemText;
        bool rc = m_pAnalyzeInputTable->getItemText(row, column, itemText);
        GT_IF_WITH_ASSERT(rc)
        {
            bool validValue = true;
            QString originalString;

            switch (column)
            {
                case 1: originalString.setNum(pfileInfo->analyzeVector()[row].m_globalWorkSize[0]); break;

                case 2: originalString.setNum(pfileInfo->analyzeVector()[row].m_globalWorkSize[1]); break;

                case 3: originalString.setNum(pfileInfo->analyzeVector()[row].m_globalWorkSize[2]); break;

                case 4: originalString.setNum(pfileInfo->analyzeVector()[row].m_localWorkSize[0]); break;

                case 5: originalString.setNum(pfileInfo->analyzeVector()[row].m_localWorkSize[1]); break;

                case 6: originalString.setNum(pfileInfo->analyzeVector()[row].m_localWorkSize[2]); break;

                case 7: originalString.setNum(pfileInfo->analyzeVector()[row].m_loopIterations); break;

                default:
                    GT_ASSERT(false);
                    break;
            }

            int aValue = itemText.toInt();

            // if turning the value back to string did not get the same result it is an invalid string
            // or the value was the same but the entire table is not valid
            if ((QString::number(aValue) != itemText) || (0 > aValue))
            {
                validValue = false;
            }
            else
            {
                // check that the logic is valid and if not notify the user before changing:
                if (!kaValidKernelTableRow(m_pAnalyzeInputTable, true))
                {
                    acMessageBox::instance().warning(AF_STR_WarningA, KA_STR_analsisSettingInvalidDataMsg);
                    validValue = false;
                }
                else
                {
                    switch (column)
                    {
                        case 1: pfileInfo->analyzeVector()[row].m_globalWorkSize[0] = aValue; break;

                        case 2: pfileInfo->analyzeVector()[row].m_globalWorkSize[1] = aValue; break;

                        case 3: pfileInfo->analyzeVector()[row].m_globalWorkSize[2] = aValue; break;

                        case 4: pfileInfo->analyzeVector()[row].m_localWorkSize[0] = aValue; break;

                        case 5: pfileInfo->analyzeVector()[row].m_localWorkSize[1] = aValue; break;

                        case 6: pfileInfo->analyzeVector()[row].m_localWorkSize[2] = aValue; break;

                        case 7: pfileInfo->analyzeVector()[row].m_loopIterations = aValue; break;

                        default:
                            GT_ASSERT(false);
                            break;
                    }
                }
            }

            // if not valid cell change or table, restore original value
            if (!validValue)
            {
                inCellChange = true;
                m_pAnalyzeInputTable->setItemText(row, column, originalString);
                inCellChange = false;
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        kaOverviewView::clearTable
// Description: clear table instead of clearContents that does not work well here
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        12/8/2013
// ---------------------------------------------------------------------------
void kaOverviewView::clearTable()
{
    GT_IF_WITH_ASSERT(NULL != m_pAnalyzeInputTable)
    {
        // clear last table and rebuild it from scratch:
        int numRows = m_pAnalyzeInputTable->rowCount();

        for (int nRow = 0; nRow < numRows; nRow++)
        {
            m_pAnalyzeInputTable->removeRow(0);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        kaOverviewView::textChanged
// Description:
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        12/8/2013
// ---------------------------------------------------------------------------
void kaOverviewView::updateTable()
{
    // clear the table and rebuild it:
    clearTable();

    // insert all items again:
    insertKernelsToTable();
}


// ---------------------------------------------------------------------------
// Name:        kaOverviewView::onOptionClicked
// Description: option url link pressed
// Arguments:   const QUrl urlLink
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        12/8/2013
// ---------------------------------------------------------------------------
void kaOverviewView::onOptionClicked(const QUrl urlLink)
{
    (void)urlLink;
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(NULL != pApplicationCommands)
    {
        pApplicationCommands->onToolsOptions(KA_STR_analyzeSettingsPageTitle);
    }
}

// ---------------------------------------------------------------------------
void kaOverviewView::updateView()
{
    // build the kernel list again:
    KA_PROJECT_DATA_MGR_INSTANCE.BuildFunctionsList(m_filePath, NULL);

    kaApplicationTreeHandler* pTreeHandler = kaApplicationTreeHandler::instance();

    // generate the html info again since the source file might be different
    GT_IF_WITH_ASSERT(NULL != pTreeHandler)
    {
        afHTMLContent htmlContent;
        GT_IF_WITH_ASSERT(pTreeHandler->getOverviewHtmlInfo(m_filePath, htmlContent))
        {
            gtString htmlStr;
            htmlContent.toString(htmlStr);
            m_pTopInformationCaption->setHtml(acGTStringToQString(htmlStr));
        }
    }

    gtString fileExtension;
    m_filePath.getFileExtension(fileExtension);

    if (fileExtension == KA_STR_kernelFileExtension)
    {
        updateTable();
    }
}

// ---------------------------------------------------------------------------
void kaOverviewView::onUpdateEdit_Copy(bool& isEnabled)
{
    isEnabled = false;

    if (m_pAnalyzeInputTable != NULL)
    {
        m_pAnalyzeInputTable->onUpdateEditCopy(isEnabled);
    }
}

// ---------------------------------------------------------------------------
void kaOverviewView::onUpdateEdit_SelectAll(bool& isEnabled)
{
    if (m_pAnalyzeInputTable != NULL)
    {
        m_pAnalyzeInputTable->onUpdateEditSelectAll(isEnabled);
    }
}

// ---------------------------------------------------------------------------
void kaOverviewView::onEdit_Copy()
{
    if (m_pAnalyzeInputTable != NULL)
    {
        m_pAnalyzeInputTable->onEditCopy();
    }
}

// ---------------------------------------------------------------------------
void kaOverviewView::onEdit_SelectAll()
{
    if (m_pAnalyzeInputTable != NULL)
    {
        m_pAnalyzeInputTable->onEditSelectAll();
    }
}

// ---------------------------------------------------------------------------
void kaOverviewView::UpdateDocument(const osFilePath& docToUpdate)
{
    GT_UNREFERENCED_PARAMETER(docToUpdate);

    updateView();
}
