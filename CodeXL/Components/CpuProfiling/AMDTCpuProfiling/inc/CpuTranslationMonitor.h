//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuTranslationMonitor.h
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/inc/CpuTranslationMonitor.h#9 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================

#ifndef _CPUTRANSLATIONMONITOR_H
#define _CPUTRANSLATIONMONITOR_H

//AMDTOsWrappers
#include <QThread>

//Backend
#include <AMDTCpuProfilingTranslation/inc/CpuProfileDataTranslation.h>


// ----------------------------------------------------------------------------------
// Class Name:           CpuTranslationMonitor
// General Description:  This class is responsible for calling the synchronized translation and
//                          waiting for it
// Author:  AMD Developer Tools Team
// Date:                 5/29/2012
// ----------------------------------------------------------------------------------
class CpuTranslationMonitor : public QThread
{
public:
    CpuTranslationMonitor(ReaderHandle* pHandle, const gtString& sessionFile, HRESULT* pRet);
    ~CpuTranslationMonitor();

    // Overrides osThread
    virtual void run();
private:
    // Disallow use of my default constructor:
    CpuTranslationMonitor();

    ReaderHandle* m_pHandle;
    gtString m_sessionFile;
    HRESULT* m_pRet;
};

#endif //_CPUTRANSLATIONMONITOR_H
