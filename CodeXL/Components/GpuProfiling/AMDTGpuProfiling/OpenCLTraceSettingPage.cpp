//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/OpenCLTraceSettingPage.cpp $
/// \version $Revision: #56 $
/// \brief  This file contains OpenCL Trace settings by the GPU Profiler
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/OpenCLTraceSettingPage.cpp#56 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtXml>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acItemDelegate.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afCSSSettings.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>

// Local:
#include <AMDTGpuProfiling/gpStringConstants.h>
#include <AMDTGpuProfiling/CLAPIFilterManager.h>
#include <AMDTGpuProfiling/OpenCLTraceSettingPage.h>
#include <AMDTGpuProfiling/ProjectSettings.h>
#include <AMDTGpuProfiling/Util.h>
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>
#include <AMDTGpuProfiling/ProfileManager.h>
#include <AMDTGpuProfiling/HSAAPIDefs.h>
#include <AMDTGpuProfiling/CLAPIDefs.h>
#include <AMDTGpuProfiling/CLAPIFilterManager.h>

// avoid conflict between Qt, which #defines "signals" and hsa_ext_amd.h which uses "signals" as a parameter name for a function
#if defined(signals)
    #pragma push_macro("signals")
    #undef signals
    #define NEED_TO_POP_SIGNALS_MACRO
#endif
#include "../HSAFdnCommon/HSAFunctionDefs.h"

#if defined (NEED_TO_POP_SIGNALS_MACRO)
    #pragma pop_macro("signals")
#endif



static const QVector<QString> gs_HSARulesStrings = { GPU_STR_ProjectSettingsRulesWarning1DisplayName, GPU_STR_ProjectSettingsRulesError1DisplayName };
static QStringList gsFullAPIsList;                             ///< Static list holding all the APIs supported

OpenCLTraceOptions* OpenCLTraceOptions::m_spInstance = nullptr;

OpenCLTraceOptions::OpenCLTraceOptions(QWidget* /*parent*/): afProjectSettingsExtension(),
    m_pOpenCLRadioButton(nullptr),
    m_pHSARadioButton(nullptr),
    m_pShowAPIErrorCodeCB(nullptr),
    m_pCollapseCallsCB(nullptr),
    m_pEnableNavigationCB(nullptr),
    m_pAPIRulesTW(nullptr),
    m_pAPIsToTraceTW(nullptr),
    m_pAPIsToTraceRB(nullptr),
    m_pGenerateSummaryPageRB(nullptr),
    m_pMaxNumberOfAPIsLB(nullptr),
    m_pMaxNumberOfAPIsSB(nullptr),
    m_pWriteDataTimeOutCB(nullptr),
    m_pTimeOutIntervalSB(nullptr),
    m_pGenerateOccupancyCB(nullptr),
    m_pOpenCLRootItem(nullptr),
    m_pHSARootItem(nullptr)
{
    if (gsFullAPIsList.isEmpty())
    {
        CLAPIDefs::Instance()->AppendAllCLFunctionsToList(gsFullAPIsList);
        HSAAPIDefs::Instance()->AppendAllHSAFunctionsToList(gsFullAPIsList);
    }
}

OpenCLTraceOptions* OpenCLTraceOptions::Instance()
{
    if (m_spInstance == nullptr)
    {
        m_spInstance = new OpenCLTraceOptions;
    }

    return m_spInstance;
}

OpenCLTraceOptions::~OpenCLTraceOptions()
{
}

