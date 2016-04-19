//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csCallsHistoryLogger.h
///
//==================================================================================

//------------------------------ csCallsHistoryLogger.h ------------------------------

#ifndef __CSCALLSHISTORYLOGGER_H
#define __CSCALLSHISTORYLOGGER_H

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <AMDTServerUtilities/Include/suCallsHistoryLogger.h>


// ----------------------------------------------------------------------------------
// Class Name:           csCallsHistoryLogger : public suCallsHistoryLogger
// General Description:
//  Logs OpenCL monitored function calls (function name, argument values, etc)
//
// Author:               Yaki Tebeka
// Creation Date:        4/11/2009
// ----------------------------------------------------------------------------------
class csCallsHistoryLogger : public suCallsHistoryLogger
{
public:
    csCallsHistoryLogger(int contextId, apMonitoredFunctionId contextCreationFunc, const gtVector<gtString>* pContextAttribs);
    virtual ~csCallsHistoryLogger();

    // Events:
    void onFrameTerminatorCall();

protected:
    // Overrides suCallsHistoryLogger:
    virtual void calculateHTMLLogFilePath(osFilePath& textLogFilePath) const;
    virtual void getHTMLLogFileHeader(gtString& htmlLogFileHeader) const;
    virtual void getHTMLLogFileFooter(gtString& htmlLogFileFooter) const;
    virtual void getPseudoArgumentHTMLLogSection(const apPseudoParameter& pseudoArgument, gtString& htmlLogFileSection);
    virtual void reportMemoryAllocationFailure();

private:
    void parseContextAttribs(const gtVector<gtString>& contextAttribs);
};


#endif //__CSCALLSHISTORYLOGGER_H

