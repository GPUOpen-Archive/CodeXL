//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspDTEConnector.h
///
//==================================================================================

//------------------------------ vspDTEConnector.h ------------------------------

#ifndef __VSPDTECONNECTOR_H
#define __VSPDTECONNECTOR_H

#include <wchar.h>

// C++:
#include <string>
#include <vector>

// Infra:
#include <Include/Public/CoreInterfaces/IDteTreeEventHandler.h>
#include <Include/Public/CoreInterfaces/IVspDTEConnector.h>

// ----------------------------------------------------------------------------------
// Class Name:           vspDTEConnector
// General Description: Manages all connection to the Design Time Extensibility (DTE)
//                      interface and maintains a pointer to the interface.
// Author:               Uri Shomroni
// Creation Date:        6/10/2010
// ----------------------------------------------------------------------------------
class vspDTEConnector : public IVspDTEConnector
{
public:
    vspDTEConnector();
    ~vspDTEConnector();

    static vspDTEConnector& instance();

    void setDTEInterface(VxDTE::_DTE* piDTE);
    void releaseDTEInterface();

    // Process creation data:
    void getActiveProjectDebugInformation(std::wstring& executableFilePath, std::wstring& workingDirectoryPath, std::wstring& commandLineArguments, std::wstring& environment, bool& isProjectTypeValid);
    void getStartupProjectDebugInformation(std::wstring& executableFilePath, std::wstring& workingDirectoryPath, std::wstring& commandLineArguments, std::wstring& environment, bool& isProjectOpen, bool& isProjectTypeValid, bool& isNonNativeProject);
    bool hasStartupProjectExecutableChanged(std::wstring& io_executableFilePath);

    // Debugging:
    bool isSolutionLoaded();
    bool resumeDebugging();
    bool stepInto();
    bool stepOver();
    bool stepOut();
    bool isDebuggingOn();

    // Breakpoints:
    bool addBreakpointInSourceLocation(const std::wstring& filePath, int lineNumber, bool enabled);
    bool removeBreakpointsInSourceLocation(const std::wstring& filePath, int lineNumber);
    bool disableFunctionBreakpoint(const std::wstring& functionName);
    bool isSourceLocationBreakpointEnabled(const wchar_t* filePath, int lineNumber, bool& isEnabled);
    bool isFunctionBreakpointEnabled(const wchar_t* functionName, bool& isEnabled);
    bool getFunctionBreakpoints(std::vector<std::wstring>& enabledBreakpoints, std::vector<std::wstring>& disabledBreakpoints);

    // Other operations:
    bool openFileAtPosition(const std::wstring& filePath, int lineNumber = 0, bool selectLine = true, int columnNumber = 0);
    bool openURL(const std::wstring& url);
    bool getWindowFromFilePath(const std::wstring& filePath, /* (VxDTE::Window*&) */ void*& piWindow);
    bool closeDocumentsOfDeletedFiles();
    bool closeAndReleaseWindow(/* (VxDTE::Window*&) */ void*& piWindow, bool close = true);
    bool getSelectedEditorText(std::wstring& selectedText);
    bool getSelectedWindowText(std::wstring& selectedText);
    void forceVariablesReevaluation();

    // Closes the Disassembly window.
    void CloseDisassemblyWindow(void);

    bool deployStartupProject(std::wstring& appUserModelId);
    bool verifyStartupProjectBuilt();
    void constructProjectsNames(std::vector<VxDTE::Project*>& projectsList, std::vector<std::wstring>& projectNames);
    void getBuildDependencies(std::vector<VxDTE::Project*>& projectsList);

    void getBuildOptions(int& onRunWhenOutOfDateAction, int& onRunOrPreviewAction, int& onRunWhenErrorsAction);
    void buildSaveTree(IDteTreeEventHandler* pHandler);
    void saveChangedFiles(bool saveSolutionAndProject);

    void getKernelSourceFilePath(std::vector<std::wstring>& programsFilePath, bool onlyFromStartupProject);
    void setHexDisplayMode();
    void syncHexDisplayModeWithVS();

    bool openSolution(const wchar_t* solutionName);
    dbgDebugMode currentDebugggerMode() const;

