//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspPackageCommandHandler.cpp
///
//==================================================================================
#include "stdafx.h"
#include <src/vspPackageCommandHandler.h>

// C++:
#include <string>
#include <sstream>
#include <cassert>

//Infra
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// Local:
#include <src/vspCoreAPI.h>
#include <src/vspDTEConnector.h>

// Defines the maximal size of work items, where the combo box is detailed:
#define VSP_MAX_WORKITEM_DETAILED 64
#define VSP_MAX_WORKITEM_IN_NON_DETAILED_COMBO 8
#define VSP_BUILD_OPTION_BUFFER_SIZE 512
#define VSP_SUPPORTED_DX_SHADERS_COUNT 6

const int NUM_OF_SHADER_MODELS = 3;



bool GetVSVersion(VsWindowsManagementMode& vsVersion)
{
    bool ret = false;
    vsVersion = VS_WMM_UNKNOWN;

#ifdef VSP_VS10BUILD
    vsVersion = VS_WMM_VS10;
    ret = true;
#elif defined VSP_VS11BUILD
    vsVersion = VS_WMM_VS11;
    ret = true;
#elif defined VSP_VS12BUILD
    vsVersion = VS_WMM_VS12;
    ret = true;
#elif defined VSP_VS14BUILD
    vsVersion = VS_WMM_VS14;
    ret = true;
#else
#error Unknown MSVC version
#endif

    return ret;
}


// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::vspPackageCommandHandler
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        25/1/2011
// ---------------------------------------------------------------------------
vspPackageCommandHandler::vspPackageCommandHandler()
{
}

// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        12/9/2010
// ---------------------------------------------------------------------------
ULONG vspPackageCommandHandler::AddRef(void)
{
    return vspCUnknown::addRef();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugThread::Release
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        12/9/2010
// ---------------------------------------------------------------------------
ULONG vspPackageCommandHandler::Release(void)
{
    return vspCUnknown::release();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugThread::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        12/9/2010
// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT retVal = S_OK;

    if (ppvObj == nullptr)
    {
        retVal = E_POINTER;
    }
    else if (riid == IID_IUnknown)
    {
        *ppvObj = (IUnknown*)this;
        AddRef();
    }
    else if (riid == IID_IDebugThread2)
    {
        *ppvObj = (IDebugThread2*)this;
        AddRef();
    }
    else // riid != IID_IUnknown, IID_IDebugThread2
    {
        retVal = E_NOINTERFACE;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::~vspPackageCommandHandler
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        29/5/2011
// ---------------------------------------------------------------------------
vspPackageCommandHandler::~vspPackageCommandHandler()
{
}

VSP_BEGIN_COMMAND_MAP(vspPackageCommandHandler)

VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidAttachProfiling, vspCommandHandler::QueryStatusHandler(&onUpdateProfileMode), vspCommandHandler::ExecHandler(&onAttach))

VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidBreakProfiling, vspCommandHandler::QueryStatusHandler(&onUpdateProfileMode), vspCommandHandler::ExecHandler(&onBreak))
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidStopProfiling, vspCommandHandler::QueryStatusHandler(&onUpdateProfileMode), vspCommandHandler::ExecHandler(&onStop))

VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidCaptureFrame, vspCommandHandler::QueryStatusHandler(&onUpdateProfileMode), vspCommandHandler::ExecHandler(&onCapture))
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidRefreshSessions, vspCommandHandler::QueryStatusHandler(&onUpdateRefreshSessions), vspCommandHandler::ExecHandler(&onRefreshSessions))

VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidCpuAssessPerformanceProfiling, vspCommandHandler::QueryStatusHandler(&onUpdateProfileMode), vspCommandHandler::ExecHandler(&onProfileMode))
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidCpuCacheLineUtilizationProfiling, vspCommandHandler::QueryStatusHandler(&onUpdateProfileMode), vspCommandHandler::ExecHandler(&onProfileMode))
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidCpuCustomProfiling, vspCommandHandler::QueryStatusHandler(&onUpdateProfileMode), vspCommandHandler::ExecHandler(&onProfileMode))
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidCpuIbsProfiling, vspCommandHandler::QueryStatusHandler(&onUpdateProfileMode), vspCommandHandler::ExecHandler(&onProfileMode))
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidCpuL2AccessProfiling, vspCommandHandler::QueryStatusHandler(&onUpdateProfileMode), vspCommandHandler::ExecHandler(&onProfileMode))
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidCpuBranchAccessProfiling, vspCommandHandler::QueryStatusHandler(&onUpdateProfileMode), vspCommandHandler::ExecHandler(&onProfileMode))
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidCpuDataAccessProfiling, vspCommandHandler::QueryStatusHandler(&onUpdateProfileMode), vspCommandHandler::ExecHandler(&onProfileMode))
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidCpuInstructionAccessProfiling, vspCommandHandler::QueryStatusHandler(&onUpdateProfileMode), vspCommandHandler::ExecHandler(&onProfileMode))
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidCpuTimerBasedProfiling, vspCommandHandler::QueryStatusHandler(&onUpdateProfileMode), vspCommandHandler::ExecHandler(&onProfileMode))
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidGpuPerformanceCounterProfiling, vspCommandHandler::QueryStatusHandler(&onUpdateProfileMode), vspCommandHandler::ExecHandler(&onProfileMode))
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidGpuApplicationTraceProfiling, vspCommandHandler::QueryStatusHandler(&onUpdateProfileMode), vspCommandHandler::ExecHandler(&onProfileMode))
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidPPOnlineProfiling, vspCommandHandler::QueryStatusHandler(&onUpdateProfileMode), vspCommandHandler::ExecHandler(&onProfileMode))

VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDWorkItemXCoordComboGetList, &onUpdateWorkItemCoord, &onWorkItemCoordGetList)
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDWorkItemXCoordCombo, &onUpdateWorkItemCoord, &onWorkItemCoordChanged)
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDWorkItemYCoordComboGetList, &onUpdateWorkItemCoord, &onWorkItemCoordGetList)
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDWorkItemYCoordCombo, &onUpdateWorkItemCoord, &onWorkItemCoordChanged)
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDWorkItemZCoordComboGetList, &onUpdateWorkItemCoord, &onWorkItemCoordGetList)
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDWorkItemZCoordCombo, &onUpdateWorkItemCoord, &onWorkItemCoordChanged)

// Update the Kernel toolbar combos
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDBitnessComboGetList, &onUpdateBitnessCombo, &onBitnessComboGetList)
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDBitnessCombo, &onUpdateBitnessCombo, &onBitnessComboChanged)
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDDXShaderModelComboGetList, &onUpdateFolderModelCombo, &onShaderModelComboGetList)
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDDXShaderModelCombo, &onUpdateFolderModelCombo, &onShaderModelComboChanged)
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDDXShaderTypeComboGetList, &onUpdateTypeCombo, &onTypeComboGetList)
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDDXShaderTypeCombo, &onUpdateTypeCombo, &onTypeComboChanged)
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDKernelNameComboGetList, nullptr, &onFunctionNameGetList)
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDKernelNameCombo, &onUpdateFunctionName, &onFunctionNameChanged)

VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, commandIDOptionsCombo, &onUpdateBuildOptions, &onBuildOptionsChanged)
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidOpenCLBuild, vspCommandHandler::QueryStatusHandler(&onUpdateOpenCLBuild), vspCommandHandler::ExecHandler(&onOpenCLBuild))
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidCancelOpenCLBuild, vspCommandHandler::QueryStatusHandler(&onUpdateCancelBuild), vspCommandHandler::ExecHandler(&onCancelBuild))
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidAddOpenCLFile, vspCommandHandler::QueryStatusHandler(&onUpdateAddOpenCLFile), vspCommandHandler::ExecHandler(&onAddOpenCLFile))
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidCreateSourceFile, vspCommandHandler::QueryStatusHandler(&onUpdateCreateSourceFile), vspCommandHandler::ExecHandler(&onCreateSourceFile))
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidOpenDeviceOptions, vspCommandHandler::QueryStatusHandler(&OnUpdateonDeviceOptions), vspCommandHandler::ExecHandler(&onDeviceOptions))
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidOpenAnalyzeSettings, nullptr, vspCommandHandler::ExecHandler(&onKernelSettingOptions))

VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidOptions, vspCommandHandler::QueryStatusHandler(&onUpdateOptions), vspCommandHandler::ExecHandler(&onOptions))
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidSystemInformation, nullptr, vspCommandHandler::ExecHandler(&onSystemInformation))
VSP_COMMAND_MAP_ENTRY(CLSID_StandardCommandSet97, cmdidSetNextStatement, vspCommandHandler::QueryStatusHandler(&onUpdateDisabledFeatures), vspCommandHandler::ExecHandler(&onDisabledFeatures))
VSP_COMMAND_MAP_ENTRY(CLSID_StandardCommandSet97, cmdidDebugProcesses, vspCommandHandler::QueryStatusHandler(&onUpdateDisabledFeatures), vspCommandHandler::ExecHandler(&onDisabledFeatures))

VSP_COMMAND_MAP_ENTRY(CLSID_StandardCommandSet97, cmdidDisplayRadix, vspCommandHandler::QueryStatusHandler(&onUpdateHexDisplayMode), vspCommandHandler::ExecHandler(&onHexDisplayMode))

VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidOpenPPCounters, vspCommandHandler::QueryStatusHandler(&onUpdatePPSelectCounters), vspCommandHandler::ExecHandler(&onPPSelectCounters))

VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidCodeXLProjectSettings, vspCommandHandler::QueryStatusHandler(&onUpdateProjectSettings), vspCommandHandler::ExecHandler(&onProjectSettings))

VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidHelpGPUForums, vspCommandHandler::QueryStatusHandler(nullptr), vspCommandHandler::ExecHandler(&onHelpOpenURL))
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidHelpOpenTeapotSample, vspCommandHandler::QueryStatusHandler(&OnUpdateSample), vspCommandHandler::ExecHandler(&OnHelpOpenTeapotSample))
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidHelpOpenMatMulSample, vspCommandHandler::QueryStatusHandler(&OnUpdateSample), vspCommandHandler::ExecHandler(&OnHelpOpenMatMulSample))
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidHelpOpenD3D12MultithreadingSample, vspCommandHandler::QueryStatusHandler(&OnUpdateSample), vspCommandHandler::ExecHandler(&OnHelpOpenD3D12MultithreadingSample))
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidViewHelp, nullptr, vspCommandHandler::ExecHandler(&onViewHelp))
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidViewQuickStart, nullptr, vspCommandHandler::ExecHandler(&onViewQuickStart))
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidHelpUpdates, nullptr, vspCommandHandler::ExecHandler(&onCheckForUpdates))
VSP_COMMAND_MAP_ENTRY(CLSID_CodeXLVSPackageCmdSet, cmdidAboutDialog, nullptr, vspCommandHandler::ExecHandler(&onAboutDialog))


VSP_END_VSCOMMAND_MAP()

HRESULT vspPackageCommandHandler::onProjectSettings(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    // Get the file name:
    std::wstring filePathAsString;
    std::wstring dummyStrings[3];
    bool isProjectTypeValid = true;
    bool isProjectOpened = false;
    bool isNonNativeProject = false;
    vspDTEConnector::instance().getStartupProjectDebugInformation(filePathAsString, dummyStrings[0], dummyStrings[1], dummyStrings[2], isProjectOpened, isProjectTypeValid, isNonNativeProject);

    // Check the VS project settings, and update the infrastructure:
    bool shouldDebug = false;
    bool shouldProfile = false;
    bool shouldFrameAnalyze = false;
    UpdateProjectSettingsFromVS(shouldDebug, shouldProfile, shouldFrameAnalyze, isProjectTypeValid);

    if (isProjectTypeValid)
    {
        VSCORE(vsc_OnProjectSettings)(filePathAsString.c_str());
    }
    else // !isProjectTypeValid
    {
        if (isProjectOpened)
        {
            // This is not a Visual C project, display a message to the user:
            VSCORE(vsc_DisplayNoVisualCProjectMsgBox)();
        }
    }

    return S_OK;

}

// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::onUpdateProjectSettings
// Description: Command handler for CodeXL Project Settings command update

// Author:      Uri Shomroni
// Date:        25/1/2011
// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onUpdateProjectSettings(vspCommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText)
{
    GT_UNREFERENCED_PARAMETER(handler);
    GT_UNREFERENCED_PARAMETER(pOleCmd);
    GT_UNREFERENCED_PARAMETER(pOleText);

    if (pOleCmd != nullptr)
    {
        DWORD commandFlags = OLECMDF_SUPPORTED;

        // Enable the command if a project is selected.
        std::wstring filePathAsString;
        std::wstring dummyStrings[3];
        bool dummyBool1 = true;
        bool dummyBool2 = true;
        bool dummyBool3 = true;
        vspDTEConnector::instance().getStartupProjectDebugInformation(filePathAsString, dummyStrings[0], dummyStrings[1], dummyStrings[2], dummyBool1, dummyBool2, dummyBool3);
        wchar_t* pFileName = nullptr;
        VSCORE(vscGetFileName)(filePathAsString.c_str(), pFileName);
        std::wstring fileName((pFileName != nullptr) ? pFileName : L"");

        // Release allocated memory.
        VSCORE(vscDeleteWcharString)(pFileName);

        bool enableCommand = !fileName.empty();

        // Do not display the dialog while a process is running in CodeXL:
        if (enableCommand)
        {
            VSCORE(vsc_OnUpdateDebugSettings)(enableCommand);
        }

        if (enableCommand)
        {
            commandFlags |= OLECMDF_ENABLED;
        }

        pOleCmd->cmdf = commandFlags;
    }

    return S_OK;
}
// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::onUpdateWorkItemCoord
// Description:

// Author:      Uri Shomroni
// Date:        25/1/2011
// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onUpdateWorkItemCoord(const vspCommandHandler& rSender, _Inout_ OLECMD* pOleCmd, _Inout_ OLECMDTEXT* pOleText)
{
    bool enableCommand = false;
    DWORD commandFlags = OLECMDF_SUPPORTED;

    // Check which coordinate is this:
    int currentCoordinateIndex = -1;
    DWORD commandId = rSender.GetId().GetId();

    switch (commandId)
    {
        case commandIDWorkItemXCoordCombo:
        case commandIDWorkItemXCoordComboGetList:
            currentCoordinateIndex = 0;
            break;

        case commandIDWorkItemYCoordCombo:
        case commandIDWorkItemYCoordComboGetList:
            currentCoordinateIndex = 1;
            break;

        case commandIDWorkItemZCoordCombo:
        case commandIDWorkItemZCoordComboGetList:
            currentCoordinateIndex = 2;
            break;

        default:
            assert(false);
            break;
    }

    // Check if the coordinate is active:
    if (currentCoordinateIndex > -1)
    {
        // Check if this coordinate is available:
        int workDimension = -1;
        enableCommand = VSCORE(vsc_GetGlobalWorkSize)(currentCoordinateIndex, workDimension);

        // If we are currently debugging kernels, but one of the dimensions is missing, disable it (but still show a list):
        if (workDimension == 0)
        {
            // Only disable the commands, but show their lists:
            enableCommand = ((commandId == commandIDWorkItemXCoordComboGetList) || (commandId == commandIDWorkItemYCoordComboGetList) || (commandId == commandIDWorkItemZCoordComboGetList));
        }
    }

    // Enable the command if this coordinate is currently active:
    if (enableCommand)
    {
        commandFlags |= OLECMDF_ENABLED;

        int currentWorkItem = 0;
        VSCORE(vsc_GetCurrentWorkItemCoordinate)(currentCoordinateIndex, currentWorkItem);
        std::wstring workItemStr;
        std::wstringstream converter;
        converter << currentWorkItem;
        workItemStr = converter.str();

        // Set the text for the current selected item:
        // Truncate the string to the buffer's size:
        unsigned int bufferSize = pOleText->cwBuf;

        if (bufferSize < workItemStr.length())
        {
            workItemStr = workItemStr.substr(0, bufferSize);
        }

        // Copy the characters to the buffer:
        lstrcpy(pOleText->rgwz, workItemStr.c_str());
        pOleText->cwActual = (ULONG)(workItemStr.length() + 1);
    }

    pOleCmd->cmdf = commandFlags;

    return S_OK;
}


// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::onWorkItemCoordGetList
// Description:

