//==================================================================================
// Copyright (c) 2014-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file Reporter.cpp
/// \brief This is Command Line Utility for CPU profiling.
///
//==================================================================================

// Qt
#include <QtGui>

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
        // m_reportFilePath.setFileExtension(L"csv");
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

bool CSVReporter::ReportSamplingSpec(gtVector<gtString>   sectionHdrs,
                                     EventEncodeVec&      eventEncodeVec,
                                     EventConfig*         pEventConfig,
                                     gtVector<gtString>   eventsNameVec)
{
    WriteSectionHeaders(sectionHdrs);

    if (NULL != pEventConfig)
    {

        for (gtUInt32 i = 0; i < eventEncodeVec.size(); i++)
        {
            gtString sampSpec(L"\n");
            sampSpec.appendFormattedString(L",\"" STR_FORMAT L"(%x)\"", eventsNameVec[i].asCharArray(),
                                           pEventConfig[i].eventSelect);
            sampSpec.appendFormattedString(L",%d", eventEncodeVec[i].eventCount);
            sampSpec.appendFormattedString(L",%x", pEventConfig[i].eventUnitMask);
            sampSpec.appendFormattedString(L"," STR_FORMAT, pEventConfig[i].bitUsr ? L"True" : L"False");
            sampSpec.appendFormattedString(L"," STR_FORMAT, pEventConfig[i].bitOs ? L"True" : L"False");

            m_reportFile.writeString(sampSpec);
        }
    }

    // Print a blank line at the end of the section
    m_reportFile.writeString(L"\n");

    return true;
}

bool CSVReporter::WriteOverviewFunction(gtVector<gtString>   sectionHdrs,
                                        FunctionInfoList&    funcList,
                                        gtVector<gtUInt64>&  totalSamples,
                                        gtUInt32             nbrCols,
                                        ColumnSpec*          pColumnSpec,
                                        EventEncodeVec&      evtEncodeVec,
                                        bool                 showPerc)
{
    WriteSectionHeaders(sectionHdrs);

    FunctionInfoMap funcMap;

    // Iterate over the function list and sort based on the sort-event mentioned by the user
    for (FunctionInfoList::iterator iter = funcList.begin(); iter != funcList.end(); iter++)
    {
        if (!(*iter).m_dataVector.empty())
        {
            double value = static_cast<double>((*iter).m_dataVector[m_sortEventIndex]);

            // If this is CLU, the insert should be based on CLU values
            if (m_isCLU)
            {
                value = GetCLUData((*iter).m_dataVector, nbrCols, pColumnSpec, evtEncodeVec);
            }

            funcMap.insert(FunctionInfoMap::value_type(value, (*iter)));
        }
    }

    int nbrFunc = 0;

    // TBD: Currently prints only the top 5 functions
    if (CSVReporter::DESCENDING_ORDER == m_sortOrder)
    {
        for (FunctionInfoMap::reverse_iterator rit = funcMap.rbegin(); ((rit != funcMap.rend()) && (nbrFunc < 5)); rit++, nbrFunc++)
        {
            FunctionInfo& funcInfo = (*rit).second;
            WriteFunctionData(funcInfo,
                              totalSamples,
                              nbrCols,
                              pColumnSpec,
                              evtEncodeVec,
                              showPerc,
                              false);
        }
    }
    else if (CSVReporter::ASCENDING_ORDER == m_sortOrder)
    {
        for (FunctionInfoMap::iterator it = funcMap.begin(); ((it != funcMap.end()) && (nbrFunc < 5)); it++, nbrFunc++)
        {
            FunctionInfo& funcInfo = (*it).second;
            WriteFunctionData(funcInfo,
                              totalSamples,
                              nbrCols,
                              pColumnSpec,
                              evtEncodeVec,
                              showPerc,
                              false);
        }
    }

    // Print a blank line at the end of the section
    m_reportFile.writeString(L"\n");

    return true;
}

