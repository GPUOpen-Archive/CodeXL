//==================================================================================
// Copyright (c) 2014-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file Report.cpp
/// \brief This is Command Line Utility for CPU profiling.
///
//==================================================================================

// Qt
#include <QtGui>

// Backend:
#include <AMDTCpuPerfEventUtils/inc/ViewConfig.h>
#include <AMDTCpuPerfEventUtils/inc/EventEngine.h>
#include <AMDTCpuProfilingTranslation/inc/CpuProfileDataTranslation.h>

// Infra:
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osMachine.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtVector.h>

// Project:
#include <Report.h>
#include <CommonUtils.h>

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #include <time.h>
#endif

#define TP_ALL_PIDS                0xFFFFFFFFUL

// Backend:

// External function.
extern bool reportError(bool appendDriverError, const wchar_t* pFormatString, ...);


HRESULT CpuProfileReport::Initialize()
{
    // Setup the Environment
    SetupEnvironment();

    // Validate Profile Type and other options
    ValidateOptions();

    // Initialize the Events XML file access
    if (isOK() && (!InitializeEventsXMLFile(m_eventsFile)))
    {
        reportError(false, L"Failed to initialize Events XML file.\n");
        m_error = E_FAIL;
    }

    return m_error;
}

bool CpuProfileReport::IsRawInputFile()
{
    return m_isRawInputFile;
}

HRESULT CpuProfileReport::Translate()
{
    HRESULT hr = S_OK;

    if (isOK() && IsRawInputFile())
    {
        // Debug search path
        gtString searchPath = m_args.GetDebugSymbolPath();
        gtString serverList = m_args.GetSymbolServerPath();
        gtString cachePath = m_args.GetSymbolCachePath();

        const wchar_t* pSearchPath = (!searchPath.isEmpty()) ? searchPath.asCharArray() : nullptr;
        const wchar_t* pServerList = (!serverList.isEmpty()) ? serverList.asCharArray() : nullptr;
        const wchar_t* pCachePath = (nullptr != pServerList && !cachePath.isEmpty()) ? cachePath.asCharArray() : nullptr;

        // TODO: Initialize Symbol engine - AuxInitializeSymbolEngine

        gtString inputFilePath = m_args.GetInputFile();
        ReaderHandle* pHandle = nullptr;
        hr = fnOpenProfile(inputFilePath.asCharArray(), &pHandle);

        if (!SUCCEEDED(hr))
        {
            reportError(false, L"The raw profile (%ls) open failed. (error code 0x%lx).\n",
                        inputFilePath.asCharArray(), hr);
            return hr;
        }

        osFilePath outputFilePath = GetEBPFilePath();

        osDirectory outDir;
        outputFilePath.getFileDirectory(outDir);

        if (! outDir.exists())
        {
            if (!outDir.create())
            {
                reportError(false, L"Failed to create output directory " STR_FORMAT L".\n", outputFilePath.asString().asCharArray());
            }
        }

        hr = fnWriteSetToFile(pHandle,
                              nullptr,
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                              outputFilePath.asString().asCharArray(),
#else
                              outputFilePath.fileDirectoryAsString().asCharArray(),
#endif
                              nullptr,
                              pSearchPath,
                              pServerList,
                              pCachePath);

        if (S_OK != hr)
        {
            gtString msg;
            gtString additional;

            if ((int)E_NODATA == hr)
            {
                additional = L"There were no data records, was the profiling paused the entire duration?";
            }
            else if ((int)E_INVALIDDATA == hr)
            {
                additional = L"The raw data file may be corrupted.";
            }

            msg.appendFormattedString(L"Could not write the profile data file (%ls).\n%ls\n",
                                      outputFilePath.asString().asCharArray(), additional.asCharArray());

            reportError(false, STR_FORMAT, msg.asCharArray());
        }

        if (S_OK != fnCloseProfile(&pHandle))
        {
            reportError(false, L"Failed to close the profile file.\n");
            hr = E_FAIL;
        }
    }

    return hr;
} // Translate


HRESULT CpuProfileReport::Report()
{
    HRESULT hr = S_OK;

    // If DB input file - -i has .cxldb file
    if (m_isDbInputFile)
    {
        return ReportFromDb();
    }

    // Initialize the report file
    bool retVal = InitializeReportFile();

    if (retVal)
    {
        // Input file - Processed profile data file
        osFilePath ebpFilePath = GetEBPFilePath();

        m_profileReader.open(ebpFilePath.asString().asCharArray());

#if AMDT_BUILD_TARGET != AMDT_WINDOWS_OS

        // Disable CLU on Linux
        if (m_profileReader.getProfileInfo()->m_isProfilingCLU)
        {
            reportError(false, L"CLU reporting is not yet supported!\n");
            return E_FAIL;
        }

#endif

        if (!m_args.GetSectionsToReport().isEmpty()
            && IsReportCallGraph()
            && !m_profileReader.getProfileInfo()->m_isCSSEnabled)
        {
            reportError(false, L"Callgraph details are not available in the profile data file - " STR_FORMAT L".\n",
                        ebpFilePath.asString().asCharArray());
        }

        // Once you have the profile reader, initialize the cores-list
        // and Obtain the profiled event's list
        InitCoresList();
        retVal = InitProfileEventsNameVec();

        if (retVal)
        {
            // Initialize the output profile data view
            InitializeViewData();

            ReportExecution();
            ReportProfileDetails();

            ReportOverviewData();
            ReportProcessData();
            ReportImixData();
        }
    }

    return hr = (retVal) ? S_OK : E_FAIL;
} // Report

#ifdef AMDT_ENABLE_DB_SUPPORT

#define STR_FORMAT L"%ls"
#define DQ_STR_FORMAT L"\"%ls\""


static void PrintSessionInfo(osFile& reportFile, AMDTProfileSessionInfo& sessionInfo)
{
    gtString s;

    s << L"CodeXL CPU Profiler - PROFILE SESSION INFO\n";
    //s.appendFormattedString(L"Target Machine," STR_FORMAT L"\n", sessionInfo.m_targetMachineName.asASCIICharArray());
    s.appendFormattedString(L"Target Application," STR_FORMAT L"\n", sessionInfo.m_targetAppPath.asCharArray());
    s.appendFormattedString(L"System Details," STR_FORMAT L"\n", sessionInfo.m_systemDetails.asCharArray());

    s << L"\n";
    reportFile.writeString(s);

    return;
}

static void PrintSampledCounters(osFile& reportFile, AMDTProfileCounterDescVec& counters, AMDTProfileSamplingConfigVec& samplingConfig)
{
    gtString s;

    s << L"Sampled Counters\n";

    s.appendFormattedString(L"Counter Name, Sampling Interval, USer, Os\n");

    int idx = 0;
    for (auto& counter : counters)
    {
        s.appendFormattedString(STR_FORMAT L", %lu, %d, %d\n",
            counter.m_name.asCharArray(), samplingConfig[idx].m_samplingInterval, samplingConfig[idx].m_userMode, samplingConfig[idx].m_osMode);

        idx++;
    }

    s << L"\n";
    reportFile.writeString(s);

    return;
}

static void PrintOverview(osFile& reportFile, gtString hdr, AMDTProfileCounterDesc& counterDesc, AMDTProfileDataVec& data)
{
    gtString s;

    s << hdr;
    s << L" OVERVIEW (Counter - ";
    s << counterDesc.m_name;
    s << L")\n";
    s.appendFormattedString(L"ID, Name, SampleCount, SamplePercentage\n");

    for (auto& aData : data)
    {
        s.appendFormattedString(L"%d, ", aData.m_id);
        s << aData.m_name;

        for (auto& aSampleValue : aData.m_sampleValue)
        {
            if (counterDesc.m_type == AMDT_PROFILE_COUNTER_TYPE_RAW)
            {
                s.appendFormattedString(L",%lu", static_cast<gtUInt64>(aSampleValue.m_sampleCount));
                s.appendFormattedString(L",%3.02f%%", static_cast<float>(aSampleValue.m_sampleCountPercentage));
            }
            else if (counterDesc.m_type == AMDT_PROFILE_COUNTER_TYPE_COMPUTED)
            {
                s.appendFormattedString(L",%5.04f", static_cast<float>(aSampleValue.m_sampleCount));
            }
        }
        s << L"\n";
    }

    s << L"\n";
    reportFile.writeString(s);

    return;
}

static void PrintAllData(osFile& reportFile, gtString hdr, AMDTProfileCounterDescVec& counterDescVec, AMDTProfileDataVec& data)
{
    gtString s;

    s << L" All Data - ";
    s << hdr;
    s << L"\n";

    s.appendFormattedString(L"ID, Name ");

    for (auto& aCounter : counterDescVec)
    {
        if (aCounter.m_type == AMDT_PROFILE_COUNTER_TYPE_RAW)
        {
            s << L"," << aCounter.m_name;
            //s << L"," << aCounter.m_name << L"-SampleCount";
            //s << L"," << aCounter.m_name << L"-SamplePercentage";
        }
        else if (aCounter.m_type == AMDT_PROFILE_COUNTER_TYPE_COMPUTED)
        {
            s << L"," << aCounter.m_name;
        }
    }
    s << L"\n";

    for (auto& aData : data)
    {
        s.appendFormattedString(L"%d, ", aData.m_id);
        s << aData.m_name;

        int idx = 0;
        for (auto& aSampleValue : aData.m_sampleValue)
        {
            if (counterDescVec[idx].m_type == AMDT_PROFILE_COUNTER_TYPE_RAW)
            {
                s.appendFormattedString(L",%lu ", static_cast<gtUInt64>(aSampleValue.m_sampleCount));
                s.appendFormattedString(L"(%3.02f%%)", static_cast<float>(aSampleValue.m_sampleCountPercentage));

                //s.appendFormattedString(L",%lu", static_cast<gtUInt64>(aSampleValue.m_sampleCount));
                //s.appendFormattedString(L",%3.02f%%", static_cast<float>(aSampleValue.m_sampleCountPercentage));
            }
            else if (counterDescVec[idx].m_type == AMDT_PROFILE_COUNTER_TYPE_COMPUTED)
            {
                s.appendFormattedString(L",%5.04f", static_cast<float>(aSampleValue.m_sampleCount));
            }

            idx++;
        }
        s << L"\n";
    }

    s << L"\n";
    reportFile.writeString(s);

    return;
}

static void GetInstOffsets(gtUInt16 srcLine, AMDTSourceAndDisasmInfoVec& srcInfoVec, gtVector<gtVAddr>& instOffsetVec)
{
    for (auto& srcInfo : srcInfoVec)
    {
        if (srcInfo.m_sourceLine == srcLine)
        {
            instOffsetVec.push_back(srcInfo.m_offset);
        }

    }

    return;
}

static void GetDisasmString(gtVAddr offset, AMDTSourceAndDisasmInfoVec& srcInfoVec, gtString& disasm, gtString& codeByte)
{
    auto instData = std::find_if(srcInfoVec.begin(), srcInfoVec.end(),
        [&offset](AMDTSourceAndDisasmInfo const& srcInfo) { return srcInfo.m_offset == offset; });

    if (instData != srcInfoVec.end())
    {
        disasm = instData->m_disasmStr;
        codeByte = instData->m_codeByteStr;
    }

    return;
}

static void GetDisasmSampleValue(gtVAddr offset, AMDTProfileInstructionDataVec& dataVec, AMDTSampleValueVec& sampleValue)
{
    auto instData = std::find_if(dataVec.begin(), dataVec.end(),
        [&offset](AMDTProfileInstructionData const& data) { return data.m_offset == offset; });

    if (instData != dataVec.end())
    {
        sampleValue = instData->m_sampleValues;
    }

    return;
}

static bool GetSourceLines(const gtString& filePath, gtVector<gtString>& srcLines)
{
    unsigned int count = 0;
    QFile file(convertToQString(filePath));

    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QTextStream srcStream(&file);
    srcStream.setCodec("UTF-8");

    while (!srcStream.atEnd())
    {
        srcLines.emplace_back(convertToGTString(srcStream.readLine()));
        count++;
    }

    file.close();
    return true;
}


