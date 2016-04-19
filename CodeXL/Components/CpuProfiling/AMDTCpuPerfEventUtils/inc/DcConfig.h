//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file DcConfig.h
/// \brief Data Collection configuration reader/writer class
///        (for custom profiling or every other profiling type).
///
//==================================================================================

#ifndef _DCCONFIG_H_
#define _DCCONFIG_H_

// Include Qt-related definitions
#include <qstring.h>
#include <qfile.h>
#include <qtextstream.h>
#include <QXmlDefaultHandler>

#include <AMDTBaseTools/Include/gtList.h>
#include "CpuPerfEventUtilsDLLBuild.h"
#include "EventEncoding.h"


struct DcEventConfig
{
    PERF_CTL pmc;               // PERF_CTL
    gtUInt64 eventCount;        // Event count aka "sampling period"; this os PERF_CTR
};

// Do not use the following type in DLL client code
typedef gtList<DcEventConfig> EventConfigList;

enum DcConfigType
{
    DCConfigInvalid,           // No or invalid DC configuration
    DCConfigTBP,               // Time-based profiling (TBP)
    DCConfigEBP,               // Event-based profiling (EBP)
    DCConfigIBS,               // Instruction-based sampling (IBS)
    DCConfigCLU,               // Cache line utilization (CLU)
    DCConfigMultiple
};

struct IbsConfig
{
    gtUInt64 fetchMaxCount;     // Maximum value of periodic fetch counter
    gtUInt64 opMaxCount;        // Maximum value of periodic op counter
    bool fetchSampling;       // Enable IBS fetch sampling
    bool opSampling;          // Enable IBS op sampling
    bool opCycleCount;      //Whether the op sampling is by cycle or dispatch
};

struct CluConfig
{
    gtUInt64 cluMaxCount;        // Maximum value of periodic op counter while CLU profiling
    bool cluSampling;            // Enable CLU sampling
    bool cluCycleCount;          // Whether the op sampling is by cycle or dispatch while CLU profiling
};

////////////////////////////////////////////////////////////////////////////////////
// CDCConfig class declaration
////////////////////////////////////////////////////////////////////////////////////

class CP_EVENT_API DcConfig : public QXmlDefaultHandler
{
public:
    ///////////////////////////////////////////////////////////////////////////////
    // Constructors, destructors, assignment
    ///////////////////////////////////////////////////////////////////////////////
    DcConfig();
    DcConfig(const DcConfig& original);
    ~DcConfig();
    const DcConfig& operator = (const DcConfig& rhs);

    ///////////////////////////////////////////////////////////////////////////////
    // Client functions for all DC configuration types
    ///////////////////////////////////////////////////////////////////////////////
    // Return the configuration type (TBP, EBP, ...)
    DcConfigType GetConfigType();
    // Return the configuration name in configName
    void GetConfigName(QString& configName);
    // Return the configuration tool type in toolTip
    void GetToolTip(QString& toolTip);
    // Return the configuration description in description
    void GetDescription(QString& description);

    // Set the configuration type
    void SetConfigType(DcConfigType configType);
    // Set the configuration name
    void SetConfigName(const QString configName);
    // Set the configuration tool tip
    void SetToolTip(const QString toolTip);
    // Set the configuration description
    void SetDescription(const QString description);

    // Read a DC configuration in XML from the specified file
    bool ReadConfigFile(const wchar_t* configFileName);
    // Write the current configuration in XML to the specified file
    bool WriteConfigFile(const wchar_t* configFileName, const wchar_t* pathToDtd);

    ///////////////////////////////////////////////////////////////////////////////
    // Client functions for TBP DC configurations
    ///////////////////////////////////////////////////////////////////////////////
    // Set the TBP timer interval (sample period)
    void SetTimerInterval(float interval);
    // Get the TBP timer interval
    float GetTimerInterval();

    ///////////////////////////////////////////////////////////////////////////////
    // Client functions for EBP DC configurations
    ///////////////////////////////////////////////////////////////////////////////
    // Set event groups and multiplex period for the configuration
    void SetEventInfo(DcEventConfig* pEventCfg, int numberOfEvents);
    // Set event groups and multiplex period for the configuration
    void SetEventInfo(const gtVector<DcEventConfig>& eventsConfigVector);
    // Return the number of event groups in the configuration
    int GetNumberOfEvents();
    // Get the event groups; Copy data to caller-supplied array of event groups
    void GetEventInfo(DcEventConfig* pEventConfig, int numberOfEvents);
    // Get the event groups; Copy data to caller-supplied vector event groups
    void GetEventInfo(gtVector<DcEventConfig>& eventsConfigVector);

