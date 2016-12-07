//==================================================================================
// Copyright (c) 2014-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file Reporter.h
/// \brief This is Command Line Utility for CPU profiling.
///
//==================================================================================

#ifndef _CPUPROFILE_REPORTER_H_
#define _CPUPROFILE_REPORTER_H_
#pragma once

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osTime.h>

// Backend:
#include <AMDTCommonHeaders/AMDTCommonProfileDataTypes.h>

// Project:
#include <Utils.h>

class Reporter
{
public:
    enum SortOrder
    {
        ASCENDING_ORDER,
        DESCENDING_ORDER
    };

    HRESULT      m_error;
    SortOrder    m_sortOrder;    // Order in which the data will be reported
    int          m_sortEventIndex;
    osFilePath   m_reportFilePath;  // output report
    osFile       m_reportFile;

    Reporter(osFilePath& filePath) : m_error(S_OK),
        m_sortOrder(DESCENDING_ORDER),
        m_sortEventIndex(-1),
        m_reportFilePath(filePath)
    {
    }

    virtual ~Reporter() { };

    bool Open();
    bool Close();
    bool IsOpened();

    void SetSortOrder(SortOrder sortOrder);
    void SetSortEventIndex(int sortEvent);

    virtual bool ReportExecution(gtVector<gtString> sectionHdrs, gtList<std::pair<gtString, gtString> > sectionData) = 0;
    virtual bool ReportProfileData(gtVector<gtString> sectionHdrs, gtList<std::pair<gtString, gtString> > sectionData) = 0;
    virtual bool ReportSamplingSpec(gtVector<gtString>&           sectionHdrs,
                                    AMDTProfileCounterDescVec&    counters,
                                    AMDTProfileSamplingConfigVec& samplingConfig) = 0;

    // OVERVIEW Reporters
    virtual bool WriteOverviewProcess(gtVector<gtString>& sectionHdrs,
        AMDTProfileDataVec& processProfileData,
        bool                 showPerc) = 0;

    virtual bool WriteOverviewModule(gtVector<gtString>& sectionHdrs,
        AMDTProfileDataVec& moduleProfileData,
        bool                 showPerc) = 0;

    virtual bool WriteOverviewFunction(gtVector<gtString>&   sectionHdrs,
                                       AMDTProfileDataVec&  funcProfileData,
                                       bool                 showPerc) = 0;

    // PROCESS Specific reporters
    virtual bool WritePidSummary(gtVector<gtString>&  sectionHdrs,
        const AMDTProfileData&      procInfo,
        bool                 showPerc,
        bool                 sepByCore) = 0;

    virtual bool WritePidModuleSummary(gtVector<gtString>&  sectionHdrs,
        AMDTProfileDataVec&      modList,
        bool                  showPerc,
        bool                  sepByCore) = 0;

     virtual bool WritePidFunctionSummary(gtVector<gtString>&   sectionHdrs,
        AMDTProfileDataVec& funcList,
        bool                 showPerc,
        bool                 sepByCore) = 0;

    virtual bool WriteCallGraphFunctionSummary(gtVector<gtString>        sectionHdrs,
                                               AMDTCallGraphFunctionVec& cgFuncsVec,
                                               bool                      showPerc) = 0;

    virtual bool WriteCallGraph(const AMDTCallGraphFunction&  self,
                                AMDTCallGraphFunctionVec&     caller,
                                AMDTCallGraphFunctionVec&     callee,
                                bool                          showPerc) = 0;

    virtual bool WriteCallGraphHdr(gtVector<gtString>  sectionHdrs) = 0;

    // IMIX Specific reporters
#ifdef AMDT_CPCLI_ENABLE_IMIX
    virtual bool WriteImixSummaryInfo(gtVector<gtString>   sectionHdrs,
                                      ImixSummaryMap&      imixSummaryMap,
                                      gtUInt64             totalSamples) = 0;

    virtual bool WriteImixInfo(gtVector<gtString>   sectionHdrs,
                               ModuleImixInfoList&  modImixInfoList,
                               gtUInt64             totalSamples) = 0;
#endif // AMDT_CPCLI_ENABLE_IMIX
};

class CSVReporter : public Reporter
{
public:
    CSVReporter(osFilePath& filePath) : Reporter(filePath)
    {
    }

    ~CSVReporter() { };

    bool ReportExecution(gtVector<gtString> sectionHdrs, gtList<std::pair<gtString, gtString> > sectionData);
    bool ReportProfileData(gtVector<gtString> sectionHdrs, gtList<std::pair<gtString, gtString> > sectionData);
    bool ReportSamplingSpec(gtVector<gtString>& sectionHdrs,
                            AMDTProfileCounterDescVec& counters,
                            AMDTProfileSamplingConfigVec& samplingConfig);

    // OVERVIEW reporters
    bool WriteOverviewProcess(
        gtVector<gtString>& sectionHdrs,
        AMDTProfileDataVec& processProfileData,
        bool                showPerc);

    bool WriteOverviewModule(
        gtVector<gtString>& sectionHdrs,
        AMDTProfileDataVec& moduleProfileData,
        bool                showPerc);

    bool WriteOverviewFunction(
        gtVector<gtString>&  sectionHdrs,
        AMDTProfileDataVec&  funcProfileData,
        bool                 showPerc);

    // PROCESS reporters
    bool WritePidSummary(
        gtVector<gtString>&    sectionHdrs,
        const AMDTProfileData& procInfo,
        bool                   showPerc,
        bool                   sepByCore);

    bool WritePidModuleSummary(
        gtVector<gtString>&  sectionHdrs,
        AMDTProfileDataVec&  modList,
        bool                 showPerc,
        bool                 sepByCore);

    bool WritePidFunctionSummary(
        gtVector<gtString>& sectionHdrs,
        AMDTProfileDataVec& funcList,
        bool                showPerc,
        bool                sepByCore);

    bool WriteCallGraphFunctionSummary(gtVector<gtString>        sectionHdrs,
                                       AMDTCallGraphFunctionVec& cgFuncsVec,
                                       bool                      showPerc);

    bool WriteCallGraph(const AMDTCallGraphFunction&     self,
                        AMDTCallGraphFunctionVec&  caller,
                        AMDTCallGraphFunctionVec&  callee,
                        bool                       showPerc);

    bool WriteCallGraphHdr(gtVector<gtString>  sectionHdrs);

#ifdef AMDT_CPCLI_ENABLE_IMIX
    bool WriteImixSummaryInfo(gtVector<gtString>   sectionHdrs,
                              ImixSummaryMap&      imixSummaryMap,
                              gtUInt64             totalSamples);

    bool WriteImixInfo(gtVector<gtString>   sectionHdrs,
                       ModuleImixInfoList&  modImixInfoList,
                       gtUInt64             totalSamples);
#endif // AMDT_CPCLI_ENABLE_IMIX

private:
    void WriteSectionHeaders(gtVector<gtString>& sectionHdrs);
};

#endif // #ifndef _CPUPROFILE_REPORTER_H_