//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ViewConfig.cpp
/// \brief View configuration class (including reader/writer)
///
//==================================================================================

#include <ViewConfig.h>
#include <AMDTBaseTools/Include/gtAssert.h>

#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
    #define DEBUG_ASSERT(booleanExpression) GT_ASSERT(booleanExpression)
#else
    #define DEBUG_ASSERT(booleanExpression)
#endif

///////////////////////////////////////////////////////////////////////
// ViewConfig class definition
///////////////////////////////////////////////////////////////////////

// Constants for initializing private data members
#define DefaultConfigName  "No name"
#define DefaultColumnTitle "No title"
#define DefaultEventSelect 0
#define DefaultUnitMask    0

const EventConfig DefaultEventConfig = { DefaultEventSelect, DefaultUnitMask, false, false };

//
// Simple class constructor
//
ViewConfig::ViewConfig()
{
    m_configName = DefaultConfigName ;
    m_toolTip = "" ;
    m_description = "" ;
    m_defaultView = false ;
    m_showPercentage = false ;
    m_numberOfColumns = 0 ;
    m_columnSpecs = NULL ;
    m_assocList.clear() ;
    m_columnList.clear() ;
}

//
// Class copy constructor
//
ViewConfig::ViewConfig(const ViewConfig& original) : QXmlDefaultHandler()
{
    m_configName = original.m_configName ;
    m_toolTip = original.m_toolTip ;
    m_description = original.m_description ;
    m_defaultView = original.m_defaultView ;
    m_showPercentage = original.m_showPercentage ;
    // Copy the array of column specifications
    m_columnSpecs = NULL ;
    m_numberOfColumns = original.m_numberOfColumns ;

    if (NULL != original.m_columnSpecs)
    {
        m_columnSpecs = new ColumnSpec[m_numberOfColumns] ;
        CopyColumnSpecs(original.m_columnSpecs, m_columnSpecs, m_numberOfColumns) ;
    }

    // TODO: The following list assign probably does not deep copy QString elements
    m_assocList.assign(original.m_assocList.begin(), original.m_assocList.end());
    // This copy constructor does *not* copy the XML handling state
    // variables. A copy should not occur when a file is being read.
}

//
// Class destructor
//
ViewConfig::~ViewConfig()
{
    m_assocList.clear() ;
    m_columnList.clear() ;

    if (m_columnSpecs != NULL) { delete [] m_columnSpecs ; }

    m_columnSpecs = NULL ;
}

//
// Class assignment operator
//
const ViewConfig& ViewConfig::operator =(const ViewConfig& rhs)
{
    if (this != &rhs)
    {
        m_configName = rhs.m_configName ;
        m_toolTip = rhs.m_toolTip ;
        m_description = rhs.m_description ;
        m_defaultView = rhs.m_defaultView ;
        m_showPercentage = rhs.m_showPercentage ;

        // Deallocate old column specs array if it exists
        if (m_columnSpecs != NULL)
        {
            delete [] m_columnSpecs ;
            m_columnSpecs = NULL ;
        }

        // Copy the array of column specifications
        m_numberOfColumns = rhs.m_numberOfColumns ;

        if (NULL != rhs.m_columnSpecs)
        {
            m_columnSpecs = new ColumnSpec[m_numberOfColumns] ;
            CopyColumnSpecs(rhs.m_columnSpecs, m_columnSpecs, m_numberOfColumns) ;
        }

        // TODO: The following list assign probably does not deep copy QString elements
        m_assocList.assign(rhs.m_assocList.begin(), rhs.m_assocList.end());
        // This assignment operator does *not* copy the XML handling state
        // variables. An assignment should not occur when a file is being read.
    }

    return (*this) ;
}

//
// Set the view configuration name
//
void ViewConfig::SetConfigName(const QString name)
{
    m_configName = name ;
}

//
// Return view configuration name through reference argument name
//
void ViewConfig::GetConfigName(QString& name)
{
    name = m_configName ;
}

void ViewConfig::SetToolTip(const QString toolTip)
{
    m_toolTip = toolTip ;
}

void ViewConfig::GetToolTip(QString& toolTip)
{
    toolTip = m_toolTip ;
}

void ViewConfig::SetDescription(const QString description)
{
    m_description = description ;
}

void ViewConfig::GetDescription(QString& description)
{
    description = m_description ;
}

///////////////////////////////////////////////////////////////////////
// Functions to copy, set and get the view column specifications
///////////////////////////////////////////////////////////////////////

//
// Copy an array of ColumnSpec -- required to perform deep copy of the
// non-primitive elements like QString. This function is used within
// ViewConfig and could be useful outside the class.
//
void ViewConfig::CopyColumnSpecs(
    ColumnSpec* fromArray, ColumnSpec* toArray, int size)
{
    for (int i = 0 ; i < size ; i++)
    {
        toArray[i] = fromArray[i] ;
    }
}

