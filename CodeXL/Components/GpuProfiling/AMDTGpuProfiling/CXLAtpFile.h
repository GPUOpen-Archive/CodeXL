//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Atp file creation and parsing
//==============================================================================

#ifndef _CXL_ATP_FILE_H_
#define _CXL_ATP_FILE_H_

#include <string>
#include <algorithm>
#include <vector>
#include <Config.h>
#include <IParserProgressMonitor.h>
#include <Defs.h>
#include <OSDefs.h>

#include "CXLBaseParser.h"

//------------------------------------------------------------------------------------
/// Interface for AtpFilePartParser
//------------------------------------------------------------------------------------
class IAtpFilePartParser
{
public:

    /// Constructor
    IAtpFilePartParser(): m_shouldStopParsing(false) {}

    /// Destructor
    virtual ~IAtpFilePartParser() {}

    /// Parse input stream
    /// \param in Input stream
    /// \return True if succeeded
    virtual bool Parse(std::istream& in, std::string& outErrorMsg) = 0;

    /// Parse header
    /// \param strKey Key name
    /// \param strVal Value
    /// \return True if succeeded
    virtual bool ParseHeader(const std::string& strKey, const std::string& strVal) = 0;

    /// Adds a progress monitor -- the IAtpFilePart will report progress as is parses its items
    /// \param pProgressMonitor the progress monitor object
    void AddProgressMonitor(IParserProgressMonitor* pProgressMonitor);

    /// Reports progress to the monitors
    /// \param strProgressMessage the progress message to display
    /// \param uiCurItem the index of the current item being parsed
    /// \param uiTotalItems the total numbers of items to be parsed
    void ReportProgress(const std::string& strProgressMessage, unsigned int uiCurItem, unsigned int uiTotalItems);

    /// Should the parsing stop?
    /// \return true if the parsing process should be stopped
    bool ShouldStopParsing() const {return m_shouldStopParsing;}

    /// Sets the current section name
    /// \param strSectionName the naem of the current section
    void SetCurrentSection(const std::string& strSectionName);

protected:
    std::vector<IParserProgressMonitor*> m_progressMonitorList; ///< Parser progress list
    bool m_shouldStopParsing;                                   ///< A flag indicating whether the user had chosen to stop the parsing
    std::string m_strCurrentSectionName;                        ///< The current Section Name
};

//------------------------------------------------------------------------------------
/// Each IAtp file class handles the following task
/// 1. Merge fragment files
/// 2. Write module specific information to header
/// 3. Write module trace result to content section
/// 4. Parse the atp file part
//------------------------------------------------------------------------------------
class IAtpFilePart
{
public:
    /// Constructor
    /// \param config Config object
    IAtpFilePart(const Config& config, bool shouldReleaseMemory) : m_config(config), m_shouldReleaseMemory(shouldReleaseMemory) {}

    /// Destructor
    virtual ~IAtpFilePart() {}

    /// Write header section
    /// If a AptFilePart wants to output to header section, implement this method
    /// \param sout Output stream
    virtual void WriteHeaderSection(SP_fileStream& sout) = 0;

    /// Write content section
    /// \param sout Output stream
    /// \param strTmpFilePath Output fragment files path
    /// \param strPID child process ID
    /// \return true if any contents were written, false otherwise
    virtual bool WriteContentSection(SP_fileStream& sout, const std::string& strTmpFilePath, const std::string& strPID) = 0;

    /// Optional method
    /// Save atp file part into a separate file (Called only in compatibility mode)
    /// \param strTmpFilePath Output fragment files path
    /// \param strPID child process ID
    virtual void SaveToFile(const std::string& strTmpFilePath, const std::string& strPID)
    {
        SP_UNREFERENCED_PARAMETER(strTmpFilePath);
        SP_UNREFERENCED_PARAMETER(strPID);
    }

    /// Is this object a type of IAtpFilePartParser
    /// Return true if this is type of IAtpFilePartParser
    bool IsParser() const { return dynamic_cast<const IAtpFilePartParser*>(this) != NULL; }

