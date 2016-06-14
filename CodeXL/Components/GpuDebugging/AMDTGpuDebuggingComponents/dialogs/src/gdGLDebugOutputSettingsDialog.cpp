//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdGLDebugOutputSettingsDialog.cpp
///
//==================================================================================

//------------------------------ gdGLDebugOutputSettingsDialog.cpp ------------------------------


// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/apGenericBreakpoint.h>
#include <AMDTAPIClasses/Include/apGLDebugOutput.h>
#include <AMDTAPIClasses/Include/Events/apGLDebugOutputMessageEvent.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/src/afUtils.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/dialogs/gdGLDebugOutputSettingsDialog.h>


#define MIN_DIALOG_WIDTH 400

// Access to the table:
#define GD_DEBUG_OUTPUT_CHECKBOX_INDEX(source, type) (type * AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES + source)


// ---------------------------------------------------------------------------
// Name:        gdGLDebugOutputSettingsDialog::gdGLDebugOutputSettingsDialog
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        7/6/2010
// ---------------------------------------------------------------------------
gdGLDebugOutputSettingsDialog::gdGLDebugOutputSettingsDialog(QWidget* pParent)
    : acDialog(pParent, true, true, QDialogButtonBox::Ok),
      m_pBreakOnGLDebugOutputReportsCheckbox(NULL),
      m_pGLDebugOutputLoggingEnableCheckbox(NULL),
      m_pSeverityText(NULL),
      m_pGLCategoriesText(NULL),
      m_isUpdatingCheckboxes(false)
{
    // Initialize array members:
    for (int i = 0; i < AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES; i++)
    {
        m_pSeverityCheckBoxes[i] = NULL;
    }

    for (int i = 0; i < AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES; i++)
    {
        m_pSourceHeaderLabels[i] = NULL;
        m_pSourceHeaderCheckBoxes[i] = NULL;
    }

    for (int i = 0; i < AP_NUMBER_OF_USER_VISIBLE_DEBUG_OUTPUT_TYPES; i++)
    {
        m_pTypeHeaderLabels[i] = NULL;
        m_pTypeHeaderCheckBoxes[i] = NULL;
    }

    for (int i = 0; i < AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES * AP_NUMBER_OF_USER_VISIBLE_DEBUG_OUTPUT_TYPES; i++)
    {
        m_pKindCheckBoxes[i] = NULL;
    }

    // Get the new project file name:
    gdGDebuggerGlobalVariablesManager& globalVarsManager = gdGDebuggerGlobalVariablesManager::instance();
    GT_UNREFERENCED_PARAMETER(&globalVarsManager);

    setWindowTitle(GD_STR_GLDebugOutputTitle);

    Qt::WindowFlags flags = windowFlags();
    flags &= ~Qt::WindowContextHelpButtonHint;
    setWindowFlags(flags);


    // Initial the dialog
    setFrameLayout();

    // Set the dialog initial values:
    setDialogInitialValues();

    connect(this, SIGNAL(accepted()), this, SLOT(onAccept()));
}

