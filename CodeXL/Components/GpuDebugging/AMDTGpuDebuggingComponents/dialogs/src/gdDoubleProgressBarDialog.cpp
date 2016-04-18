//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdDoubleProgressBarDialog.cpp
///
//==================================================================================

//------------------------------ gdDoubleProgressBarDialog.cpp ------------------------------

// Qt:
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/dialogs/gdDoubleProgressBarDialog.h>


// ---------------------------------------------------------------------------
// Name:        gdDoubleProgressBarDialog::gdDoubleProgressBarDialog
// Description: Constructor
// Arguments:   pParent - Dialog parent (can be NULL)
//              title - Double progress dialog caption title.
//              subTitle - Double progress dialog inner title.
// Author:      Eran Zinman
// Date:        5/1/2008
// ---------------------------------------------------------------------------
gdDoubleProgressBarDialog::gdDoubleProgressBarDialog(QWidget* pParent, const gtString& title, const gtString& subTitle)
    : acDialog(pParent,  true, true, QDialogButtonBox::Ok),
      m_pLogTextCtrl(NULL), m_pTotalProgress(NULL), m_pItemProgress(NULL), m_pItemText(NULL), m_dialogSubTitle(subTitle), m_pDialogButtonBox(NULL)
{
    // Set window flags (minimize / maximize / close buttons):
    Qt::WindowFlags flags = windowFlags();
    flags &= ~Qt::WindowContextHelpButtonHint;
    setWindowFlags(flags);
    setWindowTitle(acGTStringToQString(title));

    // Set dialog layout
    setDialogLayout();

}

// ---------------------------------------------------------------------------
// Name:        gdDoubleProgressBarDialog::updateText
// Description: Updates the text of the current item
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        7/1/2008
// ---------------------------------------------------------------------------
bool gdDoubleProgressBarDialog::updateText(const gtString& newText)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pItemText != NULL)
    {
        m_pItemText->setText(acGTStringToQString(newText));

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDoubleProgressBarDialog::setDialogLayout
// Description: Sets dialog layout
// Author:      Eran Zinman
// Date:        5/1/2008
// ---------------------------------------------------------------------------
void gdDoubleProgressBarDialog::setDialogLayout()
{
    // Create main sizer (vertical sizer)
    QVBoxLayout* pMainLayout = new QVBoxLayout;


    QLabel* pLabel = new QLabel(acGTStringToQString(m_dialogSubTitle));


    pMainLayout->addWidget(pLabel);

    // Create image format sizer which contain the output directory and "overwrite" check box
    QGroupBox* pProgressGroupBox = new QGroupBox;


    // Create total and current item progress bars
    m_pTotalProgress =  new QProgressBar;

    m_pItemProgress = new QProgressBar;


    QVBoxLayout* pGroupBoxLayout = new QVBoxLayout;


    // Create current item text control
    m_pItemText = new QLabel(GD_STR_DoubleProgressBarDialogCurrentItemProgress);


    // Create total progress text control
    QLabel* pTotalProgressLabel = new QLabel(GD_STR_DoubleProgressBarDialogTotalProgress);


    pGroupBoxLayout->addWidget(m_pItemText);
    pGroupBoxLayout->addWidget(m_pItemProgress);
    pGroupBoxLayout->addWidget(pTotalProgressLabel);
    pGroupBoxLayout->addWidget(m_pTotalProgress);

    pProgressGroupBox->setLayout(pGroupBoxLayout);

    // Add progress sizer to main sizer
    pMainLayout->addWidget(pProgressGroupBox);

    QGroupBox* pOutputLogGroupBox = new QGroupBox(GD_STR_DoubleProgressBarDialogOutputLog);


    // Create a text control for the output log
    m_pLogTextCtrl = new QTextEdit(pOutputLogGroupBox);

    m_pLogTextCtrl->setMinimumSize(QSize(400, 200));


    // Add output log sizer to main sizer
    pMainLayout->addWidget(pOutputLogGroupBox, 1);

    // Create the dialog buttons:
    m_pDialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok);


    // Connect the Ok button:
    bool rc = connect(m_pDialogButtonBox, SIGNAL(accepted()), this, SLOT(accept()));
    GT_ASSERT(rc);

    pMainLayout->addWidget(m_pDialogButtonBox);
    pMainLayout->setSizeConstraint(QLayout::SetFixedSize);
    setMinimumSize(QSize(400, 400));

    // Activate main sizer
    setLayout(pMainLayout);
}

// ---------------------------------------------------------------------------
// Name:        gdDoubleProgressBarDialog::enableOkButton
// Description: Enables the "OK" button (allows the user to close the dialog
//              by clicking on "ok"
// Author:      Eran Zinman
// Date:        7/1/2008
// ---------------------------------------------------------------------------
void gdDoubleProgressBarDialog::enableOkButton()
{
    // Sanity check
    GT_IF_WITH_ASSERT(m_pDialogButtonBox != NULL)
    {
        // Enable "ok" button
        m_pDialogButtonBox->setEnabled(true);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdDoubleProgressBarDialog::finishOperation
// Description: Notify the double progress bar dialog
//              that we are done, so it will start the finishing process.
//
//              The following actions are done to indicate progress finished:
//              1. Set total and current item percentage to be 100%.
//              2. Enable "OK" button so user will be able to close dialog.
//              3. Set the focus to the double progress dialog.
//              4. Make the dialog "modal"
//
// Author:      Eran Zinman
// Date:        7/1/2008
// ---------------------------------------------------------------------------
void gdDoubleProgressBarDialog::finishOperation()
{
    // Update current item and total progress to 100%;
    updateTotalProgress(100);
    updateItemProgress(100);

    // Make sure I'm on top and focused
    raise();
    setFocus();

    // Enable the "ok" button
    enableOkButton();

    // Make the dialog "modal" so the user will have to click on "ok" in order to close it.
    // Important Notice: No code will be executed before the user will click on "ok" so we
    // can safely delete this dialog when this function returns.
    // Hide the dialog so that the showModal of the VS will work correctly:
    hide();
    afApplicationCommands* pApplicationCommandInstance = afApplicationCommands::instance();

    if (pApplicationCommandInstance != NULL)
    {
        pApplicationCommandInstance->showModal(this);
    }
    else
    {
        // Application commands object was not created yet:
        this->setModal(true);
        this->exec();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdDoubleProgressBarDialog::updateGaugeValue
// Description: Update a progress bar
// Arguments:   pGauge - Gauge item to update
//              newValue - The new progress value of the gauge,
//              must be between [0..100%]
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        7/1/2008
// ---------------------------------------------------------------------------
bool gdDoubleProgressBarDialog::updateGaugeValue(QProgressBar* pGauge, int newValue)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(pGauge != NULL)
    {
        // Range check:
        GT_IF_WITH_ASSERT((newValue >= 0) && (newValue <= 100))
        {
            // Change the gauge value to the new value
            pGauge->setValue(newValue);
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdDoubleProgressBarDialog::addEntryToLog
// Description: Add an entry to the log text control
// Arguments:   logEntry - The new log entry
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        7/1/2008
// ---------------------------------------------------------------------------
bool gdDoubleProgressBarDialog::addEntryToLog(const gtASCIIString& logEntry)
{
    bool retVal = false;

    // Sanity check
    GT_IF_WITH_ASSERT(m_pLogTextCtrl != NULL)
    {
        // Append the text to the log
        m_pLogTextCtrl->append(logEntry.asCharArray());

        retVal = true;
    }

    return retVal;
}
