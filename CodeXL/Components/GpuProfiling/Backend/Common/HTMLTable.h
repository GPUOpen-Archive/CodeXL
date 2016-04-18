//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief HTML table generation classes
//==============================================================================

#ifndef _HTML_TABLE_H_
#define _HTML_TABLE_H_

#include <string>
#include <vector>

// predefine
class HTMLTable;

//------------------------------------------------------------------------------------
/// HTML Table Row
//------------------------------------------------------------------------------------
class HTMLTableRow
{
public:
    /// Constructor
    /// \param pParent HTMLTable object
    HTMLTableRow(HTMLTable* pParent);

    /// Destructor
    ~HTMLTableRow();

    /// Add Item
    /// \param idx Index
    /// \param strVal string value
    /// \return ref to itself
    HTMLTableRow& AddItem(size_t idx, const std::string& strVal);

    /// Add Item
    /// \param strHeader Column header
    /// \param strVal string value
    /// \return ref to itself
    HTMLTableRow& AddItem(std::string strHeader, const std::string& strVal);

    /// Write to std::ostream
    /// \param ostream output stream
    void WriteToStream(std::ostream& ostream);

    /// Assignement operator
    /// \param obj right operand
    /// \return ref to itself
    const HTMLTableRow& operator= (const HTMLTableRow& obj);

    /// Copy constructor
    /// \param obj object to be copied
    HTMLTableRow(const HTMLTableRow& obj);

    /// Set background color
    /// \param szColor color string
    void SetBgColor(const char* szColor);

protected:
    HTMLTable* m_pParent;                  ///< Html table object
    std::vector< std::string > m_items;    ///< items array
    std::string m_strBgColor;              ///< background color
};

//------------------------------------------------------------------------------------
/// HTML Table Column
//------------------------------------------------------------------------------------
class HTMLTableColumn
{
public:
    std::string m_strName;                 ///< Column name
    bool m_bSortable;                      ///< Flag indicating if column is sortable
    bool m_bReverseNumericSort;            ///< Flag indicating if column contains numeric data that should be sorted in reverse order (largest to smallest) the first time you sort the column

    /// Default constructor
    HTMLTableColumn()
    {
        m_strName.clear();
        m_bSortable = true;
        m_bReverseNumericSort = false;
    }
};

//------------------------------------------------------------------------------------
/// HTML Table
//------------------------------------------------------------------------------------
class HTMLTable
{
    friend class HTMLTableRow;
public:
    /// Constructor
    HTMLTable(void);

    /// Constructor
    /// \param szName name of the table
    HTMLTable(const char* szName);

    /// Destructor
    ~HTMLTable(void);

    /// Add Column
    /// \param szCol Column name string
    /// \param bSortable Flag indicating if this column is sortable
    /// \param bRevNumericSort Flag indicating if this column contains numeric data that should be sorted in reverse order (largest to smallest) the first time you sort the column
    /// \return ref to itself
    HTMLTable& AddColumn(const char* szCol, bool bSortable = true, bool bRevNumericSort = false);

    /// Add Row
    /// \param row Table row
    /// \param bFoot is it a foot row(e.g. Total Row)? Foot row stay at bottom and it's not sorted
    /// \return ref to itself
    HTMLTable& AddRow(const HTMLTableRow& row, bool bFoot = false);

    /// Save table to a string
    /// \return table html string
    std::string WriteToString();

    /// Write to std::stream
    /// \param outputStream output stream
    void WriteToStream(std::ostream& outputStream);

    /// Write script to std::stream
    /// \param outputStream output stream
    static void WriteSortableTableScript(std::ostream& outputStream);

    /// Write table style to std::stream
    /// \param outputStream output stream
    static void WriteTableStyle(std::ostream& outputStream);

    /// Clear table
    void Clear();

protected:
    /// Write table header
    /// \param outputStream output stream
    void WriteHeader(std::ostream& outputStream);

    std::vector< HTMLTableColumn > m_vecCol;           ///< table header array
    std::vector< HTMLTableRow > m_vecRow;              ///< table row array
    std::vector< HTMLTableRow > m_vecFoot;             ///< table foot items array
    bool m_bDraggable;                                 ///< Draggable table, true by default
    bool m_bSortable;                                  ///< Sortable table, true by default
    std::string m_strName;                             ///< Name of the table
private:
    /// Copy constructor
    /// \param obj object
    HTMLTable(const HTMLTable& obj);

    /// Assignment operator
    /// \param obj object
    /// \return ref to itself
    const HTMLTable& operator = (const HTMLTable& obj);
};

#endif //_HTML_TABLE_H_
