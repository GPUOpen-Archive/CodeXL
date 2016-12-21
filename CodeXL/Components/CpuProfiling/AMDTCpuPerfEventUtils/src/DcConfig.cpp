//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file DcConfig.cpp
/// \brief Data collection configuration reader/writer.
///
//==================================================================================

#include <sstream>

#include <DcConfig.h>
#include <tinyxml.h>


////////////////////////////////////////////////////////////////////////////////////
// CdConfig class definition
////////////////////////////////////////////////////////////////////////////////////

// Constants for initializing private data members
#define DefaultConfigName      "No name"
#define DefaultInterval        0.0
#define DefaultMaxCount        0


static std::string toHexString(uint64_t num)
{
    std::stringstream ss;
    ss << std::hex << num << std::dec;
    return ss.str();
}

//
// Simple class constructor
//
DcConfig::DcConfig()
{
    m_configType = DCConfigInvalid;
    m_configName = DefaultConfigName;
    m_tbpInterval = DefaultInterval;
}

//
// Class copy constructor
//
DcConfig::DcConfig(const DcConfig& original)
{
    m_configType = original.m_configType;
    m_configName = original.m_configName;
    m_toolTip = original.m_toolTip;
    m_description = original.m_description;
    m_tbpInterval = original.m_tbpInterval;
    m_ebpConfigList = original.m_ebpConfigList;
    m_ibsConfig = original.m_ibsConfig;
    m_cluConfig = original.m_cluConfig;
}

//
// Class assignment operator
//
const DcConfig& DcConfig::operator=(const DcConfig& rhs)
{
    if (this != &rhs)
    {
        m_configType = rhs.m_configType;
        m_configName = rhs.m_configName;
        m_toolTip = rhs.m_toolTip;
        m_description = rhs.m_description;
        m_tbpInterval = rhs.m_tbpInterval;
        m_ebpConfigList = rhs.m_ebpConfigList;
        m_ibsConfig = rhs.m_ibsConfig;
        m_cluConfig = rhs.m_cluConfig;
    }

    return (*this);
}

//
// Write the data configuration element to the XML output stream. Call
// the appropriate configuration type-specific private function to actually
// perform the writing.
//
bool DcConfig::WriteConfigFile(const std::string& configFileName, const std::string& /*pathToDtd*/)
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

