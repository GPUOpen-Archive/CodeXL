//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file DisplayFilterDlg.cpp
///
//==================================================================================

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>
#include <QMessageBox>

// Infra:
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTApplicationComponents/Include/acDisplay.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>

// Local:
#include <inc/AmdtCpuProfiling.h>
#include <inc/DisplayFilterDlg.h>
#include <inc/SessionWindow.h>
#include <inc/SessionViewCreator.h>

#define STR_CORE "Core"
#define STR_All "All"

#define CP_DISPLAY_SETTINGS_WIDTH 220
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define CP_DISPLAY_SETTINGS_HEIGHT 540
#else
    #define CP_DISPLAY_SETTINGS_HEIGHT 575
#endif

#define CP_DISPLAY_SETTINGS_CORE_AREA_HEIGHT 90
#define CP_DISPLAY_SETTINGS_COLUMN_AREA_HEIGHT 180
#define CP_DISPLAY_SETTINGS_HMARGIN 7
#define CP_DISPLAY_SETTINGS_LAYOUT_MARGIN 5
#define CP_DISPLAY_SETTINGS_LABEL_HEIGHT 16

const QString formHeading("Display Settings");
const QString labelStyleSheet("QLabel { background-color: rgb(236, 236, 236); }");

DisplayFilterDlg* DisplayFilterDlg::m_psMySingleInstance = nullptr;

DisplayFilterDlg& DisplayFilterDlg::instance()
{
    if (nullptr == m_psMySingleInstance)
    {
        m_psMySingleInstance = new DisplayFilterDlg(afMainAppWindow::instance());

    }

    return *m_psMySingleInstance;

}

DisplayFilterDlg::DisplayFilterDlg(QWidget* pParent): QDialog(pParent),
    m_pSessionTreeItemData(nullptr),
    m_pCurrentSessionWindow(nullptr),
    m_pProfileReader(nullptr),
    m_pProfileInfo(nullptr),
    m_pCheckBoxCore(nullptr),
    m_pCheckBoxAllCore(nullptr),
    m_pLayoutForCoreList(nullptr),
    m_pWidgetCoreList(nullptr),
    m_noOfCores(0),
    m_noOfColumn(0),
    m_pWidgetColumnList(nullptr),
    m_pVBLayoutForColumn(nullptr),
    m_pCheckBoxColumns(nullptr),
    m_plabelColumns(nullptr),
    m_plabelGeneral(nullptr),
    m_plabelCPUCores(nullptr),
    m_pComboBoxViewes(nullptr),
    m_pCheckBoxDisplaySystemDLLs(nullptr),
    m_pCheckBoxShowPercentageBars(nullptr),
    m_pCheckBoxSeparateColumnsBy(nullptr),
    m_pRadioButtonSeparateByCore(nullptr),
    m_pRadioButtonSeparateByNUMA(nullptr),
    m_pPushButtonOK(nullptr),
    m_pPushButtonCancel(nullptr),
    m_pScrollAreaColumns(nullptr),
    m_pScrollAreaCPUCore(nullptr),
    m_pMainLayout(nullptr),
    m_pComboBoxViewesLayout(nullptr),
    m_pScrollAreaColumnsLayout(nullptr),
    m_pCheckBoxDisplaySystemDLLsLayout(nullptr),
    m_pCheckBoxShowPercentageBarsLayout(nullptr),
    m_pScrollAreaCPUCoreLayout(nullptr),
    m_pLine(nullptr),
    m_pCheckBoxSeparateColumnsByLayout(nullptr),
    m_pCoreLayout(nullptr),
    m_pNUMALayout(nullptr),
    m_pButtonBox(nullptr)
{

    initializeLayout();

    QObject::connect(m_pPushButtonOK, SIGNAL(clicked()), this, SLOT(onClickOk()));
    QObject::connect(m_pPushButtonCancel, SIGNAL(clicked()), this, SLOT(onClickCancel()));
    QObject::connect(m_pCheckBoxSeparateColumnsBy, SIGNAL(stateChanged(int)), this, SLOT(onClickCheckBoxSeparateColumnsBy(int)));
    QDialog::setWindowModality(Qt::ApplicationModal);
    afLoadTitleBarIcon(this);

    Qt::WindowFlags flags = windowFlags();
    flags &= ~Qt::WindowContextHelpButtonHint;
    setWindowFlags(flags);
}

