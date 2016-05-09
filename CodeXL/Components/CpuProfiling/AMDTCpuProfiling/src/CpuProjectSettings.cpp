//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProjectSettings.cpp
/// \brief  The implementation of the project settings dialog and framework extension
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/src/CpuProjectSettings.cpp#118 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

#include <qtIgnoreCompilerWarnings.h>
#include <QtCore>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTApplicationComponents/Include/acItemDelegate.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTOSWrappers/Include/osCpuid.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afCSSSettings.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/src/afUtils.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/SharedProfileManager.h>
#include <AMDTSharedProfiling/inc/SharedProfileSettingPage.h>
#include <AMDTSharedProfiling/inc/StringConstants.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include <AMDTExecutableFormat/inc/PeFile.h>
    #include <Driver/Windows/CpuProf/inc/UserAccess/CpuProfDriver.h>
#endif

// Local:
#include <inc/StdAfx.h>
#include <inc/CpuProjectSettings.h>
#include <inc/StringConstants.h>
#include <inc/CpuProjectHandler.h>
#include <inc/ProfileConfigs.h>
#include <inc/CpuAffinityThread.h>
#include <inc/DisplayFilter.h>


#define CP_PROFILE_DURATION_DISABLE_MIN_VALUE 0
#define CP_PROFILE_DURATION_MIN_VALUE 1
#define CP_PROFILE_DURATION_MAX_VALUE 9999

// ----------------------------------------------------------------------------------
// Name:        CpuProjectSettings::CpuProjectSettings
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        5/2/2012
// ----------------------------------------------------------------------------------
CpuProjectSettings::CpuProjectSettings():
    afProjectSettingsExtension(),
    m_pEntireDurationRadio(nullptr),
    m_pProfilePausedRadio(nullptr),
    m_pProfileScheduledRadio(nullptr),
    m_pProfileType(nullptr),
    m_pStartAfterSpinBox(nullptr),
    m_pProfileDurationSpinBox(nullptr),
    m_pDurationLabel1(nullptr),
    m_pDurationLabel2(nullptr),
    m_pEndAfterCheckbox(nullptr),
    m_pDurationLabel4(nullptr),
    m_pTerminateAfterDataCollectionCheckBox(nullptr),
    m_pCodeExecInComboBox(nullptr),
    m_pCollectCSSCheckBox(nullptr),
    m_pUnwindDepthComboBox(nullptr),
    m_pOtherDepthComboBox(nullptr),
    m_pCollectionFrequencySpinBox(nullptr),
    m_pCSSLabel1(nullptr),
    m_pCSSLabel2(nullptr),
    m_pCSSLabel3(nullptr),
    m_pAffinityMaskText(nullptr),
    m_pCoresTree(nullptr),
    m_pCSSScopeLabel(nullptr),
    m_pCSSFpoSupportCheckBox(nullptr),
    m_pCSSFpoOtherCheckBox(nullptr)
{
    m_coreCount = 0;
    m_maxCoreMask = 0;

    bool rc = osGetAmountOfLocalMachineCPUs(m_coreCount);
    GT_IF_WITH_ASSERT(rc)
    {
        if (64 > m_coreCount)
        {
            m_maxCoreMask = (1ULL << m_coreCount) - 1;
        }
        else
        {
            m_maxCoreMask = GT_UINT64_MAX;
        }
    }

    rc = connect(SharedProfileSettingPage::Instance(), SIGNAL(SharedSettingsUpdated()), this, SLOT(OnSharedSettingsUpdated()));
    GT_ASSERT(rc);

    m_isInitialized = false;

    // Initialize the current profile type:
    int sessionTypeIndex = afExecutionModeManager::instance().activeSessionType();
    m_currentProfileTypeStr = acGTStringToQString(SharedProfileManager::instance().sessionTypeName(sessionTypeIndex));

}

// ----------------------------------------------------------------------------------
// Name:        CpuProjectSettings::~CpuProjectSettings
// Description:
// Author:  AMD Developer Tools Team
// Date:        5/2/2012
// ----------------------------------------------------------------------------------
CpuProjectSettings::~CpuProjectSettings()
{
}