bool DcConfig::constructDomTree(TiXmlDocument* pConfigDoc)
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
        // <!DOCTYPE dc_configuration SYSTEM \"" + pathToDtd +/dcconfig.dtd\">

        // Add <dc_configuration> to root.
        TiXmlElement* pDcConfigElem = new TiXmlElement("dc_configuration");

        if (pDcConfigElem)
        {
            pConfigDoc->LinkEndChild(pDcConfigElem);

            // Add <tbp> to <dc_configuration>
            if ((DCConfigTBP == m_configType) || (DCConfigMultiple == m_configType))
            {
                TiXmlElement* pConfigElem = new TiXmlElement("tbp");

                if (pConfigElem)
                {
                    pConfigElem->SetAttribute("name", m_configName.data());

                    std::string attrStr = std::to_string(m_tbpInterval);
                    pConfigElem->SetAttribute("interval", attrStr.data());

                    TiXmlText* pTooltip = new TiXmlText(m_toolTip.data());
                    if (pTooltip)
                    {
                        pConfigElem->LinkEndChild(pTooltip);
                    }

                    TiXmlText* pDesc = new TiXmlText(m_description.data());
                    if (pDesc)
                    {
                        pConfigElem->LinkEndChild(pDesc);
                    }

                    pDcConfigElem->LinkEndChild(pConfigElem);
                }
            }

            // Add <ebp> to <dc_configuration>
            if ((DCConfigEBP == m_configType) || (DCConfigMultiple == m_configType))
            {
                TiXmlElement* pConfigElem = new TiXmlElement("ebp");
                if (pConfigElem)
                {
                    pConfigElem->SetAttribute("name", m_configName.data());

                    for (const auto& ebpConfig : m_ebpConfigList)
                    {
                        TiXmlElement* pEventElem = new TiXmlElement("event");
                        if (pEventElem)
                        {
                            uint32_t eventSelect = ebpConfig.pmc.ucEventSelectHigh;
                            eventSelect = (eventSelect << 8) | ebpConfig.pmc.ucEventSelect;

                            std::string attrStr = toHexString(eventSelect);
                            pEventElem->SetAttribute("select", attrStr.data());

                            attrStr = toHexString(ebpConfig.pmc.ucUnitMask);
                            pEventElem->SetAttribute("mask", attrStr.data());

                            attrStr = ebpConfig.pmc.bitOsEvents ? "T" : "F";
                            pEventElem->SetAttribute("os", attrStr.data());

                            attrStr = ebpConfig.pmc.bitUsrEvents ? "T" : "F";
                            pEventElem->SetAttribute("user", attrStr.data());

                            attrStr = ebpConfig.pmc.bitEdgeEvents ? "T" : "F";
                            pEventElem->SetAttribute("edge_detect", attrStr.data());

                            attrStr = ebpConfig.pmc.hostOnly ? "T" : "F";
                            pEventElem->SetAttribute("host", attrStr.data());

                            attrStr = ebpConfig.pmc.guestOnly ? "T" : "F";
                            pEventElem->SetAttribute("guest", attrStr.data());

                            attrStr = toHexString(ebpConfig.pmc.FakeL2IMask);
                            pEventElem->SetAttribute("l2i_mask", attrStr.data());

                            attrStr = std::to_string(ebpConfig.eventCount);
                            pEventElem->SetAttribute("count", attrStr.data());

                            pConfigElem->LinkEndChild(pEventElem);
                        }
                    }

                    TiXmlText* pTooltip = new TiXmlText(m_toolTip.data());
                    if (pTooltip)
                    {
                        pConfigElem->LinkEndChild(pTooltip);
                    }

                    TiXmlText* pDesc = new TiXmlText(m_description.data());
                    if (pDesc)
                    {
                        pConfigElem->LinkEndChild(pDesc);
                    }

                    pDcConfigElem->LinkEndChild(pConfigElem);
                }
            }

            // Add <ibs> to <dc_configuration>
            if ((DCConfigIBS == m_configType) || (DCConfigMultiple == m_configType))
            {
                TiXmlElement* pConfigElem = new TiXmlElement("ibs");
                if (pConfigElem)
                {
                    pConfigElem->SetAttribute("name", m_configName.data());

                    pConfigElem->SetAttribute("fetch_sampling", m_ibsConfig.fetchSampling ? "T" : "F");
                    pConfigElem->SetAttribute("op_sampling", m_ibsConfig.opSampling ? "T" : "F");
                    pConfigElem->SetAttribute("op_cycle_count", m_ibsConfig.opCycleCount ? "T" : "F");

                    std::string attrStr = std::to_string(m_ibsConfig.fetchMaxCount);
                    pConfigElem->SetAttribute("fetch_max_count", attrStr.data());

                    attrStr = std::to_string(m_ibsConfig.opMaxCount);
                    pConfigElem->SetAttribute("op_max_count", attrStr.data());

                    TiXmlText* pTooltip = new TiXmlText(m_toolTip.data());
                    if (pTooltip)
                    {
                        pConfigElem->LinkEndChild(pTooltip);
                    }

                    TiXmlText* pDesc = new TiXmlText(m_description.data());
                    if (pDesc)
                    {
                        pConfigElem->LinkEndChild(pDesc);
                    }

                    pDcConfigElem->LinkEndChild(pConfigElem);
                }
            }

            // Add <clu> to <dc_configuration>
            if ((DCConfigCLU == m_configType) || (DCConfigMultiple == m_configType))
            {
                TiXmlElement* pConfigElem = new TiXmlElement("clu");
                if (pConfigElem)
                {
                    pConfigElem->SetAttribute("name", m_configName.data());

                    pConfigElem->SetAttribute("clu_sampling", m_cluConfig.cluSampling ? "T" : "F");
                    pConfigElem->SetAttribute("clu_cycle_count", m_cluConfig.cluCycleCount ? "T" : "F");

                    std::string attrStr = std::to_string(m_cluConfig.cluMaxCount);
                    pConfigElem->SetAttribute("clu_max_count", attrStr.data());

                    TiXmlText* pTooltip = new TiXmlText(m_toolTip.data());
                    if (pTooltip)
                    {
                        pConfigElem->LinkEndChild(pTooltip);
                    }

                    TiXmlText* pDesc = new TiXmlText(m_description.data());
                    if (pDesc)
                    {
                        pConfigElem->LinkEndChild(pDesc);
                    }

                    pDcConfigElem->LinkEndChild(pConfigElem);
                }
            }
        }
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////////
// Read data collection configuration from XML file
////////////////////////////////////////////////////////////////////////////////////

