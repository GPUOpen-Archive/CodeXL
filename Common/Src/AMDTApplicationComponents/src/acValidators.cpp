//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acValidators.cpp
///
//==================================================================================

/// Local:
#include <AMDTApplicationComponents/Include/acValidators.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

acFileNameValidator::acFileNameValidator(QObject* pParent)
{
    GT_UNREFERENCED_PARAMETER(pParent);
}

acFileNameValidator::~acFileNameValidator()
{
}

QValidator::State acFileNameValidator::validate(QString& str, int& pos) const
{
    GT_UNREFERENCED_PARAMETER(pos);

    State retVal = Acceptable;

    if (str.contains(QRegExp("[/:\"*?<>|]+")))
    {
        retVal = Invalid;
    }

    if (str.contains('\\'))
    {
        retVal = Invalid;
    }

    return retVal;
}