static void PrintFunctionDetailData(osFile& reportFile, AMDTProfileCounterDescVec& counterDescVec,
    AMDTProfileFunctionData& data,
    gtString& srcFilePath,
    AMDTSourceAndDisasmInfoVec& srcInfoVec)
{
    gtString s;

    s << L" Function Detail Data\n";

    s.appendFormattedString(L"Name,SrcLine,Offset,Disassembly,CodeByte ");

    for (auto& aCounter : counterDescVec)
    {
        if (aCounter.m_type == AMDT_PROFILE_COUNTER_TYPE_RAW)
        {
            s << L"," << aCounter.m_name;
            //s << L"," << aCounter.m_name << L"-SampleCount";
            //s << L"," << aCounter.m_name << L"-SamplePercentage";
        }
        else if (aCounter.m_type == AMDT_PROFILE_COUNTER_TYPE_COMPUTED)
        {
            s << L"," << aCounter.m_name;
        }
    }
    s << L"\n";

    // s.appendFormattedString(L"%d, ", data.m_functionInfo.m_functionId);
    s << data.m_functionInfo.m_name << "  (" << srcFilePath << ")  ";
    s << L"\n";

    gtVector<gtString> srcLines;

    bool foundSrcLine = GetSourceLines(srcFilePath, srcLines);

    for (auto& srcData : data.m_srcLineDataList)
    {
        //s.appendFormattedString(L",%d,,,", srcData.m_sourceLineNumber);
        s.appendFormattedString(L"%d,", srcData.m_sourceLineNumber);

        if (foundSrcLine)
        {
            s.appendFormattedString(DQ_STR_FORMAT, srcLines[srcData.m_sourceLineNumber - 1].asCharArray());
            s.appendFormattedString(L",,,", srcData.m_sourceLineNumber);
        }
        else
        {
            s.appendFormattedString(L",,,,", srcData.m_sourceLineNumber);
        }

        int idx = 0;
        for (auto& aSampleValue : srcData.m_sampleValues)
        {
            if (counterDescVec[idx].m_type == AMDT_PROFILE_COUNTER_TYPE_RAW)
            {
                s.appendFormattedString(L",%lu ", static_cast<gtUInt64>(aSampleValue.m_sampleCount));
                s.appendFormattedString(L"(%3.02f%%)", static_cast<float>(aSampleValue.m_sampleCountPercentage));

                //s.appendFormattedString(L",%lu", static_cast<gtUInt64>(aSampleValue.m_sampleCount));
                //s.appendFormattedString(L",%3.02f%%", static_cast<float>(aSampleValue.m_sampleCountPercentage));
            }
            else if (counterDescVec[idx].m_type == AMDT_PROFILE_COUNTER_TYPE_COMPUTED)
            {
                s.appendFormattedString(L",%5.04f", static_cast<float>(aSampleValue.m_sampleCount));
            }

            idx++;
        }
        s << L"\n";

        // For this srcLine get the list of inst offsets..
        gtVector<gtVAddr> instOffsetVec;
        GetInstOffsets(srcData.m_sourceLineNumber, srcInfoVec, instOffsetVec);

        // For this offset get the disams and samples values
        for (auto& instOffset : instOffsetVec)
        {
            gtString disasm;
            gtString codeByte;

            GetDisasmString(instOffset, srcInfoVec, disasm, codeByte);
            AMDTSampleValueVec sampleValue;
            GetDisasmSampleValue(instOffset, data.m_instDataList, sampleValue);

            s.appendFormattedString(L",,0x%lx,", instOffset);
            s.appendFormattedString(DQ_STR_FORMAT, disasm.asCharArray());
            s.appendFormattedString(L"," DQ_STR_FORMAT, codeByte.asCharArray());

            idx = 0;
            for (auto& aSampleValue : sampleValue)
            {
                if (counterDescVec[idx].m_type == AMDT_PROFILE_COUNTER_TYPE_RAW)
                {
                    s.appendFormattedString(L",%lu ", static_cast<gtUInt64>(aSampleValue.m_sampleCount));
                    s.appendFormattedString(L"(%3.02f%%)", static_cast<float>(aSampleValue.m_sampleCountPercentage));
                }
                else if (counterDescVec[idx].m_type == AMDT_PROFILE_COUNTER_TYPE_COMPUTED)
                {
                    s.appendFormattedString(L",%5.04f", static_cast<float>(aSampleValue.m_sampleCount));
                }

                idx++;
            }
            s << L"\n";
        }

        s << L"\n";
    }

    s << L"\n";
    reportFile.writeString(s);

    return;
}

static void PrintCGFunctionsHdr(osFile& reportFile)
{
    gtString s;
    s << L" CallGraph Functions\n";
    s.appendFormattedString(L"Function, Self Samples, Deep Samples");
    s << L"\n";
    reportFile.writeString(s);

    return;
}

static void PrintCGFunctions(osFile& reportFile, AMDTCallGraphFunctionVec& funcs)
{
    gtString s;

    for (auto& p : funcs)
    {
        s << p.m_functionInfo.m_name;
        s.appendFormattedString(L",%5.04f", static_cast<float>(p.m_totalSelfSamples));
        s.appendFormattedString(L",%5.04f\n", static_cast<float>(p.m_totalDeepSamples));
    }

    s << L"\n";
    reportFile.writeString(s);

    return;
}

static void PrintCGHdr(osFile& reportFile)
{
    gtString s;
    s << L" CallGraph\n";
    s.appendFormattedString(L"Self, Parent/Children, SampleCount");
    s << L"\n";
    reportFile.writeString(s);

    return;
}

static void PrintCGInfo(osFile& reportFile, AMDTCallGraphFunction& self, AMDTCallGraphFunctionVec& parent, AMDTCallGraphFunctionVec& children)
{
    gtString s;

    for (auto& p : parent)
    {
        s << ",";
        s << p.m_functionInfo.m_name;
        s.appendFormattedString(L",%5.04f\n", static_cast<float>(p.m_totalDeepSamples));
    }

    s << self.m_functionInfo.m_name;
    s.appendFormattedString(L",,%5.04f\n", static_cast<float>(self.m_totalSelfSamples));

    for (auto& c : children)
    {
        s << ",";
        s << c.m_functionInfo.m_name;
        s.appendFormattedString(L",%5.04f\n", static_cast<float>(c.m_totalDeepSamples));
    }

    s << L"\n";
    reportFile.writeString(s);

    return;
}


//static void PrintCGPath(gtVector<AMDTCallGraphPath>&  paths)
//{
//    (void)(paths);
//    return;
//}

#endif // AMDT_ENABLE_DB_SUPPORT

HRESULT CpuProfileReport::ReportFromDb()
{
#ifndef AMDT_ENABLE_DB_SUPPORT

    fprintf(stderr, " Report from DB is not yet supported.\n");
    return E_FAIL;

#else

    bool ret = false;

    cxlProfileDataReader profileDbReader;

    // if -i has .cxldb file
    if (m_isDbInputFile)
    {
        osFilePath reportFilePath = GetOutputFilePath();
        reportFilePath.setFileExtension(CPUPROFILE_CSV_REPORT_EXTENSION);
        osFile reportFile;

        if (!reportFilePath.isEmpty())
        {
            ret = reportFile.open(reportFilePath, osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);
        }

        ret = profileDbReader.OpenProfileData(m_inputFilePath.asString());

        if (ret)
        {
            AMDTProfileSessionInfo sessionInfo;

            ret = profileDbReader.GetProfileSessionInfo(sessionInfo);
            PrintSessionInfo(reportFile, sessionInfo);

            AMDTCpuTopologyVec topologyVec;
            ret = profileDbReader.GetCpuTopology(topologyVec);

            gtVector<AMDTProfileCounterDesc> counterDesc;
            ret = profileDbReader.GetSampledCountersList(counterDesc);

            AMDTProfileSamplingConfigVec samplingConfigVec;
            for (auto const& counter : counterDesc)
            {
                AMDTProfileSamplingConfig sampleConfig;
                ret = profileDbReader.GetSamplingConfiguration(counter.m_id, sampleConfig);
                samplingConfigVec.push_back(sampleConfig);
            }
            PrintSampledCounters(reportFile, counterDesc, samplingConfigVec);

            gtVector<AMDTProfileReportConfig> reportConfigs;
            ret = profileDbReader.GetReportConfigurations(reportConfigs);

            AMDTProfileDataOptions options;
            options.m_coreMask = AMDT_PROFILE_ALL_CORES;
            options.m_doSort = true;
            options.m_summaryCount = 5;
            options.m_isSeperateByCore = false;

            //for (auto const& counter : counterDesc)
            for (auto const& counter : reportConfigs[0].m_counterDescs)
            {
                options.m_counters.push_back(counter.m_id);
            }

            ret = profileDbReader.SetReportOption(options);

            gtVector<AMDTProfileProcessInfo> procInfo;
            ret = profileDbReader.GetProcessInfo(AMDT_PROFILE_ALL_PROCESSES, procInfo);
            for (auto const& process : procInfo)
            {
                fprintf(stderr, "PID : %d\n", process.m_pid);
            }

            gtVector<AMDTProfileModuleInfo> modInfo;
            ret = profileDbReader.GetModuleInfo(AMDT_PROFILE_ALL_PROCESSES, AMDT_PROFILE_ALL_MODULES, modInfo);
            for (auto const& module : modInfo)
            {
                fprintf(stderr, "Module : %d\n", module.m_moduleId);
            }

            gtVector<AMDTProfileThreadInfo> threadInfo;
            ret = profileDbReader.GetThreadInfo(AMDT_PROFILE_ALL_PROCESSES, AMDT_PROFILE_ALL_THREADS, threadInfo);
            for (auto const& thread : threadInfo)
            {
                fprintf(stderr, "Thread : %d\n", thread.m_threadId);
            }

            AMDTProfileDataVec funcProfileData;
            AMDTProfileDataVec processProfileData;
            AMDTProfileDataVec moduleProfileData;
            AMDTProfileDataVec threadProfileData;
            for (auto& counter : counterDesc)
            {
                processProfileData.clear();
                ret = profileDbReader.GetProcessSummary(counter.m_id, processProfileData);
                PrintOverview(reportFile, L"Process", counter, processProfileData);

                moduleProfileData.clear();
                ret = profileDbReader.GetModuleSummary(counter.m_id, moduleProfileData);
                PrintOverview(reportFile, L"Module", counter, moduleProfileData);

                threadProfileData.clear();
                ret = profileDbReader.GetThreadSummary(counter.m_id, threadProfileData);
                PrintOverview(reportFile, L"Thread", counter, threadProfileData);

                funcProfileData.clear();
                ret = profileDbReader.GetFunctionSummary(counter.m_id, funcProfileData);
                PrintOverview(reportFile, L"Function", counter, funcProfileData);
            }

            // Process View - "All Data" view
            gtVector<AMDTProfileData> allProcessData;
            ret = profileDbReader.GetProcessProfileData(AMDT_PROFILE_ALL_PROCESSES, AMDT_PROFILE_ALL_MODULES, allProcessData);
            PrintAllData(reportFile, L"Process", reportConfigs[0].m_counterDescs, allProcessData);

            // Module View - "All Data" view
            gtVector<AMDTProfileData> allModuleData;
            ret = profileDbReader.GetModuleProfileData(AMDT_PROFILE_ALL_PROCESSES, AMDT_PROFILE_ALL_MODULES, allModuleData);
            PrintAllData(reportFile, L"Module", reportConfigs[0].m_counterDescs, allModuleData);

            // function View - "All Data" view
            gtVector<AMDTProfileData> allFunctionData;
            ret = profileDbReader.GetFunctionProfileData(AMDT_PROFILE_ALL_PROCESSES, AMDT_PROFILE_ALL_MODULES, allFunctionData);
            PrintAllData(reportFile, L"Function", reportConfigs[0].m_counterDescs, allFunctionData);

            // Get modules data for the process
            for (auto const& process : procInfo)
            {
                gtVector<AMDTProfileData> profData;

                ret = profileDbReader.GetModuleProfileData(process.m_pid, AMDT_PROFILE_ALL_MODULES, profData);
                gtString hdr(L"Modules for Process -");
                hdr.appendFormattedString(L"%d", process.m_pid);
                PrintAllData(reportFile, hdr, reportConfigs[0].m_counterDescs, profData);
            }

            // Get processes data for the module
            for (auto const& mod : modInfo)
            {
                gtVector<AMDTProfileData> profData;

                ret = profileDbReader.GetProcessProfileData(AMDT_PROFILE_ALL_PROCESSES, mod.m_moduleId, profData);
                gtString hdr(L"Processes for Module -");
                hdr.appendFormattedString(L"%d", mod.m_moduleId);
                PrintAllData(reportFile, hdr, reportConfigs[0].m_counterDescs, profData);
            }

            // Get detailed function profiledata
            // FIXME: dont report func data..
            funcProfileData.clear();

            for (auto const& func : funcProfileData)
            {
                fprintf(stderr, "%s \n", func.m_name.asASCIICharArray());

                AMDTProfileFunctionData  functionData;
                ret = profileDbReader.GetFunctionDetailedProfileData(func.m_id,
                                                                     AMDT_PROFILE_ALL_PROCESSES,
                                                                     AMDT_PROFILE_ALL_THREADS,
                                                                     functionData);

                // if function size is zero, compute the size from instruction data.. 
                //gtUInt32 functionSize = functionData.m_functionInfo.m_size;
                //gtUInt32 startOffset = functionData.m_functionInfo.m_startOffset;
                //gtUInt32 nbrInsts = functionData.m_instDataList.size();

                //if (functionSize == 0 && nbrInsts)
                //{
                //    gtUInt32 instStartOffset = functionData.m_instDataList[0].m_offset;

                //    startOffset = (instStartOffset < startOffset) ? instStartOffset : startOffset;

                //    functionSize = functionData.m_instDataList[nbrInsts - 1].m_offset - instStartOffset;
                //}

                gtString srcFilePath;
                AMDTSourceAndDisasmInfoVec srcInfoVec;
                ret = profileDbReader.GetFunctionSourceAndDisasmInfo(func.m_id, srcFilePath, srcInfoVec);

                PrintFunctionDetailData(reportFile, reportConfigs[0].m_counterDescs, functionData, srcFilePath, srcInfoVec);
            }

            for (auto const& process : procInfo)
            {
                AMDTCounterId counterId = counterDesc[0].m_id;
                AMDTCallGraphFunctionVec cgFuncs;

                // CG Functions
                PrintCGFunctionsHdr(reportFile);
                ret = profileDbReader.GetCallGraphFunctions(process.m_pid, counterId, cgFuncs);
                PrintCGFunctions(reportFile, cgFuncs);

                PrintCGHdr(reportFile);

                // CG Parents/Children
                for (auto& cgFunc : cgFuncs)
                {
                    AMDTCallGraphFunctionVec parents;
                    AMDTCallGraphFunctionVec children;

                    ret = profileDbReader.GetCallGraphFunctionInfo(process.m_pid, cgFunc.m_functionInfo.m_functionId, parents, children);
                    PrintCGInfo(reportFile, cgFunc, parents, children);

                    //gtVector<AMDTCallGraphPath> paths;
                    //ret = profileDbReader.GetCallGraphPaths(process.m_pid, cgFunc.m_functionInfo.m_functionId, paths);
                    //PrintCGPath(paths);
                }
            }

            // close the db
            ret = profileDbReader.CloseProfileData();
        }
    }

    return S_OK;
#endif // AMDT_ENABLE_DB_SUPPORT
} // ReportFromDb

