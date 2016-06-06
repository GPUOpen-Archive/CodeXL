//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acLineEdit.cpp
///
//==================================================================================

//------------------------------ acLineEdit.cpp ------------------------------

// Qt:
#include <QtWidgets>
#include <QTableWidget>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>

// Local:
#include <AMDTApplicationComponents/Include/acLineEdit.h>
#include <inc/acStringConstants.h>


// ---------------------------------------------------------------------------
// Name:        acLineEdit::acLineEdit
// Description: Constructor
// Arguments:   QWidget* pParent
// Author:      Sigal Algranaty
// Date:        1/8/2012
// ---------------------------------------------------------------------------
acLineEdit::acLineEdit(QWidget* pParent) : QLineEdit(pParent)
{
}

// ---------------------------------------------------------------------------
// Name:        acLineEdit::~acLineEdit
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        1/8/2012
// ---------------------------------------------------------------------------
acLineEdit::~acLineEdit()
{
}

// ---------------------------------------------------------------------------
// Name:        acLineEdit::focusInEvent
// Description: Derived focus in method
// Arguments:   QFocusEvent* pEvent
/// Author:      Sigal Algranaty
// Date:        1/8/2012
// ---------------------------------------------------------------------------
void acLineEdit::focusInEvent(QFocusEvent* pEvent)
{
    QLineEdit::focusInEvent(pEvent);
    emit(focused(true));
}

// ---------------------------------------------------------------------------
// Name:        acLineEdit::focusOutEvent
// Description: Derived focus out method
// Arguments:   QFocusEvent* pEvent
// Author:      Sigal Algranaty
// Date:        1/8/2012
// ---------------------------------------------------------------------------
void acLineEdit::focusOutEvent(QFocusEvent* pEvent)
{
    QLineEdit::focusOutEvent(pEvent);
    emit(focused(false));
}


