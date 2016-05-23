//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CounterSelectionSettingPage.cpp $
/// \version $Revision: #91 $
/// \brief  This file contains GPU profile handler class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CounterSelectionSettingPage.cpp#91 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================

#include <qtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

#include <QtWidgets>
#include <QtXml>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTApplicationComponents/Include/acItemDelegate.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afCSSSettings.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// Local:
#include <AMDTGpuProfiling/gpStringConstants.h>
#include <AMDTGpuProfiling/CounterSelectionSettingPage.h>
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>
#include <AMDTGpuProfiling/ProfileManager.h>


CounterSelectionSettingWindow* CounterSelectionSettingWindow::m_spInstance = nullptr;

CounterSelectionSettingWindow::CounterSelectionSettingWindow(): afProjectSettingsExtension(),
    m_pOpenCLRadioButton(nullptr),
    m_pHSARadioButton(nullptr),
    m_pPerfCounterNotAvailableLabel(nullptr),
    m_pNumOfCounterSelectedLB(nullptr),
    m_pGenerateOccupancyCB(nullptr),
    m_pGpuTimeCollectCB(nullptr),
    m_pCounterListTW(nullptr),
    m_pLastValidListTW(nullptr),
    m_pLoadSelectionPB(nullptr),
    m_pSaveSelectionPB(nullptr),
    m_pCounterLayoutGroup(nullptr),
    m_pProfileSpecificKernelLabel(nullptr),
    m_pProfileSpecificKernelDesc(nullptr),
    m_pSpecificKernelsEdit(nullptr),
    m_pNoteLabel(nullptr),

#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS)
    m_pAdvancedInitXThread(nullptr),
#endif

    m_singleHardwareFamily(true),
    m_isRemoteSession(false),
    m_isSinglePassCollect(false),
    m_currentPassesCount(0),
    m_isPageOnDisplay(false),
    m_isCounterTreeUpdateNeeded(false)


{
}

CounterSelectionSettingWindow* CounterSelectionSettingWindow::Instance()
{
    if (m_spInstance == nullptr)
    {
        m_spInstance = new CounterSelectionSettingWindow;
    }

    return m_spInstance;
}

CounterSelectionSettingWindow::~CounterSelectionSettingWindow()
{

}

