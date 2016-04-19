//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file APIEntry.cpp
/// \brief The APIEntry structure is used to track traced call invocations.
//=============================================================================

#include "APIEntry.h"
#include "../misc.h"

//--------------------------------------------------------------------------
/// Constructor used to initialize members of new CallData instances.
/// \param inThreadId The thread Id that the call was invoked from.
/// \param inFuncId The function ID of this API call
/// \param inArguments A string containing the arguments of the invoked call.
//--------------------------------------------------------------------------
APIEntry::APIEntry(UINT inThreadId, FuncId inFuncId, const std::string& inArguments)
    : mThreadId(inThreadId)
    , mFunctionId(inFuncId)
{
    mParameters = inArguments.c_str();
}

//--------------------------------------------------------------------------
/// Constructor used to initialize members of new CallData instances.
/// \param inThreadId The thread Id that the call was invoked from.
/// \param inFuncId The function ID of this API call
//--------------------------------------------------------------------------
APIEntry::APIEntry(UINT inThreadId, FuncId inFuncId)
    : mThreadId(inThreadId)
    , mFunctionId(inFuncId)
{
}

//--------------------------------------------------------------------------
/// Virtual destructor since this is subclassed elsewhere.
//--------------------------------------------------------------------------
APIEntry::~APIEntry()
{
}

//-----------------------------------------------------------------------------
/// Convert a numeric return value into a human-readable string.
/// \param inReturnValue A numeric return value. Could be an HRESULT, or a number.
/// \param inDisplayType A flag indicating how the return value should be displayed
/// \param ioReturnValue A string used to write the return value to
//-----------------------------------------------------------------------------
void APIEntry::PrintReturnValue(const INT64 inReturnValue, const ReturnDisplayType inDisplayType, gtASCIIString& ioReturnValue) const
{
    const char* resultString = nullptr;

    switch (inReturnValue)
    {
        // *INDENT-OFF*  - don't let astyle indent this next section
        // @TODO: Can we handle the entire set of possible HRESULTs?
        PRINTENUMCASE(S_OK, resultString);
        // *INDENT-ON*
    }

    // Handle cases where the HRESULT wasn't formatted into a string above.
    if (resultString == nullptr)
    {

        if (inReturnValue == FUNCTION_RETURNS_VOID)
        {
            ioReturnValue.appendFormattedString("%s", "void");
        }
        else
        {
            switch (inDisplayType)
            {
                case RETURN_VALUE_HEX:
                    ioReturnValue.appendFormattedString("0x%016X", inReturnValue);
                    break;

                default:
                    ioReturnValue.appendFormattedString("%I64d", inReturnValue);
                    break;
            }
        }
    }
    else
    {
        ioReturnValue.appendFormattedString("%s", resultString);
    }
}