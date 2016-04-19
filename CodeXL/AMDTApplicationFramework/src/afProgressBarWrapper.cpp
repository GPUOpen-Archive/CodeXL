//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afProgressBarWrapper.cpp
///
//==================================================================================

// Qt
#include <QtWidgets>
#include <QtConcurrent/QtConcurrent>
#include <QtConcurrent/qtconcurrentmap.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acProgressDlg.h>

// Local:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTreeItemData.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afSoftwareUpdaterWindow.h>


// Static members initializations:
afProgressBarWrapper* afProgressBarWrapper::m_pMySingleInstance = nullptr;
#define AF_PROGRESSBAR_MAX_WIDTH 200
#define AF_PROGRESSBAR_MIN_RANGE_FOR_DIALOG 10000

// for debugging of progress dialog cancel event
//#include <AMDTApplicationFramework/Include/afMessageBox.h>
//
//void OnProgressDialogCancelled()
//{
//    bool canceled = true;
//    if (canceled)
//    {
//        afMessageBox::instance().critical("Show Progress Dialog", "afProgressBarWrapper::OnProgressDialogCancelled");
//    }
//}


// ---------------------------------------------------------------------------
// Name:        afProgressBarWrapper::afProgressBarWrapper
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        10/8/2011
// ---------------------------------------------------------------------------
afProgressBarWrapper::afProgressBarWrapper() :
    m_isProgressDlgShouldBeVisible(false), m_isShuttingDown(false), m_progressCallsCounter(0),
    m_pStatusBar(nullptr), m_pProgressBar(nullptr), m_pProgressDlg(nullptr), m_dlgThresholdMsec(0), m_dlgStartTimeMsec(0)
{

}


// ---------------------------------------------------------------------------
// Name:        afProgressBarWrapper::initialize
// Description: Initialize me
// Arguments:   QStatusBar* pStatusBar
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        10/8/2011
// ---------------------------------------------------------------------------
void afProgressBarWrapper::initialize(QStatusBar* pStatusBar)
{
    m_pStatusBar = pStatusBar;

    // Sanity check:
    if (m_pStatusBar != nullptr)
    {
        // Create the progress bar:
        m_pProgressBar = new QProgressBar(m_pStatusBar);

        m_pProgressBar->setMaximumWidth(AF_PROGRESSBAR_MAX_WIDTH);

        // Add the progress bar to the status bar: (permanent is on the right side)
        m_pProgressBar->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding));
        m_pStatusBar->addPermanentWidget(m_pProgressBar, 1);

        // Hide the progress bar by default:
        m_pProgressBar->hide();

        // Set the default minimum + maximum values:
        m_pProgressBar->setMaximum(100);
        m_pProgressBar->setMinimum(0);
    }


    m_dlgThresholdMsec = 0;
    m_dlgStartTimeMsec = 0;

    setProgressText(AF_STR_Ready);

    m_isProgressDlgShouldBeVisible = false;

    // init m_progressCallsCounter
    m_progressCallsCounter = 0;

    m_useDlgLastPos = false;
}