bool CSVReporter::WriteOverviewProcess(gtVector<gtString>   sectionHdrs,
                                       PidProcessInfoMap&   pidProcInfoMap,
                                       gtVector<gtUInt64>&  totalSamples,
                                       gtUInt32             nbrCols,
                                       ColumnSpec*          pColumnSpec,
                                       EventEncodeVec&      evtEncodeVec,
                                       bool                 showPerc)
{
    WriteSectionHeaders(sectionHdrs);

    // Get the map based on number of samples
    ProcessInfoMap procMap;

    // Iterate over the process map and print
    for (PidProcessInfoMap::iterator iter = pidProcInfoMap.begin(); iter != pidProcInfoMap.end(); iter++)
    {
        if (!(*iter).second.m_dataVector.empty())
        {
            double value = static_cast<double>((*iter).second.m_dataVector[m_sortEventIndex]);

            // If this is CLU, the insert should be based on CLU values
            if (m_isCLU)
            {
                value = GetCLUData((*iter).second.m_dataVector, nbrCols, pColumnSpec, evtEncodeVec);
            }

            procMap.insert(ProcessInfoMap::value_type(value, (*iter).second));
        }
    }

    int nbrProc = 0;

    if (CSVReporter::DESCENDING_ORDER == m_sortOrder)
    {
        for (ProcessInfoMap::reverse_iterator rit = procMap.rbegin(); ((rit != procMap.rend()) && (nbrProc < 5)); rit++, nbrProc++)
        {
            ProcessInfo& procInfo = (*rit).second;
            WriteProcessData(procInfo,
                             totalSamples,
                             nbrCols,
                             pColumnSpec,
                             evtEncodeVec,
                             showPerc,
                             false,
                             true);
        }
    }
    else if (CSVReporter::ASCENDING_ORDER == m_sortOrder)
    {
        for (ProcessInfoMap::iterator it = procMap.begin(); ((it != procMap.end()) && (nbrProc < 5)); it++, nbrProc++)
        {
            ProcessInfo& procInfo = (*it).second;
            WriteProcessData(procInfo,
                             totalSamples,
                             nbrCols,
                             pColumnSpec,
                             evtEncodeVec,
                             showPerc,
                             false,
                             true);
        }
    }

    // Print a blank line at the end of the section
    m_reportFile.writeString(L"\n");

    return true;
}

bool CSVReporter::WriteOverviewModule(gtVector<gtString>   sectionHdrs,
                                      ModuleInfoList&      modList,
                                      gtVector<gtUInt64>&  totalSamples,
                                      gtUInt32             nbrCols,
                                      ColumnSpec*          pColumnSpec,
                                      EventEncodeVec&      evtEncodeVec,
                                      bool                 showPerc)
{
    WriteSectionHeaders(sectionHdrs);

    // Get the map based on number of samples
    ModuleInfoMap modMap;

    // Iterate over the function list and print
    for (ModuleInfoList::iterator iter = modList.begin(); iter != modList.end(); iter++)
    {
        if (!(*iter).m_dataVector.empty())
        {
            double value = static_cast<double>((*iter).m_dataVector[m_sortEventIndex]);

            // If this is CLU, the insert should be based on CLU values
            if (m_isCLU)
            {
                value = GetCLUData((*iter).m_dataVector, nbrCols, pColumnSpec, evtEncodeVec);
            }

            modMap.insert(ModuleInfoMap::value_type(value, (*iter)));
        }
    }

    int nbrMod = 0;

    if (CSVReporter::DESCENDING_ORDER == m_sortOrder)
    {
        for (ModuleInfoMap::reverse_iterator rit = modMap.rbegin(); ((rit != modMap.rend()) && (nbrMod < 5)); rit++, nbrMod++)
        {
            ModuleInfo modInfo = (*rit).second;
            WriteModuleData(modInfo,
                            totalSamples,
                            nbrCols,
                            pColumnSpec,
                            evtEncodeVec,
                            showPerc,
                            false);
        }
    }
    else if (CSVReporter::ASCENDING_ORDER == m_sortOrder)
    {
        for (ModuleInfoMap::iterator it = modMap.begin(); ((it != modMap.end()) && (nbrMod < 5)); it++, nbrMod++)
        {
            ModuleInfo modInfo = (*it).second;
            WriteModuleData(modInfo,
                            totalSamples,
                            nbrCols,
                            pColumnSpec,
                            evtEncodeVec,
                            showPerc,
                            false);
        }
    }

    // Print a blank line at the end of the section
    m_reportFile.writeString(L"\n");

    return true;
}

