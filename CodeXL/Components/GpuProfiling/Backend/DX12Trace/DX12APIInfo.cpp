#include "DX12APIInfo.h"

using namespace GPULogger;


DX12APIInfo::DX12APIInfo() : APIInfo(), m_apiType(kAPIType_Unknown), m_apiId(FuncId_UNDEFINED), m_interfacePtrStr(""), m_isGPU(false), m_sampleId(0)
{

}

DX12GPUTraceInfo::DX12GPUTraceInfo() : DX12APIInfo(), m_commandListType(0), m_commandQueuePtrStr("")
{
    m_isGPU = true;
}
