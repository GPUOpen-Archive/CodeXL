//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CustomProfileProjectSettingsExtension.cpp
///
//==================================================================================

//------------------------------ CustomProfileProjectSettingsExtension.h ------------------------------

// QT:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTApplicationComponents/Include/acItemDelegate.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTOSWrappers/Include/osCpuid.h>
#include <AMDTAPIClasses/Include/Events/apExecutionModeChangedEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afCSSSettings.h>
#include <AMDTApplicationFramework/Include/afNewProjectDialog.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>

// CPU Profile backend:
#include <AMDTCpuProfilingControl/inc/CpuProfileControl.h>
#include <AMDTCpuPerfEventUtils/inc/EventEngine.h>

// Shared profiling tool:
#include <SharedProfileManager.h>
#include <StringConstants.h>

// Local:
#include <inc/StdAfx.h>
#include <inc/ProfileConfigs.h>
#include <inc/CpuProjectHandler.h>
#include <inc/StringConstants.h>
#include <inc/CustomProfileProjectSettingsExtension.h>
#include <inc/StringConstants.h>


const unsigned int DEFAULT_IBS_INTERVAL = 250000;
bool CustomProfileProjectSettingsExtension::m_sWasConnectedToTree = false;

CustomProfileProjectSettingsExtension::CustomProfileProjectSettingsExtension():
    afProjectSettingsExtension(),
    m_pEventFile(nullptr), m_pAvailableTree(nullptr), m_pMonitoredEventsTreeWidget(nullptr), m_pAddEvent(nullptr), m_pRemoveEvent(nullptr),
    m_pRemoveAll(nullptr), m_pDescription(nullptr), m_pSettingsLabel(nullptr), m_pSettings(nullptr), m_pDescriptionTitle(nullptr),
    m_pErrLabel(nullptr), m_pDipatchRadio(nullptr), m_pCycleRadio(nullptr), m_maxEbpEvents(0), m_wereSettingsChanged(false)
{
}

// ---------------------------------------------------------------------------
// Name:        CustomProfileProjectSettingsExtension::~CustomProfileProjectSettingsExtension
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        4/4/2012
// ---------------------------------------------------------------------------
CustomProfileProjectSettingsExtension::~CustomProfileProjectSettingsExtension()
{
}

// ---------------------------------------------------------------------------
// Name:        CustomProfileProjectSettingsExtension::initialize
// Description: Create the widget that is reading the debug setting for the debugger
// Author:  AMD Developer Tools Team
// Date:        4/4/2012
// ---------------------------------------------------------------------------
void CustomProfileProjectSettingsExtension::Initialize()
{
    QGridLayout* pMainLayout = new QGridLayout;


    QLabel* pCaption1 = new QLabel(CP_STR_cpuProfileProjectSettingsCaption);

    pCaption1->setStyleSheet(AF_STR_captionLabelStyleSheetMain);
    pMainLayout->addWidget(pCaption1, 0, 0, 1, 3);

    QLabel* pLabel1 = new QLabel(CP_STR_cpuProfileProjectSettingsMonitoredEventsDetails);

    pMainLayout->addWidget(pLabel1, 1, 0, 1, 3);

    QLabel* pLabel2 = new QLabel(CP_STR_cpuProfileProjectSettingsAvailableEvents);

    pMainLayout->addWidget(pLabel2, 2, 0, 1, 2);

    QLabel* pLabel3 = new QLabel(CP_STR_cpuProfileProjectSettingsMonitoredEvents);

    pMainLayout->addWidget(pLabel3, 2, 2, 1, 1);

    QVBoxLayout* pButtonsLayout = new QVBoxLayout;


    pButtonsLayout->addStretch();
    m_pAddEvent = new QPushButton(AF_STR_AddButton);

    pButtonsLayout->addWidget(m_pAddEvent);

    m_pRemoveEvent = new QPushButton(AF_STR_RemoveButton);

    pButtonsLayout->addWidget(m_pRemoveEvent);

    m_pRemoveAll = new QPushButton(AF_STR_RemoveAllButton);

    pButtonsLayout->addWidget(m_pRemoveAll);

    pButtonsLayout->addStretch();

    m_pAvailableTree = new QTreeWidget;

    m_pAvailableTree->setHeaderHidden(true);
    m_pAvailableTree->setColumnCount(1);
    m_pAvailableTree->setStyleSheet(AF_STR_treeWidgetWithBorderStyleSheet);
    m_pAvailableTree->setItemDelegate(new acItemDelegate);
    pMainLayout->addWidget(m_pAvailableTree, 3, 0, 1, 1);

    pMainLayout->addLayout(pButtonsLayout, 3, 1, 1, 1);

    m_pMonitoredEventsTreeWidget = new QTreeWidget;


    QStringList headerCaptions, headerTooltips;
    headerCaptions << CP_STR_cpuProfileProjectSettingsNameColumn;
    headerCaptions << CP_STR_cpuProfileProjectSettingsIntervalColumn;
    headerCaptions << CP_STR_cpuProfileProjectSettingsUnitMaskColumn;
    headerCaptions << CP_STR_cpuProfileProjectSettingsUsrColumn;
    headerCaptions << CP_STR_cpuProfileProjectSettingsOSColumn;

    headerTooltips << CP_STR_cpuProfileProjectSettingsNameColumnTooltip;
    headerTooltips << CP_STR_cpuProfileProjectSettingsIntervalColumnTooltip;
    headerTooltips << CP_STR_cpuProfileProjectSettingsUnitMaskColumnTooltip;
    headerTooltips << CP_STR_cpuProfileProjectSettingsUsrColumnTooltip;
    headerTooltips << CP_STR_cpuProfileProjectSettingsOSColumnTooltip;

    m_pMonitoredEventsTreeWidget->setHeaderLabels(headerCaptions);

    // Set the column header tooltips
    int colIndex = 0;

    for (auto tooltip : headerTooltips)
    {
        m_pMonitoredEventsTreeWidget->headerItem()->setToolTip(colIndex, tooltip);
    }

    // Resize the columns to content
    m_pMonitoredEventsTreeWidget->header()->resizeSections(QHeaderView::ResizeToContents);


    m_pMonitoredEventsTreeWidget->setStyleSheet(AF_STR_treeWidgetWithBorderStyleSheet);
    m_pMonitoredEventsTreeWidget->setItemDelegate(new acItemDelegate);
    m_pMonitoredEventsTreeWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pMainLayout->addWidget(m_pMonitoredEventsTreeWidget, 3, 2, 1, 1);

    m_pDescriptionTitle = new QLabel;

    pMainLayout->addWidget(m_pDescriptionTitle, 4, 0, 1, 2);
    m_pDescriptionTitle->setStyleSheet(AF_STR_BlueFont);


    QVBoxLayout* pSettingsLayout = new QVBoxLayout;

    m_pSettingsLabel = new QLabel("Event Settings");


    m_pSettings = new QScrollArea;

    m_pSettings->setStyleSheet(AF_STR_WhiteBG);
    pSettingsLayout->addWidget(m_pSettingsLabel);
    pSettingsLayout->addWidget(m_pSettings);
    pMainLayout->addLayout(pSettingsLayout, 4, 2, 2, 1);

    m_pDescription = new QPlainTextEdit;

    pMainLayout->addWidget(m_pDescription, 5, 0, 1, 2);

    m_pErrLabel = new QLabel;


    pMainLayout->addWidget(m_pErrLabel, 6, 0, 1, 3);

    pMainLayout->setColumnStretch(0, 0);
    pMainLayout->setColumnStretch(1, 0);
    pMainLayout->setColumnStretch(2, 1);

    setLayout(pMainLayout);

    m_pDescription->setFrameShape(QFrame::NoFrame);
    m_pSettings->setFrameShape(QFrame::NoFrame);

    //Hide the columns for the available tree
    m_pAvailableTree->headerItem()->setHidden(true);

    for (int i = 0; i < MAX_UNITMASK; i++)
    {
        m_pUnitMasks[i] = nullptr;
    }

    bool rc = connect(m_pAddEvent, SIGNAL(clicked()), this, SLOT(OnAddEvent()));
    GT_ASSERT(rc);

    rc = connect(m_pAvailableTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(OnAvailableEventDoubleClick(QTreeWidgetItem*)));
    GT_ASSERT(rc);

    rc = connect(m_pRemoveEvent, SIGNAL(clicked()), this, SLOT(OnRemoveEvent()));
    GT_ASSERT(rc);

    rc = connect(m_pRemoveAll, SIGNAL(clicked()), this, SLOT(OnRemoveAll()));
    GT_ASSERT(rc);

    rc = connect(m_pMonitoredEventsTreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(OnMonitoredEventDoubleClick(QTreeWidgetItem*, int)));
    GT_ASSERT(rc);

    rc = connect(m_pMonitoredEventsTreeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*,  QTreeWidgetItem*)), this, SLOT(OnMonitoredItemChanged(QTreeWidgetItem*)));
    GT_ASSERT(rc);

    rc = connect(m_pMonitoredEventsTreeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(OnEventEdited(QTreeWidgetItem*, int)));
    GT_ASSERT(rc);

    rc = connect(m_pAvailableTree, SIGNAL(currentItemChanged(QTreeWidgetItem*,  QTreeWidgetItem*)), this, SLOT(OnAvailableItemChanged(QTreeWidgetItem*)));
    GT_ASSERT(rc);

    // Initialize the events tree from the current profile type:
    InitializeFromProfileType(acGTStringToQString(SharedProfileManager::instance().currentSelection()));

    OnUpdateRemoveAll();
}

