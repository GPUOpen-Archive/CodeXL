//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afProgressBarWrapper.h
///
//==================================================================================

#ifndef __AFPROGRESSBARWRAPPER_H
#define __AFPROGRESSBARWRAPPER_H

#include <QtWidgets>
#include <QTime>

// Forward declaration:
QT_BEGIN_NAMESPACE
class QStatusBar;
class QProgressBar;
QT_END_NAMESPACE

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>
#include <AMDTApplicationFramework/Include/afTreeItemType.h>

class acProgressDlg;


// ----------------------------------------------------------------------------------
// Class Name:           afProgressBarWrapper : public gdProgressBar
// General Description:  implementation of the gdProgressBar for the CodeXL QT
//                       stand alone application
// Author:               Sigal Algranaty
// Creation Date:        10/8/2011
// ----------------------------------------------------------------------------------
class AF_API afProgressBarWrapper
{
public:

    virtual ~afProgressBarWrapper();

    // specific stand alone implementation
    virtual void hideProgressBar();

    void initialize(QStatusBar* pStatusBar);

    /// Show the progress bar and display the requested message in the status bar:
    /// \param progressMessage the message to display in the status bar
    /// \param maxItemCount the item count.  When maxItemCount is reached the dialog will display a completion indication of 100%, when it is 0, the progress will show infinitely
    virtual void ShowProgressBar(const gtString& progressMessage, int maxItemCount);

    /// Show a progress bar for an action taken for an item of type itemType, with the action specified in actionStr:
    /// param itemType the item type for the progress
    /// param actionStr the string of the action taken for this items
    /// param amountOfItems the amount of items which are about to be processed
    virtual void ShowProgressBar(afTreeItemType itemType, const gtString& actionStr, int amountOfItems);

    /// Shut down the progress bar
    virtual void ShutDown();

    virtual bool shouldUpdateProgress() const;

    virtual void updateProgressBar(int newValue);
    virtual void incrementProgressBar(int amount = 1);

    virtual void setProgressDetails(const gtString& newString, int newRange);
    virtual void setProgressRange(int newRange);

    virtual int progressRange() const ;

    virtual void setProgressText(const gtString& newString);
    void SetProgressDialogCaption(const gtString& caption);

    /// Show a progress dialog along with the progress bar
    /// param msg the string to display in the progress dialog
    /// param maxItemCount the number of items which are going to take place. If maxItemCount == -1 (default), range maximum does not change
    /// param dlgThreshold the threshold for showing the progress dialog
    virtual void ShowProgressDialog(const gtString& msg, int maxItemCount = 0, int dlgThresholdMsec = 0, bool showCancelButton = false, void(*callbackfunc) = nullptr);

    static afProgressBarWrapper& instance();

    // Register my instance (this function is private, to make sure that only approved
    // classes access this function:
    static bool registerInstance(afProgressBarWrapper* pProgressBarInstance);


    /// Handle cancel event in progress dialog
    bool IsDlgShown()const { return m_isProgressDlgShouldBeVisible; }

    /// Brings the progress dialog to front
    void BringToFront();

protected:

    // Do not allow the use of my default constructor;
    afProgressBarWrapper();

public:
    // This should only be used at module destruction parallel to where the instance was created:
    static void cleanupInstance();

private:
    // Determines whether Progress Dialog is displayed along with the progress bar
    bool m_isProgressDlgShouldBeVisible;

    /// True iff we're closing the wrapper
    bool m_isShuttingDown;

    // Counts calls to update the progress for GUI update
    unsigned int m_progressCallsCounter;

    // Remember progress dialog position after it is closed, in order to open it in the same position next time
    QRect m_dlgLastPos;
    bool m_useDlgLastPos;

    /// Show the progress bar dialog if:
    /// 1. The user asked to open the progress dialog
    /// 2. The progress is bigger then threshold
    /// 3. We are not shutting down
    /// \return true if the dialog is shown
    bool ShowDialogIfNeeded();

protected:

    static afProgressBarWrapper* m_pMySingleInstance;

    /// The Qt status bar object (is painted on the left side of the bottom pane)
    QStatusBar* m_pStatusBar;

    /// The Qt status bar object (is painted on the right side of the bottom pane)
    QProgressBar* m_pProgressBar;

    /// A dialog opened for performance bottle necks
    acProgressDlg* m_pProgressDlg;

    // The threshold for showing the progress dialog in milliseconds
    int m_dlgThresholdMsec;
    // The last time measurement
    int m_dlgStartTimeMsec;

    // Timer for refresh rate for the progress dialog
    QTime m_refreshTimer;

};



#endif //__AFPROGRESSBARWRAPPER_H

