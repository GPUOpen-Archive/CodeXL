//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ViewConfig.cpp
/// \brief View configuration class (including reader/writer)
///
//==================================================================================

#include <sstream>
#include <ViewConfig.h>
#include <tinyxml.h>

// Constants for initializing private data members
#define DefaultConfigName  "No name"
#define DefaultColumnTitle "No title"
#define DefaultEventSelect 0
#define DefaultUnitMask    0

const EventConfig DefaultEventConfig = { DefaultEventSelect, DefaultUnitMask, false, false };

static std::string toHexString(uint64_t num)
{
    std::stringstream ss;
    ss << std::hex << num << std::dec;
    return ss.str();
}

static ColumnSorting GetSortType(std::string sortStr)
{
    if (sortStr == "ascending")
    {
        return ColumnSorting::AscendingSort;
    }
    else if (sortStr == "descending")
    {
        return ColumnSorting::DescendingSort;
    }

    return ColumnSorting::NoSort;
}

static std::string GetSortTypeStr(ColumnSorting sortType)
{
    if (sortType == ColumnSorting::AscendingSort)
    {
        return "ascending";
    }
    else if (sortType == ColumnSorting::DescendingSort)
    {
        return "descending";
    }

    return "none";
}

///////////////////////////////////////////////////////////////////////
// ViewConfig class definition
///////////////////////////////////////////////////////////////////////

//
// Class constructor
//
ViewConfig::ViewConfig()
{
    m_configName = DefaultConfigName;
}

//
// Class copy constructor
//
ViewConfig::ViewConfig(const ViewConfig& original)
{
    m_configName = original.m_configName;
    m_toolTip = original.m_toolTip;
    m_description = original.m_description;
    m_assocList = original.m_assocList;
    m_columnList = original.m_columnList;
}

//
// Class assignment operator
//
const ViewConfig& ViewConfig::operator=(const ViewConfig& rhs)
{
    if (this != &rhs)
    {
        m_configName = rhs.m_configName;
        m_toolTip = rhs.m_toolTip;
        m_description = rhs.m_description;

        m_assocList.clear();
        m_assocList = rhs.m_assocList;

        m_columnList.clear();
        m_columnList = rhs.m_columnList;
    }

    return (*this);
}

//
// Class destructor
//
ViewConfig::~ViewConfig()
{
    m_assocList.clear();
    m_columnList.clear();
}

//
// Create data associations from column specifications.
// Generate ID/event associations for all events; This option
// eases the burden on the GUI until it can request IDs from the user
//
void ViewConfig::GenerateDataAssocs()
{
    DataAssoc da;
    std::string id;
    uint64_t id_value;

    m_assocList.clear();

    for (const auto& colSpec : m_columnList)
    {
        switch (colSpec.type)
        {
        case ColumnSum:
        case ColumnDifference:
        case ColumnProduct:
        case ColumnRatio:
            if (!FindAssocByConfig(colSpec.dataSelectRight, da))
            {
                id = "e";
                id_value = (colSpec.dataSelectRight.eventSelect << 8) |
                    (colSpec.dataSelectRight.eventUnitMask & 0xFF);
                id.append(toHexString(id_value));
                AddAssoc(id, colSpec.dataSelectRight);
            }

            // Fall through is intended! Must handle *both* right and left
        case ColumnValue:
            if (!FindAssocByConfig(colSpec.dataSelectLeft, da))
            {
                id = "e";
                id_value = (colSpec.dataSelectLeft.eventSelect << 8) |
                    (colSpec.dataSelectLeft.eventUnitMask & 0xFF);
                id.append(toHexString(id_value));
                AddAssoc(id, colSpec.dataSelectLeft);
            }
            break;

            // ColumnInvalid
        default:
            break;
        }
    }
}

//
// Define column specifications for the view. The caller provides a
// one-dimensional array of ColumnSpec structs where each ColumnSpec
// defines a column. Order of the ColumnSpecs is important and must
// be preserved; columns will be displayed in order from first to last.
//
void ViewConfig::SetColumnSpecs(const ColumnSpecList& columnSpecArray, bool generateDataAssocs)
{
    m_columnList = columnSpecArray;

    if (generateDataAssocs)
    {
        GenerateDataAssocs();
    }
}

