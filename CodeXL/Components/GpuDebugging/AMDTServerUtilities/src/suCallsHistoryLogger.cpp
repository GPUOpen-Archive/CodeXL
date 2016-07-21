//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suCallsHistoryLogger.cpp
///
//==================================================================================

//------------------------------ suCallsHistoryLogger.cpp ------------------------------

// C:
#include <AMDTOSWrappers/Include/osStdLibIncludes.h>

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTOSWrappers/Include/osTransferableObjectCreatorsManager.h>
#include <AMDTOSWrappers/Include/osTransferableObjectType.h>
#include <AMDTAPIClasses/Include/apiClassesInitFunc.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionsManager.h>
#include <AMDTAPIClasses/Include/apFunctionCall.h>
#include <AMDTAPIClasses/Include/apParameters.h>

// Local:
#include <AMDTServerUtilities/Include/suStringConstants.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>

// EGL:
#ifdef OS_OGL_ES_IMPLEMENTATION_DLL_BUILD
    #include <src/eglInitFunc.h>
#endif

// Local:
#include <AMDTServerUtilities/Include/suCallsHistoryLogger.h>

// Static members and variables initializations:
// --------------------------------------------
// Initialize raw memory size to be 1 Kb:
static const size_t INITIALE_SIZE_OF_RAW_MEMORY = 1024;

// Static argument sizes:
static size_t static_sizeOfTransferableObjectType = sizeof(osTransferableObjectType);
static size_t static_sizeOfInt = sizeof(int);
static size_t static_sizeOfUInt = sizeof(unsigned int);


// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::suCallsHistoryLogger
// Description: Constructor
//
// Author:      Yaki Tebeka
// Date:        5/7/2003
// ---------------------------------------------------------------------------
suCallsHistoryLogger::suCallsHistoryLogger(apContextID contextId, apMonitoredFunctionId creationFunc, unsigned int maxLoggedFunctions, const wchar_t* loggerMessagesLabelFormat, bool threadSafeLogging)
    : _contextId(contextId),
      m_contextCreationFunc(creationFunc),
      _isLoggingEnabled(true),
      _threadSafeLogging(threadSafeLogging),
      _loggingCSEntered(false),
      _maxLoggedFunctions(maxLoggedFunctions),
      _isHTMLLogFileActive(false),
      _rawMemoryLogger(INITIALE_SIZE_OF_RAW_MEMORY, threadSafeLogging),
      _lastCalledFunctionId(apMonitoredFunctionsAmount),
      _isInOpenGLBeginEndBlock(false),
      _allocationFailureOccur(false),
      _transferableObjTypeToParameter(NULL)
{
    // Initialize the logger messages label:
    _loggerMessagesLabel.appendFormattedString(loggerMessagesLabelFormat, contextId._contextId);

    // Initialize the _transferableObjTypeToParameter vector:
    bool rc = initializeTransferableObjectTypeVec();
    GT_ASSERT(rc);

    // Register me to receive _rawMemoryLogger memory allocation failures notifications:
    _rawMemoryLogger.registerAllocationFailureObserver(this);

    // Initialize the log file creation time to the current time:
    _logCreationTime.setFromCurrentTime();
}


// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::~suCallsHistoryLogger
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        5/5/2004
// ---------------------------------------------------------------------------
suCallsHistoryLogger::~suCallsHistoryLogger()
{
    // De-initialize the _transferableObjTypeToParameter vector:
    destroyTransferableObjectTypeVec();

    // Close the physical log file:
    closeHTMLLogFile();

    // Unregister me from receiving _rawMemoryLogger memory allocation failures notifications:
    _rawMemoryLogger.registerAllocationFailureObserver(NULL);
}


// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::onAllocationFailure
// Description: Is called when the raw memory stream fails to allocate
//              memory required for its operation.
// Author:      Yaki Tebeka
// Date:        31/1/2009
// ---------------------------------------------------------------------------
void suCallsHistoryLogger::onAllocationFailure()
{
    _allocationFailureOccur = true;
}


// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::clearLog();
// Description:
//  Is called when a frame terminator function is called.
//  Clears the calls history log (deletes all the logged function calls).
// Author:      Yaki Tebeka
// Date:        17/5/2004
// ---------------------------------------------------------------------------
void suCallsHistoryLogger::clearLog(bool isInSyncBlock)
{
    // If we're already in the sync block, there's no need to enter it again; doing so
    // causes the members to become incorrect:
    if (!isInSyncBlock)
    {
        beforeLogging();
    }

    // If the text log file is active - flush it:
    if (_isHTMLLogFileActive)
    {
        _htmlLogFile.flush();
    }

    _rawMemoryLogger.clear();
    _callLocations.clear();
    _isInOpenGLBeginEndBlock = false;
    _lastCalledFunctionId = apMonitoredFunctionsAmount;

    if (!isInSyncBlock)
    {
        afterLogging();
    }
}


// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::enableLogging
// Description: Enabled / disabled monitored functions logging.
// Arguments:   isLoggingEnabled - true - enable monitored functions logging.
//                                 false - disabled monitored functions logging.
// Author:      Yaki Tebeka
// Date:        3/3/2005
// ---------------------------------------------------------------------------
void suCallsHistoryLogger::enableLogging(bool isLoggingEnabled)
{
    _isLoggingEnabled = isLoggingEnabled;
}


// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::addFunctionCall
// Description: Logs a monitored function call.
// Arguments:   calledFunctionIndex - An index identifying the called function.
//              argumentsAmount - The amount of function arguments.
//              pArgumentList - An argument list that contains type, value pairs.
//                              Example: OS_TOBJ_ID_GL_INT_PARAMETER, 3,
//                                       OS_TOBJ_ID_GL_FLOAT_PARAMETER, 4.4
//              apFunctionDeprecationStatus functionDeprecationStatus - the function deprecation status
//
// Author:      Yaki Tebeka
// Date:        30/6/2003
//
// Implementation Notes:
//
// a. Logging called function and arguments:
//    -------------------------------------
//    The function calls are logged into a raw memory chunk.
//    Each function log has the following memory layout:
//    Header: <called function id><amount of arguments>
//    Arguments: <argument type><argument value> ...  <argument type><argument value>
//
// b. The use of stdarg:
//    -----------------
//    We decided to use stdarg for logging the called function argument values because
//    of efficiency reasons.
//    - See also "The use of stdarg" comment at the top of ApiClasses/src/apParameters.cpp
// ---------------------------------------------------------------------------
void suCallsHistoryLogger::addFunctionCall(apMonitoredFunctionId calledFunctionIndex, int argumentsAmount, va_list& pArgumentList, apFunctionDeprecationStatus functionDeprecationStatus)
{
    // Find out what is the current execution mode:
    apExecutionMode currentExecMode = suDebuggedProcessExecutionMode();

    // If monitored functions logging is enabled, and we are in debugging or analyze mode:
    if (_isLoggingEnabled && (currentExecMode != AP_PROFILING_MODE))
    {
        beforeLogging();

        // If we are about to exceed the logged functions amount limit:
        gtSize_t loggedFunctionsAmount = _callLocations.size();

        if (loggedFunctionsAmount >= _maxLoggedFunctions)
        {
            reportExceedingMaximalLoggedFunctionsAmount();
        }

        // Store the place (memory location) in which the current function was logged:
        size_t functionLogPosition = _rawMemoryLogger.currentWritePosition();
        _callLocations.push_back(functionLogPosition);

        // Log the called function index:
        _rawMemoryLogger.write((gtByte*)&calledFunctionIndex, static_sizeOfInt);

        // Write the initial value of the function redundancy status:
        static unsigned int initialRedundancyStatus = (unsigned int)AP_REDUNDANCY_UNKNOWN;
        _rawMemoryLogger.write((gtByte*)&initialRedundancyStatus, static_sizeOfUInt);

        // Write the value of the function deprecation status:
        unsigned int functionDeprecationStatusAsInt = (unsigned int)functionDeprecationStatus;
        _rawMemoryLogger.write((gtByte*)&functionDeprecationStatusAsInt, static_sizeOfUInt);

        // Write the called function into the log files:
        startLogFilesFunctionLogging(calledFunctionIndex);

        // Log the amount of arguments
        _rawMemoryLogger.write((gtByte*)&argumentsAmount, static_sizeOfInt);

        // Iterate on the argument list:
        va_list pCurrentArgument;
        va_copy(pCurrentArgument, pArgumentList);
        int currentArgumentIndex = 1;

        while (currentArgumentIndex <= argumentsAmount)
        {
            // Get and log the argument type:
            osTransferableObjectType argumentType = (osTransferableObjectType)(va_arg(pCurrentArgument , int));
            _rawMemoryLogger.write((gtByte*)&argumentType, static_sizeOfTransferableObjectType);

            // Get a parameter object that match this argument type:
            apParameter* pStatParameter = _transferableObjTypeToParameter[argumentType];

            if (pStatParameter)
            {
                // Read the parameter from the arguments list:
                pStatParameter->readValueFromArgumentsList(pCurrentArgument);

                // Log the argument:
                pStatParameter->writeSelfIntoChannel(_rawMemoryLogger);

                // Write the argument into the log files:
                bool isFirstArgument = (currentArgumentIndex == 1);
                writeArgumentIntoLogFile(*pStatParameter, isFirstArgument);
            }
            else
            {
                // We failed to find a parameter that match this argument type:
                GT_ASSERT(0);

                // Exit the loop:
                currentArgumentIndex = argumentsAmount;
            }

            // Increment the current argument index:
            currentArgumentIndex++;
        }

        // End the current function log file logging:
        endLogFilesFunctionLogging(calledFunctionIndex);

        // Free the arguments pointer:
        va_end(pCurrentArgument);

        // If a memory allocation failure occur:
        if (_allocationFailureOccur)
        {
            reportMemoryAllocationFailure();
        }

        afterLogging();
    }

    // Store the called function:
    _lastCalledFunctionId = calledFunctionIndex;

    // Handle glBegin - glEnd block:
    if (calledFunctionIndex == ap_glBegin)
    {
        _isInOpenGLBeginEndBlock = true;
    }
    else if (calledFunctionIndex == ap_glEnd)
    {
        _isInOpenGLBeginEndBlock = false;
    }
}


// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::writeFunctionRedundancyStatus
// Description: Write a function redundancy status to the stream
// Arguments: int callIndex
//            apFunctionRedundancyStatus redundancyStatus
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        8/7/2008
// ---------------------------------------------------------------------------
void suCallsHistoryLogger::writeFunctionRedundancyStatus(int callIndex, apFunctionRedundancyStatus redundancyStatus)
{
    beforeLogging();

    // Get the current write position (in order to turn it back to what it was later):
    size_t currentWritePosition = _rawMemoryLogger.currentWritePosition();

    // Get the location in which the requested call resides:
    size_t functionLocation = _callLocations[callIndex];

    // This position is supposed to point the function id, progress to the next position:
    functionLocation += static_sizeOfInt;

    // Seek the raw memory logger to this position:
    _rawMemoryLogger.seekWritePosition(functionLocation);

    // Write the redundancy status:
    unsigned int redundancyStatusAsUInt = (unsigned int)redundancyStatus;
    _rawMemoryLogger.write((gtByte*)&redundancyStatusAsUInt, static_sizeOfUInt);

    // Set the stream write position back to what it was:
    _rawMemoryLogger.seekWritePosition(currentWritePosition);

    afterLogging();
}


// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::amountOfFunctionCalls
// Description: Returns the amount of function calls currently stored in this logger.
// Author:      Yaki Tebeka
// Date:        24/4/2004
// ---------------------------------------------------------------------------
int suCallsHistoryLogger::amountOfFunctionCalls() const
{
    return (int)_callLocations.size();
}


// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::getFunctionCall
// Description: Inputs a call index and returns an apFunctionCall object that
//              represents it.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        24/4/2004
// ---------------------------------------------------------------------------
bool suCallsHistoryLogger::getFunctionCall(int callIndex, gtAutoPtr<apFunctionCall>& aptrFunctionCall) const
{
    bool rc = false;

    // callIndex range test:
    if ((0 <= callIndex) && (callIndex < amountOfFunctionCalls()))
    {
        // Get a non-const pointer to myself:
        // (This function has const semantics, but it actually modifies the read position):
        suCallsHistoryLogger& nonConstMe = *((suCallsHistoryLogger*)this);

        // Prevent access while we are here:
        bool canRead = nonConstMe.beforeLoggingWithFailure();

        if (canRead)
        {
            // Seek the raw memory read position to the beginning of the requested
            // function call raw memory log:
            nonConstMe.seekRawMemoryLoggerReadPosition(callIndex);

            // Get the logged function index:
            int functionIndex = 0;
            unsigned int redundancyStatusUInt = AP_REDUNDANCY_UNKNOWN;
            unsigned int functionDeprecationStatusAsUInt = AP_DEPRECATION_NONE;
            rc = nonConstMe._rawMemoryLogger.read((gtByte*)&functionIndex, static_sizeOfInt);
            rc = nonConstMe._rawMemoryLogger.read((gtByte*)&redundancyStatusUInt, static_sizeOfUInt) && rc;
            rc = nonConstMe._rawMemoryLogger.read((gtByte*)&functionDeprecationStatusAsUInt, static_sizeOfUInt) && rc;

            apFunctionRedundancyStatus redundancyStatus = (apFunctionRedundancyStatus)redundancyStatusUInt;
            apFunctionDeprecationStatus deprecationStatus = (apFunctionDeprecationStatus)functionDeprecationStatusAsUInt;

            if (rc)
            {
                // Create a transferable object that represents the function call:
                apFunctionCall* pFunctionCall = new apFunctionCall((apMonitoredFunctionId)functionIndex);


                // Set the function redundancy status:
                pFunctionCall->setRedundanctStatus(redundancyStatus);

                // Set the function deprecation status:
                pFunctionCall->setDeprecationStatus(deprecationStatus);

                if (pFunctionCall)
                {
                    // Get the function arguments:
                    nonConstMe.fillFunctionArguments(*pFunctionCall);

                    // Return the transferable object:
                    aptrFunctionCall = pFunctionCall;
                }
                else
                {
                    // We failed to create the apFunctionCall object:
                    rc = false;
                }
            }

            // Release the CS if we entered it:
            nonConstMe.afterLogging();
        }
    }

    return rc;
}


// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::getCalledFunctionId
// Description: Inputs a call index and returns the called function id.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        26/7/2004
// ---------------------------------------------------------------------------
bool suCallsHistoryLogger::getCalledFunctionId(int callIndex, int& calledFunctionId) const
{
    bool rc = false;

    // callIndex range test:
    if ((0 <= callIndex) && (callIndex < amountOfFunctionCalls()))
    {
        // Get a non-const pointer to myself:
        // (This function has const semantics, but it actually modifies the read position):
        suCallsHistoryLogger& nonConstMe = *((suCallsHistoryLogger*)this);

        // Prevent access while we are here:
        bool canRead = nonConstMe.beforeLoggingWithFailure();

        if (canRead)
        {
            // Seek the raw memory read position to the beginning of the requested
            // function call raw memory log:
            nonConstMe.seekRawMemoryLoggerReadPosition(callIndex);

            // Get the logged function index:
            calledFunctionId = 0;
            rc = nonConstMe._rawMemoryLogger.read((gtByte*)&calledFunctionId, static_sizeOfInt);

            // Release the CS if we entered it:
            nonConstMe.afterLogging();
        }
    }

    return rc;
}


// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::findFunctionCall
// Description: Search the frame calls log for a function call using a function
//              call sub-string.
// Arguments:   contextId - The id of the context who's log will be searched.
//              searchDirection - The search direction.
//              searchStartIndex - The index of the function call from which the
//                                 search will begin.
//              searchedString - The sub-string to which this function will search for.
//                               (Each queried function call is translated into string using
//                                apFunctionCall::asString(), then the sub-string is searched
//                                in the function call string)
//
//              foundIndex - The index of the found function call, or -1 if the search
//                           didn't find any function call that contains the input search
//                           string.
// Arguments:   apSearchDirection searchDirection
//              int searchStartIndex
//              int& foundIndex
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        15/12/2004
// ---------------------------------------------------------------------------
bool suCallsHistoryLogger::findFunctionCall(apSearchDirection searchDirection,
                                            int searchStartIndex, const gtString& searchedString,
                                            bool isCaseSensitiveSearch, int& foundIndex) const
{
    bool retVal = false;

    // Verify that the start index exists in the input context calls log:
    int amountOfFuncCalls = amountOfFunctionCalls();

    if ((0 <= searchStartIndex) && (searchStartIndex < amountOfFuncCalls))
    {
        retVal = true;

        gtString searchedStringProperCase = searchedString;

        if (!isCaseSensitiveSearch)
        {
            searchedStringProperCase.toLowerCase();
        }

        // Search down the indices
        if (searchDirection == AP_SEARCH_INDICES_DOWN)
        {
            int searchEndIndex = amountOfFunctionCalls() - 1;

            for (int i = searchStartIndex; i <= searchEndIndex; i++)
            {
                if (isFunctionCallContainingString(i, isCaseSensitiveSearch, searchedStringProperCase))
                {
                    foundIndex = i;
                    break;
                }
            }
        }
        else if (searchDirection == AP_SEARCH_INDICES_UP)
        {
            // Search up the indices
            for (int i = searchStartIndex; 0 <= i; i--)
            {
                if (isFunctionCallContainingString(i, isCaseSensitiveSearch, searchedStringProperCase))
                {
                    foundIndex = i;
                    break;
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::findStringMarker
// Description: Search the current frame function calls list for a string marker.
//              (See GL_GREMEDY_string_marker extension for more details).
// Arguments:   searchDirection - The search direction.
//              searchStartIndex - The index of the function call from which the
//                                 search will begin.
//              foundIndex - The index of the found string marker, or -1 if the search
//                           didn't find any string marker when searching the search
//                           direction.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        1/2/2005
// ---------------------------------------------------------------------------
bool suCallsHistoryLogger::findStringMarker(apSearchDirection searchDirection, int searchStartIndex, int& foundIndex) const
{
    bool retVal = false;

    // Verify that the start index exists in the input context calls log:
    int amountOfFuncCalls = amountOfFunctionCalls();

    if ((0 <= searchStartIndex) && (searchStartIndex < amountOfFuncCalls))
    {
        retVal = true;

        // If the search direction is down:
        int currentFunctionId = 0;

        if (searchDirection == AP_SEARCH_INDICES_DOWN)
        {
            // Search down the indices
            for (int i = searchStartIndex; 0 <= i; i--)
            {
                // Get the current function id:
                bool rc = getCalledFunctionId(i, currentFunctionId);

                if (rc)
                {
                    // If the current function is a string marker:
                    if (currentFunctionId == ap_glStringMarkerGREMEDY)
                    {
                        foundIndex = i;
                        break;
                    }
                }
            }
        }
        else if (searchDirection == AP_SEARCH_INDICES_UP)
        {
            // Search up the indices
            int searchEndIndex = amountOfFunctionCalls() - 1;

            for (int i = searchStartIndex; i <= searchEndIndex; i++)
            {
                // Get the current function id:
                bool rc = getCalledFunctionId(i, currentFunctionId);

                if (rc)
                {
                    // If the current function is a string marker:
                    if (currentFunctionId == ap_glStringMarkerGREMEDY)
                    {
                        foundIndex = i;
                        break;
                    }
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::startHTMLLogFileRecording
// Description: Start recoding into the text log file.
//              If the log file does not exist - creates it.
//              If the log file exists - the new log printouts will be appended into
//              the existing log file.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        18/8/2004
// ---------------------------------------------------------------------------
bool suCallsHistoryLogger::startHTMLLogFileRecording()
{
    bool retVal = true;

    if (!_isHTMLLogFileActive)
    {
        // If the text log file is not open - open it:
        if (!_htmlLogFile.isOpened())
        {
            // Get the text log file path:
            osFilePath textLogFilePath;
            calculateHTMLLogFilePath(textLogFilePath);

            // Open the text log file:
            retVal = _htmlLogFile.open(textLogFilePath, osChannel::OS_UNICODE_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);

            if (retVal)
            {
                // Store the log file path:
                _textLogFilePath = textLogFilePath;

                // Write a log file created debug string:
                gtString debugStr = SU_STR_logFileCreated;
                debugStr += textLogFilePath.asString().asCharArray();
                osOutputDebugString(debugStr.asCharArray());

                // Write the log file header:
                gtString htmlLogFileHeader;
                getHTMLLogFileHeader(htmlLogFileHeader);
                _htmlLogFile << htmlLogFileHeader;
                _htmlLogFile.flush();
            }
        }
        else
        {
            // The log file is already opened:
            outputTextLogRecordingResumedMessage();
        }

        // Mark that the text log file is active:
        _isHTMLLogFileActive = true;
    }

    return retVal;

}


// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::stopHTMLLogFileRecording
// Description: Stop recording into the text log files.
// Author:      Yaki Tebeka
// Date:        18/8/2004
// ---------------------------------------------------------------------------
void suCallsHistoryLogger::stopHTMLLogFileRecording()
{
    // If are indeed recording into the HTML log file:
    if (_isHTMLLogFileActive)
    {
        // Output a log file recoding suspended message:
        outputTextLogRecordingSuspendedMessage();
    }

    // Disable log file recording:
    _isHTMLLogFileActive = false;
}


// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::getHTMLLogFilePath
// Description: Returns the text log file path.
// Arguments:   logFilePath - will get the text log file path.
// Return Val:  bool - Success / failure (or - no log file exists).
// Author:      Yaki Tebeka
// Date:        23/3/2005
// ---------------------------------------------------------------------------
bool suCallsHistoryLogger::getHTMLLogFilePath(const osFilePath*& logFilePath) const
{
    bool retVal = false;

    const gtString& logFilePathAsString = _textLogFilePath.asString();

    if (!logFilePathAsString.isEmpty())
    {
        logFilePath = &_textLogFilePath;
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::onDebuggedProcessTerminationAlert
// Description: Is called before the debugged process is terminated.
// Author:      Yaki Tebeka
// Date:        25/1/2005
// ---------------------------------------------------------------------------
void suCallsHistoryLogger::onDebuggedProcessTerminationAlert()
{
    // If the text log file is opened:
    if (_htmlLogFile.isOpened())
    {
        // Stop the text log file recording:
        stopHTMLLogFileRecording();

        // Output the footer message:
        outputTextLogFileFooter();

        // Close the text log file:
        _htmlLogFile.close();
    }
}


// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::initializeTransferableObjectTypeVec
// Description: Initializes the _transferableObjTypeToParameter static vector.
// Author:      Yaki Tebeka
// Date:        11/5/2004
// ---------------------------------------------------------------------------
bool suCallsHistoryLogger::initializeTransferableObjectTypeVec()
{
    bool retVal = true;

    // If the static vector was not initialized yet:
    if (_transferableObjTypeToParameter == NULL)
    {
        // Initialize the ApiClasses library:
        retVal = apiClassesInitFunc();

        GT_IF_WITH_ASSERT(retVal)
        {
            // If we are building an OpenGL ES implementation -
            // initialize the EGL Library implementation:
#ifdef OS_OGL_ES_IMPLEMENTATION_DLL_BUILD
            {
                eglInitFunc();
            }
#endif

            // Allocate the _transferableObjTypeToParameter array:
            _transferableObjTypeToParameter = new apParameter*[OS_AMOUNT_OF_TRANSFERABLE_OBJECT_TYPES];


            // Get the transferable objects creator manager:
            osTransferableObjectCreatorsManager& creatorsMgr = osTransferableObjectCreatorsManager::instance();

            // Fill the _transferableObjTypeToParameter array:
            for (unsigned int i = 0; i < OS_AMOUNT_OF_TRANSFERABLE_OBJECT_TYPES; i++)
            {
                // Create an instance of the current transferable object:
                gtAutoPtr<osTransferableObject> aptrTransferableObj;
                bool rc = creatorsMgr.createObject(i, aptrTransferableObj);

                apParameter* pParameter = NULL;

                if (rc)
                {
                    // Get ownership on the transferable object memory:
                    osTransferableObject* pTransferableObj = aptrTransferableObj.releasePointedObjectOwnership();

                    // Try to down cast it to an apParameter:
                    if (pTransferableObj->isParameterObject())
                    {
                        pParameter = (apParameter*)pTransferableObj;
                    }
                }

                // Push the apParameter pointer into _transferableObjTypeToParameter.
                // (NULL value in case of a non apParameter object).
                _transferableObjTypeToParameter[i] = pParameter;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::destroyTransferableObjectTypeVec
// Description: Deletes the transferable object instances held in the
//              _transferableObjTypeToParameter static vector.
// Author:      Yaki Tebeka
// Date:        11/5/2004
// ---------------------------------------------------------------------------
void suCallsHistoryLogger::destroyTransferableObjectTypeVec()
{
    for (unsigned int i = 0; i < OS_AMOUNT_OF_TRANSFERABLE_OBJECT_TYPES; i++)
    {
        osTransferableObject* pCurrentObject = _transferableObjTypeToParameter[i];
        delete pCurrentObject;
        _transferableObjTypeToParameter[i] = NULL;
    }
}


// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::seekRawMemoryLoggerReadPosition
// Description: Seek the raw memory logger read position to the place
//              where an input call index log starts.
// Author:      Yaki Tebeka
// Date:        26/7/2004
// ---------------------------------------------------------------------------
void suCallsHistoryLogger::seekRawMemoryLoggerReadPosition(int callIndex)
{
    // Get the location in which the requested call resides:
    size_t functionLocation = _callLocations[callIndex];

    // Seek the raw memory logger to this position:
    _rawMemoryLogger.seekReadPosition(functionLocation);
}


// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::fillFunctionArguments
// Description: Inputs an apFunctionCall object and fills its argument list
//              from the raw memory logger.
// Arguments:   functionCall - The function call to be filled.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        26/4/2004
// ---------------------------------------------------------------------------
bool suCallsHistoryLogger::fillFunctionArguments(apFunctionCall& functionCall)
{
    bool rc = true;

    // Get the amount of function arguments:
    int argumentsAmount = 0;
    rc = _rawMemoryLogger.read((gtByte*)&argumentsAmount, static_sizeOfInt);

    if (rc)
    {
        // Iterate on the function arguments:
        osTransferableObjectCreatorsManager& transferableObjMgr = osTransferableObjectCreatorsManager::instance();

        for (int i = 0; i < argumentsAmount; i++)
        {
            // Read the current argument type:
            unsigned int argumentType = 0;
            rc = _rawMemoryLogger.read((gtByte*)&argumentType, static_sizeOfInt);

            if (rc)
            {
                // Create the transferable object that represents this argument type:
                gtAutoPtr<osTransferableObject> aptrTransferableObj;
                rc = transferableObjMgr.createObject(argumentType, aptrTransferableObj);

                if (rc)
                {
                    // Verify that this is an apParameter sub-class:
                    rc = aptrTransferableObj->isParameterObject();

                    if (rc)
                    {
                        // Down cast it into apParameter:
                        gtAutoPtr<apParameter> aptrCurrentParam = (apParameter*)(aptrTransferableObj.releasePointedObjectOwnership());

                        // Read its value from the raw memory logger:
                        rc = aptrCurrentParam->readSelfFromChannel(_rawMemoryLogger);

                        if (rc)
                        {
                            // If this is an additional data attached to this function
                            if (aptrCurrentParam->isPseudoParameter())
                            {
                                gtAutoPtr<apPseudoParameter> aptrPseudoParam = (apPseudoParameter*)(aptrCurrentParam.releasePointedObjectOwnership());
                                functionCall.addAdditionalDataParameter(aptrPseudoParam);
                            }
                            else
                            {
                                // This is a real parameter:
                                functionCall.addArgument(aptrCurrentParam);
                            }
                        }
                    }
                }

                if (!rc)
                {
                    // A failure happened:
                    GT_ASSERT(0);

                    // Exit the loop:
                    i = argumentsAmount;
                }
            }
        }
    }

    return rc;
}


// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::startLogFilesFunctionLogging
// Description: Start logging a function into the log files.
// Arguments:   functionId - The id of the function to be logged.
// Author:      Yaki Tebeka
// Date:        19/8/2004
// ---------------------------------------------------------------------------
void suCallsHistoryLogger::startLogFilesFunctionLogging(apMonitoredFunctionId functionId)
{
    if (_isHTMLLogFileActive)
    {
        // Clear the pseudo arguments printouts buffer:
        _pseudoArgumentsLogFilePrintBuff.makeEmpty();

        // If this is a string marker:
        if (functionId == ap_glStringMarkerGREMEDY)
        {
            _htmlLogFile << SU_STR_startStringMarkerInHTMLLog;
        }
        else
        {
            // Get the function name:
            static apMonitoredFunctionsManager& monitoredFuncMgr = apMonitoredFunctionsManager::instance();
            gtString functionNameStr = monitoredFuncMgr.monitoredFunctionName(functionId);
            _htmlLogFile << functionNameStr;
            _htmlLogFile << L"(";
        }

        // If we were asked to flush the log file after every function call,
        // we will flush it also now (this helps identifying spy crashes
        // when logging monitored function calls)
        bool shouldFlushLogFileAfterEachFunctionCall = suShouldFlushLogFileAfterEachFunctionCall();

        if (shouldFlushLogFileAfterEachFunctionCall)
        {
            _htmlLogFile.flush();
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::writeArgumentIntoLogFile
// Description: Logs a function argument into the log files.
// Arguments:   argument - The input function argument.
//              isFirstFunctionArgument - true iff this is the first argument of
//                                        the logged function.
// Author:      Yaki Tebeka
// Date:        19/8/2004
// ---------------------------------------------------------------------------
void suCallsHistoryLogger::writeArgumentIntoLogFile(const apParameter& argument, bool isFirstFunctionArgument)
{
    if (_isHTMLLogFileActive)
    {
        // If the parameter is a "pseudo parameter":
        if (argument.isPseudoParameter())
        {
            // Generate an HTML section representing this pseudo argument:
            gtString htmlLogFileSection;
            getPseudoArgumentHTMLLogSection((const apPseudoParameter&)argument, htmlLogFileSection);
            _pseudoArgumentsLogFilePrintBuff += htmlLogFileSection;
        }
        else
        {
            // This is a real parameter:

            // Add comma, if needed:
            if (!isFirstFunctionArgument)
            {
                _htmlLogFile << L", ";
            }

            // Write the argument value (as a string) into the log file:
            gtString argumentValueAsString;
            argument.valueAsString(argumentValueAsString);
            _htmlLogFile << argumentValueAsString;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::endLogFilesFunctionLogging
// Description: Ends logging a function into the log files.
// Arguments:   functionId - The id of the logged function.
// Author:      Yaki Tebeka
// Date:        19/8/2004
// ---------------------------------------------------------------------------
void suCallsHistoryLogger::endLogFilesFunctionLogging(int functionId)
{
    if (_isHTMLLogFileActive)
    {
        // If this is a string marker:
        if (functionId == ap_glStringMarkerGREMEDY)
        {
            _htmlLogFile << SU_STR_endStringMarkerInHTMLLog;
        }
        else
        {
            // Close the function arguments list:
            _htmlLogFile << L")";

            // If we have pseudo arguments printouts:
            if (!_pseudoArgumentsLogFilePrintBuff.isEmpty())
            {
                // Print them into the log file:
                _htmlLogFile << L" ";
                _htmlLogFile << _pseudoArgumentsLogFilePrintBuff;
            }

            // New line:
            _htmlLogFile << L" <br>\n";
        }

        // If we were asked to flush the log file after every function call:
        bool shouldFlushLogFileAfterEachFunctionCall = suShouldFlushLogFileAfterEachFunctionCall();

        if (shouldFlushLogFileAfterEachFunctionCall)
        {
            _htmlLogFile.flush();
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::outputTextLogFileFooter
// Description: Outputs the text log file footer into the text log file.
// Author:      Yaki Tebeka
// Date:        25/1/2005
// ---------------------------------------------------------------------------
void suCallsHistoryLogger::outputTextLogFileFooter()
{
    gtString htmlLogFileFooter;
    getHTMLLogFileFooter(htmlLogFileFooter);
    _htmlLogFile << htmlLogFileFooter;
    _htmlLogFile.flush();
}


// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::outputTextLogRecordingSuspendedMessage
// Description: Outputs into the text log file a message that tells the
//              user that the log file recording was suspended.
// Author:      Yaki Tebeka
// Date:        19/8/2004
// ---------------------------------------------------------------------------
void suCallsHistoryLogger::outputTextLogRecordingSuspendedMessage()
{
    _htmlLogFile << L"<h3><br>\n";
    _htmlLogFile << L"////////////////////////////////////////////////////////////<br>\n";
    _htmlLogFile << L"// Log file recording suspended<br>\n";

    osTime currentTime;
    currentTime.setFromCurrentTime();
    gtString timeAsString;
    currentTime.timeAsString(timeAsString, osTime::WINDOWS_STYLE, osTime::LOCAL);
    _htmlLogFile << L"// Suspension time: ";
    _htmlLogFile << timeAsString;
    _htmlLogFile << L"<br>\n";

    _htmlLogFile << L"////////////////////////////////////////////////////////////<br>\n</h3><br>\n";

    _htmlLogFile.flush();
}


// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::outputTextLogRecordingResumedMessage
// Description: Outputs into the text log file a message that tells the
//              user that the log file recording was resumed.
// Author:      Yaki Tebeka
// Date:        19/8/2004
// ---------------------------------------------------------------------------
void suCallsHistoryLogger::outputTextLogRecordingResumedMessage()
{
    _htmlLogFile << L"<h3>\n////////////////////////////////////////////////////////////<br>\n";
    _htmlLogFile << L"// Log file recording resumed<br>\n";

    osTime currentTime;
    currentTime.setFromCurrentTime();
    gtString timeAsString;
    currentTime.timeAsString(timeAsString, osTime::WINDOWS_STYLE, osTime::LOCAL);
    _htmlLogFile << L"// Resume time: ";
    _htmlLogFile << timeAsString;
    _htmlLogFile << L"<br>\n";

    _htmlLogFile << L"////////////////////////////////////////////////////////////<br>\n</h3><br>\n";

    _htmlLogFile.flush();
}


// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::isFunctionCallContainingString
// Description: Returns true iff the input string is contained in the string form
//              of the input call index.
//              The string form is generated using apFunctionCall::asString
// Arguments:   callIndex - The input call index.
//              searchedString - The input searched string.
// Return Val:  bool - true iff the searched string is contained in the string form
//                     of the input call index.
// Author:      Yaki Tebeka
// Date:        16/12/2004
// ---------------------------------------------------------------------------
bool suCallsHistoryLogger::isFunctionCallContainingString(int callIndex, bool isCaseSensitiveSearch, const gtString& searchedString) const
{
    bool retVal = false;

    // Get the function call that match the input index:
    gtAutoPtr<apFunctionCall> aptrFunctionCall;
    bool rc = getFunctionCall(callIndex, aptrFunctionCall);

    if (rc)
    {
        // Translate the function call into a string:
        gtString functionCallAsString;
        aptrFunctionCall->asString(functionCallAsString);

        if (!isCaseSensitiveSearch)
        {
            functionCallAsString.toLowerCase();
        }

        // Check if the searched string is contained in the function call string form:
        if (functionCallAsString.find(searchedString) != -1)
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::getPseudoArgumentHTMLLogSection
// Description: Returns an HTML section appropriate for a given HTML section.
// Arguments: pseudoArgument - An input pseudo argument.
//            htmlLogFileSection - The HTML log file section representing the input
//                                 pseudo argument.
// Author:      Yaki Tebeka
// Date:        8/11/2009
// ---------------------------------------------------------------------------
void suCallsHistoryLogger::getPseudoArgumentHTMLLogSection(const apPseudoParameter& pseudoArgument, gtString& htmlLogFileSection)
{
    (void)(pseudoArgument); // unused
    // We expect sub classes to implement this function (if they want).
    // This class does not do anything with pseudo arguments.
    htmlLogFileSection.makeEmpty();
}


// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::reportExceedingMaximalLoggedFunctionsAmount
// Description:
//  Reports to my debugger that the amount of logged function calls exceed
//  the maximal allowed amount of logged function calls.
//
// Author:      Yaki Tebeka
// Date:        22/8/2007
// ---------------------------------------------------------------------------
void suCallsHistoryLogger::reportExceedingMaximalLoggedFunctionsAmount()
{
    // If we are not in glBegin-glEnd block:
    if (!_isInOpenGLBeginEndBlock)
    {
        // Do not Report an error to the debug log:
        // gtString errorDescription = _loggerMessagesLabel;
        // errorDescription.appendFormattedString(SU_STR_maxLoggedFunctionsAmountReached, _maxLoggedFunctions);
        // OS_OUTPUT_DEBUG_LOG(errorDescription.asCharArray(), OS_DEBUG_LOG_ERROR);

        // Reset the log:
        clearLog(true);
    }
}


// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::reportMemoryAllocationFailure
// Description: Reports to my debugger about memory allocation failure.
// Author:      Yaki Tebeka
// Date:        31/1/2009
// ---------------------------------------------------------------------------
void suCallsHistoryLogger::reportMemoryAllocationFailure()
{
    // If we are not in glBegin-glEnd block:
    if (!_isInOpenGLBeginEndBlock)
    {
        // Get the amount of function calls:
        int functionCallsAmount = amountOfFunctionCalls();

        // Calculate the report string:
        gtString errorDescription = _loggerMessagesLabel;

        if (_allocationFailureOccur)
        {
            errorDescription.appendFormattedString(SU_STR_notEnoughMemoryForLoggingFunctions, functionCallsAmount);
        }
        else
        {
            errorDescription.appendFormattedString(SU_STR_maxLoggedFunctionsAmountReached, functionCallsAmount);
        }

        // Report an error to the debug log:
        OS_OUTPUT_DEBUG_LOG(errorDescription.asCharArray(), OS_DEBUG_LOG_ERROR);

        // Reset the log:
        clearLog(true);

        // Reset the memory allocation failure flag:
        _allocationFailureOccur = false;
    }
}


// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::printToHTMLLogFile
// Description: Prints a string directly into the HTML log file.
// Arguments: printout - The input string to be printed to the HTML log file.
// Author:      Yaki Tebeka
// Date:        8/11/2009
// ---------------------------------------------------------------------------
void suCallsHistoryLogger::printToHTMLLogFile(const gtString& printout)
{
    _htmlLogFile <<  printout;
}


// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::closeHTMLLogFile
// Description: Closes the physical log file.
// Author:      Yaki Tebeka
// Date:        8/11/2009
// ---------------------------------------------------------------------------
void suCallsHistoryLogger::closeHTMLLogFile()
{
    // If the text log file is opened:
    if (_htmlLogFile.isOpened())
    {
        // Output the footer message:
        outputTextLogFileFooter();

        // Close the text log file:
        _htmlLogFile.close();

        // Make sure that the html log file is not marked as active:
        _isHTMLLogFileActive = false;
    }
}

// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::beforeLogging
// Description: Called before a change is made to the log (raw memory stream, calls
//              locations vector, etc.)
// Author:      Uri Shomroni
// Date:        30/11/2011
// ---------------------------------------------------------------------------
void suCallsHistoryLogger::beforeLogging()
{
    // If we are thread-safe, perform some wait:
    if (_threadSafeLogging)
    {
        // Enter the critical section:
        _loggingCS.enter();
        _loggingCSEntered = true;
    }
}

// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::beforeLoggingWithFailure
// Description: Called before a change is made to the log (raw memory stream, calls
//              locations vector, etc.). This version of the function is allowed to
//              fail and should be used for reading or data that can be lost safely.
// Author:      Uri Shomroni
// Date:        27/09/2012
// ---------------------------------------------------------------------------
bool suCallsHistoryLogger::beforeLoggingWithFailure()
{
    bool retVal = true;

    // If we are thread-safe, perform some wait:
    if (_threadSafeLogging)
    {
        // Try entering the critical section:
        retVal = _loggingCS.tryEntering();

        if (retVal)
        {
            _loggingCSEntered = true;
        };
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        suCallsHistoryLogger::afterLogging
// Description: Called after a change is made to the log (raw memory stream, calls
//              locations vector, etc.)
// Author:      Uri Shomroni
// Date:        30/11/2011
// ---------------------------------------------------------------------------
void suCallsHistoryLogger::afterLogging()
{
    // Enter here even if the value of _threadSafeLogging changed:
    if (_loggingCSEntered)
    {
        // Release the waiting thread:
        _loggingCSEntered = false;
        _loggingCS.leave();
    }
}