void OpenCLTraceOptions::Initialize()
{
    // Create the captions for the page:
    QLabel* pCaption1 = new QLabel(GP_Str_ProjectSettingsMainCaption);
    QLabel* pCaption2 = new QLabel(GP_Str_ProjectSettingsOpenCLTraceOptions);
    QLabel* pCaption3 = new QLabel(GP_Str_ProjectSettingsAPITraceOptions);

    pCaption1->setStyleSheet(AF_STR_captionLabelStyleSheetMain);
    pCaption2->setStyleSheet(AF_STR_captionLabelStyleSheetMain);
    pCaption3->setStyleSheet(AF_STR_captionLabelStyleSheetMain);


    QLabel* pAPITypeLabel    = new QLabel(GP_Str_ProjectSettingsAPITypeDesc);
    m_pOpenCLRadioButton     = new QRadioButton(GP_Str_ProjectSettingsOpenCLAPI);
    m_pHSARadioButton      = new QRadioButton(GP_Str_ProjectSettingsHSAAPI);
    m_pOpenCLRadioButton->setToolTip(GP_Str_ProjectSettingsOpenCLAPITooltip);
    m_pHSARadioButton->setToolTip(GP_Str_ProjectSettingsHSAAPITooltip);

    QButtonGroup* pGroup     = new QButtonGroup;
    pGroup->addButton(m_pOpenCLRadioButton);
    pGroup->addButton(m_pHSARadioButton);

    m_pAPIsToTraceRB = new QRadioButton(GP_Str_AppTraceAPIToTrace);
    m_pCollapseCallsCB       = new QCheckBox(tr(GP_Str_AppTraceCollpase));
    m_pEnableNavigationCB    = new QCheckBox(tr(GP_Str_AppTraceEnableNavigation));
    m_pShowAPIErrorCodeCB    = new QCheckBox(tr(GP_Str_AppTraceAlwaysShowAPIErrorCodes));
    m_pGenerateSummaryPageRB = new QRadioButton(tr(GP_Str_AppTraceGenerateSummary));
    m_pGenerateOccupancyCB   = new QCheckBox(tr(GP_Str_CounterSelectionGenerateOccupancyDetails));

    m_pAPIRulesTW = new QTreeWidget();
    m_pAPIRulesTW->setColumnCount(1);
    m_pAPIRulesTW->setHeaderHidden(true);
    m_pAPIRulesTW->setItemDelegate(new acItemDelegate);
    m_pAPIRulesTW->setStyleSheet(AF_STR_treeWidgetWithBorderStyleSheet);

    m_pAPIsToTraceTW = new QTreeWidget();
    m_pAPIsToTraceTW->setColumnCount(1);
    m_pAPIsToTraceTW->setHeaderHidden(true);
    m_pAPIsToTraceTW->setDisabled(true);
    m_pAPIsToTraceTW->setItemDelegate(new acItemDelegate);
    m_pAPIsToTraceTW->setStyleSheet(AF_STR_treeWidgetWithBorderStyleSheet);

    m_pMaxNumberOfAPIsLB = new QLabel(GP_Str_AppTraceMaxAPI);
    m_pMaxNumberOfAPIsSB = new QSpinBox();

    m_pMaxNumberOfAPIsSB->setRange(1, 10000000);

    QHBoxLayout* pMaxAPIsLayout = new QHBoxLayout();

    pMaxAPIsLayout->addWidget(m_pMaxNumberOfAPIsLB);
    pMaxAPIsLayout->addWidget(m_pMaxNumberOfAPIsSB);
    pMaxAPIsLayout->addStretch();

    m_pWriteDataTimeOutCB = new QCheckBox(tr(GP_Str_AppTraceWriteTraceData));

    m_pTimeoutIntervalLB = new QLabel(tr(GP_Str_AppTraceTraceDataInterval));

    m_pTimeOutIntervalSB = new QSpinBox();

    m_pTimeOutIntervalSB->setRange(0, 10000000);

    QHBoxLayout* pTimeOutLayout = new QHBoxLayout();


    // disabling timeout mode option not supported on Linux currently (Linux always uses timeout)
    // you can only change the timeout interval
    // so on Linux we add a label for the spinbox, rather than a checkbox
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    pTimeOutLayout->addWidget(m_pWriteDataTimeOutCB);
#elif (AMDT_BUILD_TARGET == AMDT_LINUX_OS)
    pTimeOutLayout->addWidget(m_pTimeoutIntervalLB);
#endif
    pTimeOutLayout->addWidget(m_pTimeOutIntervalSB);
    pTimeOutLayout->addStretch();

    QGridLayout* pGridLayout = new QGridLayout();

    pGridLayout->addWidget(m_pGenerateSummaryPageRB, 0, 0);
    pGridLayout->addWidget(m_pAPIRulesTW, 1, 0);
    pGridLayout->addWidget(m_pAPIsToTraceRB, 0, 1);
    pGridLayout->addWidget(m_pAPIsToTraceTW, 1, 1);

    QVBoxLayout* pMainLayout = new QVBoxLayout();

    pMainLayout->addWidget(pCaption1);
    pMainLayout->addWidget(pAPITypeLabel);
    pMainLayout->addWidget(m_pOpenCLRadioButton);
    pMainLayout->addWidget(m_pHSARadioButton);
    pMainLayout->addWidget(m_pEnableNavigationCB);
    pMainLayout->addLayout(pTimeOutLayout);
    pMainLayout->addLayout(pMaxAPIsLayout);

    pMainLayout->addWidget(pCaption2);
    pMainLayout->addWidget(m_pShowAPIErrorCodeCB);
    pMainLayout->addWidget(m_pCollapseCallsCB);
    pMainLayout->addWidget(m_pGenerateOccupancyCB);

    pMainLayout->addWidget(pCaption3);
    pMainLayout->addLayout(pGridLayout);

    setLayout(pMainLayout);

    // Initialize components
    m_pCollapseCallsCB->setChecked(true);
    m_pGenerateSummaryPageRB->setChecked(true);
    m_pGenerateOccupancyCB->setChecked(true);
    m_pMaxNumberOfAPIsSB->setValue(DEFAULT_NUM_OF_API_CALLS_TO_TRACE);

    InitializeRulesTreeView();
    InitializeAPIFilterTreeView();

    bool rc = connect(m_pAPIRulesTW, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(RulesTreeItemChanged(QTreeWidgetItem*, int)));
    GT_ASSERT(rc);

    rc = connect(m_pAPIsToTraceTW, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(APIsTreeItemChanged(QTreeWidgetItem*, int)));
    GT_ASSERT(rc);

    rc = connect(m_pGenerateSummaryPageRB, SIGNAL(toggled(bool)), this, SLOT(RadioButtonToggled(bool)));
    GT_ASSERT(rc);

    rc = connect(m_pAPIsToTraceRB, SIGNAL(toggled(bool)), this, SLOT(RadioButtonToggled(bool)));
    GT_ASSERT(rc);

    rc = connect(m_pWriteDataTimeOutCB, SIGNAL(stateChanged(int)), this, SLOT(WriteTimeOutStateChanged(int)));
    GT_ASSERT(rc);

    rc = connect(m_pOpenCLRadioButton, SIGNAL(clicked()), this, SLOT(OnAPITypeRadioButtonToggled()));
    GT_ASSERT(rc);

    rc = connect(m_pHSARadioButton, SIGNAL(clicked()), this, SLOT(OnAPITypeRadioButtonToggled()));
    GT_ASSERT(rc);

    RestoreCurrentSettings();
}

gtString OpenCLTraceOptions::ExtensionXMLString()
{
    return GPU_STR_TRACE_PROJECT_SETTINGS;
}

gtString OpenCLTraceOptions::ExtensionTreePathAsString()
{
    return GPU_STR_APP_TRACE_PROJECT_TREE_PATH_STR;
}

bool OpenCLTraceOptions::GetXMLSettingsString(gtString& projectAsXMLString)
{
    bool retVal = false;
    gtString pageName = GPU_STR_TRACE_PROJECT_SETTINGS;

    retVal = getProjectSettingsXML(projectAsXMLString, pageName);
    return retVal;
}

bool OpenCLTraceOptions::getProjectSettingsXML(gtString& projectAsXMLString, gtString& projectPage)
{
    projectAsXMLString.appendFormattedString(L"<%ls>", projectPage.asCharArray());
    writeSession(projectAsXMLString, L"Current");
    projectAsXMLString.appendFormattedString(L"</%ls>", projectPage.asCharArray());

    return true;
}

void OpenCLTraceOptions::writeSession(gtString& projectAsXMLString, const gtString& type)
{
    gtString numVal;

    projectAsXMLString.append(L"<Session type=\"");
    projectAsXMLString.append(type);
    projectAsXMLString.append(L"\">");

    gtString apiTypeStr = (m_currentSettings.m_apiToTrace == APIToTrace_OPENCL) ? GPU_STR_ProjectSettingsAPITypeOpenCL : GPU_STR_ProjectSettingsAPITypeHSA;
    writeValue(projectAsXMLString, GPU_STR_ProjectSettingsAPIType, apiTypeStr);
    writeBool(projectAsXMLString, GPU_STR_ProjectSettingsGenerateOccupancy, m_currentSettings.m_generateKernelOccupancy);
    writeBool(projectAsXMLString, GPU_STR_ProjectSettingsShowErrorCode, m_currentSettings.m_alwaysShowAPIErrorCode);
    writeBool(projectAsXMLString, GPU_STR_ProjectSettingsCollapseClGetEventInfo, m_currentSettings.m_collapseClGetEventInfo);
    writeBool(projectAsXMLString, GPU_STR_ProjectSettingsEnableNavigation, m_currentSettings.m_generateSymInfo);
    writeBool(projectAsXMLString, GPU_STR_ProjectSettingsGenerateSummaryPage, m_currentSettings.m_generateSummaryPage);
    writeBool(projectAsXMLString, GPU_STR_ProjectSettingsAPIsToFilter, m_currentSettings.m_filterAPIsToTrace);
    numVal.makeEmpty();
    numVal.appendFormattedString(L"%d", m_currentSettings.m_maxAPICalls);
    writeValue(projectAsXMLString, GPU_STR_ProjectSettingsMaxAPIs, numVal);

    writeBool(projectAsXMLString, GPU_STR_ProjectSettingsWriteDataTimeOut, m_currentSettings.m_writeDataTimeOut);
    numVal.makeEmpty();
    numVal.appendFormattedString(L"%d", m_currentSettings.m_timeoutInterval);
    writeValue(projectAsXMLString, GPU_STR_ProjectSettingsTimeOutInterval, numVal);

    QStringList rulesList;
    m_currentSettings.GetListOfRules(rulesList);
    AppendTree(projectAsXMLString, acGTStringToQString(GPU_STR_ProjectSettingsRulesTree), rulesList);
    AppendTree(projectAsXMLString, acGTStringToQString(GPU_STR_ProjectSettingsAPIsFilterTree), m_currentSettings.m_pFilterManager->APIFilterSet());

    projectAsXMLString.append(L"</Session>");
}

