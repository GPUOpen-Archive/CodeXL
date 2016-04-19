//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of the TraceAnalyzer class which should
///         be utilized by all plugins.
//==============================================================================

#if defined (_WIN32)
    #include <windows.h>
#endif
#include "TraceAnalyzer.h"
#include "xml.h"

unsigned long DictKeyUsage::Add(std::string str)
{
    unsigned long dwCallId = 0;

    std::map<std::string, DictEntry>::iterator i;
    i = Calls.find(str);

    if (i == Calls.end())
    {
        //add key to dictionary
        dwCallId = (unsigned long)Calls.size();
        Calls[str] = DictEntry(dwCallId);
    }
    else
    {
        dwCallId = i->second.m_dwId;
        i->second.m_dwCount++;
    }

    return dwCallId;
}

void DictKeyUsage::Clear()
{
    Calls.clear();
}

std::map<std::string, DictEntry>& DictKeyUsage::GetMap()
{
    return Calls;
}

std::string DictKeyUsage::GetData()
{
    std::string tmp = "";

    std::map<std::string, DictEntry>::const_iterator it;

    for (it = Calls.begin(); it != Calls.end(); ++it)
    {
        tmp += XMLAttrib(FormatText("k%d", it->second.m_dwId).asCharArray(), FormatText("val='%s'", it->first.c_str()).asCharArray()).asCharArray();
    }

    return XML("keys", tmp.c_str()).asCharArray();
}


//=============================================================================
/// TraceAnalyzer
///
/// Collects api trace
//=============================================================================
TraceAnalyzer::TraceAnalyzer()
    : m_bCollectingTimingLog(false)
{
    AddCommand(CONTENT_XML, "XMLLog", "API Trace XML", "Log.xml", DISPLAY, INCLUDE, m_apiTraceXML);
    AddCommand(CONTENT_TEXT, "TXTLog", "API Trace TXT", "Log.txt", DISPLAY, INCLUDE, m_apiTraceTXT);
    AddCommand(CONTENT_TEXT, "TimingLog", "TimingLog", "TimingLog.txt", DISPLAY, INCLUDE, m_cmdTimingLog);

    SetLayerName("TraceAnalyzer");
}

//-----------------------------------------------------------------------------
/// Signals the frame analyzer that a frame is beginning and it should
/// initialize based on any received commands
//-----------------------------------------------------------------------------
void TraceAnalyzer::BeginFrame()
{
    if (IsCollectingAPICalls())
    {
        Clear();

        if (m_apiTraceTXT.IsActive() ||
            (m_cmdTimingLog.IsActive() && m_apiCallTimer.Size() == 0))
        {
            // when the api trace TXT is requested, the timing log is also generated
            // so clear it at the beginning of the frame.
            m_apiCallTimer.Clear();

            m_bCollectingTimingLog = true;

            // insert an initial timer to indicate the start of the frame
            m_apiCallTimer.Add(osGetCurrentThreadId(), m_apiCallTimer.GetRaw());
        }
    }
}