static AMDTResult printThreadSummary(AMDTThreadProfileDataHandle& tpReaderHandle, AMDTThreadId tid, osFile& reportFile, bool reportHdr);
static AMDTResult printThreadSampleData(AMDTThreadProfileDataHandle& tpReaderHandle, AMDTThreadId tid, osFile& reportFile);

static gtString printTS(AMDTUInt64 timeStamp)
{
    // Time stamp for when the event occurred.
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    FILETIME ft;
    LARGE_INTEGER ts;
    SYSTEMTIME st;
    SYSTEMTIME stLocal;
    AMDTUInt64 nanoSeconds;

    ts.QuadPart = timeStamp;
    ft.dwHighDateTime = ts.HighPart;
    ft.dwLowDateTime = ts.LowPart;

    FileTimeToSystemTime(&ft, &st);
    SystemTimeToTzSpecificLocalTime(nullptr, &st, &stLocal);

    nanoSeconds = (ts.QuadPart % 10000000) * 100;

    gtString tsStr;
    tsStr.appendFormattedString(L" %02d:%02d:%02d.%I64u , ", stLocal.wHour, stLocal.wMinute, stLocal.wSecond, nanoSeconds);
    return tsStr;
#else
    gtString tsStr;
    // tsStr.appendFormattedString(L" %016lu , ", timeStamp);

    struct timeval ts;
    struct tm time;

    ts.tv_sec = timeStamp / 1000000000;
    ts.tv_usec = (timeStamp - (ts.tv_sec * 1000000000)) / 1000;

    tzset();
    localtime_r(&(ts.tv_sec), &time);
    tsStr.appendFormattedString(L" %d:%d:%d:%06lu , ", time.tm_hour, time.tm_min, time.tm_sec, ts.tv_usec);
    return tsStr;
#endif // WINDOWS
}

static AMDTResult printAllProcessesSummary(AMDTThreadProfileDataHandle& tpReaderHandle, osFile& reportFile)
{
    AMDTResult hr = AMDT_STATUS_OK;
    AMDTUInt32 nbrProcesses = 0;
    AMDTProcessId* pProcesses = nullptr;
    AMDTUInt32 nbrProcessors = 0;

    hr = AMDTGetNumOfProcessors(tpReaderHandle, &nbrProcessors);
    gtString s(L"Number of Processors : ");
    s.appendFormattedString(L" %u \n", nbrProcessors);
    reportFile.writeString(s);

    hr = AMDTGetProcessIds(tpReaderHandle, &nbrProcesses, 0, nullptr);

    if (nbrProcesses > 0)
    {
        pProcesses = (AMDTProcessId*)malloc(sizeof(AMDTProcessId) * nbrProcesses);

        hr = AMDTGetProcessIds(tpReaderHandle, nullptr, nbrProcesses, pProcesses);
    }

    reportFile.writeString(L"\nPROCESSES :\n");
    reportFile.writeString(L"\nPID , ImageName");

    for (AMDTUInt32 i = 0; i < nbrProcesses; i++)
    {
        gtString data(L"\n");
        data.appendFormattedString(L" %lu , ", pProcesses[i]);

        AMDTProcessData procData;
        hr = AMDTGetProcessData(tpReaderHandle, pProcesses[i], &procData);

        gtString image;
        data.append(image.fromASCIIString(procData.m_pCommand));
        reportFile.writeString(data);
    }

    reportFile.writeString(L"\n");

    if (nullptr != pProcesses)
    {
        free(pProcesses);
    }

    return hr;
}

static AMDTResult printProcessSampleData(AMDTThreadProfileDataHandle& tpReaderHandle, AMDTProcessId pid, osFile& reportFile)
{
    AMDTResult hr = AMDT_STATUS_OK;
    AMDTUInt32 nbrThreads = 0;
    AMDTThreadId* pThreads = nullptr;

    hr = AMDTGetThreadIds(tpReaderHandle, pid, &nbrThreads, 0, nullptr);

    if (nbrThreads > 0)
    {
        pThreads = (AMDTThreadId*)malloc(sizeof(AMDTThreadId) * nbrThreads);

        hr = AMDTGetThreadIds(tpReaderHandle, pid, nullptr, nbrThreads, pThreads);
    }

    gtString data;
    data.appendFormattedString(L"PID - %d\n", pid);
    reportFile.writeString(data);

    for (AMDTUInt32 i = 0; i < nbrThreads; i++)
    {
        AMDTThreadId tid = pThreads[i];

        // print the thread samples
        printThreadSampleData(tpReaderHandle, tid, reportFile);

        // print thread meta data
        data.makeEmpty();
        data.appendFormattedString(L"\n THREAD SUMMARY for TID - %d\n", tid);
        reportFile.writeString(data);
        printThreadSummary(tpReaderHandle, tid, reportFile, true);
    }

    if (nullptr != pThreads)
    {
        free(pThreads);
    }

    return hr;
}

static AMDTResult printAllProcessesSampleData(AMDTThreadProfileDataHandle& tpReaderHandle, osFile& reportFile)
{
    AMDTResult hr = AMDT_STATUS_OK;
    AMDTUInt32 nbrProcesses = 0;
    AMDTProcessId* pProcesses = nullptr;

    hr = AMDTGetProcessIds(tpReaderHandle, &nbrProcesses, 0, nullptr);

    if (nbrProcesses > 0)
    {
        pProcesses = (AMDTProcessId*)malloc(sizeof(AMDTProcessId) * nbrProcesses);

        hr = AMDTGetProcessIds(tpReaderHandle, nullptr, nbrProcesses, pProcesses);
    }


    for (AMDTUInt32 i = 0; i < nbrProcesses; i++)
    {
        printProcessSampleData(tpReaderHandle, pProcesses[i], reportFile);
    }

    if (nullptr != pProcesses)
    {
        free(pProcesses);
    }

    return hr;
}

static AMDTResult printThreadSampleData(AMDTThreadProfileDataHandle& tpReaderHandle, AMDTThreadId tid, osFile& reportFile)
{
    AMDTResult hr = AMDT_STATUS_OK;

    gtString data;
    data.appendFormattedString(L"\nTHREAD ID - %lu:\n", tid);
    reportFile.writeString(data);

    AMDTUInt32 nbrRecords = 0;
    AMDTThreadSample* pThreaSampledData = nullptr;

    hr = AMDTGetThreadSampleData(tpReaderHandle, tid, &nbrRecords, &pThreaSampledData);

    if (nbrRecords > 0)
    {
        reportFile.writeString(L"PID,TID,CORE-ID,START-TS,END-TS,WAIT-TIME,TRANSITION-TIME,EXEC-TIME,THREAD-STATE,WAIT-REASON,WAIT-MODE,NBR-OF-STACKFRAMES,STACKFRAMES\n");
    }
    else
    {
        data.makeEmpty();
        data.appendFormattedString(L"No sample records for TID(%d).\n", tid);
        reportFile.writeString(data);
    }

    for (AMDTUInt32 j = 0; j < nbrRecords; j++)
    {
        data.makeEmpty();

        data.appendFormattedString(L"%d,%d,%d,", pThreaSampledData[j].m_processId,
                                   pThreaSampledData[j].m_threadId,
                                   pThreaSampledData[j].m_coreId);

        data.append(printTS(pThreaSampledData[j].m_startTS));
        data.append(printTS(pThreaSampledData[j].m_endTS));

        data.appendFormattedString(L"%u,%u,%llu ", pThreaSampledData[j].m_waitTime,
                                   pThreaSampledData[j].m_transitionTime,
                                   pThreaSampledData[j].m_execTime);

        char* pStr = nullptr;
        hr = AMDTGetThreadStateString(pThreaSampledData[j].m_threadState, &pStr);

        gtString s;
        s.fromASCIIString(pStr);
        data += L",";
        data.append(s);

        hr = AMDTGetThreadWaitReasonString(pThreaSampledData[j].m_waitReason, &pStr);
        s.makeEmpty();
        s.fromASCIIString(pStr);
        data += L",";
        data += s;

        hr = AMDTGetThreadWaitModeString(pThreaSampledData[j].m_waitMode, &pStr);
        s.makeEmpty();
        s.fromASCIIString(pStr);
        data += L",";
        data += s;

        data.appendFormattedString(L",%d,", pThreaSampledData[j].m_nbrStackFrames);

        for (AMDTUInt32 i = 0; i < pThreaSampledData[j].m_nbrStackFrames; i++)
        {
            //data.appendFormattedString(L" , 0x%llx ", pThreaSampledData[j].m_pStackFrames[i]);
            char* pfuncName;
            AMDTGetFunctionName(tpReaderHandle, pThreaSampledData[j].m_processId, pThreaSampledData[j].m_pStackFrames[i], &pfuncName);
            s.makeEmpty();
            s.fromASCIIString(pfuncName);
            data += s;
            data += L",";
            free(pfuncName);
        }

        data.append(L"\n");
        reportFile.writeString(data);
    }

    // free the pThreaSampledData
    if (nullptr != pThreaSampledData)
    {
        free(pThreaSampledData);
        pThreaSampledData = nullptr;
    }

    return hr;
}

static AMDTResult printAllThreadsSummary(AMDTThreadProfileDataHandle& tpReaderHandle, osFile& reportFile)
{
    AMDTResult hr;

    AMDTUInt32 nbrThreads = 0;
    AMDTThreadId* pThreads = nullptr;
    AMDTProcessId pid = static_cast<AMDTProcessId>(-1);

    hr = AMDTGetThreadIds(tpReaderHandle, pid, &nbrThreads, 0, nullptr);

    if (nbrThreads > 0)
    {
        pThreads = (AMDTThreadId*)malloc(sizeof(AMDTThreadId) * nbrThreads);
        hr = AMDTGetThreadIds(tpReaderHandle, pid, nullptr, nbrThreads, pThreads);
    }

    reportFile.writeString(L"\nALL THREADS SUMMARY\n");

    for (AMDTUInt32 i = 0; i < nbrThreads; i++)
    {
        bool reportHdr = (0 == i) ? true : false;
        printThreadSummary(tpReaderHandle, pThreads[i], reportFile, reportHdr);
    }

    reportFile.writeString(L"\n");

    if (nullptr != pThreads)
    {
        free(pThreads);
    }

    return hr;
}

