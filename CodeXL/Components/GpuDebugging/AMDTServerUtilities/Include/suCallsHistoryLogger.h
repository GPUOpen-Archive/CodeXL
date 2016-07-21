//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suCallsHistoryLogger.h
///
//==================================================================================

//------------------------------ suCallsHistoryLogger.h ------------------------------

#ifndef __SUCALLSHISTORYLOGGER_H
#define __SUCALLSHISTORYLOGGER_H

// C:
#include <stdarg.h>


// Infra:
#include <AMDTBaseTools/Include/gtAutoPtr.h>
#include <AMDTBaseTools/Include/gtIAllocationFailureObserver.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTAPIClasses/Include/apContextID.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osRawMemoryStream.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTAPIClasses/Include/apFunctionCall.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionId.h>
#include <AMDTAPIClasses/Include/apSearchDirection.h>

// Local:
#include <AMDTServerUtilities/Include/suSpiesUtilitiesDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:   suCallsHistoryLogger : private gtIAllocationFailureObserver
// General Description:
//   Logs monitored function calls (function name, argument values, etc)
//
// Author:               Yaki Tebeka
// Creation Date:        25/6/2003
// ----------------------------------------------------------------------------------
class SU_API suCallsHistoryLogger : private gtIAllocationFailureObserver
{
public:
    suCallsHistoryLogger(apContextID contextID, apMonitoredFunctionId creationFunc, unsigned int maxLoggedFunctions, const wchar_t* loggerMessagesLabelFormat, bool threadSafeLogging);
    virtual ~suCallsHistoryLogger();

    // Implements gtIAllocationFailureObserver:
    virtual void onAllocationFailure();

    // Events:
    virtual void onFrameTerminatorCall() {};

    // Events:
    void onDebuggedProcessTerminationAlert();

    void enableLogging(bool isLoggingEnabled);
    bool isLoggingEnabled() const { return _isLoggingEnabled; };
    void clearLog(bool isInSyncBlock = false);
    void addFunctionCall(apMonitoredFunctionId calledFunctionIndex, int argumentsAmount, va_list& pArgumentList, apFunctionDeprecationStatus functionDeprecationStatus);
    int amountOfFunctionCalls() const;
    bool getFunctionCall(int callIndex, gtAutoPtr<apFunctionCall>& aptrFunctionCall) const;
    bool getCalledFunctionId(int callIndex, int& calledFunctionId) const;
    apMonitoredFunctionId lastCalledFunctionId() const { return _lastCalledFunctionId; };
    bool isInOpenGLBeginEndBlock() const { return _isInOpenGLBeginEndBlock; };
    bool findFunctionCall(apSearchDirection searchDirection, int searchStartIndex,
                          const gtString& searchedString, bool isCaseSensitiveSearch, int& foundIndex) const;
    bool findStringMarker(apSearchDirection searchDirection, int searchStartIndex, int& foundIndex) const;
    bool getHTMLLogFilePath(const osFilePath*& logFilePath) const;
    bool startHTMLLogFileRecording();
    void stopHTMLLogFileRecording();
    bool isRecodringToHTMLLogFile() const { return _isHTMLLogFileActive; };
    void writeFunctionRedundancyStatus(int callIndex, apFunctionRedundancyStatus redundancyStatus);

    apParameter** getStaticTransferableObjTypeToParameterVector() { return _transferableObjTypeToParameter; };

protected:
    // Must be implemented by sub-classes:
    virtual void calculateHTMLLogFilePath(osFilePath& htmlLogFilePath) const = 0;
    virtual void getHTMLLogFileHeader(gtString& htmlLogFileHeader) const = 0;
    virtual void getHTMLLogFileFooter(gtString& htmlLogFileFooter) const = 0;

    // Optional overrides by sub-classes:
    virtual void getPseudoArgumentHTMLLogSection(const apPseudoParameter& pseudoArgument, gtString& htmlLogFileSection);
    virtual void reportExceedingMaximalLoggedFunctionsAmount();
    virtual void reportMemoryAllocationFailure();

    // For sub-classes usage:
    void printToHTMLLogFile(const gtString& printout);
    void closeHTMLLogFile();
    bool allocationFailureOccur() const { return _allocationFailureOccur; };
    bool isHTMLLogFileOpen() const { return _htmlLogFile.isOpened(); };
    unsigned int maxLoggedFunctionsAmount() const {return _maxLoggedFunctions;};

private:
    // Disallow use of my default constructor:
    suCallsHistoryLogger();

    bool initializeTransferableObjectTypeVec();
    void destroyTransferableObjectTypeVec();

    void seekRawMemoryLoggerReadPosition(int callIndex);
    bool fillFunctionArguments(apFunctionCall& functionCall);
    void startLogFilesFunctionLogging(apMonitoredFunctionId functionId);
    void writeArgumentIntoLogFile(const apParameter& argument, bool isFirstFunctionArgument);
    void endLogFilesFunctionLogging(int functionId);
    void outputTextLogFileFooter();
    void outputTextLogRecordingSuspendedMessage();
    void outputTextLogRecordingResumedMessage();
    bool isFunctionCallContainingString(int callIndex, bool isCaseSensitiveSearch, const gtString& searchedString) const;

    inline void beforeLogging();
    inline bool beforeLoggingWithFailure();
    inline void afterLogging();

protected:
    // The context Id;
    apContextID _contextId;

    // The function used to create the context:
    apMonitoredFunctionId m_contextCreationFunc;

    // A string used to describe the context attributes:
    gtString m_contextCreationAttribsString;

    // The calls history log creation time:
    osTime _logCreationTime;

    // A descriptive string for logger messages labeling:
    gtString _loggerMessagesLabel;

private:
    // Contains true iff the functions logging is enabled:
    bool _isLoggingEnabled;

    // Manages thread safety:
    bool _threadSafeLogging;
    bool _loggingCSEntered;
    osCriticalSection _loggingCS;

    // The maximum number of calls to be logged by the logger:
    unsigned int _maxLoggedFunctions;

    // An HTML file into which function calls can be logged:
    osFile _htmlLogFile;
    bool _isHTMLLogFileActive;

    // The text log file path (if exists):
    osFilePath _textLogFilePath;

    // A raw memory stream that stores our function calls:
    osRawMemoryStream _rawMemoryLogger;

    // Maps call no. to its location in the _rawMemoryLogger stream:
    gtVector<size_t> _callLocations;

    // Contains the last called function id:
    // (Is used when functions logging is not enabled)
    apMonitoredFunctionId _lastCalledFunctionId;

    // Is in a glBegin glEnd block ?
    // (Notice that glBegin and glEnd themselves are also considered as part of
    //  the block)
    bool _isInOpenGLBeginEndBlock;

    // Contains true when a memory allocation failure occur:
    bool _allocationFailureOccur;

    // A string buffer that holds pseudo arguments log file printouts:
    // (Pseudo arguments prints are printed at the end of a function call log
    //  printout, hence, we need a temp buffer to store their printout until we
    //  reach the end of the logged function printout):
    gtString _pseudoArgumentsLogFilePrintBuff;

    // Maps apIPCTransferableObjectTypes to apParameter instances.
    // I.E: For each value of the apIPCTransferableObjectTypes:
    //      If it represents an apParameter - Holds an instance of this apParameter.
    //      else - Holds NULL.
    apParameter** _transferableObjTypeToParameter;
};


#endif //__SUCALLSHISTORYLOGGER_H

