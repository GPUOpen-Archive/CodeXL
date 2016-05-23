//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/Util.h $
/// \version $Revision: #37 $
/// \brief :  This file contains Util class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/Util.h#37 $
// Last checkin:   $DateTime: 2016/02/03 12:12:58 $
// Last edited by: $Author: tchiu $
// Change list:    $Change: 557941 $
//=====================================================================

#ifndef _UTIL_H_
#define _UTIL_H_

#include <qtIgnoreCompilerWarnings.h>
#include <QtCore>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTBaseTools/Include/gtList.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afTreeItemType.h>

// Local:
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>
#include <AMDTGpuProfiling/OccupancyInfo.h>

class osFilePath;
class osDirectory;
class SessionTreeNodeData;

// #define INCLUDE_FRAME_ANALYSIS_PERFORMANCE_COUNTERS 1

/// Represents the type of profile performed
enum GPUProfileType
{

    /// Unknown or undefined profile type
    NA_PROFILE_TYPE = 0,

    /// Performance profile type
    PERFORMANCE,

    /// API Trace profile type
    API_TRACE,

#ifdef GP_OBJECT_VIEW_ENABLE
    /// Object Inspector
    OBJECT_INSPECTOR,
#endif

    /// Frame analysis dashboard
    FRAME_ANALYSIS,

    GPU_PROFILE_TYPES_COUNT = FRAME_ANALYSIS
};

class GPUSessionTreeItemData;

/// Class providing utility functions used by the GPU Profiler
class Util
{
public:
    static const QString ms_APISUMFILE;                         ///< API summary file name
    static const QString ms_CTXSUMFILE;                         ///< Context summary file name
    static const QString ms_TOP10KERNELFILE;                    ///< Top 10 kernel summary file name
    static const QString ms_KERNELSUMFILE;                      ///< kernel summary file name
    static const QString ms_TOP10DATAFILE;                      ///< Top 10 data transfer summary file name
    static const QString ms_BESTPRACTICESFILE;                  ///< Best practices page file name
    static const QString ms_APISUM;                             ///< API summary
    static const QString ms_CTXSUM;                             ///< Context summary
    static const QString ms_TOP10KERNEL;                        ///< Top10 kernel summary
    static const QString ms_KERNELSUM;                          ///< Kernel summary
    static const QString ms_TOP10DATA;                          ///< Top 10 Data transfer summary
    static const QString ms_BESTPRACTICES;                      ///< Best practices
    static const QString ms_KERNEL_ASSEMBLY_FILE_PREFIX;        ///< Prefix of file that should be same one defined in the backend (KernelAssembly::m_strFilePrefix)
    static const QString ms_TRUESTR;                            ///< String containing "True"
    static const QString ms_FALSESTR;                           ///< String containing "False"
    static const QString ms_APP_TRACE_OPTIONS_PAGE;             ///< the name of the APP Trace Project Setting page
    static const QString ms_ENABLE_TIMEOUT_OPTION;              ///< Message suggesting to user that they enable the trace timeout mode


    /// Gets the string representing the summary type described in the input enum
    /// \param summaryType the enum representing the summary type
    static gtString SummaryTypeToGTString(afTreeItemType summaryType);

    /// Gets page name as in combobox and tree and returns the type in the enum
    /// \param page name as in combobox and tree
    /// \return the type in the enum
    static afTreeItemType GetEnumTypeFromSumPageName(const QString& page);

    /// \param page name as in combobox and tree
    /// \return a file name to export the page data
    static QString GetShortFileNameFromSumPageName(const QString& page);

    /// Gets the name of the directory under AppData that the profiler can use to store configuration data
    /// \param[out] appDataDir the name of the directory under AppData that the profiler can use to store configuration data
    static void GetProfilerAppDataDir(osFilePath& appDataDir);

    /// Check whether we should use the internal build or not via an environment variable CODEXLGPUPROFILER.
    /// \return true if the environment variable exists, false otherwise
    static bool IsInternalBuild();

    /// Get the tool install directory from the registry key CodeBase.
    /// \param[out] installDir the full tool install path
    /// \return true if successful, false otherwise
    static bool GetInstallDirectory(osFilePath& installDir);

    /// Delete files with a specific prefix and extension in a directory.
    /// \param strDirectory the directory
    /// \param strFilePrefix the file prefix
    /// \param strFileExtension the file extension
    static void DeleteAllFilesInDirectory(const QString& strDirectory,
                                          const QString& strFilePrefix,
                                          const QString& strFileExtension);

    /// Given a string (from the cell value of the statistics table), check whether it is a kernel name.
    /// \param strText the string to check
    /// \return true if it is a kernel name, false otherwise
    static bool IsKernelName(const QString& strText);

    /// Get the project path given the ProfilerOutput directory path.
    /// \param strDirectory the ProfilerOutput path
    /// \return the project path
    static QString GetProjectDirectoryFromProfilerOutput(const QString& strDirectory);

