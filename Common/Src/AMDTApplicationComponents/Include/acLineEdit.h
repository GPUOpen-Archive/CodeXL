//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acLineEdit.h
///
//==================================================================================

//------------------------------ acLineEdit.h ------------------------------

#ifndef __ACLINEEDIT_H
#define __ACLINEEDIT_H

// For compilers that support precompilation, includes "wx/wx.h".
#include <QtWidgets>

// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:          AC_API acLineEdit : public QLineEdit
// General Description: Define a customized line edit
// Author:              Sigal Algranaty
// Creation Date:       1/8/2012
// ----------------------------------------------------------------------------------
class AC_API acLineEdit : public QLineEdit
{
    Q_OBJECT
public:

    acLineEdit(QWidget* pParent);
    ~acLineEdit();

protected:

    // Event processing:
    virtual void focusInEvent(QFocusEvent* pEvent);
    virtual void focusOutEvent(QFocusEvent* pEvent);

Q_SIGNALS:
    void focused(bool hasFocus);

};

#endif //__ACLINEEDIT_H