void CounterSelectionSettingWindow::Initialize()
{
    // Create the main captions for the page:
    QLabel* pCaption1 = new QLabel(GP_Str_CounterSelectionMainCaptionGeneral);
    QLabel* pCaption2 = new QLabel(GP_Str_CounterSelectionPreformanceCountersSelection);
    QLabel* pCaption3 = new QLabel(GP_Str_CounterSelectionProfileSpecificKernels);

    pCaption1->setStyleSheet(AF_STR_captionLabelStyleSheetMain);
    pCaption2->setStyleSheet(AF_STR_captionLabelStyleSheet);
    pCaption3->setStyleSheet(AF_STR_captionLabelStyleSheet);

    // Create the OpenCL / HSA radio button:
    QLabel* pAPITypeLabel = new QLabel(GP_Str_CounterSelectionAPITypeDesc);
    m_pOpenCLRadioButton = new QRadioButton(GP_Str_ProjectSettingsOpenCLAPI);
    m_pHSARadioButton = new QRadioButton(GP_Str_ProjectSettingsHSAAPI);

    m_pOpenCLRadioButton->setToolTip(GP_Str_ProjectSettingsOpenCLAPITooltip);
    m_pHSARadioButton->setToolTip(GP_Str_ProjectSettingsHSAAPITooltip);

    QButtonGroup* pGroup = new QButtonGroup;
    pGroup->addButton(m_pOpenCLRadioButton);
    pGroup->addButton(m_pHSARadioButton);

    m_pLoadSelectionPB = new QPushButton(GP_Str_CounterSelectionLoadSelection);
    m_pSaveSelectionPB = new QPushButton(GP_Str_CounterSelectionSaveSelection);

    m_pGenerateOccupancyCB = new QCheckBox(GP_Str_CounterSelectionGenerateOccupancyDetails);

    QHBoxLayout* hLayout = new QHBoxLayout();

    hLayout->addStretch(1);
    hLayout->addWidget(m_pLoadSelectionPB);
    hLayout->addWidget(m_pSaveSelectionPB);

    // Counter tree view list
    m_pCounterListTW = new QTreeWidget();

    m_pCounterListTW->setColumnCount(1);
    m_pCounterListTW->setHeaderHidden(true);
    m_pCounterListTW->setItemDelegate(new acItemDelegate);
    m_pCounterListTW->setStyleSheet(AF_STR_treeWidgetWithBorderStyleSheet);


    InitializeCounterTreeView(false);

    bool rc = connect(m_pCounterListTW, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(TreeWidgetItem_Changed(QTreeWidgetItem*, int)));
    GT_ASSERT(rc);
    rc = connect(m_pLoadSelectionPB, SIGNAL(clicked(bool)), this, SLOT(LoadSelectionButtonClicked()));
    GT_ASSERT(rc);
    rc = connect(m_pSaveSelectionPB, SIGNAL(clicked(bool)), this, SLOT(SaveSelectionButtonClicked()));
    GT_ASSERT(rc);

    rc = connect(m_pOpenCLRadioButton, SIGNAL(clicked()), this, SLOT(OnAPITypeRadioButtonToggled()));
    GT_ASSERT(rc);

    rc = connect(m_pHSARadioButton, SIGNAL(clicked()), this, SLOT(OnAPITypeRadioButtonToggled()));
    GT_ASSERT(rc);

    // Need to update the string based on counters selection.
    m_pNumOfCounterSelectedLB = new QLabel();

    QVBoxLayout* counterLayout = new QVBoxLayout();

    counterLayout->addWidget(m_pCounterListTW);

    m_pGpuTimeCollectCB = new QCheckBox(GP_str_GpuTimeCoolect);
    m_pGpuTimeCollectCB->setChecked(true);
    m_pGpuTimeCollectCB->setToolTip(GP_str_GpuTimeToolTip);

    counterLayout->addWidget(m_pGpuTimeCollectCB);
    rc = connect(m_pGpuTimeCollectCB, SIGNAL(toggled(bool)), this, SLOT(GpuTimeCollectChecked()));
    GT_ASSERT(rc);


    counterLayout->addWidget(m_pNumOfCounterSelectedLB);

    m_pNoteLabel = new QLabel(GP_str_MultiPassNote);
    m_pNoteLabel->setWordWrap(true);
    m_pNoteLabel->setVisible(false);
    counterLayout->addWidget(m_pNoteLabel);
    counterLayout->addWidget(m_pGenerateOccupancyCB);
    counterLayout->addLayout(hLayout);

    m_pCounterLayoutGroup = new QGroupBox();

    m_pCounterLayoutGroup->setLayout(counterLayout);

#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS)
    m_pAdvancedInitXThread = new QCheckBox(GP_Str_CounterSelectionXInitThreadsDesc);
#endif

    QVBoxLayout* pMainLayout = new QVBoxLayout();

    // Add the OpenCL / HSA radio buttons:
    pMainLayout->addWidget(pCaption1);
    pMainLayout->addWidget(pAPITypeLabel);
    pMainLayout->addWidget(m_pOpenCLRadioButton);
    pMainLayout->addWidget(m_pHSARadioButton);

    QLabel* pMultiPassExpLabel = new QLabel(GP_str_CollectingMultiCountersLabel);
    pMultiPassExpLabel->setWordWrap(true);
    m_pPerfCounterNotAvailableLabel = new QLabel(QString("<font color = 'red'><b>%1</b></font>").arg(GP_str_PerfCounterNotAvailable));

    // Creating all widgets for the 'Profile Specific Kernels' section
    QHBoxLayout* pProfileSpecificKernelLayout = new QHBoxLayout();
    // Padding to create a gap from end of frame
    pProfileSpecificKernelLayout->setContentsMargins(0, 0, 0, 15);

    m_pProfileSpecificKernelDesc = new QLabel(GP_str_ProfileSpecificKernelsDesc);

    QVBoxLayout* pProfileSpecificKernelRight = new QVBoxLayout();
    // align right layout widgets to right one
    pProfileSpecificKernelRight->setContentsMargins(0, 10, 0, 0);
    m_pSpecificKernelsEdit = new QLineEdit();
    pProfileSpecificKernelRight->addWidget(m_pSpecificKernelsEdit);

    pProfileSpecificKernelLayout->addWidget(m_pProfileSpecificKernelDesc);
    pProfileSpecificKernelLayout->addLayout(pProfileSpecificKernelRight, 1);

    pMainLayout->addWidget(pCaption2);
    pMainLayout->addWidget(pMultiPassExpLabel);
    pMainLayout->addWidget(m_pPerfCounterNotAvailableLabel, 1, Qt::AlignTop);
    m_pPerfCounterNotAvailableLabel->setHidden(true);

    pMainLayout->addWidget(m_pCounterLayoutGroup);

    pMainLayout->addWidget(pCaption3);
    pMainLayout->addLayout(pProfileSpecificKernelLayout);

#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS)
    QLabel* pCaption4 = new QLabel(GP_Str_CounterSelectionAdvancedOptions);
    pMainLayout->addWidget(pCaption4);
    pMainLayout->addWidget(m_pAdvancedInitXThread);
#endif

    setLayout(pMainLayout);

    RestoreCurrentSettings();

    // Register to the remote check box change event.
    bool isRegistered = connect(&(afProjectManager::instance()), SIGNAL(OnRemoteHostCheckBoxCheckChange(bool)), this, SLOT(RemoteSessionStatusChangedHandler(bool)));
    GT_ASSERT_EX(isRegistered, L"Failed to register to the remote context change event.");

    // Check if the HSA runtime and OpenCL runtime are installed on this machine, and enable / disable widgets accordingly
    bool isCatalystInstalled = (afGlobalVariablesManager::instance().InstalledAMDComponentsBitmask() & AF_AMD_CATALYST_COMPONENT);
    bool isHSAInstalled = (afGlobalVariablesManager::instance().InstalledAMDComponentsBitmask() & AF_AMD_HSA_COMPONENT);

    if (isHSAInstalled && !isCatalystInstalled)
    {
        m_pGpuTimeCollectCB->setChecked(false);
    }
}

gtString CounterSelectionSettingWindow::ExtensionXMLString()
{
    return GPU_STR_COUNTER_PROJECT_SETTINGS;
}

gtString CounterSelectionSettingWindow::ExtensionTreePathAsString()
{
    return GPU_STR_TRACE_PROJECT_TREE_PATH_STR;
}

bool CounterSelectionSettingWindow::GetXMLSettingsString(gtString& projectAsXMLString)
{
    bool retVal = false;
    gtString pageName = GPU_STR_COUNTER_PROJECT_SETTINGS;

    retVal = getProjectSettingsXML(projectAsXMLString, pageName);
    return retVal;
}

bool CounterSelectionSettingWindow::SetSettingsFromXMLString(const gtString& projectAsXMLString)
{
    bool retVal = setProjectSettingsXML(projectAsXMLString);

    RestoreCurrentSettings();

    m_isCounterTreeUpdateNeeded = true;

    return retVal;
}