static AMDTResult printThreadSummary(AMDTThreadProfileDataHandle& tpReaderHandle, AMDTThreadId tid, osFile& reportFile, bool reportHdr)
{
    AMDTResult hr = AMDT_STATUS_OK;
    AMDTThreadData threadData;

    hr = AMDTGetThreadData(tpReaderHandle, tid, &threadData);

    if (reportHdr)
    {
        reportFile.writeString(L"PID,TID,AFFINITY,CREATE-TS,TERMINATE-TS,TOTAL-WAIT-TIME,TOTAL-TRANSITION-TIME,TOTAL-EXEC-TIME,NBR-CS,NBR-CORE-SWITCHES\n");
    }

    gtString data;
    data.appendFormattedString(L"%d,%d,0x%lx,", threadData.m_processId, threadData.m_threadId, threadData.m_affinity);

    data.append(printTS(threadData.m_threadCreateTS));
    data.append(printTS(threadData.m_threadTerminateTS));

    data.appendFormattedString(L"%llu,%llu,%llu,%llu,%llu", threadData.m_totalWaitTime,
                               threadData.m_totalTransitionTime,
                               threadData.m_totalExeTime,
                               threadData.m_nbrOfContextSwitches,
                               threadData.m_nbrOfCoreSwitches);

    reportFile.writeString(data);
    reportFile.writeString(L"\n");

    return hr;
}


HRESULT CpuProfileReport::ReportTP()
{
    HRESULT retVal = S_OK;

    if (IsTpInputFile())
    {
        osFilePath reportFilePath = GetOutputFilePath();
        reportFilePath.setFileExtension(CPUPROFILE_CSV_REPORT_EXTENSION);
        osFile reportFile;

        if (!reportFilePath.isEmpty())
        {
            retVal = reportFile.open(reportFilePath, osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);
        }

        AMDTResult hr;
        AMDTThreadProfileDataHandle tpReaderHandle;
        AMDTProcessId interestingPids[64];    // max 64 pids
        AMDTProcessId interestingPid = static_cast<AMDTProcessId>(-1);

        // input file
        const char* pLogFilePath = m_inputFilePath.asString().asASCIICharArray();

        hr = AMDTOpenThreadProfile(pLogFilePath, &tpReaderHandle);

        if (!m_args.GetPidsList().empty())
        {
            // just support one now
            for (AMDTUInt32 i = 0; i < m_args.GetPidsList().size(); i++)
            {
                interestingPid = m_args.GetPidsList().at(i);
                interestingPids[i] = interestingPid;
            }

            hr = AMDTSetFilterProcesses(tpReaderHandle, 1, interestingPids);
        }

        hr = AMDTProcessThreadProfileData(tpReaderHandle);

        // print the all processes summary
        printAllProcessesSummary(tpReaderHandle, reportFile);

        // Set the symbol server path
        gtString symServer(L"http://msdl.microsoft.com/download/symbols");
        hr = AMDTSetSymbolSearchPath(tpReaderHandle, nullptr, symServer.asASCIICharArray(), nullptr);

        // for the interesting processes and its child processs print the thread sampled data
        if (TP_ALL_PIDS == interestingPid)
        {
            printAllProcessesSampleData(tpReaderHandle, reportFile);
        }
        else
        {
            for (AMDTUInt32 i = 0; i < m_args.GetPidsList().size(); i++)
            {
                interestingPid = m_args.GetPidsList().at(i);

                printProcessSampleData(tpReaderHandle, interestingPid, reportFile);

                // report child processes - only the first level
                AMDTProcessData procData;
                hr = AMDTGetProcessData(tpReaderHandle, interestingPid, &procData);

                for (AMDTUInt32 j = 0; j < procData.m_nbrChildProcesses; j++)
                {
                    interestingPid = procData.m_pChildProcesses[j];
                    printProcessSampleData(tpReaderHandle, interestingPid, reportFile);
                }
            }
        }

        reportFile.writeString(L"\n");

        // print the all threads summary
        if (TP_ALL_PIDS == interestingPid)
        {
            printAllThreadsSummary(tpReaderHandle, reportFile);
        }

        hr = AMDTCloseThreadProfile(tpReaderHandle);

        retVal = (AMDT_STATUS_OK == hr) ? S_OK : E_FAIL;
    }

    return retVal;
}

//      Private Member Functions

void CpuProfileReport::SetupEnvironment()
{
    // Set up the environmental variable so the event files can be found.
    osEnvironmentVariable eventDataPath;
    eventDataPath._name = L"CPUPerfAPIDataPath";
    osFilePath eventFilePath;

    if (osGetCurrentApplicationDllsPath(eventFilePath) || osGetCurrentApplicationPath(eventFilePath))
    {
        eventFilePath.clearFileName();
        eventFilePath.clearFileExtension();

        eventFilePath.appendSubDirectory(L"Data");
        eventFilePath.appendSubDirectory(L"Events");

        gtString eventFilePathStr = eventFilePath.fileDirectoryAsString();

        eventDataPath._value.appendFormattedString(L"%ls/", eventFilePathStr.asCharArray());
        osSetCurrentProcessEnvVariable(eventDataPath);

        m_error = S_OK;
    }

    return;
}

void CpuProfileReport::ValidateOptions()
{
    // REPORT Options
    //  -i <input file - PRD/EBP>
    //  -o <output path dir> output dir, in which processed or report files will be generated
    //  -F csv | text        file format - only csv is supported // not reqd
    //  -V <view xmL>        by default only the raw data will be reported
    //
    //  -R <overview|process|module|callgraph|imix|all> Sections to be reported
    //                       process & module are mutually exclusive
    //                       module & callgraph - NOT possible
    //
    //  -e <event-index>     Use this to sort the OverView and select the CallGraph
    //
    //  -I                   ignore system modules
    //  -N                   seperate by Numa; By defualt it is core - CXL-1.6 ?
    //  -O                   seperate by Core; By defualt it is core - CXL-1.6 ?
    //  -P                   Show Percentage
    //  -c <core mask>       core affinity mask
    //  -D <debug symbol paths>
    //  -S <symbol server directories>
    //  -X                  enable cache in default path (output-dir/cache), if it already exists, error...
    //  -L                  Limit - 1.6, percent-cutoff,cumulative-cutoff,minimum-count

    m_error = E_FAIL;
    osCpuid cpuInfo;

    // validate for valid input dir
    if (! m_args.GetInputFile().isEmpty())
    {
        m_inputFilePath.setFullPathFromString(m_args.GetInputFile());

        if (m_inputFilePath.isDirectory())
        {
            reportError(false, L"Input file (" STR_FORMAT L") is not specified.\n", m_inputFilePath.asString(true).asCharArray());
            return;
        }

        if (! m_inputFilePath.exists())
        {
            reportError(false, L"Input file (" STR_FORMAT L") does not exist.\n", m_inputFilePath.asString(true).asCharArray());
            return;
        }

        gtString fileExtension;
        m_inputFilePath.getFileExtension(fileExtension);
        m_isRawInputFile = (0 == fileExtension.compareNoCase(CPUPROFILE_RAWFILE_EXTENSION)) ? true : false;
        m_isEbpInputFile = (0 == fileExtension.compareNoCase(CPUPROFILE_EBP_REPORT_EXTENSION)) ? true : false;
        m_isTpInputFile = (0 == fileExtension.compareNoCase(THREAD_PROFILE_DATAFILE_EXTENSION)) ? true : false;
        m_isDbInputFile = (0 == fileExtension.compareNoCase(CPUPROFILE_DBFILE_EXTENSION)) ? true : false;

        if (!m_isRawInputFile && !m_isEbpInputFile && !m_isTpInputFile && !m_isDbInputFile)
        {
            reportError(false, L"Invalid input file (" STR_FORMAT L").\n", m_inputFilePath.asString(true).asCharArray());
            return;
        }
    }
    else
    {
        reportError(false, L"Input file is missing. Please use option(-i) to specify input file.\n");
        return;
    }

    // validate the output file path
    if (! m_args.GetOutputFile().isEmpty())
    {
        osFilePath outputPath(m_args.GetOutputFile());
        outputPath.reinterpretAsDirectory();

        if (! outputPath.exists())
        {
            reportError(false, L"Output path (" STR_FORMAT L") does not exists.\n", m_args.GetOutputFile().asCharArray());
            return;
        }

        if (! outputPath.isDirectory())
        {
            reportError(false, L"Output path (" STR_FORMAT L") is not a directory.\n", m_args.GetOutputFile().asCharArray());
            return;
        }
    }
    else
    {
        reportError(false, L"Output file path is missing. Please use option(-o) to specify the output file path.\n");
        return;
    }

    // Check the output file format
    if (! m_args.GetOutputFileFormat().isEmpty())
    {
        // Only CSV is supported
        if (! m_args.GetOutputFileFormat().toLowerCase().compare(L"csv"))
        {
            m_reportType = CPU_PROFILE_REPORT_TYPE_CSV;
        }
        else
        {
            reportError(false, L"Unsupported output file format(" STR_FORMAT L") specified with option(-F).\n", m_args.GetOutputFileFormat().asCharArray());
            return;
        }
    }

    // Validate the report sections requested to be printed
    gtString reportSections = m_args.GetSectionsToReport().isEmpty() ? CPU_PROFILE_REPORT_ALL
                              : m_args.GetSectionsToReport();

    if (!reportSections.isEmpty())
    {
        int pos;
        int startPosition = 0;
        bool invalidReportSection = false;

        do
        {
            pos = reportSections.find(L",", startPosition);
            int endPosition = (-1 != pos) ? (pos - 1) : reportSections.length();

            gtString tmpStr;
            reportSections.getSubString(startPosition, endPosition, tmpStr);

            if (!tmpStr.toLowerCase().compare(CPU_PROFILE_REPORT_ALL))
            {
                m_isReportOverview = true;
                m_isReportAggregateByProcess = true;
                m_isReportAggregateByModule = false;
                m_isReportCallGraph = true;
            }
            else if (!tmpStr.toLowerCase().compare(CPU_PROFILE_REPORT_OVERVIEW))
            {
                m_isReportOverview = true;
            }
            else if (!tmpStr.toLowerCase().compare(CPU_PROFILE_REPORT_PROCESS))
            {
                m_isReportAggregateByProcess = true;
            }
            else if (!tmpStr.toLowerCase().compare(CPU_PROFILE_REPORT_MODULE))
            {
                m_isReportAggregateByModule = true;
            }
            else if (!tmpStr.toLowerCase().compare(CPU_PROFILE_REPORT_CALLGRAPH))
            {
                m_isReportCallGraph = true;
            }
            else if (!tmpStr.toLowerCase().compare(CPU_PROFILE_REPORT_IMIX))
            {
                m_isReportImix = true;
            }
            else
            {
                invalidReportSection = true;
            }

            startPosition = pos + 1;
        }
        while (-1 != pos);

        if (invalidReportSection)
        {
            reportError(false, L"Invalid report section (" STR_FORMAT L") specified with option(-R).\n",
                        reportSections.asCharArray());
            return;
        }

        // TODO:
        if (m_isReportAggregateByModule)
        {
            reportError(false, L"Aggregate by Module is not yet supported!!\n");
            return;
        }

        if (m_isReportAggregateByProcess && m_isReportAggregateByModule)
        {
            reportError(false, L"Aggregate by Process and Aggregate by Module together not supported.\n");
            return;
        }

        if (m_isReportAggregateByModule && m_isReportCallGraph)
        {
            reportError(false, L"Aggregate by Module and Callgraph together not supported.\n");
            return;
        }
    }

    // Validate the core affinity mask
    // TODO: Currently this supports only upto 64 cores
    if (static_cast<gtUInt64>(-1) != m_args.GetCoreAffinityMask())
    {
        int nbrCores;
        osGetAmountOfLocalMachineCPUs(nbrCores);

        gtUInt64 tmpAffinity = 0;

        for (int core = 0; core < nbrCores; core++)
        {
            tmpAffinity <<= 1;
            tmpAffinity |= 1;
        }

        if (tmpAffinity < m_args.GetCoreAffinityMask())
        {
            reportError(false, L"Invalid core affinity mask (0x%lx) specified with option(-c).\n", m_args.GetCoreAffinityMask());
            return;
        }
    }

    // Fill-in the m_coresList from the coreAffinity mask
    // If seperate-by-core and core-affinity-mask not used, then report for all the cores
    if (m_args.IsReportByCore())
    {
        gtUInt64 coreMask = m_args.GetCoreAffinityMask();

        if (static_cast<gtUInt64>(-1) == coreMask)
        {
            int nbrCores;

            // FIXME.. this should be from profile reader
            osGetAmountOfLocalMachineCPUs(nbrCores);

            for (int core = 0; core < nbrCores; core++)
            {
                coreMask <<= 1;
                coreMask |= 1;
            }
        }

        gtUInt32 coreId = 0;

        while (coreMask)
        {
            if (coreMask & 0x1)
            {
                m_coresList.push_back(coreId);

                coreMask >>= 1;
                coreId++;
            }
        }
    }

    // For predefined profile configs, construct the profile XML file path
    osFilePath profileFilePath;

    if (! m_args.GetViewConfigName().isEmpty())
    {
        // Construct the profile path
        if (osGetCurrentApplicationDllsPath(profileFilePath) || osGetCurrentApplicationPath(profileFilePath))
        {
            profileFilePath.clearFileName();
            profileFilePath.clearFileExtension();

            profileFilePath.appendSubDirectory(L"Data");
            profileFilePath.appendSubDirectory(L"Views");

            // TODO: For TBP, no need to add family sub directory
            {
                gtString familySubDir;
                cpuInfo.getFamily() >= FAMILY_OR
                ? familySubDir.appendFormattedString(L"0x%x_0x%x", cpuInfo.getFamily(), (cpuInfo.getModel() >> 4))
                : familySubDir.appendFormattedString(L"0x%x", cpuInfo.getFamily());

                profileFilePath.appendSubDirectory(familySubDir);
            }

            profileFilePath.setFileName(m_args.GetViewConfigName().toLowerCase());
            profileFilePath.setFileExtension(L"xml");
        }

        m_viewConfigXMLPath = profileFilePath;

        if (! m_viewConfigXMLPath.exists())
        {
            reportError(false, L"View XML path (" STR_FORMAT L") does not exists.\n", m_viewConfigXMLPath.asString().asCharArray());
            return;
        }
    }

    m_error = S_OK;
    return;
}


