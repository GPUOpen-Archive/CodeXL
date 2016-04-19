//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file DcConfig.cpp
/// \brief Data collection configuration reader/writer.
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuPerfEventUtils/src/DcConfig.cpp#2 $
// Last checkin:   $DateTime: 2016/04/14 02:12:44 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569058 $
//=====================================================================

#include <DcConfig.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osCpuid.h>

#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
    #define DEBUG_ASSERT(booleanExpression) GT_ASSERT(booleanExpression)
#else
    #define DEBUG_ASSERT(booleanExpression)
#endif

////////////////////////////////////////////////////////////////////////////////////
// CdConfig class definition
////////////////////////////////////////////////////////////////////////////////////

// Constants for initializing private data members
#define DefaultConfigName      "No name"
//0.0 for a float 0, not just integer 0
#define DefaultInterval        0.0
#define DefaultMultiplexPeriod 1
#define DefaultProcCore        "Opteron"
#define DefaultMultiplier      10
#define DefaultMaxCount        0

//
// Simple class constructor
//
DcConfig::DcConfig()
{
    m_configType = DCConfigInvalid ;
    m_configName = DefaultConfigName ;
    m_toolTip = "" ;
    m_description = "" ;
    // Time-based profiling configuration attributes
    m_interval = DefaultInterval ;
    // Event-based profiling configuration attributes
    m_numberOfEvents = 0 ;
    m_pEventConfigs = NULL ;
    // IBS configuration attributes
    m_IBSConfig.fetchSampling = false ;
    m_IBSConfig.fetchMaxCount = DefaultMaxCount ;
    m_IBSConfig.opSampling = false ;
    m_IBSConfig.opMaxCount = DefaultMaxCount ;
    m_IBSConfig.opCycleCount = true;
}

//
// Class copy constructor
//
DcConfig::DcConfig(const DcConfig& original) : QXmlDefaultHandler()
{
    m_configType = original.m_configType ;
    m_configName = original.m_configName ;
    m_toolTip = original.m_toolTip ;
    m_description = original.m_description ;
    // Time-based profiling configuration attributes
    m_interval = original.m_interval ;
    // Event-based profiling configuration attributes
    m_numberOfEvents = original.m_numberOfEvents ;
    m_pEventConfigs = NULL ;

    if (NULL != original.m_pEventConfigs)
    {
        m_pEventConfigs = new DcEventConfig[m_numberOfEvents] ;
        memcpy(m_pEventConfigs, original.m_pEventConfigs, m_numberOfEvents * sizeof(DcEventConfig)) ;
    }

    // IBS configuration attributes
    m_IBSConfig.fetchSampling = original.m_IBSConfig.fetchSampling ;
    m_IBSConfig.fetchMaxCount = original.m_IBSConfig.fetchMaxCount ;
    m_IBSConfig.opSampling = original.m_IBSConfig.opSampling ;
    m_IBSConfig.opMaxCount = original.m_IBSConfig.opMaxCount ;
    m_IBSConfig.opCycleCount = original.m_IBSConfig.opCycleCount;
    // This copy constructor does *not* copy the XML handling state
    // variables. A copy should not occur when a file is being read.
}

//
// Class destructor
//
DcConfig::~DcConfig()
{
    if (m_pEventConfigs != NULL) { delete [] m_pEventConfigs ; }

    m_pEventConfigs = NULL ;
    m_numberOfEvents = 0;
}