//
// Make column specifications from an array of EventConfig values. Use
// the list of title strings to initialize the column titles. Create a
// column spec for each event (show value, no sorting.) Generate data
// associations, if enabled.
//
void ViewConfig::MakeColumnSpecs(const std::vector<EventConfig> events, const std::vector<std::string>& titles, bool generateDataAssocs)
{
    m_columnList.clear();

    if (events.size() == titles.size() && events.size() > 0)
    {
        // Initialize each column spec for an event
        size_t numberOfEvents = events.size();

        for (size_t i = 0; i < numberOfEvents; i++)
        {
            ColumnSpec colSpec;
            colSpec.type = ColumnValue;
            colSpec.sorting = NoSort;
            colSpec.dataSelectLeft = events[i];
            colSpec.dataSelectRight.eventSelect = DefaultEventSelect;
            colSpec.dataSelectRight.eventUnitMask = DefaultUnitMask;
            colSpec.title = titles[i];

            m_columnList.push_back(colSpec);
        }

        if (generateDataAssocs)
        {
            GenerateDataAssocs();
        }
    }
}


//
// Return column specifications in the array allocated by the caller.
// The array *must* be large enough to hold all of the view's column
// specifications. GetNumberOfColumns() can be called to determine the
// size of the array to be allocated.
//
void ViewConfig::GetColumnSpecs(ColumnSpecList& columnSpecArray)
{
    columnSpecArray = m_columnList;
}


///////////////////////////////////////////////////////////////////////
// Id/event configuration association
///////////////////////////////////////////////////////////////////////

//
// Define new id/event configuration association
//
void ViewConfig::AddAssoc(const std::string& id, const EventConfig& config)
{
    // This function does not check for duplicates
    DataAssoc da;

    da.eventId = id;
    da.eventConfig.eventSelect = config.eventSelect;
    da.eventConfig.eventUnitMask = config.eventUnitMask;
    da.eventConfig.bitOs = config.bitOs;
    da.eventConfig.bitUsr = config.bitUsr;

    m_assocList.push_back(da);
}

void ViewConfig::AddAssoc(const std::string& id, uint16_t select, uint8_t unitMask)
{
    // This function does not check for duplicates
    DataAssoc da;

    da.eventId = id;
    da.eventConfig.eventSelect = select;
    da.eventConfig.eventUnitMask = unitMask;

    m_assocList.push_back(da);
}

//
// Find data/event configuration association by id or value
//
bool ViewConfig::FindAssocById(const std::string& id, DataAssoc& assoc)
{
    for (const auto& da : m_assocList)
    {
        if (da.eventId == id)
        {
            assoc = da;
            return true;
        }
    }

    return false;
}

bool ViewConfig::FindAssocByConfig(const EventConfig& config, DataAssoc& assoc)
{
    for (const auto& da : m_assocList)
    {
        if ((da.eventConfig.eventSelect == config.eventSelect) &&
            (da.eventConfig.eventUnitMask == config.eventUnitMask))
        {
            assoc = da;
            return true;
        }
    }

    return false;
}

bool ViewConfig::FindAssocByValue(uint16_t select, uint8_t unitMask, DataAssoc& assoc)
{
    for (const auto& da : m_assocList)
    {
        if ((da.eventConfig.eventSelect == select) &&
            (da.eventConfig.eventUnitMask == unitMask))
        {
            assoc = da;
            return true;
        }
    }

    return false;
}

//
// Return the EventConfig value associated with an event ID. If the
// ID is not found, then return NULL.
//
EventConfig ViewConfig::GetConfigById(const char* id)
{
    DataAssoc da;

    if (id != nullptr && FindAssocById(id, da))
    {
        return da.eventConfig;
    }

    return DefaultEventConfig;
}

