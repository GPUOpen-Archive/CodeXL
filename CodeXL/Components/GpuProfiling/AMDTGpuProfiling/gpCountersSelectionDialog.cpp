//------------------------------ gpCountersSelectionDialog.cpp ------------------------------

// Compiler warnings:
#include <qtIgnoreCompilerWarnings.h>
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
#include <AMDTGpuProfiling/gpCountersSelectionDialog.h>
#include <AMDTGpuProfiling/gpExecutionMode.h>
#include <AMDTGpuProfiling/gpSessionView.h>
#include <AMDTGpuProfiling/ProfileManager.h>

// Register the gdBreakpointsItemData type so we can store retrieve from QVariant without casting
//Q_DECLARE_METATYPE(AMDTPwrCounterDesc *)

// Constants for dialog sizes:
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #define PP_COUNTERS_SELECTION_LISTS_MIN_WIDTH        380
#else
    #define PP_COUNTERS_SELECTION_LISTS_MIN_WIDTH        280
#endif
#define PP_COUNTERS_SELECTION_LISTS_MIN_HEIGHT       440

#define GP_COUNTERS_NAME_COL    0
#define GP_COUNTERS_NUM_OF_COL  1

// ---------------------------------------------------------------------------
gpCountersSelectionDialog::gpCountersSelectionDialog(QString& selectedPreset)
    : acDialog(afMainAppWindow::instance(), true, true, QDialogButtonBox::Ok),
      m_pAddButton(nullptr), m_pRemoveButton(nullptr), m_pRemoveAllButton(nullptr),
      m_pAvailableCounters(nullptr), m_pSelectedCounters(nullptr),
      m_pDialogDescription(nullptr), m_pCounterDescription(nullptr),
      m_pAvailableCountersText(nullptr), m_pSelectedCountersText(nullptr),
      m_wereSettingsChanged(false)
{
    // Set window flags
    Qt::WindowFlags flags = windowFlags();
    flags &= ~Qt::WindowContextHelpButtonHint;
    setWindowFlags(flags);

    // Get the CodeXL project type title as string:
    QString title = afGlobalVariablesManager::instance().ProductNameA();
    title.append(GPU_STR_counterSelectionDialogTitle);

    // Set the title:
    setWindowTitle(title);

    // Add the Icon to the dialog:
    afLoadTitleBarIcon(this);

    // Build the dialog items and layout them:
    SetDialogLayout();

    if (m_pAvailableCounters != nullptr && m_pSelectedCounters != nullptr)
    {
        // all counters are available in both tree but in the selected the are initially all hidden
        AddAvailableCounters(m_pAvailableCounters);
        AddAvailableCounters(m_pSelectedCounters);

        SetInitialSelectedCounters(selectedPreset);
        SetSelectedCountersRootsVisibility();
        SetButtonStates();
        m_pAvailableCounters->expandAll();
        m_pSelectedCounters->expandAll();
    }
}

// ---------------------------------------------------------------------------
gpCountersSelectionDialog::~gpCountersSelectionDialog()
{
}

