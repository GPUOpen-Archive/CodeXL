//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CelWriter.h
///
//==================================================================================

#ifndef _CELWRITER_H_
#define _CELWRITER_H_

#include "../ProfilingAgentsDataDLLBuild.h"
#include "CelHeader.h"
#include <ProfilingAgents/Utils/CrtFile.h>

class AGENTDATA_API CelWriter
{
public:
    CelWriter();
    ~CelWriter();
    bool Initialize(const wchar_t* pCelFileName, int bitness = 0);
    void WriteAppDomainCreationFinished(AppDomainID appDomainId, gtUInt64 systime,
                                        unsigned int cSize, const wchar_t* pAppDomainName);
    void WriteAppDomainShutdownStarted(AppDomainID appDomainId, gtUInt64 systime);

    void WriteAssemblyLoadFinished(AssemblyID assemblyId, AppDomainID appDomainId,
                                   gtUInt64 systime, unsigned int cSize, const wchar_t* pAssemblyName);
    void WriteAssemblyUnloadStarted(AssemblyID assemblyId, gtUInt64 systime);

    void WriteModuleLoadFinished(ModuleID moduleId, gtUInt64 systime, unsigned int cSize, const wchar_t* pModuleName);
    void WriteModuleUnloadStarted(ModuleID moduleId, gtUInt64 systime);

    void WriteModuleAttachedToAssembly(ModuleID moduleId, AssemblyID assemlbyId,
                                       gtUInt64 assemblyLoadAddr, const wchar_t* pAssemblyFilePath);

    void WriteClassLoadFinished(ClassID classId, ModuleID moduleId, gtUInt64 systime, unsigned int cSize, const wchar_t* pClassName);
    void WriteClassUnloadStarted(ClassID classId, gtUInt64 systime);

    void WriteJITCompilationFinished(ModuleID moduleId, FunctionID funcId,
                                     const wchar_t* pClassName, const wchar_t* pFuncName, const wchar_t* pJncFileName,
                                     gtUInt64 systime, LPCBYTE startAddr, unsigned int codeSize);
    void ReWriteJITCompilation(ModuleID moduleId, FunctionID funcId,
                               const wchar_t* pClassName, const wchar_t* pFuncName, const wchar_t* pJncFileName,
                               gtUInt64 systime, gtUInt64 startAddr, unsigned int codeSize);
    void WriteFunctionUnloadStarted(FunctionID funcId, gtUInt64 systime);

protected:
    void WriteHeader(int bitness);
    void WriteString(const wchar_t* pString);

    CrtFile m_fileStream;
    gtUInt32 m_entryCount;
};

#endif // _CELWRITER_H_