void CpuProjectSettings::Initialize()
{
    QGridLayout* pMainLayout = new QGridLayout;

    // Create the OpenGL frame terminators group box:
    QLabel* pCaption1 = new QLabel(CP_STR_cpuProfileProjectSettingsCallStackCollectionCaption);

    pCaption1->setStyleSheet(AF_STR_captionLabelStyleSheetMain);
    int currentRow = 0;
    pMainLayout->addWidget(pCaption1, currentRow, 0, 1, 2);

    m_pCollectCSSCheckBox = new QCheckBox(CP_STR_cpuProfileProjectSettingsCallStackCollection);
    m_pCollectCSSCheckBox->setToolTip(CP_STR_cpuProfileProjectSettingsCallStackCollectionTooltip);
    currentRow++;
    pMainLayout->addWidget(m_pCollectCSSCheckBox, currentRow, 0, 1, 1);

    m_pProfileType = new QLabel();
    m_pProfileType->setWordWrap(true);

    pMainLayout->addWidget(m_pProfileType, currentRow, 1, 1, 1);

    // ******** CSS Layout *************
    currentRow++;
    QGridLayout* pCssShiftRightLayout = new QGridLayout;

    int gridRowcounter = 0;
    m_pCssRightLayoutWidget = new QWidget;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    // **** code executed in.. ***

    // combo
    m_pCodeExecInComboBox = new QComboBox;
    m_pCodeExecInComboBox->setInsertPolicy(QComboBox::InsertAtTop);
    m_pCodeExecInComboBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    m_pCodeExecInComboBox->setToolTip(CP_STR_cpuProfileProjectSettingsCallStackModeTooltip);
    m_pCodeExecInComboBox->addItem(CP_STR_cpuProfileProjectSettingsCallStackUserSpace, CP_CSS_SCOPE_USER);
    m_pCodeExecInComboBox->addItem(CP_STR_cpuProfileProjectSettingsCallStackKernelSpace, CP_CSS_SCOPE_KERNEL);
    m_pCodeExecInComboBox->addItem(CP_STR_cpuProfileProjectSettingsCallStackUserKernelSpaces, CP_CSS_SCOPE_ALL);

    // label
    m_pCSSScopeLabel = new QLabel(CP_STR_cpuProfileProjectSettingsCallStackCodeExecutedIn);

    // add to layout
    pCssShiftRightLayout->addWidget(m_pCSSScopeLabel, gridRowcounter, 0, Qt::AlignLeft);
    pCssShiftRightLayout->addWidget(m_pCodeExecInComboBox, gridRowcounter, 1, Qt::AlignLeft);

#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    // *** collect evry.. ***
    gridRowcounter++;
    m_pCSSLabel2 = new QLabel(CP_STR_cpuProfileProjectSettingsCallStackCollectionEvery);
    pCssShiftRightLayout->addWidget(m_pCSSLabel2, gridRowcounter, 0, Qt::AlignLeft);

    QHBoxLayout* pCollectEveryLayout = new QHBoxLayout;
    m_pCollectionFrequencySpinBox = new QSpinBox;
    m_pCollectionFrequencySpinBox->setToolTip(CP_STR_cpuProfileProjectSettingsCallStackCollectionEveryTooltip);
    m_pCollectionFrequencySpinBox->setRange(CP_CSS_MIN_UNWIND_INTERVAL, CP_CSS_MAX_UNWIND_INTERVAL);
    m_pCSSLabel3 = new QLabel("samples");

    pCollectEveryLayout->addWidget(m_pCollectionFrequencySpinBox, 0, Qt::AlignLeft);
    pCollectEveryLayout->addWidget(m_pCSSLabel3, 0, Qt::AlignLeft);
    pCollectEveryLayout->addStretch();
    pCssShiftRightLayout->addLayout(pCollectEveryLayout, gridRowcounter, 1, Qt::AlignLeft);

    // ****** depth collection ******
    gridRowcounter++;
    QLabel* tmpEmptyLabel = new QLabel;
    pCssShiftRightLayout->addWidget(tmpEmptyLabel, gridRowcounter, 0, Qt::AlignLeft);

    gridRowcounter++;
    m_pCSSLabel1 = new QLabel(CP_STR_cpuProfileProjectSettingsUnwindDepth);
    pCssShiftRightLayout->addWidget(m_pCSSLabel1, gridRowcounter, 0, Qt::AlignLeft);


    // *** Time based ***
    gridRowcounter++;
    // label
    QLabel* m_pTimeBaseLabel = new QLabel(CP_STR_cpuProfileProjectSettingTimeBasedSampling);
    pCssShiftRightLayout->addWidget(m_pTimeBaseLabel, gridRowcounter, 0, Qt::AlignLeft);

    QHBoxLayout* pLayoutTimeBased = new QHBoxLayout;

    // combo
    m_pUnwindDepthComboBox = new QComboBox;
    m_pUnwindDepthComboBox->setInsertPolicy(QComboBox::InsertAtTop);
    m_pUnwindDepthComboBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    // Build the string list for the CSS unwind depth:
    m_pUnwindDepthComboBox->addItem(QString(CP_STR_cpuProfileProjectSettingsDepthMinimal).arg(CP_CSS_MIN_UNWIND_DEPTH), QVariant(CP_CSS_MIN_UNWIND_DEPTH));
    m_pUnwindDepthComboBox->addItem(QString(CP_STR_cpuProfileProjectSettingsDepthLow).arg(CP_CSS_LOW_UNWIND_DEPTH), QVariant(CP_CSS_LOW_UNWIND_DEPTH));
    m_pUnwindDepthComboBox->addItem(QString(CP_STR_cpuProfileProjectSettingsDepthMedium).arg(CP_CSS_MEDIUM_UNWIND_DEPTH), QVariant(CP_CSS_MEDIUM_UNWIND_DEPTH));
    m_pUnwindDepthComboBox->addItem(QString(CP_STR_cpuProfileProjectSettingsDepthHigh).arg(CP_CSS_HIGH_UNWIND_DEPTH), QVariant(CP_CSS_HIGH_UNWIND_DEPTH));
    m_pUnwindDepthComboBox->addItem(QString(CP_STR_cpuProfileProjectSettingsDepthMaximal).arg(CP_CSS_MAX_UNWIND_DEPTH), QVariant(CP_CSS_MAX_UNWIND_DEPTH));

    m_pUnwindDepthComboBox->setCurrentText(QString(CP_STR_cpuProfileProjectSettingsDepthHigh).arg(CP_CSS_HIGH_UNWIND_DEPTH));
    m_pUnwindDepthComboBox->setToolTip(CP_STR_cpuProfileProjectSettingsDepthTooltip);
    pLayoutTimeBased->addWidget(m_pUnwindDepthComboBox);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    // checkbox
    m_pCSSFpoSupportCheckBox = new QCheckBox(CP_STR_cpuProfileProjectSettingsCallStackFpo);
    m_pCSSFpoSupportCheckBox->setToolTip(CP_STR_cpuProfileProjectSettingsCallStackFpoTooltip);
    m_pCSSFpoSupportCheckBox->setChecked(true);
    pLayoutTimeBased->addWidget(m_pCSSFpoSupportCheckBox);
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    pCssShiftRightLayout->addLayout(pLayoutTimeBased, gridRowcounter, 1, Qt::AlignLeft);

    // *** other cpu combo
    gridRowcounter ++;
    // label
    QLabel* m_pOtherLabel = new QLabel(CP_STR_cpuProfileProjectSettingOtherCpuProfiling);
    int labelSize = m_pOtherLabel->sizeHint().width() + 20;

    pCssShiftRightLayout->addWidget(m_pOtherLabel, gridRowcounter, 0, Qt::AlignLeft);

    QHBoxLayout* pLayoutOther = new QHBoxLayout;

    //combo
    m_pOtherDepthComboBox = new QComboBox;
    m_pOtherDepthComboBox->setInsertPolicy(QComboBox::InsertAtTop);
    m_pOtherDepthComboBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    // Build the string list for the CSS unwind depth:
    m_pOtherDepthComboBox->addItem(QString(CP_STR_cpuProfileProjectSettingsDepthMinimal).arg(CP_CSS_MIN_UNWIND_DEPTH), QVariant(CP_CSS_MIN_UNWIND_DEPTH));
    m_pOtherDepthComboBox->addItem(QString(CP_STR_cpuProfileProjectSettingsDepthLow).arg(CP_CSS_LOW_UNWIND_DEPTH), QVariant(CP_CSS_LOW_UNWIND_DEPTH));
    m_pOtherDepthComboBox->addItem(QString(CP_STR_cpuProfileProjectSettingsDepthMedium).arg(CP_CSS_MEDIUM_UNWIND_DEPTH), QVariant(CP_CSS_MEDIUM_UNWIND_DEPTH));
    m_pOtherDepthComboBox->addItem(QString(CP_STR_cpuProfileProjectSettingsDepthHigh).arg(CP_CSS_HIGH_UNWIND_DEPTH), QVariant(CP_CSS_HIGH_UNWIND_DEPTH));
    m_pOtherDepthComboBox->addItem(QString(CP_STR_cpuProfileProjectSettingsDepthMaximal).arg(CP_CSS_MAX_UNWIND_DEPTH), QVariant(CP_CSS_MAX_UNWIND_DEPTH));

    m_pOtherDepthComboBox->setCurrentText(QString(CP_STR_cpuProfileProjectSettingsDepthLow).arg(CP_CSS_LOW_UNWIND_DEPTH));
    m_pOtherDepthComboBox->setToolTip(CP_STR_cpuProfileProjectSettingsDepthOtherTooltip);

    pLayoutOther->addWidget(m_pOtherDepthComboBox);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    // checkbox
    m_pCSSFpoOtherCheckBox = new QCheckBox(CP_STR_cpuProfileProjectSettingsCallStackFpo);
    m_pCSSFpoOtherCheckBox->setToolTip(CP_STR_cpuProfileProjectSettingsCallStackFpoTooltip);
    pLayoutOther->addWidget(m_pCSSFpoOtherCheckBox);
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    pCssShiftRightLayout->addLayout(pLayoutOther, gridRowcounter, 1, Qt::AlignLeft);

    QHBoxLayout* pCssLayout = new QHBoxLayout;

    //add dummy checkbox for shifting right the pCssShiftRightLayout layout
    QCheckBox* pDummyEmptyCheckBox = new QCheckBox("");
    QSpacerItem* pSpaceItem = new QSpacerItem(pDummyEmptyCheckBox->sizeHint().width() - 4, pDummyEmptyCheckBox->sizeHint().height());
    pCssLayout->addSpacerItem(pSpaceItem);
    delete pDummyEmptyCheckBox;

    m_pCssRightLayoutWidget->setLayout(pCssShiftRightLayout);
    pCssLayout->addWidget(m_pCssRightLayoutWidget);
    pMainLayout->addLayout(pCssLayout, currentRow, 0, 1, 3);

    // ********** Collection Schedule *******
    QLabel* pCaption2 = new QLabel(PM_STR_sharedProfileSettingsDataCollectionSchedule);
    pCaption2->setToolTip(PM_STR_sharedProfileSettingsCollectDataScheduleTooltip);

    currentRow++;
    pCaption2->setStyleSheet(AF_STR_captionLabelStyleSheet);
    pMainLayout->addWidget(pCaption2, currentRow, 0, 1, 4);

    m_pEntireDurationRadio = new QRadioButton(PM_STR_sharedProfileSettingsEntireDuration);
    m_pEntireDurationRadio->setToolTip(PM_STR_sharedProfileSettingsEntireDurationTooltip);
    m_pProfilePausedRadio = new QRadioButton(PM_STR_sharedProfileSettingsProfilePaused);
    m_pProfilePausedRadio->setToolTip(PM_STR_sharedProfileSettingsProfilePausedTooltip);
    m_pProfileScheduledRadio = new QRadioButton(PM_STR_sharedProfileSettingsProfileScheduled);
    m_pProfileScheduledRadio->setToolTip(PM_STR_sharedProfileSettingsProfileScheduledTooltip);

    QButtonGroup* pGroup2 = new QButtonGroup;

    pGroup2->addButton(m_pEntireDurationRadio);
    pGroup2->addButton(m_pProfilePausedRadio);
    pGroup2->addButton(m_pProfileScheduledRadio);

    QVBoxLayout* pRadioLayout2 = new QVBoxLayout;


    pRadioLayout2->addWidget(m_pEntireDurationRadio);
    pRadioLayout2->addWidget(m_pProfilePausedRadio);
    pRadioLayout2->addWidget(m_pProfileScheduledRadio);

    // Add the radio buttons layout to the main layout:
    QLabel* pLabel3 = new QLabel(PM_STR_sharedProfileSettingsCollectDataSchedule);
    pLabel3->setMinimumWidth(labelSize);

    currentRow++;
    pMainLayout->addWidget(pLabel3, currentRow, 0, Qt::AlignLeft | Qt::AlignTop);
    pMainLayout->addLayout(pRadioLayout2, currentRow, 1);


    m_pDurationLabel1 = new QLabel(PM_STR_sharedProfileSettingsStartAfter);
    m_pStartAfterSpinBox = new QSpinBox;
    m_pStartAfterSpinBox->setToolTip(PM_STR_sharedProfileSettingsStartAfterTooltip);
    m_pDurationLabel2 = new QLabel(PM_STR_sharedProfileSettingsSeconds);


    QHBoxLayout* pLayoutStartData = new QHBoxLayout;

    m_pDurationLabel1->setContentsMargins(20, 0, 0, 0);
    pLayoutStartData->addWidget(m_pDurationLabel1, 0, Qt::AlignLeft);
    pLayoutStartData->addWidget(m_pStartAfterSpinBox, 0, Qt::AlignLeft);
    pLayoutStartData->addWidget(m_pDurationLabel2, 0, Qt::AlignLeft);
    pLayoutStartData->addStretch();
    currentRow++;
    pMainLayout->addLayout(pLayoutStartData, currentRow, 1, 1, 2);

    QHBoxLayout* pLayoutEndData = new QHBoxLayout;

    pLayoutEndData->setContentsMargins(20, 0, 0, 0);

    m_pEndAfterCheckbox = new QCheckBox(PM_STR_sharedProfileSettingsEndAfter);

    m_pProfileDurationSpinBox = new QSpinBox;
    m_pProfileDurationSpinBox->setToolTip(PM_STR_sharedProfileSettingsEndAfterTooltip);
    m_pDurationLabel4 = new QLabel(PM_STR_sharedProfileSettingsAdditionalSeconds);
    m_pDurationLabel4->setWordWrap(true);
    m_pTerminateAfterDataCollectionCheckBox = new QCheckBox(PM_STR_sharedProfileSettingsTerminateAfter);
    m_pTerminateAfterDataCollectionCheckBox->setToolTip(PM_STR_sharedProfileSettingsTerminateAfterTooltip);
    QHBoxLayout* pDummyLayout = new QHBoxLayout;
    pDummyLayout->setContentsMargins(20, 0, 0, 0);
    pDummyLayout->addWidget(m_pTerminateAfterDataCollectionCheckBox);

    pLayoutEndData->addWidget(m_pEndAfterCheckbox, 0, Qt::AlignLeft);
    pLayoutEndData->addWidget(m_pProfileDurationSpinBox, 0, Qt::AlignLeft);
    pLayoutEndData->addWidget(m_pDurationLabel4, 0, Qt::AlignLeft);
    pLayoutEndData->addStretch();

    currentRow++;
    pMainLayout->addLayout(pLayoutEndData, currentRow, 1);

    currentRow++;
    pMainLayout->addLayout(pDummyLayout, currentRow, 1);

    m_pStartAfterSpinBox->setRange(CP_PROFILE_DURATION_DISABLE_MIN_VALUE, CP_PROFILE_DURATION_MAX_VALUE);
    m_pProfileDurationSpinBox->setRange(CP_PROFILE_DURATION_DISABLE_MIN_VALUE, CP_PROFILE_DURATION_MAX_VALUE);

    int fourDigitsWidth = QFontMetrics(m_pProfileDurationSpinBox->font()).boundingRect("9999").width();
    m_pProfileDurationSpinBox->resize(fourDigitsWidth, -1);
    m_pStartAfterSpinBox->resize(fourDigitsWidth, -1);


    QLabel* pCaption3 = new QLabel(CP_STR_cpuProfileProjectSettingsHWScopeCaption);

    pCaption3->setStyleSheet(AF_STR_captionLabelStyleSheet);
    currentRow++;
    pMainLayout->addWidget(pCaption3, currentRow, 0, 1, 2);

    m_pCoresTree = new QTreeWidget;

    m_pCoresTree->setHeaderHidden(true);
    m_pCoresTree->setColumnCount(1);
    m_pCoresTree->setStyleSheet(AF_STR_treeWidgetWithBorderStyleSheet);
    m_pCoresTree->setItemDelegate(new acItemDelegate);

    // Set the vertical size policy according to the screen height:
    // When the vertical screen space is smaller then 800, we should shrink the tree, or it will cause the whole dialog to be to high,
    // in this case, the "Ok" button is not applicable.
    int screenH = QApplication::desktop()->screenGeometry().height();
    QSizePolicy::Policy treeVerticalSizePolicy = (screenH > 800) ? QSizePolicy::MinimumExpanding : QSizePolicy::Maximum;
    m_pCoresTree->setSizePolicy(QSizePolicy::MinimumExpanding, treeVerticalSizePolicy);
    m_pCoresTree->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // Build the cores tree:
    InitializeCoresTree();

    m_pAffinityMaskText = new QLineEdit;
    int maxWidth = m_pAffinityMaskText->fontMetrics().boundingRect("888888").width() + 8;
    m_pAffinityMaskText->setMaximumWidth(maxWidth);
    QGridLayout* hardwareScopeGrid = new QGridLayout;

    hardwareScopeGrid->addWidget(m_pCoresTree, 0, 0, 1, 2, Qt::AlignLeft);

    QHBoxLayout* pComboLayout = new QHBoxLayout;
    QLabel* pCPUAffinityLabel = new QLabel(CP_STR_cpuProfileProjectSettingsCPUAffinityMask);
    pCPUAffinityLabel->setToolTip(CP_STR_cpuProfileProjectSettingsCPUAffinityMaskTooltip);
    pComboLayout->addWidget(pCPUAffinityLabel, 0, Qt::AlignRight);
    pComboLayout->addWidget(m_pAffinityMaskText, 0, Qt::AlignRight);
    hardwareScopeGrid->addLayout(pComboLayout, 1, 0, 1, 2, Qt::AlignRight);

    currentRow++;
    pMainLayout->addLayout(hardwareScopeGrid, currentRow, 0, 1, 2, Qt::AlignLeft);

    // Add stretch:
    QLabel* pSpacer2 = new QLabel;
    pSpacer2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    currentRow++;
    pMainLayout->addWidget(pSpacer2, currentRow, 0, 1, 2);


    bool rc = connect(m_pAffinityMaskText, SIGNAL(textEdited(const QString&)), SLOT(OnAffinityTextEdited(const QString&)));
    GT_ASSERT(rc);

    rc = connect(m_pCoresTree, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(OnCoresTreeItemChanged(QTreeWidgetItem*, int)));
    GT_ASSERT(rc);

    rc = connect(SharedProfileSettingPage::Instance(), SIGNAL(ProfileTypeChanged(const QString&, const QString&)), this, SLOT(OnProfileTypeChanged(const QString&, const QString&)));
    GT_ASSERT(rc);

    rc = connect(m_pEndAfterCheckbox, SIGNAL(stateChanged(int)), this, SLOT(OnEndDataCollectionCheckBoxStateChanged()));
    GT_ASSERT(rc);

    // Connect radio buttons click to slots:
    rc = connect(m_pEntireDurationRadio, SIGNAL(clicked()), this, SLOT(OnScheduleRadioClick()));
    GT_ASSERT(rc);

    rc = connect(m_pProfilePausedRadio, SIGNAL(clicked()), this, SLOT(OnScheduleRadioClick()));
    GT_ASSERT(rc);

    rc = connect(m_pProfileScheduledRadio, SIGNAL(clicked()), this, SLOT(OnScheduleRadioClick()));
    GT_ASSERT(rc);

    //Initialize the gui values
    RestoreCurrentSettings();

    setLayout(pMainLayout);

    m_isInitialized = true;
}

