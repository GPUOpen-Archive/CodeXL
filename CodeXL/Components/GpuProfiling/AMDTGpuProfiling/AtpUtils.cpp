//==============================================================================
// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief RCP ATP parsing util for CodeXL implementation
//==============================================================================

#include "AtpUtils.h"

ProfileDataParserLoader* ProfileDataParserLoader::m_pParserInterfaceLoader = nullptr;

void OnParse(AtpInfoType apiInfoType, bool& stopParsing)
{
    AtpUtils::Instance()->OnParseCallbackHandler(apiInfoType, stopParsing);
}

void SetApiNum(osThreadId threadId, unsigned int apiNum)
{
    AtpUtils::Instance()->OnSetApiNumCallBackHandler(threadId, apiNum);
}

void ReportProgressOnParsing(const std::string& strProgressMessage, unsigned int uiCurItem, unsigned int uiTotalItems)
{
    AtpUtils::Instance()->OnReportProgress(strProgressMessage, uiCurItem, uiTotalItems);
}


bool AtpUtils::IsModuleLoaded() const
{
    return m_bModuleLoaded;
}

AtpUtils::AtpUtils()
{
    m_AtpPArserFunc = nullptr;
    m_AtpDataHandlerFunc = nullptr;
    m_bModuleLoaded = false;
}


void AtpUtils::LoadModule()
{
    if (!m_bModuleLoaded)
    {
        m_ParserHandlerList.clear();

        std::string errorString;
        m_bModuleLoaded = ProfileDataParserLoader::Instance()->LoadParser(errorString);

        if (m_bModuleLoaded)
        {
            m_AtpDataHandlerFunc = ProfileDataParserLoader::Instance()->GetAtpDataHandlerFunc();
            m_AtpPArserFunc = ProfileDataParserLoader::Instance()->GetAtpParserFunctionPointer();
        }
    }
}

void AtpUtils::AddToCallBackHandlerList(ICallBackParserHandler* icallBackHandler)
{
    if (std::find(m_ParserHandlerList.begin(), m_ParserHandlerList.end(), icallBackHandler) == m_ParserHandlerList.end())
    {
        m_ParserHandlerList.push_back(icallBackHandler);
    }
}

void AtpUtils::RemoveHandlerFromCallBackHandlerList(ICallBackParserHandler* icallBackhandler)
{
    std::vector<ICallBackParserHandler*>::iterator callBackParserIter;
    callBackParserIter = std::find(m_ParserHandlerList.begin(), m_ParserHandlerList.end(), icallBackhandler);

    if (callBackParserIter != m_ParserHandlerList.end())
    {
        m_ParserHandlerList.erase(callBackParserIter);
    }
}

void AtpUtils::OnParseCallbackHandler(AtpInfoType apiInfoType, bool& stopParsing)
{
    for (std::vector<ICallBackParserHandler*>::iterator iter = m_ParserHandlerList.begin(); iter != m_ParserHandlerList.end(); ++iter)
    {
        (*iter)->OnParseCallHandler(apiInfoType, stopParsing);
    }
}

void AtpUtils::OnSetApiNumCallBackHandler(osThreadId osthreadId, unsigned int apiNum)
{
    for (std::vector<ICallBackParserHandler*>::iterator iter = m_ParserHandlerList.begin(); iter != m_ParserHandlerList.end(); ++iter)
    {
        (*iter)->OnSetApiNumCallHandler(osthreadId, apiNum);
    }
}

void AtpUtils::OnReportProgress(const std::string& strProgressMessage, unsigned int uiCurItem, unsigned int uiTotalItems)
{
    for (std::vector<ICallBackParserHandler*>::iterator iter = m_ParserHandlerList.begin(); iter != m_ParserHandlerList.end(); ++iter)
    {
        (*iter)->OnParserProgressCallHandler(strProgressMessage, uiCurItem, uiTotalItems);
    }
}

AtpParserFunc AtpUtils::GetAtpParserFunctionPointer() const
{
    return m_AtpPArserFunc;
}

AtpDataHandlerFunc AtpUtils::GetAtpDataHandlerFunc() const
{
    return m_AtpDataHandlerFunc;
}
