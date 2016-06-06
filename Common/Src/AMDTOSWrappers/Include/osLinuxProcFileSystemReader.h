//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osLinuxProcFileSystemReader.h
///
//=====================================================================

//------------------------------ osLinuxProcFileSystemReader.h ------------------------------

#ifndef __OSLINUXPROCFILESYSTEMREADER_H
#define __OSLINUXPROCFILESYSTEMREADER_H

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTOSWrappers/Include/osOSWrappersDLLBuild.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osCPUSampledData.h>
#include <AMDTOSWrappers/Include/osPhysicalMemorySampledData.h>
#include <AMDTOSWrappers/Include/osSystemResourcesDataSampler.h>

// The size of a buffer that holds the currently parsed file content:
#define OS_PROC_FS_FILE_CONTENT_BUF_SIZE 4096

// The maximal amount of supported CPUs:
#define OS_SUPPORTED_CPUS_AMOUNT 32


// ----------------------------------------------------------------------------------
// Class Name:           osLinuxProcFileSystemReader
// General Description:
//   Responsible for reading data from the Linux /proc pseudo file system.
//
//   The proc file system is a pseudo files ystem which is used as an interface to
//   kernel data structures. It is commonly mounted at /proc. Most of it is read-only,
//   but some files allow kernel variables to be changed.
//
//   For more information about the /proc pseudo file system see "man 5 proc".
//
// Author:      AMD Developer Tools Team
// Creation Date:        12/4/2005
// ----------------------------------------------------------------------------------
class osLinuxProcFileSystemReader : public osSystemResourcesDataSampler
{
public:
    osLinuxProcFileSystemReader();
    virtual ~osLinuxProcFileSystemReader();

    bool getKernelVersion(int& majorVersion, int& minorVersion, int& revision);
    bool getVersionString(gtString& osVersion);

    virtual bool updateCPUsData();
    virtual int cpusAmount() const;
    virtual bool getGlobalCPUData(osCPUSampledData& cpuStatistics) const;
    virtual bool getCPUData(int cpuIndex, osCPUSampledData& cpuStatistics) const;
    bool getCPUtype(int cpuIndex, gtString& cpuType);
    bool getDomainName(gtString& domainName);

    virtual bool updatePhysicalMemoryData();
    virtual bool getPhysicalMemoryData(osPhysicalMemorySampledData& memoryStatistics) const;
    const osPhysicalMemorySampledData& physicalMemoryData() const;

private:
    bool readFileIntoContentBuffer(const char* fileFullPath, char* pBuff, int buffLength);
    bool updateKernelVersion();
    bool updatePerCPUStatisticalData(const char*& pCurrPos);
    bool readCPUStatistics(const char*& pCurrPos, osCPUSampledData& cpuStatistics);
    bool readMemoryValue(const char* pStr, gtUInt64& value) const;
    const char* findNextToken(const char* pCurrPos);
    const char* findNextLine(const char* pCurrPos);

private:
    // Contains true iff
    bool _isLinuxOS;

    // Holds the Linux kernel version:
    int _kernelMajorVersion;
    int _kernelMinorVersion;
    int _kernelRevision;

    // Contains true iff the kernel version is 2.6 or higher:
    bool _is26KernelOrHigher;

    // Holds memory related data:
    osPhysicalMemorySampledData _physicalMemoryData;

    // Holds global CPU data:
    osCPUSampledData _globalCPUData;

    // Holds the CPUs amount:
    // (1 if we don't have "per CPU" data)
    int _cpusAmount;

    // Holds per CPU data:
    osCPUSampledData _cpuData[OS_SUPPORTED_CPUS_AMOUNT];

    // Buffer that holds the currently read file content:
    char _readFileContentBuff[OS_PROC_FS_FILE_CONTENT_BUF_SIZE];
};


#endif //__OSLINUXPROCFILESYSTEMREADER_H

