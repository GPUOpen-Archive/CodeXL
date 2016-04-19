//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfilingOptions.cpp
/// \brief Implementation of the CpuProfilingOptions class
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/src/CpuProfilingOptions.cpp#24 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================

// Qt:
#include <QtCore>
#include <QtWidgets>

// Infar:
#include <AMDTAPIClasses/Include/apBasicParameters.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>

//local
#include <inc/CpuProfilingOptions.h>
#include <inc/StringConstants.h>

const int DEFAULT_MAX_SYM_COLUMN_WIDTH = 250;
#define DIASSEMBLY_INSTRUCTIONS_CHUNK_SIZE_VAR_NAME    "disassemblyInstrcutionsChunckSize"
#define DIASSEMBLY_INSTRUCTIONS_CHUNK_SIZE_VAR_NAME_L L"disassemblyInstrcutionsChunckSize"

// Static members:
CpuProfilingOptions* CpuProfilingOptions::m_pMySingleInstance = nullptr;


CpuProfilingOptions::CpuProfilingOptions()
{
    m_options.maxSymColWidth = DEFAULT_MAX_SYM_COLUMN_WIDTH;

    clear();
}

CpuProfilingOptions::~CpuProfilingOptions()
{
}

CpuProfilingOptions& CpuProfilingOptions::instance()
{
    // If this class single instance was not already created:
    if (nullptr == m_pMySingleInstance)
    {
        // Create it:
        m_pMySingleInstance = new CpuProfilingOptions;
        GT_ASSERT(m_pMySingleInstance);
    }

    return *m_pMySingleInstance;
}

PROFILE_OPTIONS* CpuProfilingOptions::options()
{
    return &m_options;
}

void CpuProfilingOptions::emitSettingsUpdated()
{
    emit settingsUpdated();
}

void CpuProfilingOptions::clear()
{
    m_options.addDebug = false;
    m_options.debugSearchPaths.clear();
    m_options.enableSymServer = false;

    osFilePath defaultSymbolDirPath;
    afGetUserDataFolderPath(defaultSymbolDirPath);
    defaultSymbolDirPath.appendSubDirectory(L"Symbols");
    m_options.symbolDownloadDir = QString::fromWCharArray(defaultSymbolDirPath.asString().asCharArray());

    //Set Microsoft's default symbol server
    m_options.symSrvList = "http://msdl.microsoft.com/download/symbols;";
    m_options.useSymSrvMask = 1L;
}

void CpuProfilingOptions::writeBool(gtString& projectAsXMLString, const gtString& key, const bool value)
{
    gtString val;
    val = value ? L"T" : L"F";
    writeValue(projectAsXMLString, key, val);
}

void CpuProfilingOptions::writeValue(gtString& projectAsXMLString, const gtString& key, const gtString& value)
{
    projectAsXMLString.append(L"<");
    projectAsXMLString.append(key);
    projectAsXMLString.append(L">");
    projectAsXMLString.append(value);
    projectAsXMLString.append(L"</");
    projectAsXMLString.append(key);
    projectAsXMLString.append(L">");
}

// ----------------------------------------------------------------------------------
// Name:        CpuProfilingOptions::getProjectSettingsXML
// Description: Writes the settings to the project file
// Author:  AMD Developer Tools Team
// Date:        5/3/2012
// ----------------------------------------------------------------------------------
bool CpuProfilingOptions::getProjectSettingsXML(gtString& projectAsXMLString)
{
    gtString numVal;
    projectAsXMLString.append(L"<");
    projectAsXMLString.append(CPU_STR_PROJECT_EXTENSION);
    projectAsXMLString.append(L">");

    writeBool(projectAsXMLString, L"addDebug", m_options.addDebug);
    writeValue(projectAsXMLString, L"debugSearchPaths", acQStringToGTString(m_options.debugSearchPaths));
    writeBool(projectAsXMLString, L"enableSymServer", m_options.enableSymServer);
    numVal.makeEmpty();
    numVal.appendFormattedString(L"%d", m_options.maxSymColWidth);
    writeValue(projectAsXMLString, L"maxSymColWidth", numVal);
    writeValue(projectAsXMLString, L"symbolDownloadDir", acQStringToGTString(m_options.symbolDownloadDir));
    writeValue(projectAsXMLString, L"symSrvList", acQStringToGTString(m_options.symSrvList));
    numVal.makeEmpty();
    numVal.appendFormattedString(L"%u", m_options.useSymSrvMask);
    writeValue(projectAsXMLString, L"useSymSrvMask", numVal);

    // Disassembly instructions block size.
    // First convert to string representation.
    gtString disassemblyInstructionsChunkSizeAsStr;
    disassemblyInstructionsChunkSizeAsStr.appendFormattedString(L"%d", m_options.disassemblyInstrcutionsChunkSize);

    // Write the value.
    writeValue(projectAsXMLString, DIASSEMBLY_INSTRUCTIONS_CHUNK_SIZE_VAR_NAME_L, disassemblyInstructionsChunkSizeAsStr);

    projectAsXMLString.append(L"</");
    projectAsXMLString.append(CPU_STR_PROJECT_EXTENSION);
    projectAsXMLString.append(L">");

    return true;
}

