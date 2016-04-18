//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afCreatorAction.cpp
///
//==================================================================================

#include <AMDTApplicationFramework/Include/afCreatorAction.h>

// ---------------------------------------------------------------------------
// Name:        afCreatorAction::afCreatorAction
// Description: Creator without icon
// Author:      Gilad Yarnitzky
// Date:        21/7/2011
// ---------------------------------------------------------------------------
afCreatorAction::afCreatorAction(const gtString& text, QObject* pParent, afActionExecutorAbstract* pCreator) :
    QAction(QString::fromWCharArray(text.asCharArray()), pParent), _pActionExecutor(pCreator), _actionGlobalIndex(-1)
{

}

// ---------------------------------------------------------------------------
// Name:        afCreatorAction::afCreatorAction
// Description: Creator with icon
// Author:      Gilad Yarnitzky
// Date:        21/7/2011
// ---------------------------------------------------------------------------
afCreatorAction::afCreatorAction(const QIcon& icon, const gtString& text, QObject* pParent, afActionExecutorAbstract* pCreator) :
    QAction(icon, QString::fromWCharArray(text.asCharArray()), pParent), _pActionExecutor(pCreator), _actionGlobalIndex(-1)
{

}


// ---------------------------------------------------------------------------
// Name:        afCreatorAction::~afCreatorAction
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        21/7/2011
// ---------------------------------------------------------------------------
afCreatorAction::~afCreatorAction(void)
{
}