// ----------------------------------------------------------------------------------
// Name:        CpuProjectSettings::ExtensionXMLString
// Description: Returns the name to set to the project settings tab
// Author:  AMD Developer Tools Team
// Date:        5/2/2012
// ----------------------------------------------------------------------------------
gtString CpuProjectSettings::ExtensionXMLString()
{
    gtString retVal = CPU_STR_PROJECT_EXTENSION;
    return retVal;
}

gtString CpuProjectSettings::ExtensionTreePathAsString()
{
    gtString retVal = CP_STR_cpuProfileTreePathString;
    return retVal;
}

// ----------------------------------------------------------------------------------
// Name:        CpuProjectSettings::getXMLSettingsString
// Description: Get the current debugger project settings as XML string
// Arguments:   gtASCIIString& projectAsXMLString
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        5/2/2012
// ----------------------------------------------------------------------------------
bool CpuProjectSettings::GetXMLSettingsString(gtString& projectAsXMLString)
{
    bool retVal = false;
    //get the XML string of the current options from the handler
    retVal = CpuProjectHandler::instance().getProjectSettingsXML(projectAsXMLString);
    return retVal;
}

// ----------------------------------------------------------------------------------
// Name:        CpuProjectSettings::setSettingsFromXMLString
// Description: Set the project settings from the XML string, delivered from the project file
// Arguments:   const gtString& projectAsXMLString
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        5/2/2012
// ----------------------------------------------------------------------------------
bool CpuProjectSettings::SetSettingsFromXMLString(const gtString& projectAsXMLString)
{
    bool retVal = false;

    //Load the string as the current settings for the handler
    retVal = CpuProjectHandler::instance().setProjectSettingsXML(projectAsXMLString);

    //Update gui from settings
    RestoreCurrentSettings();

    return retVal;
}

