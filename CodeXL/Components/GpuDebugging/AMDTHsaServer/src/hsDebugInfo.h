//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file hsDebugInfo.h
///
//==================================================================================

#ifndef __HSDEBUGINFO_H
#define __HSDEBUGINFO_H

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osFilePath.h>


class hsDebugInfo
{
public:
    hsDebugInfo(const void* pBinary, size_t binarySize, const void* kernelArgs);
    ~hsDebugInfo();

    bool LineToAddrs(gtUInt64 line, gtVector<gtUInt64>& o_addrs) const;
    bool AddrToLine(gtUInt64 addr, gtUInt64& o_line) const;
    const osFilePath& GetSourceFilePath() const { return m_sourceFilePath; };
    bool EvaluateVariable(const gtString& varName, gtString& o_varValue, gtString* o_pVarValueHex, gtString* o_pVarType) const;
    bool ListVariables(gtVector<gtString>& o_variables) const;
    bool GetStepBreakpoints(bool stepOut, gtVector<gtUInt64>& o_bps) const;
    bool GetStepInBreakpoints(gtVector<gtUInt64>& o_bps) const;
    bool IsInitialized() const { return nullptr != m_hDbgInfo; };

private:
    hsDebugInfo() = delete;
    hsDebugInfo(const hsDebugInfo&) = delete;
    hsDebugInfo(hsDebugInfo&&) = delete;
    hsDebugInfo& operator=(const hsDebugInfo&) = delete;
    hsDebugInfo& operator=(hsDebugInfo&&) = delete;

    void determineSourceFilePath(const char* pSrc, size_t srcLen);

    void* m_hDbgInfo;
    const void* m_pKernelArgs;
    osFilePath m_sourceFilePath;
};

#endif // __HSDEBUGINFO_H