//
// Class assignment operator
//
const DcConfig& DcConfig::operator =(const DcConfig& rhs)
{
    if (this != &rhs)
    {
        m_configType = rhs.m_configType ;
        m_configName = rhs.m_configName ;
        m_toolTip = rhs.m_toolTip ;
        m_description = rhs.m_description ;
        // Time-based profiling configuration attributes
        m_interval = rhs.m_interval ;
        // Event-based profiling configuration attributes
        m_numberOfEvents = rhs.m_numberOfEvents ;

        if (m_pEventConfigs != NULL)
        {
            delete [] m_pEventConfigs ;
            m_pEventConfigs = NULL ;
        }

        if (NULL != rhs.m_pEventConfigs)
        {
            m_pEventConfigs = new DcEventConfig[m_numberOfEvents] ;
            memcpy(m_pEventConfigs, rhs.m_pEventConfigs, m_numberOfEvents * sizeof(DcEventConfig)) ;
        }

        // IBS configuration attributes
        m_IBSConfig.fetchSampling = rhs.m_IBSConfig.fetchSampling ;
        m_IBSConfig.fetchMaxCount = rhs.m_IBSConfig.fetchMaxCount ;
        m_IBSConfig.opSampling = rhs.m_IBSConfig.opSampling ;
        m_IBSConfig.opMaxCount = rhs.m_IBSConfig.opMaxCount ;
        m_IBSConfig.opCycleCount = rhs.m_IBSConfig.opCycleCount;
        // This assignment operator does *not* copy the XML handling state
        // variables. An assignment should not occur when a file is being read.
    }

    return (*this) ;
}

////////////////////////////////////////////////////////////////////////////////////
// DCConfig mutator functions
////////////////////////////////////////////////////////////////////////////////////

//
// Set the data collection configuration type.
//
void DcConfig::SetConfigType(DcConfigType configType)
{
    m_configType = configType ;
}

//
// Set the data collection configuration name.
//
void DcConfig::SetConfigName(const QString configName)
{
    m_configName = configName ;
}

//
// Set the data collection configuration tool tip.
//
void DcConfig::SetToolTip(const QString toolTip)
{
    m_toolTip = toolTip ;
}

//
// Set the data collection configuration description.
//
void DcConfig::SetDescription(const QString description)
{
    m_description = description ;
}

//
// Set the timer interval. TBP only.
//
void DcConfig::SetTimerInterval(float interval)
{
    m_interval = interval ;
}

//
// Set the event information. EBP only.
//
void DcConfig::SetEventInfo(
    DcEventConfig* pEventCfg, int numberOfEvents)
{
    m_numberOfEvents = numberOfEvents ;

    // Delete existing event group array to avoid memory leak
    if (m_pEventConfigs != NULL) { delete [] m_pEventConfigs ; }

    m_pEventConfigs = NULL;

    if (numberOfEvents > 0)
    {
        m_pEventConfigs = new DcEventConfig[numberOfEvents] ;
        DEBUG_ASSERT((pEventCfg != NULL) && (m_pEventConfigs != NULL)) ;
        memcpy(m_pEventConfigs, pEventCfg, sizeof(DcEventConfig)*numberOfEvents) ;
    }
}

void DcConfig::SetEventInfo(const gtVector<DcEventConfig>& eventsConfigVector)
{
    m_numberOfEvents = (int)eventsConfigVector.size();

    // Delete existing event group array to avoid memory leak
    if (m_pEventConfigs != NULL) { delete[] m_pEventConfigs; }

    m_pEventConfigs = NULL;

    if (eventsConfigVector.size() > 0)
    {
        m_pEventConfigs = new DcEventConfig[m_numberOfEvents];
        DEBUG_ASSERT(m_pEventConfigs != NULL);

        for (int i = 0; i < m_numberOfEvents; i++)
        {
            m_pEventConfigs[i].eventCount = eventsConfigVector[i].eventCount;
            m_pEventConfigs[i].pmc = eventsConfigVector[i].pmc;
        }
    }
}

//
// Set the IBS configuration information. IBS only.
//
void DcConfig::SetIBSInfo(IbsConfig* pIBS)
{
    DEBUG_ASSERT(pIBS != NULL) ;

    if (pIBS != NULL)
    {
        m_IBSConfig.fetchSampling = pIBS->fetchSampling ;
        m_IBSConfig.fetchMaxCount = pIBS->fetchMaxCount ;
        m_IBSConfig.opSampling = pIBS->opSampling ;
        m_IBSConfig.opMaxCount = pIBS->opMaxCount ;
        m_IBSConfig.opCycleCount = pIBS->opCycleCount;
    }
}

//
// Set the CLU configuration information. CLU only.
//
void DcConfig::SetCLUInfo(CluConfig* pCLU)
{
    DEBUG_ASSERT(pCLU != NULL) ;

    if (pCLU != NULL)
    {
        m_cluConfig.cluSampling = pCLU->cluSampling ;
        m_cluConfig.cluMaxCount = pCLU->cluMaxCount ;
        m_cluConfig.cluCycleCount = pCLU->cluCycleCount;
    }
}

