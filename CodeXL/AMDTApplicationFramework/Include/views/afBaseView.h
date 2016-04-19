//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afBaseView.h
///
//==================================================================================

#ifndef __AFBASEVIEW
#define __AFBASEVIEW

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>

class afProgressBarWrapper;

// ----------------------------------------------------------------------------------
// Class Name:           afBaseView
// General Description:  Basic CodeXL view.
//                       The class is used for implementation of all basic functionality for
//                       CodeXL views
// ----------------------------------------------------------------------------------
class AF_API afBaseView
{
public:
    // Constructor
    afBaseView(afProgressBarWrapper* pProgressBar);

    // Destructor
    virtual ~afBaseView();

    // Edit menu commands
    virtual void onUpdateEdit_Copy(bool& isEnabled)         { (void)(isEnabled); }
    virtual void onUpdateEdit_SelectAll(bool& isEnabled)        { (void)(isEnabled); }
    virtual void onUpdateEdit_Find(bool& isEnabled)         { (void)(isEnabled); }
    virtual void onUpdateEdit_FindNext(bool& isEnabled)     { (void)(isEnabled); }

    virtual void onEdit_Copy()          { ; }
    virtual void onEdit_SelectAll()         { ; }
    virtual void onEdit_Find()          { ; }
    virtual void onEdit_FindNext()          { ; }
    /// \param[out] selectedText returns the text currently selected
    virtual void GetSelectedText(gtString& selectedText) { (void)(selectedText); }

    afProgressBarWrapper* progressBar() const {return _pOwnerProgressBar;};

protected:

    // Progress bar functions:
    void hideProgressBar();

    bool shouldUpdateProgress() const;

    void updateProgressBar(int newValue);
    void incrementProgressBar(int amount = 1);

    void setProgressDetails(const gtString& newString, int newRange);
    void setProgressRange(int newRange);

    int progressRange() const ;

    void setProgressText(const gtString& newString);


protected:

    // The frame I should use in order to update the progress:
    afProgressBarWrapper* _pOwnerProgressBar;

};

#endif  // __AFBASEVIEW
