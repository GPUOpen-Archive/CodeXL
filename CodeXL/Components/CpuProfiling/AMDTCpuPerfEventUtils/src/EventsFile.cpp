//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file EventsFile.cpp
/// \brief Implementation of the EventsFile class.
///
//==================================================================================

#include <EventsFile.h>


#ifdef USE_TINYXML_PARSER

#include <tinyxml.h>

#define DEFAULT_COUNTER_MASK 0x0F

//
// Open XML file and set up handler to parse the XML file
//
bool EventsFile::Open(const std::string& strEventsFile)
{
    TiXmlDocument eventFile(strEventsFile.data());
    bool loadOkay = eventFile.LoadFile();

    if (loadOkay)
    {
        TiXmlElement* rootElement = eventFile.RootElement();
        loadOkay = parseDomTree(rootElement);
    }

    return loadOkay;
}

//
// Print events in the event list to standard output.
// Used primarily for debugging purposes. Clients will typically not need
// to call this function.
//
void EventsFile::PrintEvents()
{
    for (const auto& ev : m_eventList)
    {
        printf("%02X '%s'\n\tAbbrev: '%s'\n\tSource: %s Counters: %X\n\t Description: %s\n",
            ev.m_value, ev.m_name.data(), ev.m_abbrev.data(),
            ev.m_source.data(), ev.m_counters, ev.m_description.data());

        for (const auto& um : ev.m_unitMaskList)
        {
            printf("\t%02x %s\n", um.m_value, um.m_name.data());
        }
    }
}

/*
* There are five main elements in our events xml file.
* 1) <cpu_events>  -> Begins the document
* 2) <source>      -> Contains events pertaining to a specific processor unit
* 3) <event>       -> Describes an event
* 4) <mask>        -> Located within <event> element and contains info for valid masks
* 5) <description> -> within <event> and contains a textual description of the event
*/
bool EventsFile::parseDomTree(TiXmlElement* pRoot)
{
    bool rc = false;
    m_eventList.clear();

    // Root element: <cpu_events>
    if (pRoot != nullptr)
    {
        CpuEvent event;
        event.m_counters = DEFAULT_COUNTER_MASK;

        // <cpu_events> child element: <source>
        TiXmlElement* pSourceChild = pRoot->FirstChildElement("source");

        while (pSourceChild)
        {
            // <source> attribute: unit
            event.m_source = pSourceChild->Attribute("unit");

            // <source> attribute: counters
            const char* strSCounters = pSourceChild->Attribute("counters");
            if (strSCounters)
            {
                event.m_counters = std::stoi(strSCounters, nullptr, 16);
            }

            // <source> child element: <event>
            TiXmlElement* pEventChild = pSourceChild->FirstChildElement("event");

            while (pEventChild)
            {
                // <event> attribute: name
                event.m_name = pEventChild->Attribute("name");

                // <event> attribute: abbreviation
                event.m_abbrev = pEventChild->Attribute("abbreviation");

                // <event> attribute: value
                const char* strValue = pEventChild->Attribute("value");
                if (strValue)
                {
                    event.m_value = std::stoi(strValue, nullptr, 16);
                }
                else
                {
                    event.m_value = 0xBAD;
                }

                // <event> attribute: counters
                const char* strECounters = pEventChild->Attribute("counters");

                if (strECounters)
                {
                    // Override source counter with valid event counter
                    event.m_counters = std::stoi(strECounters, nullptr, 16);
                }

                // <event> attribute: minimumModel
                pEventChild->Attribute("minimumModel", reinterpret_cast<int*>(&event.m_minValidModel));

                // <event> child element: <mask>
                TiXmlElement* pMaskChild = pEventChild->FirstChildElement("mask");
                event.m_unitMaskList.clear();

                while (pMaskChild)
                {
                    UnitMask unitMask;

                    // <mask> attribute: value
                    pMaskChild->Attribute("value", reinterpret_cast<int32_t*>(&unitMask.m_value));

                    // <mask> attribute: name
                    unitMask.m_name = pMaskChild->Attribute("name");

                    // Add new mask to list
                    event.m_unitMaskList.push_back(unitMask);

                    // Visit next sibling of <mask> element
                    pMaskChild = pMaskChild->NextSiblingElement("mask");
                }

                // <event> child element: <description>
                TiXmlElement* pDescChild = pEventChild->FirstChildElement("description");

                // Fetch description text
                if (pDescChild)
                {
                    event.m_description = pDescChild->GetText();
                }
                else
                {
                    event.m_description.clear();
                }

                // Add event to list
                m_eventList.push_back(event);

                // Visit next sibling of <event> element
                pEventChild = pEventChild->NextSiblingElement("event");
            }

            // Visit next sibling of <source> element
            pSourceChild = pSourceChild->NextSiblingElement("source");
        }
    }

    if (m_eventList.size() > 0)
    {
        rc = true;
    }

    return rc;
}

