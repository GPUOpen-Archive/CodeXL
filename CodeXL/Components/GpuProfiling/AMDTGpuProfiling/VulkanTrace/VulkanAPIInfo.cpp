#include "VulkanAPIInfo.h"

VKAPIInfo::VKAPIInfo() : APIInfo(), m_apiType(vkAPIType_Unknown), m_apiId(FuncId_vkUNDEFINED), m_isGPU(false), m_sampleId(0)
{

}

VKGPUTraceInfo::VKGPUTraceInfo() : VKAPIInfo(), m_commandListType(0)
{
    m_isGPU = true;
}
