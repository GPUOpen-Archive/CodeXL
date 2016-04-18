//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ClrJncReader.h
///
//==================================================================================

#ifndef _CLRJNCREADER_H_
#define _CLRJNCREADER_H_

#include "../ProfilingAgentsDataDLLBuild.h"
#include <AMDTExecutableFormat/inc/ExecutableFile.h>
#include <objbase.h>
#pragma warning( push )
#pragma warning( disable : 4091)
#include <cor.h>
#include <corprof.h>
#pragma warning( pop )
class AGENTDATA_API ClrJncReader
{
public:
    ClrJncReader();
    ~ClrJncReader();

    bool Open(const wchar_t* pImageName);
    void Close();

    unsigned int GetSectionNum() const;
    bool GetModuleName(wchar_t* pModuleName, unsigned int strLength) const;
    bool GetClassName(wchar_t* pClassName, unsigned int strLength) const;
    bool GetFunctionName(wchar_t* pFuncName, unsigned int strLength) const;
    gtUInt64 GetJITLoadAddress() const;
    bool GetILInfo(unsigned int* pILOffsetToImage, unsigned int* pILSize) const;

    const gtUByte* GetCodeBytesOfTextSection(unsigned int* pSectionOffset, unsigned int* pSectionSize) const;
    const gtUByte* GetILMetaData() const;
    unsigned int GetILNativeMapCount() const;
    const COR_DEBUG_IL_TO_NATIVE_MAP* GetILNativeMapInfo() const;
    bool Is64Bit() const { return m_pExecutable->Is64Bit(); }

private:
    bool DoesSectionExist(char* pSectionName);

    ExecutableFile* m_pExecutable;
    gtUInt64 m_LoadAddr;
    wchar_t m_ClassName[OS_MAX_PATH];
    wchar_t m_ModuleName[OS_MAX_PATH];
    wchar_t m_FuncName[OS_MAX_PATH];
    unsigned int m_ILoffset;
    const gtUByte* m_pILMetaData;
    unsigned int m_ILSize;
    unsigned int m_ILNativeMapCount;
    const COR_DEBUG_IL_TO_NATIVE_MAP* m_pILNativeMap;
};

#endif // _CLRJNCREADER_H_