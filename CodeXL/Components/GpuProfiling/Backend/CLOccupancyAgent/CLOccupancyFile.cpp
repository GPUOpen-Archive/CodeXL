//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Merge tmp file to occupancy File
//==============================================================================

#include <string>
#include <sstream>
#include <fstream>
#include <iterator>
#include <iostream>
#include "CLOccupancyFile.h"
#include "../Common/FileUtils.h"
#include "../Common/LocaleSetting.h"
#include "../Common/StringUtils.h"

#include <AMDTBaseTools/Include/gtString.h>

using namespace std;

void WriteOccupancyHeader(std::ostream& sout, const CLOccupancyHdr& occupancyHeader, char cListSeparator)
{
    string convertedInfo;
    sout << "# ProfilerVersion=" << GPUPROFILER_BACKEND_MAJOR_VERSION << "." << GPUPROFILER_BACKEND_MINOR_VERSION << "." << GPUPROFILER_BACKEND_BUILD_NUMBER << std::endl;
    StringUtils::WideStringToUtf8String(occupancyHeader.m_strAppName.asCharArray(), convertedInfo);
    sout << "# Application=" << convertedInfo.c_str() << std::endl;
    StringUtils::WideStringToUtf8String(occupancyHeader.m_strAppArgs.asCharArray(), convertedInfo);
    sout << "# ApplicationArg=" << convertedInfo.c_str() << std::endl;
    sout << "# ListSeparator=" << occupancyHeader.m_listSeparator << std::endl;

    sout << left << "Thread ID" << cListSeparator;
    sout << left << "Kernel Name" << cListSeparator;
    sout << left << "Device Name" << cListSeparator;
    sout << left << "Number of compute units" << cListSeparator;
    sout << left << "Max. number of wavefronts per CU" << cListSeparator;
    sout << left << "Max. number of work-group per CU" << cListSeparator;
    sout << left << "Max. number of VGPR" << cListSeparator;
    sout << left << "Max. number of SGPR" << cListSeparator;
    sout << left << "Max. amount of LDS" << cListSeparator;
    sout << left << "Number of VGPR used" << cListSeparator;
    sout << left << "Number of SGPR used" << cListSeparator;
    sout << left << "Amount of LDS used" << cListSeparator;
    sout << left << "Size of wavefront" << cListSeparator;
    sout << left << "Work-group size" << cListSeparator;
    sout << left << "Wavefronts per work-group" << cListSeparator;
    sout << left << "Max work-group size" << cListSeparator;
    sout << left << "Max wavefronts per work-group" << cListSeparator;
    sout << left << "Global work size" << cListSeparator;
    sout << left << "Maximum global work size" << cListSeparator;
    sout << left << "Nbr VGPR-limited waves" << cListSeparator;
    sout << left << "Nbr SGPR-limited waves" << cListSeparator;
    sout << left << "Nbr LDS-limited waves" << cListSeparator;
    sout << left << "Nbr of WG limited waves" << cListSeparator;
    sout << left << "Kernel occupancy";
}

bool MergeTmpCLOccupancyFile(const std::string& strOutputFile,
                             const gtString& strTmpFilePath,
                             const std::string& strFilePrefix,
                             const CLOccupancyHdr& occupancyHeader)
{
    bool bStatus = true;

    // Check that the file extension is valid for the occupancy output file
    string strExtension("");
    string strOccupancyFile;

    strExtension = FileUtils::GetFileExtension(strOutputFile);

    if (strExtension != OCCUPANCY_EXT)
    {
        if ((strExtension == TRACE_EXT) || (strExtension == PERF_COUNTER_EXT))
        {
            string strBaseFileName = FileUtils::GetBaseFileName(strOutputFile);
            strOccupancyFile = strBaseFileName + "." + OCCUPANCY_EXT;
        }
        else
        {
            strOccupancyFile = strOutputFile + "." + OCCUPANCY_EXT;
        }
    }
    else
    {
        strOccupancyFile = strOutputFile;
    }

    // Optional
    std::stringstream headerStream;
    char separator = LocaleSetting::GetListSeparator();
    WriteOccupancyHeader(headerStream, occupancyHeader, separator);

    wstring strUnicodePrefix;
    wstring strUnicodeExt;
    StringUtils::Utf8StringToWideString(strFilePrefix, strUnicodePrefix);
    StringUtils::Utf8StringToWideString(TMP_OCCUPANCY_EXT, strUnicodeExt);

    FileUtils::MergeTmpTraceFiles(strOccupancyFile, strTmpFilePath, strUnicodePrefix.c_str(), strUnicodeExt.c_str(), headerStream.str().c_str(), FileUtils::MergeSummaryType_None);

    return bStatus;
}
