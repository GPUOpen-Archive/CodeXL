//==================================================================================
// Copyright (c) 2014-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file Reporter.cpp
/// \brief This is Command Line Utility for CPU profiling.
///
//==================================================================================

// STL
#include <map>

// Infra:
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osMachine.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtVector.h>

// Backend
#include <AMDTCpuPerfEventUtils/inc/EventEngine.h>

// Project:
#include <Reporter.h>
#include <CommonUtils.h>

//
// Helper Functions
//
inline static void AddPercentageValue(gtString& dataStr, float leftData, float rightData)
{
    float floatVal = 0.0;

    if (leftData && rightData)
    {
        floatVal = (leftData / rightData) * static_cast<float>(100.0);
    }

    dataStr.appendFormattedString(L" (%3.02f%%)", floatVal);
}

//
//  Reporter
//

bool Reporter::Open()
{
    bool retVal = false;
    m_error = E_FAIL;

    if (!m_reportFilePath.isEmpty())
    {
        retVal = m_reportFile.open(m_reportFilePath, osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);
        m_error = S_OK;
    }

    return retVal;
}

bool Reporter::Close()
{
    m_reportFile.close();
    return true;
}

bool Reporter::IsOpened()
{
    return m_reportFile.isOpened();
}

void Reporter::SetSortOrder(SortOrder sortOrder)
{
    m_sortOrder = sortOrder;
    return;
}

void Reporter::SetSortEventIndex(int sortEventIndex)
{
    m_sortEventIndex = sortEventIndex;
    return;
}

bool CSVReporter::ReportExecution(gtVector<gtString> sectionHdrs, gtList<std::pair<gtString, gtString> > sectionData)
{
    WriteSectionHeaders(sectionHdrs);

    gtList<std::pair<gtString, gtString> >::iterator iter = sectionData.begin();
    gtList<std::pair<gtString, gtString> >::iterator iterEnd = sectionData.end();

    for (; iter != iterEnd; iter++)
    {
        std::pair<gtString, gtString> keyValue;
        gtString dataStr(L"\n");

        keyValue = (*iter);

        if (! keyValue.first.isEmpty())
        {
            dataStr.appendFormattedString(STR_FORMAT, keyValue.first.asCharArray());
        }

        if (! keyValue.second.isEmpty())
        {
            dataStr.appendFormattedString(L"," DQ_STR_FORMAT, keyValue.second.asCharArray());
        }

        m_reportFile.writeString(dataStr);
    }

    // Print a blank line at the end of the section
    m_reportFile.writeString(L"\n");

    return true;
}

bool CSVReporter::ReportProfileData(gtVector<gtString> sectionHdrs, gtList<std::pair<gtString, gtString> > sectionData)
{
    WriteSectionHeaders(sectionHdrs);

    gtList<std::pair<gtString, gtString> >::iterator iter = sectionData.begin();
    gtList<std::pair<gtString, gtString> >::iterator iterEnd = sectionData.end();

    for (; iter != iterEnd; iter++)
    {
        std::pair<gtString, gtString> keyValue;
        gtString dataStr(L"\n");

        keyValue = (*iter);

        if (! keyValue.first.isEmpty())
        {
            dataStr.appendFormattedString(STR_FORMAT, keyValue.first.asCharArray());
        }

        if (! keyValue.second.isEmpty())
        {
            dataStr.appendFormattedString(L"," DQ_STR_FORMAT, keyValue.second.asCharArray());
        }

        m_reportFile.writeString(dataStr);
    }

    // Print a blank line at the end of the section
    m_reportFile.writeString(L"\n");

    return true;
}