// ----------------------------------------------------------------------------------
// Name:        CpuProjectSettings::SaveCurrentSettings
// Description: Get the current project settings from the controls, and store into
//              the current project properties
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        5/2/2012
// ----------------------------------------------------------------------------------
bool CpuProjectSettings::SaveCurrentSettings()
{
    bool retVal = false;

    //Update current settings from controls
    CPUSessionTreeItemData* pSettings = CpuProjectHandler::instance().getProjectSettings();
    retVal = (nullptr != pSettings);

    GT_IF_WITH_ASSERT(retVal)
    {
        pSettings->m_startAffinity = m_pAffinityMaskText->text().toULongLong(nullptr, 0);
        pSettings->SetShouldCollectCSS(m_pCollectCSSCheckBox->isChecked());
        pSettings->m_cssInterval = m_pCollectionFrequencySpinBox->value();

        if (nullptr != m_pCodeExecInComboBox)
        {
            pSettings->m_cssScope = static_cast<CpuProfileCssScope>((m_pCodeExecInComboBox->currentData()).toInt());
        }

        if (nullptr != m_pCSSFpoSupportCheckBox)
        {
            pSettings->m_isTimeBasedCssSupportFpo = m_pCSSFpoSupportCheckBox->isChecked();
        }

        if (nullptr != m_pCSSFpoOtherCheckBox)
        {
            pSettings->m_isOtherCpuCssSupportFpo = m_pCSSFpoOtherCheckBox->isChecked();
        }

        if (nullptr != m_pUnwindDepthComboBox)
        {
            pSettings->m_timeBasedCssDepthLevel = m_pUnwindDepthComboBox->currentData().toUInt();
        }

        if (nullptr != m_pOtherDepthComboBox)
        {
            pSettings->m_otherCpuCssDepthLevel = m_pOtherDepthComboBox->currentData().toUInt();
        }

        // Koushik: BUG367296
        // Save the current settings in CpuProjectHandler::m_savedCurrentSettings
        CPUSessionTreeItemData* pSavedSettings = CpuProjectHandler::instance().getSavedProjectSettings();

        pSavedSettings->CopyFrom(pSettings, false);

        SharedProfileSettingPage::Instance()->CurrentSharedProfileSettings().m_shouldProfileEntireDuration = m_pEntireDurationRadio->isChecked();
        SharedProfileSettingPage::Instance()->CurrentSharedProfileSettings().m_terminateAfterDataCollectionIsDone = m_pTerminateAfterDataCollectionCheckBox->isChecked();
        SharedProfileSettingPage::Instance()->CurrentSharedProfileSettings().m_isProfilePaused = m_pProfilePausedRadio->isChecked();
        SharedProfileSettingPage::Instance()->CurrentSharedProfileSettings().m_startDelay = m_pStartAfterSpinBox->value();
        SharedProfileSettingPage::Instance()->CurrentSharedProfileSettings().m_profileDuration = m_pProfileDurationSpinBox->value();

        pSettings->CopyFrom(SharedProfileSettingPage::Instance()->CurrentSharedProfileSettings());
    }

    return retVal;
}

