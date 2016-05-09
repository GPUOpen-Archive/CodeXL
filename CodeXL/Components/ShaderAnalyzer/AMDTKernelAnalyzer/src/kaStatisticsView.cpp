//------------------------------ kaStatisticsView.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTApplicationComponents/Include/acSourceCodeDefinitions.h>
#include <AMDTApplicationComponents/Include/acListCtrl.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Framework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// Local:
#include <AMDTKernelAnalyzer/src/kaApplicationCommands.h>
#include <AMDTKernelAnalyzer/src/kaBackendManager.h>
#include <AMDTKernelAnalyzer/src/kaKernelView.h>
#include <AMDTKernelAnalyzer/src/kaProjectDataManager.h>
#include <AMDTKernelAnalyzer/src/kaStatisticsView.h>
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>
#include <AMDTKernelAnalyzer/src/kaUtils.h>
#include <AMDTKernelAnalyzer/src/kaDataAnalyzerFunctions.h>

// common
#include "DeviceInfoUtils.h"

// Backend.
#include <AMDTBackEnd/Include/beDriverUtils.h>

// This is the used SGPRs array for CI, SI devices.
const unsigned gSGPRs_CI_SI[7] = { 48, 56, 64, 72, 80, 96, 96 };
const unsigned gSGPRs_CI_SI_MAX_WAVES_PER_SIMD[] = { 10, 9, 8, 7, 6, 5, 4 };
const unsigned gSGPRs_CI_SI_NUM_OF_VALUES = 7;

// This is the used SGPRs array for VI where the the driver is D3D and the
// device is Iceland or Tonga.
const unsigned gSGPRs_VI_ICELAND_TONGA_DX[4] = { 64, 80, 96, 96 };
const unsigned gSGPRs_VI_ICELAND_TONGA_DX_MAX_WAVES_PER_SIMD[] = { 10, 8, 6, 5 };
const unsigned gSGPRs_VI_ICELAND_TONGA_DX_NUM_OF_VALUES = 4;

// This is the used SGPRs array where the device is of VI/PI/AI, and the
// driver is OpenCL or OpenGL, OR the device is of VI but is neither Iceland nor Tonga
// and the driver is D3D.
const unsigned gSGPRs_VI_PI_AI_DEFAULT[3] = { 80, 96, 96 };
const unsigned gSGPRs_VI_PI_AI_DEFAULT_ICELAND_OR_TONGA[3] = { 80, 96, 96 };
const unsigned gSGPRs_VI_PI_AI_DEFAULT_MAX_WAVES_PER_SIMD[] = { 10, 8, 7 };
const unsigned gSGPRs_VI_PI_AI_DEFAULT_NUM_OF_VALUES = 3;

const int gVGPRs[10] = { 24, 28, 32, 36, 40, 48, 64, 84, 128, 128 };
const QColor gColorTable[10] = { { 156, 255, 148 }, { 177, 255, 150 }, { 187, 255, 149 }, { 202, 255, 148 },
    { 234, 255, 128 }, { 255, 255, 127 }, { 255, 237, 128 }, { 255, 192, 128 },
    { 255, 149, 128 }, { 255, 128, 128 }
};


// Defines the width and height of the lower table (which determines the width of the upper tables):
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define KA_SCROLL_WIDGET_WIDTH 1000
    #define KA_SCROLL_WIDGET_HEIGHT 600
    #define KA_SCROLL_WIDGET_SPACE 50
#else
    #define KA_SCROLL_WIDGET_WIDTH 1250
    #define KA_SCROLL_WIDGET_HEIGHT 700
    #define KA_SCROLL_WIDGET_SPACE 300
#endif

#define KA_RESOURCE_COLUMN_WIDTH 200
#define KA_USAGE_COLUMN_WIDTH 450
#define KA_CONSTRAINT_COLUMN_WIDTH KA_SCROLL_WIDGET_WIDTH - KA_SCROLL_WIDGET_SPACE - KA_RESOURCE_COLUMN_WIDTH - KA_USAGE_COLUMN_WIDTH
#define KA_RECOMMANDED_COLUMN_WIDTH 150

#define KA_SCROLL_WIDGET_NO_LDS_HEIGHT 500

#define KA_V6_FAMILY_NAME " v6"
#define KA_V8_FAMILY_NAME " v8"
#define KA_DEVICE_NAME_ICELAND  "Iceland"
#define KA_DEVICE_NAME_TONGA    "Tonga"

const int KA_MAX_LDS_V6 = 32768;
const int KA_MAX_LDS_V7 = 16384;
// ---------------------------------------------------------------------------
// Name:        kaStatisticsView::kaStatisticsView
// Description: Constructor
// Author:      Gilad Yarnitzky
// Date:        18/8/2013
// ---------------------------------------------------------------------------
kaStatisticsView::kaStatisticsView(QWidget* pParent, const osFilePath& kernelFilePath, const osFilePath& detailedFilePath, kaPlatform platform, const gtString& buildProfile) : kaTableView(pParent, detailedFilePath, 2),
    m_pKernelInfo(nullptr), m_pInfoGroupBoxTab1(nullptr), m_pSCSourceShaderTab2(nullptr), m_pCSDataTab3(nullptr), m_pInfoTabWidget(nullptr),
    m_pSGRPsInfoTableLabels(nullptr), m_pSGRPsInfoTable(nullptr), m_pVGRPsInfoTableLabels(nullptr), m_pVGRPsInfoTable(nullptr), m_pLDSInfoTableLabels(nullptr),
    m_pLDSInfoTable(nullptr), m_pLDSInfoLine(nullptr), m_pMaxWavesTable(nullptr), m_pAdviceBox(nullptr), m_pRecommendLabel(nullptr), m_pDynamicLDS(nullptr),
    m_LDSUsed(0), m_pScrollArea(nullptr), m_pScrollAreaWidget(nullptr), m_tablesResized(false), m_wasDynamicChangedByUser(false),
    m_kernelFilePath(kernelFilePath), m_platform(platform), m_buildProfile(buildProfile), m_shouldCheckWorkGroupValues(true)
{
    GetKernelDataInfo(pParent);


    if (m_pKernelInfo != nullptr && platform == kaPlatformDirectX && (buildProfile.find(KA_STR_ComputeShaderProfilePrefix) != -1))
    {
        // Only for detected DX Compute shader get local workgroup dimensions for given entry point
        gtString fileName;
        detailedFilePath.getFileName(fileName);
        int startPosition = fileName.find(L"_") + 1;
        int endPosition = fileName.reverseFind(L"_cs_") - 1;

        if (startPosition < fileName.length() &&
            endPosition < fileName.length() &&
            startPosition < endPosition)
        {
            gtString entryPoint;
            fileName.getSubString(startPosition, endPosition, entryPoint);
            gtVector<int> localWorkgroupDimensions;
            localWorkgroupDimensions.resize(3);
            UpdateLocalWorkgroupDimensions(KA_PROJECT_DATA_MGR_INSTANCE.dataFileByPath(kernelFilePath), entryPoint, buildProfile);
        }
    }

    initializeMainTable(KA_STR_statisticsTableInfo, KA_STR_statisticsTableRows, KA_STR_statisticsTableRowsTooltip);

    m_wasDynamicChangedByUser = false;

    m_tablesResized = false;

    // Create scroll area info
    m_pScrollArea = new QScrollArea(this);

    m_pScrollAreaWidget = new QWidget;

    int scrollW = (int)acScalePixelSizeToDisplayDPI(KA_SCROLL_WIDGET_WIDTH);
    int scrollH = (int)acScalePixelSizeToDisplayDPI(KA_SCROLL_WIDGET_HEIGHT);
    m_pScrollAreaWidget->resize(scrollW, scrollH);

    // needed for the colors selection in base class, so it created but will be hidden
    m_pTableInformationCaption = new QLabel(KA_STR_statisticsTableInfo);
    m_pTableInformationCaption->setVisible(false);

    // Create the combobox area:
    m_kernelName = kaUtils::GetKernelNameFromPath(detailedFilePath);
    QHBoxLayout* pHBoxLayout = new QHBoxLayout;
    QString comboLabel = QString(KA_STR_statisticsTableComboText).arg(m_kernelName);
    m_pComboDescription = new QLabel(comboLabel);
    m_pDeviceComboBox = new QComboBox(this);
    m_pDeviceComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    bool rc = connect(m_pDeviceComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnDeviceComboBoxChanged(int)));
    GT_ASSERT(rc);

    pHBoxLayout->addWidget(m_pComboDescription);
    pHBoxLayout->addWidget(m_pDeviceComboBox);
    pHBoxLayout->addStretch(1);

    m_pMainLayout->addLayout(pHBoxLayout);

    // Create the stacked widget area:
    m_pStackedLayout = new QStackedLayout;
    m_pNILayerTableWidget = new QWidget;
    m_pSILayerTableWidget = new QWidget;
    m_pStackedLayout->addWidget(m_pNILayerTableWidget);
    m_pStackedLayout->addWidget(m_pSILayerTableWidget);

    // Create the NI layer:
    QVBoxLayout* pNIVBoxLayout = new QVBoxLayout;
    pNIVBoxLayout->addWidget(m_pAnalysisTable);
    pNIVBoxLayout->addStretch(1);

    m_pNILayerTableWidget->setLayout(pNIVBoxLayout);

    // create the SI layer:
    QVBoxLayout* pSIVBoxLayout = new QVBoxLayout;

    // Create LDS information:
    CreateUpperPanel(pSIVBoxLayout);

    // Create the upper table:
    CreateUpperTable(pSIVBoxLayout);

    // Create the lower table:
    CreateLowerTable();
    // Add layout to contain the table so we can have margins
    QVBoxLayout* pVLayout = new QVBoxLayout;
    pVLayout->addWidget(m_pLowerTable);
    pVLayout->setContentsMargins(0, 15, 0, 15);
    pSIVBoxLayout->addLayout(pVLayout);

    // Create the tab widget for the information tables:
    m_pInfoTabWidget = new QTabWidget;

    pSIVBoxLayout->addWidget(m_pInfoTabWidget, 1, 0);

    CreateInformationTables();
    CreateISATabs();

    m_pSILayerTableWidget->setLayout(pSIVBoxLayout);

    m_pMainLayout->addLayout(m_pStackedLayout);

    m_pScrollAreaWidget->setLayout(m_pMainLayout);

    m_pScrollArea->setWidget(m_pScrollAreaWidget);

    QColor whiteColor(255, 255, 255);
    QPalette newPalette = m_pScrollArea->palette();
    newPalette.setColor(QPalette::Window, Qt::white);
    m_pScrollArea->setPalette(newPalette);

    readDataFile();

    updateBGColors();

    resizeToContent(m_pAnalysisTable);

    if (nullptr != m_pAnalysisTable)
    {
        m_pAnalysisTable->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    }

    if (m_pDeviceComboBox->count() == 0)
    {
        m_pAnalysisTable->setVisible(false);
        m_pAnalysisTable->setEnabled(false);
        m_pComboDescription->setText(KA_STR_statisticsNoStatisticsToShow);
        m_pDeviceComboBox->setVisible(false);
        m_pDeviceComboBox->setEnabled(false);
    }
    else
    {
        // Select last item in the combobox:
        m_pAnalysisTable->setVisible(true);
        m_pAnalysisTable->setEnabled(true);
        m_pDeviceComboBox->setVisible(true);
        m_pDeviceComboBox->setEnabled(true);
        m_pDeviceComboBox->setCurrentIndex(m_pDeviceComboBox->count() - 1);
        comboLabel = QString(KA_STR_statisticsTableComboText).arg(m_kernelName);
        m_pComboDescription->setText(comboLabel);
    }

}

