//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file defines consts and functions for the API Analyzer HTML generation
//==============================================================================

#ifndef _ANALYZER_HTML_UTILS_H_
#define _ANALYZER_HTML_UTILS_H_

#include <string>
#include <sstream>

extern const char* gs_THREAD_ID_TAG;   ///< tid string constant
extern const char* gs_SEQUENCE_ID_TAG; ///< seqid string constant
extern const char* gs_VIEW_TAG;        ///< view string constant

extern const char* gs_VIEW_TRACE_TAG;                  ///< trace view type string constant
extern const char* gs_VIEW_TIMELINE_HOST_TAG;          ///< host timeline view type string constant
extern const char* gs_VIEW_TIMELINE_DEVICE_TAG;        ///< device timeline view type string constant
extern const char* gs_VIEW_TIMELINE_DEVICE_NO_API_TAG; ///< device timeline (for non-api items) view type string constant

/// enum for view type when navigating from a summary page to the timline or trace view
enum AnalyzerHTMLViewType
{
    AnalyzerHTMLViewType_None = -1,          ///< no view type
    AnalyzerHTMLViewType_Trace = 0,          ///< trace view type
    AnalyzerHTMLViewType_TimelineHost,       ///< host timeline type
    AnalyzerHTMLViewType_TimelineDevice,     ///< device timeline type
    AnalyzerHTMLViewType_TimelineDeviceNoAPI ///< device timeline (for non-api items) type
};

/// Converts string input to AnalyzerHTMLViewType
/// \param inputString the string input
/// \return the AnalyzerHTMLViewType that corresponds to the input string (or AnalyzerHTMLViewType_None) if invalid string input
AnalyzerHTMLViewType StringToAnalyzerHTMLType(const std::string& inputString);

/// Appends two key value pairs for use in an href
/// \param  keyValue1 the first key/value pair
/// \param  keyValue2 the second key/value pair
/// \return a string with the two key'value pairs appended for use in an href
std::string AppendHTMLKeyValue(const std::string& keyValue1, const std::string& keyValue2);

/// template function to generate a key=value string
/// \param key the key name
/// \param value the key's value
/// \return a key-value string
template <typename T>
std::string GenerateHTMLKeyValue(const char* key, T value)
{
    std::stringstream ss;
    ss << key << "=" << value;

    return ss.str();
}

/// template function to generate a href string used in the Summary pages
/// \param hrefTarget the href target string
/// \param value the value associated with the href
/// \return the href string used in the Summary pages
template<typename T>
std::string GenerateHref(const std::string& hrefTarget, T value)
{
    std::stringstream ss;
    ss << "<a href=#?";
    ss << hrefTarget;
    ss << ">";
    ss << value;
    ss << "</a>";

    return ss.str();
}

#endif // _ANALYZER_HTML_UTILS_H_