    /// Given a string of kernel name appended with a device name, get the kernel name only.
    /// \param strKernelName the kernel name appended with a device name
    /// \return the kernel name
    static QString StripDeviceNameFromKernelName(const QString& strKernelName);

    /// Given a string, return the first substring (a string inside quotes is considered a single substring).
    /// \param str the input string
    /// \return the first substring
    static QString GetFirstSubString(const QString& str);

    /// Check whether the input string is zero.
    /// \param strValue the input string
    /// \return true if the input string is zero, false otherwise
    static bool IsZeroValue(const QString& strValue);

    /// Remove trailing zero decimals from the value (for example: 3.00 becomes 3).
    /// \param strValue the value in string
    /// \return the updated string without trailing zero decimals
    static QString RemoveTrailingZero(const QString& strValue);

    /// For string type argument, add \ before "
    /// \param strArgs command line arguments string
    /// \return modified command line arguments
    static QString ProcessCmdLineArgsStr(const QString& strArgs);

    ///  convert '/' to '\\' and "\\\\" to "\\" in file path.
    /// \param strPath full file path
    /// \return file path with double back slash
    static QString ToBackSlash(const QString& strPath);

    ///  remove file/directory and report status
    /// \param strPath full file path
    /// \param[out] strDetailReport of deletion
    /// \return True if success
    static bool RemoveFileOrDirectory(const QString& strPath, QString& strDetailReport);

    ///  Get the filename from path
    /// \param strPath path string
    /// \return file name without path
    static QString GetFileNameFromPath(const QString& strPath);

    /// Remove duplicated slash
    /// \param strPath html file path
    /// \return Path without duplicated slash
    static QString RemoveDupSlash(const QString& strPath);

    /// Gets a URL-ish string from a file name
    /// \param strFileName the input filename
    /// \return string that can be passed to a WebBrowser.Navigate call
    static QString GetURLFromFileName(const QString& strFileName);

    /// Cleans any existing session files in the session dir
    /// \param sessionDir the session dir to clean
    static void CleanSessionDir(const osDirectory& sessionDir);

    /// Translate into Qt compatible path, old path
    /// may be windows/linux, path string may be containing
    /// multiple slash, with.without end slash for a folder
    /// but this is translating then to Qt understandable path.
    /// \param strOldPath path to be converted
    /// \return Path that is Qt compatible
    static QString ToQtPath(const QString& strOldPath);

    /// Convert bool to string
    /// \param val the boolean value to convert to string
    /// \return string representation of that bool
    static QString BoolToQString(bool val);

    /// Display a warning box with message
    /// \param strMessage the warning message to be displayed
    static void ShowWarningBox(const QString& strMessage);

    /// Display an error box with message
    /// \param strMessage the error message to be displayed
    static void ShowErrorBox(const QString& strMessage);

    /// Logs a warning to the CodeXL log
    /// \param strMessage the warning message to be logged
    static void LogWarning(const QString& strMessage);

    /// Logs an error to the CodeXL log
    /// \param strMessage the error message to be logged
    static void LogError(const QString& strMessage);

    /// Will convert ModelIndex to QString
    /// \param model Standard model item
    /// \param index constant modelIndex
    /// \return data stored into specified model item
    static QString ToString(QStandardItemModel* model, const QModelIndex& index);

    /// Convert string to bool
    /// \param strBoolVal the string to convert
    /// \return bool representation of that string
    static bool QStringToBool(const QString& strBoolVal);

    /// Get a string representation of the specified profiler type -- used in the GPUSessionTreeItemData Explorer
    /// \param profileType the profile type
    /// \return a string representation of the specified profiler type
    static QString GetProfileTypeName(GPUProfileType profileType);

    /// Append file extension if it is missing.
    /// \param fileName file name string
    /// \param fileExtension expected file extension string
    /// \return file name with expected file extension
    static QString AppendFileExtension(const QString& fileName, const QString& fileExtension);

    /// To mark checked or unchecked of TreeWidgetItem/s having specified string.
    /// \param treeWidget TreeWidget of which TreeWidgetItem needs to mark check/uncheck.
    /// \param strTreeWidgetItem Will apply the change to the TreeWidgetItem which has exact string.
    /// \param checked flag indicating whether or not the items should be checked or unchecked
    /// \param checkAll flag indicating if the entire tree should be searched or if the search should stop after the first match
    static void SetTreeWidgetItemChecked(QTreeWidget* treeWidget, const QString& strTreeWidgetItem, bool checked, bool checkAll);

    /// To mark checked or unchecked of all TreeWidgetItems of the specified TreeWidget.
    /// \param treeWidget TreeWidget of which all items need to mark check/uncheck.
    /// \param checked flag indicating whether or not the items should be checked or unchecked
    static void SetCheckState(QTreeWidget* treeWidget, bool checked);

    /// To find a specific node in TreeWidegt
    /// \param treeWidget root of the tree widget
    /// \param nodeName contains node name
    /// \returns QTreeWidgetItem if already present else null.
    static QTreeWidgetItem* FindTreeItem(QTreeWidget* treeWidget, const QString& nodeName);

