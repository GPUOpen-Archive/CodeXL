//==============================================================================
// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Interface for the objects for handling atp file parsing
//==============================================================================

#ifndef _I_CALL_BACK_PARSER_HANDLER_H_
#define _I_CALL_BACK_PARSER_HANDLER_H_

#include <IAtpDataHandler.h>

/// pure virtual class for handling atp fil parsing
class ICallBackParserHandler
{
public:
    /// Callback function for the Atp file parsing callback function variable OnParse
    /// \param[in] apiInfoType api type of tha parsed atp row
    /// \param[out] stopParsing flag to indicating to continue parsing or not
    virtual void OnParseCallHandler(AtpInfoType apiInfoType, bool& stopParsing) = 0;

    /// Callback function for the Atp file parsing callback function variable SetApiNum
    /// \param[in] threadId thread Id
    /// \param[in] apiNum total number of the api calls in the atp file
    virtual void OnSetApiNumCallHandler(osThreadId threadId, unsigned int apiNum) = 0;

    /// Callback function for the Atp file parsing callback function variable ReportProgressOnParsing
    /// \param[in] strProgressMessage message of the progress
    /// \param[in] uiCurItem index of the current item
    /// \param[in] uiTotalItems total number of the API count
    virtual void OnParserProgressCallHandler(const char* strProgressMessage, unsigned int uiCurItem, unsigned int uiTotalItems) = 0;

    /// Destructor
    virtual ~ICallBackParserHandler() {};
};

#endif // _I_CALL_BACK_PARSER_HANDLER_H_
