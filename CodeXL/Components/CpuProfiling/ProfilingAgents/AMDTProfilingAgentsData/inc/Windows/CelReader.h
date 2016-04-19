//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CelReader.h
///
//==================================================================================

#ifndef _CELREADER_H_
#define _CELREADER_H_

#include "../ProfilingAgentsDataDLLBuild.h"
#include "CelHeader.h"
#include <ProfilingAgents/Utils/CrtFile.h>

class AGENTDATA_API CelReader
{
public:
    CelReader();
    ~CelReader();
    bool Open(const wchar_t* pCelFileName);
    void Close();

    // Get number records in the CEL file.
    // NOTE: it could be zero if the CEL file is still written by
    //          CELWriter in a CLR profile Agent.
    gtUInt32 GetNumRecord() const;

    // Get record type. Base on record type, then call the followed GetXXXRecord()
    CLREventType GetNextRecordType();

    //Get AppDomainCreation record which only fill id, loadtime, name
    HRESULT GetAppDomainCreationRecord(AppDomainRecord* pDomainRec);

    // Get appDomain unload time.
    HRESULT GetAppDomainShutdownRecord(AppDomainID* pAppDomainId, gtUInt64* pSystime);

    // Get AssemblyLoadRecord which returns assemblyid, app id, load time, name, etc.
    HRESULT GetAssemblyLoadRecord(AssemblyRecord* pAsmRec);

    // Get assembly unload time.
    HRESULT GetAssemblyUnloadRecord(AssemblyID* pAssemblyId, gtUInt64* pSystime);

    // module id, load time, name,
    HRESULT GetModuleLoadRecord(ModuleRecord* pModRec);

    // module unload time,
    HRESULT GetModuleUnloadRecord(ModuleID* pModId, gtUInt64* pSystime);

    // module id and assemblyid
    HRESULT GetModuleAttachedToAssemblyRec(ModuleID* pModuleId, AssemblyID* pAssemblyId,
                                           gtUInt64* pAssemblyLoadAddr, wchar_t* pAssemlbyFullName, unsigned int size);

    // class id, module id, load time, name
    HRESULT GetClassLoadRecord(ClassRecord* pClassRec);

    // class unload time,
    HRESULT GetClassUnloadRec(ClassID* pClassId, gtUInt64* pSystime);

    //function id, class name, function name, jncfilename, load time, load address, size
    HRESULT GetJITCompilationFinished(FunctionRecord* pFuncRec);

    //function id, unload time.
    HRESULT GetFunctionUnloadStarted(FunctionID* pFuncId, gtUInt64* pSystime);

    // is 32-bit
    bool Is32BitProcess() const;

protected:
    void ReadHeader();
    void ReadString(wchar_t* pString);

    CrtFile         m_fileStream;
    CLREventType    m_curEventType;
    CelHeader       m_celHeader;
};

#endif // _CELREADER_H_