// Actually this one returns the absolute path with only the base file name.
// with this base name
osFilePath& CpuProfileReport::GetOutputFilePath()
{
    m_error = S_OK;

    if (m_outputFilePath.isEmpty())
    {
        gtString outputFile = m_args.GetOutputFile();

        if (outputFile.isEmpty())
        {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
            wchar_t  tmpPath[OS_MAX_PATH] = { L'\0' };
            GetTempPathW(OS_MAX_PATH, tmpPath);
            outputFile = gtString(tmpPath);
#else
            // TODO: USe P_tmpdir on Linux
            wchar_t  tmpPath[OS_MAX_PATH] = L"/tmp/";
            outputFile = gtString(tmpPath);
#endif // AMDT_BUILD_TARGET
        }

        m_outputFilePath = osFilePath(outputFile);
        m_outputFilePath.reinterpretAsDirectory();

        // Get the basename from the input file and create a output dir
        // in which the output files EBP/IMD will be created
        osFilePath inputFile(m_args.GetInputFile());
        gtString inputFileName;
        inputFile.getFileName(inputFileName);

        m_outputFilePath.appendSubDirectory(inputFileName);

        // check if the base dir exists
        osDirectory osDir;
        m_outputFilePath.getFileDirectory(osDir);

        if (! osDir.exists())
        {
            osDir.create();
        }
    }

    if (isOK())
    {
        // Add the output file name - which is base of the input file name
        gtString inputBaseName;
        m_inputFilePath.getFileName(inputBaseName);
        m_outputFilePath.setFileName(inputBaseName);
        m_outputFilePath.setFileExtension(L"ebp");
    }

    return m_outputFilePath;
}

osFilePath& CpuProfileReport::GetEBPFilePath()
{
    if (IsRawInputFile())
    {
        return GetOutputFilePath();
    }
    else
    {
        return m_inputFilePath;
    }
}

// Actually this one returns the absolute path with only the base file name.
// with this base name
bool CpuProfileReport::InitializeReportFile()
{
    m_error = S_OK;

    // CSV Reporter
    osFilePath reportFilePath = GetOutputFilePath();

    if (CPU_PROFILE_REPORT_TYPE_CSV == m_reportType)
    {
        reportFilePath.setFileExtension(CPUPROFILE_CSV_REPORT_EXTENSION);
    }
    else if (CPU_PROFILE_REPORT_TYPE_TEXT == m_reportType)
    {
        reportFilePath.setFileExtension(CPUPROFILE_TEXT_REPORT_EXTENSION);
    }
    else if (CPU_PROFILE_REPORT_TYPE_XML == m_reportType)
    {
        reportFilePath.setFileExtension(CPUPROFILE_XML_REPORT_EXTENSION);
    }
    else
    {
        return false;
    }

    m_pReporter = new CSVReporter(reportFilePath);
    m_pReporter->Open();

    return true;
}

void CpuProfileReport::ReadRunInfo()
{
    if (isOK())
    {
        // construct the RI file path
        osFilePath rifilePath;
        rifilePath = m_args.GetInputFile();

        if (isOK())
        {
            rifilePath.setFileExtension(L"ri");

            // populate RI data
            // RunInfo rInfo;
        }
    }
}

void CpuProfileReport::ReportExecution()
{
    gtVector<gtString> sectionHdrs;
    gtList<std::pair<gtString, gtString> > sectionDataList;

    // Get the profile session data from the profile reader:
    CpuProfileInfo* pProfileInfo = m_profileReader.getProfileInfo();

    gtString tmpStr;

    // EXECUTION
    //    Target Path:,classic.exe
    //    Working Directory:,c:\temp
    //    Command Line Arguments:,123 -i o
    //    Environment Variables:
    //    Cpu Details:
    //    Operating System:

    if (nullptr != pProfileInfo)
    {
        sectionHdrs.push_back(CODEXL_REPORT_HDR);
        sectionHdrs.push_back(EXECUTION_SECTION_HDR);

        std::pair<gtString, gtString> keyValuePair;

        // Target Path
        keyValuePair.first = EXECUTION_TARGET_PATH;
        keyValuePair.second = m_profileReader.getProfileInfo()->m_targetPath;
        sectionDataList.push_back(keyValuePair);

        // Target Args
        keyValuePair.first = EXECUTION_TARGET_ARGS;
        keyValuePair.second = m_profileReader.getProfileInfo()->m_cmdArguments;
        sectionDataList.push_back(keyValuePair);

        // Working Directory
        keyValuePair.first = EXECUTION_WORK_DIR;
        keyValuePair.second = m_profileReader.getProfileInfo()->m_wrkDirectory;
        sectionDataList.push_back(keyValuePair);

        // Environment Variables
        keyValuePair.first = EXECUTION_TARGET_ENV_VARS;
        keyValuePair.second = m_profileReader.getProfileInfo()->m_envVariables;
        sectionDataList.push_back(keyValuePair);

        // Cpu Details
        keyValuePair.first = PROFILE_CPU_DETAILS;
        tmpStr = L"";//reuse variable
        tmpStr.appendFormattedString(L"Family(0x%lx), Model(%ld), Number of Cores(%ld)",
                                     m_profileReader.getProfileInfo()->m_cpuFamily,
                                     m_profileReader.getProfileInfo()->m_cpuModel,
                                     m_profileReader.getProfileInfo()->m_numCpus);
        keyValuePair.second = tmpStr;
        sectionDataList.push_back(keyValuePair);

        // Target Operating System
        keyValuePair.first = PROFILE_TAREGET_OS;

        if (!(m_profileReader.getProfileInfo()->m_osName.isEmpty()))
        {
            tmpStr = m_profileReader.getProfileInfo()->m_osName;
        }
        else
        {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
            tmpStr = L"Windows";
#else
            tmpStr = L"Linux";
#endif
        }

        keyValuePair.second = tmpStr;
        sectionDataList.push_back(keyValuePair);
    }

    if ((nullptr != m_pReporter) && m_pReporter->IsOpened())
    {
        m_pReporter->ReportExecution(sectionHdrs, sectionDataList);
    }

    return;
}

void CpuProfileReport::ReportProfileDetails()
{
    gtVector<gtString> sectionHdrs;
    gtList<std::pair<gtString, gtString> > sectionDataList;

    // Get the profile session data from the profile reader:
    CpuProfileInfo* pProfileInfo = m_profileReader.getProfileInfo();

    gtString tmpStr;

    // PROFILE DETAILS
    // Profile Session Type:,TBP
    // Profile Scope:,SWP
    // CPU Affinity:,0xf
    // Call Stack Sampling:,True
    // Profile Start Time:,10-11-2014
    // Profile End Time:,11-11-2014
    // Profile Duration:,10 seconds
    // Data Folder:,c:\temp
    // SamplingEvents,
    //  ,cpu-cycles
    //  ,retired-instructions

    if (nullptr != pProfileInfo)
    {
        sectionHdrs.push_back(PROFILE_DETAILS_SECTION_HDR);

        std::pair<gtString, gtString> keyValuePair;

        // Profile Session Type
        keyValuePair.first = PROFILE_SESSION_TYPE;
        keyValuePair.second = m_profileReader.getProfileInfo()->m_profType;
        sectionDataList.push_back(keyValuePair);

        // Profile Scope
        keyValuePair.first = PROFILE_SCOPE;
        keyValuePair.second = m_profileReader.getProfileInfo()->m_profScope;
        sectionDataList.push_back(keyValuePair);

        // Core Affinity Mask
        keyValuePair.first = PROFILE_CPU_AFFINITY;
        CoreTopologyMap* pCoreTop = m_profileReader.getTopologyMap();
        gtUInt64 coreMask = 0;

        if (nullptr != pCoreTop)
        {
            CoreTopologyMap::iterator it = pCoreTop->begin();

            for (; it != pCoreTop->end(); it++)
            {
                coreMask |= (1ULL << (*it).second.processor);
            }
        }

        tmpStr.makeEmpty();//reuse var
        tmpStr.appendFormattedString(L"0x%lx", coreMask);
        keyValuePair.second = tmpStr;
        sectionDataList.push_back(keyValuePair);

        // Profile Start Time
        keyValuePair.first = PROFILE_START_TIME;
        keyValuePair.second = m_profileReader.getProfileInfo()->m_profStartTime;
        sectionDataList.push_back(keyValuePair);

        // Profile End Time
        keyValuePair.first = PROFILE_END_TIME;
        keyValuePair.second = m_profileReader.getProfileInfo()->m_profEndTime;
        sectionDataList.push_back(keyValuePair);

        // Profile Duration
        keyValuePair.first = PROFILE_DURATION;
        osTime startTime, endTime;
        startTime.setFromDateTimeString(osTime::LOCAL, m_profileReader.getProfileInfo()->m_profStartTime, osTime::NAME_SCHEME_FILE);
        endTime.setFromDateTimeString(osTime::LOCAL, m_profileReader.getProfileInfo()->m_profEndTime, osTime::NAME_SCHEME_FILE);
        gtUInt64 profileDuration = endTime.secondsFrom1970() - startTime.secondsFrom1970();

        tmpStr.makeEmpty();
        tmpStr.appendFormattedString(L"%llu seconds", profileDuration);
        keyValuePair.second = tmpStr;
        sectionDataList.push_back(keyValuePair);

        // Profile Data Folder
        keyValuePair.first = PROFILE_DATA_FOLDER;
        keyValuePair.second = m_profileReader.getProfileInfo()->m_profDirectory; // TBD: Do we need to report this ?
        sectionDataList.push_back(keyValuePair);

        keyValuePair.first = PROFILE_CSS;
        keyValuePair.second = m_profileReader.getProfileInfo()->m_isCSSEnabled ? L"True" : L"False";
        sectionDataList.push_back(keyValuePair);

        keyValuePair.first = PROFILE_CSS_DEPTH;
        tmpStr.makeEmpty();
        tmpStr.appendFormattedString(L"%ld", m_profileReader.getProfileInfo()->m_cssUnwindDepth);
        keyValuePair.second = tmpStr;
        sectionDataList.push_back(keyValuePair);

        const wchar_t* pCssScopeStr = nullptr;

        switch (m_profileReader.getProfileInfo()->m_cssScope)
        {
            case CP_CSS_SCOPE_USER:
                pCssScopeStr = L"User space";
                break;

            case CP_CSS_SCOPE_KERNEL:
                pCssScopeStr = L"Kernel space";
                break;

            case CP_CSS_SCOPE_ALL:
                pCssScopeStr = L"User space and Kernel space";
                break;

            default:
                break;
        }

        if (nullptr != pCssScopeStr)
        {
            keyValuePair.first = PROFILE_CSS_SCOPE;
            keyValuePair.second = pCssScopeStr;
            sectionDataList.push_back(keyValuePair);
        }

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        keyValuePair.first = PROFILE_CSS_FPO;
        keyValuePair.second = m_profileReader.getProfileInfo()->m_isCssSupportFpo ? L"True" : L"False";
        sectionDataList.push_back(keyValuePair);
#endif

        keyValuePair.first = PROFILE_CSS_FREQUENCY;
        tmpStr.makeEmpty();
        tmpStr.appendFormattedString(L"%ld", 1); // TODO: update RI file
        keyValuePair.second = tmpStr;
        sectionDataList.push_back(keyValuePair);
    }

    if ((nullptr != m_pReporter) && m_pReporter->IsOpened())
    {
        m_pReporter->ReportProfileData(sectionHdrs, sectionDataList);
    }

    // Report Monitored events
    ReportMonitoredEventDetails();

    return;
}


