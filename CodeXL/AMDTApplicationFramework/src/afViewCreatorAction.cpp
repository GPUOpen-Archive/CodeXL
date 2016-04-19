//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afViewCreatorAction.cpp
///
//==================================================================================

// Local:
#include <AMDTApplicationFramework/Include/afViewCreatorAction.h>


// ---------------------------------------------------------------------------
// Name:        afViewCreatorAction::afViewCreatorAction
// Description: constructor
// Author:      Sigal Algranaty
// Date:        1/9/2011
// ---------------------------------------------------------------------------
afViewCreatorAction::afViewCreatorAction(const gtString& text, QObject* pParent) :
    QAction(QString::fromWCharArray(text.asCharArray()), pParent), _actionGlobalIndex(-1)
{

}

// ---------------------------------------------------------------------------
// Name:        afViewCreatorAction::afViewCreatorAction
// Description: constructor
// Author:      Sigal Algranaty
// Date:        1/9/2011
// ---------------------------------------------------------------------------
afViewCreatorAction::afViewCreatorAction(const QIcon& icon, const gtString& text, QObject* pParent) :
    QAction(icon, QString::fromWCharArray(text.asCharArray()), pParent), _actionGlobalIndex(-1)
{

}


// ---------------------------------------------------------------------------
// Name:        afViewCreatorAction::~afViewCreatorAction
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        1/9/2011
// ---------------------------------------------------------------------------
afViewCreatorAction::~afViewCreatorAction(void)
{
}