//
// Write the tool tip element to the XML output stream.
//
void DcConfig::WriteTooltip(QTextStream& xmlStream, QString& text)
{
    xmlStream << "    <tool_tip>" << text << "</tool_tip>\n" ;
}

//
// Write the description element to the XML output stream.
//
void DcConfig::WriteDescription(QTextStream& xmlStream, QString& text)
{
    xmlStream << "    <description>" << text << "</description>\n" ;
}

//
// Write a string-valued attribute to the XML output stream.
//
void DcConfig::WriteStringAttr(QTextStream& xmlStream,
                               const char* attr, QString& value)
{
    xmlStream << ' ' << attr << "=\"" << value << "\"" ;
}

//
// Write an integer attribute in decimal to the XML output stream.
//
void DcConfig::WriteDecimalAttr(QTextStream& xmlStream, const char* attr, gtUInt64 value)
{
    QString decimal_value ;
    decimal_value.setNum(value, 10) ;
    xmlStream << ' ' << attr << "=\"" ;
    xmlStream << decimal_value << "\"" ;
}

//
// Write an integer attribute in hexdecimal to the XML output stream.
//
void DcConfig::WriteHexAttr(QTextStream& xmlStream, const char* attr, gtUInt64 value)
{
    QString hex_value ;
    hex_value.setNum(value, 16) ;
    xmlStream << ' ' << attr << "=\"" ;
    xmlStream << hex_value << "\"" ;
}

//
// Write a float attribute in decimal to the XML output stream.
//
void DcConfig::WriteFloatAttr(QTextStream& xmlStream,
                              const char* attr, float value)
{
    QString decimal_value ;
    decimal_value.setNum(value, 'f', 4) ;
    xmlStream << ' ' << attr << "=\"" ;
    xmlStream << decimal_value << "\"" ;
}

//
// Write an Boolean attribute as "T" or "F" to the XML output stream.
//
void DcConfig::WriteBoolAttr(QTextStream& xmlStream,
                             const char* attr, bool value)
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
// Write a TBP configuration element to the XML output stream.
//
void DcConfig::WriteTBP(QTextStream& xmlStream)
{
    xmlStream << "  <tbp" ;
    WriteStringAttr(xmlStream, "name", m_configName) ;
    WriteFloatAttr(xmlStream, "interval", m_interval) ;
    xmlStream << ">\n" ;
    WriteTooltip(xmlStream, m_toolTip) ;
    WriteDescription(xmlStream, m_description) ;
    xmlStream << "  </tbp>\n" ;
}

//
// Write an event element to the XML output stream.
//
void DcConfig::WriteEvent(QTextStream& xmlStream, DcEventConfig* pEvt)
{
    unsigned int evtSelect = pEvt->pmc.ucEventSelectHigh;
    evtSelect = (evtSelect << 8) + pEvt->pmc.ucEventSelect;

    xmlStream << "      <event" ;
    WriteHexAttr(xmlStream, "select", evtSelect) ;
    WriteHexAttr(xmlStream, "mask", pEvt->pmc.ucUnitMask) ;
    WriteBoolAttr(xmlStream, "os", pEvt->pmc.bitOsEvents) ;
    WriteBoolAttr(xmlStream, "user", pEvt->pmc.bitUsrEvents) ;
    WriteBoolAttr(xmlStream, "edge_detect", pEvt->pmc.bitEdgeEvents) ;
    WriteBoolAttr(xmlStream, "host", pEvt->pmc.hostOnly) ;
    WriteBoolAttr(xmlStream, "guest", pEvt->pmc.guestOnly) ;
    WriteDecimalAttr(xmlStream, "count", pEvt->eventCount) ;
    WriteHexAttr(xmlStream, "l2i_mask", pEvt->pmc.FakeL2IMask) ;
    xmlStream << "></event>\n" ;
}

