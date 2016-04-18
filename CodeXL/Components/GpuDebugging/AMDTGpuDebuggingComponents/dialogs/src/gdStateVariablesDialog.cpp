//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdStateVariablesDialog.cpp
///
//==================================================================================

//------------------------------ gdStateVariablesDialog.cpp ------------------------------

// Warnings:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTAPIClasses/Include/apOpenGLAPIType.h>
#include <AMDTAPIClasses/Include/apOpenGLStateVariableId.h>
#include <AMDTApplicationComponents/Include/acDisplay.h>
#include <AMDTApplicationComponents/Include/acListCtrl.h>
#include <AMDTApplicationComponents/Include/acQTextFilterCtrl.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/dialogs/gdStateVariablesDialog.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdStateVariablesView.h>

// Dialog items sizes:
#define GD_STATE_VARS_DIALOG_LISTS_WIDTH 250
#define GD_STATE_VARS_DIALOG_AVAILABLE_LIST_HEIGHT 300

// ---------------------------------------------------------------------------
// Name:        gdStateVariablesDialog::gdStateVariablesDialog
// Description: Constructor
// Arguments:   QWidget* pParent
// Return Val:
// Author:      Yoni Rabin
// Date:        18/4/2012
// ---------------------------------------------------------------------------
gdStateVariablesDialog::gdStateVariablesDialog(QWidget* pParent)
    : QDialog(afMainAppWindow::instance()),
      m_pMainHorizontalLayout(NULL), m_pMainVerticalLayout(NULL), m_pButtonsVerticalLayout(NULL), m_pListVerticalLayout(NULL), m_pChosenListVerticalLayout(NULL),
      m_pAddButton(NULL), m_pRemoveButton(NULL), m_pRemoveAllButton(NULL),
      m_pList(NULL),  m_pChosenList(NULL),
      m_pDescription(NULL), m_pListText(NULL), m_pChosenListText(NULL),
      m_pStatesFilter(NULL),
      m_activatedStateVar(""),
      m_changingList(false)
{
    (void)(pParent); // unused
    // Set window flags (minimize / maximize / close buttons):
    Qt::WindowFlags flags = windowFlags();
    flags &= ~Qt::WindowContextHelpButtonHint;
    setWindowFlags(flags);

    // Add the Icon to the dialog
    afLoadTitleBarIcon(this);

    // Get the CodeXL project type title as string and add the specific dialog caption:
    QString title = afGlobalVariablesManager::instance().ProductNameA();
    title.append(acGTStringToQString(GD_STR_StateVariablesViewCaptionDefault));
    setWindowTitle(title);

    // Set the layout components:
    doLayout();

    // Connect events:
    bool rc = connect(m_pAddButton, SIGNAL(clicked()), this, SLOT(onAdd()));
    GT_ASSERT(rc);
    rc = connect(m_pRemoveButton, SIGNAL(clicked()), this, SLOT(onRemove()));
    GT_ASSERT(rc);
    rc = connect(m_pRemoveAllButton, SIGNAL(clicked()), this, SLOT(onRemoveAll()));
    GT_ASSERT(rc);
    rc = connect(m_pList, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(onAdd()));
    GT_ASSERT(rc);
    rc = connect(m_pChosenList, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(onRemove()));
    GT_ASSERT(rc);
    rc = connect(m_pChosenList, SIGNAL(keyPressed(QKeyEvent*)), this, SLOT(onChosenListKeyPressed(QKeyEvent*)));
    GT_ASSERT(rc);
    rc = connect(m_pStatesFilter, SIGNAL(textChanged(const QString&)), this, SLOT(onFilteredTextChanged(const QString&)));
    GT_ASSERT(rc);
    rc = connect(m_pList, SIGNAL(currentItemChanged(QTableWidgetItem*, QTableWidgetItem*)), this, SLOT(onLeftListSelectionChanged()));
    GT_ASSERT(rc);
    rc = connect(m_pChosenList, SIGNAL(currentItemChanged(QTableWidgetItem*, QTableWidgetItem*)), this, SLOT(onRightListSelectionChanged()));
    GT_ASSERT(rc);
    rc = connect(m_pList, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(onLeftListSelectionChanged()));
    GT_ASSERT(rc);
    rc = connect(m_pChosenList, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(onRightListSelectionChanged()));
    GT_ASSERT(rc);
    rc = connect(m_pStatesFilter, SIGNAL(focused(bool)), this, SLOT(onFilterFocused(bool)));
    GT_ASSERT(rc);

}

