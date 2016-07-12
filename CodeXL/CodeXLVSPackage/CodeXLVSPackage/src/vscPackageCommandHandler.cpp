//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscPackageCommandHandler.cpp
///
//==================================================================================

#include "stdafx.h"
#include <Include/Public/vscPackageCommandHandler.h>
#include <Include/../CodeXLVSPackageUi/CommandIds.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTAPIClasses/Include/apFunctionCall.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSAPIWrappers/Include/oaDriver.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTKernelAnalyzer/src/kaDataTypes.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>

// AMDTGpuDebuggingComponents:
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdMultiWatchView.h>

// AMDTSharedProfiling:
#include <SharedProfileManager.h>
#include <StringConstants.h>

// Profiling
#include <AMDTGpuProfiling/ProfileManager.h>
#include <AMDTGpuProfiling/gpExecutionMode.h>

// Kernel Analyzer
#include <AMDTKernelAnalyzer/src/kaApplicationCommands.h>
#include <AMDTKernelAnalyzer/src/kaApplicationTreeHandler.h>
#include <AMDTKernelAnalyzer/src/kaBackendManager.h>
#include <AMDTKernelAnalyzer/src/kaKernelView.h>
#include <AMDTKernelAnalyzer/src/kaMultiSourceView.h>
#include <AMDTKernelAnalyzer/src/kaProjectDataManager.h>
#include <AMDTKernelAnalyzer/src/kaSourceCodeView.h>
#include <AMDTKernelAnalyzer/src/kaTreeDataExtension.h>
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>

// Power profiling
#include <AMDTPowerProfiling/src/ppAppController.h>
#include <AMDTPowerProfiling//src/ppCountersSelectionDialog.h>

// Local:
#include <src/vscApplicationCommands.h>
#include <src/vscBuildListDialog.h>
#include <Include/vscCoreInternalUtils.h>
#include <src/vspKernelAnalyzerEditorManager.h>
#include <src/vspSaveListDialog.h>
#include <Include/vspStringConstants.h>
#include <Include/Public/CoreInterfaces/IVscCoreAPI.h>

// C++:
#include <algorithm>


class vscPackageCommandHandler
{
public:
    vscPackageCommandHandler() {}
    ~vscPackageCommandHandler() {}
};

