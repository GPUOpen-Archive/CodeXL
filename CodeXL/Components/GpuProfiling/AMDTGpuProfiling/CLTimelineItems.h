//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CLTimelineItems.h $
/// \version $Revision: #12 $
/// \brief  This file contains the timeline item classes used for CL APIs
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CLTimelineItems.h#12 $
// Last checkin:   $DateTime: 2015/08/31 01:56:31 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 538740 $
//=====================================================================

#ifndef _CL_TIMELINE_ITEMS_H_
#define _CL_TIMELINE_ITEMS_H_

#include "APITimelineItems.h"

#include "OccupancyInfo.h"
#include <IOccupancyInfoDataHandler.h>


/// HostAPITimelineItem descendant for OpenCL API items
class CLAPITimelineItem : public HostAPITimelineItem
{
public:
    /// Initializes a new instance of the CLAPITimelineItem class
    /// \param startTime the start time for this timeline item.
    /// \param endTime the end time for this timeline item.
    /// \param apiIndex the index of this api in the application's call sequence
    CLAPITimelineItem(quint64 startTime, quint64 endTime, int apiIndex);

    /// Gets the device type string for this API
    /// \return the device type string for this API
    QString deviceType() const { return m_strDeviceType; }

    /// Sets the device type string for this API
    /// \param newDeviceType the device type string for this API
    void setDeviceType(const QString& newDeviceType) { m_strDeviceType = newDeviceType; }

    /// Gets the command type string for this API
    /// \return the command type string for this API
    QString commandType() const { return m_strCommandType; }

    /// Sets the command type string for this API
    /// \param newCommandType the command type string for this API
    void setCommandType(const QString& newCommandType) { m_strCommandType = newCommandType; }

    /// Gets the submit time for this API
    /// \return the submit time for this API
    quint64 submitTime() const { return m_nSubmitTime; }

    /// Sets the submit time for this API
    /// \param newSubmitTime the submit time for this API
    void setSubmitTime(const quint64 newSubmitTime) { m_nSubmitTime = newSubmitTime; }

    /// Gets the queue time for this API
    /// \return the queue time for this API
    quint64 queueTime() const { return m_nQueueTime; }

    /// Sets the queue time for this API
    /// \param newQueueTime the queue time for this API
    void setQueueTime(const quint64 newQueueTime) { m_nQueueTime = newQueueTime; }

    /// Fill in a TimelineItemToolTip instance with a set of name/value pairs that will be displayed in the tooltip for this timeline item
    /// \param tooltip qcTimelineItemToolTip instance that should get populated with name/value pairs
    virtual void tooltipItems(acTimelineItemToolTip& tooltip) const;

    /// Gets a string representing the memory size (using the appropriate scale)
    /// \param dataSizeInBytes the number of bytes being transferred
    /// \param precision the precision for the resulting string
    /// \return a string representing the data size (using the appropriate scale)
    static QString getDataSizeString(quint64 dataSizeInBytes, int precision);

#ifdef SHOW_KERNEL_LAUNCH_AND_COMPLETION_LATENCY
    /// Sets the completion latency timestamp
    /// \param newCompletionLatencyTime the completion latency timestamp
    void setCompletionLatencyTime(quint64 newCompletionLatencyTime) { m_nCompletionLatencyTime = newCompletionLatencyTime; }
#endif

private:
    QString            m_strDeviceType;             ///< the device type for this API
    QString            m_strCommandType;            ///< the command type for this API

    quint64            m_nSubmitTime;               ///< the submit time for this API
    quint64            m_nQueueTime;                ///< the queue time for this API

#ifdef SHOW_KERNEL_LAUNCH_AND_COMPLETION_LATENCY
    quint64            m_nCompletionLatencyTime;    ///< the completion latency timestamp
#endif
};

/// CLAPITimelineItem descendant for kernel API items
class CLKernelTimelineItem : public CLAPITimelineItem
{
public:
    /// Initializes a new instance of the CLKernelTimelineItem class
    /// \param startTime the start time for this timeline item.
    /// \param endTime the end time for this timeline item.
    /// \param apiIndex the index of this api in the application's call sequence
    CLKernelTimelineItem(quint64 startTime, quint64 endTime, int apiIndex);

    /// Gets the global work size for this API
    /// \return the global work size for this API
    QString globalWorkSize() const { return m_strGlobalWorkSize; }

    /// Sets the global work size for this API
    /// \param newGlobalWorkSize the global work size for this API
    void setGlobalWorkSize(QString newGlobalWorkSize) { m_strGlobalWorkSize = newGlobalWorkSize; }

    /// Gets the local work size for this API
    /// \return the local work size for this API
    QString localWorkSize() const { return m_strLocalWorkSize; }

    /// Sets the local work size for this API
    /// \param newLocalWorkSize the local work size for this API
    void setLocalWorkSize(QString newLocalWorkSize) { m_strLocalWorkSize = newLocalWorkSize; }

