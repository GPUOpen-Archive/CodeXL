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
#pragma once

#include <string>
#include <cstdint>
#include <list>

#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include "CpuPerfEventUtilsDLLBuild.h"

#if defined (_WIN32)
// Error description: 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
// However, this warning is caused by private members that do not have DLL exports definitions.
// Here suppress the warning caused by private member m_eventList.
#pragma warning( disable : 4251 )
#endif

struct UnitMask
{
    std::string     m_name;               // Human readable name
    uint32_t        m_value = 0;          // Unit mask value
};

typedef std::list<UnitMask> UnitMaskList;

struct CpuEvent
{
    std::string     m_name;               // Human readable name for event
    std::string     m_abbrev;             // Abbreviated name
    std::string     m_source;             // Source unit for event
    std::string     m_description;        // Description of the event from the BKDG
    uint32_t        m_value = 0;          // Event value
    uint32_t        m_counters = 0;       // Permitted counters for this event
    uint32_t        m_minValidModel = 0;  //The minimum model number for which the event is valid
    UnitMaskList   m_unitMaskList;       // List of valid unit masks for the event
};

typedef std::list<CpuEvent> EventList;

class TiXmlElement;

class CP_EVENT_API EventsFile
{
public:
    EventsFile() = default;
    virtual ~EventsFile() { m_eventList.clear(); };

    // Open and read event information from XML file
    bool Open(const std::string& strEventFile);

    // Print events from event list to standard output (debugging aid)
    void PrintEvents();

    // Find event using symbolic name; Return true if found, else false
    bool FindEventByName(const std::string& eventName, CpuEvent& event);

    // Find event using event select value; Return true if found, else false
    bool FindEventByValue(uint32_t value, CpuEvent& event);

    // Return true if event/unit mask are valid WRT set of events
    bool ValidateEvent(uint32_t value, uint32_t unitMask);

    // Return iterator pointing to first event in the event list
    EventList::const_iterator FirstEvent();

    // Return iterator pointing to end of the event list
    EventList::const_iterator EndOfEvents();

    // Return iterator pointing to first entry in unit mask list
    UnitMaskList::const_iterator FirstUnitMask(const CpuEvent& event);

    // Return iterator pointing to end of the unit mask list
    UnitMaskList::const_iterator EndOfUnitMasks(const CpuEvent& event);

private:
    // Process the DOM tree and update m_eventList
    bool parseDomTree(TiXmlElement* pRoot);

    // Cached list of performance counter events
    EventList m_eventList;
};