// ---------------------------------------------------------------------------
// Name:        kaStatisticsView::~kaStatisticsView
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        18/8/2013
// ---------------------------------------------------------------------------
kaStatisticsView::~kaStatisticsView()
{
}

// ---------------------------------------------------------------------------
void kaStatisticsView::resizeEvent(QResizeEvent* event)
{
    GT_UNREFERENCED_PARAMETER(event);

    int scrollW = (int)acScalePixelSizeToDisplayDPI(KA_SCROLL_WIDGET_WIDTH);

    if (!m_tablesResized && m_pScrollAreaWidget->width() == scrollW)
    {
        // get the total width of columns
        int totalColumns = 0;

        for (int col = 0; col < 3; col++)
        {
            totalColumns += m_pUpperTable->columnWidth(col);
        }

        // Resize three tables columns width to the new layout width.
        // The base sum of the columns width is defined to be 1000 so no need calculate that
        // Update two upper tables:
        for (int col = 0; col < 3; col++)
        {
            int columnWidth = m_pUpperTable->columnWidth(col) * (m_pScrollAreaWidget->width() - KA_SCROLL_WIDGET_SPACE) / totalColumns;

            if (columnWidth == m_pUpperTable->columnWidth(col))
            {
                m_tablesResized = true;
            }

            m_pUpperTable->setColumnWidth(col, columnWidth);
            m_pMaxWavesTable->setColumnWidth(col, columnWidth);
        }

        // Set the maximum table width so the last column custom draw will reach the end of the table
        // and not just the end of the column
        m_pUpperTable->setMaximumWidth(totalColumns + 2);
        m_pMaxWavesTable->setMaximumWidth(totalColumns + 2);

        // Do the same action to the lower table:
        totalColumns = 0;

        for (int col = 0; col < 4; col++)
        {
            totalColumns += m_pLowerTable->columnWidth(col);
        }

        // Update lower table:
        for (int col = 0; col < 4; col++)
        {
            int columnWidth = m_pLowerTable->columnWidth(col) * (m_pScrollAreaWidget->width() - KA_SCROLL_WIDGET_SPACE) / totalColumns;
            m_pLowerTable->setColumnWidth(col, columnWidth);
        }

        m_pLowerTable->setMaximumWidth(totalColumns + 2);
    }

    m_pScrollArea->resize(size());
}

// ---------------------------------------------------------------------------
void kaStatisticsView::GetKernelDataInfo(QWidget* pParent)
{
    osFilePath sourcePath;

    // Store the source path for the local workgroup information:
    kaKernelView* pKernelView = qobject_cast<kaKernelView*>(pParent);
    GT_IF_WITH_ASSERT(pKernelView)
    {
        sourcePath = pKernelView->sourceFile();
    }

    // Derive the entry point's name from the file path.
    osDirectory fileDirectory;
    m_filePath.getFileDirectory(fileDirectory);
    kaSourceFile* pfileInfo = KA_PROJECT_DATA_MGR_INSTANCE.dataFileByPath(sourcePath);
    GT_IF_WITH_ASSERT(nullptr != pfileInfo)
    {
        QString kernelNameStr = kaUtils::GetKernelNameFromPath(m_filePath);
        QString LDSWorkGroupStr;
        int numKernels = pfileInfo->analyzeVector().size();

        for (int nKernel = 0; nKernel < numKernels; nKernel++)
        {
            kaProjectDataManagerAnaylzeData currentData = pfileInfo->analyzeVector()[nKernel];

            if (currentData.m_kernelName == kernelNameStr)
            {
                m_pKernelInfo = &pfileInfo->analyzeVector()[nKernel];
            }
        }
    }
}
// ---------------------------------------------------------------------------
void kaStatisticsView::CreateUpperPanel(QVBoxLayout* pVBoxLayout)
{
    bool isShader = IsShaderFile();

    QString workgroupString[] = { KA_STR_statisticsTableLocalWorkGroup, KA_STR_statisticsTableLocalWorkGroupX, KA_STR_statisticsTableLocalWorkGroupY, KA_STR_statisticsTableLocalWorkGroupZ };

    QHBoxLayout* pHBoxLayout = new QHBoxLayout;

    // Add the work group items
    QLabel* pLabel;

    for (int nItem = 0; nItem < 4; nItem++)
    {
        pLabel = new QLabel(workgroupString[nItem]);

        if (isShader)
        {
            pLabel->setEnabled(false);
        }

        pHBoxLayout->addWidget(pLabel);

        if (0 != nItem)
        {
            m_pWorkgroup[nItem - 1] = new QLineEdit;
            QRect rect = m_pWorkgroup[nItem - 1]->fontMetrics().boundingRect("8888");
            m_pWorkgroup[nItem - 1]->setFixedSize(rect.width() + 4, rect.height() + 4);
            m_pWorkgroup[nItem - 1]->setValidator(new QIntValidator);

            pHBoxLayout->addWidget(m_pWorkgroup[nItem - 1]);

            if (isShader)
            {
                m_pWorkgroup[nItem - 1]->setEnabled(false);
            }
            else
            {
                bool rc = connect(m_pWorkgroup[nItem - 1], SIGNAL(editingFinished()), this, SLOT(OnWorkGroupEdit()));
                GT_ASSERT(rc);

                rc = connect(m_pWorkgroup[nItem - 1], SIGNAL(textEdited(const QString&)), this, SLOT(OnWorkGroupTextEdit(const QString&)));
                GT_ASSERT(rc);
            }
        }
    }

    pHBoxLayout->addStretch(1);

    // add the dynamic LDS items:
    if (IsLdsEnabled())
    {
        pLabel = new QLabel(KA_STR_statisticsTableDynamicLDS);
        pHBoxLayout->addWidget(pLabel);
        m_pDynamicLDS = new QLineEdit;
        m_pDynamicLDS->setText("0");
        QRect rect = m_pDynamicLDS->fontMetrics().boundingRect("888888");
        m_pDynamicLDS->setFixedSize(rect.width() + 4, rect.height() + 4);
        pHBoxLayout->addWidget(m_pDynamicLDS);
        pLabel = new QLabel(KA_STR_statisticsTableDynamicBytes);
        pHBoxLayout->addWidget(pLabel);
        pHBoxLayout->addStretch(3);

        bool rc = connect(m_pDynamicLDS, SIGNAL(editingFinished()), this, SLOT(OnDynamicLDSEdit()));
        GT_ASSERT(rc);
    }
    else
    {
        int scrollW = (int)acScalePixelSizeToDisplayDPI(KA_SCROLL_WIDGET_WIDTH);
        int scrollH = (int)acScalePixelSizeToDisplayDPI(KA_SCROLL_WIDGET_HEIGHT);
        m_pScrollAreaWidget->resize(scrollW, scrollH);
    }

    pVBoxLayout->addLayout(pHBoxLayout);
}

// ---------------------------------------------------------------------------
bool kaStatisticsView::IsShaderFile() const
{
    bool retval = false;

    if (m_platform == kaPlatformDirectX || m_platform == kaPlatformOpenGL || m_platform == kaPlatformVulkan)
    {
        retval = true;
    }

    return retval;
}

// ---------------------------------------------------------------------------
bool kaStatisticsView::IsComputeShader() const
{
    bool retval = false;

    if (m_platform == kaPlatformDirectX && m_buildProfile.startsWith(KA_STR_ComputeShaderProfilePrefix))
    {
        retval = true;
    }

    return retval;
}

bool kaStatisticsView::IsLdsEnabled() const
{
    bool retval = false;
    bool isShader = IsShaderFile();

    if (!isShader || IsComputeShader())
    {
        retval = true;
    }

    return retval;
}