bool OpenCLTraceOptions::SetSettingsFromXMLString(const gtString& projectAsXMLString)
{
    bool retVal = setProjectSettingsXML(projectAsXMLString);
    RestoreCurrentSettings();
    return retVal;
}

bool OpenCLTraceOptions::setProjectSettingsXML(const gtString& projectAsXMLString)
{
    QString qtStr = acGTStringToQString(projectAsXMLString);

    QDomDocument doc;
    doc.setContent(qtStr.toUtf8());

    QDomElement rootElement = doc.documentElement();
    QDomNode rootNode = rootElement.firstChild();
    QDomNode childNode = rootNode.firstChild();

    QString nodeVal;
    bool val;

    while (!childNode.isNull())
    {
        val = false;
        nodeVal = childNode.firstChild().nodeValue();

        if (nodeVal == "T")
        {
            val = true;
        }

        if (childNode.nodeName() == acGTStringToQString(GPU_STR_ProjectSettingsGenerateOccupancy))
        {
            m_currentSettings.m_generateKernelOccupancy = val;
        }
        else if (childNode.nodeName() == acGTStringToQString(GPU_STR_ProjectSettingsShowErrorCode))
        {
            m_currentSettings.m_alwaysShowAPIErrorCode = val;
        }
        else if (childNode.nodeName() == acGTStringToQString(GPU_STR_ProjectSettingsCollapseClGetEventInfo))
        {
            m_currentSettings.m_collapseClGetEventInfo = val;
        }
        else if (childNode.nodeName() == acGTStringToQString(GPU_STR_ProjectSettingsEnableNavigation))
        {
            m_currentSettings.m_generateSymInfo = val;
        }
        else if (childNode.nodeName() == acGTStringToQString(GPU_STR_ProjectSettingsGenerateSummaryPage))
        {
            m_currentSettings.m_generateSummaryPage = val;
        }
        else if (childNode.nodeName() == acGTStringToQString(GPU_STR_ProjectSettingsAPIsToFilter))
        {
            m_currentSettings.m_filterAPIsToTrace = val;
        }
        else if (childNode.nodeName() == acGTStringToQString(GPU_STR_ProjectSettingsMaxAPIs))
        {
            m_currentSettings.m_maxAPICalls = nodeVal.toInt();
        }
        else if (childNode.nodeName() == acGTStringToQString(GPU_STR_ProjectSettingsWriteDataTimeOut))
        {
            m_currentSettings.m_writeDataTimeOut = val;
        }
        else if (childNode.nodeName() == acGTStringToQString(GPU_STR_ProjectSettingsTimeOutInterval))
        {
            m_currentSettings.m_timeoutInterval = nodeVal.toInt();
        }
        else if (childNode.nodeName() == acGTStringToQString(GPU_STR_ProjectSettingsAPIType))
        {
            if (nodeVal == acGTStringToQString(GPU_STR_ProjectSettingsAPITypeOpenCL))
            {
                m_currentSettings.m_apiToTrace = APIToTrace_OPENCL;
            }
            else if (nodeVal == acGTStringToQString(GPU_STR_ProjectSettingsAPITypeHSA))
            {
                m_currentSettings.m_apiToTrace = APIToTrace_HSA;
            }
            else
            {
                m_currentSettings.m_apiToTrace = APIToTrace_OPENCL;
                GT_ASSERT_EX(false, L"Invalid project settings option");
            }
        }
        else if (childNode.nodeName() == acGTStringToQString(GPU_STR_ProjectSettingsRulesTree))
        {
            if (childNode.hasChildNodes())
            {
                UpdateTreeWidgetFromXML(childNode, m_pAPIRulesTW, false);
            }
            else
            {
                if (m_pAPIRulesTW != nullptr)
                {
                    Util::SetCheckState(m_pAPIRulesTW, true);
                }
            }

            UpdateRuleList();
        }
        else if (childNode.nodeName() == acGTStringToQString(GPU_STR_ProjectSettingsAPIsFilterTree))
        {
            if (childNode.hasChildNodes())
            {
                UpdateTreeWidgetFromXML(childNode, m_pAPIsToTraceTW, true);
            }
            else
            {
                if (m_pAPIsToTraceTW != nullptr)
                {
                    Util::SetCheckState(m_pAPIsToTraceTW, true);
                }
            }

            UpdateAPIFilterList();
        }

        childNode = childNode.nextSibling();
    }

    return true;
}

void OpenCLTraceOptions::RestoreDefaultProjectSettings()
{
    m_currentSettings.RestoreDefault();
    m_currentSettings.m_pFilterManager->SetAPIFilterSet(gsFullAPIsList);

    RestoreCurrentSettings();

    // reset all items
    if (m_pAPIRulesTW != nullptr)
    {
        Util::SetCheckState(m_pAPIRulesTW, true);
    }

    if (m_pAPIsToTraceTW != nullptr)
    {
        Util::SetCheckState(m_pAPIsToTraceTW, true);
    }
}

