//=====================================================================
// Copyright (c) 2013 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/APITimelineItems.h $
/// \version $Revision: #7 $
/// \brief This file contains the base timeline item classes used for all APIs
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/APITimelineItems.h#7 $
// Last checkin:   $DateTime: 2015/09/01 08:00:53 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 538926 $
//=====================================================================

#ifndef _API_TIMELINE_ITEMS_H_
#define _API_TIMELINE_ITEMS_H_

#include <AMDTApplicationComponents/Include/Timeline/acTimeline.h>


//forward declarations
class TraceTableItem;

/// qcAPITimelineItem descendant for API items that have an associated trace table item -- this allows for faster navigation between timeline items and trace table items.
class APITimelineItem : public acAPITimelineItem
{
public:

    /// Initializes a new instance of the APITimelineItem class
    APITimelineItem();

    /// Initializes a new instance of the APITimelineItem class
    /// \param startTime the start time for this timeline item.
    /// \param endTime the end time for this timeline item.
    /// \param apiIndex the index of this api in the application's call sequence
    APITimelineItem(quint64 startTime, quint64 endTime, int apiIndex);

    /// Gets the trace table item for this API
    /// \return the trace table item for this API
    TraceTableItem* traceTableItem() const { return m_pTraceTableItem; }

    /// Sets the trace table item for this API
    /// \param newTraceTableItem the trace table item for this API
    void setTraceTableItem(TraceTableItem* newTraceTableItem) { m_pTraceTableItem = newTraceTableItem; }

    /// Fill in a TimelineItemToolTip instance with a set of name/value pairs that will be displayed in the tooltip for this timeline item
    /// \param tooltip acTimelineItemToolTip instance that should get populated with name/value pairs
    virtual void tooltipItems(acTimelineItemToolTip& tooltip) const;

protected:
    TraceTableItem* m_pTraceTableItem; ///< the trace table item for this API
};

/// APITimelineItem descendant for dispatch API items
class DispatchAPITimelineItem : public APITimelineItem
{
public:
    /// Initializes a new instance of the DispatchAPITimelineItem class
    /// \param deviceItem the device timeline item
    /// \param item the main timeline item being replaced with this one
    DispatchAPITimelineItem(acAPITimelineItem* deviceItem, acAPITimelineItem* item);

    /// Initializes a new instance of the DispatchAPITimelineItem class
    DispatchAPITimelineItem(const QString& text);

    /// Gets the device timeline item associated with the enqueue item
    /// \return the device timeline item associated with the enqueue item
    acAPITimelineItem* deviceItem() const { return m_deviceItem; }

    /// Sets the device timeline item associated with the enqueue item
    /// \param pDeviceItem the device timeline item associated with the enqueue item
    void setDeviceItem(acAPITimelineItem* pDeviceItem) { m_deviceItem = pDeviceItem; }

private:
    acAPITimelineItem* m_deviceItem; ///< the device timeline item associated with the enqueue item
};

/// APITimelineItem descendant for API items that have an associated host item -- used for device timeline items to easily correlate them back to their associated host timeline item
class HostAPITimelineItem : public APITimelineItem
{
public:
    /// Initializes a new instance of the HostAPITimelineItem class
    /// \param startTime the start time for this timeline item.
    /// \param endTime the end time for this timeline item.
    /// \param apiIndex the index of this api in the application's call sequence
    HostAPITimelineItem(quint64 startTime, quint64 endTime, int apiIndex);

    /// Gets the host item for this API
    /// \return the host item for this API
    acAPITimelineItem* hostItem() const { return m_pHostItem; }

    /// Sets the host item for this API
    /// \param newHostItem the host item for this API
    void setHostItem(acAPITimelineItem* newHostItem) { m_pHostItem = newHostItem; }

private:
    acAPITimelineItem* m_pHostItem; ///< the host item for this API
};


/// APITimelineItem descendant for API items that have an associated host item -- used for device timeline items to easily correlate them back to their associated host timeline item
class PerfMarkerTimelineItem : public acTimelineItem
{
    Q_OBJECT

public:
    /// Initializes a new instance of the HostAPITimelineItem class
    /// \param startTime the start time for this timeline item.
    /// \param endTime the end time for this timeline item.
    PerfMarkerTimelineItem(quint64 startTime, quint64 endTime);

    /// Gets the trace table item for this API
    /// \return the trace table item for this API
    TraceTableItem* traceTableItem() const { return m_pTraceTableItem; }

    /// Sets the trace table item for this API
    /// \param newTraceTableItem the trace table item for this API
    void setTraceTableItem(TraceTableItem* newTraceTableItem) { m_pTraceTableItem = newTraceTableItem; }

private:
    TraceTableItem* m_pTraceTableItem; ///< the trace table item for this API
};


#endif // _API_TIMELINE_ITEMS_H_