bool CounterSelectionSettingWindow::setProjectSettingsXML(const gtString& projectAsXMLString)
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
        else if (childNode.nodeName() == acGTStringToQString(GPU_STR_ProjectSettingsGpuTimeCollect))
        {
            m_currentSettings.m_measureKernelExecutionTime = val;
        }
        else if (childNode.nodeName() == acGTStringToQString(GPU_STR_ProjectSettingsIsRemoteSession))
        {
            bool isRemoteSession = val;
            // If we are loading a session, the CounterSelectionSettingPage
            // should update its state accordingly.
            RemoteSessionStatusChangedHandler(isRemoteSession);
        }
        else if (childNode.nodeName() == acGTStringToQString(GPU_STR_ProjectSettingsCounterTree))
        {
            if (childNode.hasChildNodes())
            {
                UpdateCountersCheckedListFromXML(childNode, m_currentSettings.m_checkedCounterList, false);
            }
            else
            {
                if (m_pCounterListTW != nullptr)
                {
                    Util::SetCheckState(m_pCounterListTW, !Util::IsInternalBuild());
                }
            }

            UpdateTreeWidgetFromCountersList(m_pCounterListTW, m_currentSettings.m_checkedCounterList, false);
        }
        else if (childNode.nodeName() == acGTStringToQString(GPU_STR_ProjectSettingsAPIType))
        {
            if (nodeVal == acGTStringToQString(GPU_STR_ProjectSettingsAPITypeOpenCL))
            {
                m_currentSettings.m_api = APIToTrace_OPENCL;
            }
            else if (nodeVal == acGTStringToQString(GPU_STR_ProjectSettingsAPITypeHSA))
            {
                m_currentSettings.m_api = APIToTrace_HSA;
            }
            else
            {
                m_currentSettings.m_api = APIToTrace_OPENCL;
                GT_ASSERT_EX(false, L"Invalid project settings option");
            }
        }
        else if (childNode.nodeName() == acGTStringToQString(GPU_STR_ProjectSettingsSpecificKernels))
        {
            m_currentSettings.m_specificKernels = acQStringToGTString(nodeVal);
        }

        else if (childNode.nodeName() == acGTStringToQString(GPU_STR_ProjectSettingsCallXInitThreads))
        {
            m_currentSettings.m_shouldCallXInitThread = val;
        }

        childNode = childNode.nextSibling();
    }

    return true;
}

void CounterSelectionSettingWindow::RestoreDefaultProjectSettings()
{
    // Restore the settings to default values:
    m_currentSettings.RestoreDefault(m_fullCounterNamesList);

    // Select the maximal counters set that don't require more multi-pass:
    DisableCountersForSinglePass();

    RestoreCurrentSettings();

    m_isCounterTreeUpdateNeeded = true;
    m_isPageOnDisplay = true;

    m_pCounterListTW->blockSignals(true);

    // Initialize with defaults
    if (m_pCounterListTW != nullptr)
    {
        bool isInternalBuild = Util::IsInternalBuild();
        Util::SetCheckState(m_pCounterListTW, !isInternalBuild);

        UpdateCountersTreeCheckState();

        UpdateLabel();
    }

    m_pCounterListTW->blockSignals(false);
}

bool CounterSelectionSettingWindow::RestoreCurrentSettings()
{
    m_pCounterListTW->blockSignals(true);
    m_pOpenCLRadioButton->setChecked(m_currentSettings.m_api == APIToTrace_OPENCL);
    m_pHSARadioButton->setChecked(m_currentSettings.m_api == APIToTrace_HSA);
    m_pGenerateOccupancyCB->setChecked(m_currentSettings.m_generateKernelOccupancy);
    m_pGpuTimeCollectCB->setChecked(m_currentSettings.m_measureKernelExecutionTime);
    m_pSpecificKernelsEdit->setText(acGTStringToQString(m_currentSettings.m_specificKernels));
    const bool isHsaEnabled = Util::IsHSAEnabled();
    m_pHSARadioButton->setEnabled(isHsaEnabled);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    m_pHSARadioButton->setChecked(false);
#endif

#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS)

    // Check if the catalyst and HSA are installed, and enable / check the OpenCL / HSA button accordingly:
    bool isCatalystInstalled = (afGlobalVariablesManager::instance().InstalledAMDComponentsBitmask() & AF_AMD_CATALYST_COMPONENT);
    m_pOpenCLRadioButton->setEnabled(isCatalystInstalled);

    if (!isCatalystInstalled)
    {
        m_pHSARadioButton->setChecked(true);
    }

    m_pAdvancedInitXThread->setChecked(m_currentSettings.m_shouldCallXInitThread);
#endif

    // Make sure that the widgets are checked / un-checked according to the API type:
    OnAPITypeRadioButtonToggled();

    // Update the tree from the counters checked list:
    UpdateTreeWidgetFromCountersList(m_pCounterListTW, m_currentSettings.m_checkedCounterList, false);

    UpdateCountersTreeCheckState();
    UpdateLabel();

    UpdateProjectSettings();

    m_pCounterListTW->blockSignals(false);

    return true;
}

void CounterSelectionSettingWindow::UpdateCounterCapture()
{
    m_currentSettings.m_checkedCounterList.clear();

    if (m_pCounterListTW)
    {
        QString counterName;
        bool isItemChecked = false;

        for (QMap<HardwareFamily, QString>::const_iterator it = m_hardwareFamilyTreeNodeMap.begin(); it != m_hardwareFamilyTreeNodeMap.end(); ++it)
        {
            HardwareFamily hwFamily = it.key();
            GT_ASSERT(m_treeNodeMap.contains(hwFamily));

            for (int i = 0; i < CounterManager::Instance()->GetHWCounterCount(hwFamily); ++i)
            {
                CounterManager::Instance()->GetHWCounterName(hwFamily, i, counterName);
                counterName = counterName.trimmed();

                QTreeWidgetItem* treeItem = nullptr;

                if (m_treeNodeMap[hwFamily].contains(counterName))
                {
                    treeItem = m_treeNodeMap[hwFamily][counterName];
                }

                isItemChecked = treeItem != nullptr && (treeItem->checkState(0) == Qt::Checked) ? true : false;

                // Will log only checked counters (too many items unchecked in internal mode)
                if (isItemChecked)
                {
                    if (!m_currentSettings.m_checkedCounterList.contains(counterName))
                    {
                        m_currentSettings.m_checkedCounterList.insert(counterName, isItemChecked);
                    }
                }

                CounterManager::Instance()->SetHWCounterCapture(hwFamily, counterName, isItemChecked);
            }
        }
    }
}