gtString CustomProfileProjectSettingsExtension::ExtensionXMLString()
{
    return CPU_STR_PROJECT_EXTENSION_CUSTOM;
}

gtString CustomProfileProjectSettingsExtension::ExtensionTreePathAsString()
{
    return CP_STR_cpuProfileCustomTreePathString;
}

bool CustomProfileProjectSettingsExtension::GetXMLSettingsString(gtString& projectAsXMLString)
{
    bool retVal = true;

    // Currently the custom page is not written to the XML. These are actualy global settings.
    // We skip this functionality:
    projectAsXMLString.appendFormattedString(L"<%ls>", ExtensionXMLString().asCharArray());
    projectAsXMLString.appendFormattedString(L"</%ls>", ExtensionXMLString().asCharArray());

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        CustomProfileProjectSettingsExtension::setSettingsFromXMLString
// Description: Get the project settings from the XML string
// Arguments:   const gtString& projectAsXMLString
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        8/4/2012
// ---------------------------------------------------------------------------
bool CustomProfileProjectSettingsExtension::SetSettingsFromXMLString(const gtString& projectAsXMLString)
{
    (void)(projectAsXMLString); // unused
    bool retVal = true;
    // Currently the custom page is not written to the XML. These are actualy global settings.
    // We skip this functionality:
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        CustomProfileProjectSettingsExtension::SaveCurrentSettings
// Description: Get the current project settings from the controls, and store into
//              the current project properties
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        4/4/2012
// ---------------------------------------------------------------------------
bool CustomProfileProjectSettingsExtension::SaveCurrentSettings()
{
    bool retVal = true;

    unsigned int ebpCount = 0;

    for (int i = 0, count = m_pMonitoredEventsTreeWidget->topLevelItemCount(); i < count; ++i)
    {
        QTreeWidgetItem* pItem = m_pMonitoredEventsTreeWidget->topLevelItem(i);
        gtUInt16 eventSelect = (pItem->type() - QTreeWidgetItem::UserType);

        if (IsPmcEvent(eventSelect) || IsL2IEvent(eventSelect))
        {
            ebpCount++;
        }
    }

    unsigned int ebpIndex = 0;
    CPUSessionTreeItemData opts;

    if (0 != ebpCount)
    {
        for (unsigned int i = 0; i < ebpCount; i++)
        {
            DcEventConfig config;
            opts.m_eventsVector.push_back(config);
        }

    }

    osCpuid cpuInfo;

    GT_IF_WITH_ASSERT(m_pMonitoredEventsTreeWidget != nullptr)
    {
        for (int i = 0; i < m_pMonitoredEventsTreeWidget->topLevelItemCount(); ++i)
        {
            QTreeWidgetItem* pItem = m_pMonitoredEventsTreeWidget->topLevelItem(i);
            gtUInt16 eventSelect = (pItem->type() - QTreeWidgetItem::UserType);
            gtUByte unitMask = pItem->text(EVENT_UNITMASK_COLUMN).toUShort(nullptr, 16);

            if (IsTimerEvent(eventSelect))
            {
                opts.m_msInterval = pItem->text(EVENT_INTERVAL_COLUMN).toFloat();
            }
            else if (IsIbsOpEvent(eventSelect))
            {
                opts.m_opSample = true;
                opts.m_opInterval = pItem->text(EVENT_INTERVAL_COLUMN).toULong();
                opts.m_opCycleCount = unitMask == 0;
            }
            else if (IsIbsFetchEvent(eventSelect))
            {
                opts.m_fetchSample = true;
                opts.m_fetchInterval = pItem->text(EVENT_INTERVAL_COLUMN).toULong();
            }
            else if (IsIbsCluEvent(eventSelect))
            {
                opts.m_cluSample = true;
                opts.m_cluInterval = pItem->text(EVENT_INTERVAL_COLUMN).toULong();
                opts.m_cluCycleCount = unitMask == 0;
            }
            else if (IsL2IEvent(eventSelect))
            {
                GT_IF_WITH_ASSERT((ebpIndex < opts.m_eventsVector.size()) && (ebpIndex < ebpCount))
                {
                    opts.m_eventsVector[ebpIndex].pmc.perf_ctl = 0;
                    opts.m_eventsVector[ebpIndex].pmc.ucEventSelect = eventSelect & 0xFF;
                    opts.m_eventsVector[ebpIndex].pmc.ucEventSelectHigh = (eventSelect >> 8) & 0xF;
                    opts.m_eventsVector[ebpIndex].pmc.bitEnabled = true;
                    opts.m_eventsVector[ebpIndex].pmc.bitSampleEvents = true;
                    opts.m_eventsVector[ebpIndex].pmc.ucUnitMask = unitMask;
                    // Set FakeL2IMask represents that the event is L2I event
                    opts.m_eventsVector[ebpIndex].pmc.FakeL2IMask = FAKE_L2I_MASK_VALUE;

                    opts.m_eventsVector[ebpIndex].eventCount = pItem->text(EVENT_INTERVAL_COLUMN).toULongLong();
                    ++ebpIndex;
                }
            }
            else
            {
                GT_IF_WITH_ASSERT((ebpIndex < opts.m_eventsVector.size()) && (ebpIndex < ebpCount))
                {
                    opts.m_eventsVector[ebpIndex].pmc.perf_ctl = 0;
                    opts.m_eventsVector[ebpIndex].pmc.ucEventSelect = eventSelect & 0xFF;
                    opts.m_eventsVector[ebpIndex].pmc.ucEventSelectHigh = (eventSelect >> 8) & 0xF;
                    opts.m_eventsVector[ebpIndex].pmc.bitEnabled = true;
                    opts.m_eventsVector[ebpIndex].pmc.bitSampleEvents = true;
                    opts.m_eventsVector[ebpIndex].pmc.ucUnitMask = unitMask;

                    if (Qt::Checked == pItem->checkState(EVENT_USR_COLUMN))
                    {
                        opts.m_eventsVector[ebpIndex].pmc.bitUsrEvents = 1;
                    }

                    if (Qt::Checked == pItem->checkState(EVENT_OS_COLUMN))
                    {
                        opts.m_eventsVector[ebpIndex].pmc.bitOsEvents = 1;
                    }

                    // Disable OS mode event counting while profiling within guest OS on VMware
                    if (cpuInfo.hasHypervisor() && cpuInfo.getHypervisorVendorId() == HV_VENDOR_VMWARE)
                    {
                        opts.m_eventsVector[ebpIndex].pmc.bitOsEvents = 0;
                    }

                    opts.m_eventsVector[ebpIndex].eventCount = pItem->text(EVENT_INTERVAL_COLUMN).toULongLong();
                    ++ebpIndex;
                }
            }
        }
    }

    ProfileConfigs::instance().setCustomConfig(&opts);

    // Find and open the events file.
    osFilePath eventFilePath(osFilePath::OS_CODEXL_DATA_PATH);
    eventFilePath.appendSubDirectory(L"Events");

    if (m_eventEngine.Initialize(acGTStringToQString(eventFilePath.asString())))
    {
        m_pEventFile = m_eventEngine.GetEventFile(cpuInfo.getFamily(), cpuInfo.getModel());
    }

    return retVal;

}

void CustomProfileProjectSettingsExtension::RestoreDefaultProjectSettings()
{
}

bool CustomProfileProjectSettingsExtension::AreSettingsValid(gtString& invalidMessageStr)
{
    bool retVal = true;

    QString errorString;
    unsigned int ebpCount = 0;

    if (!validateIntervals(errorString) || !validateUsrOs(errorString) || !validateUnique(errorString, ebpCount))
    {
        invalidMessageStr = acQStringToGTString(errorString);
        retVal = false;
    }

    return retVal;
}

bool CustomProfileProjectSettingsExtension::RestoreCurrentSettings()
{
    bool retVal = false;

    emit m_pRemoveAll->clicked();

    addConfigurationToTree(CUSTOM_PROFILE_NAME, m_pMonitoredEventsTreeWidget);
    OnUpdateRemoveAll();

    // Resize the columns to content
    m_pMonitoredEventsTreeWidget->header()->resizeSections(QHeaderView::ResizeToContents);

    m_wereSettingsChanged = false;

    return retVal;
}


void CustomProfileProjectSettingsExtension::hideSettingsBox()
{
    m_pSettingsLabel->hide();
    m_pSettings->hide();

    if (nullptr != m_pDipatchRadio)
    {
        m_pDipatchRadio->hide();
    }

    if (nullptr != m_pCycleRadio)
    {
        m_pCycleRadio->hide();
    }

    for (int i = 0; i < MAX_UNITMASK; i++)
    {
        if (nullptr != m_pUnitMasks[i])
        {
            m_pUnitMasks[i]->setText("Reserved");
            m_pUnitMasks[i]->hide();
            m_pUnitMasks[i]->setChecked(false);
        }
    }
}

bool CustomProfileProjectSettingsExtension::InitializeFromProfileType(const QString& currentProfileType)
{
    bool bRet(false);
    osCpuid cpuInfo;

    QVBoxLayout* pLay = new QVBoxLayout;
    GT_IF_WITH_ASSERT(nullptr != pLay)
    {
        QWidget* pWidget = new QWidget;


        //Add IBS ops radio buttons
        m_pDipatchRadio = new QRadioButton;

        m_pDipatchRadio->setText("Count ops dispatched");
        m_pDipatchRadio->setChecked(true);
        pLay->addWidget(m_pDipatchRadio); //Row 1

        m_pCycleRadio = new QRadioButton;

        m_pCycleRadio->setText("Count clock cycles");
        pLay->addWidget(m_pCycleRadio); //Row 2

        connect(m_pDipatchRadio, SIGNAL(clicked(bool)), this, SLOT(OnOpsCountChanged()));
        connect(m_pCycleRadio, SIGNAL(clicked(bool)), this, SLOT(OnOpsCountChanged()));

        //add EBP unit mask check boxes
        for (int i = 0; i < MAX_UNITMASK; i++)
        {
            m_pUnitMasks[i] = new QCheckBox;


            pLay->addWidget(m_pUnitMasks[i]); //1st two overlap with the IBS ops radio buttons
            connect(m_pUnitMasks[i], SIGNAL(clicked(bool)), this, SLOT(OnUnitMaskChanged()));
        }

        pLay->addStretch(1); //Add a spacer item to the bottom to align items to top

        hideSettingsBox();
        pWidget->setLayout(pLay);
        m_pSettings->setWidget(pWidget);
        m_pSettings->setWidgetResizable(true);
        bRet = true;
    }

    if (bRet)
    {
        CustomEventItemDelegate* pDelegateMon = new CustomEventItemDelegate(m_pMonitoredEventsTreeWidget);
        CustomEventItemDelegate* pDelegateAv = new CustomEventItemDelegate(m_pAvailableTree);

        m_pMonitoredEventsTreeWidget->setItemDelegate(pDelegateMon);
        m_pAvailableTree->setItemDelegate(pDelegateAv);

        fnGetEventCounters(&m_maxEbpEvents);
        m_maxEbpEvents *= 8; // 8 is the arbitrary limit of profiles per counter per core
    }

    if (bRet)
    {
        // Find and open the events file.
        osFilePath eventFilePath(osFilePath::OS_CODEXL_DATA_PATH);
        eventFilePath.appendSubDirectory(L"Events");

        if (m_eventEngine.Initialize(acGTStringToQString(eventFilePath.asString())))
        {
            m_pEventFile = m_eventEngine.GetEventFile(cpuInfo.getFamily(), cpuInfo.getModel());
        }

        QString title = "Advanced CPU Profiling Configuration Events[*] - CPU: ";

        if (cpuInfo.isCpuAmd())
        {
            title.append("AMD ");
        }

        title.append("Family " + QString::number(cpuInfo.getFamily()));
        title.append(" Model " + QString::number(cpuInfo.getModel()));
        title.append(" Stepping " + QString::number(cpuInfo.getStepping()));
        setWindowTitle(title);
    }

    //If this is not an AMD system,
    if ((bRet) && (!cpuInfo.isCpuAmd()))
    {
        QStringList list;
        list << PM_profileTypeAssesPerformance;
        //taunt the user with AMD configuration names
        QTreeWidgetItem* pTaunt = new QTreeWidgetItem(m_pAvailableTree, list);

        bRet = nullptr != pTaunt;

        if (bRet)
        {
            list.clear();
            list << PM_profileTypeInstructionBasedSampling;
            pTaunt = new QTreeWidgetItem(m_pAvailableTree, list);

            bRet = nullptr != pTaunt;
        }

        if (bRet)
        {
            list.clear();
            list << PM_profileTypeInvestigateBranching;
            pTaunt = new QTreeWidgetItem(m_pAvailableTree, list);

            bRet = nullptr != pTaunt;
        }

        if (bRet)
        {
            list.clear();
            list << PM_profileTypeInvestigateDataAccess;
            pTaunt = new QTreeWidgetItem(m_pAvailableTree, list);

            bRet = nullptr != pTaunt;
        }

        if (bRet)
        {
            list.clear();
            list << PM_profileTypeInvestigateInstructionAccess;
            pTaunt = new QTreeWidgetItem(m_pAvailableTree, list);

            bRet = nullptr != pTaunt;
        }

        if (bRet)
        {
            list.clear();
            list << PM_profileTypeInvestigateInstructionL2CacheAccess;
            pTaunt = new QTreeWidgetItem(m_pAvailableTree, list);

            bRet = nullptr != pTaunt;
        }
    }

    if (bRet)
    {
        //Read all available profiles
        gtVector<gtString> proList = ProfileConfigs::instance().getListOfProfiles();
        gtVector<gtString>::const_iterator it = proList.begin();
        gtVector<gtString>::const_iterator itEnd = proList.end();

        for (; ((it != itEnd) && (bRet)); it++)
        {
#if (AMDT_BUILD_TARGET != AMDT_WINDOWS_OS)

            if (CLU_PROFILE_NAME == *it)
            {
                continue;
            }

#endif //(AMDT_BUILD_TARGET != AMDT_WINDOWS_OS)

            //Add custom config to other tree
            if (CUSTOM_PROFILE_NAME == *it)
            {
                addConfigurationToTree(*it, m_pMonitoredEventsTreeWidget);
                continue;
            }

            //Add each to available tree
            bRet = addConfigurationToTree(*it, m_pAvailableTree);
        }
    }

    if (bRet)
    {
        //Add all events and various event sources
        bRet = addAllEventsToAvailable(cpuInfo.isIbsAvailable(), cpuInfo.isCpuAmd());
    }

    highlightMonitored();

    if (bRet)
    {
        //Set the inital selection
        QList<QTreeWidgetItem*> found = m_pAvailableTree->findItems(currentProfileType, Qt::MatchFixedString, EVENT_NAME_COLUMN);

        if (!found.empty())
        {
            m_pAvailableTree->setCurrentItem(found.at(0));
        }
        else
        {
            m_pAvailableTree->setCurrentItem(m_pAvailableTree->topLevelItem(0));
        }
    }

    // Resize the columns to content
    m_pMonitoredEventsTreeWidget->header()->resizeSections(QHeaderView::ResizeToContents);

    return bRet;
}

//Add the selected event to the monitored list
void CustomProfileProjectSettingsExtension::monitorEvent(QTreeWidgetItem* pAvailableItem)
{
    bool addEvent = true;

    if (nullptr != FindMonitoredEventItem(*pAvailableItem))
    {
        if (!IsDuplicatableEvent(*pAvailableItem))
        {
            addEvent = false;
        }
        else if (QMessageBox::Ok != acMessageBox::instance().question(AF_STR_QuestionA, "Click Ok if you want the duplicate event. Note that you will need to change the Unit mask, Usr, or Os settings before the configuration is valid.", QMessageBox::Ok | QMessageBox::Cancel))
        {
            addEvent = false;
        }
    }

    if (addEvent)
    {
        QTreeWidgetItem* pNewConfig = new QTreeWidgetItem(pAvailableItem->type());

        GT_IF_WITH_ASSERT(nullptr != pNewConfig)
        {
            *pNewConfig = *pAvailableItem;
            pNewConfig->setForeground(EVENT_NAME_COLUMN, palette().windowText());
            m_pMonitoredEventsTreeWidget->addTopLevelItem(pNewConfig);
            m_pMonitoredEventsTreeWidget->scrollToItem(pNewConfig);
            m_pMonitoredEventsTreeWidget->setCurrentItem(pNewConfig);

            // Resize the columns to content
            m_pMonitoredEventsTreeWidget->header()->resizeSections(QHeaderView::ResizeToContents);
        }
    }
}

void CustomProfileProjectSettingsExtension::OnAddEvent()
{
    QTreeWidgetItem* pItem = m_pAvailableTree->currentItem();

    if (pItem != nullptr)
    {
        SettingsWereChanged();

        m_pErrLabel->hide();
        setWindowModified(true);

        if (0 == pItem->childCount())
        {
            //Single event
            monitorEvent(pItem);

            if (!IsDuplicatableEvent(*pItem))
            {
                m_pAddEvent->setEnabled(false);
            }
        }
        else
        {
            if (CP_STR_cpuProfileProjectSettingsCustomAllEvents != pItem->text(EVENT_NAME_COLUMN) &&
                CP_STR_cpuProfileProjectSettingsCustomHardwareParent != pItem->text(EVENT_NAME_COLUMN))
            {
                bool allowDuplicates = false;

                //Add all the children of the predefined configuration
                for (int i = 0, count = pItem->childCount(); i < count; ++i)
                {
                    QTreeWidgetItem* pEventItem = pItem->child(i);

                    monitorEvent(pEventItem);

                    if (!allowDuplicates && IsDuplicatableEvent(*pEventItem))
                    {
                        allowDuplicates = true;
                    }
                }

                m_pAddEvent->setEnabled(allowDuplicates);
            }
            else if (pItem->childCount() > 3)
            {
                m_pErrLabel->show();
                m_pErrLabel->setText("Adding too many events is unreasonable");
            }
        }

        highlightMonitored();
        OnUpdateRemoveAll();

        // Resize the columns to content
        m_pMonitoredEventsTreeWidget->header()->resizeSections(QHeaderView::ResizeToContents);
    }
}

void CustomProfileProjectSettingsExtension::OnRemoveEvent()
{
    QTreeWidgetItem* pItem = m_pMonitoredEventsTreeWidget->currentItem();

    if (pItem != nullptr)
    {
        QTreeWidgetItem* pAvailableItem = m_pAvailableTree->currentItem();

        if (nullptr != pAvailableItem)
        {
            if (nullptr != FindAvailableEventItem(*pAvailableItem, *pItem))
            {
                m_pAddEvent->setEnabled(true);
            }
        }

        SettingsWereChanged();
        delete pItem;
        m_pErrLabel->hide();
        setWindowModified(true);
        highlightMonitored();
        OnUpdateRemoveAll();
    }
}

void CustomProfileProjectSettingsExtension::OnRemoveAll()
{
    //Remove from bottom up, so not to change the indexes
    for (int i = m_pMonitoredEventsTreeWidget->topLevelItemCount() - 1; i >= 0; --i)
    {
        delete m_pMonitoredEventsTreeWidget->takeTopLevelItem(i);
    }

    m_pErrLabel->hide();
    setWindowModified(true);
    highlightMonitored();
    OnUpdateRemoveAll();
    m_pAddEvent->setEnabled(true);

    SettingsWereChanged();

}

void CustomProfileProjectSettingsExtension::OnEventEdited(QTreeWidgetItem* pItem, int column)
{
    if ((nullptr == pItem) || (nullptr == m_pMonitoredEventsTreeWidget->currentItem()))
    {
        return;
    }

    if (pItem != m_pMonitoredEventsTreeWidget->currentItem())
    {
        //Initial event add, no update needed

        return;
    }

    setWindowModified(true);

    gtUInt16 eventSelect = (pItem->type() - QTreeWidgetItem::UserType);

    if (IsTimerEvent(eventSelect))
    {
        //For TBP, only the interval is modifyable
        float interval = pItem->text(EVENT_INTERVAL_COLUMN).toFloat();
        pItem->setData(EVENT_INTERVAL_COLUMN, Qt::EditRole, QVariant(interval));
    }
    else if (GetIbsFetchEvent() == eventSelect)
    {
        //For IBS Fetch, only the interval is modifyable
        unsigned long long interval = pItem->text(EVENT_INTERVAL_COLUMN).toULongLong();
        pItem->setData(EVENT_INTERVAL_COLUMN, Qt::EditRole, QVariant(interval));
    }
    else
    {
        //For EBP, IBS_OP and CLU, the unit mask is useful
        if (EVENT_UNITMASK_COLUMN == column)
        {
            gtUByte unitMask = pItem->text(EVENT_UNITMASK_COLUMN).toUShort(nullptr, 16);

            //Ensure valid format
            if (GetIbsOpEvent() == eventSelect || GetIbsCluEvent() == eventSelect)
            {
                if (unitMask > 1)
                {
                    unitMask = 1;
                }

                pItem->setToolTip(EVENT_UNITMASK_COLUMN, opToolTip(0 == unitMask));
            }
            else if (nullptr != m_pEventFile)
            {
                //Only EBP left
                CpuEvent oneEvent;

                if (m_pEventFile->FindEventByValue(eventSelect, oneEvent))
                {
                    UnitMaskList::const_iterator umit;
                    gtUByte verify(0);

                    for (umit = m_pEventFile->FirstUnitMask(oneEvent);
                         umit != m_pEventFile->EndOfUnitMasks(oneEvent); ++umit)
                    {
                        verify |= unitMask & (1 << umit->value);
                    }

                    unitMask = verify;
                    pItem->setToolTip(EVENT_UNITMASK_COLUMN, eventUnitMaskTip(eventSelect, unitMask));
                }
            }

            pItem->setText(EVENT_UNITMASK_COLUMN, "0x" + QString::number(unitMask, 16));
            OnMonitoredItemChanged(pItem);
        }
        else if (EVENT_INTERVAL_COLUMN == column)
        {
            unsigned long long interval = pItem->text(EVENT_INTERVAL_COLUMN).toULongLong();
            pItem->setData(EVENT_INTERVAL_COLUMN, Qt::EditRole, QVariant(interval));
        }
    }
}

// When either of the radio buttons are clicked for the IBS ops dispatch or cycle
void CustomProfileProjectSettingsExtension::OnOpsCountChanged()
{
    QTreeWidgetItem* pItem = m_pMonitoredEventsTreeWidget->currentItem();

    if (nullptr == pItem)
    {
        return;
    }

    setWindowModified(true);
    gtUInt16 eventSelect = (pItem->type() - QTreeWidgetItem::UserType);

    if (GetIbsOpEvent() == eventSelect || GetIbsCluEvent() == eventSelect)
    {
        pItem->setText(EVENT_UNITMASK_COLUMN, "0x" + QString::number(m_pCycleRadio->isChecked() ? 0 : 1));
        pItem->setToolTip(EVENT_UNITMASK_COLUMN, opToolTip(m_pCycleRadio->isChecked()));
    }
}

// When any of the unit mask boxes are checked or unchecked
void CustomProfileProjectSettingsExtension::OnUnitMaskChanged()
{
    QTreeWidgetItem* pItem = m_pMonitoredEventsTreeWidget->currentItem();

    if (nullptr == pItem)
    {
        return;
    }

    setWindowModified(true);
    gtUInt16 eventSelect = (pItem->type() - QTreeWidgetItem::UserType);

    if (IsPmcEvent(eventSelect) || IsL2IEvent(eventSelect))
    {
        gtUByte unitMask(0);

        //Set the unit mask from the checkboxes
        for (int i = 0; i < MAX_UNITMASK; i++)
        {
            if (m_pUnitMasks[i]->isChecked())
            {
                unitMask |= 1 << i;
            }
        }

        pItem->setText(EVENT_UNITMASK_COLUMN, "0x" + QString::number(unitMask, 16));
        pItem->setToolTip(EVENT_UNITMASK_COLUMN, eventUnitMaskTip(eventSelect, unitMask));
    }
}

bool CustomProfileProjectSettingsExtension::validateUnique(QString& errString, unsigned int& ebpCount)
{
    bool timeSet(false);
    bool opsSet(false);
    bool fetchSet(false);
    bool cluSet(false);
    bool bRet(true);

    for (int i = 0; ((i < m_pMonitoredEventsTreeWidget->topLevelItemCount()) && (bRet)); ++i)
    {
        QTreeWidgetItem* pItem = m_pMonitoredEventsTreeWidget->topLevelItem(i);
        gtUInt16 eventSelect = (pItem->type() - QTreeWidgetItem::UserType);

        if (IsTimerEvent(eventSelect))
        {
            if (timeSet)
            {
                errString = "Only one timer event is valid per profile";
                m_pMonitoredEventsTreeWidget->setFocus();
                m_pMonitoredEventsTreeWidget->setCurrentItem(pItem);
                bRet = false;
            }
            else
            {
                timeSet = true;
            }
        }
        else if (GetIbsOpEvent() == eventSelect)
        {
            if (opsSet)
            {
                errString = "Only one IBS Ops event is valid per profile";
                m_pMonitoredEventsTreeWidget->setFocus();
                m_pMonitoredEventsTreeWidget->setCurrentItem(pItem);
                bRet = false;
            }
            else
            {
                opsSet = true;
            }
        }
        else if (GetIbsFetchEvent() == eventSelect)
        {
            if (fetchSet)
            {
                errString = "Only one IBS Fetch event is valid per profile";
                m_pMonitoredEventsTreeWidget->setFocus();
                m_pMonitoredEventsTreeWidget->setCurrentItem(pItem);
                bRet = false;
            }
            else
            {
                fetchSet = true;
            }
        }
        else if (GetIbsCluEvent() == eventSelect)
        {
            if (cluSet)
            {
                errString = "Only one CLU event is valid per profile";
                m_pMonitoredEventsTreeWidget->setFocus();
                m_pMonitoredEventsTreeWidget->setCurrentItem(pItem);
                bRet = false;
            }
            else
            {
                cluSet = true;
            }
        }
        else
        {
            //Verify ebp count is within constraints
            if (++ebpCount == m_maxEbpEvents)
            {
                m_pMonitoredEventsTreeWidget->setFocus();
                m_pMonitoredEventsTreeWidget->setCurrentItem(pItem);
                errString.sprintf("The CPU profiling supports a maximum of %d events for one profile", m_maxEbpEvents);
                bRet = false;
            }

            //Check for duplicate PMC events
            for (int j = i + 1; ((j < m_pMonitoredEventsTreeWidget->topLevelItemCount()) && (bRet)); j++)
            {
                QTreeWidgetItem* pCheck = m_pMonitoredEventsTreeWidget->topLevelItem(j);

                if ((pItem->type() == pCheck->type()) &&
                    (pItem->text(EVENT_UNITMASK_COLUMN) == pCheck->text(EVENT_UNITMASK_COLUMN)) &&
                    (pItem->checkState(EVENT_USR_COLUMN) == pCheck->checkState(EVENT_USR_COLUMN)) &&
                    (pItem->checkState(EVENT_OS_COLUMN) == pCheck->checkState(EVENT_OS_COLUMN)))
                {
                    m_pMonitoredEventsTreeWidget->setFocus();
                    m_pMonitoredEventsTreeWidget->setCurrentItem(pCheck);
                    errString = "Events cannot be exactly duplicated, please change the Unit Mask, Usr or Os";
                    bRet = false;
                }
            }
        }
    }

    return bRet;
}

bool CustomProfileProjectSettingsExtension::validateIntervals(QString& errString)
{
    bool bRet(true);

    for (int i = 0; i < m_pMonitoredEventsTreeWidget->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* pItem = m_pMonitoredEventsTreeWidget->topLevelItem(i);
        gtUInt16 eventSelect = (pItem->type() - QTreeWidgetItem::UserType);

        // Baskar:
        // Minimum timer interval supported for TBP is 1 milli-second.
        if (((0 == pItem->text(EVENT_INTERVAL_COLUMN).toULongLong()) &&
             (!IsTimerEvent(eventSelect))) ||
            (1 >  pItem->text(EVENT_INTERVAL_COLUMN).toFloat()))
        {
            bRet = false;
            m_pMonitoredEventsTreeWidget->setFocus();
            m_pMonitoredEventsTreeWidget->setCurrentItem(pItem);
            errString = "This event needs a valid interval";
            break;
        }
    }

    return bRet;
}

bool CustomProfileProjectSettingsExtension::validateUsrOs(QString& errString)
{
    bool bRet(true);

    for (int i = 0; i < m_pMonitoredEventsTreeWidget->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* pItem = m_pMonitoredEventsTreeWidget->topLevelItem(i);
        gtUInt16 eventSelect = (pItem->type() - QTreeWidgetItem::UserType);

        if ((IsPmcEvent(eventSelect)) && (Qt::Unchecked == pItem->checkState(EVENT_USR_COLUMN))
            && (Qt::Unchecked == pItem->checkState(EVENT_OS_COLUMN)))
        {
            bRet = false;
            m_pMonitoredEventsTreeWidget->setFocus();
            m_pMonitoredEventsTreeWidget->setCurrentItem(pItem);
            errString = "The event needs the User, Os, or both checked to generate samples";
            break;
        }
    }

    return bRet;
}

QString CustomProfileProjectSettingsExtension::opToolTip(bool cycleCount)
{
    QString strRet;

    if (!cycleCount)
    {
        strRet = "Count dispatched ops";
    }
    else
    {
        strRet = "Count clock cycles";
    }

    return strRet;
}

//Describe the unit mask in the tool tip
QString CustomProfileProjectSettingsExtension::eventUnitMaskTip(gtUInt16 event, gtUByte unitMask)
{
    QString strRet;

    if (nullptr != m_pEventFile)
    {
        CpuEvent oneEvent;

        if (m_pEventFile->FindEventByValue(event, oneEvent))
        {
            UnitMaskList::const_iterator umit;

            for (umit = m_pEventFile->FirstUnitMask(oneEvent);
                 umit != m_pEventFile->EndOfUnitMasks(oneEvent); ++umit)
            {
                if ((unitMask & (1 << umit->value)) > 0)
                {
                    if (!strRet.isEmpty())
                    {
                        strRet.append("\n");
                    }

                    strRet.append(umit->name);
                }
            }
        }
    }

    return strRet;
}

bool CustomProfileProjectSettingsExtension::addConfigurationToTree(const gtString& name, QTreeWidget* pTree)
{
    bool bRet(true);
    CPUSessionTreeItemData opts;
    CpuEvent oneEvent;
    bRet = ProfileConfigs::instance().getProfileConfigByType(name, &opts);

    if (bRet)
    {
        //Add the parent item
        QTreeWidgetItem* pParent = new QTreeWidgetItem(QStringList(QString::fromWCharArray(name.asCharArray())));

        bRet = nullptr != pParent;

        //If applicable, add the timer
        if ((bRet) && (opts.m_msInterval > 0))
        {
            QTreeWidgetItem* pItem = new QTreeWidgetItem(pParent, ((int) QTreeWidgetItem::UserType + GetTimerEvent()));

            bRet = nullptr != pParent;

            if (bRet)
            {
                pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
                pItem->setText(EVENT_NAME_COLUMN, buildEventName(GetTimerEvent(), CP_STR_cpuProfileProjectSettingsCustomTimerEvent));
                pItem->setData(EVENT_INTERVAL_COLUMN, Qt::EditRole, QVariant(opts.m_msInterval));
                pItem->setToolTip(EVENT_INTERVAL_COLUMN, CP_STR_cpuProfileProjectSettingsCustomMilliseconds);
            }
        }

        //If applicable, add the IBS
        if ((bRet) && (opts.m_opSample) && (nullptr != m_pEventFile))
        {
            QTreeWidgetItem* pItem = new QTreeWidgetItem(pParent, ((int) QTreeWidgetItem::UserType + GetIbsOpEvent()));

            bRet = nullptr != pParent;

            if (bRet)
            {
                //look up name of ibs event
                if (m_pEventFile->FindEventByValue(GetIbsOpEvent(), oneEvent))
                {
                    pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
                    pItem->setText(EVENT_NAME_COLUMN, buildEventName(GetIbsOpEvent(), oneEvent.name));
                    pItem->setData(EVENT_INTERVAL_COLUMN, Qt::EditRole, QVariant((uint)opts.m_opInterval));
                    pItem->setText(EVENT_UNITMASK_COLUMN, "0x" + QString::number(opts.m_opCycleCount ? 0 : 1, 16));
                    pItem->setToolTip(EVENT_UNITMASK_COLUMN, opToolTip(opts.m_opCycleCount));
                }
                else
                {
                    //The IBS event isn't valid for this system
                    delete pItem;
                }
            }
        }

        if ((bRet) && (opts.m_fetchSample) && (nullptr != m_pEventFile))
        {
            QTreeWidgetItem* pItem = new QTreeWidgetItem(pParent, ((int) QTreeWidgetItem::UserType + GetIbsFetchEvent()));

            bRet = nullptr != pParent;

            if (bRet)
            {
                //look up name of ibs event
                if (m_pEventFile->FindEventByValue(GetIbsFetchEvent(), oneEvent))
                {
                    pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
                    pItem->setText(EVENT_NAME_COLUMN, buildEventName(GetIbsFetchEvent(), oneEvent.name));
                    pItem->setData(EVENT_INTERVAL_COLUMN, Qt::EditRole, QVariant((uint)opts.m_fetchInterval));
                }
                else
                {
                    //The IBS event isn't valid for this system
                    delete pItem;
                }
            }
        }

        //If applicable, add the CLU
        if ((bRet) && (opts.m_cluSample) && (nullptr != m_pEventFile))
        {
            QTreeWidgetItem* pItem = new QTreeWidgetItem(pParent, ((int) QTreeWidgetItem::UserType + GetIbsCluEvent()));

            bRet = nullptr != pParent;

            if (bRet)
            {
                if (m_pEventFile->FindEventByValue(GetIbsCluEvent(), oneEvent))
                {
                    pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
                    pItem->setText(EVENT_NAME_COLUMN, buildEventName(GetIbsCluEvent(), CP_STR_cpuProfileProjectSettingsCustomCLUEvent));
                    pItem->setData(EVENT_INTERVAL_COLUMN, Qt::EditRole, QVariant((uint)opts.m_cluInterval));
                    pItem->setText(EVENT_UNITMASK_COLUMN, "0x" + QString::number(opts.m_cluCycleCount ? 0 : 1, 16));
                    pItem->setToolTip(EVENT_UNITMASK_COLUMN, opToolTip(opts.m_cluCycleCount));
                }
                else
                {
                    //The CLU event isn't valid for this system
                    delete pItem;
                }
            }
        }

        //Add each of the events
        if ((bRet) && (nullptr != m_pEventFile))
        {
            for (int i = 0; ((i < (int)opts.m_eventsVector.size()) && (bRet)); i++)
            {
                gtUInt16 eventSelect = GetEvent12BitSelect(opts.m_eventsVector[i].pmc);
                QTreeWidgetItem* pItem = new QTreeWidgetItem(pParent, ((int) QTreeWidgetItem::UserType + eventSelect));

                bRet = nullptr != pParent;

                if (bRet)
                {
                    //look up name of event
                    if (m_pEventFile->FindEventByValue(eventSelect, oneEvent))
                    {
                        pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
                        pItem->setText(EVENT_NAME_COLUMN, buildEventName(eventSelect, oneEvent.name));
                        pItem->setData(EVENT_INTERVAL_COLUMN, Qt::EditRole, QVariant((qulonglong)opts.m_eventsVector[i].eventCount));
                        pItem->setText(EVENT_UNITMASK_COLUMN, "0x" + QString::number(opts.m_eventsVector[i].pmc.ucUnitMask, 16));
                        pItem->setToolTip(EVENT_UNITMASK_COLUMN, eventUnitMaskTip(eventSelect, opts.m_eventsVector[i].pmc.ucUnitMask));
                        pItem->setCheckState(EVENT_USR_COLUMN, (0 != opts.m_eventsVector[i].pmc.bitUsrEvents) ? Qt::Checked : Qt::Unchecked);
                        pItem->setToolTip(EVENT_USR_COLUMN, CP_STR_cpuProfileProjectSettingsUsrColumnTooltip);
                        pItem->setCheckState(EVENT_OS_COLUMN, (0 != opts.m_eventsVector[i].pmc.bitOsEvents) ? Qt::Checked : Qt::Unchecked);
                        pItem->setToolTip(EVENT_OS_COLUMN, CP_STR_cpuProfileProjectSettingsOSColumnTooltip);
                    }
                    else
                    {
                        //The event isn't valid for this system
                        delete pItem;
                    }
                }
            }
        }

        if (bRet)
        {
            if (pTree == m_pAvailableTree)
            {
                m_pAvailableTree->addTopLevelItem(pParent);
            }
            else
            {
                // Add all the items straight to tree
                QList<QTreeWidgetItem*> pList = pParent->takeChildren();
                pTree->addTopLevelItems(pList);
                delete pParent;
            }
        }
    }

    return bRet;
}

QString CustomProfileProjectSettingsExtension::buildEventName(gtUInt16 event, const QString& eventName)
{
    QString strRet;
    strRet.sprintf("[%03X] ", event);
    strRet.append(eventName);
    return strRet;
}

QTreeWidgetItem* CustomProfileProjectSettingsExtension::FindMonitoredEventItem(const QTreeWidgetItem& availableItem) const
{
    QTreeWidgetItem* pMonitoredItem = nullptr;

    for (int i = 0, count = m_pMonitoredEventsTreeWidget->topLevelItemCount(); i < count; ++i)
    {
        QTreeWidgetItem* pItem = m_pMonitoredEventsTreeWidget->topLevelItem(i);

        if (pItem->text(EVENT_NAME_COLUMN) == availableItem.text(EVENT_NAME_COLUMN))
        {
            pMonitoredItem = pItem;
            break;
        }
    }

    return pMonitoredItem;
}

QTreeWidgetItem* CustomProfileProjectSettingsExtension::FindAvailableEventItem(QTreeWidgetItem& availableParent,
        const QTreeWidgetItem& monitoredItem) const
{
    QTreeWidgetItem* pAvailableItem = nullptr;

    int count = availableParent.childCount();

    if (0 == count)
    {
        if (availableParent.text(EVENT_NAME_COLUMN) == monitoredItem.text(EVENT_NAME_COLUMN))
        {
            pAvailableItem = &availableParent;
        }
    }
    else
    {
        for (int i = 0; i < count; ++i)
        {
            QTreeWidgetItem* pAvailableChild = availableParent.child(i);

            if (pAvailableChild->text(EVENT_NAME_COLUMN) == monitoredItem.text(EVENT_NAME_COLUMN))
            {
                pAvailableItem = pAvailableChild;
                break;
            }
        }
    }

    return pAvailableItem;
}

bool CustomProfileProjectSettingsExtension::IsDuplicatableEvent(const QTreeWidgetItem& item) const
{
    gtUInt16 eventSelect = (item.type() - QTreeWidgetItem::UserType);
    return (IsPmcEvent(eventSelect) || IsL2IEvent(eventSelect) || GetIbsOpEvent() == eventSelect || GetIbsCluEvent() == eventSelect);
}

bool CustomProfileProjectSettingsExtension::IsValidEvent(const QTreeWidgetItem& item) const
{
    return (item.type() >= QTreeWidgetItem::UserType);
}

void CustomProfileProjectSettingsExtension::OnAvailableItemChanged(QTreeWidgetItem* pCurrent)
{
    QString description;
    (void) m_pAvailableTree->indexOfTopLevelItem(pCurrent);

    if (nullptr == pCurrent)
    {
        return;
    }

    //Space to align text
    m_pDescriptionTitle->setText(" " + pCurrent->text(EVENT_NAME_COLUMN));

    if (0 == pCurrent->childCount())
    {
        bool allowAddition = IsValidEvent(*pCurrent) && (IsDuplicatableEvent(*pCurrent) || nullptr == FindMonitoredEventItem(*pCurrent));
        m_pAddEvent->setEnabled(allowAddition);

        gtUInt16 eventSelect = (pCurrent->type() - QTreeWidgetItem::UserType);

        if (IsPmcEvent(eventSelect) || IsL2IEvent(eventSelect))
        {
            CpuEvent oneEvent;

            if (m_pEventFile->FindEventByValue(eventSelect, oneEvent))
            {
                description = oneEvent.description;
            }
        }
        else if (GetIbsOpEvent() == eventSelect)
        {
            description = "Instruction execution sampling provides information about the execution behavior for one tagged micro-op associated with an instruction.  Instructions that decode to more than one micro-op return different performance data depending upon which micro-op associated with the instruction is tagged.";
        }
        else if (GetIbsFetchEvent() == eventSelect)
        {
            description = "Instruction fetch sampling provides information about instruction TLB and instruction cache behavior for tagged fetch instructions.";
        }
        else if (GetIbsCluEvent() == eventSelect)
        {
            description = "Cache line utilization provides information about the utilization of cache lines in L1 Data cache which helps to find out the issue related to data locality and data access pattern.";
        }
        else if (IsTimerEvent(eventSelect))
        {
            description = "Samples are taken at discrete time intervals.";
        }
        else
        {
            description = "This configuration is only available on AMD platforms";
        }
    }
    else
    {
        bool allowAddition = false;

        for (int i = 0, count = pCurrent->childCount(); i < count; ++i)
        {
            QTreeWidgetItem* pEventItem = pCurrent->child(i);

            if (IsDuplicatableEvent(*pEventItem) || nullptr == FindMonitoredEventItem(*pEventItem))
            {
                allowAddition = true;
                break;
            }
        }

        m_pAddEvent->setEnabled(allowAddition);

        //Grab the profile configuration description
        description = QString::fromWCharArray(ProfileConfigs::instance().getProfileDescription(pCurrent->text(EVENT_NAME_COLUMN).toStdWString().c_str()).asCharArray());

        // for events that don't have config file
        if (description.isEmpty())
        {
            if (CP_STR_cpuProfileProjectSettingsCustomAllEvents == pCurrent->text(EVENT_NAME_COLUMN))
            {
                description = CP_STR_cpuProfileProjectSettingsCustomAllEventsDescription;
            }
            else if (CP_STR_cpuProfileProjectSettingsCustomHardwareParent == pCurrent->text(EVENT_NAME_COLUMN))
            {
                description = CP_STR_cpuProfileProjectSettingsCustomHardwareParentDescription;
            }
        }
    }

    // The xml string is defining line breaks with \n. We need to remove it here, cause Qt is
    // implementing the line breaks according to the widget size
    description.replace("\\n", "");

    m_pDescription->setPlainText(description);
}

void CustomProfileProjectSettingsExtension::OnMonitoredItemChanged(QTreeWidgetItem* pCurrent)
{
    hideSettingsBox();

    bool isItemValid = (nullptr != pCurrent);
    m_pRemoveEvent->setEnabled(isItemValid);

    if (isItemValid)
    {
        gtUInt16 eventSelect = (pCurrent->type() - QTreeWidgetItem::UserType);
        gtUByte unitMask = pCurrent->text(EVENT_UNITMASK_COLUMN).toUShort(nullptr, 16);

        if (IsPmcEvent(eventSelect) || IsL2IEvent(eventSelect))
        {
            if (nullptr != m_pEventFile)
            {
                CpuEvent oneEvent;

                if (m_pEventFile->FindEventByValue(eventSelect, oneEvent))
                {
                    UnitMaskList::const_iterator umit;

                    for (umit = m_pEventFile->FirstUnitMask(oneEvent);
                         umit != m_pEventFile->EndOfUnitMasks(oneEvent); ++umit)
                    {
                        m_pSettingsLabel->show();
                        m_pSettings->show();
                        m_pUnitMasks[umit->value]->setVisible(true);
                        m_pUnitMasks[umit->value]->setText(umit->name);
                        m_pUnitMasks[umit->value]->setChecked((unitMask & (1 << umit->value)) > 0);
                    }
                }
            }
        }
        else if (GetIbsOpEvent() == eventSelect || GetIbsCluEvent() == eventSelect)
        {
            if ((nullptr != m_pDipatchRadio) && (nullptr != m_pCycleRadio))
            {
                m_pSettingsLabel->show();
                m_pSettings->show();
                m_pDipatchRadio->show();
                m_pCycleRadio->show();
                m_pDipatchRadio->setChecked(1 == unitMask);
                m_pCycleRadio->setChecked(1 != unitMask);
            }
        }
    }
}

void CustomProfileProjectSettingsExtension::OnSettingsTreeSelectionAboutToChange()
{
    // Check if this is a Custom CPU Profiling session.
    bool isCustomCpuProfile = (SharedProfileManager::instance().selectedSessionTypeName() == acQStringToGTString(PM_profileTypeCustomProfile));

    if (m_wereSettingsChanged && !isCustomCpuProfile)
    {
        int userSelection = acMessageBox::instance().question(AF_STR_QuestionA, CP_STR_cpuProfileProjectSettingsCustomTypeChangeQuestion, QMessageBox::Yes | QMessageBox::No);

        if (userSelection == QMessageBox::Yes)
        {
            // Set the custom profile as selected:
            SharedProfileManager::instance().SelectProfileType(acQStringToGTString(PM_profileTypeCustomProfilePrefix));
        }

        m_wereSettingsChanged = false;
    }
}

bool CustomProfileProjectSettingsExtension::addAllEventsToAvailable(bool isIbsAvailable, bool isAmdSystem)
{
    bool bRet(false);
    CpuEvent oneEvent;

    //Add the parent item
    QTreeWidgetItem* pParent = new QTreeWidgetItem(QStringList(CP_STR_cpuProfileProjectSettingsCustomAllEvents));

    bRet = nullptr != pParent;

    //If add the timer
    if (bRet)
    {
        QTreeWidgetItem* pItem = new QTreeWidgetItem(pParent, ((int) QTreeWidgetItem::UserType + GetTimerEvent()));

        bRet = nullptr != pParent;

        if (bRet)
        {
            pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
            pItem->setText(EVENT_NAME_COLUMN, buildEventName(GetTimerEvent(), CP_STR_cpuProfileProjectSettingsCustomTimerEvent));
            pItem->setData(EVENT_INTERVAL_COLUMN, Qt::EditRole, QVariant(DEFAULT_TIMER_INTERVAL));
            pItem->setToolTip(EVENT_INTERVAL_COLUMN, CP_STR_cpuProfileProjectSettingsCustomMilliseconds);
        }
    }

    //Only show timer event on non-AMD systems
    if (!isAmdSystem)
    {
        //Add all events
        m_pAvailableTree->addTopLevelItem(pParent);
        return bRet;
    }

    //If applicable, add the IBS
    if ((bRet) && (isIbsAvailable) && (nullptr != m_pEventFile))
    {
        QTreeWidgetItem* pItem = new QTreeWidgetItem(pParent, ((int) QTreeWidgetItem::UserType + GetIbsOpEvent()));

        bRet = nullptr != pParent;

        if (bRet)
        {
            //look up name of ibs event
            if (m_pEventFile->FindEventByValue(GetIbsOpEvent(), oneEvent))
            {
                pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
                pItem->setText(EVENT_NAME_COLUMN, buildEventName(GetIbsOpEvent(), oneEvent.name));
                pItem->setData(EVENT_INTERVAL_COLUMN, Qt::EditRole, QVariant(DEFAULT_IBS_INTERVAL));
                // IBS Op Dispatch count
                pItem->setText(EVENT_UNITMASK_COLUMN, "0x1");
                pItem->setToolTip(EVENT_UNITMASK_COLUMN, opToolTip(false));
            }
            else
            {
                //The IBS event isn't valid for this system
                delete pItem;
            }
        }

        pItem = new QTreeWidgetItem(pParent, ((int) QTreeWidgetItem::UserType + GetIbsFetchEvent()));

        bRet = nullptr != pParent;

        if (bRet)
        {
            //look up name of ibs event
            if (m_pEventFile->FindEventByValue(GetIbsFetchEvent(), oneEvent))
            {
                pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
                pItem->setText(EVENT_NAME_COLUMN, buildEventName(GetIbsFetchEvent(), oneEvent.name));
                pItem->setData(EVENT_INTERVAL_COLUMN, Qt::EditRole, QVariant(DEFAULT_IBS_INTERVAL));
            }
            else
            {
                //The IBS event isn't valid for this system
                delete pItem;
            }
        }

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
        pItem = new QTreeWidgetItem(pParent, ((int) QTreeWidgetItem::UserType + GetIbsCluEvent()));

        bRet = nullptr != pParent;

        if (bRet)
        {
            if (m_pEventFile->FindEventByValue(GetIbsCluEvent(), oneEvent))
            {
                pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
                pItem->setText(EVENT_NAME_COLUMN, buildEventName(GetIbsCluEvent(), CP_STR_cpuProfileProjectSettingsCustomCLUEvent));
                pItem->setData(EVENT_INTERVAL_COLUMN, Qt::EditRole, QVariant(DEFAULT_IBS_INTERVAL));
                pItem->setText(EVENT_UNITMASK_COLUMN, "0x1");
                pItem->setToolTip(EVENT_UNITMASK_COLUMN, opToolTip(false));
            }
            else
            {
                delete pItem;
            }
        }

#endif //(AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    }

    //Add the hardware parent item
    QTreeWidgetItem* pHardwareParent = new QTreeWidgetItem(QStringList(CP_STR_cpuProfileProjectSettingsCustomHardwareParent));

    bRet &= nullptr != pHardwareParent;

    //Add each of the events to the "All events" and the source
    if ((bRet) && (nullptr != m_pEventFile))
    {
        SourceMap sources;
        SourceMap::iterator srcIt;
        EventList::const_iterator it = m_pEventFile->FirstEvent();
        EventList::const_iterator endIt = m_pEventFile->EndOfEvents();

        for (; it != endIt; ++it)
        {
            //All PMC events are less than the Timer event,
            if (!IsPmcEvent(it->value) && !IsL2IEvent(it->value))
            {
                break;
            }

            QTreeWidgetItem* pItem = new QTreeWidgetItem(pParent, ((int) QTreeWidgetItem::UserType + it->value));


            srcIt = sources.find(it->source);

            if (sources.end() == srcIt)
            {
                QTreeWidgetItem* pHardwareSource = new QTreeWidgetItem(pHardwareParent);

                pHardwareSource->setText(EVENT_NAME_COLUMN, extendSourceName(it->source));
                srcIt = sources.insert(it->source, pHardwareSource);
            }

            QTreeWidgetItem* pSourceItem = new QTreeWidgetItem(srcIt.value(), ((int) QTreeWidgetItem::UserType + it->value));


            pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
            pSourceItem->setFlags(pSourceItem->flags() | Qt::ItemIsEditable);
            pItem->setText(EVENT_NAME_COLUMN, buildEventName(it->value, it->name));
            pSourceItem->setText(EVENT_NAME_COLUMN, buildEventName(it->value, it->name));

            //Default Count
            gtUInt64 count(50000);

            switch (it->value)
            {
                case 0x76:
                    count = 1000000;
                    break;

                case 0xd1:
                    count = 10000;
                    break;

                default:
                    break;
            }

            pItem->setData(EVENT_INTERVAL_COLUMN, Qt::EditRole, QVariant((qulonglong)count));
            pSourceItem->setData(EVENT_INTERVAL_COLUMN, Qt::EditRole, QVariant((qulonglong)count));

            gtUByte mask(0);
            QString maskTip;
            //Set all unit masks of the event on by default
            UnitMaskList::const_iterator umit;

            for (umit = m_pEventFile->FirstUnitMask(*it);
                 umit != m_pEventFile->EndOfUnitMasks(*it); ++umit)
            {
                if (umit->value >= (size_t) MAX_UNITMASK)
                {
                    continue;
                }

                mask |= 1 << umit->value;

                if (!maskTip.isEmpty())
                {
                    maskTip.append("\n");
                }

                maskTip.append(umit->name);
            }

            pItem->setText(EVENT_UNITMASK_COLUMN, "0x" + QString::number(mask, 16));
            pSourceItem->setText(EVENT_UNITMASK_COLUMN, "0x" + QString::number(mask, 16));
            pItem->setToolTip(EVENT_UNITMASK_COLUMN, maskTip);
            pSourceItem->setToolTip(EVENT_UNITMASK_COLUMN, maskTip);

            pItem->setCheckState(EVENT_USR_COLUMN, Qt::Checked);
            pSourceItem->setCheckState(EVENT_USR_COLUMN, Qt::Checked);
            pItem->setToolTip(EVENT_USR_COLUMN, CP_STR_cpuProfileProjectSettingsUsrColumnTooltip);
            pSourceItem->setToolTip(EVENT_USR_COLUMN, CP_STR_cpuProfileProjectSettingsUsrColumnTooltip);
            pItem->setCheckState(EVENT_OS_COLUMN, Qt::Checked);
            pSourceItem->setCheckState(EVENT_OS_COLUMN, Qt::Checked);
            pItem->setToolTip(EVENT_OS_COLUMN, CP_STR_cpuProfileProjectSettingsOSColumnTooltip);
            pSourceItem->setToolTip(EVENT_OS_COLUMN, CP_STR_cpuProfileProjectSettingsOSColumnTooltip);
        }

        //Add each Hw source list
        m_pAvailableTree->addTopLevelItem(pHardwareParent);

        //Add all events
        m_pAvailableTree->addTopLevelItem(pParent);
    }

    return bRet;
}

QString CustomProfileProjectSettingsExtension::extendSourceName(const QString& source)
{
    QString strRet;

    //DC = Data Cache Events
    if ("DC" == source)
    {
        strRet = "Data Cache Events";
    }
    else if ("FP" == source)
    {
        strRet = "Floating Point Events";
    }
    else if (("FR" == source) || ("EX" == source) || ("EU" == source))
    {
        strRet = "Execution Unit Events";
    }
    else if ("IC" == source)
    {
        strRet = "Instruction Cache Events";
    }
    else if (("L2" == source) || ("BU" == source))
    {
        strRet = "L2 Cache and System Interface Events";
    }
    else if ("L3" == source)
    {
        strRet = "L3 Cache Events";
    }
    else if ("LS" == source)
    {
        strRet = "Load/Store and TLB Events";
    }
    else if ("NB" == source)
    {
        strRet = "Memory Controller, Crossbar, and Link Events";
    }
    else if ("CU" == source)
    {
        strRet = "Compute Unit Events";
    }
    else if ("DE" == source)
    {
        strRet = "Decoder Events";
    }
    else if ("ME" == source)
    {
        strRet = "Memory Events";
    }
    else if ("L2I" == source)
    {
        strRet = "L2I Events";
    }
    else if ("DSM" == source)
    {
        strRet = "DSM Events";
    }
    else if ("DSM L2I" == source)
    {
        strRet = "DSM L2I Events";
    }

    return strRet;
}

//Highlight any monitored events in the available tree
void CustomProfileProjectSettingsExtension::highlightMonitored()
{
    QStringList monitored;
    int i;

    for (i = 0; (i < m_pMonitoredEventsTreeWidget->topLevelItemCount()); ++i)
    {
        QTreeWidgetItem* pItem = m_pMonitoredEventsTreeWidget->topLevelItem(i);
        monitored.append(pItem->text(EVENT_NAME_COLUMN));
    }

    for (i = 0; (i < m_pAvailableTree->topLevelItemCount()); ++i)
    {
        QTreeWidgetItem* pItem = m_pAvailableTree->topLevelItem(i);
        recurseHighlightChildren(pItem, monitored);
    }
}

//Highlight any appropriate children in the item, recursively
void CustomProfileProjectSettingsExtension::recurseHighlightChildren(QTreeWidgetItem* pItem, const QStringList& monitoredItems)
{
    for (int i = 0; (i < pItem->childCount()); ++i)
    {
        QTreeWidgetItem* pChild = pItem->child(i);
        recurseHighlightChildren(pChild, monitoredItems);
    }

    if (monitoredItems.contains(pItem->text(EVENT_NAME_COLUMN)))
    {
        pItem->setForeground(EVENT_NAME_COLUMN, QColor(Qt::blue));
    }
    else
    {
        pItem->setForeground(EVENT_NAME_COLUMN, palette().windowText());
    }
}

void CustomProfileProjectSettingsExtension::OnMonitoredEventDoubleClick(QTreeWidgetItem* pItem, int column)
{
    if ((nullptr != pItem) && (column == EVENT_NAME_COLUMN))
    {
        OnRemoveEvent();
        OnUpdateRemoveAll();
    }
}

void CustomProfileProjectSettingsExtension::OnAvailableEventDoubleClick(QTreeWidgetItem* pItem)
{
    //If the double-clicked item is an individual event, add it
    if ((nullptr != pItem) && (0 == pItem->childCount()) && m_pAddEvent->isEnabled())
    {
        OnAddEvent();
    }

    OnUpdateRemoveAll();
}


void CustomProfileProjectSettingsExtension::OnUpdateRemoveAll()
{
    m_pRemoveEvent->setEnabled(nullptr != m_pMonitoredEventsTreeWidget->currentItem());
    m_pRemoveAll->setEnabled(0 < m_pMonitoredEventsTreeWidget->topLevelItemCount());
}

void CustomProfileProjectSettingsExtension::SettingsWereChanged()
{
    m_wereSettingsChanged = true;

    if (!m_sWasConnectedToTree)
    {
        bool rc = connect(&afNewProjectDialog::instance(), SIGNAL(SettingsTreeSelectionAboutToChange()), this, SLOT(OnSettingsTreeSelectionAboutToChange()));
        GT_ASSERT(rc);
        rc = connect(&afNewProjectDialog::instance(), SIGNAL(OkButtonClicked()), this, SLOT(OnSettingsTreeSelectionAboutToChange()));
        GT_ASSERT(rc);
    }
}


//Future enhancements - restrict inputs
CustomEventItemDelegate::CustomEventItemDelegate(QObject* pParent) : QItemDelegate(pParent)
{
}

QWidget* CustomEventItemDelegate::createEditor(QWidget* pParent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QWidget* retVal = nullptr;
    QTreeWidget* pMonitoredEvents = (QTreeWidget*)parent();
    GT_IF_WITH_ASSERT(pParent != nullptr)
    {
        QTreeWidgetItem* pItem = pMonitoredEvents->currentItem();

        GT_IF_WITH_ASSERT(pItem != nullptr)
        {
            gtUInt16 eventSelect = (pItem->type() - QTreeWidgetItem::UserType);

            if (index.column() > EVENT_NAME_COLUMN)
            {
                //Only EBP uses usr/os/edge columns
                // Fetch & TBP shouldn't use the unit mask column
                if (((index.column() > EVENT_UNITMASK_COLUMN) && (!IsPmcEvent(eventSelect)))
                    || ((index.column() == EVENT_UNITMASK_COLUMN) && ((IsIbsFetchEvent(eventSelect)) || (IsTimerEvent(eventSelect)))))
                {
                    retVal = nullptr;
                }
                else
                {
                    retVal = QItemDelegate::createEditor(pParent, option, index);
                }
            }
            else
            {
                retVal = nullptr;
            }
        }
    }
    return retVal;
}