//
// Write an EBP configuration element to the XML output stream.
//
void DcConfig::WriteEBP(QTextStream& xmlStream)
{
    DcEventConfig* pEC ;

    xmlStream << "  <ebp" ;
    WriteStringAttr(xmlStream, "name", m_configName) ;
    // num_groups is not read. It is written as a debugging aid
    WriteDecimalAttr(xmlStream, "num_events", m_numberOfEvents) ;
    xmlStream << ">\n" ;

    pEC = m_pEventConfigs;

    if ((m_pEventConfigs != NULL) && (m_numberOfEvents >= 1))
    {
        for (int i = 0; i < m_numberOfEvents; i++)
        {
            WriteEvent(xmlStream, pEC);
            pEC++;
        }

    }

    WriteTooltip(xmlStream, m_toolTip) ;
    WriteDescription(xmlStream, m_description) ;
    xmlStream << "  </ebp>\n" ;
}

//
// Write an IBS configuration element to the XML output stream.
//
void DcConfig::WriteIBS(QTextStream& xmlStream)
{
    xmlStream << "  <ibs" ;
    WriteStringAttr(xmlStream, "name", m_configName) ;
    xmlStream << "\n      " ;
    WriteBoolAttr(xmlStream, "fetch_sampling", m_IBSConfig.fetchSampling) ;
    WriteDecimalAttr(xmlStream, "fetch_max_count", m_IBSConfig.fetchMaxCount) ;
    WriteBoolAttr(xmlStream, "op_sampling", m_IBSConfig.opSampling) ;
    WriteDecimalAttr(xmlStream, "op_max_count", m_IBSConfig.opMaxCount) ;
    WriteBoolAttr(xmlStream, "op_cycle_count", m_IBSConfig.opCycleCount);
    xmlStream << ">\n" ;
    WriteTooltip(xmlStream, m_toolTip) ;
    WriteDescription(xmlStream, m_description) ;
    xmlStream << "  </ibs>\n" ;
}

//
// Write an CLU configuration element to the XML output stream.
//
void DcConfig::WriteCLU(QTextStream& xmlStream)
{
    xmlStream << "  <clu" ;
    WriteStringAttr(xmlStream, "name", m_configName) ;
    xmlStream << "\n      " ;
    WriteBoolAttr(xmlStream, "clu_sampling", m_cluConfig.cluSampling) ;
    WriteDecimalAttr(xmlStream, "clu_max_count", m_cluConfig.cluMaxCount) ;
    WriteBoolAttr(xmlStream, "clu_cycle_count", m_cluConfig.cluCycleCount);
    xmlStream << ">\n" ;
    WriteTooltip(xmlStream, m_toolTip) ;
    WriteDescription(xmlStream, m_description) ;
    xmlStream << "  </clu>\n" ;
}

//
// Write the data configuration element to the XML output stream. Call
// the appropriate configuration type-specific private function to actually
// perform the writing.
//
bool DcConfig::WriteConfigFile(const wchar_t* configFileName,
                               const wchar_t* pathToDtd)
{
    QFile* pFile = NULL ;
    QTextStream xmlStream ;

    if (m_configType == DCConfigInvalid)
    {
        // No valid configuration data
        return (false) ;
    }

    pFile = new QFile(QString::fromWCharArray(configFileName)) ;

    if (! pFile->open(QIODevice::WriteOnly))
    {
        // Fill open error
        return (false) ;
    }

    xmlStream.setDevice(pFile) ;
    // Set XML file character encoding
    xmlStream.setCodec("UTF-8");

    xmlStream << "<?xml version=\"1.0\"?>\n" ;
    xmlStream << "<!DOCTYPE dc_configuration SYSTEM \"" << QString::fromWCharArray(pathToDtd);
    xmlStream << "/dcconfig.dtd\">\n";
    xmlStream << "<dc_configuration>\n" ;

    if ((DCConfigTBP == m_configType) || (DCConfigMultiple == m_configType))
    {
        WriteTBP(xmlStream) ;
    }

    if ((DCConfigEBP == m_configType) || (DCConfigMultiple == m_configType))
    {
        WriteEBP(xmlStream) ;
    }

    if ((DCConfigIBS == m_configType) || (DCConfigMultiple == m_configType))
    {
        WriteIBS(xmlStream) ;
    }

    if ((DCConfigCLU == m_configType) || (DCConfigMultiple == m_configType))
    {
        WriteCLU(xmlStream) ;
    }

    xmlStream << "</dc_configuration>\n" ;

    // Close file and deallocate QFile explicitly
    xmlStream.setDevice(0) ;
    pFile->close() ;
    delete pFile ;
    return (true) ;
}

