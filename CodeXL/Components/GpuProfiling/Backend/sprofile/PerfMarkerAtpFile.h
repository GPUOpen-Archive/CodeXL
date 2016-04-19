//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file PerfMarkerAtpFile.h
/// \brief  PerfMarker Atp File writer and parser
//==============================================================================

#ifndef _PERF_MARKER_ATP_FILE_H_
#define _PERF_MARKER_ATP_FILE_H_

#include <AMDTOSWrappers/Include/osOSDefinitions.h>

#include "AtpFile.h"

//------------------------------------------------------------------------------------
/// PerfMarker Entry
//------------------------------------------------------------------------------------
class PerfMarkerEntry
{
public:

    /// Perf Marker entry type
    enum PerfMarkerType
    {
        PerfMarkerType_Begin = 0,  ///< Begin Perf Marker
        PerfMarkerType_End = 1     ///< End Perf Marker
    };

    /// Constructor
    /// \param markerType the type of perf marker entry
    /// \param timestamp the timestamp for the perf marker entry
    /// \param tid the thread id for the perf marker entry
    PerfMarkerEntry(PerfMarkerType markerType, ULONGLONG timestamp, osThreadId tid) : m_markerType(markerType), m_timestamp(timestamp), m_tid(tid) {}

    /// Destructor
    virtual ~PerfMarkerEntry() {}

    PerfMarkerType m_markerType;     ///< Type of perf marker
    ULONGLONG m_timestamp;           ///< Timestamp
    osThreadId m_tid;                ///< Thread ID
};

//------------------------------------------------------------------------------------
/// PerfMarker Begin Entry
//------------------------------------------------------------------------------------
class PerfMarkerBeginEntry : public PerfMarkerEntry
{
public:
    /// Constructor
    /// \param markerType the type of perf marker entry
    /// \param timestamp the timestamp for the perf marker entry
    /// \param tid the thread id for the perf marker entry
    /// \param strName Marker name
    /// \param strGroup Group name
    PerfMarkerBeginEntry(PerfMarkerType markerType, ULONGLONG timestamp, osThreadId tid, const std::string& strName, const std::string& strGroup) :
        PerfMarkerEntry(markerType, timestamp, tid), m_strName(strName), m_strGroup(strGroup) {}

    std::string m_strName;           ///< Marker name
    std::string m_strGroup;          ///< Group name
};

//------------------------------------------------------------------------------------
/// cl perfmarker trace result
//------------------------------------------------------------------------------------
class PerfMarkerAtpFilePart : public IAtpFilePart, public IAtpFilePartParser, public BaseParser<PerfMarkerEntry>
{
public:
    /// Constructor
    /// \param config Config object
    PerfMarkerAtpFilePart(const Config& config, bool shouldReleaseMemory = true) : IAtpFilePart(config, shouldReleaseMemory)
    {
        m_strPartName = "perfmarker";
        m_sections.push_back("CodeXL Perfmarker Output");
        m_sections.push_back("CodeXL OpenCL Perfmarker Output"); // add this section for backward compatibility -- this allows us to parse a file created with older versions of the ActivityLogger
    }

    /// Write header section
    /// If a AptFilePart wants to output to header section, implement this method
    /// \param sout Output stream
    void WriteHeaderSection(SP_fileStream& sout)
    {
        SP_UNREFERENCED_PARAMETER(sout);
    }

    /// Write content section
    /// \param sout Output stream
    /// \param strTmpFilePath Output fragment files path
    /// \param strPID child process ID
    /// \return true if any contents were written, false otherwise
    bool WriteContentSection(SP_fileStream& sout, const std::string& strTmpFilePath, const std::string& strPID);

    /// Save atp file part into a separate file (Called only in compatibility mode)
    /// \param strTmpFilePath Output fragment files path
    /// \param strPID child process ID
    void SaveToFile(const std::string& strTmpFilePath, const std::string& strPID);

    /// Parse input stream
    /// \param in Input stream
    /// \return True if succeeded
    bool Parse(std::istream& in, std::string& outErrorMsg) override;

    /// Parse header
    /// \param strKey Key name
    /// \param strVal Value
    /// \return True if succeeded
    bool ParseHeader(const std::string& strKey, const std::string& strVal);
};

#endif // _PERF_MARKER_ATP_FILE_H_