DisplayFilterDlg::~DisplayFilterDlg()
{
    if (nullptr != m_pCheckBoxCore)
    {
        delete [] m_pCheckBoxCore;
    }
}


QDialog::DialogCode DisplayFilterDlg::displayDialog(const QString& sessionPath, bool enableOnlySystemDll)
{
    QDialog::DialogCode retVal = QDialog::Rejected;

    m_enableOnlySystemDll = enableOnlySystemDll;

    // Get the session view creat
    SessionViewCreator* pSessionViewCreator = AmdtCpuProfiling::sessionViewCreator();
    GT_IF_WITH_ASSERT(pSessionViewCreator != nullptr)
    {
        // Find the session window related to this path:
        osFilePath localPath(acQStringToGTString(sessionPath));
        m_pCurrentSessionWindow = pSessionViewCreator->findSessionWindow(localPath);

        GT_IF_WITH_ASSERT(m_pCurrentSessionWindow != nullptr)
        {
            m_pProfileReader = &m_pCurrentSessionWindow->profileReader();
            GT_IF_WITH_ASSERT(m_pProfileReader != nullptr)
            {
                m_pProfileInfo = m_pCurrentSessionWindow->profileReader().getProfileInfo();
            }

            SessionDisplaySettings* pSessionDisplaySettings = m_pCurrentSessionWindow->sessionDisplaySettings();
            m_pSessionTreeItemData = (afApplicationTreeItemData*)m_pCurrentSessionWindow->displayedItemData();
            GT_IF_WITH_ASSERT((nullptr != m_pProfileReader) && (nullptr != m_pProfileInfo) && (nullptr != m_pSessionTreeItemData) && (pSessionDisplaySettings != nullptr))
            {
                // Copy the settings to the display settings member:
                CPUGlobalDisplayFilter& globalFilter = CPUGlobalDisplayFilter::instance();
                m_displaySettings.CopyFrom(*pSessionDisplaySettings);
                m_displaySystemDLLs = globalFilter.m_displaySystemDLLs;
                m_displayPercentageInColumn = globalFilter.m_displayPercentageInColumn;

                if (m_displaySettings.m_separateBy == SEPARATE_BY_NONE)
                {
                    m_pCheckBoxSeparateColumnsBy->setChecked(false);
                    m_pRadioButtonSeparateByNUMA->setEnabled(false);
                    m_pRadioButtonSeparateByCore->setEnabled(false);
                }
                else
                {
                    m_pCheckBoxSeparateColumnsBy->setChecked(true);
                    m_pRadioButtonSeparateByCore->setEnabled(true);
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                    m_pRadioButtonSeparateByNUMA->setEnabled(true);
#else
                    m_pRadioButtonSeparateByCore->setChecked(true);
#endif

                    if (m_displaySettings.m_separateBy == SEPARATE_BY_NUMA)
                    {
                        m_pRadioButtonSeparateByNUMA->setChecked(true);
                    }
                    else if (m_displaySettings.m_separateBy == SEPARATE_BY_CORE)
                    {
                        m_pRadioButtonSeparateByCore->setChecked(true);
                    }
                }

                m_pCheckBoxDisplaySystemDLLs->setChecked(m_displaySystemDLLs);
                m_pCheckBoxShowPercentageBars->setChecked(m_displayPercentageInColumn);
            }

            initializeConfiguration();
            addFinalLayout();
            GT_IF_WITH_ASSERT(nullptr != m_pScrollAreaCPUCore)
            {
                QScrollBar* pVscrollbar =  m_pScrollAreaCPUCore->verticalScrollBar();

                if (nullptr != pVscrollbar)
                {
                    pVscrollbar->setValue(0);
                }
            }
        }
    }

    m_pCheckBoxDisplaySystemDLLs->setEnabled(true);
    disableAllControlsExceptSystemDll(m_enableOnlySystemDll);

    // Disable show percentage checkbox when CLU profiled
    if (m_displaySettings.m_pProfileInfo->m_isProfilingCLU)
    {
        m_pCheckBoxShowPercentageBars->setChecked(false);
        m_pCheckBoxShowPercentageBars->setEnabled(false);
    }

    // Display the dialog:
    int rc = exec();

    retVal = (QDialog::DialogCode)rc;

    return retVal;
}