////////////////////////////////////////////////////////////////////////////////////
// DCConfig accessor functions
////////////////////////////////////////////////////////////////////////////////////

//
// Return the data collection configuration type to the caller.
//
DcConfigType DcConfig::GetConfigType()
{
    return (m_configType) ;
}

//
// Return the configuration name to the caller.
//
void DcConfig::GetConfigName(QString& configName)
{
    configName = m_configName ;
}

//
// Return the tool tip string to the caller.
//
void DcConfig::GetToolTip(QString& toolTip)
{
    toolTip = m_toolTip ;
}

//
// Return the description string to the caller.
//
void DcConfig::GetDescription(QString& description)
{
    description = m_description ;
}

//
// Return the timer interval to the caller. TBP only.
//
float DcConfig::GetTimerInterval()
{
    return (m_interval) ;
}

//
// Return the number of event groups to the caller. EBP only.
//
int DcConfig::GetNumberOfEvents()
{
    return (m_numberOfEvents) ;
}

//
// Return event configuration information to the caller. The caller
// must provide an array of DcEventConfig structs as the destination of
// the configuration data, i.e., storage management is the responsibility
// of the caller. EBP only.
//
void DcConfig::GetEventInfo(
    DcEventConfig* pEventCfg, int numberOfEvents)
{
    //
    // Warning this is only for gcc so I do not think this is a good idea
    // this is also not being built live on Linux which should also be addressed.
    //
    DEBUG_ASSERT(pEventCfg != NULL) ;
    DEBUG_ASSERT(numberOfEvents >= 0) ; // Change to >= 0 from > 0

    if ((numberOfEvents > 0) && (pEventCfg != NULL))
    {
        if (m_pEventConfigs != NULL)
        {
            memcpy(pEventCfg, m_pEventConfigs, sizeof(DcEventConfig)*numberOfEvents) ;
        }
    }
}
//
// Return event configuration information to the caller. The caller
// must provide an array of DcEventConfig structs as the destination of
// the configuration data, i.e., storage management is the responsibility
// of the caller. EBP only.
//
void DcConfig::GetEventInfo(gtVector<DcEventConfig>& eventsConfigVector)
{
    eventsConfigVector.clear();
    eventsConfigVector.reserve(m_numberOfEvents);

    if (m_pEventConfigs != NULL)
    {
        for (int i = 0; i < m_numberOfEvents; i++)
        {
            DcEventConfig config;
            config.eventCount = m_pEventConfigs[i].eventCount;
            config.pmc = m_pEventConfigs[i].pmc;
            eventsConfigVector.push_back(config);
        }
    }
}

//
// Return IBS configuration information to the caller. The caller
// must provide an IbsConfig struct as the destination of the configuration
// data. IBS only.
//
void DcConfig::GetIBSInfo(IbsConfig* pIBS)
{
    DEBUG_ASSERT(pIBS != NULL) ;

    if (pIBS != NULL)
    {
        pIBS->fetchSampling = m_IBSConfig.fetchSampling ;
        pIBS->fetchMaxCount = m_IBSConfig.fetchMaxCount ;
        pIBS->opSampling = m_IBSConfig.opSampling ;
        pIBS->opMaxCount = m_IBSConfig.opMaxCount ;
        pIBS->opCycleCount = m_IBSConfig.opCycleCount;
    }
}

//
// Return CLU configuration information to the caller. The caller
// must provide an CluConfig struct as the destination of the configuration
// data. CLU only.
//
void DcConfig::GetCLUInfo(CluConfig* pCLU)
{
    DEBUG_ASSERT(pCLU != NULL) ;

    if (pCLU != NULL)
    {
        pCLU->cluSampling = m_cluConfig.cluSampling;
        pCLU->cluMaxCount = m_cluConfig.cluMaxCount;
        pCLU->cluCycleCount = m_cluConfig.cluCycleCount;
    }
}

