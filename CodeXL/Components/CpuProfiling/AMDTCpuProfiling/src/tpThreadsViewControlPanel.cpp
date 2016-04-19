//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpThreadsViewControlPanel.cpp
///
//==================================================================================

//------------------------------ tpThreadsView.cpp ------------------------------

// Qt
#include <qtIgnoreCompilerWarnings.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acNavigationChart.h>
#include <AMDTApplicationComponents/Include/acQHTMLWindow.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimeline.h>

// Local:
#include <inc/StringConstants.h>
#include <inc/tpThreadsViewControlPanel.h>


tpThreadsViewControlPanel::tpThreadsViewControlPanel(QWidget* pParent, tpSessionData* pSessionData) : QWidget(pParent),
    m_pTpSettingsLink(nullptr),
    m_pDisplaySettingDialog(nullptr),
    m_pDisplayTopCheckBox(nullptr),
    m_pDisplayTopSpinBox(nullptr),
    m_pSessionData(pSessionData)
{
    BuildPanelLayout();
}

tpThreadsViewControlPanel::~tpThreadsViewControlPanel()
{

}

void tpThreadsViewControlPanel::BuildPanelLayout()
{
    QVBoxLayout* pMainLayout = new QVBoxLayout;

    QHBoxLayout* pFirstLineLayout = new QHBoxLayout;

    QLabel* pTitle = new QLabel(CP_STR_ThreadProfileTypeName);
    pTitle->setStyleSheet("QLabel{font-weight: bold;}");
    pFirstLineLayout->addWidget(pTitle);

    // init display setting dialog window
    m_pDisplaySettingDialog = new tpDisplaySettingsDialog(nullptr, m_pSessionData);

    // create link label
    m_pTpSettingsLink = new QLabel(QString(CP_STR_SettingsLinkLAbel).arg("0").arg("0").arg("0").arg("0").arg("0"));
    m_pTpSettingsLink->setTextFormat(Qt::RichText);
    pFirstLineLayout->addWidget(m_pTpSettingsLink);
    pFirstLineLayout->addStretch();

    bool rc = connect(m_pTpSettingsLink, SIGNAL(linkActivated(const QString&)), this, SLOT(OnDisplaySettingsClicked(const QString&)));
    GT_ASSERT(rc);
    // update the label with current settings data
    int threadsNum = UpdateSettingsLinkLabel();

    QHBoxLayout* pSecondLineLayout = new QHBoxLayout;

    m_pDisplayTopCheckBox = new QCheckBox(CP_STR_ControlPanelDisplayTop);
    pSecondLineLayout->addWidget(m_pDisplayTopCheckBox);
    rc = connect(m_pDisplayTopCheckBox, SIGNAL(stateChanged(int)), this, SLOT(ControlPanelWidgetChanged(int)));
    GT_ASSERT(rc);

    m_pDisplayTopSpinBox = new QSpinBox;
    m_pDisplayTopSpinBox->setRange(1, 20);
    pSecondLineLayout->addWidget(m_pDisplayTopSpinBox);
    rc = connect(m_pDisplayTopSpinBox, SIGNAL(valueChanged(int)), this, SLOT(ControlPanelWidgetChanged(int)));
    GT_ASSERT(rc);

    QLabel* pLabel = new QLabel(QString(CP_STR_ControlPanelSignificantThreads).arg(threadsNum));
    pSecondLineLayout->addWidget(pLabel);
    pSecondLineLayout->addStretch();

    pMainLayout->addLayout(pFirstLineLayout);
    pMainLayout->addLayout(pSecondLineLayout);

    setLayout(pMainLayout);
}

void tpThreadsViewControlPanel::OnDisplaySettingsClicked(const QString& str)
{
    GT_UNREFERENCED_PARAMETER(str);

    // open thread profiling settings dialog window
    GT_IF_WITH_ASSERT(m_pDisplaySettingDialog != nullptr)
    {
        // if ok button was clicked - update the link label
        if (QDialog::Accepted == m_pDisplaySettingDialog->exec())
        {
            UpdateSettingsLinkLabel();
            ControlPanelWidgetChanged(0);
        }
    }
}

void tpThreadsViewControlPanel::ControlPanelWidgetChanged(int val)
{
    GT_UNREFERENCED_PARAMETER(val);

    tpControlPanalData dataControlPanel;

    // set displayed top threads number
    dataControlPanel.m_displayTopThreadsNum = 0;
    GT_IF_WITH_ASSERT(m_pDisplayTopCheckBox != nullptr)
    {
        if (m_pDisplayTopCheckBox->checkState() == Qt::Checked)
        {
            dataControlPanel.m_displayTopThreadsNum = m_pDisplayTopSpinBox->value();
        }
    }

    GT_IF_WITH_ASSERT(m_pDisplaySettingDialog != nullptr)
    {
        // set process and threads map
        m_pDisplaySettingDialog->SelectedProcessesAndThreadsMap(dataControlPanel.m_selectedProcessThreadsMap);

        // set cores list
        m_pDisplaySettingDialog->SelectedCoresList(dataControlPanel.m_selectedCoresList);
    }

    emit OnControlPanelDateChanged(dataControlPanel);
}

void tpThreadsViewControlPanel::ControlPanelWidgetChanged(QString val)
{
    GT_UNREFERENCED_PARAMETER(val);
    ControlPanelWidgetChanged(0);
}

int tpThreadsViewControlPanel::UpdateSettingsLinkLabel()
{
    QString selectedCoresStr;
    int selectedProcsNum = 0;
    int totalProcsNum = m_pSessionData->TotalProcessesCount();
    int selectedThreadsNum = 0;
    int totalThreadsNum = m_pSessionData->TotalThreadsCount();

    GT_IF_WITH_ASSERT(m_pDisplaySettingDialog != nullptr)
    {
        // get data from display setting dialog window instance
        m_pDisplaySettingDialog->GetTpSettingsData(selectedCoresStr, selectedProcsNum, selectedThreadsNum);

        // set the link label text
        m_pTpSettingsLink->setText(QString(CP_STR_SettingsLinkLAbel).arg(selectedCoresStr)
                                   .arg(selectedProcsNum)
                                   .arg(totalProcsNum)
                                   .arg(selectedThreadsNum)
                                   .arg(totalThreadsNum));
    }

    return totalThreadsNum;
}