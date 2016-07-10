//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscApplicationCommands.cpp
///
//==================================================================================

//------------------------------ vspApplicationCommands.cpp ------------------------------
#include "stdafx.h"

// Local:
#include <src/vscApplicationCommands.h>

// Qt:
#include <QtWidgets>

// C++:
#include <string>
#include <sstream>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/apExecutionMode.h>
#include <AMDTAPIClasses/Include/apKernelFunctionNameBreakpoint.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionBreakPoint.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionsManager.h>
#include <AMDTAPIClasses/Include/Events/apMDIViewCreateEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acSourceCodeView.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTOSWrappers/Include/osCallStack.h>
#include <AMDTOSWrappers/Include/osCallStackFrame.h>
#include <AMDTOSWrappers/Include/osDirectory.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afSaveProjectCommand.h>

// AMDTGpuDebuggingComponents:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdHTMLProperties.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdUpdateUIEvent.h>
#include <AMDTGpuDebuggingComponents/Include/dialogs/gdBreakpointsDialog.h>

// CPU Profile:
#include <AMDTCpuProfiling/Inc/AmdtCpuProfiling.h>
#include <AMDTCpuProfiling/inc/SessionViewCreator.h>

// GPU Profile:
#include <AMDTGpuProfiling/gpViewsCreator.h>

// Kernel analyzer:
#include <AMDTKernelAnalyzer/src/kaBackendManager.h>
#include <AMDTKernelAnalyzer/src/kaKernelView.h>
#include <AMDTKernelAnalyzer/src/kaMultiSourceView.h>
#include <AMDTKernelAnalyzer/src/kaSourceCodeView.h>

// Local:
#include <CodeXLVSPackage/Include/vspStringConstants.h>
#include <src/vspGRApiFunctions.h>
#include <src/vspSourceCodeViewer.h>
#include <src/vspUtils.h>
#include <src/vspWindowsManager.h>
#include <src/vspQTWindowPaneImpl.h>
#include <Include/Public/vscWindowsManager.h>
#include <Include/Public/vscUtils.h>
#include <Include/Public/vscCoreUtils.h>
#include <Include/Public/vscDTEConnector.h>
#include <Include/Public/vscVspDTEInvoker.h>
#include <Include/Public/vscEditorDocument.h>


// Static data members initializations.
IVscApplicationCommandsOwner* vscApplicationCommands::m_pOwner = NULL;

// ---------------------------------------------------------------------------
// Name:        vspApplicationCommands::vspApplicationCommands
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        7/2/2011
// ---------------------------------------------------------------------------
vscApplicationCommands::vscApplicationCommands()
{
    // clear stages paths map
    m_stagePathMap.clear();

    // Get the binary directory.
    osFilePath cxlBinDirPath;
    bool isOk = cxlBinDirPath.SetInstallRelatedPath(osFilePath::OS_CODEXL_BINARIES_PATH);
    GT_ASSERT_EX(isOk, L"Failed to extract CodeXL binaries directory.");

    if (cxlBinDirPath.exists())
    {
        // Add the DX search path.
        kaBackendManager::AddDxBinSearchPath(cxlBinDirPath.asString());
    }

    kaBackendManager& theBackendManager = kaBackendManager::instance();
    bool rc = connect(&theBackendManager, SIGNAL(printMessageForUser(const QString&)), this, SLOT(PrintToDebugLog(const QString&)));
    GT_ASSERT(rc);
}

// ---------------------------------------------------------------------------
// Name:        vspApplicationCommands::~vspApplicationCommands
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        7/2/2011
// ---------------------------------------------------------------------------
vscApplicationCommands::~vscApplicationCommands() {}