void vsc_OnUpdateProfileMode(unsigned long commandId, int activeMode, bool& enableCommand, bool& shouldShowCommand, bool& isChecked)
{
    shouldShowCommand = true;

    switch (commandId)
    {
        case cmdidBreakProfiling:
        {
            if (activeMode == vsProfileMode)
            {
                enableCommand = SharedProfileManager::instance().enableVsProfileAction(SharedProfileManager::SPM_VS_PAUSE, isChecked, shouldShowCommand);
            }
            else if (activeMode == vsDebugMode)
            {
                gdApplicationCommands* pApplicationCommand = gdApplicationCommands::gdInstance();
                GT_IF_WITH_ASSERT(pApplicationCommand != nullptr)
                {
                    pApplicationCommand->onUpdateDebugBreak(enableCommand);
                }
            }
            else if (activeMode == vsFrameAnalyisMode)
            {
                shouldShowCommand = false;
                enableCommand = false;
            }
        }
        break;

        case cmdidStopProfiling:
        {
            if (activeMode == vsProfileMode)
            {
                enableCommand = SharedProfileManager::instance().enableVsProfileAction(SharedProfileManager::SPM_VS_STOP, isChecked, shouldShowCommand);
            }
            else if (activeMode == vsDebugMode)
            {
                gdApplicationCommands* pApplicationCommand = gdApplicationCommands::gdInstance();
                GT_IF_WITH_ASSERT(pApplicationCommand != nullptr)
                {
                    pApplicationCommand->onUpdateDebugStopDebugging(enableCommand);
                }
            }
            else if (activeMode == vsFrameAnalyisMode)
            {
                gpExecutionMode* pFrameAnalysisManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();

                if (pFrameAnalysisManager != nullptr)
                {
                    // enable only if the frame analysis is not running
                    enableCommand = pFrameAnalysisManager->IsStopEnabled();
                }
            }
        }
        break;

        case cmdidAttachProfiling:
        {
            enableCommand = enableCommand && SharedProfileManager::instance().enableVsProfileAction(SharedProfileManager::SPM_VS_ATTACH, isChecked, shouldShowCommand);
        }
        break;

        case cmdidCpuAssessPerformanceProfiling:
        {
            enableCommand = SharedProfileManager::instance().enableVsProfileAction(SharedProfileManager::SPM_VS_CPU_ASSESS_PERF, isChecked, shouldShowCommand);
        }
        break;

        case cmdidCpuCacheLineUtilizationProfiling:
        {
            enableCommand = SharedProfileManager::instance().enableVsProfileAction(SharedProfileManager::SPM_VS_CPU_CLU, isChecked, shouldShowCommand);
        }
        break;

        case cmdidCpuCustomProfiling:
        {
            enableCommand = SharedProfileManager::instance().enableVsProfileAction(SharedProfileManager::SPM_VS_CPU_CUSTOM, isChecked, shouldShowCommand);
        }
        break;

        case cmdidCpuIbsProfiling:
        {
            enableCommand = SharedProfileManager::instance().enableVsProfileAction(SharedProfileManager::SPM_VS_CPU_IBS, isChecked, shouldShowCommand);
        }
        break;

        case cmdidCpuL2AccessProfiling:
        {
            enableCommand = SharedProfileManager::instance().enableVsProfileAction(SharedProfileManager::SPM_VS_CPU_L2, isChecked, shouldShowCommand);
        }
        break;

        case cmdidCpuBranchAccessProfiling:
        {
            enableCommand = SharedProfileManager::instance().enableVsProfileAction(SharedProfileManager::SPM_VS_CPU_BR, isChecked, shouldShowCommand);
        }
        break;

        case cmdidCpuDataAccessProfiling:
        {
            enableCommand = SharedProfileManager::instance().enableVsProfileAction(SharedProfileManager::SPM_VS_CPU_DATA_ACCESS, isChecked, shouldShowCommand);
        }
        break;

        case cmdidCpuInstructionAccessProfiling:
        {
            enableCommand = SharedProfileManager::instance().enableVsProfileAction(SharedProfileManager::SPM_VS_CPU_INST_ACCESS, isChecked, shouldShowCommand);
        }
        break;

        case cmdidCpuTimerBasedProfiling:
        {
            enableCommand = SharedProfileManager::instance().enableVsProfileAction(SharedProfileManager::SPM_VS_CPU_TIMER, isChecked, shouldShowCommand);
        }
        break;

        case cmdidGpuPerformanceCounterProfiling:
        {
            enableCommand = SharedProfileManager::instance().enableVsProfileAction(SharedProfileManager::SPM_VS_GPU_PERF_COUNT, isChecked, shouldShowCommand);
        }
        break;

        case cmdidGpuApplicationTraceProfiling:
        {
            enableCommand = SharedProfileManager::instance().enableVsProfileAction(SharedProfileManager::SPM_VS_GPU_APP_TRACE, isChecked, shouldShowCommand);
        }
        break;

        case cmdidPPOnlineProfiling:
        {
            enableCommand = SharedProfileManager::instance().enableVsProfileAction(SharedProfileManager::SPM_VS_PP_ONLINE, isChecked, shouldShowCommand);
        }
        break;

        case cmdidCaptureFrame:
        case cmdidCaptureFrameGPU:
        case cmdidCaptureFrameCPU:
        {
            if (activeMode == vsFrameAnalyisMode)
            {
                gpExecutionMode* pFrameAnalysisManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();

                if (pFrameAnalysisManager != nullptr)
                {
                    bool isProcessRunning = (pFrameAnalysisManager->getCurrentRunModeMask() & AF_DEBUGGED_PROCESS_EXISTS);
                    bool isPaused = (pFrameAnalysisManager->getCurrentRunModeMask() & AF_DEBUGGED_PROCESS_PAUSED);
                    enableCommand = isProcessRunning && !isPaused;
                }
            }
            else
            {
                shouldShowCommand = false;
                enableCommand = false;
            }
        }
        break;

        default:
        {
            // Unexpected value!
            GT_ASSERT(false);
            shouldShowCommand = false;
        }
        break;
    }
}

bool vsc_GetGlobalWorkSize(int currentCoordinateIndex, int& workDimension)
{
    return gaGetKernelDebuggingGlobalWorkSize(currentCoordinateIndex, workDimension);
}


bool vsc_GetGlobalWorkOffset(int currentCoordinateIndex, int& workOffset)
{
    return gaGetKernelDebuggingGlobalWorkOffset(currentCoordinateIndex, workOffset);
}


bool vsc_GetCurrentWorkItemCoordinate(int coordinate, int& value)
{
    return gaGetKernelDebuggingCurrentWorkItemCoordinate(coordinate, value);
}

bool vsc_SetCurrentWorkItemCoordinate(int coordinate, int value)
{
    return gaSetKernelDebuggingCurrentWorkItemCoordinate(coordinate, value);
}

void vsc_OnHexDisplayMode()
{
    gaSetHexDisplayMode(!gaIsHexDisplayMode());
}

bool vsc_OnUpdateHexDisplayMode()
{
    return gaIsHexDisplayMode();
}

int vsc_ActiveProfileMode()
{
    int retVal = vsDebugMode;

    if (afExecutionModeManager::instance().isActiveMode(PM_STR_PROFILE_MODE))
    {
        retVal = vsProfileMode;
    }
    else if (afExecutionModeManager::instance().isActiveMode(PM_STR_FrameAnalysisMode))
    {
        retVal = vsFrameAnalyisMode;
    }

    // for optimization reasons the check for vsKernelAnalyzeMode mode is not done since the code that use this function
    // does not handle cases vsKernelAnalyzeMode

    return retVal;
}