bool OpenCLTraceOptions::RestoreCurrentSettings()
{
    m_pOpenCLRadioButton->setChecked(m_currentSettings.m_apiToTrace == APIToTrace_OPENCL);
    m_pHSARadioButton->setChecked(m_currentSettings.m_apiToTrace == APIToTrace_HSA);
    m_pShowAPIErrorCodeCB->setChecked(m_currentSettings.m_alwaysShowAPIErrorCode);
    m_pCollapseCallsCB->setChecked(m_currentSettings.m_collapseClGetEventInfo);
    m_pEnableNavigationCB->setChecked(m_currentSettings.m_generateSymInfo);
    m_pGenerateOccupancyCB->setChecked(m_currentSettings.m_generateKernelOccupancy);
    m_pGenerateSummaryPageRB->setChecked(m_currentSettings.m_generateSummaryPage);
    m_pAPIsToTraceRB->setChecked(m_currentSettings.m_filterAPIsToTrace);
    m_pMaxNumberOfAPIsSB->setValue(m_currentSettings.m_maxAPICalls);

    m_pTimeOutIntervalSB->setValue(m_currentSettings.m_timeoutInterval);
    const bool isHsaEnabled = Util::IsHSAEnabled();
    m_pHSARadioButton->setEnabled(isHsaEnabled);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    m_pWriteDataTimeOutCB->setChecked(m_currentSettings.m_writeDataTimeOut);
    m_pTimeOutIntervalSB->setEnabled(m_currentSettings.m_writeDataTimeOut);
#elif (AMDT_BUILD_TARGET == AMDT_LINUX_OS)
    m_pWriteDataTimeOutCB->setChecked(true);

    // Check if the catalyst and HSA are installed, and enable / check the OpenCL / HSA button accordingly:
    bool isCatalystInstalled = (afGlobalVariablesManager::instance().InstalledAMDComponentsBitmask() & AF_AMD_CATALYST_COMPONENT);
    m_pOpenCLRadioButton->setEnabled(isCatalystInstalled);

    if (!isCatalystInstalled)
    {
        m_pHSARadioButton->setChecked(true);
    }

#endif

    OnAPITypeRadioButtonToggled();

    if (m_pGenerateSummaryPageRB->isChecked())
    {
        m_pAPIRulesTW->setEnabled(true);
        m_pAPIsToTraceTW->setEnabled(false);
    }
    else if (m_pAPIsToTraceRB->isChecked())
    {
        m_pAPIRulesTW->setEnabled(false);
        m_pAPIsToTraceTW->setEnabled(true);
    }

    // restore tree lists
    RestoreTreesDataFromSettings();

    return true;
}

bool OpenCLTraceOptions::AreSettingsValid(gtString& invalidMessageStr)
{
    if (m_pAPIsToTraceRB->isChecked() &&
        (Util::ItemsSelectedInTreeWidget(m_pAPIsToTraceTW) == 0))
    {
        invalidMessageStr = L"At least one item under \"APIs to trace\" must be selected.";
        return false;
    }

    SaveCurrentSettings();
    return true;
}

bool OpenCLTraceOptions::SetTraceOptions(APITraceOptions& apiTraceOptions)
{
    apiTraceOptions.m_alwaysShowAPIErrorCode = m_pShowAPIErrorCodeCB->isChecked();
    apiTraceOptions.m_collapseClGetEventInfo = m_pCollapseCallsCB->isChecked();
    apiTraceOptions.m_generateKernelOccupancy = m_pGenerateOccupancyCB->isChecked();
    apiTraceOptions.m_generateSummaryPage = m_pGenerateSummaryPageRB->isChecked();
    apiTraceOptions.m_generateSymInfo = m_pEnableNavigationCB->isChecked();
    apiTraceOptions.m_maxAPICalls = m_pMaxNumberOfAPIsSB->value();
    apiTraceOptions.m_filterAPIsToTrace = m_pAPIsToTraceRB->isChecked();
    apiTraceOptions.m_apiToTrace = m_pHSARadioButton->isChecked() ? APIToTrace_HSA : APIToTrace_OPENCL;
    UpdateAPIFilterList();

    return true;
}

bool OpenCLTraceOptions::SaveCurrentSettings()
{
    const bool isHsaEnabled = Util::IsHSAEnabled();
    m_pHSARadioButton->setEnabled(isHsaEnabled);

    m_currentSettings.m_apiToTrace = m_pOpenCLRadioButton->isChecked() ? APIToTrace_OPENCL : APIToTrace_HSA;
    m_currentSettings.m_alwaysShowAPIErrorCode = m_pShowAPIErrorCodeCB->isChecked();
    m_currentSettings.m_collapseClGetEventInfo = m_pCollapseCallsCB->isChecked();
    m_currentSettings.m_generateSymInfo = m_pEnableNavigationCB->isChecked();
    m_currentSettings.m_generateKernelOccupancy = m_pGenerateOccupancyCB->isChecked();
    m_currentSettings.m_generateSummaryPage = m_pGenerateSummaryPageRB->isChecked();
    m_currentSettings.m_filterAPIsToTrace = m_pAPIsToTraceRB->isChecked();
    m_currentSettings.m_maxAPICalls = m_pMaxNumberOfAPIsSB->value();
    m_currentSettings.m_writeDataTimeOut = m_pWriteDataTimeOutCB->isChecked();
    m_currentSettings.m_timeoutInterval = m_pTimeOutIntervalSB->value();

    UpdateRuleList();
    UpdateAPIFilterList();
    UpdateProjectSettings();

    return true;
}

void OpenCLTraceOptions::UpdateRuleList()
{
    QTreeWidgetItemIterator iterator(m_pAPIRulesTW);

    // Set all rules to disabled:
    m_currentSettings.ResetRules(false);

    while ((*iterator) != nullptr)
    {
        if ((*iterator)->childCount() == 0)
        {
            if ((*iterator)->checkState(0) == Qt::Checked)
            {
                m_currentSettings.EnableRule((*iterator)->text(0).trimmed(), true);
            }
        }

        ++iterator;
    }
}

void OpenCLTraceOptions::RestoreTreesDataFromSettings()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pAPIRulesTW != nullptr)
    {
        // Restore rules tree from settings:
        QTreeWidgetItemIterator iter(m_pAPIRulesTW);

        // First uncheck all:
        while ((*iter) != nullptr)
        {
            (*iter)->setCheckState(0, Qt::Unchecked);
            ++iter;
        }

        QTreeWidgetItemIterator iterator(m_pAPIRulesTW);

        // check all items saved in list
        while ((*iterator) != nullptr)
        {
            if ((*iterator)->childCount() == 0)
            {
                bool shouldCheck = m_currentSettings.IsRuleEnabled((*iterator)->text(0));

                if (shouldCheck)
                {
                    (*iterator)->setCheckState(0, Qt::Checked);
                }
            }

            ++iterator;
        }

        // fix tree items top levels state
        UpdateTreeCheckState(m_pAPIRulesTW);
    }

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pAPIsToTraceTW != nullptr)
    {
        // Restore API tree from settings:
        QTreeWidgetItemIterator iter(m_pAPIsToTraceTW);

        // First uncheck all:
        while ((*iter) != nullptr)
        {
            (*iter)->setCheckState(0, Qt::Unchecked);
            ++iter;
        }

        QTreeWidgetItemIterator iterator(m_pAPIsToTraceTW);

        // check all items saved in list
        while ((*iterator) != nullptr)
        {
            if ((*iterator)->childCount() == 0)
            {
                bool shouldCheck = m_currentSettings.m_pFilterManager->IsEnabled((*iterator)->text(0));

                if (shouldCheck)
                {
                    (*iterator)->setCheckState(0, Qt::Checked);
                }
            }

            ++iterator;
        }

        // fix tree items top levels state
        UpdateTreeCheckState(m_pAPIsToTraceTW);
    }
}

