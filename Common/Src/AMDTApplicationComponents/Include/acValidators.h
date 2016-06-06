//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acValidators.h
///
//==================================================================================

#ifndef __ACVALIDATORS_H
#define __ACVALIDATORS_H

// Qt:
#include <QValidator>

// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>



/// -----------------------------------------------------------------------------------------------
/// \class Name: AC_API acFileNameValidator : public QValidator
/// \brief Description:  This class is used for validating a string as a valid file name
/// -----------------------------------------------------------------------------------------------
class AC_API acFileNameValidator : public QValidator
{

public:

    acFileNameValidator(QObject* pParent = NULL);
    ~acFileNameValidator();

    //
    /// Overriding QValidator. Validate a string as a valid file name
    /// \param str - the string to validate
    /// \param pos - the position of the string
    /// \return Invalid for an invalid string, Acceptable for an acceptable string
    virtual State validate(QString& str, int& pos) const;

};

#endif //__ACVALIDATORS_H