bool CSVReporter::WritePidSummary(gtVector<gtString>  sectionHdrs,
                                  ProcessInfo&         procInfo,
                                  gtVector<gtUInt64>&  totalSamples,
                                  gtUInt32             nbrCols,
                                  ColumnSpec*          pColumnSpec,
                                  EventEncodeVec&      evtEncodeVec,
                                  bool                 showPerc,
                                  bool                 sepByCore)
{
    WriteSectionHeaders(sectionHdrs);

    WriteProcessData(procInfo,
                     totalSamples,
                     nbrCols,
                     pColumnSpec,
                     evtEncodeVec,
                     showPerc,
                     sepByCore,
                     false);

    // Print a blank line at the end of the section
    m_reportFile.writeString(L"\n");

    return true;
}

bool CSVReporter::WritePidModuleSummary(gtVector<gtString>   sectionHdrs,
                                        ModuleInfoList&      modList,
                                        gtVector<gtUInt64>&  totalSamples,
                                        gtUInt32             nbrCols,
                                        ColumnSpec*          pColumnSpec,
                                        EventEncodeVec&      evtEncodeVec,
                                        bool                 showPerc,
                                        bool                 sepByCore)
{
    WriteSectionHeaders(sectionHdrs);

    // Get the map based on number of samples
    typedef std::multimap<double, ModuleInfo> modInfoMap;
    modInfoMap mMap;

    // Iterate over the function list and print
    for (ModuleInfoList::iterator iter = modList.begin(); iter != modList.end(); iter++)
    {
        if (!(*iter).m_dataVector.empty())
        {
            double value = static_cast<double>((*iter).m_dataVector[m_sortEventIndex]);

            // If this is CLU, the insert should be based on CLU values
            if (m_isCLU)
            {
                value = GetCLUData((*iter).m_dataVector, nbrCols, pColumnSpec, evtEncodeVec);
            }

            mMap.insert(modInfoMap::value_type(value, (*iter)));
        }
    }

    int nbrMod = 0;

    if (CSVReporter::DESCENDING_ORDER == m_sortOrder)
    {
        for (modInfoMap::reverse_iterator rit = mMap.rbegin(); rit != mMap.rend(); rit++, nbrMod++)
        {
            ModuleInfo& modInfo = (*rit).second;

            WriteModuleData(modInfo,
                            totalSamples,
                            nbrCols,
                            pColumnSpec,
                            evtEncodeVec,
                            showPerc,
                            sepByCore);
        }
    }
    else if (CSVReporter::ASCENDING_ORDER == m_sortOrder)
    {
        for (modInfoMap::iterator it = mMap.begin(); it != mMap.end(); it++, nbrMod++)
        {
            ModuleInfo& modInfo = (*it).second;

            WriteModuleData(modInfo,
                            totalSamples,
                            nbrCols,
                            pColumnSpec,
                            evtEncodeVec,
                            showPerc,
                            sepByCore);
        }
    }

    // Print a blank line at the end of the section
    m_reportFile.writeString(L"\n");

    return true;
}

