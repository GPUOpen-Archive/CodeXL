//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/APIColorMap.cpp $
/// \version $Revision: #14 $
/// \brief  This file contains the api color map singleton class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/APIColorMap.cpp#14 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================

#include <qtIgnoreCompilerWarnings.h>

// Qt:
#include <QtCore>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Backend
#include "../DX12Trace/DX12FunctionDefs.h"

// Local:
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>
#include <AMDTGpuProfiling/APIColorMap.h>

///< color used for perf marker timeline items
static const QColor s_LIGHT_BLUE_PERF_MARKER_COLOR = QColor(173, 216, 230);


APIColorMap::APIColorMap()
{
    m_commandListsColors << QColor(173, 216, 230);
    m_commandListsColors << QColor(173, 0, 230);
    m_commandListsColors << QColor(173, 216, 0);
    m_commandListsColors << QColor(0, 216, 230);
    m_commandListsColors << QColor(173, 23, 230);
    m_commandListsColors << QColor(173, 216, 23);
    m_commandListsColors << QColor(23, 216, 230);
    m_commandListsColors << QColor(173, 255, 230);
    m_commandListsColors << QColor(255, 216, 230);
    m_commandListsColors << QColor(173, 216, 255);
    m_commandListsColors << QColor(255, 255, 230);
    m_commandListsColors << QColor(173, 255, 255);

}

APIColorMap::~APIColorMap()
{
    for (ColorMap::iterator i = m_apiColorMap.begin(); i != m_apiColorMap.end(); ++i)
    {
        SAFE_DELETE(*i);
    }

    for (APIToColorMap::iterator i = m_apiTypeToColorMap.begin(); i != m_apiTypeToColorMap.end(); ++i)
    {
        SAFE_DELETE(*i);
    }

    m_apiColorMap.clear();
    m_apiTypeToColorMap.clear();
}

QColor APIColorMap::GetAPIColor(const QString& apiName, const QColor& defaultColor)
{
    // check the cache first
    if (m_apiColorMap.contains(apiName))
    {
        APIColorInfo* colorInfo = m_apiColorMap[apiName];

        if (colorInfo->GetUseDefaultColor())
        {
            return defaultColor;
        }
        else
        {
            return colorInfo->GetColor();
        }
    }

    // not in the cache, figure out the color to use
    QColor retVal = defaultColor;
    bool useDefault = false;

    if (apiName == "clEnqueueNDRangeKernel" || apiName == "clEnqueueTask" || apiName == "clEnqueueNativeKernel")
    {
        retVal = Qt::darkGreen;
    }
    else if (apiName == "clEnqueueReadBuffer" || apiName == "clEnqueueReadBufferRect" || apiName == "clEnqueueReadImage")
    {
        retVal = Qt::darkBlue;
    }
    else if (apiName == "clEnqueueWriteBuffer" || apiName == "clEnqueueWriteBufferRect" || apiName == "clEnqueueWriteImage")
    {
        retVal = Qt::blue;
    }
    else if (apiName == "clEnqueueCopyBuffer" || apiName == "clEnqueueCopyBufferRect" || apiName == "clEnqueueCopyImage" ||
             apiName == "clEnqueueCopyImageToBuffer" || apiName == "clEnqueueCopyBufferToImage" || apiName == "clEnqueueMapBuffer" ||
             apiName == "clEnqueueMapImage" || apiName == "clEnqueueUnmapMemObject" || apiName == "clEnqueueAcquireGLObjects" ||
             apiName == "clEnqueueFillImage" || apiName == "clEnqueueFillBuffer" ||
             apiName == "clEnqueueReleaseGLObjects" || apiName == "clEnqueueAcquireD3D10ObjectsKHR" || apiName == "clEnqueueReleaseD3D10ObjectsKHR")
    {
        retVal = Qt::darkMagenta;
    }
    else
    {
        useDefault = true;
    }

    // add to cache
    APIColorInfo* colorInfo = new APIColorInfo(retVal, useDefault);

    m_apiColorMap.insert(apiName, colorInfo);

    return retVal;
}