////////////////////////////////////////////////////////////////////////////////////
// Read data collection confguration from XML file
////////////////////////////////////////////////////////////////////////////////////

//
// Set up the XML file for reading, then start the parse. The XML parse
// proceeds and calls the callback functions below. Return false if the
// parser detected an error.
//
bool DcConfig::ReadConfigFile(const wchar_t* configFileName)
{
    // Create a file, an XML input source and a simple reader
    QFile xmlFile(QString::fromWCharArray(configFileName)) ;
    QXmlInputSource source(& xmlFile) ;
    QXmlSimpleReader reader ;
    // Connect this object's handler interface to the XML reader
    reader.setContentHandler(this) ;
    reader.setErrorHandler(this);
    // Return true if the parse succeeds and no XML semantic errors
    return (reader.parse(source) && (!m_xmlSemanticError)) ;
}

////////////////////////////////////////////////////////////////////////////////////
// Callback functions for reading a data collection configuration in XML
////////////////////////////////////////////////////////////////////////////////////

//
// startDocument() is called at the beginning of an XML document.
// Prepare to read a data collection configuration by clearing the
// current configuration.
//
bool DcConfig::startDocument()
{
    m_configName = DefaultConfigName ;
    m_toolTip = "" ;
    m_description = "" ;
    // Clear all DC configuration info as if this was a new object
    m_configType = DCConfigInvalid ;
    // Time-based profiling configuration attributes
    m_interval = DefaultInterval ;
    // Event-based profiling configuration attributes
    m_numberOfEvents = 0 ;

    if (m_pEventConfigs != NULL) { delete [] m_pEventConfigs ; }

    m_pEventConfigs = NULL ;
    // IBS configuration attributes
    m_IBSConfig.fetchSampling = false ;
    m_IBSConfig.fetchMaxCount = DefaultMaxCount ;
    m_IBSConfig.opSampling = false ;
    m_IBSConfig.opMaxCount = DefaultMaxCount ;
    m_IBSConfig.opCycleCount = true;

    // CLU configuration attributes
    m_cluConfig.cluSampling = false ;
    m_cluConfig.cluMaxCount = DefaultMaxCount ;
    m_cluConfig.cluCycleCount = true;

    // Initialize all XML-related working variables
    m_toolTipIsOpen = false ;
    m_descriptionIsOpen = false ;
    m_xmlSemanticError = false ;
    m_eventConfigList.clear() ;

    return (true) ;
}

//
// endDocument() is called at the beginning of an XML document.
//
bool DcConfig::endDocument()
{
    m_eventConfigList.clear() ;
    return (true) ;
}

