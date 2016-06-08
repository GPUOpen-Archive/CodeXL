//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acFindParameters.h
///
//==================================================================================

//------------------------------ acFindParameters.h ------------------------------

#ifndef __ACFINDPARAMETERS_H
#define __ACFINDPARAMETERS_H

// Qt:
#include <QDialog>

// Infra:
#include <AMDTBaseTools/Include/gtASCIIString.h>

// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>

class AC_API acFindParameters
{
public:

    acFindParameters(const acFindParameters& other);
    acFindParameters& operator=(const acFindParameters& other);

    static acFindParameters& Instance();

    void Clear();

    /// The requested find expression:
    QString m_findExpr;

    /// True iff the user should only find case sensitive results:
    bool m_isCaseSensitive;

    /// True iff we should search up:
    bool m_isSearchUp;

    /// Used was the find implementation - first line from which the search should be executed:
    int m_findFirstLine;
    bool m_lastResult;
    bool m_findFromStart;

    /// Should the find respond to text change, or only to enter?
    bool m_shouldRespondToTextChange;

private:

    acFindParameters();
    static acFindParameters* m_spMySingleInstance;
};

#endif

