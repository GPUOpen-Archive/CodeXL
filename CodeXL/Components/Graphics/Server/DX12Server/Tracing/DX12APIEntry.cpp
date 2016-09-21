//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   DX12APIEntry.cpp
/// \brief  A DX12-specific entry for a traced API call.
//==============================================================================

#include <Strsafe.h>

#include <AMDTBaseTools/Include/gtStringConstants.h>
#include "DX12APIEntry.h"
#include "../DX12Defines.h"
#include "../DXCommonSource/StringifyDxgiFormatEnums.h"
#include "../Objects/DX12ObjectDatabaseProcessor.h"
#include "../Tracing/DX12TraceAnalyzerLayer.h"
#include "../Util/DX12Utilities.h"
#include "../../Common/TypeToString.h"
#include "../SymbolSerializers/DX12Serializers.h"
#include "../SymbolSerializers/DX12CoreSymbolSerializers.h"

//-----------------------------------------------------------------------------
/// Constructor used to initialize members of new DX12APIEntry instances.
/// \param inThreadId The thread Id that the call was invoked from.
/// \param inInterfaceWrapper The wrapped instance used to invoke the API call.
/// \param inFunctionId The FunctionId of the API call.
/// \param inArguments The arguments used in the invocation of the API function.
/// \param inReturnValue The return value of the API call.
/// \param inReturnValueFlags The flags used in printing the return value.
//-----------------------------------------------------------------------------
DX12APIEntry::DX12APIEntry(UINT inThreadId, IUnknown* inInterfaceWrapper, FuncId inFunctionId, const std::string& inArguments, INT64 inReturnValue, ReturnDisplayType inReturnValueFlags)
    : APIEntry(inThreadId, inFunctionId, inArguments)
    , mWrapperInterface(inInterfaceWrapper)
    , mReturnValue(inReturnValue)
    , mReturnValueFlags(inReturnValueFlags)
    , mSampleId(0)
    , mPreBottomTimestamp(0)
    , mPostBottomTimestamp(0)
    , mbGotProfileResults(false)
{
}

//-----------------------------------------------------------------------------
/// Constructor used to initialize members of new DX12APIEntry instances.
/// \param inThreadId The thread Id that the call was invoked from.
/// \param inInterfaceWrapper The wrapped instance used to invoke the API call.
/// \param inFunctionId The FunctionId of the API call.
/// \param inNumParameters The number of parameters used to invoke this API call.
/// \param inParameters A ParameterEntry structure describing the parameter metadata for this API call.
//-----------------------------------------------------------------------------
DX12APIEntry::DX12APIEntry(UINT inThreadId, IUnknown* inInterfaceWrapper, FuncId inFunctionId, UINT32 inNumParameters, ParameterEntry* inParameters)
    : APIEntry(inThreadId, inFunctionId, inNumParameters)
    , mWrapperInterface(inInterfaceWrapper)
    , mReturnValue(FUNCTION_RETURNS_VOID)
    , mReturnValueFlags(RETURN_VALUE_DECIMAL)
    , mSampleId(0)
    , mPreBottomTimestamp(0)
    , mPostBottomTimestamp(0)
    , mbGotProfileResults(false)
{
    if (mParameterBuffer != nullptr)
    {
        for (UINT32 i = 0; i < inNumParameters; i++)
        {
            AddParameter(i, inParameters[i].mType, inParameters[i].mData);
        }
    }
}

//-----------------------------------------------------------------------------
/// Retrieve the API name string for a DX12APIEntry instance.
/// \returns A null-terminated string containing the API name.
//-----------------------------------------------------------------------------
const char* DX12APIEntry::GetAPIName() const
{
    return DX12TraceAnalyzerLayer::Instance()->GetFunctionNameFromId(this->mFunctionId);
}