//
// Create data associations from column specifications.
// Generate ID/event associations for all events; This option
// eases the burden on the GUI until it can request IDs from the user
//
void ViewConfig::GenerateDataAssocs(ColumnSpec* columnSpecArray, int numberOfColumns)
{
    DataAssoc da ;
    QString id ;
    gtUInt64 id_value ;

#ifdef DEBUG
    GT_ASSERT(numberOfColumns > 0);
    GT_ASSERT(columnSpecArray != NULL);
#else
    (void)(columnSpecArray); // unused
    (void)(numberOfColumns); // unused
#endif

    m_assocList.clear() ;

    for (int i = 0 ; i < m_numberOfColumns ; i++)
    {
        switch (m_columnSpecs[i].type)
        {
            case ColumnSum:
            case ColumnDifference:
            case ColumnProduct:
            case ColumnRatio:

                if (! FindAssocByConfig(m_columnSpecs[i].dataSelectRight, da))
                {
                    id = "e" ;
                    id_value = (m_columnSpecs[i].dataSelectRight.eventSelect << 8) |
                               (m_columnSpecs[i].dataSelectRight.eventUnitMask & 0xFF) ;
                    id.append(QString::number(id_value, 16)) ;
                    AddAssoc(id, m_columnSpecs[i].dataSelectRight) ;
                }

            // Fall through is intended! Must handle *both* right and left
            case ColumnValue:

                if (! FindAssocByConfig(m_columnSpecs[i].dataSelectLeft, da))
                {
                    id = "e" ;
                    id_value = (m_columnSpecs[i].dataSelectLeft.eventSelect << 8) |
                               (m_columnSpecs[i].dataSelectLeft.eventUnitMask  & 0xFF) ;
                    id.append(QString::number(id_value, 16)) ;
                    AddAssoc(id, m_columnSpecs[i].dataSelectLeft) ;
                }

                break ;

            default:    // ColumnInvalid
                break ;
        }
    }
}

//
// Define column specifications for the view. The caller provides a
// one-dimensional array of ColumnSpec structs where each ColumnSpec
// defines a column. Order of the ColumnSpecs is important and must
// be preserved; columns will be displayed in order from first to last.
//
void ViewConfig::SetColumnSpecs(ColumnSpec* columnSpecArray, int numberOfColumns, const int generateDataAssocs)
{
    DEBUG_ASSERT(numberOfColumns > 0) ;
    DEBUG_ASSERT(columnSpecArray != NULL) ;

    // Delete existing column spec array to avoid memory leak
    if (m_columnSpecs != NULL) { delete [] m_columnSpecs ; }

    m_columnSpecs = NULL ;

    m_numberOfColumns = numberOfColumns ;

    if (m_numberOfColumns > 0)
    {
        m_columnSpecs = new ColumnSpec[m_numberOfColumns] ;
        DEBUG_ASSERT(m_columnSpecs != NULL) ;
        // Element-by-element copy needed to copy QString
        CopyColumnSpecs(columnSpecArray, m_columnSpecs, m_numberOfColumns) ;
    }
    else
    {
        m_numberOfColumns = 0 ;
        return ;
    }

    if (generateDataAssocs)
    {
        GenerateDataAssocs(columnSpecArray, numberOfColumns) ;
    }
}

//
// Make column specifications from an array of EventConfig values. Use
// the list of title strings to initialize the column titles. Create a
// column spec for each event (show value, no sorting.) Generate data
// associations, if enabled.
//
void ViewConfig::MakeColumnSpecs(int numberOfEvents, EventConfig* pEvents, QStringList titles, const int generateDataAssocs)
{
    DEBUG_ASSERT(pEvents != NULL) ;
    DEBUG_ASSERT(numberOfEvents == titles.size()) ;

    // Delete existing column spec array to avoid memory leak
    if (m_columnSpecs != NULL) { delete [] m_columnSpecs ; }

    m_columnSpecs = NULL ;

    // Create a new column spec array
    m_numberOfColumns = numberOfEvents ;

    if (m_numberOfColumns > 0)
    {
        m_columnSpecs = new ColumnSpec[m_numberOfColumns] ;
        DEBUG_ASSERT(m_columnSpecs != NULL) ;
    }
    else
    {
        m_numberOfColumns = 0 ;
        return ;
    }

    // Initialize each column spec for an event
    QStringList::iterator qslit = titles.begin() ;

    for (int i = 0 ; i < numberOfEvents ; i++)
    {
        m_columnSpecs[i].type = ColumnValue ;
        m_columnSpecs[i].sorting = NoSort ;
        m_columnSpecs[i].visible = true ;
        m_columnSpecs[i].dataSelectLeft = pEvents[i] ;
        m_columnSpecs[i].dataSelectRight.eventSelect = DefaultEventSelect ;
        m_columnSpecs[i].dataSelectRight.eventUnitMask = DefaultUnitMask ;
        m_columnSpecs[i].title = (*qslit) ;
        qslit++ ;
    }

    if (generateDataAssocs)
    {
        GenerateDataAssocs(m_columnSpecs, m_numberOfColumns) ;
    }
}