bool DisplayFilterDlg::initializeLayout()
{
    bool retVal = true;

    QString strLabel("<html><body><b>&nbsp;&nbsp;%1</pre></b></body></html>");
    QPalette pal;

    m_plabelColumns = new QLabel;

    m_plabelColumns->setTextFormat(Qt::RichText);
    m_plabelColumns->setText(strLabel.arg("Columns"));
    m_plabelColumns->setStyleSheet(labelStyleSheet);
    m_plabelColumns->setFixedHeight(acScalePixelSizeToDisplayDPI(CP_DISPLAY_SETTINGS_LABEL_HEIGHT));

    m_plabelGeneral = new QLabel;

    m_plabelGeneral->setTextFormat(Qt::RichText);
    m_plabelGeneral->setText(strLabel.arg("General"));
    m_plabelGeneral->setStyleSheet(labelStyleSheet);
    m_plabelGeneral->setFixedHeight(acScalePixelSizeToDisplayDPI(CP_DISPLAY_SETTINGS_LABEL_HEIGHT));

    m_plabelCPUCores = new QLabel;

    m_plabelCPUCores->setTextFormat(Qt::RichText);
    m_plabelCPUCores->setText(strLabel.arg("CPU Cores"));
    m_plabelCPUCores->setStyleSheet(labelStyleSheet);
    m_plabelCPUCores->setFixedHeight(acScalePixelSizeToDisplayDPI(CP_DISPLAY_SETTINGS_LABEL_HEIGHT));

    m_pComboBoxViewes = new QComboBox;

    m_pCheckBoxDisplaySystemDLLs = new QCheckBox("Display system modules", this);

    m_pCheckBoxShowPercentageBars = new QCheckBox("Show percentages", this);

    m_pCheckBoxSeparateColumnsBy = new QCheckBox("Display data per:", this);

    m_pRadioButtonSeparateByCore = new QRadioButton("Core", this);

    m_pRadioButtonSeparateByNUMA = new QRadioButton("NUMA", this);

    m_pPushButtonOK = new QPushButton("OK");

    m_pPushButtonCancel = new QPushButton("Cancel");


    m_pScrollAreaColumns = new QScrollArea;

    m_pScrollAreaColumns->setFrameShape(QFrame::NoFrame);

    m_pScrollAreaCPUCore = new QScrollArea;

    m_pScrollAreaCPUCore->setFrameShape(QFrame::NoFrame);
    m_pScrollAreaCPUCore->setFixedHeight(acScalePixelSizeToDisplayDPI(CP_DISPLAY_SETTINGS_CORE_AREA_HEIGHT));

#if AMDT_BUILD_TARGET != AMDT_WINDOWS_OS
    m_pRadioButtonSeparateByNUMA->setEnabled(false);
#endif

    //Set background color
    pal = palette();
    pal.setColor(backgroundRole(), Qt::white);
    setPalette(pal);

    setWindowTitle(formHeading);

    return retVal;
}

bool DisplayFilterDlg::initializeConfiguration()
{
    bool retVal = true;

    if (m_pProfileInfo->m_numCpus > 0)
    {
        retVal = populateCoreList(m_pProfileInfo->m_numCpus);
    }

    retVal = retVal && populateColumnList();
    return retVal;

}