void OpenCLTraceOptions::UpdateTreeCheckState(QTreeWidget* tree)
{
    QTreeWidgetItem* item;

    // for all top level items in tree
    for (int i = 0; i < tree->topLevelItemCount(); i++)
    {
        // fix check state by calculating children state
        item = tree->topLevelItem(i);

        if (nullptr != item)
        {
            item->setCheckState(0, GetSubTreeCheckState(item));
        }
    }
}

Qt::CheckState OpenCLTraceOptions::GetSubTreeCheckState(QTreeWidgetItem* parent)
{
    Qt::CheckState retVal = Qt::Unchecked;

    GT_IF_WITH_ASSERT(nullptr != parent)
    {
        int checkedNum = 0;

        // stop condition - leaf item
        if (parent->childCount() == 0)
        {
            retVal = parent->checkState(0);
        }
        else
        {
            // for all children - get state
            for (int i = 0; i < parent->childCount(); i++)
            {
                if (GetSubTreeCheckState(parent->child(i)) == Qt::Checked)
                {
                    checkedNum++;
                }
            }

            // if all children are checked - set checked
            if (checkedNum == parent->childCount())
            {
                retVal = Qt::Checked;
            }
            // else - if one or more children checked - set PartiallyChecked
            else if (checkedNum > 0)
            {
                retVal = Qt::PartiallyChecked;
            }

            //else set unchecked
        }
    }

    return retVal;
}

void OpenCLTraceOptions::UpdateAPIFilterList()
{

    // Get the current project settings:
    ProjectSettings* pCurrProjectSetting = ProfileManager::Instance()->GetCurrentProjectSettings();

    if (nullptr != pCurrProjectSetting)
    {

        // Sanity check:
        GT_IF_WITH_ASSERT(m_pHSARadioButton != nullptr)
        {
            QStringList filterList;

            // Go over the tree and build a list of all the checked API functions:
            QTreeWidgetItemIterator iterator(m_pAPIsToTraceTW);
            QStringList apisList;

            // Check if this is an HSA / CL:
            bool isHSA = m_pHSARadioButton->isChecked();

            while ((*iterator) != nullptr)
            {
                if ((*iterator)->childCount() == 0)
                {
                    if ((*iterator)->checkState(0) == Qt::Unchecked)
                    {
                        // Check if the current function should be added (checked + belongs to the right API):
                        bool shouldAdd = false;

                        if (isHSA)
                        {
                            HSA_API_Type hsaType = HSAAPIDefs::Instance()->ToHSAAPIType((*iterator)->text(0).trimmed());
                            shouldAdd = (hsaType != HSA_API_Type_UNKNOWN);
                        }
                        else
                        {
                            CL_FUNC_TYPE clType = CLAPIDefs::Instance()->ToCLAPIType((*iterator)->text(0).trimmed());
                            shouldAdd = (clType != CL_FUNC_TYPE_Unknown);
                        }

                        if (shouldAdd)
                        {
                            apisList << (*iterator)->text(0).trimmed();
                        }
                    }
                }

                ++iterator;
            }

            pCurrProjectSetting->m_traceOptions.m_pFilterManager->Save(apisList, pCurrProjectSetting->m_traceOptions.m_apiToTrace);

        }
    }
}

void OpenCLTraceOptions::UpdateProjectSettings()
{
    if (ProfileManager::Instance()->GetCurrentProjectSettings())
    {
        ProfileManager::Instance()->GetCurrentProjectSettings()->m_traceOptions.m_maxAPICalls = m_currentSettings.m_maxAPICalls;
        ProfileManager::Instance()->GetCurrentProjectSettings()->m_traceOptions.m_alwaysShowAPIErrorCode = m_currentSettings.m_alwaysShowAPIErrorCode;
        ProfileManager::Instance()->GetCurrentProjectSettings()->m_traceOptions.m_collapseClGetEventInfo = m_currentSettings.m_collapseClGetEventInfo;
        ProfileManager::Instance()->GetCurrentProjectSettings()->m_traceOptions.m_generateKernelOccupancy = m_currentSettings.m_generateKernelOccupancy;
        ProfileManager::Instance()->GetCurrentProjectSettings()->m_traceOptions.m_generateSummaryPage = m_currentSettings.m_generateSummaryPage;
        ProfileManager::Instance()->GetCurrentProjectSettings()->m_traceOptions.m_generateSymInfo = m_currentSettings.m_generateSymInfo;
        ProfileManager::Instance()->GetCurrentProjectSettings()->m_traceOptions.m_apiToTrace = m_currentSettings.m_apiToTrace;

        ProfileManager::Instance()->GetCurrentProjectSettings()->m_traceOptions.m_mode = (m_currentSettings.m_writeDataTimeOut == true) ? TIMEOUT : NORMAL;
        ProfileManager::Instance()->GetCurrentProjectSettings()->m_traceOptions.m_timeoutInterval = m_currentSettings.m_timeoutInterval;
    }
}

