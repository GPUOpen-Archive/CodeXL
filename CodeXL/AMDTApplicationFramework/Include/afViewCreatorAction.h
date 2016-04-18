//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afViewCreatorAction.h
///
//==================================================================================

#ifndef __AFVIEWCREATORACTION_H
#define __AFVIEWCREATORACTION_H

// Forward deceleration:
class afViewCreatorAbstract;

// System:
#include <QAction>

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:          afViewCreatorAction : public QAction
// General Description: This action is used for storing view actions data
// Author:              Sigal Algranaty
// Creation Date:       1/9/2011
// ----------------------------------------------------------------------------------
class AF_API afViewCreatorAction : public QAction
{
    Q_OBJECT

public:

    afViewCreatorAction(const gtString& text, QObject* pParent);
    afViewCreatorAction(const QIcon& icon, const gtString& text, QObject* pParent);

    virtual ~afViewCreatorAction();

    // Get the index of the action in the creator:
    void setActionGlobalIndex(int index) { _actionGlobalIndex = index; }
    int actionGlobalIndex() const { return _actionGlobalIndex; }

protected:

    // Index of global action in the creator:
    int _actionGlobalIndex;
};

#endif //__AFVIEWCREATORACTION_H

