//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This class parses the csv file.
//==============================================================================

#ifndef _CSV_FILE_PARSER_H_
#define _CSV_FILE_PARSER_H_

#include <vector>
#include <map>
#include <set>
#include <string>
#include "BaseParser.h"

//------------------------------------------------------------------------------------
/// CSV Row class
//------------------------------------------------------------------------------------
class CSVRow
{
public:
    /// Constructor
    /// \param header CSV column header
    CSVRow(const std::vector<std::string>& header)
        : m_header(header)
    {
        for (std::vector<std::string>::const_iterator it = header.begin(); it != header.end(); ++it)
        {
            m_data.insert(std::pair<std::string, std::string>(*it, std::string("")));
        }
    }

    /// Query row data from column name
    /// \param strColumn column name
    /// \return row data, if not available, return empty string
    std::string GetRowData(const std::string& strColumn) const
    {
        std::map<std::string, std::string>::const_iterator it = m_data.find(strColumn);

        if (it != m_data.end())
        {
            return it->second;
        }
        else
        {
            return std::string("");
        }
    }

    /// Set row data based on column name
    /// \param strColumn column name
    /// \param val value to be set
    void SetRowData(const std::string& strColumn, const std::string& val)
    {
        if (m_data.find(strColumn) != m_data.end())
        {
            m_data[strColumn] = val;
        }
    }

    /// Fill row data with raw input
    bool FromRowData(const std::vector<std::string>& row)
    {
        if (row.size() > m_header.size())
        {
            // more items than column count
            return false;
        }
        else
        {
            // it's fine to have incomplete row data
            for (size_t i = 0; i < row.size(); ++i)
            {
                m_data[m_header[i]] = row[i];
            }

            return true;
        }
    }

    /// Convenient accessor
    /// \param strCol column name
    /// \return row data, if not available, return empty string
    std::string& operator[](const std::string& strCol)
    {
        return m_data[strCol];
    }

    /// Get columns
    const std::vector<std::string>& GetColumns() const
    {
        return m_header;
    }

protected:
    std::map<std::string, std::string> m_data;   ///< data map
    const std::vector<std::string>& m_header;    ///< header/column

private:
    /// Copy constructor
    /// \param obj object
    CSVRow(const CSVRow& obj);

    /// Assignment operator
    /// \param obj object
    /// \return ref to itself
    const CSVRow& operator = (const CSVRow& obj);
};

//------------------------------------------------------------------------------------
/// CSV File base class
//------------------------------------------------------------------------------------
class CSVFileBase
{
public:
    /// Get columns
    /// \return list of column items
    const std::vector<std::string>& GetColumns() const { return m_columns; }

    /// Get Headers
    /// \return list of header items
    const std::vector<std::string>& GetHeaders() const { return m_headers; }

    /// Set list separator
    /// \param ch List separator
    void SetListSeparator(char ch) { m_cSeparator = ch; }
protected:
    /// Constructor
    CSVFileBase();

    /// Destructor
    virtual ~CSVFileBase();

    char m_cSeparator;                        ///< CSV file separator, default to ,
    char m_cCommentOpen;                      ///< Common open charactor
    std::vector<std::string> m_columns;       ///< Column items
    std::vector< CSVRow* > m_rows;            ///< data
    std::vector<std::string> m_headers;       ///< Header items
};

//------------------------------------------------------------------------------------
/// CSV file parser class
//------------------------------------------------------------------------------------
class CSVFileParser :
    public BaseFileParser<CSVRow>, public CSVFileBase
{
public:
    /// Constructor
    CSVFileParser(void);

    /// Destructor
    ~CSVFileParser(void);

    /// Parse input file
    /// return true if succeeded
    bool Parse();

private:
    /// Copy constructor
    /// \param obj object
    CSVFileParser(const CSVFileParser& obj);

    /// Assignment operator
    /// \param obj object
    /// \return ref to itself
    const CSVFileParser& operator = (const CSVFileParser& obj);
};

//------------------------------------------------------------------------------------
/// CSV file writter class
//------------------------------------------------------------------------------------
class CSVFileWriter : public CSVFileBase
{
public:
    /// Constructor
    /// \param fileName Output file name
    CSVFileWriter(const std::string& fileName);

    /// Destructor
    ~CSVFileWriter();

    /// Write to file
    /// return true if succeeded
    bool Flush();

    /// Close writer, reset states
    bool Close();

    /// Add column
    /// \param colName Column name
    void AddColumn(const std::string& colName);

    /// Add column name array
    /// \param colNames Column names
    void AddColumns(const std::vector<std::string>& colNames);

    /// Add header
    /// Header is output with m_cCommentOpen as the first character
    /// e.g. #Version=1.2.3
    /// \param strHeader the header string to add
    void AddHeader(const std::string& strHeader);

    /// Add row
    /// \return row pointer
    CSVRow* AddRow();
private:
    std::string m_strFilename;          ///< Output file name
    bool m_bWrittenHeaderAndColumnRow;  ///< A flag indicating whether or not header and column has been written
    std::set<std::string> m_columnSet;  ///< Used internally to enable quick lookup
};

#endif //_CSV_FILE_PARSER_H_
