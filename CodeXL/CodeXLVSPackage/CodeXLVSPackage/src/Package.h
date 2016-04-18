//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file Package.h
///
//==================================================================================

// Package.h

#pragma once

// Forward declarations:
class vspPackageCommandHandler;
class vspTimer;

// C++:
#include <string>

// Core interfaces:
#include <Include/Public/CoreInterfaces/IVscDebugEngineOwner.h>
#include <Include/Public/CoreInterfaces/IVscEventObserverOwner.h>
#include <Include/Public/CoreInterfaces/IVscWindowsManagerOwner.h>
#include <Include/Public/CoreInterfaces/IProgressBarDataProvider.h>
#include <Include/Public/CoreInterfaces/IVscApplicationCommandsOwner.h>
#include <Include/Public/CoreInterfaces/IVscSourceCodeViewerOwner.h>
#include <Include/Public/CoreInterfaces/IVscGRApiFunctionsOwner.h>
#include <Include/Public/CoreInterfaces/IVscBreakpointsManagerOwner.h>

#include <atlstr.h>
#include <VSLCommandTarget.h>


#include "resource.h"       // main symbols
#include "Guids.h"
#include "..\CodeXLVSPackageUI\Resource.h"

#include "..\CodeXLVSPackageUI\CommandIds.h"

#include <commctrl.h>

#include "EditorFactory.h"
#include "ProfileSessionEditorFactory.h"
#include "KernelAnalyzerEditorFactory.h"

// Local:
#include <src/vspPackageCommandHandler.h>
#include <src/vspDTEConnector.h>
#include <src/vspToolWindow.h>
#include <src/vspPackageWrapper.h>
#include <Include/vspStringConstants.h>

class vscAppEventObserver;

using namespace VSL;

// TODO: Relocate this constant to the core code space and
// expose it via a function to this (package) code space since
// this constant is already defined at the core.
#define VSP_AMOUNT_OF_MULTIWATCH_VIEWS 3

/***************************************************************************
CCodeXLVSPackagePackage handles the necessary registration for this package.

See EditorFactory.h for the details of the Editor key section in
CodeXLVSPackage.pkgdef.

See the Package C++ reference sample for the details of the Package key section in
CodeXLVSPackage.pkgdef.

See the MenuAndCommands C++ reference sample for the details of the Menu key section in
CodeXLVSPackage.pkgdef.

See EditorDocument.h for the details of the KeyBindingTables key section in
CodeXLVSPackage.pkgdef.

The following Projects key section exists in CodeXLVSPackage.pkgdef in order to
register the new file template.

//The first GUID below is the GUID for the Miscellaneous Files project type, and can be changed
//  to the GUID of any other project you wish.
[$RootKey$\Projects\{A2FE74E1-B743-11d0-AE1A-00A0C90FFFC3}\AddItemTemplates\TemplateDirs\{ecdfbaee-ad99-452d-874c-99fce5a48b8e}\/1]
@="#100"
"TemplatesDir"="$PackageFolder$\Templates"
"SortPriority"=dword:00004E20

The contents of CodeXLVSPackage.vsdir, which is located a the location registered above are:

myext.myext|{ab02f9cb-42e8-467c-a242-d9bb2e1918a0}|#106|80|#109|{ab02f9cb-42e8-467c-a242-d9bb2e1918a0}|401|0|#107
The meaning of the fields are as follows:
- Default.rtf - the default .RTF file
- {ab02f9cb-42e8-467c-a242-d9bb2e1918a0} - same as CLSID_CodeXLVSPackagePackage
- #106 - the literal value of IDS_EDITOR_NAME in CodeXLVSPackageUI.rc,
which is displayed under the icon in the new file dialog.
- 80 - the display ordering priority
- #109 - the literal value of IDS_FILE_DESCRIPTION in CodeXLVSPackageUI.rc, which is displayed
in the description window in the new file dialog.
- {ab02f9cb-42e8-467c-a242-d9bb2e1918a0} - resource dll package guid
- 401 - the literal value of IDI_FILE_ICON in CodeXLVSPackage.rc (not CodeXLVSPackageUI.rc),
which is the icon to display in the new file dialog.
- 0 - template flags, which are unused here(we don't use this - see vsshell.idl)
- #107 - the literal value of IDS_DEFAULT_NAME in CodeXLVSPackageUI.rc, which is the base
name of the new files (i.e. myext1.myext, myext2.myext, etc.).

***************************************************************************/