bool CpuProjectSettings::RestoreCurrentSettings()
{
    CPUSessionTreeItemData* pSettings = CpuProjectHandler::instance().getProjectSettings();

    // Sanity check:
    if (m_isInitialized)
    {
        // Update current settings from controls:
        m_pAffinityMaskText->setText("0x" + QString::number(pSettings->m_startAffinity, 16));

        for (int i = 0; i < m_pCoresTree->topLevelItemCount(); i++)
        {
            CPUTreeItem* pCPUItem = dynamic_cast<CPUTreeItem*>(m_pCoresTree->topLevelItem(i));

            if (pCPUItem != nullptr)
            {
                pCPUItem->SetMask(pSettings->m_startAffinity);
            }
        }

        // Get the original CSS collection setting:
        bool shouldCollectCSS = pSettings->ShouldCollectCSS(false);
        m_pCollectCSSCheckBox->setChecked(shouldCollectCSS);

        m_pCollectionFrequencySpinBox->setValue(pSettings->m_cssInterval);

        if (nullptr != m_pCodeExecInComboBox)
        {
            m_pCodeExecInComboBox->setCurrentIndex((pSettings->m_cssScope) - CP_CSS_SCOPE_USER);
        }

        if (nullptr != m_pCSSFpoSupportCheckBox)
        {
            m_pCSSFpoSupportCheckBox->setChecked(pSettings->m_isTimeBasedCssSupportFpo);
        }

        if (nullptr != m_pCSSFpoOtherCheckBox)
        {
            m_pCSSFpoOtherCheckBox->setChecked(pSettings->m_isOtherCpuCssSupportFpo);
        }

        int index = m_pUnwindDepthComboBox->findData(pSettings->m_timeBasedCssDepthLevel);
        m_pUnwindDepthComboBox->setCurrentIndex(index);

        index = m_pOtherDepthComboBox->findData(pSettings->m_otherCpuCssDepthLevel);
        m_pOtherDepthComboBox->setCurrentIndex(index);

        const SessionTreeNodeData& sharedProfileSettings = SharedProfileSettingPage::Instance()->CurrentSharedProfileSettings();
        int startDelayValue = (sharedProfileSettings.m_startDelay >= 0) ? sharedProfileSettings.m_startDelay : 0;
        int durationValue = (sharedProfileSettings.m_profileDuration >= 0) ? sharedProfileSettings.m_profileDuration : 0;

        m_pStartAfterSpinBox->setValue(startDelayValue);

        disconnect(m_pEndAfterCheckbox, SIGNAL(stateChanged(int)), this, SLOT(OnEndDataCollectionCheckBoxStateChanged()));

        m_pEndAfterCheckbox->setChecked(durationValue > 0);
        m_pProfileDurationSpinBox->setValue(durationValue);
        // loading saved settings, can assume that duration has value when m_pEndAfterCheckbox is enabled and checked
        m_pProfileDurationSpinBox->setEnabled(durationValue > 0);

        m_pProfilePausedRadio->setChecked(sharedProfileSettings.m_isProfilePaused);

        if (!sharedProfileSettings.m_isProfilePaused)
        {
            bool isScheduled = (sharedProfileSettings.m_startDelay > 0) || (sharedProfileSettings.m_profileDuration > 0);

            m_pEntireDurationRadio->setChecked(!isScheduled);
            m_pProfileScheduledRadio->setChecked(isScheduled);
        }

        m_pTerminateAfterDataCollectionCheckBox->setChecked(sharedProfileSettings.m_terminateAfterDataCollectionIsDone);

        bool rc = connect(m_pEndAfterCheckbox, SIGNAL(stateChanged(int)), this, SLOT(OnEndDataCollectionCheckBoxStateChanged()));
        GT_ASSERT(rc);

        OnScheduleRadioClick();

        bool isUserModelId = !afProjectManager::instance().currentProjectSettings().windowsStoreAppUserModelID().isEmpty();
        QString exeFullPath = acGTStringToQString(afProjectManager::instance().currentProjectSettings().executablePath().asString());
        OnExecutableChanged(exeFullPath, true, isUserModelId);
    }

    return true;
}

