//=============================================================
// Copyright (c) 2013 Advanced Micro Devices, Inc.
//
/// \file   cpCpuProfilingToolbar.cpp
/// \author GPU Developer Tools
/// \version $Revision: $
/// \brief Description:
//
//=============================================================
// $Id: $
// Last checkin:   $DateTime: $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: $
//=============================================================

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>

/// Local:
#include <inc/cpCpuProfilingToolbar.h>
#include <inc/StringConstants.h>

// Icons:
#include <images/PercentIcon.xpm>
#include <images/CallSamplingIcon.xpm>
#include <images/ExpandCollapseIcon.xpm>
#include <images/SixtyFourIcon.xpm>
#include <images/PidTidIcon.xpm>
#include <images/CodeBytesIcon.xpm>

// Static members initializations:
cpCpuProfilingToolbar* cpCpuProfilingToolbar::m_psMySingleInstance = nullptr;


cpCpuProfilingToolbar::cpCpuProfilingToolbar(QWidget* pParent) :
    acToolBar(pParent, ""),
    m_pViewsComboBox(nullptr),
    m_pPercentButton(nullptr),
    m_pSeparateComboBox(nullptr),
    m_pCpuFilterButton(nullptr),
    m_pCssButton(nullptr),
    m_pExpandCollapseButton(nullptr),
    m_p64bitButton(nullptr),
    m_pAggregateByMPComboBox(nullptr),
    m_MPAggregation(nullptr),
    m_pPidComboBox(nullptr),
    m_pTidComboBox(nullptr),
    m_pPidTidButton(nullptr),
    m_pSysLibs(nullptr),
    m_pFunctionsComboBox(nullptr),
    m_pWhiteSpaceButton(nullptr),
    m_pSrcDasmSel(nullptr),
    m_pCodeBytesButton(nullptr)

{
    // Create the toolbar object name:
    QString toolBarQtName("");
    setObjectName(toolBarQtName);

    // Hide me until CPU profile views are shown:
    hide();
}

cpCpuProfilingToolbar& cpCpuProfilingToolbar::instance()
{
    afMainAppWindow* pMainWindow = afMainAppWindow::instance();
    GT_IF_WITH_ASSERT(pMainWindow != nullptr)
    {
        // If my single instance was not created yet - create it:
        if (m_psMySingleInstance == nullptr)
        {
            m_psMySingleInstance = new cpCpuProfilingToolbar(m_psMySingleInstance);


            m_psMySingleInstance->initialize();
        }
    }

    return *m_psMySingleInstance;
}

void cpCpuProfilingToolbar::initialize()
{
    initializeSystemTabWidgets();
}

void cpCpuProfilingToolbar::initializeSystemTabWidgets()
{
    QLabel* pLabel = new QLabel(QString("Aggregate by: "), this);


    m_pAggregateByMPComboBox = new QComboBox(this);


    m_pAggregateByMPComboBox->insertItem(0, QString("Modules"));
    m_pAggregateByMPComboBox->insertItem(1, QString("Processes"));
    m_pAggregateByMPComboBox->setCurrentIndex(m_MPAggregation);

    cpCpuProfilingToolbar::instance().addWidget(pLabel);
    cpCpuProfilingToolbar::instance().addWidget(m_pAggregateByMPComboBox);


    pLabel = new QLabel(QString("View : "), this);


    m_pViewsComboBox = new QComboBox(this);


    m_pPercentButton = new QToolButton(this);
    m_pPercentButton->setIcon(QIcon(XpmPercentIcon));
    m_pPercentButton->setCheckable(true);
    m_pPercentButton->setToolTip(QString("Show Percentage"));

    m_pSeparateComboBox = new QComboBox(this);

    m_pSeparateComboBox->addItem("Separate none");
    m_pSeparateComboBox->addItem("Separate by core");

    m_pCpuFilterButton = new QPushButton("CPU Filter", this);

    m_pCpuFilterButton->setCheckable(true);

    cpCpuProfilingToolbar::instance().addWidget(pLabel);
    cpCpuProfilingToolbar::instance().addWidget(m_pViewsComboBox);
    cpCpuProfilingToolbar::instance().addWidget(m_pPercentButton);
    cpCpuProfilingToolbar::instance().addWidget(m_pSeparateComboBox);
    cpCpuProfilingToolbar::instance().addWidget(m_pCpuFilterButton);

    m_pCssButton = new QToolButton(this);

    m_pCssButton->setIcon(QIcon(XpmCallSamplingIcon));
    m_pCssButton->setEnabled(false);
    m_pCssButton->setToolTip("View Call Chain");

    m_pExpandCollapseButton = new QToolButton(this);

    m_pExpandCollapseButton->setIcon(QIcon(XpmExpandCollapse));
    m_pExpandCollapseButton->setCheckable(true);

    m_pExpandCollapseButton->setToolTip(QString("Expand / Collapse All"));

    m_p64bitButton = new QToolButton(this);

    m_p64bitButton->setIcon(QIcon(XpmSixtyFour));
    m_p64bitButton->setCheckable(true);
    m_p64bitButton->setToolTip(QString("Show 64-bit Column"));

    cpCpuProfilingToolbar::instance().addWidget(m_pCssButton);
    addWidget(m_pExpandCollapseButton);
    addWidget(m_p64bitButton);

    // Setup Pid Combobox
    QLabel* pPidLabel = new QLabel(QString("Pid: "), &cpCpuProfilingToolbar::instance());


    m_pPidComboBox = new QComboBox(&cpCpuProfilingToolbar::instance());


    m_pPidComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_pPidComboBox->setToolTip("Select PID");
    m_pPidComboBox->setCurrentIndex(-1);

    // Setup Tid Combobox
    QLabel* pTidLabel = new QLabel(QString(" Tid: "), &cpCpuProfilingToolbar::instance());


    m_pTidComboBox = new QComboBox(&cpCpuProfilingToolbar::instance());


    m_pTidComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_pTidComboBox->setToolTip("Select TID");

    m_pPidTidButton = new QToolButton(&cpCpuProfilingToolbar::instance());

    m_pPidTidButton->setIcon(QIcon(XpmPidTid));
    m_pPidTidButton->setCheckable(true);

    addWidget(pPidLabel);
    addWidget(m_pPidComboBox);
    addWidget(pTidLabel);
    addWidget(m_pTidComboBox);
    addWidget(m_pPidTidButton);

    m_pSysLibs = new QCheckBox(tr("Display System Libraries Functions"));

    m_pSysLibs->setCheckState(Qt::Unchecked);

    addWidget(m_pSysLibs);

    // Functions Combo box
    addWidget(new QLabel("Function : ", this));
    m_pFunctionsComboBox = new QComboBox(this);


    addWidget(m_pFunctionsComboBox);

    m_pWhiteSpaceButton = new QPushButton("White Space", this);

    m_pWhiteSpaceButton->setCheckable(true);
    m_pWhiteSpaceButton->setToolTip("Remove white space from source code");

    addWidget(m_pWhiteSpaceButton);

    // Src/Dasm Combo box
    addWidget(new QLabel("Type : ", this));
    m_pSrcDasmSel = new QComboBox(this);


    m_pCodeBytesButton = new QToolButton(this);

    m_pCodeBytesButton->setIcon(QIcon(XpmCodeBytes));
    m_pCodeBytesButton->setCheckable(true);

    m_pCodeBytesButton->setToolTip(QString("Hide / Show Code Bytes Column"));

    addWidget(m_pSrcDasmSel);
    addWidget(m_pCodeBytesButton);
}
