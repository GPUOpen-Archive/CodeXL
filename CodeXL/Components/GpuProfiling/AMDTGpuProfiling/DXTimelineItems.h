#ifndef _DX_TIMELINE_ITEMS_H_
#define _DX_TIMELINE_ITEMS_H_

// Local:
#include <AMDTGpuProfiling/APITimelineItems.h>

/// holds the information for DX timeline item
class DXAPITimelineItem : public APITimelineItem
{
public:
    /// Initializes a new instance of the CLAPITimelineItem class
    /// \param startTime the start time for this timeline item.
    /// \param endTime the end time for this timeline item.
    /// \param apiIndex the index of this api in the application's call sequence
    DXAPITimelineItem(quint64 startTime, quint64 endTime, int apiIndex);

    /// Fill in a TimelineItemToolTip instance with a set of name/value pairs that will be displayed in the tooltip for this timeline item
    /// \param tooltip acTimelineItemToolTip instance that should get populated with name/value pairs
    virtual void tooltipItems(acTimelineItemToolTip& tooltip) const;

    void SetInterfaceName(const QString& interfaceName) { m_interfaceName = interfaceName; }

protected:

    /// Holds the interface name for this API call
    QString m_interfaceName;

};
#endif // _DX_TIMELINE_ITEMS_H