// ---------------------------------------------------------------------------
// Name:        afProgressBarWrapper::registerInstance
// Description: Register my single instance
// Arguments:   gdProgressBarWrapper* pApplicationCommandsInstance
// Author:      Sigal Algranaty
// Date:        7/2/2011
// ---------------------------------------------------------------------------
bool afProgressBarWrapper::registerInstance(afProgressBarWrapper* pProgressBarInstance)
{
    bool retVal = false;

    // if an instance was created before delete it and register the new version
    // this can happen in VS during activation of the application for example then afProgressBarWrapper is created when
    // we wanted to register the VS later
    if (m_pMySingleInstance != nullptr)
    {
        delete m_pMySingleInstance;
        m_pMySingleInstance = nullptr;
    }

    // Do not allow multiple registration for my instance:
    GT_IF_WITH_ASSERT(m_pMySingleInstance == nullptr)
    {
        m_pMySingleInstance = pProgressBarInstance;
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afProgressBarWrapper::cleanupInstance
// Description: Cleans up the instance. Should ONLY be called once to destroy the instance
//              in parallel to the registerInstance function.
// Author:      Uri Shomroni
// Date:        12/9/2012
// ---------------------------------------------------------------------------
void afProgressBarWrapper::cleanupInstance()
{
    if (nullptr != m_pMySingleInstance)
    {
        static bool onlyOnce = true;
        GT_ASSERT(onlyOnce);
        onlyOnce = false;

        delete m_pMySingleInstance;
        m_pMySingleInstance = nullptr;
    }
}

// ---------------------------------------------------------------------------
// Name:        afProgressBarWrapper::~afProgressBarWrapper
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        10/8/2011
// ---------------------------------------------------------------------------
afProgressBarWrapper::~afProgressBarWrapper()
{
    if (m_pProgressDlg)
    {
        delete m_pProgressDlg;
        m_pProgressDlg = nullptr;
    }
}


// ---------------------------------------------------------------------------
// Name:        afProgressBarWrapper::instance
// Description:
// Return Val:  afProgressBarWrapper&
// Author:      Sigal Algranaty
// Date:        10/8/2011
// ---------------------------------------------------------------------------
afProgressBarWrapper& afProgressBarWrapper::instance()
{
    if (m_pMySingleInstance == nullptr)
    {
        m_pMySingleInstance = new afProgressBarWrapper;
        GT_ASSERT(m_pMySingleInstance);
    }

    return *m_pMySingleInstance;
}

// ---------------------------------------------------------------------------
// Name:        hideProgressBar
// Description: Hide progress bar
// Author:      Sigal Algranaty
// Date:        10/8/2011
// ---------------------------------------------------------------------------
void afProgressBarWrapper::hideProgressBar()
{
    if (IsDlgShown() == true && false == m_isShuttingDown)
    {
        setProgressText(AF_STR_Ready);

        if (m_pProgressBar != nullptr)
        {
            m_pProgressBar->hide();
        }

        if (m_pProgressDlg != nullptr)
        {
            m_dlgLastPos = m_pProgressDlg->geometry();

            if (m_dlgLastPos.x() < 0)
            {
                m_dlgLastPos.setX(0);
            }

            if (m_dlgLastPos.y() < 0)
            {
                m_dlgLastPos.setY(0);
            }

            m_useDlgLastPos = true;

            delete m_pProgressDlg;
            m_pProgressDlg = nullptr;
        }

        m_isProgressDlgShouldBeVisible = false;


        if (afMainAppWindow::instance() != nullptr)
        {
            afMainAppWindow::instance()->updateGeometry();
            afMainAppWindow::instance()->setFocus();
            afMainAppWindow::instance()->raise();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        shouldUpdateProgress
// Description: should progress bar be updated
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        10/8/2011
// ---------------------------------------------------------------------------
bool afProgressBarWrapper::shouldUpdateProgress() const
{
    bool retVal = (m_pStatusBar != nullptr);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        updateProgressBar
// Description: update progress bar with new value
// Arguments:   int newValue
// Author:      Sigal Algranaty
// Date:        10/8/2011
// ---------------------------------------------------------------------------
void afProgressBarWrapper::updateProgressBar(int newValue)
{
    // Sanity check:
    if (m_pProgressBar != nullptr)
    {
        //Only show the progress bar if we need to
        if (m_pProgressBar->isHidden())
        {
            m_pProgressBar->show();
        }

        m_pProgressBar->setValue(newValue);
    }

    if (m_pProgressDlg != nullptr)
    {
        // Only show the progress bar if we need to
        if (ShowDialogIfNeeded())
        {
            m_pProgressDlg->SetValue(newValue);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        incrementProgressBar
// Description: progress bar default increment value
// Arguments:   int amount /*= 1*/
// Author:      Sigal Algranaty
// Date:        10/8/2011
// ---------------------------------------------------------------------------
void afProgressBarWrapper::incrementProgressBar(int amount)
{
    if (amount != 0)
    {
        int value = 0;

        if (m_pProgressBar != nullptr)
        {
            value = m_pProgressBar->value();
        }
        else if (m_pProgressDlg != nullptr)
        {
            value = m_pProgressDlg->Value();
        }

        // Sanity check:
        if (m_pProgressBar != nullptr)
        {
            //Only show the progress bar if we need to
            if (m_pProgressBar->isHidden())
            {
                m_pProgressBar->show();
            }

            m_pProgressBar->setValue(value + amount);

        }

        if (m_pProgressDlg != nullptr)
        {
            // Only show the progress bar if we need to
            if (ShowDialogIfNeeded())
            {
                m_pProgressDlg->SetValue(value + amount);
            }
        }
    }
}




// ---------------------------------------------------------------------------
// Name:        setProgressDetails
// Description: Set progress bar information
// Arguments:   const gtString& newString
//              int newRange
// Author:      Sigal Algranaty
// Date:        10/8/2011
// ---------------------------------------------------------------------------
void afProgressBarWrapper::setProgressDetails(const gtString& newString, int newRange)
{
    // Sanity check:
    if (m_pProgressBar != nullptr)
    {
        m_pProgressBar->setRange(0, newRange);

        // Reset the value:
        m_pProgressBar->setValue(0);

    }

    if (m_pProgressDlg != nullptr)
    {
        m_pProgressDlg->SetRange(0, newRange);

        // Reset the value:
        m_pProgressDlg->SetValue(0);
    }

    // Set the text:
    setProgressText(newString);
}

// ---------------------------------------------------------------------------
// Name:        setProgressRange
// Description:
// Arguments:   int newRange
// Author:      Sigal Algranaty
// Date:        10/8/2011
// ---------------------------------------------------------------------------
void afProgressBarWrapper::setProgressRange(int newRange)
{
    // Sanity check:
    if (m_pProgressBar != nullptr)
    {
        m_pProgressBar->setRange(0, newRange);
    }

    if (m_pProgressDlg != nullptr)
    {
        m_pProgressDlg->SetRange(0, newRange);
    }
}

// ---------------------------------------------------------------------------
// Name:        progressRange
// Description: Get progress bar range
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        10/8/2011
// ---------------------------------------------------------------------------
int afProgressBarWrapper::progressRange() const
{
    int retVal = 0;

    // Sanity check:
    if (m_pProgressBar != nullptr)
    {
        retVal = m_pProgressBar->maximum();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        setProgressText
// Description: Set progress bar displayed text
// Arguments:   const gtString& newString
// Author:      Sigal Algranaty
// Date:        10/8/2011
// ---------------------------------------------------------------------------
void afProgressBarWrapper::setProgressText(const gtString& newString)
{
    QString newQStr = acGTStringToQString(newString);

    // Sanity check:
    if (m_pProgressBar != nullptr)
    {
        m_pStatusBar->showMessage(newQStr);
    }

    if (m_pProgressDlg != nullptr)
    {
        if (m_isProgressDlgShouldBeVisible)
        {
            QString currentText;
            m_pProgressDlg->GetLabelText(currentText);

            if (newQStr != currentText)
            {
                m_pProgressDlg->SetLabelText(newQStr);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        ShowProgressDialog
// Description: Calls to display a progress dialog
// Arguments:   const gtString& msg - the message to display in the dialog
// Arguments:   unsigned int maxItemCount - the number of items which will be performed
// Author:      Naama Zur
// Date:        21/9/2015
// ---------------------------------------------------------------------------
void afProgressBarWrapper::ShowProgressDialog(const gtString& msg, int maxItemCount, int dlgThresholdMsec, bool showCancelButton, void(*callbackfunc))
{
    if (!m_isShuttingDown)
    {
        if (m_pProgressDlg == nullptr)
        {
            m_pProgressDlg = new acProgressDlg(afMainAppWindow::instance());
        }

        GT_IF_WITH_ASSERT(m_pProgressDlg != nullptr)
        {
            m_isProgressDlgShouldBeVisible = true;
            m_pProgressDlg->SetLabelText(acGTStringToQString(msg));
            // for debug of callbackfunc use OnProgressDialogCancelled
            m_pProgressDlg->ShowCancelButton(showCancelButton, (funcPtr)callbackfunc);

            setProgressDetails(msg, maxItemCount);

            m_dlgThresholdMsec = dlgThresholdMsec;

            if (m_dlgThresholdMsec > 0)
            {
                m_refreshTimer.start();
                m_dlgStartTimeMsec = 0;
            }

            if (m_useDlgLastPos)
            {
                m_pProgressDlg->setGeometry(m_dlgLastPos);
            }

            ShowDialogIfNeeded();

        }
    }
}

void afProgressBarWrapper::ShowProgressBar(const gtString& progressMessage, int maxItemCount)
{
    if (m_pStatusBar != nullptr)
    {
        m_pStatusBar->showMessage(acGTStringToQString(progressMessage));

        if (maxItemCount == 0)
        {
            m_pProgressBar->setMinimum(0);
            m_pProgressBar->setMaximum(0);
            m_pProgressBar->setValue(0);
        }
        else
        {
            m_pProgressBar->setMinimum(0);
            m_pProgressBar->setMaximum(maxItemCount);
            m_pProgressBar->setValue(0);
        }

        m_pProgressBar->show();
    }

    if (m_pProgressDlg != nullptr)
    {
        if (ShowDialogIfNeeded())
        {
            m_pProgressDlg->SetLabelText(acGTStringToQString(progressMessage));
            m_pProgressDlg->SetRange(0, maxItemCount);
        }
    }

}

void afProgressBarWrapper::ShowProgressBar(afTreeItemType itemType, const gtString& actionStr, int amountOfItems)
{
    gtString itemName;
    bool rc = afApplicationTreeItemData::itemTypeAsString(itemType, itemName);
    gtString progressMessage;
    progressMessage.appendFormattedString(AF_STR_ProgressMessageFormat, actionStr.asCharArray(), L" ", itemName.asCharArray());

    if ((rc) && (m_pProgressDlg != nullptr))
    {
        // Build the progress string (i.e: "Loading Textures..."):

        setProgressText(progressMessage);

        setProgressRange(amountOfItems);

        if (m_pProgressBar != nullptr)
        {
            m_pProgressBar->show();
        }

        m_pProgressDlg->SetLabelText(acGTStringToQString(progressMessage));
        m_pProgressDlg->SetRange(0, amountOfItems);

        ShowDialogIfNeeded();
    }
}


void afProgressBarWrapper::ShutDown()
{

    hideProgressBar();
    m_isShuttingDown = true;
}

bool afProgressBarWrapper::ShowDialogIfNeeded()
{
    bool retVal = false;

    // check if user defined a threshold for display, and if so: if this threshold has passed
    bool hasThresholdPeriodPassed = true;

    if (m_dlgThresholdMsec > 0)
    {
        int msecs = m_refreshTimer.elapsed();
        {
            if (msecs - m_dlgStartTimeMsec < m_dlgThresholdMsec)
            {
                hasThresholdPeriodPassed = false;
            }
        }
    }


    if (m_isProgressDlgShouldBeVisible && hasThresholdPeriodPassed && !m_isShuttingDown)
    {
        afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
        GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
        {
            m_pProgressDlg->show();

            retVal = true;
        }
    }

    return retVal;
}

void afProgressBarWrapper::BringToFront()
{
    if ((m_pProgressDlg != nullptr) && !m_pProgressDlg->isHidden() && m_isProgressDlgShouldBeVisible)
    {
        m_pProgressDlg->raise();
    }
}

void afProgressBarWrapper::SetProgressDialogCaption(const gtString& caption)
{
    if (m_pProgressDlg != nullptr)
    {
        m_pProgressDlg->SetHeader(acGTStringToQString(caption));
    }
}
