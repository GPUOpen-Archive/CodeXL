//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afCreatorAction.h
///
//==================================================================================

#ifndef __AFCREATORACTION_H
#define __AFCREATORACTION_H

// Forward deceleration:
class afActionExecutorAbstract;

// System:
#include <QAction>

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:          afCreatorAction : public QAction
// General Description: Action that stores its creator and index of the action
//                      This class is used in the menu trigger event and aboutToShow
// Author:              Gilad Yarnitzky
// Creation Date:       21/7/2011
// ----------------------------------------------------------------------------------
class AF_API afCreatorAction : public QAction
{
    Q_OBJECT

public:

    afCreatorAction(const gtString& text, QObject* pParent, afActionExecutorAbstract* pCreator);
    afCreatorAction(const QIcon& icon, const gtString& text, QObject* pParent, afActionExecutorAbstract* pCreator);
    virtual ~afCreatorAction();

    // Get the creator:
    afActionExecutorAbstract* actionExecutor()  { return _pActionExecutor; }

    // Get the index of the action in the creator:
    void setActionGlobalIndex(int index) {_actionGlobalIndex = index;}
    int actionGlobalIndex() const { return _actionGlobalIndex; }

protected:

    // Creator of the action:
    afActionExecutorAbstract* _pActionExecutor;

    // Index of action in the creator:
    int _actionGlobalIndex;
};

#endif //__AFCREATORACTION_H

