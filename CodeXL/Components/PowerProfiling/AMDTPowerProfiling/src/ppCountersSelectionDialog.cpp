//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppCountersSelectionDialog.cpp
///
//==================================================================================

//------------------------------ ppCountersSelection.cpp ------------------------------

// Compiler warnings:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acDisplay.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTApplicationComponents/Include/acItemDelegate.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afCSSSettings.h>

// Local:
#include <AMDTPowerProfiling/src/ppCountersSelectionDialog.h>
#include <AMDTPowerProfiling/src/ppAidFunctions.h>
#include <AMDTPowerProfiling/Include/ppStringConstants.h>
#include <AMDTPowerProfiling/src/ppAppController.h>
#include <AMDTAPIClasses/Include/apProjectSettings.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>

// Register the gdBreakpointsItemData type so we can store retrieve from QVariant without casting
Q_DECLARE_METATYPE(AMDTPwrCounterDesc*)

// Constants for dialog sizes:
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #define PP_COUNTERS_SELECTION_LISTS_MIN_WIDTH        380
#else
    #define PP_COUNTERS_SELECTION_LISTS_MIN_WIDTH        280
#endif
#define PP_COUNTERS_SELECTION_LISTS_MIN_HEIGHT       440

#define PP_COUNTERS_NAME_COL    0
#define PP_COUNTERS_NUM_OF_COL  1

