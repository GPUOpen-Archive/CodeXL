//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ViewConfig.h
/// \brief Interface for the system view configuration (of supported events) class.
///
//==================================================================================

#ifndef _VIEWCONFIG_H_
#define _VIEWCONFIG_H_

// Include Qt-related definitions
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QXmlDefaultHandler>

#include "CpuPerfEventUtilsDLLBuild.h"
#include <AMDTBaseTools/Include/gtList.h>

//
// Performance event configuration which is used to select
// event data to be shown in a view
//
typedef struct
{
    gtUInt16 eventSelect;     // Event select value
    gtUByte eventUnitMask;   // Event unit mask
    bool bitOs;
    bool bitUsr;
}
EventConfig, *pEventConfig;

//
// ColumnType specifies how column content is used and shown
//
typedef enum
{
    ColumnInvalid,     // For initialization
    ColumnValue,       // Show data as a simple umodified value
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
typedef struct
{
    ColumnType    type;             // Controls data display
    ColumnSorting sorting;          // Control data sorting
    bool          visible;          // Shows column (T) or hides it (F)
    QString       title;            // Column title or legend to display
    EventConfig   dataSelectLeft;   // Selects data for left operand/value
    EventConfig   dataSelectRight;  // Selects data for right operand
} ColumnSpec;

//
// This type is for internal use only. It is not exportable through
// the DLL interface. It is used during XML processing to build a list
// of column specifications. Once the list is built, it is copied to
// an array of column specifications and the list is discarded. This
// lets us handle a variable number of column specs on input before
// the exact number of columns is known.
//
typedef gtList<ColumnSpec> ColumnSpecList;

//
// Maintains an association between a readable ID and an event
// configuration value. The readable IDs are used in the <output>
// section of the XML file to refer to data for specific events.
//
typedef struct
{
    QString     eventId;     // Event/data identifier
    EventConfig eventConfig; // Event configuration value
} DataAssoc;

//
// This type is for internal use only. It is not exportable through
// the DLL interface. It maintains the list of ID/event associations
// for the view configuration. This list determines the events that
// are needed to display the view. Thus, if data for an event are to
// to be displayed, there must be an entry for the event in this list.
//
typedef gtList<DataAssoc> DataAssocList;

///////////////////////////////////////////////////////////////////////////////
// ViewConfig class declaration
///////////////////////////////////////////////////////////////////////////////

class CP_EVENT_API ViewConfig : public QXmlDefaultHandler
{
public:

    ///////////////////////////////////////////////////////////////////////
    // Constructors, destructors, assignment
    ///////////////////////////////////////////////////////////////////////
    ViewConfig();
    ViewConfig(const ViewConfig& original);
    ~ViewConfig();
    const ViewConfig& operator = (const ViewConfig& rhs);

    ///////////////////////////////////////////////////////////////////////
    // Client functions for a view configuration
    ///////////////////////////////////////////////////////////////////////

    // Basic mutator and accessor methods
    void SetConfigName(const QString name);
    void GetConfigName(QString& name);
    void SetDefaultView(bool flag) { m_defaultView = flag; }
    bool GetDefaultView() { return (m_defaultView); }
    void SetShowPercentage(bool flag) { m_showPercentage = flag; }
    bool GetShowPercentage() { return (m_showPercentage); }
    void SetToolTip(const QString toolTip);
    void GetToolTip(QString& toolTip);
    void SetDescription(const QString description);
    void GetDescription(QString& description);

    // Determine if this view is displayable from available events
    bool isViewDisplayable(EventConfig* pAvailable, int numberOfEvents, bool testUnitMask = false);

    // Define column specifications for the view; Generate ID/event
    // associations when the argument generateDataSpecs is true
    void SetColumnSpecs(ColumnSpec* columnSpecArray, int numberOfColumns,
                        const int generateDataAssocs = true);
    // Make column specifications from an array of EventConfig values
    void MakeColumnSpecs(int numberOfEvents, EventConfig* pEvents, QStringList titles, const int generateDataAssocs = true);
    // Return the number of columns in the view (number of column specs)
    int GetNumberOfColumns() { return (m_numberOfColumns); }
    // Return column specifications in the array allocated by the caller
    void GetColumnSpecs(ColumnSpec* columnSpecArray);
    // Static class method to copy an array of ColumnSpec
    static void CopyColumnSpecs(ColumnSpec* fromArray, ColumnSpec* toArray, int size);

    // Read a view configuration in XML from the specified file
    bool ReadConfigFile(QString configFileName);
    // Write the current view configuration in XML to the specified file
    bool WriteConfigFile(QString configFileName, QString InstallDir);

    // Define a data/event configuration association
    void AddAssoc(const QString id, const EventConfig& config);
    void AddAssoc(const QString id, unsigned int select, unsigned int unitMask);
    // Find data/event configuration association by id or value
    bool FindAssocById(const QString id, DataAssoc& assoc);
    bool FindAssocByConfig(const EventConfig& config, DataAssoc& assoc);
    bool FindAssocByValue(unsigned int select, unsigned int unitMask, DataAssoc& assoc);
    // Return EventConfig given an event ID
    EventConfig GetConfigById(const QString id);

    ///////////////////////////////////////////////////////////////////////
    // Callback functions which override QXmlDefaultHandler
    ///////////////////////////////////////////////////////////////////////
    bool startDocument();
    bool endDocument();
    bool startElement(const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts);
    bool endElement(const QString& namespaceURI, const QString& localName, const QString& qName);
    bool characters(const QString& ch);

private:
    ///////////////////////////////////////////////////////////////////////
    // Private functions
    ///////////////////////////////////////////////////////////////////////
    // Extract an 8-bit unit mask value from an event configuration
    unsigned int ExtractUnitMask(EventConfig config);
    // Generate data/event configuration associations for column specs
    void GenerateDataAssocs(ColumnSpec* columnSpecArray, int numberOfColumns);
    // Return true if config is in the array of event configurations
    bool FindEventConfig(const EventConfig& config, EventConfig* pArray, int numberOfEvents, bool testUnitMask);

    ///////////////////////////////////////////////////////////////////////
    // Private functions to assist configuration writing
    ///////////////////////////////////////////////////////////////////////
    // Write tool tip element to the XML stream
    void WriteTooltip(QTextStream& xmlStream, QString& text);
    // Write description element to the XML stream
    void WriteDescription(QTextStream& xmlStream, QString& text);
    // Write string-valued attribute to the XML stream
    void WriteStringAttr(QTextStream& xmlStream, char* attr, const QString& value);
    void WriteStringAttr(QTextStream& xmlStream, const char* attr, const QString& value);
    // Write integer-valued attribute (decimal format) to the XML stream
    void WriteDecimalAttr(QTextStream& xmlStream, char* attr, gtUInt64 value);
    // Write integer-valued attribute (hexadecimal format) to the XML stream
    void WriteHexAttr(QTextStream& xmlStream, char* attr, gtUInt64 value);
    void WriteHexAttr(QTextStream& xmlStream, const char* attr, gtUInt64 value);
    // Write float-valued attribute (decimal format) to the XML stream
    void WriteFloatAttr(QTextStream& xmlStream, char* attr, float value);
    // Write Boolean-valued attribute to the XML stream
    void WriteBoolAttr(QTextStream& xmlStream, char* attr, bool value);
    void WriteBoolAttr(QTextStream& xmlStream, const char* attr, bool value);
    // Write the <data> element to the XML stream
    void WriteData(QTextStream& xmlStream);
    // Write an event ID to the XML stream
    void WriteEventId(QTextStream& xmlStream, char* attr, const EventConfig& config);
    void WriteEventId(QTextStream& xmlStream, const char* attr, const EventConfig& config);
    // Write an arithmetic relationship to the XML stream
    void WriteArith(QTextStream& xmlStream, const ColumnSpec& column);
    // Write a <column> element to the XML stream
    void WriteColumn(QTextStream& xmlStream, const ColumnSpec& column);
    // Write the <output> element to the XML stream
    void WriteOutput(QTextStream& xmlStream);
    // Write the <view> element to the XML stream
    void WriteView(QTextStream& xmlStream);


private:
    // Configuration attributes
    QString m_configName;        // Configuration name
    QString m_toolTip;           // Tooltip for the configuration
    QString m_description;       // Short description of the configuration
    bool m_defaultView;          // Candidate default view (T) or not (F)
    bool m_showPercentage;       // Show %'tage column in view (T) or not (F)
    DataAssocList m_assocList;   // List of ID/event select associations
    int m_numberOfColumns;       // Number of columns / columns specs
    ColumnSpec* m_columnSpecs;   // Column specifications (1D array)

    // Private data members for XML handling
    bool m_viewIsOpen;           // <view> element is open
    bool m_dataIsOpen;           // <data> element is open
    bool m_columnIsOpen;         // <column> element is open
    bool m_outputIsOpen;         // <output> element is open
    bool m_toolTipIsOpen;        // <tool_tip> element is open
    bool m_descriptionIsOpen;    // <dscription> element is open
    bool m_xmlSemanticError;     // Is true on XML semantic error
    ColumnSpecList m_columnList; // Temporary list of column specs
};

#endif  // _VIEWCONFIG_H_
