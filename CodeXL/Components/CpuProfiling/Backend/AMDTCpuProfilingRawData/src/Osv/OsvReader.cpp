//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file OsvReader.cpp
///
//==================================================================================

#include <OsvReader.h>

OsvReader::OsvReader()
{
    m_pOsvData = NULL;
    m_pCurEntry = NULL;
    m_pLastEntry = NULL;
    m_levelStack.clear();
}


OsvReader::~OsvReader()
{
    if (m_pOsvData)
    {
        delete m_pOsvData;
        m_pOsvData = NULL;
    }
}


bool OsvReader::open(const wchar_t* path)
{
    QFile xmlFile(QString::fromWCharArray(path));
    QXmlInputSource source(&xmlFile);
    QXmlSimpleReader reader;

    /* set our xml handler to do work on the data */
    reader.setContentHandler(this);

    return reader.parse(source);
}

// This method is invoked by the XML handler at the start of the
// XML file. Prepare for parsing.
//
bool OsvReader::startDocument()
{
    if (m_pOsvData)
    {
        delete m_pOsvData;
        m_pOsvData = NULL;
    }

    return true;
}


//
// This method is invoked by the XML handler when the start of an
// element is encountered in the XML file. Dispatch control based
// on the element type.
//
bool OsvReader::startElement(const QString& namespaceURI,
                             const QString& localName,
                             const QString& qName,
                             const QXmlAttributes& atts)
{
    (void)(namespaceURI); // unused
    (void)(localName); // unused

    /* There are three main elements in our xml file.
     * 1) <OSV> -> Begins the document
     * 2) <Entry>   ->  Entry
     */
    if (qName == "Entry")
    {
        if (m_pCurEntry)
        {
            // This is nesting entry.
            if (m_pLastEntry)
            {
                m_pLastEntry->sibling = m_pCurEntry;
            }
            else if (m_levelStack.size() != 0)
            {
                m_levelStack.back()->child = m_pCurEntry;
            }

            // Also add to the stack
            m_levelStack.push_back(m_pCurEntry);
            m_pLastEntry = NULL;
        }

        // Setup for new entry
        m_pCurEntry = new OsvDataItem();

        if (!m_pCurEntry)
        {
            return false;
        }

        m_pCurEntry->header = atts.value("hdr");
        m_pCurEntry->value  = atts.value("val");
        m_pCurEntry->link   = atts.value("lnk");

        if (!m_pOsvData)
        {
            m_pOsvData = m_pCurEntry;
        }
    }

    return true;
}

//
// This method is invoked by the XML handler when the end of an
// element is encountered in the XML file. Dispatch control based
// on the element name.
//
bool OsvReader::endElement(const QString& namespaceURI, const QString& localName, const QString& qName)
{
    (void)(namespaceURI); // unused
    (void)(localName); // unused

    if (qName == "Entry")
    {
        if (m_pCurEntry)
        {
            if (m_pLastEntry)
            {
                m_pLastEntry->sibling = m_pCurEntry;
            }
            else if (m_levelStack.size() != 0)
            {
                m_levelStack.back()->child = m_pCurEntry;
            }

            m_pLastEntry = m_pCurEntry;
            m_pCurEntry = NULL;
        }
        else
        {
            // Go back to last parent
            m_pLastEntry = m_levelStack.back();
            m_levelStack.pop_back();
            m_pCurEntry = NULL;
        }
    }

    return true;
}