//
// Return column specifications in the array allocated by the caller.
// The array *must* be large enough to hold all of the view's column
// specifications. GetNumberOfColumns() can be called to determine the
// size of the array to be allocated.
//
void ViewConfig::GetColumnSpecs(ColumnSpec* columnSpecArray)
{
    DEBUG_ASSERT(columnSpecArray != NULL) ;
    DEBUG_ASSERT((m_numberOfColumns > 0) && (m_columnSpecs != NULL)) ;

    if ((m_numberOfColumns > 0) && (m_columnSpecs != NULL))
    {
        CopyColumnSpecs(m_columnSpecs, columnSpecArray, m_numberOfColumns) ;
    }
}


///////////////////////////////////////////////////////////////////////
// Id/event configuration association
///////////////////////////////////////////////////////////////////////

//
// Define new id/event configuration association
//
void ViewConfig::AddAssoc(const QString id, const EventConfig& config)
{
    // This function does not check for duplicates
    DataAssoc da ;
    da.eventId = id ;
    da.eventConfig.eventSelect = config.eventSelect ;
    da.eventConfig.eventUnitMask = config.eventUnitMask ;
    da.eventConfig.bitOs = config.bitOs;
    da.eventConfig.bitUsr = config.bitUsr;
    m_assocList.push_back(da) ;
}

void ViewConfig::AddAssoc(const QString id, unsigned int select, unsigned int unitMask)
{
    // This function does not check for duplicates
    DataAssoc da ;
    da.eventId = id ;
    da.eventConfig.eventSelect = select ;
    da.eventConfig.eventUnitMask = unitMask ;
    m_assocList.push_back(da) ;
}

//
// Find data/event configuration association by id or value
//
bool ViewConfig::FindAssocById(const QString id, DataAssoc& assoc)
{
    DataAssocList::const_iterator dalit ;

    for (dalit = m_assocList.begin() ; dalit != m_assocList.end() ; dalit++)
    {
        if (dalit->eventId == id)
        {
            assoc = (*dalit) ;
            return (true) ;
        }
    }

    return (false) ;
}

bool ViewConfig::FindAssocByConfig(const EventConfig& config,
                                   DataAssoc& assoc)
{
    DataAssocList::const_iterator dalit ;

    for (dalit = m_assocList.begin() ; dalit != m_assocList.end() ; dalit++)
    {
        if ((dalit->eventConfig.eventSelect == config.eventSelect) &&
            (dalit->eventConfig.eventUnitMask  == config.eventUnitMask))
        {
            assoc = (*dalit) ;
            return (true) ;
        }
    }

    return (false) ;
}

bool ViewConfig::FindAssocByValue(unsigned int select, unsigned int unitMask, DataAssoc& assoc)
{
    DataAssocList::const_iterator dalit ;

    for (dalit = m_assocList.begin() ; dalit != m_assocList.end() ; dalit++)
    {
        if ((dalit->eventConfig.eventSelect == select) &&
            (dalit->eventConfig.eventUnitMask  == unitMask))
        {
            assoc = (*dalit) ;
            return (true) ;
        }
    }

    return (false) ;
}

//
// Return the EventConfig value associated with an event ID. If the
// ID is not found, then return NULL.
//
EventConfig ViewConfig::GetConfigById(const QString id)
{
    DataAssoc da ;

    if ((! id.isNull()) && (FindAssocById(id, da)))
    {
        return (da.eventConfig) ;
    }

    return (DefaultEventConfig) ;
}

//
// Find the specified event configuration in an array of event
// configurations. Return true if found.
//
bool ViewConfig::FindEventConfig(const EventConfig& config, EventConfig* pArray, int numberOfEvents, bool testUnitMask)
{
    for (int i = 0 ; i < numberOfEvents ; i++)
    {
        if ((config.eventSelect == pArray[i].eventSelect) && (config.bitOs == pArray[i].bitOs)
            && (config.bitUsr == pArray[i].bitUsr))
        {
            // Event select values match
            if (testUnitMask)
            {
                if (config.eventUnitMask == pArray[i].eventUnitMask)
                {
                    // Found it, return true
                    return (true) ;
                }
            }
            else
            {
                // Found it, return true
                return (true) ;
            }
        }
    }

    // Not in the array, return false
    return (false) ;

}

//
// Determine if this view (configuration) is displayable given a
// set of available events. The available events are supplied in a
// one dimensional array of EventConfig values. The argument
// numberOfEvents specifies the number of elements in the array.
// Walk the list of data associations (id/event config pairs) and
// try to find each event configuration in the array of available
// events. If all required events can be found in the array of
// available events, then the view is displayable and true is returned.
//
bool ViewConfig::isViewDisplayable(EventConfig* pAvailable, int numberOfEvents, bool testUnitMask)
{
    DataAssocList::const_iterator dalit ;

    for (dalit = m_assocList.begin() ; dalit != m_assocList.end() ; dalit++)
    {
        if (! FindEventConfig(dalit->eventConfig, pAvailable,
                              numberOfEvents, testUnitMask))
        {
            return (false) ;
        }
    }

    return (true) ;
}

