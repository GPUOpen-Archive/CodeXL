//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ProcessMonitor.h
/// \brief  The export description for the CodeAnalyst component
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/inc/ProcessMonitor.h#14 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================
#ifndef _PROCESSMONITOR_H
#define _PROCESSMONITOR_H

#include <QtCore>
#include <QtWidgets>
//AMDTOsWrappers
#include <AMDTOSWrappers/Include/osThread.h>

//deferred
class CPUSessionTreeItemData;

class ProfileProcessMonitor : public osThread
{
public:
    ProfileProcessMonitor(osProcessId launchedProcessId, CPUSessionTreeItemData* pSession, osTime overheadStamp, bool attached = false);
    ~ProfileProcessMonitor();

    // Overrides osThread
    virtual int entryPoint();
    virtual void beforeTermination();

    bool processEnded();
private:
    // Disallow use of my default constructor:
    ProfileProcessMonitor();

    //Fill the string with the duration
    void fillDuration(const wchar_t* label, gtUInt32 halfSeconds);

    // The process Id of the launcher process:
    osProcessId m_launcherProcessId;

    // The session profile options
    CPUSessionTreeItemData* m_pSession;

    bool m_processEnded;
    bool m_attachedProcess;

    unsigned int m_overheadSec;

    //The actual duration of the profile in 500 millisecond units
    gtUInt32 m_actualDuration;
};

#endif //_PROCESSMONITOR_H