bool CSVReporter::WritePidFunctionSummary(gtVector<gtString>   sectionHdrs,
                                          FunctionInfoList&    funcList,
                                          gtVector<gtUInt64>&  totalSamples,
                                          gtUInt32             nbrCols,
                                          ColumnSpec*          pColumnSpec,
                                          EventEncodeVec&      evtEncodeVec,
                                          bool                 showPerc,
                                          bool                 sepByCore)
{
    WriteSectionHeaders(sectionHdrs);

    // Get the map based on number of samples
    typedef std::multimap<double, FunctionInfo> funcInfoMap;
    funcInfoMap fMap;

    // Iterate over the function list and print
    for (FunctionInfoList::iterator iter = funcList.begin(); iter != funcList.end(); iter++)
    {
        double value = static_cast<double>((*iter).m_dataVector[m_sortEventIndex]);

        // If this is CLU, the insert should be based on CLU values
        if (m_isCLU)
        {
            value = GetCLUData((*iter).m_dataVector, nbrCols, pColumnSpec, evtEncodeVec);
        }

        fMap.insert(funcInfoMap::value_type(value, (*iter)));
    }

    if (CSVReporter::DESCENDING_ORDER == m_sortOrder)
    {
        for (funcInfoMap::reverse_iterator rit = fMap.rbegin(); rit != fMap.rend(); rit++)
        {
            FunctionInfo& funcInfo = (*rit).second;

            WriteFunctionData(funcInfo,
                              totalSamples,
                              nbrCols,
                              pColumnSpec,
                              evtEncodeVec,
                              showPerc,
                              sepByCore);
        }
    }
    else if (CSVReporter::ASCENDING_ORDER == m_sortOrder)
    {
        for (funcInfoMap::iterator it = fMap.begin(); it != fMap.end(); it++)
        {
            FunctionInfo& funcInfo = (*it).second;

            WriteFunctionData(funcInfo,
                              totalSamples,
                              nbrCols,
                              pColumnSpec,
                              evtEncodeVec,
                              showPerc,
                              sepByCore);
        }
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

bool CSVReporter::WriteColumData(gtVector<gtUInt64>&  totalSamples,
                                 gtVector<gtUInt64>&  dataVector,
                                 gtUInt32             nbrCols,
                                 ColumnSpec*          pColumnSpec,
                                 EventEncodeVec&      evtEncodeVec,
                                 bool                 showPerc,
                                 bool                 sepByCore,
                                 gtString&            dataLine)
{
    (void)sepByCore; // unused
    ColumnSpec* pColSpec = pColumnSpec;

    for (gtUInt32 i = 0; i < nbrCols; i++)
    {
        float floatVal = 0.0;
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
            case ColumnValue:
                if (showPerc)
                {
                    dataLine.appendFormattedString(L",%ld", dataVector[eventLeftIndex]);
                    AddPercentageValue(dataLine, static_cast<float>(dataVector[eventLeftIndex]), totalSamples[eventLeftIndex]);
                }
                else
                {
                    dataLine.appendFormattedString(L",%ld", dataVector[eventLeftIndex]);
                }

                break;

            case ColumnSum:
                dataLine.appendFormattedString(L",%ld", (dataVector[eventLeftIndex] +
                                                         dataVector[eventRightIndex]));
                break;

            case ColumnDifference:
                dataLine.appendFormattedString(L",%ld", (dataVector[eventLeftIndex] -
                                                         dataVector[eventRightIndex]));
                break;

            case ColumnProduct:
                dataLine.appendFormattedString(L",%ld", (dataVector[eventLeftIndex] *
                                                         dataVector[eventRightIndex]));
                break;

            case ColumnRatio:
                if ((static_cast<float>(dataVector[eventRightIndex]) > static_cast<float>(0.0))
                    && (static_cast<float>(dataVector[eventLeftIndex]) > static_cast<float>(0.0)))
                {
                    floatVal = (static_cast<float>(dataVector[eventLeftIndex]) * eventLeftCount) /
                               (static_cast<float>(dataVector[eventRightIndex] * eventRightCount));
                }

                dataLine.appendFormattedString(L",%3.02f", floatVal);
                break;

            case ColumnPercentage:
                if ((static_cast<float>(dataVector[eventRightIndex]) > static_cast<float>(0.0))
                    && (static_cast<float>(dataVector[eventLeftIndex]) > static_cast<float>(0.0)))
                {
                    floatVal = ((static_cast<float>(dataVector[eventLeftIndex]) / static_cast<float>(64.0)) /
                                (static_cast<float>(dataVector[eventRightIndex]))) * static_cast<float>(100.0);
                }

                dataLine.appendFormattedString(L",%3.02f", floatVal);
                break;

            case ColumnInvalid:
                break;
        } // switch column type

        pColSpec++;
    } // for loop to iterate over all the columns

    return true;
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

bool CSVReporter::WriteFunctionData(FunctionInfo&          funcInfo,
                                    gtVector<gtUInt64>&    totalSamples,
                                    gtUInt32               nbrCols,
                                    ColumnSpec*            pColumnSpec,
                                    EventEncodeVec&        evtEncodeVec,
                                    bool                   showPerc,
                                    bool                   sepByCore)
{
    bool retVal = false;
    gtString funcLine(L"\n");

    // TODO: what to report if the function name is missing ?
    if (!funcInfo.m_functionName.isEmpty())
    {
        funcLine.appendFormattedString(DQ_STR_FORMAT, funcInfo.m_functionName.asCharArray());

        WriteColumData(totalSamples,
                       funcInfo.m_dataVector,
                       nbrCols,
                       pColumnSpec,
                       evtEncodeVec,
                       showPerc,
                       sepByCore, // separate by core
                       funcLine);

        gtString modName;
        funcInfo.m_pModule->extractFileName(modName);
        funcLine.appendFormattedString(L", " STR_FORMAT, modName.asCharArray());

        m_reportFile.writeString(funcLine);
        retVal = true;
    }

    return retVal;
}

bool CSVReporter::WriteProcessData(ProcessInfo&         procInfo,
                                   gtVector<gtUInt64>&  totalSamples,
                                   gtUInt32             nbrCols,
                                   ColumnSpec*          pColumnSpec,
                                   EventEncodeVec&      evtEncodeVec,
                                   bool                 showPerc,
                                   bool                 sepByCore,
                                   bool                 appendPid)
{
    bool retVal = true;

    gtString procLine(L"\n");

    gtString procName = (!procInfo.m_processName.isEmpty()) ? procInfo.m_processName : L"NO PROCESS NAME";

    procLine.appendFormattedString(STR_FORMAT, procName.asCharArray());

    if (appendPid)
    {
        procLine.appendFormattedString(L",%ld", procInfo.m_pid);
    }

    WriteColumData(totalSamples,
                   procInfo.m_dataVector,
                   nbrCols,
                   pColumnSpec,
                   evtEncodeVec,
                   showPerc,
                   sepByCore,
                   procLine);

    m_reportFile.writeString(procLine);

    return retVal;
}

bool CSVReporter::WriteModuleData(ModuleInfo&          modInfo,
                                  gtVector<gtUInt64>&  totalSamples,
                                  gtUInt32             nbrCols,
                                  ColumnSpec*          pColumnSpec,
                                  EventEncodeVec&      evtEncodeVec,
                                  bool                 showPerc,
                                  bool                 sepByCore)
{
    bool retVal = true;

    gtString modLine(L"\n");

    gtString modName = (!modInfo.m_moduleName.isEmpty()) ? modInfo.m_moduleName : L"NO MODULE NAME";
    modLine.appendFormattedString(STR_FORMAT, modName.asCharArray());

    WriteColumData(totalSamples,
                   modInfo.m_dataVector,
                   nbrCols,
                   pColumnSpec,
                   evtEncodeVec,
                   showPerc,
                   sepByCore, // separate by core
                   modLine);

    m_reportFile.writeString(modLine);

    return retVal;
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