//
// Set up the XML file for reading, then start the parse. The XML parse
// proceeds and calls the callback functions below. Return false if the
// parser detected an error.
//
bool DcConfig::ReadConfigFile(const std::string& configFileName)
{
    TiXmlDocument dcConfigFile(configFileName.data());
    bool loadOkay = dcConfigFile.LoadFile();

    if (loadOkay)
    {
        TiXmlElement* rootElement = dcConfigFile.RootElement();
        loadOkay = parseDomTree(rootElement);
    }

    return loadOkay;
}

////////////////////////////////////////////////////////////////////////////////////
// Function for reading a data collection configuration in XML
////////////////////////////////////////////////////////////////////////////////////

bool DcConfig::parseDomTree(TiXmlElement* pRoot)
{
    bool rc = true;

    // Root element: <dc_configuration>
    if (pRoot)
    {
        TiXmlElement* pConfigElem = pRoot->FirstChildElement();

        while (pConfigElem)
        {
            const char* attrStr = pConfigElem->Attribute("name");
            if (attrStr)
            {
                m_configName = attrStr;
            }

            // One of type: <tbp | ebp | ibs | clu>
            const char* pName = pConfigElem->Value();

            if (pName)
            {
                std::string elemName = pName;

                if (elemName == "tbp")
                {
                    m_configType = (m_configType == DCConfigInvalid) ? DCConfigTBP : DCConfigMultiple;

                    attrStr = pConfigElem->Attribute("interval");
                    if (attrStr)
                    {
                        m_tbpInterval = strtof(attrStr, nullptr);
                    }
                }
                else if (elemName == "ebp")
                {
                    m_configType = (m_configType == DCConfigInvalid) ? DCConfigEBP : DCConfigMultiple;

                    TiXmlElement* pEventElem = pConfigElem->FirstChildElement("event");

                    while (pEventElem)
                    {
                        // Process event element attributes
                        DcEventConfig ebpConfig;
                        uint64_t eventSelect = 0;

                        ebpConfig.pmc.bitEnabled = 1U;

                        attrStr = pEventElem->Attribute("select");
                        if (attrStr)
                        {
                            eventSelect = strtoll(attrStr, nullptr, 16);
                            ebpConfig.pmc.ucEventSelect = eventSelect & 0xFF;
                            ebpConfig.pmc.ucEventSelectHigh = (eventSelect >> 8) & 0xF;
                        }

                        attrStr = pEventElem->Attribute("mask");
                        if (attrStr)
                        {
                            auto mask = strtoll(attrStr, nullptr, 16);
                            ebpConfig.pmc.ucUnitMask = static_cast<uint8_t>(mask);
                        }

                        attrStr = pEventElem->Attribute("os");
                        if (attrStr)
                        {
                            ebpConfig.pmc.bitOsEvents = (attrStr[0] == 'T') ? 1U : 0U;
                        }

                        attrStr = pEventElem->Attribute("user");
                        if (attrStr)
                        {
                            ebpConfig.pmc.bitUsrEvents = (attrStr[0] == 'T') ? 1U : 0U;
                        }

                        attrStr = pEventElem->Attribute("edge_detect");
                        if (attrStr)
                        {
                            ebpConfig.pmc.bitEdgeEvents = (attrStr[0] == 'T') ? 1U : 0U;
                        }

                        attrStr = pEventElem->Attribute("host");
                        if (attrStr)
                        {
                            ebpConfig.pmc.hostOnly = (attrStr[0] == 'T') ? 1U : 0U;
                        }

                        attrStr = pEventElem->Attribute("guest");
                        if (attrStr)
                        {
                            ebpConfig.pmc.guestOnly = (attrStr[0] == 'T') ? 1U : 0U;
                        }

                        if ((eventSelect & (FAKE_L2I_EVENT_ID_PREFIX << 12)) == (FAKE_L2I_EVENT_ID_PREFIX << 12))
                        {
                            // L2I event
                            ebpConfig.pmc.FakeL2IMask = FAKE_L2I_MASK_VALUE;
                            ebpConfig.pmc.bitOsEvents = 0U;
                            ebpConfig.pmc.bitUsrEvents = 0U;
                        }

                        attrStr = pEventElem->Attribute("l2i_mask");
                        if (attrStr)
                        {
                            ebpConfig.pmc.FakeL2IMask = strtoll(attrStr, nullptr, 16);
                        }

                        attrStr = pEventElem->Attribute("count");
                        if (attrStr)
                        {
                            ebpConfig.eventCount = strtoll(attrStr, nullptr, 10);
                        }

                        ebpConfig.pmc.bitSampleEvents = (ebpConfig.eventCount > 0) ? 1U : 0U;

                        m_ebpConfigList.push_back(ebpConfig);

                        pEventElem = pEventElem->NextSiblingElement("event");
                    }
                }
                else if (elemName == "ibs")
                {
                    m_configType = (m_configType == DCConfigInvalid) ? DCConfigIBS : DCConfigMultiple;

                    attrStr = pConfigElem->Attribute("fetch_sampling");
                    if (attrStr)
                    {
                        m_ibsConfig.fetchSampling = (attrStr[0] == 'T') ? true : false;
                    }

                    attrStr = pConfigElem->Attribute("op_sampling");
                    if (attrStr)
                    {
                        m_ibsConfig.opSampling = (attrStr[0] == 'T') ? true : false;
                    }

                    attrStr = pConfigElem->Attribute("op_cycle_count");
                    if (attrStr)
                    {
                        m_ibsConfig.opCycleCount = (attrStr[0] == 'T') ? true : false;
                    }

                    attrStr = pConfigElem->Attribute("fetch_max_count");
                    if (attrStr)
                    {
                        m_ibsConfig.fetchMaxCount = strtoll(attrStr, nullptr, 10);
                    }

                    attrStr = pConfigElem->Attribute("op_max_count");
                    if (attrStr)
                    {
                        m_ibsConfig.opMaxCount = strtoll(attrStr, nullptr, 10);
                    }
                }
                else if (elemName == "clu")
                {
                    m_configType = (m_configType == DCConfigInvalid) ? DCConfigCLU : DCConfigMultiple;

                    attrStr = pConfigElem->Attribute("clu_sampling");
                    if (attrStr)
                    {
                        m_cluConfig.cluSampling = (attrStr[0] == 'T') ? true : false;
                    }

                    attrStr = pConfigElem->Attribute("clu_cycle_count");
                    if (attrStr)
                    {
                        m_cluConfig.cluCycleCount = (attrStr[0] == 'T') ? true : false;
                    }

                    attrStr = pConfigElem->Attribute("clu_max_count");
                    if (attrStr)
                    {
                        m_cluConfig.cluMaxCount = strtoll(attrStr, nullptr, 10);
                    }
                }
                else
                {
                    m_configType = DCConfigInvalid;
                }
            }

            TiXmlHandle configElemHandle(pConfigElem);

            const char* str = configElemHandle.FirstChild("tool_tip").ToElement()->GetText();
            if (str)
            {
                m_toolTip = str;
            }

            str = configElemHandle.FirstChild("description").ToElement()->GetText();
            if (str)
            {
                m_description = str;
            }

            pConfigElem = pConfigElem->NextSiblingElement();
        }
    }

    return rc;
}