// Author:      Uri Shomroni
// Date:        25/1/2011
// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onWorkItemCoordGetList(_In_ vspCommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pVarOut)
{
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);

    int currentCoordinateIndex = -1;
    DWORD commandId = 0;
    assert(pSender != nullptr);

    if (pSender != nullptr)
    {
        commandId = pSender->GetId().GetId();
    }

    // Check which coordinate is this:
    switch (commandId)
    {
        case commandIDWorkItemXCoordComboGetList:
            currentCoordinateIndex = 0;
            break;

        case commandIDWorkItemYCoordComboGetList:
            currentCoordinateIndex = 1;
            break;

        case commandIDWorkItemZCoordComboGetList:
            currentCoordinateIndex = 2;
            break;

        default:
            assert(false);
            break;
    }

    if (currentCoordinateIndex > -1)
    {
        VSL_CHECKPOINTER(pVarOut, E_INVALIDARG);

        // Clear the out the value here in case of failure
        ::VariantClear(pVarOut);

        // Get the dimension for this axis from the kernel debugging mechanism:
        int workDimension = 0;
        VSCORE(vsc_GetGlobalWorkSize)(currentCoordinateIndex, workDimension);

        // Get the offset for this coordinate:
        int workDimensionOffset = 0;
        VSCORE(vsc_GetGlobalWorkOffset)(currentCoordinateIndex, workDimensionOffset);

        // Define the low / high bound of the combo work items:
        int coordLowBound = workDimensionOffset;
        int coordHighBound = workDimensionOffset + workDimension;

        // Check if the combo should be detailed or not:
        int amountOfItemsInCombo = VSP_MAX_WORKITEM_IN_NON_DETAILED_COMBO + 2;

        if (workDimension < VSP_MAX_WORKITEM_DETAILED)
        {
            amountOfItemsInCombo = workDimension;
        }

        // Define the array of strings for the combo list:
        ATL::CComSafeArray<BSTR> coordStringsArray(amountOfItemsInCombo);

        // Build the combo list:
        if (workDimension < VSP_MAX_WORKITEM_DETAILED)
        {
            // Even if the box is disabled, show the "0" value:
            for (int i = coordLowBound; (i < (int)coordHighBound) || (i == coordLowBound); i++)
            {
                // Create the current zoom coordinate string:
                std::wstring currentCoord;
                std::wstringstream converter;
                converter << i;
                currentCoord = converter.str();

                // Copy the strings to a BSTR:
                BSTR coordValueAsBSTR = SysAllocString(currentCoord.c_str());
                coordStringsArray.SetAt(i - coordLowBound, coordValueAsBSTR);
            }
        }
        else
        {
            // Even if the box is disabled, show the "0" value:
            for (int i = coordLowBound; (i < (int)coordLowBound + VSP_MAX_WORKITEM_IN_NON_DETAILED_COMBO) || (i == coordLowBound); i++)
            {
                // Create the current zoom coordinate string:
                std::wstring currentCoord;
                std::wstringstream converter;
                converter << i;
                currentCoord = converter.str();

                // Copy the strings to a BSTR:
                BSTR coordValueAsBSTR = SysAllocString(currentCoord.c_str());
                coordStringsArray.SetAt(i - coordLowBound, coordValueAsBSTR);
            }

            // Add "..." string to the combo:
            BSTR coordValueAsBSTR = SysAllocString(L"...");
            coordStringsArray.SetAt(VSP_MAX_WORKITEM_IN_NON_DETAILED_COMBO, coordValueAsBSTR);

            // Add "Type a number" string to the combo:
            std::wstring typeStr;
            std::wstringstream converter;
            converter << L"Type a number (" << coordLowBound << L" - " << (coordHighBound - 1) << L")";
            typeStr = converter.str();
            coordValueAsBSTR = SysAllocString(typeStr.c_str());
            coordStringsArray.SetAt(VSP_MAX_WORKITEM_IN_NON_DETAILED_COMBO + 1, coordValueAsBSTR);
        }

        V_ARRAY(pVarOut) = coordStringsArray.Detach();
        V_VT(pVarOut) = VT_ARRAY | VT_BSTR;
    }

    return S_OK;
}


// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::onWorkItemCoordChanged
// Description:

// Author:      Uri Shomroni
// Date:        25/1/2011
// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onWorkItemCoordChanged(_In_ vspCommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(flags);

    bool isValid = false;

    int currentCoordinateIndex = -1;
    DWORD commandId = 0;
    assert(pSender != nullptr);

    if (pSender != nullptr)
    {
        commandId = pSender->GetId().GetId();
    }

    // Check which coordinate is this:
    switch (commandId)
    {
        case commandIDWorkItemXCoordCombo:
            currentCoordinateIndex = 0;
            break;

        case commandIDWorkItemYCoordCombo:
            currentCoordinateIndex = 1;
            break;

        case commandIDWorkItemZCoordCombo:
            currentCoordinateIndex = 2;
            break;

        default:
            assert(false);
            break;
    }

    if (currentCoordinateIndex > -1)
    {
        // The user typed a value as text:
        if (pIn != nullptr)
        {
            if (pIn->vt == VT_BSTR)
            {
                // Get the user string as gtString:
                std::wstring coordinateText = pIn->bstrVal;

                // Check if the string is a valid integer value:
                int coordValue = -1;
                isValid =  VSCORE(vsc_ToInteger)(coordinateText.c_str(), coordValue);

                if (isValid)
                {
                    // Try to set the new coordinate value:
                    isValid = VSCORE(vsc_SetCurrentWorkItemCoordinate)(currentCoordinateIndex, coordValue);

                    if (isValid && (pOut != nullptr))
                    {
                        // Copy the string to the VARIANT structure:
                        V_VT(pOut) = VT_BSTR;
                        V_BSTR(pOut) = SysAllocString(pIn->bstrVal);
                    }
                }
            }
        }
    }

    if (!isValid)
    {
        // Append the current zoom level to the string:
        std::wstring currentCoordinateText;
        int currentCoordinate = 0;
        VSCORE(vsc_GetCurrentWorkItemCoordinate)(currentCoordinateIndex, currentCoordinate);
        std::wstringstream converter;
        converter << currentCoordinate;
        currentCoordinateText = converter.str();

        // Copy the current coordinate to the output VARIANT:
        if (pOut != nullptr)
        {
            // Copy the string to the VARIANT structure:
            V_VT(pOut) = VT_BSTR;
            V_BSTR(pOut) = SysAllocString(currentCoordinateText.c_str());
        }
    }

    return S_OK;
}

// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::onUpdateDisabledFeatures
// Description: update Command handler for all the features we want to disable
// Author:      Sigal Algranaty
// Date:        31/5/2011
// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onUpdateDisabledFeatures(vspCommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText)
{
    GT_UNREFERENCED_PARAMETER(pOleText);

    if (pOleCmd != nullptr)
    {
        // Check if CodeXL process exists (our debug engine is up):
        bool doesProcessExists = VSCORE(vsc_IsDebuggedProcessExists)();

        if (!doesProcessExists)
        {
            // Make sure it is not even marked as supported by CodeXL
            // handler.SetSupported(false);
            handler.SetSupported(false);
            pOleCmd->cmdf = 0;
        }
        else
        {
            // Make sure it is disabled by making it supported by not enabled
            pOleCmd->cmdf = OLECMDF_SUPPORTED;
        }
    }

    return S_OK;
}


// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::onDisabledFeatures
// Description: Dummy functions to notify CodeXL is not really handling this command
// Author:      Gilad Yarnitzky
// Date:        8/11/2011
// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onDisabledFeatures(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    return OLECMDERR_E_NOTSUPPORTED;
}

// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::onHexDisplayMode
// Description: Command handler for hex mode command
// Author:      Gilad Yarnitzky
// Date:        19/5/2011
// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onHexDisplayMode(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    HRESULT retVal = S_OK;

    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    // Change the hex mode:
    VSCORE(vsc_OnHexDisplayMode)();

    vspDTEConnector& theDTEConnector = vspDTEConnector::instance();
    theDTEConnector.setHexDisplayMode();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::onUpdateHexDisplayMode