// ---------------------------------------------------------------------------
// Name:        gdStateVariablesDialog::~gdStateVariablesDialog
// Description: Destructor
// Author:      Yoni Rabin
// Date:        19/4/2012
// ---------------------------------------------------------------------------
gdStateVariablesDialog::~gdStateVariablesDialog()
{
}

// ---------------------------------------------------------------------------
// Name:        gdStateVariablesDialog::setActivatedStateVar
// Description: Setter for ActivatedStateVar member
// Arguments:   gtString activatedStateVar
// Return Val:  void
// Author:      Yoni Rabin
// Date:        18/4/2012
// ---------------------------------------------------------------------------
void gdStateVariablesDialog::setActivatedStateVar(const QString& activatedStateVar)
{
    m_activatedStateVar = activatedStateVar;
    selectActivatedStateVar();

}

// ---------------------------------------------------------------------------
// Name:        gdStateVariablesDialog::selectActivatedStateVar
// Description: Select the activated row from the left list
// Author:      Yoni Rabin
// Date:        24/5/2012
// ---------------------------------------------------------------------------
void gdStateVariablesDialog::selectActivatedStateVar()
{
    GT_IF_WITH_ASSERT(m_pList != NULL)
    {
        bool found = false;

        if (m_activatedStateVar.length() > 0)
        {
            for (int i = 0; i < m_pList->rowCount(); ++i)
            {
                QTableWidgetItem* pItem = m_pList->item(i, 0);
                GT_IF_WITH_ASSERT(NULL != pItem)
                {
                    if (m_activatedStateVar == pItem->text())
                    {
                        m_pList->setCurrentItem(pItem, QItemSelectionModel::SelectCurrent);
                        //m_pList->setFocus(Qt::MouseFocusReason);
                        found = true;
                        break;
                    }
                }
            }
        }

        if (!found)
        {
            m_activatedStateVar = "";
        }
    }
    //onLeftListSelectionChanged();
}


// ---------------------------------------------------------------------------
// Name:        gdStateVariablesDialog::getValidStateVariableTypesMask
// Description: Retrieves the valid state variable types mask for the current platform
// Author:      Yoni Rabin
// Date:        19/4/2012
// ---------------------------------------------------------------------------
unsigned int gdStateVariablesDialog::getValidStateVariableTypesMask()
{
    unsigned int validStateVariablesMask = AP_OPENGL_STATE_VAR;

    // On Mac OpenGL (Not OpenGL ES) - CGL state variables are also valid:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    {
        validStateVariablesMask |= AP_CGL_STATE_VAR;
    }
    // On Windows OpenGL (Not OpenGL ES) - WGL state variables are also valid:
#elif (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    {
        validStateVariablesMask |= AP_WGL_STATE_VAR;
    }
#endif

    return validStateVariablesMask;
}