void CpuProfileReport::ReportMonitoredEventDetails()
{
    gtVector<gtString> sectionHdrs;

    sectionHdrs.push_back(PROFILE_SAMPLING_EVENTS);
    m_pReporter->ReportSamplingSpec(sectionHdrs,
                                    m_profileReader.getProfileInfo()->m_eventVec,
                                    m_pEventConfig,
                                    m_monitoredEventsNameVec);

    return;
}


void CpuProfileReport::ReportOverviewData()
{
    gtVector<gtString> sectionHdrs;
    gtString tmpStr;
    gtVector<gtUInt64> totalProcSamples;

    bool sepByCore = m_args.IsReportByCore();

    gtUInt64 flags = m_args.IsIgnoreSystemModules() ? SAMPLE_IGNORE_SYSTEM_MODULES : 0;
    flags |= m_args.IsReportByCore() ? SAMPLE_SEPARATE_BY_CORE : 0;

    gtUInt64 coreMask = m_args.GetCoreAffinityMask();

    bool rc = GetProcessInfoMap(m_profileReader,
                                sepByCore,
                                coreMask,
                                m_procInfoMap);

    // Get the module details
    if (rc)
    {
        ProcessIdType pid = static_cast<ProcessIdType>(-1);
        rc = GetModuleInfoList(m_profileReader,
                               pid,
                               flags,
                               coreMask,
                               m_moduleInfoList,
                               m_totalModSamples,
                               &m_procInfoMap);
    }

    // Get function data from all the processes
    if (rc)
    {
        rc = GetOverviewFunctionData();
    }

    // Get the coloumn spec
    gtUInt32 nbrCols = m_overviewConfig.GetNumberOfColumns();
    ColumnSpec* columnArray = new ColumnSpec[nbrCols];
    m_overviewConfig.GetColumnSpecs(columnArray);

    // Report Overview only if requested
    if (rc && IsReportOverview())
    {
        // Overview FUNCTIONS
        tmpStr = OVERVIEW_FUNCTIONS_SECTION_HDR;
        tmpStr.appendFormattedString(L" (Sort Event - ");
        tmpStr.appendFormattedString(STR_FORMAT, m_sortEventName.asCharArray());
        tmpStr.appendFormattedString(L")");
        sectionHdrs.push_back(tmpStr);

        tmpStr.makeEmpty();
        tmpStr.appendFormattedString(STR_FORMAT, OVERVIEW_SECTION_FUNCTIONS);

        // Add the profiled event's names
        for (gtUInt32 i = 0; i < m_profileEventsNameVec.size(); i++)
        {
            tmpStr.appendFormattedString(L"," STR_FORMAT, m_profileEventsNameVec[i].asCharArray());
        }

        tmpStr.appendFormattedString(L"," STR_FORMAT, OVERVIEW_SECTION_MODULE);
        sectionHdrs.push_back(tmpStr);

        rc = m_pReporter->WriteOverviewFunction(sectionHdrs,
                                                m_funcInfoList,
                                                m_totalModSamples,
                                                nbrCols,
                                                columnArray,
                                                m_profileReader.getProfileInfo()->m_eventVec,
                                                m_args.IsShowPercentage());
    }

    // Overview PROCESS
    if (rc && IsReportOverview())
    {
        sectionHdrs.clear();
        tmpStr.makeEmpty();

        tmpStr = OVERVIEW_PROCESSES_SECTION_HDR;
        tmpStr.appendFormattedString(L" (Sort Event - ");
        tmpStr.appendFormattedString(STR_FORMAT, m_sortEventName.asCharArray());
        tmpStr.appendFormattedString(L")");
        sectionHdrs.push_back(tmpStr);

        tmpStr.makeEmpty();
        tmpStr.appendFormattedString(STR_FORMAT L"," STR_FORMAT , OVERVIEW_SECTION_PROCESS, OVERVIEW_SECTION_PID);

        // Add the profiled event's names
        for (gtUInt32 i = 0; i < m_profileEventsNameVec.size(); i++)
        {
            tmpStr.appendFormattedString(L"," STR_FORMAT, m_profileEventsNameVec[i].asCharArray());
        }

        sectionHdrs.push_back(tmpStr);

        if (m_procInfoMap.size() > 1)
        {
            rc = m_pReporter->WriteOverviewProcess(sectionHdrs,
                                                   m_procInfoMap,
                                                   m_totalModSamples,
                                                   nbrCols,
                                                   columnArray,
                                                   m_profileReader.getProfileInfo()->m_eventVec,
                                                   m_args.IsShowPercentage());
        }
    }

    // Overview - MODULE
    if (rc && IsReportOverview())
    {
        sectionHdrs.clear();
        tmpStr.makeEmpty();

        tmpStr.makeEmpty();
        tmpStr = OVERVIEW_MODULES_SECTION_HDR;
        tmpStr.appendFormattedString(L" (Sort Event - ");
        tmpStr.appendFormattedString(STR_FORMAT, m_sortEventName.asCharArray());
        tmpStr.appendFormattedString(L")");
        sectionHdrs.push_back(tmpStr);

        tmpStr.makeEmpty();
        tmpStr.appendFormattedString(STR_FORMAT, OVERVIEW_SECTION_MODULE);

        // Add the profiled event's names
        for (gtUInt32 i = 0; i < m_profileEventsNameVec.size(); i++)
        {
            tmpStr.appendFormattedString(L"," STR_FORMAT, m_profileEventsNameVec[i].asCharArray());
        }

        sectionHdrs.push_back(tmpStr);

        rc = m_pReporter->WriteOverviewModule(sectionHdrs,
                                              m_moduleInfoList,
                                              m_totalModSamples,
                                              nbrCols,
                                              columnArray,
                                              m_profileReader.getProfileInfo()->m_eventVec,
                                              m_args.IsShowPercentage());

    }

    if (nullptr != columnArray)
    {
        delete[] columnArray;
    }

    return;
}


void CpuProfileReport::ReportProcessData()
{
    if (!IsReportAggregateByProcess() && !IsReportCallGraph())
    {
        return;
    }

    gtVector<gtString> sectionHdrs;
    gtString tmpStr;

    gtUInt64 flags = m_args.IsIgnoreSystemModules() ? SAMPLE_IGNORE_SYSTEM_MODULES : 0;
    flags |= m_args.IsReportByCore() ? SAMPLE_SEPARATE_BY_CORE : 0;
    gtUInt64 coreMask = m_args.GetCoreAffinityMask();

    // Get the column spec
    gtUInt32 nbrCols = m_viewConfig.GetNumberOfColumns();
    ColumnSpec* columnArray = new ColumnSpec[nbrCols];
    m_viewConfig.GetColumnSpecs(columnArray);

    // Now that we have got the columnspec from view config, check
    // whether all the events are in event vector
    for (gtUInt32 i = 0; i < nbrCols; i++)
    {
        EventMaskType eventLeft = EncodeEvent(columnArray[i].dataSelectLeft.eventSelect,
                                              columnArray[i].dataSelectLeft.eventUnitMask,
                                              columnArray[i].dataSelectLeft.bitOs,
                                              columnArray[i].dataSelectLeft.bitUsr);
        EventMaskType eventRight = EncodeEvent(columnArray[i].dataSelectRight.eventSelect,
                                               columnArray[i].dataSelectRight.eventUnitMask,
                                               columnArray[i].dataSelectRight.bitOs,
                                               columnArray[i].dataSelectRight.bitUsr);

        if (columnArray[i].dataSelectLeft.eventSelect
            && (m_evtIndexMap.find(eventLeft) == m_evtIndexMap.end()))
        {
            reportError(false, L"Invalid/Incorrect View Config XML files is specified with option(-V).\n");
            return;
        }

        if (columnArray[i].dataSelectRight.eventSelect
            && (m_evtIndexMap.find(eventRight) == m_evtIndexMap.end()))
        {
            reportError(false, L"Invalid/Incorrect View Config XML files is specified with option(-V).\n");
            return;
        }
    }

    // Get the map based on number of samples
    ProcessInfoMap pMap;
    // Iterate over the process map and print
    int sortEventIndex = m_args.GetSortEventIndex() == -1 ? 0 : m_args.GetSortEventIndex();

    for (PidProcessInfoMap::iterator iter = m_procInfoMap.begin(); iter != m_procInfoMap.end(); iter++)
    {
        double value = static_cast<double>((*iter).second.m_dataVector[sortEventIndex]);
        pMap.insert(ProcessInfoMap::value_type(value, (*iter).second));
    }

    int nbrProc = 0;

    // TBD - just print the top 5 processes
    for (ProcessInfoMap::reverse_iterator it = pMap.rbegin(); ((it != pMap.rend()) && (nbrProc < 5)); it++, nbrProc++)
    {
        ProcessInfo* pProc = &((*it).second);
        ProcessIdType pid = pProc->m_pid;

        if (IsReportAggregateByProcess())
        {
            // PROCESS HEADER
            gtString procHdr(PROCESS_PROFILE_HDR);
            procHdr.appendFormattedString(STR_FORMAT, pProc->m_processName.asCharArray());
            procHdr.appendFormattedString(L" (PID - %ld)", pProc->m_pid);

            sectionHdrs.clear();
            sectionHdrs.push_back(procHdr);
            tmpStr.makeEmpty();
            tmpStr.appendFormattedString(STR_FORMAT, OVERVIEW_SECTION_PROCESS);

            for (gtUInt32 i = 0; i < nbrCols; i++)
            {
                tmpStr.appendFormattedString(L"," STR_FORMAT, convertToGTString(columnArray[i].title).asCharArray());
            }

            sectionHdrs.push_back(tmpStr);

            // REPORT the process
            m_pReporter->WritePidSummary(sectionHdrs,
                                         *pProc,
                                         m_totalModSamples,
                                         nbrCols,
                                         columnArray,
                                         m_profileReader.getProfileInfo()->m_eventVec,
                                         m_args.IsShowPercentage(),
                                         m_args.IsReportByCore());

            // Get the module data for this pid
            ModuleInfoList modInfo;
            gtVector<gtUInt64> totalModSamples;

            GetModuleInfoList(m_profileReader,
                              pid,
                              flags,
                              coreMask,
                              modInfo,
                              totalModSamples,
                              nullptr);

            // MODULE section Hdrs
            sectionHdrs.clear();
            sectionHdrs.push_back(MODULE_SUMMARY_SECTION_HDR);
            tmpStr.makeEmpty();
            tmpStr.appendFormattedString(STR_FORMAT, OVERVIEW_SECTION_MODULE);

            for (gtUInt32 i = 0; i < nbrCols; i++)
            {
                tmpStr.appendFormattedString(L"," STR_FORMAT, convertToGTString(columnArray[i].title).asCharArray());
            }

            sectionHdrs.push_back(tmpStr);

            if (modInfo.empty())
            {
                tmpStr.makeEmpty();
                tmpStr.appendFormattedString(STR_FORMAT, NO_SAMPLES_TO_DISPLAY);
                sectionHdrs.push_back(tmpStr);
            }

            m_pReporter->WritePidModuleSummary(sectionHdrs,
                                               modInfo,
                                               totalModSamples,
                                               nbrCols,
                                               columnArray,
                                               m_profileReader.getProfileInfo()->m_eventVec,
                                               m_args.IsShowPercentage(),
                                               m_args.IsReportByCore());

            //  FUNCTIONS  SUMMARY
            FunctionInfoList funcInfoList;
            GetFunctionData(pid, (ThreadIdType) - 1, funcInfoList);

            sectionHdrs.clear();
            sectionHdrs.push_back(FUNCTION_SUMMARY_SECTION_HDR);
            tmpStr.makeEmpty();
            tmpStr.appendFormattedString(STR_FORMAT, OVERVIEW_SECTION_FUNCTIONS);

            for (gtUInt32 i = 0; i < nbrCols; i++)
            {
                tmpStr.appendFormattedString(L"," STR_FORMAT, convertToGTString(columnArray[i].title).asCharArray());
            }

            tmpStr.appendFormattedString(L"," STR_FORMAT, OVERVIEW_SECTION_MODULE);

            sectionHdrs.push_back(tmpStr);

            if (funcInfoList.empty())
            {
                tmpStr.makeEmpty();
                tmpStr.appendFormattedString(STR_FORMAT, NO_SAMPLES_TO_DISPLAY);
                sectionHdrs.push_back(tmpStr);
            }

            m_pReporter->WritePidFunctionSummary(sectionHdrs,
                                                 funcInfoList,
                                                 totalModSamples,
                                                 nbrCols,
                                                 columnArray,
                                                 m_profileReader.getProfileInfo()->m_eventVec,
                                                 m_args.IsShowPercentage(),
                                                 m_args.IsReportByCore());
        }

        // Report CallGraph
        if (IsReportCallGraph() && pProc->m_hasCSS)
        {
            ReportCSSData(pProc->m_pid);
        }
    } // iterate over the process list

    if (nullptr != columnArray)
    {
        delete[] columnArray;
    }
}