class ATL_NO_VTABLE CodeXLVSPackage :
// CComObjectRootEx and CComCoClass are used to implement a non-thread safe COM object, and
// a partial implementation for IUknown (the COM map below provides the rest).
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CodeXLVSPackage, &CLSID_CodeXLVSPackage>,
// Provides the implementation for IVsPackage to make this COM object into a VS Package.
    public IVsPackageImpl<CodeXLVSPackage, &CLSID_CodeXLVSPackage>,
    public IOleCommandTargetImpl<CodeXLVSPackage>,
// Provides consumers of this object with the ability to determine which interfaces support
// extended error information.
    public ATL::ISupportErrorInfoImpl<&__uuidof(IVsPackage)>,
    public IProgressBarEventHandler,
    public IVscApplicationCommandsOwner,
    public IVscSourceCodeViewerOwner,
    public IVscDebugEngineOwner,
    public IVscBreakpointsManagerOwner,
    public IVscEventObserverOwner,
    public IVscWindowsManagerOwner,
    public IVscGRApiFunctionsOwner
{
public:

    // Provides a portion of the implementation of IUnknown, in particular the list of interfaces
    // the CCodeXLVSPackagePackage object will support via QueryInterface
    BEGIN_COM_MAP(CodeXLVSPackage)
    COM_INTERFACE_ENTRY(IVsPackage)
    COM_INTERFACE_ENTRY(IOleCommandTarget)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    END_COM_MAP()

    // COM objects typically should not be cloned, and this prevents cloning by declaring the
    // copy constructor and assignment operator private (NOTE:  this macro includes the decleration of
    // a private section, so everything following this macro and preceding a public or protected
    // section will be private).
    VSL_DECLARE_NOT_COPYABLE(CodeXLVSPackage)

public:
    CodeXLVSPackage();
    ~CodeXLVSPackage();

    // This method will be called after IVsPackage::SetSite is called with a valid site
    void PostSited(IVsPackageEnums::SetSiteResult /*result*/);
    void PreClosing();

    // This function provides the error information if it is not possible to load
    // the UI dll. It is for this reason that the resource IDS_E_BADINSTALL must
    // be defined inside this dll's resources.
    static const LoadUILibrary::ExtendedErrorInfo& GetLoadUILibraryErrorInfo();

    // DLL is registered with VS via a pkgdef file. Don't do anything if asked to
    // self-register.
    static HRESULT WINAPI UpdateRegistry(BOOL bRegister);

    bool createDialogBasedAssertionFailureHandler();
    void TriggerClockTick();

    // NOTE - the arguments passed to these macros can not have names longer then 30 characters
    // Definition of the commands handled by this package
    //Limit of 31 commands per map
    VSL_BEGIN_COMMAND_MAP()
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidLaunchOpenCLDebugging, CommandHandler::QueryStatusHandler(&onUpdateLaunchOpenCLDebugging), CommandHandler::ExecHandler(&OnStartButton))

    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidConfigureRemoteHost, CommandHandler::QueryStatusHandler(&OnUpdateConfigureRemoteHost), CommandHandler::ExecHandler(&OnConfigureRemoteHost))
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidStartButtonOnToolbar, CommandHandler::QueryStatusHandler(&OnUpdateStartButton), CommandHandler::ExecHandler(&OnStartButton))

    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidDebugMode, CommandHandler::QueryStatusHandler(&onUpdateMode), CommandHandler::ExecHandler(&onModeClicked))
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidProfileMode, CommandHandler::QueryStatusHandler(&onUpdateMode), CommandHandler::ExecHandler(&onModeClicked))
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidFrameAnalysis, CommandHandler::QueryStatusHandler(&onUpdateMode), CommandHandler::ExecHandler(&onModeClicked))
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidAnalyzeMode, CommandHandler::QueryStatusHandler(&onUpdateMode), CommandHandler::ExecHandler(&onModeClicked))
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidCodeXLProfileDropdownMenu, CommandHandler::QueryStatusHandler(&onUpdateMode), CommandHandler::ExecHandler(&onModeClicked))

    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidAPIStep, CommandHandler::QueryStatusHandler(&onUpdateFrameStep), CommandHandler::ExecHandler(&onAPIStep))
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidFrameStep, CommandHandler::QueryStatusHandler(&onUpdateFrameStep), CommandHandler::ExecHandler(&onFrameStep))
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidDrawStep, CommandHandler::QueryStatusHandler(&onUpdateFrameStep), CommandHandler::ExecHandler(&onDrawStep))
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidGDStepInto, CommandHandler::QueryStatusHandler(&onUpdateStepInto), CommandHandler::ExecHandler(&onStepInto))
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidGDStepOver, CommandHandler::QueryStatusHandler(&onUpdateStepOver), CommandHandler::ExecHandler(&onStepOver))
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidGDStepOut, CommandHandler::QueryStatusHandler(&onUpdateStepOut), CommandHandler::ExecHandler(&onStepOut))


    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidCodeXLExplorer, NULL, CommandHandler::ExecHandler(&onCodeXLExplorerView))
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidCallsHistoryViewer, NULL, CommandHandler::ExecHandler(&onCallsHistoryView))
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidPropertiesView, NULL, CommandHandler::ExecHandler(&onPropertiesView))
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidMemoryView, NULL, CommandHandler::ExecHandler(&onMemoryView))
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidStateVariablesView, NULL, CommandHandler::ExecHandler(&onStateVariablesView))
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidStatisticsView, NULL, CommandHandler::ExecHandler(&onStatisticsView))

    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidOpenCLBreakpoints, CommandHandler::QueryStatusHandler(&onUpdateOpenCLBreakpoints), CommandHandler::ExecHandler(&onOpenCLBreakpoints))
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidCodeXLDebuggingBreakpointsMenu, CommandHandler::QueryStatusHandler(&onUpdateOpenCLBreakpoints), NULL)
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidAddOpenCLMultiWatchFromSourceCode, CommandHandler::QueryStatusHandler(&onUpdateMultiWatchViews), CommandHandler::ExecHandler(&onAddKernelMultiWatchFromSourceCode))
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidAddOpenCLMultiWatchFromLocalsView, CommandHandler::QueryStatusHandler(&onUpdateMultiWatchViews), CommandHandler::ExecHandler(&onAddKernelMultiWatchFromLocalsView))
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidAddOpenCLMultiWatchFromWatchView, CommandHandler::QueryStatusHandler(&onUpdateMultiWatchViews), CommandHandler::ExecHandler(&onAddKernelMultiWatchFromWatchView))
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidOpenCLMultiWatch1, CommandHandler::QueryStatusHandler(&onUpdateMultiWatchViews), CommandHandler::ExecHandler(&onAddKernelMultiWatch1))
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidOpenCLMultiWatch2, CommandHandler::QueryStatusHandler(&onUpdateMultiWatchViews), CommandHandler::ExecHandler(&onAddKernelMultiWatch2))
    VSL_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidOpenCLMultiWatch3, CommandHandler::QueryStatusHandler(&onUpdateMultiWatchViews), CommandHandler::ExecHandler(&onAddKernelMultiWatch3))


    VSL_END_VSCOMMAND_MAP()

    // The tool map implements IVsPackage::CreateTool that is called by VS to create a tool window
    // when appropriate.
    VSL_BEGIN_TOOL_MAP()
    VSL_TOOL_ENTRY(CLSID_VSPCallsHistoryPersistanceId, onCallsHistoryView(NULL, 0, NULL, NULL))
    VSL_TOOL_ENTRY(CLSID_VSPPropertiesPersistanceId, onPropertiesView(NULL, 0, NULL, NULL))
    VSL_TOOL_ENTRY(CLSID_VSPMemoryPersistanceId, onMemoryView(NULL, 0, NULL, NULL))
    VSL_TOOL_ENTRY(CLSID_VSPStateVariablesPersistanceId, onStateVariablesView(NULL, 0, NULL, NULL))
    VSL_TOOL_ENTRY(CLSID_VSPObjectsExplorerPersistanceId,  onCodeXLExplorerView(NULL, 0, NULL, NULL))
    VSL_TOOL_ENTRY(CLSID_VSPStatisticsPersistanceId, onStatisticsView(NULL, 0, NULL, NULL))
    VSL_TOOL_ENTRY(CLSID_VSPMultiwatch1PersistanceId, onAddKernelMultiWatch1(NULL, 0, NULL, NULL))
    VSL_TOOL_ENTRY(CLSID_VSPMultiwatch2PersistanceId, onAddKernelMultiWatch2(NULL, 0, NULL, NULL))
    VSL_TOOL_ENTRY(CLSID_VSPMultiwatch3PersistanceId, onAddKernelMultiWatch3(NULL, 0, NULL, NULL))
    VSL_END_TOOL_MAP()

    // Command handler called when the user selects the "Start OpenCL Debugging" command:
    void OnStartButton(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);

    void executeProfileSession();
    void executeDebugSession(bool isProjectTypeValid);
    void executeFrameAnalysis();
    void buildOpenCLFile();

    void updateProjectSettingsFromVS(bool& shouldDebug, bool& shouldProfile, bool& shouldFrameAnalyze, bool& isProjectTypeValid);

    // Query status handler called when the IDE wants to update the "Start OpenCL Debugging" command:
    void onUpdateLaunchOpenCLDebugging(CommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText);

    void OnUpdateConfigureRemoteHost(CommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText);
    void OnUpdateStartButton(CommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText);

    void OnConfigureRemoteHost(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);

    void onUpdateLaunchDebugAction(bool& isActionEnabled, bool& isActionChecked);
    void onUpdateLaunchProfileAction(bool& isActionEnabled, bool& isActionChecked);
    void onUpdateLaunchFrameAnalysisAction(bool& isActionEnabled, bool& isActionChecked);

    // Command handler called when the user selects the "Step Into" command:
    void onStepInto(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);

    // Query status handler called when the IDE wants to update the "Step Into" command:
    void onUpdateStepInto(CommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText);

    // Command handler called when the user selects the "Step Over" command:
    void onStepOver(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    void onStepOut(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);

    void onDrawStep(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    void onFrameStep(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    void onAPIStep(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);

    // Query status handler called when the IDE wants to update the "Step Over" command:
    void onUpdateStepOver(CommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText);
    void onUpdateStepOut(CommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText);
    void onUpdateFrameStep(CommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText);

    // the following functions are command handlers called when the user selects the command to show the one of our tool windows:
    void onCallsHistoryView(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    void onCodeXLExplorerView(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    void onPropertiesView(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    void onMemoryView(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    void onStatisticsView(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);

    void onStateVariablesView(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    void onOpenCLBreakpoints(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    void onUpdateOpenCLBreakpoints(CommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText);
    void onAddKernelMultiWatchFromSourceCode(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    void onAddKernelMultiWatchFromLocalsView(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    void onAddKernelMultiWatchFromWatchView(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    void onAddKernelMultiWatch1(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    void onAddKernelMultiWatch2(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    void onAddKernelMultiWatch3(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    void onUpdateMultiWatchViews(CommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText);
    void onUpdateMode(CommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText);
    void onModeClicked(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);
    void onUpdateProfileSessionType(CommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText);
    void onProfileSessionType(CommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut);

    // Utilities:
    bool setToolWindowCaption(int windowId, const std::wstring& windowCaption);
    void verifyBaseViewsCreated();
    void openMultiWatchView(int multiWatchWindowIndex, const std::wstring& variableName = L"");

    const WCHAR* getVSRegistryRootPath(bool tempRegPath);
    bool validateDebugSettings(const std::wstring& executableFilePath, const std::wstring& workingDirectoryPath, bool isProjectTypeValid);
    bool validateStartDebugAfterBuild();
    bool registerLanguageServices(const std::wstring& registryRoot);
    void createSendErrorReportDialog();
    void initializeDebuggerPlugin();

    // Used by vspPackageWrapper:
    void onNewDebugEngine(void* pNewDebugEngine);

    // Region: IVscApplicationCommandsOwner - begin.
    virtual void CloseDocumentsOfDeletedFiles() const;
    virtual bool SaveFileWithPath(const wchar_t* pFilePathStr) const;
    virtual bool GetFunctionBreakpoints(wchar_t**& pEnabledFunctionBreakpointsBuffer, int& enabledFuncBreakpointsCount, wchar_t**& pDisabledFunctionBreakpointsBuffer, int& disabledFuncBreakpointsCount) const;
    virtual bool CloseFile(const wchar_t* pFilePath) const;
    virtual bool OpenFileAtPosition(const wchar_t* pFilePath, int lineNumber = 0, int columnNumber = 0, bool selectLine = true) const;
    virtual void DeleteWcharStrBuffers(wchar_t**& pBuffersArray, int buffersArraySize) const;
    virtual bool RaiseStatisticsView();
    virtual bool RaiseMemoryView();
    virtual void SetToolWindowCaption(int windowId, const wchar_t* windowCaption);
    virtual void ClearBuildPane();
    /// \param messageString message to display
    /// \param outputOnlyToLog
    /// \param filePathAndName when build error message is connected to error, this string represents the file were the error is located
    /// \param line when build error message is connected to error, this int represents the error line
    virtual void OutputBuildMessage(const wchar_t* pMsgToPrint, bool outputOnlyToLog, const wchar_t* pfile, int line);
    virtual void UpdateProjectSettingsFromStartupProject();
    virtual void ivacoOwnerDeleteWCharStr(wchar_t*& pStr);
    // Region: IVscApplicationCommandsOwner - end.

    // Region: IVscSourceCodeViewOwner - begin.
    virtual bool scvOwnerGetWindowFromFilePath(const wchar_t* pFilePath, void*& pWindowBuffer) const;
    virtual bool scvOwnerCloseAndReleaseWindow(void*& pWindow, bool closeWindow = true) const;
    virtual bool scvOwnerOpenFileAtPosition(const wchar_t* pFilePath, int lineNumber = 0, int columnNumber = 0, bool selectLine = true) const;
    // Region: IVscSourceCodeViewOwner - end.

    // Region: IVscDebugEngineOwner - Begin.
    virtual bool IsAnyOpenedFileModified() const;
    virtual void ClearOpenFiles() const;
    virtual void InformPackageOfNewDebugEngine(void* pNewEngine) const;
    // Region: IVscDebugEngineOwner - End.

    // Region: IVscBreakpointsManagerOwner - Begin.
    virtual bool IsSrcLocationBreakpointEnabled(const wchar_t* filePath, int lineNumber, bool& isEnabled) const;
    virtual bool IsFuncBreakpointEnabled(const wchar_t* functionName, bool& isEnabled) const;
    virtual bool DisableFuncBreakpoint(const wchar_t* functionName) const;
    virtual bool AddBreakpointInSourceLocation(const wchar_t* filePath, int lineNumber, bool enabled) const;
    virtual bool RemoveBreakpointsInSourceLocation(const wchar_t* filePath, int lineNumber) const;
    // Region: IVscBreakpointsManagerOwner - End.

    // Region: IVscEventObserverOwner - Begin.
    virtual void CloseDisassemblyWindow() const;
    virtual void ForceVariablesReevaluation() const;
    virtual void ClearMessagePane() const;
    virtual void OutputMessage(const wchar_t* pMsgBuffer, bool outputOnlyToLog);
    virtual void DeleteWCharStringBuffer(wchar_t*& pBuffer);
    // Region: IVscEventObserverOwner - End.

    // Region: IVscWindowsManagerOwner - Begin.
    virtual void SaveChangedFiles(bool saveSolutionAndProject) const;
    virtual bool OpenSolution(const wchar_t* solutionName) const;
    virtual bool GetVsWindowsManagementMode(VsWindowsManagementMode& buffer) const;
    virtual bool ivwmoVerifyBaseViewsCreated();
    // Region: IVscWindowsManagerOwner - Begin.

    // Region: IProgressBarEventHandler - Begin.
    virtual void SetProgressInfo(const wchar_t* pProgBarLabel, bool isInProgress, unsigned long progBarComplete, unsigned long progBarRange);
    virtual bool ShouldUpdateProgress();
    // Region: IProgressBarEventHandler - End.

    // Region: IVscGRApiFunctionsOwner - Begin.
    virtual void SetHexDisplayMode();
    // Region: IVscGRApiFunctionsOwner - End.

private:
    void checkgDEBBugerInstallation();
    void registerDebugEngineInTemporaryHive();
    void unregisterDebugEngineFromTemporaryHive();

private:
    // A handle to the core implementation.
    void* m_pCoreImpl;

    // Is the site cache for the package valid?
    bool m_isSited;

    // Define an object that would handle the package commands:
    vspPackageCommandHandler* _pPackageCommandHandler;

    // Cookie returned when registering editor
    VSCOOKIE m_dwEditorCookie;
    VSCOOKIE m_dwProfileSessionEditorCookie;
    VSCOOKIE m_dwKernelAnalyzerEditorCookie;
    VSCOOKIE _oleCommandTargetCookie1;
    VSCOOKIE _oleCommandTargetCookie2;

    enum vspWhenOutOfDateFlags
    {
        VSP_ALWAYS_BUILD = 1,
        VSP_NEVER_BUILD = 2,
        VSP_PROMPT_TO_BUILD = 4
    };

    enum vspBeforeBuildFlags
    {
        VSP_SAVE_ALL = 0,
        VSP_PROMPT_TO_SAVE = 1,
        VSP_DONT_SAVE = 2,
        VSP_SAVE_CHANGE_TO_OPEN_ONLY = 3
    };

    enum vspWhenFailBuildFlags
    {
        VSP_LAUNCH_OLD_VERSION = 1,
        VSP_DO_NOT_LAUNCH = 2,
        VSP_PROMPT_TO_LAUNCH = 4
    };

private:
    // This is vspProgressBar's member which was relocated here.
    // It should be passed to it when providing the VS logic to vspProgressBar.
    VSCOOKIE _userCookie;

    // The tool windows for CodeXL views:
    vspToolWindow* _pCallsHistoryToolWindow;
    vspToolWindow* _pPropertiesToolWindow;
    vspToolWindow* _pObjectsExplorerToolWindow;
    vspToolWindow* _pStatisticsToolWindow;
    vspToolWindow* _pMemoryToolWindow;
    vspToolWindow* _pStateVariablesToolWindow;
    vspToolWindow* _pMultiWatchToolWindows[VSP_AMOUNT_OF_MULTIWATCH_VIEWS];

    unsigned int m_installedComponentsBitmask;

    // A pointer to the vspTimer object
    vspTimer* m_pTimer;
};

// This exposes CCodeXLVSPackagePackage for instantiation via DllGetClassObject; however, an instance
// can not be created by CoCreateInstance, as CCodeXLVSPackagePackage is specifically registered with
// VS, not the the system in general.
OBJECT_ENTRY_AUTO(CLSID_CodeXLVSPackage, CodeXLVSPackage)
