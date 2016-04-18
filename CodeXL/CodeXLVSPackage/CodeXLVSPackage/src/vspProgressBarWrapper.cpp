//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspProgressBarWrapper.cpp
///
//==================================================================================

//------------------------------ vspProgressBarWrapper.cpp ------------------------------
#include "stdafx.h"

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationFramework/Include/afTreeItemType.h>

// Local:
#include <src/vspProgressBarWrapper.h>

#include <AMDTApplicationComponents/Include/acProgressDlg.h>


// Static data members initializations:
IProgressBarEventHandler* vscProgressBarWrapper::_pOwner = NULL;

// ---------------------------------------------------------------------------
// Name:        vspProgressBarWrapper::vspProgressBarWrapper
// Description:
// Return Val:
// Author:      Sigal Algranaty
// Date:        7/2/2011
// ---------------------------------------------------------------------------
vscProgressBarWrapper::vscProgressBarWrapper() :
    _inProgress(false),
    _progressBarLabel(L""),
    _progressBarComplete(0),
    _progressBarRange(0)
{

}

// ---------------------------------------------------------------------------
// Name:        vspProgressBarWrapper::~vspProgressBarWrapper
// Description:
// Return Val:
// Author:      Sigal Algranaty
// Date:        7/2/2011
// ---------------------------------------------------------------------------
vscProgressBarWrapper::~vscProgressBarWrapper()
{
}


// ---------------------------------------------------------------------------
// Name:        showProgressBar
// Description: Show progress bar
// Arguments:   gdMonitoredObjectType itemType
//              const gtString& actionWord = GD_STR_ImagesAndBuffersViewerLoadingMessage
//              int range=100
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        14/2/2011
// ---------------------------------------------------------------------------
void vscProgressBarWrapper::ShowProgressBar(int itemType, const gtString& actionWord, int range)
{
    // Generate a loading message (generic message):
    gtString loadingMessage = actionWord;

    // Generate a generic message according to the viewer item type:
    switch (itemType)
    {
        case AF_TREE_ITEM_CL_IMAGES_NODE:
            loadingMessage.append(GD_STR_ImagesAndBuffersViewerLoadingCLImagesHeaderMessage);
            break;

        case AF_TREE_ITEM_GL_TEXTURES_NODE:
            loadingMessage.append(GD_STR_ImagesAndBuffersViewerLoadingGLTexturesHeaderMessage);
            break;

        case AF_TREE_ITEM_GL_TEXTURE:
            loadingMessage.append(GD_STR_ImagesAndBuffersViewerLoadingGLTextureMessage);
            break;

        case AF_TREE_ITEM_CL_IMAGE:
            loadingMessage.append(GD_STR_ImagesAndBuffersViewerLoadingCLImageMessage);
            break;

        case AF_TREE_ITEM_GL_PBUFFER_STATIC_BUFFER:
        case AF_TREE_ITEM_GL_STATIC_BUFFER:
        case AF_TREE_ITEM_GL_RENDER_BUFFER:
        case AF_TREE_ITEM_GL_FBO:
        case AF_TREE_ITEM_GL_VBO:
        case AF_TREE_ITEM_CL_BUFFER:
        case AF_TREE_ITEM_CL_SUB_BUFFER:
            loadingMessage.append(GD_STR_ImagesAndBuffersViewerLoadingBufferMessage);
            break;

        case AF_TREE_ITEM_GL_VBO_NODE:
            loadingMessage.append(GD_STR_ImagesAndBuffersViewerInitializingBuffersMessage);
            break;

        default:
        {
            // Just put a generic message:
            loadingMessage = GD_STR_ImagesAndBuffersViewerPleaseWaitMessage;

            // Unknown item type
            GT_ASSERT_EX(false, L"unknown item type!");
        }
        break;

        case AF_TREE_ITEM_ITEM_NONE:
            //No extra loading message needed
            break;
    }

    // Update progress bar information:
    _progressBarLabel = loadingMessage;
    _progressBarRange = range;
    _progressBarComplete = 0;
    _inProgress = true;

    // Update the progress bar:
    SetProgressBarInfo();
}

void vscProgressBarWrapper::ShowProgressBar(const gtString& progressMessage, int itemCount /*= 0*/)
{
    _inProgress = true;
    afProgressBarWrapper::ShowProgressBar(progressMessage, itemCount);
}

void vscProgressBarWrapper::ShowProgressBar(afTreeItemType itemType, const gtString& actionStr, int amountOfItems)
{
    _inProgress = true;
    afProgressBarWrapper::ShowProgressBar(itemType, actionStr, amountOfItems);
}

void vscProgressBarWrapper::ShutDown()
{
    _inProgress = false;
}

