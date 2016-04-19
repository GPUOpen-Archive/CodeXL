//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file OsvReader.h
///
//==================================================================================

#ifndef _OSVREADER_H_
#define _OSVREADER_H_

#include "CpuProfilingRawDataDLLBuild.h"
#include "OsvData.h"
#include <AMDTBaseTools/Include/gtList.h>

#include <qxml.h>
#include <QXmlDefaultHandler>

class CP_RAWDATA_API OsvReader : public QXmlDefaultHandler
{
public:
    OsvReader();
    virtual ~OsvReader();

    bool open(const wchar_t* path);

    OsvDataItem* getFirstEntry() { return m_pOsvData; }

    ///////////////////////////////////////////////////////////////////////
    // Method overrides for QXmlDefaultHandler
    ///////////////////////////////////////////////////////////////////////
    bool startDocument();
    bool startElement(const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes& atts);
    bool endElement(const QString& namespaceURI, const QString& localName, const QString& qName);
    bool characters(const QString& ch) { (void)(ch); return true; }

private:
    // This is the root
    OsvDataItem* m_pOsvData;

    // Temp
    OsvDataItem* m_pCurEntry;
    OsvDataItem* m_pLastEntry;
    gtList<OsvDataItem*> m_levelStack;

};

#endif // _OSVREADER_H_
