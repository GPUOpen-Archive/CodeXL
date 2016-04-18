//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afLineEdit.h
///
//==================================================================================

#ifndef __AFLINEEDIT_H
#define __AFLINEEDIT_H

// Qt
#include <QLineEdit>


// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:          afLineEdit
// General Description: This class inherits QLineEdit. Use this class when a user edit
//                      completion is required. Make sure to store a unique object name.
//                      This object name is used to store the edit history in the framework.
// ----------------------------------------------------------------------------------
class AF_API afLineEdit : public QLineEdit
{
    Q_OBJECT

public:

    /// Constructor:
    afLineEdit(const QString& objectName, QWidget* pParent = nullptr);
    afLineEdit(const QString& objectName, const QString& contents, QWidget* pParent = nullptr);

    /// Destructor:
    ~afLineEdit();

protected slots:

    /// Is handling the editing finished signal:
    void OnEditFinished();

private:

    /// Initializes the line edit completer:
    void InitilaizeCompleter();


};

#endif //__AFLINEEDIT_H

