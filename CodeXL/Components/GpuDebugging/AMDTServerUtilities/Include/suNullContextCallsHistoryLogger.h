//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suNullContextCallsHistoryLogger.h
///
//==================================================================================

//------------------------------ suNullContextCallsHistoryLogger.h ------------------------------

#ifndef __SUNULLCONTEXTCALLSHISTORYLOGGER_H
#define __SUNULLCONTEXTCALLSHISTORYLOGGER_H

// Local:
#include <AMDTServerUtilities/Include/suCallsHistoryLogger.h>


// ----------------------------------------------------------------------------------
// Class Name:          suNullContextCallsHistoryLogger : public suCallsHistoryLogger
// General Description:
//  Logs Null context monitored function calls (function name, argument values, etc)
// Author:             Sigal Algranaty
// Creation Date:      21/3/2010
// ----------------------------------------------------------------------------------
class suNullContextCallsHistoryLogger : public suCallsHistoryLogger
{
public:
    suNullContextCallsHistoryLogger();
    virtual ~suNullContextCallsHistoryLogger();

    void onFrameTerminatorCall();

protected:
    // Overrides suCallsHistoryLogger:
    virtual void calculateHTMLLogFilePath(osFilePath& textLogFilePath) const;
    virtual void getHTMLLogFileHeader(gtString& htmlLogFileHeader) const;
    virtual void getHTMLLogFileFooter(gtString& htmlLogFileFooter) const;
};


#endif //__CSCALLSHISTORYLOGGER_H