//
// Returns an iterator addressing the first element in the event list.
//
EventList::const_iterator EventsFile::FirstEvent()
{
    return m_eventList.begin();
}

//
// Returns an iterator that addresses the location succeeding the last
// element in the event list.
//
EventList::const_iterator EventsFile::EndOfEvents()
{
    return m_eventList.end();
}

//
// Returns an iterator addressing the first element in the unit mask list.
//
UnitMaskList::const_iterator EventsFile::FirstUnitMask(const CpuEvent& event)
{
    return event.m_unitMaskList.begin();
}

//
// Returns an iterator that addresses the location succeeding the last
// element in the unit mask list.
//
UnitMaskList::const_iterator EventsFile::EndOfUnitMasks(const CpuEvent& event)
{
    return event.m_unitMaskList.end();
}

//
// Search the event list for the CpuEvent structure with the
// symbolic name specified by the argument eventName. Return a
// reference to the structure through the argument event. Return
// true if an entry was found. Return false if an entry was not
// found in the event list.
//
bool EventsFile::FindEventByName(const std::string& eventName, CpuEvent& event)
{
    for (const auto& ev : m_eventList)
    {
        if (ev.m_name == eventName)
        {
            event = ev;
            return true;
        }
    }

    return false;
}

//
// Search the event list for the CpuEvent structure with the
// event select specified by the argument value. Return a
// reference to the structure through the argument event. Return
// true if an entry was found. Return false if an entry was not
// found in the event list.
//
bool EventsFile::FindEventByValue(uint32_t value, CpuEvent& event)
{
    for (const auto& ev : m_eventList)
    {
        if (ev.m_value == value)
        {
            event = ev;
            return true;
        }
    }

    return false;
}

//
// Return true if event select and unit mask are valid WRT set of events.
// An event select is valid if there is an event in the event list with
// that event select value. An unit mask is valid if there is a reference
// unit mask for every bit that is set in the unit mask. The truth table
// for detecting a single invalid unit mask bit is:
//      unitMask | reference | invalid
//      ------------------------------
//          0    |     0     |    0     <<<< Treat as a don't care
//          0    |     1     |    0     <<<< Treat as a don't care
//          1    |     0     |    1     <<<< Invalid
//          1    |     1     |    0     <<<< Valid
// The formula is: invalid = unitMask & (~reference)
//
bool EventsFile::ValidateEvent(uint32_t eventSelect, uint32_t unitMask)
{
    CpuEvent referenceEvent;

    // Find the reference event in the event list
    if (!FindEventByValue(eventSelect, referenceEvent))
    {
        // Return false if event was not found for this event select.
        // This indicates an invalid event select.
        return false;
    }

    // The easy case. If the reference does not have any defined
    // unit masks, then the unit mask under test must be zero.
    if (referenceEvent.m_unitMaskList.size() == 0)
    {
        // Return true if no unit mask bits are set
        return (unitMask == 0);
    }

    // Another easy one. If the reference has at least one defined
    // unit mask, then at last one unit mask bit must be set.
    if ((referenceEvent.m_unitMaskList.size() != 0) && (unitMask == 0))
    {
        return false;
    }

    // Validate that enabled bits are valid by comparing with reference.
    // Compute a single reference mask by accumulating the logical
    // OR of the individual unit masks
    uint32_t reference = 0;

    for (const auto& um : referenceEvent.m_unitMaskList)
    {
        reference |= (1 << um.m_value);
    }

    // Validate the unit mask against the reference mask
    // If all bits are clear, then the unit mask is valid
    bool valid = (unitMask & (~reference)) == 0;

    return valid;
}


#else

#include <QXmlInputSource>

#include <AMDTBaseTools/Include/gtAssert.h>

#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
    #define DEBUG_ASSERT(booleanExpression) GT_ASSERT(booleanExpression)
#else
    #define DEBUG_ASSERT(booleanExpression)
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

EventsFile::EventsFile()
{
    m_CurrentCounters = DEFAULT_COUNTER_MASK ;
}

EventsFile::~EventsFile()
{

}

//
// EventsFile::Open()
// Open XML file and set up handler to parse the XML file
//
bool
//EventsFile::Open(const wchar_t *events_file_path )
EventsFile::Open(QString strEventsFile)
{
    QFile xmlFile(strEventsFile);
    QXmlInputSource source(&xmlFile);
    QXmlSimpleReader reader;

    /* set our xml handler to do work on the data */
    reader.setContentHandler(this);

    return reader.parse(source);
}

//
// EventsFile::Close()
// Deallocate events in the events list
//
void EventsFile::Close()
{
    m_EventList.clear();
}

