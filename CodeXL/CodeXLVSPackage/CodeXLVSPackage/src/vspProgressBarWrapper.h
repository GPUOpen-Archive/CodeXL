//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspProgressBarWrapper.h
///
//==================================================================================

//------------------------------ ProgressBar.h ------------------------------

#ifndef __VSPPROGRESSBARWRAPPER_H
#define __VSPPROGRESSBARWRAPPER_H

// Infra:
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <Include/Public/CoreInterfaces/IProgressBarDataProvider.h>

// ----------------------------------------------------------------------------------
// Class Name:              vspProgressBarWrapper : public gdProgressBar
// General Description:     implementation of the gdProgressBar for the CodeXL
//                          stand alone application
//                          The class contain only function with different implementations
//                          in standalone and VS package
// Author:               Gilad Yarnitzky
// Creation Date:        14/2/2011
// ----------------------------------------------------------------------------------
class vscProgressBarWrapper : public afProgressBarWrapper
{
    friend class CodeXLVSPackage;

public:

    virtual ~vscProgressBarWrapper();
    vscProgressBarWrapper();

    static void setOwner(IProgressBarEventHandler* pOwner);


    /// Show the progress bar and display the requested message in the status bar:
    /// \param progressMessage the message to display in the status bar
    /// \param itemCount the item count. When it is 0, the progress will show infinitely
    virtual void ShowProgressBar(const gtString& progressMessage, int itemCount = 0);

    /// Show a progress bar for an action taken for an item of type itemType, with the action specified in actionStr:
    /// param itemType the item type for the progress
    /// param actionStr the string of the action taken for this items
    /// param amountOfItems the amount of items which are about to be processed
    virtual void ShowProgressBar(afTreeItemType itemType, const gtString& actionStr, int amountOfItems);

    /// Shut down the progress bar
    virtual void ShutDown();

    /// Show a progress dialog along with the progress bar
    /// param msg the string to display in the progress dialog
    /// param itemCount the number of items which are going to take place. If itemCount == -1 (default), range maximum does not change
    /// param dlgThreshold the threshold for showing the progress dialog
    virtual void ShowProgressDialog(const gtString& msg, int itemCount = -1, int dlgThresholdMsec = 0, bool showCancelButton = false, void(*callbackfunc) = nullptr);

    // specific stand alone implementation
    virtual void ShowProgressBar(int itemType, const gtString& actionWord = GD_STR_ImagesAndBuffersViewerLoadingMessage, int range = 100);
    virtual void hideProgressBar();

    virtual bool shouldUpdateProgress() const;

    virtual void updateProgressBar(int newValue);
    virtual void incrementProgressBar(int amount = 1);

    virtual void setProgressDetails(const gtString& newString, int newRange);
    virtual void setProgressRange(int newRange);

    virtual int progressRange() const ;

    virtual void setProgressText(const gtString& newString);

    void SetProgressBarInfo();

protected:
    static IProgressBarEventHandler* _pOwner;
    BOOL _inProgress;
    gtString _progressBarLabel;
    ULONG _progressBarComplete;
    ULONG _progressBarRange;
};



#endif //__VSPPROGRESSBARWRAPPER_H