bool DisplayFilterDlg::populateCoreList(int noOfCores)
{
    bool retVal = true;

    if (nullptr == m_pWidgetCoreList || m_noOfCores != noOfCores)
    {
        m_noOfCores = noOfCores;

        m_pWidgetCoreList = new QWidget(m_pScrollAreaCPUCore);
        RETURN_FALSE_IF_NULL(m_pWidgetCoreList);
        m_pScrollAreaCPUCore->setWidgetResizable(true);
        m_pWidgetCoreList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_pLayoutForCoreList = new QGridLayout(m_pWidgetCoreList);

        RETURN_FALSE_IF_NULL(m_pLayoutForCoreList);
        m_pLayoutForCoreList->setMargin(0);
        m_pLayoutForCoreList->setSizeConstraint(QLayout::SetMaximumSize);

        //Add new items
        if (nullptr != m_pCheckBoxCore)
        {
            delete [] m_pCheckBoxCore;
        }

        m_pCheckBoxCore = new QCheckBox[m_noOfCores];
        RETURN_FALSE_IF_NULL(m_pCheckBoxCore);
        m_pCheckBoxAllCore = new QCheckBox;
        RETURN_FALSE_IF_NULL(m_pCheckBoxAllCore);
        //All
        m_pCheckBoxAllCore->setText(STR_All);
        m_pLayoutForCoreList->addWidget(m_pCheckBoxAllCore, 0, 0);
        QObject::connect(m_pCheckBoxAllCore, SIGNAL(stateChanged(int)), this, SLOT(onClickAllCoreItem(int)));

        //Individual core
        QString strCoreName;

        for (int i = 0; i < m_noOfCores; ++i)
        {
            strCoreName = QString("%1 %2").arg(STR_CORE).arg(i);
            m_pCheckBoxCore[i].setText(strCoreName);

            if ((m_noOfCores > 10) && (1 == i % 2) && (i < 10))
            {
                strCoreName.append(" ");
            }

            m_pLayoutForCoreList->addWidget(m_pCheckBoxCore + i, i / 2 + 1, i % 2,  Qt::AlignLeft);
            QObject::connect(m_pCheckBoxCore + i, SIGNAL(stateChanged(int)), this, SLOT(onClickCoreItem(int)));
        }

        m_pWidgetCoreList->setLayout(m_pLayoutForCoreList);
        m_pScrollAreaCPUCore->setWidget(m_pWidgetCoreList);
    }

    for (int i = 0; i < m_noOfCores; ++i)
    {
        m_pCheckBoxCore[i].setChecked(true);
    }

    CoreFilter::Iterator it;
    CoreFilter::Iterator itStart = m_displaySettings.m_cpuFilter.begin();
    CoreFilter::Iterator itEnd = m_displaySettings.m_cpuFilter.end();

    for (it = itStart; it != itEnd; ++it)
    {
        m_pCheckBoxCore[(*it)].setChecked(false);
    }

    return retVal;
}

bool DisplayFilterDlg::populateColumnList()
{
    bool retVal = true;

    QObject::disconnect(m_pComboBoxViewes, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onChangeView(const QString&)));
    m_pComboBoxViewes->clear();


    QStringList availableViews = m_displaySettings.getListOfViews(m_pProfileInfo->m_eventVec.size());

    for (int i = 0; i < availableViews.size(); ++i)
    {
        m_pComboBoxViewes->addItem(availableViews.at(i));
    }

    QString viewName = m_displaySettings.m_displayFilterName;
    int index = m_pComboBoxViewes->findText(viewName);

    if (index != -1)  // -1 for not found
    {
        m_pComboBoxViewes->setCurrentIndex(index);
    }

    QObject::connect(m_pComboBoxViewes, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onChangeView(const QString&)));
    onChangeView(m_pComboBoxViewes->currentText());

    return retVal;
}

