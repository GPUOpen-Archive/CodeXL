//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file OclJncReader.h
///
//==================================================================================

#ifndef _OCLJNCREADER_H_
#define _OCLJNCREADER_H_

#ifdef TBI

#include "ProfilingAgentsDataDLLBuild.h"
#include <AMDTExecutableFormat/inc/ExecutableFile.h>
#include <AMDTDisassembler/inc/LibDisassembler.h>
#include <objbase.h>
#include <cor.h>
#include <corprof.h>

class AGENTDATA_API OclJncReader
{
public:
    OclJncReader();
    ~OclJncReader();

    bool Open(const wchar_t* pImageName, gtVAddr loadAddr);
    void Close();

    unsigned int GetSectionNum();
    bool GetModuleName(wchar_t* pModuleName, unsigned int strLength);
    bool GetClassName(wchar_t* pClassName, unsigned int strLength);
    bool GetFunctionName(wchar_t* pFuncName, unsigned int strLength);
    gtUInt64 GetJITLoadAddress();

    gtUByte* GetCodeBytesOfTextSection(unsigned int* pSectionOffset, unsigned int* pSectionSize);
    OPERAND_SIZE OperandSize() const { return m_pe.OperandSize();}
    gtUInt32 GetCodeOffset() const { return m_pe.GetCodeOffset(); }
    LinenumMap* GetpLineMap() { return m_pJitLineMap; }
    gtVAddr GetLastAddress() const { return m_lastAddress; }
    HRESULT Disassemble(gtVAddr address, char* disassembly, gtUInt32* numBytes, bool* isPCRelative, gtUInt32* dispVal);
    void GetCodeBytes(gtVAddr address, gtUByte* bytes, gtUInt32 size);

private:
    bool DoesSectionExist(char* pSectionName);

    ExecutableFile m_pe;
    gtUInt64 m_LoadAddr;
    wchar_t m_ClassName[_MAX_PATH];
    wchar_t m_ModuleName[_MAX_PATH];
    wchar_t m_FuncName[_MAX_PATH];
    gtUInt32 m_numSections;
    gtRVAddr m_sectionStartVAddr;
    gtRVAddr m_sectionEndVAddr;
    gtVAddr m_codeBaseVAddr;
    LibDisassembler m_dasm;
    LinenumMap* m_pJitLineMap;
    gtUByte* m_pCode;
};

#endif // TBI

#endif // _OCLJNCREADER_H_
