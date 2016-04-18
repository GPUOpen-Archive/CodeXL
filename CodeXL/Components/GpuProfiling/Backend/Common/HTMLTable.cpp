//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief HTML table generation classes
//==============================================================================

#include <sstream>
#include "HTMLTable.h"
#include "Logger.h"
#include "JScript.h"

using std::stringstream;
using std::string;
using std::vector;

using namespace GPULogger;

HTMLTableRow::HTMLTableRow(HTMLTable* pParent) : m_pParent(pParent), m_strBgColor("#FFFFFF") {}

HTMLTableRow::~HTMLTableRow()
{
    m_items.clear();
}

void HTMLTableRow::SetBgColor(const char* szColor)
{
    m_strBgColor = szColor;
}

HTMLTableRow& HTMLTableRow::AddItem(size_t idx, const std::string& strVal)
{
    SpAssert(idx < m_pParent->m_vecCol.size());

    if (idx >= m_pParent->m_vecCol.size())
    {
        return *this;
    }

    while (m_items.size() <= idx)
    {
        m_items.push_back("");
    }

    m_items[ idx ] = strVal;
    return *this;
}

HTMLTableRow& HTMLTableRow::AddItem(std::string strHeader, const std::string& strVal)
{
    size_t idx = 0;

    for (vector< HTMLTableColumn >::iterator it = m_pParent->m_vecCol.begin(); it != m_pParent->m_vecCol.end(); it++)
    {
        if (strHeader == it->m_strName)
        {
            AddItem(idx, strVal);
        }

        idx++;
    }

    return *this;
}

void HTMLTableRow::WriteToStream(std::ostream& ostream)
{
    ostream << "<tr bgcolor = \"";
    ostream << m_strBgColor << "\"";
    ostream << ">";

    for (vector< std::string >::iterator it = m_items.begin(); it != m_items.end(); it++)
    {
        ostream << "<td>";
        ostream << *it;
        ostream << "</td>\t";
    }

    ostream << "</tr>";
}

const HTMLTableRow& HTMLTableRow::operator= (const HTMLTableRow& obj)
{
    if (this != &obj)
    {
        m_pParent = obj.m_pParent;
        m_strBgColor = obj.m_strBgColor;
        m_items.clear();

        for (size_t i = 0; i < obj.m_items.size(); i++)
        {
            m_items.push_back(obj.m_items[i]);
        }
    }

    return *this;
}

HTMLTableRow::HTMLTableRow(const HTMLTableRow& obj)
{
    m_pParent = obj.m_pParent;
    m_strBgColor = obj.m_strBgColor;
    m_items.clear();

    for (size_t i = 0; i < obj.m_items.size(); i++)
    {
        m_items.push_back(obj.m_items[i]);
    }
}

HTMLTable::HTMLTable(void)
{
    m_bDraggable = true;
    m_bSortable = true;
    m_strName = "Unnamed table";
}

HTMLTable::HTMLTable(const char* szName)
{
    m_bDraggable = true;
    m_bSortable = true;
    m_strName = szName;
}

HTMLTable::~HTMLTable(void)
{
    m_vecCol.clear();
    m_vecRow.clear();
}

void HTMLTable::WriteHeader(std::ostream& outputStream)
{
    outputStream << "<tr>\n\t";

    for (vector< HTMLTableColumn >::iterator it = m_vecCol.begin(); it != m_vecCol.end(); it++)
    {
        if (!it->m_bSortable)
        {
            outputStream << "<th class=\"sorttable_nosort\" >";
        }
        else if (it->m_bReverseNumericSort)
        {
            outputStream << "<th class=\"sorttable_revnumeric\" >";
        }
        else
        {
            outputStream << "<th>";
        }

        outputStream << (*it).m_strName;
        outputStream << "</th>\t";
    }

    outputStream << "\n</tr>\n";
}

void HTMLTable::WriteSortableTableScript(std::ostream& outputStream)
{
    outputStream << "<script type=\"text/javascript\">\n";
    outputStream << "<!--\n";

    outputStream << g_szDragTableJS;
    outputStream << "//-->\n";
    outputStream << "</script>\n";

    outputStream << "<script type=\"text/javascript\">\n";
    outputStream << "<!--\n";

    outputStream << g_szSortedTableJS;
    outputStream << "//-->\n";
    outputStream << "</script>\n";
}

void HTMLTable::WriteTableStyle(std::ostream& outputStream)
{
    const char* style =
    {
        "<style type=\"text/css\">\n"
        "th {\n"
        "font-size:1em;\n"
        "text-align:left;\n"
        "padding-top:5px;\n"
        "padding-bottom:4px;\n"
        "background-color:#969696;\n"
        "color:#fff;\n"

        "}\n"

        "td, th {\n"
        "font-size:0.75em;\n"
        "border:1px solid #969696;\n"
        "padding:3px 7px 2px 7px;\n"
        "}\n"
        "table {\n"
        "     font-family:\"Trebuchet MS\", Arial, Helvetica, sans-serif;\n"
        "     width:100%;\n"
        "     border-collapse:collapse;\n"
        "}\n"
        "</style>\n"
    };

    outputStream << style;
}

void HTMLTable::WriteToStream(std::ostream& outputStream)
{
    if (m_bDraggable && m_bSortable)
    {
        outputStream << "<table class=\"draggable sortable\">\n";
    }
    else if (m_bSortable)
    {
        outputStream << "<table class=\"sortable\">\n";
    }
    else if (m_bDraggable)
    {
        outputStream << "<table class=\"draggable\">\n";
    }
    else
    {
        outputStream << "<table>\n";
    }

    WriteHeader(outputStream);

    for (std::vector< HTMLTableRow >::iterator it = m_vecRow.begin(); it != m_vecRow.end(); it++)
    {
        outputStream << "\t";
        (*it).WriteToStream(outputStream);
        outputStream << "\n";
    }

    // Write foot row if any
    if (m_vecFoot.size() > 0)
    {
        outputStream << "<tfoot>\n";

        for (std::vector< HTMLTableRow >::iterator it = m_vecFoot.begin(); it != m_vecFoot.end(); it++)
        {
            outputStream << "\t";
            (*it).WriteToStream(outputStream);
            outputStream << "\n";
        }

        outputStream << "</tfoot>\n";
    }

    outputStream << "</table>\n";
}

std::string HTMLTable::WriteToString()
{
    stringstream ss;
    WriteToStream(ss);
    return ss.str();
}

void HTMLTable::Clear()
{
    m_vecRow.clear();
    m_vecCol.clear();
}


HTMLTable& HTMLTable::AddRow(const HTMLTableRow& row, bool bFoot)
{
    if (!bFoot)
    {
        m_vecRow.push_back(row);
    }
    else
    {
        m_vecFoot.push_back(row);
    }

    return *this;
}

HTMLTable& HTMLTable::AddColumn(const char* strCol, bool bSortable, bool bRevNumericSort)
{
    HTMLTableColumn col;
    col.m_strName = strCol;
    col.m_bSortable = bSortable;
    col.m_bReverseNumericSort = bRevNumericSort;
    m_vecCol.push_back(col);
    return *this;
}