// ----------------------------------------------------------------------------------
// Name:        CpuProjectSettings::RestoreDefaultProjectSettings
// Description: Restore default project settings
// Author:  AMD Developer Tools Team
// Date:        5/2/2012
// ----------------------------------------------------------------------------------
void CpuProjectSettings::RestoreDefaultProjectSettings()
{
    //Default settings are set in the constructor
    CPUSessionTreeItemData restore;
    CPUSessionTreeItemData* pSettings = CpuProjectHandler::instance().getProjectSettings();


    pSettings->CopyFrom(&restore, false);
    RestoreCurrentSettings();
}

// ----------------------------------------------------------------------------------
// Name:        CpuProjectSettings::AreSettingsValid
// Description: Check if the current settings are valid
// Arguments:   gtString& invalidMessageStr
// Return Val:  bool - Whether settings are valid
// Author:  AMD Developer Tools Team
// Date:        5/2/2012
// ----------------------------------------------------------------------------------
bool CpuProjectSettings::AreSettingsValid(gtString& invalidMessageStr)
{
    bool retVal = true;

    //if 0 != affinity
    gtUInt64 afMask = m_pAffinityMaskText->text().toULongLong(nullptr, 0);

    retVal = retVal && (0 != afMask);

    if (!retVal)
    {
        invalidMessageStr = L"No CPUs or cores are selected for CPU profile. Please select at least one cpu or core to be viewed";
        m_pAffinityMaskText->setFocus();
    }

    return retVal;
}

void CpuProjectSettings::OnAffinityTextEdited(const QString& text)
{
    (void)(text); // unused

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pAffinityMaskText != nullptr)
    {
        gtUInt64 afMask = m_pAffinityMaskText->text().toULongLong(nullptr, 0);

        // If the entered mask is greater than the allowed max, reset it
        if (afMask > m_maxCoreMask)
        {
            m_pAffinityMaskText->setText("0x" + QString::number(m_maxCoreMask, 16));
            afMask = m_maxCoreMask;
        }

        // Update the tree with the mask:
        UpdateCoreTreeMask(afMask);
    }
}