//
// Write the tool tip element to the XML output stream.
//
void ViewConfig::WriteTooltip(QTextStream& xmlStream, QString& text)
{
    xmlStream << "    <tool_tip>" << text << "</tool_tip>\n" ;
}

//
// Write the description element to the XML output stream.
//
void ViewConfig::WriteDescription(QTextStream& xmlStream, QString& text)
{
    xmlStream << "    <description>" << text << "</description>\n" ;
}

//
// Write a string-valued attribute to the XML output stream.
//
void ViewConfig::WriteStringAttr(QTextStream& xmlStream, char* attr, const QString& value)
{
    xmlStream << ' ' << attr << "=\"" << value << "\"" ;
}

//
// Write a string-valued attribute to the XML output stream.
//
void ViewConfig::WriteStringAttr(QTextStream& xmlStream, const char* attr, const QString& value)
{
    xmlStream << ' ' << attr << "=\"" << value << "\"" ;
}

//
// Write an integer attribute in decimal to the XML output stream.
//
void ViewConfig::WriteDecimalAttr(QTextStream& xmlStream, char* attr, gtUInt64 value)
{
    QString decimal_value ;
    decimal_value.setNum(value, 10) ;
    xmlStream << ' ' << attr << "=\"" ;
    xmlStream << decimal_value << "\"" ;
}

//
// Write an integer attribute in hexdecimal to the XML output stream.
//
void ViewConfig::WriteHexAttr(QTextStream& xmlStream, char* attr, gtUInt64 value)
{
    QString hex_value ;
    hex_value.setNum(value, 16) ;
    xmlStream << ' ' << attr << "=\"" ;
    xmlStream << hex_value << "\"" ;
}

void ViewConfig::WriteHexAttr(QTextStream& xmlStream, const char* attr, gtUInt64 value)
{
    QString hex_value ;
    hex_value.setNum(value, 16) ;
    xmlStream << ' ' << attr << "=\"" ;
    xmlStream << hex_value << "\"" ;
}

//
// Write a float attribute in decimal to the XML output stream.
//
void ViewConfig::WriteFloatAttr(QTextStream& xmlStream, char* attr, float value)
{
    QString decimal_value ;
    decimal_value.setNum(value, 'f', 4) ;
    xmlStream << ' ' << attr << "=\"" ;
    xmlStream << decimal_value << "\"" ;
}

//
// Write an Boolean attribute as "T" or "F" to the XML output stream.
//
void ViewConfig::WriteBoolAttr(QTextStream& xmlStream, char* attr, bool value)
{
    xmlStream << ' ' << attr ;

    if (value)
    {
        xmlStream << "=\"T\"" ;
    }
    else
    {
        xmlStream << "=\"F\"" ;
    }
}

void ViewConfig::WriteBoolAttr(QTextStream& xmlStream, const char* attr, bool value)
{
    xmlStream << ' ' << attr ;

    if (value)
    {
        xmlStream << "=\"T\"" ;
    }
    else
    {
        xmlStream << "=\"F\"" ;
    }
}

//
// Write the <data> element to the XML output stream.
//
void ViewConfig::WriteData(QTextStream& xmlStream)
{
    DataAssocList::iterator dalit ;

    xmlStream << "    <data>\n" ;

    for (dalit = m_assocList.begin() ; dalit != m_assocList.end() ; dalit++)
    {
        xmlStream << "      <event" ;
        WriteStringAttr(xmlStream, "id", dalit->eventId) ;
        WriteHexAttr(xmlStream, "select", dalit->eventConfig.eventSelect) ;
        WriteHexAttr(xmlStream, "mask", dalit->eventConfig.eventUnitMask) ;
        WriteBoolAttr(xmlStream, "Os", dalit->eventConfig.bitOs);
        WriteBoolAttr(xmlStream, "Usr", dalit->eventConfig.bitUsr);
        xmlStream << " />\n" ;
    }

    xmlStream << "    </data>\n" ;
}

//
// Write an event ID to the XML output stream.
//
void ViewConfig::WriteEventId(QTextStream& xmlStream, char* attr, const EventConfig& config)
{
    QString id ;
    DataAssoc da ;

    if (FindAssocByConfig(config, da))
    {
        id = da.eventId ;
    }
    else
    {
        // This case is an internal error. The data section has
        // already been written -- too late to generate an ID!
        xmlStream << "<!-- Missing event ID --> " ;
        id = "ERROR" ;
    }

    WriteStringAttr(xmlStream, attr, id) ;
}

void ViewConfig::WriteEventId(QTextStream& xmlStream, const char* attr, const EventConfig& config)
{
    QString id ;
    DataAssoc da ;

    if (FindAssocByConfig(config, da))
    {
        id = da.eventId ;
    }
    else
    {
        // This case is an internal error. The data section has
        // already been written -- too late to generate an ID!
        xmlStream << "<!-- Missing event ID --> " ;
        id = "ERROR" ;
    }

    WriteStringAttr(xmlStream, attr, id) ;
}