// ---------------------------------------------------------------------------
// Name:        gdGLDebugOutputSettingsDialog::setFrameLayout
// Description: Build the dialog.
// Author:      Sigal Algranaty
// Date:        7/6/2010
// ---------------------------------------------------------------------------
void gdGLDebugOutputSettingsDialog::setFrameLayout()
{
    // Create the main layout:
    QVBoxLayout* pMainLayout = new QVBoxLayout;
    pMainLayout->addSpacing(10);
    pMainLayout->addWidget(new QLabel(GD_STR_GLDebugOutputSubTitle), 0, Qt::AlignLeft);
    pMainLayout->addSpacing(10);

    // Allocate a text for the categories:
    QGroupBox* pGeneralSettingsGroupBox = new QGroupBox(GD_STR_GLDebugOutputGeneralSettings);


    QVBoxLayout* pGeneralSettingsLayout = new QVBoxLayout;


    // Add the Break on GLDebugOutput Reports:
    m_pGLDebugOutputLoggingEnableCheckbox = new QCheckBox(GD_STR_GLDebugOutputLoggingEnabled);


    bool rc = connect(m_pGLDebugOutputLoggingEnableCheckbox, SIGNAL(stateChanged(int)), this, SLOT(onGLDebugOutputLoggingEnableClick(int)));
    GT_ASSERT(rc);


    // Add the Break on GLDebugOutput Reports:
    m_pBreakOnGLDebugOutputReportsCheckbox = new QCheckBox(GD_STR_GLDebugOutputBreakOnReports);


    // Add the check boxes to the sizer:
    pGeneralSettingsLayout->addWidget(m_pGLDebugOutputLoggingEnableCheckbox, 0, Qt::AlignLeft | Qt::AlignTop);
    pGeneralSettingsLayout->addSpacing(5);
    pGeneralSettingsLayout->addWidget(m_pBreakOnGLDebugOutputReportsCheckbox, 0);
    pGeneralSettingsLayout->addSpacing(5);


    // Create the severity combo box sizer:
    QVBoxLayout* pSeveritiesLayout = new QVBoxLayout;

    // Create a text for the severity combo box:
    m_pSeverityText = new QLabel(GD_STR_GLDebugOutputMessagesSeverity);

    // Add the text to the combo box sizer:
    pSeveritiesLayout->addWidget(m_pSeverityText, 0, Qt::AlignLeft);
    pSeveritiesLayout->addStretch();

    // Get the debug output severities:
    for (int i = 0; i < AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES; i++)
    {
        // Get the value of the current severity:
        bool isCurrentSeverityEnabled = false;
        rc = gaGetGLDebugOutputSeverityEnabled((apGLDebugOutputSeverity)i, isCurrentSeverityEnabled);
        GT_ASSERT(rc);

        gtString severityAsString;
        rc = apGLDebugOutputSeverityAsString((apGLDebugOutputSeverity)i, severityAsString);
        GT_ASSERT(rc);

        m_pSeverityCheckBoxes[i] = new QCheckBox(severityAsString.asASCIICharArray());
        pSeveritiesLayout->addWidget(m_pSeverityCheckBoxes[i], 0, Qt::AlignLeft);
    }

    // Add the combo box to the sizer:
    pGeneralSettingsLayout->addLayout(pSeveritiesLayout, 1);

    pGeneralSettingsGroupBox->setLayout(pGeneralSettingsLayout);

    // Add the general setting sizer to the main one:
    pMainLayout->addWidget(pGeneralSettingsGroupBox, 0, Qt::AlignLeft);
    pMainLayout->addSpacing(10);

    QGridLayout* pMessageKindsLayout = new QGridLayout;
    pMessageKindsLayout->setContentsMargins(5, 5, 5, 5);

    gtString sourceHeaderStrings[AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES];

    for (int s = 0; s < AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES; s++)
    {
        // Add header checkboxes for sources:
        bool rcSrc = apGLDebugOutputSourceAsString((apGLDebugOutputSource)s, sourceHeaderStrings[s]);
        GT_ASSERT(rcSrc);

        m_pSourceHeaderLabels[s] = new QLabel(sourceHeaderStrings[s].asASCIICharArray());
        m_pSourceHeaderCheckBoxes[s] = new QCheckBox;
        QString headerTooltip = GD_STR_GLDebugOutputMessageCheckboxTooltip;
        ((headerTooltip += sourceHeaderStrings[s].asASCIICharArray()) += GD_STR_GLDebugOutputMessageCheckboxSeparator) += GD_STR_GLDebugOutputMessageCheckboxAllTypes;
        m_pSourceHeaderCheckBoxes[s]->setToolTip(headerTooltip);
        m_pSourceHeaderCheckBoxes[s]->setTristate(true);

        rc = connect(m_pSourceHeaderCheckBoxes[s], SIGNAL(stateChanged(int)), this, SLOT(onSourceHeaderClick(int)));
        GT_ASSERT(rc);

        pMessageKindsLayout->addWidget(m_pSourceHeaderLabels[s], s + 2, 0, 1, 1, Qt::AlignLeft | Qt::AlignTop);
        pMessageKindsLayout->addWidget(m_pSourceHeaderCheckBoxes[s], s + 2, 1, 1, 1, Qt::AlignRight | Qt::AlignTop);
    }

    for (int t = 0; t < AP_NUMBER_OF_USER_VISIBLE_DEBUG_OUTPUT_TYPES; t++)
    {
        // Add header checkboxes for types:
        gtString typeHeaderString;
        bool rcTyp = apGLDebugOutputTypeAsString((apGLDebugOutputType)t, typeHeaderString, true);
        GT_ASSERT(rcTyp);

        gtString typeHeaderFullString;
        rcTyp = apGLDebugOutputTypeAsString((apGLDebugOutputType)t, typeHeaderFullString);
        GT_ASSERT(rcTyp);

        m_pTypeHeaderLabels[t] = new QLabel(typeHeaderString.asASCIICharArray());
        m_pTypeHeaderCheckBoxes[t] = new QCheckBox;
        QString headerTooltip = GD_STR_GLDebugOutputMessageCheckboxTooltip;
        ((headerTooltip += GD_STR_GLDebugOutputMessageCheckboxAllSources) += GD_STR_GLDebugOutputMessageCheckboxSeparator) += typeHeaderFullString.asASCIICharArray();
        m_pTypeHeaderCheckBoxes[t]->setToolTip(headerTooltip);
        m_pTypeHeaderCheckBoxes[t]->setTristate(true);

        rc = connect(m_pTypeHeaderCheckBoxes[t], SIGNAL(stateChanged(int)), this, SLOT(onTypeHeaderClick(int)));
        GT_ASSERT(rc);

        pMessageKindsLayout->addWidget(m_pTypeHeaderLabels[t], 0, t + 2, 1, 1, Qt::AlignHCenter | Qt::AlignTop);
        pMessageKindsLayout->addWidget(m_pTypeHeaderCheckBoxes[t], 1, t + 2, 1, 1, Qt::AlignHCenter | Qt::AlignBottom);

        // Add checkboxes for each specific kind (source x type combination):
        for (int s = 0; s < AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES; s++)
        {
            int currentIdx = GD_DEBUG_OUTPUT_CHECKBOX_INDEX(s, t);
            m_pKindCheckBoxes[currentIdx] = new QCheckBox;
            QString tooltip = GD_STR_GLDebugOutputMessageCheckboxTooltip;
            ((tooltip += sourceHeaderStrings[s].asASCIICharArray()) += GD_STR_GLDebugOutputMessageCheckboxSeparator) += typeHeaderFullString.asASCIICharArray();
            m_pKindCheckBoxes[currentIdx]->setToolTip(tooltip);

            rc = connect(m_pKindCheckBoxes[currentIdx], SIGNAL(stateChanged(int)), this, SLOT(onKindCheckboxClick(int)));
            GT_ASSERT(rc);

            pMessageKindsLayout->addWidget(m_pKindCheckBoxes[currentIdx], s + 2, t + 2, 1, 1, Qt::AlignHCenter | Qt::AlignTop);
        }
    }

    // Allocate a text for the categories:
    m_pGLCategoriesText = new QGroupBox(GD_STR_GLDebugOutputEnableCategories);


    m_pGLCategoriesText->setLayout(pMessageKindsLayout);

    pMainLayout->addWidget(m_pGLCategoriesText, 1);
    pMainLayout->addSpacing(10);

    // Add the OK + Cancel buttons:
    gtString imagesDirPathAsString;
    bool rc1 = afGetApplicationImagesPath(imagesDirPathAsString);
    GT_IF_WITH_ASSERT(rc1)
    {
        // Calculate the CodeXL logo path:
        imagesDirPathAsString.append(osFilePath::osPathSeparator);
        imagesDirPathAsString.append(AF_STR_CodeXLDialogSmallLogoFileName);
    }

    QHBoxLayout* pBottomButtonsLayout = getBottomButtonLayout(true, imagesDirPathAsString);
    pMainLayout->addLayout(pBottomButtonsLayout);

    setLayout(pMainLayout);

}