void vsc_OnProfileMode(unsigned long commandId)
{
    switch (commandId)
    {
        case cmdidCpuAssessPerformanceProfiling:
            SharedProfileManager::instance().vsProfileAction(SharedProfileManager::SPM_VS_CPU_ASSESS_PERF);
            break;

        case cmdidCpuCacheLineUtilizationProfiling:
            SharedProfileManager::instance().vsProfileAction(SharedProfileManager::SPM_VS_CPU_CLU);
            break;

        case cmdidCpuCustomProfiling:
            SharedProfileManager::instance().vsProfileAction(SharedProfileManager::SPM_VS_CPU_CUSTOM);
            break;

        case cmdidCpuIbsProfiling:
            SharedProfileManager::instance().vsProfileAction(SharedProfileManager::SPM_VS_CPU_IBS);
            break;

        case cmdidCpuL2AccessProfiling:
            SharedProfileManager::instance().vsProfileAction(SharedProfileManager::SPM_VS_CPU_L2);
            break;

        case cmdidCpuBranchAccessProfiling:
            SharedProfileManager::instance().vsProfileAction(SharedProfileManager::SPM_VS_CPU_BR);
            break;

        case cmdidCpuDataAccessProfiling:
            SharedProfileManager::instance().vsProfileAction(SharedProfileManager::SPM_VS_CPU_DATA_ACCESS);
            break;

        case cmdidCpuInstructionAccessProfiling:
            SharedProfileManager::instance().vsProfileAction(SharedProfileManager::SPM_VS_CPU_INST_ACCESS);
            break;

        case cmdidCpuTimerBasedProfiling:
            SharedProfileManager::instance().vsProfileAction(SharedProfileManager::SPM_VS_CPU_TIMER);
            break;

        case cmdidGpuPerformanceCounterProfiling:
            SharedProfileManager::instance().vsProfileAction(SharedProfileManager::SPM_VS_GPU_PERF_COUNT);
            break;

        case cmdidGpuApplicationTraceProfiling:
            SharedProfileManager::instance().vsProfileAction(SharedProfileManager::SPM_VS_GPU_APP_TRACE);
            break;

        case cmdidPPOnlineProfiling:
            SharedProfileManager::instance().vsProfileAction(SharedProfileManager::SPM_VS_PP_ONLINE);
            break;

        default:
        {
            // Unexpected value!
            GT_ASSERT(false);
        }
        break;
    }

    // Update the toolbar commands:
    GT_IF_WITH_ASSERT(afApplicationCommands::instance() != nullptr)
    {
        afApplicationCommands::instance()->updateToolbarCommands();
    }
}

void vsc_OnBreak()
{
    bool isInProfileMode = afExecutionModeManager::instance().isActiveMode(PM_STR_PROFILE_MODE);

    if (isInProfileMode)
    {
        SharedProfileManager::instance().vsProfileAction(SharedProfileManager::SPM_VS_PAUSE);
    }
    else
    {
        gdApplicationCommands* pApplicationCommand = gdApplicationCommands::gdInstance();
        GT_IF_WITH_ASSERT(pApplicationCommand != nullptr)
        {
            pApplicationCommand->onDebugBreak();
        }
    }
}

void vsc_OnAttach()
{
    bool isInProfileMode = afExecutionModeManager::instance().isActiveMode(PM_STR_PROFILE_MODE);

    if (isInProfileMode)
    {
        SharedProfileManager::instance().vsProfileAction(SharedProfileManager::SPM_VS_ATTACH);
    }
}

void vsc_OnStop()
{
    int activeMode = vsc_ActiveProfileMode();

    if (activeMode == vsProfileMode)
    {
        SharedProfileManager::instance().vsProfileAction(SharedProfileManager::SPM_VS_STOP);
    }
    else if (activeMode == vsDebugMode)
    {
        gdApplicationCommands* pApplicationCommand = gdApplicationCommands::gdInstance();
        GT_IF_WITH_ASSERT(pApplicationCommand != nullptr)
        {
            pApplicationCommand->onDebugStopDebugging();
        }
    }
    else if (activeMode == vsFrameAnalyisMode)
    {
        gpExecutionMode* pFrameAnalysisManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();

        if (pFrameAnalysisManager != nullptr)
        {
            // enable only if the frame analysis is not running
            pFrameAnalysisManager->stopCurrentRun();
        }
    }
}