void DisplayFilterDlg::onChangeView(const QString& newlySelectedView)
{
    (void)(newlySelectedView); // unused

    QString viewName = m_pComboBoxViewes->currentText();
    m_displaySettings.m_displayFilterName = viewName;
    m_displaySettings.calculateDisplayedColumns(m_pProfileReader->getTopologyMap());

    const int noOfColumn = m_displaySettings.m_availableDataColumnCaptions.size();
    m_noOfColumn = noOfColumn;

    if (noOfColumn != 0)
    {
        // Remove old items:
        if (nullptr != m_pVBLayoutForColumn)
        {
            const int noOfColumnOld = m_pVBLayoutForColumn->count();

            for (int i = 0 ; i < noOfColumnOld ; ++i)
            {
                m_pVBLayoutForColumn->removeWidget(m_pCheckBoxColumns + i);
            }

            delete [] m_pCheckBoxColumns;
            delete m_pVBLayoutForColumn;
        }

        m_pVBLayoutForColumn = new QVBoxLayout(m_pScrollAreaColumns);
        m_pWidgetColumnList = new QWidget(m_pScrollAreaColumns);
        RETURN_IF_NULL(m_pVBLayoutForColumn);
        m_pScrollAreaColumns->setFixedHeight(acScalePixelSizeToDisplayDPI(CP_DISPLAY_SETTINGS_COLUMN_AREA_HEIGHT));
        m_pVBLayoutForColumn->setMargin(0);

        // Add new items:
        m_pCheckBoxColumns = new QCheckBox[noOfColumn];

        for (int i = 0 ; i < noOfColumn ; ++i)
        {
            m_pCheckBoxColumns[i].setText(m_displaySettings.m_availableDataColumnCaptions.at(i));
            m_pVBLayoutForColumn->addWidget(m_pCheckBoxColumns + i);
            m_pCheckBoxColumns[i].setChecked(false);
        }

        m_pVBLayoutForColumn->addStretch();

        // Add enable and disable state:
        int noOfColumnDisplayed = (int)m_displaySettings.m_displayedDataColumnsIndices.size();

        for (int i = 0 ; i < noOfColumnDisplayed ; ++i)
        {
            int displayedIndex = m_displaySettings.m_displayedDataColumnsIndices.at(i);

            if ((0 <= displayedIndex) && (displayedIndex < noOfColumn))
            {
                int checkBoxIndex = displayedIndex;
                bool shouldFilter = m_displaySettings.m_filteredDataColumnsCaptions.contains(m_pCheckBoxColumns[checkBoxIndex].text());
                m_pCheckBoxColumns[checkBoxIndex].setChecked(!shouldFilter);
            }
        }

        m_pWidgetColumnList->setLayout(m_pVBLayoutForColumn);
        m_pScrollAreaColumns->setWidget(m_pWidgetColumnList);
    }
}

void DisplayFilterDlg::onClickAllCoreItem(int state)
{
    for (int i = 0 ; i < m_noOfCores ; ++i)
    {
        QObject::disconnect(m_pCheckBoxCore + i, SIGNAL(stateChanged(int)), this, SLOT(onClickCoreItem(int)));
        m_pCheckBoxCore[i].setChecked(state);
        QObject::connect(m_pCheckBoxCore + i, SIGNAL(stateChanged(int)), this, SLOT(onClickCoreItem(int)));
    }
}

void DisplayFilterDlg::onClickCoreItem(int state)
{
    (void)(state); // unused

    bool all = true;

    for (int i = 0 ; i < m_noOfCores ; ++i)
    {
        if (!m_pCheckBoxCore[i].isChecked())
        {
            all = false;
            break;
        }
    }

    QObject::disconnect(m_pCheckBoxAllCore, SIGNAL(stateChanged(int)), this, SLOT(onClickAllCoreItem(int)));
    m_pCheckBoxAllCore->setChecked(all);
    QObject::connect(m_pCheckBoxAllCore, SIGNAL(stateChanged(int)), this, SLOT(onClickAllCoreItem(int)));
}

