//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  HSA Atp File writer and parser
//==============================================================================

#ifndef _HSA_ATP_FILE_H_
#define _HSA_ATP_FILE_H_

#include "../Common/APIInfo.h"
#include "../sprofile/AtpFile.h"
#include "../Common/FileUtils.h"
#include "HSAAPIInfo.h"

//------------------------------------------------------------------------------------
/// HSA API trace result
//------------------------------------------------------------------------------------
class HSAAtpFilePart : public IAtpFilePart, public IAtpFilePartParser, public BaseParser<HSAAPIInfo>
{
public:
    /// Constructor
    /// \param config Config object
    HSAAtpFilePart(const Config& config, bool shouldReleaseMemory = true);

    /// Destructor
    ~HSAAtpFilePart(void);

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

    /// Parse input stream
    /// \param in Input stream
    /// \return True if succeeded
    bool Parse(std::istream& in, std::string& outErrorMsg) override;

    /// Parse header
    /// \param strKey Key name
    /// \param strVal Value
    /// \return True if succeeded
    bool ParseHeader(const std::string& strKey, const std::string& strVal);

protected:
    /// Parse host side timestamp
    /// \param buf Input string
    /// \param[out] pAPIInfo API info object
    /// \param bTimeoutMode Timeout mode flag
    /// \return true if no error occurs
    bool ParseHostTimestamp(const char* buf, HSAAPIInfo* pAPIInfo, bool bTimeoutMode = false);

    /// Parse device side timestamp
    /// \param buf Input string
    /// \param[out] pDispatchInfo Dispatch info object
    /// \return true if no error occurs
    bool ParseDeviceTimestamp(const char* buf, HSADispatchInfo* pDispatchInfo);

    /// Create API info object from API name
    /// \param strAPIName API name
    /// \return API info object
    HSAAPIInfo* CreateAPIInfo(const std::string& strAPIName);

private:
    HSAAPIInfoMap m_HSAAPIInfoMap;   ///< HSA API info map
    HSADispatchInfoList m_HSADispatchInfoList; ///< HSA Dispatch Info list

    unsigned int m_dispatchIndex;  ///< dispatch index, incremented while parsing kernel dispatch timestamps

    unsigned int m_atpMajorVer;    ///< major version of the .atp file
    unsigned int m_atpMinorVer;    ///< minor version of the .atp file
};

#endif //_HSA_ATP_FILE_H_