    /// Will return number of nodes are checked in TreeWidget
    /// \param treeWidget tree widget
    /// \returns number of checked items in the list
    static int ItemsSelectedInTreeWidget(QTreeWidget* treeWidget);

    /// Updates the specified tree node's children (recursively) and optionally its parents check state, based on the node's check state.  Can also update a count passed in (used by the counter selection UI to display how many counters are selected)
    /// \param treeWidgetItem the tree node whose children/parent needs to be updates
    /// \param updateParent flag indicating whether or not the node's parents should be updated
    /// \param updateCount flag indicating whether or not the counter passed in via the counterCount param should be updated
    /// \param[in,out] counterCount the counter that should be updated based on the check state
    static void UpdateTreeWidgetItemCheckState(QTreeWidgetItem* treeWidgetItem, bool updateParent, bool updateCount, int& counterCount);

    /// Gets the list of kernel files for the specified kernel in the specified session
    /// \param pSession the session whose kernel files are needed
    /// \param strKernelName the kernel whose kernel files are needed
    /// \param[out] kernelFiles the list of kernel files found for the specified session and kernel
    /// \return true if kernel files are found, false otherwise
    static bool GetKernelFiles(const GPUSessionTreeItemData* pSession, const QString& strKernelName, gtList<osFilePath>& kernelFiles);

    /// Indicates whether or not any source is available for the specified kernel
    /// \param pSession the session whose kernel is to be checked
    /// \param strKernelName the name of the kernel to be checked
    /// \return true if any source code is available to be displayed
    static bool IsCodeAvailable(const GPUSessionTreeItemData* pSessionData, const QString& strKernelName);

    /// Indicates whether or not any source is available for the specified kernel
    /// \param pSession the session whose kernel is to be checked
    /// \param strKernelName the name of the kernel to be checked
    /// \return true if any source code is available to be displayed

    /// Checks the input filePath to see if it represents a file contained in a CodeXL sample (i.e the Teapot).
    /// If so, then it returns the full path of the source file as it would appear in an installed build.
    /// This is done because the debug info for the pre-built Teapot sample has paths to the build machine.
    /// We need to change the path so that the files can be opened in an installed build.
    /// There is similar code in gdCallsStackListCtrl.cpp that does the same thing on the debugger side.
    /// The duplicated code should be moved into the framework so we only have one place to update if additional samples are added in the future.
    /// \param filePath the input file path
    /// \return the filepath as it appears in an installed build
    static osFilePath GetInstalledPathForSampleFile(const osFilePath& filePath);

    /// Helper function that parse version string
    /// \param versionStr input version string
    /// \param majorVersion output major version number
    /// \param minorVersion output minor version number
    /// \return True if succeed
    static bool ParseOccupancyFileVersionString(const QString& versionStr, int& majorVersion, int& minorVersion);

    /// Load the occupancy file related to the session
    /// \param sessionFilePath the session file path
    /// \param table to be filled containing the occupancy info for each of the threads
    /// \param pSessionData the session data
    static bool LoadOccupancyFile(const osFilePath& sessionFilePath, OccupancyTable& occupancyTable, GPUSessionTreeItemData* pSessionData);

    /// Parses the occupancy file line
    /// \param line the line
    /// \param delimiter the separator between the strings
    /// \param table to be filled containing the occupancy info for each of the threads
    /// \return true for success (all occupancy parameters are found in the string)
    static bool ParseOccupancySingleLine(const QString& line, const QString& delimiter, OccupancyTable& occupancyTable);

    /// Clear the occupancy table
    /// \param occTable the table to clear
    static void ClearOccupancyTable(OccupancyTable& occTable);

    /// Compares the two device names to see if they are identical or if they differ only in cases where spaces have been replaced by underscores
    /// \param strDeviceName1 the first device name
    /// \param strDeviceName2 the second device name
    /// \return true if the two devices names are identical or if they differ only in cases where spaces have been replaced by underscores, false otherwise
    static bool CheckOccupancyDeviceName(const QString& strDeviceName1, const QString& strDeviceName2);
    
    /// Verifies if profiled machine has HSA support  both remote and local dependant on projects type(remote or local)
    /// \return true if profiled machine supports HSA
    static bool IsHSAEnabled();

private:
    /// prevent creation of an instance of this static class
    Util();

    /// prevent creation of an instance of this static class
    Util(const Util& obj);

    /// disable default assignment operator
    Util& operator=(const Util& obj);

    /// string containing the name of the directory under AppData that the profiler can use to store configuration data
    static QString profilerAppDataDir;

    /// Helper function for UpdateTreeWidgetItemCheckState -- recursive function to update a node's check state based on its children's check state
    /// \param treeWidgetItem the tree item whose parent should be updated
    /// \param checkChildren flag indicating whether or not it needs to check all children.  If false it is assumed that the node should be partially checked (this is an optimization)
    static void UpdateParentTreeWidgetItemCheckState(QTreeWidgetItem* treeWidgetItem, bool checkChildren);
};

#endif // _UTIL_H_