void CounterSelectionSettingWindow::UpdateProjectSettings()
{
    if (ProfileManager::Instance()->GetCurrentProjectSettings())
    {
        ProfileManager::Instance()->GetCurrentProjectSettings()->m_counterOptions.m_generateKernelOccupancy = m_currentSettings.m_generateKernelOccupancy;
        ProfileManager::Instance()->GetCurrentProjectSettings()->m_counterOptions.m_specificKernels = m_currentSettings.m_specificKernels;
        ProfileManager::Instance()->GetCurrentProjectSettings()->m_counterOptions.m_checkedCounterList = m_currentSettings.m_checkedCounterList;
        ProfileManager::Instance()->GetCurrentProjectSettings()->m_counterOptions.m_measureKernelExecutionTime = m_currentSettings.m_measureKernelExecutionTime;
        ProfileManager::Instance()->GetCurrentProjectSettings()->m_counterOptions.m_api = m_currentSettings.m_api;
        ProfileManager::Instance()->GetCurrentProjectSettings()->m_counterOptions.m_shouldCallXInitThread = m_currentSettings.m_shouldCallXInitThread;
    }
}

bool CounterSelectionSettingWindow::IsCountersSelected()
{
    if (Util::ItemsSelectedInTreeWidget(m_pCounterListTW) > 0)
    {
        return true;
    }

    return false;
}

void CounterSelectionSettingWindow::SetCounterCheckState(const QString& strCounterName, bool checkState)
{
    Qt::CheckState qCheckState = checkState ? Qt::Checked : Qt::Unchecked;

    for (QMap<HardwareFamily, QString>::const_iterator it = m_hardwareFamilyTreeNodeMap.begin(); it != m_hardwareFamilyTreeNodeMap.end(); ++it)
    {
        HardwareFamily hwFamily = it.key();
        GT_ASSERT(m_treeNodeMap.contains(hwFamily));

        if (m_treeNodeMap[hwFamily].contains(strCounterName) &&
            m_treeNodeMap[hwFamily][strCounterName] != nullptr)
        {
            m_treeNodeMap[hwFamily][strCounterName]->setCheckState(0, qCheckState);
        }
    }
}

bool CounterSelectionSettingWindow::AreSettingsValid(gtString& invalidMessageStr)
{
    bool retVal = true;

    if (!ValidateSpecificKernelsText())
    {
        invalidMessageStr = GPU_STR_ProjectSettingsSpecificKernelErrMsg;
        retVal = false;
    }

    SaveCurrentSettings();

    // HSA API currently supports only 1 pass counters selection:
    //if we have dummy devices added ,then this machine probably has no catalyst, so below error message irrelevant
    bool countersDisabledOnMachine = CounterManager::Instance()->IsDummyDevicesAdded() && !m_isRemoteSession;

    if (!m_isSinglePassCollect && m_currentSettings.m_api == APIToTrace_HSA && countersDisabledOnMachine == false)
    {
        // Output a warning that more than 1 pass is required and some counters will be removed:
        QString userMessage = QString(GP_Str_CounterSelectionHSAPassesWarning).arg(m_currentPassesCount);
        acMessageBox::instance().warning(afGlobalVariablesManager::ProductNameA(), userMessage);

        // Select the maximal counters set that don't require more multi-pass:
        DisableCountersForSinglePass();
    }

    return retVal;
}

bool CounterSelectionSettingWindow::SaveCurrentSettings()
{
    const bool isHsaEnabled = Util::IsHSAEnabled();
    m_pHSARadioButton->setEnabled(isHsaEnabled);

    m_currentSettings.m_api = m_pOpenCLRadioButton->isChecked() ? APIToTrace_OPENCL : APIToTrace_HSA;
    m_currentSettings.m_generateKernelOccupancy = m_pGenerateOccupancyCB->isChecked();
    m_currentSettings.m_measureKernelExecutionTime = m_pGpuTimeCollectCB->isChecked();
    m_currentSettings.m_specificKernels = acQStringToGTString(m_pSpecificKernelsEdit->text());
#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS)
    m_currentSettings.m_shouldCallXInitThread = m_pAdvancedInitXThread->isChecked();
#endif

    UpdateCounterCapture();
    UpdateProjectSettings();
    return true;
}

bool CounterSelectionSettingWindow::getProjectSettingsXML(gtString& projectAsXMLString, gtString& projectPage)
{
    projectAsXMLString.append(L"<");
    projectAsXMLString.append(projectPage.asCharArray());
    projectAsXMLString.append(L">");

    writeSession(projectAsXMLString, L"Current");

    projectAsXMLString.append(L"</");
    projectAsXMLString.append(projectPage.asCharArray());
    projectAsXMLString.append(L">");

    return true;
}