bool ViewConfig::AddEventsToElement(TiXmlElement* pRootElem)
{
    if (pRootElem)
    {
        for (const auto& event : m_assocList)
        {
            TiXmlElement* pEventElem = new TiXmlElement("event");

            if (pEventElem)
            {
                pEventElem->SetAttribute("id", event.eventId.data());

                std::string valueStr = toHexString(event.eventConfig.eventSelect);
                pEventElem->SetAttribute("select", valueStr.data());

                valueStr = toHexString(event.eventConfig.eventUnitMask);
                pEventElem->SetAttribute("mask", valueStr.data());

                valueStr = (event.eventConfig.bitOs) ? "T" : "F";
                pEventElem->SetAttribute("Os", valueStr.data());

                valueStr = (event.eventConfig.bitUsr) ? "T" : "F";
                pEventElem->SetAttribute("Usr", valueStr.data());

                pRootElem->LinkEndChild(pEventElem);
            }
        }
    }

    return true;
}

TiXmlElement* ViewConfig::CreateArithmeticElement(const std::string& name, const ColumnSpec& column, bool isBinary)
{
    TiXmlElement* pArithElem = new TiXmlElement(name.data());

    if (pArithElem)
    {
        DataAssoc da;

        if (isBinary)
        {
            if (FindAssocByConfig(column.dataSelectLeft, da))
            {
                pArithElem->SetAttribute("left", da.eventId.data());
            }
            else
            {
                //"<!-- Missing left event ID -->"
                TiXmlComment* pText = new TiXmlComment(" Missing left event ID ");
                pArithElem->LinkEndChild(pText);
            }

            if (FindAssocByConfig(column.dataSelectRight, da))
            {
                pArithElem->SetAttribute("right", da.eventId.data());
            }
            else
            {
                //"<!-- Missing right event ID -->"
                TiXmlComment* pText = new TiXmlComment(" Missing right event ID ");
                pArithElem->LinkEndChild(pText);
            }
        }
        else
        {
            if (FindAssocByConfig(column.dataSelectLeft, da))
            {
                pArithElem->SetAttribute("id", da.eventId.data());
            }
            else
            {
                //"<!-- Missing event ID -->"
                TiXmlComment* pText = new TiXmlComment(" Missing event ID ");
                pArithElem->LinkEndChild(pText);
            }
        }
    }

    return pArithElem;
}

bool ViewConfig::AddArithmeticToElement(const ColumnSpec& column, TiXmlElement* pRootElem)
{
    if (pRootElem)
    {
        switch (column.type)
        {
        case ColumnType::ColumnValue:
        {
            TiXmlElement* pValElem = CreateArithmeticElement("value", column, false);

            if (pValElem)
            {
                pRootElem->LinkEndChild(pValElem);
            }

            break;
        }
        case ColumnType::ColumnSum:
        {
            TiXmlElement* pSumElem = CreateArithmeticElement("sum", column, true);

            if (pSumElem)
            {
                pRootElem->LinkEndChild(pSumElem);
            }

            break;
        }
        case ColumnType::ColumnProduct:
        {
            TiXmlElement* pProductElem = CreateArithmeticElement("product", column, true);

            if (pProductElem)
            {
                pRootElem->LinkEndChild(pProductElem);
            }

            break;
        }
        case ColumnType::ColumnRatio:
        {
            TiXmlElement* pRatioElem = CreateArithmeticElement("ratio", column, true);

            if (pRatioElem)
            {
                pRootElem->LinkEndChild(pRatioElem);
            }

            break;
        }
        case ColumnType::ColumnDifference:
        {
            TiXmlElement* pDiffElem = CreateArithmeticElement("difference", column, true);

            if (pDiffElem)
            {
                pRootElem->LinkEndChild(pDiffElem);
            }

            break;
        }
        default:
        {
            //<!-- Invalid column type -->"
            TiXmlComment* pText = new TiXmlComment(" Invalid column type ");
            pRootElem->LinkEndChild(pText);
            break;
        }
        }
    }

    return true;
}

bool ViewConfig::AddColumnsToElement(TiXmlElement* pRootElem)
{
    if (pRootElem)
    {
        for (const auto& column : m_columnList)
        {
            TiXmlElement* pColElem = new TiXmlElement("column");

            if (pColElem)
            {
                pColElem->SetAttribute("title", column.title.data());
                pColElem->SetAttribute("sort", GetSortTypeStr(column.sorting).data());
                AddArithmeticToElement(column, pColElem);

                pRootElem->LinkEndChild(pColElem);
            }
        }
    }

    return true;
}

