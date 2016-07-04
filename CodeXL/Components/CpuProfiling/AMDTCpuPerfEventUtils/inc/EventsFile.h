//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file EventsFile.h
/// \brief Definitions for the class that will interface with the XML file
///        containing the event definitions for a processor.
/// \note We will try to maintain Windows/Linux compatibility with this class
///       for use with Linux application that use performance counter events.
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuPerfEventUtils/inc/EventsFile.h#2 $
// Last checkin:   $DateTime: 2016/04/14 02:12:44 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569058 $
//=====================================================================

#ifndef _EVENTSFILE_H_
#define _EVENTSFILE_H_

#if defined (_WIN32)
#pragma warning(push)
#pragma warning(disable: 4244 4275 4800)
#endif

#include <QXmlDefaultHandler>
#if defined (_WIN32)
#pragma warning( pop )
#endif

#include <AMDTBaseTools/Include/gtList.h>
#include "CpuPerfEventUtilsDLLBuild.h"

//
// Unit mask values are stored as the index of the bit to enable
// in the performance counter control register. Here is an example
// for Athlon64/Opteron event 0x20 Segment Register Loads:
//    Hardware  value  name
//    --------  -----  ----
//      0x01      0     ES
//      0x02      1     CS
//      0x04      2     SS
//      0x08      3     DS
//      0x10      4     FS
//      0x20      5     GS
//
struct UnitMask
{
    QString         name;           // Human readable name
    unsigned int    value;          // Unit mask value
};

typedef gtList<UnitMask>      UnitMaskList;

//
// Performance counter events are stored as CpuEvent structs.
//
struct CpuEvent
{
    QString         name;           // Human readable name for event
    QString         abbrev;         // Abbreviated name
    QString         source;         // Source unit for event
    QString         description;    // Description of the event from the BKDG
    unsigned int    m_dataType;     // Event value
    unsigned int    value;          // Event value
    unsigned int    nUnitMasks;     // Number of valid unit masks for the event
    UnitMaskList    m_UnitMaskList; // List of valid unit masks for the event
    int m_minValidModel; //The minimum model number for which the event is valid
    unsigned long int counters;     // Permitted counters for this event
};

typedef gtList<CpuEvent>      EventList;

//
// In some AMD processor families (e.g., Family 15h), a set of
// rules govern the assignment of events to performance counters.
// The assignment generally depends upon the event source and the
// number of events which a counter can count in a single clock cycle.
// The XML <source> and <event> tags have an attribute, "counters",
// which specifies a bit mask for permitted event-to-counter assignment.
// Each bit corresponds to a permitted performance counter
// (bit 0 -> counter 0, bit 1 -> counter 1, etc.)
//
// Permissible counters for an event are determined in the following way:
//   * If an <event> has the counters attribute, use the mask specified
//     by the attribute. Otherwise, use the current parent <source> mask.
//   * If the surrounding <source> has the counters attribute, use the
//     mask specified by the attribute for all <events> within the
//     surrounding <source> ... </source>.
//   * Otherwise, use the default mask, which is the "classic" mask
//     allowing all four performance counters.
// DEFAULT_COUNTER_MASK specifies the default mask value.
//
#define DEFAULT_COUNTER_MASK 0x0F

//
// EventsFile: Events File Class
// Maintains a list of available performance events and associated unit
// masks. Provides method functions to: read events from an XML file,
// search the list of events, and validate an event.
//
// This class inherits from the Qt class QXmlDefaultHandler, which is the
// default handler for reading XML files. It supports a SAX-like interface.
// EventsFile overrides certain method functions in QXmlDefaultHandler
// which are callbacks from the Qt XML parser. The callbacks are used
// to build up the list of performance counter events.
//
class CP_EVENT_API EventsFile  : public QXmlDefaultHandler
{
public:
    EventsFile();
    virtual ~EventsFile();

    ///////////////////////////////////////////////////////////////////////
    // Client interface
    ///////////////////////////////////////////////////////////////////////
    // Open and read event information from XML file
    bool    Open(const QString strEventFile);
    // Close and release storage
    void    Close();
    // Print events from event list to standard output (debugging aid)
    void    PrintEvents();
    // Find event using symbolic name; Return true if found, else false
    bool    FindEventByName(QString eventName, CpuEvent& event);
    // Find event using event select value; Return true if found, else false
    bool    FindEventByValue(unsigned int value, CpuEvent& event);

    bool    FindEventByValue(unsigned int value, CpuEvent** ppEvent);

    // Return true if event/unit mask are valid WRT set of events
    bool ValidateEvent(unsigned int value, unsigned int unitMask);

    // Return iterator pointing to first event in the event list
    EventList::const_iterator FirstEvent();
    // Return iterator pointing to end of the event list
    EventList::const_iterator EndOfEvents();
    // Return iterator pointing to first entry in unit mask list
    UnitMaskList::const_iterator    FirstUnitMask(const CpuEvent& event);
    // Return iterator pointing to end of the unit mask list
    UnitMaskList::const_iterator    EndOfUnitMasks(const CpuEvent& event);

    ///////////////////////////////////////////////////////////////////////
    // Method overrides for QXmlDefaultHandler
    ///////////////////////////////////////////////////////////////////////
    bool startDocument();
    bool startElement(const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts);
    bool endElement(const QString& namespaceURI, const QString& localName, const QString& qName);
    bool characters(const QString& ch);

private:
    EventList   m_EventList;          // List of perf counter events
    QString     m_CurrentSourceUnit;  // Current source unit (XML handling)
    unsigned int    m_CurrentCounters;    // Current permitted counters for a source (XML handling)
    bool m_descriptionElement;

};

#endif // _EVENTSFILE_H_