bool CSVReporter::ReportSamplingSpec(
    gtVector<gtString>& sectionHdrs,
    AMDTProfileCounterDescVec& counters,
    AMDTProfileSamplingConfigVec& samplingConfig)
{
    WriteSectionHeaders(sectionHdrs);
    int idx = 0;

    for (auto& counter : counters)
    {
        gtString sampSpec(L"\n");

        if (IsPmcEvent(static_cast<gtUInt16>(samplingConfig[idx].m_hwEventId)))
        {
            sampSpec.appendFormattedString(L",\"" STR_FORMAT L" (0x%x)\"", counter.m_name.asCharArray(), samplingConfig[idx].m_hwEventId);
        }
        else
        {
            sampSpec.appendFormattedString(L",\"" STR_FORMAT L"\"", counter.m_name.asCharArray());
        }

        sampSpec.appendFormattedString(L",%d", samplingConfig[idx].m_samplingInterval);
        sampSpec.appendFormattedString(L",%x", samplingConfig[idx].m_unitMask);
        sampSpec.appendFormattedString(L"," STR_FORMAT, samplingConfig[idx].m_userMode ? L"True" : L"False");
        sampSpec.appendFormattedString(L"," STR_FORMAT, samplingConfig[idx].m_osMode ? L"True" : L"False");

        m_reportFile.writeString(sampSpec);
        ++idx;
    }

    // Print a blank line at the end of the section
    m_reportFile.writeString(L"\n");
    return true;
}

bool CSVReporter::WriteOverviewFunction(
    gtVector<gtString>&   sectionHdrs,
    AMDTProfileDataVec&   funcProfileData,
    bool                  showPerc)
{
    WriteSectionHeaders(sectionHdrs);
    m_reportFile.writeString(L"\n");

    int count = 1;

    for (const auto& funcInfo : funcProfileData)
    {
        if (count > 5)
        {
            break;
        }

        gtString rowStr;
        rowStr.appendFormattedString(L"%ls", funcInfo.m_name.asCharArray());

        for (const auto& ev : funcInfo.m_sampleValue)
        {
            if (showPerc)
            {
                rowStr.appendFormattedString(L",%0.2f%%", ev.m_sampleCountPercentage);
            }
            else
            {
                rowStr.appendFormattedString(L",%u", static_cast<unsigned int>(ev.m_sampleCount));
            }
        }

        m_reportFile.writeString(rowStr);
        m_reportFile.writeString(L"\n");
        ++count;
    }

    // Print a blank line at the end of the section
    m_reportFile.writeString(L"\n");
    return true;
}

bool CSVReporter::WriteOverviewProcess(
    gtVector<gtString>& sectionHdrs,
    AMDTProfileDataVec& processProfileData,
    bool                showPerc)
{
    WriteSectionHeaders(sectionHdrs);
    m_reportFile.writeString(L"\n");

    for (const auto& proc : processProfileData)
    {
        gtString rowStr;
        rowStr.appendFormattedString(L"%ls (PID - %u)", proc.m_name.asCharArray(), proc.m_id);

        for (const auto& sample : proc.m_sampleValue)
        {
            if (showPerc)
            {
                rowStr.appendFormattedString(L",%0.2f%%", sample.m_sampleCountPercentage);
            }
            else
            {
                rowStr.appendFormattedString(L",%u", static_cast<unsigned>(sample.m_sampleCount));
            }
        }

        m_reportFile.writeString(rowStr);
        m_reportFile.writeString(L"\n");
    }

    m_reportFile.writeString(L"\n");
    return true;
}

bool CSVReporter::WriteOverviewModule(
    gtVector<gtString>& sectionHdrs,
    AMDTProfileDataVec& moduleProfileData,
    bool                showPerc)
{
    WriteSectionHeaders(sectionHdrs);
    m_reportFile.writeString(L"\n");
    int count = 1;

    for (const auto& module : moduleProfileData)
    {
        if (count > 5)
        {
            break;
        }

        gtString rowStr;
        rowStr.appendFormattedString(L"%ls", module.m_name.asCharArray());

        for (const auto& sample : module.m_sampleValue)
        {
            if (showPerc)
            {
                rowStr.appendFormattedString(L",%0.2f%%", sample.m_sampleCountPercentage);
            }
            else
            {
                rowStr.appendFormattedString(L",%u", static_cast<unsigned>(sample.m_sampleCount));
            }
        }

        m_reportFile.writeString(rowStr);
        m_reportFile.writeString(L"\n");
        ++count;
    }

    m_reportFile.writeString(L"\n");
    return true;
}

