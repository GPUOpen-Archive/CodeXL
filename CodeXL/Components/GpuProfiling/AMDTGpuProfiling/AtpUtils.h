//==============================================================================
// Copyright (c) 2017-2018 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief RCP ATP parsing util for CodeXL
//==============================================================================

#ifndef _ATP_UTILS_H_
#define _ATP_UTILS_H_

#include "ICallBackParserHandler.h"
#include <IAtpDataHandler.h>
#include <ProfileDataParserLoader.h>
#include "TSingleton.h"
#include <vector>

/// Callback function for the Atp file parsing callback function variable OnParse
/// \param[in] apiInfoType api type of tha parsed atp row
/// \param[out] stopParsing flag to indicating to continue parsing or not
void OnParse(AtpInfoType apiInfoType, bool& stopParsing);

/// Callback function for the Atp file parsing callback function variable SetApiNum
/// \param[in] threadId thread Id
/// \param[in] apiNum total number of the api calls in the atp file
void SetApiNum(osThreadId threadId, unsigned int apiNum);

/// Callback function for the Atp file parsing callback function variable ReportProgressOnParsing
/// \param[in] strProgressMessage message of the progress
/// \param[in] uiCurItem index of the current item
/// \param[in] uiTotalItems total number of the API count
void ReportProgressOnParsing(const char* strProgressMessage, unsigned int uiCurItem, unsigned int uiTotalItems);


/// Helper Class to handle ATP file parsing for CodeXL
class AtpUtils : public TSingleton<AtpUtils>
{
    // friend class for handling the private constructor
    friend TSingleton<AtpUtils>;
public:

    /// Returns module is loaded or not
    /// \return true if library is initialized otherwise false
    bool IsModuleLoaded() const;

    /// Loads the profile data parser library
    void LoadModule();

    /// Add the ICallBackParserHanler object to the list
    /// \param[in] icallBackHandler pointer to ICallBackParserHandler object
    void AddToCallBackHandlerList(ICallBackParserHandler* icallBackHandler);

    /// Removes the ICallBackParserHanler object to the list
    /// \param[in] icallBackHandler pointer to ICallBackParserHandler object
    void RemoveHandlerFromCallBackHandlerList(ICallBackParserHandler* icallBackHandler);

    /// Calls the OnParse function for all the ICallBackParserHandler object in the list
    /// \param[in] apiInfoType api type of tha parsed atp row
    /// \param[out] stopParsing flag to indicating to continue parsing or not
    void OnParseCallbackHandler(AtpInfoType apiInfoType, bool& stopParsing);

    /// Calls the SetApiNum function for all the ICallBackParserHandler object in the list
    /// \param[in] osthreadId thread Id
    /// \param[in] apiNum total number of the api calls in the atp file
    void OnSetApiNumCallBackHandler(osThreadId osthreadId, unsigned int apiNum);

    /// Calls the ReportProgress function for all the ICallBackParserHandler object in the list
    /// \param[in] strProgressMessage message of the progress
    /// \param[in] uiCurItem index of the current item
    /// \param[in] uiTotalItems total number of the API count
    void OnReportProgress(const char* strProgressMessage, unsigned int uiCurItem, unsigned int uiTotalItems);

    /// Get the AtpParserFunctionPointer
    /// \return function pointer of the AtpParserFunc from the library
    AtpParserFunc GetAtpParserFunctionPointer() const;

    /// Get the AtpDataHandlerFunctionPointer
    /// \return function pointer of the AtpDataHandlerFunc from the library
    AtpDataHandlerFunc GetAtpDataHandlerFunc() const;
private:

    /// Constructor
    AtpUtils();

    AtpParserFunc m_AtpPArserFunc;                              ///< atpparser callback function pointer
    AtpDataHandlerFunc m_AtpDataHandlerFunc;                    ///< atpdatahandler callback function pointer
    bool m_bModuleLoaded;                                       ///< flag indicating parser library is loaded or not
    std::vector<ICallBackParserHandler*> m_ParserHandlerList;   ///< list of the ICallBackParserHandler objects
};


#endif // _ATP_UTILS_H_
