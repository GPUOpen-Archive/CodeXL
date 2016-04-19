//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file contains the function to analyze profile/trace output
//==============================================================================

#include <iostream>
#include "Analyze.h"
#include "AtpFile.h"
#include "../CLTraceAgent/CLAtpFile.h"
#include "../HSAFdnTrace/HSAAtpFile.h"
#include "../sanalyze/HSAKernelSummarizer.h"
#include "../sanalyze/APISummarizer.h"
#include "../sanalyze/CLKernelSummarizer.h"
#include "../sanalyze/CLMemSummarizer.h"
#include "../sanalyze/CLContextSummarizer.h"
#include "../sanalyze/CLAPIAnalyzer.h"
#include "../sanalyze/CLObjRefTracker.h"
#include "../sanalyze/CLRetCodeAnalyzer.h"
#include "../sanalyze/CLDeprecatedFunctionAnalyzer.h"
#include "../sanalyze/CLAPIRules.h"
#include "../sanalyze/CLDataTransferAnalyzer.h"
#include "../sanalyze/CLSyncAnalyzer.h"
#include "../sanalyze/HSAAPIAnalyzer.h"
#include "../sanalyze/HSARetCodeAnalyzer.h"
#include "../sanalyze/HSAObjRefTracker.h"
#include "../Common/FileUtils.h"
#include "../Common/HTMLTable.h"

using namespace std;

