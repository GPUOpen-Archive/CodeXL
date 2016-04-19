//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This analyzer class analyzes hsa return codes; reports errors for failures
//==============================================================================

#include <sstream>
#include "../Common/StringUtils.h"
#include "HSARetCodeAnalyzer.h"

HSARetCodeAnalyzer::HSARetCodeAnalyzer()
{
    m_strName = "HSARetCodeAnalyzer";
    GetNoReturnCodeAPIs(m_noStatusAPIs);
}

HSARetCodeAnalyzer::~HSARetCodeAnalyzer(void)
{
}

void HSARetCodeAnalyzer::FlattenedAPIAnalyze(APIInfo* pAPIInfo)
{
    SP_UNREFERENCED_PARAMETER(pAPIInfo);
    return;
}

void HSARetCodeAnalyzer::Clear()
{
    m_msgList.clear();
}

void HSARetCodeAnalyzer::Analyze(APIInfo* pAPIInfo)
{
    HSAAPIInfo* pHSAApiInfo = dynamic_cast<HSAAPIInfo*>(pAPIInfo);

    if (nullptr != pHSAApiInfo)
    {
        // if this API is one that reports status, check the return value for "HSA_STATUS_SUCCESS"
        if (pHSAApiInfo->m_apiID < HSA_API_Type_Non_API_First && m_noStatusAPIs.find(pHSAApiInfo->m_apiID) == m_noStatusAPIs.end())
        {
            std::string hsaRetVal = pHSAApiInfo->m_strRet;

            static const char* s_szHSASUCCESS = "HSA_STATUS_SUCCESS";      ///< successful API return value
            static const char* s_szHSAINFOBREAK = "HSA_STATUS_INFO_BREAK"; ///< HSA_STATUS_INFO_BREAK is not an error

            if (pHSAApiInfo->m_strRet.find(s_szHSASUCCESS) == std::string::npos && pHSAApiInfo->m_strRet.find(s_szHSAINFOBREAK) == std::string::npos)
            {
                // error found
                std::stringstream ss;
                ss << pHSAApiInfo->m_strName << " returns " << hsaRetVal;
                APIAnalyzerMessage msg;
                msg.type = MSGTYPE_Error;
                msg.uiSeqID = pHSAApiInfo->m_uiSeqID;
                msg.uiDisplaySeqID = pHSAApiInfo->m_uiDisplaySeqID;
                msg.bHasDisplayableSeqId = pHSAApiInfo->m_bHasDisplayableSeqId;
                msg.uiTID = pHSAApiInfo->m_tid;
                msg.strMsg = ss.str();
                m_msgList.push_back(msg);
                return;
            }
        }
    }
}

void HSARetCodeAnalyzer::EndAnalyze()
{
}