void DisplayFilterDlg::onClickOk()
{
    // Update data:
    GT_IF_WITH_ASSERT((nullptr != m_pProfileReader) && (nullptr != m_pProfileInfo) && (nullptr != m_pSessionTreeItemData))
    {
        // col
        bool atLeastOneCore = true;

        // CPU:
        CoreFilter tmpCpuFilter;

        for (int i = 0 ; i < m_noOfCores ; ++i)
        {
            if (! m_pCheckBoxCore[i].isChecked())
            {
                tmpCpuFilter.append(i);
            }
        }

        if ((0 < m_noOfCores) && (tmpCpuFilter.size() == m_noOfCores))
        {

            m_pCheckBoxCore[0].setChecked(true);
            tmpCpuFilter.clear();
            atLeastOneCore = false;
        }

        // Update the hidden column list:
        bool atLeastOneColumn = true;
        updateHiddenColumnList();

        if ((0 < m_noOfColumn) && (m_displaySettings.m_filteredDataColumnsCaptions.size() == m_noOfColumn))
        {
            m_pCheckBoxColumns[0].setChecked(true);
            m_displaySettings.m_filteredDataColumnsCaptions.clear();
            atLeastOneColumn = false;
        }

        QString strErrorMessage;

        if (!atLeastOneColumn)
        {
            strErrorMessage += "At least one column must be selected to view...";
        }

        if (!atLeastOneCore)
        {
            if (0 != strErrorMessage.size())
            {
                strErrorMessage += QString("\n");
            }

            strErrorMessage += "At least one core must be selected to view...";
        }

        if ((!atLeastOneCore) || (!atLeastOneColumn))
        {
            acMessageBox::instance().critical("CodeXL Error", strErrorMessage);
            return;
        }

        m_displaySettings.m_cpuFilter = tmpCpuFilter;


        if (m_pCheckBoxSeparateColumnsBy->isChecked())
        {

            if (m_pRadioButtonSeparateByCore->isChecked())
            {
                m_displaySettings.m_separateBy = SEPARATE_BY_CORE;
            }

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
            else if (m_pRadioButtonSeparateByNUMA->isChecked())
            {
                m_displaySettings.m_separateBy = SEPARATE_BY_NUMA;
            }

#endif
        }
        else
        {
            m_displaySettings.m_separateBy = SEPARATE_BY_NONE;
        }

        m_displayPercentageInColumn = m_pCheckBoxShowPercentageBars->isChecked();
        m_displaySystemDLLs = m_pCheckBoxDisplaySystemDLLs->isChecked();

        // Get the display filter configuration:
        m_displaySettings.m_displayFilterName = m_pComboBoxViewes->currentText();
        m_displaySettings.calculateDisplayedColumns(m_pProfileReader->getTopologyMap());
    }

    accept();
}

void DisplayFilterDlg::onClickCancel()
{
    reject();
}

void DisplayFilterDlg::onClickCheckBoxSeparateColumnsBy(int state)
{
    m_pRadioButtonSeparateByCore->setEnabled(state == Qt::Checked);

    if ((!m_pRadioButtonSeparateByCore->isChecked()) &&
        (!m_pRadioButtonSeparateByNUMA->isChecked()))
    {
        m_pRadioButtonSeparateByCore->setChecked(state == Qt::Checked);
    }

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    m_pRadioButtonSeparateByNUMA->setEnabled(state == Qt::Checked);
#endif
}

