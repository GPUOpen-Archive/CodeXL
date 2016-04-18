//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file OsvWriter.cpp
///
//==================================================================================

#include <OsvWriter.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

OsvWriter::OsvWriter()
{
    clear();
}


OsvWriter::~OsvWriter()
{
    close();
}

void OsvWriter::clear()
{
    m_path = L"";
    m_pOsvData = NULL;
    m_pCur = NULL;
    m_curLevel = 1;
}

void OsvWriter::close()
{
    if (m_fileStream.isOpened())
    {
        m_fileStream.close();
    }

    clear();
}

bool OsvWriter::open(const gtString& path)
{
    if (path.isEmpty())
    {
        return false;
    }

    // Close and reopen if already open
    if (isOpen())
    {
        close();
    }

    // Windows Note:
    // The profile files are opened with UTF-8 encoding.
    //
    if (!m_fileStream.open(path.asCharArray(), WINDOWS_SWITCH(FMODE_TEXT("w"), FMODE_TEXT("w, ccs=UTF-8"))))
    {
        return false;
    }

    // Set path name
    m_path = path;
    return true;
}

void OsvWriter::appendTopic(OsvDataItem* pData)
{
    if (NULL != pData)
    {
        if (NULL == m_pOsvData)
        {
            m_pOsvData = pData;
        }
        else
        {
            OsvDataItem* pTmp = m_pOsvData;

            while (pTmp->sibling)
            {
                pTmp = pTmp->sibling;
            }

            pTmp->sibling = pData;
        }
    }
}

bool OsvWriter::write(const gtString& path)
{
    bool ret = open(path);

    if (ret)
    {
        // Header
        WriteFormat(L"<?xml version=\"1.0\"?>\n");
        WriteFormat(L"<OSV version=\"%u\" >\n", OSV_VERSION);

        // Body
        WriteData(m_pOsvData);

        // End
        WriteFormat(L"</OSV>");
    }

    return ret;
}

void OsvWriter::WriteOneEntry(OsvDataItem* pData)
{
    for (unsigned int i = 0; i < m_curLevel; i++)
    {
        WriteFormat(L"\t");
    }

    WriteFormat(L"<Entry hdr=\"%ls\" ", pData->header.toStdWString().c_str());

    if (!pData->value.isEmpty())
    {
        // We need to replace the ampersand "&" by "&amp;" since XML reader
        // will not read the value in properly
        int curIndex = 0;

        if (!pData->value.contains("&amp;"))
        {
            while ((curIndex = pData->value.indexOf('&', curIndex)) != -1)
            {
                curIndex++;
                pData->value.insert(curIndex, "amp;");
            }
        }

        WriteFormat(L"val=\"%ls\" " , pData->value.toStdWString().c_str());
    }

    if (!pData->link.isEmpty())
    {
        WriteFormat(L"lnk = \"%ls\" ", pData->link.toStdWString().c_str());
    }
}

void OsvWriter::WriteEndEntry()
{
    m_curLevel--;

    if (m_curLevel == 0)
    {
        return;
    }

    for (unsigned int i = 0; i < m_curLevel; i++)
    {
        WriteFormat(L"\t");
    }

    WriteFormat(L"</Entry>\n");
}

void OsvWriter::WriteData(OsvDataItem* pData)
{
    for (; pData ; pData = pData->sibling)
    {
        WriteOneEntry(pData);

        if (pData->child)
        {
            WriteFormat(L">\n");
            m_curLevel++;
            WriteData(pData->child);
        }
        else
        {
            WriteFormat(L"/>\n");
        }

        if (!pData->sibling)
        {
            break;
        }
    }

    WriteEndEntry();
}

void OsvWriter::WriteFormat(const wchar_t* pFormat, ...)
{
    va_list args;
    va_start(args, pFormat);
    vfwprintf(m_fileStream.getHandler(), pFormat, args);
    va_end(args);
}