// ---------------------------------------------------------------------------
void gpCountersSelectionDialog::SetDialogLayout()
{
    // Create layouts:
    QVBoxLayout* pTopLayoutV = new QVBoxLayout();
    QHBoxLayout* pMainLayoutH = new QHBoxLayout();
    QVBoxLayout* pCenterButtonsLayoutV = new QVBoxLayout();
    QVBoxLayout* pRightLayoutV = new QVBoxLayout();
    QVBoxLayout* pLeftLayoutV = new QVBoxLayout();
    QHBoxLayout* pBottomButtonsLayoutH = new QHBoxLayout();

    // Create dialog labels:
    m_pDialogDescription = new QLabel(GPU_STR_counterSelectionDialogDescription);
    m_pAvailableCountersText = new QLabel(GPU_STR_counterSelectionDialogAvailableCountersTitle);
    m_pSelectedCountersText = new QLabel(GPU_STR_counterSelectionDialogActiveCountersTitle);
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
    QGroupBox* pMainGroupBox = new QGroupBox(GPU_STR_counterSelectionDialogGroupBoxTitle);

    pBottomButtonsLayoutH = getBottomButtonLayout(false);

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
    m_pAvailableCounters = new acTreeCtrl(nullptr);
    m_pAvailableCounters->setMinimumSize(listsMinWidth, listsMinHeight);

    m_pAvailableCounters->setColumnCount(GP_COUNTERS_NUM_OF_COL);
    m_pAvailableCounters->setHeaderHidden(true);
    m_pAvailableCounters->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_pAvailableCounters->setItemDelegate(new acItemDelegate);
    m_pAvailableCounters->clear();

    // Right tree ctrl, displays selected counters
    m_pSelectedCounters = new acTreeCtrl(nullptr);
    m_pSelectedCounters->setMinimumSize(listsMinWidth, listsMinHeight);

    m_pSelectedCounters->setColumnCount(GP_COUNTERS_NUM_OF_COL);
    m_pSelectedCounters->setHeaderHidden(true);
    m_pSelectedCounters->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_pSelectedCounters->setItemDelegate(new acItemDelegate);
    m_pSelectedCounters->clear();

    QLabel* pNewPresetLabel = new QLabel(GPU_STR_counterSelectionDialogPresetLabel);

    m_pNewPresetName = new QTextEdit();

    // Central layout with buttons:
    pCenterButtonsLayoutV->addStretch(1);
    pCenterButtonsLayoutV->addWidget(m_pAddButton, 0, Qt::AlignTop);
    pCenterButtonsLayoutV->addWidget(m_pRemoveButton, 0, Qt::AlignTop);
    pCenterButtonsLayoutV->addWidget(m_pRemoveAllButton, 0, Qt::AlignTop);
    pCenterButtonsLayoutV->addStretch(1);

    pLeftLayoutV->addWidget(m_pAvailableCountersText, 0, Qt::AlignLeft);
    pLeftLayoutV->addWidget(m_pAvailableCounters, 1, Qt::AlignTop);
    pLeftLayoutV->addWidget(m_pCounterDescription, 0, Qt::AlignTop);
    pLeftLayoutV->setMargin(5);

    pRightLayoutV->addWidget(m_pSelectedCountersText, 0, Qt::AlignLeft);
    pRightLayoutV->addWidget(m_pSelectedCounters, 1);
    pRightLayoutV->addWidget(pNewPresetLabel);
    pRightLayoutV->addWidget(m_pNewPresetName, 0);
    pRightLayoutV->setMargin(5);

    pMainLayoutH->setContentsMargins(0, 3, 3, 3);
    pMainLayoutH->addLayout(pLeftLayoutV);
    pMainLayoutH->addLayout(pCenterButtonsLayoutV);
    pMainLayoutH->addLayout(pRightLayoutV);

    // Main GroupBox:
    pMainGroupBox->setLayout(pMainLayoutH);

    // Top layout:
    pTopLayoutV->addWidget(m_pDialogDescription, 0, Qt::AlignLeft);
    pTopLayoutV->addSpacing(3);
    pTopLayoutV->addWidget(pMainGroupBox, 0, Qt::AlignLeft);
    pTopLayoutV->addLayout(pBottomButtonsLayoutH);

    setLayout(pTopLayoutV);

    // Connect the buttons to slots:
    bool rc = connect(m_pAvailableCounters, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(OnLTItemDoubleClick()));
    GT_ASSERT(rc);

    rc = connect(m_pAvailableCounters, SIGNAL(itemSelectionChanged()), this, SLOT(OnLTSelectionChanged()));
    GT_ASSERT(rc);

    rc = connect(m_pSelectedCounters, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(OnRTItemDoubleClick()));
    GT_ASSERT(rc);

    rc = connect(m_pSelectedCounters, SIGNAL(itemSelectionChanged()), this, SLOT(OnRTSelectionChanged()));
    GT_ASSERT(rc);

    rc = connect(m_pAddButton, SIGNAL(clicked()), this, SLOT(OnAdd()));
    GT_ASSERT(rc);

    rc = connect(m_pRemoveButton, SIGNAL(clicked()), this, SLOT(OnRemove()));
    GT_ASSERT(rc);

    rc = connect(m_pRemoveAllButton, SIGNAL(clicked()), this, SLOT(OnRemoveAll()));
    GT_ASSERT(rc);

    // we don't want auto accept
    QList<QPushButton*> pushButtons = findChildren<QPushButton*>();

    for (int i = 0; i < pushButtons.count(); i++)
    {
        if (pushButtons[i]->text() == "OK")
        {
            rc = disconnect(pushButtons[i], SIGNAL(clicked()), this, SLOT(accept()));
            GT_ASSERT(rc);
            rc = connect(pushButtons[i], SIGNAL(clicked()), this, SLOT(OnOk()));
            GT_ASSERT(rc);
        }
    }
}