// Description: update Command handler for hex mode command
// Author:      Gilad Yarnitzky
// Date:        19/5/2011
// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onUpdateHexDisplayMode(vspCommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText)
{
    HRESULT retVal = S_OK;
    GT_UNREFERENCED_PARAMETER(&handler);
    GT_UNREFERENCED_PARAMETER(pOleText);

    if (pOleCmd != nullptr)
    {
        // Initialize the command flags:
        pOleCmd->cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;

        if (VSCORE(vsc_OnUpdateHexDisplayMode)())
        {
            pOleCmd->cmdf |= OLECMDF_LATCHED;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::onUpdateProfileMode
// Description: Called to update a profile mode's menu item status
// Author:      Uri Shomroni
// Date:        14/5/2012
// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onUpdateProfileMode(vspCommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText)
{
    HRESULT retVal = S_OK;
    DWORD commandId = handler.GetId().GetId();
    bool enableCommand = true;
    int activeMode = VSCORE(vsc_ActiveProfileMode)();
    bool shouldShowCommand = true;
    bool isChecked = false;

    if (pOleText != nullptr)
    {
        std::wstring commandName;
        wchar_t* pCommandNameStr = nullptr;
        bool rc = VSCORE(vscGetExecutionCommandName)(commandId, pCommandNameStr);
        assert(pCommandNameStr != nullptr);

        if (pCommandNameStr != nullptr && rc)
        {
            // Copy and release the allocated string.
            commandName = pCommandNameStr;
            VSCORE(vscDeleteWcharString)(pCommandNameStr);

            // Truncate the string to the buffer's size:
            unsigned int bufferSize = pOleText->cwBuf;

            if (bufferSize < commandName.length())
            {
                commandName = commandName.substr(0, bufferSize);
            }

            // Copy the characters to the buffer:
            lstrcpy(pOleText->rgwz, commandName.c_str());
            pOleText->cwActual = (ULONG)(commandName.length() + 1);
        }
    }

    VSCORE(vsc_OnUpdateProfileMode)(static_cast<unsigned long>(commandId), activeMode, enableCommand, shouldShowCommand, isChecked);

    DWORD commandFlags = OLECMDF_SUPPORTED;

    if (enableCommand)
    {
        commandFlags |= OLECMDF_ENABLED;
    }

    if (!shouldShowCommand)
    {
        commandFlags |= OLECMDF_INVISIBLE;
    }

    if (isChecked)
    {
        commandFlags |= OLECMDF_LATCHED;
    }

    pOleCmd->cmdf = commandFlags;

    return retVal;
}

HRESULT vspPackageCommandHandler::onUpdateRefreshSessions(vspCommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText)
{
    GT_UNREFERENCED_PARAMETER(pOleText);
    GT_UNREFERENCED_PARAMETER(handler);

    bool shouldShowCommand = VSCORE(vsc_IsFrameAnalysisModeSelected());
    bool enableCommand = VSCORE(vsc_IsInRunningMode());
    DWORD commandFlags = OLECMDF_SUPPORTED;
    HRESULT retVal = S_OK;

    if (enableCommand)
    {
        commandFlags |= OLECMDF_ENABLED;
    }

    if (!shouldShowCommand)
    {
        commandFlags |= OLECMDF_INVISIBLE;
    }

    pOleCmd->cmdf = commandFlags;
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::onProfileMode
// Description: Called when a profile mode is selected from the menu:
// Author:      Uri Shomroni
// Date:        14/5/2012
// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onProfileMode(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    HRESULT retVal = S_OK;

    if (nullptr != pSender)
    {
        DWORD commandId = pSender->GetId().GetId();

        // Invoke the core logic.
        VSCORE(vsc_OnProfileMode)(commandId);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::onBreak
// Description: Called when the user click break command
// Author:      Sigal Algranaty
// Date:        7/11/2012
// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onBreak(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    // If we are not during debugging:
    if (!VSCORE(vsc_IsDebuggedProcessExists)())
    {
        // Invoke core logic.
        VSCORE(vsc_OnBreak)();
    }
    else // vsc_IsDebuggedProcessExists()
    {
        // Directly call the break button:
        vspDTEConnector::instance().breakDebugging();
    }

    return S_OK;
}

HRESULT vspPackageCommandHandler::onAttach(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    // Invoke core logic.
    VSCORE(vsc_OnAttach)();

    return S_OK;
}

// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::onBreak
// Description: Called when the user click break command
// Author:      Sigal Algranaty
// Date:        7/11/2012
// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onStop(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    // If we are not during debugging:
    if (!VSCORE(vsc_IsDebuggedProcessExists)())
    {
        // Invoke core logic.
        VSCORE(vsc_OnStop)();
    }
    else // vsc_IsDebuggedProcessExists()
    {
        // Directly call the stop button:
        vspDTEConnector::instance().stopDebugging();
    }

    return S_OK;
}

HRESULT vspPackageCommandHandler::onCapture(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    VSCORE(vsc_OnCapture)();

    return S_OK;
}

HRESULT vspPackageCommandHandler::onRefreshSessions(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    VSCORE(vsc_RefreshFrameAnalysisSessionsFromServer)();

    return S_OK;
}
// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::onOptions
// Description: Command handler for CodeXL->Options command
// Arguments:    vspCommandHandler* pSender
//              DWORD flags
//              VARIANT* pIn
//              VARIANT* pOut
// Return Val:  HRESULT
// Author:      Yoni Rabin
// Date:        31/5/2012
// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onOptions(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    VSCORE(vsc_ShowOptionsDialog)();
    return S_OK;
}

// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::onUpdateOptions
// Description: Called to update the options dialog command
// Arguments:   vspCommandHandler& handler
//              OLECMD* pOleCmd
//              OLECMDTEXT* pOleText
// Return Val:  HRESULT
// Author:      Sigal Algranaty
// Date:        21/4/2011
// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onUpdateOptions(vspCommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText)
{
    GT_UNREFERENCED_PARAMETER(&handler);
    GT_UNREFERENCED_PARAMETER(pOleText);

    if (pOleCmd != nullptr)
    {
        // Initialize the command flags:
        pOleCmd->cmdf = OLECMDF_SUPPORTED;

        // Options dialog is available only when there is no debugged process does not exist:
        if (!VSCORE(vsc_IsDebuggedProcessExists)())
        {
            pOleCmd->cmdf |= OLECMDF_ENABLED;
        }
    }

    return S_OK;
}

// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::onSystemInformation
// Description: Command handler for CodeXL->System Information command
// Arguments:    vspCommandHandler* pSender
//              DWORD flags
//              VARIANT* pIn
//              VARIANT* pOut
// Return Val:  HRESULT
// Author:      Yoni Rabin
// Date:        31/5/2012
// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onSystemInformation(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    VSCORE(vsc_ShowSystemInformationDialog)();
    return S_OK;
}


// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::onUpdateOpenCLBuild
// Description: update UI of the OpenCL build command
// Author:      Gilad Yarnitzky
// Date:        29/8/2013
// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onUpdateOpenCLBuild(vspCommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText)
{
    GT_UNREFERENCED_PARAMETER(&handler);
    GT_UNREFERENCED_PARAMETER(pOleText);

    bool isActionEnabled = true, isActionChecked = false, isActionVisible = true;

    // Invoke the core logic.
    VSCORE(vsc_OnUpdateOpenCLBuild_IsActionEnabled)(isActionEnabled);
    VSCORE(vsc_OnUpdateOpenCLBuild_IsActionVisible)(isActionVisible);

    // set the command text
    wchar_t* pBuildString = nullptr;
    VSCORE(vsc_GetBuildCommandString)(pBuildString);

    if (pBuildString != nullptr && wcscmp(pBuildString, L"") != 0)
    {
        // the string already gets the "build " from the GetBuildCommandString
        wchar_t buildString[256];
        buildString[0] = 0;
        wcscat_s(buildString, 256, pBuildString);
        lstrcpy(pOleText->rgwz, buildString);
        pOleText->cwActual = (ULONG)wcslen(pBuildString);
        VSCORE(vscDeleteWcharString)(pBuildString);
    }

    if (pOleCmd != nullptr)
    {
        DWORD commandFlags = OLECMDF_SUPPORTED;

        if (isActionEnabled)
        {
            commandFlags |= OLECMDF_ENABLED;
        }

        if (isActionChecked)
        {
            commandFlags |= OLECMDF_LATCHED;
        }

        if (!isActionVisible)
        {
            commandFlags |= OLECMDF_INVISIBLE;
        }

        pOleCmd->cmdf = commandFlags;
    }

    return S_OK;
}

// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::onOpenCLBuild
// Description: update UI of the device options
// Author:      Gilad Yarnitzky
// Date:        3/2/2014
// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::OnUpdateonDeviceOptions(vspCommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText)
{
    GT_UNREFERENCED_PARAMETER(&handler);
    GT_UNREFERENCED_PARAMETER(pOleText);

    bool isActionEnabled = false;

    if (!VSCORE(vscIsInBuild)() && VSCORE(vsc_OnUpdateMode_AnalyzeMode_IsChecked()))
    {
        isActionEnabled = true;
    }

    DWORD commandFlags = OLECMDF_SUPPORTED;

    if (isActionEnabled)
    {
        commandFlags |= OLECMDF_ENABLED;
    }

    pOleCmd->cmdf = commandFlags;

    return S_OK;
}


// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::onOpenCLBuild
// Description: Handle the build command
// Author:      Gilad Yarnitzky
// Date:        29/8/2013
// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onOpenCLBuild(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    // Invoke core logic.
    VSCORE(vsc_OnOpenCLBuild)();
    return S_OK;
}


// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::onUpdateAddOpenCLFile
// Description: update UI of the add OpenCL file command
// Author:      Gilad Yarnitzky
// Date:        29/8/2013
// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onUpdateAddOpenCLFile(vspCommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText)
{
    GT_UNREFERENCED_PARAMETER(&handler);
    GT_UNREFERENCED_PARAMETER(pOleText);

    if (pOleCmd != nullptr)
    {
        // Initialize the command flags:
        pOleCmd->cmdf = OLECMDF_SUPPORTED;

        // Invoke core logic.
        bool isActionVisible = VSCORE(vsc_OnUpdateAddOpenCLFile_IsActionVisible)();
        bool solutionLoaded = vspDTEConnector::instance().isSolutionLoaded();

        if (solutionLoaded)
        {
            pOleCmd->cmdf |= OLECMDF_ENABLED;
        }

        if (!isActionVisible)
        {
            pOleCmd->cmdf |= OLECMDF_INVISIBLE;
        }
    }

    return S_OK;
}

// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::onOpenCLBuild
// Description: Handle the add OpenCL file command
// Author:      Gilad Yarnitzky
// Date:        29/8/2013
// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onAddOpenCLFile(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    VSCORE(vsc_OnUpdateAddOpenCLFile_AddFileCommand)();
    return S_OK;
}


HRESULT vspPackageCommandHandler::onUpdateCreateSourceFile(vspCommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText)
{
    GT_UNREFERENCED_PARAMETER(&handler);
    GT_UNREFERENCED_PARAMETER(pOleText);

    if (pOleCmd != nullptr)
    {
        // Initialize the command flags:
        pOleCmd->cmdf = OLECMDF_SUPPORTED;

        // Invoke core logic.
        bool isActionVisible = VSCORE(vsc_OnUpdateCreateSourceFile_IsActionVisible)();
        bool solutionLoaded = vspDTEConnector::instance().isSolutionLoaded();

        if (solutionLoaded)
        {
            pOleCmd->cmdf |= OLECMDF_ENABLED;
        }

        if (!isActionVisible)
        {
            pOleCmd->cmdf |= OLECMDF_INVISIBLE;
        }
    }

    return S_OK;
}

HRESULT vspPackageCommandHandler::onCreateSourceFile(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    VSCORE(vsc_OnUpdateCreateSourceFile_CreateFileCommand)();
    return S_OK;
}

// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::onDeviceOptions
// Description: Handle toolbar open device options
// Author:      Gilad Yarnitzky
// Date:        3/10/2013
// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onDeviceOptions(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    // Invoke core logic.
    VSCORE(vsc_OnDeviceOptions)();
    return S_OK;
}

void vspPackageCommandHandler::UpdateProjectSettingsFromVS(bool& shouldDebug, bool& shouldProfile, bool& shouldFrameAnalyze, bool& isProjectTypeValid)
{
    // Get the project details we need to debug/profile:
    std::wstring executableFilePath;
    std::wstring workingDirectoryPath;
    std::wstring commandLineArguments;
    std::wstring environment;
    bool isProjectOpened = false;
    bool isNonNativeProject = false;
    vspDTEConnector& theDTEConnector = vspDTEConnector::instance();
    theDTEConnector.getStartupProjectDebugInformation(executableFilePath, workingDirectoryPath, commandLineArguments, environment, isProjectOpened, isProjectTypeValid, isNonNativeProject);

    // Invoke the core logic.
    VSCORE(vsc_UpdateProjectSettingsFromVS)(executableFilePath.c_str(), workingDirectoryPath.c_str(),
                                            commandLineArguments.c_str(), environment.c_str(), shouldDebug, shouldProfile, shouldFrameAnalyze);
}

// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::onKernelSettingOptions
// Description: Handle toolbar open kernel settings page
// Author:      Gilad Yarnitzky
// Date:        3/10/2013
// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onKernelSettingOptions(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);


    // Check the VS project settings, and update the infrastructure:
    bool shouldDebug = false;
    bool shouldProfile = false;
    bool shouldFrameAnalyze = false;
    bool isProjectTypeValid = true;
    UpdateProjectSettingsFromVS(shouldDebug, shouldProfile, shouldFrameAnalyze, isProjectTypeValid);

    // Invoke core logic.
    VSCORE(vsc_OnKernelSettingOptions)();
    return S_OK;
}

// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onUpdateBitnessCombo(const vspCommandHandler& rSender, _Inout_ OLECMD* pOleCmd, _Inout_ OLECMDTEXT* pOleText)
{
    GT_UNREFERENCED_PARAMETER(&rSender);
    GT_UNREFERENCED_PARAMETER(&pOleText);

    if (pOleCmd != nullptr)
    {
        // Initialize the command flags:
        pOleCmd->cmdf = OLECMDF_SUPPORTED;

        if (VSCORE(vsc_OnUpdateMode_AnalyzeMode_IsChecked()))
        {
            pOleCmd->cmdf |= OLECMDF_ENABLED;
        }

        wchar_t* pBitnessNameStr = nullptr;
        wchar_t* pBitnessNameStrToUse = nullptr;

        VSCORE(vsc_GetSelectedBitness)(pBitnessNameStr);
        assert(pBitnessNameStr != nullptr);

        // if pTypeNameStr is empty
        if (pBitnessNameStr == nullptr || wcscmp(pBitnessNameStr, L"") == 0)
        {
            // set the default value to be the first from KA_STR_VS_toolbarTargetData
            const int NUM_OF_BITNESS = 2;
            ATL::CComSafeArray<BSTR> bitnessStringsArray(NUM_OF_BITNESS);
            wchar_t* bitnessStrings[NUM_OF_BITNESS] = { AF_STR_loadProjectArchitecture32Bit, AF_STR_loadProjectArchitecture64Bit };
            pBitnessNameStrToUse = bitnessStrings[0];
            VSCORE(vsc_SetSelectedBitness)(pBitnessNameStrToUse);
        }
        else
        {
            pBitnessNameStrToUse = pBitnessNameStr;
        }

        // Copy the characters to the buffer - set the combo value
        lstrcpy(pOleText->rgwz, pBitnessNameStrToUse);
        int len = wcslen(pBitnessNameStrToUse);
        pOleText->cwActual = (ULONG)(len);
        VSCORE(vscDeleteWcharString)(pBitnessNameStr);
    }

    return S_OK;
}

// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onBitnessComboGetList(_In_ vspCommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pVarOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);

    if (pVarOut != nullptr)
    {
        // Define the array of strings for the combo list:
        ATL::CComSafeArray<BSTR> bitnessStringsArray(2);
        wchar_t* bitnessStrings[2] = { AF_STR_loadProjectArchitecture32Bit, AF_STR_loadProjectArchitecture64Bit };

        for (int nBitness = 0; nBitness < 2; nBitness++)
        {
            BSTR bitnessBSTR = SysAllocString(bitnessStrings[nBitness]);
            bitnessStringsArray.SetAt(nBitness, bitnessBSTR);
        }

        V_ARRAY(pVarOut) = bitnessStringsArray.Detach();
        V_VT(pVarOut) = VT_ARRAY | VT_BSTR;
    }

    return S_OK;
}

// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onBitnessComboChanged(_In_ vspCommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);

    if (pIn != nullptr)
    {
        if (pIn->vt == VT_BSTR)
        {
            // Invoke core logic.
            VSCORE(vsc_SetSelectedBitness)(pIn->bstrVal);

            // Copy the string to the VARIANT structure:
            if (pOut != nullptr)
            {
                // Copy the string to the VARIANT structure:
                V_VT(pOut) = VT_BSTR;
                V_BSTR(pOut) = SysAllocString(pIn->bstrVal);
            }
        }

        VSCORE(vsc_UpdateToolbarCommands)();
    }

    if (pOut != nullptr)
    {
        wchar_t* pBitnessStr;
        VSCORE(vsc_GetSelectedBitness)(pBitnessStr);
        assert(pBitnessStr != nullptr);

        if (pBitnessStr != nullptr)
        {
            // Copy the string to the VARIANT structure:
            V_VT(pOut) = VT_BSTR;
            V_BSTR(pOut) = SysAllocString(pBitnessStr);

            VSCORE(vscDeleteWcharString)(pBitnessStr);
        }
    }

    return S_OK;
}

// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onUpdateFolderModelCombo(const vspCommandHandler& rSender, _Inout_ OLECMD* pOleCmd, _Inout_ OLECMDTEXT* pOleText)
{
    GT_UNREFERENCED_PARAMETER(&rSender);
    GT_UNREFERENCED_PARAMETER(&pOleText);

    if (pOleCmd != nullptr)
    {
        // Initialize the command flags:
        pOleCmd->cmdf = OLECMDF_SUPPORTED;
        wchar_t* pModelNameStr = nullptr;
        wchar_t* pSelectedTreeDoc = nullptr;
        wchar_t* pModelNameStrToUse = nullptr;

        if (VSCORE(vsc_IsDXFolderSelected)())
        {
            pOleCmd->cmdf |= OLECMDF_ENABLED;
            VSCORE(vsc_GetDXFolderModel)(pModelNameStr);

            // if pTypeNameStr is empty
            if (pModelNameStr == nullptr || wcscmp(pModelNameStr, L"") == 0)
            {
                // set the default value to be the first from KA_STR_VS_toolbarTargetData
                ATL::CComSafeArray<BSTR> typeStringsArray(NUM_OF_SHADER_MODELS);
                wchar_t* modelStrings[NUM_OF_SHADER_MODELS] = { KA_STR_VS_toolbarDXShaderModel };
                pModelNameStrToUse = modelStrings[0];
                VSCORE(vsc_SetDXFolderModel)(pModelNameStrToUse, pSelectedTreeDoc);
            }
            else
            {
                pModelNameStrToUse = pModelNameStr;
            }

            // Copy the characters to the buffer - set the combo value
            lstrcpy(pOleText->rgwz, pModelNameStrToUse);
            int len = wcslen(pModelNameStrToUse);
            pOleText->cwActual = (ULONG)(len);
            VSCORE(vscDeleteWcharString)(pModelNameStr);
        }
    }

    return S_OK;
}

// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onShaderModelComboGetList(_In_ vspCommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pVarOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);

    if (pVarOut != nullptr)
    {
        // Define the array of strings for the combo list:

        ATL::CComSafeArray<BSTR> targetStringsArray(NUM_OF_SHADER_MODELS);
        wchar_t* targetStrings[NUM_OF_SHADER_MODELS] = { KA_STR_VS_toolbarDXShaderModel };

        for (int nTarget = 0; nTarget < NUM_OF_SHADER_MODELS; nTarget++)
        {
            BSTR targetBSTR = SysAllocString(targetStrings[nTarget]);
            targetStringsArray.SetAt(nTarget, targetBSTR);
        }

        V_ARRAY(pVarOut) = targetStringsArray.Detach();
        V_VT(pVarOut) = VT_ARRAY | VT_BSTR;
    }

    return S_OK;

}

// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onShaderModelComboChanged(_In_ vspCommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);

    if (pIn != nullptr)
    {
        if (pIn->vt == VT_BSTR)
        {
            // Get the active document file Path:
            wchar_t* pSelectedTreeDoc = nullptr;
            // Invoke core logic.
            VSCORE(vsc_SetDXFolderModel)(pIn->bstrVal, pSelectedTreeDoc);

            // Copy the string to the VARIANT structure:
            if (pOut != nullptr)
            {
                // Copy the string to the VARIANT structure:
                V_VT(pOut) = VT_BSTR;
                V_BSTR(pOut) = SysAllocString(pIn->bstrVal);
            }
        }
    }

    if (pOut != nullptr)
    {
        wchar_t* pModelNameStr = nullptr;
        VSCORE(vsc_GetDXFolderModel)(pModelNameStr);

        if (pModelNameStr != nullptr)
        {
            // Copy the string to the VARIANT structure:
            V_VT(pOut) = VT_BSTR;
            V_BSTR(pOut) = SysAllocString(pModelNameStr);

            VSCORE(vscDeleteWcharString)(pModelNameStr);
        }
    }

    return S_OK;
}

// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onUpdateTypeCombo(const vspCommandHandler& rSender, _Inout_ OLECMD* pOleCmd, _Inout_ OLECMDTEXT* pOleText)
{
    GT_UNREFERENCED_PARAMETER(&rSender);
    GT_UNREFERENCED_PARAMETER(&pOleText);

    if (pOleCmd != nullptr)
    {
        // Initialize the command flags:
        pOleCmd->cmdf = OLECMDF_SUPPORTED;
        wchar_t* pSelectedTreeDoc = nullptr;
        wchar_t* pTypeNameStr = nullptr;
        wchar_t* pTypeNameStrToUse = nullptr;

        if (VSCORE(vsc_IsDXShaderSelected)() && !VSCORE(vsc_IsSourceFileSelected)())
        {
            pOleCmd->cmdf |= OLECMDF_ENABLED;
            VSCORE(vsc_GetSelectedDXShaderType)(pTypeNameStr);

            // if pTypeNameStr is empty
            if (pTypeNameStr == nullptr || wcscmp(pTypeNameStr, L"") == 0)
            {
                // set the default value to be the first from KA_STR_VS_toolbarTargetData
                ATL::CComSafeArray<BSTR> typeStringsArray(VSP_SUPPORTED_DX_SHADERS_COUNT);
                wchar_t* typeStrings[VSP_SUPPORTED_DX_SHADERS_COUNT] = { KA_STR_toolbarDXShaderTypesGT };
                pTypeNameStrToUse = typeStrings[0];
                VSCORE(vsc_SetSelectedDXShaderType)(pTypeNameStrToUse, pSelectedTreeDoc);
            }
            else
            {
                pTypeNameStrToUse = pTypeNameStr;
            }

            // Copy the characters to the buffer - set the combo value
            lstrcpy(pOleText->rgwz, pTypeNameStrToUse);
            int len = wcslen(pTypeNameStrToUse);
            pOleText->cwActual = (ULONG)(len);
            VSCORE(vscDeleteWcharString)(pTypeNameStr);
        }
    }

    return S_OK;
}

// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onTypeComboGetList(_In_ vspCommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pVarOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);

    if (pVarOut != nullptr)
    {
        // Define the array of strings for the combo list:
        ATL::CComSafeArray<BSTR> typeStringsArray(VSP_SUPPORTED_DX_SHADERS_COUNT);
        wchar_t* typeStrings[VSP_SUPPORTED_DX_SHADERS_COUNT] = { KA_STR_toolbarDXShaderTypesGT };

        for (int nType = 0; nType < VSP_SUPPORTED_DX_SHADERS_COUNT; nType++)
        {
            BSTR typeBSTR = SysAllocString(typeStrings[nType]);
            typeStringsArray.SetAt(nType, typeBSTR);
        }

        V_ARRAY(pVarOut) = typeStringsArray.Detach();
        V_VT(pVarOut) = VT_ARRAY | VT_BSTR;
    }

    return S_OK;

}

// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onTypeComboChanged(_In_ vspCommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);

    if (pIn != nullptr)
    {
        if (pIn->vt == VT_BSTR)
        {
            // Get the active document file Path:
            wchar_t* pSelectedTreeDoc = nullptr;
            // Invoke core logic.
            VSCORE(vsc_SetSelectedDXShaderType)(pIn->bstrVal, pSelectedTreeDoc);

            // Copy the string to the VARIANT structure:
            if (pOut != nullptr)
            {
                // Copy the string to the VARIANT structure:
                V_VT(pOut) = VT_BSTR;
                V_BSTR(pOut) = SysAllocString(pIn->bstrVal);
            }
        }
    }

    if (pOut != nullptr)
    {
        wchar_t* pTypeNameStr = nullptr;
        VSCORE(vsc_GetSelectedDXShaderType)(pTypeNameStr);
        assert(pTypeNameStr != nullptr);

        if (pTypeNameStr != nullptr)
        {
            // Copy the string to the VARIANT structure:
            V_VT(pOut) = VT_BSTR;
            V_BSTR(pOut) = SysAllocString(pTypeNameStr);

            VSCORE(vscDeleteWcharString)(pTypeNameStr);
        }
    }

    return S_OK;
}

// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::onUpdateFunctionName
// Description: Handler the update of the Kernel Name combobox
// Author:      Gilad Yarnitzky
// Date:        1/9/2013
// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onUpdateFunctionName(const vspCommandHandler& rSender, _Inout_ OLECMD* pOleCmd, _Inout_ OLECMDTEXT* pOleText)
{
    GT_UNREFERENCED_PARAMETER(&rSender);

    if (pOleCmd != nullptr)
    {
        // Initialize the command flags:
        pOleCmd->cmdf = OLECMDF_SUPPORTED;

        // check that cl or dx file is currently selected
        if (VSCORE(vsc_IsDXShaderSelected)() || VSCORE(vsc_IsCLFileSelected)())
        {
            wchar_t* pSelectedTreeDoc = nullptr;
            bool isEnabled = false;
            wchar_t* kernelName = nullptr;
            size_t kernelNameStrSize = 0;
            VSCORE(vsc_OnUpdateFunctionName)(pSelectedTreeDoc, (int)pOleText->cwBuf, kernelName, kernelNameStrSize, isEnabled);

            if (isEnabled)
            {
                pOleCmd->cmdf |= OLECMDF_ENABLED;
            }

            if (kernelName != nullptr)
            {
                // Copy the characters to the buffer:
                lstrcpy(pOleText->rgwz, kernelName);
                pOleText->cwActual = (ULONG)(kernelNameStrSize);

                // Release the allocated string.
                VSCORE(vscDeleteWcharString)(kernelName);
            }
        }
    }

    return S_OK;
}

// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::onFunctionNameGetList
// Description: Handle the fill data of the Kernel Name combobox
// Author:      Gilad Yarnitzky
// Date:        1/9/2013
// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onFunctionNameGetList(_In_ vspCommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pVarOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);

    if (pVarOut != nullptr)
    {
        // Get the active document file Path:
        wchar_t* pSelectedTreeDoc = nullptr;

        int numKernels = 0;
        wchar_t** pKernelStringsArrayBuffer = nullptr;
        bool isOk = VSCORE(vsc_onFunctionNameGetList)(pSelectedTreeDoc, pKernelStringsArrayBuffer, numKernels);

        if (isOk && pKernelStringsArrayBuffer != nullptr && numKernels > 0)
        {
            // Define the array of strings for the combo list:
            ATL::CComSafeArray<BSTR> kernelsStringsArray(numKernels);

            for (int i = 0; i < numKernels; ++i)
            {
                assert(pKernelStringsArrayBuffer[i] != nullptr);

                if ((pKernelStringsArrayBuffer[i] != nullptr))
                {
                    BSTR kernelValueAsBSTR = SysAllocString(pKernelStringsArrayBuffer[i]);
                    kernelsStringsArray.SetAt(i, kernelValueAsBSTR);

                    // Release the allocated string.
                    VSCORE(vscDeleteWcharString)(pKernelStringsArrayBuffer[i]);
                }
            }

            // Release the allocated string array.
            VSCORE(vscDeleteWcharStringArray)(pKernelStringsArrayBuffer);

            if (isOk)
            {

                V_ARRAY(pVarOut) = kernelsStringsArray.Detach();
                V_VT(pVarOut) = VT_ARRAY | VT_BSTR;
            }
        }
    }

    return S_OK;
}

// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::onFunctionNameChanged
// Description: Handle the change in the kernel name combo list
// Author:      Gilad Yarnitzky
// Date:        1/9/2013
// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onFunctionNameChanged(_In_ vspCommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);

    if (pIn != nullptr)
    {
        if (pIn->vt == VT_BSTR)
        {
            wchar_t* pSelectedTreeDoc = nullptr;
            std::wstring filePathAsStr;
            VSCORE(vsc_OnFunctionNameChanged)(pIn->bstrVal, pSelectedTreeDoc, filePathAsStr.c_str());

            // Copy the string to the VARIANT structure:
            if (pOut != nullptr)
            {
                // Copy the string to the VARIANT structure:
                V_VT(pOut) = VT_BSTR;
                V_BSTR(pOut) = SysAllocString(pIn->bstrVal);
            }
        }
    }

    if (pOut != nullptr)
    {
        wchar_t* pKernelNameStr = nullptr;
        VSCORE(vsc_GetSelectedFunctionName)(pKernelNameStr);

        assert(pKernelNameStr != nullptr);

        if (pKernelNameStr != nullptr)
        {
            // Copy the string to the VARIANT structure:
            V_VT(pOut) = VT_BSTR;
            V_BSTR(pOut) = SysAllocString(pKernelNameStr);

            VSCORE(vscDeleteWcharString)(pKernelNameStr);
        }
    }

    return S_OK;
}

// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::onUpdateBuildOptions
// Description: Handle the update of the build options field
// Author:      Gilad Yarnitzky
// Date:        1/9/2013
// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onUpdateBuildOptions(const vspCommandHandler& rSender, _Inout_ OLECMD* pOleCmd, _Inout_ OLECMDTEXT* pOleText)
{
    GT_UNREFERENCED_PARAMETER(&rSender);

    if (pOleCmd != nullptr)
    {
        // Initialize the command flags:
        pOleCmd->cmdf = OLECMDF_SUPPORTED;

        if (VSCORE(vsc_OnUpdateMode_AnalyzeMode_IsChecked()))
        {
            pOleCmd->cmdf |= OLECMDF_ENABLED;
        }

        wchar_t* pBuildOptionsStr = new wchar_t[VSP_BUILD_OPTION_BUFFER_SIZE];
        int strSize = VSP_BUILD_OPTION_BUFFER_SIZE;

        // Invoke core logic.
        VSCORE(vsc_GetBuildOptionsStr)((int)pOleText->cwBuf, pBuildOptionsStr, strSize);

        if (pOleText != nullptr && pBuildOptionsStr != nullptr)
        {
            // Copy the characters to the buffer:
            lstrcpy(pOleText->rgwz, pBuildOptionsStr);
            pOleText->cwActual = (ULONG)(strSize + 1);
        }

        delete [] pBuildOptionsStr;
    }

    return S_OK;
}


// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::onBuildOptionsChanged
// Description: Handle the change of the field of the options build
// Author:      Gilad Yarnitzky
// Date:        1/9/2013
// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onBuildOptionsChanged(_In_ vspCommandHandler* pSender, DWORD flags, _In_ VARIANT* pIn, _Out_ VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);

    // The user typed a value as text:
    if (nullptr != pIn)
    {
        if ((VT_ARRAY | VT_VARIANT) == pIn->vt)
        {
            SAFEARRAY* pInArray = pIn->parray;

            if (nullptr != pInArray)
            {
                PVOID pArrayData = pInArray->pvData;

                if (nullptr != pArrayData)
                {
                    VARIANT* pVariantArray = (VARIANT*)pArrayData;

                    // Get the fifth elements that has the string
                    if (VT_BSTR == pVariantArray[4].vt)
                    {
                        BSTR buildOptionBstr = pVariantArray[4].bstrVal;

                        if (nullptr != buildOptionBstr)
                        {
                            // Get the user string as gtString:
                            VSCORE(vsc_OnBuildOptionsChanged_SetBuildOptions)(pVariantArray[4].bstrVal);

                            if (pOut != nullptr && pOut->vt == VT_I4)
                            {
                                pOut->intVal = VSFILTERKEYS_DODEFAULT;
                            }
                        }
                    }
                }
            }
        }
    }
    else if (nullptr != pOut)
    {
        wchar_t* pBuildOptionsStr = nullptr;
        VSCORE(vsc_OnBuildOptionsChanged_GetBuildOptions)(pBuildOptionsStr);
        assert(pBuildOptionsStr != nullptr);

        if (pBuildOptionsStr != nullptr)
        {
            // Copy the string to the VARIANT structure:
            V_VT(pOut) = VT_BSTR;
            V_BSTR(pOut) = SysAllocString(pBuildOptionsStr);

            // Release the allocated string.
            VSCORE(vscDeleteWcharString)(pBuildOptionsStr);
        }
    }

    return S_OK;
}