// ---------------------------------------------------------------------------
// Name:        gdStateVariablesDialog::doLayout
// Description: Building the dialog
// Author:      Yoni Rabin
// Date:        18/4/2012
// ---------------------------------------------------------------------------
void gdStateVariablesDialog::doLayout()
{
    // Group Box for all the dialog components:
    m_pMainGroupBox = new QGroupBox(GD_STR_StateVariableBoxTitle);

    // Create layouts:
    m_pMainVerticalLayout = new QVBoxLayout();
    m_pMainHorizontalLayout = new QHBoxLayout;

    // Layout for the buttons:
    m_pButtonsVerticalLayout = new QVBoxLayout;
    m_pListVerticalLayout = new QVBoxLayout;
    m_pChosenListVerticalLayout = new QVBoxLayout;

    // Place the horizontal layout in the titled group box:
    m_pMainGroupBox->setLayout(m_pMainHorizontalLayout);

    // Description of the dialog:
    m_pDescription = new QLabel(GD_STR_StateVariableDescription);

    // Description of the listCtrls
    m_pListText = new QLabel(GD_STR_StateVariableListText);

    // Description of chosen (right) listCtrl
    m_pChosenListText = new QLabel(GD_STR_StateVariableChoosenListText);

    // Initialize left acListCtrl:
    m_pList = new acListCtrl(this, AC_DEFAULT_LINE_HEIGHT, false);

    m_pList->setAutoScroll(true);
    m_pList->setColumnCount(1);
    m_pList->horizontalHeader()->hide();
    m_pList->verticalHeader()->hide();

    // Initialize Selected State variables list control:
    m_pChosenList = new acListCtrl(this, AC_DEFAULT_LINE_HEIGHT, true);

    m_pChosenList->setAutoScroll(true);
    m_pChosenList->setColumnCount(1);
    m_pChosenList->horizontalHeader()->hide();
    m_pChosenList->verticalHeader()->hide();

    // Create Buttons:

    // Add Button:
    m_pAddButton = new QPushButton(AF_STR_AddButton, this);
    m_pAddButton->setEnabled(false);

    // Remove Button:
    m_pRemoveButton = new QPushButton(AF_STR_RemoveButton, this);
    m_pRemoveButton->setEnabled(false);

    // Remove All Button:
    m_pRemoveAllButton = new QPushButton(AF_STR_RemoveAllButton, this);
    m_pRemoveAllButton->setEnabled(false);

    // Set the min size to the other buttons (according to the longest item):
    QSize minButtonSize = m_pRemoveButton->size();
    m_pRemoveAllButton->setMinimumSize(minButtonSize);
    m_pRemoveButton->setMinimumSize(minButtonSize);
    m_pAddButton->setMinimumSize(minButtonSize);

    // Text filter:
    m_pStatesFilter = new acQTextFilterCtrl();
    m_pStatesFilter->setFixedWidth(acScalePixelSizeToDisplayDPI(GD_STATE_VARS_DIALOG_LISTS_WIDTH));
    m_pStatesFilter->setFocusPolicy(Qt::StrongFocus);

    // Set list sizes:
    m_pList->setMinimumSize(acScalePixelSizeToDisplayDPI(GD_STATE_VARS_DIALOG_LISTS_WIDTH), acScalePixelSizeToDisplayDPI(GD_STATE_VARS_DIALOG_AVAILABLE_LIST_HEIGHT));
    m_pChosenList->setMinimumSize(acScalePixelSizeToDisplayDPI(GD_STATE_VARS_DIALOG_LISTS_WIDTH), acScalePixelSizeToDisplayDPI(GD_STATE_VARS_DIALOG_AVAILABLE_LIST_HEIGHT + 5));

    // Add the OK + Cancel buttons:
    QDialogButtonBox* pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    pButtonBox->setCenterButtons(true);

    // Connect the buttons to slots:
    bool rc = connect(pButtonBox, SIGNAL(accepted()), this, SLOT(onOk()));
    GT_ASSERT(rc);
    rc = connect(pButtonBox, SIGNAL(rejected()), this, SLOT(reject()));
    GT_ASSERT(rc);

    // Add the items to the layouts:
    m_pButtonsVerticalLayout->addStretch(1);
    m_pButtonsVerticalLayout->addWidget(m_pAddButton, 0, Qt::AlignTop);
    m_pButtonsVerticalLayout->setSpacing(15);
    m_pButtonsVerticalLayout->addWidget(m_pRemoveButton, 0, Qt::AlignTop);
    m_pButtonsVerticalLayout->setSpacing(15);
    m_pButtonsVerticalLayout->addWidget(m_pRemoveAllButton, 0, Qt::AlignTop);
    m_pButtonsVerticalLayout->addStretch(1);

    m_pListVerticalLayout->addWidget(m_pListText);
    m_pListVerticalLayout->addWidget(m_pList, 1);
    m_pListVerticalLayout->addWidget(m_pStatesFilter, 1, Qt::AlignTop);
    m_pListVerticalLayout->setMargin(5);

    m_pChosenListVerticalLayout->addWidget(m_pChosenListText);
    m_pChosenListVerticalLayout->addWidget(m_pChosenList, 1, Qt::AlignTop);
    m_pChosenListVerticalLayout->setMargin(5);

    // Layout for the two variables lists and the buttons:
    m_pMainHorizontalLayout->setMargin(10);
    m_pMainHorizontalLayout->addLayout(m_pListVerticalLayout, 1);
    m_pMainHorizontalLayout->addLayout(m_pButtonsVerticalLayout, 0);
    m_pMainHorizontalLayout->addLayout(m_pChosenListVerticalLayout, 1);

    // Add the dialog description:
    m_pMainVerticalLayout->setMargin(10);
    m_pMainVerticalLayout->addWidget(m_pDescription, 0, Qt::AlignLeft);

    // Add the dialog and the Ok/Cancel buttons to the final Layout:
    m_pMainVerticalLayout->addWidget(m_pMainGroupBox, 1, Qt::AlignLeft);
    m_pMainVerticalLayout->addWidget(pButtonBox, 0, Qt::AlignRight);
    m_pMainVerticalLayout->addSpacing(10);
    m_pMainVerticalLayout->setSizeConstraint(QLayout::SetFixedSize);

    rc = setDialogMonitoredVariables();
    GT_ASSERT(rc);

    // Populate left list with state variables:
    rc = setDialogActiveVariables();
    GT_ASSERT(rc);

    // Initialize the filter with the left list of monitored State Variables:
    m_pStatesFilter->initialize(m_pList);

    // Activate:
    setLayout(m_pMainVerticalLayout);

    // Put the filter line in focus:
    m_pStatesFilter->setFocus(Qt::ActiveWindowFocusReason);
}