//
// startElement() is called at the beginning of an XML element.
// Dispatch control using the element name. Handle an element-specific
// attributes. Set-up state variables to handle the contents of the
// element as the XML parse proceeds.
//
bool DcConfig::startElement(
    const QString& namespaceURI, const QString& localName,
    const QString& qName, const QXmlAttributes& atts)
{
    (void)(namespaceURI); // unused
    (void)(localName); // unused
    QString boolValue ;
    QString elementName = qName.toLower() ;

    if (elementName == "ebp")
    {
        /////////////////////////////////////////////////////////
        // <ebp> Read and store event-based profiling parameters
        /////////////////////////////////////////////////////////
        DEBUG_ASSERT(!atts.value("name").isNull()) ;

        if (DCConfigInvalid == m_configType)
        {
            m_configType = DCConfigEBP ;
        }
        else
        {
            m_configType = DCConfigMultiple;
        }

        // Use the default name when configuration name is missing
        if (! atts.value("name").isNull())
        {
            m_configName = atts.value("name") ;
        }
    }
    else if (elementName == "event")
    {
        /////////////////////////////////////////////////////////
        // <event> Read and store
        // event parameters in the last event group on list
        /////////////////////////////////////////////////////////
        DEBUG_ASSERT(!atts.value("select").isNull()) ;
        DEBUG_ASSERT(!atts.value("mask").isNull()) ;
        DEBUG_ASSERT(!atts.value("os").isNull()) ;
        DEBUG_ASSERT(!atts.value("user").isNull()) ;
        DEBUG_ASSERT(!atts.value("count").isNull()) ;
        // "edge_detect" is an optional attribute
        // "host" is an optional attribute
        // "guest" is an optional attribute
        DcEventConfig ec ;  // Allocated on stack
        memset(&ec, 0, sizeof(DcEventConfig)) ;
        unsigned int evtSelect = atts.value("select").toInt(0, 16) ;
        ec.pmc.ucEventSelect = evtSelect & 0xFF;
        ec.pmc.ucEventSelectHigh = (evtSelect >> 8) & 0xF;
        ec.pmc.bitEnabled = 1;
        ec.pmc.bitSampleEvents = 1;

        ec.pmc.ucUnitMask = atts.value("mask").toInt(0, 16) ;

        // CODEXL-907 BSOD: Disable Counting of events occurring on OS mode while profiling in guest OS on VMware
        osCpuid cpuInfo;

        if (!cpuInfo.hasHypervisor() || cpuInfo.getHypervisorVendorId() != HV_VENDOR_VMWARE)
        {
            boolValue = atts.value("os").toUpper();
            ec.pmc.bitOsEvents = (boolValue == "T");
        }

        boolValue = atts.value("user").toUpper() ;
        ec.pmc.bitUsrEvents = (boolValue == "T") ;

        if ((evtSelect & (FAKE_L2I_EVENT_ID_PREFIX << 12)) == (FAKE_L2I_EVENT_ID_PREFIX << 12))
        {
            // L2I event
            ec.pmc.FakeL2IMask = FAKE_L2I_MASK_VALUE;
            ec.pmc.bitOsEvents = 0;
            ec.pmc.bitUsrEvents = 0;
        }

        if (! atts.value("edge_detect").isNull())
        {
            boolValue = atts.value("edge_detect").toUpper() ;
            ec.pmc.bitEdgeEvents = (boolValue == "T") ;
        }

        if (! atts.value("host").isNull())
        {
            boolValue = atts.value("host").toUpper() ;
            ec.pmc.hostOnly = (boolValue == "T") ;
        }

        if (! atts.value("guest").isNull())
        {
            boolValue = atts.value("guest").toUpper() ;
            ec.pmc.guestOnly = (boolValue == "T") ;
        }

        if (! atts.value("l2i_mask").isNull())
        {
            ec.pmc.FakeL2IMask = atts.value("l2i_mask").toInt(0, 16) ;
        }

        ec.eventCount = atts.value("count").toInt(0, 10) ;
        m_eventConfigList.push_back(ec) ;
        m_numberOfEvents++;
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
    else if (elementName == "tbp")
    {
        /////////////////////////////////////////////////////////
        // <tbp> time-based profiling configuration parameters
        /////////////////////////////////////////////////////////
        DEBUG_ASSERT(!atts.value("name").isNull()) ;
        DEBUG_ASSERT(!atts.value("interval").isNull()) ;

        if (DCConfigInvalid == m_configType)
        {
            m_configType = DCConfigTBP ;
        }
        else
        {
            m_configType = DCConfigMultiple;
        }

        // if (atts.value("name").isNull()) m_xmlSemanticError = true ;
        // Use the default name when configuration name is missing
        if (! atts.value("name").isNull())
        {
            m_configName = atts.value("name") ;
        }

        m_interval = atts.value("interval").toFloat(0) ;
    }
    else if (elementName == "ibs")
    {
        /////////////////////////////////////////////////////////
        // <ibs> ibs configuration parameters
        /////////////////////////////////////////////////////////
        DEBUG_ASSERT(!atts.value("name").isNull()) ;
        DEBUG_ASSERT(!atts.value("fetch_sampling").isNull()) ;
        DEBUG_ASSERT(!atts.value("fetch_max_count").isNull()) ;
        DEBUG_ASSERT(!atts.value("op_sampling").isNull()) ;
        DEBUG_ASSERT(!atts.value("op_max_count").isNull()) ;

        if (DCConfigInvalid == m_configType)
        {
            m_configType = DCConfigIBS ;
        }
        else
        {
            m_configType = DCConfigMultiple;
        }

        // Use the default name when configuration name is missing
        if (! atts.value("name").isNull())
        {
            m_configName = atts.value("name") ;
        }

        m_IBSConfig.fetchSampling = false ;
        m_IBSConfig.fetchMaxCount = DefaultMaxCount ;
        m_IBSConfig.opSampling = false ;
        m_IBSConfig.opMaxCount = DefaultMaxCount ;
        boolValue = atts.value("fetch_sampling").toUpper() ;
        m_IBSConfig.fetchSampling = (boolValue == "T") ;
        boolValue = atts.value("op_sampling").toUpper() ;
        m_IBSConfig.opSampling = (boolValue == "T") ;
        m_IBSConfig.fetchMaxCount = atts.value("fetch_max_count").toInt(0, 10) ;
        m_IBSConfig.opMaxCount = atts.value("op_max_count").toInt(0, 10) ;

        // Use the default when attribute is missing
        if (! atts.value("op_cycle_count").isNull())
        {
            boolValue = atts.value("op_cycle_count").toUpper() ;
            m_IBSConfig.opCycleCount = (boolValue == "T") ;
        }
    }
    else if (elementName == "clu")
    {
        /////////////////////////////////////////////////////////
        // <clu> clu configuration parameters
        /////////////////////////////////////////////////////////
        DEBUG_ASSERT(!atts.value("name").isNull()) ;
        DEBUG_ASSERT(!atts.value("clu_sampling").isNull()) ;
        DEBUG_ASSERT(!atts.value("clu_max_count").isNull()) ;

        if (DCConfigInvalid == m_configType)
        {
            m_configType = DCConfigCLU ;
        }
        else
        {
            m_configType = DCConfigMultiple;
        }

        // Use the default name when configuration name is missing
        if (! atts.value("name").isNull())
        {
            m_configName = atts.value("name") ;
        }

        boolValue = atts.value("clu_sampling").toUpper() ;
        m_cluConfig.cluSampling = (boolValue == "T") ;
        m_cluConfig.cluMaxCount = atts.value("clu_max_count").toInt(0, 10) ;

        // Use the default when attribute is missing
        if (! atts.value("clu_cycle_count").isNull())
        {
            boolValue = atts.value("clu_cycle_count").toUpper() ;
            m_cluConfig.cluCycleCount = (boolValue == "T") ;
        }
    }
    else if (elementName == "dc_configuration")
    {
        /////////////////////////////////////////////////////////
        // <dc_configuration>
        /////////////////////////////////////////////////////////
    }
    else if (elementName == "group")
    {
        //ignore
    }
    else
    {
        // Unrecognized element name
        return (false) ;
    }

    return (true) ;
}

//
// endElement() is called at the end of an XML element. Dispatch
// control using the element name to perform element-specific
// finalization of the element's contents.
//
bool DcConfig::endElement(
    const QString& namespaceURI, const QString& localName,
    const QString& qName)
{
    (void)(namespaceURI); // unused
    (void)(localName); // unused
    QString elementName = qName.toLower() ;

    if (elementName == "ebp")
    {
        /////////////////////////////////////////////////////////
        // </ebp> Copy event group list to array
        /////////////////////////////////////////////////////////
        DcEventConfig* pEC = nullptr;

        if ((m_numberOfEvents >= 1) && ((size_t)m_numberOfEvents == (size_t)m_eventConfigList.size()))
        {
            // Allocate array to hold event groups
            m_pEventConfigs = new DcEventConfig[m_numberOfEvents];

            // Copy event groups from list to the new array
            pEC = m_pEventConfigs;

            for (EventConfigList::const_iterator eg = m_eventConfigList.begin(), egEnd = m_eventConfigList.end(); eg != egEnd; ++eg)
            {
                pEC->eventCount = eg->eventCount;
                pEC->pmc.perf_ctl = eg->pmc.perf_ctl;
                pEC++;
            }

            // Clear the event group list now that we're done with it
            m_eventConfigList.clear();
        }
    }
    else if (elementName == "tool_tip")
    {
        /////////////////////////////////////////////////////////
        // </tool_tip> Close the open tooltip element
        /////////////////////////////////////////////////////////
        m_toolTipIsOpen = false ;
    }
    else if (elementName == "description")
    {
        /////////////////////////////////////////////////////////
        // </description> Close the open description element
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
bool DcConfig::characters(const QString& ch)
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
