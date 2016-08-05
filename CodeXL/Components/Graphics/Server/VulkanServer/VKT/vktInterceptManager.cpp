//==============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   vktInterceptManager.cpp
/// \brief  Implementation file for Vulkan API interception manager.
///         This critical class implements InterceptorBase, and has the critical
///         role of determining what to do as different API functions are
///         intercepted.
//==============================================================================

#include "vktLayerManager.h"

#include "vktInterceptManager.h"
#include "Tracing/vktTraceAnalyzerLayer.h"
#include "FrameDebugger/vktFrameDebuggerLayer.h"
#include "Objects/vktObjectDatabaseProcessor.h"
#include "Objects/vktInstanceBase.h"
#include "Objects/Wrappers/vktWrappedCmdBuf.h"
#include "Objects/Wrappers/vktWrappedQueue.h"

//-----------------------------------------------------------------------------
/// Constructor.
//-----------------------------------------------------------------------------
VktInterceptManager::VktInterceptManager() :
    m_profilerEnabled(false)
{
}

//-----------------------------------------------------------------------------
/// Retrieve a pointer to the parent LayerManager used by this tool.
/// \returns A pointer to the parent LayerManager used by this tool.
//-----------------------------------------------------------------------------
ModernAPILayerManager* VktInterceptManager::GetParentLayerManager()
{
    return VktLayerManager::GetLayerManager();
}

//-----------------------------------------------------------------------------
/// Initialize a simple struct with basic device information.
/// \param device The device.
/// \param physicalDevice The physical device.
/// \returns A DeviceInfo struct.
//-----------------------------------------------------------------------------
DeviceInfo VktInterceptManager::InitializeDeviceInfo(VkDevice device, VkPhysicalDevice physicalDevice)
{
    ScopeLock lock(&m_deviceInfoMapMutex);

    DeviceInfo resultInfo = DeviceInfo();

    DeviceInfoMap::iterator deviceInfoIter = m_deviceInfoMap.find(device);

    if (deviceInfoIter == m_deviceInfoMap.end())
    {
        resultInfo.physicalDevice = physicalDevice;
        resultInfo.device         = device;

        m_deviceInfoMap[device] = resultInfo;
    }
    else
    {
        // The DeviceInfo structure already exists- return it so it gets reused.
        resultInfo = deviceInfoIter->second;
    }

    return resultInfo;
}

//-----------------------------------------------------------------------------
/// Get a string representation of a queue's capabilities.
/// \param queue The queue
/// \return The queue caps string.
//-----------------------------------------------------------------------------
std::string VktInterceptManager::GetQueueDesc(VkQueue queue)
{
    std::string out = "";

    UINT queueFlags = GetWrappedQueue(queue)->GetQueueFlags();

    out = "(";

    bool separator = false;

    if (queueFlags & VK_QUEUE_GRAPHICS_BIT)
    {
        out.append("Gfx");
        separator = true;
    }

    if (queueFlags & VK_QUEUE_COMPUTE_BIT)
    {
        if (separator == true)
        {
            out.append("|");
        }

        out.append("Compute");
        separator = true;
    }

    if (queueFlags & VK_QUEUE_TRANSFER_BIT)
    {
        if (separator == true)
        {
            out.append("|");
        }

        out.append("Xfer");
        separator = true;
    }

    if (queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
    {
        if (separator == true)
        {
            out.append("|");
        }

        out.append("SpBind");
    }

    out.append(")");

    return out;
}

//-----------------------------------------------------------------------------
/// Fetch device information.
/// \param device The device.
/// \returns A DeviceInfo struct.
//-----------------------------------------------------------------------------
DeviceInfo VktInterceptManager::FindDeviceInfo(VkDevice device)
{
    DeviceInfo deviceInfo = DeviceInfo();

    if (m_deviceInfoMap.find(device) != m_deviceInfoMap.end())
    {
        deviceInfo = m_deviceInfoMap[device];
    }

    return deviceInfo;
}

//-----------------------------------------------------------------------------
/// Handler used before the real runtime implementation of an API call has been invoked.
/// \param funcId The function ID for the call being traced.
/// \param pParams The function's arguments.
/// \param paramCount The function's number of arguments.
/// \param pWrappedCmdBuf The number of parameters for this API call.
//-----------------------------------------------------------------------------
VktAPIEntry* VktInterceptManager::PreCall(FuncId funcId, ParameterEntry* pParams, int paramCount, VktWrappedCmdBuf* pWrappedCmdBuf)
{
    VktTraceAnalyzerLayer* pTraceAnalyzerLayer = VktTraceAnalyzerLayer::Instance();

    DWORD threadId = osGetCurrentThreadId();
    VktAPIEntry* pNewEntry = new VktAPIEntry(threadId, funcId, pParams, paramCount, pWrappedCmdBuf);

    if (pWrappedCmdBuf != nullptr)
    {
        VktFrameProfilerLayer::Instance()->PreCall(funcId, pWrappedCmdBuf);
    }

    pTraceAnalyzerLayer->BeforeAPICall();

    return pNewEntry;
}

//-----------------------------------------------------------------------------
/// Responsible for the post-call instrumentation of every Vulkan API call.
/// \param pNewEntry The new API entry for this call.
/// \param returnValue The return value for this call.
//-----------------------------------------------------------------------------
void VktInterceptManager::PostCall(VktAPIEntry* pNewEntry, int returnValue)
{
    VktTraceAnalyzerLayer* pTraceAnalyzerLayer = VktTraceAnalyzerLayer::Instance();

    pTraceAnalyzerLayer->LogAPICall(pNewEntry);

    if (pNewEntry->m_pWrappedCmdBuf != nullptr)
    {
        VktFrameProfilerLayer::Instance()->PostCall(pNewEntry, pNewEntry->mFunctionId, pNewEntry->m_pWrappedCmdBuf);
    }

    pNewEntry->SetReturnValue(returnValue);
}