    bool isSampleProject();

    // Edit and continue functions
    void clearOpenedFiles();
    bool wasAnyOpenedFileModified();
    void addFileToOpenedList(const wchar_t* filePath);
    bool isFileOpened(const std::wstring& filePath);

    //bool closeOpenedFile(const osFilePath& filePath);
    bool closeOpenedFile(const wchar_t* filePath);

    /// Get the amount of opened files:
    int GetOpenedFilesCount();

    /// Get the path for the opened file at index:
    bool GetOpenedFileAt(int index, std::wstring& filePath);

    /// Get the path for the opened file at index:
    void GetListOfFilesContainedAtDirectory(const std::wstring& folderStr, std::vector<std::wstring>& listOfFilePathContained);

    bool saveFileWithPath(const std::wstring& filePath);
    bool getActiveDocumentFileFullPath(std::wstring& filePath);

    bool GetActiveWindowHandle(HWND& handleWindow);

    void ShowOutputWindow();

    /// Issue a save all command before starting a "Start ..." in codeXL
    void SaveAllCommand();

protected:

    void registerCommandListener();
    void getDebugInformationFromProject(VxDTE::Project& project, std::wstring& executableFilePath, std::wstring& workingDirectoryPath, std::wstring& commandLineArguments, std::wstring& environment, bool& isProjectTypeValid, bool& isNonNativeProject);
    void getIntegerPropertyValue(VxDTE::Properties& properties, const std::wstring& propertyName, int& propertyValue, bool& isPropertyPresent);
    void getStringPropertyValue(VxDTE::Properties& properties, const std::wstring& propertyName, std::wstring& propertyValue, bool& isPropertyPresent);
    void evaluateUserMacros(VxDTE::Project& project, VxDTE::Configuration& activeConfiguration, bool onlyPath, std::wstring& executableFilePath, std::wstring& workingDirectoryPath, std::wstring& commandLineArguments, std::wstring& environment);
    void evaluateUserMacrosInString(/* (VCProjectEngineLibrary::VCConfiguration*) */ void* pActiveConfigurationAsVoidPointer, std::wstring& evalString);
    void verifyNonRelativePath(VxDTE::Project& project, std::wstring& projectPath);
    void getConfigurationTargetPath(VxDTE::Configuration& configuration, std::wstring& targetPath);
    bool getConfigurationFullName(VxDTE::Configuration& configuration, std::wstring& configrationFullName);
    void cleanURLString(std::wstring& urlString);
    VxDTE::Project* getProjectByName(VxDTE::_Solution& solution, const std::wstring& projectName);
    bool addProjectBuildDependencies(std::vector<VxDTE::Project*>& projectsList, VxDTE::Project* currentProject);
    bool projectUpToDate(VxDTE::Project& project);
    bool getCLFilesFromProject(VxDTE::Project* pProject, std::vector<std::wstring>& programsFilePath);
    void processProjectNameForSearch(std::wstring& projectNameString);

    // Overrides IVspDTEConnector
    virtual void vspDtecBuildSaveTree(IDteTreeEventHandler* pDteTreeEventHandler);
    virtual void vspDtecGetKernelSourceFilePath(wchar_t**& pProgramFilePaths, int& programFilePathsSize, bool isOnlyFromStartupProject);
    virtual void vspDtecDeleteWCharStrBuffers(wchar_t**& pBuffersArr, int arrSize);
    virtual void vspDtecGetActiveDocumentFileFullPath(wchar_t*& pBuffer);
    virtual void vspDtecGetListOfFilesContainedAtDirectory(const wchar_t* pFolderStr, wchar_t**& pListOfFilePathContained, int& listOfPathSize);
    virtual void vspDtecSaveFileWithPath(const wchar_t* folderStr);
    virtual bool vspDtecResumeDebugging();

private:

    // A handle to the core implementation instance.
    void* m_pCoreImple;

    friend class vspSingletonsDelete;

private:
    static vspDTEConnector* _pMySingleInstance;
    VxDTE::_DTE* _piDTE;
    VxDTE::Debugger* _piDebugger;
    VxDTE::ItemOperations* _piItemOperations;
};

#endif //__VSPDTECONNECTOR_H