bool OpenCLTraceOptions::GenerateRules(const QString& ruleFile)
{
    try
    {
        QFile file(ruleFile);

        if (file.open(QIODevice::WriteOnly))
        {
            QTreeWidgetItem* item;
            QTextStream write(&file);
            QString state;
            item = Util::FindTreeItem(m_pAPIRulesTW, "Detect resource leaks");

            if (item != nullptr)
            {
                state = item->checkState(0) == Qt::Checked ? Util::ms_TRUESTR : Util::ms_FALSESTR;

                if (m_currentSettings.m_apiToTrace == APIToTrace_OPENCL)
                {
                    write << "APITrace.APIRules.RefTracker=" << state;
                }
                else
                {
                    write << "APITrace.APIRules.HSARefTracker=" << state;
                }

                state.clear();
            }

            item = Util::FindTreeItem(m_pAPIRulesTW, "Detect deprecated API calls");

            if (item != nullptr)
            {
                state = item->checkState(0) == Qt::Checked ? Util::ms_TRUESTR : Util::ms_FALSESTR;
                write << "\nAPITrace.APIRules.DeprecatedFunctionAnalyzer=" << state;
                state.clear();
            }

            item = Util::FindTreeItem(m_pAPIRulesTW, "Detect unnecessary blocking writes");

            if (item != nullptr)
            {
                state = item->checkState(0) == Qt::Checked ? Util::ms_TRUESTR : Util::ms_FALSESTR;
                write << "\nAPITrace.APIRules.BlockingWrite=" << state;
                state.clear();
            }

            item = Util::FindTreeItem(m_pAPIRulesTW, "Detect non-optimized work size");

            if (item != nullptr)
            {
                state = item->checkState(0) == Qt::Checked ? Util::ms_TRUESTR : Util::ms_FALSESTR;
                write << "\nAPITrace.APIRules.BadWorkGroupSize=" << state;
                state.clear();
            }

            item = Util::FindTreeItem(m_pAPIRulesTW, "Detect non-optimized data transfer");

            if (item != nullptr)
            {
                state = item->checkState(0) == Qt::Checked ? Util::ms_TRUESTR : Util::ms_FALSESTR;
                write << "\nAPITrace.APIRules.DataTransferAnalyzer=" << state;
                state.clear();
            }

            item = Util::FindTreeItem(m_pAPIRulesTW, "Detect redundant synchronization");

            if (item != nullptr)
            {
                state = item->checkState(0) == Qt::Checked ? Util::ms_TRUESTR : Util::ms_FALSESTR;
                write << "\nAPITrace.APIRules.SyncAnalyzer=" << state;
                state.clear();
            }

            item = Util::FindTreeItem(m_pAPIRulesTW, "Detect failed API calls");

            if (item != nullptr)
            {
                state = item->checkState(0) == Qt::Checked ? Util::ms_TRUESTR : Util::ms_FALSESTR;

                if (m_currentSettings.m_apiToTrace == APIToTrace_OPENCL)
                {
                    write << "\nAPITrace.APIRules.RetCodeAnalyzer=" << state;
                }
                else
                {
                    write << "\nAPITrace.APIRules.HSARetCodeAnalyzer=" << state;
                }

                state.clear();
            }

            file.close();
        }
    }
    catch (...)
    {
        return false;
    }

    return true;
}

void OpenCLTraceOptions::InitializeRulesTreeView()
{
    if (!m_pAPIRulesTW)
    {
        return;
    }

    QTreeWidgetItem* pCurrentRuleGroup = nullptr;

    QTreeWidgetItem* pRootItem = new QTreeWidgetItem();

    pRootItem->setFlags(pRootItem->flags() | Qt::ItemIsUserCheckable);
    pRootItem->setText(0, GP_Str_ProjectSettingsAllCLRulesItemName);
    pRootItem->setCheckState(0, Qt::Checked);

    m_pAPIRulesTW->clear();
    m_pAPIRulesTW->addTopLevelItem(pRootItem);
    m_pAPIRulesTW->expandAll();

    if (!ProfileManager::Instance()->GetCurrentProjectSettings())
    {
        return;
    }

    QMap<CLAPIRule*, bool>::const_iterator iter = ProfileManager::Instance()->GetCurrentProjectSettings()->m_traceOptions.m_rules.constBegin();

    for (; iter != ProfileManager::Instance()->GetCurrentProjectSettings()->m_traceOptions.m_rules.constEnd(); iter++)
    {
        CLAPIRule* pRule = iter.key();

        if (pRule != nullptr)
        {
            QString type = CLAPIRule::GetRuleTypeString(pRule->GetType());
            pCurrentRuleGroup = Util::FindTreeItem(m_pAPIRulesTW, type);

            if (!pCurrentRuleGroup)
            {
                QTreeWidgetItem* pNewRuleGroup = new QTreeWidgetItem();

                pNewRuleGroup->setFlags(pNewRuleGroup->flags() | Qt::ItemIsUserCheckable);
                pNewRuleGroup->setText(0, type);
                pNewRuleGroup->setCheckState(0, Qt::Checked);
                pRootItem->addChild(pNewRuleGroup);
                pCurrentRuleGroup = pNewRuleGroup;
            }

            QTreeWidgetItem* pNewRuleTreeItem = new QTreeWidgetItem();

            pNewRuleTreeItem->setFlags(pNewRuleTreeItem->flags() | Qt::ItemIsUserCheckable);
            pNewRuleTreeItem->setText(0, pRule->m_displayName);
            pNewRuleTreeItem->setToolTip(0, pRule->m_displayName);
            pNewRuleTreeItem->setCheckState(0, Qt::Checked);
            pCurrentRuleGroup->addChild(pNewRuleTreeItem);

            if (gs_HSARulesStrings.contains(pRule->m_displayName))
            {
                m_hsaRulesTreeWidgetItems << pNewRuleTreeItem;

                if (!m_hsaRulesTreeWidgetItems.contains(pCurrentRuleGroup))
                {
                    m_hsaRulesTreeWidgetItems << pCurrentRuleGroup;
                }
            }
        }
    }
}

void OpenCLTraceOptions::InitializeAPIFilterTreeView()
{
    if (nullptr == m_pAPIsToTraceTW)
    {
        return;
    }

    // Create 2 top level items, one for each API type:
    m_pOpenCLRootItem = new QTreeWidgetItem;
    m_pHSARootItem = new QTreeWidgetItem;

    m_pOpenCLRootItem->setFlags(m_pOpenCLRootItem->flags() | Qt::ItemIsUserCheckable);
    m_pOpenCLRootItem->setText(0, GP_Str_ProjectSettingsAllCLAPIsItemName);
    m_pOpenCLRootItem->setCheckState(0, Qt::Checked);

    m_pHSARootItem->setFlags(m_pHSARootItem->flags() | Qt::ItemIsUserCheckable);
    m_pHSARootItem->setText(0, GP_Str_ProjectSettingsAllHSAAPIsItemName);
    m_pHSARootItem->setCheckState(0, Qt::Checked);

    m_pAPIsToTraceTW->clear();
    m_pAPIsToTraceTW->addTopLevelItem(m_pOpenCLRootItem);
    m_pAPIsToTraceTW->addTopLevelItem(m_pHSARootItem);
    m_pAPIsToTraceTW->expandAll();

    AddCLAPIFunctionsToTree();
    AddHSAAPIFunctionsToTree();

    m_pAPIsToTraceTW->sortItems(0, Qt::AscendingOrder);
}

void OpenCLTraceOptions::HandleTreeWidgetItemChangeEvent(QTreeWidget* topOfTree, QTreeWidgetItem* item, int column, QString topOfTreeString)
{
    (void)(column); // unused
    (void)(topOfTreeString); // unused

    if ((nullptr != topOfTree) && (nullptr != item))
    {
        int dummyVal = 0;
        Util::UpdateTreeWidgetItemCheckState(item, true, false, dummyVal);
    }
}

void OpenCLTraceOptions::RulesTreeItemChanged(QTreeWidgetItem* item, int column)
{
    disconnect(m_pAPIRulesTW, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(RulesTreeItemChanged(QTreeWidgetItem*, int)));
    HandleTreeWidgetItemChangeEvent(m_pAPIRulesTW, item, column, GP_Str_ProjectSettingsAllCLRulesItemName);
    connect(m_pAPIRulesTW, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(RulesTreeItemChanged(QTreeWidgetItem*, int)));
}