    /// Gets the occupancy info for this kernel
    /// \return the occupancy info for this kernel
    IOccupancyInfoDataHandler* occupancyInfo() const { return m_pOccupancyInfo; }

    /// Sets the occupancy info for this kernel
    /// \param newOccupancyInfo the occupancy info for this kernel
    void setOccupancyInfo(IOccupancyInfoDataHandler* newOccupancyInfo) { m_pOccupancyInfo = newOccupancyInfo; }

    /// Fill in a TimelineItemToolTip instance with a set of name/value pairs that will be displayed in the tooltip for this timeline item
    /// \param tooltip qcTimelineItemToolTip instance that should get populated with name/value pairs
    virtual void tooltipItems(acTimelineItemToolTip& tooltip) const;

private:
    QString                     m_strGlobalWorkSize; ///< the global work size for this API
    QString                     m_strLocalWorkSize;  ///< the local work size for this API
    IOccupancyInfoDataHandler*  m_pOccupancyInfo;    ///< the occupancy info for this kernel
};

/// CLAPITimelineItem descendant for data transfer API items
class CLMemTimelineItem : public CLAPITimelineItem
{
public:
    /// Initializes a new instance of the CLMemTimelineItem class
    /// \param startTime the start time for this timeline item.
    /// \param endTime the end time for this timeline item.
    /// \param apiIndex the index of this api in the application's call sequence
    CLMemTimelineItem(quint64 startTime, quint64 endTime, int apiIndex);

    /// Gets the data transfer size for this API
    /// \return the data transfer size for this API
    quint64 dataTransferSize() const { return m_nDataTransferSize; }

    /// Sets the data transfer size for this API
    /// \param newDataTransferSize the data transfer size for this API
    void setDataTransferSize(quint64 newDataTransferSize) { m_nDataTransferSize = newDataTransferSize; }

    /// Fill in a TimelineItemToolTip instance with a set of name/value pairs that will be displayed in the tooltip for this timeline item
    /// \param tooltip qcTimelineItemToolTip instance that should get populated with name/value pairs
    virtual void tooltipItems(acTimelineItemToolTip& tooltip) const;

private:
    quint64 m_nDataTransferSize; ///< the data transfer size for this API
};

/// CLDataEnqueueOperationsTimelineItem descendant for fill operations API items
class CLOtherEnqueueOperationsTimelineItem : public CLAPITimelineItem
{
public:
    /// Initializes a new instance of the CLNonDispatchEnqueueOperationsTimelineItem class
    /// \param startTime the start time for this timeline item.
    /// \param endTime the end time for this timeline item.
    /// \param apiIndex the index of this api in the application's call sequence
    CLOtherEnqueueOperationsTimelineItem(quint64 startTime, quint64 endTime, int apiIndex) : CLAPITimelineItem(startTime, endTime, apiIndex) {};
};

/// CLDataEnqueueOperationsTimelineItem descendant for fill operations API items
class CLDataEnqueueOperationsTimelineItem : public CLOtherEnqueueOperationsTimelineItem
{
public:
    /// Initializes a new instance of the CLNonDispatchEnqueueOperationsTimelineItem class
    /// \param startTime the start time for this timeline item.
    /// \param endTime the end time for this timeline item.
    /// \param apiIndex the index of this api in the application's call sequence
    CLDataEnqueueOperationsTimelineItem(quint64 startTime, quint64 endTime, int apiIndex) : CLOtherEnqueueOperationsTimelineItem(startTime, endTime, apiIndex) {};

    /// Gets the data size for this API
    /// \return the data size for this API
    quint64 dataSize() const { return m_nDataSize; }

    /// Sets the data transfer size for this API
    /// \param newDataTransferSize the data transfer size for this API
    void setDataSize(quint64 newDataSize) { m_nDataSize = newDataSize; }

    /// Fill in a TimelineItemToolTip instance with a set of name/value pairs that will be displayed in the tooltip for this timeline item
    /// \param tooltip qcTimelineItemToolTip instance that should get populated with name/value pairs
    virtual void tooltipItems(acTimelineItemToolTip& tooltip) const;

private:

    quint64 m_nDataSize; ///< the data size of the enqueue operation (relevant for some of the operations)

};

/// APITimelineItem descendant for clGetEventInfo API items
class CLGetEventInfoTimelineItem : public APITimelineItem
{
public:
    /// Initializes a new instance of the CLGetEventInfoAPITimelineItem class
    /// \param startTime the start time of the timeline item
    /// \param endTime the end time of the timeline item
    /// \param apiIndex the index of the api call that this timeline item corresponds to
    CLGetEventInfoTimelineItem(quint64 startTime, quint64 endTime, int apiIndex) : APITimelineItem(startTime, endTime, apiIndex) {}

    /// Fill in a TimelineItemToolTip instance with a set of name/value pairs that will be displayed in the tooltip for this timeline item
    /// \param tooltip qcTimelineItemToolTip instance that should get populated with name/value pairs
    virtual void tooltipItems(acTimelineItemToolTip& tooltip) const;
};


#endif // _CL_TIMELINE_ITEMS_H