//
// Write an arithmetic relationship to the XML output stream.
//
void ViewConfig::WriteArith(QTextStream& xmlStream, const ColumnSpec& column)
{
    switch (column.type)
    {
        case ColumnSum:
        {
            xmlStream << "        <sum" ;
            break ;
        }

        case ColumnDifference:
        {
            xmlStream << "        <difference" ;
            break ;
        }

        case ColumnProduct:
        {
            xmlStream << "        <product" ;
            break ;
        }

        case ColumnRatio:
        {
            xmlStream << "        <ratio" ;
            break ;
        }

        default:
        {
            // Internal error
            xmlStream << "        <!-- Invalid column type -->\n" ;
            xmlStream << "        <ERROR" ;
            break ;
        }
    }

    WriteEventId(xmlStream, "left", column.dataSelectLeft) ;
    WriteEventId(xmlStream, "right", column.dataSelectRight) ;
    xmlStream << " />\n" ;
}

//
// Write a <column> element to the XML output stream.
//
void ViewConfig::WriteColumn(QTextStream& xmlStream, const ColumnSpec& column)
{
    QString sort ;
    xmlStream << "      <column" ;
    WriteStringAttr(xmlStream, "title", column.title) ;

    // All this to write the sort attribute!
    switch (column.sorting)
    {
        case NoSort:
        {
            sort = "none" ;
            break ;
        }

        case AscendingSort:
        {
            sort = "ascending" ;
            break ;
        }

        case DescendingSort:
        {
            sort = "descending" ;
            break ;
        }

        default:
        {
            // Internal error
            xmlStream << "<!-- Invalid sort --> " ;
            sort = "ERROR" ;
        }
    }

    WriteStringAttr(xmlStream, "sort", sort) ;
    WriteBoolAttr(xmlStream, "visible", column.visible) ;
    xmlStream << ">\n" ;

    if (column.type == ColumnValue)
    {
        // Handle the simple case of a raw value
        xmlStream << "        <value" ;
        WriteEventId(xmlStream, "id", column.dataSelectLeft) ;
        xmlStream << " />\n" ;
    }
    else
    {
        // Handle arithmetic relationships (sum, ratio, etc.)
        WriteArith(xmlStream, column) ;
    }

    xmlStream << "      </column>\n" ;
}

//
// Write the <output> element to the XML output stream.
//
void ViewConfig::WriteOutput(QTextStream& xmlStream)
{
    QString sort, left, right ;
    xmlStream << "    <output>\n" ;

    for (int i = 0 ; i < m_numberOfColumns ; i++)
    {
        // Write <column> element for each column in the view
        WriteColumn(xmlStream, m_columnSpecs[i]) ;
    }

    xmlStream << "    </output>\n" ;
}

//
// Write the <view> element to the XML output stream.
//
void ViewConfig::WriteView(QTextStream& xmlStream)
{
    xmlStream << "  <view" ;
    WriteStringAttr(xmlStream, "name", m_configName) ;
    WriteBoolAttr(xmlStream, "default_view", m_defaultView) ;
    WriteBoolAttr(xmlStream, "show_percentage", m_showPercentage) ;
    xmlStream << ">\n" ;
    WriteData(xmlStream) ;
    WriteOutput(xmlStream) ;
    WriteTooltip(xmlStream, m_toolTip) ;
    WriteDescription(xmlStream, m_description) ;
    xmlStream << "  </view>\n" ;
}

//
// Write the view configuration element to the XML output stream.
//
bool ViewConfig::WriteConfigFile(QString configFileName, QString InstallDir)
{
    QFile* pFile = NULL ;
    QTextStream xmlStream ;

    pFile = new QFile(configFileName) ;

    if (! pFile->open(QIODevice::WriteOnly))
    {
        // Fill open error
        return (false) ;
    }

    xmlStream.setDevice(pFile) ;
    // Set XML file character encoding
    xmlStream.setCodec("UTF-8");

    xmlStream << "<?xml version=\"1.0\"?>\n" ;
    xmlStream << "<!DOCTYPE view_configuration SYSTEM \"" << InstallDir;
    xmlStream << "ViewConfigs/viewconfig.dtd\">\n";
    xmlStream << "<view_configuration>\n" ;
    WriteView(xmlStream) ;
    xmlStream << "</view_configuration>\n" ;

    // Close file and deallocate QFile explicitly
    xmlStream.setDevice(0) ;
    pFile->close() ;
    delete pFile ;
    return (true) ;
}

///////////////////////////////////////////////////////////////////////
// Read view confguration from XML file
///////////////////////////////////////////////////////////////////////

