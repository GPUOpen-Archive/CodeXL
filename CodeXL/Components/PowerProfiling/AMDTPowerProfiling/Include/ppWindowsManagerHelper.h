//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppWindowsManagerHelper.h
///
//==================================================================================

//------------------------------ vspPowerProfilingEditorManager.h ------------------------------

#ifndef __PPWINDOWSMANAGERHELPER_H
#define __PPWINDOWSMANAGERHELPER_H

// Qt
#include <QtWidgets>

// Local:
#include <AMDTPowerProfiling/src/ppAppController.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/SessionTreeNodeData.h>

class PP_API ppWindowsManagerHelper : public QObject
{
    Q_OBJECT

public:
    ppWindowsManagerHelper();
    ~ppWindowsManagerHelper();

    /// Activate the session window belongs to pSessionData:
    virtual bool ActivateSessionWindow(SessionTreeNodeData* pSessionData);

protected:
    /// singleton instance
    static ppAppController* m_spMySingleInstance;

};
#endif //PPWINDOWSMANAGERHELPER
