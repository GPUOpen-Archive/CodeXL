//==============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Helpers for profile data CSV File Parsing and merging
//==============================================================================

#ifndef _CSV_FILE_MERGER_H_
#define _CSV_FILE_MERGER_H_

#include "../Common/CSVFileParser.h"

typedef std::vector<CSVRow*> CSVRowList;
typedef std::pair<std::string, unsigned int> MethodFileIndexPair;
typedef std::pair<std::string, MethodFileIndexPair> HeaderPair;
typedef std::vector<HeaderPair> HeaderList;
typedef std::pair<std::string, unsigned int> ThreadFileIndexPair;
typedef std::set<ThreadFileIndexPair> MappedThreadSet;
typedef std::vector<MappedThreadSet> MappedThreadSetList;

class KernelRowData :
    public IParserListener<CSVRow>
{

public:

    /// Returns the numbers of rows in the profile data CSV file
    /// \return number of rows
    unsigned int GetRowCount();

    /// Returns the number of entries in CSV file of the profile data for the thread
    /// \param[in] threadId thread ID
    /// \return number of rows
    unsigned int GetRowCountByThreadId(const std::string& threadId);

    /// Returns the list of unique threads in the profile data CSV file
    /// \return list of unique threads
    std::vector<std::string> GetUniqueThreads();

    /// Returns the value of the column in a row entry for the thread
    /// \param[in] threadId thread name
    /// \param[in] rowIndex index of the row entry in the file
    /// \param[in] columnName name of the column
    /// \return value of column in a row entry for the thread
    std::string GetValueByThreadId(const std::string& threadId, const unsigned int& rowIndex, const std::string& columnName);

    /// Returns the row pointer for the given row for a thread
    /// \param[in] threadId id of the thread
    /// \param[in] rowIndex index of the row in the file
    CSVRow* GetRowByThreadId(const std::string& threadId, const unsigned int& rowIndex);

    /// Implementation of the OnParse virtual function
    /// \param obj pointer to the object parsed
    /// \param[out] stopParsing flag indicating if parsing should stop after this item
    void OnParse(CSVRow* csvRow, bool& stopParsing);

private:

    std::map<std::string, CSVRowList> m_rowsByThreadId;   ///< Map of the parsed rows object pointer by unique threads
    CSVRowList m_rows;                                    ///< List of the pointers of parsed rows object

};

class KernelRowDataHelper
{

public:

    /// Removes the line feed from the string if present
    /// \param[in] string string for removing the line feed
    /// \return string without line feed
    static std::string RemoveLineFeed(const std::string& string);

    /// Checks the two CSVRows are same or not
    /// \param[in] firstRow CSVRow object pointer
    /// \param[in] secondRow CSVRow object pointer
    /// \return return true if the both row matches by thread Id otherwise false
    static bool IsThreadMappingRow(CSVRow* firstRow, CSVRow* secondRow);

    /// Checks the column name is common column or not
    /// \param[in] columnName name of the column
    /// \return returns true if columnName is common column otherwise false
    static bool IsCommonColumn(const std::string& columnName);

    /// Creates Headers along with its original name and corresponding file Index from the all of the counter files
    /// \param[in] counterFileList list of the counter files
    /// \param[in] includeTime flag to indicate to include GPUTime or not
    /// \return list of the header pairs
    static HeaderList CreateHeader(const std::vector<std::string>& counterFileList, bool includeTime);

    /// Sorts the mapped threads based on their execution order in the original CSV file
    /// \param[in] allFilesRowData map of the parsed KernelRow Data
    /// \param[in] mappedThreads list of the sets of the mapped threads between different files
    /// \return returns the list of the sorted sets of the pair of mapped threads of deifferent CSV files
    static MappedThreadSetList SortMappedThreadByExecutionOrder(std::map<unsigned int, KernelRowData*> allFilesRowData, MappedThreadSetList mappedThreads);

    /// Maps the threads between different CSV files
    /// \param[in] allFilesRowData map of the file Index and its corresponding parsed row data from the CSV file
    /// \return list of the sets of the mapped threads
    static MappedThreadSetList GetMappedThreads(std::map<unsigned int, KernelRowData*> allFilesRowData);

private:

    static const std::string m_methodString;
    static const std::string m_executionOrderString;
    static const std::string m_threadIdString;
    static const std::string m_callIndexString;
    static const std::string m_globalWorkSizeString;
    static const std::string m_workGroupSizeString;
    static const std::string m_localMemSizeString;
    static const std::string m_vgprsString;
    static const std::string m_sgprsString;
    static const std::string m_scratchRegsString;

};

#endif
