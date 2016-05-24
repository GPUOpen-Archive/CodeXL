#include "VulkanAPIInfo.h"

using namespace GPULogger;


VKAPIInfo::VKAPIInfo() : APIInfo(), m_apiType(vkAPIType_Unknown), m_apiId(FuncId_vkUNDEFINED), m_isGPU(false)
{

}

VKGPUTraceInfo::VKGPUTraceInfo() : VKAPIInfo(), m_commandListType(0)
{
    m_isGPU = true;
}