void CounterSelectionSettingWindow::writeSession(gtString& projectAsXMLString, const gtString& type)
{
    projectAsXMLString.append(L"<Session type=\"");
    projectAsXMLString.append(type);
    projectAsXMLString.append(L"\">");

    // Indicates that we are in a remote session.
    gtString apiTypeStr = (m_currentSettings.m_api == APIToTrace_OPENCL) ? GPU_STR_ProjectSettingsAPITypeOpenCL : GPU_STR_ProjectSettingsAPITypeHSA;
    writeValue(projectAsXMLString, GPU_STR_ProjectSettingsAPIType, apiTypeStr);
    writeBool(projectAsXMLString, GPU_STR_ProjectSettingsIsRemoteSession, afProjectManager::instance().currentProjectSettings().isRemoteTarget());

    writeBool(projectAsXMLString, GPU_STR_ProjectSettingsGenerateOccupancy, m_currentSettings.m_generateKernelOccupancy);

    writeBool(projectAsXMLString, GPU_STR_ProjectSettingsGpuTimeCollect, m_currentSettings.m_measureKernelExecutionTime);

    writeValue(projectAsXMLString, GPU_STR_ProjectSettingsSpecificKernels, m_currentSettings.m_specificKernels);

    writeBool(projectAsXMLString, GPU_STR_ProjectSettingsCallXInitThreads, m_currentSettings.m_shouldCallXInitThread);

    // Counter tree widget
    AppendTree(projectAsXMLString, acGTStringToQString(GPU_STR_ProjectSettingsCounterTree), m_currentSettings.m_checkedCounterList);

    projectAsXMLString.append(L"</Session>");
}

