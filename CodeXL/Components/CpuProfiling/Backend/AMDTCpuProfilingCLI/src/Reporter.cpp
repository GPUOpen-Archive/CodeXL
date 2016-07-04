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
        floatVal = (leftData / rightData) * 100.0;
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

void Reporter::SetCLU(bool isCLU)
{
    m_isCLU = isCLU;
    return;
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

        sampSpec.appendFormattedString(L",\"" STR_FORMAT L"(%x)\"", counter.m_name.asCharArray(), samplingConfig[idx].m_hwEventId);
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
        rowStr.appendFormattedString(L"%ls,%u", proc.m_name.asCharArray(), proc.m_id);

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

bool CSVReporter::WriteCallGraphFunctionSummary(gtVector<gtString>    sectionHdrs,
                                                CGSampleFunctionMap&  cgFuncMap,
                                                EventMaskType         eventId,
                                                bool                  reportPerc)
{
    GT_UNREFERENCED_PARAMETER(eventId);

    if (cgFuncMap.size() == 0)
    {
        return false;
    }

    WriteSectionHeaders(sectionHdrs);


    CGSampleFunctionMap::reverse_iterator rit = cgFuncMap.rbegin();
    gtUInt64 totalDeepSamples = (*rit).second->m_deepCount;

    for (; rit != cgFuncMap.rend(); rit++)
    {
        CGFunctionInfo* pFuncNode = (*rit).second;

#if 0

        if (NULL == pFuncNode->m_pModule || NULL == pFuncNode->m_pFunction ||
            pFuncNode->m_pModule->isUnchartedFunction(*pFuncNode->m_pFunction))
        {
            fprintf(stderr, "No debug info available\n");
        }

#endif //0

        if (pFuncNode->m_deepCount != 0)
        {
            gtString tmpStr(L"\n");
            tmpStr += L"\"";
            tmpStr += pFuncNode->m_funcName;
            tmpStr += L"\"";
            tmpStr.appendFormattedString(L",%ld", pFuncNode->m_selfCount);

            if (reportPerc)
            {
                AddPercentageValue(tmpStr, static_cast<float>(pFuncNode->m_selfCount), static_cast<float>(totalDeepSamples));
            }

            tmpStr.appendFormattedString(L",%ld", pFuncNode->m_deepCount);

            if (reportPerc)
            {
                AddPercentageValue(tmpStr, static_cast<float>(pFuncNode->m_deepCount), static_cast<float>(totalDeepSamples));
            }

            tmpStr.appendFormattedString(L",%u", pFuncNode->m_pathCount);

            gtString sourceInfo;

            if (NULL != pFuncNode->m_pFunction)
            {
                pFuncNode->m_pFunction->getSourceInfo(sourceInfo);
            }

            tmpStr += L",";
            tmpStr += sourceInfo;

            gtString modFileName;

            if (NULL != pFuncNode->m_pModule && !pFuncNode->m_pModule->getPath().isEmpty())
            {
                pFuncNode->m_pModule->extractFileName(modFileName);
            }

            tmpStr += L",";
            tmpStr += modFileName;

            m_reportFile.writeString(tmpStr);
        }
    }

    m_reportFile.writeString(L"\n");

    return true;
}

bool CSVReporter::WriteCallGraph(gtVector<gtString>    sectionHdrs,
                                 CGSampleFunctionMap&  cgFuncMap,
                                 EventMaskType         eventId,
                                 bool                  reportPerc)
{
    GT_UNREFERENCED_PARAMETER(eventId);

    if (cgFuncMap.size() == 0)
    {
        return false;
    }

    WriteSectionHeaders(sectionHdrs);

    gtUInt32 index = 1;

    for (CGSampleFunctionMap::reverse_iterator rit = cgFuncMap.rbegin(); rit != cgFuncMap.rend(); rit++, index++)
    {
        CGFunctionInfo* pFuncNode = (*rit).second;

#if 0

        if (NULL == pFuncNode->m_pModule || NULL == pFuncNode->m_pFunction ||
            pFuncNode->m_pModule->isUnchartedFunction(*pFuncNode->m_pFunction))
        {
            fprintf(stderr, "No debug info available\n");
        }

#endif //0

        if (pFuncNode->m_deepCount != 0)
        {
            // Write the parents
            WriteParentsData(*pFuncNode, reportPerc);

            WriteSelf(*pFuncNode, reportPerc);

            // Write the children
            WriteChildrenData(*pFuncNode, reportPerc);

            m_reportFile.writeString(L"\n");
        }
    }

    m_reportFile.writeString(L"\n");

    return true;
}

bool CSVReporter::WriteImixSummaryInfo(gtVector<gtString>   sectionHdrs,
                                       ImixSummaryMap&      imixSummaryMap,
                                       gtUInt64             totalSamples)
{
    WriteSectionHeaders(sectionHdrs);

    // Sort the map by samples count ascending order
    multimap<gtUInt64, const string> sortedImixSummaryMap;

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
            floatVal = (leftData / rightData) * 100.0;
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
        multimap<gtUInt64, const string> sortedImixMap;

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

void CSVReporter::WriteSelf(const CGFunctionInfo& funcNode, bool reportPerc)
{
    gtString selfData(L"\n");

    selfData.appendFormattedString(L"[%d]", funcNode.m_index);
    selfData += L",";

    selfData += L"\"";
    selfData += funcNode.m_funcName;
    selfData.appendFormattedString(L" [%ld]", funcNode.m_index);  // function index
    selfData += L"\"";
    selfData += L","; // parent/children function-name column

    selfData.appendFormattedString(L",%ld", funcNode.m_selfCount);

    if (reportPerc)
    {
        AddPercentageValue(selfData, static_cast<float>(funcNode.m_selfCount), static_cast<float>(funcNode.m_deepCount));
    }

    selfData.appendFormattedString(L",%ld", funcNode.m_deepCount);

    gtString sourceInfo;

    if (NULL != funcNode.m_pFunction)
    {
        funcNode.m_pFunction->getSourceInfo(sourceInfo);
    }

    selfData += L",";
    selfData += sourceInfo;

    gtString modFileName;

    if (NULL != funcNode.m_pModule && !funcNode.m_pModule->getPath().isEmpty())
    {
        funcNode.m_pModule->extractFileName(modFileName);
    }

    selfData += L",";
    selfData += modFileName;

    m_reportFile.writeString(selfData);
}

void CSVReporter::WriteParentsData(const CGFunctionInfo& funcNode, bool reportPerc)
{
    for (const auto& parentNode : funcNode.m_parents)
    {
        if (parentNode.m_deepCount)
        {
            gtString parentData(L"\n");
            parentData += L","; // Index
            parentData += L","; // Self

            parentData += L"\"";
            parentData += parentNode.m_pFuncInfo->m_funcName;
            parentData.appendFormattedString(L" [%ld]", parentNode.m_pFuncInfo->m_index);  // function index
            parentData += L"\"";

            parentData += L",";    // self samples
            parentData += L",";

            parentData.appendFormattedString(L"%ld", parentNode.m_deepCount);

            if (reportPerc)
            {
                AddPercentageValue(parentData, static_cast<float>(parentNode.m_deepCount), static_cast<float>(funcNode.m_deepCount));
            }

            m_reportFile.writeString(parentData);
        }
    }
}

void CSVReporter::WriteChildrenData(const CGFunctionInfo& funcNode, bool reportPerc)
{
    for (const auto& childNode : funcNode.m_children)
    {
        if (childNode.m_deepCount)
        {
            gtString childData(L"\n");
            childData += L","; // index
            childData += L","; // self

            childData += L"\"";
            childData += childNode.m_pFuncInfo->m_funcName;
            childData.appendFormattedString(L" [%ld]", childNode.m_pFuncInfo->m_index);  // function index
            childData += L"\"";

            childData += L","; // self samples
            childData += L","; // self samples

            childData.appendFormattedString(L"%ld", childNode.m_deepCount);

            if (reportPerc)
            {
                AddPercentageValue(childData, static_cast<float>(childNode.m_deepCount), static_cast<float>(funcNode.m_deepCount));
            }

            m_reportFile.writeString(childData);
        }
    }
}

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

double CSVReporter::GetCLUData(gtVector<gtUInt64>&  dataVector,
                               gtUInt32             nbrCols,
                               ColumnSpec*          pColumnSpec,
                               EventEncodeVec&      evtEncodeVec)
{
    ColumnSpec* pColSpec = pColumnSpec;

    double cluVal = 0.0;
    bool foundVal = false;

    for (gtUInt32 i = 0; (i < nbrCols) && (!foundVal); i++)
    {
        EventMaskType eventLeft = EncodeEvent(pColSpec->dataSelectLeft.eventSelect, pColSpec->dataSelectLeft.eventUnitMask,
                                              pColSpec->dataSelectLeft.bitOs, pColSpec->dataSelectLeft.bitUsr);
        EventMaskType eventRight = EncodeEvent(pColSpec->dataSelectRight.eventSelect, pColSpec->dataSelectRight.eventUnitMask,
                                               pColSpec->dataSelectRight.bitOs, pColSpec->dataSelectRight.bitUsr);

        gtUInt64 eventLeftCount;
        gtUInt64 eventRightCount;
        gtUInt32 eventLeftIndex;
        gtUInt32 eventRightIndex;

        GetEventDetailForEventMask(evtEncodeVec, eventLeft, eventLeftIndex, eventLeftCount);
        GetEventDetailForEventMask(evtEncodeVec, eventRight, eventRightIndex, eventRightCount);

        switch (pColSpec->type)
        {
            case ColumnPercentage:
                if ((static_cast<float>(dataVector[eventRightIndex]) > static_cast<float>(0.0))
                    && (static_cast<float>(dataVector[eventLeftIndex]) > static_cast<float>(0.0)))
                {
                    cluVal = ((static_cast<float>(dataVector[eventLeftIndex]) / static_cast<float>(64.0)) /
                              (static_cast<float>(dataVector[eventRightIndex]))) * static_cast<float>(100.0);
                }

                foundVal = true;
                break;

            default:
                break;
        } // switch column type

        pColSpec++;
    } // for loop to iterate over all the columns

    return cluVal;
}