//-----------------------------------------------------------------------------
/// Adds XML for the supplied API call to the APITrace log.
/// This should only be called if the apiTraceXML command is active.
/// \param pstrDevice the device that the call was made on
/// \param pstrInterface the interface that the call is related to
/// \param pstrFunction the name of the API entrypoint
/// \param pstrParameters the parameters supplied to the function call
/// \param pstrReturnValue the return value of the function call
//-----------------------------------------------------------------------------
void TraceAnalyzer::AddAPICall(const char* pstrDevice,
                               const char* pstrInterface,
                               const char* pstrFunction,
                               const char* pstrParameters,
                               const char* pstrReturnValue)
{
    ScopeLock lock(&m_mutex);

    DWORD threadId = osGetCurrentThreadId();

    if (m_bCollectingTimingLog)
    {
        m_apiCallTimer.Add(threadId, m_startTime);
    }

    if (m_apiTraceXML.IsActive())
    {
        for (std::vector<std::string>::const_iterator str = m_strDebug.begin(); str != m_strDebug.end(); ++str)
        {
            unsigned long dwCallsId = m_Calls.Add("OutputDebugString");
            unsigned long dwParamsId = m_Params.Add(str->c_str());
            m_Lines.push_back(std::pair< unsigned long, unsigned long>(dwCallsId, dwParamsId));
        }

        ClearOutputDebugString();

        unsigned long dwCallsId = m_Calls.Add(FormatText("%s_%s", pstrInterface, pstrFunction).asCharArray());
        unsigned long dwParamsId = m_Params.Add(FormatText("%s = %s", pstrParameters, pstrReturnValue).asCharArray());

        m_Lines.push_back(std::pair<unsigned long, unsigned long>(dwCallsId, dwParamsId));
    }

    if (m_apiTraceTXT.IsActive())
    {
        for (std::vector<std::string>::const_iterator str = m_strDebug.begin(); str != m_strDebug.end(); ++str)
        {
            m_apiTraceString += FormatText("%u OutputDebugString %s\n", threadId, str->c_str()).asCharArray();
        }

        ClearOutputDebugString();

        m_apiTraceString += FormatText("%u %s_%s(%s) = %s\n", threadId, pstrInterface, pstrFunction, pstrParameters, pstrReturnValue).asCharArray();
    }

    PS_UNREFERENCED_PARAMETER(pstrDevice);
}

//-----------------------------------------------------------------------------
/// Signals the frame analyzer that the frame is ended and any collected data should be sent
//-----------------------------------------------------------------------------
void TraceAnalyzer::EndFrame()
{
    if (m_apiTraceXML.IsActive())
    {
        m_apiTraceXML.Send(GetAPITrace().c_str());
    }

    if (m_apiTraceTXT.IsActive())
    {
        // add main context information to the beginning of the API Trace
        void* pDevice = this->GetActiveDevice();

        if (pDevice != NULL)
        {
            std::string mainContextString = "";
            mainContextString += FormatText("MainContext=0x%p\n", this->GetActiveDevice()).asCharArray();
            m_apiTraceString = mainContextString + m_apiTraceString;
        }

        m_bCollectingTimingLog = false;
        m_apiTraceTXT.Send(m_apiTraceString.c_str());
        m_apiTraceString.clear();
    }

    if (m_cmdTimingLog.IsActive())
    {
        m_bCollectingTimingLog = false;
        m_cmdTimingLog.Send(m_apiCallTimer.GetLogAsString().c_str());
        m_apiCallTimer.Clear();
    }
}

/// Clears the current API Trace Log
void TraceAnalyzer::Clear()
{
    m_Calls.Clear();
    m_Params.Clear();
    m_Lines.clear();
    ClearOutputDebugString();
    m_apiTraceString.clear();
}

unsigned long TraceAnalyzer::GetSyntaxColour(const char* pStr)
{
    PS_UNREFERENCED_PARAMETER(pStr);
    return 0x00000000;
}

std::string TraceAnalyzer::GetAPITrace()
{
    std::string out = "";
    std::string tmp = "";

    // coarsely estimate strings final size to speed up string concatenation
    out.reserve(m_Calls.GetMap().size() * 80);
    tmp.reserve(m_Lines.size() * 80);

    // add colour according to the function name
    std::map<std::string, DictEntry>::const_iterator it;

    for (it = m_Calls.GetMap().begin(); it != m_Calls.GetMap().end(); ++it)
    {
        tmp += XMLAttrib(FormatText("k%d", it->second.m_dwId).asCharArray(), FormatText("val='%s' col='#%x'", it->first.c_str(), GetSyntaxColour((char*)it->first.c_str())).asCharArray()).asCharArray();
    }

    out += XML("FunctionNames", XML("keys", tmp.c_str()).asCharArray()).asCharArray();

    //add keys for parameters
    out += XML("FunctionParams", m_Params.GetData().c_str()).asCharArray();

    // add calls list
    tmp = "";

    for (size_t i = 0; i < m_Lines.size(); i++)
    {
        tmp += XMLAttrib(FormatText("k%d", m_Lines[i].first).asCharArray(), FormatText("prm='%d'", m_Lines[i].second).asCharArray()).asCharArray();
    }

    out += XML("CallsList", tmp.c_str()).asCharArray();

    return out;
}