bool vspPackageCommandHandler::IsSolutionLoaded() const
{
    return vspDTEConnector::instance().isSolutionLoaded();
}


// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onPPSelectCounters(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    VSCORE(vsc_OnOpenPPSelectCounters)();
    return S_OK;
}

// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onUpdatePPSelectCounters(vspCommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText)
{
    GT_UNREFERENCED_PARAMETER(&handler);
    GT_UNREFERENCED_PARAMETER(pOleText);

    if (pOleCmd != nullptr)
    {
        // Initialize the command flags:
        pOleCmd->cmdf = OLECMDF_SUPPORTED;

        bool isActionVisible = false;
        bool isEnabled = false;
        // Invoke core logic.
        VSCORE(vsc_OnUpdatePPSelectCounters)(isActionVisible, isEnabled);
        bool solutionLoaded = vspDTEConnector::instance().isSolutionLoaded();

        if (solutionLoaded && isEnabled)
        {
            pOleCmd->cmdf |= OLECMDF_ENABLED;
        }

        if (!isActionVisible)
        {
            pOleCmd->cmdf |= OLECMDF_INVISIBLE;
        }
    }

    return S_OK;
}

// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::onHelpOpenURL
// Description: Command handler for the CodeXL->Help->GPU Developer Tools support forum command
// note:        Moved from vspPackageCommandHandler::onHelpGPUForums
// Author:      Sigal Algranaty
// Date:        25/5/2011
// ---------------------------------------------------------------------------
void vspPackageCommandHandler::onHelpOpenURL(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    assert(pSender != nullptr);

    if (pSender != nullptr)
    {
        if (pSender->GetId().GetId() == cmdidHelpGPUForums)
        {
            // Open the GPU forums URL:
            wchar_t* pUrlStr = nullptr;
            VSCORE(vscGetHelpDevToolsSupportForumURL)(pUrlStr);
            assert(pUrlStr != nullptr);

            if (pUrlStr != nullptr)
            {
                vspDTEConnector::instance().openURL(pUrlStr);

                // Release the allocated string.
                VSCORE(vscDeleteWcharString)(pUrlStr);
            }
        }
        else
        {
            // Unsupported help command
            assert(false);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::onUpdateOptions
// Description: Called to update the teapot sample command
// Author:      Sigal Algranaty
// Date:        20/6/2011
// ---------------------------------------------------------------------------
void vspPackageCommandHandler::OnUpdateSample(vspCommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText)
{
    GT_UNREFERENCED_PARAMETER(pOleText);

    DWORD commandId = handler.GetId().GetId();

    if (pOleCmd != nullptr)
    {
        // Initialize the command flags:
        pOleCmd->cmdf = OLECMDF_SUPPORTED;

        // The teapot sample command is available only when the user is in design mode:
        dbgDebugMode currentMode = vspDTEConnector::instance().currentDebugggerMode();

        // check if in running mode
        bool isEnabled = VSCORE(vsc_IsInRunningMode)();

        if (currentMode == dbgDesignMode && isEnabled)
        {
            pOleCmd->cmdf |= OLECMDF_ENABLED;
        }

        if (commandId == cmdidHelpOpenD3D12MultithreadingSample)
        {
            // Only enable DX12 sample in VS15
            // Identify the VS version.
            VsWindowsManagementMode vsVersion = VS_WMM_UNKNOWN;
            bool rc = GetVSVersion(vsVersion);

            if (rc)
            {
                bool isVisible = (vsVersion == VS_WMM_VS14);

                if (!isVisible)
                {
                    // Show the DX12 sample only for VS15
                    pOleCmd->cmdf |= OLECMDF_INVISIBLE;
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:            onHelpOpenTeapotSample
// Description: Open the solution of the teapot example
// Author:      Gilad Yarnitzkyfa
// Date:        31/5/2011
// ---------------------------------------------------------------------------
void vspPackageCommandHandler::OnHelpOpenTeapotSample(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    VSCORE(vsc_OpenTeapotExample)();
}

void vspPackageCommandHandler::OnHelpOpenMatMulSample(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    VSCORE(vsc_OpenMatMulExample)();
}


void vspPackageCommandHandler::OnHelpOpenD3D12MultithreadingSample(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    VSCORE(vsc_OpenD3D12MultithreadingExample)();
}

// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::onViewHelp
// Description: Command handler for CodeXL->View Help command
// Author:      Gilad Yarnitzky
// Date:        2/2/2011
// ---------------------------------------------------------------------------
void vspPackageCommandHandler::onViewHelp(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    // Show the help about dialog:
    VSCORE(vsc_ViewHelp)();
}

// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::onViewQuickStart
// Description: Command handler for CodeXL->View Help command
// Author:      Gilad Yarnitzky
// Date:        2/2/2011
// ---------------------------------------------------------------------------
void vspPackageCommandHandler::onViewQuickStart(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    // Show the help about dialog:
    VSCORE(vsc_OnViewQuickStart)();
}
// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::onCheckForUpdates
// Description: Command handler for CodeXL->Check for updates command
// Author:      Sigal Algranaty
// Date:        1/3/2012
// ---------------------------------------------------------------------------
void vspPackageCommandHandler::onCheckForUpdates(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    // Show the help about dialog:
    VSCORE(vsc_ShowCheckForUpdateDialog)();
}

// ---------------------------------------------------------------------------
// Name:        vspPackageCommandHandler::onAboutDialog
// Description: Command handler for CodeXL->About Dialog command
// Author:      Gilad Yarnitzky
// Date:        1/2/2011
// ---------------------------------------------------------------------------
void vspPackageCommandHandler::onAboutDialog(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    // Show the help about dialog:
    VSCORE(vsc_ShowAboutDialog)();
}

// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onUpdateCancelBuild(vspCommandHandler& handler, OLECMD* pOleCmd, OLECMDTEXT* pOleText)
{
    GT_UNREFERENCED_PARAMETER(&handler);
    GT_UNREFERENCED_PARAMETER(pOleText);

    bool isActionEnabled = true, isActionChecked = false, isActionVisible = true;

    // Invoke the core logic.
    VSCORE(vsc_OnUpdateCancelBuild_IsActionEnabled)(isActionEnabled);
    VSCORE(vsc_OnUpdateCancelBuild_IsActionVisible)(isActionVisible);

    if (pOleCmd != nullptr)
    {
        DWORD commandFlags = OLECMDF_SUPPORTED;

        if (isActionEnabled)
        {
            commandFlags |= OLECMDF_ENABLED;
        }

        if (isActionChecked)
        {
            commandFlags |= OLECMDF_LATCHED;
        }

        if (!isActionVisible)
        {
            commandFlags |= OLECMDF_INVISIBLE;
        }

        pOleCmd->cmdf = commandFlags;
    }

    return S_OK;
}

// ---------------------------------------------------------------------------
HRESULT vspPackageCommandHandler::onCancelBuild(vspCommandHandler* pSender, DWORD flags, VARIANT* pIn, VARIANT* pOut)
{
    GT_UNREFERENCED_PARAMETER(pSender);
    GT_UNREFERENCED_PARAMETER(flags);
    GT_UNREFERENCED_PARAMETER(pIn);
    GT_UNREFERENCED_PARAMETER(pOut);

    // Invoke core logic.
    VSCORE(vsc_OnCancelBuild)();
    return S_OK;
}