bool CSVReporter::WritePidSummary(
    gtVector<gtString>&    sectionHdrs,
    const AMDTProfileData& procInfo,
    bool                   showPerc,
    bool                   sepByCore)
{
    //TODO: use sepByCore while generating report
    GT_UNREFERENCED_PARAMETER(sepByCore);

    WriteSectionHeaders(sectionHdrs);
    m_reportFile.writeString(L"\n");

    gtString rowStr;
    rowStr.appendFormattedString(L"%ls", procInfo.m_name.asCharArray());

    for (const auto& sample : procInfo.m_sampleValue)
    {
        if (showPerc)
        {
            rowStr.appendFormattedString(L",%0.2f%%", sample.m_sampleCountPercentage);
        }
        else
        {
            rowStr.appendFormattedString(L",%u", static_cast<unsigned>(sample.m_sampleCount));
        }
    }

    m_reportFile.writeString(rowStr);

    // Print a blank line at the end of the section
    m_reportFile.writeString(L"\n");
    return true;
}

bool CSVReporter::WritePidModuleSummary(gtVector<gtString>&  sectionHdrs,
    AMDTProfileDataVec&      modList,
    bool                  showPerc,
    bool                  sepByCore)
{
    //TODO: use sepByCore while generating report
    GT_UNREFERENCED_PARAMETER(sepByCore);

    WriteSectionHeaders(sectionHdrs);
    m_reportFile.writeString(L"\n");

    for (const auto& modInfo : modList)
    {
        gtString rowStr;
        rowStr.appendFormattedString(L"%ls", modInfo.m_name.asCharArray());

        for (const auto& sample : modInfo.m_sampleValue)
        {
            if (showPerc)
            {
                rowStr.appendFormattedString(L",%0.2f%%", sample.m_sampleCountPercentage);
            }
            else
            {
                rowStr.appendFormattedString(L",%u", static_cast<unsigned>(sample.m_sampleCount));
            }
        }

        m_reportFile.writeString(rowStr);
        m_reportFile.writeString(L"\n");
    }

    // Print a blank line at the end of the section
    m_reportFile.writeString(L"\n");
    return true;
}

bool CSVReporter::WritePidFunctionSummary(gtVector<gtString>&   sectionHdrs,
    AMDTProfileDataVec& funcList,
    bool                 showPerc,
    bool                 sepByCore)
{
    //TODO: use sepByCore while generating report
    GT_UNREFERENCED_PARAMETER(sepByCore);

    WriteSectionHeaders(sectionHdrs);
    m_reportFile.writeString(L"\n");

    for (const auto& funcInfo : funcList)
    {
        gtString rowStr;
        rowStr.appendFormattedString(L"%ls", funcInfo.m_name.asCharArray());

        for (const auto& sample : funcInfo.m_sampleValue)
        {
            if (showPerc)
            {
                rowStr.appendFormattedString(L",%0.2f%%", sample.m_sampleCountPercentage);
            }
            else
            {
                rowStr.appendFormattedString(L",%u", static_cast<unsigned>(sample.m_sampleCount));
            }
        }

        m_reportFile.writeString(rowStr);
        m_reportFile.writeString(L"\n");
    }

    // Print a blank line at the end of the section
    m_reportFile.writeString(L"\n");
    return true;
}

bool CSVReporter::WriteCallGraphFunctionSummary(
    gtVector<gtString>        sectionHdrs,
    AMDTCallGraphFunctionVec& cgFuncsVec,
    bool                      showPerc)
{
    bool retVal = false;

    WriteSectionHeaders(sectionHdrs);

    for (const auto& cgFunc : cgFuncsVec)
    {
        if (cgFunc.m_totalDeepSamples > 0)
        {
            gtString tmpStr(L"\n");
            tmpStr += L"\"";
            tmpStr += cgFunc.m_functionInfo.m_name;
            tmpStr += L"\"";
            tmpStr.appendFormattedString(L",%u", static_cast<unsigned>(cgFunc.m_totalSelfSamples));
            tmpStr.appendFormattedString(L",%u", static_cast<unsigned>(cgFunc.m_totalDeepSamples));

            if (showPerc)
            {
                // tmpStr.appendFormattedString(L" (%3.02f%%)", cgFunc.m_deepSamplesPerc);
                tmpStr.appendFormattedString(L",%3.02f%%", cgFunc.m_deepSamplesPerc);
            }

            tmpStr.appendFormattedString(L",%u", cgFunc.m_pathCount);

            tmpStr += L",";
            tmpStr += cgFunc.m_srcFile;

            tmpStr += L",";
            tmpStr += cgFunc.m_functionInfo.m_modulePath;

            m_reportFile.writeString(tmpStr);

            retVal = true;
        }
    }

    m_reportFile.writeString(L"\n");

    return retVal;
}

