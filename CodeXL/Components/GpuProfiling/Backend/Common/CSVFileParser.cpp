//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class parses the csv file.
//==============================================================================

#include <iostream>
#include <algorithm>
#include "CSVFileParser.h"
#include "Defs.h"
#include "StringUtils.h"
#include "FileUtils.h"
#include "Logger.h"
#include "AMDTMutex.h"

using namespace std;

static AMDTMutex g_mtx;

CSVFileBase::CSVFileBase()
{
    m_cSeparator = ',';
    m_cCommentOpen = '#';
}

CSVFileBase::~CSVFileBase(void)
{
    for (vector<CSVRow*>::iterator it = m_rows.begin(); it != m_rows.end(); ++it)
    {
        CSVRow* pRow = *it;
        SAFE_DELETE(pRow);
    }

    m_rows.clear();
}

CSVFileParser::CSVFileParser(void)
    : CSVFileBase()
{
    SP_TODO("Get separator from file header.")
}

CSVFileParser::~CSVFileParser(void)
{}

bool CSVFileParser::Parse()
{
    const int BUFSIZE = 2048;

    if (m_bFileOpen)
    {
        char buf[BUFSIZE];
        TRY_READ(fin, buf)
        bool stopParsing = false;

        while (buf[0] == m_cCommentOpen)
        {
            m_headers.push_back(string(buf).substr(1));
            TRY_READ(fin, buf)
        }

        string strSeparator;
        strSeparator = m_cSeparator;
        // Read header
        StringUtils::Split(m_columns, string(buf), strSeparator);

        if (m_columns.empty())
        {
            m_bWarning = true;
            m_strWarningMsg = "Empty column header.";
            return true;
        }

        while (!fin.eof() && !stopParsing)
        {
            // read row
            TRY_READ(fin, buf)

            if (strlen(buf) == 0)
            {
                continue;
            }

            vector<string> rowData;
            string strRow = string(buf);
            StringUtils::Split(rowData, strRow, strSeparator);

            CSVRow* pRow = new(nothrow) CSVRow(m_columns);

            SpAssertRet(pRow != NULL) false;

            bool bRet = pRow->FromRowData(rowData);

            if (!bRet)
            {
                m_bWarning = true;
                m_strWarningMsg = "Failed to parse row: " + strRow;
                return false;
            }

            for (std::vector<IParserListener<CSVRow>*>::iterator it = m_listenerList.begin(); it != m_listenerList.end() && !stopParsing; ++it)
            {
                if (pRow != NULL)
                {
                    (*it)->OnParse(pRow, stopParsing);
                }
            }

            m_rows.push_back(pRow);
        }
    }
    else
    {
        return false;
    }

    return true;
}

CSVFileWriter::CSVFileWriter(const std::string& fileName)
{
    m_strFilename = fileName;
    m_bWrittenHeaderAndColumnRow = false;

    if (FileUtils::FileExist(m_strFilename))
    {
        cout << "Specified output file " << m_strFilename << " already exists. It will be overwritten.\n";
        remove(m_strFilename.c_str());
    }
}

CSVFileWriter::~CSVFileWriter()
{

}

bool CSVFileWriter::Close()
{
    bool bRet = Flush();
    m_bWrittenHeaderAndColumnRow = false;
    return bRet;
}

bool CSVFileWriter::Flush()
{
    AMDTScopeLock lock(&g_mtx);

    ofstream fout;
    fout.open(m_strFilename.c_str(), fstream::out | ios::app);

    if (fout.fail())
    {
        return false;
    }

    if (!m_bWrittenHeaderAndColumnRow)
    {
        // print header
        for (std::vector<std::string>::iterator it = m_headers.begin(); it != m_headers.end(); ++it)
        {
            fout << m_cCommentOpen << *it << endl;
        }

        // print column
        for (size_t i = 0; i < m_columns.size(); ++i)
        {
            fout << m_columns[i];

            if (i != m_columns.size() - 1)
            {
                fout << m_cSeparator << " ";
            }
        }

        fout << endl;
        m_bWrittenHeaderAndColumnRow = true;
    }

    // print rows
    for (std::vector< CSVRow* >::iterator it = m_rows.begin(); it != m_rows.end(); ++it)
    {
        for (size_t i = 0; i < m_columns.size(); ++i)
        {
            CSVRow& row = **it;
            fout << row[m_columns[i]];

            if (i != m_columns.size() - 1)
            {
                fout << m_cSeparator << " ";
            }
        }

        fout << endl;
    }

    for (vector<CSVRow*>::iterator it = m_rows.begin(); it != m_rows.end(); ++it)
    {
        CSVRow* pRow = *it;
        SAFE_DELETE(pRow);
    }

    m_rows.clear();

    fout.close();

    return true;
}

void CSVFileWriter::AddColumn(const std::string& colName)
{
    if (m_columnSet.find(colName) == m_columnSet.end())
    {
        m_columns.push_back(colName);
        m_columnSet.insert(colName);
    }
}

void CSVFileWriter::AddColumns(const std::vector<string>& colNames)
{
    for (vector<string>::const_iterator it = colNames.begin(); it != colNames.end(); ++it)
    {
        AddColumn(*it);
    }
}

void CSVFileWriter::AddHeader(const std::string& strHeader)
{
    m_headers.push_back(strHeader);
}

CSVRow* CSVFileWriter::AddRow()
{
    CSVRow* pNewRow = new(nothrow) CSVRow(m_columns);
    SpAssertRet(pNewRow != NULL) NULL;

    m_rows.push_back(pNewRow);
    return pNewRow;
}
