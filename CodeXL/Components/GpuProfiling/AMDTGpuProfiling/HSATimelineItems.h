//=====================================================================
// Copyright (c) 2013 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/HSATimelineItems.h $
/// \version $Revision: #6 $
/// \brief  This file contains the timeline item classes used for HSA APIs
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/HSATimelineItems.h#6 $
// Last checkin:   $DateTime: 2016/01/07 12:10:47 $
// Last edited by: $Author: chesik $
// Change list:    $Change: 554414 $
//=====================================================================

#ifndef _HSA_TIMELINE_ITEMS_H_
#define _HSA_TIMELINE_ITEMS_H_

#include "APITimelineItems.h"

#include "OccupancyInfo.h"

/// HostAPITimelineItem descendant for HSA dispatch API items
class HSADispatchTimelineItem : public HostAPITimelineItem
{
public:
    /// Initializes a new instance of the HSAAPITimelineItem class
    /// \param startTime the start time for this timeline item.
    /// \param endTime the end time for this timeline item.
    /// \param apiIndex the index of this api in the application's call sequence
    HSADispatchTimelineItem(quint64 startTime, quint64 endTime, int apiIndex);

    /// Gets the queue handle string for this API
    /// \return the queue handle string for this API
    QString queueHandle() const { return m_strQueueHandle; }

    /// Sets the newQueueHandle handle string for this API
    /// \param newQueueHandle the queue handle string for this API
    void setQueueHandle(const QString& newQueueHandle) { m_strQueueHandle = newQueueHandle; }

    /// Gets the device type string for this API
    /// \return the device type string for this API
    QString deviceType() const { return m_strDeviceType; }

    /// Sets the device type string for this API
    /// \param newDeviceType the device type string for this API
    void setDeviceType(const QString& newDeviceType) { m_strDeviceType = newDeviceType; }

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

    /// Gets the offset for this API
    /// \return the offset for this API
    QString offset() const { return m_strOffset; }

    /// Sets the offset for this API
    /// \param newOffset the offset for this API
    void setOffset(QString newOffset) { m_strOffset = newOffset; }

    /// Gets the occupancy info for this kernel
    /// \return the occupancy info for this kernel
    IOccupancyInfoDataHandler* occupancyInfo() const { return m_pOccupancyInfo; }

    /// Sets the occupancy info for this kernel
    /// \param pNewOccupancyInfo the occupancy info for this kernel
    void setOccupancyInfo(IOccupancyInfoDataHandler* pNewOccupancyInfo) { m_pOccupancyInfo = pNewOccupancyInfo; }

    /// Fill in a TimelineItemToolTip instance with a set of name/value pairs that will be displayed in the tooltip for this timeline item
    /// \param tooltip acTimelineItemToolTip instance that should get populated with name/value pairs
    virtual void tooltipItems(acTimelineItemToolTip& tooltip) const;

private:
    QString                         m_strQueueHandle;    ///< the queue handle for this API
    QString                         m_strDeviceType;     ///< the device type for this API
    QString                         m_strGlobalWorkSize; ///< the global work size for this API
    QString                         m_strLocalWorkSize;  ///< the local work size for this API
    QString                         m_strOffset;         ///< the offset for this API
    IOccupancyInfoDataHandler*      m_pOccupancyInfo;    ///< the occupancy info for this kernel
};

/// HostAPITimelineItem descendant for HSA memory API items
class HSAMemoryTimelineItem : public HostAPITimelineItem
{
public:
    /// Initializes a new instance of the HSAAPITimelineItem class
    /// \param startTime the start time for this timeline item.
    /// \param endTime the end time for this timeline item.
    /// \param apiIndex the index of this api in the application's call sequence
    /// \param size the size of the memory operation
    HSAMemoryTimelineItem(quint64 startTime, quint64 endTime, int apiIndex, size_t size);

    /// Fill in a TimelineItemToolTip instance with a set of name/value pairs that will be displayed in the tooltip for this timeline item
    /// \param tooltip acTimelineItemToolTip instance that should get populated with name/value pairs
    virtual void tooltipItems(acTimelineItemToolTip& tooltip) const;

protected:
    size_t m_size;                ///< the memory size for this API
};

/// HSAMemoryTimelineItem descendant for HSA memory transfer API items
class HSAMemoryTransferTimelineItem : public HSAMemoryTimelineItem
{
public:
    /// Initializes a new instance of the HSAAPITimelineItem class
    /// \param transferStartTime the start time for this data transfer.
    /// \param transferEndTime the end time for this data transfer.
    /// \param apiIndex the index of this api in the application's call sequence
    /// \param size the size of the memory operation
    /// \param srcAgent the source agent of the memory transfer
    /// \param dstAgent the destination agent of the memory transfer
    HSAMemoryTransferTimelineItem(quint64 transferStartTime, quint64 transferEndTime, int apiIndex, size_t size, QString srcAgent, QString dstAgent);

    /// Fill in a TimelineItemToolTip instance with a set of name/value pairs that will be displayed in the tooltip for this timeline item
    /// \param tooltip acTimelineItemToolTip instance that should get populated with name/value pairs
    virtual void tooltipItems(acTimelineItemToolTip& tooltip) const;

protected:
    QString m_srcAgent;  ///< the source agent of the transfer
    QString m_dstAgent;  ///< the destination agent of the transfer
};

#endif // _HSA_TIMELINE_ITEMS_H_
