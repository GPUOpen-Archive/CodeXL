//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file defines consts and functions for the API Analyzer HTML generation
//==============================================================================

#include "AnalyzerHTMLUtils.h"

const char* gs_THREAD_ID_TAG = "tid";
const char* gs_SEQUENCE_ID_TAG = "seqid";
const char* gs_VIEW_TAG = "view";

const char* gs_VIEW_TRACE_TAG = "trace";
const char* gs_VIEW_TIMELINE_HOST_TAG = "timeline_host";
const char* gs_VIEW_TIMELINE_DEVICE_TAG = "timeline_device";
const char* gs_VIEW_TIMELINE_DEVICE_NO_API_TAG = "timeline_device_no_api";

AnalyzerHTMLViewType StringToAnalyzerHTMLType(const std::string& inputString)
{
    AnalyzerHTMLViewType retVal = AnalyzerHTMLViewType_None;

    if (gs_VIEW_TRACE_TAG == inputString)
    {
        retVal = AnalyzerHTMLViewType_Trace;
    }
    else if (gs_VIEW_TIMELINE_HOST_TAG == inputString)
    {
        retVal = AnalyzerHTMLViewType_TimelineHost;
    }
    else if (gs_VIEW_TIMELINE_DEVICE_TAG == inputString)
    {
        retVal = AnalyzerHTMLViewType_TimelineDevice;
    }
    else if (gs_VIEW_TIMELINE_DEVICE_NO_API_TAG == inputString)
    {
        retVal = AnalyzerHTMLViewType_TimelineDeviceNoAPI;
    }

    return retVal;
}
std::string AppendHTMLKeyValue(const std::string& keyValue1, const std::string& keyValue2)
{
    std::stringstream ss;
    ss << keyValue1;
    ss << "&";
    ss << keyValue2;

    return ss.str();
}