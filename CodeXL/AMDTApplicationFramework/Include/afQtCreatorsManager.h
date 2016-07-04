//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afQtCreatorsManager.h
///
//==================================================================================

#ifndef __AFQTCREATORSMANAGER_H
#define __AFQTCREATORSMANAGER_H

// Qt:
#include <QtWidgets>

// Forward declaration:
class afViewCreatorAbstract;
class afActionExecutorAbstract;
class afMenuActionsExecutor;
class afRecentProjectsActionsExecutor;

// infra:
#include <AMDTAPIClasses/Include/Events/apEvent.h>
#include <AMDTBaseTools/Include/gtVector.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:          afQtCreatorsManager
// General Description: A singleton that manage all the Qt creators:
//                      views, menus, tool bars
// Author:              Gilad Yarnitzky
// Creation Date:       14/7/2011
// ----------------------------------------------------------------------------------
class AF_API afQtCreatorsManager : public QObject
{
    Q_OBJECT

public:
    static afQtCreatorsManager& instance();

    // Initialize the creators manager:
    void initialize();

    // Get the static view creators:
    const gtVector<afViewCreatorAbstract*>& viewsCreators() const { return m_viewsCreators; }

    // Get the actions creators:
    const gtVector<afActionExecutorAbstract*>& actionsCreators() const { return m_actionsExecutors; }

    // register a view creator:
    void registerViewCreator(afViewCreatorAbstract* pViewCreator);

    // register an action creator:
    void registerActionExecutor(afActionExecutorAbstract* pActionExecutor);

    /// Emit a signal that the application tree was created:
    void EmitApplicationTreeCreatedSignal() {emit ApplicationTreeCreated();};

signals:

    /// Signals that the application tree was created:
    void ApplicationTreeCreated();
private:
    // Only afSingletonsDelete can delete my instance:
    friend class afSingletonsDelete;

protected:

    afQtCreatorsManager(void);
    ~afQtCreatorsManager(void);

    // Singleton instance:
    static afQtCreatorsManager* m_spMySingleInstance;

    // Static views creators:
    gtVector<afViewCreatorAbstract*> m_viewsCreators;

    // Actions creators:
    gtVector<afActionExecutorAbstract*> m_actionsExecutors;

    // Framework actions executors:
    afMenuActionsExecutor* m_pMenuCommandsExecutor;

    // Command executor:
    afRecentProjectsActionsExecutor* m_pRecentProjectsActionsExecutor;

};

#endif // __AFQTCREATORSMANAGER_H