//-----------------------------------------------------------------------------
/// Write a DX12-specific APITrace response line into the incoming string stream.
/// \param out The stringstream instance that each trace response line is written to.
/// \param inStartTime The start time for the API call.
/// \param inEndTime The end time for the API call.
//-----------------------------------------------------------------------------
void DX12APIEntry::AppendAPITraceLine(gtASCIIString& out, double inStartTime, double inEndTime) const
{
    gtASCIIString returnValueString;
    PrintReturnValue(mReturnValue, mReturnValueFlags, returnValueString);

    // Use the database processor to get a pointer to the object database.
    DX12ObjectDatabaseProcessor* databaseProcessor = DX12ObjectDatabaseProcessor::Instance();
    DX12WrappedObjectDatabase* objectDatabase = static_cast<DX12WrappedObjectDatabase*>(databaseProcessor->GetObjectDatabase());

    // Use the object database to retrieve wrapper info for the given interface.
    IDX12InstanceBase* wrapperInfo = objectDatabase->GetMetadataObject(mWrapperInterface);

    // APIType APIFunctionId InterfacePtr D3D12Interface_FunctionName(Parameters) = ReturnValue StartMillisecond EndMillisecond SampleId
    void* handle = nullptr;
    const char* type = "\0";

    gtASCIIString parameterString;
    const char* parameters = GetParameterString(parameterString);

    if (wrapperInfo)
    {
        handle = wrapperInfo->GetApplicationHandle();
        type = wrapperInfo->GetTypeAsString();
    }
    else
    {
        // if there's no wrapper, it's a Present call.
        // Assume type is a swap chain until the Present call is wrapped properly
        type = "IDXGISwapChain";
    }

    out += IntToString(DX12TraceAnalyzerLayer::Instance()->GetAPIGroupFromAPI(mFunctionId));
    out += " ";

    out += IntToString(mFunctionId);
    out += " ";

    out += "0x";
    out += UINT64ToHexString((UINT64)handle);
    out += " ";

    out += type;
    out += "_";

    out += GetAPIName();

    out += "(";
    out += parameters;
    out += ") = ";

    out += returnValueString.asCharArray();

    out += " ";
    out += DoubleToString(inStartTime);

    out += " ";
    out += DoubleToString(inEndTime);

    out += " ";
    out += UINT64ToString(mSampleId);

    out += "\n";
}

//-----------------------------------------------------------------------------
/// Check if this logged APIEntry is a Draw call.
/// \returns True if the API is a draw call. False if it's not.
//-----------------------------------------------------------------------------
bool DX12APIEntry::IsDrawCall() const
{
    return DX12FrameProfilerLayer::Instance()->ShouldProfileFunction(mFunctionId);
}

//-----------------------------------------------------------------------------
/// Set the return value for this logged APIEntry
/// \param inReturnValue The return value
/// \param inReturnValueFlags A flag indicating how the return value should
/// be displayed
//-----------------------------------------------------------------------------
void DX12APIEntry::SetReturnValue(INT64 inReturnValue, ReturnDisplayType inReturnValueFlags)
{
    mReturnValue = inReturnValue;
    mReturnValueFlags = inReturnValueFlags;
}

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
void DX12APIEntry::AddParameter(unsigned int index, int type, const void* pParameterValue)
{
    PsAssert(index < mNumParameters);

    if (mParameterBuffer)
    {
        unsigned int bufferLength = BYTES_PER_PARAMETER_DATA;
        unsigned int length = 0;
        char* buffer = mParameterBuffer;

        // find index and write data type
        buffer += (index * BYTES_PER_PARAMETER);
        memcpy(buffer, &type, sizeof(PARAMETER_TYPE));

        // increment pointer and write data length. Standard data types don't store length
        // to save time since the length is known and will be fixed up when displaying the data
        buffer += sizeof(PARAMETER_TYPE);

        switch (type)
        {
            // TODO: have this as a look up table
            case PARAMETER_POINTER:
            case PARAMETER_POINTER_SPECIAL:
                length = (char)sizeof(void*);
                break;

            case PARAMETER_INT:
            case PARAMETER_UNSIGNED_INT:
                length = (char)sizeof(unsigned int);
                break;

            case PARAMETER_UNSIGNED_CHAR:
                length = (char)sizeof(unsigned char);
                break;

            case PARAMETER_FLOAT:
                length = (char)sizeof(float);
                break;

            case PARAMETER_BOOL:
                length = (char)sizeof(BOOL);
                break;

            case PARAMETER_UINT64:
                length = (char)sizeof(UINT64);
                break;

            case PARAMETER_SIZE_T:
                length = (char)sizeof(size_t);
                break;

            case PARAMETER_STRING:
                length = (char)strlen((const char*)pParameterValue);
                length++;                       // add null terminator to length

                if (length > bufferLength)
                {
                    // 1 byte less to account for terminator
                    length = bufferLength - 1;
                    Log(logMESSAGE, "DX12APIEntry::AddParameter: string parameter too large. String will be truncated\n");
                }

                break;

            case PARAMETER_WIDE_STRING:
                length = (char)wcslen((const LPCWSTR)pParameterValue);
                length++;                       // add null terminator to length
                length *= sizeof(WCHAR);        // convert length to bytes

                if (length > bufferLength)
                {
                    // round down length to even number
                    length = ((bufferLength - 2) & 0xfffffffe);
                    Log(logMESSAGE, "DX12APIEntry::AddParameter: wide string parameter wide string too large. String will be truncated\n");
                }

                break;

            case PARAMETER_DX12_DXGI_FORMAT:
                length = (char)sizeof(DXGI_FORMAT);
                break;

            case PARAMETER_DX12_PRIMITIVE_TOPOLOGY:
                length = (char)sizeof(D3D12_PRIMITIVE_TOPOLOGY);
                break;

            case PARAMETER_DX12_QUERY_TYPE:
                length = (char)sizeof(D3D12_QUERY_TYPE);
                break;

            case PARAMETER_DX12_PREDICATION_OP:
                length = (char)sizeof(D3D12_PREDICATION_OP);
                break;

            case PARAMETER_DX12_COMMAND_LIST:
                length = (char)sizeof(D3D12_COMMAND_LIST_TYPE);
                break;

            case PARAMETER_DX12_FEATURE:
                length = (char)sizeof(D3D12_FEATURE);
                break;

            case PARAMETER_DX12_DESCRIPTOR_HEAP:
                length = (char)sizeof(D3D12_DESCRIPTOR_HEAP_TYPE);
                break;

            case PARAMETER_DX12_HEAP_TYPE:
                length = (char)sizeof(D3D12_HEAP_TYPE);
                break;

            case PARAMETER_DX12_RESOURCE_STATES:
                length = (char)sizeof(D3D12_RESOURCE_STATES);
                break;

            default:
                break;
        }

        bufferSize_t len = (bufferSize_t)length;
        memcpy(buffer, &len, sizeof(bufferSize_t));
        buffer += sizeof(bufferSize_t);

        if (type == PARAMETER_POINTER || type == PARAMETER_POINTER_SPECIAL)
        {
            // write the value of the pointer
            memcpy_s(buffer, bufferLength, &pParameterValue, length);
        }
        else
        {
            // make sure last 2 bytes are null characters incase it's a string or wide string.
            // This is used as the terminator for a truncated string
            int* bufPtr = (int*)&buffer[bufferLength - 4];
            *bufPtr = 0;
            // write what the pointer is pointing to
            memcpy_s(buffer, bufferLength, pParameterValue, length);
        }
    }
}

