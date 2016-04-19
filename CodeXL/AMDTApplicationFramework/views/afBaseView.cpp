//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afBaseView.cpp
///
//==================================================================================

// Qt
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/views/afBaseView.h>

// ---------------------------------------------------------------------------
// Name:        gdThumbnailView::afBaseView
// Description: Constructor
// Arguments:   pParent - The thumbnail view's parent
// Author:      Sigal Algranaty
// Date:        7/11/2010
// ---------------------------------------------------------------------------
afBaseView::afBaseView(afProgressBarWrapper* pProgressBar):
    _pOwnerProgressBar(pProgressBar)
{
}

// ---------------------------------------------------------------------------
// Name:        gdThumbnailView::~gdThumbnailView
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        7/11/2010
// ---------------------------------------------------------------------------
afBaseView::~afBaseView()
{
}


// ---------------------------------------------------------------------------
// Name:        afBaseView::updateProgressBar
// Description: Updates the progress bar in the progress dialog
// Arguments:   newValue - New progress bar value
// Author:      Sigal Algranaty
// Date:        7/11/2010
// ---------------------------------------------------------------------------
void afBaseView::updateProgressBar(int newValue)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pOwnerProgressBar != nullptr)
    {
        _pOwnerProgressBar->updateProgressBar(newValue);
    }
}

// ---------------------------------------------------------------------------
// Name:        afBaseView::hideProgressBar
// Description: Make sure progress bar is at 100% and kill the progress dialog
// Author:      Eran Zinman
// Date:        27/12/2007
// ---------------------------------------------------------------------------
void afBaseView::hideProgressBar()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pOwnerProgressBar != nullptr)
    {
        _pOwnerProgressBar->hideProgressBar();
    }
}


// ---------------------------------------------------------------------------
// Name:        afBaseView::setProgressDetails
// Description: If the progress bar exists, sets its descriptive string to newString
// Author:      Sigal Algranaty
// Date:        8/11/2010
// ---------------------------------------------------------------------------
void afBaseView::setProgressDetails(const gtString& newString, int newRange)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pOwnerProgressBar != nullptr)
    {
        _pOwnerProgressBar->setProgressDetails(newString, newRange);
    }
}

// ---------------------------------------------------------------------------
// Name:        afBaseView::setProgressRange
// Description: If the progress bar exists, sets its range
// Author:      Sigal Algranaty
// Date:        8/11/2010
// ---------------------------------------------------------------------------
void afBaseView::setProgressRange(int newRange)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pOwnerProgressBar != nullptr)
    {
        _pOwnerProgressBar->setProgressRange(newRange);
    }
}

// ---------------------------------------------------------------------------
// Name:        afBaseView::setProgressText
// Description: If the progress bar exists, sets its descriptive string to newString
// Author:      Uri Shomroni
// Date:        12/11/2008
// ---------------------------------------------------------------------------
void afBaseView::setProgressText(const gtString& newString)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pOwnerProgressBar != nullptr)
    {
        _pOwnerProgressBar->setProgressText(newString);
    }
}

// ---------------------------------------------------------------------------
// Name:        afBaseView::incrementProgressBar
// Description: increments the progress bar dialog by a number
// Author:      Uri Shomroni
// Date:        12/11/2008
// ---------------------------------------------------------------------------
void afBaseView::incrementProgressBar(int amount)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pOwnerProgressBar != nullptr)
    {
        _pOwnerProgressBar->incrementProgressBar(amount);
    }
}


// ---------------------------------------------------------------------------
// Name:        afBaseView::progressRange
// Description: Return the current progress range
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        11/11/2010
// ---------------------------------------------------------------------------
int afBaseView::progressRange() const
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pOwnerProgressBar != nullptr)
    {
        return _pOwnerProgressBar->progressRange();
    }

    return 0;
}


// ---------------------------------------------------------------------------
// Name:        shouldUpdateProgress
// Description: return if progress bar should be updated
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        14/2/2011
// ---------------------------------------------------------------------------
bool afBaseView::shouldUpdateProgress() const
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pOwnerProgressBar != nullptr)
    {
        return _pOwnerProgressBar->shouldUpdateProgress();
    }

    return false;
}