void CpuProjectSettings::OnExecutableChanged(const QString& exePath, bool isChangeFinal, bool isUserModelId)
{
    // will be called only when exe selection changed in profoile settings -
    // not in new exe run and not on attach to new exe in current project
    // this function called every time we open the settings page

    GT_UNREFERENCED_PARAMETER(isChangeFinal);
    GT_UNREFERENCED_PARAMETER(isUserModelId);

    // Set the current exe path:
    m_currentExePath = exePath;

    // Enable / disable the CSS check box:
    EnableCSSCheckBox();

}

void CpuProjectSettings::InitializeCoresTree()
{
    // Create topology map to read the cores from:
    CoreTopologyMap tempTopology;

    // For each core, use the CpuAffinityThread to find the processor and node:
    for (int count = 0; count < m_coreCount; count++)
    {
        CoreTopology oneCore;
        oneCore.numaNode = 0;
        oneCore.processor = 0;

        CpuAffinityThread threadAff(count, &oneCore);
        threadAff.execute();

        // Wait for the thread to finish.  It should take 1 ms at the most:
        while (threadAff.isAlive())
        {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
            SwitchToThread();
#endif
        }

        tempTopology.insert(CoreTopologyMap::value_type(count, oneCore));
    }

    // Iterate the core in the topology map:
    QMap<int, QTreeWidgetItem*> sockMap;
    CoreTopologyMap::iterator it = tempTopology.begin();

    for (; it != tempTopology.end(); it++)
    {

        QString cpuName = CP_STR_cpuProfileProjectSettingsCPUPrefix + QString::number(it->second.processor);
        QTreeWidgetItem* pCPUItem = nullptr;

        if (!sockMap.contains(it->second.processor))
        {
            pCPUItem = new CPUTreeItem;

            pCPUItem->setText(0, cpuName);

            m_pCoresTree->addTopLevelItem(pCPUItem);
            pCPUItem->setFlags(pCPUItem->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            pCPUItem->setCheckState(0, Qt::Checked);
            sockMap.insert(it->second.processor, pCPUItem);
        }
        else
        {
            pCPUItem = sockMap[it->second.processor];
        }

        GT_IF_WITH_ASSERT(pCPUItem != nullptr)
        {
            // Get the core number and build name:
            int core = it->first;
            QString coreName = CP_STR_cpuProfileProjectSettingsCorePrefix + QString::number(core);

            CoreTreeItem* pCoreItem = new CoreTreeItem;

            pCoreItem->setText(0, coreName);
            pCoreItem->m_mask = 1ULL << it->first;

            pCoreItem->setFlags(pCPUItem->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            pCoreItem->setCheckState(0, Qt::Checked);

            pCPUItem->addChild(pCoreItem);
        }
    }

    m_pCoresTree->expandAll();
}

void CpuProjectSettings::UpdateCoreTreeMask(gtUInt64 afMask)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pCoresTree != nullptr)
    {
        for (int i = 0; i < m_pCoresTree->topLevelItemCount(); i++)
        {
            CPUTreeItem* pCPUItem = dynamic_cast<CPUTreeItem*>(m_pCoresTree->topLevelItem(i));

            if (pCPUItem != nullptr)
            {
                pCPUItem->SetMask(afMask);
            }
        }
    }
}


void CpuProjectSettings::OnCoresTreeItemChanged(QTreeWidgetItem* pItem, int column)
{
    Q_UNUSED(pItem);
    Q_UNUSED(column);

    gtUInt64 mask = 0;

    QTreeWidgetItem* pParent = pItem;

    while (pParent->parent() != nullptr)
    {
        pParent = pParent->parent();
    }

    m_pCoresTree->blockSignals(true);
    UpdateTreeWidgetItemCheckState(pItem);
    m_pCoresTree->blockSignals(false);


    // Sanity check:
    GT_IF_WITH_ASSERT(m_pCoresTree != nullptr)
    {
        // Go over the CPUs and cores, and update the CPU affinity mask:
        for (int i = 0 ; i < m_pCoresTree->topLevelItemCount(); i++)
        {
            CPUTreeItem* pCPUItem = dynamic_cast<CPUTreeItem*>(m_pCoresTree->topLevelItem(i));

            if (pCPUItem != nullptr)
            {
                gtUInt64 currentCPUItemMask = pCPUItem->GetMask();
                mask |= currentCPUItemMask;
            }
        }
    }

    // Set the mask as text:
    m_pAffinityMaskText->setText("0x" + QString::number(mask, 16));

}

void CpuProjectSettings::UpdateTreeWidgetItemCheckState(QTreeWidgetItem* pTreeWidgetItem)
{
    if (pTreeWidgetItem != nullptr)
    {
        Qt::CheckState checkState = pTreeWidgetItem->checkState(0);

        // first set all children to have the same state as this node
        for (int i = 0; i < pTreeWidgetItem->childCount(); i++)
        {
            bool doUpdateCount = pTreeWidgetItem->child(i)->checkState(0) != checkState;

            if (doUpdateCount)
            {
                pTreeWidgetItem->child(i)->setCheckState(0, checkState);
            }

            UpdateTreeWidgetItemCheckState(pTreeWidgetItem->child(i));
        }

        // now walk the parent chain to check the parent's check state
        UpdateParentTreeWidgetItemCheckState(pTreeWidgetItem->parent());
    }
}

void CpuProjectSettings::UpdateParentTreeWidgetItemCheckState(QTreeWidgetItem* pTreeWidgetItem)
{
    if (pTreeWidgetItem != nullptr)
    {

        Qt::CheckState checkState = Qt::Unchecked;

        for (int i = 0; i < pTreeWidgetItem->childCount(); i++)
        {
            if (i == 0)
            {
                checkState = pTreeWidgetItem->child(i)->checkState(0);
            }
            else
            {
                if (checkState != pTreeWidgetItem->child(i)->checkState(0))
                {
                    checkState = Qt::PartiallyChecked;
                    break;
                }
            }
        }

        pTreeWidgetItem->setCheckState(0, checkState);

        UpdateParentTreeWidgetItemCheckState(pTreeWidgetItem->parent());
    }
}