//
// EventsFile::PrintEvents()
// Print events in the event list to standard output.
// Used primarily for debugging purposes. Clients will typically not need
// to call this function.
//
void EventsFile::PrintEvents()
{
    EventList::iterator elit;

    for (elit = m_EventList.begin(); elit != m_EventList.end(); ++elit)
    {
        printf("%02X '%s'\n\tAbbrev: '%s'\n\tMin model: %d Source: %s Counters: %lX\n\t Description: %s\n",
               elit->value, elit->name.toLatin1().data(),
               elit->abbrev.toLatin1().data(), elit->m_minValidModel,
               elit->source.toLatin1().data(), elit->counters, elit->description.toLatin1().data());

        UnitMaskList::iterator umit;

        for (umit = elit->m_UnitMaskList.begin(); umit != elit->m_UnitMaskList.end(); ++umit)
        {
            printf("\t");
            printf("%02x %s\n", umit->value, umit->name.toLatin1().data());
        }
    }
}


//
// EventsFile::startDocument()
// This method is invoked by the XML handler at the start of the
// XML file. Prepare for parsing.
//
bool EventsFile::startDocument()
{
    m_CurrentSourceUnit = "NONE";
    m_descriptionElement = false;
    return true;
}

//
// EventsFile::startElement()
// This method is invoked by the XML handler when the start of an
// element is encountered in the XML file. Dispatch control based
// on the element type.
//
bool EventsFile::startElement(const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts)
{
    (void)(namespaceURI); // unused
    (void)(localName); // unused
    //printf( "<%s>", qName.data() );

    /* There are four main elements in our xml file.
     * 1) <cpu_events> -> Begins the document
     * 2) <source>  ->  contains events pertaining to a specific processor unit
     * 2) <event>   ->  describes an event
     * 3) <mask>    ->  located within <event> element and contains info
     *                  for valid masks
     * 4) <description> -> within <event> and contains a textual description of the event
     */
    if (qName == "source")
    {
        DEBUG_ASSERT(m_CurrentSourceUnit == "NONE");
        DEBUG_ASSERT(!atts.value("unit").isNull());
        // All events within this element will have the following
        // source unit
        m_CurrentSourceUnit = atts.value("unit");

        // All events within this element will have the following
        // permitted counter mask (unless overridden)
        if (! atts.value("counters").isNull())
        {
            m_CurrentCounters = atts.value("counters").toInt(0, 16) ;
        }
        else
        {
            m_CurrentCounters = DEFAULT_COUNTER_MASK ;
        }
    }
    else if (qName == "event")
    {
        CpuEvent    event;

        // Add this event to our event list
        event.name          = atts.value("name");
        event.abbrev        = atts.value("abbreviation");
        event.source        = m_CurrentSourceUnit;
        // Returns 0 if attribute not given, exactly what we want
        event.m_minValidModel = atts.value("minimumModel").toInt();
        event.value         = atts.value("value").toUInt(0, 16);
        event.m_dataType      = atts.value("datatype").toUInt();
        event.nUnitMasks    = 0;

        // Override the source permitted counters mask when the
        // counters attribute is specified
        if (! atts.value("counters").isNull())
        {
            event.counters = atts.value("counters").toInt(0, 16) ;
        }
        else
        {
            event.counters = m_CurrentCounters ;
        }

        DEBUG_ASSERT(!atts.value("name").isNull());
        DEBUG_ASSERT(!atts.value("value").isNull());

        m_EventList.push_back(event);
    }
    else if (qName == "mask")
    {
        // these upcoming masks will apply to the last event we pushed on
        // the list stack
        CpuEvent&   lastEvent = m_EventList.back();
        UnitMask    unitMask;

        DEBUG_ASSERT(!atts.value("name").isNull());
        DEBUG_ASSERT(!atts.value("value").isNull());

        unitMask.name   = atts.value("name");
        unitMask.value  = atts.value("value").toUInt();

        lastEvent.nUnitMasks++;
        lastEvent.m_UnitMaskList.push_back(unitMask);
    }
    else if (qName == "description")
    {
        m_descriptionElement = true;
    }

    return true;
}

//
// EventsFile::endElement()
// This method is invoked by the XML handler when the end of an
// element is encountered in the XML file. Dispatch control based
// on the element name.
//
bool EventsFile::endElement(const QString& namespaceURI, const QString& localName, const QString& qName)
{
    (void)(namespaceURI); // unused
    (void)(localName); // unused

    if (qName == "source")
    {
        m_CurrentSourceUnit = "NONE";
    }
    else if (qName == "description")
    {
        m_descriptionElement = false;
    }

    //printf( "</%s>", qName.data() );
    return true;
}