// ---------------------------------------------------------------------------
// Name:        gdGLDebugOutputSettingsDialog::setDialogInitialValues
// Description: Set the initial values of the dialog
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        7/6/2010
// ---------------------------------------------------------------------------
bool gdGLDebugOutputSettingsDialog::setDialogInitialValues()
{
    bool retVal = false;

    // Get the debug output category mask:
    gtUInt64 kindMask = 0;
    bool rc1 = gaGetGLDebugOutputKindMask(kindMask);
    GT_ASSERT(rc1);

    // Get the break on status:
    bool breakOnGLDebugOutputMessages = false;
    bool doesBreakpointExist = false, isEnabled = false;
    bool rc2 = gaGetGenericBreakpointStatus(AP_BREAK_ON_DEBUG_OUTPUT, doesBreakpointExist, isEnabled);
    breakOnGLDebugOutputMessages = doesBreakpointExist && isEnabled;
    GT_ASSERT(rc2);

    // Get the debug output enable status:
    bool isGLDebugOutputLoggingEnabled = false;
    bool rc3 = gaGetGLDebugOutputLoggingEnabledStatus(isGLDebugOutputLoggingEnabled);
    GT_ASSERT(rc3);

    // Set the values:
    m_pGLDebugOutputLoggingEnableCheckbox->setChecked(isGLDebugOutputLoggingEnabled);
    m_pBreakOnGLDebugOutputReportsCheckbox->setChecked(breakOnGLDebugOutputMessages);

    // Get the current severity:
    for (int i = 0; i < AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES; i++)
    {
        bool isCurrentSeverityEnabled = false;
        bool rcSev = gaGetGLDebugOutputSeverityEnabled((apGLDebugOutputSeverity)i, isCurrentSeverityEnabled);
        GT_ASSERT(rcSev);

        m_pSeverityCheckBoxes[i]->setChecked(isCurrentSeverityEnabled);
    }

    // Set the categories statuses:
    bool atLeastOneInSourceTrue[AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES] = {0};
    bool atLeastOneInSourceFalse[AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES] = {0};
    bool atLeastOneInTypeTrue[AP_NUMBER_OF_DEBUG_OUTPUT_TYPES] = {0};
    bool atLeastOneInTypeFalse[AP_NUMBER_OF_DEBUG_OUTPUT_TYPES] = {0};

    // Trip the kind checkboxes:
    for (int s = 0; s < AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES; s++)
    {
        for (int t = 0; t < AP_NUMBER_OF_USER_VISIBLE_DEBUG_OUTPUT_TYPES; t++)
        {
            // Is the current kind flagged?
            gtUInt64 currentFlag = 1ULL << GD_DEBUG_OUTPUT_CHECKBOX_INDEX(s, t);
            bool shouldCheck = (currentFlag == (kindMask & currentFlag));

            // Check it and note that its source / type were hit or missed:
            m_pKindCheckBoxes[GD_DEBUG_OUTPUT_CHECKBOX_INDEX(s, t)]->setChecked(shouldCheck);

            if (shouldCheck)
            {
                atLeastOneInSourceTrue[s] = true;
                atLeastOneInTypeTrue[t] = true;
            }
            else
            {
                atLeastOneInSourceFalse[s] = true;
                atLeastOneInTypeFalse[t] = true;
            }
        }
    }

    // Trip the source / type header checkboxes:
    for (int s = 0; s < AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES; s++)
    {
        if (atLeastOneInSourceTrue[s])
        {
            if (atLeastOneInSourceFalse[s])
            {
                m_pSourceHeaderCheckBoxes[s]->setCheckState(Qt::PartiallyChecked);
            }
            else
            {
                m_pSourceHeaderCheckBoxes[s]->setCheckState(Qt::Checked);
            }
        }
        else
        {
            m_pSourceHeaderCheckBoxes[s]->setCheckState(Qt::Unchecked);

            // Sanity check:
            GT_ASSERT(atLeastOneInSourceFalse[s]);
        }
    }

    for (int t = 0; t < AP_NUMBER_OF_USER_VISIBLE_DEBUG_OUTPUT_TYPES; t++)
    {
        if (atLeastOneInTypeTrue[t])
        {
            if (atLeastOneInTypeFalse[t])
            {
                m_pTypeHeaderCheckBoxes[t]->setCheckState(Qt::PartiallyChecked);
            }
            else
            {
                m_pTypeHeaderCheckBoxes[t]->setCheckState(Qt::Checked);
            }
        }
        else
        {
            m_pTypeHeaderCheckBoxes[t]->setCheckState(Qt::Unchecked);

            // Sanity check:
            GT_ASSERT(atLeastOneInTypeFalse[t]);
        }
    }

    // Set the categories check boxes status:
    setGLDebugOutputDialogItemsStatus();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdGLDebugOutputSettingsDialog::onAccept
// Description: Handling dialog accept signal
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        12/8/2012
// ---------------------------------------------------------------------------
void gdGLDebugOutputSettingsDialog::onAccept()
{
    // Check if the user chose to break on debug output reports:
    bool breakOnDebugOutputMessages = (m_pBreakOnGLDebugOutputReportsCheckbox->isChecked());
    apGenericBreakpoint breakpoint(AP_BREAK_ON_DEBUG_OUTPUT);
    breakpoint.setEnableStatus(breakOnDebugOutputMessages);
    bool rc1 = gaSetBreakpoint(breakpoint);
    GT_ASSERT(rc1);

    // Check if the user chose to enable the Logging:
    bool isGLDebugOutputEnabled = m_pGLDebugOutputLoggingEnableCheckbox->isChecked();
    bool rc2 = gaEnableGLDebugOutputLogging(isGLDebugOutputEnabled);
    GT_ASSERT(rc2);

    gtUInt64 debugOutputKindMask = 0;

    for (int s = 0; s < AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES; s++)
    {
        for (int t = 0; t < AP_NUMBER_OF_USER_VISIBLE_DEBUG_OUTPUT_TYPES; t++)
        {
            // Is the current kind flagged?
            bool shouldTrip = m_pKindCheckBoxes[GD_DEBUG_OUTPUT_CHECKBOX_INDEX(s, t)]->isChecked();

            if (shouldTrip)
            {
                gtUInt64 currentFlag = 1ULL << GD_DEBUG_OUTPUT_CHECKBOX_INDEX(s, t);
                debugOutputKindMask |= currentFlag;
            }
        }
    }

    // Set the category into the infra:
    bool rc3 = gaSetGLDebugOutputKindMask(debugOutputKindMask);
    GT_ASSERT(rc3);

    for (int i = 0; i < AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES; i++)
    {
        // Set the severity into the infra:
        bool isSeverityEnabled = m_pSeverityCheckBoxes[i]->isChecked();
        bool rc4 = gaSetGLDebugOutputSeverityEnabled((apGLDebugOutputSeverity)i, isSeverityEnabled);
        GT_ASSERT(rc4);
    }
}


// ---------------------------------------------------------------------------
// Name:        void gdGLDebugOutputSettingsDialog::onGLDebugOutputLoggingEnableClick
// Description: Called when clicking the Enable / Disable GLDebugOutputLogging.
// Author:      Sigal Algranaty
// Date:        7/6/2010
// ---------------------------------------------------------------------------
void gdGLDebugOutputSettingsDialog::onGLDebugOutputLoggingEnableClick(int state)
{
    GT_UNREFERENCED_PARAMETER(state);

    setGLDebugOutputDialogItemsStatus();
}

// ---------------------------------------------------------------------------
// Name:        gdGLDebugOutputSettingsDialog::onSourceHeaderClick
// Description: Called when clicking a source header checkbox
// Author:      Uri Shomroni
// Date:        29/6/2014
// ---------------------------------------------------------------------------
void gdGLDebugOutputSettingsDialog::onSourceHeaderClick(int state)
{
    if (!m_isUpdatingCheckboxes)
    {
        QCheckBox* pSender = qobject_cast<QCheckBox*>(sender());

        if (NULL != pSender)
        {
            m_isUpdatingCheckboxes = true;

            // "Skip" the partially checked value:
            if (state == Qt::PartiallyChecked)
            {
                pSender->setCheckState(Qt::Checked);
            }

            // Locate the correct source:
            int foundSource = -1;

            for (int s = 0; s < AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES; s++)
            {
                if (m_pSourceHeaderCheckBoxes[s] == pSender)
                {
                    foundSource = s;
                    break;
                }
            }

            // Sanity check:
            GT_IF_WITH_ASSERT((-1 < foundSource) && (AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES > foundSource))
            {
                // Apply to all children:
                bool shouldCheck = state != Qt::Unchecked;

                for (int t = 0; t < AP_NUMBER_OF_USER_VISIBLE_DEBUG_OUTPUT_TYPES; t++)
                {
                    // Check the checkbox and update its header:
                    m_pKindCheckBoxes[GD_DEBUG_OUTPUT_CHECKBOX_INDEX(foundSource, t)]->setChecked(shouldCheck);
                    updateTypeHeaderCheckbox(t);
                }
            }

            m_isUpdatingCheckboxes = false;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdGLDebugOutputSettingsDialog::onTypeHeaderClick
// Description: Called when clicking a type header checkbox
// Author:      Uri Shomroni
// Date:        29/6/2014
// ---------------------------------------------------------------------------
void gdGLDebugOutputSettingsDialog::onTypeHeaderClick(int state)
{
    if (!m_isUpdatingCheckboxes)
    {
        QCheckBox* pSender = qobject_cast<QCheckBox*>(sender());

        if (NULL != pSender)
        {
            m_isUpdatingCheckboxes = true;

            // "Skip" the partially checked value:
            if (state == Qt::PartiallyChecked)
            {
                pSender->setCheckState(Qt::Checked);
            }

            // Locate the correct type:
            int foundType = -1;

            for (int t = 0; t < AP_NUMBER_OF_USER_VISIBLE_DEBUG_OUTPUT_TYPES; t++)
            {
                if (m_pTypeHeaderCheckBoxes[t] == pSender)
                {
                    foundType = t;
                    break;
                }
            }

            // Sanity check:
            GT_IF_WITH_ASSERT((-1 < foundType) && (AP_NUMBER_OF_USER_VISIBLE_DEBUG_OUTPUT_TYPES > foundType))
            {
                // Apply to all children:
                bool shouldCheck = state != Qt::Unchecked;

                for (int s = 0; s < AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES; s++)
                {
                    // Check the checkbox and update its header:
                    m_pKindCheckBoxes[GD_DEBUG_OUTPUT_CHECKBOX_INDEX(s, foundType)]->setChecked(shouldCheck);
                    updateSourceHeaderCheckbox(s);
                }
            }

            m_isUpdatingCheckboxes = false;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdGLDebugOutputSettingsDialog::onKindCheckboxClick
// Description: Called when clicking a kind checkbox
// Author:      Uri Shomroni
// Date:        29/6/2014
// ---------------------------------------------------------------------------
void gdGLDebugOutputSettingsDialog::onKindCheckboxClick(int state)
{
    GT_UNREFERENCED_PARAMETER(state);

    if (!m_isUpdatingCheckboxes)
    {
        QCheckBox* pSender = qobject_cast<QCheckBox*>(sender());

        if (NULL != pSender)
        {
            m_isUpdatingCheckboxes = true;

            // Find the correct checkbox:
            int foundSource = -1;
            int foundType = -1;
            bool goOn = true;

            for (int s = 0; s < AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES && goOn; s++)
            {
                for (int t = 0; t < AP_NUMBER_OF_USER_VISIBLE_DEBUG_OUTPUT_TYPES && goOn; t++)
                {
                    if (pSender == m_pKindCheckBoxes[GD_DEBUG_OUTPUT_CHECKBOX_INDEX(s, t)])
                    {
                        foundSource = s;
                        foundType = t;
                        goOn = false;
                    }
                }
            }

            GT_IF_WITH_ASSERT((-1 < foundSource) && (AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES > foundSource))
            {
                updateSourceHeaderCheckbox(foundSource);
            }


            GT_IF_WITH_ASSERT((-1 < foundType) && (AP_NUMBER_OF_USER_VISIBLE_DEBUG_OUTPUT_TYPES > foundType))
            {
                updateTypeHeaderCheckbox(foundType);
            }

            m_isUpdatingCheckboxes = false;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        void gdGLDebugOutputSettingsDialog::setGLDebugOutputDialogItemsStatus
// Description: Set the GLDebugOutput commands Dialog Enable / Disable status.
// Author:      Sigal Algranaty
// Date:        7/6/2010
// ---------------------------------------------------------------------------
void gdGLDebugOutputSettingsDialog::setGLDebugOutputDialogItemsStatus()
{
    // Get the checked status:
    bool isChecked = m_pGLDebugOutputLoggingEnableCheckbox->isChecked();

    m_pBreakOnGLDebugOutputReportsCheckbox->setEnabled(isChecked);

    m_pSeverityText->setEnabled(isChecked);
    m_pGLCategoriesText->setEnabled(isChecked);

    for (int i = 0; i < AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES; i++)
    {
        m_pSeverityCheckBoxes[i]->setEnabled(isChecked);
    }

    for (int i = 0; i < AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES; i++)
    {
        m_pSourceHeaderLabels[i]->setEnabled(isChecked);
        m_pSourceHeaderCheckBoxes[i]->setEnabled(isChecked);
    }

    for (int i = 0; i < AP_NUMBER_OF_USER_VISIBLE_DEBUG_OUTPUT_TYPES; i++)
    {
        m_pTypeHeaderLabels[i]->setEnabled(isChecked);
        m_pTypeHeaderCheckBoxes[i]->setEnabled(isChecked);
    }

    for (int i = 0; i < AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES * AP_NUMBER_OF_USER_VISIBLE_DEBUG_OUTPUT_TYPES; i++)
    {
        m_pKindCheckBoxes[i]->setEnabled(isChecked);
    }

}

// ---------------------------------------------------------------------------
// Name:        gdGLDebugOutputSettingsDialog::onKindCheckboxClick
// Description: Updates a source header checkbox from its children
// Author:      Uri Shomroni
// Date:        29/6/2014
// ---------------------------------------------------------------------------
void gdGLDebugOutputSettingsDialog::updateSourceHeaderCheckbox(int s)
{
    // Sanity:
    GT_IF_WITH_ASSERT((-1 < s) && (AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES > s))
    {
        // This is only expected to be called during the update functions:
        GT_ASSERT(m_isUpdatingCheckboxes);

        bool atLeastOneTrue = false;
        bool atLeastOneFalse = false;

        for (int t = 0; t < AP_NUMBER_OF_USER_VISIBLE_DEBUG_OUTPUT_TYPES && (!(atLeastOneTrue && atLeastOneFalse)); t++)
        {
            bool isChecked = m_pKindCheckBoxes[GD_DEBUG_OUTPUT_CHECKBOX_INDEX(s, t)]->isChecked();

            if (isChecked)
            {
                atLeastOneTrue = true;
            }
            else
            {
                atLeastOneFalse = true;
            }
        }

        if (atLeastOneTrue)
        {
            if (atLeastOneFalse)
            {
                m_pSourceHeaderCheckBoxes[s]->setCheckState(Qt::PartiallyChecked);
            }
            else
            {
                m_pSourceHeaderCheckBoxes[s]->setCheckState(Qt::Checked);
            }
        }
        else
        {
            m_pSourceHeaderCheckBoxes[s]->setCheckState(Qt::Unchecked);

            // Sanity check:
            GT_ASSERT(atLeastOneFalse);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdGLDebugOutputSettingsDialog::onKindCheckboxClick
// Description: Updates a type header checkbox from its children
// Author:      Uri Shomroni
// Date:        29/6/2014
// ---------------------------------------------------------------------------
void gdGLDebugOutputSettingsDialog::updateTypeHeaderCheckbox(int t)
{
    // Sanity:
    GT_IF_WITH_ASSERT((-1 < t) && (AP_NUMBER_OF_USER_VISIBLE_DEBUG_OUTPUT_TYPES > t))
    {
        // This is only expected to be called during the update functions:
        GT_ASSERT(m_isUpdatingCheckboxes);

        bool atLeastOneTrue = false;
        bool atLeastOneFalse = false;

        for (int s = 0; s < AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES && (!(atLeastOneTrue && atLeastOneFalse)); s++)
        {
            bool isChecked = m_pKindCheckBoxes[GD_DEBUG_OUTPUT_CHECKBOX_INDEX(s, t)]->isChecked();

            if (isChecked)
            {
                atLeastOneTrue = true;
            }
            else
            {
                atLeastOneFalse = true;
            }
        }

        if (atLeastOneTrue)
        {
            if (atLeastOneFalse)
            {
                m_pTypeHeaderCheckBoxes[t]->setCheckState(Qt::PartiallyChecked);
            }
            else
            {
                m_pTypeHeaderCheckBoxes[t]->setCheckState(Qt::Checked);
            }
        }
        else
        {
            m_pTypeHeaderCheckBoxes[t]->setCheckState(Qt::Unchecked);

            // Sanity check:
            GT_ASSERT(atLeastOneFalse);
        }
    }
}
