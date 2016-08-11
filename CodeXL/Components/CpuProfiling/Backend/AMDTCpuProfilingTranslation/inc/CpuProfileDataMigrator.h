//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfileDataMigrator.h
/// \brief EBP file format to CXLDB file migration class.
///
//==================================================================================

#pragma once

#include <memory> // For unique_ptr
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTCpuProfilingRawData/inc/ProfilerDataDBWriter.h>
#include <AMDTCpuProfilingRawData/inc/RunInfo.h>


class DataMigrator
{
public:
    explicit DataMigrator(const gtString& inFileStr);
    explicit DataMigrator(const osFilePath& inFilePath);
    DataMigrator(const DataMigrator&) = delete;
    DataMigrator& operator=(const DataMigrator&) = delete;
    virtual ~DataMigrator() {}

    gtString GetSourceFilePath() const;
    gtString GetTargetFilePath() const;
    bool Migrate(bool deleteSrcFiles = false);

    const wchar_t* SourceFileExt = L"ebp";
    const wchar_t* TargetFileExt = L"cxlcpdb";

private:
    bool doMigrate();
    bool doDeleteSrcFiles();
    bool updateModuleInstanceInfo(NameModuleMap& moduleMap);

    bool WriteSessionInfoIntoDB(const CpuProfileInfo& profileInfo, const RunInfo& runInfo);
    bool WriteTopologyInfoIntoDB(const CoreTopologyMap& topMap);
    bool WriteSamplingEventInfoIntoDB(const EventEncodeVec& eventVec);
    bool WriteSamplingConfigInfoIntoDB(const EventEncodeVec& eventVec);
    bool WriteCoreSamplingConfigInfoIntoDB(const EventEncodeVec& eventVec, const RunInfo& runInfo);
    bool WriteProcessInfoIntoDB(const PidProcessMap& processMap);
    bool WriteThreadInfoIntoDB(const NameModuleMap& moduleMap);
    bool WriteModuleInfoIntoDB(const NameModuleMap& moduleMap);
    bool WriteModuleInstanceInfoIntoDB(const NameModuleMap& moduleMap);
    bool WriteFunctionInfoIntoDB(const NameModuleMap& moduleMap);
    bool WriteSampleProfileDataIntoDB(const NameModuleMap& modMap);
    // Uncomment this line, when add support for .css import
    //bool WriteCallgraphProfileDataIntoDB(const PidProcessMap& processMap, const NameModuleMap& moduleMap);
    bool WriteJitInfoIntoDB(const NameModuleMap& modMap);
    bool WriteFinish();

    osFilePath m_sourceFilePath;
    osFilePath m_targetFilePath;

    std::unique_ptr<ProfilerDataDBWriter> m_dbWriter;
};