//
// Set up the XML file for reading, then start the parse. The XML parse
// proceeds and calls the callback functions below. Return false if the
// parser detected an error.
//
bool ViewConfig::ReadConfigFile(QString configFileName)
{
    // Create a file, an XML input source and a simple reader
    QFile xmlFile(configFileName);
    QXmlInputSource source(&xmlFile) ;
    QXmlSimpleReader reader ;
    // Connect this object's handler interface to the XML reader
    reader.setContentHandler(this) ;
    // Return true if the parse succeeds and no XML semantic errors
    return (reader.parse(source) && (!m_xmlSemanticError)) ;
}

///////////////////////////////////////////////////////////////////////
// Callback functions for reading a view configuration in XML
///////////////////////////////////////////////////////////////////////

//
// startDocument() is called at the beginning of an XML document.
// Prepare to read a data collection configuration by clearing the
// current configuration.
//
bool ViewConfig::startDocument()
{
    m_configName = DefaultConfigName ;
    m_toolTip = "" ;
    m_description = "" ;
    m_defaultView = false ;
    m_showPercentage = false ;

    // Discard any column specifications
    if (m_columnSpecs != NULL) { delete [] m_columnSpecs ; }

    m_columnSpecs = NULL ;
    m_numberOfColumns = 0 ;
    // Discard any event ID/config associations
    m_assocList.clear() ;

    // Initialize all XML-related working variables
    m_viewIsOpen = false ;
    m_dataIsOpen = false ;
    m_outputIsOpen = false ;
    m_columnIsOpen = false ;
    m_toolTipIsOpen = false ;
    m_descriptionIsOpen = false ;
    m_xmlSemanticError = false ;
    m_columnList.clear() ;

    return (true) ;
}

//
// endDocument() is called at the beginning of an XML document.
//
bool ViewConfig::endDocument()
{
    return (true) ;
}

