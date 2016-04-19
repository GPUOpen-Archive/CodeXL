//------------------------------ kaAnalysisView.cpp ------------------------------

// Infra:
#include <AMDTApplicationComponents/Include/acListCtrl.h>

// Local:
#include <AMDTKernelAnalyzer/src/kaAnalysisView.h>
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>

// ---------------------------------------------------------------------------
// Name:        kaAnalysisView::kaAnalysisView
// Description: Constructor
// Author:      Gilad Yarnitzky
// Date:        18/8/2013
// ---------------------------------------------------------------------------
kaAnalysisView::kaAnalysisView(QWidget* pParent, const osFilePath& kernelFilePath) : kaTableView(pParent, kernelFilePath, 1, 3)
{
    initializeMainTable(KA_STR_analysisTableInfo, KA_STR_analysisTableRows, KA_STR_analysisTableRowsTooltip);

    m_pTableInformationCaption = new QLabel(KA_STR_analysisTableInfo);

    m_pMainLayout->addWidget(m_pTableInformationCaption);
    m_pMainLayout->addWidget(m_pAnalysisTable);
    m_pMainLayout->addStretch(1);

    setLayout(m_pMainLayout);

    readDataFile();

    if (NULL != m_pAnalysisTable)
    {
        m_pAnalysisTable->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    }

    updateBGColors();

    resizeToContent(m_pAnalysisTable);
}


// ---------------------------------------------------------------------------
// Name:        kaAnalysisView::~kaAnalysisView
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        18/8/2013
// ---------------------------------------------------------------------------
kaAnalysisView::~kaAnalysisView()
{

}