// ---------------------------------------------------------------------------
void kaStatisticsView::CreateUpperTable(QVBoxLayout* pVBoxLayout)
{
    m_pUpperTable = new acListCtrl(nullptr, KA_ANALYSIS_TABLE_HEIGHT);
    m_pUpperTable->verticalHeader()->hide();
    m_pUpperTable->horizontalHeader()->hide();

    m_pUpperTable->setColumnCount(3);

    int rowNum = 0;

    m_pUpperTable->insertRow(rowNum);
    addItem(m_pUpperTable, rowNum, 0, KA_STR_statisticsTableResource);
    addItem(m_pUpperTable, rowNum, 1, KA_STR_statisticsTableUsage);
    addItem(m_pUpperTable, rowNum, 2, KA_STR_statisticsTableConstraint);

    m_pUpperTable->insertRow(++rowNum);
    addItem(m_pUpperTable, rowNum, 0, KA_STR_statisticsTableSGPRsRange);

    m_pUpperTable->insertRow(++rowNum);
    addItem(m_pUpperTable, rowNum, 0, KA_STR_statisticsTableVGPRsRange);

    if (IsLdsEnabled())
    {
        m_pUpperTable->insertRow(++rowNum);
        addItem(m_pUpperTable, rowNum, 0, KA_STR_statisticsTableLDSRange);

        m_pUpperTable->insertRow(++rowNum);
        QString finalLDSInfo = QString(KA_STR_statisticsTableLDSDynamicaAvailable).arg(m_pDynamicLDS->text());
        addItem(m_pUpperTable, rowNum, 0, finalLDSInfo);
        QTableWidgetItem* pItem = m_pUpperTable->item(rowNum, 0);

        if (nullptr != pItem)
        {
            QPixmap lightBulb;
            acSetIconInPixmap(lightBulb, AC_ICON_ANALYZE_STATISTICS_LIGHTBULB);
            pItem->setIcon(QIcon(lightBulb));
        }
    }

    for (int row = 0; row <= rowNum; row++)
    {
        for (int column = 0; column < 3; column++)
        {
            QTableWidgetItem* pItem = m_pUpperTable->item(row, column);

            if (nullptr == pItem)
            {
                addItem(m_pUpperTable, row, column, "");
                pItem = m_pUpperTable->item(row, column);
            }

            GT_IF_WITH_ASSERT(nullptr != pItem)
            {
                // Set the background color:
                if (0 == row)
                {
                    pItem->setData(Qt::BackgroundColorRole, m_a2Color);
                }

                if (column > 0)
                {
                    pItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
                }
            }
        }
    }

    m_pUpperTable->setColumnWidth(0, KA_RESOURCE_COLUMN_WIDTH);
    m_pUpperTable->setColumnWidth(1, KA_USAGE_COLUMN_WIDTH);
    m_pUpperTable->setColumnWidth(2, KA_CONSTRAINT_COLUMN_WIDTH);

    if (IsLdsEnabled())
    {
        // Now that we have all the items it is safe to set the span of the advice
        m_pUpperTable->setSpan(rowNum, 0, 1, 2);
    }

    // Create the max wave table:
    m_pMaxWavesTable = new acListCtrl(nullptr, KA_ANALYSIS_TABLE_HEIGHT);
    m_pMaxWavesTable->verticalHeader()->hide();
    m_pMaxWavesTable->horizontalHeader()->hide();
    m_pMaxWavesTable->setColumnCount(3);
    m_pMaxWavesTable->insertRow(0);
    addItem(m_pMaxWavesTable, 0, 0, "");
    addItem(m_pMaxWavesTable, 0, 1, KA_STR_statisticsTableMaxWave);
    addItem(m_pMaxWavesTable, 0, 2, "10");

    for (int nColumn = 0 ; nColumn < 3 ; nColumn++)
    {
        QTableWidgetItem* pItem = m_pMaxWavesTable->item(0, nColumn);

        if (nullptr != pItem)
        {
            QFont itemFont = pItem->font();
            itemFont.setBold(true);

            if (1 == nColumn)
            {
                pItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignRight);
                itemFont.setUnderline(true);
            }

            pItem->setFont(itemFont);
        }
    }

    m_pMaxWavesTable->setColumnWidth(0, KA_RESOURCE_COLUMN_WIDTH);
    m_pMaxWavesTable->setColumnWidth(1, KA_USAGE_COLUMN_WIDTH);
    m_pMaxWavesTable->setColumnWidth(2, KA_CONSTRAINT_COLUMN_WIDTH);
    m_pMaxWavesTable->setFrameShape(QFrame::NoFrame);

    // Add recommendation to the user:
    QPixmap lightBulb;
    acSetIconInPixmap(lightBulb, AC_ICON_ANALYZE_STATISTICS_LIGHTBULB);
    QLabel* pLightBulb = new QLabel;
    pLightBulb->setPixmap(lightBulb);
    m_pRecommendLabel = new QLabel(KA_STR_statisticsTableAdvice);
    m_pAdviceBox = new QHBoxLayout;
    m_pAdviceBox->addWidget(pLightBulb);
    m_pAdviceBox->addWidget(m_pRecommendLabel);
    m_pAdviceBox->addStretch(1);


    m_pUpperTable->setFixedHeight(m_pUpperTable->verticalHeader()->length() + 2);
    m_pUpperTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pUpperTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pMaxWavesTable->setFixedHeight(m_pMaxWavesTable->verticalHeader()->length() + 2);
    m_pMaxWavesTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pMaxWavesTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Set the drawing delegate:
    kaStatisticsViewSIMDDelegate* pDelegateUpper = new kaStatisticsViewSIMDDelegate(m_pUpperTable, 2, true);
    m_pUpperTable->setItemDelegate(pDelegateUpper);
    kaStatisticsViewSIMDDelegate* pDelegateMax = new kaStatisticsViewSIMDDelegate(m_pMaxWavesTable, 2, false);
    m_pMaxWavesTable->setItemDelegate(pDelegateMax);

    DisableTableSelection(m_pMaxWavesTable, 0, 1);
    DisableTableSelection(m_pUpperTable, 0, 1);

    if (IsLdsEnabled())
    {
        DisableTableSelection(m_pUpperTable, rowNum, 1);
    }

    pVBoxLayout->addWidget(m_pUpperTable);
    pVBoxLayout->addWidget(m_pMaxWavesTable);
    pVBoxLayout->addLayout(m_pAdviceBox);
}

// ---------------------------------------------------------------------------
void kaStatisticsView::CreateLowerTable()
{
    m_pLowerTable = new acListCtrl(nullptr, KA_ANALYSIS_TABLE_HEIGHT);
    m_pLowerTable->verticalHeader()->hide();
    m_pLowerTable->horizontalHeader()->hide();

    m_pLowerTable->setColumnCount(4);
    m_pLowerTable->insertRow(0);
    addItem(m_pLowerTable, 0, 0, KA_STR_statisticsTableResource);
    addItem(m_pLowerTable, 0, 1, KA_STR_statisticsTableRecommendedUsage);
    addItem(m_pLowerTable, 0, 2, KA_STR_statisticsTableUsage);
    addItem(m_pLowerTable, 0, 3, KA_STR_statisticsTablePerformanceImpact);
    m_pLowerTable->insertRow(1);
    addItem(m_pLowerTable, 1, 0, KA_STR_statisticsTableISASize, KA_STR_statisticsTableISASizeToolTip);
    addItem(m_pLowerTable, 1, 1, KA_STR_statisticsTable32KB);
    m_pLowerTable->insertRow(2);
    addItem(m_pLowerTable, 2, 0, KA_STR_statisticsTableScratchRegisters, KA_STR_statisticsTableScratchRegistersToolTip);
    addItem(m_pLowerTable, 2, 1, KA_STR_statisticsTable0);

    for (int row = 0; row < 3; row++)
    {
        for (int column = 0; column < 4; column++)
        {
            QTableWidgetItem* pItem = m_pLowerTable->item(row, column);

            if (nullptr == pItem)
            {
                addItem(m_pLowerTable, row, column, "");
                pItem = m_pLowerTable->item(row, column);
            }

            GT_IF_WITH_ASSERT(nullptr != pItem)
            {
                // Set the background color:
                if (0 == row)
                {
                    pItem->setData(Qt::BackgroundColorRole, m_a2Color);
                }

                if (column > 0 && column < 3)
                {
                    pItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
                }
            }
        }
    }

    m_pLowerTable->setColumnWidth(0, KA_RESOURCE_COLUMN_WIDTH);
    m_pLowerTable->setColumnWidth(1, KA_RECOMMANDED_COLUMN_WIDTH);
    m_pLowerTable->setColumnWidth(2, KA_USAGE_COLUMN_WIDTH - KA_RECOMMANDED_COLUMN_WIDTH);
    m_pLowerTable->setColumnWidth(3, KA_CONSTRAINT_COLUMN_WIDTH);

    m_pLowerTable->setFixedHeight(m_pLowerTable->verticalHeader()->length() + 2);
    m_pLowerTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pLowerTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    DisableTableSelection(m_pLowerTable, 0, 1);

    // Set the drawing delegate:
    kaStatisticsViewSIMDDelegate* pDelegate = new kaStatisticsViewSIMDDelegate(m_pLowerTable, -1, true);
    m_pLowerTable->setItemDelegate(pDelegate);

    // resizeToContent(m_pLowerTable);
}


void kaStatisticsView::RebuildSGPRsInformationTable(const QString& deviceDetails,
                                                    unsigned actualSGPRs, int& sgprsWavesLimit)
{
    GT_IF_WITH_ASSERT(m_pSGRPsInfoTable != nullptr)
    {
        // First remove all columns.
        int currColumnCount = m_pSGRPsInfoTable->columnCount();

        for (int column = 0; column < currColumnCount; ++column)
        {
            m_pSGRPsInfoTable->removeColumn(column);
        }

        int currRowCount = m_pSGRPsInfoTable->rowCount();

        for (int row = 0; row < currRowCount; ++row)
        {
            m_pSGRPsInfoTable->removeRow(row);
        }

        // Check the family and name of the device.
        const bool isViDevice = ((deviceDetails.indexOf(KA_V8_FAMILY_NAME) != -1));

        // Calculate the number of columns, and find the relevant data arrays for our device.
        size_t numOfColumns = 0;
        const unsigned* pMaxWavesPerSimdVals = nullptr;
        const unsigned* pNumOfSgprsUsedVals = nullptr;

        if (isViDevice)
        {
            numOfColumns = gSGPRs_VI_PI_AI_DEFAULT_NUM_OF_VALUES;
            pMaxWavesPerSimdVals = gSGPRs_VI_PI_AI_DEFAULT_MAX_WAVES_PER_SIMD;
            pNumOfSgprsUsedVals = gSGPRs_VI_PI_AI_DEFAULT;
        }
        else
        {
            numOfColumns = gSGPRs_CI_SI_NUM_OF_VALUES;
            pMaxWavesPerSimdVals = gSGPRs_CI_SI_MAX_WAVES_PER_SIMD;
            pNumOfSgprsUsedVals = gSGPRs_CI_SI;
        }

        // Set the number of columns and add another row.
        m_pSGRPsInfoTable->setColumnCount(numOfColumns);
        m_pSGRPsInfoTable->insertRow(0);

        // Set the Max waves per SIMD row's values.
        for (size_t column = 0; column < numOfColumns; column++)
        {
            QString stringToAdd = QString::number(pMaxWavesPerSimdVals[column]);
            addItem(m_pSGRPsInfoTable, 0, column, stringToAdd);

            // Set Color and alignment
            QTableWidgetItem* pItem = m_pSGRPsInfoTable->item(0, column);

            if (nullptr != pItem)
            {
                unsigned colorIndex = 10 - pMaxWavesPerSimdVals[column];
                pItem->setData(Qt::BackgroundColorRole, gColorTable[colorIndex]);
                pItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);

                // Make item non-selectable.
                Qt::ItemFlags itemFlags = pItem->flags();
                itemFlags &= ~Qt::ItemIsSelectable;
                pItem->setFlags(itemFlags);
            }
        }

        // Set the Number of SGPRs used row.
        for (size_t column = 0; column < numOfColumns; column++)
        {
            QString stringToAdd = QString::number(pNumOfSgprsUsedVals[column]);

            if (column == 0)
            {
                stringToAdd = "<=" + stringToAdd;
            }
            else if (column == numOfColumns - 1)
            {
                stringToAdd = stringToAdd + "<";
            }
            else
            {
                QString lowerLimit = QString::number(pNumOfSgprsUsedVals[column - 1] + 1);
                stringToAdd = lowerLimit + "-" + stringToAdd;
            }

            // Add the item to the table.
            addItem(m_pSGRPsInfoTable, 1, column, stringToAdd);

            // Set Color and alignment.
            QTableWidgetItem* pItem = m_pSGRPsInfoTable->item(1, column);

            if (nullptr != pItem)
            {
                // Set the text.
                pItem->setText(stringToAdd);
                unsigned colorIndex = 10 - pMaxWavesPerSimdVals[column];
                pItem->setData(Qt::BackgroundColorRole, gColorTable[colorIndex]);
                pItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);

                // Make item non-selectable.
                Qt::ItemFlags itemFlags = pItem->flags();
                itemFlags &= ~Qt::ItemIsSelectable;
                pItem->setFlags(itemFlags);

                // Make the font smaller.
                QFont itemFont = pItem->font();
                itemFont.setPointSize(itemFont.pointSize() - 1);
                pItem->setFont(itemFont);
            }
        }

        // Prevent the table from changing its size.
        m_pSGRPsInfoTableLabels->setFixedSize(m_pSGRPsInfoTableLabels->horizontalHeader()->length() + 2, m_pSGRPsInfoTableLabels->verticalHeader()->length() + 2);

        // Take the lowest value as the default for max waves per SIMD.
        sgprsWavesLimit = pMaxWavesPerSimdVals[numOfColumns - 1];

        // Highlight the relevant item.
        unsigned sgprsIndex = 0;

        for (; sgprsIndex < numOfColumns; ++sgprsIndex)
        {
            if (actualSGPRs <= pNumOfSgprsUsedVals[sgprsIndex])
            {
                sgprsWavesLimit = pMaxWavesPerSimdVals[sgprsIndex];
                break;
            }
        }

        QString SGPRsWavesLimitStr = QString::number(sgprsWavesLimit);
        m_pUpperTable->setItemText(1, 2, SGPRsWavesLimitStr);
        HighlighInfoItem(m_pSGRPsInfoTable, sgprsIndex);
    }
}