void gpCountersSelectionDialog::AddAvailableCounters(acTreeCtrl* pTreeCtrl)
{

    // Sanity check:
    gpExecutionMode* pFrameAnalysisManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();
    GT_IF_WITH_ASSERT((pTreeCtrl != nullptr) && (pFrameAnalysisManager != nullptr))
    {
        const std::map<QString, gpPerfCounter*>& countersMap = pFrameAnalysisManager->ProjectSettings().CountersMap();
        std::map<QString, gpPerfCounter*>::const_iterator counterIt = countersMap.begin();

        while (counterIt != countersMap.end())
        {
            gpPerfCounter* pCounterData = (*counterIt).second;

            if (pCounterData != nullptr)
            {
                QTreeWidgetItem* pParent = nullptr;
                QList<QTreeWidgetItem*> itemsFound = pTreeCtrl->findItems(pCounterData->m_parent, Qt::MatchCaseSensitive | Qt::MatchFixedString);

                if (itemsFound.size() == 1)
                {
                    pParent = itemsFound[0];
                }
                else if (itemsFound.size() == 0)
                {
                    // Create one item to the available tree
                    pParent = new QTreeWidgetItem(QStringList(pCounterData->m_parent));
                    pTreeCtrl->addTopLevelItem(pParent);
                }
                else
                {
                    // There can't be more than one parent with the same name
                    GT_ASSERT(false);
                }

                GT_IF_WITH_ASSERT(pParent != nullptr)
                {
                    QTreeWidgetItem* pChild = new QTreeWidgetItem(QStringList(pCounterData->m_name));
                    pParent->addChild(pChild);

                    QVariant dataAsVaraiant = qVariantFromValue((void*)pCounterData);
                    pChild->setData(0, Qt::UserRole, dataAsVaraiant);
                }
            }

            counterIt++;
        }
    }
}