// ---------------------------------------------------------------------------
// Name:        vspApplicationCommands::openBreakpointsDialog
// Description: Open the breakpoints dialog
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        7/2/2011
// ---------------------------------------------------------------------------
bool vscApplicationCommands::openBreakpointsDialog()
{
    bool retVal = false;

    // Perform the command only if it is enabled:
    if (isBreakpointsDialogCommandEnabled())
    {
        // Set up the API so the breakpoints dialog will show the correct data:
        bool isProcessAlreadyRunning = gaDebuggedProcessExists();

        if (!isProcessAlreadyRunning)
        {
            updateAPIBreakpointsForBreakpointsDialog();
        }

        // Get the monitored objects tree (should be the application top level window):
        // vspMonitoredObjectsTree* pMonitoredObjectsTree = vspWindowsManager::instance().monitoredObjectsTree(NULL, wxDefaultSize);

        // Load the Breakpoints dialog
        gdBreakpointsDialog dialog(NULL);

        vspWindowsManager::instance().showModal(&dialog);
        retVal = true;

        // Clear up the breakpoints we created, we only want real API breakpoints to be present when the debugger wants them to be bound:
        if (!isProcessAlreadyRunning)
        {
            // Note this is to clean up the API, since even if the gaRemoveAllBreakpoints function was called,
            // our debug engine and breakpoint interfaces did not get the change since they're not running:
            vspGRApiFunctions::vspInstance().removeAllAPIBreakpoints();
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdaApplicationCommands::isBreakpointsDialogCommandEnabled
// Description: Return true if the breakpoints dialog command is enabled
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        7/2/2011
// ---------------------------------------------------------------------------
bool vscApplicationCommands::isBreakpointsDialogCommandEnabled()
{
    bool retVal = false;

    if (afExecutionModeManager::instance().isActiveMode(GD_STR_executionMode))
    {
        // Get current execution mode;
        apExecutionMode currentExecMode = AP_DEBUGGING_MODE;
        gaGetDebuggedProcessExecutionMode(currentExecMode);

        retVal = true;

        // Disable event for profiling mode:
        if (currentExecMode == AP_PROFILING_MODE)
        {
            retVal = false;
        }
    }

    return retVal;
}

bool vscApplicationCommands::OpenFileAtLine(const osFilePath& filePath, int lineNumber, int programCounterIndex, int viewIndex)
{
    GT_UNREFERENCED_PARAMETER(programCounterIndex);

    bool retVal = false;

    // Set a creation event for the CPU / GPU views creators:
    gtString ext;
    filePath.getFileExtension(ext);

    if (ext == AF_STR_CpuProfileFileExtension)
    {
        // Trigger a CPU Profile view creation event:
        apMDIViewCreateEvent* pCpuProfileViewEvent = new apMDIViewCreateEvent(AF_STR_CPUProfileViewsCreatorID, filePath, AF_STR_Empty, viewIndex, lineNumber);

        apEventsHandler::instance().registerPendingDebugEvent(*pCpuProfileViewEvent);

        SessionViewCreator* pSessionViewCreator = AmdtCpuProfiling::sessionViewCreator();
        GT_IF_WITH_ASSERT(pSessionViewCreator != NULL)
        {
            pSessionViewCreator->setCreationEvent(pCpuProfileViewEvent);
            pSessionViewCreator->displayOpenSession(filePath, lineNumber);
        }
    }
    else if (ext == AF_STR_GpuProfileTraceFileExtension || ext == AF_STR_GpuProfileSessionFileExtension)
    {
        // Trigger a GPU Profile view creation event:
        apMDIViewCreateEvent* pGpuProfileViewEvent = new apMDIViewCreateEvent(AF_STR_GPUProfileViewsCreatorID, filePath, AF_STR_Empty, viewIndex, lineNumber);

        apEventsHandler::instance().registerPendingDebugEvent(*pGpuProfileViewEvent);

        // Get the session view creator:
        gpViewsCreator* pCreator = gpViewsCreator::Instance();
        GT_IF_WITH_ASSERT(pCreator != NULL)
        {
            pCreator->setCreationEvent(pGpuProfileViewEvent);
        }
    }
    else if (ext == AF_STR_PowerProfileSessionFileExtension)
    {
        // Trigger a GPU Profile view creation event:
        apMDIViewCreateEvent* pGpuProfileViewEvent = new apMDIViewCreateEvent(AF_STR_PowerProfileViewsCreatorID, filePath, AF_STR_Empty, viewIndex, lineNumber);
        apEventsHandler::instance().registerPendingDebugEvent(*pGpuProfileViewEvent);

        // Get the session view creator:
        gpViewsCreator* pCreator = gpViewsCreator::Instance();
        GT_IF_WITH_ASSERT(pCreator != NULL)
        {
            pCreator->setCreationEvent(pGpuProfileViewEvent);
        }
    }

    // Open the file in Visual Studio. Select the line if this is a kernel (i.e.: we have a line number):
    bool selectLine = (lineNumber > 0);

    GT_IF_WITH_ASSERT(m_pOwner != NULL)
    {
        retVal = m_pOwner->OpenFileAtPosition(filePath.asString().asCharArray(), lineNumber, 0, selectLine);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        closeFile
// Description: Closes the specified file if it is open
// Arguments:   osFilePath filePath
// Return Val:  bool - Success / failure.
// Author:      Chris Hesik
// Date:        31/5/2012
// ---------------------------------------------------------------------------
bool vscApplicationCommands::closeFile(const osFilePath& filePath)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pOwner != NULL)
    {
        ret = m_pOwner->CloseFile(filePath.asString().asCharArray());
    }
    return ret;
}

// ---------------------------------------------------------------------------
// Name:        vspApplicationCommands::updateAPIBreakpointsForBreakpointsDialog
// Description: Before showing the breakpoints dialog, called to update the API
//              breakpoints (if the debug engine hasn't bound them).
// Author:      Uri Shomroni
// Date:        10/2/2011
// ---------------------------------------------------------------------------
void vscApplicationCommands::updateAPIBreakpointsForBreakpointsDialog()
{
    // First, clear any API breakpoints remaining from previous runs:
    bool rcRem = vspGRApiFunctions::vspInstance().removeAllAPIBreakpoints();
    GT_ASSERT(rcRem);

    GT_IF_WITH_ASSERT(m_pOwner != NULL)
    {
        // Get all the breakpoint function names:
        wchar_t** enabledBreakpointNames = NULL;
        wchar_t** disabledBreakpointNames = NULL;

        int enabledBreakpointNamesSize = 0;
        int disabledBreakpointNamesSize = 0;

        bool rcNm = m_pOwner->GetFunctionBreakpoints(enabledBreakpointNames, enabledBreakpointNamesSize, disabledBreakpointNames, disabledBreakpointNamesSize);
        GT_ASSERT(rcNm);

        for (int i = 0; i < enabledBreakpointNamesSize; i++)
        {
            bool rcAddCurrent;

            // Add the current breakpoint:
            rcAddCurrent = updateAPISingleBreakpoint(enabledBreakpointNames[i], true);
        }

        for (int i = 0; i < disabledBreakpointNamesSize; i++)
        {
            bool rcAddCurrent;

            // Add the current breakpoint:
            rcAddCurrent = updateAPISingleBreakpoint(disabledBreakpointNames[i], false);
        }

        // Release the buffers.
        m_pOwner->DeleteWcharStrBuffers(enabledBreakpointNames, enabledBreakpointNamesSize);
    }
}

// ---------------------------------------------------------------------------
// Name:        vspApplicationCommands::updateAPISingleBreakpoint
// Description: Update a single breakpoint to the API
// Arguments:   breakpointName - the breakpoint name
//              isEnabled - is the breakpoint enabled
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        11/7/2011
// ---------------------------------------------------------------------------
bool vscApplicationCommands::updateAPISingleBreakpoint(const gtString& breakpointName, bool isEnabled)
{
    bool retVal = false;

    // Static variables for string comparison:
    static const gtString kernelFunctionNameBreakpointPrefix = GD_STR_KernelFunctionNameBreakpointPrefix;
    static const int kernelFunctionNameBreakpointPrefixLength = kernelFunctionNameBreakpointPrefix.length();

    // Check if it is a monitored function breakpoint:
    apMonitoredFunctionId currentFuncId = apMonitoredFunctionsManager::instance().monitoredFunctionId(breakpointName.asCharArray());

    if (currentFuncId < apMonitoredFunctionsAmount)
    {
        // Add it to the API:
        apMonitoredFunctionBreakPoint currentFunctionBreakpoint(currentFuncId);
        currentFunctionBreakpoint.setEnableStatus(isEnabled);
        retVal = vspGRApiFunctions::vspInstance().setAPIBreakpoint(currentFunctionBreakpoint);
        GT_ASSERT(retVal);
    }
    else
    {
        // Try to convert the breakpoint to a generic breakpoint:
        apGenericBreakpointType breakpointType = AP_BREAK_TYPE_UNKNOWN;
        bool isGeneric = apGenericBreakpoint::breakpointTypeFromString(breakpointName, breakpointType);

        if (isGeneric)
        {
            // Add it to the API:
            apGenericBreakpoint currentGenericBreakpoint(breakpointType);
            currentGenericBreakpoint.setEnableStatus(isEnabled);
            retVal = vspGRApiFunctions::vspInstance().setAPIBreakpoint(currentGenericBreakpoint);
            GT_ASSERT(retVal);
        }
        else if (breakpointName.startsWith(kernelFunctionNameBreakpointPrefix))
        {
            // If this is a kernel function name:
            gtString kernelFuncName = breakpointName;
            kernelFuncName.truncate(kernelFunctionNameBreakpointPrefixLength, -1);

            // Add it to the API:
            apKernelFunctionNameBreakpoint currentFunctionNameBreakpoint(kernelFuncName);
            currentFunctionNameBreakpoint.setEnableStatus(isEnabled);
            retVal = vspGRApiFunctions::vspInstance().setAPIBreakpoint(currentFunctionNameBreakpoint);
            GT_ASSERT(retVal);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspApplicationCommands::showModal
// Description: qt
// Arguments:   QDialog* pDialog
// Return Val:  int
// Author:      Yoni Rabin
// Date:        31/05/2012
// ---------------------------------------------------------------------------
int vscApplicationCommands::showModal(QDialog* pDialog)
{
    int retVal = QDialog::Accepted;

    GT_IF_WITH_ASSERT(pDialog != NULL)
    {
        retVal = vspWindowsManager::instance().showModal(pDialog);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
void vscApplicationCommands::updateToolbarCommands()
{
    // Send an update UI event to be handled in the main thread:
    gdUpdateUIEvent updateUIEve;
    apEventsHandler::instance().registerPendingDebugEvent(updateUIEve);
}

// ---------------------------------------------------------------------------
// Name:        vspApplicationCommands::setApplicationCaption
// Description: Set the application caption
// Arguments:   const gtString& caption
// Author:      Sigal Algranaty
// Date:        2/8/2011
// ---------------------------------------------------------------------------
void vscApplicationCommands::setApplicationCaption(const gtString& caption)
{
    GT_UNREFERENCED_PARAMETER(caption);

    // CodeXL does not change the VS app title caption. Nothing to do here.
}
// ---------------------------------------------------------------------------
// Name:        vspApplicationCommands::displayOpenCLProgramSourceCode
// Description: The VS implementation for program source
// Arguments:   afApplicationTreeItemData* pProgramItemData
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        27/7/2011
// ---------------------------------------------------------------------------
void vscApplicationCommands::displayOpenCLProgramSourceCode(afApplicationTreeItemData* pProgramItemData)
{
    // Display the selected kernel / program source code:
    vspSourceCodeViewer::instance().displayOpenCLProgramSourceCode(pProgramItemData);
}

// ---------------------------------------------------------------------------
// Name:        vspApplicationCommands::displayOpenGLSLShaderCode
// Description: The VS implementation for shader source
// Arguments:   afApplicationTreeItemData* pShaderItemData
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        27/7/2011
// ---------------------------------------------------------------------------
void vscApplicationCommands::displayOpenGLSLShaderCode(afApplicationTreeItemData* pShaderItemData)
{
    vspSourceCodeViewer::instance().displayOpenGLSLShaderCode(pShaderItemData);
}

// ---------------------------------------------------------------------------
// Name:        vspApplicationCommands::displayImageBufferObject
// Description: Display an image / buffer object in VS
// Arguments:   gdDebugApplicationTreeData* pItemData
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        27/7/2011
// ---------------------------------------------------------------------------
bool vscApplicationCommands::displayImageBufferObject(afApplicationTreeItemData* pItemData, const gtString& itemText)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(pItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            int objectName;

            // Create a text file with the description of the current buffer / image object:
            gtString fileName;
            objectName = (pGDData->_contextId.isOpenGLContext()) ? pGDData->_objectOpenGLName : pGDData->_objectOpenCLName;
            bool rcFileName = gdHTMLProperties::objectDataToHTMLLink(*pItemData, -1, fileName);
            GT_IF_WITH_ASSERT(rcFileName)
            {
                // Build the file path:
                // Get the User AppData directory
                osFilePath imageObjectsFilePath;
                imageObjectsFilePath.setPath(osFilePath::OS_USER_APPLICATION_DATA);

                // Add the application directory to it
                imageObjectsFilePath.appendSubDirectory(afGlobalVariablesManager::ProductName());

                // Add the VS_Cache files directory:
                imageObjectsFilePath.appendSubDirectory(_T(VSP_STR_VSCacheFolderName));

                // Get the project & debugged application name:
                gtString projectName;
                osFilePath currentProject = afProjectManager::instance().currentProjectSettings().executablePath();
                currentProject.getFileName(projectName);

                // Create the folder if not created:
                osDirectory directoryPath;
                directoryPath.setDirectoryPath(imageObjectsFilePath);
                bool rcCreateDir = directoryPath.create();
                GT_IF_WITH_ASSERT(rcCreateDir)
                {
                    // Add the VS_Cache files directory:
                    imageObjectsFilePath.appendSubDirectory(projectName);
                    directoryPath.setDirectoryPath(imageObjectsFilePath);
                    directoryPath.create();

                    // Write the files to the cache folder:
                    imageObjectsFilePath.setFileName(fileName);
                    imageObjectsFilePath.setFileExtension(AF_STR_CodeXMLImageBuffersFilesExtension);

                    // Just save the file:
                    osFile objectfile;
                    retVal = objectfile.open(imageObjectsFilePath, osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);
                    GT_IF_WITH_ASSERT(retVal)
                    {
                        // Append the context string to the item string:
                        gtString itemNameWithContext;
                        pGDData->_contextId.toString(itemNameWithContext);

                        // Append the context as string to the item name:
                        itemNameWithContext.appendFormattedString(L"%ls ", itemText.asCharArray());

                        // Write the string to the file:
                        objectfile << itemNameWithContext;

                        // Only save the file without any cont3
                        objectfile.close();

                        // Open the file in Visual Studio. Select the line if this is a kernel (i.e.: we have a line number):
                        OpenFileAtLine(imageObjectsFilePath, -1, false);
                    }
                }
            }
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspApplicationCommands::raiseStatisticsView
// Description: Raise statistics view through the package commands
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        31/7/2011
// ---------------------------------------------------------------------------
bool vscApplicationCommands::raiseStatisticsView()
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(m_pOwner != NULL)
    {
        retVal = m_pOwner->RaiseStatisticsView();
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspApplicationCommands::raiseMemoryView
// Description: Raise memory view through the package commands
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        31/7/2011
// ---------------------------------------------------------------------------
bool vscApplicationCommands::raiseMemoryView()
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(m_pOwner != NULL)
    {
        // Open the memory view:
        m_pOwner->RaiseMemoryView();
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspApplicationCommands::setWindowCaption
// Description: Set the requested window caption
// Arguments:   wxWindow* pWindow - the window handle
//              const gtString& windowCaption - the requested caption
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        21/8/2011
// ---------------------------------------------------------------------------
bool vscApplicationCommands::setWindowCaption(QWidget* pWidget, const gtString& windowCaption)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(pWidget != NULL)
    {
        // Get the window id from the widget pointer:
        int windowId = vspWindowsManager::instance().commandIdFromWidget(pWidget);

        GT_IF_WITH_ASSERT(m_pOwner != NULL)
        {
            // Set the caption for the tool window:
            m_pOwner->SetToolWindowCaption(windowId, windowCaption.asCharArray());
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        vspApplicationCommands::showMessageBox
// Description: VSP message box implementation
// Arguments:   const gtString& caption
//              const gtString& message
//              osMessageBoxIcon icon /*= osMessageBox::OS_DISPLAYED_INFO_ICON*/
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        30/8/2011
// ---------------------------------------------------------------------------
void vscApplicationCommands::showMessageBox(const gtString& caption, const gtString& message, osMessageBox::osMessageBoxIcon icon)
{
    GT_UNREFERENCED_PARAMETER(icon);

    acMessageBox::instance().information(caption.asASCIICharArray(), message.asASCIICharArray());
}


// ---------------------------------------------------------------------------
// Name:        vspApplicationCommands::getStartDebuggingCommandName
// Description:
// Arguments:    gtString& debuggingCommandStr
//              bool addKeyboardShortcut
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        30/8/2011
// ---------------------------------------------------------------------------
void vscApplicationCommands::buildProcessStopString(gtString& propertiesInfo)
{
    // Build an HTML string with the current package start debugging command text:
    gtString msg;

    wchar_t* pCmdName    = NULL;
    wchar_t* pVerbName   = NULL;
    vscUtilsGetStartActionCommandName(pCmdName, pVerbName);

    // Get the current start debugging command string:
    gtString verbName(pVerbName != NULL ? pVerbName : L"");
    gtString commandName(pCmdName != NULL ? pCmdName : L"");

    // Release the allocated strings.
    vscDeleteWcharString(pCmdName);
    vscDeleteWcharString(pVerbName);

    // Append the start debugging string to the start debugging comment:
    msg.appendFormattedString(VSP_STR_PropertiesViewStartDebuggingPackageComment, verbName.asCharArray(), commandName.asCharArray());
    msg.append(VSP_STR_keyboardShortcutRunString);

    // Build the HTML string:
    afHTMLContent htmlContent(AF_STR_PropertiesProcessNotRunning);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_NO_BG_LINE, msg);
    htmlContent.toString(propertiesInfo);
}

#define VSP_NUMBER_OF_TESTED_MODULE_PREFIXES 2
// ---------------------------------------------------------------------------
// Name:        vspApplicationCommands::shouldReportClientApplicationCrash
// Description: Returns true if the call stack indicates we should report the crash
//              to the CodeXL crash report server.
// Author:      Uri Shomroni
// Date:        8/11/2011
// ---------------------------------------------------------------------------
bool vscApplicationCommands::shouldReportClientApplicationCrash(const osCallStack& clientAppCrashStack)
{
    bool retVal = false;

    // In Visual Studio, we only want to report crashes that passed through some CodeXL code.
    // Note this filter is only applied to client modules, server modules get treated elsewhere (isSpyFrame):
    static const gtString CodeXLModuleNameStarters[VSP_NUMBER_OF_TESTED_MODULE_PREFIXES] =
    {
        L"amdt",        // Prefix for AMD Tools dlls (and current standalone ones)
        L"codexl",      // Prefix for CodeXL package dlls
    };

    // Iterate the frames:
    int numberOfStackFrames = clientAppCrashStack.amountOfStackFrames();

    for (int i = 0; i < numberOfStackFrames; i++)
    {
        // Get the current stack frame:
        const osCallStackFrame* pCurrentFrame = clientAppCrashStack.stackFrame(i);
        GT_IF_WITH_ASSERT(pCurrentFrame != NULL)
        {
            // Get the module file name:
            gtString currentFrameFilenameLowercase;
            bool rcNm = pCurrentFrame->moduleFilePath().getFileName(currentFrameFilenameLowercase);

            if (rcNm)
            {
                // Go through the prefixes (note that to support 64-bit and debug configurations,
                // we check for "starts with" instead of "is equal":
                currentFrameFilenameLowercase.toLowerCase();

                for (int j = 0; j < VSP_NUMBER_OF_TESTED_MODULE_PREFIXES; j++)
                {
                    // If we have a match:
                    if (currentFrameFilenameLowercase.startsWith(CodeXLModuleNameStarters[j]))
                    {
                        // Stop looking:
                        retVal = true;
                        break;
                    }
                }
            }

            if (retVal)
            {
                // We found at least one CodeXL frame, we can stop here:
                break;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspApplicationCommands::OnFileSaveProject
// Description: We should not implement this in the package
// Author:      Sigal Algranaty
// Date:        15/4/2012
// ---------------------------------------------------------------------------
void vscApplicationCommands::OnFileSaveProject()
{
    // Execute the save command only when we are no during the load process:
    if (!m_sIsInLoadProcess)
    {
        // Get the current executable file path:
        osFilePath executableFilePath = afProjectManager::instance().currentProjectSettings().executablePath();

        // Get the current project name:
        gtString projectName = afProjectManager::instance().currentProjectSettings().projectName();

        // If no project is active, there's nothing to do:
        if (!projectName.isEmpty())
        {
            // Get the project path:
            osFilePath vsProjectFilePath;
            afGetVisualStudioProjectFilePath(executableFilePath, projectName, vsProjectFilePath);

            // If there is a loaded project:
            if (!(vsProjectFilePath.asString().isEmpty()))
            {
                // Save the current project into a file:
                afSaveProjectCommand saveprojectCmd(vsProjectFilePath);
                bool rc = saveprojectCmd.execute();
                GT_ASSERT(rc);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vspApplicationCommands::OnFileSaveProjectAs
// Description: We should not implement this in the package
// Author:      Sigal Algranaty
// Date:        15/4/2012
// ---------------------------------------------------------------------------
void vscApplicationCommands::OnFileSaveProjectAs()
{
    GT_ASSERT_EX(false, L"Should not get here");
}


// ---------------------------------------------------------------------------
// Name:        vspApplicationCommands::onFileOpenStartupDialog
// Description: We should not implement this in the package
// Author:      Sigal Algranaty
// Date:        15/4/2012
// ---------------------------------------------------------------------------
void vscApplicationCommands::onFileOpenStartupDialog()
{
    GT_ASSERT_EX(false, L"Should not get here");
}

void vscApplicationCommands::OnFileCloseProject(bool shouldOpenWelcomePage)
{
    GT_UNREFERENCED_PARAMETER(shouldOpenWelcomePage);
    // Solution is closed, clear the settings for the infra and plugins
    // Let the project settings extension know that the current settings are cleared:
    afProjectManager::instance().EmitClearCurretProjectSettings();

    // Before loading a new project, restore the default settings for all extensions:
    afProjectManager::instance().restoreDefaultExtensionsProjectSettings();

}

// ---------------------------------------------------------------------------
// Name:        vspApplicationCommands::onFileOpenStartupDialog
// Description: Updates the project settings from the startup project
// Author:      Uri Shomroni
// Date:        21/5/2012
// ---------------------------------------------------------------------------
void vscApplicationCommands::updateProjectSettingsFromImplementation()
{
    GT_IF_WITH_ASSERT(m_pOwner != NULL)
    {
        m_pOwner->UpdateProjectSettingsFromStartupProject();
        afApplicationCommands::updateProjectSettingsFromImplementation();
    }
}

// ---------------------------------------------------------------------------
void vscApplicationCommands::closeDocumentsOfDeletedFiles()
{
    GT_IF_WITH_ASSERT(m_pOwner != NULL)
    {
        m_pOwner->CloseDocumentsOfDeletedFiles();
    }
}

// ---------------------------------------------------------------------------
bool vscApplicationCommands::saveMDIFile(const osFilePath& filePath)
{
    bool ret = false;
    GT_IF_WITH_ASSERT(m_pOwner != NULL)
    {
        ret = m_pOwner->SaveFileWithPath(filePath.asString().asCharArray());
    }
    return ret;
}

// ---------------------------------------------------------------------------
void vscApplicationCommands::ClearInformationView()
{
    GT_IF_WITH_ASSERT(m_pOwner != NULL)
    {
        m_pOwner->ClearBuildPane();
    }
}

void vscApplicationCommands::AddStringToInformationView(const QString& messageToDisplay)
{
    GT_IF_WITH_ASSERT(m_pOwner != NULL)
    {
        QString msg = messageToDisplay;
        // Match kernel or shader file,  Can handle mixed forward slash and backward slash
        QRegExp clFilePathAndNamExp(QString(AF_STR_FILE_PATHEXTENSIONS_QREGEXP), Qt::CaseInsensitive);

        QRegExp glslStageExp(QString(AF_STR_BUILDING_STAGE_QREGEXP), Qt::CaseInsensitive);


        while (msg.length() > 0)
        {
            // remove new lines from begging of the message
            while (!msg.isEmpty() && msg.startsWith(VS_STR_NewLineA))
            {
                msg = msg.mid(1);
            }

            //If the output message does not contain line numbers it should be sent as is
            if (msg.contains(AF_STR_BUILD_STARTED, Qt::CaseInsensitive))
            {
                gtString msgToAdd;
                QStringList stagesLines = msg.split("\n");
                bool isBuildingStage = msg.contains(AF_STR_BUILDING_STAGE, Qt::CaseInsensitive);
                m_stagePathMap.clear();

                for (const QString& it : stagesLines)
                {
                    if (isBuildingStage)
                    {
                        // populate stagePathMap by turning stage name to lower case and associating stage shader file path
                        if (0 <= glslStageExp.lastIndexIn(it) && 0 <= clFilePathAndNamExp.indexIn(it))
                        {
                            m_stagePathMap[glslStageExp.capturedTexts()[0].toLower()] = clFilePathAndNamExp.capturedTexts()[0];
                        }
                    }

                    msgToAdd = acQStringToGTString(it);
                    m_pOwner->OutputBuildMessage(msgToAdd.asCharArray(), false, L"", 0);
                }

                msg = "";
            }
            else
            {
                handleShaderBuild(glslStageExp, clFilePathAndNamExp, msg);
            }
        }
    }
}

void vscApplicationCommands::handleShaderBuild(const QRegExp& glslStageExp, const QRegExp& clFilePathAndNamExp, QString& msg)
{
    // looking for line 123: so the line number can be isolated
    const QRegExp lineNumExp(QString(AF_STR_CL_LINENUM_QREGEXP));

    // for openGL files, looking for :digits:
    const QRegExp glsllineNumExp(AF_STR_GLSL_LINENUM_QREGEXP);

    // for directX files, looking for (123, in order to isolate line number
    const QRegExp hlsllineNumExp(AF_STR_HLSL_LINENUM_QREGEXP);


    // Check if the line has line number
    QStringList matched;
    QString lineNumText;
    // pathDelimiter indicates how file path is separated from the rest of error message
    QChar pathDelimiter;

    //after build started check if it's stage shader build
    if (!m_stagePathMap.empty())
    {
        gtString msgToAdd = acQStringToGTString(msg);

        if (0 <= glsllineNumExp.indexIn(msg))
        {
            matched = glsllineNumExp.capturedTexts();
            lineNumText = matched[0].mid(1, matched[0].size() - 2);

            if (0 <= glslStageExp.indexIn(msg))
            {
                QString stageName = glslStageExp.capturedTexts()[0];
                QString stageFilePath = m_stagePathMap[stageName];
                bool lineNumOk = false;
                int line = lineNumText.toInt(&lineNumOk);

                if (lineNumOk)
                {
                    // in the background, visual studio counts lines starting zero
                    line = line > 0 ? (line - 1) : line;

                    m_pOwner->OutputBuildMessage(msgToAdd.asCharArray(), false, acQStringToGTString(stageFilePath).asCharArray(), line);
                }
            }
        }
        else
        {
            m_pOwner->OutputBuildMessage(msgToAdd.asCharArray(), false, L"", 0);
        }

        msg = "";
    }
    else
    {
        // gets the start location of the file name and path
        int from = clFilePathAndNamExp.indexIn(msg);

        if (from < 0)
        {
            // the current message doesn't have file in it, post it as is
            gtString msgToAdd = acQStringToGTString(msg);
            m_pOwner->OutputBuildMessage(msgToAdd.asCharArray(), false, L"", 0);
            msg = "";
        }
        else
        {
            if (from > 0)
            {
                // current message has the format expected, however need to post the part of it that has no file attached
                gtString msgToAdd = acQStringToGTString(msg.mid(0, from));
                m_pOwner->OutputBuildMessage(msgToAdd.asCharArray(), false, L"", 0);
            }

            bool msgPosted = false;
            // find the end of the file path string

            // Check if the line has line number
            if (0 <= lineNumExp.indexIn(msg))
            {
                matched = lineNumExp.capturedTexts();
                lineNumText = matched[0];
                pathDelimiter = ',';
            }
            else if (0 <= hlsllineNumExp.indexIn(msg))
            {
                matched = hlsllineNumExp.capturedTexts();
                lineNumText = matched[0];
                pathDelimiter = '(';
            }
            else if (0 <= glsllineNumExp.indexIn(msg))
            {
                matched = glsllineNumExp.capturedTexts();
                lineNumText = matched[0].mid(1, matched[0].size() - 2);
                pathDelimiter = ' ';
            }

            int to = msg.indexOf(QRegExp(AF_STR_FILE_EXTENSIONS_QREGEXP), from);

            //find file path using pathDelimiter
            while ((to < msg.length() - 1) && (msg.at(to) != pathDelimiter))
            {
                to++;
            }

            int len = (to > from) ? to - from : 0;

            // extracting the file path and name
            QString filePathAndName = msg.mid(from, len);
            gtString file = acQStringToGTString(filePathAndName);
            osFilePath filePath(acQStringToGTString(filePathAndName));
            gtString fileExtension;
            bool extRes = filePath.getFileExtension(fileExtension);
            GT_ASSERT(extRes);

            // find the end of current message
            if (fileExtension.isEqualNoCase(L"cl"))
            {
                to = msg.indexOf(QString("^"), from);
            }
            else
            {
                to = msg.indexOf(QString("\n"), from);
            }

            QString curMsg = msg.mid(from, to - from);

            if (filePath.exists())
            {
                // Extract line number
                QRegExp numExp(QString("[0-9]+"));

                if (0 <= numExp.indexIn(lineNumText))
                {
                    matched = numExp.capturedTexts();
                    lineNumText = matched[0];
                    bool lineNumOk = false;
                    int line = lineNumText.toInt(&lineNumOk);

                    if (lineNumOk)
                    {
                        // in the background, visual studio counts lines starting zero
                        line = line > 0 ? (line - 1) : line;
                        gtString msgToAdd = acQStringToGTString(curMsg);
                        file = acQStringToGTString(filePathAndName);
                        m_pOwner->OutputBuildMessage(msgToAdd.asCharArray(), false, file.asCharArray(), line);
                        msgPosted = true;
                    }
                }
            }

            if (!msgPosted)
            {
                // in case something went wrong with extracting file and line number, post message without spacial handling
                gtString msgToAdd = acQStringToGTString(curMsg.mid(0, from));
                m_pOwner->OutputBuildMessage(msgToAdd.asCharArray(), false, L"", 0);
            }

            // updating msg
            msg = msg.mid(from + curMsg.length());
        }
    }
}

// ---------------------------------------------------------------------------
void vscApplicationCommands::setOwner(IVscApplicationCommandsOwner* pOwner)
{
    m_pOwner = pOwner;
    GT_ASSERT(m_pOwner != NULL);
}

// ---------------------------------------------------------------------------
void vscApplicationCommands::setViewLineNumbers(bool show)
{
    GT_UNREFERENCED_PARAMETER(show);
}

// ---------------------------------------------------------------------------
void vscApplicationCommands::PrintToDebugLog(const QString& msg)
{
    AddStringToInformationView(msg);
}

// ---------------------------------------------------------------------------
QMessageBox::StandardButton vscApplicationCommands::ShowMessageBox(QMessageBox::Icon type, const QString& title, const QString& text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
    return vspWindowsManager::instance().ShowMessageBox(type, title, text, buttons, defaultButton);
}

void vscApplicationCommands::GetListOfOpenedWindowsForFile(const gtString& containingDirectory, gtVector<osFilePath>& listOfOpenedWindows)
{
    // Get the amount of opened windows:
    std::list<std::wstring> listOfPathContained;
    std::wstring directoryAsStr(containingDirectory.asCharArray());

    vscVspDTEInvoker_GetListOfFilesContainedAtDirectory(directoryAsStr, listOfPathContained);

    for (auto iter = listOfPathContained.begin(); iter != listOfPathContained.end(); iter++)
    {
        gtString openedWindowPath = (*iter).c_str();
        listOfOpenedWindows.push_back(osFilePath(openedWindowPath));
    }
}

const gtString vscApplicationCommands::captionPrefix()
{
    static bool sInitialized = false;
    static gtString retVal;

    if (!sInitialized)
    {
        retVal = afGlobalVariablesManager::ProductName();
        retVal.append(L" ");
        sInitialized = true;
    }

    return retVal;
}

void vscApplicationCommands::SaveAllMDISubWindowsForFilePath(const osFilePath& filePath)
{
    // handle the special case of the kernel analyzer files
    QVector<vscEditorDocument*>& openDocuments = vscEditorDocument::OpenDocuments();
    QVector<vscEditorDocument*>::iterator docIter = openDocuments.begin();

    while (docIter != openDocuments.end())
    {
        vspQTWindowPaneImpl* implPane = (*docIter)->ImplPane();

        if (implPane != nullptr)
        {
            QWidget* pCurrentSubWindow = implPane->createdQTWidget();

            // Get the widget from the window:

            kaKernelView* pKernelView = qobject_cast<kaKernelView*>(pCurrentSubWindow);

            if (nullptr != pKernelView &&
                nullptr != pKernelView->GetActiveMultiSourceView() &&
                nullptr != pKernelView->GetActiveMultiSourceView()->SourceView() &&
                pKernelView->GetActiveMultiSourceView()->SourceView()->filePath() == filePath)
            {
                if (pKernelView->GetActiveMultiSourceView()->SourceView()->IsModified())
                {
                    pKernelView->FileSave();
                    break;
                }
            }
        }

        docIter++;
    }

    // let VS save the file with the same path
    vscVspDTEInvoker_SaveFileWithPath(filePath.asString().asCharArray());
}