bool APITraceAnalyze(const Config& config)
{
    AtpFileParser parser;
    CLAtpFilePart clFile(config);
    parser.AddAtpFilePart(&clFile);

    HSAAtpFilePart hsaTrace(config);
    parser.AddAtpFilePart(&hsaTrace);

    HSAAPISummarizer hsaApiSum;
    HSAKernelSummarizer hsaKernelSum;

    CLAPISummarizer clApiSum;
    CLKernelSummarizer clKernelSum;
    CLMemSummarizer clMemSum;
    CLContextSummarizer clContextsum;

    string strWorkingDir;

    parser.LoadFile(config.analyzeOps.strAtpFile.c_str());
    FileUtils::GetWorkingDirectory(config.analyzeOps.strAtpFile, strWorkingDir);
    strWorkingDir += "/";


    string atpName = config.analyzeOps.strAtpFile;
    size_t idx0 = atpName.find_last_of("/\\");

    if (idx0 >= 0)
    {
        atpName = atpName.substr(idx0 + 1);
    }

    size_t idx1 = atpName.find_last_of(".");

    if (idx1 >= 0)
    {
        atpName = atpName.substr(0, idx1);
    }

    string filePrefix = strWorkingDir + atpName + '.';

    if (config.analyzeOps.bAPISummary)
    {
        clFile.AddListener(&clApiSum);
        hsaTrace.AddListener(&hsaApiSum);
    }

    if (config.analyzeOps.bTop10KernelSummary || config.analyzeOps.bKernelSummary)
    {
        clFile.AddListener(&clKernelSum);
        hsaTrace.AddListener(&hsaKernelSum);
    }

    if (config.analyzeOps.bTop10DataTransferSummary)
    {
        clFile.AddListener(&clMemSum);
    }

    if (config.analyzeOps.bContextSummary)
    {
        clFile.AddListener(&clContextsum);
    }

    // OpenCL analyzers
    CLAPIAnalyzerManager clAnalyzerMgr;
    CLObjRefTracker refTracker(&clAnalyzerMgr);
    CLRetCodeAnalyzer retCodeAnalyzer(&clAnalyzerMgr);
    CLDeprecatedFunctionAnalyzer deprecatedFunctionAnalyzer(&clAnalyzerMgr);
    CLDataTransferAnalyzer datTransfterAnalyzer(&clAnalyzerMgr);
    SimpleCLAPIRuleManager* ruleMgr = GetSimpleCLAPIRuleManager();
    CLSyncAnalyzer syncAnalyzer(&clAnalyzerMgr);
    clAnalyzerMgr.AddAnalyzer(&refTracker);
    clAnalyzerMgr.AddAnalyzer(ruleMgr);
    clAnalyzerMgr.AddAnalyzer(&retCodeAnalyzer);
    clAnalyzerMgr.AddAnalyzer(&deprecatedFunctionAnalyzer);
    clAnalyzerMgr.AddAnalyzer(&datTransfterAnalyzer);
    clAnalyzerMgr.AddAnalyzer(&syncAnalyzer);

    bool bEnableAnalyzer = clAnalyzerMgr.SetEnable(config.analyzeOps);

    if (bEnableAnalyzer)
    {
        clFile.AddListener(&clAnalyzerMgr);
    }

    // HSA analyzers
    HSAAPIAnalyzerManager hsaAnalyzerMgr;
    HSARetCodeAnalyzer hsaRetCodeAnalyzer;
    HSAObjRefTracker hsaObjRefTracker;
    hsaAnalyzerMgr.AddAnalyzer(&hsaRetCodeAnalyzer);
    hsaAnalyzerMgr.AddAnalyzer(&hsaObjRefTracker);

    bEnableAnalyzer = hsaAnalyzerMgr.SetEnable(config.analyzeOps);

    if (bEnableAnalyzer)
    {
        hsaTrace.AddListener(&hsaAnalyzerMgr);
    }

    cout << "Generating summary pages...\n";
    cout << "Parsing API trace file...\n";
    bool parseRet = parser.Parse();
    bool bWarning;
    string strMsg;
    parser.GetParseWarning(bWarning, strMsg);

    if (!(parseRet || bWarning))
    {
        cout << "Failed to parse .atp file.\n";
    }

    if (bWarning)
    {
        cout << strMsg << endl;
    }

    bool anySummaryPageShouldBeGenerated = false;
    bool summaryPagesGenerated = false;

    if (config.analyzeOps.bContextSummary)
    {
        summaryPagesGenerated |= clContextsum.GenerateHTMLPage((filePrefix + CLCTX_SUM).c_str());
        anySummaryPageShouldBeGenerated = true;
    }

    if (config.analyzeOps.bAPISummary)
    {
        summaryPagesGenerated |= clApiSum.GenerateHTMLPage((filePrefix + CLAPI_SUM).c_str());
        summaryPagesGenerated |= hsaApiSum.GenerateHTMLPage((filePrefix + HSAAPI_SUM).c_str());
        anySummaryPageShouldBeGenerated = true;
    }

    if (config.analyzeOps.bTop10KernelSummary)
    {
        summaryPagesGenerated |= clKernelSum.GenerateTopXKernelHTMLPage((filePrefix + CLTOP10_KERNEL).c_str());
        summaryPagesGenerated |= hsaKernelSum.GenerateTopXKernelHTMLPage((filePrefix + HSATOP10_KERNEL).c_str());
        anySummaryPageShouldBeGenerated = true;
    }

    if (config.analyzeOps.bKernelSummary)
    {
        summaryPagesGenerated |= clKernelSum.GenerateKernelSummaryHTMLPage((filePrefix + CLKERNEL_SUM).c_str());
        summaryPagesGenerated |= hsaKernelSum.GenerateKernelSummaryHTMLPage((filePrefix + HSAKERNEL_SUM).c_str());
        anySummaryPageShouldBeGenerated = true;
    }

    if (config.analyzeOps.bTop10DataTransferSummary)
    {
        summaryPagesGenerated |= clMemSum.GenerateHTMLPage((filePrefix + CLTOP10_DATA).c_str());
        anySummaryPageShouldBeGenerated = true;
    }

    if (anySummaryPageShouldBeGenerated && !summaryPagesGenerated)
    {
        cout << "No summary pages generated\n";
    }

    if (bEnableAnalyzer)
    {
        clAnalyzerMgr.GenerateHTMLPage((filePrefix + CLBEST_PRACTICES).c_str());
        hsaAnalyzerMgr.GenerateHTMLPage((filePrefix + HSABEST_PRACTICES).c_str());
    }

    parser.Close();

    return true;
}