//
// Write the view configuration element to the XML output stream.
//
bool ViewConfig::WriteConfigFile(const std::string& configFileName, const std::string& /*pathToDtd*/)
{
    // pathToDtd is the path to DTD file. TinyXml doesn't support DTD, hence ignore it.

    TiXmlDocument configDoc;

    bool rc = constructDomTree(&configDoc);

    if (rc)
    {
        rc = configDoc.SaveFile(configFileName.data());
    }

    return rc;
}

bool ViewConfig::constructDomTree(TiXmlDocument* pConfigDoc)
{
    if (pConfigDoc)
    {
        // Add <?xml version="1.0"?>
        TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "", "");
        if (decl)
        {
            pConfigDoc->LinkEndChild(decl);
        }

        // Skip !DOCTYPE element, as it is not supported by TinyXml.
        // <!DOCTYPE view_configuration SYSTEM \"" + pathToDtd + "ViewConfigs/viewconfig.dtd\">

        // Add <view_configuration> to root.
        TiXmlElement* pViewConfigElem = new TiXmlElement("view_configuration");

        if (pViewConfigElem)
        {
            pConfigDoc->LinkEndChild(pViewConfigElem);

            // Add <view> to <view_configuration>.
            TiXmlElement* pViewElem = new TiXmlElement("view");

            if (pViewElem)
            {
                pViewConfigElem->LinkEndChild(pViewElem);

                // Set attribute "name" to <view>.
                pViewElem->SetAttribute("name", m_configName.data());

                // Add <data> to <view>.
                TiXmlElement* pDataElem = new TiXmlElement("data");
                if (pDataElem)
                {
                    AddEventsToElement(pDataElem);
                    pViewElem->LinkEndChild(pDataElem);
                }

                // Add <output> to <view>
                TiXmlElement* pOutputElem = new TiXmlElement("output");
                if (pOutputElem)
                {
                    AddColumnsToElement(pOutputElem);
                    pViewElem->LinkEndChild(pOutputElem);
                }

                // Add <tool_tip> to <view>
                TiXmlElement* pTooltipElem = new TiXmlElement("tool_tip");
                if (pTooltipElem)
                {
                    TiXmlText* pTextElem = new TiXmlText(m_toolTip.data());
                    pTooltipElem->LinkEndChild(pTextElem);
                    pViewElem->LinkEndChild(pTooltipElem);
                }

                // Add <description> to <view>
                TiXmlElement* pDescElem = new TiXmlElement("description");
                if (pDescElem)
                {
                    TiXmlText* pTextElem = new TiXmlText(m_description.data());
                    pDescElem->LinkEndChild(pTextElem);
                    pViewElem->LinkEndChild(pDescElem);
                }
            }
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////
// Read view configuration from XML file
///////////////////////////////////////////////////////////////////////

//
// Set up the XML file for reading, then start the parse. Return false if the
// parser detected an error.
//
bool ViewConfig::ReadConfigFile(const std::string& configFileName)
{
    TiXmlDocument eventFile(configFileName.data());
    bool loadOkay = eventFile.LoadFile();

    if (loadOkay)
    {
        TiXmlElement* rootElement = eventFile.RootElement();
        loadOkay = parseDomTree(rootElement);
    }

    return loadOkay;
}

///////////////////////////////////////////////////////////////////////
// Function for parsing the DOM tree of view configuration in XML
///////////////////////////////////////////////////////////////////////

bool ViewConfig::parseDomTree(TiXmlElement* pRoot)
{
    bool rc = false;

    // Root element: <view_configuration>
    if (pRoot)
    {
        TiXmlElement* pViewElement = pRoot->FirstChildElement("view");

        if (pViewElement)
        {
            const char* attrStr = pViewElement->Attribute("name");
            if (attrStr)
            {
                m_configName = attrStr;
            }

            TiXmlElement* pDataElement = pViewElement->FirstChildElement("data");

            if (pDataElement)
            {
                TiXmlElement* pEventElement = pDataElement->FirstChildElement("event");

                while (pEventElement)
                {
                    DataAssoc da;

                    attrStr = pEventElement->Attribute("id");
                    if (attrStr)
                    {
                        da.eventId = attrStr;
                    }

                    attrStr = pEventElement->Attribute("select");
                    if (attrStr)
                    {
                        da.eventConfig.eventSelect = static_cast<uint16_t>(strtol(attrStr, nullptr, 16));
                    }

                    attrStr = pEventElement->Attribute("mask");
                    if (attrStr)
                    {
                        da.eventConfig.eventUnitMask = static_cast<uint8_t>(strtol(attrStr, nullptr, 16));
                    }

                    attrStr = pEventElement->Attribute("Os");
                    if (attrStr)
                    {
                        da.eventConfig.bitOs = (attrStr[0] == 'T') ? true : false;
                    }

                    attrStr = pEventElement->Attribute("Usr");
                    if (attrStr)
                    {
                        da.eventConfig.bitUsr = (attrStr[0] == 'T') ? true : false;
                    }

                    // Insert event into m_assocList
                    m_assocList.push_back(da);

                    pEventElement = pEventElement->NextSiblingElement("event");
                }
            }

            TiXmlElement* pOutputElement = pViewElement->FirstChildElement("output");

            if (pOutputElement)
            {
                TiXmlElement* pColElement = pOutputElement->FirstChildElement("column");

                while (pColElement)
                {
                    ColumnSpec cs;

                    attrStr = pColElement->Attribute("title");
                    if (attrStr)
                    {
                        cs.title = attrStr;
                    }

                    attrStr = pColElement->Attribute("sort");
                    if (attrStr)
                    {
                        cs.sorting = GetSortType(attrStr);
                    }

                    TiXmlElement* pElement = pColElement->FirstChildElement();
                    if (pElement)
                    {
                        // Process pElement
                        ParseColumnSpecElement(pElement, cs);

                        // Insert column into m_columnList
                        m_columnList.push_back(cs);
                    }

                    pColElement = pColElement->NextSiblingElement("column");
                }
            }

            TiXmlElement* pTooltipElem = pViewElement->FirstChildElement("tool_tip");
            if (pTooltipElem)
            {
                attrStr = pTooltipElem->GetText();
                if (attrStr)
                {
                    m_toolTip = attrStr;
                }
            }

            TiXmlElement* pDescElem = pViewElement->FirstChildElement("description");
            if (pDescElem)
            {
                attrStr = pDescElem->GetText();
                if (attrStr)
                {
                    m_description = attrStr;
                }
            }
        }
    }

    if (m_assocList.size() > 0 && m_columnList.size() > 0)
    {
        rc = true;
    }

    return rc;
}

bool ViewConfig::ParseColumnSpecElement(TiXmlElement* const pElement, ColumnSpec& cs)
{
    if (pElement)
    {
        std::string typeStr = pElement->Value();

        if (typeStr == "value")
        {
            cs.type = ColumnType::ColumnValue;
            cs.dataSelectLeft = GetConfigById(pElement->Attribute("id"));
        }
        else if (typeStr == "sum")
        {
            cs.type = ColumnType::ColumnSum;
            cs.dataSelectLeft = GetConfigById(pElement->Attribute("left"));
            cs.dataSelectRight = GetConfigById(pElement->Attribute("right"));
        }
        else if (typeStr == "difference")
        {
            cs.type = ColumnType::ColumnDifference;
            cs.dataSelectLeft = GetConfigById(pElement->Attribute("left"));
            cs.dataSelectRight = GetConfigById(pElement->Attribute("right"));
        }
        else if (typeStr == "product")
        {
            cs.type = ColumnType::ColumnDifference;
            cs.dataSelectLeft = GetConfigById(pElement->Attribute("left"));
            cs.dataSelectRight = GetConfigById(pElement->Attribute("right"));
        }
        else if (typeStr == "ratio")
        {
            cs.type = ColumnType::ColumnRatio;
            cs.dataSelectLeft = GetConfigById(pElement->Attribute("left"));
            cs.dataSelectRight = GetConfigById(pElement->Attribute("right"));
        }
        else
        {
            cs.type = ColumnType::ColumnInvalid;
        }
    }

    return true;
}