bool CSVReporter::WriteCallGraphHdr(gtVector<gtString>  sectionHdrs)
{
    WriteSectionHeaders(sectionHdrs);
    m_reportFile.writeString(L"\n");

    return true;
}

bool CSVReporter::WriteCallGraph(const AMDTCallGraphFunction&     self,
                                 AMDTCallGraphFunctionVec&  caller,
                                 AMDTCallGraphFunctionVec&  callee,
                                 bool                       showPerc)
{
    (void)(showPerc);
    bool retVal = true;

    // Write the Parents/Callers
    for (const auto& cgFunc : caller)
    {
        if (cgFunc.m_totalDeepSamples > 0)
        {
            gtString tmpStr(L"\n");
            tmpStr += L",,";
            tmpStr.appendFormattedString(L"                %u", static_cast<unsigned>(cgFunc.m_totalDeepSamples));
            tmpStr.appendFormattedString(L" (%3.02f%%)", cgFunc.m_deepSamplesPerc);
            tmpStr += L",";
            tmpStr += L"\"                ";
            tmpStr += cgFunc.m_functionInfo.m_name;
            tmpStr += L"\"";

            tmpStr += L",";
            tmpStr += cgFunc.m_functionInfo.m_modulePath;

            m_reportFile.writeString(tmpStr);
        }
    }

    // Print Self
    if (self.m_totalDeepSamples > 0)
    {
        gtString tmpStr(L"\n");

        float selfSamplePerc = 0.0;

        if (self.m_totalSelfSamples && self.m_totalDeepSamples)
        {
            selfSamplePerc = (static_cast<float>(self.m_totalSelfSamples) / static_cast<float>(self.m_totalDeepSamples)) * static_cast<float>(100.0);
        }

        tmpStr += L",";
        tmpStr.appendFormattedString(L"%u", static_cast<unsigned>(self.m_totalDeepSamples));
        tmpStr.appendFormattedString(L" (%3.02f%%)", self.m_deepSamplesPerc);
        tmpStr += L",";
        tmpStr.appendFormattedString(L"%u", static_cast<unsigned>(self.m_totalSelfSamples));
        tmpStr.appendFormattedString(L" (%3.02f%%)", selfSamplePerc);

        tmpStr += L",";
        tmpStr += L"\"";
        tmpStr += self.m_functionInfo.m_name;
        tmpStr += L"\"";

        tmpStr += L",";
        tmpStr += self.m_functionInfo.m_modulePath;

        tmpStr += L",";
        tmpStr += self.m_srcFile;

        m_reportFile.writeString(tmpStr);
    }

    // Write the Children/Callees
    for (const auto& cgFunc : callee)
    {
        if (cgFunc.m_functionInfo.m_name.compare(L"[self]") && cgFunc.m_totalDeepSamples > 0)
        {
            gtString tmpStr(L"\n");
            tmpStr += L",,";
            tmpStr.appendFormattedString(L"                %u", static_cast<unsigned>(cgFunc.m_totalDeepSamples));
            tmpStr.appendFormattedString(L" (%3.02f%%)", cgFunc.m_deepSamplesPerc);

            tmpStr += L",";
            tmpStr += L"\"                ";
            tmpStr += cgFunc.m_functionInfo.m_name;
            tmpStr += L"\"";

            tmpStr += L",";
            tmpStr += cgFunc.m_functionInfo.m_modulePath;

            m_reportFile.writeString(tmpStr);
        }
    }

    m_reportFile.writeString(L"\n");

    return retVal;
}