QColor APIColorMap::GetAPIColor(ProfileSessionDataItem::ProfileItemType itemType, unsigned int apiId, const QColor& defaultColor)
{
    QColor retVal = defaultColor;

    QPair<ProfileSessionDataItem::ProfileItemType, unsigned int> itemId(itemType, apiId);

    // check the cache first
    if (m_apiTypeToColorMap.contains(itemId))
    {
        APIColorInfo* colorInfo = m_apiTypeToColorMap[itemId];

        if (!colorInfo->GetUseDefaultColor())
        {
            retVal = colorInfo->GetColor();
        }
    }
    else
    {
        if (itemType.m_itemMainType == ProfileSessionDataItem::CL_API_PROFILE_ITEM)
        {
            bool useDefault = true;

            if (itemType.IsDispatchCommand())
            {
                if (itemType.m_itemSubType & CL_ENQUEUE_KERNEL)
                {
                    retVal = Qt::darkGreen;
                }
                else if (itemType.m_itemSubType & CL_ENQUEUE_MEM)
                {
                    if ((apiId == CL_FUNC_TYPE_clEnqueueReadBuffer) || (apiId == CL_FUNC_TYPE_clEnqueueReadBufferRect) || (apiId == CL_FUNC_TYPE_clEnqueueReadImage))
                    {
                        retVal = Qt::blue;
                    }
                    else if ((apiId == CL_FUNC_TYPE_clEnqueueWriteBuffer) || (apiId == CL_FUNC_TYPE_clEnqueueWriteBufferRect) || (apiId == CL_FUNC_TYPE_clEnqueueWriteImage))
                    {
                        retVal = Qt::darkBlue;
                    }
                }
                else if (itemType.m_itemSubType & CL_ENQUEUE_DATA_OPERATIONS)
                {
                    retVal = Qt::darkMagenta;
                }

                useDefault = false;
            }

            // add to cache
            APIColorInfo* colorInfo = new APIColorInfo(retVal, useDefault);

            m_apiTypeToColorMap.insert(itemId, colorInfo);
        }

        else if ((itemType.m_itemMainType == ProfileSessionDataItem::DX12_API_PROFILE_ITEM)
            || (itemType.m_itemMainType == ProfileSessionDataItem::DX12_GPU_PROFILE_ITEM))
        {
            // Get the DX12 API type
            eAPIType apiType = DX12FunctionDefs::GetAPIGroupFromAPI((FuncId)itemType.m_itemSubType);

            switch (apiType)
            {
            case kAPIType_BindingCommand:
            {
                retVal = QColor::fromRgb(14, 130, 94);
                break;
            }

            case kAPIType_ClearCommand:
            {
                retVal = QColor::fromRgb(200, 24, 216);
                break;
            }

            case kAPIType_Command:
            {
                retVal = QColor::fromRgb(255, 128, 0);
                break;
            }

            case kAPIType_Copy:
            {
                retVal = QColor::fromRgb(0, 156, 51);
                break;
            }

            case kAPIType_Create:
            {
                retVal = QColor::fromRgb(0, 156, 51);
                break;
            }

            case kAPIType_Debug:
            {
                retVal = QColor::fromRgb(216, 24, 31);
                break;
            }

            case kAPIType_DrawCommand:
            {
                retVal = QColor::fromRgb(60, 24, 216);
                break;
            }

            case kAPIType_General:
            {
                retVal = QColor::fromRgb(57, 151, 17);
                break;
            }

            case kAPIType_Paging:
            {
                retVal = QColor::fromRgb(24, 109, 216);
                break;
            }

            case kAPIType_Resource:
            {
                retVal = QColor::fromRgb(230, 138, 0);
                break;
            }

            case kAPIType_StageCommand:
            {
                retVal = QColor::fromRgb(127, 24, 216);
                break;
            }

            case kAPIType_Synchronization:
            {
                retVal = QColor::fromRgb(216, 24, 165);
                break;
            }

            default:
                break;
            }
        }

        else if ((itemType.m_itemMainType == ProfileSessionDataItem::VK_API_PROFILE_ITEM)
            || (itemType.m_itemMainType == ProfileSessionDataItem::VK_GPU_PROFILE_ITEM))
        {
            // Get the Vulkan API type
            vkAPIType apiType = vulkanFunctionDefs::GetAPIGroupFromAPI((vk_FUNC_TYPE)itemType.m_itemSubType);

            switch (apiType)
            {
            case vkAPIType_Command:
            {
                retVal = QColor::fromRgb(255, 128, 0);
                break;
            }

            case vkAPIType_CommandDraw:
            {
                retVal = QColor::fromRgb(60, 24, 216);
                break;
            }
            case vkAPIType_General:
            {
                retVal = QColor::fromRgb(57, 151, 17);
                break;
            }
            case vkAPIType_Destroy:
            case vkAPIType_Create:
            {
                retVal = QColor::fromRgb(0, 156, 51);
                break;
            }
            case vkAPIType_Get:
            {
                retVal = QColor::fromRgb(216, 24, 31);
                break;
            }
            case vkAPIType_Memory:
            {
                retVal = QColor::fromRgb(230, 138, 0);
                break;
            }
            case vkAPIType_Queue:
            {
                retVal = QColor::fromRgb(14, 130, 94);
                break;
            }
            case vkAPIType_Sync:
            {
                retVal = QColor::fromRgb(216, 24, 165);
                break;
            }
            case vkAPIType_KHR:
            {
            }
            default:
                break;
            }

            // add to cache
            APIColorInfo* pColorInfo = new APIColorInfo(retVal, false);

            m_apiTypeToColorMap.insert(itemId, pColorInfo);
        }
    }

    return retVal;
}

QColor APIColorMap::GetPerfMarkersColor() const
{
    return s_LIGHT_BLUE_PERF_MARKER_COLOR;
}

QColor APIColorMap::GetCommandListColor(int index) const
{
    QColor retVal = s_LIGHT_BLUE_PERF_MARKER_COLOR;
    int indexInVec = index % (m_commandListsColors.size() - 1);
    GT_IF_WITH_ASSERT(indexInVec < m_commandListsColors.size() && index >= 0)
    {
        retVal = m_commandListsColors[indexInVec];
    }
    return retVal;
}

