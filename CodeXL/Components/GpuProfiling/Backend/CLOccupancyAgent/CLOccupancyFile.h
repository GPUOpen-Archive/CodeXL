//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Merge tmp file to occupancy File
//==============================================================================

#ifndef _CL_OCCUPANCY_FILE_H_
#define _CL_OCCUPANCY_FILE_H_

#include <string>

#include <AMDTBaseTools/Include/gtString.h>

struct CLOccupancyHdr
{
    int m_iVersionMajor = 0;
    int m_iVersionMinor = 0;
    gtString m_strAppName;
    gtString m_strAppArgs;
    char m_listSeparator;
};

/// Write occupancy header
/// \param sout output stream
/// \param header Occupancy file header
/// \param cListSeparator List separator
void WriteOccupancyHeader(std::ostream& sout, const CLOccupancyHdr& header, char cListSeparator = ',');

/// Merge tmp cl trace file
/// \param [in] strOutputFile output occupancy file
/// \param [in] strTmpFilePath cl occupancy file path
/// \param [in] strFilePrefix file prefix
/// \param [in] occupancyHeader file header
/// \return true if succeed
bool MergeTmpCLOccupancyFile(const std::string& strOutputFile,
                             const gtString& strTmpFilePath,
                             const std::string& strFilePrefix,
                             const CLOccupancyHdr& occupancyHeader);

#endif