void vsc_OnCapture(int commandID)
{
    gpExecutionMode* pFrameAnalysisManager = ProfileManager::Instance()->GetFrameAnalysisModeManager();

    if (pFrameAnalysisManager != nullptr)
    {
        switch (commandID)
        {
        case cmdidCaptureFrame:
            pFrameAnalysisManager->OnFrameAnalysisCapture(gpExecutionMode::FrameAnalysisCaptureType_LinkedTrace);
            break;

        case cmdidCaptureFrameGPU:
            pFrameAnalysisManager->OnFrameAnalysisCapture(gpExecutionMode::FrameAnalysisCaptureType_GPUTrace);
            break;

        case cmdidCaptureFrameCPU:
            pFrameAnalysisManager->OnFrameAnalysisCapture(gpExecutionMode::FrameAnalysisCaptureType_APITrace);
            break;

        default:
            // we should not reach here with any other command id
            GT_ASSERT(false);
            break;
        }
    }
}
void vsc_OnUpdateOpenCLBuild_IsActionEnabled(bool& isActionEnabled)
{
    kaApplicationCommands::instance().onUpdateBuildCommand(isActionEnabled);
}

bool vscIsInBuild()
{
    return kaBackendManager::instance().isInBuild();
}

void vsc_OnOpenCLBuild()
{
    gtVector<osFilePath> activeFiles;
    GT_IF_WITH_ASSERT(kaApplicationCommands::instance().activeCLFiles(activeFiles))
    {
        // close editor if it is open:
        kaApplicationTreeHandler::instance()->CloseEditor();

        kaApplicationCommands::instance().buildCommand(activeFiles);

        // update the UI
        if (nullptr != afApplicationCommands::instance())
        {
            afApplicationCommands::instance()->updateToolbarCommands();
        }
    }

}

void vsc_OnUpdateCancelBuild_IsActionEnabled(bool& isActionEnabled)
{
    kaApplicationCommands::instance().onUpdateCancelBuildCommand(isActionEnabled);
}

void vsc_OnUpdateCancelBuild_IsActionVisible(bool& isActionVisible)
{
    isActionVisible = (afProjectManager::instance().currentProjectSettings().lastActiveMode() == KA_STR_executionMode);
}

void vsc_OnCancelBuild()
{
    if (kaBackendManager::instance().isInBuild())
    {
        // close editor if it is open:
        kaApplicationTreeHandler::instance()->CloseEditor();
        kaApplicationCommands::instance().cancelBuildCommand();

        // update the UI
        if (nullptr != afApplicationCommands::instance())
        {
            afApplicationCommands::instance()->updateToolbarCommands();
        }
    }
}


bool vsc_OnUpdateAddOpenCLFile_IsActionVisible()
{
    return (afProjectManager::instance().currentProjectSettings().lastActiveMode() == KA_STR_executionMode);
}

void vsc_OnUpdateAddOpenCLFile_AddFileCommand()
{
    gtVector<osFilePath> addedFilePaths;
    kaApplicationCommands::instance().AddFileCommand(addedFilePaths);

    for (const osFilePath& it : addedFilePaths)
    {
        if (it.exists())
        {
            kaApplicationTreeHandler* pTreeHandler = kaApplicationTreeHandler::instance();

            if (pTreeHandler != nullptr)
            {
                afApplicationTreeItemData* pItemData = pTreeHandler->GetSelectedItemData();
                afTreeItemType itemType = AF_TREE_ITEM_ITEM_NONE;

                if (pItemData != nullptr)
                {
                    const afApplicationTreeItemData* pParentProgramItemData = pTreeHandler->FindParentItemDataOfType(pItemData, AF_TREE_ITEM_KA_PROGRAM);

                    if (pTreeHandler->ShouldAddFileToProgramBranch(pItemData, pParentProgramItemData, it, itemType))
                    {
                        pTreeHandler->AddFileNodeToProgramBranch(it, pParentProgramItemData, itemType);
                    }
                }
            }
        }
    }
}

void vsc_OnUpdateCreateSourceFile_CreateFileCommand()
{
    kaProgram* pProgram = kaProjectDataManager::instance().GetActiveProgram();
    osFilePath createdFilePath;
    kaApplicationTreeHandler* pTreeHandler = kaApplicationTreeHandler::instance();

    if (pTreeHandler != nullptr)
    {
        kaPipelinedProgram::PipelinedStage stage = kaPipelinedProgram::KA_PIPELINE_STAGE_NONE;
        afApplicationTreeItemData* pItemData = pTreeHandler->GetSelectedItemData();
        afTreeItemType itemType = AF_TREE_ITEM_ITEM_NONE;

        if (pItemData != nullptr)
        {
            itemType = pItemData->m_itemType;
            stage = kaRenderingProgram::TreeItemTypeToRenderingStage(itemType);

            if (pProgram != nullptr)
            {
                if (stage == kaPipelinedProgram::KA_PIPELINE_STAGE_NONE)
                {
                    kaProgramTypes programType = pProgram->GetBuildType();

                    if (programType == kaProgramGL_Compute || programType == kaProgramVK_Compute)
                    {
                        stage = kaPipelinedProgram::KA_PIPELINE_STAGE_COMP;
                    }
                }
            }
        }

        kaApplicationCommands::instance().NewFileCommand(false, createdFilePath, pProgram, stage);

        if (createdFilePath.exists())
        {
            // if file node selected under program/folder
            if (itemType != AF_TREE_ITEM_KA_PROGRAM && itemType != AF_TREE_ITEM_ITEM_NONE)
            {
                // get parent program item data
                pItemData = pTreeHandler->GetProgramItemData(pProgram);
            }

            // we are under program branch
            pTreeHandler->AddFileNodeToProgramBranch(createdFilePath, pItemData, AF_TREE_ITEM_ITEM_NONE);
        }
    }
}

