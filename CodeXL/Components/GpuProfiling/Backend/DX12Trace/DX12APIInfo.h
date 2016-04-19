#ifndef _DX12_API_INFO_H_
#define _DX12_API_INFO_H_

#include "../Common/APIInfo.h"
#include "Server/DX12Server/D3D12Enumerations.h"

//------------------------------------------------------------------------------------
/// DX12 API Base class
//------------------------------------------------------------------------------------
class DX12APIInfo : public APIInfo
{
public:

    /// Constructor
    DX12APIInfo();

    /// Virtual destructor
    virtual ~DX12APIInfo() {}

    /// DX12 API Type
    eAPIType m_apiType;

    /// DX12 API ID, defined in ../Common/DX12FunctionEnumDefs.h
    FuncId m_apiId;

    // The interface pointer as string
    std::string m_interfacePtrStr;

    /// True for GPU items
    bool m_isGPU;

    /// The command sample ID
    int m_sampleId;
};

class DX12GPUTraceInfo : public DX12APIInfo
{
public:

    /// Constructor
    DX12GPUTraceInfo();

    /// Command list type (D3D12_COMMAND_LIST_TYPE)
    int m_commandListType;

    /// The command queue pointer as string
    std::string m_commandQueuePtrStr;

};

#endif //_DX12_API_INFO_H_