// ---------------------------------------------------------------------------
bool ppCountersSelectionDialog::OpenCountersSelectionDialog(AMDTPwrCategory category)
{
    bool retVal = false;

    // Check if this is a remote session.
    const apProjectSettings& theProjSettings = afProjectManager::instance().currentProjectSettings();
    bool isRemoteSession = theProjSettings.isRemoteTarget();

    // if the middle tier is not initialized try to init again, maybe it was a problem because another instance was holding the BE
    if (!ppAppController::instance().IsMidTierInitialized() || isRemoteSession)
    {
        ppAppController::instance().InitMiddleTier(true);
    }

    PPResult lastInitStatus = ppAppController::instance().MidTierInitError();

    if (ppAppController::instance().IsMidTierInitialized())
    {
        if ((lastInitStatus == PPR_WARNING_IGPU_DISABLED) || (lastInitStatus == PPR_WARNING_SMU_DISABLED))
        {
            QString warningMessage = (lastInitStatus == PPR_WARNING_IGPU_DISABLED) ? PP_STR_iGPUDisabledMsg : PP_STR_SmuDisabledMsg;
            acMessageBox::instance().warning(afGlobalVariablesManager::ProductNameA(), warningMessage);
        }

        ppCountersSelectionDialog dialog(nullptr);
        dialog.SetIntialCounterCategory(category);

        afApplicationCommands* pAppCommands = afApplicationCommands::instance();

        if (nullptr != pAppCommands)
        {
            int rc = pAppCommands->showModal(&dialog);

            if (QDialog::Accepted == rc)
            {
                retVal = true;
            }
        }
    }
    else
    {
        ppAppController::instance().ShowFailedErrorDialog();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
ppCountersSelectionDialog::ppCountersSelectionDialog(QWidget* pParent)
    : acDialog(afMainAppWindow::instance(), true, true, QDialogButtonBox::Ok),
      m_pMainGroupBox(nullptr), m_pTopLayoutV(nullptr), m_pMainLayoutH(nullptr), m_pCenterButtonsLayoutV(nullptr),
      m_pRightLayoutV(nullptr), m_pLeftLayoutV(nullptr),
      m_pBottomButtonsLayoutH(nullptr),
      m_pAddButton(nullptr), m_pRemoveButton(nullptr), m_pRemoveAllButton(nullptr),
      m_pAvailableCountersTW(nullptr), m_pSelectedCountersTW(nullptr),
      m_pDialogDescription(nullptr), m_pCounterDescription(nullptr),
      m_pAvailableCountersText(nullptr), m_pSelectedCountersText(nullptr),
      m_apuPowerCounterId(-1),
      m_dgpuPowerCounterId(-1),
      m_wereSettingsChanged(false)
{
    (void)(pParent); // unused

    // Set window flags
    Qt::WindowFlags flags = windowFlags();
    flags &= ~Qt::WindowContextHelpButtonHint;
    setWindowFlags(flags);

    // Get the CodeXL project type title as string:
    QString title = afGlobalVariablesManager::instance().ProductNameA();
    title.append(PP_STR_DialogTitle);

    // Set the title:
    setWindowTitle(title);

    // Add the Icon to the dialog:
    afLoadTitleBarIcon(this);

    // Build the dialog items and layout them:
    SetDialogLayout();

    LoadSelectedCounters();
}

// ---------------------------------------------------------------------------
ppCountersSelectionDialog::~ppCountersSelectionDialog()
{
}

// ---------------------------------------------------------------------------
void ppCountersSelectionDialog::SetIntialCounterCategory(AMDTPwrCategory category)
{
    // setting focus
    GT_IF_WITH_ASSERT(nullptr != m_pAvailableCountersTW)
    {
        int i = 0;
        bool categoryFound = false;

        while (i < m_pAvailableCountersTW->topLevelItemCount() && !categoryFound)
        {
            int currentCategory = m_pAvailableCountersTW->topLevelItem(i)->whatsThis(PP_COUNTERS_NAME_COL).toInt();

            if (currentCategory == category && m_pAvailableCountersTW->topLevelItem(i)->childCount() > 0)
            {
                m_pAvailableCountersTW->topLevelItem(i)->child(0)->setSelected(true);
                categoryFound = true;
            }

            i++;
        }

        // cannot focus on category received, focus on first node
        if (!categoryFound)
        {
            if (m_pAvailableCountersTW->topLevelItemCount() > 0 &&
                m_pAvailableCountersTW->topLevelItem(0)->childCount() > 0)
            {
                // set focus to the first node of left tree, available counters tree
                m_pAvailableCountersTW->topLevelItem(0)->child(0)->setSelected(true);
            }
            else
            {
                m_pAvailableCountersTW->setFocus(Qt::NoFocusReason);
            }
        }
    }
}

// ---------------------------------------------------------------------------
void ppCountersSelectionDialog::SetDialogLayout()
{
    // Create layouts:
    m_pTopLayoutV = new QVBoxLayout();
    m_pMainLayoutH = new QHBoxLayout();
    m_pCenterButtonsLayoutV = new QVBoxLayout();
    m_pRightLayoutV = new QVBoxLayout();
    m_pLeftLayoutV = new QVBoxLayout();
    m_pBottomButtonsLayoutH = new QHBoxLayout();

    // Create dialog labels:
    m_pDialogDescription = new QLabel(PP_STR_DialogDescription);
    m_pAvailableCountersText = new QLabel(PP_STR_AvailableCountersTitle);
    m_pSelectedCountersText = new QLabel(PP_STR_ActiveCountersTitle);
    m_pCounterDescription = new QTextEdit();

    int descMinHeight = acScalePixelSizeToDisplayDPI(80);
    m_pCounterDescription->setAlignment(Qt::AlignTop);
    m_pCounterDescription->setMinimumHeight(descMinHeight);
    m_pCounterDescription->setWordWrapMode(QTextOption::WordWrap);
    m_pCounterDescription->setFrameShape(QFrame::NoFrame);
    m_pCounterDescription->setTextInteractionFlags(Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse);
    // Setting background color to inactive
    QColor bgColor = acGetSystemDefaultBackgroundColor();
    QPalette p = m_pCounterDescription->palette();
    p.setColor(m_pCounterDescription->backgroundRole(), bgColor);
    p.setColor(QPalette::Base, bgColor);
    m_pCounterDescription->setPalette(p);

    // Group Box for all the dialog components:
    m_pMainGroupBox = new QGroupBox(PP_STR_DialogGroupBoxTitle);

    m_pBottomButtonsLayoutH = getBottomButtonLayout(false);

    // Add / Remove / Remove All buttons
    // Add Button:
    m_pAddButton = new QPushButton(AF_STR_AddButton, this);

    m_pAddButton->setDefault(false);
    m_pAddButton->setAutoDefault(false);
    m_pAddButton->setEnabled(false);

    // Remove Button:
    m_pRemoveButton = new QPushButton(AF_STR_RemoveButton, this);

    m_pRemoveButton->setDefault(false);
    m_pRemoveButton->setAutoDefault(false);
    m_pRemoveButton->setEnabled(false);

    // Remove All Button:
    m_pRemoveAllButton = new QPushButton(AF_STR_RemoveAllButton, this);

    m_pRemoveAllButton->setDefault(false);
    m_pRemoveAllButton->setAutoDefault(false);
    m_pRemoveAllButton->setEnabled(false);

    // Set the min size to the other buttons (according to the longest item):
    QSize minButtonSize = m_pRemoveAllButton->size();
    m_pRemoveAllButton->setMinimumSize(minButtonSize);
    m_pRemoveButton->setMinimumSize(minButtonSize);
    m_pAddButton->setMinimumSize(minButtonSize);

    // Left tree ctrl, displays all counters
    int listsMinWidth = acScalePixelSizeToDisplayDPI(PP_COUNTERS_SELECTION_LISTS_MIN_WIDTH);
    int listsMinHeight = acScalePixelSizeToDisplayDPI(PP_COUNTERS_SELECTION_LISTS_MIN_HEIGHT);
    m_pAvailableCountersTW = new acTreeCtrl(nullptr);
    m_pAvailableCountersTW->setMinimumSize(listsMinWidth, listsMinHeight);

    m_pAvailableCountersTW->setColumnCount(PP_COUNTERS_NUM_OF_COL);
    m_pAvailableCountersTW->setHeaderHidden(true);
    m_pAvailableCountersTW->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_pAvailableCountersTW->setItemDelegate(new acItemDelegate);
    m_pAvailableCountersTW->clear();

    // Right tree ctrl, displays selected counters
    m_pSelectedCountersTW = new acTreeCtrl(nullptr);
    m_pSelectedCountersTW->setMinimumSize(listsMinWidth, listsMinHeight);

    m_pSelectedCountersTW->setColumnCount(PP_COUNTERS_NUM_OF_COL);
    m_pSelectedCountersTW->setHeaderHidden(true);
    m_pSelectedCountersTW->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_pSelectedCountersTW->setItemDelegate(new acItemDelegate);
    m_pSelectedCountersTW->clear();

    // Load tree with data
    PopulateCountersDisplay();

    m_pAvailableCountersTW->expandAll();
    m_pSelectedCountersTW->expandAll();

    // Central layout with buttons:
    m_pCenterButtonsLayoutV->addStretch(1);
    m_pCenterButtonsLayoutV->addWidget(m_pAddButton, 0, Qt::AlignTop);
    m_pCenterButtonsLayoutV->addWidget(m_pRemoveButton, 0, Qt::AlignTop);
    m_pCenterButtonsLayoutV->addWidget(m_pRemoveAllButton, 0, Qt::AlignTop);
    m_pCenterButtonsLayoutV->addStretch(1);

    m_pLeftLayoutV->addWidget(m_pAvailableCountersText, 0, Qt::AlignLeft);
    m_pLeftLayoutV->addWidget(m_pAvailableCountersTW, 1, Qt::AlignTop);
    m_pLeftLayoutV->addWidget(m_pCounterDescription, 0, Qt::AlignTop);
    m_pLeftLayoutV->setMargin(5);

    m_pRightLayoutV->addWidget(m_pSelectedCountersText, 0, Qt::AlignLeft);
    m_pRightLayoutV->addWidget(m_pSelectedCountersTW, 1);
    m_pRightLayoutV->setMargin(5);

    m_pMainLayoutH->setContentsMargins(0, 3, 3, 3);
    m_pMainLayoutH->addLayout(m_pLeftLayoutV);
    m_pMainLayoutH->addLayout(m_pCenterButtonsLayoutV);
    m_pMainLayoutH->addLayout(m_pRightLayoutV);

    // Main GroupBox:
    m_pMainGroupBox->setLayout(m_pMainLayoutH);

    // Top layout:
    m_pTopLayoutV->addWidget(m_pDialogDescription, 0, Qt::AlignLeft);
    m_pTopLayoutV->addSpacing(3);
    m_pTopLayoutV->addWidget(m_pMainGroupBox, 0, Qt::AlignLeft);
    m_pTopLayoutV->addLayout(m_pBottomButtonsLayoutH);

    setLayout(m_pTopLayoutV);

    // Connect the buttons to slots:
    bool rc = connect(m_pAvailableCountersTW, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(OnLTItemDoubleClick()));
    GT_ASSERT(rc);

    rc = connect(m_pAvailableCountersTW, SIGNAL(itemSelectionChanged()), this, SLOT(OnLTSelectionChanged()));
    GT_ASSERT(rc);

    rc = connect(m_pSelectedCountersTW, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(OnRTItemDoubleClick()));
    GT_ASSERT(rc);

    rc = connect(m_pSelectedCountersTW, SIGNAL(itemSelectionChanged()), this, SLOT(OnRTSelectionChanged()));
    GT_ASSERT(rc);

    rc = connect(this, SIGNAL(accepted()), this, SLOT(OnOk()));
    GT_ASSERT(rc);

    rc = connect(m_pAddButton, SIGNAL(clicked()), this, SLOT(OnAdd()));
    GT_ASSERT(rc);

    rc = connect(m_pRemoveButton, SIGNAL(clicked()), this, SLOT(OnRemove()));
    GT_ASSERT(rc);

    rc = connect(m_pRemoveAllButton, SIGNAL(clicked()), this, SLOT(OnRemoveAll()));
    GT_ASSERT(rc);
}

// ---------------------------------------------------------------------------
void ppCountersSelectionDialog::OnOk()
{
    if (m_wereSettingsChanged)
    {
        // Check if we're in power profile:
        bool isPowerProfile = (SharedProfileManager::instance().selectedSessionTypeName() == PP_STR_OnlineProfileName);

        if (!isPowerProfile)
        {
            int userSelection = acMessageBox::instance().question(AF_STR_QuestionA, PP_STR_CounterSelectionSwitchToPowerProfileQuestion, QMessageBox::Yes | QMessageBox::No);

            if (userSelection == QMessageBox::Yes)
            {
                // Set the custom profile as selected:
                SharedProfileManager::instance().SelectProfileType(PP_STR_OnlineProfileName);
            }

            m_wereSettingsChanged = false;
        }
    }

    gtVector<int> counterIds;
    QTreeWidgetItemIterator it(m_pSelectedCountersTW);

    // Iterate over selected counters tree and create a list
    while (*it)
    {
        QTreeWidgetItem* pItem = it.operator * ();
        GT_IF_WITH_ASSERT(nullptr != pItem)
        {
            if (nullptr != pItem && pItem->childCount() == 0 && !pItem->isHidden())
            {
                // column 1 holds counter ID for nodes
                QString str = pItem->whatsThis(PP_COUNTERS_NAME_COL);
                counterIds.push_back(str.toInt());
            }
        }
        ++it;
    }

    if (counterIds.size() > 0)
    {
        ppAppController::instance().SetCurrentProjectEnabledCounters(counterIds);
    }
}

// ---------------------------------------------------------------------------
void ppCountersSelectionDialog::OnLTItemDoubleClick()
{
    GT_IF_WITH_ASSERT(nullptr != m_pAvailableCountersTW)
    {
        // double click event should result in 1 selected item
        if (m_pAvailableCountersTW->selectedItems().count() == 1)
        {
            QTreeWidgetItem* pItem = m_pAvailableCountersTW->selectedItems().at(0);

            GT_IF_WITH_ASSERT(nullptr != pItem)
            {
                // double click on a root item should expand / close the item and not act as add
                // call OnAdd only for a node item
                if (pItem->childCount() == 0)
                {
                    OnAdd();
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
void ppCountersSelectionDialog::OnRTItemDoubleClick()
{
    GT_IF_WITH_ASSERT(nullptr != m_pSelectedCountersTW)
    {

        // double click event should result in 1 selected item
        if (m_pSelectedCountersTW->selectedItems().count() == 1)
        {
            QTreeWidgetItem* pItem = m_pSelectedCountersTW->selectedItems().at(0);

            GT_IF_WITH_ASSERT(nullptr != pItem)
            {
                // double click on a root item should expand / close the item and not act as remove
                // call OnRemove only for a node item
                if (pItem->childCount() == 0)
                {
                    OnRemove();
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
void ppCountersSelectionDialog::OnAdd()
{
    GT_IF_WITH_ASSERT(nullptr != m_pAvailableCountersTW && nullptr != m_pSelectedCountersTW)
    {
        m_pAvailableCountersTW->blockSignals(true);

        // if a root is selected, select all items beneath it
        for (QTreeWidgetItem* pItem : m_pAvailableCountersTW->selectedItems())
        {
            GT_IF_WITH_ASSERT(nullptr != pItem)
            {
                if (pItem->childCount() > 0)
                {
                    for (int i = 0; i < pItem->childCount(); i++)
                    {
                        pItem->child(i)->setSelected(true);
                    }
                }
            }
        }

        // Iterate over the selected items in the left list:
        for (QTreeWidgetItem* pItem : m_pAvailableCountersTW->selectedItems())
        {
            GT_IF_WITH_ASSERT(nullptr != pItem)
            {
                if (pItem->childCount() == 0 && pItem->textColor(0) != Qt::blue)
                {
                    // Set the left list item color to blue, indicating that it is in the right list:
                    pItem->setTextColor(PP_COUNTERS_NAME_COL, Qt::blue);
                    // counter id is saved on the second column
                    int counterId = pItem->whatsThis(PP_COUNTERS_NAME_COL).toInt();
                    AddToSelectedCounterTree(counterId);
                }

                pItem->setSelected(false);
            }
        }

        SetButtonStates();
        SetSelectedCountersRootsVisibility();
        m_pAvailableCountersTW->blockSignals(false);

        // Mark that the counters list is changed:
        m_wereSettingsChanged = true;
    }
}

// ---------------------------------------------------------------------------
void ppCountersSelectionDialog::OnRemove()
{
    // Sanity Check:
    GT_IF_WITH_ASSERT(nullptr != m_pSelectedCountersTW && nullptr != m_pAvailableCountersTW)
    {
        m_pSelectedCountersTW->blockSignals(true);

        // if a root is selected, select all items beneath it
        for (QTreeWidgetItem* pItem : m_pSelectedCountersTW->selectedItems())
        {
            GT_IF_WITH_ASSERT(nullptr != pItem)
            {
                if (pItem->childCount() > 0)
                {
                    for (int i = 0; i < pItem->childCount(); i++)
                    {
                        pItem->child(i)->setSelected(true);
                    }
                }
            }
        }

        for (QTreeWidgetItem* pItem : m_pSelectedCountersTW->selectedItems())
        {
            GT_IF_WITH_ASSERT(nullptr != pItem)
            {
                // If a node
                if (pItem->childCount() == 0)
                {
                    if (pItem->text(0) == PP_STR_Counter_Power_TotalAPU)
                    {
                        // counter cannot be removed - notify user
                        acMessageBox::instance().information(PP_STR_CounterSelectionRemoveErrorPrefix, QString(PP_STR_Counter_Power_TotalAPU) + PP_STR_CounterSelectionRemoveErrorPostfix, QMessageBox::Ok);
                    }
                    else
                    {
                        // Find the item of the left tree with the same catalog item, set color back 2 black
                        QList<QTreeWidgetItem*> matchingItems = m_pAvailableCountersTW->findItems(pItem->text(PP_COUNTERS_NAME_COL), Qt::MatchExactly | Qt::MatchRecursive, 0);

                        // Every node on left tree must have a matching node in left tree
                        GT_IF_WITH_ASSERT(matchingItems.count() > 0)
                        {
                            matchingItems.at(0)->setTextColor(PP_COUNTERS_NAME_COL, Qt::black);
                        }

                        // Hide the tree item chosen
                        pItem->setHidden(true);
                    }
                }
            }
        }

        SetSelectedCountersRootsVisibility();

        SetButtonStates();
        m_pSelectedCountersTW->blockSignals(false);

        // Mark that the counters list is changed:
        m_wereSettingsChanged = true;
    }
}

// ---------------------------------------------------------------------------
void ppCountersSelectionDialog::OnRemoveAll()
{
    // Sanity Check:
    GT_IF_WITH_ASSERT(nullptr != m_pSelectedCountersTW && nullptr != m_pAvailableCountersTW)
    {
        m_pSelectedCountersTW->blockSignals(true);

        // Find all items being removed in the left tree and un-mark them
        for (int i = 0; i < m_pAvailableCountersTW->topLevelItemCount(); i++)
        {
            QTreeWidgetItem* rootItem = m_pAvailableCountersTW->topLevelItem(i);

            for (int j = 0; j < rootItem->childCount(); j++)
            {
                QTreeWidgetItem* pChild = rootItem->child(j);

                if (nullptr != pChild && pChild->text(PP_COUNTERS_NAME_COL) != PP_STR_Counter_Power_TotalAPU)
                {
                    pChild->setTextColor(PP_COUNTERS_NAME_COL, Qt::black);
                }
            }
        }

        for (int i = 0; i < m_pSelectedCountersTW->topLevelItemCount(); i++)
        {
            QTreeWidgetItem* rootItem = m_pSelectedCountersTW->topLevelItem(i);

            for (int j = 0; j < rootItem->childCount(); j++)
            {
                QTreeWidgetItem* pChild = rootItem->child(j);

                if (nullptr != pChild && pChild->text(PP_COUNTERS_NAME_COL) != PP_STR_Counter_Power_TotalAPU)
                {
                    pChild->setHidden(true);
                }
            }
        }

        if (m_apuPowerCounterId >= 0)
        {
            // After we cleared all counters, bring back the one counter that must be selected
            AddToSelectedCounterTree(m_apuPowerCounterId);
        }
        else
        {
            // if APU counter does not found
            if (m_dgpuPowerCounterId >= 0)
            {
                AddToSelectedCounterTree(m_dgpuPowerCounterId);
            }
        }

        SetSelectedCountersRootsVisibility();
        SetButtonStates();
        m_pSelectedCountersTW->blockSignals(false);

        // Mark that the counters list is changed:
        m_wereSettingsChanged = true;
    }
}

// ---------------------------------------------------------------------------
void ppCountersSelectionDialog::PopulateCountersDisplay()
{
    gtList<PPDevice*> systemDevices;

    ppAppController& appController = ppAppController::instance();

    for (int i = 0; i < AMDT_PWR_CATEGORY_CNT; i++)
    {
        // Get a list of all counters in current category
        gtSet<int> tmpCountersIdByCategory;
        bool rc = appController.GetAllCountersInCategory(AMDTPwrCategory(i), tmpCountersIdByCategory);
        gtVector<int> countersIdByCategory(tmpCountersIdByCategory.size());

        if (rc)
        {
            std::copy(tmpCountersIdByCategory.begin(), tmpCountersIdByCategory.end(), countersIdByCategory.begin());
        }

        // 'Process Id..' counters under count category are not supported
        if (rc && (AMDTPwrCategory(i) == AMDT_PWR_CATEGORY_COUNT) && (tmpCountersIdByCategory.size() > 0))
        {
            for (int j = countersIdByCategory.size() - 1; j >= 0; j--)
            {
                ppControllerCounterData* pCounterData = appController.GetCounterInformationById(countersIdByCategory[j]);

                if (nullptr != pCounterData && pCounterData->m_name.startsWith(PP_STR_CounterSelectionProcessIdPrefix, Qt::CaseInsensitive))
                {
                    countersIdByCategory.removeItem(j);
                }
            }
        }

        // If there are counters available in current category
        if (countersIdByCategory.size() > 0)
        {
            // Create category root
            QString categoryName = ppAidFunction::CounterCategoryToStr(AMDTPwrCategory(i));
            QTreeWidgetItem* pLTCategoryRoot = new QTreeWidgetItem();
            pLTCategoryRoot->setFlags(pLTCategoryRoot->flags() | Qt::ItemIsSelectable);
            pLTCategoryRoot->setText(PP_COUNTERS_NAME_COL, categoryName);
            pLTCategoryRoot->setWhatsThis(PP_COUNTERS_NAME_COL, QString::number(i));

            // Getting the icon for current category
            QPixmap pixmap;
            GT_ASSERT(acSetIconInPixmap(pixmap, ppAidFunction::GetCategoryIconId(AMDTPwrCategory(i))));
            QIcon icon(pixmap);
            pLTCategoryRoot->setIcon(PP_COUNTERS_NAME_COL, icon);

            QTreeWidgetItem* pRTCategoryRoot = new QTreeWidgetItem(*pLTCategoryRoot);
            // Determine the display sort order for the counters
            appController.SortCountersInCategory(AMDTPwrCategory(i), countersIdByCategory);

            // Create all tree items for this root - category
            for (int counterId : countersIdByCategory)
            {
                ppControllerCounterData* pCounterData = appController.GetCounterInformationById(counterId);

                GT_IF_WITH_ASSERT(nullptr != pCounterData)
                {
                    QTreeWidgetItem* pLTItem = new QTreeWidgetItem();
                    pLTItem->setText(PP_COUNTERS_NAME_COL, pCounterData->m_name);
                    pLTItem->setWhatsThis(PP_COUNTERS_NAME_COL, QString::number(counterId));

                    QIcon icon1(pixmap);
                    pLTItem->setIcon(PP_COUNTERS_NAME_COL, icon1);

                    pLTCategoryRoot->addChild(pLTItem);

                    QTreeWidgetItem* pRTItem = new QTreeWidgetItem(*pLTItem);
                    pRTCategoryRoot->addChild(pRTItem);
                    pRTItem->setData(0, Qt::UserRole, QVariant(counterId));
                }
            }

            // Sanity check, both trees should be identical
            GT_ASSERT(pLTCategoryRoot->childCount() == pRTCategoryRoot->childCount());

            m_pAvailableCountersTW->addTopLevelItem(pLTCategoryRoot);
            m_pSelectedCountersTW->addTopLevelItem(pRTCategoryRoot);
        }

        // Sanity check, both trees should be identical
        GT_ASSERT(m_pAvailableCountersTW->topLevelItemCount() == m_pSelectedCountersTW->topLevelItemCount());
    }
}

// ---------------------------------------------------------------------------
void ppCountersSelectionDialog::LoadSelectedCounters()
{
    gtVector<int> selectedCounters;
    ppAppController& appController = ppAppController::instance();

    appController.GetCurrentProjectEnabledCounters(selectedCounters);

    // 'Hide' the entire tree and then show only selected counters
    for (int i = 0; i < m_pSelectedCountersTW->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* rootItem = m_pSelectedCountersTW->topLevelItem(i);

        for (int j = 0; j < rootItem->childCount(); j++)
        {
            rootItem->child(j)->setHidden(true);
        }

        rootItem->setHidden(true);
    }

    // Create a map that per category id stores a list with all counters in that category
    // mark all selected counters in available counters tree
    for (int counterId : selectedCounters)
    {
        ppControllerCounterData* pCounterData = appController.GetCounterInformationById(counterId);

        GT_IF_WITH_ASSERT(nullptr != pCounterData)
        {
            // Find the item of the left tree with the same catalog item, set color back 2 black
            QList<QTreeWidgetItem*> matchingItemsOnLT = m_pAvailableCountersTW->findItems(pCounterData->m_name, Qt::MatchExactly | Qt::MatchRecursive, 0);
            QList<QTreeWidgetItem*> matchingItemsOnRT = m_pSelectedCountersTW->findItems(pCounterData->m_name, Qt::MatchExactly | Qt::MatchRecursive, 0);

            // Every node on left tree must have a matching node in left tree
            // However we might have counters that are not supported and therefor not on display
            if (matchingItemsOnLT.count() > 0 && matchingItemsOnLT.count() > 0)
            {
                matchingItemsOnLT.at(0)->setTextColor(PP_COUNTERS_NAME_COL, Qt::blue);

                if (pCounterData->m_name == PP_STR_Counter_Power_TotalAPU)
                {
                    m_apuPowerCounterId = counterId;
                }
                else if (pCounterData->m_name.contains(PP_STR_Counter_Power_DGPU))
                {
                    m_dgpuPowerCounterId = counterId;
                }

                matchingItemsOnRT.at(0)->setHidden(false);
            }
        }
    }

    SetSelectedCountersRootsVisibility();
}

// ---------------------------------------------------------------------------
void ppCountersSelectionDialog::SetButtonStates()
{
    GT_IF_WITH_ASSERT(nullptr != m_pAddButton && nullptr != m_pRemoveButton && nullptr != m_pRemoveAllButton)
    {
        GT_IF_WITH_ASSERT(nullptr != m_pAvailableCountersTW && nullptr != m_pSelectedCountersTW)
        {
            // isLeftSelected is true when we have at list 1 new node selected (not blue)
            bool isLeftSelected = false;
            QList <int> topLevelItemsSelected;

            for (QTreeWidgetItem* pItem : m_pAvailableCountersTW->selectedItems())
            {
                if (pItem->childCount() == 0)
                {
                    if (pItem->textColor(PP_COUNTERS_NAME_COL) != Qt::blue)
                    {
                        isLeftSelected = true;
                        break;
                    }
                }
                else
                {
                    // saving indexes of root items selected
                    int index = m_pAvailableCountersTW->indexOfTopLevelItem(pItem);

                    // if a valid index
                    if (index >= 0)
                    {
                        topLevelItemsSelected.push_back(index);
                    }
                }
            }

            // Add button will be enabled if we have at least one item to add
            // if we haven't found one yet, go over the roots selected and check if they have any node to add
            while (!isLeftSelected && topLevelItemsSelected.count() > 0)
            {
                int index = topLevelItemsSelected.front();
                topLevelItemsSelected.pop_front();
                QTreeWidgetItem* pItem = m_pAvailableCountersTW->topLevelItem(index);

                GT_IF_WITH_ASSERT(nullptr != pItem)
                {
                    for (int i = 0; i < pItem->childCount(); i++)
                    {
                        // found an item that was added already
                        if (pItem->child(i)->textColor(PP_COUNTERS_NAME_COL) != Qt::blue)
                        {
                            isLeftSelected = true;
                            break;
                        }
                    }
                }
            }

            if (m_pAddButton->isEnabled() != isLeftSelected)
            {
                m_pAddButton->setEnabled(isLeftSelected);
            }

            // isRightSelected is true when we have at list 1 node selected
            QList<QTreeWidgetItem*> selectedItemsOnRT = m_pSelectedCountersTW->selectedItems();
            bool isRightSelected = selectedItemsOnRT.count() > 0;

            if ((selectedItemsOnRT.count() == 1) && (selectedItemsOnRT[0]->text(PP_COUNTERS_NAME_COL) == PP_STR_Counter_Power_TotalAPU))
            {
                // special handling for selecting the POWER root, item cannot be removed
                isRightSelected = false;
            }

            if (m_pRemoveButton->isEnabled() != isRightSelected)
            {
                m_pRemoveButton->setEnabled(isRightSelected);
            }

            bool rightTreeHasItems = false;

            for (int i = 0; i < m_pSelectedCountersTW->topLevelItemCount(); i++)
            {
                if (!m_pSelectedCountersTW->topLevelItem(i)->isHidden())
                {
                    rightTreeHasItems = true;
                    break;
                }
            }

            if (m_pRemoveAllButton->isEnabled() != rightTreeHasItems)
            {
                m_pRemoveAllButton->setEnabled(rightTreeHasItems);
            }
        }
    }
}

// ---------------------------------------------------------------------------
void ppCountersSelectionDialog::OnLTSelectionChanged()
{
    SetButtonStates();

    // Update selected counter description
    QString description;
    QString heading;

    int selectionCount = m_pAvailableCountersTW->selectedItems().count();

    if (selectionCount > 1)
    {
        // Multiple selection
        heading = PP_STR_MultipleCountersSelectedHeading;
        description = PP_STR_MultipleCountersSelectedMsg;
    }
    else if (selectionCount == 1 && m_pAvailableCountersTW->selectedItems().at(0)->childCount() == 0)
    {
        ppAppController& appController = ppAppController::instance();

        // Only one node is selected
        int counterId = m_pAvailableCountersTW->selectedItems().at(0)->whatsThis(PP_COUNTERS_NAME_COL).toInt();
        ppControllerCounterData* pCounterData = appController.GetCounterInformationById(counterId);

        GT_IF_WITH_ASSERT(nullptr != pCounterData)
        {
            heading = pCounterData->m_name + AF_STR_Colon;
            description = pCounterData->m_description;
        }
    }

    // else {} no description displayed per root item

    QString text = QString(PP_STR_CounterDescriptionCaption).arg(heading).arg(description);
    m_pCounterDescription->setText(text);
}

// ---------------------------------------------------------------------------
void ppCountersSelectionDialog::OnRTSelectionChanged()
{
    SetButtonStates();
}

// ---------------------------------------------------------------------------
void ppCountersSelectionDialog::AddToSelectedCounterTree(int counterId)
{
    ppAppController& appController = ppAppController::instance();
    ppControllerCounterData* pCounterData = appController.GetCounterInformationById(counterId);

    GT_IF_WITH_ASSERT((nullptr != pCounterData) && (m_pSelectedCountersTW != nullptr))
    {
        // Look for the item with the same counter id:
        // Notice: We do not look for the name, since the name is not unique (in case of 2 dGPUs):
        QTreeWidgetItemIterator iter(m_pSelectedCountersTW);
        QTreeWidgetItem* pMatchingItem = nullptr;

        while (*iter)
        {
            if (!(*iter)->data(0, Qt::UserRole).isNull())
            {
                if ((*iter)->data(0, Qt::UserRole).toInt() == counterId)
                {
                    pMatchingItem = *iter;
                    break;
                }
            }

            iter++;
        }

        GT_IF_WITH_ASSERT(pMatchingItem != nullptr)
        {
            pMatchingItem->setHidden(false);
        }
    }
}

// ---------------------------------------------------------------------------
void  ppCountersSelectionDialog::SetSelectedCountersRootsVisibility()
{
    for (int i = 0; i < m_pSelectedCountersTW->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* rootItem = m_pSelectedCountersTW->topLevelItem(i);
        bool hasSelectedCounters = false;

        for (int j = 0; j < rootItem->childCount(); j++)
        {
            if (!rootItem->child(j)->isHidden())
            {
                hasSelectedCounters = true;
                break;
            }
        }

        rootItem->setHidden(!hasSelectedCounters);
    }
}

// ---------------------------------------------------------------------------