//
// startElement() is called at the beginning of an XML element.
// Dispatch control using the element name. Handle an element-specific
// attributes. Set-up state variables to handle the contents of the
// element as the XML parse proceeds.
//
bool ViewConfig::startElement(const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts)
{
    (void)(namespaceURI); // unused
    (void)(localName); // unused
    QString boolValue ;
    QString elementName = qName.toLower() ;

    if (elementName == "view")
    {
        /////////////////////////////////////////////////////////
        // <view> element
        /////////////////////////////////////////////////////////
        DEBUG_ASSERT(!atts.value("name").isNull()) ;

        // Use the default name when configuration name is missing
        if (! atts.value("name").isNull())
        {
            m_configName = atts.value("name") ;
        }

        if (! atts.value("default_view").isNull())
        {
            boolValue = atts.value("default_view").toUpper() ;
            m_defaultView = (boolValue == "T") ;
        }

        if (! atts.value("show_percentage").isNull())
        {
            boolValue = atts.value("show_percentage").toUpper() ;
            m_showPercentage = (boolValue == "T") ;
        }

        m_viewIsOpen = true ;
    }
    else if (elementName == "data")
    {
        /////////////////////////////////////////////////////////
        // <data> element
        /////////////////////////////////////////////////////////
        m_assocList.clear() ;
        m_dataIsOpen = true ;
    }
    else if (elementName == "event")
    {
        /////////////////////////////////////////////////////////
        // <event> element
        /////////////////////////////////////////////////////////
        DataAssoc da ;
        unsigned int select, unitMask ;
        bool bitOs(true), bitUsr(true);

        DEBUG_ASSERT(!atts.value("id").isNull()) ;
        DEBUG_ASSERT(!atts.value("select").isNull()) ;
        DEBUG_ASSERT(!atts.value("mask").isNull()) ;

        select = atts.value("select").toInt(0, 16) ;
        unitMask = atts.value("mask").toInt(0, 16) ;

        if (!atts.value("Os").isNull())
        {
            bitOs = (atts.value("Os") == "T");
        }

        if (!atts.value("Usr").isNull())
        {
            bitUsr = (atts.value("Usr") == "T");
        }

        da.eventId = atts.value("id") ;
        da.eventConfig.eventSelect = select ;
        da.eventConfig.eventUnitMask = unitMask ;
        da.eventConfig.bitOs = bitOs ;
        da.eventConfig.bitUsr = bitUsr ;
        m_assocList.push_back(da) ;
    }
    else if (elementName == "output")
    {
        /////////////////////////////////////////////////////////
        // <output> element
        /////////////////////////////////////////////////////////
        // Discard an pre-existing column specifications
        if (m_columnSpecs != NULL) { delete [] m_columnSpecs ; }

        m_columnSpecs = NULL ;
        m_numberOfColumns = 0 ;
        m_columnList.clear() ;
        m_outputIsOpen = true ;
    }
    else if (elementName == "column")
    {
        /////////////////////////////////////////////////////////
        // <column> element
        /////////////////////////////////////////////////////////
        ColumnSpec cs ;
        cs.type = ColumnInvalid ;
        cs.sorting = NoSort ;
        cs.visible = true ;
        cs.dataSelectLeft.eventSelect = DefaultEventSelect ;
        cs.dataSelectLeft.eventUnitMask = DefaultUnitMask ;
        cs.dataSelectLeft.bitOs = true;
        cs.dataSelectLeft.bitUsr = true;
        cs.dataSelectRight.eventSelect = DefaultEventSelect ;
        cs.dataSelectRight.eventUnitMask = DefaultUnitMask ;
        cs.dataSelectRight.bitOs = true;
        cs.dataSelectRight.bitUsr = true;

        if (! atts.value("title").isNull())
        {
            cs.title = atts.value("title") ;
        }
        else
        {
            cs.title = DefaultColumnTitle ;
        }

        if (atts.value("sort").toLower() == "ascending")
        {
            cs.sorting = AscendingSort ;
        }
        else if (atts.value("sort").toLower() == "descending")
        {
            cs.sorting = DescendingSort ;
        }

        if (! atts.value("visible").isNull())
        {
            boolValue = atts.value("visible").toUpper() ;
            cs.visible = (boolValue == "T") ;
        }

        m_columnList.push_back(cs) ;
        m_numberOfColumns++ ;
        m_columnIsOpen = true ;
    }
    else if (elementName == "value")
    {
        /////////////////////////////////////////////////////////
        // <value> element
        /////////////////////////////////////////////////////////
        if (m_columnIsOpen && (! m_columnList.empty()))
        {
            ColumnSpec& cs = m_columnList.back() ;
            EventConfig ec = GetConfigById(atts.value("id")) ;
            cs.type = ColumnValue ;
            cs.dataSelectLeft.eventSelect = ec.eventSelect ;
            cs.dataSelectLeft.eventUnitMask = ec.eventUnitMask ;
            cs.dataSelectLeft.bitOs  = ec.bitOs ;
            cs.dataSelectLeft.bitUsr = ec.bitUsr;
        }
    }
    else if (elementName == "ratio")
    {
        /////////////////////////////////////////////////////////
        // <ratio> element
        /////////////////////////////////////////////////////////
        if (m_columnIsOpen && (! m_columnList.empty()))
        {
            ColumnSpec& cs = m_columnList.back() ;
            EventConfig ec_left = GetConfigById(atts.value("left")) ;
            EventConfig ec_right = GetConfigById(atts.value("right")) ;
            cs.type = ColumnRatio ;
            cs.dataSelectLeft.eventSelect = ec_left.eventSelect ;
            cs.dataSelectLeft.eventUnitMask = ec_left.eventUnitMask ;
            cs.dataSelectLeft.bitOs  = ec_left.bitOs ;
            cs.dataSelectLeft.bitUsr = ec_left.bitUsr;
            cs.dataSelectRight.eventSelect = ec_right.eventSelect ;
            cs.dataSelectRight.eventUnitMask = ec_right.eventUnitMask ;
            cs.dataSelectRight.bitOs  = ec_right.bitOs ;
            cs.dataSelectRight.bitUsr = ec_right.bitUsr;
        }
    }
    else if (elementName == "sum")
    {
        /////////////////////////////////////////////////////////
        // <sum> element
        /////////////////////////////////////////////////////////
        if (m_columnIsOpen && (! m_columnList.empty()))
        {
            ColumnSpec& cs = m_columnList.back() ;
            EventConfig ec_left = GetConfigById(atts.value("left")) ;
            EventConfig ec_right = GetConfigById(atts.value("right")) ;
            cs.type = ColumnSum ;
            cs.dataSelectLeft.eventSelect = ec_left.eventSelect ;
            cs.dataSelectLeft.eventUnitMask = ec_left.eventUnitMask ;
            cs.dataSelectLeft.bitOs  = ec_left.bitOs ;
            cs.dataSelectLeft.bitUsr = ec_left.bitUsr;
            cs.dataSelectRight.eventSelect = ec_right.eventSelect ;
            cs.dataSelectRight.eventUnitMask = ec_right.eventUnitMask ;
            cs.dataSelectRight.bitOs  = ec_right.bitOs ;
            cs.dataSelectRight.bitUsr = ec_right.bitUsr;
        }
    }
    else if (elementName == "difference")
    {
        /////////////////////////////////////////////////////////
        // <difference> element
        /////////////////////////////////////////////////////////
        if (m_columnIsOpen && (! m_columnList.empty()))
        {
            ColumnSpec& cs = m_columnList.back() ;
            EventConfig ec_left = GetConfigById(atts.value("left")) ;
            EventConfig ec_right = GetConfigById(atts.value("right")) ;
            cs.type = ColumnDifference ;
            cs.dataSelectLeft.eventSelect = ec_left.eventSelect ;
            cs.dataSelectLeft.eventUnitMask = ec_left.eventUnitMask ;
            cs.dataSelectLeft.bitOs  = ec_left.bitOs ;
            cs.dataSelectLeft.bitUsr = ec_left.bitUsr;
            cs.dataSelectRight.eventSelect = ec_right.eventSelect ;
            cs.dataSelectRight.eventUnitMask = ec_right.eventUnitMask ;
            cs.dataSelectRight.bitOs  = ec_right.bitOs ;
            cs.dataSelectRight.bitUsr = ec_right.bitUsr;
        }
    }
    else if (elementName == "product")
    {
        /////////////////////////////////////////////////////////
        // <product> element
        /////////////////////////////////////////////////////////
        if (m_columnIsOpen && (! m_columnList.empty()))
        {
            ColumnSpec& cs = m_columnList.back() ;
            EventConfig ec_left = GetConfigById(atts.value("left")) ;
            EventConfig ec_right = GetConfigById(atts.value("right")) ;
            cs.type = ColumnProduct ;
            cs.dataSelectLeft.eventSelect = ec_left.eventSelect ;
            cs.dataSelectLeft.eventUnitMask = ec_left.eventUnitMask ;
            cs.dataSelectLeft.bitOs  = ec_left.bitOs ;
            cs.dataSelectLeft.bitUsr = ec_left.bitUsr;
            cs.dataSelectRight.eventSelect = ec_right.eventSelect ;
            cs.dataSelectRight.eventUnitMask = ec_right.eventUnitMask ;
            cs.dataSelectRight.bitOs  = ec_right.bitOs ;
            cs.dataSelectRight.bitUsr = ec_right.bitUsr;
        }
    }
    else if (elementName == "tool_tip")
    {
        /////////////////////////////////////////////////////////
        // <tool_tip> Set the current tooltip string to empty
        /////////////////////////////////////////////////////////
        m_toolTip = "" ;
        m_toolTipIsOpen = true ;
    }
    else if (elementName == "description")
    {
        /////////////////////////////////////////////////////////
        // <description> Set the current description string to empty
        /////////////////////////////////////////////////////////
        m_description = "" ;
        m_descriptionIsOpen = true ;
    }
    else if (elementName == "view_configuration")
    {
        /////////////////////////////////////////////////////////
        // <view_configuration>
        /////////////////////////////////////////////////////////
    }
    else
    {
        /////////////////////////////////////////////////////////
        // Unrecognized element name
        /////////////////////////////////////////////////////////
        return (false) ;
    }

    return (true) ;
}