void gpCountersSelectionDialog::SetInitialSelectedCounters(QString& selectedPreset)
{
    OnRemoveAll();
    m_wereSettingsChanged = false;

    // Sanity check:
    gpExecutionMode* pFrameAnalysisManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();
    GT_IF_WITH_ASSERT(pFrameAnalysisManager != nullptr)
    {
        const std::map<QString, gpPerfCounter*>& countersMap = pFrameAnalysisManager->ProjectSettings().CountersMap();
        std::map<QString, QStringList>& presetCountersLists = pFrameAnalysisManager->ProjectSettings().PresetCountersLists();


        int numCounters = presetCountersLists[selectedPreset].size();

        for (int nCounter = 0; nCounter < numCounters; nCounter++)
        {
            // check if the counter is available for the specific device that we got the image from
            if (countersMap.count(presetCountersLists[selectedPreset][nCounter]) == 1)
            {
                const QStringList& presetCountersList = presetCountersLists.at(selectedPreset);
                QString currentCounterName = presetCountersList.at(nCounter);
                gpPerfCounter* pCounterData = countersMap.at(currentCounterName);

                GT_IF_WITH_ASSERT(nullptr != pCounterData && nullptr != m_pAvailableCounters)
                {
                    // Look for the item in the selected tree. there should be one item exactly.
                    QTreeWidgetItem* pItem = nullptr;
                    QList<QTreeWidgetItem*> itemsFound = m_pAvailableCounters->findItems(pCounterData->m_name, Qt::MatchCaseSensitive | Qt::MatchFixedString | Qt::MatchRecursive);

                    if (itemsFound.size() == 1)
                    {
                        pItem = itemsFound[0];
                    }
                    else
                    {
                        // There can't be more than one parent with the same name
                        GT_ASSERT(false);
                    }

                    GT_IF_WITH_ASSERT(pItem != nullptr)
                    {
                        // Set the left list item color to blue, indicating that it is in the right list:
                        pItem->setTextColor(GP_COUNTERS_NAME_COL, Qt::blue);

                        QVariant itemData = pItem->data(0, Qt::UserRole);
                        AddToSelectedCounterTree(itemData.value<void*>());
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
void gpCountersSelectionDialog::OnOk()
{
    bool accecptOK = true;

    // Sanity check:
    gpExecutionMode* pFrameAnalysisManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();
    GT_IF_WITH_ASSERT(pFrameAnalysisManager != nullptr)
    {
        std::map<QString, QStringList>& presetCountersLists = pFrameAnalysisManager->ProjectSettings().PresetCountersLists();

        m_newPresetName = m_pNewPresetName->toPlainText();

        if (m_wereSettingsChanged)
        {
            if (m_newPresetName.isEmpty())
            {
                accecptOK = false;
                acMessageBox::instance().information(AF_STR_InformationA, GPU_STR_counterSelectionDialogMustEnterNameError);
            }
            else if (presetCountersLists.count(m_newPresetName) == 1)
            {
                accecptOK = false;
                // check that the set does not already appear
                acMessageBox::instance().information(AF_STR_InformationA, GPU_STR_counterSelectionDialogMustEnterNewNameError);
            }
            else
            {
                // save the set and exist
                // get all the selected items (visible items in the selected list)
                QStringList countersList;

                for (int i = 0; i < m_pSelectedCounters->topLevelItemCount(); i++)
                {
                    QTreeWidgetItem* rootItem = m_pSelectedCounters->topLevelItem(i);

                    for (int j = 0; j < rootItem->childCount(); j++)
                    {
                        QTreeWidgetItem* pChild = rootItem->child(j);

                        if (!pChild->isHidden())
                        {
                            countersList.append(pChild->text(0));
                        }
                    }
                }

                presetCountersLists[m_newPresetName] = countersList;
            }
        }

        if (accecptOK)
        {
            accept();
        }
    }
}

// ---------------------------------------------------------------------------
void gpCountersSelectionDialog::OnLTItemDoubleClick()
{
    GT_IF_WITH_ASSERT(nullptr != m_pAvailableCounters)
    {
        // double click event should result in 1 selected item
        if (m_pAvailableCounters->selectedItems().count() == 1)
        {
            QTreeWidgetItem* pItem = m_pAvailableCounters->selectedItems().at(0);

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
void gpCountersSelectionDialog::OnRTItemDoubleClick()
{
    GT_IF_WITH_ASSERT(nullptr != m_pSelectedCounters)
    {

        // double click event should result in 1 selected item
        if (m_pSelectedCounters->selectedItems().count() == 1)
        {
            QTreeWidgetItem* pItem = m_pSelectedCounters->selectedItems().at(0);

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
void gpCountersSelectionDialog::OnAdd()
{
    GT_IF_WITH_ASSERT(nullptr != m_pAvailableCounters && nullptr != m_pSelectedCounters)
    {
        m_pAvailableCounters->blockSignals(true);

        // if a root is selected, select all items beneath it
        for (QTreeWidgetItem* pItem : m_pAvailableCounters->selectedItems())
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
        for (QTreeWidgetItem* pItem : m_pAvailableCounters->selectedItems())
        {
            GT_IF_WITH_ASSERT(nullptr != pItem)
            {
                if (pItem->childCount() == 0 && pItem->textColor(0) != Qt::blue)
                {
                    // Set the left list item color to blue, indicating that it is in the right list:
                    pItem->setTextColor(GP_COUNTERS_NAME_COL, Qt::blue);

                    QVariant itemData = pItem->data(0, Qt::UserRole);
                    AddToSelectedCounterTree(itemData.value<void*>());
                }

                pItem->setSelected(false);
            }
        }

        SetButtonStates();
        SetSelectedCountersRootsVisibility();
        m_pAvailableCounters->blockSignals(false);

        // Mark that the counters list is changed:
        m_wereSettingsChanged = true;
    }
}

// ---------------------------------------------------------------------------
void gpCountersSelectionDialog::OnRemove()
{
    // Sanity Check:
    GT_IF_WITH_ASSERT(nullptr != m_pSelectedCounters && nullptr != m_pAvailableCounters)
    {
        m_pSelectedCounters->blockSignals(true);

        // if a root is selected, select all items beneath it
        for (QTreeWidgetItem* pItem : m_pSelectedCounters->selectedItems())
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

        for (QTreeWidgetItem* pItem : m_pSelectedCounters->selectedItems())
        {
            GT_IF_WITH_ASSERT(nullptr != pItem)
            {
                // If a node
                if (pItem->childCount() == 0)
                {
                    // Find the item of the left tree with the same catalog item, set color back 2 black
                    QList<QTreeWidgetItem*> matchingItems = m_pAvailableCounters->findItems(pItem->text(GP_COUNTERS_NAME_COL), Qt::MatchExactly | Qt::MatchRecursive, 0);

                    // Every node on left tree must have a matching node in left tree
                    GT_IF_WITH_ASSERT(matchingItems.count() > 0)
                    {
                        matchingItems.at(0)->setTextColor(GP_COUNTERS_NAME_COL, Qt::black);
                    }

                    // Hide the tree item chosen
                    pItem->setHidden(true);
                }
            }
        }

        SetSelectedCountersRootsVisibility();

        SetButtonStates();
        m_pSelectedCounters->blockSignals(false);

        // Mark that the counters list is changed:
        m_wereSettingsChanged = true;
    }
}

// ---------------------------------------------------------------------------
void gpCountersSelectionDialog::OnRemoveAll()
{
    // Sanity Check:
    GT_IF_WITH_ASSERT(nullptr != m_pSelectedCounters && nullptr != m_pAvailableCounters)
    {
        m_pSelectedCounters->blockSignals(true);

        // Find all items being removed in the left tree and un-mark them
        for (int i = 0; i < m_pAvailableCounters->topLevelItemCount(); i++)
        {
            QTreeWidgetItem* rootItem = m_pAvailableCounters->topLevelItem(i);

            for (int j = 0; j < rootItem->childCount(); j++)
            {
                QTreeWidgetItem* pChild = rootItem->child(j);
                pChild->setTextColor(GP_COUNTERS_NAME_COL, Qt::black);
            }
        }

        for (int i = 0; i < m_pSelectedCounters->topLevelItemCount(); i++)
        {
            QTreeWidgetItem* rootItem = m_pSelectedCounters->topLevelItem(i);

            for (int j = 0; j < rootItem->childCount(); j++)
            {
                QTreeWidgetItem* pChild = rootItem->child(j);
                pChild->setHidden(true);
            }
        }

        SetSelectedCountersRootsVisibility();
        SetButtonStates();
        m_pSelectedCounters->blockSignals(false);

        // Mark that the counters list is changed:
        m_wereSettingsChanged = true;
    }
}

// ---------------------------------------------------------------------------
void gpCountersSelectionDialog::SetButtonStates()
{
    GT_IF_WITH_ASSERT(nullptr != m_pAddButton && nullptr != m_pRemoveButton && nullptr != m_pRemoveAllButton)
    {
        GT_IF_WITH_ASSERT(nullptr != m_pAvailableCounters && nullptr != m_pSelectedCounters)
        {
            // isLeftSelected is true when we have at list 1 new node selected (not blue)
            bool isLeftSelected = false;
            QList <int> topLevelItemsSelected;

            for (QTreeWidgetItem* pItem : m_pAvailableCounters->selectedItems())
            {
                if (pItem->childCount() == 0)
                {
                    if (pItem->textColor(GP_COUNTERS_NAME_COL) != Qt::blue)
                    {
                        isLeftSelected = true;
                        break;
                    }
                }
                else
                {
                    // saving indexes of root items selected
                    int index = m_pAvailableCounters->indexOfTopLevelItem(pItem);

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
                QTreeWidgetItem* pItem = m_pAvailableCounters->topLevelItem(index);

                GT_IF_WITH_ASSERT(nullptr != pItem)
                {
                    for (int i = 0; i < pItem->childCount(); i++)
                    {
                        // found an item that was added already
                        if (pItem->child(i)->textColor(GP_COUNTERS_NAME_COL) != Qt::blue)
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
            QList<QTreeWidgetItem*> selectedItemsOnRT = m_pSelectedCounters->selectedItems();
            bool isRightSelected = selectedItemsOnRT.count() > 0;

            if (m_pRemoveButton->isEnabled() != isRightSelected)
            {
                m_pRemoveButton->setEnabled(isRightSelected);
            }

            bool rightTreeHasItems = false;

            for (int i = 0; i < m_pSelectedCounters->topLevelItemCount(); i++)
            {
                if (!m_pSelectedCounters->topLevelItem(i)->isHidden())
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
void gpCountersSelectionDialog::OnLTSelectionChanged()
{
    SetButtonStates();

    // Update selected counter description
    QString description;
    QString heading;

    GT_IF_WITH_ASSERT(nullptr != m_pAvailableCounters && nullptr != m_pCounterDescription)
    {
        int selectionCount = m_pAvailableCounters->selectedItems().count();

        if (selectionCount > 1)
        {
            // Multiple selection
            heading = GPU_STR_counterSelectionDialogMultipleCountersSelectedHeading;
            description = GPU_STR_counterSelectionDialogMultipleCountersSelectedMsg;
        }
        else if (selectionCount == 1 && m_pAvailableCounters->selectedItems().at(0)->childCount() == 0)
        {
            QTreeWidgetItem* pItem = m_pAvailableCounters->selectedItems().at(0);
            QVariant itemData = pItem->data(0, Qt::UserRole);
            gpPerfCounter* pCounterData = (gpPerfCounter*)itemData.value<void*>();

            if (pCounterData != nullptr)
            {
                heading = pCounterData->m_name;
                description = pCounterData->m_description;
            }
        }

        // else {} no description displayed per root item

        QString text = QString(GPU_STR_counterSelectionDialogCounterDescriptionCaption).arg(heading).arg(description);
        m_pCounterDescription->setText(text);
    }
}

// ---------------------------------------------------------------------------
void gpCountersSelectionDialog::OnRTSelectionChanged()
{
    SetButtonStates();
}

// ---------------------------------------------------------------------------
void gpCountersSelectionDialog::AddToSelectedCounterTree(void* gpCounterData)
{
    // Look for the item with the same counter id:
    // Notice: We do not look for the name, since the name is not unique (in case of 2 dGPUs):
    QTreeWidgetItemIterator iter(m_pSelectedCounters);
    QTreeWidgetItem* pMatchingItem = nullptr;

    while (*iter)
    {
        if (!(*iter)->data(0, Qt::UserRole).isNull())
        {
            QVariant itemData = (*iter)->data(0, Qt::UserRole);

            if (itemData.value<void*>() == gpCounterData)
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
        pMatchingItem->parent()->setExpanded(true);
    }
}

// ---------------------------------------------------------------------------
void  gpCountersSelectionDialog::SetSelectedCountersRootsVisibility()
{
    GT_IF_WITH_ASSERT(nullptr != m_pSelectedCounters)
    {
        for (int i = 0; i < m_pSelectedCounters->topLevelItemCount(); i++)
        {
            QTreeWidgetItem* pRootItem = m_pSelectedCounters->topLevelItem(i);
            bool hasSelectedCounters = false;

            for (int j = 0; j < pRootItem->childCount(); j++)
            {
                if (!pRootItem->child(j)->isHidden())
                {
                    hasSelectedCounters = true;
                    break;
                }
            }

            pRootItem->setHidden(!hasSelectedCounters);
        }
    }
}
