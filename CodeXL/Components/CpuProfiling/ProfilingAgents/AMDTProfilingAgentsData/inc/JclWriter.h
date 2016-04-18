//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file JclWriter.h
/// \brief This file contains an interface to write JCL file for Java code profiling.
///
//==================================================================================

#ifndef _JCLWRITER_H_
#define _JCLWRITER_H_

#ifndef _WIN32
    #include <pthread.h>
#endif

#include "ProfilingAgentsDataDLLBuild.h"
#include "JclHeader.h"
#include <ProfilingAgents/Utils/CrtFile.h>

class AGENTDATA_API JclWriter
{
public:
    JclWriter(const wchar_t* pJclName, const wchar_t* pAppName, int pid, bool closeOnRecordWrite = false);
    ~JclWriter();

    bool Initialize();
    bool WriteLoadRecord(JitLoadRecord* pRecord);
    bool WriteUnloadRecord(JitUnloadRecord* pRecord);

protected:
    void WriteHeader();

protected:
    CrtFile  m_fileStream;
    int      m_pid;
    int      m_recordCount;
    wchar_t* m_pAppName; // Java or CLR app
    unsigned m_lenAppName;
    wchar_t  m_fileName[OS_MAX_PATH];

    //
    // During data-translation, CodeXL will copy all the JNC and  JCL files created by
    // CodeXL's java-profile-agent-library (which is running in Java JVM's process context)
    // from the TEMP folder to CodeXL-Profile-Session folder.
    //
    // Normally, CodeXL will copy those files only after JVM process is completed. But
    // If the JVM process is still running (and it has opened the JCL file) and the
    // profile duration is complete, then CodeXL will try to copy those JCL/JNC files.
    // While JVM has a handle to JCL file, Windows OS does not allow CodeXL to copy that file.
    // And hence, CodeXL won't be able to attribute samples to JIT'ed methods in various
    // scenarios like:
    //   1. Launch java through CodeXL, but stop profiling before java finishes
    //      without terminating java process.
    //   2. Launch java profile through command-line (by using -agentpath option) and
    //      either profile using "attach process" or "SWP"
    // To avoid this problem, JCLWriter can open the JCL file whenever it wants to write
    // a metadata record into the JCL file and close it immediately after that.
    //
    // Note: There is a performance penalty here.. but that seems negligible as CodeXL
    //       will end up writing only few hundred records at the maximum.
    //
    bool     m_closeOnRecordWrite;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    CRITICAL_SECTION  m_criticalSection;
#else
    pthread_mutex_t   m_criticalSection;
#endif
};

#endif // _JCLWRITER_H_