    ///////////////////////////////////////////////////////////////////////////////
    // Client functions for IBS DC configurations
    ///////////////////////////////////////////////////////////////////////////////
    // Set IBS configuration info using the specified struct
    void SetIBSInfo(IbsConfig* pIBS);
    // Get IBS configuration info; Copy info to caller-supplied struct
    void GetIBSInfo(IbsConfig* pIBS);

    ///////////////////////////////////////////////////////////////////////////////
    // Client functions for CLU DC configurations
    ///////////////////////////////////////////////////////////////////////////////
    // Set CLU configuration info using the specified struct
    void SetCLUInfo(CluConfig* pCLU);
    // Get CLU configuration info; Copy info to caller-supplied struct
    void GetCLUInfo(CluConfig* pCLU);

    ///////////////////////////////////////////////////////////////////////////////
    // Callback functions which override QXmlDefaultHandler
    ///////////////////////////////////////////////////////////////////////////////
    bool startDocument();
    bool endDocument();
    bool startElement(const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts);
    bool endElement(const QString& namespaceURI, const QString& localName, const QString& qName);
    bool characters(const QString& ch);

private:
    // Common configuration attributes
    DcConfigType     m_configType;        // Kind of configuration (TBP, EBP, ...)
    QString          m_configName;        // Configuration name
    QString          m_toolTip;           // Configuration's tool tip
    QString          m_description;       // Description of the configuration
    // Time-based profiling configuration attributes
    float            m_interval;          // Timer interval (sample period)
    // Event-based profiling configuration attributes
    int              m_numberOfEvents;    // Number of event groups
    DcEventConfig*    m_pEventConfigs;       // Points to array of event groups
    // IBS configuration attributes
    IbsConfig        m_IBSConfig;         // Holds IBS-specific configuration info
    // CLU configuration attributes
    CluConfig        m_cluConfig;         // Holds CLU-specific configuration info

    // Private data members for XML handling
    EventConfigList  m_eventConfigList;    // Holds event configure as they come in
    bool             m_toolTipIsOpen;     // Tooltip is currently open when true
    bool             m_descriptionIsOpen; // Description is currently open
    bool             m_xmlSemanticError;  // Is true on XML semantic error

    ///////////////////////////////////////////////////////////////////////////////
    // Private functions to assist configuration writing
    ///////////////////////////////////////////////////////////////////////////////
    // Write tool tip element to the XML stream
    void WriteTooltip(QTextStream& xmlStream, QString& text);
    // Write description element to the XML stream
    void WriteDescription(QTextStream& xmlStream, QString& text);
    // Write string-valued attribute to the XML stream
    void WriteStringAttr(QTextStream& xmlStream, const char* attr, QString& value);
    // Write integer-valued attribute (decimal format) to the XML stream
    void WriteDecimalAttr(QTextStream& xmlStream, const char* attr, gtUInt64 value);
    // Write integer-valued attribute (hexadecimal format) to the XML stream
    void WriteHexAttr(QTextStream& xmlStream, const char* attr, gtUInt64 value);
    // Write float-valued attribute (decimal format) to the XML stream
    void WriteFloatAttr(QTextStream& xmlStream, const char* attr, float value);
    // Write Boolean-valued attribute to the XML stream
    void WriteBoolAttr(QTextStream& xmlStream, const char* attr, bool value);
    // Write an event group element to the XML stream
    void WriteEvent(QTextStream& xmlStream, DcEventConfig* pEvt);
    // Write TBP configuration to the XML stream
    void WriteTBP(QTextStream& xmlStream);
    // Write EBP configuration to the XML stream
    void WriteEBP(QTextStream& xmlStream);
    // Write IBS configuration to the XML stream
    void WriteIBS(QTextStream& xmlStream);
    // Write CLU configuration to the XML stream
    void WriteCLU(QTextStream& xmlStream);
};

#endif // _DCCONFIG_H_