//
// CEventsXmlHandler::characters()
// This method is invoked by the XML handler when text data is
// encountered within a non-empty XML element.
//
bool EventsFile::characters(const QString& ch)
{
    //printf( "%s", ch.data() );
    //Should only happen for description elements
    if (m_descriptionElement)
    {
        CpuEvent&   lastEvent = m_EventList.back();
        lastEvent.description = ch;
    }

    return true;
}

//
// Returns an iterator addressing the first element in the event list.
//
EventList::const_iterator EventsFile::FirstEvent()
{
    return m_EventList.begin();
}

//
// Returns an iterator that addresses the location succeeding the last
// element in the event list.
//
EventList::const_iterator EventsFile::EndOfEvents()
{
    return m_EventList.end();
}

//
// Returns an iterator addressing the first element in the unit mask list.
//
UnitMaskList::const_iterator EventsFile::FirstUnitMask(const CpuEvent& event)
{
    return event.m_UnitMaskList.begin();
}

//
// Returns an iterator that addresses the location succeeding the last
// element in the unit mask list.
//
UnitMaskList::const_iterator EventsFile::EndOfUnitMasks(const CpuEvent& event)
{
    return event.m_UnitMaskList.end();
}

//
// Search the event list for the CpuEvent structure with the
// symbolic name specified by the argument eventName. Return a
// reference to the structure through the argument event. Return
// true if an entry was found. Return false if an entry was not
// found in the event list.
//
bool EventsFile::FindEventByName(QString eventName, CpuEvent& event)
{
    for (EventList::iterator elit = m_EventList.begin(), elitEnd = m_EventList.end(); elit != elitEnd; ++elit)
    {
        if (elit->name == eventName)
        {
            event = (*elit);

            return true;
        }
    }

    return false;
}

//
// Search the event list for the CpuEvent structure with the
// event select specified by the argument value. Return a
// reference to the structure through the argument event. Return
// true if an entry was found. Return false if an entry was not
// found in the event list.
//
bool EventsFile::FindEventByValue(unsigned int value, CpuEvent& event)
{
    for (EventList::iterator elit = m_EventList.begin(), elitEnd = m_EventList.end(); elit != elitEnd; ++elit)
    {
        if (elit->value == value)
        {
            event = (*elit);
            return true;
        }
    }

    return false;
}

bool EventsFile::FindEventByValue(unsigned int value, CpuEvent** ppEvent)
{
    if (NULL == ppEvent)
    {
        return false;
    }

    for (EventList::iterator elit = m_EventList.begin(), elitEnd = m_EventList.end(); elit != elitEnd; ++elit)
    {
        if (elit->value == value)
        {
            *ppEvent = &(*elit);
            return true;
        }
    }

    return false;
}

//
// Return true if event select and unit mask are valid WRT set of events.
// An event select is valid if there is an event in the event list with
// that event select value. An unit mask is valid if there is a reference
// unit mask for every bit that is set in the unit mask. The truth table
// for detecting a single invalid unit mask bit is:
//      unitMask | reference | invalid
//      ------------------------------
//          0    |     0     |    0     <<<< Treat as a don't care
//          0    |     1     |    0     <<<< Treat as a don't care
//          1    |     0     |    1     <<<< Invalid
//          1    |     1     |    0     <<<< Valid
// The formula is: invalid = unitMask & (~reference)
//
bool EventsFile::ValidateEvent(unsigned int eventSelect, unsigned int unitMask)
{
    CpuEvent referenceEvent ;

    // Find the reference event in the event list
    if (! FindEventByValue(eventSelect, referenceEvent))
    {
        // Return false if event was not found for this event select.
        // This indicates an invalud event select.
        return (false) ;
    }

    // The easy case. If the reference does not have any defined
    // unit masks, then the unit mask under test must be zero.
    if (referenceEvent.nUnitMasks == 0)
    {
        // Return true if no unit mask bits are set
        return (unitMask == 0) ;
    }

    // Another easy one. If the reference has at least one defined
    // unit mask, then at last one unit mask bit must be set.
    if ((referenceEvent.nUnitMasks != 0) && (unitMask == 0))
    {
        return (false) ;
    }

    // Validate that enabled bits are valid by comparing with reference.
    // Compute a single reference mask by accumulating the logical
    // OR of the individual unit masks
    unsigned int reference = 0 ;
    UnitMaskList::const_iterator umit;

    for (umit = FirstUnitMask(referenceEvent);
         umit != EndOfUnitMasks(referenceEvent); ++umit)
    {
        reference |= (1 << umit->value) ;
    }

    // Validate the unit mask against the reference mask
    // If all bits are clear, then the unit mask is valid
    bool valid = (unitMask & (~reference)) == 0 ;
    return (valid) ;
}

#endif