void vscProgressBarWrapper::ShowProgressDialog(const gtString& msg, int itemCount, int dlgThresholdMsec, bool showCancelButton, void(*callbackfunc))
{
    _inProgress = true;
    afProgressBarWrapper::ShowProgressDialog(msg, itemCount, dlgThresholdMsec, showCancelButton, callbackfunc);
}

// ---------------------------------------------------------------------------
// Name:        hideProgressBar
// Description: Hide progress bar
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        14/2/2011
// ---------------------------------------------------------------------------
void vscProgressBarWrapper::hideProgressBar()
{
    // Update progress bar information:
    _inProgress = false;

    // Update the progress bar:
    SetProgressBarInfo();

    afProgressBarWrapper::hideProgressBar();
}

// ---------------------------------------------------------------------------
// Name:        shouldUpdateProgress
// Description: should progress bar be updated
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        14/2/2011
// ---------------------------------------------------------------------------
bool vscProgressBarWrapper::shouldUpdateProgress() const
{
    bool ret = false;
    GT_IF_WITH_ASSERT(_pOwner != NULL)
    {
        return _pOwner->ShouldUpdateProgress();
    }
    return ret;
}


// ---------------------------------------------------------------------------
// Name:        updateProgressBar
// Description: update progress bar with new value
// Arguments:   int newValue
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        14/2/2011
// ---------------------------------------------------------------------------
void vscProgressBarWrapper::updateProgressBar(int newValue)
{
    // Update progress bar information:
    _progressBarComplete = newValue;

    // Update the progress bar:
    SetProgressBarInfo();
    afProgressBarWrapper::updateProgressBar(newValue);
}

// ---------------------------------------------------------------------------
// Name:        incrementProgressBar
// Description: progress bar default increment value
// Arguments:   int amount /*= 1*/
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        14/2/2011
// ---------------------------------------------------------------------------
void vscProgressBarWrapper::incrementProgressBar(int amount /*= 1*/)
{
    // Update progress bar information:
    _progressBarComplete += amount;

    if (_progressBarComplete > _progressBarRange)
    {
        _progressBarComplete = _progressBarRange;
    }

    // Update the progress bar:
    SetProgressBarInfo();
    afProgressBarWrapper::incrementProgressBar(amount);
}


// ---------------------------------------------------------------------------
// Name:        setProgressDetails
// Description: Set progress bar information
// Arguments:   const gtString& newString
//              int newRange
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        14/2/2011
// ---------------------------------------------------------------------------
void vscProgressBarWrapper::setProgressDetails(const gtString& newString, int newRange)
{
    // Update progress bar information:
    _progressBarRange = newRange;
    _progressBarLabel = newString;
    _inProgress = true;

    // Update the progress bar:
    SetProgressBarInfo();
    afProgressBarWrapper::setProgressDetails(newString, newRange);
}

// ---------------------------------------------------------------------------
// Name:        setProgressRange
// Description:
// Arguments:   int newRange
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        14/2/2011
// ---------------------------------------------------------------------------
void vscProgressBarWrapper::setProgressRange(int newRange)
{
    // Update progress bar information:
    _progressBarRange = newRange;

    // Update the progress bar:
    SetProgressBarInfo();
    afProgressBarWrapper::setProgressRange(newRange);
}

// ---------------------------------------------------------------------------
// Name:        progressRange
// Description: Get progress bar range
// Return Val:  int
// Author:      Gilad Yarnitzky
// Date:        14/2/2011
// ---------------------------------------------------------------------------
int vscProgressBarWrapper::progressRange() const
{
    return _progressBarRange;
}

// ---------------------------------------------------------------------------
// Name:        setProgressText
// Description: Set progress bar displayed text
// Arguments:   const gtString& newString
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        14/2/2011
// ---------------------------------------------------------------------------
void vscProgressBarWrapper::setProgressText(const gtString& newString)
{
    afProgressBarWrapper::setProgressText(newString);

    // Update progress bar information:
    _progressBarLabel = newString;

    // Update the progress bar:
    SetProgressBarInfo();
    afProgressBarWrapper::setProgressText(newString);
}


// ---------------------------------------------------------------------------
// Name:        SetProgressBarInfo
// Description: Set progress bar info based on attributes set
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        14/2/2011
// ---------------------------------------------------------------------------
void vscProgressBarWrapper::SetProgressBarInfo()
{
    GT_IF_WITH_ASSERT(_pOwner != NULL)
    {
        _pOwner->SetProgressInfo(_progressBarLabel.asCharArray(), _inProgress, _progressBarComplete, _progressBarRange);
    }
}

void vscProgressBarWrapper::setOwner(IProgressBarEventHandler* pOwner)
{
    _pOwner = pOwner;
    GT_ASSERT(pOwner != NULL);
}
