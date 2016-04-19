//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief IParserListener base class
//==============================================================================

#ifndef _I_PARSER_LISTENER_H_
#define _I_PARSER_LISTENER_H_

#include <string>

#include <AMDTOSWrappers/Include/osOSDefinitions.h>

#include "../Common/OSUtils.h"

//------------------------------------------------------------------------------------
/// Abstract class
/// Parser Listener
//------------------------------------------------------------------------------------
template <class T>
class IParserListener
{
public:

    /// Virtual function
    /// \param threadId the parsed thread ID
    /// \param apiNum the amount of API calls for this thread
    virtual void SetAPINum(osThreadId threadId, unsigned int apiNum)
    {
        SP_UNREFERENCED_PARAMETER(threadId);
        SP_UNREFERENCED_PARAMETER(apiNum);

    };

    /// Pure virtual function
    /// \param obj pointer to the object parsed
    /// \param[out] stopParsing flag indicating if parsing should stop after this item
    virtual void OnParse(T* obj, bool& stopParsing) = 0;

    /// Virtual destructor
    virtual ~IParserListener() {}
};

#endif