// Report is aggregated by Module
// TODO: Add support for event filtering (option -e)
void CpuProfileReport::ReportImixData()
{
    if (!IsReportImix())
    {
        return;
    }

    gtVector<gtString> sectionHdrs;
    gtUInt64 totalSamples = 0;
    gtUInt64 coreMask = m_args.GetCoreAffinityMask();
    gtUInt64 flags = 0;

    flags = m_args.IsIgnoreSystemModules() ? SAMPLE_IGNORE_SYSTEM_MODULES : 0;
    // Only "group-by-module" is supported now
    flags |= SAMPLE_GROUP_BY_MODULE;

    ProcessIdType pid = static_cast<ProcessIdType>(ALL_PROCESS_IDS);
    ThreadIdType tid = static_cast<ThreadIdType>(ALL_THREAD_IDS);

    ModuleImixInfoList modImixInfoList;

    bool rc = GetImixInfoList(m_profileReader, pid, tid, flags, coreMask, modImixInfoList, totalSamples);

    if (rc)
    {
        ImixSummaryMap imixSummaryMap;
        rc = GetImixSummaryMap(m_profileReader, modImixInfoList, imixSummaryMap, totalSamples);

        if (rc)
        {
            gtString tempStr(L"IMIX SUMMARY REPORT\n");
            tempStr.appendFormattedString(L"Total Samples Taken = %llu\n", totalSamples);
            sectionHdrs.push_back(tempStr);

            tempStr = L"Disassembly";
            tempStr.appendFormattedString(L",Samples Percentage");
            tempStr.appendFormattedString(L",Samples Count\n");
            sectionHdrs.push_back(tempStr);

            m_pReporter->WriteImixSummaryInfo(sectionHdrs, imixSummaryMap, totalSamples);
        }
    }

    if (rc)
    {
        gtString tempStr = L"IMIX MODULE-WISE REPORT\n\n";
        sectionHdrs.clear();
        sectionHdrs.push_back(tempStr);

        m_pReporter->WriteImixInfo(sectionHdrs, modImixInfoList, totalSamples);
    }
}

bool CpuProfileReport::ReportCSSData(ProcessIdType pid)
{
    bool retVal = false;

    // Construct the callgraph function list
    int sortEventIndex = m_args.GetSortEventIndex();

    EventMaskType eventType = (sortEventIndex != -1) ? m_profileReader.getProfileInfo()->m_eventVec[sortEventIndex].eventMask
                              : EventMaskType(-1);

    // Hack to disable the OS/USER flags for TIMER event
    gtUInt16 eventSel;
    gtUByte unitMask;
    bool os;
    bool user;
    DecodeEvent(eventType, &eventSel, &unitMask, &os, &user);

    // Timer event, IBS Fetch/Op event do not have the OS/USER settings
    if (IsTimerEvent(eventSel) || IsIbsFetchEvent(eventSel) || IsIbsOpEvent(eventSel))
    {
        os = user = false;
        eventType = EncodeEvent(eventSel,
                                unitMask,
                                os,
                                user);
    }

    retVal = GetCSSData(pid, eventType, m_callGraphFuncMap);

    // Headers
    gtVector<gtString> sectionHdrs;

    gtString cgHdr(CALLGRAPH_HDR);
    cgHdr.appendFormattedString(L" (PID - %ld)", pid);
    cgHdr.appendFormattedString(L" (Sort Event - ");
    cgHdr.appendFormattedString(STR_FORMAT, m_sortEventName.asCharArray());
    cgHdr.appendFormattedString(L")");
    sectionHdrs.push_back(cgHdr);

    sectionHdrs.push_back(CALLGRAPH_FUNCTION_SUMMARY_HDR);

    gtString tmpStr(CALLGRAPH_FUCNTION);
    tmpStr += L",";
    tmpStr += CALLGRAPH_SELF_SAMPLES;
    tmpStr += L",";
    tmpStr += CALLGRAPH_DEEP_SAMPLES;
    tmpStr += L",";
    tmpStr += CALLGRAPH_PATH_COUNT;
    tmpStr += L",";
    tmpStr += CALLGRAPH_SOURCE_FILE;
    tmpStr += L",";
    tmpStr += CALLGRAPH_MODULE;
    sectionHdrs.push_back(tmpStr);

    // Create the map based on the number of deep samples
    CGSampleFunctionMap cgSampleFuncMap;

    for (CGFunctionInfoMap::iterator it = m_callGraphFuncMap.begin(); it != m_callGraphFuncMap.end(); it++)
    {
        CGFunctionInfo* pFuncNode = (*it).second;
        cgSampleFuncMap.insert(CGSampleFunctionMap::value_type(pFuncNode->m_deepCount, pFuncNode));
    }

    // Set the index
    gtUInt64 funcIndex = 0;

    for (CGSampleFunctionMap::reverse_iterator rit = cgSampleFuncMap.rbegin(); rit != cgSampleFuncMap.rend(); rit++)
    {
        CGFunctionInfo* pFuncNode = (*rit).second;
        pFuncNode->m_index = ++funcIndex;
    }

    // write the callgraph function table
    bool showPerc = true;
    m_pReporter->WriteCallGraphFunctionSummary(sectionHdrs, cgSampleFuncMap, eventType, showPerc);

    //
    // Write the callgraph
    //
    sectionHdrs.clear();
    sectionHdrs.push_back(CALLGRAPH_SECTION_HDR);

    tmpStr.makeEmpty();
    tmpStr += L",";
    tmpStr += L",";
    tmpStr += CALLGRAPH_PARENTS;
    sectionHdrs.push_back(tmpStr);

    tmpStr.makeEmpty();
    tmpStr += CALLGRAPH_INDEX;
    tmpStr += L",";
    tmpStr += CALLGRAPH_FUCNTION;
    tmpStr += L",";
    tmpStr += L",";
    tmpStr += CALLGRAPH_SELF_SAMPLES;
    tmpStr += L",";
    tmpStr += CALLGRAPH_DEEP_SAMPLES;
    tmpStr += L",";
    tmpStr += CALLGRAPH_SOURCE_FILE;
    tmpStr += L",";
    tmpStr += CALLGRAPH_MODULE;
    sectionHdrs.push_back(tmpStr);

    tmpStr.makeEmpty();
    tmpStr += L",";
    tmpStr += L",";
    tmpStr += CALLGRAPH_CHILDREN;
    sectionHdrs.push_back(tmpStr);

    m_pReporter->WriteCallGraph(sectionHdrs, cgSampleFuncMap, eventType, showPerc);

    ClearCSSData();

    return retVal;
}


void CpuProfileReport::ClearCSSData()
{
    CGFunctionInfoMap::iterator iter = m_callGraphFuncMap.begin();

    for (; iter != m_callGraphFuncMap.end(); iter++)
    {
        CGFunctionInfo* pFunc = (*iter).second;

        if (nullptr != pFunc)
        {
            delete pFunc;
        }
    }

    m_callGraphFuncMap.clear();
}


bool CpuProfileReport::GetOverviewFunctionData()
{
    bool retVal = false;

    // Ignore/Report system modules.
    gtUInt64 flags = m_args.IsIgnoreSystemModules() ? SAMPLE_IGNORE_SYSTEM_MODULES : 0;
    gtUInt64 coreMask = m_args.GetCoreAffinityMask();

    // For overview, we need to do "group-by-module"
    flags |= SAMPLE_GROUP_BY_MODULE;

    // Retrieve the function data for all the PIDs
    //  By default,
    //      Ignore System Modules
    //      Group By Process
    //      No Group By Thread

    retVal = GetFunctionInfoList(m_profileReader,
                                 static_cast<ProcessIdType>(-1),
                                 static_cast<ThreadIdType>(-1),
                                 flags,
                                 coreMask,          // core mask
                                 m_funcInfoList);

    return retVal;
}


bool CpuProfileReport::GetFunctionData(ProcessIdType pid, ThreadIdType tid, FunctionInfoList& funcInfoList)
{
    bool retVal = false;

    // Ignore/Report system modules.
    gtUInt64 flags = m_args.IsIgnoreSystemModules() ? SAMPLE_IGNORE_SYSTEM_MODULES : 0;
    gtUInt64 coreMask = m_args.GetCoreAffinityMask();

    // Retrieve the function data for all the PIDs
    //  By default,
    //      Ignore System Modules
    //      Group By Process
    //      No Group By Thread

    retVal = GetFunctionInfoList(m_profileReader,
                                 pid,
                                 tid,
                                 flags,
                                 coreMask,          // core mask
                                 funcInfoList);

    return retVal;
}


CGFunctionInfo* GetCSSFuncNode(CGFunctionInfoMap& cgFunctionMap, gtString& funcName, CpuProfileFunction* pFunc, CpuProfileModule* pMod)
{
    CGFunctionInfoMap::iterator pit = cgFunctionMap.find(funcName);
    CGFunctionInfo* pInfo = nullptr;

    if (pit != cgFunctionMap.end())
    {
        pInfo = (*pit).second;
    }
    else
    {
        pInfo = new CGFunctionInfo(funcName, pMod, pFunc);
        cgFunctionMap.insert(CGFunctionInfoMap::value_type(funcName, pInfo));
    }

    return pInfo;
}