void CounterSelectionSettingWindow::AddFamilyToTree(HardwareFamily hardwareFamily)
{
    int counterCount = CounterManager::Instance()->GetHWCounterGroupCount(hardwareFamily);

    if (counterCount > 0)
    {
        QTreeWidgetItem* root = new QTreeWidgetItem();
        root->setFlags(root->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        QString strCounterDisplayName;
        CounterMap counterMap;

        if (m_singleHardwareFamily)
        {
            strCounterDisplayName = "All Counters";
        }
        else
        {
            CounterManager::Instance()->GetHardwareFamilyDisplayName(hardwareFamily, strCounterDisplayName);
            strCounterDisplayName.append(" Counters");
        }

        QString strDeviceList;

        CounterManager::Instance()->GetHardwareFamilyDevicesDisplayList(hardwareFamily, strDeviceList);

        if (!strDeviceList.isEmpty())
        {
            strCounterDisplayName.append(" (").append(strDeviceList).append(")");
        }

        root->setText(0, strCounterDisplayName);
        root->setCheckState(0, Qt::Unchecked);
        m_pCounterListTW->addTopLevelItem(root);
        m_pCounterListTW->expandItem(root);

        m_numOfCountersSelected.insert(strCounterDisplayName, 0);
        m_hardwareFamilyTreeNodeMap.insert(hardwareFamily, strCounterDisplayName);

        QTreeWidgetItem* counterGroups;
        QTreeWidgetItem* child;

        // add counter group names
        for (int i = 0; i < counterCount; ++i)
        {
            QString strCounterGroupName;

            if (CounterManager::Instance()->GetHWCounterGroupName(hardwareFamily, i, strCounterGroupName))
            {

                counterGroups = new QTreeWidgetItem();
                counterGroups->setFlags(counterGroups->flags() | Qt::ItemIsUserCheckable);
                counterGroups->setText(0, strCounterGroupName);
                counterGroups->setCheckState(0, Qt::Unchecked);

                for (int j = 0; j < CounterManager::Instance()->GetHWCounterCountInGroup(hardwareFamily, i); ++j)
                {
                    QString strCounterName;

                    if (CounterManager::Instance()->GetHWCounterNameInGroup(hardwareFamily, i, j, strCounterName))
                    {
                        child = new QTreeWidgetItem();
                        child->setFlags(child->flags() | Qt::ItemIsUserCheckable);
                        child->setText(0, strCounterName);
                        child->setCheckState(0, Qt::Unchecked);
                        QString strCounterDesc;

                        if (CounterManager::Instance()->GetCounterDesc(hardwareFamily, strCounterName, strCounterDesc))
                        {
                            child->setToolTip(0, strCounterDesc);
                        }

                        counterGroups->addChild(child);
                        counterMap.insert(strCounterName, child);
                        m_fullCounterNamesList << strCounterName;
                    }
                }

                root->addChild(counterGroups);
                m_treeNodeMap.insert(hardwareFamily, counterMap);
            }
        }
    }
}

void CounterSelectionSettingWindow::InitializeCounterTreeView(bool isRemoteSession)
{
    m_pCounterListTW->blockSignals(true);

    m_pCounterListTW->clear();
    int numFamiliesSupported = 0;

    bool bVISupported = false;
    bool bCISupported = false;
    bool bSISupported = false;

    if (isRemoteSession || CounterManager::Instance()->IsHardwareFamilySupported(VOLCANIC_ISLANDS_FAMILY))
    {
        numFamiliesSupported++;
        bVISupported = true;
    }

    if (isRemoteSession || CounterManager::Instance()->IsHardwareFamilySupported(SEA_ISLANDS_FAMILY))
    {
        numFamiliesSupported++;
        bCISupported = true;
    }

    if (isRemoteSession || CounterManager::Instance()->IsHardwareFamilySupported(SOUTHERN_ISLANDS_FAMILY))
    {
        numFamiliesSupported++;
        bSISupported = true;
    }

    m_singleHardwareFamily = numFamiliesSupported == 1;

    // Log the supported hardware families on the machine:
    gtString message;
    message.appendFormattedString(L"Supported HW families: VI: %d, CI: %d, SI: %d", bVISupported, bCISupported, bSISupported);
    OS_OUTPUT_DEBUG_LOG(message.asCharArray(), OS_DEBUG_LOG_INFO);


    if (bVISupported)
    {
        AddFamilyToTree(VOLCANIC_ISLANDS_FAMILY);
    }

    if (bCISupported)
    {
        AddFamilyToTree(SEA_ISLANDS_FAMILY);
    }

    if (bSISupported)
    {
        AddFamilyToTree(SOUTHERN_ISLANDS_FAMILY);
    }

    m_pCounterListTW->blockSignals(false);

    // Once the tree is filled with all default counters, update the counters list to contain the same list:
    UpdateCounterCapture();
}

void CounterSelectionSettingWindow::LoadCaptureSettings()
{
    // uncheck all nodes
    Util::SetCheckState(m_pCounterListTW, false);

    // set selected counters to the selectedCountersListBox.
    for (QMap<HardwareFamily, QString>::const_iterator it = m_hardwareFamilyTreeNodeMap.begin(); it != m_hardwareFamilyTreeNodeMap.end(); ++it)
    {
        for (int i = 0; i < CounterManager::Instance()->GetHWCounterCount(it.key()); ++i)
        {
            bool counterCapture = false;

            HardwareFamily hwFamily = it.key();

            if (CounterManager::Instance()->GetHWCounterCapture(hwFamily, i, counterCapture))
            {
                QString strCounter;

                // set checks for captured counters
                if (counterCapture && CounterManager::Instance()->GetHWCounterName(hwFamily, i, strCounter))
                {
                    SetCounterCheckState(strCounter, counterCapture);
                }
            }
        }
    }

    UpdateLabel();
}

void CounterSelectionSettingWindow::UpdateLabel()
{
    if (nullptr != m_pCounterListTW)
    {
        m_pCounterLayoutGroup->setHidden(false);
        m_pPerfCounterNotAvailableLabel->setHidden(true);

        m_pCounterListTW->blockSignals(true);

        QString strLabelText;
        m_isSinglePassCollect = false;

        int i = 0;

        for (QMap<HardwareFamily, QString>::const_iterator it = m_hardwareFamilyTreeNodeMap.begin();
             it != m_hardwareFamilyTreeNodeMap.end(); ++it, i++)
        {
            QString strPassCounts;
            QStringList enabledCounters;
            FillTreeWidgetStringList(m_pCounterListTW, enabledCounters);

            // add number of counters selected to label
            int numCounters = enabledCounters.count();

            QString hardwareFamilyDisplayName;
            QString strDeviceList;

            // sanity check - the index of top level item in tree is ok (top og tree = hardware family)
            if (i < m_pCounterListTW->topLevelItemCount())
            {
                // add title to the top level item
                QTreeWidgetItem* item = m_pCounterListTW->topLevelItem(i);

                CounterManager::Instance()->GetHardwareFamilyDevicesDisplayList(it.key(), strDeviceList);

                if (!strDeviceList.isEmpty())
                {
                    hardwareFamilyDisplayName.append(strDeviceList).append(QString(" - %1").arg(numCounters));
                    hardwareFamilyDisplayName.append(" counters selected");
                    // create item with same text as in the source tree
                    item->setText(0, hardwareFamilyDisplayName);
                }
            }

            QList<int> numReqPassesForDevs;
            QString message(strDeviceList);

            if (CounterManager::Instance()->GetHardwareFamilyPassCountDisplayList(it.key(),
                                                                                  enabledCounters,
                                                                                  IsGpuTimeCollected(),
                                                                                  numReqPassesForDevs))
            {
                if (numReqPassesForDevs[0] == 1)
                {
                    message.append(": This counter combination requires 1 pass");
                    m_pNoteLabel->setVisible(false);
                    m_isSinglePassCollect = true;
                    m_currentPassesCount = 1;
                }
                else if (numReqPassesForDevs[0] > 1)
                {
                    message.append(QString(": This counter combination <font color = 'red'><b> requires %1 passes</b></font>").arg(numReqPassesForDevs[0]));
                    m_pNoteLabel->setVisible(true);
                    m_currentPassesCount = numReqPassesForDevs[0];
                }
                else if (numReqPassesForDevs[0] == 0)
                {
                    // Treat no passes as single pass (this flag is used for warning, and the warning is irrelevant when no counters are available):
                    m_isSinglePassCollect = true;
                    message = "";
                    m_pNoteLabel->setVisible(false);
                }
            }

            if (!strLabelText.isEmpty())
            {
                strLabelText.prepend("<br>");
            }

            strLabelText.prepend(message);
        }

        m_pNumOfCounterSelectedLB->setText(strLabelText);

        // if the list contains dummy devices (sample device for each hardware family), show message of perf counters not available
        // but, if the session is remote session - show the list of dummy devices
        if (nullptr != m_pCounterLayoutGroup &&
            nullptr != m_pPerfCounterNotAvailableLabel &&
            CounterManager::Instance()->IsDummyDevicesAdded() &&
            !m_isRemoteSession)
        {
            m_pCounterLayoutGroup->setHidden(true);
            m_pPerfCounterNotAvailableLabel->setHidden(false);
        }

        m_pCounterListTW->blockSignals(false);
    }
}

void CounterSelectionSettingWindow::FillTreeWidgetStringList(QTreeWidget* tree, QStringList& strList)
{
    GT_ASSERT(tree != nullptr)
    {
        QTreeWidgetItemIterator iterator(tree);

        GT_ASSERT((*iterator) != nullptr)
        {
            // go over the tree
            while (*iterator)
            {
                // if end point checkbox (item dont have children) and it checked - add it to the string list
                if (((*iterator)->childCount() == 0) && ((*iterator)->checkState(0) == Qt::Checked))
                {
                    strList.append((*iterator)->text(0));
                }

                iterator++;
            }
        }
    }

    // remove duplicated strings in the strings list
    strList.removeDuplicates();
}

bool CounterSelectionSettingWindow::LoadCounterFile(const QString& strCounterFile)
{
    if (!QFile::exists(strCounterFile))
    {
        return false;
    }

    QFile file(strCounterFile);
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    // uncheck all nodes
    Util::SetCheckState(m_pCounterListTW, false);

    // read all the counters until end of file
    while (!file.atEnd())
    {
        QString strLine = QString::fromLatin1(file.readLine());

        if (strLine.isEmpty())
        {
            continue;
        }

        SetCounterCheckState(strLine.trimmed(), true);
    }

    file.close();

    return true;
}

bool CounterSelectionSettingWindow::SaveCounterFile(QString strCounterFile)
{
    if (QFile::exists(strCounterFile))
    {
        QFile::remove(strCounterFile);
    }

    QFile file(strCounterFile);
    file.open(QIODevice::WriteOnly);
    QTextStream write(&file);

    QTreeWidgetItemIterator iterator(m_pCounterListTW);

    while (*iterator)
    {
        if (((*iterator)->childCount() == 0) && ((*iterator)->checkState(0) == Qt::Checked))
        {
            write << (*iterator)->text(0) << QString("\n");
        }

        iterator++;
    }

    file.close();

    return true;
}

void CounterSelectionSettingWindow::UpdateIdenticalCounters(QTreeWidgetItem* item, bool checked)
{
    if (!m_singleHardwareFamily)
    {
        int childCount = item->childCount();

        if (childCount == 0)
        {
            SetCounterCheckState(item->text(0), checked);
        }
        else
        {
            for (int i = 0; i < childCount; i++)
            {
                UpdateIdenticalCounters(item->child(i), checked);
            }
        }
    }
}

void CounterSelectionSettingWindow::TreeWidgetItem_Changed(QTreeWidgetItem* item, int column)
{
    (void)(column); // unused
    disconnect(m_pCounterListTW, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(TreeWidgetItem_Changed(QTreeWidgetItem*, int)));

    if (!m_isPageOnDisplay)
    {
        // meaning counters selection changed not by user, needs to update check state
        m_isCounterTreeUpdateNeeded = true;
    }

    QTreeWidgetItem* parent = item;

    while (parent->parent() != nullptr)
    {
        parent = parent->parent();
    }

    Util::UpdateTreeWidgetItemCheckState(item, true, true, m_numOfCountersSelected[parent->text(0)]);

    bool rc = connect(m_pCounterListTW, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(TreeWidgetItem_Changed(QTreeWidgetItem*, int)));
    GT_ASSERT(rc);

    UpdateIdenticalCounters(item, item->checkState(0) == Qt::Checked);

    UpdateLabel();
}

void CounterSelectionSettingWindow::LoadSelectionButtonClicked()
{
    QString defaultFileLocation = acGTStringToQString(afProjectManager::instance().currentProjectFilePath().fileDirectoryAsString());
    QString loadFileName = afApplicationCommands::instance()->ShowFileSelectionDialog("Load Counter Selection File", defaultFileLocation, "Counter Selection File (*.csl)", nullptr);

    if (!loadFileName.isEmpty())
    {
        if (QFile::exists(loadFileName))
        {
            LoadCounterFile(loadFileName);
        }
    }
}

void CounterSelectionSettingWindow::GpuTimeCollectChecked()
{
    UpdateLabel();
}

bool CounterSelectionSettingWindow::IsGpuTimeCollected()
{
    return m_pGpuTimeCollectCB->isChecked();
}

bool CounterSelectionSettingWindow::IsSinglePassChecked()
{
    return m_isSinglePassCollect;
}

void CounterSelectionSettingWindow::SaveSelectionButtonClicked()
{
    QString defaultFileLocation = acGTStringToQString(afProjectManager::instance().currentProjectFilePath().fileDirectoryAsString());
    QString savedFileName = afApplicationCommands::instance()->ShowFileSelectionDialog("Save Counter Selection File", defaultFileLocation, "Counter Selection File (*.csl)", nullptr, true);

    // Append file extension if it is missing.
    savedFileName = Util::AppendFileExtension(savedFileName, ".csl");

    if (!savedFileName.isEmpty())
    {
        SaveCounterFile(savedFileName);
    }
}

// Handler for a remote context change event.
void CounterSelectionSettingWindow::RemoteSessionStatusChangedHandler(bool isRemoteSession)
{
    // We don't need to act, as long as the state has not changed.
    if (isRemoteSession != m_isRemoteSession)
    {
        // Update the state flag.
        m_isRemoteSession = isRemoteSession;

        // Clear old state.
        m_numOfCountersSelected.clear();
        m_hardwareFamilyTreeNodeMap.clear();
        m_treeNodeMap.clear();

        // Adjust the CounterManager's state to our context.
        // This member function will also generate our new state.
        CounterManager::Instance()->AdjustCounterManagerState(isRemoteSession);

        // Now adjust the GUI.
        InitializeCounterTreeView(isRemoteSession);

        // Update the selected counters label, and select all counters.
        RestoreCurrentSettings();
    }
}

void CounterSelectionSettingWindow::UpdateCountersTreeCheckState()
{
    // UpdateTreeTopLevelCheckState can be heavy so call it only if we are about to show the tree
    // and only if there are any changes to tree check state
    if (m_isPageOnDisplay && m_isCounterTreeUpdateNeeded)
    {
        disconnect(m_pCounterListTW, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(TreeWidgetItem_Changed(QTreeWidgetItem*, int)));

        UpdateTreeTopLevelCheckState(m_pCounterListTW);
        m_isCounterTreeUpdateNeeded = false;

        bool rc = connect(m_pCounterListTW, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(TreeWidgetItem_Changed(QTreeWidgetItem*, int)));
        GT_ASSERT(rc);
    }
}

void CounterSelectionSettingWindow::UpdateTreeTopLevelCheckState(QTreeWidget* pTree)
{
    Qt::CheckState state;

    // go over all top level items and get their check state
    for (int i = 0; i < pTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = pTree->topLevelItem(i);
        state = GetItemCheckStateByChildrenState(item);
        // the state return param is needed only for the recursive call
        GT_UNREFERENCED_PARAMETER(state);
    }
}

Qt::CheckState CounterSelectionSettingWindow::GetItemCheckStateByChildrenState(QTreeWidgetItem* pParent)
{
    Qt::CheckState cs = Qt::Unchecked;
    int numOfChecked = 0;
    int numOfUnchecked = 0;
    static const QStringList HSADefaultCheckedList =
    {
        "Wavefronts",
        "VALUInsts",
        "SALUInsts",
        "VFetchInsts",
        "SFetchInsts",
        "VWriteInsts",
        "FlatVMemInsts",
        "SALUBusy",
        "FlatLDSInsts",
        "FetchSize",
        "WriteSize",
        "CacheHit",
        "MemUnitStalled"
    };

    GT_IF_WITH_ASSERT(nullptr != pParent)
    {
        int numOfChildren = pParent->childCount();

        // if no children - return the current checked state
        if (0 == numOfChildren)
        {
            // set the parent state
            const QString name = pParent->text(0);

            //set checked only HSA default check boxes
            if (m_pHSARadioButton->isChecked())
            {
                cs = HSADefaultCheckedList.contains(name) ? Qt::Checked : Qt::Unchecked;
                pParent->setCheckState(0, cs);
            }
            else
            {
                if (m_currentSettings.m_checkedCounterList.contains(name))
                {
                    cs = m_currentSettings.m_checkedCounterList[name] == true? Qt::Checked : Qt::Unchecked;
                }
                //by default set counter unchecked
                else
                {
                    //in internal builds disable check boxes by default
                    cs = (Util::IsInternalBuild() == true)? Qt::Unchecked : Qt::Checked;
                }
                pParent->setCheckState(0, cs);
            }
        }
        else
        {
            // go over all children
            for (int i = 0; i < numOfChildren; i++)
            {
                QTreeWidgetItem* child = pParent->child(i);

                // get the state of the child
                cs = GetItemCheckStateByChildrenState(child);

                if (Qt::Checked == cs)
                {
                    numOfChecked++;
                }
                else if (Qt::Unchecked == cs)
                {
                    numOfUnchecked++;
                }
            }

            // if all children are Checked - set checked
            if (numOfChildren == numOfChecked)
            {
                cs = Qt::Checked;
            }
            // if all children are Unchecked - set Unchecked
            else if (numOfChildren == numOfUnchecked)
            {
                cs = Qt::Unchecked;
            }
            else
            {
                // else - set PartiallyChecked
                cs = Qt::PartiallyChecked;
            }

            // set the parent state
            pParent->setCheckState(0, cs);
        }
    }

    return cs;
}

void CounterSelectionSettingWindow::showEvent(QShowEvent* pEvent)
{
    GT_UNREFERENCED_PARAMETER(pEvent);
    m_isPageOnDisplay = true;
    UpdateCountersTreeCheckState();
}

void CounterSelectionSettingWindow::hideEvent(QHideEvent* pEvent)
{
    GT_UNREFERENCED_PARAMETER(pEvent);
    m_isPageOnDisplay = false;
}

bool CounterSelectionSettingWindow::ValidateSpecificKernelsText()
{
    bool isValid = true;

    gtString kernels = acQStringToGTString(m_pSpecificKernelsEdit->text());
    kernels += AF_STR_Semicolon;
    gtStringTokenizer strTokenizer(kernels, AF_STR_Semicolon);
    gtString curKernel;

    QRegExp kernelNameExp(QString("[a-zA-Z_][a-zA-Z0-9_]+"));

    while (isValid && strTokenizer.getNextToken(curKernel))
    {
        curKernel = curKernel.trim();

        if (!curKernel.isEmpty())
        {
            isValid = kernelNameExp.exactMatch(acGTStringToQString(curKernel));
        }
    }

    return isValid;
}

void CounterSelectionSettingWindow::OnAPITypeRadioButtonToggled()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pOpenCLRadioButton != nullptr) && (m_pGpuTimeCollectCB != nullptr))
    {
        // Check if the OpenCL or HSA is selected:
        APIToTrace apiType = m_pOpenCLRadioButton->isChecked() ? APIToTrace_OPENCL : APIToTrace_HSA;
        m_pGpuTimeCollectCB->setEnabled(apiType == APIToTrace_OPENCL);

        // Uncheck the time collection check box for HSA:
        if (apiType == APIToTrace_HSA)
        {
            m_pGpuTimeCollectCB->setChecked(false);
        }

        UpdateTreeTopLevelCheckState(m_pCounterListTW);
    }
}