void DisplayFilterDlg::disableAllControlsExceptSystemDll(bool disable)
{
    GT_IF_WITH_ASSERT((m_pCheckBoxCore != nullptr) && (m_pCheckBoxAllCore != nullptr) && (m_pComboBoxViewes != nullptr) &&
                      (m_pCheckBoxShowPercentageBars != nullptr) && (m_pRadioButtonSeparateByCore != nullptr) && (m_pRadioButtonSeparateByNUMA != nullptr))
    {
        m_pCheckBoxCore->setEnabled(!disable);
        m_pCheckBoxAllCore->setEnabled(!disable);

        for (int i = 0 ; i < m_noOfCores ; ++i)
        {
            m_pCheckBoxCore[i].setEnabled(!disable);
        }

        for (int i = 0 ; i < m_noOfColumn ; ++i)
        {
            m_pCheckBoxColumns[i].setEnabled(!disable);
        }

        m_pComboBoxViewes->setEnabled(!disable);
        m_pCheckBoxShowPercentageBars->setEnabled(!disable);
        m_pCheckBoxSeparateColumnsBy->setEnabled(!disable);

        if (m_pCheckBoxSeparateColumnsBy->isChecked())
        {
            m_pRadioButtonSeparateByCore->setEnabled(!disable);
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
            m_pRadioButtonSeparateByNUMA->setEnabled(!disable);
#endif
        }

        // Notice: Sigal 10/9/2013: We have a bug, we cannot support separate by
        // option in overview window. Therefore, we want to enable the user to change this option
        // whenever it is set.
        if (m_displaySettings.m_separateBy != SEPARATE_BY_NONE)
        {
            m_pCheckBoxSeparateColumnsBy->setEnabled(true);
        }
    }
}