// ---------------------------------------------------------------------------
// Name:        gdStateVariablesDialog::onAdd
// Description: Add a state variable
//              Copy the State variable from m_pList to m_pChosenList
// Author:      Yoni Rabin
// Date:        18/4/2012
// ---------------------------------------------------------------------------
void gdStateVariablesDialog::onAdd()
{
    // Sanity Check:
    GT_IF_WITH_ASSERT(m_pList != NULL && m_pChosenList != NULL)
    {
        // Iterate over the selected items in the left list:
        foreach (QTableWidgetItem* pVar, m_pList->selectedItems())
        {
            GT_IF_WITH_ASSERT(pVar != NULL)
            {
                // Get the item data:
                QVariant itemData = pVar->data(Qt::UserRole);
                int varID = itemData.toInt();

                // Get the item name:
                QString varName = pVar->text();

                // Add the item to the right list:
                addChosenListItem(varID, varName);

                // Set the left list item color to blue, indicating that it is in the right list:
                pVar->setTextColor(Qt::blue);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStateVariablesDialog::onRemove
// Description: Is called when the user marks one item or more for removal from the right list.
//              Deletes the variable/s from the _pChosenStateVarsList
// Author:      Yoni Rabin
// Date:        19/4/2012
// ---------------------------------------------------------------------------
void gdStateVariablesDialog::onRemove()
{
    // Sanity Check:
    GT_IF_WITH_ASSERT(m_pList != NULL && m_pChosenList != NULL)
    {
        // Iterate over the selected items in the right list:
        foreach (QTableWidgetItem* var, m_pChosenList->selectedItems())
        {
            GT_IF_WITH_ASSERT(var != NULL)
            {
                // Get the item data:
                QVariant itemData = var->data(Qt::UserRole);
                int rightVarId = itemData.toInt();

                // Get the item name:
                QString varName = var->text();

                // Remove the row:
                m_pChosenList->removeRow(var->row());

                // Get the item in the left list using the stored Id - which is set to be the item's index in the list:
                // this works because the left list doesn't allow deletion:
                for (int i = 0; i < m_pList->rowCount(); ++i)
                {
                    QTableWidgetItem* pVarLeft = m_pList->item(i, 0);
                    GT_IF_WITH_ASSERT(pVarLeft != NULL)
                    {
                        int leftVarId = pVarLeft->data(Qt::UserRole).toInt();

                        // If we find the removed item set its color to black and continue:
                        if (rightVarId == leftVarId)
                        {
                            pVarLeft->setTextColor(Qt::black);
                            break;
                        }
                    }
                }
            }
        }

        if (m_pChosenList->rowCount() == 0)
        {
            m_pRemoveButton->setEnabled(false);
            m_pRemoveAllButton->setEnabled(false);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStateVariablesDialog::onRemoveAll
// Description: Remove all variables
//              Delete all variables from the m_pChosenList and reset the left items color
// Return Val:  void
// Author:      Yoni Rabin
// Date:        19/4/2012
// ---------------------------------------------------------------------------
void gdStateVariablesDialog::onRemoveAll()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pList != NULL && m_pChosenList != NULL)
    {
        // Remove all selected State vars from the right list control:
        m_pChosenList->clearList();

        // Color all left list control items in black:
        for (int i = 0; i < m_pList->rowCount(); ++i)
        {
            QTableWidgetItem* pVarLeft = m_pList->item(i, 0);
            // Set the color of left list items back to black:
            GT_IF_WITH_ASSERT(pVarLeft != NULL)
            {
                pVarLeft->setTextColor(Qt::black);
            }
        }

        m_pRemoveButton->setEnabled(false);
        m_pRemoveAllButton->setEnabled(false);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStateVariablesDialog::setDialogMonitoredVariables
// Description: Populate the left list with the states
// Return Val:  bool - Success / failure.
// Author:      Yoni Rabin
// Date:        19/4/2012
// ---------------------------------------------------------------------------
bool gdStateVariablesDialog::setDialogMonitoredVariables()
{
    bool retVal = true;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pList != NULL)
    {
        gtString stateVariableName;

        // Clear the current list content:
        m_pList->clearList();

        // Get a mask of valid state variable types for the current project and platform:
        unsigned int validStateVariablesMask = getValidStateVariableTypesMask();

        // Get the amount of state variables:
        int amountOfStateVariables = 0;
        retVal = gaGetAmountOfOpenGLStateVariables(amountOfStateVariables);
        GT_IF_WITH_ASSERT(retVal)
        {
            // Iterate the state variables:
            for (int stateVariableId = 0; stateVariableId < amountOfStateVariables; stateVariableId++)
            {
                // Get the function type (OpenGL / OpenGL ES):
                unsigned int stateVariableGlobalType = 0;
                GT_IF_WITH_ASSERT(gaGetOpenGLStateVariableGlobalType(stateVariableId, stateVariableGlobalType))
                {
                    // If this is a valid state variable (for the current project and platform):
                    bool isStateVariableTypeValid = ((stateVariableGlobalType & validStateVariablesMask) != 0);

                    if (isStateVariableTypeValid)
                    {
                        // If we failed to get the monitored variable name:
                        if (!gaGetOpenGLStateVariableName(stateVariableId, stateVariableName))
                        {
                            stateVariableName = GD_STR_StateVariablesUnknownVar;
                        }

                        // Add the located state variable to the left list:
                        bool rc = addListItem(m_pList, stateVariableName, stateVariableId);
                        GT_ASSERT(rc);
                        retVal = retVal && rc;
                    }
                }
            }

            // Sort the State variables by name:
            m_pList->sortByColumn(0, Qt::AscendingOrder);
            // int rows = m_pList->rowCount();
            m_pList->show();

        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdStateVariablesDialog::setDialogActiveVariables
// Description: Add the active variables into the dialog
// Return Val:  bool - Success / failure.
// Author:      Yoni Rabin
// Date:        19/4/2012
// ---------------------------------------------------------------------------
bool gdStateVariablesDialog::setDialogActiveVariables()
{
    bool rc = true;
    int amountOfSelectedVariables = 0;

    // Get the application command instance:
    gdApplicationCommands* pApplicationCommands = gdApplicationCommands::gdInstance();
    GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
    {
        // Get the state variables view:
        gdStateVariablesView* pStateVariableView = pApplicationCommands->stateVariablesView();
        GT_IF_WITH_ASSERT(pStateVariableView != NULL)
        {
            amountOfSelectedVariables = pStateVariableView->numberOfVariableRows();

            // Iterate on the selected OpenGL State Variables:
            for (int i = 0; i < amountOfSelectedVariables; i++)
            {
                // Get the Selected Variable name:
                QString varName;
                pStateVariableView->getItemText(i, 0, varName);

                // Get the Variable List index:
                int varListIndex = varNameToListIndex(varName);

                GT_IF_WITH_ASSERT(varListIndex != -1)
                {
                    QVariant itemData;
                    m_pList->getItemData(varListIndex, itemData);
                    GT_ASSERT(itemData.type() == QVariant::Int);

                    // Add the State variable Id and name to the right list:
                    rc = rc && addChosenListItem(itemData.toInt(), varName);
                }
                else
                {
                    gtString errorMessage = L"Unknown variable: ";
                    errorMessage.append(acQStringToGTString(varName));
                    rc = false;
                    GT_ASSERT_EX(0, errorMessage.asCharArray());
                }
            }
        }
    }
    // Color the chosen state vars in the left list:
    colorActiveInLeftListCtrl();

    GT_ASSERT(rc);
    return true;
}

// ---------------------------------------------------------------------------
// Name:        gdStateVariablesDialog::varNameToListIndex
// Description: Finds the index of a certain name in the left list
// Arguments:   const gtString &varName
// Return Val:  int
// Author:      Yoni Rabin
// Date:        19/4/2012
// ---------------------------------------------------------------------------
int gdStateVariablesDialog::varNameToListIndex(const QString& varName)
{
    GT_ASSERT(m_pList != NULL);
    int varListIndex = -1;

    for (int i = 0; i < m_pList->rowCount(); ++i)
    {
        QTableWidgetItem* pVarLeft = m_pList->item(i, 0);
        GT_IF_WITH_ASSERT(pVarLeft != NULL)
        {
            // Get the current state variable name:
            if (varName == pVarLeft->text())
            {
                // return the index of the state variable:
                varListIndex = pVarLeft->row();
                break;
            }
        }
    }

    return varListIndex;
}

// ---------------------------------------------------------------------------
// Name:        gdStateVariablesDialog::colorActiveInLeftListCtrl
// Description: Colors the active state variables in the left list control.
// Author:      Yoni Rabin
// Date:        22/4/2012
// ---------------------------------------------------------------------------
void gdStateVariablesDialog::colorActiveInLeftListCtrl()
{
    GT_ASSERT(m_pChosenList != NULL);

    // Loop over right list:
    for (int i = 0; i < m_pChosenList->rowCount(); ++i)
    {
        QTableWidgetItem* pChosenItem = m_pChosenList->item(i, 0);
        GT_IF_WITH_ASSERT(pChosenItem != NULL)
        {
            int rightId = pChosenItem->data(Qt::UserRole).toInt();

            // Loop over left list:
            for (int j = 0; j < m_pList->rowCount(); ++j)
            {
                QTableWidgetItem* pItem = m_pList->item(j, 0);
                GT_IF_WITH_ASSERT(pItem != NULL)
                {
                    int leftId = pItem->data(Qt::UserRole).toInt();

                    // If we have found a match, set the color to blue:
                    if (leftId == rightId)
                    {
                        pItem->setTextColor(Qt::blue);
                        break;
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStateVariablesDialog::addChosenListItem
// Description: Add a State Variable to the right list control (_pChosenStateVarsList)
// Arguments:   int stateVarID
//              const QString& varName
// Return Val:  bool - Success / failure.
// Author:      Yoni Rabin
// Date:        19/4/2012
// ---------------------------------------------------------------------------
bool gdStateVariablesDialog::addChosenListItem(int varID, const QString& varName)
{
    GT_ASSERT(m_pChosenList != NULL);
    bool rc = true;

    bool isVarAlreadyMarked = false;

    for (int i = 0; i < m_pChosenList->rowCount(); ++i)
    {
        QTableWidgetItem* pItem = m_pChosenList->item(i, 0);
        GT_IF_WITH_ASSERT(pItem != NULL)
        {
            QVariant item = pItem->data(Qt::UserRole);
            GT_ASSERT(item.type() == QVariant::Int);

            // Check if the state variable exists in right list:
            if (varID == item.toInt())
            {
                isVarAlreadyMarked = true;
                break;
            }
        }
    }

    // Check whether the specified State var is not already present in the right list:
    if (!isVarAlreadyMarked)
    {
        gtString gtVarName;
        gtVarName.fromASCIIString(varName.toLatin1());
        rc = addListItem(m_pChosenList, gtVarName, varID);
    }

    GT_RETURN_WITH_ASSERT(rc);
}

// ---------------------------------------------------------------------------
// Name:        gdStateVariablesDialog::onOk
// Description:
// Author:      Yoni Rabin
// Date:        12/4/2012
// ---------------------------------------------------------------------------
void gdStateVariablesDialog::onOk()
{
    m_selectedList.clear();
    int numItems = m_pChosenList->rowCount();

    for (int i = 0; i < numItems; ++i)
    {
        gtString variableName;
        m_pChosenList->getItemText(i, 0, variableName);
        m_selectedList.push_back(variableName);
    }

    accept();
}

const gtVector<gtString>& gdStateVariablesDialog::getSelectedStateVariables()
{
    return m_selectedList;
}

// ---------------------------------------------------------------------------
// Name:        gdStateVariablesDialog::addListItem
// Description: Add a new row to a list
// Arguments:   acListCtrl* pList
//              const gtString& name
//              int id
// Return Val:  bool - Success / failure.
// Author:      Yoni Rabin
// Date:        19/4/2012
// ---------------------------------------------------------------------------
bool gdStateVariablesDialog::addListItem(acListCtrl* pList, const gtString& name, int id)
{
    bool retVal = false;
    GT_IF_WITH_ASSERT(pList != NULL)
    {
        QVariant itemData(id);
        QTableWidgetItem* pItem = new QTableWidgetItem(acGTStringToQString(name));
        pItem->setData(Qt::UserRole, itemData);
        pList->addRow(acGTStringToQString(name), itemData);
        retVal = true;
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdStateVariablesDialog::onChosenListKeyPressed
// Description: Responds to delete and backspace and removes the selected rows
// Arguments:   QKeyEvent* pEvent
// Author:      Yoni Rabin
// Date:        23/4/2012
// ---------------------------------------------------------------------------
void gdStateVariablesDialog::onChosenListKeyPressed(QKeyEvent* pEvent)
{
    GT_IF_WITH_ASSERT(pEvent != NULL)
    {
        if (Qt::Key_Delete == pEvent->key() || Qt::Key_Backspace == pEvent->key())
        {
            onRemove();
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdStateVariablesDialog::onLeftListSelectionChanged
// Description: slot to update button status
// Author:      Yoni Rabin
// Date:        17/5/2012
// ---------------------------------------------------------------------------
void gdStateVariablesDialog::onLeftListSelectionChanged()
{
    if (!m_changingList)
    {
        m_changingList = true;
        bool isLeftSelected = (m_pList->amountOfSelectedRows() > 0);
        bool isRightSelected = (m_pChosenList->amountOfSelectedRows() > 0);

        if (!m_pList->hasFocus() && isLeftSelected && m_activatedStateVar.isEmpty())
        {
            m_pList->clearSelection();
            isLeftSelected = false;
        }

        if (!m_pChosenList->hasFocus() && isRightSelected)
        {
            m_pChosenList->clearSelection();
            isRightSelected = false;
        }

        setButtonStates(true);
        //m_pList->setFocus(Qt::MouseFocusReason);
        m_changingList = false;
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStateVariablesDialog::onRightListSelectionChanged
// Description: slot to update button status
// Author:      Yoni Rabin
// Date:        17/5/2012
// ---------------------------------------------------------------------------
void gdStateVariablesDialog::onRightListSelectionChanged()
{
    GT_IF_WITH_ASSERT(m_pList != NULL && m_pChosenList != NULL)
    {
        if (!m_changingList)
        {
            m_changingList = true;
            bool isLeftSelected = (m_pList->amountOfSelectedRows() > 0);
            bool isRightSelected = (m_pChosenList->amountOfSelectedRows() > 0);

            if (!m_pList->hasFocus() && isLeftSelected)
            {
                m_pList->clearSelection();
                isLeftSelected = false;
            }

            if (!m_pChosenList->hasFocus() && isRightSelected)
            {
                m_pChosenList->clearSelection();
                isRightSelected = false;
            }

            setButtonStates(false);
            m_changingList = false;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStateVariablesDialog::setButtonStates
// Description: enables/disables buttons
// Arguments:   bool hasLeftChanged
// Return Val:  void
// Author:      Yoni Rabin
// Date:        24/5/2012
// ---------------------------------------------------------------------------
void gdStateVariablesDialog::setButtonStates(bool hasLeftChanged)
{
    (void)(hasLeftChanged); // unused
    GT_IF_WITH_ASSERT(m_pAddButton != NULL && m_pRemoveButton != NULL && m_pRemoveAllButton != NULL)
    {
        bool isLeftSelected = (m_pList->amountOfSelectedRows() > 0);

        if (m_pAddButton->isEnabled() != isLeftSelected)
        {
            m_pAddButton->setEnabled(isLeftSelected);
        }

        bool isRightSelected = (m_pChosenList->amountOfSelectedRows() > 0);

        if (m_pRemoveButton->isEnabled() != isRightSelected)
        {
            m_pRemoveButton->setEnabled(isRightSelected);
        }

        bool anyRowsToRemove = (m_pChosenList->rowCount() > 0);

        if (m_pRemoveAllButton->isEnabled() != anyRowsToRemove)
        {
            m_pRemoveAllButton->setEnabled(anyRowsToRemove);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStateVariablesDialog::onFilteredTextChanged
// Description: colors the select rows blue
// Arguments:   const QString & filterText
// Return Val:  void
// Author:      Yoni Rabin
// Date:        16/5/2012
// ---------------------------------------------------------------------------
void gdStateVariablesDialog::onFilteredTextChanged(const QString& filterText)
{
    QString currentFilter = m_pStatesFilter->GetFilterString();

    if (currentFilter != filterText)
    {
        colorActiveInLeftListCtrl();
    }
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        onFilterFocused
/// \brief Description: If the filter has been clicked while it is in initial state, remove text
/// \param[in]          hasFocus
/// \return void
/// -----------------------------------------------------------------------------------------------
void gdStateVariablesDialog::onFilterFocused(bool hasFocus)
{
    GT_IF_WITH_ASSERT(m_pStatesFilter != NULL)
    {
        if (hasFocus)
        {
            if (m_pStatesFilter->isDefaultString())
            {
                m_pStatesFilter->clear();
            }
        }
    }
}