bool vsc_OnUpdateCreateSourceFile_IsActionVisible()
{
    return (afProjectManager::instance().currentProjectSettings().lastActiveMode() == KA_STR_executionMode);
}

void vsc_OnDeviceOptions()
{
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(nullptr != pApplicationCommands)
    {
        pApplicationCommands->onToolsOptions(KA_STR_analyzeSettingsPageTitle);
    }
}

void vsc_OnKernelSettingOptions()
{
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(nullptr != pApplicationCommands)
    {
        pApplicationCommands->OnProjectSettings(KA_STR_projectSettingExtensionDisplayName);
    }
}

bool vsc_onFunctionNameGetList(const wchar_t* filePathAsStr, wchar_t**& FunctionStringsArrayBuffer, int& arraySizeBuffer)
{
    GT_UNREFERENCED_PARAMETER(filePathAsStr);
    bool ret = false;
    arraySizeBuffer = 0;
    FunctionStringsArrayBuffer = nullptr;
    // get selected file
    kaApplicationTreeHandler* pTreeHandler = kaApplicationTreeHandler::instance();

    if (pTreeHandler != nullptr)
    {
        //Get selected item data
        const afApplicationTreeItemData* pItemData = pTreeHandler->GetSelectedItemData();

        if (pItemData != nullptr)
        {
            kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pItemData->extendedItemData());

            if (pKAData != nullptr)
            {
                osFilePath activeDocPath(pKAData->filePath());
                kaSourceFile* pCurrentFile = kaProjectDataManager::instance().dataFileByPath(activeDocPath);

                GT_IF_WITH_ASSERT(pCurrentFile != nullptr)
                {
                    if (pCurrentFile->analyzeVector().size() > 0)
                    {
                        // Remove duplicates
                        QStringList functionsList;

                        for (const auto& it : pCurrentFile->analyzeVector())
                        {
                            functionsList << it.m_kernelName;
                        }

                        if (functionsList.size() > 0)
                        {
                            functionsList.removeDuplicates();
                            const size_t functionListSize = functionsList.size();

                            if (functionListSize > 0)
                            {
                                // Assign the array size buffer.
                                arraySizeBuffer = static_cast<int>(functionListSize);

                                // Allocate the output array.
                                FunctionStringsArrayBuffer = new wchar_t* [functionListSize] { nullptr };

                                // Now copy and allocate each Function name string.
                                int index = 0;

                                for (const QString& it : functionsList)
                                {
                                    // Copy the string.
                                    gtString FunctionName(acQStringToGTString(it));

                                    // Allocate the string pointer to be returned.
                                    wchar_t* pFunctionNameStr = nullptr;
                                    pFunctionNameStr = vscAllocateAndCopy(FunctionName);
                                    GT_ASSERT(pFunctionNameStr != nullptr);

                                    // Assign the output buffer's element in opposite order.
                                    FunctionStringsArrayBuffer[index] = (pFunctionNameStr != nullptr) ? pFunctionNameStr : L"";
                                    index++;
                                }

                                ret = true;
                            }
                        }
                    }
                }
            }
        }
    }

    return ret;
}

void vsc_OnFunctionNameChanged(const wchar_t* FunctionNameStr, const wchar_t* pActiveTreeDocFileFullPath, const wchar_t* pActiveMDIDocPath)
{
    GT_UNREFERENCED_PARAMETER(pActiveTreeDocFileFullPath);
    GT_UNREFERENCED_PARAMETER(pActiveMDIDocPath);

    if (FunctionNameStr != nullptr)
    {
        int lineNumber = -1;
        int fileId = -1;
        gtString typeName;
        kaProjectDataManager::instance().SetEntryPointOrKernel(FunctionNameStr, fileId, lineNumber, typeName);
        vsc_UpdateToolbarCommands();
        osFilePath activeDocPath;
        kaProjectDataManager::instance().GetFilePathByID(fileId, activeDocPath);
        // check if pActiveMDIDocPath is for a kaKernel document that needs special handling. if not use VS normal open file command
        gtString activeDocName(pActiveMDIDocPath);
        int deviceFileName = activeDocName.find(KA_STR_kernelViewFile);
        int clxExt = activeDocName.find(KA_STR_kernelViewExtension);

        if (deviceFileName != -1 && clxExt == deviceFileName + 8) // position cxltxt relative to devices in devices.clxtx is 8 characters offset
        {
            QWidget* pActiveView = vspKernelAnalyzerEditorManager::instance().activeView();
            kaKernelView* pKernelView = qobject_cast<kaKernelView*>(pActiveView);

            if (pKernelView != nullptr)
            {
                kaMultiSourceView* pMultiSourceView = pKernelView->GetActiveMultiSourceView();

                if (pMultiSourceView != nullptr)
                {
                    pMultiSourceView->SourceView()->setProgramCounter(lineNumber, 0);
                }
            }
        }
        else
        {
            vscApplicationCommands::instance()->OpenFileAtLine(activeDocPath, lineNumber, -1);
        }
    }
}