// ----------------------------------------------------------------------------------
// Name:        CpuProjectHandler::setProjectSettingsXML
// Description: Reads the properties and profiles from the project file
// Author:  AMD Developer Tools Team
// Date:        5/3/2012
// ----------------------------------------------------------------------------------
bool CpuProfilingOptions::setProjectSettingsXML(const gtString& projectAsXMLString)
{
    QXmlInputSource source;
    source.setData(acGTStringToQString(projectAsXMLString));
    QXmlSimpleReader reader ;
    // Connect this object's handler interface to the XML reader
    reader.setContentHandler(this) ;
    reader.setErrorHandler(this);

    return reader.parse(source);
}


bool CpuProfilingOptions::startDocument()
{
    //Clear settings
    clear();
    m_CpuProfileExtension = false;
    return true;
}

bool CpuProfilingOptions::endDocument()
{
    return true;
}

bool CpuProfilingOptions::startElement(
    const QString& namespaceURI, const QString& localName,
    const QString& qName, const QXmlAttributes& atts)
{
    (void)(namespaceURI); // unused
    (void)(localName); // unused
    (void)(atts); // unused

    if (qName == QString::fromWCharArray(CPU_STR_PROJECT_EXTENSION))
    {
        m_CpuProfileExtension = true;
    }

    return true;
}

bool CpuProfilingOptions::endElement(
    const QString& namespaceURI, const QString& localName,
    const QString& qName)
{
    (void)(namespaceURI); // unused
    (void)(localName); // unused
    bool retVal = true;

    if ((m_CpuProfileExtension) && (qName == "addDebug"))
    {
        m_options.addDebug = (m_xmlContent == "T");
    }
    else if ((m_CpuProfileExtension) && (qName == "debugSearchPaths"))
    {
        m_options.debugSearchPaths = m_xmlContent;
    }
    else if ((m_CpuProfileExtension) && (qName == "enableSymServer"))
    {
        m_options.enableSymServer = (m_xmlContent == "T");
    }
    else if ((m_CpuProfileExtension) && (qName == "maxSymColWidth"))
    {
        m_options.maxSymColWidth = m_xmlContent.toUInt();
    }
    else if ((m_CpuProfileExtension) && (qName == "symbolDownloadDir"))
    {
        m_options.symbolDownloadDir = m_xmlContent;
    }
    else if ((m_CpuProfileExtension) && (qName == "symSrvList"))
    {
        m_options.symSrvList = m_xmlContent;
    }
    else if ((m_CpuProfileExtension) && (qName == "useSymSrvMask"))
    {
        m_options.useSymSrvMask = m_xmlContent.toULong();
    }
    else if ((m_CpuProfileExtension) && (qName == DIASSEMBLY_INSTRUCTIONS_CHUNK_SIZE_VAR_NAME))
    {
        m_options.disassemblyInstrcutionsChunkSize = m_xmlContent.toUInt();
    }
    else if (qName == QString::fromWCharArray(CPU_STR_PROJECT_EXTENSION))
    {
        m_CpuProfileExtension = false;
    }
    else
    {
        retVal = false;
    }

    return retVal;
}

bool CpuProfilingOptions::characters(const QString& ch)
{
    m_xmlContent = ch;
    return true;
}
