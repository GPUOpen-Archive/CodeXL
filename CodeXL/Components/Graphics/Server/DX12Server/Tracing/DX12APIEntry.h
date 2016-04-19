//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   DX12APIEntry.h
/// \brief  A DX12-specific entry for a traced API call.
//=============================================================================

#ifndef DX12APIENTRY_H
#define DX12APIENTRY_H

#include "../../Common/Tracing/APIEntry.h"
#include "../DX12Defines.h"

//-----------------------------------------------------------------------------
/// The DX12APIEntry class is used to track all DX12 API calls that are traced at runtime.
//-----------------------------------------------------------------------------
class DX12APIEntry : public APIEntry
{
public:
    //-----------------------------------------------------------------------------
    /// Constructor used to initialize members of new DX12APIEntry instances.
    /// \param inThreadId The thread Id that the call was invoked from.
    /// \param inInterfaceWrapper The wrapped instance used to invoke the API call.
    /// \param inFunctionId The FunctionId of the API call.
    /// \param inArguments The arguments used in the invocation of the API function.
    /// \param inReturnValue The return value of the API call.
    /// \param inReturnValueFlags The flags used in printing the return value.
    //-----------------------------------------------------------------------------
    DX12APIEntry(UINT inThreadId, IUnknown* inInterfaceWrapper, FuncId inFunctionId, const std::string& inArguments, INT64 inReturnValue, ReturnDisplayType inReturnValueFlags);

    //-----------------------------------------------------------------------------
    /// Constructor used to initialize members of new DX12APIEntry instances.
    /// \param inThreadId The thread Id that the call was invoked from.
    /// \param inInterfaceWrapper The wrapped instance used to invoke the API call.
    /// \param inFunctionId The FunctionId of the API call.
    /// \param inNumParameters The number of parameters used to invoke this API call.
    /// \param inParameters A ParameterEntry structure describing the parameter metadata for this API call.
    //-----------------------------------------------------------------------------
    DX12APIEntry(UINT inThreadId, IUnknown* inInterfaceWrapper, FuncId inFunctionId, UINT32 inNumParameters, ParameterEntry* inParameters);

    //-----------------------------------------------------------------------------
    /// Default destructor
    //-----------------------------------------------------------------------------
    virtual ~DX12APIEntry();

    //-----------------------------------------------------------------------------
    /// Use API-specific methods to determine the API name for this entry.
    /// \returns A null-terminated string containing the API name.
    //-----------------------------------------------------------------------------
    virtual const char* GetAPIName() const;

    //-----------------------------------------------------------------------------
    /// Append this APIEntry's information to the end of the API Trace response.
    /// \param out The API Trace response stream to append to.
    /// \param inStartTime The start time for the API call.
    /// \param inEndTime The end time for the API call.
    //-----------------------------------------------------------------------------
    virtual void AppendAPITraceLine(gtASCIIString& out, double inStartTime, double inEndTime) const;

    //-----------------------------------------------------------------------------
    /// Get the API parameters as a single string. Build the string from the
    /// individual parameters if necessary
    /// \return parameter string
    //-----------------------------------------------------------------------------
    const char* GetParameterString() const;

    //-----------------------------------------------------------------------------
    /// Check if this logged APIEntry is a Draw call.
    /// \returns True if the API is a draw call. False if it's not.
    //-----------------------------------------------------------------------------
    bool IsDrawCall() const;

    //-----------------------------------------------------------------------------
    /// Set the return value for this logged APIEntry
    /// \param inReturnValue The return value
    /// \param inReturnValueFlags A flag indicating how the return value should
    /// be displayed
    //-----------------------------------------------------------------------------
    void SetReturnValue(INT64 inReturnValue, ReturnDisplayType inReturnValueFlags);

    /// The return value for this API call.
    INT64 mReturnValue;

    /// The display flags for the return value for this API call. Used to indicate how to display the value (hex, decimal, bitfield etc)
    ReturnDisplayType mReturnValueFlags;

    /// The optional sample Id used to associate this API call with GPA profiling results.
    UINT64 mSampleId;

    /// This is an IUnknown* to the Wrapped_ID3D12 wrapper interface.
    IUnknown* mWrapperInterface;

private:
    //-----------------------------------------------------------------------------
    /// Add a single API trace parameter to the list
    /// \param index the parameter index (0-based)
    /// \param type the data type of the parameter
    /// \param pParameterValue pointer to the parameter value
    /// Each buffer parameter is BYTES_PER_PARAMETER bytes. The first 4 bytes
    /// are for the parameter data type. The next byte is the number of bytes
    /// used to store the parameter value. The remaining space is used for the
    /// parameter value.
    //-----------------------------------------------------------------------------
    void AddParameter(unsigned int index, int type, const void* pParameterValue);

    //-----------------------------------------------------------------------------
    /// Convert an API parameter from raw data to a string for display
    /// \param paramType the data type of the parameter
    /// \param dataLength the number of bytes used to contain the data
    /// \param pRawData a pointer to the raw data
    /// \param ioParameterString a buffer passed in where the string is to be stored
    //-----------------------------------------------------------------------------
    void GetParameterAsString(PARAMETER_TYPE paramType, const char dataLength, const char* pRawData, char* ioParameterString) const;

    /// The number of parameters this API call takes
    UINT32 mNumParameters;

    /// A flag set when results are retrieved from GPA.
    bool mbGotProfileResults;

    /// The pre-Bottom timestamp for this entry
    double mPreBottomTimestamp;

    /// The post-Bottom timestamp for this entry
    double mPostBottomTimestamp;

    /// buffer used to store raw parameter data for formatting later
    char* mParameterBuffer;
};

#endif // DX12APIENTRY_H