void vsc_OnUpdateFunctionName(const wchar_t* filePathStr, int bufSize, wchar_t* pOutStr, size_t& outStrSizeBuffer, bool& isEnabled)
{
    GT_UNREFERENCED_PARAMETER(filePathStr);
    pOutStr = nullptr;
    outStrSizeBuffer = 0;
    isEnabled = true;
    gtString functionName;
    kaProjectDataManager::instance().GetInitialEntryPointOrKernel(functionName);

    // Set the text for the current selected kernelName:
    // Truncate the string to the buffer's size:
    if (bufSize < functionName.length())
    {
        functionName.truncate(0, bufSize - 1);
    }

    pOutStr = vscAllocateAndCopy(functionName);
    outStrSizeBuffer = functionName.length() + 1;
}

void vsc_GetSelectedFunctionName(wchar_t*& pFunctionNameBuffer)
{
    pFunctionNameBuffer = nullptr;
    gtString funcName;
    kaProjectDataManager::instance().GetInitialEntryPointOrKernel(funcName);
    pFunctionNameBuffer = vscAllocateAndCopy(funcName);
}

void vsc_GetBuildOptionsStr(int maxSize, wchar_t*& pBuildOptionsBuffer, int& strSizeBuffer)
{
    if (pBuildOptionsBuffer != nullptr)
    {
        gtString commandOptions = acQStringToGTString(kaProjectDataManager::instance().BuildOptions());

        // Set the text for the current selected kernelName:
        // Truncate the string to the buffer's size:
        //int bufferSize = (int)pOleText->cwBuf;
        if (maxSize < commandOptions.length())
        {
            commandOptions.truncate(0, maxSize - 1);
        }

        // Prepare the output.
        strSizeBuffer = commandOptions.length();
        const wchar_t* pUnderlyingBuffer = commandOptions.asCharArray();

        // Get the size of the string to be built.
        const size_t sz = strSizeBuffer + 1;

        // Copy the string.
        std::copy(pUnderlyingBuffer, pUnderlyingBuffer + sz, pBuildOptionsBuffer);
        pBuildOptionsBuffer[sz - 1] = L'\0';
    }
}

void vsc_OnBuildOptionsChanged_SetBuildOptions(const wchar_t* str)
{
    gtString gtPlatform = kaProjectDataManager::instance().CurrentFileInfo(kaFilePlatform);
    kaProgram* pActiveProgram = kaProjectDataManager::instance().GetActiveProgram();
    QString buildOptionsAsQStr = QString::fromWCharArray(str);

    if (pActiveProgram != nullptr && kaProjectDataManager::instance().IsProgramPipeLine(pActiveProgram) == false)
    {
        if (pActiveProgram->GetBuildType() == kaProgramCL)
        {
            kaApplicationCommands::instance().setBuildOptions(buildOptionsAsQStr);
        }
        else
        {
            kaProjectDataManager::instance().SetShaderBuildOptions(buildOptionsAsQStr);
        }
    }
}

void vsc_OnBuildOptionsChanged_GetBuildOptions(wchar_t*& pBuildOptionsStrBuffer)
{
    kaProjectDataManager& theProjectDataManager = kaProjectDataManager::instance();

    // get build options for cl/directx
    QString buildOptions;
    kaProgram* pActiveProgram = kaProjectDataManager::instance().GetActiveProgram();

    if (pActiveProgram != nullptr && kaProjectDataManager::instance().IsProgramPipeLine(pActiveProgram) == false)
    {
        if (pActiveProgram->GetBuildType() == kaProgramCL)
        {
            buildOptions = theProjectDataManager.BuildOptions();
        }
        else
        {
            buildOptions =  kaProjectDataManager::instance().ShaderBuildOptions();
        }
    }

    gtString buildOptionsAsStr = acQStringToGTString(buildOptions);
    pBuildOptionsStrBuffer = vscAllocateAndCopy(buildOptionsAsStr);
}

void vsc_ShowOptionsDialog()
{
    vspWindowsManager::instance().showOptionsDialog();
}

void vsc_ShowSystemInformationDialog()
{
    vspWindowsManager::instance().showSystemInformationDialog();
}

bool vsc_ToInteger(const wchar_t* pStr, int& resultBuffer)
{
    // Init.
    bool ret = false;
    resultBuffer = 0;

    if (pStr != nullptr)
    {
        // Do the conversion.
        gtString str(pStr);
        ret = str.toIntNumber(resultBuffer);
    }

    return ret;
}

void vsc_OnUpdateOpenCLBuild_IsActionVisible(bool& isActionVisible)
{
    isActionVisible = (afProjectManager::instance().currentProjectSettings().lastActiveMode() == KA_STR_executionMode);
}