bool CpuProfileReport::GetCSSData(ProcessIdType pid, EventMaskType eventSel, CGFunctionInfoMap& cgFunctionMap)
{
    bool retVal = false;
    bool ignoreSystemModuleSamples = m_args.IsIgnoreSystemModules();

    if (nullptr != m_pCss)
    {
        delete m_pCss;
        m_pCss = nullptr;
    }

    // TODO
    // gtString debugSearch = m_args.m_debugSymbolPaths;
    // gtString symDir = m_args.m_symbolServerDirs;
    // gtString symList;

    m_pCss = new CGCallback(&m_profileReader);

    gtString reportFilePath = GetEBPFilePath().fileDirectoryAsString();
    reportFilePath.appendFormattedString(STR_FORMAT, PATH_SEPARATOR);

    retVal = m_pCss->Initialize(reportFilePath, pid);

    // Construct the Callgraph function map
    if (retVal)
    {
        m_pCss->SetEventId(eventSel);

        FunctionGraph& funcGraph = m_pCss->GetFunctionGraph();
        const EventMaskType eventId = m_pCss->GetEventId();

        for (FunctionGraph::const_node_iterator itFunc = funcGraph.GetBeginNode(), itFuncEnd = funcGraph.GetEndNode();
             itFunc != itFuncEnd; ++itFunc)
        {
            const FunctionGraph::Node& funcNode = *itFunc;
            CGFunctionMetaData* pNodeMetadata = static_cast<CGFunctionMetaData*>(funcNode.m_val);

            bool isLeafNode = false;

            for (PathIndexSet::const_iterator it = funcNode.m_pathIndices.begin(), itEnd = funcNode.m_pathIndices.end();
                 it != itEnd; ++it)
            {
                const FunctionGraph::Path& path = *funcGraph.GetPath(*it);
                const LeafFunctionList& leaves = path.GetData();

                // Check if this function found in the path..
                bool isFoundInPath = false;

                for (FunctionGraph::Path::const_iterator itNode = path.begin(), itNodeEnd = path.end(); itNode != itNodeEnd; ++itNode)
                {
                    if (&*itNode == &funcNode)
                    {
                        isFoundInPath = true;
                        break;
                    }
                }

                const FunctionGraph::Node* pPrevLeafNode = nullptr;

                for (LeafFunctionList::const_iterator itLeaf = leaves.begin(), itLeafEnd = leaves.end(); itLeaf != itLeafEnd; ++itLeaf)
                {
                    gtUInt64 selfCount = 0ULL;
                    gtUInt64 deepCount = 0ULL;
                    gtUInt64 pathCount = 0ULL;

                    const LeafFunction& leaf = *itLeaf;
                    const FunctionGraph::Node* pCurLeafNode = leaf.m_pNode;
                    CGFunctionMetaData* pCurLeafNodeMetadata = static_cast<CGFunctionMetaData*>(pCurLeafNode->m_val);

                    // Ignore if the leaf node is from system module
                    if (ignoreSystemModuleSamples
                        && (nullptr != pCurLeafNodeMetadata->m_pModule)
                        && (pCurLeafNodeMetadata->m_pModule->isSystemModule()))
                    {
                        continue;
                    }

                    if (eventId == EventMaskType(-1) || eventId == leaf.m_eventId)
                    {
                        isLeafNode = (&funcNode == leaf.m_pNode) ? true : false;

                        if (isLeafNode && !isFoundInPath)
                        {
                            // Found leaf node
                            selfCount += leaf.m_count;
                            deepCount += leaf.m_count;
                            pathCount++;
                        }

                        if (!isLeafNode && isFoundInPath)
                        {
                            deepCount += leaf.m_count;
                            pathCount++;
                        }

                        // compute the path count
                        if (pPrevLeafNode != leaf.m_pNode)
                        {
                            pPrevLeafNode = leaf.m_pNode;
                        }
                    }

                    // Is leaf node, find parent, otherwise find parent and children
                    const FunctionGraph::Node* pParent = nullptr;
                    const FunctionGraph::Node* pChildren = nullptr;
                    const FunctionGraph::Node* pPrevNode = nullptr;

                    for (FunctionGraph::Path::const_iterator itNode = path.begin(), itNodeEnd = path.end(); itNode != itNodeEnd; ++itNode)
                    {
                        if (!isFoundInPath)
                        {
                            // leaf...
                            pParent = &*itNode;
                            break;
                        }
                        else
                        {
                            if (&*itNode == &funcNode)
                            {
                                pChildren = (nullptr != pPrevNode) ? pPrevNode : pCurLeafNode;
                                pParent = ((++itNode) == itNodeEnd) ? nullptr : &*(itNode);
                                break;
                            }
                        }

                        pPrevNode = &*itNode;
                    }

                    // Add/update leaf node in CGFunctionInfoMap
                    CGFunctionInfo* pSelfNode = nullptr;
                    CGFunctionInfoMap::iterator nit = cgFunctionMap.find(pNodeMetadata->m_funcName);

                    if (nit == cgFunctionMap.end())
                    {
                        pSelfNode = new CGFunctionInfo(pNodeMetadata->m_funcName, pNodeMetadata->m_pModule, pNodeMetadata->m_pFunction);

                        if (isLeafNode)
                        {
                            pSelfNode->m_selfCount += selfCount;
                        }

                        pSelfNode->m_deepCount += deepCount;
                        pSelfNode->m_pathCount += pathCount;
                        cgFunctionMap.insert(CGFunctionInfoMap::value_type(pSelfNode->m_funcName, pSelfNode));
                    }
                    else
                    {
                        pSelfNode = (*nit).second;

                        if (isLeafNode)
                        {
                            pSelfNode->m_selfCount += selfCount;
                        }

                        pSelfNode->m_deepCount += deepCount;
                        pSelfNode->m_pathCount += pathCount;
                    }

                    if (nullptr != pSelfNode)
                    {
                        // Add/update the parent details in the function-node
                        if (nullptr != pParent)
                        {
                            bool foundParent = false;
                            CGFunctionMetaData* parentData = static_cast<CGFunctionMetaData*>(pParent->m_val);

                            for (auto& parent : pSelfNode->m_parents)
                            {
                                if (!parentData->m_funcName.compare(parent.m_pFuncInfo->m_funcName))
                                {
                                    parent.m_deepCount += deepCount;
                                    foundParent = true;
                                }
                            }

                            if (!foundParent)
                            {
                                CGFunctionInfo* pInfo = nullptr;
                                pInfo = GetCSSFuncNode(cgFunctionMap, parentData->m_funcName, parentData->m_pFunction, parentData->m_pModule);

                                if (nullptr != pInfo)
                                {
                                    CGParentChildInfo parentInfo(pInfo);
                                    parentInfo.m_deepCount = deepCount;
                                    pSelfNode->m_parents.push_back(parentInfo);
                                }
                            }
                        }

                        // Add/update the child details in the self function-node
                        if (nullptr != pChildren)
                        {
                            bool foundChild = false;
                            CGFunctionMetaData* childrenData = static_cast<CGFunctionMetaData*>(pChildren->m_val);

                            for (auto& child : pSelfNode->m_children)
                            {
                                if (!childrenData->m_funcName.compare(child.m_pFuncInfo->m_funcName))
                                {
                                    child.m_deepCount += deepCount;
                                    foundChild = true;
                                }
                            }

                            if (!foundChild)
                            {
                                CGFunctionInfo* pInfo = nullptr;
                                pInfo = GetCSSFuncNode(cgFunctionMap, childrenData->m_funcName, childrenData->m_pFunction, childrenData->m_pModule);

                                if (nullptr != pInfo)
                                {
                                    CGParentChildInfo childInfo(pInfo);
                                    childInfo.m_deepCount = deepCount;
                                    pSelfNode->m_children.push_back(childInfo);
                                }
                            }
                        } // add child details
                    }
                } // Iterate over the path
            } // iterate over the leaf nodes
        } // iterate over the FunctionGraph
    }

    return retVal;
}


bool CpuProfileReport::InitProfileEventsNameVec()
{
    bool retVal = true;

    if (m_profileEventsNameVec.size() != 0)
    {
        return true;
    }

    EventEncodeVec eventVec;
    eventVec = m_profileReader.getProfileInfo()->m_eventVec;

    int sortEventIndex = m_args.GetSortEventIndex();

    if ((sortEventIndex >= 0) && static_cast<unsigned int>(sortEventIndex) >= eventVec.size())
    {
        reportError(false, L"Invalid event index(%d) specified with option(-e).\n", sortEventIndex);
        return false;
    }

    // Get the Events to Index Map
    GetEventToIndexMap(m_profileReader, m_evtIndexMap);

    // Event info
    EventEncodeVec::const_iterator eit = eventVec.begin(), eitEnd = eventVec.end();
    gtUInt32 nbrEvents = eventVec.size();
    gtUInt32 nbrCols = (!m_args.IsReportByCore()) ? nbrEvents
                       : nbrEvents * m_coresList.size();
    m_monitoredEventsNameVec.resize(nbrEvents);
    m_profileEventsNameVec.resize(nbrCols);
    m_eventsVec.resize(nbrEvents);

    m_pEventConfig = new EventConfig[nbrEvents];

    int colIdx = 0;

    for (int i = 0; eit != eitEnd; ++eit, ++i)
    {
        // Find the event name and add it to the vector..
        EventEncodeType evtType = *eit;
        gtUInt16 eventSel;
        gtUByte unitMask;
        bool user;
        bool os;

        DecodeEvent(evtType.eventMask, &eventSel, &unitMask, &os, &user);

        // Initialize event config array
        m_pEventConfig[i].eventSelect = eventSel;
        m_pEventConfig[i].eventUnitMask = unitMask;
        m_pEventConfig[i].bitUsr = user;
        m_pEventConfig[i].bitOs = os;

        CpuEvent* pCpuEvent;
        gtString eventName;

        if (m_eventsFile.FindEventByValue(eventSel, &pCpuEvent))
        {
            eventName = convertToGTString(pCpuEvent->name);
        }
        else if (IsTimerEvent(eventSel))
        {
            eventName = L"Timer";
        }
        else
        {
            eventName = L"Unknown";
        }

        // sampling events
        m_monitoredEventsNameVec[i] = eventName;
        m_eventsVec[i] = eventSel;

        // Coloumn headers
        if (m_args.IsReportByCore())
        {
            for (gtList<gtUInt32>::iterator it = m_coresList.begin(); it != m_coresList.end(); it++)
            {
                eventName.appendFormattedString(L" (Core-%d)", (*it));
                m_profileEventsNameVec[i] = eventName;
                colIdx++;
            }
        }
        else
        {
            m_profileEventsNameVec[i] = eventName;
        }
    }

    // set the sort event name
    m_sortEventName = (-1 == sortEventIndex) ? L"All Events" : m_monitoredEventsNameVec[sortEventIndex].asCharArray();

    return retVal;
}


bool CpuProfileReport::InitCoresList()
{
    // Fill-in the m_coresList from the coreAffinity mask
    // If seperate-by-core and core-affinity-mask not used, then report for all the cores
    if (m_args.IsReportByCore())
    {
        gtUInt64 coreMask = m_args.GetCoreAffinityMask();

        if (static_cast<gtUInt64>(-1) == coreMask)
        {
            gtUInt32 nbrCores = m_profileReader.getProfileInfo()->m_numCpus;

            for (gtUInt32 core = 0; core < nbrCores; core++)
            {
                coreMask <<= 1;
                coreMask |= 1;
            }
        }

        gtUInt32 coreId = 0;

        while (coreMask)
        {
            if (coreMask & 0x1)
            {
                m_coresList.push_back(coreId);

                coreMask >>= 1;
                coreId++;
            }
        }
    }

    return true;
}


bool CpuProfileReport::SetCLUViewConfig(ViewConfig& viewCfg)
{
    viewCfg.SetConfigName(QString("CLU"));

    QStringList eventList;

    gtUInt32 nbrEvents = m_monitoredEventsNameVec.size();

    for (gtUInt32 i = 0; i < nbrEvents; i++)
    {
        eventList += QString(m_monitoredEventsNameVec[i].asASCIICharArray());
    }

    // Index of the L1 evictions event
    EventMaskType L1EvictEvent = EncodeEvent(CLU_EVENT_L1_EVICTIONS, 0, true, true);
    EventMaskType accessesEvent = EncodeEvent(CLU_EVENT_ACCESSES, 0, true, true);
    EventMaskType bytesAccessedEvent = EncodeEvent(CLU_EVENT_BYTES_ACCESSED, 0, true, true);
    int evictionsIndex = GetIndexForEvent(m_evtIndexMap, L1EvictEvent);
    int accessesIndex = GetIndexForEvent(m_evtIndexMap, accessesEvent);
    int bytesAccessedIndex = GetIndexForEvent(m_evtIndexMap, bytesAccessedEvent);

    if (nbrEvents > 0)
    {
        ColumnSpec* pColumnSpec = new ColumnSpec[nbrEvents];

        for (gtUInt32 i = 0; i < nbrEvents; i++)
        {
            pColumnSpec[i].type = ColumnValue;
            pColumnSpec[i].sorting = NoSort;
            pColumnSpec[i].visible = true;
            pColumnSpec[i].dataSelectLeft = m_pEventConfig[i];
            pColumnSpec[i].dataSelectRight.eventSelect = 0;
            pColumnSpec[i].dataSelectRight.eventUnitMask = 0;
            pColumnSpec[i].dataSelectRight.bitOs = 0;
            pColumnSpec[i].dataSelectRight.bitUsr = 0;
            pColumnSpec[i].title = m_monitoredEventsNameVec[i].asASCIICharArray();

            if (CLU_EVENT_CLU_PERCENTAGE == m_eventsVec[i])
            {
                pColumnSpec[i].dataSelectRight = m_pEventConfig[evictionsIndex];
                pColumnSpec[i].type = ColumnPercentage;
            }
            else if (CLU_EVENT_BYTES_PER_L1_EVICTION == m_eventsVec[i])
            {
                pColumnSpec[i].dataSelectLeft = m_pEventConfig[bytesAccessedIndex];
                pColumnSpec[i].dataSelectRight = m_pEventConfig[evictionsIndex];
                pColumnSpec[i].type = ColumnRatio;
            }
            else if (CLU_EVENT_ACCESSES_PER_L1_EVICTION == m_eventsVec[i])
            {
                pColumnSpec[i].dataSelectLeft = m_pEventConfig[accessesIndex];
                pColumnSpec[i].dataSelectRight = m_pEventConfig[evictionsIndex];
                pColumnSpec[i].type = ColumnRatio;
            }
        }

        viewCfg.SetColumnSpecs(pColumnSpec, nbrEvents, false);
        viewCfg.SetDescription("This special view has all of the data from the profile available.");

        if (nullptr != pColumnSpec)
        {
            delete[] pColumnSpec;
        }
    }

    return true;
}


bool CpuProfileReport::SetAllDataViewConfig(ViewConfig& viewCfg)
{
    viewCfg.SetConfigName(QString("All Data"));
    QStringList eventList;

    gtUInt32 nbrEvents = m_monitoredEventsNameVec.size();

    for (gtUInt32 i = 0; i < nbrEvents; i++)
    {
        eventList += QString(m_monitoredEventsNameVec[i].asASCIICharArray());
    }

    // Make column specifications
    viewCfg.MakeColumnSpecs(nbrEvents, m_pEventConfig, eventList);
    viewCfg.SetDescription("This special view has all of the data from the profile available.");

    return true;
}


void CpuProfileReport::InitializeViewData()
{
    int sortEventIndex = m_args.GetSortEventIndex() == -1 ? 0 : m_args.GetSortEventIndex();

    if (m_viewConfigXMLPath.isEmpty())
    {
        if (m_profileReader.getProfileInfo()->m_isProfilingCLU)
        {
            SetCLUViewConfig(m_viewConfig);
            SetCLUViewConfig(m_overviewConfig);

            m_pReporter->SetCLU(true);
            m_pReporter->SetSortOrder(Reporter::ASCENDING_ORDER);
            sortEventIndex = 0;
        }
        else
        {
            // All data
            SetAllDataViewConfig(m_viewConfig);
            SetAllDataViewConfig(m_overviewConfig);
        }
    }
    else
    {
        m_viewConfig.ReadConfigFile(convertToQString(m_viewConfigXMLPath.asString()));
        SetAllDataViewConfig(m_overviewConfig);
    }

    m_pReporter->SetSortEventIndex(sortEventIndex);
}
