//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsCallsHistoryLogger.h
///
//==================================================================================

//------------------------------ gsCallsHistoryLogger.h ------------------------------

#ifndef __GSCALLSHISTORYLOGGER_H
#define __GSCALLSHISTORYLOGGER_H

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>

// Local:
#include <AMDTServerUtilities/Include/suCallsHistoryLogger.h>


// ----------------------------------------------------------------------------------
// Class Name:           gsCallsHistoryLogger : public suCallsHistoryLogger
// General Description:
//  Logs monitored function calls (function name, argument values, etc)
//
// Author:               Yaki Tebeka
// Creation Date:        4/11/2009
// ----------------------------------------------------------------------------------
class gsCallsHistoryLogger : public suCallsHistoryLogger
{
public:
    gsCallsHistoryLogger(int contextId, apMonitoredFunctionId contextCreationFunc, const gtVector<gtString>* pContextAttribs);
    virtual ~gsCallsHistoryLogger();

    // Events:
    virtual void onFrameTerminatorCall();
    virtual void onContextDeletion();

protected:
    // Overrides suCallsHistoryLogger:
    virtual void calculateHTMLLogFilePath(osFilePath& textLogFilePath) const;
    virtual void getHTMLLogFileHeader(gtString& htmlLogFileHeader) const;
    virtual void getHTMLLogFileFooter(gtString& htmlLogFileFooter) const;
    virtual void getPseudoArgumentHTMLLogSection(const apPseudoParameter& pseudoArgument, gtString& htmlLogFileSection);
    virtual void reportMemoryAllocationFailure();

private:
    void outputContextDeletionMessage();
    void getAssociatedTextureHTMLLogSection(const apAssociatedTextureNamesPseudoParameter& assciatedTexture, gtString& htmlLogFileSection);
    void getAssociatedProgramHTMLLogSection(const apAssociatedProgramNamePseudoParameter& associatedProgram, gtString& htmlLogFileSection);
    void getAssociatedShaderHTMLLogSection(const apAssociatedShaderNamePseudoParameter& associatedShader, gtString& htmlLogFileSection);
    void parseContextAttribs(const gtVector<gtString>& contextAttribs);

    // Do not allow the use of the default, copy and move constructors:
    gsCallsHistoryLogger() = delete;
    gsCallsHistoryLogger(const gsCallsHistoryLogger&) = delete;
    gsCallsHistoryLogger(gsCallsHistoryLogger&&) = delete;

};

#endif //__GSCALLSHISTORYLOGGER_H