//
// endElement() is called at the end of an XML element. Dispatch
// control using the element name to perform element-specific
// finalization of the element's contents.
//
bool ViewConfig::endElement(const QString& namespaceURI, const QString& localName, const QString& qName)
{
    (void)(namespaceURI); // unused
    (void)(localName); // unused
    QString elementName = qName.toLower() ;

    if (elementName == "data")
    {
        /////////////////////////////////////////////////////////
        // </data>
        /////////////////////////////////////////////////////////
        m_dataIsOpen = false ;
    }
    else if (elementName == "output")
    {
        /////////////////////////////////////////////////////////
        // </output>
        /////////////////////////////////////////////////////////
        ColumnSpecList::const_iterator cs ;
        ColumnSpec* pCS ;

        DEBUG_ASSERT(m_numberOfColumns >= 1) ;
        DEBUG_ASSERT((size_t)m_numberOfColumns == (size_t)m_columnList.size()) ;

        // Delete existing column spec array to avoid memory leak
        if (m_columnSpecs != NULL)
        {
            delete [] m_columnSpecs ;
            m_columnSpecs = NULL ;
        }

        // Allocate array to hold column specifications
        m_columnSpecs = new ColumnSpec[m_numberOfColumns] ;
        // Copy event groups from list to the new array
        pCS = m_columnSpecs ;

        for (cs = m_columnList.begin() ; cs != m_columnList.end() ; cs++)
        {
            pCS->type = cs->type ;
            pCS->sorting = cs->sorting ;
            pCS->visible = cs->visible ;
            pCS->title = cs->title ;
            pCS->dataSelectLeft = cs->dataSelectLeft ;
            pCS->dataSelectRight = cs->dataSelectRight ;
            pCS++ ;
        }

        m_columnList.clear() ;
        m_outputIsOpen = false ;
    }
    else if (elementName == "column")
    {
        /////////////////////////////////////////////////////////
        // </column>
        /////////////////////////////////////////////////////////
        m_columnIsOpen = false ;
    }
    else if (elementName == "tool_tip")
    {
        /////////////////////////////////////////////////////////
        // </tool_tip>
        /////////////////////////////////////////////////////////
        m_toolTipIsOpen = false ;
    }
    else if (elementName == "description")
    {
        /////////////////////////////////////////////////////////
        // </description>
        /////////////////////////////////////////////////////////
        m_descriptionIsOpen = false ;
    }

    return (true) ;
}

//
// characters() is called to return text that is the content of
// a non-empty XML element. Tooltip and description elements are
// the only DC configuration elements that are non-empty.
//
bool ViewConfig::characters(const QString& ch)
{
    if (m_descriptionIsOpen)
    {
        // Add characters to the end of the description text
        m_description.append(ch.simplified()) ;
    }
    else if (m_toolTipIsOpen)
    {
        // Add characters to the end of the tooltip text
        m_toolTip.append(ch.simplified()) ;
    }

    return (true) ;
}