// ---------------------------------------------------------------------------
void kaStatisticsView::CreateInformationTables()
{
    bool isLDSEnabled = IsLdsEnabled();

    m_pInfoGroupBoxTab1 = new QGroupBox;
    m_pInfoGroupBoxTab1->setFlat(true);
    QVBoxLayout* pInfoLayout = new QVBoxLayout;
    m_pInfoGroupBoxTab1->setLayout(pInfoLayout);
    QLabel* pUpperLine = new QLabel(KA_STR_statisticsTableTipGroupDescription);
    pInfoLayout->addWidget(pUpperLine);

    InitTableAndLabels(m_pSGRPsInfoTableLabels, m_pSGRPsInfoTable, pInfoLayout);
    InitTableAndLabels(m_pVGRPsInfoTableLabels, m_pVGRPsInfoTable, pInfoLayout);

    m_pSGRPsInfoTable->setColumnCount(7);
    m_pVGRPsInfoTable->setColumnCount(10);
    m_pSGRPsInfoTableLabels->setColumnCount(1);
    m_pVGRPsInfoTableLabels->setColumnCount(1);

    if (isLDSEnabled)
    {
        InitTableAndLabels(m_pLDSInfoTableLabels, m_pLDSInfoTable, pInfoLayout);
        m_pLDSInfoLine = new QLabel(KA_STR_statisticsTableLDSUsedInfo);
        pInfoLayout->addWidget(m_pLDSInfoLine);

        m_pLDSInfoTable->setColumnCount(10);
        m_pLDSInfoTableLabels->setColumnCount(1);
    }

    // Add text section
    addItem(m_pSGRPsInfoTableLabels, 0, 0, KA_STR_statisticsTableMaxWavesForSIMD);
    addItem(m_pSGRPsInfoTableLabels, 1, 0, KA_STR_statisticsTableSGPRsWaves);
    addItem(m_pVGRPsInfoTableLabels, 0, 0, KA_STR_statisticsTableMaxWavesForSIMD);
    addItem(m_pVGRPsInfoTableLabels, 1, 0, KA_STR_statisticsTableVGPRsWaves);

    if (isLDSEnabled)
    {
        addItem(m_pLDSInfoTableLabels, 0, 0, KA_STR_statisticsTableMaxWavesForSIMD);
        addItem(m_pLDSInfoTableLabels, 1, 0, KA_STR_statisticsTableLDSWaves);
    }

    for (int nColumn = 0; nColumn < 10; nColumn++)
    {
        for (int nRow = 0; nRow < 2; nRow++)
        {
            for (int nTable = 0; nTable < 3; nTable++)
            {
                if (nColumn < 7 || nTable > 0)
                {
                    if (isLDSEnabled || nTable != 2)
                    {
                        QString stringToAdd;
                        QString stringToAdd2;

                        switch (nRow)
                        {
                            case 0: stringToAdd = QString::number(10 - nColumn); break;

                            case 1:
                            {
                                switch (nTable)
                                {
                                    case 0: stringToAdd = QString::number(gSGPRs_CI_SI[nColumn]); break;

                                    case 1: stringToAdd = QString::number(gVGPRs[nColumn]); break;

                                    case 2: stringToAdd = QString::number(0);               break;

                                    default: break;
                                };
                            }

                            default: break;
                        };

                        if (1 == nRow)
                        {
                            if (0 == nColumn)
                            {
                                stringToAdd = "<=" + stringToAdd;
                            }
                            else if (9 == nColumn || (6 == nColumn && nTable == 0))
                            {
                                stringToAdd = stringToAdd + "<";
                            }
                            else
                            {
                                QString lowerLimit;

                                switch (nTable)
                                {
                                    case 0: lowerLimit = QString::number(gSGPRs_CI_SI[nColumn - 1] + 1); break;

                                    case 1: lowerLimit = QString::number(gVGPRs[nColumn - 1] + 1); break;

                                    case 2: lowerLimit = QString::number(0);               break;

                                    default: break;
                                };

                                stringToAdd = lowerLimit + "-" + stringToAdd;
                            }
                        }

                        switch (nTable)
                        {
                            case 0:
                                addItem(m_pSGRPsInfoTable, nRow, nColumn, stringToAdd);
                                break;

                            case 1:
                                addItem(m_pVGRPsInfoTable, nRow, nColumn, stringToAdd);
                                break;

                            case 2:
                                addItem(m_pLDSInfoTable, nRow, nColumn, stringToAdd);
                                break;

                            default:
                                break;
                        }

                        // Set Color and alignment
                        QTableWidgetItem* pItem = nullptr;

                        switch (nTable)
                        {
                            case 0:
                                pItem = m_pSGRPsInfoTable->item(nRow, nColumn);
                                break;

                            case 1:
                                pItem = m_pVGRPsInfoTable->item(nRow, nColumn);
                                break;

                            case 2:
                                pItem = m_pLDSInfoTable->item(nRow, nColumn);
                                break;

                            default:
                                break;
                        }

                        if (nullptr != pItem)
                        {
                            pItem->setData(Qt::BackgroundColorRole, gColorTable[nColumn]);
                            pItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
                            // make item not selectable:
                            Qt::ItemFlags itemFlags = pItem->flags();
                            itemFlags &= ~Qt::ItemIsSelectable;
                            pItem->setFlags(itemFlags);

                            // for row 1 make the font smaller
                            if (1 == nRow)
                            {
                                QFont itemFont = pItem->font();
                                itemFont.setPointSize(itemFont.pointSize() - 1);
                                pItem->setFont(itemFont);
                            }
                        }
                    }
                }
            }
        }
    }

    m_pSGRPsInfoTableLabels->setFixedSize(m_pSGRPsInfoTableLabels->horizontalHeader()->length() + 2, m_pSGRPsInfoTableLabels->verticalHeader()->length() + 2);
    m_pSGRPsInfoTable->setFixedSize(m_pSGRPsInfoTable->horizontalHeader()->length() + 2, m_pSGRPsInfoTable->verticalHeader()->length() + 2);
    m_pVGRPsInfoTableLabels->setFixedSize(m_pVGRPsInfoTableLabels->horizontalHeader()->length() + 2, m_pVGRPsInfoTableLabels->verticalHeader()->length() + 2);
    m_pVGRPsInfoTable->setFixedSize(m_pVGRPsInfoTable->horizontalHeader()->length() + 2, m_pVGRPsInfoTable->verticalHeader()->length() + 2);

    if (isLDSEnabled)
    {
        m_pLDSInfoTableLabels->setFixedSize(m_pLDSInfoTableLabels->horizontalHeader()->length() + 2, m_pLDSInfoTableLabels->verticalHeader()->length() + 2);
        m_pLDSInfoTable->setFixedSize(m_pLDSInfoTable->horizontalHeader()->length() + 2, m_pLDSInfoTable->verticalHeader()->length() + 2);
    }

    // set delegates:
    for (int nTable = 0; nTable < 3; nTable++)
    {
        m_pInfoDelegates[nTable] = new kaStatisticsViewTipDelegate();
    }

    m_pSGRPsInfoTable->setItemDelegate(m_pInfoDelegates[0]);
    m_pVGRPsInfoTable->setItemDelegate(m_pInfoDelegates[1]);

    if (isLDSEnabled)
    {
        m_pLDSInfoTable->setItemDelegate(m_pInfoDelegates[2]);
    }

    //    QGroupBox* pFinalTipGroup = new QGroupBox();
    QLabel* pFinalTipLabel = new QLabel(KA_STR_statisticsTableFinalTip);
    QPixmap finalTipIcon;
    acSetIconInPixmap(finalTipIcon, AC_ICON_WARNING_INFO);
    QLabel* pFinalTipIcon = new QLabel();
    pFinalTipIcon->setPixmap(finalTipIcon);

    QHBoxLayout* pFinalTipLayout = new QHBoxLayout;
    pFinalTipLayout->addWidget(pFinalTipIcon);
    pFinalTipLayout->addWidget(pFinalTipLabel);
    pFinalTipLayout->addStretch(1);

    pInfoLayout->addLayout(pFinalTipLayout);
    m_pInfoGroupBoxTab1->setContentsMargins(0, 10, 0, 0);

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pInfoTabWidget != nullptr)
    {
        // Add the information group box to the tab widget:
        QScrollArea* pScrollArea = new QScrollArea;
        pScrollArea->setWidget(m_pInfoGroupBoxTab1);
        pScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        pScrollArea->setWidgetResizable(true);
        pScrollArea->setContentsMargins(0, 0, 0, 0);
        pScrollArea->setBackgroundRole(QPalette::Base);
        pScrollArea->setFrameStyle(0);
        m_pInfoTabWidget->addTab(pScrollArea, KA_STR_statisticsTableTipReferenceTableCaptionTab1);
    }

    pInfoLayout->addStretch(1);

    SetTablesContextMenuPolicy();

}