void vsc_OnUpdatePPSelectCounters(bool& isActionVisible, bool& isActionEnabled)
{
    isActionVisible = (afExecutionModeManager::instance().isActiveMode(PM_STR_PROFILE_MODE));
    bool midTierAllows = (ppAppController::instance().IsMidTierInitialized() || (ppAppController::instance().MidTierInitError() != PPR_NOT_SUPPORTED));
    isActionEnabled = isActionVisible && !ppAppController::instance().SessionIsOn() && midTierAllows;
}

void vsc_OnOpenPPSelectCounters()
{
    ppCountersSelectionDialog::OpenCountersSelectionDialog();
}

void vsc_DisplayNoVisualCProjectMsgBox()
{
    acMessageBox::instance().critical(AF_STR_ErrorA, VSP_STR_NonSupportedProjectType);
}

void vsc_OnProjectSettings(const wchar_t* filePathStr)
{
    osFilePath executableFilePath(filePathStr);
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
    {
        // Open project settings with the current profile session type page:
        if (afProjectManager::instance().currentProjectSettings().lastActiveMode() == PM_STR_PROFILE_MODE)
        {
            SharedProfileManager::instance().onInvokeProjectSettings();
        }
        else
        {
            pApplicationCommands->OnProjectSettings(afExecutionModeManager::instance().activeMode()->ProjectSettingsPath());
        }
    }
}
void vsc_OnUpdateDebugSettings(bool& enableCommand)
{
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
    {
        // Get the framework state for the dialog:
        pApplicationCommands->onUpdateDebugSettings(enableCommand);
    }
}

void vsc_SetDXFolderModel(const wchar_t* modelNameStr, const wchar_t* filePathStr)
{
    GT_UNREFERENCED_PARAMETER(filePathStr);

    gtString modelName = modelNameStr;

    kaDxFolder* pProgram = dynamic_cast<kaDxFolder*>(kaProjectDataManager::instance().GetActiveProgram());

    if (pProgram != nullptr)
    {
        pProgram->SetProgramLevelShaderModel(modelName);
    }
}


void vsc_GetDXFolderModel(wchar_t*& pModelNameBuffer)
{
    pModelNameBuffer = nullptr;
    kaDxFolder* pProgram = dynamic_cast<kaDxFolder*>(kaProjectDataManager::instance().GetActiveProgram());

    if (pProgram != nullptr)
    {
        gtString modelName = pProgram->GetProgramLevelShaderModel();
        pModelNameBuffer = vscAllocateAndCopy(modelName);
    }
}

void vsc_SetSelectedDXShaderType(const wchar_t* typeNameStr, const wchar_t* filePathStr)
{
    GT_UNREFERENCED_PARAMETER(filePathStr);
    gtString fileType = typeNameStr;
    //Get active program
    kaProgram* pProgram = kaProjectDataManager::instance().GetActiveProgram();
    kaApplicationTreeHandler* pTreeHandler = kaApplicationTreeHandler::instance();

    if (pTreeHandler != nullptr && pProgram != nullptr)
    {
        //Get selected item data
        const afApplicationTreeItemData* pItemData = pTreeHandler->GetSelectedItemData();

        if (pItemData != nullptr)
        {
            kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pItemData->extendedItemData());

            if (pKAData != nullptr)
            {
                int fileId = kaProjectDataManager::instance().GetFileID(pKAData->filePath());
                kaDxFolder* pDXFolder = dynamic_cast<kaDxFolder*>(pProgram);

                if (pDXFolder != nullptr)
                {
                    pDXFolder->SetFileSelectedType(fileId, fileType);
                }
            }
        }
    }
}


void vsc_GetSelectedDXShaderType(wchar_t*& pTypeNameBuffer)
{
    pTypeNameBuffer = nullptr;
    gtString fileType;
    kaProjectDataManager::instance().GetSelectedDXShaderType(fileType);
    pTypeNameBuffer = vscAllocateAndCopy(fileType);
}

void vsc_UpdateToolbarCommands()
{
    afApplicationCommands::instance()->updateToolbarCommands();
}

void vsc_GetActiveStaticAnalyzerTreeDocument(wchar_t*& pDocumentNameBuffer, bool& wasFileChanged)
{
    static gtString lastFileSelected;

    wasFileChanged = false;
    pDocumentNameBuffer = nullptr;

    const afApplicationTreeItemData* pItemData = kaApplicationTreeHandler::instance()->activeItemData();

    if (pItemData != nullptr)
    {
        if (nullptr != pItemData &&
            (pItemData->m_itemType == AF_TREE_ITEM_KA_FILE) || (pItemData->m_itemType == AF_TREE_ITEM_KA_DEVICE) || (pItemData->m_itemType == AF_TREE_ITEM_KA_STATISTICS) || (pItemData->m_itemType == AF_TREE_ITEM_KA_ANALYSIS))
        {
            // select the correct platform based on the file type
            kaTreeDataExtension* pKAData = qobject_cast<kaTreeDataExtension*>(pItemData->extendedItemData());

            if (nullptr != pKAData)
            {
                gtString fileExt;
                osFilePath filePath = pKAData->filePath();
                gtString filePathAsStr = filePath.asString();
                pDocumentNameBuffer = vscAllocateAndCopy(filePathAsStr);

                if (filePathAsStr.compare(lastFileSelected) != 0)
                {
                    lastFileSelected = filePathAsStr;
                    wasFileChanged = true;
                }
            }
        }
    }
}