void CounterSelectionSettingWindow::DisableCountersForSinglePass()
{
    if (m_currentSettings.m_api == APIToTrace_HSA)
    {
        auto iter = m_hardwareFamilyTreeNodeMap.begin();
        auto iterEnd = m_hardwareFamilyTreeNodeMap.end();

        for (; iter != iterEnd; ++iter)
        {
            QStringList userSelectionEnabledCounters;
            QStringList singlePassEnabledCounters;
            FillTreeWidgetStringList(m_pCounterListTW, userSelectionEnabledCounters);

            QString hardwareFamilyDisplayName;
            QString strDeviceList;

            // Disable counters until the list contain only a single pass combination:
            bool rc = CounterManager::Instance()->GetHardwareFamilyPassCountLimitedCountersList(iter.key(), userSelectionEnabledCounters, 1, singlePassEnabledCounters);
            GT_ASSERT(rc);

            auto iterCounters = m_currentSettings.m_checkedCounterList.begin();
            auto iterCountersEnd = m_currentSettings.m_checkedCounterList.end();

            for (; iterCounters != iterCountersEnd; iterCounters++)
            {
                // Check if this counter should be selected / not selected:
                QString counterName = iterCounters.key();
                bool isEnabled = singlePassEnabledCounters.contains(counterName);
                m_currentSettings.m_checkedCounterList[counterName] = isEnabled;
            }
        }
    }
}
