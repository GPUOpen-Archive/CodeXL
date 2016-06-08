//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apKernelSourceCodeBreakpoint.h
///
//==================================================================================

//------------------------------ apKernelSourceCodeBreakpoint.h ------------------------------

#ifndef __APKERNELSOURCECODEBREAKPOINT_H
#define __APKERNELSOURCECODEBREAKPOINT_H

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/apBreakPoint.h>



// ----------------------------------------------------------------------------------
// Class Name:           AP_API apKernelSourceCodeBreakpoint : public apBreakPoint
// General Description: Represents a breakpoint in a kernel / program's source code
// Author:  AMD Developer Tools Team
// Creation Date:        1/11/2010
// ----------------------------------------------------------------------------------
class AP_API apKernelSourceCodeBreakpoint : public apBreakPoint
{
public:
    apKernelSourceCodeBreakpoint(const osFilePath& unresolvedProgramFilePath, int lineNum = -1);
    apKernelSourceCodeBreakpoint(oaCLProgramHandle hProgram = OA_CL_NULL_HANDLE, int lineNum = -1);
    ~apKernelSourceCodeBreakpoint();

    // Overrides apBreakPoint:
    virtual bool compareToOther(const apBreakPoint& otherBreakpoint) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Accessors:
    void setProgramHandle(oaCLProgramHandle hProgram) {_programHandle = hProgram; if (hProgram == OA_CL_NULL_HANDLE) { m_isUnresolved = true; }};
    oaCLProgramHandle programHandle() const {return _programHandle;};
    void setLineNumber(int lineNum) {_lineNumber = lineNum;};
    int lineNumber() const {return _lineNumber;};

    // Breakpoint resolution:
    bool isUnresolved() const {return m_isUnresolved;};
    void setUnresolvedPath(const osFilePath& path) {m_unresolvedFilePath = path; m_isUnresolved = true;};

    const osFilePath& unresolvedPath() const {return m_unresolvedFilePath;};
    void resolveBreakpoint(oaCLProgramHandle programHandle) {_programHandle = programHandle; m_isUnresolved = false;};
    void resolvedHSABreakpoint(const gtString& kernelName) { m_isHSAILBreakpoint = true; m_hsailKernelName = kernelName; m_isUnresolved = false; };
    bool isHSAILBreakpoint() const { return m_isHSAILBreakpoint; };
    const gtString& hsailKernelName() const { return m_hsailKernelName; };

private:
    bool m_isUnresolved;
    bool m_isHSAILBreakpoint;
    gtString m_hsailKernelName;
    osFilePath m_unresolvedFilePath;
    oaCLProgramHandle _programHandle;
    int _lineNumber;
};

#endif //__APKERNELSOURCECODEBREAKPOINT_H

