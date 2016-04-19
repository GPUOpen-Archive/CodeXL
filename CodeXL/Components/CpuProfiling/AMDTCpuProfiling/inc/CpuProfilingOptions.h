//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CpuProfilingOptions.h
/// \brief  Cpu Profiling options, per user
/// Based on CAOptions from CodeAnalyst
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/inc/CpuProfilingOptions.h#17 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================
#ifndef _CPUPROFILINGOPTIONS_H
#define _CPUPROFILINGOPTIONS_H

// Qt:
#include <QtCore>
#include <QtWidgets>
#include <QXmlDefaultHandler>

//infrastructure
#include <AMDTBaseTools/Include/gtString.h>


struct PROFILE_OPTIONS
{
    bool addDebug;
    gtUInt32 useSymSrvMask;   //Assumes a maximum of 32 symbol servers added
    QString symbolDownloadDir;
    bool enableSymServer;
    QString symSrvList;
    QString debugSearchPaths;
    int maxSymColWidth;
    unsigned int disassemblyInstrcutionsChunkSize;
};

class CpuProfilingOptions: public QObject, public QXmlDefaultHandler
{
    Q_OBJECT
public:
    static CpuProfilingOptions& instance();
    virtual ~CpuProfilingOptions();

    PROFILE_OPTIONS* options();
    void emitSettingsUpdated();
    void clear();

    bool getProjectSettingsXML(gtString& projectAsXMLString);
    bool setProjectSettingsXML(const gtString& projectAsXMLString);

    // Override QXmlDefaultHandler
    bool startDocument() ;
    bool endDocument() ;
    bool startElement(
        const QString& namespaceURI, const QString& localName,
        const QString& qName, const QXmlAttributes& atts) ;
    bool endElement(
        const QString& namespaceURI, const QString& localName,
        const QString& qName) ;
    bool characters(const QString& ch) ;

signals:
    void settingsUpdated();

protected:
    CpuProfilingOptions();

    void writeBool(gtString& projectAsXMLString, const gtString& key, const bool value);
    void writeValue(gtString& projectAsXMLString, const gtString& key, const gtString& value);

    /// The singleton instance
    static CpuProfilingOptions* m_pMySingleInstance;

    PROFILE_OPTIONS m_options;

    bool m_CpuProfileExtension;
    QString m_xmlContent;
};

#endif //_CPUPROFILINGOPTIONS_H
