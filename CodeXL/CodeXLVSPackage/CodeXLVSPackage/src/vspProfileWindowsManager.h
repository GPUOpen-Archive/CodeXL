//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspProfileWindowsManager.h
///
//==================================================================================

#ifndef __VSPPROFILEWINDOWSMANAGER_H
#define __VSPPROFILEWINDOWSMANAGER_H

// Qt:
#include <QWidget>

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>


///
class vspProfileWindowsManager : public apIEventsObserver
{
    friend class vspSingletonsDelete;
public:
    // Get my single instance:
    static vspProfileWindowsManager& instance();

    // Destructor:
    ~vspProfileWindowsManager();


    // Debugged process events callback function:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);

    // Event observer name:
    virtual const wchar_t* eventObserverName() const { return L"vspProfileWindowsManager"; };

private:

    // Only my instance() method should create my single instance:
    vspProfileWindowsManager();

private:

    // Single instance static member:
    static vspProfileWindowsManager* m_pMySingleInstance;


};

#endif //__VSPPROFILEWINDOWSMANAGER_H

