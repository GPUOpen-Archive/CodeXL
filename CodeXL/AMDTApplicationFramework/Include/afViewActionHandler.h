//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afViewActionHandler.h
///
//==================================================================================

#ifndef __AFVIEWACTIONHANDLER_H
#define __AFVIEWACTIONHANDLER_H

// System:
#include <QObject>

// Infra:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>

class QWidget;
class QAction;

// ----------------------------------------------------------------------------------
// Class Name:          afViewActionHandler : public QObject
// General Description: Handle visibility action of a view from the menu/toolbar
//                      this handler generated dynamically for each view created
// Author:              Gilad Yarnitzky
// Creation Date:       20/7/2011
// ----------------------------------------------------------------------------------
class AF_API afViewActionHandler : public QObject
{
    Q_OBJECT

public:
    // Constructors based on QAction:
    afViewActionHandler(QWidget* pControlledWidget, QAction* pAction);

    // Destructor:
    virtual ~afViewActionHandler(void);

    // Get the controlled widget
    QWidget* controlledWidget() { return _pControlledWidget; }

    // Get the related action:
    QAction* action() { return _pAction; }

public slots:

    // Handle the toggle state:
    void onViewClicked();
    void onUpdateUI();
    void onWindowActionClicked();

protected:
    // view that is controlled by this action:
    QWidget* _pControlledWidget;

    // The action that create the events:
    QAction* _pAction;
};


#endif //__AFVIEWACTIONHANDLER_H