    /// Is strSecName belongs to this AtpFilePart
    /// \param strSecName Section name
    bool HasSection(const std::string& strSecName) const
    {
        // strip off leading "AMD " in case we are loading an .atp file created with a previous version of CodeXL
        static const std::string strLegacySectionPrefix("AMD ");

        std::string strTrimmedSectionName(strSecName);

        if (0 == strSecName.compare(0, strLegacySectionPrefix.size(), strLegacySectionPrefix))
        {
            strTrimmedSectionName = strTrimmedSectionName.substr(strLegacySectionPrefix.size());
        }

        std::vector<std::string>::const_iterator it = std::find(m_sections.begin(), m_sections.end(), strTrimmedSectionName);
        return it != m_sections.end();
    }

protected:
    /// Helper function to generate section string
    /// \param strSectionName Section name
    std::string GetSectionHeader(const std::string& strSectionName);

    /// Disable copy constructor and assignment operator
    IAtpFilePart& operator = (const IAtpFilePart& rhs);
    IAtpFilePart(const IAtpFilePart& rhs);

    const Config& m_config;                ///< Config
    std::string m_strPartName;             ///< Atp part name
    std::vector<std::string> m_sections;   ///< section names
    bool m_shouldReleaseMemory;            ///< Should release the APIInfo items, or not

};

//------------------------------------------------------------------------------------
/// Atp file class
//------------------------------------------------------------------------------------
class AtpFile
{
public:
    /// Constructor
    AtpFile() {}

    /// Destructor
    virtual ~AtpFile() {}

    /// Add atp file part
    /// \param pPart Atp file part object
    void AddAtpFilePart(IAtpFilePart* pPart)
    {
        m_parts.push_back(pPart);
    }

protected:
    std::vector<IAtpFilePart*> m_parts; ///< Atp file parts
    int m_AtpFileVersionMajor;          ///< Atp file version major, this is separate from profiler version
    int m_AtpFileVersionMinor;          ///< Atp file version minor, this is separate from profiler version
};

//------------------------------------------------------------------------------------
/// Manages atp file creation.
///   1. In non-time-out mode, each trace module generates one file ($OutputFileName.$ModeName.atp)
///      It merges all trace result file into one.
///   2. In time-out mode, it calls each IAtpFile to merge its fragment files into one file $OutputFileName.$ModeName.atp,
///      then merge them into one file.
//------------------------------------------------------------------------------------
class AtpFileWriter : public AtpFile
{
public:
    /// Constructor
    /// \param config Config object
    /// \param strPID Child process ID
    AtpFileWriter(const Config& config, const std::string& strPID);

    /// Destructor
    ~AtpFileWriter();

    /// Save to atp file
    void SaveToAtpFile();

private:
    /// Disable copy constructor and assignment operator
    AtpFileWriter& operator = (const AtpFileWriter& rhs);
    AtpFileWriter(const AtpFileWriter& rhs);

    const Config& m_config;             ///< Config object
    std::string m_strPID;               ///< child process ID
};

typedef std::map<std::string, std::string> HeaderMap;

//------------------------------------------------------------------------------------
/// Manages atp file parsing.
/// Atp file consists of two parts.
/// 1. Atp header, each AtpFilePart can have its own headers, each AtpFilePart should
///    parse its own header, otherwise it's ignored
/// 2. Atp content, content is divided into sections, AtpFilePart is responsible for
///    parsing its own sections, otherwise, it's ignored
//------------------------------------------------------------------------------------
class AtpFileParser : public AtpFile, public BaseFileParser<>
{
public:
    /// Constructor
    AtpFileParser(int atpFileVersion = 0) : AtpFile(), m_atpFileVersion(atpFileVersion), m_shouldStopParsing(false) {}

    /// Destructor
    ~AtpFileParser() {}

    /// Parse atp file
    bool Parse();

protected:

    /// Helper function to parse section name
    /// \param input string
    /// \param[out] sectionName Output section name
    bool ParseSectionName(const std::string& input, std::string& sectionName);

    /// Checks if a line is a header line. If it is, parse it, keep the header data, and call the parsers callbacks
    /// \return true if the line is a header line, false if this line does not belong to the header
    bool ParseSuspectedHeaderLine(const std::string& headerLine);

    /// Parse the file sections
    /// \param sectionLine the section line for parse
    /// \return true for success parsing
    bool ParseFileSectionsLine(const std::string& sectionLine);

protected:

    HeaderMap m_headerMap;  ///< Header key-value pair map

    int m_atpFileVersion;   ///< An integer containing the trace file version

    bool m_shouldStopParsing; ///< True iff the parsing should be stopped.
};

#endif // _ATP_FILE_H_