void kaStatisticsView::SetTablesContextMenuPolicy(Qt::ContextMenuPolicy policy)
{
    if (nullptr != m_pSGRPsInfoTableLabels)
    {
        m_pSGRPsInfoTableLabels->setContextMenuPolicy(policy);
    }

    if (nullptr != m_pSGRPsInfoTable)
    {
        m_pSGRPsInfoTable->setContextMenuPolicy(policy);
    }

    if (nullptr != m_pVGRPsInfoTableLabels)
    {
        m_pVGRPsInfoTableLabels->setContextMenuPolicy(policy);
    }

    if (nullptr != m_pVGRPsInfoTable)
    {
        m_pVGRPsInfoTable->setContextMenuPolicy(policy);
    }

    if (nullptr != m_pLDSInfoTableLabels)
    {
        m_pLDSInfoTableLabels->setContextMenuPolicy(policy);
    }

    if (nullptr != m_pUpperTable)
    {
        m_pUpperTable->setContextMenuPolicy(policy);
    }

    if (nullptr != m_pLowerTable)
    {
        m_pLowerTable->setContextMenuPolicy(policy);
    }

    if (nullptr != m_pLDSInfoTable)
    {
        m_pLDSInfoTable->setContextMenuPolicy(policy);
    }

    if (nullptr != m_pMaxWavesTable)
    {
        m_pMaxWavesTable->setContextMenuPolicy(policy);
    }
}

// ---------------------------------------------------------------------------
void kaStatisticsView::InitTableAndLabels(acListCtrl*& pLabelSide, acListCtrl*& pInfoSide, QBoxLayout* pLayoutToInsert)
{
    pLabelSide = new acListCtrl(nullptr, KA_ANALYSIS_TABLE_HEIGHT);
    pInfoSide = new acListCtrl(nullptr, KA_ANALYSIS_TABLE_HEIGHT);

    // Hide the headers:
    pLabelSide->verticalHeader()->hide();
    pLabelSide->horizontalHeader()->hide();
    pInfoSide->verticalHeader()->hide();
    pInfoSide->horizontalHeader()->hide();

    // insert two rows:
    pLabelSide->insertRow(0);
    pLabelSide->insertRow(1);
    pInfoSide->insertRow(0);
    pInfoSide->insertRow(1);

    pLabelSide->setFrameShape(QFrame::NoFrame);
    QHBoxLayout* pHBoxLayout = new QHBoxLayout;
    pHBoxLayout->addWidget(pLabelSide, 0, Qt::AlignLeft);
    pHBoxLayout->addWidget(pInfoSide, 0, Qt::AlignLeft);
    pHBoxLayout->addStretch(1);
    pLayoutToInsert->addLayout(pHBoxLayout);
}

// ---------------------------------------------------------------------------
bool kaStatisticsView::handleDeviceData(QString& deviceName, QString& familyName, QString& deviceInfo)
{
    GT_IF_WITH_ASSERT(m_pDeviceComboBox)
    {
        // do not insert device of CPU family to devices combo
        if (familyName != "CPU")
        {
            // insert the name of the family name and device alphabetically after  "general old devices"
            QString deviceFullName = familyName + ": " + deviceName;
            int numDevices = m_pDeviceComboBox->count();
            int beforeDevice = 0;

            // start looking only after the "general old device" string if there is one
            if (numDevices > 0 && m_pDeviceComboBox->itemText(0) == KA_STR_statisticsTableComboGeneralDeviceLine)
            {
                beforeDevice = 1;
            }

            while (beforeDevice < numDevices)
            {
                if (deviceFullName < m_pDeviceComboBox->itemText(beforeDevice))
                {
                    beforeDevice++;
                }
                else
                {
                    break;
                }
            }

            // Save the device info:
            m_mapDeviceNameToData[deviceFullName] = deviceInfo;

            m_pDeviceComboBox->insertItem(beforeDevice, deviceFullName);
        }
    }

    return true;
}

