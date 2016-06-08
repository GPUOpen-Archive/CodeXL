//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apThreadCreatedEvent.h
///
//==================================================================================

//------------------------------ apThreadCreatedEvent.h ------------------------------

#ifndef __APTHREADCREATEDEVENT
#define __APTHREADCREATEDEVENT

// Forward declarations:
template <class TransferableObjectType> class osTransferableObjectCreator;

// Infra
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apThreadCreatedEvent
// General Description:
//   Is triggered when a thread is created within the debugged process.
// Author:  AMD Developer Tools Team
// Creation Date:        08/5/2005
// ----------------------------------------------------------------------------------
class AP_API apThreadCreatedEvent : public apEvent
{
public:
    apThreadCreatedEvent(const osThreadId& threadOSId, const osThreadId& lwpOSId, const osTime& threadCreationTime, osInstructionPointer threadStartAddress);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Overrides apEvent:
    virtual EventType eventType() const;
    virtual apEvent* clone() const;

    // Self functions:
    const osThreadId& threadOSId() const { return _threadOSId; };
    const osThreadId& lwpOSId() const { return _lwpOSId; };
    const osTime& threadCreationTime() const { return _threadCreationTime; };
    osInstructionPointer threadStartAddress() const { return _threadStartAddress; };
    const osFilePath& threadStartModuleFilePath() const { return _threadStartModuleFilePath; };
    const gtString& threadStartFunctionName() const { return _threadStartFunctionName; };
    const osFilePath& startFunctionSourceCodeFile() const { return _startFunctionSourceCodeFile; };
    int startFunctionSourceCodeFileLineNum() const { return _startFunctionSourceCodeFileLineNum; };

    void setThreadStartModuleFilePath(const osFilePath& moduleFilePath) { _threadStartModuleFilePath = moduleFilePath; };
    void setThreadStartFunctionName(const gtString& functionName) { _threadStartFunctionName = functionName; };
    void setThreadStartSourceCodeDetails(const osFilePath& sourceCodeFile, int lineNumber)
    { _startFunctionSourceCodeFile = sourceCodeFile; _startFunctionSourceCodeFileLineNum = lineNumber; };

private:
    friend class osTransferableObjectCreator<apThreadCreatedEvent>;

    // Do not allow the use of the default constructor:
    apThreadCreatedEvent();

private:
    // The thread OS id:
    osThreadId _threadOSId;

    // The LWP OS id:
    osThreadId _lwpOSId;

    // The thread creation time:
    osTime _threadCreationTime;

    // The thread start address:
    osInstructionPointer _threadStartAddress;

    // The path of the module in which this address resides:
    osFilePath _threadStartModuleFilePath;

    // The name of the function on which the start address points:
    gtString _threadStartFunctionName;

    // The details of the source code that is related to this address:
    osFilePath _startFunctionSourceCodeFile;
    int _startFunctionSourceCodeFileLineNum;
};


#endif  // __APTHREADCREATEDEVENT