void OpenCLTraceOptions::APIsTreeItemChanged(QTreeWidgetItem* item, int column)
{
    disconnect(m_pAPIsToTraceTW, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(APIsTreeItemChanged(QTreeWidgetItem*, int)));
    HandleTreeWidgetItemChangeEvent(m_pAPIsToTraceTW, item, column, GP_Str_ProjectSettingsAllCLAPIsItemName);
    connect(m_pAPIsToTraceTW, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(APIsTreeItemChanged(QTreeWidgetItem*, int)));
}

void OpenCLTraceOptions::RadioButtonToggled(bool /*isChecked*/)
{
    if (m_pGenerateSummaryPageRB->isChecked())
    {
        m_pAPIRulesTW->setEnabled(true);
        m_pAPIsToTraceTW->setEnabled(false);
    }
    else if (m_pAPIsToTraceRB->isChecked())
    {
        m_pAPIRulesTW->setEnabled(false);
        m_pAPIsToTraceTW->setEnabled(true);
    }
}

void OpenCLTraceOptions::OnAPITypeRadioButtonToggled()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pOpenCLRadioButton != nullptr) && (m_pShowAPIErrorCodeCB != nullptr) && (m_pCollapseCallsCB != nullptr) && (m_pGenerateOccupancyCB != nullptr))
    {
        // Check if the OpenCL or HSA is selected:
        APIToTrace api = m_pOpenCLRadioButton->isChecked() ? APIToTrace_OPENCL : APIToTrace_HSA;
        m_pShowAPIErrorCodeCB->setEnabled(api == APIToTrace_OPENCL);
        m_pCollapseCallsCB->setEnabled(api == APIToTrace_OPENCL);
        m_pGenerateOccupancyCB->setEnabled(api == APIToTrace_OPENCL);

        // Go over the rules tree and hide / show the relevant items:
        FitTreesToAPIType(api);
    }
}

void OpenCLTraceOptions::WriteTimeOutStateChanged(int state)
{
    bool enable = (state == (Qt::Checked)) ? true : false;
    m_pTimeOutIntervalSB->setEnabled(enable);
}

void OpenCLTraceOptions::FitTreesToAPIType(APIToTrace api)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pAPIRulesTW != nullptr) && (m_pAPIsToTraceTW != nullptr) && (m_pOpenCLRootItem != nullptr) && (m_pHSARootItem != nullptr))
    {
        // Set the text for the top level item in the rules tree:
        QString rulesCaption = (api == APIToTrace_OPENCL) ? GP_Str_ProjectSettingsAllCLRulesItemName : GP_Str_ProjectSettingsAllHSARulesItemName;
        GT_IF_WITH_ASSERT(m_pAPIRulesTW->topLevelItem(0) != nullptr)
        {
            m_pAPIRulesTW->topLevelItem(0)->setText(0, rulesCaption);
        }

        QTreeWidgetItemIterator iter(m_pAPIRulesTW);

        while (*iter)
        {
            // Check if this tree item should be displayed:
            bool shouldShow = true;

            if (api != APIToTrace_OPENCL)
            {
                shouldShow = m_hsaRulesTreeWidgetItems.contains(*iter) || (m_pAPIRulesTW->indexOfTopLevelItem(*iter) >= 0);
            }

            m_pAPIRulesTW->setItemHidden(*iter, !shouldShow);

            iter++;
        }

        // Show / hide the matching API root node:
        m_pAPIsToTraceTW->setItemHidden(m_pOpenCLRootItem, api != APIToTrace_OPENCL);
        m_pAPIsToTraceTW->setItemHidden(m_pHSARootItem, api != APIToTrace_HSA);
    }
}

void OpenCLTraceOptions::AddCLAPIFunctionsToTree()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pOpenCLRootItem != nullptr)
    {
        QStringList groupNamesForItem;

        for (int index = CL_FUNC_TYPE_clGetPlatformIDs; index < CL_FUNC_TYPE_Unknown; index++)
        {
            CL_FUNC_TYPE type = CL_FUNC_TYPE(index);
            CLAPIGroups groups = CLAPIDefs::Instance()->GetCLAPIGroup(type);

            groupNamesForItem.clear();

            if (groups != CLAPIGroup_Unknown)
            {
                if ((groups & CLAPIGroup_CLObjectCreate) == CLAPIGroup_CLObjectCreate)
                {
                    groupNamesForItem.append(CLAPIDefs::Instance()->GroupToString(CLAPIGroup_CLObjectCreate));
                }

                if ((groups & CLAPIGroup_CLObjectRetain) == CLAPIGroup_CLObjectRetain)
                {
                    groupNamesForItem.append(CLAPIDefs::Instance()->GroupToString(CLAPIGroup_CLObjectRetain));
                }

                if ((groups & CLAPIGroup_CLObjectRelease) == CLAPIGroup_CLObjectRelease)
                {
                    groupNamesForItem.append(CLAPIDefs::Instance()->GroupToString(CLAPIGroup_CLObjectRelease));
                }

                if ((groups & CLAPIGroup_QueryInfo) == CLAPIGroup_QueryInfo)
                {
                    groupNamesForItem.append(CLAPIDefs::Instance()->GroupToString(CLAPIGroup_QueryInfo));
                }

                if ((groups & CLAPIGroup_EnqueueDataTransfer) == CLAPIGroup_EnqueueDataTransfer)
                {
                    groupNamesForItem.append(CLAPIDefs::Instance()->GroupToString(CLAPIGroup_EnqueueDataTransfer));
                }

                if ((groups & CLAPIGroup_EnqueueKernel) == CLAPIGroup_EnqueueKernel)
                {
                    groupNamesForItem.append(CLAPIDefs::Instance()->GroupToString(CLAPIGroup_EnqueueKernel));
                }

                if ((groups & CLAPIGroup_EnqueueOther) == CLAPIGroup_EnqueueOther)
                {
                    groupNamesForItem.append(CLAPIDefs::Instance()->GroupToString(CLAPIGroup_EnqueueOther));
                }

                if ((groups & CLAPIGroup_OpenGLInterOp) == CLAPIGroup_OpenGLInterOp)
                {
                    groupNamesForItem.append(CLAPIDefs::Instance()->GroupToString(CLAPIGroup_OpenGLInterOp));
                }

                if ((groups & CLAPIGroup_DirectXInterOp) == CLAPIGroup_DirectXInterOp)
                {
                    groupNamesForItem.append(CLAPIDefs::Instance()->GroupToString(CLAPIGroup_DirectXInterOp));
                }

                if ((groups & CLAPIGroup_Synchronization) == CLAPIGroup_Synchronization)
                {
                    groupNamesForItem.append(CLAPIDefs::Instance()->GroupToString(CLAPIGroup_Synchronization));
                }

                if ((groups & CLAPIGroup_SetCallback) == CLAPIGroup_SetCallback)
                {
                    groupNamesForItem.append(CLAPIDefs::Instance()->GroupToString(CLAPIGroup_SetCallback));
                }

                if ((groups & CLAPIGroup_Extensions) == CLAPIGroup_Extensions)
                {
                    groupNamesForItem.append(CLAPIDefs::Instance()->GroupToString(CLAPIGroup_Extensions));
                }

                if ((groups & CLAPIGroup_Other) == CLAPIGroup_Other)
                {
                    groupNamesForItem.append(CLAPIDefs::Instance()->GroupToString(CLAPIGroup_Other));
                }
            }

            QTreeWidgetItem* pCurrentCLAPIGroup = nullptr;

            if (groups != CLAPIGroup_Unknown)
            {
                for (int i = 0; i < groupNamesForItem.count(); i++)
                {
                    pCurrentCLAPIGroup = Util::FindTreeItem(m_pAPIsToTraceTW, groupNamesForItem[i]);

                    if (!pCurrentCLAPIGroup)
                    {
                        QTreeWidgetItem* newCLAPIGroup = new QTreeWidgetItem();

                        newCLAPIGroup->setFlags(newCLAPIGroup->flags() | Qt::ItemIsUserCheckable);
                        newCLAPIGroup->setText(0, groupNamesForItem[i]);
                        newCLAPIGroup->setCheckState(0, Qt::Checked);
                        m_pOpenCLRootItem ->addChild(newCLAPIGroup);
                        pCurrentCLAPIGroup = newCLAPIGroup;
                    }

                    QTreeWidgetItem* newCLAPI = new QTreeWidgetItem();

                    newCLAPI->setFlags(newCLAPI->flags() | Qt::ItemIsUserCheckable);
                    newCLAPI->setText(0, CLAPIDefs::Instance()->GetOpenCLAPIString(type));
                    newCLAPI->setCheckState(0, Qt::Checked);
                    pCurrentCLAPIGroup->addChild(newCLAPI);
                }
            }
        }
    }
}

