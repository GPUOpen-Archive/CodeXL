//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktAPIEntry.h
/// \brief  A Vulkan-specific entry for a traced API call.
//=============================================================================

#ifndef __VKT_API_ENTRY_H__
#define __VKT_API_ENTRY_H__

#include "../../../Common/Tracing/APIEntry.h"
#include "../Util/vktUtil.h"

class VktWrappedCmdBuf;

//-----------------------------------------------------------------------------
/// Used to track all Vulkan API calls that are traced at runtime.
/// All API calls can be traced, and only some can be profiled.
//-----------------------------------------------------------------------------
class VktAPIEntry : public APIEntry
{
public:
    VktAPIEntry(UINT inThreadId, FuncId inFunctionId, const std::string& inArguments, VktWrappedCmdBuf* pWrappedCmdBuf);
    VktAPIEntry(UINT inThreadId, FuncId inFunctionId, ParameterEntry* pParams, UINT32 paramCount, VktWrappedCmdBuf* pWrappedCmdBuf);

    virtual ~VktAPIEntry() {}

    virtual const char* GetAPIName() const;
    virtual void AppendAPITraceLine(gtASCIIString& out, double startTime, double endTime) const;
    virtual bool IsDrawCall() const;

    void SetReturnValue(INT64 inReturnValue);

    /// Associate a traced API call with the profiler results coming out of QueueSubmit.
    UINT64 m_sampleId;

    /// The return value of the call.
    VkResult m_returnValue;

    /// Used for GPU trace.
    VktWrappedCmdBuf* m_pWrappedCmdBuf;

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
    virtual void GetParameterAsString(PARAMETER_TYPE paramType, const char dataLength, const char* pRawData, char* ioParameterString) const;
};

#endif // __VKT_API_ENTRY_H__