#ifdef AMDT_CPCLI_ENABLE_IMIX
bool CSVReporter::WriteImixSummaryInfo(gtVector<gtString>   sectionHdrs,
                                       ImixSummaryMap&      imixSummaryMap,
                                       gtUInt64             totalSamples)
{
    WriteSectionHeaders(sectionHdrs);

    // Sort the map by samples count ascending order
    std::multimap<gtUInt64, const std::string> sortedImixSummaryMap;

    for (auto it : imixSummaryMap)
    {
        sortedImixSummaryMap.insert({it.second, it.first});
    }

    // Print sorted summary map in descending order
    for (auto it = sortedImixSummaryMap.rbegin(); it != sortedImixSummaryMap.rend(); ++it)
    {
        gtString tmpString;
        gtString inst;

        inst.fromASCIIString(it->second.c_str());
        tmpString.append(L"\"").append(inst).append(L"\"");

        float floatVal = 0.0;
        float leftData = static_cast<float>(it->first);
        float rightData = static_cast<float>(totalSamples);

        if (leftData && rightData)
        {
            floatVal = (leftData / rightData) * static_cast<float>(100.0);
        }

        tmpString.appendFormattedString(L",%3.02f", floatVal);
        tmpString.appendFormattedString(L",%llu,", it->first);

        m_reportFile.writeString(tmpString);
        m_reportFile.writeString(L"\n");
    }

    m_reportFile.writeString(L"\n");

    return true;
}

bool CSVReporter::WriteImixInfo(gtVector<gtString>   sectionHdrs,
                                ModuleImixInfoList&  modImixInfoList,
                                gtUInt64             totalSamples)
{
    GT_UNREFERENCED_PARAMETER(totalSamples);

    WriteSectionHeaders(sectionHdrs);

    for (auto& mit : modImixInfoList)
    {
        gtString moduleHdr;

        moduleHdr.appendFormattedString(L"Module Path = %ls\n", mit.m_pModule->getPath().asCharArray());
        moduleHdr.appendFormattedString(L"Module Samples = %llu\n", mit.m_samplesCount);

        moduleHdr.appendFormattedString(L"Disassembly");
        moduleHdr.appendFormattedString(L",Samples Percentage");
        moduleHdr.appendFormattedString(L",Samples Count\n");

        m_reportFile.writeString(moduleHdr);

        // Sort the map by samples count ascending order
        std::multimap<gtUInt64, const std::string> sortedImixMap;

        for (auto sit : mit.m_InstMap)
        {
            sortedImixMap.insert({ sit.second, sit.first });
        }

        // Print sorted summary map in descending order
        for (auto it = sortedImixMap.rbegin(); it != sortedImixMap.rend(); ++it)
        {
            gtString tmpString;
            gtString inst;

            inst.fromASCIIString(it->second.c_str());
            tmpString.append(L"\"").append(inst).append(L"\"");

            float floatVal = 0.0;
            float leftData = static_cast<float>(it->first);
            float rightData = static_cast<float>(mit.m_samplesCount);

            if (leftData && rightData)
            {
                floatVal = (leftData / rightData) * 100.0;
            }

            tmpString.appendFormattedString(L",%3.02f", floatVal);
            tmpString.appendFormattedString(L",%llu,", it->first);

            m_reportFile.writeString(tmpString);
            m_reportFile.writeString(L"\n");
        }

        m_reportFile.writeString(L"\n");
    }

    return true;
}
#endif // AMDT_CPCLI_ENABLE_IMIX

void CSVReporter::WriteSectionHeaders(gtVector<gtString>& sectionHdrs)
{
    unsigned int idx;

    for (idx = 0; idx < sectionHdrs.size(); idx++)
    {
        if (! sectionHdrs[idx].isEmpty())
        {
            gtString hdrStr(L"\n");
            hdrStr.appendFormattedString(STR_FORMAT, sectionHdrs[idx].asCharArray());

            m_reportFile.writeString(hdrStr);
        }
    }
}