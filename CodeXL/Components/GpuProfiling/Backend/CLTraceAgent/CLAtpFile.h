//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief CL Atp File writer and parser
//==============================================================================

#ifndef _CL_ATP_FILE_H_
#define _CL_ATP_FILE_H_

#include <string>
#include <map>
#include <set>

#include <AMDTOSWrappers/Include/osOSDefinitions.h>

#include "../CLCommon/CLPlatformInfo.h"
#include "../Common/FileUtils.h"
#include "APIInfoManagerBase.h"
#include "CLAPIInfo.h"
#include "../sprofile/AtpFile.h"

typedef std::map<osThreadId, std::vector<CLAPIInfo*> > CLAPIInfoMap;
typedef std::pair<osThreadId, std::vector<CLAPIInfo*> > CLAPIInfoMapPair;

//------------------------------------------------------------------------------------
/// GPU Timestamp data
//------------------------------------------------------------------------------------
struct GPUTimestamp
{
    ULONGLONG      m_ullQueuedTimestamp = 0;   ///< Queued Timestamp
    ULONGLONG      m_ullSubmitTimestamp = 0;   ///< Submit Timestamp
    ULONGLONG      m_ullRunningTimestamp = 0;  ///< Running Timestamp
    ULONGLONG      m_ullCompleteTimestamp = 0; ///< Complete Timestamp
};

typedef std::list<GPUTimestamp> GPUTimestampList;
typedef std::map<std::string, GPUTimestampList> EventMap;
typedef EventMap::iterator EventMapIt;
typedef std::pair<std::string, GPUTimestampList> EventMapPair;

//------------------------------------------------------------------------------------
/// CL API trace result
//------------------------------------------------------------------------------------
class CLAtpFilePart : public IAtpFilePart, public IAtpFilePartParser, public BaseParser<CLAPIInfo>
{
public:
    /// Constructor
    /// \param config Config object
    CLAtpFilePart(const Config& config, bool shouldReleaseMemory = true);

    /// Destructor
    ~CLAtpFilePart();

    /// Write header section
    /// If a AptFilePart wants to output to header section, implement this method
    /// \param sout Output stream
    void WriteHeaderSection(SP_fileStream& sout);

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

    // For IAtpFilePartParser

    /// Parse input stream
    /// \param in Input stream
    /// \return True if succeeded
    bool Parse(std::istream& in, std::string& outErrorMsg) override;

    /// Parse header
    /// \param strKey Key name
    /// \param strVal Value
    /// \return True if succeeded
    bool ParseHeader(const std::string& strKey, const std::string& strVal);

private:
    /// Update tmp timestamp file
    /// \param strTmpFilePath Tmp file path
    /// \param strFilePrefix File prefix
    bool UpdateTmpTimestampFiles(const std::string& strTmpFilePath, const std::string& strFilePrefix);

    /// Load GPU timestamps raw file
    /// \param strFile File path
    /// \param[out] vTimestamps Event handle to timestamps map
    bool LoadGPUTimestampRaw(const std::string& strFile, EventMap& vTimestamps);

    /// Merge timestamps into tmp timestamp files
    /// \param strFile GPU timestamps raw file
    /// \param vTimestamps Event handle to timestamps map
    /// \param[out] apis Output APIs
    bool MergeTimestamp(const std::string& strFile, EventMap& vTimestamps, std::vector<CLAPIInfo*>& apis);

    std::vector<std::string> m_excludedAPIs;  ///< excluded APIs
    CLAPIInfoMap m_CLAPIInfoMap;              ///< API Map key = threadID
};

// The following functions to be factored into CLAtpFilePart::Parse()

/// Parse timestamp from .atp file
/// \param[in]    szBuf          timestamp string
/// \param[out]   pAPIInfo       APIInfo object
/// \param[in]    bTimeoutMode   Specify whether or not is parsing special timestamp file for timeout mode.
/// \return True if succeed.
bool ParseTimestamp(const char* szBuf, CLAPIInfo* pAPIInfo, bool bTimeoutMode = false);

/// Create CLAPIInfo object from API name
/// \param[in]    strAPIName     API Name
/// \return CLAPIInfo object
CLAPIInfo* CreateAPIInfo(const std::string& strAPIName);


#endif //_CL_ATP_FILE_H_