void OpenCLTraceOptions::AddHSAAPIFunctionsToTree()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pHSARootItem != nullptr)
    {
        QStringList groupNamesForItem;

        for (int index = HSA_API_Type_UNKNOWN + 1; index < HSA_API_Type_COUNT; index++)
        {
            HSA_API_Type type = HSA_API_Type(index);
            HSAAPIGroups groups = HSAAPIDefs::Instance()->GetHSAAPIGroup(type);

            groupNamesForItem.clear();

            if (groups != HSAAPIGroup_Unknown)
            {
                if (groups == HSAAPIGroup_Agent)
                {
                    groupNamesForItem.append(HSAAPIDefs::Instance()->GroupToString(HSAAPIGroup_Agent));
                }

                if (groups == HSAAPIGroup_CodeObject)
                {
                    groupNamesForItem.append(HSAAPIDefs::Instance()->GroupToString(HSAAPIGroup_CodeObject));
                }

                if (groups == HSAAPIGroup_Executable)
                {
                    groupNamesForItem.append(HSAAPIDefs::Instance()->GroupToString(HSAAPIGroup_Executable));
                }

                if (groups == HSAAPIGroup_ExtensionsGeneral)
                {
                    groupNamesForItem.append(HSAAPIDefs::Instance()->GroupToString(HSAAPIGroup_ExtensionsGeneral));
                }

                if (groups == HSAAPIGroup_ExtensionsFinalizer)
                {
                    groupNamesForItem.append(HSAAPIDefs::Instance()->GroupToString(HSAAPIGroup_ExtensionsFinalizer));
                }

                if (groups == HSAAPIGroup_ExtensionsImage)
                {
                    groupNamesForItem.append(HSAAPIDefs::Instance()->GroupToString(HSAAPIGroup_ExtensionsImage));
                }

                if (groups == HSAAPIGroup_ExtensionsSampler)
                {
                    groupNamesForItem.append(HSAAPIDefs::Instance()->GroupToString(HSAAPIGroup_ExtensionsSampler));
                }

                if (groups == HSAAPIGroup_InitShutDown)
                {
                    groupNamesForItem.append(HSAAPIDefs::Instance()->GroupToString(HSAAPIGroup_InitShutDown));
                }

                if (groups == HSAAPIGroup_ISA)
                {
                    groupNamesForItem.append(HSAAPIDefs::Instance()->GroupToString(HSAAPIGroup_ISA));
                }

                if (groups == HSAAPIGroup_Memory)
                {
                    groupNamesForItem.append(HSAAPIDefs::Instance()->GroupToString(HSAAPIGroup_Memory));
                }

                if (groups == HSAAPIGroup_QueryInfo)
                {
                    groupNamesForItem.append(HSAAPIDefs::Instance()->GroupToString(HSAAPIGroup_QueryInfo));
                }

                if (groups == HSAAPIGroup_Queue)
                {
                    groupNamesForItem.append(HSAAPIDefs::Instance()->GroupToString(HSAAPIGroup_Queue));
                }

                if (groups == HSAAPIGroup_Signal)
                {
                    groupNamesForItem.append(HSAAPIDefs::Instance()->GroupToString(HSAAPIGroup_Signal));
                }
            }

            QTreeWidgetItem* pCurrentHSAAPIGroup = nullptr;

            if (groups != HSA_API_Type_UNKNOWN)
            {
                for (int i = 0; i < groupNamesForItem.count(); i++)
                {
                    pCurrentHSAAPIGroup = Util::FindTreeItem(m_pAPIsToTraceTW, groupNamesForItem[i]);

                    if (!pCurrentHSAAPIGroup)
                    {
                        QTreeWidgetItem* pNewHSAAPIGroup = new QTreeWidgetItem();

                        pNewHSAAPIGroup->setFlags(pNewHSAAPIGroup->flags() | Qt::ItemIsUserCheckable);
                        pNewHSAAPIGroup->setText(0, groupNamesForItem[i]);
                        pNewHSAAPIGroup->setCheckState(0, Qt::Checked);
                        m_pHSARootItem->addChild(pNewHSAAPIGroup);
                        pCurrentHSAAPIGroup = pNewHSAAPIGroup;
                    }

                    QTreeWidgetItem* pNewHSAAPI = new QTreeWidgetItem();

                    pNewHSAAPI->setFlags(pNewHSAAPI->flags() | Qt::ItemIsUserCheckable);
                    pNewHSAAPI->setText(0, HSAAPIDefs::Instance()->GetHSAPIString(type));
                    pNewHSAAPI->setCheckState(0, Qt::Checked);
                    pCurrentHSAAPIGroup->addChild(pNewHSAAPI);
                }
            }
        }
    }
}