void CpuProjectSettings::OnScheduleRadioClick()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pProfilePausedRadio != nullptr) && (m_pStartAfterSpinBox != nullptr) && (m_pEntireDurationRadio != nullptr) &&
                      (m_pProfileDurationSpinBox != nullptr) && (m_pTerminateAfterDataCollectionCheckBox != nullptr) &&
                      (m_pDurationLabel1 != nullptr) && (m_pDurationLabel2 != nullptr) && (m_pEndAfterCheckbox != nullptr) && (m_pDurationLabel4 != nullptr))
    {
        // Check if entire duration should be profiled:
        bool shouldEnableScheduleWidgets = m_pProfileScheduledRadio->isChecked();
        m_pStartAfterSpinBox->setEnabled(shouldEnableScheduleWidgets);
        m_pDurationLabel1->setEnabled(shouldEnableScheduleWidgets);
        m_pDurationLabel2->setEnabled(shouldEnableScheduleWidgets);
        m_pEndAfterCheckbox->setEnabled(shouldEnableScheduleWidgets);

        if (sender() == m_pEntireDurationRadio ||
            sender() == m_pProfilePausedRadio ||
            sender() == m_pProfileScheduledRadio)
        {
            OnEndDataCollectionCheckBoxStateChanged();
        }

        if (!shouldEnableScheduleWidgets)
        {
            // Restore the default values for the schedule controls:
            m_pStartAfterSpinBox->setValue(0);
            m_pProfileDurationSpinBox->setValue(0);
            m_pEndAfterCheckbox->setChecked(Qt::Unchecked);
            m_pTerminateAfterDataCollectionCheckBox->setChecked(Qt::Unchecked);
        }
    }
}
void CpuProjectSettings::OnEndDataCollectionCheckBoxStateChanged()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pProfileDurationSpinBox != nullptr) && (m_pDurationLabel4 != nullptr) && (m_pTerminateAfterDataCollectionCheckBox != nullptr) && (m_pEndAfterCheckbox != nullptr))
    {
        bool shouldEnable = m_pEndAfterCheckbox->isEnabled() && m_pEndAfterCheckbox->isChecked();

        m_pProfileDurationSpinBox->setEnabled(shouldEnable);
        m_pDurationLabel4->setEnabled(shouldEnable);
        m_pTerminateAfterDataCollectionCheckBox->setEnabled(shouldEnable);

        if (shouldEnable)
        {
            m_pProfileDurationSpinBox->setRange(CP_PROFILE_DURATION_MIN_VALUE, CP_PROFILE_DURATION_MAX_VALUE);
            m_pProfileDurationSpinBox->setValue(CP_PROFILE_DURATION_MIN_VALUE);
        }
        else
        {
            m_pProfileDurationSpinBox->setRange(CP_PROFILE_DURATION_DISABLE_MIN_VALUE, CP_PROFILE_DURATION_MAX_VALUE);
            m_pProfileDurationSpinBox->setValue(CP_PROFILE_DURATION_DISABLE_MIN_VALUE);
        }
    }
}

void CpuProjectSettings::OnProfileTypeChanged(const QString& oldProfileType, const QString& newProfileType)
{
    GT_UNREFERENCED_PARAMETER(oldProfileType);

    // Sanity check:
    GT_IF_WITH_ASSERT(nullptr != m_pCollectCSSCheckBox)
    {
        // Update profile type label:
        m_pProfileType->setText(QString(CP_STR_cpuProfileProjectSettingsCurrentProfileType).arg(newProfileType));

        // Set the new profile type:
        m_currentProfileTypeStr = newProfileType;

        // Enable / disable the CSS check box:
        EnableCSSCheckBox();

    }
}

void CpuProjectSettings::OnSharedSettingsUpdated()
{
    RestoreCurrentSettings();
}

void CpuProjectSettings::EnableCSSCheckBox()
{
    // Check if the current executable supports CSS:
    bool isExeSupportingCSS = !CPUSessionTreeItemData::ShouldDisableCSS(m_currentExePath);

    // Check if the current profile type supports CSS:
    bool isProfileTypeSupportingCSS = m_currentProfileTypeStr.startsWith("CPU");

    if (m_currentProfileTypeStr.endsWith(PM_profileTypeCLU))
    {
        isProfileTypeSupportingCSS = false;
    }

    bool shouleEnable = isProfileTypeSupportingCSS && isExeSupportingCSS;

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pCollectCSSCheckBox != nullptr) && (m_pCssRightLayoutWidget != nullptr) && (m_pProfileType != nullptr))
    {
        m_pCollectCSSCheckBox->setEnabled(shouleEnable);
        m_pCollectCSSCheckBox->setChecked(shouleEnable);
        m_pCssRightLayoutWidget->setEnabled(shouleEnable);
        m_pProfileType->setEnabled(shouleEnable);
    }
}

gtUInt64 CPUTreeItem::GetMask()
{
    gtUInt64 mask = 0;

    for (int i = 0; i < childCount(); i++)
    {
        CoreTreeItem* pChild = dynamic_cast<CoreTreeItem*>(child(i));

        if (pChild != nullptr)
        {
            if (pChild->checkState(0) == Qt::Checked)
            {
                mask |= pChild->m_mask;
            }
        }
    }

    return mask;
}

void CPUTreeItem::SetMask(gtUInt64 mask)
{
    for (int i = 0; i < childCount(); i++)
    {
        CoreTreeItem* pChild = dynamic_cast<CoreTreeItem*>(child(i));

        if (pChild != nullptr)
        {
            bool shouldCheck = ((mask & pChild->m_mask) > 0);
            Qt::CheckState state = shouldCheck ? Qt::Checked : Qt::Unchecked;
            pChild->setCheckState(0, state);
        }
    }
}