void DisplayFilterDlg::addFinalLayout()
{
    if (nullptr != m_pMainLayout)
    {
        delete m_pMainLayout;
        delete m_pLine;
    }

    m_pMainLayout = new QVBoxLayout;

    m_pMainLayout->setMargin(acScalePixelSizeToDisplayDPI(CP_DISPLAY_SETTINGS_LAYOUT_MARGIN));
    m_pMainLayout->addWidget(m_plabelColumns);

    m_pComboBoxViewesLayout = new QHBoxLayout;

    m_pComboBoxViewesLayout->addSpacing(acScalePixelSizeToDisplayDPI(CP_DISPLAY_SETTINGS_HMARGIN));
    m_pComboBoxViewesLayout->addWidget(m_pComboBoxViewes);
    m_pComboBoxViewesLayout->addSpacing(acScalePixelSizeToDisplayDPI(CP_DISPLAY_SETTINGS_HMARGIN));
    m_pMainLayout->addLayout(m_pComboBoxViewesLayout);

    m_pScrollAreaColumnsLayout = new QHBoxLayout;

    m_pScrollAreaColumnsLayout->addSpacing(acScalePixelSizeToDisplayDPI(CP_DISPLAY_SETTINGS_HMARGIN));
    m_pScrollAreaColumnsLayout->addWidget(m_pScrollAreaColumns);
    m_pScrollAreaColumnsLayout->addSpacing(acScalePixelSizeToDisplayDPI(CP_DISPLAY_SETTINGS_HMARGIN));
    m_pMainLayout->addLayout(m_pScrollAreaColumnsLayout);

    m_pMainLayout->addWidget(m_plabelGeneral);

    m_pCheckBoxDisplaySystemDLLsLayout = new QHBoxLayout;

    m_pCheckBoxDisplaySystemDLLsLayout->addSpacing(acScalePixelSizeToDisplayDPI(CP_DISPLAY_SETTINGS_HMARGIN));
    m_pCheckBoxDisplaySystemDLLsLayout->addWidget(m_pCheckBoxDisplaySystemDLLs);
    m_pCheckBoxDisplaySystemDLLsLayout->addSpacing(acScalePixelSizeToDisplayDPI(CP_DISPLAY_SETTINGS_HMARGIN));
    m_pMainLayout->addLayout(m_pCheckBoxDisplaySystemDLLsLayout);

    m_pCheckBoxShowPercentageBarsLayout = new QHBoxLayout;

    m_pCheckBoxShowPercentageBarsLayout->addSpacing(acScalePixelSizeToDisplayDPI(CP_DISPLAY_SETTINGS_HMARGIN));
    m_pCheckBoxShowPercentageBarsLayout->addWidget(m_pCheckBoxShowPercentageBars);
    m_pCheckBoxShowPercentageBarsLayout->addSpacing(acScalePixelSizeToDisplayDPI(CP_DISPLAY_SETTINGS_HMARGIN));
    m_pMainLayout->addLayout(m_pCheckBoxShowPercentageBarsLayout);

    m_pMainLayout->addWidget(m_plabelCPUCores);

    m_pScrollAreaCPUCoreLayout = new QHBoxLayout;

    m_pScrollAreaCPUCoreLayout->addSpacing(acScalePixelSizeToDisplayDPI(CP_DISPLAY_SETTINGS_HMARGIN));
    m_pScrollAreaCPUCoreLayout->addWidget(m_pScrollAreaCPUCore);
    m_pScrollAreaCPUCoreLayout->addSpacing(acScalePixelSizeToDisplayDPI(CP_DISPLAY_SETTINGS_HMARGIN));
    m_pMainLayout->addLayout(m_pScrollAreaCPUCoreLayout);
    m_pLine = new(nothrow) QFrame(this);

    m_pLine->setFrameShape(QFrame::HLine);
    m_pLine->setFixedHeight(1);
    m_pMainLayout->addWidget(m_pLine);


    m_pCheckBoxSeparateColumnsByLayout = new QHBoxLayout;


    m_pCheckBoxSeparateColumnsByLayout->addSpacing(acScalePixelSizeToDisplayDPI(CP_DISPLAY_SETTINGS_HMARGIN));
    m_pCheckBoxSeparateColumnsByLayout->addWidget(m_pCheckBoxSeparateColumnsBy);

    m_pCheckBoxSeparateColumnsByLayout->addSpacing(acScalePixelSizeToDisplayDPI(CP_DISPLAY_SETTINGS_HMARGIN));
    m_pMainLayout->addLayout(m_pCheckBoxSeparateColumnsByLayout);

    m_pCoreLayout = new QHBoxLayout;
    const int radioButtonIndentation = acScalePixelSizeToDisplayDPI(20) + acScalePixelSizeToDisplayDPI(CP_DISPLAY_SETTINGS_HMARGIN);

    m_pCoreLayout->addSpacing(radioButtonIndentation);
    m_pCoreLayout->addWidget(m_pRadioButtonSeparateByCore);
    m_pMainLayout->addLayout(m_pCoreLayout);


    m_pNUMALayout = new QHBoxLayout;

    m_pNUMALayout->addSpacing(radioButtonIndentation);
    m_pNUMALayout->addWidget(m_pRadioButtonSeparateByNUMA);
    m_pMainLayout->addLayout(m_pNUMALayout);

    m_pMainLayout->addStretch();

    m_pButtonBox = new QHBoxLayout;

    m_pButtonBox->addStretch();
    m_pButtonBox->addWidget(m_pPushButtonOK);
    m_pButtonBox->addWidget(m_pPushButtonCancel);
    m_pMainLayout->addLayout(m_pButtonBox);

    setLayout(m_pMainLayout);
    setFixedSize(acScalePixelSizeToDisplayDPI(CP_DISPLAY_SETTINGS_WIDTH), acScalePixelSizeToDisplayDPI(CP_DISPLAY_SETTINGS_HEIGHT));
}


void DisplayFilterDlg::updateHiddenColumnList()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pCheckBoxSeparateColumnsBy != nullptr) && (m_pRadioButtonSeparateByNUMA != nullptr) && (m_pRadioButtonSeparateByCore != nullptr))
    {
        // Clear the list of displayed column indices:
        m_displaySettings.m_filteredDataColumnsCaptions.clear();

        for (int i = 0 ; i < m_noOfColumn ; ++i)
        {
            bool isChecked = m_pCheckBoxColumns[i].isChecked();

            if (!isChecked)
            {
                m_displaySettings.m_filteredDataColumnsCaptions.push_back(m_pCheckBoxColumns[i].text());
            }
        }
    }
}
