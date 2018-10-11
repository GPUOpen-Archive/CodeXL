//=====================================================================
// Copyright (c) 2013-2018 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file
/// \brief  This file contains the api color map singleton class
//=====================================================================

#include <qtIgnoreCompilerWarnings.h>

// Qt:
#include <QtCore>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acColours.h>


// Local:
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>
#include <AMDTGpuProfiling/APIColorMap.h>

///< color used for perf marker timeline items
static const QColor s_PERF_MARKER_COLOR = QColor(255, 123, 0); // orange

///< color used for command buffer timeline items
static const QColor s_COMMAND_BUFFER_COLOR = QColor(173, 216, 0); // light blue

APIColorMap::APIColorMap()
{
    m_commandListsColors << acQAMD_GREEN_OVERLAP_COLOUR;
    m_commandListsColors << acQAMD_RED_OVERLAP_COLOUR;
    m_commandListsColors << acQAMD_CYAN_OVERLAP_COLOUR;
    m_commandListsColors << acQAMD_ORANGE_OVERLAP_COLOUR;
    m_commandListsColors << acGetCodeXLColorScaleColor(AC_CODEXL_BLUE, 0);
    m_commandListsColors << acQAMD_GRAY1_COLOUR;
    m_commandListsColors << acGetCodeXLColorScaleColor(AC_CODEXL_MAGENTA, 2);
}

APIColorMap::~APIColorMap()
{
    for (ColorMap::iterator i = m_apiColorMap.begin(); i != m_apiColorMap.end(); ++i)
    {
        SAFE_DELETE(*i);
    }

    m_apiColorMap.clear();
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

/*QColor APIColorMap::GetAPIColor(ProfileSessionDataItem::ProfileItemType itemType, unsigned int apiId, const QColor& defaultColor)
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

    }

    return retVal;
}*/

QColor APIColorMap::GetPerfMarkersColor() const
{
    return s_PERF_MARKER_COLOR;
}

QColor APIColorMap::GetCommandListColor(int index) const
{
    QColor retVal = s_COMMAND_BUFFER_COLOR;
    int indexInVec = index % (m_commandListsColors.size() - 1);
    GT_IF_WITH_ASSERT(indexInVec < m_commandListsColors.size() && index >= 0)
    {
        retVal = m_commandListsColors[indexInVec];
    }
    return retVal;
}

