//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief File Utils functions
//==============================================================================

#ifndef _FILE_UTILS_H_
#define _FILE_UTILS_H_

#include <string>
#include <vector>
#include <unordered_set>
#include <ostream>
#include "Defs.h"

// This defs are needs since in linux using ofstream is the better behaviour: wofstream << wchar_t is not working correctly if you mix char & wchar_t
// and utf8 writing works better and correctly with ofstream. In windows we use wofstreams in order to support in the future unicode file names

#ifdef _WIN32
    #define SP_outStream std::wostream
    #define SP_fileStream std::wofstream
    #define SP_stringStream std::wstringstream
#else
    #define SP_outStream std::ostream
    #define SP_fileStream std::ofstream
    #define SP_stringStream std::stringstream
#endif

/// \addtogroup Common
// @{

/// Utility functions to manipulate files.
namespace FileUtils
{
// *INDENT-OFF*
#ifdef _DEBUG
/// In a debug build, this function displays either a Windows message box or a Linux console message,
/// essentially pausing the application begin profiled to allow you to attach a debugger.
void CheckForDebuggerAttach(void);
#endif

#ifndef _WIN32
/// Replace Tilde with home path
/// \param strHome home path
/// \param strInOut input and output
/// \return true if tilde found and replaced
bool ReplaceTilde(std::string& strHome, std::string& strInOut);
#endif
// *INDENT-ON*

/// Get the fully-qualified filename of the temp file used to pass parameters
/// between CodeXLGpuProfiler and the servers.  The file is stored in %TEMP% on Windows
/// and in the user's home directory on Linux.  The file is named tmp.spdata on
/// Windows and .spdata on Linux
/// \return fully-qualified file name of temp file
std::string GetTempFile();

/// Get tmp trace path
/// \return fully-qualified name of the temp path
std::string GetTempFragFilePath();

/// Get tmp trace path
/// \return fully-qualified name of the temp path
gtString GetTempFragFilePathAsUnicode();

/// Pass parameters between CodeXLGpuProfiler and servers through the temp file
/// specified by GetTempFile
/// \param params Parameters that are passed to servers
void PassParametersByFile(Parameters params);

/// Retrieve parameters from the temp file specified by GetTempFile
/// \param[out] params Parameters that are passed to servers
/// \return true if succeed
bool GetParametersFromFile(Parameters& params);

/// Loads the contents of params.m_strKernelFile into params.m_kernelFilterList
/// \param[in,out] params Parameters that are passed to servers
/// \param doOutputError output an error to the console if file cannot be read
/// \return true if file could be read, false otherwise (or if no file was specified)
bool ReadKernelListFile(Parameters& params, bool doOutputError = true);

/// Delete the temp file specified by GetTempFile
void DeleteTmpFile();

/// Delete fragment files generated from previous time-out mode.
/// \param[in] szOutputPath Optional, output path
void RemoveFragFiles(const char* szOutputPath = NULL);

/// Get a directory from a file.
/// \param strFilename  the full file path
/// \param strOutputDir the output directory
/// \return true if successful, false otherwise
bool GetWorkingDirectory(const std::string& strFilename, std::string& strOutputDir);

/// Get executable file name (without path)
/// \return executable file name string
std::string GetExeName();

/// Get executable file path
/// \return executable file path string
std::string GetExePath();

gtString GetExePathAsUnicode();

/// Get base file name
/// \param strFileName Input file name
/// \return base file name
std::string GetBaseFileName(const std::string& strFileName);

/// Get file extension
/// \param strFileName Input file name
/// \return extension
std::string GetFileExtension(const std::string& strFileName);

/// Get current app's full path
/// \return full path string
std::string GetExeFullPath();


/// Get current app's full path
/// \return full path string
gtString GetExeFullPathAsUnicode();

/// Get default output path. For windows, it's "CodeXL"
/// directory under the current user's "Documents" directory. For Linux,
/// it's the user's home directory
/// \return default output directory
std::string GetDefaultOutputPath();

/// Get default profile output file name
/// \param[in, opt] appendToDefaultFileName string to append to the default file name
/// \return default output file name -- Session1.csv in the directory returned by GetDefaultOptionPath
std::string GetDefaultProfileOutputFile(const std::string& appendToDefaultFileName = std::string());

/// Get default trace output file name
/// \return default trace output file name -- apitrace.atp in the directory returned by GetDefaultOptionPath
std::string GetDefaultTraceOutputFile();

/// Get default occupancy output file name
/// \return default occupancy output file name -- Session1.occupancy in the directory returned by GetDefaultOptionPath
std::string GetDefaultOccupancyOutputFile();

/// Get default perfmarker output file name
/// \return default perfmarker output file name -- apitrace.amdtperfmarker in the directory returned by GetDefaultOptionPath
std::string GetDefaultPerfMarkerOutputFile();

/// Get default sub-kernel profile output file name
/// \return default sub-kernel profile output file name -- subkernelprofile.txt in the directory returned by GetDefaultOptionPath
std::string GetDefaultSubKernelProfileOutputFile();

/// Get default thread trace output directory path
/// \return Default thread trace output directory path -- clthreadtrace in the directory returned by GetDefaultOptionPath
std::string GetDefaultThreadTraceOutputDir();

/// Merge the contents of the two specified files into a combined file, which can include an additional string written to the start of the file
/// \param strNewFileName the name of the merged file
/// \param strFileName1 the name of the first file to merge
/// \param strFileName2 the name of the second file to merge
/// \param strFileHeader an optional string that will be written to the top of the merged file.
/// \return true if the files are successfully merged into the new file, false otherwise
bool MergeFiles(const std::wstring& strNewFileName, const std::wstring& strFileName1,
                const std::wstring& strFileName2, const std::string& strFileHeader = "");

/// Write a unicode string to a file output.
/// \param strFilename the full file path
/// \param strMessage the string message to be written to the file
/// \return true if successful, false otherwise
bool WriteFile(const std::wstring& strFilename, const std::string& strMessage);

/// Write a unicode string to a file output.
/// \param strFilename the full file path
/// \param strMessage the string message to be written to the file
/// \return true if successful, false otherwise
bool WriteFile(const std::string& strFilename, const std::string& strMessage);

/// Write an array of strings to a file.
/// \param strFilename the full file path.
/// \param vLines an array of lines.
/// \return true if successful, false otherwise
bool WriteFile(const std::wstring& strFilename, const std::vector<std::string>& vLines);

/// Write an array of strings to a file.
/// \param strFilename the full file path.
/// \param vLines an array of lines.
/// \return true if successful, false otherwise
bool WriteFile(const std::string& strFilename, const std::vector<std::string>& vLines);

/// Read a unicode file name to a string.
/// \param strFilename the full file path
/// \param strOut the output string
/// \param bOutputError output an error to the console if file cannot be read
/// \return true if successful, false otherwise (file does not exist)
bool ReadFile(const std::wstring& strFilename, std::string& strOut, bool bOutputError = true);

/// Read a file to a string.
/// \param strFilename the full file path
/// \param strOut the output string
/// \param bOutputError output an error to the console if file cannot be read
/// \return true if successful, false otherwise (file does not exist)
bool ReadFile(const std::string& strFilename, std::string& strOut, bool bOutputError = true);

/// Read a unicode file name to a list of lines.
/// \param strFilename the full file path
/// \param lines the output lines
/// \param bSkipEmptyLines skip empty lines
/// \param bOutputError output an error to the console if file cannot be read
/// \return true if successful, false otherwise (file does not exist)
bool ReadFile(const std::wstring& strFilename, std::vector<std::string>& lines, bool bSkipEmptyLines = false, bool bOutputError = true);

/// Read a file to a list of lines.
/// \param strFilename the full file path
/// \param lines the output lines
/// \param bSkipEmptyLines skip empty lines
/// \param bOutputError output an error to the console if file cannot be read
/// \return true if successful, false otherwise (file does not exist)
bool ReadFile(const std::string& strFilename, std::vector<std::string>& lines, bool bSkipEmptyLines = false, bool bOutputError = true);

/// Read a unicode file name to an unordered set of lines.
/// \param strFilename the full file path
/// \param lines the output lines
/// \param bSkipEmptyLines skip empty lines
/// \param bOutputError output an error to the console if file cannot be read
/// \return true if successful, false otherwise (file does not exist)
bool ReadFile(const std::wstring& strFilename, std::unordered_set<std::string>& lines, bool bSkipEmptyLines = false, bool bOutputError = true);

/// Read a file to an unordered set of lines.
/// \param strFilename the full file path
/// \param lines the output lines
/// \param bSkipEmptyLines skip empty lines
/// \param bOutputError output an error to the console if file cannot be read
/// \return true if successful, false otherwise (file does not exist)
bool ReadFile(const std::string& strFilename, std::unordered_set<std::string>& lines, bool bSkipEmptyLines = false, bool bOutputError = true);

/// Determine whether a file exists or not
/// \param fileName filename string
/// \return whether a file exists
bool FileExist(const std::string& fileName);

/// Get files under specified directory
/// \param strDirPath directory path
/// \param filesOut output files
/// \param filter search filter
/// \return true if succeed
bool GetFilesUnderDir(const std::string& strDirPath, std::vector<std::string>& filesOut, std::string filter = "");

/// Enum to define what summary info should be included when merging tmp trace file
typedef enum
{
    MergeSummaryType_None,                 ///< don't include any summary information, just the contents of the files
    MergeSummaryType_TidAndNumEntries,     ///< include the thread id and number of entries when merging individual files into a single file
    MergeSummaryType_CumulativeNumEntries, ///< include a cumulative number of entries (but no thread id) when merging individual files into a single file
} MergeSummaryType;

/// Merge tmp trace files(api trace, timestamps, stack trace, perfmarker), delete tmp file after merging
/// \param strOutputFile output file
/// \param strTmpFilesDirPath input tmp files dir path
/// \param strFilePrefix file prefix, we use process ID as prefix
/// \param szFileExt the file extension to use
/// \param szHeader output file header
/// \param mergeSummaryType The type of summary info to write for each tmp file being merged
/// \return true if successful
bool MergeTmpTraceFiles(const std::string& strOutputFile,
                        const gtString& strTmpFilesDirPath,
                        const gtString& strFilePrefix,
                        const gtString& szFileExt,
                        const char* szHeader = NULL,
                        MergeSummaryType mergeSummaryType = MergeSummaryType_TidAndNumEntries);

/// Interface function for the real MergeTmpTraceFiles that converts the input to the correct format from non unicode to unicode
/// \param strOutputFile output file
/// \param strTmpFilesDirPath input tmp files dir path
/// \param strFilePrefix file prefix, we use process ID as prefix
/// \param szFileExt the file extension to use
/// \param szHeader output file header
/// \param mergeSummaryType The type of summary info to write for each tmp file being merged
/// \return true if successful
bool MergeTmpTraceFiles(SP_outStream& sout,
                        const gtString& strTmpFilesDirPath,
                        const gtString& strFilePrefix,
                        const gtString& szFileExt,
                        const char* szHeader,
                        MergeSummaryType mergeSummaryType = MergeSummaryType_TidAndNumEntries);

/// Interface function for the real MergeTmpTraceFiles that converts the input to the correct format from none unicode to unicode
/// \param strOutputFile output file
/// \param strTmpFilesDirPath input tmp files dir path
/// \param strFilePrefix file prefix, we use process ID as prefix
/// \param szFileExt the file extension to use
/// \param szHeader output file header
/// \param mergeSummaryType The type of summary info to write for each tmp file being merged
/// \return true if successful
bool MergeTmpTraceFiles(const std::string& strOutputFile,
                        const std::string& strTmpFilesDirPath,
                        const std::string& strFilePrefix,
                        const std::string& szFileExt,
                        const char* szHeader = NULL,
                        MergeSummaryType mergeSummaryType = MergeSummaryType_TidAndNumEntries);

/// Merge tmp trace files(api trace, timestamps, stack trace, perfmarker), delete tmp file after merging
/// \param sout output stream
/// \param strTmpFilesDirPath input tmp files dir path
/// \param strFilePrefix file prefix, we use process ID as prefix
/// \param szFileExt the file extension to use
/// \param szHeader output file header
/// \param mergeSummaryType The type of summary info to write for each tmp file being merged
/// \return true if successful
bool MergeTmpTraceFiles(SP_outStream& sout,
                        const std::string& strTmpFilesDirPath,
                        const std::string& strFilePrefix,
                        const std::string& szFileExt,
                        const char* szHeader,
                        MergeSummaryType mergeSummaryType = MergeSummaryType_TidAndNumEntries);

/// Get profiler binary path from CL_AGENT
/// \param outPath Binary path (including last '\')
/// \param is64bit Is x64 bit path
/// \return true if operation succeed
bool GetProfilerBinaryPath(gtString& outPath, bool& is64bit);

/// Get absolution path
/// \param input input path
/// \return absolute path
std::string ToAbsPath(const std::string& input);

/// Load API Rules config file
/// \param strFileName config file name
/// \param op AnalyzeOps object
void LoadAPIRulesConfig(const std::string& strFileName, AnalyzeOps& op);
} // FileUtils

// @}

#endif //_FILE_UTILS_H_
