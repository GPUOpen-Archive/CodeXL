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
    , mNumParameters(0)
    , mParameterBuffer(nullptr)
{
    mParameters = inArguments.c_str();
}

//--------------------------------------------------------------------------
/// Constructor used to initialize members of new CallData instances.
/// \param inThreadId The thread Id that the call was invoked from.
/// \param inFuncId The function ID of this API call
/// \param inNumParameters The number of params for this api call
//--------------------------------------------------------------------------
APIEntry::APIEntry(UINT inThreadId, FuncId inFuncId, UINT32 inNumParameters)
    : mThreadId(inThreadId)
    , mFunctionId(inFuncId)
    , mNumParameters(inNumParameters)
    , mParameterBuffer(nullptr)
    , mParameters("")
{
    if (inNumParameters != 0)
    {
        mParameterBuffer = new char[inNumParameters * BYTES_PER_PARAMETER];
    }
}

//--------------------------------------------------------------------------
/// Virtual destructor since this is subclassed elsewhere.
//--------------------------------------------------------------------------
APIEntry::~APIEntry()
{
    SAFE_DELETE_ARRAY(mParameterBuffer);
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

//-----------------------------------------------------------------------------
/// Get the API parameters as a single string. Build the string from the
/// individual parameters if necessary
/// \return parameter string
//-----------------------------------------------------------------------------
const char* APIEntry::GetParameterString() const
{
    static gtASCIIString parameterString;

    if (mNumParameters == 0)
    {
        return mParameters.asCharArray();
    }
    else
    {
        parameterString = "";
        // get the API function parameters from the raw memory buffer
        int arrayCount = 0;
        char* buffer = mParameterBuffer;

        if (buffer != nullptr)
        {
            for (UINT32 loop = 0; loop < mNumParameters; loop++)
            {
                char* ptr = buffer;
                PARAMETER_TYPE paramType;
                memcpy(&paramType, ptr, sizeof(PARAMETER_TYPE));
                ptr += sizeof(PARAMETER_TYPE);
                unsigned char length = *ptr++;

                if (length < BYTES_PER_PARAMETER)
                {
                    // if an array token is found, add an opening brace and start the array elements countdown
                    if (paramType == PARAMETER_ARRAY)
                    {
                        memcpy(&arrayCount, ptr, sizeof(unsigned int));
                        parameterString += "[ ";
                    }
                    else
                    {
                        char parameter[BYTES_PER_PARAMETER] = {};

                        GetParameterAsString(paramType, length, ptr, parameter);
                        parameterString += parameter;

                        // check to see if this is the last array element. If so, output a closing brace
                        // before the comma separator (if needed)
                        if (arrayCount > 0)
                        {
                            arrayCount--;

                            if (arrayCount == 0)
                            {
                                parameterString += " ]";
                            }
                        }

                        // if there are more parameters to come, insert a comma delimiter
                        if ((loop + 1) < mNumParameters)
                        {
                            parameterString += ", ";
                        }
                    }
                }

                // point to next parameter in the buffer
                buffer += BYTES_PER_PARAMETER;
            }
        }

        return parameterString.asCharArray();
    }
}