//-----------------------------------------------------------------------------
/// Convert an API parameter from raw data to a string for display
/// \param paramType the data type of the parameter
/// \param dataLength the number of bytes used to contain the data
/// \param pRawData a pointer to the raw data
/// \param ioParameterString a buffer passed in where the string is to be stored
//-----------------------------------------------------------------------------
void DX12APIEntry::GetParameterAsString(PARAMETER_TYPE paramType, UINT dataLength, const char* pRawData, char* ioParameterString) const
{
    int bufferLength = BYTES_PER_PARAMETER_DATA;

    switch (paramType)
    {
        case PARAMETER_POINTER:
        {
            void* data = nullptr;
            memcpy(&data, pRawData, sizeof(void*));
            StringCbPrintf(ioParameterString, bufferLength, GT_POINTER_FORMAT, data);
            break;
        }

        case PARAMETER_POINTER_SPECIAL:
        {
            void* data = nullptr;
            memcpy(&data, pRawData, sizeof(void*));
            StringCbPrintf(ioParameterString, bufferLength, "+" GT_POINTER_FORMAT, data);
            break;
        }

        case PARAMETER_INT:
        {
            int data = 0;
            memcpy(&data, pRawData, sizeof(int));
            StringCbPrintf(ioParameterString, bufferLength, "%d", data);
            break;
        }

        case PARAMETER_UNSIGNED_INT:
        {
            unsigned int data = 0;
            memcpy(&data, pRawData, sizeof(unsigned int));
            StringCbPrintf(ioParameterString, bufferLength, "%u", data);
            break;
        }

        case PARAMETER_UNSIGNED_CHAR:
        {
            unsigned char data = 0;
            memcpy(&data, pRawData, sizeof(unsigned char));
            StringCbPrintf(ioParameterString, bufferLength, "%hhu", data);
            break;
        }

        case PARAMETER_BOOL:
        {
            BOOL data = FALSE;
            memcpy(&data, pRawData, sizeof(BOOL));
            StringCbPrintf(ioParameterString, bufferLength, "%s", DX12Util::PrintBool(data));
            break;
        }

        case PARAMETER_FLOAT:
        {
            float data = 0.0f;
            memcpy(&data, pRawData, sizeof(float));
            StringCbPrintf(ioParameterString, bufferLength, "%f", data);
            break;
        }

        case PARAMETER_UINT64:
        {
            UINT64 data = 0;
            memcpy(&data, pRawData, sizeof(UINT64));
            StringCbPrintf(ioParameterString, bufferLength, "%llu", data);
            break;
        }

        case PARAMETER_SIZE_T:
        {
            size_t data = 0;
            memcpy(&data, pRawData, sizeof(size_t));
            StringCbPrintf(ioParameterString, bufferLength, "%Iu", data);
            break;
        }

        case PARAMETER_STRING:
        {
            // copy string data directly to output buffer
            memcpy_s(ioParameterString, bufferLength, pRawData, dataLength);
            break;
        }

        case PARAMETER_WIDE_STRING:
        {
            StringCbPrintf(ioParameterString, bufferLength, "%ls", pRawData);
            break;
        }

        case PARAMETER_DX12_GUID:
        {
            GUID guid = {};
            memcpy(&guid, pRawData, sizeof(GUID));
            gtASCIIString guidString;
            DX12Util::PrintREFIID(guid, guidString);
            StringCbPrintf(ioParameterString, bufferLength, "%s", guidString.asCharArray());
            break;
        }

        case PARAMETER_DX12_REFIID:
        {
            IID riid = {};
            memcpy(&riid, pRawData, sizeof(IID));
            gtASCIIString refiidString;
            DX12Util::PrintREFIID(riid, refiidString);
            StringCbPrintf(ioParameterString, bufferLength, "%s", refiidString.asCharArray());
            break;
        }

        case PARAMETER_DX12_DXGI_FORMAT:
        {
            DXGI_FORMAT format = {};
            memcpy(&format, pRawData, sizeof(DXGI_FORMAT));
            gtASCIIString dxgiFormat = Stringify_DXGI_FORMAT(format);
            StringCbPrintf(ioParameterString, bufferLength, "%s", dxgiFormat.asCharArray());
            break;
        }

        case PARAMETER_DX12_PRIMITIVE_TOPOLOGY:
        {
            D3D12_PRIMITIVE_TOPOLOGY primitiveTopology = {};
            memcpy(&primitiveTopology, pRawData, sizeof(D3D12_PRIMITIVE_TOPOLOGY));
            StringCbPrintf(ioParameterString, bufferLength, "%s", DX12CustomSerializers::WritePrimitiveTopology(primitiveTopology));
            break;
        }

        case PARAMETER_DX12_QUERY_TYPE:
        {
            D3D12_QUERY_TYPE type = {};
            memcpy(&type, pRawData, sizeof(D3D12_QUERY_TYPE));
            StringCbPrintf(ioParameterString, bufferLength, "%s", DX12CoreSerializers::WriteQueryTypeEnumAsString(type));
            break;
        }

        case PARAMETER_DX12_PREDICATION_OP:
        {
            D3D12_PREDICATION_OP operation = {};
            memcpy(&operation, pRawData, sizeof(D3D12_PREDICATION_OP));
            StringCbPrintf(ioParameterString, bufferLength, "%s", DX12CoreSerializers::WritePredicationOpEnumAsString(operation));
            break;
        }

        case PARAMETER_DX12_COMMAND_LIST:
        {
            D3D12_COMMAND_LIST_TYPE type = {};
            memcpy(&type, pRawData, sizeof(D3D12_COMMAND_LIST_TYPE));
            StringCbPrintf(ioParameterString, bufferLength, "%s", DX12CoreSerializers::WriteCommandListTypeEnumAsString(type));
            break;
        }

        case PARAMETER_DX12_FEATURE:
        {
            D3D12_FEATURE feature = {};
            memcpy(&feature, pRawData, sizeof(D3D12_FEATURE));
            StringCbPrintf(ioParameterString, bufferLength, "%s", DX12CoreSerializers::WriteFeatureEnumAsString(feature));
            break;
        }

        case PARAMETER_DX12_DESCRIPTOR_HEAP:
        {
            D3D12_DESCRIPTOR_HEAP_TYPE type = {};
            memcpy(&type, pRawData, sizeof(D3D12_DESCRIPTOR_HEAP_TYPE));
            StringCbPrintf(ioParameterString, bufferLength, "%s", DX12CoreSerializers::WriteDescriptorHeapTypeEnumAsString(type));
            break;
        }

        case PARAMETER_DX12_HEAP_TYPE:
        {
            D3D12_HEAP_TYPE type = {};
            memcpy(&type, pRawData, sizeof(D3D12_HEAP_TYPE));
            StringCbPrintf(ioParameterString, bufferLength, "%s", DX12CoreSerializers::WriteHeapTypeEnumAsString(type));
            break;
        }

        case PARAMETER_DX12_RESOURCE_STATES:
        {
            D3D12_RESOURCE_STATES states = {};
            memcpy(&states, pRawData, sizeof(D3D12_RESOURCE_STATES));
            StringCbPrintf(ioParameterString, bufferLength, "%s", DX12CoreSerializers::WriteResourceStatesEnumAsString(states));
            break;
        }

        default:
        {
            PsAssert(0);
            break;
        }
    }
}