/// Update the view by reloading the file and rebuilding the table:
// ---------------------------------------------------------------------------
bool kaStatisticsView::updateView()
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(nullptr != m_pDeviceComboBox)
    {
        // get the current item in the combobox for restore:
        QString itemToRestore = m_pDeviceComboBox->currentText();

        // clear the combobox before loading the file again:
        m_pDeviceComboBox->clear();

        // update local workgroup dimensions for dx compute shader
        if ((m_pKernelInfo != nullptr) && !m_kernelFilePath.isEmpty() && (m_buildProfile.find(KA_STR_ComputeShaderProfilePrefix) != -1))
        {
            UpdateLocalWorkgroupDimensions(KA_PROJECT_DATA_MGR_INSTANCE.dataFileByPath(m_kernelFilePath), acQStringToGTString(m_pKernelInfo->m_kernelName), m_buildProfile);
        }

        retVal = kaTableView::updateView();

        // find the old item and restore it if possible. if not select last item:
        int restoreIndex = m_pDeviceComboBox->findText(itemToRestore);

        if (-1 != restoreIndex)
        {
            m_pDeviceComboBox->setCurrentIndex(restoreIndex);
        }
        else
        {
            m_pDeviceComboBox->setCurrentIndex(m_pDeviceComboBox->count() - 1);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
void kaStatisticsView::OnDeviceComboBoxChanged(int deviceIndex)
{
    // if this is after a true selection and not clear action
    if (deviceIndex >= 0)
    {
        GT_IF_WITH_ASSERT(nullptr != m_pStackedLayout && nullptr != m_pDeviceComboBox)
        {
            // check if the stacked layout need to be updated:
            QString deviceText = m_pDeviceComboBox->itemText(deviceIndex);

            if (KA_STR_statisticsTableComboGeneralDeviceLine == deviceText)
            {
                m_pStackedLayout->setCurrentIndex(0);
            }
            else
            {
                m_pStackedLayout->setCurrentIndex(1);

                // Fill the data of the selected device:
                QString deviceData = m_mapDeviceNameToData[deviceText];
                GT_IF_WITH_ASSERT(!deviceData.isEmpty())
                {
                    QStringList deviceDataList = deviceData.split(",");

                    GT_IF_WITH_ASSERT(deviceDataList.count() == 13)
                    {
                        // Lower table information:
                        QPixmap meetsRecommanded;
                        acSetIconInPixmap(meetsRecommanded, AC_ICON_ANALYZE_STATISTICS_MEETS);
                        QPixmap mayDegrade;
                        acSetIconInPixmap(mayDegrade, AC_ICON_ANALYZE_STATISTICS_DEGRADE);
                        // Add the isa size info:
                        int isaSize = deviceDataList[kaBackendManager::kaStatISASize].toInt();
                        QString isaSizeStr = BytesToKBString(isaSize);

                        m_pLowerTable->setItemText(1, 2, isaSizeStr);
                        QTableWidgetItem* pItem = m_pLowerTable->item(1, 3);

                        if (nullptr != pItem)
                        {
                            pItem->setText((isaSize < 32768) ? KA_STR_statisticsTableMeetsRecommended : KA_STR_statisticsTableMayDegrade);
                            pItem->setIcon((isaSize < 32768) ? meetsRecommanded : mayDegrade);
                        }

                        // Add scratch reg info:
                        int scratchReg = deviceDataList[kaBackendManager::kaStatScratchRegs].toInt();
                        m_pLowerTable->setItemText(2, 2, deviceDataList[kaBackendManager::kaStatScratchRegs]);
                        pItem = m_pLowerTable->item(2, 3);

                        if (nullptr != pItem)
                        {
                            pItem->setText((0 == scratchReg) ? KA_STR_statisticsTableMeetsRecommended : KA_STR_statisticsTableMayDegrade);
                            pItem->setIcon((0 == scratchReg) ? meetsRecommanded : mayDegrade);
                        }

                        // Upper table information:
                        // SGPRs
                        int SGPRsVal = deviceDataList[kaBackendManager::kaStatSGPRs].toInt();
                        QString SGPRsStr = deviceDataList[kaBackendManager::kaStatSGPRs] + " " + KA_STR_statisticsTableRegisters;
                        m_pUpperTable->setItemText(1, 1, SGPRsStr);

                        int SGPRsWavesLimit = 0;
                        RebuildSGPRsInformationTable(deviceText, SGPRsVal, SGPRsWavesLimit);

                        // VGPRs
                        int VGPRsVal = deviceDataList[kaBackendManager::kaStatVGPRs].toInt();
                        QString VGPRsStr = deviceDataList[kaBackendManager::kaStatVGPRs] + " " + KA_STR_statisticsTableRegisters;
                        m_pUpperTable->setItemText(2, 1, VGPRsStr);
                        int VGPRsWavesLimit = 1;

                        for (int nVGPRs = 8; nVGPRs >= 0; nVGPRs--)
                        {
                            if (VGPRsVal <= gVGPRs[nVGPRs])
                            {
                                VGPRsWavesLimit = 10 - nVGPRs;
                            }
                        }

                        QString VGPRsWavesLimitStr = QString::number(VGPRsWavesLimit);
                        m_pUpperTable->setItemText(2, 2, VGPRsWavesLimitStr);
                        HighlighInfoItem(m_pVGRPsInfoTable, 10 - VGPRsWavesLimit);

                        int LDSWavesLimit = 0;

                        if (IsLdsEnabled())
                        {
                            // LDS Size
                            m_LDSUsed = deviceDataList[kaBackendManager::kaStatLDSSize].toInt();
                            QString usedLdsStr = QString::number(m_LDSUsed) + " " + KA_STR_statisticsTableDynamicBytes;
                            m_pUpperTable->setItemText(3, 1, usedLdsStr);

                            // update the LDS if it is the maximum to the dynamic memory
                            bool maximumLDSUsed = false;

                            if (deviceText.indexOf(KA_V6_FAMILY_NAME) != -1)
                            {
                                if (KA_MAX_LDS_V6 == m_LDSUsed)
                                {
                                    maximumLDSUsed = true;
                                }
                            }
                            else
                            {
                                if (KA_MAX_LDS_V7 == m_LDSUsed)
                                {
                                    maximumLDSUsed = true;
                                }
                            }

                            if (maximumLDSUsed)
                            {
                                if (!m_wasDynamicChangedByUser)
                                {
                                    QString dynamicStr;
                                    dynamicStr.setNum(m_LDSUsed);

                                    if (nullptr != m_pDynamicLDS)
                                    {
                                        m_pDynamicLDS->setText(dynamicStr);
                                    }
                                }

                                m_LDSUsed = 0;
                            }

                            // Get the LDS limits based on the current LDS and dynamic value
                            LDSWavesLimit = UpdateWorkgroupBasedInfo();
                            m_pUpperTable->setItemText(3, 2, QString::number(LDSWavesLimit));
                            HighlighInfoItem(m_pLDSInfoTable, 10 - LDSWavesLimit);
                        }

                        // Update the Max waves Table
                        CalcMaximumConstrain();

                        UpdateRecommendation(SGPRsWavesLimit, VGPRsWavesLimit, LDSWavesLimit);

                        // Update the ISA sections in the information table widget:
                        UpdateISASections(deviceText);
                    }
                }
            }

        }
    }

    ResizeToLargestOfThree(m_pSGRPsInfoTableLabels, m_pVGRPsInfoTableLabels, m_pLDSInfoTableLabels);
    ResizeToLargestOfThree(m_pSGRPsInfoTable, m_pVGRPsInfoTable, m_pLDSInfoTable);
}

// ---------------------------------------------------------------------------
void kaStatisticsView::CalcMaximumConstrain()
{
    GT_IF_WITH_ASSERT(nullptr != m_pUpperTable)
    {
        // Take the constrains from the table:
        QString SGPRConstrainStr, VGPRConstrainStr, LDSConstrainStr;
        bool rc = m_pUpperTable->getItemText(1, 2, SGPRConstrainStr);
        rc &= m_pUpperTable->getItemText(2, 2, VGPRConstrainStr);

        if (IsLdsEnabled())
        {
            rc &= m_pUpperTable->getItemText(3, 2, LDSConstrainStr);
        }

        GT_IF_WITH_ASSERT(rc)
        {
            bool rcToInt;
            // For each constrain make sure it is 10 in case there are problems in the conversion.
            int SGPRContrain = SGPRConstrainStr.toInt(&rcToInt);

            if (!rcToInt)
            {
                SGPRContrain = 10;
                GT_ASSERT(false);
            }

            int VGPRContrain = VGPRConstrainStr.toInt(&rcToInt);

            if (!rcToInt)
            {
                VGPRContrain = 10;
                GT_ASSERT(false);
            }

            int maxWaves = min(SGPRContrain, VGPRContrain);

            if (IsLdsEnabled())
            {
                int LDSContrain = LDSConstrainStr.toInt(&rcToInt);

                if (!rcToInt)
                {
                    LDSContrain = 10;
                    GT_ASSERT(false);
                }

                maxWaves = min(maxWaves, LDSContrain);
            }

            QString maxWavesStr = QString::number(maxWaves);
            m_pMaxWavesTable->setItemText(0, 2, maxWavesStr);
        }
    }
}

// ---------------------------------------------------------------------------
int kaStatisticsView::UpdateWorkgroupBasedInfo()
{
    int retLDSWavesLimit = 1;

    int LDSWavesTable[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    GT_IF_WITH_ASSERT(nullptr != m_pKernelInfo)
    {
        QString LDSWorkGroupStr;
        // Create LDS table
        int usedLDSForWaves = 1;
        int LDSWaves = 10;
        int currentWaves;
        // Local worksize of 0 is treated as 1 since the dimension is really 1 when executed.
        float totalWorkSize = ceil(((0 == m_pKernelInfo->m_localWorkSize[0] ? 1.0 : m_pKernelInfo->m_localWorkSize[0]) *
                                    (0 == m_pKernelInfo->m_localWorkSize[1] ? 1.0 : m_pKernelInfo->m_localWorkSize[1]) *
                                    (0 == m_pKernelInfo->m_localWorkSize[2] ? 1.0 : m_pKernelInfo->m_localWorkSize[2])) / 64.0);

        GT_IF_WITH_ASSERT(totalWorkSize >= 1.0)
        {
            if (IsLdsEnabled())
            {
                while (LDSWaves > 1 && usedLDSForWaves < 65536)
                {
                    currentWaves = (int)(totalWorkSize * floor(65536.0 / usedLDSForWaves)) / 4.0;

                    if (currentWaves > 10)
                    {
                        currentWaves = 10;
                    }

                    LDSWaves = currentWaves;

                    if (LDSWaves >= 0)
                    {
                        LDSWavesTable[LDSWaves] = usedLDSForWaves;
                        usedLDSForWaves++;
                    }
                    else
                    {
                        break;
                    }
                }

                // the upper limit is the same x<= and x> instead of x+1 that is in [1]
                LDSWavesTable[1] = LDSWavesTable[2];

                QString ldsInfoStr = QString(KA_STR_statisticsTableLDSUsedInfo).arg(m_pKernelInfo->m_localWorkSize[0]).arg(m_pKernelInfo->m_localWorkSize[1]).arg(m_pKernelInfo->m_localWorkSize[2]).arg(m_pDynamicLDS->text());
                m_pLDSInfoLine->setText(ldsInfoStr);

                // the default value is assuming we passed the max LDS for a single wave:
                int usedDynamic = m_pDynamicLDS->text().toInt();
                int possibleDynamicVal = 65536 - m_LDSUsed - usedDynamic;

                // update the constraint item
                for (int nLDS = 2; nLDS < 11; nLDS++)
                {
                    if ((m_LDSUsed + usedDynamic) <= LDSWavesTable[nLDS])
                    {
                        retLDSWavesLimit = nLDS;
                        possibleDynamicVal = LDSWavesTable[nLDS] - m_LDSUsed - usedDynamic;
                    }
                }

                QString LDSWavesLimitStr = QString::number(retLDSWavesLimit);
                m_pUpperTable->setItemText(3, 2, LDSWavesLimitStr);
                HighlighInfoItem(m_pLDSInfoTable, 10 - retLDSWavesLimit);

                // update the possible number of dynamic value:
                if (retLDSWavesLimit > 1)
                {
                    QString possibleDynamicStr = QString(KA_STR_statisticsTableLDSDynamicaAvailable).arg(possibleDynamicVal);
                    m_pUpperTable->setItemText(4, 0, possibleDynamicStr);
                    m_pUpperTable->setRowHidden(4, false);
                }
                else
                {
                    m_pUpperTable->setRowHidden(4, true);
                }

                m_pUpperTable->setFixedHeight(m_pUpperTable->verticalHeader()->length() + 2);

                // Update workgroup controls
                for (int nWorkgroup = 0; nWorkgroup < 3; nWorkgroup++)
                {
                    QString workgroupStr = QString::number(m_pKernelInfo->m_localWorkSize[nWorkgroup]);
                    m_pWorkgroup[nWorkgroup]->setText(workgroupStr);
                }

                // Update the info table
                for (int nLDS = 0; nLDS < 10; nLDS++)
                {
                    QString ldsStr = QString::number(LDSWavesTable[10 - nLDS]);
                    // Add the "," for thousands
                    gtString ldsGtStr = acQStringToGTString(ldsStr).addThousandSeperators();
                    ldsStr = acGTStringToQString(ldsGtStr);

                    if (LDSWavesTable[10 - nLDS] == 0)
                    {
                        ldsStr = "NA";
                    }

                    if (nLDS == 0)
                    {
                        ldsStr = "<=" + ldsStr;
                    }
                    else if (nLDS == 9 && LDSWavesTable[10 - nLDS] < 65535)
                    {
                        ldsStr = ldsStr + "<";
                    }
                    else
                    {
                        QString lowerLimit = QString::number(LDSWavesTable[10 - nLDS + 1] + 1);
                        ldsGtStr = acQStringToGTString(lowerLimit).addThousandSeperators();
                        lowerLimit = acGTStringToQString(ldsGtStr);
                        ldsStr = lowerLimit + "-" + ldsStr;
                    }

                    m_pLDSInfoTable->setItemText(1, nLDS, ldsStr);
                }
            }

            // Update LDS used in table:
            QString LDSValStr = BytesToKBString(m_LDSUsed);
            int dynamicVal = m_pDynamicLDS->text().toInt();
            QString dynamicStr = BytesToKBString(dynamicVal);
            QString totalStr = BytesToKBString(dynamicVal + m_LDSUsed);
            QString finalLDSString = QString(KA_STR_statisticsTableLDSUsage).arg(totalStr).arg(LDSValStr).arg(dynamicStr);
            m_pUpperTable->setItemText(3, 1, finalLDSString);
        }

        // need to update recommendation since lds limit might have changed
        QTableWidgetItem* pSGPRWavesItem = m_pUpperTable->item(1, 2);
        QTableWidgetItem* pVGPRWavesItem = m_pUpperTable->item(2, 2);

        if (nullptr != pSGPRWavesItem && nullptr != pVGPRWavesItem)
        {
            int SGPRsWavesLimit = pSGPRWavesItem->text().toInt();
            int VGPRsWavesLimit = pVGPRWavesItem->text().toInt();

            UpdateRecommendation(SGPRsWavesLimit, VGPRsWavesLimit, retLDSWavesLimit);
        }

        CalcMaximumConstrain();
    }

    return retLDSWavesLimit;
}

// ---------------------------------------------------------------------------
void kaStatisticsView::UpdateRecommendation(int SGPRsLimit, int VGPRsLimit, int LDSLimit)
{
    int aVals[3];
    QString aValsStr[3] = { KA_STR_statisticsTableSGPRs, KA_STR_statisticsTableVGPRs, KA_STR_statisticsTableLDS };
    aVals[0] = SGPRsLimit;
    aVals[1] = VGPRsLimit;
    aVals[2] = LDSLimit;

    // bubble sort on the array based on vals
    for (int i = 0; i < 2; i++)
    {
        for (int j = i; j < 2; j++)
        {
            if (aVals[j] > aVals[j + 1])
            {
                // move the upper value up and the related string:
                int temp = aVals[j];
                aVals[j] = aVals[j + 1];
                aVals[j + 1] = temp;
                QString tempStr = aValsStr[j];
                aValsStr[j] = aValsStr[j + 1];
                aValsStr[j + 1] = tempStr;
            }
        }
    }

    QString recommendStr;

    // Build the string
    if (aVals[0] < aVals[1])
    {
        recommendStr = QString(KA_STR_statisticsTableAdviceOneToTwo).arg(aValsStr[0]).arg(aValsStr[1]).arg(aValsStr[2]);
    }
    else if (aVals[0] < aVals[2])
    {
        recommendStr = QString(KA_STR_statisticsTableAdviceTwoToOne).arg(aValsStr[0]).arg(aValsStr[1]).arg(aValsStr[2]);
    }

    // show bulb
    QLayoutItem* pLightBulb = m_pAdviceBox->itemAt(0);

    if (nullptr != pLightBulb)
    {
        QWidget* pWidget = pLightBulb->widget();

        if (nullptr != pWidget)
        {
            pWidget->setVisible(!recommendStr.isEmpty());
        }
    }

    if (recommendStr.isEmpty())
    {
        // clear the text
        m_pRecommendLabel->setText("");
    }
    else
    {
        QString finalText = QString(KA_STR_statisticsTableAdvice).arg(recommendStr);
        m_pRecommendLabel->setText(finalText);
    }
}

// ---------------------------------------------------------------------------
void kaStatisticsView::HighlighInfoItem(acListCtrl* pInfoTable, int itemIndex)
{
    GT_IF_WITH_ASSERT(nullptr != pInfoTable)
    {
        for (int col = 0; col < pInfoTable->columnCount(); col++)
        {
            for (int row = 0; row < pInfoTable->rowCount(); row++)
            {
                QTableWidgetItem* pItem = pInfoTable->item(row, col);

                if (nullptr != pItem)
                {
                    QFont itemFont = pItem->font();
                    itemFont.setBold(col == itemIndex);
                    pItem->setFont(itemFont);
                }
            }
        }

        QAbstractItemDelegate* pItemDelegate = pInfoTable->itemDelegate();
        kaStatisticsViewTipDelegate* pTipDelegate = qobject_cast<kaStatisticsViewTipDelegate*>(pItemDelegate);
        GT_IF_WITH_ASSERT(nullptr != pTipDelegate)
        {
            pTipDelegate->setColumn(itemIndex);
        }
    }
}

// ---------------------------------------------------------------------------
void kaStatisticsView::ResizeToLargestOfThree(acListCtrl* pTable0, acListCtrl* pTable1, acListCtrl* pTable2)
{
    // find the largest width in all the tables:
    acListCtrl* pTables[3];
    pTables[0] = pTable0;
    pTables[1] = pTable1;
    pTables[2] = pTable2;

    // min width is 50 pixels:
    int maxColumnWidth = 50;

    for (int nTable = 0; nTable < 3; nTable++)
    {
        if (nullptr != pTables[nTable])
        {
            pTables[nTable]->setFont(font());

            for (int col = 0; col < pTables[nTable]->columnCount(); col++)
            {
                for (int row = 0; row < pTables[nTable]->rowCount(); row++)
                {
                    QTableWidgetItem* pItem = pTables[nTable]->item(row, col);

                    if (nullptr != pItem)
                    {
                        QString text = pItem->text();
                        QRect rect = pTables[nTable]->fontMetrics().boundingRect(text);
                        maxColumnWidth = qMax(rect.width() + 15, maxColumnWidth);
                    }
                }
            }
        }
    }

    // go through all tables and update column width
    for (int nTable = 0; nTable < 3; nTable++)
    {
        if (nullptr != pTables[nTable])
        {
            pTables[nTable]->setIgnoreResize(true);

            for (int col = 0; col < pTables[nTable]->columnCount(); col++)
            {
                pTables[nTable]->setColumnWidth(col, maxColumnWidth);
            }

            pTables[nTable]->setFixedSize(pTables[nTable]->horizontalHeader()->length() + 2, pTables[nTable]->verticalHeader()->length() + 2);
            pTables[nTable]->setIgnoreResize(false);
        }
    }

}

// ---------------------------------------------------------------------------
QString kaStatisticsView::BytesToKBString(int iValue)
{
    QString retStr = QString::number(iValue);

    if (iValue <= 0)
    {
        retStr = KA_STR_NA;
    }
    else
    {
        gtString retGtStr = acQStringToGTString(retStr).addThousandSeperators();
        retStr = acGTStringToQString(retGtStr);
        retStr = retStr + " " + KA_STR_statisticsTableDynamicBytes;
    }

    return retStr;
}

void kaStatisticsView::OnWorkGroupTextEdit(const QString& text)
{
    GT_UNREFERENCED_PARAMETER(text);
    m_shouldCheckWorkGroupValues = true;
}

// ---------------------------------------------------------------------------
void kaStatisticsView::OnWorkGroupEdit()
{
    int workGroup[3];
    GT_IF_WITH_ASSERT(nullptr != m_pKernelInfo)
    {
        if (m_shouldCheckWorkGroupValues)
        {
            m_shouldCheckWorkGroupValues = false;

            for (int nWorkitem = 0; nWorkitem < 3; nWorkitem++)
            {
                workGroup[nWorkitem] = m_pWorkgroup[nWorkitem]->text().toInt();
            }

            bool isValid = true;

            if ((0 == workGroup[0] && 0 == workGroup[1] && 0 == workGroup[2]) ||
                (workGroup[0] < 0 || workGroup[1] < 0 || workGroup[2] < 0))
            {
                isValid = false;
            }

            // if values are valid check for multiplication
            if (isValid)
            {
                if ((0 != m_pKernelInfo->m_globalWorkSize[0] % (workGroup[0] == 0 ? 1 : workGroup[0])) ||
                    (0 != m_pKernelInfo->m_globalWorkSize[1] % (workGroup[1] == 0 ? 1 : workGroup[1])) ||
                    (0 != m_pKernelInfo->m_globalWorkSize[2] % (workGroup[2] == 0 ? 1 : workGroup[2])))
                {
                    isValid = false;
                }
            }

            // check that local data x * y * z <= 256
            if (isValid)
            {
                if ((workGroup[0] == 0 ? 1 : workGroup[0]) *
                    (workGroup[1] == 0 ? 1 : workGroup[1]) *
                    (workGroup[2] == 0 ? 1 : workGroup[2]) > 256)
                {
                    isValid = false;
                }
            }

            for (int nWorkitem = 0; nWorkitem < 3; nWorkitem++)
            {
                m_pKernelInfo->m_localWorkSize[nWorkitem] = workGroup[nWorkitem];
            }

            if (isValid)
            {
                UpdateWorkgroupBasedInfo();

                ResizeToLargestOfThree(m_pSGRPsInfoTable, m_pVGRPsInfoTable, m_pLDSInfoTable);
            }
            else
            {
                // notify the use it is not a valid workgroup info
                acMessageBox::instance().warning(AF_STR_WarningA, KA_STR_analsisSettingInvalidDataMsg);
            }
        }
    }
}

// ---------------------------------------------------------------------------
void kaStatisticsView::OnDynamicLDSEdit()
{
    // get current LDS value
    int newValue = m_pDynamicLDS->text().toInt();

    // check if the value is new. if not - there is no need to update again
    if (m_lastDynamicLDS != newValue)
    {
        m_lastDynamicLDS = newValue;

        if (newValue >= 0)
        {
            UpdateWorkgroupBasedInfo();
        }
        else
        {
            // the LDS value is negative
            acMessageBox::instance().warning(AF_STR_WarningA, KA_STR_analsisSettingInvalidLdsDataMsg);
        }

        m_wasDynamicChangedByUser = true;
    }
}

// ---------------------------------------------------------------------------
void kaStatisticsView::DisableTableSelection(acListCtrl* pTable, int startRow, int numRows)
{
    GT_IF_WITH_ASSERT(nullptr != pTable && numRows > 0)
    {
        int numColumns = pTable->columnCount();

        for (int nCol = 0; nCol < numColumns; nCol++)
        {
            for (int nRow = startRow; nRow < startRow + numRows; nRow++)
            {
                QTableWidgetItem* pItem = pTable->item(nRow, nCol);

                if (nullptr != pItem)
                {
                    Qt::ItemFlags itemFlags = pItem->flags();
                    itemFlags &= ~Qt::ItemIsSelectable;
                    pItem->setFlags(itemFlags);
                }
            }
        }
    }
}

void kaStatisticsView::CreateISATabs()
{
    gtString kernelExtension;
    m_kernelFilePath.getFileExtension(kernelExtension);
    bool shouldShowShaderSource = (kernelExtension == AF_STR_clSourceFileExtension);

    if (shouldShowShaderSource)
    {
        // Add the ISA Tabs to the tab widget:

        m_pSCSourceShaderTab2 = new QLabel;
        m_pCSDataTab3 = new QLabel;

        m_pSCSourceShaderTab2->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        m_pCSDataTab3->setAlignment(Qt::AlignTop | Qt::AlignLeft);

        m_pSCSourceShaderTab2->setTextInteractionFlags(Qt::TextSelectableByMouse);
        m_pCSDataTab3->setTextInteractionFlags(Qt::TextSelectableByMouse);

        // Create the scrolling area for the ssource shader and CS data tabs:
        QScrollArea* pScrollArea2 = new QScrollArea;
        pScrollArea2->setWidget(m_pSCSourceShaderTab2);
        pScrollArea2->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        pScrollArea2->setWidgetResizable(true);
        pScrollArea2->setContentsMargins(0, 0, 0, 0);
        pScrollArea2->setBackgroundRole(QPalette::Base);
        pScrollArea2->setFrameStyle(0);

        QScrollArea* pScrollArea3 = new QScrollArea;
        pScrollArea3->setWidget(m_pCSDataTab3);
        pScrollArea3->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        pScrollArea3->setWidgetResizable(true);
        pScrollArea3->setContentsMargins(0, 0, 0, 0);
        pScrollArea3->setBackgroundRole(QPalette::Base);
        pScrollArea3->setFrameStyle(0);


        m_pInfoTabWidget->addTab(pScrollArea2, KA_STR_statisticsTableTipSCSourceDumpCaptionTab2);
        m_pInfoTabWidget->addTab(pScrollArea3, KA_STR_statisticsTableTipComputeShaderDataCaptionTab3);

        // Set a monotype font for the shaders text:
        QFont monoSpaceFont(AC_SOURCE_CODE_EDITOR_DEFAULT_FONT_FAMILY);
        monoSpaceFont.setStyleHint(QFont::Monospace);
        monoSpaceFont.setPointSize(AC_SOURCE_CODE_EDITOR_DEFAULT_FONT_SIZE);
        m_pSCSourceShaderTab2->setFont(monoSpaceFont);
        m_pCSDataTab3->setFont(monoSpaceFont);


        // Get the text from the device combo box:
        if (m_pDeviceComboBox != nullptr)
        {
            UpdateISASections(m_pDeviceComboBox->currentText());
        }
    }
}

void kaStatisticsView::UpdateISASections(const QString& deviceText)
{
    // Get the kernel file extension:
    gtString kernelExtension;
    m_kernelFilePath.getFileExtension(kernelExtension);
    bool shouldShowShaderSource = (kernelExtension == AF_STR_clSourceFileExtension);

    if (shouldShowShaderSource)
    {
        GT_IF_WITH_ASSERT((m_pCSDataTab3 != nullptr) && (m_pSCSourceShaderTab2 != nullptr))
        {
            // First, clear the former contents, and disable the tabs in case that there is no new content.
            m_pCSDataTab3->clear();
            m_pSCSourceShaderTab2->clear();
            m_pInfoTabWidget->setTabEnabled(1, false);
            m_pInfoTabWidget->setTabEnabled(2, false);

            // Make sure that the file path is initialized:
            if (!m_filePath.isEmpty() && !deviceText.isEmpty() && shouldShowShaderSource)
            {
                // Build the ISA file path for this device:
                osFilePath isaFilePath = m_filePath;
                gtString fileName;

                // deviceText is taken from devices combo box. String is expected to be in the format: FamilyName : DeviceName
                // We want to extract the DeviceName and use it as file name:
                gtString gtDeviceText = acQStringToGTString(deviceText);
                int colonPosition = gtDeviceText.findFirstOf(AF_STR_ColonW);
                GT_IF_WITH_ASSERT(colonPosition > 0)
                {
                    gtDeviceText.getSubString(colonPosition + 2, fileName.length() - 1, fileName);
                }

                // remove the statistics_ from file name
                gtString initialFileName;
                isaFilePath.getFileName(initialFileName);
                gtString seperator(KA_STR_fileSectionSeperatorW);
                int statEnd = initialFileName.find(seperator);
                GT_IF_WITH_ASSERT(statEnd != -1)
                {
                    initialFileName.extruct(0, statEnd);
                }

                // add the device name in the beginning to give the new isa file name in the current format:
                fileName.append(initialFileName);
                isaFilePath.setFileName(fileName);
                isaFilePath.setFileExtension(KA_STR_kernelViewISAExtension);

                QString isaSection, scShaderSection, csDataSection;
                bool rc = kaApplicationCommands::instance().ParseISAFile(isaFilePath, isaSection, scShaderSection, csDataSection);
                GT_IF_WITH_ASSERT(rc)
                {
                    // Set the text for both tabs:
                    m_pSCSourceShaderTab2->setText(scShaderSection);
                    m_pCSDataTab3->setText(csDataSection);

                    // Make sure that the kernel data tabs are enabled.
                    m_pInfoTabWidget->setTabEnabled(1, true);
                    m_pInfoTabWidget->setTabEnabled(2, true);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
void kaStatisticsViewSIMDDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    // Should the color indication be drawn
    if (index.column() == m_useColor)
    {
        GT_IF_WITH_ASSERT(nullptr != m_pOwningTable)
        {
            QTableWidgetItem* pItem = m_pOwningTable->item(index.row(), index.column());

            if (nullptr != pItem)
            {
                if (pItem->icon().isNull())
                {
                    QBrush backgroundBrush = pItem->background();

                    if (pItem->isSelected())
                    {
                        backgroundBrush = QBrush(m_pOwningTable->palette().color(QPalette::Highlight));
                    }

                    QRect wavesRect = option.rect;
                    painter->fillRect(wavesRect, backgroundBrush);

                    wavesRect.adjust(1, 1, -1, -1);
                    int wavesValue = pItem->text().toInt();

                    if (wavesValue > 0 && wavesValue <= 10)
                    {
                        wavesRect.setWidth(wavesRect.width() * wavesValue / 10);
                        QBrush fillBrush(gColorTable[10 - wavesValue]);
                        painter->fillRect(wavesRect, fillBrush);

                        QFont itemFont = pItem->font();
                        painter->setFont(itemFont);
                        // Draw the text in the middle of the rect:
                        painter->drawText(wavesRect, Qt::AlignHCenter | Qt::AlignVCenter, pItem->text());
                    }
                    else
                    {
                        this->QStyledItemDelegate::paint(painter, option, index);
                    }
                }
                else
                {
                    // if the cell has an icon draw it normaly
                    this->QStyledItemDelegate::paint(painter, option, index);
                }
            }
        }
    }
    else
    {
        // Now paint the normal cell contents
        this->QStyledItemDelegate::paint(painter, option, index);
    }

    // Should the columns splitters be drawn
    if ((index.column() > 0) && m_showColumns)
    {
        // Paint the line
        painter->save();
        painter->setPen(QColor(0, 0, 0, 220));
        painter->drawLine(option.rect.topLeft(), option.rect.bottomLeft());
        painter->restore();
    }
}

// ---------------------------------------------------------------------------
void kaStatisticsViewTipDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    // First draw the normal information
    this->QStyledItemDelegate::paint(painter, option, index);

    if (index.column() == m_column)
    {
        painter->save();
        int numPixels = 4;

        for (int nPixel = 0; nPixel < numPixels; nPixel++)
        {
            QColor lighColor = QColor(gColorTable[m_column].red() + (255 - gColorTable[m_column].red()) * (numPixels - nPixel) / (numPixels + 1), gColorTable[m_column].green() + (255 - gColorTable[m_column].green()) * (numPixels - nPixel) / (numPixels + 1), gColorTable[m_column].blue() + (255 - gColorTable[m_column].blue()) * (numPixels - nPixel) / (numPixels + 1));
            QColor darkColor = QColor(gColorTable[m_column].red() * (nPixel + 1) / (numPixels + 1), gColorTable[m_column].green() * (nPixel + 1) / (numPixels + 1), gColorTable[m_column].blue() * (nPixel + 1) / (numPixels + 1));
            QPen lightPen(lighColor);
            lightPen.setWidth(1);
            QPen darkPen(darkColor);
            darkPen.setWidth(1);
            painter->setPen(darkPen);
            QRect drawRect = option.rect;

            if (index.row() == 0)
            {
                drawRect.adjust(nPixel, nPixel, -nPixel, 0);
            }
            else
            {
                drawRect.adjust(nPixel, 0, -nPixel, -nPixel);
            }

            if (index.row() != 1)
            {
                painter->drawLine(drawRect.topLeft(), drawRect.topRight());
            }

            painter->drawLine(drawRect.topLeft(), drawRect.bottomLeft());
            painter->setPen(lightPen);
            painter->drawLine(drawRect.topRight(), drawRect.bottomRight());

            if (index.row() != 0)
            {
                painter->drawLine(drawRect.bottomLeft(), drawRect.bottomRight());
            }
        }

        painter->restore();
    }
}

QString kaStatisticsView::GetKernelName(const osFilePath& detailedPath)
{
    // Get the kernel name from the Detailed path:
    osFilePath kernelFilePath(detailedPath);
    kernelFilePath.setFileName(L"");
    kernelFilePath.setFileExtension(L"");

    gtString kernelFullPath = kernelFilePath.asString();
    int kernelNameStart = kernelFullPath.reverseFind(osFilePath::osPathSeparator);
    QString kernelNameAsQt(KA_STR_mdiKernelNamePlaceHolder);
    GT_IF_WITH_ASSERT(kernelNameStart != -1)
    {
        gtString kernelName;
        kernelFullPath.getSubString(kernelNameStart + 1, kernelFullPath.length(), kernelName);
        kernelNameAsQt = acGTStringToQString(kernelName);
    }

    return kernelNameAsQt;
}

void kaStatisticsView::UpdateLocalWorkgroupDimensions(kaSourceFile* pFile, const gtString& entryPoint, const gtString& shaderProfile)
{
    if (pFile != nullptr && (shaderProfile.find(KA_STR_ComputeShaderProfilePrefix) != -1))
    {
        QString shaderContents;
        bool rc = kaReadFileAsQString(pFile->filePath(), shaderContents);

        GT_IF_WITH_ASSERT(rc)
        {
            QString currentName = acGTStringToQString(entryPoint);
            int functionIndex = shaderContents.indexOf(currentName);

            if (functionIndex != -1)
            {
                QString dimensionsCode = shaderContents.mid(0, functionIndex);
                int numthreadsIndex = dimensionsCode.lastIndexOf("numthreads");

                if (numthreadsIndex != -1)
                {
                    dimensionsCode = dimensionsCode.mid(numthreadsIndex, functionIndex);
                    int openBracketIndex = dimensionsCode.indexOf("(");
                    int closeBracketIndex = dimensionsCode.indexOf(")");

                    if (openBracketIndex != -1 && closeBracketIndex != -1)
                    {
                        dimensionsCode = dimensionsCode.mid(openBracketIndex + 1, closeBracketIndex - openBracketIndex - 1);
                        QStringList dimensionsList = dimensionsCode.split(",");

                        if (dimensionsList.size() == 3)
                        {
                            bool isNumber;
                            int dimensionElement = -1;

                            for (int i = 0; i < 3; ++i)
                            {
                                dimensionElement = dimensionsList[i].toInt(&isNumber);

                                if (!isNumber)
                                {
                                    //may be a definition is used - try to find it's numerical value in the same file
                                    int dimValueIndex = shaderContents.indexOf(dimensionsList[i]);

                                    if (dimValueIndex != -1)
                                    {
                                        QString dimensionDef = shaderContents.mid(dimValueIndex + 1, shaderContents.indexOf("\n") - dimValueIndex);
                                        QRegExp rx("(\\d+)");
                                        int pos = rx.indexIn(dimensionDef, 0);

                                        if (pos != -1)
                                        {
                                            dimensionElement = (rx.cap(1)).toInt();
                                        }
                                    }
                                }

                                m_pKernelInfo->m_localWorkSize[i] = dimensionElement;
                            }
                        }
                    }
                }
            }
        }
    }

}