void vsc_GetBuildCommandString(wchar_t*& pBuildCommandBuffer)
{
    QString buildString;
    kaApplicationTreeHandler* pTreeHandler = kaApplicationTreeHandler::instance();

    if (pTreeHandler != nullptr)
    {
        buildString = pTreeHandler->GetBuildCommandString();
        gtString buildStringAsStr = acQStringToGTString(buildString);
        pBuildCommandBuffer = vscAllocateAndCopy(buildStringAsStr);
    }
}

void vsc_FreeStrMemory(wchar_t*& pStr)
{
    delete[] pStr;
    pStr = nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
bool vsc_IsDirectXFile(const wchar_t* filePathStr)
{
    bool ret = false;

    if (filePathStr != nullptr && wcslen(filePathStr) != 0)
    {
        osFilePath filePath(filePathStr);

        if (filePath.IsMatchingExtension(KA_STR_VS_DXShaderExtensions))
        {
            ret = true;
        }
    }

    return ret;
}

//---------------------------------------------------------------------------------------------------------------------
bool vsc_IsOpenGLFile(const wchar_t* filePathStr)
{
    bool ret = false;

    if (filePathStr != nullptr && wcslen(filePathStr) != 0)
    {
        osFilePath filePath(filePathStr);

        // if not CL file return true
        if (filePath.IsMatchingExtension(KA_STR_VS_GLShaderExtensions))
        {
            ret = true;
        }
    }

    return ret;
}

//---------------------------------------------------------------------------------------------------------------------
void vsc_SetSelectedBitness(const wchar_t* bitnessStr)
{
    gtString architectureStr = bitnessStr;
    kaProjectDataManager::instance().SetProjectArchitectureFromString(architectureStr);
}

//---------------------------------------------------------------------------------------------------------------------
void vsc_GetSelectedBitness(wchar_t*& pBitnessBuffer)
{
    pBitnessBuffer = nullptr;
    gtString bitnessStr;
    kaProjectDataManager::instance().GetProjectArchitectureAsString(bitnessStr);
    pBitnessBuffer = vscAllocateAndCopy(bitnessStr);
}

//---------------------------------------------------------------------------------------------------------------------
bool vsc_IsPlatformFile(const wchar_t* filePathStr, const wchar_t* platformFilesExtensions)
{
    bool isPlatformFile = false;
    osFilePath filePath(filePathStr);
    isPlatformFile = filePath.IsMatchingExtension(platformFilesExtensions);
    return isPlatformFile;
}


bool vsc_IsCurrentPlatform(const wchar_t* candidatePlatform)
{
    bool platformFits = false;

    if (kaProjectDataManager::instance().GetActiveProgram() != nullptr)
    {
        gtString progName = kaProjectDataManager::instance().GetActiveProgram()->GetProgramTypeAsString(kaProjectDataManager::instance().GetActiveProgram()->GetBuildType());
        if (wcscmp(candidatePlatform, progName.asCharArray()) == 0)
        {
            platformFits = true;
        }
    }

    return platformFits;
}


//------------------------------------------------------------------------
bool vsc_IsSourceFileSelected()
{
    bool isSourceFileSelected = kaProjectDataManager::instance().IsSourceFileSelected();
    return isSourceFileSelected;
}

//------------------------------------------------------------------------
bool vsc_IsDXShaderSelected()
{
    bool isDXShaderSelected = kaProjectDataManager::instance().IsDXShaderSelected();
    return isDXShaderSelected;
}
//------------------------------------------------------------------------
bool vsc_IsCLFileSelected()
{
    bool isCLFileSelected = kaProjectDataManager::instance().IsCLFileSelected();
    return isCLFileSelected;
}

//------------------------------------------------------------------------
bool vsc_IsDXFolderSelected()
{
    bool isShaderSelected = false;
    kaProgram* pProgram = kaProjectDataManager::instance().GetActiveProgram();
    kaApplicationTreeHandler* pTreeHandler = kaApplicationTreeHandler::instance();

    if (pTreeHandler != nullptr && pProgram != nullptr)
    {
        const afApplicationTreeItemData* pItemData = pTreeHandler->GetSelectedItemData();

        if (pItemData != nullptr && pItemData->m_itemType == AF_TREE_ITEM_KA_PROGRAM && pProgram->GetBuildType() == kaProgramDX)
        {
            isShaderSelected = true;
        }
    }

    return isShaderSelected;
}