//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apSourceCodeBreakpoint.h
///
//==================================================================================

//------------------------------ apSourceCodeBreakpoint.h ------------------------------

#ifndef __APSOURCECODEBREAKPOINT_H
#define __APSOURCECODEBREAKPOINT_H

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/apBreakPoint.h>



// ----------------------------------------------------------------------------------
// Class Name:          apSourceCodeBreakpoint : public apBreakPoint
// General Description: Represents a breakpoint in a source code. The breakpoint is not
//                      set to the API but represent only a breakpoint that is not bound yet
// Author:  AMD Developer Tools Team
// Creation Date:       21/9/2011
// ----------------------------------------------------------------------------------
class AP_API apSourceCodeBreakpoint : public apBreakPoint
{
public:
    apSourceCodeBreakpoint(const osFilePath& filePath, int lineNumber);
    apSourceCodeBreakpoint();
    ~apSourceCodeBreakpoint();

    // Overrides apBreakPoint:
    virtual bool compareToOther(const apBreakPoint& otherBreakpoint) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    void setFilePath(const osFilePath& filePath) {_filePath = filePath;};
    const osFilePath& filePath() const {return _filePath;};
    void setLineNumber(int lineNumber) {_lineNumber = lineNumber;};
    int lineNumber() const {return _lineNumber;};

private:
    osFilePath _filePath;
    int _lineNumber;
};


#endif //__APSOURCECODEBREAKPOINT_H

