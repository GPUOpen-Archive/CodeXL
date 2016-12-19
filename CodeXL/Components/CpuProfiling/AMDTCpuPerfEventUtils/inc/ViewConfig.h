//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ViewConfig.h
/// \brief Interface for the system view configuration (of supported events) class.
///
//==================================================================================
#pragma once

#include <vector>
#include <cstdint>

#include "CpuPerfEventUtilsDLLBuild.h"

#if defined (_WIN32)
// Error description: 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
// However, this warning is caused by private members that do not have DLL exports definitions.
#pragma warning( disable : 4251 )
#endif

//
// Performance event configuration which is used to select
// event data to be shown in a view
//
struct EventConfig
{
    uint16_t eventSelect = 0;     // Event select value
    uint8_t eventUnitMask = 0;    // Event unit mask
    bool bitOs = false;
    bool bitUsr = false;

    EventConfig() = default;

    EventConfig(uint16_t eventSelect, uint8_t eventUnitMask, bool bitOs, bool bitUsr) :
        eventSelect(eventSelect), eventUnitMask(eventUnitMask), bitOs(bitOs), bitUsr(bitUsr) {}

    ~EventConfig() = default;
};

//
// ColumnType specifies how column content is used and shown
//
typedef enum
{
    ColumnInvalid,     // For initialization
    ColumnValue,       // Show data as a simple unmodified value
    ColumnSum,         // Compute and show sum of two data values
    ColumnDifference,  // Compute and show difference of two data values
    ColumnProduct,     // Compute and show product of two data values
    ColumnRatio,       // Compute and show ratio of two data values
    ColumnPercentage,  // compute the percentage
} ColumnType;

//
// ColumnSorting controls how data is sorted in a column
//
typedef enum
{
    NoSort,            // Don't sort data in column
    AscendingSort,     // Sort in ascending order
    DescendingSort     // Sort in descending order
} ColumnSorting;

//
// A ColumnSpec describes a column in a view
//
struct ColumnSpec
{
    ColumnType    type = ColumnInvalid; // Controls data display
    ColumnSorting sorting = NoSort;     // Control data sorting
    std::string   title;                // Column title or legend to display
    EventConfig   dataSelectLeft;       // Selects data for left operand/value
    EventConfig   dataSelectRight;      // Selects data for right operand

    ColumnSpec() = default;
    ~ColumnSpec() = default;
};

typedef std::vector<ColumnSpec> ColumnSpecList;

//
// Maintains an association between a readable ID and an event
// configuration value. The readable IDs are used in the <output>
// section of the XML file to refer to data for specific events.
//
struct DataAssoc
{
    std::string eventId;     // Event/data identifier
    EventConfig eventConfig; // Event configuration value

    DataAssoc() = default;
    ~DataAssoc() = default;
};

typedef std::vector<DataAssoc> DataAssocList;

///////////////////////////////////////////////////////////////////////////////
// ViewConfig class declaration
///////////////////////////////////////////////////////////////////////////////

class TiXmlElement;
class TiXmlDocument;

class CP_EVENT_API ViewConfig
{
public:
    ///////////////////////////////////////////////////////////////////////
    // Constructors, destructors, assignment
    ///////////////////////////////////////////////////////////////////////
    ViewConfig();
    ViewConfig(const ViewConfig& original);
    ~ViewConfig();
    const ViewConfig& operator=(const ViewConfig& rhs);

    ///////////////////////////////////////////////////////////////////////
    // Client functions for a view configuration
    ///////////////////////////////////////////////////////////////////////

    // Basic mutator and accessor methods
    void SetConfigName(const std::string& name) { m_configName = name; };
    void GetConfigName(std::string& name) { name = m_configName; };
    void SetToolTip(const std::string& toolTip) { m_toolTip = toolTip; };
    void GetToolTip(std::string& toolTip) { toolTip = m_toolTip; };
    void SetDescription(const std::string& description) { m_description = description; };
    void GetDescription(std::string& description) { description = m_description; };

    // Define column specifications for the view; Generate ID/event
    // associations when the argument generateDataSpecs is true
    void SetColumnSpecs(const ColumnSpecList& columnSpecArray, bool generateDataAssocs = true);

    // Make column specifications from an array of EventConfig values
    void MakeColumnSpecs(const std::vector<EventConfig> events, const std::vector<std::string>& titles, bool generateDataAssocs = true);

    // Return the number of columns in the view (number of column specs)
    uint32_t GetNumberOfColumns() { return m_columnList.size(); }

    // Return column specifications in the array allocated by the caller
    void GetColumnSpecs(ColumnSpecList& columnSpecArray);

    // Read a view configuration in XML from the specified file
    bool ReadConfigFile(const std::string& configFileName);

    // Write the current view configuration in XML to the specified file
    bool WriteConfigFile(const std::string& configFileName, const std::string& installDir);

    // Find data/event configuration association by id or value
    bool FindAssocById(const std::string& id, DataAssoc& assoc);
    bool FindAssocByConfig(const EventConfig& config, DataAssoc& assoc);
    bool FindAssocByValue(uint16_t select, uint8_t unitMask, DataAssoc& assoc);

    // Return EventConfig given an event ID
    EventConfig GetConfigById(const char* id);

private:
    ///////////////////////////////////////////////////////////////////////
    // Private functions
    ///////////////////////////////////////////////////////////////////////

    // Define a data/event configuration association
    void AddAssoc(const std::string& id, const EventConfig& config);
    void AddAssoc(const std::string& id, uint16_t select, uint8_t unitMask);

    bool parseDomTree(TiXmlElement* pRoot);
    bool ParseColumnSpecElement(TiXmlElement* const pElement, ColumnSpec& cs);

    bool constructDomTree(TiXmlDocument* pConfigDoc);

    // Generate data/event configuration associations for column specs
    void GenerateDataAssocs();

    ///////////////////////////////////////////////////////////////////////
    // Private functions to assist configuration writing
    ///////////////////////////////////////////////////////////////////////
    bool AddEventsToElement(TiXmlElement* pRootElem);
    bool AddColumnsToElement(TiXmlElement* pRootElem);
    bool AddArithmeticToElement(const ColumnSpec& column, TiXmlElement* pRootElem);
    TiXmlElement* CreateArithmeticElement(const std::string& name, const ColumnSpec& column, bool isBinary);

private:
    // Configuration attributes
    std::string m_configName;           // Configuration name
    std::string m_toolTip;              // Tooltip for the configuration
    std::string m_description;          // Short description of the configuration

    DataAssocList m_assocList;          // List of ID/event select associations
    ColumnSpecList m_columnList;        // List of column specs
};
