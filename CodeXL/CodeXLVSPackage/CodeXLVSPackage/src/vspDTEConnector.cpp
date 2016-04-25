//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspDTEConnector.cpp
///
//==================================================================================

//------------------------------ vspDTEConnector.cpp ------------------------------

#include "stdafx.h"
namespace VxDTE
{
#include <dte80.h>
};


// C++:
#include <cassert>
#include <algorithm>
#include <sstream>
#include <queue>

// TinyXML:
#include <tinyxml.h>

// Local:
#include <src/Package.h>
#include <src/vspCoreAPI.h>
#include <src/vspDTEConnector.h>
#include <CodeXLVSPackage/Include/vspStringConstants.h>
#include <src/vspUtils.h>
#include <src/vscVsUtils.h>

#pragma warning(push)
#pragma warning(disable : 4336)
// See https://connect.microsoft.com/VisualStudio/feedback/details/503783/unable-to-access-new-compiler-options-from-envdte-vcprojectengine-dll
// and http://connectppe.microsoft.com/VisualStudio/feedback/details/518484/typelibrary-generation-fails-on-microsoft-visualstudio-project-visualc-vcprojectengine
#ifdef VSP_VS11BUILD
    // When building the VS11 / VS12 / VS14 extension with the VS10 runtime, we need to specify the full path for the type library, since the relative path starts at the VS10 folder:
    #import <C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcpackages\vcpb2.tlb> rename("PropertySheet", "VC_PropertySheet") rename("GetObject", "VC_GetObject") raw_interfaces_only named_guids
#elif defined VSP_VS12BUILD
    #import <C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcpackages\vcpb2.tlb> rename("PropertySheet", "VC_PropertySheet") rename("GetObject", "VC_GetObject") raw_interfaces_only named_guids
#elif defined VSP_VS14BUILD
    #import <C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcpackages\vcpb2.tlb> rename("PropertySheet", "VC_PropertySheet") rename("GetObject", "VC_GetObject") raw_interfaces_only named_guids
#else
    #import <VC\vcpackages\vcpb2.tlb> rename("PropertySheet", "VC_PropertySheet") rename("GetObject", "VC_GetObject") raw_interfaces_only named_guids
#endif
#import <MSEnv\VSLangProj.olb> raw_interfaces_only named_guids
//#import <Common7\IDE\PublicAssemblies\Microsoft.VisualStudio.VCProjectEngine.dll>
#pragma warning(pop)

static IVsHierarchy* getProjectHierarchy(VxDTE::Project& project);
static bool isAppContainer(VxDTE::Project& project);
static bool parseAppxRecipe(VxDTE::Project& project, BSTR configurationName, std::wstring& appUserModelId);

// Static members initializations:
vspDTEConnector* vspDTEConnector::_pMySingleInstance = NULL;

#define prjStartActionProject 0
// Indicates that the executable file (for Windows Application and Console Application projects) or the Start Page (for Web projects) should be started when the application is debugged. Class library projects cannot be started directly.
#define prjStartActionProgram 1
// Indicates that a specific program should be started when the application is debugged.
#define prjStartActionURL 2
// Indicates that a particular URL should be accessed when the application is debugged.
#define prjStartActionNone 3
// Indicates that no project, program, or URL should be started when the application is debugged.


// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::vspDTEConnector
// Description: Constructor
// Author:      Uri Shomroni
// Date:        6/10/2010
// ---------------------------------------------------------------------------
vspDTEConnector::vspDTEConnector()
    : _piDTE(NULL), _piDebugger(NULL), _piItemOperations(NULL), m_pCoreImple(VSCORE(vscDTEConnector_CreateInstance)())
{

}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::~vspDTEConnector
// Description: Destructor
// Author:      Uri Shomroni
// Date:        6/10/2010
// ---------------------------------------------------------------------------
vspDTEConnector::~vspDTEConnector()
{
    // Release the DTE and the interfaces we got from it:
    releaseDTEInterface();

    // Release the core instance.
    VSCORE(vscDTEConnector_DestroyInstance)(m_pCoreImple);
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::instance
// Description: Returns the single instance of this class. Creates it on
//              the first call to this function.
// Author:      Uri Shomroni
// Date:        6/10/2010
// ---------------------------------------------------------------------------
vspDTEConnector& vspDTEConnector::instance()
{
    if (_pMySingleInstance == NULL)
    {
        _pMySingleInstance = new vspDTEConnector;

    }

    return *_pMySingleInstance;
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::setDTEInterface
// Description: Sets the DTE interface for this class
// Author:      Uri Shomroni
// Date:        6/10/2010
// ---------------------------------------------------------------------------
void vspDTEConnector::setDTEInterface(VxDTE::_DTE* piDTE)
{
    // We shouldn't get here twice, but make sure that we didn't, anyway:
    if (_piDTE != NULL)
    {
        assert(_piDTE == NULL);
        _piDTE->Release();
        _piDTE = NULL;
    }

    if (_piDebugger != NULL)
    {
        assert(_piDebugger == NULL);
        _piDebugger->Release();
        _piDebugger = NULL;
    }

    if (_piItemOperations != NULL)
    {
        assert(_piItemOperations == NULL);
        _piItemOperations->Release();
        _piItemOperations = NULL;
    }

    // Copy the pointer:
    _piDTE = piDTE;

    // Register command listener
    registerCommandListener();

    // If we got a pointer, retain it:
    if (_piDTE != NULL)
    {
        _piDTE->AddRef();

        // Get the Debugger interface:
        VxDTE::Debugger* piDebugger = NULL;
        HRESULT hr = _piDTE->get_Debugger(&piDebugger);
        bool isOk = (SUCCEEDED(hr)) && (piDebugger != NULL);
        assert(isOk);

        if (isOk)
        {
            _piDebugger = piDebugger;
        }

        // Get the item operations interface:
        VxDTE::ItemOperations* piItemOperations = NULL;
        hr = _piDTE->get_ItemOperations(&piItemOperations);
        isOk = (SUCCEEDED(hr)) && (piItemOperations != NULL);
        assert(isOk);

        if (isOk)
        {
            _piItemOperations = piItemOperations;
        }
    }
}

void vspDTEConnector::registerCommandListener()
{
    assert(_piDTE != NULL);

    if (_piDTE != NULL)
    {
        VxDTE::Globals* piGlobals = NULL;
        HRESULT hr = _piDTE->get_Globals(&piGlobals);

        if (SUCCEEDED(hr) && (piGlobals != NULL))
        {
            VARIANT radix = {0};
            hr = piGlobals->get_VariableValue(L"radix", &radix);
            VARIANT names = {0};
            piGlobals->get_VariableNames(&names);
            SAFEARRAY* globalsAsArray = names.parray;

            for (int nProject = 0 ; nProject < (int)globalsAsArray->rgsabound[0].cElements ; nProject++)
            {
                // Get the project from the array:
                VARIANT* pGlobalVarArray = (VARIANT*)globalsAsArray->pvData;
                VARIANT pGlobalVar = pGlobalVarArray[nProject];

                if (pGlobalVar.vt == VT_BSTR)
                {

                }
            }

            piGlobals->Release();
        }
    }


    /*  GT_IF_WITH_ASSERT(_piDTE != NULL)
    {
    // Connect to command listener:
    VxDTE::Events* piEvents = NULL;
    HRESULT hr = _piDTE->get_Events(&piEvents);
    if (SUCCEEDED(hr) && (piEvents != NULL))
    {
    VxDTE::_CommandEvents* piCommandsEvents = NULL;
    hr = piEvents->get_CommandEvents(L"{00000000-0000-0000-0000-000000000000}", 0, (VxDTE::CommandEvents**)(&piCommandsEvents));
    if (SUCCEEDED(hr) && (piCommandsEvents != NULL))
    {
    piCommandsEvents->Release();
    }
    // Release events interface:
    piEvents->Release();
    }
    }
    */
}
// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::releaseDTEInterface
// Description: Releases the DTE and its dependent interfaces
// Author:      Uri Shomroni
// Date:        15/12/2010
// ---------------------------------------------------------------------------
void vspDTEConnector::releaseDTEInterface()
{
    // Release the DTE:
    if (_piDTE != NULL)
    {
        _piDTE->Release();
        _piDTE = NULL;
    }

    // Release the Debugger:
    if (_piDebugger != NULL)
    {
        _piDebugger->Release();
        _piDebugger = NULL;
    }

    // Release the item operations interface:
    if (_piItemOperations != NULL)
    {
        _piItemOperations->Release();
        _piItemOperations = NULL;
    }

}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::getActiveProjectDebugInformation
// Description: Gets the active project from the DTE and extracts the details
//              needed to debug it.
// Author:      Uri Shomroni
// Date:        6/10/2010
// ---------------------------------------------------------------------------
void vspDTEConnector::getActiveProjectDebugInformation(std::wstring& executableFilePath, std::wstring& workingDirectoryPath, std::wstring& commandLineArguments, std::wstring& environment, bool& isProjectTypeValid)
{
    executableFilePath.clear();
    workingDirectoryPath.clear();
    commandLineArguments.clear();
    environment.clear();
    isProjectTypeValid = false;

    assert(_piDTE != NULL);

    if (_piDTE != NULL)
    {
        // Get the active projects (we expect this to be an array of variants):
        VARIANT activeSolutionProjects = {0};
        HRESULT hr = _piDTE->get_ActiveSolutionProjects(&activeSolutionProjects);

        if (SUCCEEDED(hr) && (activeSolutionProjects.vt == (VT_ARRAY | VT_VARIANT)))
        {
            // Get the projects as an array:
            SAFEARRAY* pProjects = activeSolutionProjects.parray;

            if ((pProjects != NULL) && (pProjects->cDims > 0))
            {
                // Get the first variant in the array (we expect it to be a dispatch):
                VARIANT* pFirstActiveProjectAsVar = (VARIANT*)(pProjects->pvData);

                if ((pFirstActiveProjectAsVar != NULL) && (pFirstActiveProjectAsVar->vt == VT_DISPATCH))
                {
                    // Query for the project interface:
                    IDispatch* pFirstActiveProjectAsDispatch = pFirstActiveProjectAsVar->pdispVal;
                    VxDTE::Project* pFirstActiveProject = NULL;
                    hr = pFirstActiveProjectAsDispatch->QueryInterface(VxDTE::IID_Project, (void**)&pFirstActiveProject);

                    if (SUCCEEDED(hr) && (pFirstActiveProject != NULL))
                    {
                        // Get the debug information from the project:
                        bool isNonNativeProject = false;
                        getDebugInformationFromProject(*pFirstActiveProject, executableFilePath, workingDirectoryPath, commandLineArguments, environment, isProjectTypeValid, isNonNativeProject);

                        // Release the project:
                        pFirstActiveProject->Release();
                    }

                    // Don't release the dispatch, the VariantClear function should do this for us:
                    // pFirstActiveProjectAsDispatch->Release();
                }

                // Don't clear the variant, the main VariantClear function should do this for us:
                // VariantClear(pFirstActiveProjectAsVar);
            }

            // Don't destroy the array, the VariantClear function should do this for us:
            // SafeArrayDestroy(pProjects);
        }

        // Clear the active projects variant:
        VariantClear(&activeSolutionProjects);
    }
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::getStartupProjectDebugInformation
// Description: Gets the active project from the DTE and extracts the details
//              needed to debug it.
// Author:      Uri Shomroni
// Date:        6/10/2010
// ---------------------------------------------------------------------------
void vspDTEConnector::getStartupProjectDebugInformation(std::wstring& executableFilePath, std::wstring& workingDirectoryPath, std::wstring& commandLineArguments, std::wstring& environment,
                                                        bool& isProjectOpen, bool& isProjectTypeValid, bool& isNonNativeProject)
{
    executableFilePath.clear();
    workingDirectoryPath.clear();
    commandLineArguments.clear();
    environment.clear();
    isProjectTypeValid = false;
    isProjectOpen = false;
    isNonNativeProject = false;

    assert(_piDTE != NULL);

    if (_piDTE != NULL)
    {
        // Get the solution:
        VxDTE::_Solution* pSolution = NULL;
        HRESULT hr = _piDTE->get_Solution((VxDTE::Solution**)(&pSolution));

        if (SUCCEEDED(hr) && (pSolution != NULL))
        {
            // Get the build interface:
            VxDTE::SolutionBuild* pSolutionBuild = NULL;
            hr = pSolution->get_SolutionBuild(&pSolutionBuild);

            if (SUCCEEDED(hr) && (pSolutionBuild != NULL))
            {
                // Get the startup projects (we expect this to be an array of variants):
                VARIANT startupProjects = {0};
                hr = pSolutionBuild->get_StartupProjects(&startupProjects);

                if (SUCCEEDED(hr) && (startupProjects.vt == (VT_ARRAY | VT_VARIANT)))
                {
                    // Get the project paths as an array:
                    SAFEARRAY* pStartupProjectPathsArray = startupProjects.parray;

                    if ((pStartupProjectPathsArray != NULL) && (pStartupProjectPathsArray->cDims > 0))
                    {
                        // Get the first variant in the array (we expect it to be a string):
                        VARIANT* pFirstStartupProjectPathAsVar = (VARIANT*)(pStartupProjectPathsArray->pvData);

                        if ((pFirstStartupProjectPathAsVar != NULL) && (pFirstStartupProjectPathAsVar->vt == VT_BSTR))
                        {
                            // Use the project path (as a variant) to access the appropriate item in the solution.
                            // Solution items are projects:
                            VxDTE::Project* pFirstStartupProject = NULL;
                            hr = pSolution->Item(*pFirstStartupProjectPathAsVar, &pFirstStartupProject);

                            if (!SUCCEEDED(hr) || (pFirstStartupProject == NULL))
                            {
                                // If we couldn't get the item directly, the project is probably inside a folder. We need to look for it manually:
                                std::wstring projectNameString(pFirstStartupProjectPathAsVar->bstrVal);
                                processProjectNameForSearch(projectNameString);
                                pFirstStartupProject = getProjectByName(*pSolution, projectNameString);
                            }

                            if (pFirstStartupProject != NULL)
                            {
                                // Get the debug information from the project:
                                getDebugInformationFromProject(*pFirstStartupProject, executableFilePath, workingDirectoryPath, commandLineArguments, environment, isProjectTypeValid, isNonNativeProject);

                                // Release the project:
                                pFirstStartupProject->Release();
                            }
                        }

                        // Don't clear the variant, the main VariantClear function should do this for us:
                        // VariantClear(pFirstStartupProjectPathAsVar);
                    }

                    // Don't destroy the array, the VariantClear function should do this for us:
                    // SafeArrayDestroy(pStartupProjectPathsArray);
                    isProjectOpen = true;
                }
                else if (SUCCEEDED(hr) && (startupProjects.vt == VT_EMPTY))
                {
                    // There is no project:
                    isProjectTypeValid = true;
                    isProjectOpen = false;
                }


                // Clear the startup projects variant:
                VariantClear(&startupProjects);
            }

            // Release the build interface:
            pSolutionBuild->Release();
        }

        // Release the solution:
        pSolution->Release();
    }
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::hasStartupProjectExecutableChanged
// Description: Gets the executable string from the startup project. If it is different
//              than io_executableFilePath, io_executableFilePath is overwritten with the
//              new value and true is returned. Otherwise, false is returned.
// Author:      Uri Shomroni
// Date:        20/10/2010
// ---------------------------------------------------------------------------
bool vspDTEConnector::hasStartupProjectExecutableChanged(std::wstring& io_executableFilePath)
{
    bool retVal = false;
    bool foundCommand = false;

    if (_piDTE != NULL)
    {
        // Get the solution:
        VxDTE::_Solution* pSolution = NULL;
        HRESULT hr = _piDTE->get_Solution((VxDTE::Solution**)(&pSolution));

        if (SUCCEEDED(hr) && (pSolution != NULL))
        {
            // Get the build interface:
            VxDTE::SolutionBuild* pSolutionBuild = NULL;
            hr = pSolution->get_SolutionBuild(&pSolutionBuild);

            if (SUCCEEDED(hr) && (pSolutionBuild != NULL))
            {
                // Get the startup projects (we expect this to be an array of variants):
                VARIANT startupProjects = {0};
                hr = pSolutionBuild->get_StartupProjects(&startupProjects);

                if (SUCCEEDED(hr) && (startupProjects.vt == (VT_ARRAY | VT_VARIANT)))
                {
                    // Get the project paths as an array:
                    SAFEARRAY* pStartupProjectPathsArray = startupProjects.parray;

                    if ((pStartupProjectPathsArray != NULL) && (pStartupProjectPathsArray->cDims > 0))
                    {
                        // Get the first variant in the array (we expect it to be a string):
                        VARIANT* pFirstStartupProjectPathAsVar = (VARIANT*)(pStartupProjectPathsArray->pvData);

                        if ((pFirstStartupProjectPathAsVar != NULL) && (pFirstStartupProjectPathAsVar->vt == VT_BSTR))
                        {
                            // Use the project path (as a variant) to access the appropriate item in the solution.
                            // Solution items are projects:
                            VxDTE::Project* pFirstStartupProject = NULL;
                            hr = pSolution->Item(*pFirstStartupProjectPathAsVar, &pFirstStartupProject);

                            if (!SUCCEEDED(hr) || (pFirstStartupProject == NULL))
                            {
                                // If we couldn't get the item directly, the project is probably inside a folder. We need to look for it manually:
                                std::wstring projectNameString(pFirstStartupProjectPathAsVar->bstrVal);
                                processProjectNameForSearch(projectNameString);
                                pFirstStartupProject = getProjectByName(*pSolution, projectNameString);
                            }

                            if (pFirstStartupProject != NULL)
                            {
                                // Get the project's configuration manager:
                                VxDTE::ConfigurationManager* pConfigManager = NULL;
                                hr = pFirstStartupProject->get_ConfigurationManager(&pConfigManager);

                                if (SUCCEEDED(hr) && (pConfigManager != NULL))
                                {
                                    // Get the active configuration:
                                    VxDTE::Configuration* pActiveConfiguration = NULL;
                                    hr = pConfigManager->get_ActiveConfiguration(&pActiveConfiguration);

                                    if (SUCCEEDED(hr) && (pActiveConfiguration != NULL))
                                    {
                                        // Get the configuration's properties object:
                                        VxDTE::Properties* pActiveConfigurationProperties = NULL;
                                        hr = pActiveConfiguration->get_Properties(&pActiveConfigurationProperties);

                                        if (SUCCEEDED(hr) && pActiveConfigurationProperties != NULL)
                                        {
                                            // Get the executable path:
                                            static std::wstring currentPathVal;
                                            static const std::wstring commandPropertyName = L"Command"; // Native projects
                                            static const std::wstring startActionPropertyName = L"StartAction"; // CLR projects
                                            static const std::wstring startProgramPropertyName = L"StartProgram"; // CLR projects

                                            getStringPropertyValue(*pActiveConfigurationProperties, commandPropertyName, currentPathVal, foundCommand);

                                            if (foundCommand)
                                            {
                                                // If this is a Visual C project, evaluate macros:
                                                if (currentPathVal.empty())
                                                {
                                                    currentPathVal = L"$(TargetPath)";
                                                }

                                                // If the path has macros, evaluate them:
                                                static const std::wstring userMacroStart = L"$(";

                                                if (-1 != currentPathVal.find(userMacroStart))
                                                {
                                                    // Try to evaluate any macros such as $(TargetPath), etc.:
                                                    static std::wstring ignored;
                                                    evaluateUserMacros(*pFirstStartupProject, *pActiveConfiguration, true, currentPathVal, ignored, ignored, ignored);
                                                }
                                            }
                                            else // !foundCommand
                                            {
                                                // Check if this is a CLR project:
                                                int startAction = -1;
                                                bool startActionDefined = false;
                                                getIntegerPropertyValue(*pActiveConfigurationProperties, startActionPropertyName, startAction, startActionDefined);

                                                if (startActionDefined)
                                                {
                                                    // Check if the selected launch method is "external program":
                                                    if (prjStartActionProgram == startAction)
                                                    {
                                                        getStringPropertyValue(*pActiveConfigurationProperties, startProgramPropertyName, currentPathVal, foundCommand);
                                                    }
                                                }

                                                // For cases where StartAction was not defined or we could not get the StartProgram value, fall back to the target URL:
                                                if (!foundCommand)
                                                {
                                                    getConfigurationTargetPath(*pActiveConfiguration, currentPathVal);

                                                    foundCommand = true;
                                                }
                                            }

                                            // If we found something, compare it:
                                            if (foundCommand)
                                            {
                                                // If the value changed:
                                                if (currentPathVal != io_executableFilePath)
                                                {
                                                    // Mark it as changed and save the new value:
                                                    io_executableFilePath = currentPathVal;
                                                    retVal = true;
                                                }
                                            }
                                        }

                                        // Release the configuration:
                                        pActiveConfiguration->Release();
                                    }

                                    // Release the configuration manager:
                                    pConfigManager->Release();
                                }

                                // Release the project:
                                pFirstStartupProject->Release();
                            }
                        }

                        // Don't clear the variant, the main VariantClear function should do this for us:
                        // VariantClear(pFirstStartupProjectPathAsVar);
                    }

                    // Don't destroy the array, the VariantClear function should do this for us:
                    // SafeArrayDestroy(pStartupProjectPathsArray);
                }
                else if (SUCCEEDED(hr) && (startupProjects.vt == VT_EMPTY))
                {
                    foundCommand = false;
                }

                // Clear the startup projects variant:
                VariantClear(&startupProjects);
            }

            // Release the build interface:
            pSolutionBuild->Release();
        }

        // Release the solution:
        pSolution->Release();
    }

    // If we failed at any point, consider the return value to be an empty string:
    if (!foundCommand)
    {
        // There is no project:
        if (!io_executableFilePath.empty())
        {
            io_executableFilePath.clear();
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::isSolutionLoaded
// Description: Returns true iff a solution is loaded that has projects
// Author:      Uri Shomroni
// Date:        20/10/2010
// ---------------------------------------------------------------------------
bool vspDTEConnector::isSolutionLoaded()
{
    bool retVal = false;

    assert(_piDTE != NULL);

    if (_piDTE != NULL)
    {
        // Check if we have a solution:
        VxDTE::_Solution* piSolution = NULL;
        HRESULT hr = _piDTE->get_Solution((VxDTE::Solution**)(&piSolution));

        if (SUCCEEDED(hr) && (piSolution != NULL))
        {
            // Check if this solution has projects:
            VxDTE::Projects* piProjects = NULL;
            hr = piSolution->get_Projects(&piProjects);

            if (SUCCEEDED(hr) && (piProjects != NULL))
            {
                // If we have at least one project, return true:
                long amountOfProjects = 0;
                hr = piProjects->get_Count(&amountOfProjects);
                retVal = SUCCEEDED(hr) && (amountOfProjects > 0);

                // Release the projects:
                piProjects->Release();
            }

            // Release the solution:
            piSolution->Release();
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::getSelectedEditorText
// Description: Returns the current active document selected text.
// Author:      Sigal Algranaty
// Date:        6/3/2011
// ---------------------------------------------------------------------------
bool vspDTEConnector::getSelectedEditorText(std::wstring& selectedText)
{
    bool retVal = false;

    assert(_piDTE != NULL);

    if (_piDTE != NULL)
    {
        // Check if we have an active document:
        VxDTE::Document* piDocument = NULL;
        HRESULT hr = _piDTE->get_ActiveDocument((VxDTE::Document**)(&piDocument));

        if (SUCCEEDED(hr) && (piDocument != NULL))
        {
            // Check if this solution has projects:
            VxDTE::TextSelection* piTextSelection = NULL;
            hr = piDocument->get_Selection((IDispatch**)&piTextSelection);

            if (SUCCEEDED(hr) && (piTextSelection != NULL))
            {
                BSTR pText = NULL;
                hr = piTextSelection->get_Text(&pText);

                if (SUCCEEDED(hr) && (pText != NULL))
                {
                    // Copy the selected text to a string:
                    selectedText = pText;
                    retVal = true;
                    SysFreeString(pText);
                }

                // Release the text selection:
                piTextSelection->Release();
            }

            // Release the document:
            piDocument->Release();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::forceVariablesReevaluation
// Description: Forces the expressions to be re-evaluated in windows such as Watch,
//              Locals, etc.
//              This is done by forcing the evaluation of a dummy expression.
// Author:      Uri Shomroni
// Date:        20/3/2011
// ---------------------------------------------------------------------------
void vspDTEConnector::forceVariablesReevaluation()
{
    if (_piDebugger != NULL)
    {
        // Create the dummy items used for this:
        BSTR dummyExprBSTR = SysAllocString(VSP_STR_ForceVariableRefreshPseudoVariable);
        VxDTE::Expression* piExpr = NULL;
        HRESULT hr = _piDebugger->GetExpression(dummyExprBSTR, VARIANT_FALSE, 1000, &piExpr);

        if (SUCCEEDED(hr) && piExpr != NULL)
        {
            // Release the expression, we do not need it:
            piExpr->Release();
        }

        // Release the string:
        SysFreeString(dummyExprBSTR);
    }
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::shouldBuildProject
// Description: checks if projects needs build
// projectNames - list of project names that needs to be built.
// Return Val:  true - Needs build
// Author:      Gilad Yarnitzky
// Date:        24/3/2011
// ---------------------------------------------------------------------------
void vspDTEConnector::constructProjectsNames(std::vector<VxDTE::Project*>& projectsList, std::vector<std::wstring>& projectNames)
{
    assert(_piDTE != NULL);

    if (_piDTE != NULL)
    {
        // Get the solution:
        VxDTE::_Solution* pSolution = NULL;
        HRESULT hr = _piDTE->get_Solution((VxDTE::Solution**)(&pSolution));

        if (SUCCEEDED(hr) && (pSolution != NULL))
        {
            // Get the build interface:
            VxDTE::SolutionBuild* pSolutionBuild = NULL;
            hr = pSolution->get_SolutionBuild(&pSolutionBuild);

            if (SUCCEEDED(hr) && (pSolutionBuild != NULL))
            {

                VxDTE::SolutionConfiguration* pSolutionConfiguration = NULL;
                hr = pSolutionBuild->get_ActiveConfiguration(&pSolutionConfiguration);

                if (SUCCEEDED(hr) && (pSolutionBuild != NULL))
                {
                    VxDTE::SolutionContexts* pSolutionContexts = NULL;
                    hr = pSolutionConfiguration->get_SolutionContexts(&pSolutionContexts);

                    if (SUCCEEDED(hr) && (pSolutionContexts != NULL))
                    {
                        long numContexts = 0;
                        hr = pSolutionContexts->get_Count(&numContexts);

                        if (SUCCEEDED(hr))
                        {
                            VARIANT index = {0};
                            index.intVal = 0;
                            index.vt = VT_INT;

                            for (int nContext = 1; nContext <= numContexts; nContext ++)
                            {
                                index.intVal = nContext;
                                VxDTE::SolutionContext* pSolutionContext = NULL;
                                hr = pSolutionContexts->Item(index, &pSolutionContext);

                                if (SUCCEEDED(hr) && (pSolutionContext != NULL))
                                {
                                    // Check if the context is at the project dependency list at all:
                                    BSTR contextNameBSTR = NULL;
                                    BSTR projectShortNameBSTR = NULL;
                                    HRESULT hrProjectName = S_FALSE;

                                    hr = pSolutionContext->get_ProjectName(&contextNameBSTR);

                                    if (SUCCEEDED(hr) && (contextNameBSTR != NULL))
                                    {
                                        bool foundProject = false;

                                        for (int nProject = 0 ; nProject < (int)projectsList.size(); nProject++)
                                        {
                                            BSTR projectNameBSTR = NULL;
                                            hr = projectsList[nProject]->get_UniqueName(&projectNameBSTR);

                                            if (SUCCEEDED(hr) && (projectNameBSTR != NULL))
                                            {
                                                if (_tcsicmp(projectNameBSTR, contextNameBSTR) == 0)
                                                {
                                                    foundProject = true;
                                                    hrProjectName = projectsList[nProject]->get_Name(&projectShortNameBSTR);
                                                    break;
                                                }

                                                SysFreeString(projectNameBSTR);
                                            }
                                        }

                                        if (foundProject)
                                        {
                                            // build the name as it should appear:
                                            BSTR contextConfigNameBSTR = NULL;
                                            BSTR platformNameBSTR = NULL;

                                            HRESULT hrConfigName = pSolutionContext->get_ConfigurationName(&contextConfigNameBSTR);
                                            HRESULT hrPlatformName = pSolutionContext->get_PlatformName(&platformNameBSTR);

                                            std::wstring projectName;

                                            if (SUCCEEDED(hrProjectName) && (projectShortNameBSTR != NULL))
                                            {
                                                projectName = projectShortNameBSTR;
                                            }

                                            if (SUCCEEDED(hrConfigName) && (contextConfigNameBSTR != NULL))
                                            {
                                                projectName.append(L" - ");
                                                projectName.append(contextConfigNameBSTR);
                                            }

                                            if (SUCCEEDED(hrPlatformName) && (platformNameBSTR != NULL))
                                            {
                                                projectName.append(L" ");
                                                projectName.append(platformNameBSTR);
                                            }

                                            projectNames.push_back(projectName);

                                            if (contextConfigNameBSTR != NULL)
                                            {
                                                SysFreeString(contextConfigNameBSTR);
                                            }

                                            if (platformNameBSTR != NULL)
                                            {
                                                SysFreeString(platformNameBSTR);
                                            }

                                            if (projectShortNameBSTR != NULL)
                                            {
                                                SysFreeString(projectShortNameBSTR);
                                            }
                                        }

                                        SysFreeString(contextNameBSTR);
                                    }

                                    // Release the context interface:
                                    pSolutionContext->Release();
                                }
                            }
                        }

                        // Release the contexts interface:
                        pSolutionContexts->Release();
                    }

                    // Release the configuration interface:
                    pSolutionConfiguration->Release();
                }

                // Release the build interface:
                pSolutionBuild->Release();
            }

            // Release the solution:
            pSolution->Release();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::deployStartupProject
// Description: deploys the project
// Author:      Ehud Katz
// Date:        28/7/2013
// ---------------------------------------------------------------------------
bool vspDTEConnector::deployStartupProject(std::wstring& appUserModelId)
{
    bool delpoySucceeded = false;

    assert(_piDTE != NULL);

    if (_piDTE != NULL)
    {
        // Get the solution:
        VxDTE::_Solution* pSolution = NULL;
        HRESULT hr = _piDTE->get_Solution((VxDTE::Solution**)(&pSolution));

        if (SUCCEEDED(hr) && (pSolution != NULL))
        {
            // Get the build interface:
            VxDTE::SolutionBuild* pSolutionBuild = NULL;
            hr = pSolution->get_SolutionBuild(&pSolutionBuild);

            if (SUCCEEDED(hr) && (pSolutionBuild != NULL))
            {
                VxDTE::SolutionBuild2* pSolutionBuild2 = NULL;
                hr = pSolutionBuild->QueryInterface(&pSolutionBuild2);

                if (SUCCEEDED(hr) && (pSolutionBuild2 != NULL))
                {
                    // Get the startup projects (we expect this to be an array of variants):
                    VARIANT startupProjects = {0};
                    hr = pSolutionBuild2->get_StartupProjects(&startupProjects);

                    if (SUCCEEDED(hr) && (startupProjects.vt == (VT_ARRAY | VT_VARIANT)))
                    {
                        // Get the project paths as an array:
                        SAFEARRAY* pStartupProjectPathsArray = startupProjects.parray;

                        if ((pStartupProjectPathsArray != NULL) && (pStartupProjectPathsArray->cDims > 0))
                        {
                            // Get the first variant in the array (we expect it to be a string):
                            VARIANT* pFirstStartupProjectPathAsVar = (VARIANT*)(pStartupProjectPathsArray->pvData);

                            if ((pFirstStartupProjectPathAsVar != NULL) && (pFirstStartupProjectPathAsVar->vt == VT_BSTR))
                            {
                                // Use the project path (as a variant) to access the appropriate item in the solution.
                                // Solution items are projects:
                                VxDTE::Project* pFirstStartupProject = NULL;
                                hr = pSolution->Item(*pFirstStartupProjectPathAsVar, &pFirstStartupProject);

                                if (!SUCCEEDED(hr) || (pFirstStartupProject == NULL))
                                {
                                    // If we couldn't get the item directly, the project is probably inside a folder. We need to look for it manually:
                                    std::wstring projectNameString(pFirstStartupProjectPathAsVar->bstrVal);
                                    processProjectNameForSearch(projectNameString);
                                    pFirstStartupProject = getProjectByName(*pSolution, projectNameString);
                                }

                                if (pFirstStartupProject != NULL)
                                {
                                    if (isAppContainer(*pFirstStartupProject))
                                    {
                                        BSTR projectUniqueName = NULL;
                                        hr = pFirstStartupProject->get_UniqueName(&projectUniqueName);

                                        if (SUCCEEDED(hr) && (projectUniqueName != NULL))
                                        {
                                            VxDTE::ConfigurationManager* pConfigManager = NULL;
                                            hr = pFirstStartupProject->get_ConfigurationManager(&pConfigManager);

                                            if (SUCCEEDED(hr) && (pConfigManager != NULL))
                                            {
                                                // Get the active configuration:
                                                VxDTE::Configuration* pActiveConfiguration = NULL;
                                                hr = pConfigManager->get_ActiveConfiguration(&pActiveConfiguration);

                                                if (SUCCEEDED(hr) && (pActiveConfiguration != NULL))
                                                {
                                                    std::wstring activeConfigurationName;

                                                    if (getConfigurationFullName(*pActiveConfiguration, activeConfigurationName))
                                                    {
                                                        BSTR activeConfigurationNameBSTR = SysAllocString(activeConfigurationName.c_str());
                                                        hr = pSolutionBuild2->DeployProject(activeConfigurationNameBSTR, projectUniqueName, VARIANT_TRUE);

                                                        if (SUCCEEDED(hr))
                                                        {
                                                            delpoySucceeded = parseAppxRecipe(*pFirstStartupProject, activeConfigurationNameBSTR, appUserModelId);
                                                        }

                                                        SysFreeString(activeConfigurationNameBSTR);
                                                        pActiveConfiguration->Release();
                                                    }
                                                }

                                                pConfigManager->Release();
                                                // Release the project:
                                            }

                                            SysFreeString(projectUniqueName);
                                        }
                                    }
                                    else
                                    {
                                        delpoySucceeded = true;
                                    }

                                    pFirstStartupProject->Release();
                                }

                                // Don't clear the variant, the main VariantClear function should do this for us:
                                // VariantClear(pFirstStartupProjectPathAsVar);
                            }

                            // Don't destroy the array, the VariantClear function should do this for us:
                            // SafeArrayDestroy(pStartupProjectPathsArray);
                        }

                        // Clear the startup projects variant:
                        VariantClear(&startupProjects);
                    }

                    // Release the 2nd build interface:
                    pSolutionBuild2->Release();
                }

                // Release the build interface:
                pSolutionBuild->Release();
            }

            // Release the solution:
            pSolution->Release();
        }
    }

    return delpoySucceeded;
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::verifyStartupProjectBuilt
// Description: builds the project
// Author:      Gilad Yarnitzky
// Date:        23/3/2011
// ---------------------------------------------------------------------------
bool vspDTEConnector::verifyStartupProjectBuilt()
{
    bool compileSucceeded = true;

    assert(_piDTE != NULL);

    if (_piDTE != NULL)
    {
        // Get the solution:
        VxDTE::_Solution* pSolution = NULL;
        HRESULT hr = _piDTE->get_Solution((VxDTE::Solution**)(&pSolution));

        if (SUCCEEDED(hr) && (pSolution != NULL))
        {
            // Get the build interface:
            VxDTE::SolutionBuild* pSolutionBuild = NULL;
            hr = pSolution->get_SolutionBuild(&pSolutionBuild);

            if (SUCCEEDED(hr) && (pSolutionBuild != NULL))
            {
                // Get the startup projects (we expect this to be an array of variants):
                VARIANT startupProjects = {0};
                hr = pSolutionBuild->get_StartupProjects(&startupProjects);

                if (SUCCEEDED(hr) && (startupProjects.vt == (VT_ARRAY | VT_VARIANT)))
                {
                    // Get the project paths as an array:
                    SAFEARRAY* pStartupProjectPathsArray = startupProjects.parray;

                    if ((pStartupProjectPathsArray != NULL) && (pStartupProjectPathsArray->cDims > 0))
                    {
                        // Get the first variant in the array (we expect it to be a string):
                        VARIANT* pFirstStartupProjectPathAsVar = (VARIANT*)(pStartupProjectPathsArray->pvData);

                        if ((pFirstStartupProjectPathAsVar != NULL) && (pFirstStartupProjectPathAsVar->vt == VT_BSTR))
                        {
                            // Use the project path (as a variant) to access the appropriate item in the solution.
                            // Solution items are projects:
                            VxDTE::Project* pFirstStartupProject = NULL;
                            hr = pSolution->Item(*pFirstStartupProjectPathAsVar, &pFirstStartupProject);

                            if (!SUCCEEDED(hr) || (pFirstStartupProject == NULL))
                            {
                                // If we couldn't get the item directly, the project is probably inside a folder. We need to look for it manually:
                                std::wstring projectNameString(pFirstStartupProjectPathAsVar->bstrVal);
                                processProjectNameForSearch(projectNameString);
                                pFirstStartupProject = getProjectByName(*pSolution, projectNameString);
                            }

                            if (pFirstStartupProject != NULL)
                            {
                                BSTR projectUniqueName = NULL;
                                hr = pFirstStartupProject->get_UniqueName(&projectUniqueName);

                                if (SUCCEEDED(hr) && (projectUniqueName != NULL))
                                {
                                    VxDTE::ConfigurationManager* pConfigManager = NULL;
                                    hr = pFirstStartupProject->get_ConfigurationManager(&pConfigManager);

                                    if (SUCCEEDED(hr) && (pConfigManager != NULL))
                                    {
                                        // Get the active configuration:
                                        VxDTE::Configuration* pActiveConfiguration = NULL;
                                        hr = pConfigManager->get_ActiveConfiguration(&pActiveConfiguration);

                                        if (SUCCEEDED(hr) && (pActiveConfiguration != NULL))
                                        {
                                            std::wstring activeConfigurationName;

                                            if (getConfigurationFullName(*pActiveConfiguration, activeConfigurationName))
                                            {
                                                BSTR activeConfigurationNameBSTR = SysAllocString(activeConfigurationName.c_str());
                                                hr = pSolutionBuild->BuildProject(activeConfigurationNameBSTR, projectUniqueName, VARIANT_TRUE);
                                                long numFailed;
                                                pSolutionBuild->get_LastBuildInfo(&numFailed);

                                                if (numFailed != 0)
                                                {
                                                    compileSucceeded = false;
                                                }

                                                SysFreeString(activeConfigurationNameBSTR);
                                                pActiveConfiguration->Release();
                                            }
                                        }

                                        pConfigManager->Release();
                                        // Release the project:
                                    }

                                    SysFreeString(projectUniqueName);
                                }

                                pFirstStartupProject->Release();
                            }

                            // Don't clear the variant, the main VariantClear function should do this for us:
                            // VariantClear(pFirstStartupProjectPathAsVar);
                        }

                        // Don't destroy the array, the VariantClear function should do this for us:
                        // SafeArrayDestroy(pStartupProjectPathsArray);
                    }

                    // Clear the startup projects variant:
                    VariantClear(&startupProjects);
                }

                // Release the build interface:
                pSolutionBuild->Release();
            }

            // Release the solution:
            pSolution->Release();
        }
    }

    return compileSucceeded;
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::getBuildDependencies
// Description: creates the list of dependency projects for active project
// projectsList: Projects that active projects depends on (including itself)
// Author:      Gilad Yarnitzky
// Date:        27/3/2011
// ---------------------------------------------------------------------------
void vspDTEConnector::getBuildDependencies(std::vector<VxDTE::Project*>& projectsList)
{
    // Create the list of startup projects list that will be the root of the tree
    // of the dependency look tree:
    assert(_piDTE != NULL);

    if (_piDTE != NULL)
    {
        // Get the solution:
        VxDTE::_Solution* pSolution = NULL;
        HRESULT hr = _piDTE->get_Solution((VxDTE::Solution**)(&pSolution));

        if (SUCCEEDED(hr) && (pSolution != NULL))
        {
            // Get the build interface:
            VxDTE::SolutionBuild* pSolutionBuild = NULL;
            hr = pSolution->get_SolutionBuild(&pSolutionBuild);

            if (SUCCEEDED(hr) && (pSolutionBuild != NULL))
            {
                VARIANT pProjects = {0};

                pSolutionBuild->get_StartupProjects(&pProjects);

                if (SUCCEEDED(hr) && (pProjects.vt == (VT_ARRAY | VT_VARIANT)))
                {
                    // Get the project paths as an array:
                    SAFEARRAY* pStartupProjectPathsArray = pProjects.parray;

                    if ((pStartupProjectPathsArray != NULL) && (pStartupProjectPathsArray->cDims > 0))
                    {
                        // Get the first variant in the array (we expect it to be a string):
                        VARIANT* pFirstStartupProjectPathAsVar = (VARIANT*)(pStartupProjectPathsArray->pvData);

                        if ((pFirstStartupProjectPathAsVar != NULL) && (pFirstStartupProjectPathAsVar->vt == VT_BSTR))
                        {
                            // Use the project path (as a variant) to access the appropriate item in the solution.
                            // Solution items are projects:
                            VxDTE::Project* pFirstStartupProject = NULL;
                            hr = pSolution->Item(*pFirstStartupProjectPathAsVar, &pFirstStartupProject);

                            if (!SUCCEEDED(hr) || (pFirstStartupProject == NULL))
                            {
                                // If we couldn't get the item directly, the project is probably inside a folder. We need to look for it manually:
                                std::wstring projectNameString(pFirstStartupProjectPathAsVar->bstrVal);
                                processProjectNameForSearch(projectNameString);
                                pFirstStartupProject = getProjectByName(*pSolution, projectNameString);
                            }

                            if (pFirstStartupProject != NULL)
                            {
                                if (addProjectBuildDependencies(projectsList, pFirstStartupProject))
                                {
                                    projectsList.push_back(pFirstStartupProject);
                                }
                            }
                        }
                    }

                    // Clear the startup projects:
                    VariantClear(&pProjects);
                }

                // Release the build interface:
                pSolutionBuild->Release();
            }

            // Release the solution:
            pSolution->Release();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::addProjectBuildDependencies
// Description: add all dependency projects of a specific project to the list
// projectsList: Projects that active projects depends on (including itself)
// currentProject: specific project to add its dependencies to the list
// Author:      Gilad Yarnitzky
// Date:        27/3/2011
// ---------------------------------------------------------------------------
bool vspDTEConnector::addProjectBuildDependencies(std::vector<VxDTE::Project*>& projectsList, VxDTE::Project* currentProject)
{
    // First set the flag based on the project status and not just its dependencies:
    bool needsBuild = !projectUpToDate(*currentProject);

    assert(_piDTE != NULL);

    if (_piDTE != NULL)
    {
        BSTR projectNameChecked = NULL;
        HRESULT hr = currentProject->get_UniqueName(&projectNameChecked);

        if (SUCCEEDED(hr) && (NULL != projectNameChecked))
        {
            // Special case for CodeXL sample, which supposed to be up to data:
            if (_tcsstr(projectNameChecked, VSP_STR_TeapotSampleName) != NULL)
            {
                needsBuild = false;
            }

            else if (_tcsstr(projectNameChecked, VSP_STR_MatMulSampleName) != NULL)
            {
                needsBuild = false;
            }
            else
            {
                // Get the solution:
                VxDTE::_Solution* pSolution = NULL;
                hr = _piDTE->get_Solution((VxDTE::Solution**)(&pSolution));

                if (SUCCEEDED(hr) && (pSolution != NULL))
                {
                    // Get the build interface:
                    VxDTE::SolutionBuild* pSolutionBuild = NULL;
                    hr = pSolution->get_SolutionBuild(&pSolutionBuild);

                    if (SUCCEEDED(hr) && (pSolutionBuild != NULL))
                    {
                        VxDTE::BuildDependencies* pBuildDependencies;

                        hr = pSolutionBuild->get_BuildDependencies(&pBuildDependencies);

                        if (SUCCEEDED(hr) && (pBuildDependencies != NULL))
                        {
                            long numDependencies;
                            hr = pBuildDependencies->get_Count(&numDependencies);

                            if (SUCCEEDED(hr))
                            {
                                VARIANT dependItem = {0};
                                dependItem.vt = VT_INT;
                                dependItem.intVal = 0;

                                for (long nDepend = 1 ; nDepend <= numDependencies ; nDepend++)
                                {
                                    dependItem.intVal = nDepend;
                                    VxDTE::BuildDependency* pBuildDependency;
                                    hr = pBuildDependencies->Item(dependItem, &pBuildDependency);

                                    if (SUCCEEDED(hr) && (pBuildDependency != NULL))
                                    {
                                        VxDTE::Project* pProject;
                                        hr = pBuildDependency->get_Project(&pProject);

                                        if (SUCCEEDED(hr) && (pProject != NULL))
                                        {
                                            BSTR projectName = NULL;
                                            hr = pProject->get_UniqueName(&projectName);

                                            if (SUCCEEDED(hr) && (NULL != projectName))
                                            {
                                                // If the two are identical add the new project dependent projects to the list
                                                if (_tcsicmp(projectName, projectNameChecked) == 0)
                                                {
                                                    VARIANT pProjectsList = {0};

                                                    hr = pBuildDependency->get_RequiredProjects(&pProjectsList);

                                                    if (SUCCEEDED(hr) && (pProjectsList.vt == (VT_ARRAY | VT_VARIANT)))
                                                    {
                                                        SAFEARRAY* pProjectsArray = pProjectsList.parray;

                                                        if (pProjectsArray != NULL && pProjectsArray->cDims > 0)
                                                        {
                                                            for (int nProject = 0 ; nProject < (int)pProjectsArray->rgsabound[0].cElements ; nProject++)
                                                            {
                                                                // Get the project from the array:
                                                                VARIANT* pProgramVarArray = (VARIANT*)pProjectsArray->pvData;
                                                                VARIANT pProgramVar = pProgramVarArray[nProject];

                                                                if (pProgramVar.vt == VT_DISPATCH)
                                                                {
                                                                    // Check that the program is not already in the list of programs:
                                                                    IDispatch* pProgramVarDispatch = pProgramVar.pdispVal;

                                                                    VxDTE::Project* projectToAdd = NULL;
                                                                    hr = pProgramVarDispatch->QueryInterface(VxDTE::IID_Project, (void**)&projectToAdd);

                                                                    if (SUCCEEDED(hr) && (projectToAdd != NULL))
                                                                    {
                                                                        BSTR projectToAddName = NULL;
                                                                        hr = projectToAdd->get_UniqueName(&projectToAddName);

                                                                        if (SUCCEEDED(hr) && (NULL != projectToAddName))
                                                                        {
                                                                            bool projectFound = false;

                                                                            for (int nExisting = 0 ; nExisting < (int)projectsList.size(); nExisting++)
                                                                            {
                                                                                BSTR projectExistingName = NULL;
                                                                                hr = projectsList[nExisting]->get_UniqueName(&projectExistingName);

                                                                                if (SUCCEEDED(hr) && (NULL != projectExistingName))
                                                                                {
                                                                                    if (_tcsicmp(projectExistingName, projectToAddName) == 0)
                                                                                    {
                                                                                        projectFound = true;
                                                                                    }

                                                                                    SysFreeString(projectExistingName);
                                                                                }

                                                                                SysFreeString(projectToAddName);
                                                                            }

                                                                            // check if the project and dependencies is out of date
                                                                            bool outOfDate = (!projectUpToDate(*projectToAdd) || addProjectBuildDependencies(projectsList, projectToAdd));

                                                                            if (outOfDate)
                                                                            {
                                                                                needsBuild = true;
                                                                            }

                                                                            if (!projectFound)
                                                                            {
                                                                                // Add project:
                                                                                // If the project itself is out of date we need it or
                                                                                // If on if its dependency project is out of date we need it:
                                                                                if (outOfDate)
                                                                                {
                                                                                    projectsList.push_back(projectToAdd);
                                                                                }
                                                                            }
                                                                            else
                                                                            {
                                                                                projectToAdd->Release();
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }

                                                        // Clear the projects array:
                                                        VariantClear(&pProjectsList);
                                                    }
                                                }

                                                SysFreeString(projectName);
                                            }

                                            // Release project item:
                                            pProject->Release();
                                        }

                                        // Release Dependency item:
                                        pBuildDependency->Release();
                                    }
                                }
                            }

                            // Release Dependency list:
                            pBuildDependencies->Release();
                        }

                        // Release the build interface:
                        pSolutionBuild->Release();
                    }

                    // Release the solution:
                    pSolution->Release();
                }
            }

            // Release the project name:
            SysFreeString(projectNameChecked);
        }
    }

    return needsBuild;
};

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::projectUpToDate
// Description: If the project is a Visual C project, uses the VCProjectEngine
//              namespace's VCConfiguration::Eval function to check if the project
//              is up to date
// Author:      Gilad Yarnitzky
// Date:        31/3/2011
// ---------------------------------------------------------------------------
bool vspDTEConnector::projectUpToDate(VxDTE::Project& project)
{
    // This is a translation to C++ of the C# code here: http://bitbucket.org/sweinst/windows/src/89222c99e96e/SVTools/SVTools/DebugWizard/DebugWizardForm.cs
    // Note that attempting to get the VCProject interface directly from the Project interface fails.
    // So Does getting the VCConfiguration directly from the Configuration interface (or through its Dispatch).
    // As the C# code illustrates, we must go up to the project object and ask for its configuration by name.

    bool projectUpToDate = true;

    VxDTE::ConfigurationManager* pConfigManager = NULL;
    HRESULT hr = project.get_ConfigurationManager(&pConfigManager);

    if (SUCCEEDED(hr) && (pConfigManager != NULL))
    {
        // Get the active configuration:
        VxDTE::Configuration* pActiveConfiguration = NULL;
        hr = pConfigManager->get_ActiveConfiguration(&pActiveConfiguration);

        if (SUCCEEDED(hr) && (pActiveConfiguration != NULL))
        {
            // Get the project "object", i.e. its dispatch interface:
            IDispatch* pProjectAsDispatch = NULL;
            hr = project.get_Object(&pProjectAsDispatch);

            if (SUCCEEDED(hr) && (pProjectAsDispatch != NULL))
            {
                // Get the project's "Visual C Project" interface. If this fails, the project is not a VC/VC++ project,
                // so we don't support it (and it doesn't support user macros):
                VCProjectEngineLibrary::VCProject* pProjectAsVCProject = NULL;
                hr = pProjectAsDispatch->QueryInterface(VCProjectEngineLibrary::IID_VCProject, (void**)(&pProjectAsVCProject));

                if (SUCCEEDED(hr) && (pProjectAsVCProject != NULL))
                {
                    // Get the configurations object:
                    IDispatch* pVCConfigurationsAsDispatch = NULL;
                    hr = pProjectAsVCProject->get_Configurations(&pVCConfigurationsAsDispatch);

                    if (SUCCEEDED(hr) && (pVCConfigurationsAsDispatch != NULL))
                    {
                        // Get the configurations object's (configuration) collection interface
                        VCProjectEngineLibrary::IVCCollection* pVCConfigurations = NULL;
                        hr = pVCConfigurationsAsDispatch->QueryInterface(VCProjectEngineLibrary::IID_IVCCollection, (void**)(&pVCConfigurations));

                        if (SUCCEEDED(hr) && (pVCConfigurations != NULL))
                        {
                            // Get the active configuration's full name:
                            std::wstring configurationFullName;
                            bool rcConfName = getConfigurationFullName(*pActiveConfiguration, configurationFullName);

                            if (rcConfName && (!configurationFullName.empty()))
                            {
                                // Create a variant with the active configuration's name:
                                VARIANT activeConfNameAsVar = {0};
                                activeConfNameAsVar.vt = VT_BSTR;
                                activeConfNameAsVar.bstrVal = SysAllocString(configurationFullName.c_str());

                                // Use the variant to get the active configuration from the Visual C configurations collection:
                                IDispatch* pActiveConfigurationAsDispatch = NULL;
                                hr = pVCConfigurations->Item(activeConfNameAsVar, &pActiveConfigurationAsDispatch);

                                if (SUCCEEDED(hr) && (pActiveConfigurationAsDispatch != NULL))
                                {
                                    // Get the configuration's Visual C configuration interface:
                                    VCProjectEngineLibrary::VCConfiguration* pActiveConfigurationAsVCConfig = NULL;
                                    hr = pActiveConfigurationAsDispatch->QueryInterface(VCProjectEngineLibrary::IID_VCConfiguration, (void**)(&pActiveConfigurationAsVCConfig));

                                    if (SUCCEEDED(hr) && (pActiveConfigurationAsVCConfig != NULL))
                                    {
                                        VARIANT_BOOL upToDateVar = VARIANT_TRUE;
                                        hr = pActiveConfigurationAsVCConfig->get_UpToDate(&upToDateVar);

                                        if (SUCCEEDED(hr))
                                        {
                                            if (upToDateVar == VARIANT_FALSE)
                                            {
                                                projectUpToDate = false;
                                            }
                                        }

                                        // Release the VCConfiguration interface:
                                        pActiveConfigurationAsVCConfig->Release();
                                    }

                                    // Release the VC configuration's dispatch interface:
                                    pActiveConfigurationAsDispatch->Release();
                                }

                                // Clear the name variant:
                                SysFreeString(activeConfNameAsVar.bstrVal);
                                activeConfNameAsVar.bstrVal = NULL;
                                VariantClear(&activeConfNameAsVar);
                            }

                            // Release the configurations collection's IVCCollection interface:
                            pVCConfigurations->Release();
                        }

                        // Release the configurations collection's dispatch interface:
                        pVCConfigurationsAsDispatch->Release();
                    }

                    // Release the project's VCProject interface:
                    pProjectAsVCProject->Release();
                }

                // Release the project object:
                pProjectAsDispatch->Release();
            }

            // Release the configuration:
            pActiveConfiguration->Release();
        }

        // Release the configuration manager:
        pConfigManager->Release();
    }

    return projectUpToDate;
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::resumeDebugging
// Description: Resumes debugging or starts a normal debugging session, equivalent
//              to pressing the F5 key (or the built-in "Play" button)
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        20/10/2010
// ---------------------------------------------------------------------------
bool vspDTEConnector::resumeDebugging()
{
    bool retVal = false;

    assert(_piDebugger != NULL);

    if (_piDebugger != NULL)
    {
        // Tell the debugger to go:
        HRESULT hr = _piDebugger->Go(VARIANT_FALSE);
        retVal = SUCCEEDED(hr);

        // TO_DO: there seems to be a problem that causes the built-in tool bars not to update.
        // we need to find some sort of UI controller to tell to update.
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::addBreakpointInSourceLocation
// Description: Adds a breakpoint at the requested location.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        8/11/2010
// ---------------------------------------------------------------------------
bool vspDTEConnector::addBreakpointInSourceLocation(const std::wstring& filePath, int lineNumber, bool enabled)
{
    bool retVal = false;

    assert(_piDebugger != NULL);

    if (_piDebugger != NULL)
    {
        // Get the breakpoints collection:
        VxDTE::Breakpoints* piBreakpoints = NULL;
        HRESULT hr = _piDebugger->get_Breakpoints(&piBreakpoints);

        if (SUCCEEDED(hr) && (piBreakpoints != NULL))
        {
            // Add the breakpoint (Ignore the returned Breakpoints interface parameter, as we only add one breakpoint):
            BSTR filePathAsBSTR = SysAllocString(filePath.c_str());
            hr = piBreakpoints->Add(NULL, filePathAsBSTR, (long)lineNumber, 0, NULL, VxDTE::dbgBreakpointConditionTypeWhenTrue, NULL, NULL, 0, NULL, 0, VxDTE::dbgHitCountTypeNone, NULL);

            if (SUCCEEDED(hr))
            {
                retVal = true;
            }

            SysFreeString(filePathAsBSTR);

            // Release the breakpoints interface:
            piBreakpoints->Release();
            piBreakpoints = NULL;
        }

        // If we want this to be disabled, we must find it in the breakpoints array and disable it:
        if (retVal && !enabled)
        {
            // Get the breakpoints array, as it may have changed:
            hr = _piDebugger->get_Breakpoints(&piBreakpoints);

            if (SUCCEEDED(hr) && (piBreakpoints != NULL))
            {
                // Get the number of breakpoints:
                long numberOfBreakpoints = 0;
                hr = piBreakpoints->get_Count(&numberOfBreakpoints);

                if (SUCCEEDED(hr))
                {
                    // Iterate the breakpoints:
                    VARIANT iAsVar = {0};
                    iAsVar.intVal = 0;
                    iAsVar.vt = VT_INT;

                    for (long i = 0; i < numberOfBreakpoints; i++)
                    {
                        // Get the current breakpoint interface:
                        iAsVar.intVal = (INT)i + 1; // DTE indices are 1-based.
                        VxDTE::Breakpoint* piCurrentBreakpoint = NULL;
                        hr = piBreakpoints->Item(iAsVar, &piCurrentBreakpoint);

                        if (SUCCEEDED(hr) && (piCurrentBreakpoint != NULL))
                        {
                            // Get this breakpoint's file location (ignore non-"file-line" breakpoints):
                            BSTR currentBreakpointFileName = NULL;
                            hr = piCurrentBreakpoint->get_File(&currentBreakpointFileName);

                            if (SUCCEEDED(hr) && (currentBreakpointFileName != NULL))
                            {
                                // Get the line Number:
                                long currentBreakpointLineNumber = -1;
                                hr = piCurrentBreakpoint->get_FileLine(&currentBreakpointLineNumber);

                                if (SUCCEEDED(hr) && (currentBreakpointLineNumber == lineNumber) && (!filePath.compare(currentBreakpointFileName)))
                                {
                                    piCurrentBreakpoint->put_Enabled(VARIANT_FALSE);
                                }

                                // Release the string:
                                SysFreeString(currentBreakpointFileName);
                            }

                            // Release the breakpoint:
                            piCurrentBreakpoint->Release();
                        }
                    }
                }

                // Release the breakpoints interface:
                piBreakpoints->Release();
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::removeBreakpointsInSourceLocation
// Description: Removes all breakpoints at the requested location
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        8/11/2010
// ---------------------------------------------------------------------------
bool vspDTEConnector::removeBreakpointsInSourceLocation(const std::wstring& filePath, int lineNumber)
{
    bool retVal = false;

    // Will get the breakpoints we need to delete:
    std::vector<VxDTE::Breakpoint*> breakpointsToBeDeleted;

    assert(_piDebugger != NULL);

    if (_piDebugger != NULL)
    {
        // Get the breakpoints collection:
        VxDTE::Breakpoints* piBreakpoints = NULL;
        HRESULT hr = _piDebugger->get_Breakpoints(&piBreakpoints);

        if (SUCCEEDED(hr) && (piBreakpoints != NULL))
        {
            // Iterate the breakpoints:
            long numOfBreakpoints = 0;
            hr = piBreakpoints->get_Count(&numOfBreakpoints);

            if (SUCCEEDED(hr))
            {
                retVal = true;
                const std::wstring filePathAsString = filePath;
                VARIANT iAsVar = {0};
                iAsVar.vt = VT_INT;
                iAsVar.intVal = 0;

                // Iterate the breakpoints:
                for (long i = 0; i < numOfBreakpoints; i++)
                {
                    VxDTE::Breakpoint* piCurrentBreakpoint = NULL;
                    iAsVar.intVal = (INT)i + 1; // DTE indices are 1-based.
                    hr = piBreakpoints->Item(iAsVar, &piCurrentBreakpoint);

                    if (SUCCEEDED(hr) && piCurrentBreakpoint != NULL)
                    {
                        bool deleteBkpt = false;

                        // Get the breakpoint file path:
                        BSTR currentBreakpointFilePathAsBSTR = NULL;
                        hr = piCurrentBreakpoint->get_File(&currentBreakpointFilePathAsBSTR);

                        if (SUCCEEDED(hr) && (currentBreakpointFilePathAsBSTR != NULL))
                        {
                            // Compare the value:
                            if (filePathAsString == currentBreakpointFilePathAsBSTR)
                            {
                                // Get the line number:
                                long currentBreakpointLineNum = -1;
                                hr = piCurrentBreakpoint->get_FileLine(&currentBreakpointLineNum);

                                if (SUCCEEDED(hr))
                                {
                                    // Compare the value:
                                    if ((int)currentBreakpointLineNum == lineNumber)
                                    {
                                        // This breakpoint matches, delete it:
                                        deleteBkpt = true;
                                    }
                                }
                            }

                            // Release the path string:
                            SysFreeString(currentBreakpointFilePathAsBSTR);
                        }

                        // If we aren't going to delete this breakpoint, release the interface:
                        if (deleteBkpt)
                        {
                            breakpointsToBeDeleted.push_back(piCurrentBreakpoint);
                        }
                        else
                        {
                            piCurrentBreakpoint->Release();
                        }
                    }
                }
            }

            // Release the breakpoints interface:
            piBreakpoints->Release();
        }
    }

    // Delete all the breakpoints we accumulated:
    int numberOfBreakpointsToDelete = (int)breakpointsToBeDeleted.size();

    for (int i = 0; i < numberOfBreakpointsToDelete; i++)
    {
        // Delete the breakpoint:
        VxDTE::Breakpoint* piCurrentBreakpoint = breakpointsToBeDeleted[i];
        HRESULT hr = piCurrentBreakpoint->Delete();
        retVal = retVal && SUCCEEDED(hr);

        // Release the interface:
        piCurrentBreakpoint->Release();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::disableFunctionBreakpoint
// Description: Finds the requested breakpoint if it exists, and disables it
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        8/2/2011
// ---------------------------------------------------------------------------
bool vspDTEConnector::disableFunctionBreakpoint(const std::wstring& functionName)
{
    bool retVal = false;

    // Get the breakpoints collection:
    VxDTE::Breakpoints* piBreakpoints = NULL;
    HRESULT hr = _piDebugger->get_Breakpoints(&piBreakpoints);

    if (SUCCEEDED(hr) && (piBreakpoints != NULL))
    {
        // Get the number of breakpoints:
        long numberOfBreakpoints = 0;
        hr = piBreakpoints->get_Count(&numberOfBreakpoints);

        if (SUCCEEDED(hr))
        {
            // Iterate the breakpoints:
            VARIANT iAsVar = {0};
            iAsVar.vt = VT_INT;
            iAsVar.intVal = 0;

            for (long i = 0; i < numberOfBreakpoints; i++)
            {
                // Get the current breakpoint interface:
                iAsVar.intVal = (INT)i + 1; // DTE indices are 1-based.
                VxDTE::Breakpoint* piCurrentBreakpoint = NULL;
                hr = piBreakpoints->Item(iAsVar, &piCurrentBreakpoint);

                if (SUCCEEDED(hr) && (piCurrentBreakpoint != NULL))
                {
                    // Get this breakpoint's function name (ignore non-"function-offset" breakpoints):
                    BSTR currentBreakpointFunctionName = NULL;
                    hr = piCurrentBreakpoint->get_FunctionName(&currentBreakpointFunctionName);

                    if (SUCCEEDED(hr) && (currentBreakpointFunctionName != NULL))
                    {
                        if (functionName == std::wstring(currentBreakpointFunctionName))
                        {
                            piCurrentBreakpoint->put_Enabled(VARIANT_FALSE);

                            // We found and disabled at least one instance of this breakpoint, consider it a success:
                            retVal = true;
                        }

                        // Release the string:
                        SysFreeString(currentBreakpointFunctionName);
                    }

                    // Release the breakpoint:
                    piCurrentBreakpoint->Release();
                }
            }
        }

        // Release the breakpoints interface:
        piBreakpoints->Release();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::isSourceLocationBreakpointEnabled
// Description: If the specified breakpoint exists, sets isEnabled to its status
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        8/2/2011
// ---------------------------------------------------------------------------
bool vspDTEConnector::isSourceLocationBreakpointEnabled(const wchar_t* filePath, int lineNumber, bool& isEnabled)
{
    bool retVal = false;

    // Get the breakpoints collection:
    VxDTE::Breakpoints* piBreakpoints = NULL;
    HRESULT hr = _piDebugger->get_Breakpoints(&piBreakpoints);

    if (SUCCEEDED(hr) && (piBreakpoints != NULL))
    {
        // Get the number of breakpoints:
        long numberOfBreakpoints = 0;
        hr = piBreakpoints->get_Count(&numberOfBreakpoints);

        if (SUCCEEDED(hr))
        {
            // Iterate the breakpoints:
            VARIANT iAsVar = {0};
            iAsVar.intVal = 0;
            iAsVar.vt = VT_INT;

            for (long i = 0; i < numberOfBreakpoints; i++)
            {
                // Get the current breakpoint interface:
                iAsVar.intVal = (INT)i + 1; // DTE indices are 1-based.
                VxDTE::Breakpoint* piCurrentBreakpoint = NULL;
                hr = piBreakpoints->Item(iAsVar, &piCurrentBreakpoint);

                if (SUCCEEDED(hr) && (piCurrentBreakpoint != NULL))
                {
                    // Get this breakpoint's file location (ignore non-"file-line" breakpoints):
                    BSTR currentBreakpointFileName = NULL;
                    hr = piCurrentBreakpoint->get_File(&currentBreakpointFileName);

                    if (SUCCEEDED(hr) && (currentBreakpointFileName != NULL))
                    {
                        // Get the line Number:
                        long currentBreakpointLineNumber = -1;
                        hr = piCurrentBreakpoint->get_FileLine(&currentBreakpointLineNumber);

                        if (SUCCEEDED(hr) && (currentBreakpointLineNumber == lineNumber) && (filePath == std::wstring(currentBreakpointFileName)))
                        {
                            VARIANT_BOOL isEnabledAsVar = VARIANT_FALSE;
                            hr = piCurrentBreakpoint->get_Enabled(&isEnabledAsVar);

                            if (SUCCEEDED(hr))
                            {
                                // Return the value:
                                isEnabled = (isEnabledAsVar != VARIANT_FALSE);

                                // We found the breakpoint, stop looking:
                                retVal = true;
                                break;
                            }
                        }

                        // Release the string:
                        SysFreeString(currentBreakpointFileName);
                    }

                    // Release the breakpoint:
                    piCurrentBreakpoint->Release();
                }
            }
        }

        // Release the breakpoints interface:
        piBreakpoints->Release();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::isFunctionBreakpointEnabled
// Description: If the specified breakpoint exists, sets isEnabled to its status
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        8/2/2011
// ---------------------------------------------------------------------------
bool vspDTEConnector::isFunctionBreakpointEnabled(const wchar_t* functionName, bool& isEnabled)
{
    bool retVal = false;

    // Get the breakpoints collection:
    VxDTE::Breakpoints* piBreakpoints = NULL;
    HRESULT hr = _piDebugger->get_Breakpoints(&piBreakpoints);

    if (SUCCEEDED(hr) && (piBreakpoints != NULL))
    {
        // Get the number of breakpoints:
        long numberOfBreakpoints = 0;
        hr = piBreakpoints->get_Count(&numberOfBreakpoints);

        if (SUCCEEDED(hr))
        {
            // Iterate the breakpoints:
            VARIANT iAsVar = {0};
            iAsVar.vt = VT_INT;
            iAsVar.intVal = 0;

            for (long i = 0; i < numberOfBreakpoints; i++)
            {
                // Get the current breakpoint interface:
                iAsVar.intVal = (INT)i + 1; // DTE indices are 1-based.
                VxDTE::Breakpoint* piCurrentBreakpoint = NULL;
                hr = piBreakpoints->Item(iAsVar, &piCurrentBreakpoint);

                if (SUCCEEDED(hr) && (piCurrentBreakpoint != NULL))
                {
                    // Get this breakpoint's function name (ignore non-"function-offset" breakpoints):
                    BSTR currentBreakpointFunctionName = NULL;
                    hr = piCurrentBreakpoint->get_FunctionName(&currentBreakpointFunctionName);

                    if (SUCCEEDED(hr) && (currentBreakpointFunctionName != NULL))
                    {
                        if (functionName == std::wstring(currentBreakpointFunctionName))
                        {
                            VARIANT_BOOL isEnabledAsVar = VARIANT_FALSE;
                            hr = piCurrentBreakpoint->get_Enabled(&isEnabledAsVar);

                            if (SUCCEEDED(hr))
                            {
                                // Return the value:
                                isEnabled = (isEnabledAsVar != VARIANT_FALSE);

                                // We found the breakpoint, stop looking:
                                retVal = true;
                                break;
                            }
                        }

                        // Release the string:
                        SysFreeString(currentBreakpointFunctionName);
                    }

                    // Release the breakpoint:
                    piCurrentBreakpoint->Release();
                }
            }
        }

        // Release the breakpoints interface:
        piBreakpoints->Release();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::getFunctionBreakpoints
// Description: Gets all function breakpoints
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        10/2/2011
// ---------------------------------------------------------------------------
bool vspDTEConnector::getFunctionBreakpoints(std::vector<std::wstring>& enabledBreakpoints, std::vector<std::wstring>& disabledBreakpoints)
{
    bool retVal = false;
    enabledBreakpoints.clear();
    disabledBreakpoints.clear();

    // Get the breakpoints collection:
    VxDTE::Breakpoints* piBreakpoints = NULL;
    HRESULT hr = _piDebugger->get_Breakpoints(&piBreakpoints);
    retVal = SUCCEEDED(hr);

    if (retVal && (piBreakpoints != NULL))
    {
        // Get the number of breakpoints:
        long numberOfBreakpoints = 0;
        hr = piBreakpoints->get_Count(&numberOfBreakpoints);

        if (SUCCEEDED(hr))
        {
            // Iterate the breakpoints:
            VARIANT iAsVar = {0};
            iAsVar.vt = VT_INT;
            iAsVar.intVal = 0;

            for (long i = 0; i < numberOfBreakpoints; i++)
            {
                // Get the current breakpoint interface:
                iAsVar.intVal = (INT)i + 1; // DTE indices are 1-based.
                VxDTE::Breakpoint* piCurrentBreakpoint = NULL;
                hr = piBreakpoints->Item(iAsVar, &piCurrentBreakpoint);

                if (SUCCEEDED(hr) && (piCurrentBreakpoint != NULL))
                {
                    // Get this breakpoint's function name (ignore non-"function-offset" breakpoints):
                    BSTR currentBreakpointFunctionName = NULL;
                    hr = piCurrentBreakpoint->get_FunctionName(&currentBreakpointFunctionName);

                    if (SUCCEEDED(hr) && (currentBreakpointFunctionName != NULL))
                    {
                        VARIANT_BOOL isEnabledAsVar = VARIANT_FALSE;
                        hr = piCurrentBreakpoint->get_Enabled(&isEnabledAsVar);

                        if (SUCCEEDED(hr))
                        {
                            // Add the function name to a vector according to its enable status:
                            bool isEnabled = (isEnabledAsVar != VARIANT_FALSE);

                            if (isEnabled)
                            {
                                enabledBreakpoints.push_back(std::wstring(currentBreakpointFunctionName));
                            }
                            else // !isEnabled
                            {
                                disabledBreakpoints.push_back(std::wstring(currentBreakpointFunctionName));
                            }
                        }

                        // Release the string:
                        SysFreeString(currentBreakpointFunctionName);
                    }

                    // Release the breakpoint:
                    piCurrentBreakpoint->Release();
                }
            }
        }

        // Release the breakpoints interface:
        piBreakpoints->Release();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::openFileAtPosition
// Description: Opens a given file (same as File > Open) at the location specified
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        13/10/2010
// ---------------------------------------------------------------------------
bool vspDTEConnector::openFileAtPosition(const std::wstring& filePath, int lineNumber /*= 0*/, bool selectLine /*= true*/, int columnNumber /*= 0*/)
{
    GT_UNREFERENCED_PARAMETER(columnNumber);

    bool retVal = false;

    assert(_piItemOperations != NULL);

    if (_piItemOperations != NULL)
    {
        // Open the file:
        VxDTE::Window* piOpenedWindow = NULL;
        BSTR filePathAsBSTR = SysAllocString(filePath.c_str());
        HRESULT hr = _piItemOperations->OpenFile(filePathAsBSTR, NULL, &piOpenedWindow);
        retVal = SUCCEEDED(hr);

        if (retVal && (piOpenedWindow != NULL))
        {
            // Add the file to the opened files list if needed
            addFileToOpenedList(filePath.c_str());

            // Get the document:
            VxDTE::Document* piDocument = NULL;
            hr = piOpenedWindow->get_Document(&piDocument);

            if (SUCCEEDED(hr) && (piDocument != NULL))
            {
                // Get the selection (its type is generic since we don't know what's the document type:
                IDispatch* piSelectionAsDispatch = NULL;
                hr = piDocument->get_Selection(&piSelectionAsDispatch);

                if (SUCCEEDED(hr) && (piSelectionAsDispatch != NULL))
                {
                    // See if this selection responds to the text selection interface:
                    VxDTE::TextSelection* piSelection = NULL;
                    hr = piSelectionAsDispatch->QueryInterface(__uuidof(VxDTE::TextSelection), (void**)(&piSelection));

                    if (SUCCEEDED(hr) && (piSelection != NULL))
                    {
                        if (lineNumber == 0)
                        {
                            // Goto line with line number 0 results with error:
                            lineNumber = 1;
                        }

                        // Get the current selected line:
                        // NOTICE: If the line is already selected, the file is for some reason opened at line 1,
                        // therefor, we should check if the line is selected, and select only if it's not selected:
                        long currentSelectedLine = 0;
                        piSelection->get_CurrentLine(&currentSelectedLine);

                        if (currentSelectedLine != lineNumber)
                        {
                            // Select the requested line:
                            hr = piSelection->GotoLine((long)lineNumber, selectLine ? (VARIANT_BOOL)TRUE : (VARIANT_BOOL)FALSE);
                            assert(SUCCEEDED(hr));
                        }

                        // Release the selection:
                        piSelection->Release();
                    }

                    // Release the dispatch interface:
                    piSelectionAsDispatch->Release();
                }

                // Release the document:
                piDocument->Release();
            }

            // Release the window:
            piOpenedWindow->Release();
        }
        else
        {
            VSCORE(vscPrintDebugMsgToDebugLog)(filePath.c_str());
        }

        // Free the file path string:
        SysFreeString(filePathAsBSTR);
    }
    else
    {
        VSCORE(vscPrintDebugMsgToDebugLog)(VSP_STR_FileOperationPointerNotInitialized);
    }


    return retVal;
}

bool vspDTEConnector::openURL(const std::wstring& url)
{
    bool retVal = false;

    assert(_piItemOperations != NULL);

    if (_piItemOperations != NULL)
    {
        // Open the file:
        VxDTE::Window* piOpenedWindow = NULL;
        BSTR filePathAsBSTR = SysAllocString(url.c_str());
        HRESULT hr = _piItemOperations->Navigate(filePathAsBSTR,  vsNavigateOptionsDefault, &piOpenedWindow);
        retVal = SUCCEEDED(hr);

        // Free the file path string:
        SysFreeString(filePathAsBSTR);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::getWindowFromFilePath
// Description: Gets the window interface for the given file path. We cannot use
//              OpenFile here, as the requested document might not exist anymore.
//              Instead, we iterate the windows and try to find the one we need.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        10/3/2011
// ---------------------------------------------------------------------------
bool vspDTEConnector::getWindowFromFilePath(const std::wstring& filePath, /* (VxDTE::Window*&) */ void*& piWindow)
{
    bool retVal = false;
    piWindow = NULL;

    assert(_piDTE != NULL);

    if (_piDTE != NULL)
    {
        // Get the windows collection:
        VxDTE::Windows* piWindows = NULL;
        HRESULT hr = _piDTE->get_Windows(&piWindows);

        if (SUCCEEDED(hr) && (piWindows != NULL))
        {
            // Get the amount of windows:
            long amountOfWindows = 0;
            hr = piWindows->get_Count(&amountOfWindows);

            if (SUCCEEDED(hr) && (amountOfWindows > 0))
            {
                // Iterate the windows:
                VARIANT iAsVAR = {0};
                iAsVAR.vt = VT_INT;

                for (long i = 0; i < amountOfWindows; i++)
                {
                    // DTE indices are 1-based:
                    iAsVAR.intVal = (INT)i + 1;

                    // Get the current window:
                    VxDTE::Window* piCurrentWindow = NULL;
                    hr = piWindows->Item(iAsVAR, &piCurrentWindow);

                    if (SUCCEEDED(hr) && (piCurrentWindow != NULL))
                    {
                        bool foundWindow = false;

                        // Get the document:
                        VxDTE::Document* piCurrentDocument = NULL;
                        hr = piCurrentWindow->get_Document(&piCurrentDocument);

                        if (SUCCEEDED(hr) && (piCurrentDocument != NULL))
                        {
                            // Get the document's full path:
                            BSTR currentDocumentFullPath = NULL;
                            hr = piCurrentDocument->get_FullName(&currentDocumentFullPath);

                            if (SUCCEEDED(hr) && (currentDocumentFullPath != NULL))
                            {
                                // Check if this is our document:
                                foundWindow = (filePath == currentDocumentFullPath);

                                // Free the string:
                                SysFreeString(currentDocumentFullPath);
                            }

                            // Release the document:
                            piCurrentDocument->Release();
                        }

                        if (foundWindow)
                        {
                            // This is the correct window, return it and stop looking:
                            piWindow = (void*)piCurrentWindow;
                            retVal = true;
                            break;
                        }
                        else // !correctWindow
                        {
                            // This is not the correct window, release it:
                            piCurrentWindow->Release();
                        }
                    }
                }
            }

            // Release the windows:
            piWindows->Release();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::closeDocumentsOfDeletedFiles
// Description: Pass through all the open windows and check if the path to the file
//              still exists, if not close the window. Should use this function after
//              deleting a node from the tree that points to files.
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        10/7/2013
// ---------------------------------------------------------------------------
bool vspDTEConnector::closeDocumentsOfDeletedFiles()
{
    bool retVal = false;

    assert(_piDTE != NULL);

    if (_piDTE != NULL)
    {
        // Get the windows collection:
        VxDTE::Windows* piWindows = NULL;
        HRESULT hr = _piDTE->get_Windows(&piWindows);

        if (SUCCEEDED(hr) && (piWindows != NULL))
        {
            // Get the amount of windows:
            long amountOfWindows = 0;
            hr = piWindows->get_Count(&amountOfWindows);

            if (SUCCEEDED(hr) && (amountOfWindows > 0))
            {
                // Iterate the windows:
                VARIANT iAsVAR = {0};
                iAsVAR.vt = VT_INT;

                for (long i = 0; i < amountOfWindows; i++)
                {
                    // DTE indices are 1-based:
                    iAsVAR.intVal = (INT)i + 1;

                    // Get the current window:
                    VxDTE::Window* piCurrentWindow = NULL;
                    hr = piWindows->Item(iAsVAR, &piCurrentWindow);

                    if (SUCCEEDED(hr) && (piCurrentWindow != NULL))
                    {
                        // Get the document:
                        VxDTE::Document* piCurrentDocument = NULL;
                        hr = piCurrentWindow->get_Document(&piCurrentDocument);

                        if (SUCCEEDED(hr) && (piCurrentDocument != NULL))
                        {
                            // Get the document's full path:
                            BSTR currentDocumentFullPath = NULL;
                            hr = piCurrentDocument->get_FullName(&currentDocumentFullPath);

                            if (SUCCEEDED(hr) && (currentDocumentFullPath != NULL))
                            {
                                if (!VSCORE(vscIsPathExists)(currentDocumentFullPath))
                                {
                                    void* windowToClose = (void*)piCurrentWindow;
                                    closeAndReleaseWindow(windowToClose);
                                }

                                // Free the string:
                                SysFreeString(currentDocumentFullPath);
                            }

                            // Release the document:
                            piCurrentDocument->Release();
                        }
                    }
                }
            }

            // Release the windows:
            piWindows->Release();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::closeAndReleaseWindow
// Description: Closes the window and releases the interface
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        10/3/2011
// ---------------------------------------------------------------------------
bool vspDTEConnector::closeAndReleaseWindow(void*& piWindow, bool close)
{
    bool retVal = false;

    // Cast the pointer:
    VxDTE::Window* piWindowAsWindow = (VxDTE::Window*)piWindow;

    assert(piWindowAsWindow != NULL);

    if (piWindowAsWindow != NULL)
    {
        retVal = true;

        // Close the window:
        if (close)
        {
            HRESULT hrC = piWindowAsWindow->Close();
            retVal = retVal && SUCCEEDED(hrC);
        }

        // Release the interface:
        HRESULT hrR = piWindowAsWindow->Release();
        retVal = retVal && SUCCEEDED(hrR);
    }

    // Reset the pointer:
    piWindow = NULL;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::getDebugInformationFromProject
// Description: Extracts the details needed to debug a project from the object representing it.
// Author:      Uri Shomroni
// Date:        6/10/2010
// ---------------------------------------------------------------------------
void vspDTEConnector::getDebugInformationFromProject(VxDTE::Project& project, std::wstring& executableFilePath, std::wstring& workingDirectoryPath, std::wstring& commandLineArguments, std::wstring& environment, bool& isProjectTypeValid, bool& isNonNativeProject)
{
    executableFilePath.clear();
    workingDirectoryPath.clear();
    commandLineArguments.clear();
    environment.clear();
    isProjectTypeValid = false;
    isNonNativeProject = false;

    // Get the project's configuration manager:
    VxDTE::ConfigurationManager* pConfigManager = NULL;
    HRESULT hr = project.get_ConfigurationManager(&pConfigManager);

    if (SUCCEEDED(hr) && (pConfigManager != NULL))
    {
        // Get the active configuration:
        VxDTE::Configuration* pActiveConfiguration = NULL;
        hr = pConfigManager->get_ActiveConfiguration(&pActiveConfiguration);

        if (SUCCEEDED(hr) && (pActiveConfiguration != NULL))
        {
            // Get the configuration's properties object:
            VxDTE::Properties* pActiveConfigurationProperties = NULL;
            hr = pActiveConfiguration->get_Properties(&pActiveConfigurationProperties);

            if (SUCCEEDED(hr) && pActiveConfigurationProperties != NULL)
            {
                // Uri 21/7/13 - Leave this #if-ed out, used for debugging purposes, to see which properties can be gotten from
                // new / unknown project types:
#if 0
                {
                    std::wstring props;

                    VARIANT iVar = {0};
                    iVar.vt = VT_UINT;
                    iVar.uintVal = (UINT)1;
                    bool goOn = true;

                    while (goOn)
                    {
                        VxDTE::Property* pProperty = NULL;
                        HRESULT hrProp = pActiveConfigurationProperties->Item(iVar, &pProperty);
                        goOn = SUCCEEDED(hrProp);

                        if (goOn)
                        {
                            BSTR propertyName = NULL;
                            hrProp = pProperty->get_Name(&propertyName);

                            if (SUCCEEDED(hrProp))
                            {
                                VARIANT propertyValue = {0};
                                hrProp = pProperty->get_Value(&propertyValue);

                                if (SUCCEEDED(hrProp))
                                {
                                    std::wstring propNmGTString = propertyName;
                                    std::wstring propValGTString;

                                    switch (propertyValue.vt)
                                    {
                                        case VT_BSTR:
                                            propValGTString = propertyValue.bstrVal;
                                            break;

                                        case VT_BOOL:
                                            propValGTString = (VARIANT_FALSE != propertyValue.boolVal) ? L"TRUE" : L"FALSE";
                                            break;

                                        default:
                                        {
                                            // Most other values are numbers:
                                            int propValInt = -1;
                                            bool propValIsInt = false;
                                            getIntegerPropertyValue(*pActiveConfigurationProperties, propNmGTString, propValInt, propValIsInt);

                                            if (propValIsInt)
                                            {
                                                propValGTString.appendFormattedString(L"%d", propValInt);
                                            }
                                            else
                                            {
                                                propValGTString.appendFormattedString(L"Error (VT = %d)", propertyValue.vt);
                                            }
                                        }
                                        break;
                                    }

                                    VariantClear(&propertyValue);

                                    props.append(propNmGTString).append('=').append(propValGTString).append('\n');
                                }

                                SysFreeString(propertyName);
                            }

                            iVar.uintVal++;
                        }
                    }
                }
#endif

                // Ask for each property by its name:
                static const std::wstring commandPropertyName            = L"Command";
                static const std::wstring workingDirectoryPropertyName   = L"WorkingDirectory";
                static const std::wstring commandArgumentsPropertyName   = L"CommandArguments";
                static const std::wstring environmentPropertyName        = L"LocalDebuggerEnvironment";
                bool isCommandPresent = true;
                bool isWorkDirPresent = true;
                bool isCmdArgsPresent = true;
                bool isEnvironPresent = true;
                getStringPropertyValue(*pActiveConfigurationProperties, commandPropertyName, executableFilePath, isCommandPresent);
                getStringPropertyValue(*pActiveConfigurationProperties, workingDirectoryPropertyName, workingDirectoryPath, isWorkDirPresent);
                getStringPropertyValue(*pActiveConfigurationProperties, commandArgumentsPropertyName, commandLineArguments, isCmdArgsPresent);
                getStringPropertyValue(*pActiveConfigurationProperties, environmentPropertyName, environment, isEnvironPresent);

                // A Visual C project will have all these properties:
                isProjectTypeValid = isCommandPresent && isWorkDirPresent && isCmdArgsPresent;
                bool isVCProject = isProjectTypeValid;

                if (isVCProject)
                {
                    // This macro evaluates to the inherited "environment" value filed - which is what is filled in if the environment is not present.
                    if (environment.empty())
                    {
                        environment = L"$(LocalDebuggerEnvironment)";
                    }

                    if (executableFilePath.empty())
                    {
                        executableFilePath = L"$(TargetPath)";
                    }
                }
                else
                {
                    // If this doesn't have the VCProject values, try getting the values from a C# / VB.NET project:
                    static const std::wstring startActionPropertyName = L"StartAction";
                    static const std::wstring startProgramPropertyName = L"StartProgram";
                    static const std::wstring startWorkingDirectoryPropertyName = L"StartWorkingDirectory";
                    static const std::wstring startArgumentsPropertyName = L"StartArguments";
                    int startAction = -1;
                    bool isStartActionPresent = false;
                    getIntegerPropertyValue(*pActiveConfigurationProperties, startActionPropertyName, startAction, isStartActionPresent);

                    if (isStartActionPresent && (prjStartActionProgram == startAction))
                    {
                        getStringPropertyValue(*pActiveConfigurationProperties, startProgramPropertyName, executableFilePath, isCommandPresent);
                    }

                    getStringPropertyValue(*pActiveConfigurationProperties, startWorkingDirectoryPropertyName, workingDirectoryPath, isWorkDirPresent);
                    getStringPropertyValue(*pActiveConfigurationProperties, startArgumentsPropertyName, commandLineArguments, isCmdArgsPresent);

                    isProjectTypeValid = isStartActionPresent &&
                                         ((prjStartActionProgram != startAction) || (isCommandPresent && (!executableFilePath.empty()))) &&
                                         isWorkDirPresent && isCmdArgsPresent;

                    if (isProjectTypeValid)
                    {
                        // The project is valid, but not native:
                        isNonNativeProject = true;
                    }
                }

                // Release the properties:
                pActiveConfigurationProperties->Release();

                if (isVCProject)
                {
                    // Try to evaluate any macros such as $(TargetPath), etc.:
                    evaluateUserMacros(project, *pActiveConfiguration, false, executableFilePath, workingDirectoryPath, commandLineArguments, environment);
                }

                // The working directory and executable path might be relative paths, try to compensate for this:
                if (!executableFilePath.empty())
                {
                    verifyNonRelativePath(project, executableFilePath);
                }

                if (!workingDirectoryPath.empty())
                {
                    // If the working directory path is empty, it's not a relative path:
                    verifyNonRelativePath(project, workingDirectoryPath);
                }
            }

            // Get the target path as a fallback for the executable path (this is what VS does if the debugging executable is not defined):
            // Uri, 7/2/12 - this fallback seems to make crashes when trying to debug an "exe project" which is a normal exe and not a project (case 8007).
            // Until we can test this on the crash configuration (32-bit), we'll just leave this commented out
            if (executableFilePath.empty())
            {
                getConfigurationTargetPath(*pActiveConfiguration, executableFilePath);

                // A Visual C project will have all these properties:
                isProjectTypeValid = !executableFilePath.empty();
            }

            // Get the executable's directory as a fallback for the working directory (this is what visual studio does if the debugging work dir is not selected):
            if (workingDirectoryPath.empty())
            {
                wchar_t* pBuffer = NULL;
                VSCORE(vscGetFileDirectoryAsString)(executableFilePath.c_str(), pBuffer);
                assert(pBuffer != NULL);

                if (pBuffer != NULL)
                {
                    workingDirectoryPath = pBuffer;

                    // Release the allocated string
                    VSCORE(vscDeleteWcharString)(pBuffer);
                }
            }

            // Release the configuration:
            pActiveConfiguration->Release();
        }

        // Release the configuration manager:
        pConfigManager->Release();
    }
}
// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::getIntegerPropertyValue
// Description: Gets a named value from a property collection.
// Author:      Uri Shomroni
// Date:        18/6/2013
// ---------------------------------------------------------------------------
void vspDTEConnector::getIntegerPropertyValue(VxDTE::Properties& properties, const std::wstring& propertyName, int& propertyValue, bool& isPropertyPresent)
{
    // Clear the output parameter:
    propertyValue = -1;

    // Create a variant that will hold the property name:
    VARIANT propertyNameAsVar = {0};
    propertyNameAsVar.vt = VT_BSTR;
    propertyNameAsVar.bstrVal = SysAllocString(propertyName.c_str());

    // Get the property by its name:
    VxDTE::Property* pProperty = NULL;
    HRESULT hr = properties.Item(propertyNameAsVar, &pProperty);
    isPropertyPresent = SUCCEEDED(hr);

    if (SUCCEEDED(hr) && (pProperty != NULL))
    {
        // Get the property value:
        VARIANT propertyValueAsVar = {0};
        hr = pProperty->get_Value(&propertyValueAsVar);

        if (SUCCEEDED(hr))
        {
            // According to the property type, fill its value into the output parameter:
            switch (propertyValueAsVar.vt)
            {
                case VT_I1:
                {
                    propertyValue = (int)propertyValueAsVar.cVal;
                }
                break;

                case VT_I2:
                {
                    propertyValue = (int)propertyValueAsVar.iVal;
                }
                break;

                case VT_I4:
                {
                    propertyValue = (int)propertyValueAsVar.lVal;
                }
                break;

                case VT_INT:
                {
                    propertyValue = (int)propertyValueAsVar.intVal;
                }
                break;

                case VT_UI1:
                {
                    propertyValue = (int)propertyValueAsVar.bVal;
                }
                break;

                case VT_UI2:
                {
                    propertyValue = (int)propertyValueAsVar.uiVal;
                }
                break;

                case VT_UI4:
                {
                    propertyValue = (int)propertyValueAsVar.ulVal;
                }
                break;

                case VT_UINT:
                {
                    propertyValue = (int)propertyValueAsVar.uintVal;
                }
                break;

                // TO_DO: support other types as needed.

                default:
                {
                    // Unsupported type:
                    assert(false);
                }
                break;
            }
        }

        // Clear the value variant:
        VariantClear(&propertyValueAsVar);

        // Release the property:
        pProperty->Release();
    }

    // Clear the name variant:
    VariantClear(&propertyNameAsVar);
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::getStringPropertyValue
// Description: Gets a named value from a property collection.
// Author:      Uri Shomroni
// Date:        6/10/2010
// ---------------------------------------------------------------------------
void vspDTEConnector::getStringPropertyValue(VxDTE::Properties& properties, const std::wstring& propertyName, std::wstring& propertyValue, bool& isPropertyPresent)
{
    propertyValue = L"";

    // Create a variant that will hold the property name:
    VARIANT propertyNameAsVar = {0};
    propertyNameAsVar.vt = VT_BSTR;
    propertyNameAsVar.bstrVal = SysAllocString(propertyName.c_str());

    // Get the property by its name:
    VxDTE::Property* pProperty = NULL;
    HRESULT hr = properties.Item(propertyNameAsVar, &pProperty);
    isPropertyPresent = SUCCEEDED(hr);

    if (SUCCEEDED(hr) && (pProperty != NULL))
    {
        // Get the property value:
        VARIANT propertyValueAsVar = {0};
        hr = pProperty->get_Value(&propertyValueAsVar);

        if (SUCCEEDED(hr))
        {
            // According to the property type, fill its value into the output parameter:
            switch (propertyValueAsVar.vt)
            {
                case VT_BSTR:
                {
                    // This is a string, just copy it:
                    BSTR propertyValueAsBSTR = propertyValueAsVar.bstrVal;

                    if (propertyValueAsBSTR != NULL)
                    {
                        propertyValue = propertyValueAsBSTR;
                    }
                }
                break;

                // TO_DO: support other types as needed.

                default:
                {
                    // Unsupported type:
                    assert(false);
                }
                break;
            }
        }

        // Clear the value variant:
        VariantClear(&propertyValueAsVar);

        // Release the property:
        pProperty->Release();
    }

    // Clear the name variant:
    VariantClear(&propertyNameAsVar);
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::evaluateUserMacros
// Description: If the project is a Visual C project, uses the VCProjectEngine
//              namespace's VCConfiguration::Eval function to evalutate the user
//              macros (e.g. $(TargetPath) ) in the parameters.
//              onlyPath - evaluate all parameters, or only the first one?
// Author:      Uri Shomroni
// Date:        7/10/2010
// ---------------------------------------------------------------------------
void vspDTEConnector::evaluateUserMacros(VxDTE::Project& project, VxDTE::Configuration& activeConfiguration, bool onlyPath, std::wstring& executableFilePath, std::wstring& workingDirectoryPath, std::wstring& commandLineArguments, std::wstring& environment)
{
    // This is a translation to C++ of the C# code here: http://bitbucket.org/sweinst/windows/src/89222c99e96e/SVTools/SVTools/DebugWizard/DebugWizardForm.cs
    // Note that attempting to get the VCProject interface directly from the Project interface fails.
    // So Does getting the VCConfiguration directly from the Configuration interface (or through its Dispatch).
    // As the C# code illustrates, we must go up to the project object and ask for its configuration by name.

    // Get the project "object", i.e. its dispatch interface:
    IDispatch* pProjectAsDispatch = NULL;
    HRESULT hr = project.get_Object(&pProjectAsDispatch);

    if (SUCCEEDED(hr) && (pProjectAsDispatch != NULL))
    {
        // Get the project's "Visual C Project" interface. If this fails, the project is not a VC/VC++ project,
        // so we don't support it (and it doesn't support user macros):
        VCProjectEngineLibrary::VCProject* pProjectAsVCProject = NULL;
        hr = pProjectAsDispatch->QueryInterface(VCProjectEngineLibrary::IID_VCProject, (void**)(&pProjectAsVCProject));

        if (SUCCEEDED(hr) && (pProjectAsVCProject != NULL))
        {
            // Get the configurations object:
            IDispatch* pVCConfigurationsAsDispatch = NULL;
            hr = pProjectAsVCProject->get_Configurations(&pVCConfigurationsAsDispatch);

            if (SUCCEEDED(hr) && (pVCConfigurationsAsDispatch != NULL))
            {
                // Get the configurations object's (configuration) collection interface
                VCProjectEngineLibrary::IVCCollection* pVCConfigurations = NULL;
                hr = pVCConfigurationsAsDispatch->QueryInterface(VCProjectEngineLibrary::IID_IVCCollection, (void**)(&pVCConfigurations));

                if (SUCCEEDED(hr) && (pVCConfigurations != NULL))
                {
                    // Get the active configuration's full name:
                    std::wstring configurationFullName;
                    bool rcConfName = getConfigurationFullName(activeConfiguration, configurationFullName);

                    if (rcConfName && (!configurationFullName.empty()))
                    {
                        // Create a variant with the active configuration's name:
                        VARIANT activeConfNameAsVar = {0};
                        activeConfNameAsVar.vt = VT_BSTR;
                        activeConfNameAsVar.bstrVal = SysAllocString(configurationFullName.c_str());

                        // Use the variant to get the active configuration from the Visual C configurations collection:
                        IDispatch* pActiveConfigurationAsDispatch = NULL;
                        hr = pVCConfigurations->Item(activeConfNameAsVar, &pActiveConfigurationAsDispatch);

                        if (SUCCEEDED(hr) && (pActiveConfigurationAsDispatch != NULL))
                        {
                            // Get the configuration's Visual C configuration interface:
                            VCProjectEngineLibrary::VCConfiguration* pActiveConfigurationAsVCConfig = NULL;
                            hr = pActiveConfigurationAsDispatch->QueryInterface(VCProjectEngineLibrary::IID_VCConfiguration, (void**)(&pActiveConfigurationAsVCConfig));

                            if (SUCCEEDED(hr) && (pActiveConfigurationAsVCConfig != NULL))
                            {
                                // Evaluate each parameter:
                                evaluateUserMacrosInString((void*)pActiveConfigurationAsVCConfig, executableFilePath);

                                if (!onlyPath)
                                {
                                    evaluateUserMacrosInString((void*)pActiveConfigurationAsVCConfig, workingDirectoryPath);
                                    evaluateUserMacrosInString((void*)pActiveConfigurationAsVCConfig, commandLineArguments);
                                    evaluateUserMacrosInString((void*)pActiveConfigurationAsVCConfig, environment);
                                }

                                // Release the VCConfiguration interface:
                                pActiveConfigurationAsVCConfig->Release();
                            }

                            // Release the VC configuration's dispatch interface:
                            pActiveConfigurationAsDispatch->Release();
                        }

                        // Clear the name variant:
                        SysFreeString(activeConfNameAsVar.bstrVal);
                        activeConfNameAsVar.bstrVal = NULL;
                        VariantClear(&activeConfNameAsVar);
                    }

                    // Release the configurations collection's IVCCollection interface:
                    pVCConfigurations->Release();
                }

                // Release the configurations collection's dispatch interface:
                pVCConfigurationsAsDispatch->Release();
            }

            // Release the project's VCProject interface:
            pProjectAsVCProject->Release();
        }

        // Release the project object:
        pProjectAsDispatch->Release();
    }
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::evaluateUserMacrosInString
// Description: Uses pActiveConfigurationAsVoidPointer (which is actually of type VCProjectEngineLibrary::VCConfiguration*)
//              to evaluate user macros in evalString.
// Author:      Uri Shomroni
// Date:        7/10/2010
// ---------------------------------------------------------------------------
void vspDTEConnector::evaluateUserMacrosInString(/* (VCProjectEngineLibrary::VCConfiguration*) */ void* pActiveConfigurationAsVoidPointer, std::wstring& evalString)
{
    // Cast the pointer to its real type:
    VCProjectEngineLibrary::VCConfiguration* pActiveConfigurationAsVCConfig = (VCProjectEngineLibrary::VCConfiguration*)pActiveConfigurationAsVoidPointer;

    // Ask the Visual C configuration object to evaluate the string:
    BSTR evalStringEval = NULL;
    BSTR evalStringUnEval = SysAllocString(evalString.c_str());
    HRESULT hr = pActiveConfigurationAsVCConfig->Evaluate(evalStringUnEval, &evalStringEval);

    if (SUCCEEDED(hr) && (evalStringEval != NULL))
    {
        // Copy the value:
        evalString = evalStringEval;

        // Release the output string:
        SysFreeString(evalStringEval);
    }

    // Release the input string:
    SysFreeString(evalStringUnEval);
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::verifyNonRelativePath
// Description: Attempts to make sure projectPath is non-relative: If it does not
//              exist as it is, try perpending the project directory to it.
// Author:      Uri Shomroni
// Date:        7/2/2011
// ---------------------------------------------------------------------------
void vspDTEConnector::verifyNonRelativePath(VxDTE::Project& project, std::wstring& projectPath)
{
    if (!VSCORE(vscIsPathExists)(projectPath.c_str()))
    {
        // Get the project file's path:
        BSTR projectFileFullPathAsBSTR = NULL;
        HRESULT hr = project.get_FileName(&projectFileFullPathAsBSTR);

        if (SUCCEEDED(hr) && (projectFileFullPathAsBSTR != NULL))
        {
            // Construct the contextualized path from the project file directory and the relative path:
            std::wstring projectFileFullPathAsString(projectFileFullPathAsBSTR);

            wchar_t* dirAsStr = NULL;
            VSCORE(vscGetFileDirectoryAsString)(projectFileFullPathAsBSTR, dirAsStr);

            std::wstringstream stream;
            stream << VSCORE(vscGetOsPathSeparator)() << dirAsStr;
            std::wstring contextualizedProjectPathAsString(stream.str());

            // If it exists:
            if (VSCORE(vscIsPathExists)(contextualizedProjectPathAsString.c_str()))
            {
                // Use it instead of the original:
                projectPath = contextualizedProjectPathAsString;
            }

            // Release the string:
            SysFreeString(projectFileFullPathAsBSTR);

            // Release the string:
            VSCORE(vscDeleteWcharString)(dirAsStr);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::getConfigurationTargetPath
// Description: Gets the build tool's output path from configuration
// Author:      Uri Shomroni
// Date:        7/10/2010
// ---------------------------------------------------------------------------
void vspDTEConnector::getConfigurationTargetPath(VxDTE::Configuration& configuration, std::wstring& targetPath)
{
    // Get the configuration's output groups:
    VxDTE::OutputGroups* pConfigurationOutputGroups = NULL;
    HRESULT hr = configuration.get_OutputGroups(&pConfigurationOutputGroups);

    if (SUCCEEDED(hr) && (pConfigurationOutputGroups != NULL))
    {
        // Get the number of groups:
        long numberOfOutputGroups = 0;
        hr = pConfigurationOutputGroups->get_Count(&numberOfOutputGroups);

        if (SUCCEEDED(hr) && (numberOfOutputGroups > 0))
        {
            // Create the number 1 as a variant:
            VARIANT primaryOutputStr = {0};
            primaryOutputStr.vt = VT_BSTR;
            primaryOutputStr.bstrVal = SysAllocString(L"Primary Output");

            // Get the first output group:
            VxDTE::OutputGroup* pFirstOutputGroup = NULL;
            hr = pConfigurationOutputGroups->Item(primaryOutputStr, &pFirstOutputGroup);

            if (SUCCEEDED(hr) && (pFirstOutputGroup != NULL))
            {
                // Get its output file URLs (we expect this to be an array of variants):
                VARIANT outputFileURLs = {0};
                hr = pFirstOutputGroup->get_FileURLs(&outputFileURLs);

                if (SUCCEEDED(hr) && (outputFileURLs.vt == (VT_ARRAY | VT_VARIANT)))
                {
                    // Get the URLs as an array:
                    SAFEARRAY* pURLs = outputFileURLs.parray;

                    if ((pURLs != NULL) && (pURLs->cDims > 0))
                    {
                        // Get the first URL as a variant (we expect it to be a string):
                        VARIANT* firstURLAsVar = (VARIANT*)(pURLs->pvData);

                        if ((firstURLAsVar != NULL) && (firstURLAsVar->vt == VT_BSTR))
                        {
                            // Get the string value:
                            BSTR firstURL = firstURLAsVar->bstrVal;

                            if (firstURL != NULL)
                            {
                                // Return it, after cleaning any "file://" prefixes:
                                targetPath = firstURL;
                                cleanURLString(targetPath);
                            }

                            // Do not release the string, the main VariantClear function should do this for us:
                            // SysFreeString(firstURL);
                        }

                        // Don't clear the variant, the main VariantClear function should do this for us:
                        // VariantClear(firstURLAsVar);
                    }

                    // Don't destroy the array, the VariantClear function should do this for us:
                    // SafeArrayDestroy(pURLs);
                }

                // Clear the variant:
                VariantClear(&outputFileURLs);

                // Release the output group:
                pFirstOutputGroup->Release();
            }
        }

        // Release the output groups object:
        pConfigurationOutputGroups->Release();
    }
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::getConfigurationFullName
// Description: Gets the full name of the configuration, composed of the configuration
//              and platform, e.g. "Debug|Win32"
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        23/3/2011
// ---------------------------------------------------------------------------
bool vspDTEConnector::getConfigurationFullName(VxDTE::Configuration& configuration, std::wstring& configrationFullName)
{
    bool retVal = false;

    // Get the configuration name:
    BSTR configurationNameBSTR = NULL;
    HRESULT hr = configuration.get_ConfigurationName(&configurationNameBSTR);

    if (SUCCEEDED(hr) && (configurationNameBSTR != NULL))
    {
        retVal = true;

        // Set it as the return string:
        configrationFullName = configurationNameBSTR;

        // Try and get the platform:
        BSTR platformNameBSTR = NULL;
        hr = configuration.get_PlatformName(&platformNameBSTR);

        if (SUCCEEDED(hr) && (platformNameBSTR != NULL))
        {
            // Add that to the output string:
            configrationFullName.append(L"|").append(platformNameBSTR);

            // Release the string:
            SysFreeString(platformNameBSTR);
        }

        // Release the string:
        SysFreeString(configurationNameBSTR);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::cleanURLString
// Description: Clears urlString from any "file://" prefixes
// Author:      Uri Shomroni
// Date:        6/10/2010
// ---------------------------------------------------------------------------
void vspDTEConnector::cleanURLString(std::wstring& urlString)
{
    // If this string has the prefix:
    static const std::wstring fileURLStart = L"file:/";

    if (!urlString.compare(0, fileURLStart.size(), fileURLStart))
    {
        // Remove the prefix:
        urlString = urlString.substr(fileURLStart.size());

        // This prefix can have an arbitrary number of slashes, remove them all:
        static const wchar_t slash = L'/';

        while (!urlString.empty() && (urlString.at(0) == slash))
        {
            urlString = urlString.substr(1);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::getProjectByName
// Description: Tries to get a project named from inside the solution hierarchy.
//              The returned project needs to be released by the caller.
// Author:      Uri Shomroni
// Date:        9/2/2011
// ---------------------------------------------------------------------------
VxDTE::Project* vspDTEConnector::getProjectByName(VxDTE::_Solution& solution, const std::wstring& projectName)
{
    VxDTE::Project* retVal = NULL;
    VARIANT iAsVar = {0};
    iAsVar.vt = VT_INT;
    int projectNameLength = projectName.length();

    // Will fill with projects to check:
    std::queue<VxDTE::Project*> projectsToCheck;

    // Initialized the queue with the solution projects:
    VxDTE::Projects* pSolutionProjects = NULL;
    HRESULT hr = solution.get_Projects(&pSolutionProjects);

    if (SUCCEEDED(hr) && (pSolutionProjects != NULL))
    {
        // Get the amount of items:
        long numberOfProjects = 0;
        hr = pSolutionProjects->get_Count(&numberOfProjects);

        if (SUCCEEDED(hr))
        {
            // Get each item and add it to the queue:
            for (long i = 0; i < numberOfProjects; i++)
            {
                // Get the current item:
                iAsVar.intVal = (INT)i + 1; // DTE indices are 1-based.
                VxDTE::Project* pCurrentProject = NULL;
                hr = pSolutionProjects->Item(iAsVar, &pCurrentProject);

                if (SUCCEEDED(hr) && (pCurrentProject != NULL))
                {
                    projectsToCheck.push(pCurrentProject);
                }
            }
        }

        // Release the projects interface:
        pSolutionProjects->Release();
    }

    // Get the constant for later comparison:
    static std::wstring solutionFolderProjectKind;

    if (solutionFolderProjectKind.empty())
    {
        // Convert to wide characters,
        std::wstringstream converter;
        converter << VxDTE::vsProjectKindSolutionItems;
        solutionFolderProjectKind = std::wstring(converter.str());
    }

    static std::wstring solutionFolderProjectItemsKind;

    if (solutionFolderProjectItemsKind.empty())
    {
        // Convert to wide characters,
        std::wstringstream converter;
        converter << VxDTE::vsProjectItemsKindSolutionItems;
        solutionFolderProjectItemsKind = std::wstring(converter.str());
    }

    // Keep going until we checked all the hierarchy:
    while (!projectsToCheck.empty())
    {
        // Get the first project and remove it from the queue:
        VxDTE::Project* pCurrentProject = projectsToCheck.front();
        projectsToCheck.pop();

        // Sanity check:
        assert(pCurrentProject != NULL);

        if (pCurrentProject != NULL)
        {
            // Once we found the project we were looking for, we can just empty the rest of the queue:
            if (retVal == NULL)
            {
                // Check if it is a non-folder project:
                BSTR currentProjectKind = NULL;
                hr = pCurrentProject->get_Kind(&currentProjectKind);

                if (SUCCEEDED(hr) && (currentProjectKind != NULL))
                {
                    // If this is not a solution folder, check if it is our project:
                    if ((solutionFolderProjectKind != currentProjectKind) && (solutionFolderProjectItemsKind != currentProjectKind))
                    {
                        // Get the name:
                        BSTR currentProjectName = NULL;
                        hr = pCurrentProject->get_FullName(&currentProjectName);

                        if (SUCCEEDED(hr) && (currentProjectName != NULL))
                        {
                            // If the project path ends with the given "name" (which is the relative path):
                            std::wstring currentProjectFullPathAsString = currentProjectName;
                            int fullPathLength = currentProjectFullPathAsString.length();

                            if (fullPathLength >= projectNameLength)
                            {
                                currentProjectFullPathAsString =
                                    currentProjectFullPathAsString.substr(fullPathLength - projectNameLength, fullPathLength - 1);

                                if (projectName == currentProjectFullPathAsString)
                                {
                                    // This is the project we were looking for:
                                    retVal = pCurrentProject;
                                }
                            }

                            // Release the name string:
                            SysFreeString(currentProjectName);
                        }
                    }

                    // Release the kind string:
                    SysFreeString(currentProjectKind);
                }

                // Add all this project or folder's sub-projects to the queue as well:
                VxDTE::ProjectItems* pCurrentProjectItems = NULL;
                hr = pCurrentProject->get_ProjectItems(&pCurrentProjectItems);

                if (SUCCEEDED(hr) && (pCurrentProjectItems != NULL))
                {
                    // Iterate the project items:
                    long numberOfProjectItems = 0;
                    hr = pCurrentProjectItems->get_Count(&numberOfProjectItems);

                    if (SUCCEEDED(hr))
                    {
                        for (long i = 0; i < numberOfProjectItems; i++)
                        {
                            iAsVar.intVal = (INT)i + 1; // DTE indices are 1-based.

                            VxDTE::ProjectItem* pCurrentProjectItem = NULL;
                            hr = pCurrentProjectItems->Item(iAsVar, &pCurrentProjectItem);

                            if (SUCCEEDED(hr) && pCurrentProjectItem != NULL)
                            {
                                // If a project item is a sub-project, add it to our queue:
                                VxDTE::Project* pCurrentProjectItemAsSubProject = NULL;
                                hr = pCurrentProjectItem->get_SubProject(&pCurrentProjectItemAsSubProject);

                                if (SUCCEEDED(hr) && (pCurrentProjectItemAsSubProject != NULL))
                                {
                                    projectsToCheck.push(pCurrentProjectItemAsSubProject);
                                }

                                // Release the project item interface:
                                pCurrentProjectItem->Release();
                            }
                        }
                    }

                    // Release the project items interface:
                    pCurrentProjectItems->Release();
                }
            }

            // Except for the returned project, release all others:
            if (retVal != pCurrentProject)
            {
                pCurrentProject->Release();
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::getSelectedWindowText
// Description: Return the current window selected text
// TO_DO: Sigal Remove from this class to vspVSUtils or such
// Arguments:   std::wstring& selectedText
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        13/4/2011
// ---------------------------------------------------------------------------
bool vspDTEConnector::getSelectedWindowText(std::wstring& selectedText)
{
    bool retVal = false;
    // Get the interface of the output pane:
    CodeXLVSPackage* pPackage = vspPackageWrapper::instance().getPackage();
    assert(pPackage != NULL);

    if (pPackage != NULL)
    {
        IVsTrackSelectionEx* spTrackSelection = NULL;
        HRESULT hr = pPackage->GetVsSiteCache().QueryService(SID_SVsTrackSelectionEx, &spTrackSelection);

        if (spTrackSelection != NULL)
        {
            IVsHierarchy* spHierarchy = NULL;
            hr = spTrackSelection->GetCurrentSelection(&spHierarchy, 0, NULL, NULL);
            assert((SUCCEEDED(hr)) && (spHierarchy != NULL));

            if ((SUCCEEDED(hr)) && (spHierarchy != NULL))
            {
                // Test
                BSTR bstrName = NULL;
                hr = spHierarchy->GetCanonicalName(0, &bstrName);

                if (SUCCEEDED(hr) && (NULL != bstrName))
                {
                    selectedText = bstrName;
                    SysFreeString(bstrName);
                    retVal = true;
                }

                spHierarchy->Release();
            }

            spTrackSelection->Release();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::getBuildOptions
// Description: Get the option flags: Out of date action, Before building (RunOrPreviewAction) action,
//              Run on Error action.
// Author:      Gilad Yarnitzky
// Date:        11/4/2011
// ---------------------------------------------------------------------------
void vspDTEConnector::getBuildOptions(int& onRunWhenOutOfDateAction, int& onRunOrPreviewAction, int& onRunWhenErrorsAction)
{
    assert(_piDTE != NULL);

    if (_piDTE != NULL)
    {
        VxDTE::Properties* piProperties = NULL;
        BSTR catName1 = SysAllocString(L"Environment");
        BSTR pageName1 = SysAllocString(L"ProjectsAndSolution");
        HRESULT hr = _piDTE->get_Properties(catName1, pageName1, &piProperties);

        if (SUCCEEDED(hr) && (piProperties != NULL))
        {
            long nProperties = 0;
            hr = piProperties->get_Count(&nProperties);

            if (SUCCEEDED(hr))
            {
                VARIANT buildVar = {0};
                buildVar.vt = VT_INT;

                for (int nCount = 1 ; nCount <= nProperties; nCount++)
                {
                    buildVar.intVal = nCount;
                    VxDTE::Property* piCurrentProperty = NULL;
                    hr = piProperties->Item(buildVar, &piCurrentProperty);

                    if (SUCCEEDED(hr) && (piCurrentProperty != NULL))
                    {
                        BSTR propertyName = NULL;
                        hr = piCurrentProperty->get_Name(&propertyName);

                        if (SUCCEEDED(hr) && (propertyName != NULL))
                        {
                            if (_tcsicmp(propertyName, L"OnRunWhenOutOfDate") == 0)
                            {

                                VARIANT onRunWhenOutOfDateVar = {0};
                                // Store the out of date option value:
                                piCurrentProperty->get_Value(&onRunWhenOutOfDateVar);
                                onRunWhenOutOfDateAction = onRunWhenOutOfDateVar.intVal;
                            }
                            else if (_tcsicmp(propertyName, L"OnRunOrPreview") == 0)
                            {
                                VARIANT onRunOrPreviewVar = {0};
                                // Store the out of date option value:
                                piCurrentProperty->get_Value(&onRunOrPreviewVar);
                                onRunOrPreviewAction = onRunOrPreviewVar.intVal;
                            }
                            else if (_tcsicmp(propertyName, L"OnRunWhenErrors") == 0)
                            {
                                VARIANT onRunWhenErrorsVar = {0};
                                // Store the out of date option value:
                                piCurrentProperty->get_Value(&onRunWhenErrorsVar);
                                onRunWhenErrorsAction = onRunWhenErrorsVar.intVal;
                            }

                            SysFreeString(propertyName);
                        }

                        // Release current property:
                        piCurrentProperty->Release();
                    }
                }
            }

            // Release the properties
            piProperties->Release();
        }

        SysFreeString(catName1);
        SysFreeString(pageName1);
    }
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::buildSaveTree
// Description: Get the option flags: Build the changed files tree
// Author:      Gilad Yarnitzky
// Date:        16/4/2011
// ---------------------------------------------------------------------------
void vspDTEConnector::buildSaveTree(IDteTreeEventHandler* pHandler)
{
    assert(_piDTE != NULL);
    assert(pHandler != NULL);

    if ((_piDTE != NULL) && (pHandler != NULL))
    {
        VxDTE::_Solution* piSolution = NULL;
        HRESULT hr = _piDTE->get_Solution((VxDTE::Solution**)&piSolution);

        if (SUCCEEDED(hr) && (piSolution != NULL))
        {
            // Add the solution to the tree as the root:
            BSTR solutionFileName = NULL;
            hr = piSolution->get_FileName(&solutionFileName);

            if (SUCCEEDED(hr) && (solutionFileName != NULL))
            {
                std::wstring solutionName(solutionFileName);
                // Get only the name from the full name:
                int nameStart = solutionName.find_last_of(L"\\");

                if (nameStart != -1)
                {
                    solutionName = solutionName.substr(nameStart + 1, solutionName.length() - nameStart);
                }

                // If solution was changed mark the root node as needed because of change:
                VARIANT_BOOL solutionSaved = VARIANT_TRUE;
                hr = piSolution->get_Saved(&solutionSaved);

                if (SUCCEEDED(hr) && (solutionSaved == VARIANT_FALSE))
                {
                    pHandler->vscAddSolutionAsItemToTree(solutionFileName);
                }

                SysFreeString(solutionFileName);

                // Pass through all open documents and find changed documents:
                VxDTE::Documents* piDocuments = NULL;
                hr = _piDTE->get_Documents(&piDocuments);

                if (SUCCEEDED(hr) && (piDocuments != NULL))
                {
                    long numDocuments = 0;
                    hr = piDocuments->get_Count(&numDocuments);

                    if (SUCCEEDED(hr))
                    {
                        VARIANT docItem = {0};
                        docItem.vt = VT_INT;

                        for (long nDocument = 1 ; nDocument <= numDocuments ; nDocument++)
                        {
                            docItem.intVal = nDocument;
                            VxDTE::Document* piDocument = NULL;
                            hr = piDocuments->Item(docItem, &piDocument);

                            if (SUCCEEDED(hr) && (piDocument != NULL))
                            {
                                // Check if document was changed at all:
                                VARIANT_BOOL documentSaved = VARIANT_TRUE;
                                hr = piDocument->get_Saved(&documentSaved);

                                if (SUCCEEDED(hr) && (documentSaved == VARIANT_FALSE))
                                {
                                    VxDTE::ProjectItem* piProjectItem = NULL;
                                    hr = piDocument->get_ProjectItem(&piProjectItem);

                                    if (SUCCEEDED(hr) && (piProjectItem != NULL))
                                    {
                                        VxDTE::Project* piProject;
                                        hr = piProjectItem->get_ContainingProject(&piProject);

                                        if (SUCCEEDED(hr) && (piProject != NULL))
                                        {
                                            // For each document get the project and name:
                                            BSTR documentName = NULL;
                                            BSTR documentProjectName = NULL;

                                            HRESULT hrDocName = piDocument->get_Name(&documentName);
                                            HRESULT hrDocProjName = piProject->get_Name(&documentProjectName);

                                            if (SUCCEEDED(hrDocName) && SUCCEEDED(hrDocProjName) && (documentName != NULL) && (documentProjectName != NULL))
                                            {
                                                pHandler->vscAddIOpenDocumentAsItemToTree(documentName, documentProjectName, solutionName.c_str());
                                            }

                                            // Release strings:
                                            if (documentName != NULL)
                                            {
                                                SysFreeString(documentName);
                                            }

                                            if (documentProjectName != NULL)
                                            {
                                                SysFreeString(documentProjectName);
                                            }

                                            // Release project:
                                            piProject->Release();
                                        }

                                        // Release project item:
                                        piProjectItem->Release();
                                    }
                                }

                                // Release document:
                                piDocument->Release();
                            }
                        }
                    }

                    // Release documents:
                    piDocuments->Release();
                }

                // Pass through all the projects and see if any were changed:
                VxDTE::Projects* piProjects = NULL;
                hr = piSolution->get_Projects(&piProjects);

                if (SUCCEEDED(hr) && (piProjects != NULL))
                {
                    long numProjects = 0;
                    hr = piProjects->get_Count(&numProjects);

                    if (SUCCEEDED(hr))
                    {
                        VxDTE::Project* ipProject = NULL;
                        VARIANT projItem = {0};
                        projItem.vt = VT_INT;

                        for (long nProject = 1 ; nProject <= numProjects ; nProject++)
                        {
                            projItem.intVal = nProject;
                            hr = piProjects->Item(projItem, &ipProject);

                            if (SUCCEEDED(hr) && (ipProject != NULL))
                            {
                                VARIANT_BOOL projectSaved = VARIANT_TRUE;
                                hr = ipProject->get_Saved(&projectSaved);

                                if (SUCCEEDED(hr) && (projectSaved == VARIANT_FALSE))
                                {
                                    BSTR projectName = NULL;
                                    hr = ipProject->get_Name(&projectName);

                                    if (SUCCEEDED(hr) && (projectName != NULL))
                                    {
                                        pHandler->vscAddOpenProjectAsItemToTree(projectName, solutionName.c_str());

                                        // Release name:
                                        SysFreeString(projectName);
                                    }
                                }

                                // Release project:
                                ipProject->Release();
                            }
                        }
                    }

                    // Release projects:
                    piProjects->Release();
                }
            }

            // Release solution:
            piSolution->Release();
        }

        // Expand the whole tree:
        pHandler->vscExpandWholeTree();
    }
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::saveChangedFiles
// Description: Save open documents, solution and project (if needed)
// Author:      Gilad Yarnitzky
// Date:        14/4/2011
// ---------------------------------------------------------------------------
void vspDTEConnector::saveChangedFiles(bool saveSolutionAndProject)
{
    // save all open documents
    assert(_piDTE);

    if (_piDTE != NULL)
    {
        VxDTE::Documents* piDocuments;
        HRESULT hr = _piDTE->get_Documents(&piDocuments);

        if (SUCCEEDED(hr) && (piDocuments != NULL))
        {
            piDocuments->SaveAll();
            // Release documents
            piDocuments->Release();
        }

        // Save solutions and projects if needed
        if (saveSolutionAndProject)
        {
            // Save solution
            VxDTE::_Solution* piSolution = NULL;
            hr = _piDTE->get_Solution((VxDTE::Solution**)&piSolution);

            if (SUCCEEDED(hr) && (piSolution != NULL))
            {
                // Save Solution:
                VARIANT_BOOL solutionSaved = VARIANT_TRUE;
                hr = piSolution->get_Saved(&solutionSaved);

                if (SUCCEEDED(hr) && (solutionSaved == VARIANT_FALSE))
                {
                    BSTR solutionFullName = NULL;
                    hr = piSolution->get_FullName(&solutionFullName);

                    if (SUCCEEDED(hr) && (solutionFullName != NULL))
                    {
                        hr = piSolution->SaveAs(solutionFullName);

                        // Release Name:
                        SysFreeString(solutionFullName);
                    }
                }

                if (SUCCEEDED(hr))
                {

                    VxDTE::Projects* piProjects = NULL;
                    hr = piSolution->get_Projects(&piProjects);

                    if (SUCCEEDED(hr) && (piProjects != NULL))
                    {
                        long numProjects = 0;
                        hr = piProjects->get_Count(&numProjects);

                        if (SUCCEEDED(hr))
                        {
                            VxDTE::Project* ipProject = NULL;
                            VARIANT projItem = { 0 };
                            projItem.vt = VT_INT;

                            for (long nProject = 1; nProject <= numProjects; nProject++)
                            {
                                projItem.intVal = nProject;
                                hr = piProjects->Item(projItem, &ipProject);

                                if (SUCCEEDED(hr) && (ipProject != NULL))
                                {
                                    VARIANT_BOOL projectSaved = VARIANT_TRUE;
                                    hr = ipProject->get_Saved(&projectSaved);

                                    if (SUCCEEDED(hr) && (projectSaved == VARIANT_FALSE))
                                    {
                                        ipProject->Save();
                                    }

                                    // Release project:
                                    ipProject->Release();
                                }
                            }
                        }

                        // Release projects:
                        piProjects->Release();
                    }
                }

                // Release Solution:
                piSolution->Release();
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::getKernelSourceFilePath
// Description: Get list of Kernel source files for the solution.
//              Since it is more probable that the cl files would be related to
//              start-up project, add them first to the vector, to avoid redundant
//              future file comparisons in the spy
// Author:      Gilad Yarnitzky
// Date:        20/4/2011
// ---------------------------------------------------------------------------
void vspDTEConnector::getKernelSourceFilePath(std::vector<std::wstring>& programsFilePath, bool onlyFromStartupProject)
{
    assert(_piDTE != NULL);

    if (_piDTE != NULL)
    {
        // Get the solution:
        VxDTE::_Solution* pSolution = NULL;
        HRESULT hr = _piDTE->get_Solution((VxDTE::Solution**)(&pSolution));

        if (SUCCEEDED(hr) && (pSolution != NULL))
        {
            if (onlyFromStartupProject)
            {
                // Get the build interface:
                VxDTE::SolutionBuild* pSolutionBuild = NULL;
                hr = pSolution->get_SolutionBuild(&pSolutionBuild);

                if (SUCCEEDED(hr) && (pSolutionBuild != NULL))
                {
                    VARIANT pProjects = {0};

                    pSolutionBuild->get_StartupProjects(&pProjects);

                    if (SUCCEEDED(hr) && (pProjects.vt == (VT_ARRAY | VT_VARIANT)))
                    {
                        // Get the project paths as an array:
                        SAFEARRAY* pStartupProjectPathsArray = pProjects.parray;

                        if ((pStartupProjectPathsArray != NULL) && (pStartupProjectPathsArray->cDims > 0))
                        {
                            // Get the first variant in the array (we expect it to be a string):
                            VARIANT* pFirstStartupProjectPathAsVar = (VARIANT*)(pStartupProjectPathsArray->pvData);

                            if ((pFirstStartupProjectPathAsVar != NULL) && (pFirstStartupProjectPathAsVar->vt == VT_BSTR))
                            {
                                // Use the project path (as a variant) to access the appropriate item in the solution.
                                // Solution items are projects:
                                VxDTE::Project* pFirstStartupProject = NULL;
                                hr = pSolution->Item(*pFirstStartupProjectPathAsVar, &pFirstStartupProject);

                                if (!SUCCEEDED(hr) || (pFirstStartupProject == NULL))
                                {
                                    // If we couldn't get the item directly, the project is probably inside a folder. We need to look for it manually:
                                    std::wstring projectNameString(pFirstStartupProjectPathAsVar->bstrVal);
                                    processProjectNameForSearch(projectNameString);
                                    pFirstStartupProject = getProjectByName(*pSolution, projectNameString);
                                }

                                if (pFirstStartupProject != NULL)
                                {
                                    // Add the cl files related to first startup project:
                                    getCLFilesFromProject(pFirstStartupProject, programsFilePath);
                                }
                            }
                        }

                        // Clear the startup projects:
                        VariantClear(&pProjects);
                    }

                    // Release the build interface:
                    pSolutionBuild->Release();
                }
            }
            else // !onlyFromStartupProject
            {
                VARIANT iAsVar = {0};
                iAsVar.vt = VT_INT;

                // Will fill with projects to check:
                std::queue<VxDTE::Project*> projectsToCheck;

                // Initialized the queue with the solution projects:
                VxDTE::Projects* pSolutionProjects = NULL;
                hr = pSolution->get_Projects(&pSolutionProjects);

                if (SUCCEEDED(hr) && (pSolutionProjects != NULL))
                {
                    // Get the amount of items:
                    long numberOfProjects = 0;
                    hr = pSolutionProjects->get_Count(&numberOfProjects);

                    if (SUCCEEDED(hr))
                    {
                        // Get each item and add it to the queue:
                        for (long i = 0; i < numberOfProjects; i++)
                        {
                            // Get the current item:
                            iAsVar.intVal = (INT)i + 1; // DTE indices are 1-based.
                            VxDTE::Project* pCurrentProject = NULL;
                            hr = pSolutionProjects->Item(iAsVar, &pCurrentProject);

                            if (SUCCEEDED(hr) && (pCurrentProject != NULL))
                            {
                                projectsToCheck.push(pCurrentProject);
                            }
                        }
                    }

                    // Release the projects interface:
                    pSolutionProjects->Release();
                }

                // Keep going until we checked all the hierarchy:
                while (!projectsToCheck.empty())
                {
                    // Get the first project and remove it from the queue:
                    VxDTE::Project* pCurrentProject = projectsToCheck.front();
                    projectsToCheck.pop();

                    // Sanity check:
                    assert(pCurrentProject != NULL);

                    if (pCurrentProject != NULL)
                    {
                        // Add any CL files in the project to the vector:
                        getCLFilesFromProject(pCurrentProject, programsFilePath);

                        // Add all this project or folder's sub-projects to the queue as well:
                        VxDTE::ProjectItems* pCurrentProjectItems = NULL;
                        hr = pCurrentProject->get_ProjectItems(&pCurrentProjectItems);

                        if (SUCCEEDED(hr) && (pCurrentProjectItems != NULL))
                        {
                            // Iterate the project items:
                            long numberOfProjectItems = 0;
                            hr = pCurrentProjectItems->get_Count(&numberOfProjectItems);

                            if (SUCCEEDED(hr))
                            {
                                for (long i = 0; i < numberOfProjectItems; i++)
                                {
                                    iAsVar.intVal = (INT)i + 1; // DTE indices are 1-based.

                                    VxDTE::ProjectItem* pCurrentProjectItem = NULL;
                                    hr = pCurrentProjectItems->Item(iAsVar, &pCurrentProjectItem);

                                    if (SUCCEEDED(hr) && pCurrentProjectItem != NULL)
                                    {
                                        // If a project item is a sub-project, add it to our queue:
                                        VxDTE::Project* pCurrentProjectItemAsSubProject = NULL;
                                        hr = pCurrentProjectItem->get_SubProject(&pCurrentProjectItemAsSubProject);

                                        if (SUCCEEDED(hr) && (pCurrentProjectItemAsSubProject != NULL))
                                        {
                                            projectsToCheck.push(pCurrentProjectItemAsSubProject);
                                        }

                                        // Release the project item interface:
                                        pCurrentProjectItem->Release();
                                    }
                                }
                            }

                            // Release the project items interface:
                            pCurrentProjectItems->Release();
                        }

                        // Release the project interface:
                        pCurrentProject->Release();
                    }
                }
            }

            // Release the solution:
            pSolution->Release();
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::stepInto
// Description: Call the VS DTE step into command
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        8/5/2011
// ---------------------------------------------------------------------------
bool vspDTEConnector::stepInto()
{
    bool retVal = false;

    assert(_piDebugger != NULL);

    if (_piDebugger != NULL)
    {
        // Call the debugger step into function:
        HRESULT hr = _piDebugger->StepInto();

        if (SUCCEEDED(hr))
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::stepOver
// Description: Call the VS DTE step over command
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        8/5/2011
// ---------------------------------------------------------------------------
bool vspDTEConnector::stepOver()
{
    bool retVal = false;

    assert(_piDebugger != NULL);

    if (_piDebugger != NULL)
    {
        // Call the debugger step over function:
        HRESULT hr = _piDebugger->StepOver();

        if (SUCCEEDED(hr))
        {
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::stepOver
// Description: Call the VS DTE step over command
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        8/5/2011
// ---------------------------------------------------------------------------
bool vspDTEConnector::stepOut()
{
    bool retVal = false;

    assert(_piDebugger != NULL);

    if (_piDebugger != NULL)
    {
        // Call the debugger step over function:
        HRESULT hr = _piDebugger->StepOut();

        if (SUCCEEDED(hr))
        {
            retVal = true;
        }
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::isDebuggingOn
// Description: Return true iff the debugger is active
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        8/5/2011
// ---------------------------------------------------------------------------
bool vspDTEConnector::isDebuggingOn()
{
    bool retVal = false;

    // Sanity Check:
    assert(_piDebugger != NULL);

    if (_piDebugger != NULL)
    {
        // Get what is the debugger current mode:
        VxDTE::dbgDebugMode debugMode = dbgDesignMode;
        HRESULT hr = _piDebugger->get_CurrentMode(&debugMode);

        if (SUCCEEDED(hr))
        {
            retVal = (debugMode != dbgDesignMode) ;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::setHexDisplayMode
// Description: Set hex display mode.
// Author:      Gilad Yarnitzky
// Date:        19/5/2011
// ---------------------------------------------------------------------------
void vspDTEConnector::setHexDisplayMode()
{
    assert(_piDebugger != NULL);

    if (_piDebugger != NULL)
    {
        // Check if the hex display mode is on:
        VARIANT_BOOL hexDisplayMode = VSCORE(vscDTEConnector_IsHexDisplayMode)() ? VARIANT_TRUE : VARIANT_FALSE;

        // Set the hex display mode in VS debugger:
        _piDebugger->put_HexDisplayMode(hexDisplayMode);
    }
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::syncHexDisplayModeWithVS
// Description: sync hex display mode to VS
// Author:      Gilad Yarnitzky
// Date:        5/7/2011
// ---------------------------------------------------------------------------
void vspDTEConnector::syncHexDisplayModeWithVS()
{

    assert(_piDebugger != NULL);

    if (_piDebugger != NULL)
    {
        // Get the hex display mode in VS debugger:
        VARIANT_BOOL hexDisplayModeinVS = VARIANT_FALSE;
        HRESULT hr = _piDebugger->get_HexDisplayMode(&hexDisplayModeinVS);
        assert(SUCCEEDED(hr));

        if (SUCCEEDED(hr))
        {
            // Set CodeXL hex mode:
            VSCORE(vscDTEConnector_ChangeHexDisplayMode)(hexDisplayModeinVS == VARIANT_TRUE);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::currentDebugggerMode
// Description: Returns the current debugger mode
// Return Val:  dbgDebugMode
// Author:      Sigal Algranaty
// Date:        21/6/2011
// ---------------------------------------------------------------------------
dbgDebugMode vspDTEConnector::currentDebugggerMode() const
{
    dbgDebugMode retVal = dbgDesignMode;
    assert(_piDebugger != NULL);

    if (_piDebugger != NULL)
    {
        HRESULT hr;
        hr = _piDebugger->get_CurrentMode(&retVal);
        assert(SUCCEEDED(hr));
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::openSolution
// Description: Open a specific solution
// Author:      Gilad Yarnitzky
// Date:        31/5/2011
// ---------------------------------------------------------------------------
bool vspDTEConnector::openSolution(const wchar_t* solutionName)
{
    bool retVal = false;

    assert(_piDTE != NULL);

    if (_piDTE != NULL)
    {
        // Get the solution:
        VxDTE::_Solution* pSolution = NULL;
        HRESULT hr = _piDTE->get_Solution((VxDTE::Solution**)(&pSolution));

        if (SUCCEEDED(hr) && (pSolution != NULL))
        {
            BSTR solutioNameBSTR = SysAllocString(solutionName);

            // Open the Solution:
            hr = pSolution->Open(solutioNameBSTR);

            if (SUCCEEDED(hr))
            {
                retVal = true;
            }

            // Free the string:
            SysFreeString(solutioNameBSTR);

            // Release the solution:
            pSolution->Release();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::isSolutionSampleProject
// Description: checks if the open solution is a sample solution: GRTeaPot
// Author:      Gilad Yarnitzky
// Date:        26/6/2011
// ---------------------------------------------------------------------------
bool vspDTEConnector::isSampleProject()
{
    bool isSampleProject = false;

    assert(_piDTE != NULL);

    if (_piDTE != NULL)
    {
        // Get the solution:
        VxDTE::_Solution* pSolution = NULL;
        HRESULT hr = _piDTE->get_Solution((VxDTE::Solution**)(&pSolution));

        if (SUCCEEDED(hr) && (pSolution != NULL))
        {
            // Get the build interface:
            VxDTE::SolutionBuild* pSolutionBuild = NULL;
            hr = pSolution->get_SolutionBuild(&pSolutionBuild);

            if (SUCCEEDED(hr) && (pSolutionBuild != NULL))
            {
                VARIANT pProjects = {0};

                pSolutionBuild->get_StartupProjects(&pProjects);

                if (SUCCEEDED(hr) && (pProjects.vt == (VT_ARRAY | VT_VARIANT)))
                {
                    // Get the project paths as an array:
                    SAFEARRAY* pStartupProjectPathsArray = pProjects.parray;

                    if ((pStartupProjectPathsArray != NULL) && (pStartupProjectPathsArray->cDims > 0))
                    {
                        // Get the first variant in the array (we expect it to be a string):
                        VARIANT* pFirstStartupProjectPathAsVar = (VARIANT*)(pStartupProjectPathsArray->pvData);

                        if ((pFirstStartupProjectPathAsVar != NULL) && (pFirstStartupProjectPathAsVar->vt == VT_BSTR))
                        {
                            // Use the project path (as a variant) to access the appropriate item in the solution.
                            // Solution items are projects:
                            VxDTE::Project* pFirstStartupProject = NULL;
                            hr = pSolution->Item(*pFirstStartupProjectPathAsVar, &pFirstStartupProject);

                            if (!SUCCEEDED(hr) || (pFirstStartupProject == NULL))
                            {
                                // If we couldn't get the item directly, the project is probably inside a folder. We need to look for it manually:
                                std::wstring projectNameString(pFirstStartupProjectPathAsVar->bstrVal);
                                processProjectNameForSearch(projectNameString);
                                pFirstStartupProject = getProjectByName(*pSolution, projectNameString);
                            }

                            if (pFirstStartupProject != NULL)
                            {
                                BSTR projectNameChecked = NULL;
                                hr = pFirstStartupProject->get_UniqueName(&projectNameChecked);

                                if (SUCCEEDED(hr) && (NULL != projectNameChecked))
                                {
                                    // Special case for GRTeaPot. it is always up to date:
                                    if (_tcsicmp(projectNameChecked, VSP_STR_TeapotSampleName) == 0)
                                    {
                                        isSampleProject = true;
                                    }

                                    // Release the project name:
                                    SysFreeString(projectNameChecked);
                                }
                            }
                        }
                    }

                    // Clear the startup projects:
                    VariantClear(&pProjects);
                }

                // Release the build interface:
                pSolutionBuild->Release();
            }

            // Release the solution:
            pSolution->Release();
        }
    }

    return isSampleProject;
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::getCLFilesFromProject
// Description: Looking for cl files within a specific project
// Arguments:   VxDTE::Project* pProject
//              gtVector<osFilePath>& programsFilePath
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        30/10/2011
// ---------------------------------------------------------------------------
bool vspDTEConnector::getCLFilesFromProject(VxDTE::Project* pProject, std::vector<std::wstring>& programsFilePath)
{
    bool retVal = false;

    // Sanity check:
    assert(pProject != NULL);

    if (pProject != NULL)
    {
        VxDTE::ConfigurationManager* pConfigManager = NULL;
        HRESULT hr = pProject->get_ConfigurationManager(&pConfigManager);

        if (SUCCEEDED(hr) && (pConfigManager != NULL))
        {
            // Get the active configuration:
            VxDTE::Configuration* pActiveConfiguration = NULL;
            hr = pConfigManager->get_ActiveConfiguration(&pActiveConfiguration);

            if (SUCCEEDED(hr) && (pActiveConfiguration != NULL))
            {
                // Get the project "object", i.e. its dispatch interface:
                IDispatch* pProjectAsDispatch = NULL;
                hr = pProject->get_Object(&pProjectAsDispatch);

                if (SUCCEEDED(hr) && (pProjectAsDispatch != NULL))
                {
                    // Get the project's "Visual C Project" interface. If this fails, the project is not a VC/VC++ project,
                    // so we don't support it (and it doesn't support user macros):
                    VCProjectEngineLibrary::VCProject* pProjectAsVCProject = NULL;
                    hr = pProjectAsDispatch->QueryInterface(VCProjectEngineLibrary::IID_VCProject, (void**)(&pProjectAsVCProject));

                    if (SUCCEEDED(hr) && (pProjectAsVCProject != NULL))
                    {
                        // Get the configurations object:
                        IDispatch* pVCConfigurationsAsDispatch = NULL;
                        hr = pProjectAsVCProject->get_Configurations(&pVCConfigurationsAsDispatch);

                        if (SUCCEEDED(hr) && (pVCConfigurationsAsDispatch != NULL))
                        {
                            // Get the configurations object's (configuration) collection interface
                            VCProjectEngineLibrary::IVCCollection* pVCConfigurations = NULL;
                            hr = pVCConfigurationsAsDispatch->QueryInterface(VCProjectEngineLibrary::IID_IVCCollection, (void**)(&pVCConfigurations));

                            if (SUCCEEDED(hr) && (pVCConfigurations != NULL))
                            {
                                // Get the active configuration's full name:
                                std::wstring configurationFullName;
                                bool rcConfName = getConfigurationFullName(*pActiveConfiguration, configurationFullName);

                                if (rcConfName && (!configurationFullName.empty()))
                                {
                                    // Create a variant with the active configuration's name:
                                    VARIANT activeConfNameAsVar = {0};
                                    activeConfNameAsVar.vt = VT_BSTR;
                                    activeConfNameAsVar.bstrVal = SysAllocString(configurationFullName.c_str());

                                    // Use the variant to get the active configuration from the Visual C configurations collection:
                                    IDispatch* pActiveConfigurationAsDispatch = NULL;
                                    hr = pVCConfigurations->Item(activeConfNameAsVar, &pActiveConfigurationAsDispatch);

                                    if (SUCCEEDED(hr) && (pActiveConfigurationAsDispatch != NULL))
                                    {
                                        // Get the configuration's Visual C configuration interface:
                                        VCProjectEngineLibrary::VCConfiguration* pActiveConfigurationAsVCConfig = NULL;
                                        hr = pActiveConfigurationAsDispatch->QueryInterface(VCProjectEngineLibrary::IID_VCConfiguration, (void**)(&pActiveConfigurationAsVCConfig));

                                        if (SUCCEEDED(hr) && (pActiveConfigurationAsVCConfig != NULL))
                                        {
                                            IDispatch* pConfigurationProjectAsDisplatch = NULL;
                                            hr = pActiveConfigurationAsVCConfig->get_project(&pConfigurationProjectAsDisplatch);

                                            if (SUCCEEDED(hr) && (pConfigurationProjectAsDisplatch != NULL))
                                            {
                                                VCProjectEngineLibrary::VCProject* pConfigurationProject = NULL;
                                                hr = pConfigurationProjectAsDisplatch->QueryInterface(VCProjectEngineLibrary::IID_VCProject, (void**)(&pConfigurationProject));

                                                if (SUCCEEDED(hr) && (pConfigurationProject != NULL))
                                                {
                                                    IDispatch* pItemsAsDispatch = NULL;
                                                    hr = pConfigurationProject->get_Files(&pItemsAsDispatch);

                                                    if (SUCCEEDED(hr) && (pItemsAsDispatch != NULL))
                                                    {
                                                        VCProjectEngineLibrary::IVCCollection* pProjectItems;
                                                        hr = pItemsAsDispatch->QueryInterface(VCProjectEngineLibrary::IID_IVCCollection, (void**)(&pProjectItems));

                                                        if (SUCCEEDED(hr) && (pProjectItems != NULL))
                                                        {
                                                            long numItems = 0;
                                                            hr = pProjectItems->get_Count(&numItems);

                                                            if (SUCCEEDED(hr))
                                                            {
                                                                retVal = true;

                                                                VARIANT currentItem = {0};
                                                                currentItem.vt = VT_INT;

                                                                for (long nItem = 1 ; nItem <= numItems ; nItem++)
                                                                {
                                                                    IDispatch* pItemAsDispatch = NULL;
                                                                    currentItem.intVal = nItem;
                                                                    hr = pProjectItems->Item(currentItem, &pItemAsDispatch);

                                                                    if (SUCCEEDED(hr) && (pItemAsDispatch != NULL))
                                                                    {
                                                                        VCProjectEngineLibrary::VCFile* pProjectFile = NULL;
                                                                        hr = pItemAsDispatch->QueryInterface(VCProjectEngineLibrary::IID_VCFile, (void**)(&pProjectFile));

                                                                        if (SUCCEEDED(hr) && (pProjectFile != NULL))
                                                                        {
                                                                            BSTR itemPathBSTR = NULL;
                                                                            // Get the file path and add it:
                                                                            hr = pProjectFile->get_FullPath(&itemPathBSTR);

                                                                            if (SUCCEEDED(hr) && (itemPathBSTR != NULL))
                                                                            {
                                                                                // Check if the item type is .cl:
                                                                                wchar_t* pFileExtension = NULL;
                                                                                VSCORE(vscExtractFileExtension)(itemPathBSTR, pFileExtension);
                                                                                assert(pFileExtension != NULL);

                                                                                if (pFileExtension != NULL)
                                                                                {
                                                                                    if (vspVsUtilsIsEqualNoCase(pFileExtension, L"cl"))
                                                                                    {
                                                                                        programsFilePath.push_back(std::wstring(itemPathBSTR));
                                                                                    }

                                                                                    // Release the allocated string.
                                                                                    VSCORE(vscDeleteWcharString)(pFileExtension);
                                                                                }

                                                                                SysFreeString(itemPathBSTR);
                                                                            }
                                                                        }

                                                                        // Release item as dispatch:
                                                                        pItemAsDispatch->Release();
                                                                    }
                                                                }
                                                            }
                                                        }

                                                        // Release the items Dispatch interface:
                                                        pItemsAsDispatch->Release();
                                                    }

                                                    // Build recursively the list of the names for each name in the list of files:
                                                }
                                            }

                                            // Release the VC project dispatch interface:
                                            pConfigurationProjectAsDisplatch->Release();
                                        }

                                        // Release the VC configuration's dispatch interface:
                                        pActiveConfigurationAsDispatch->Release();
                                    }

                                    // Clear the name variant:
                                    SysFreeString(activeConfNameAsVar.bstrVal);
                                    activeConfNameAsVar.bstrVal = NULL;
                                    VariantClear(&activeConfNameAsVar);
                                }

                                // Release the configurations collection's IVCCollection interface:
                                pVCConfigurations->Release();
                            }

                            // Release the configurations collection's dispatch interface:
                            pVCConfigurationsAsDispatch->Release();
                        }

                        // Release the project's VCProject interface:
                        pProjectAsVCProject->Release();
                    }

                    // Release the project object:
                    pProjectAsDispatch->Release();
                }

                // Release the configuration:
                pActiveConfiguration->Release();
            }

            // Release the configuration manager:
            pConfigManager->Release();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::processProjectNameForSearch
// Description: Processes a file path (which can contain multiple path separators
//              and relative path values, to be searchable in a resolved path.
// Author:      Uri Shomroni
// Date:        1/11/2011
// ---------------------------------------------------------------------------
void vspDTEConnector::processProjectNameForSearch(std::wstring& projectNameString)
{
    static std::wstring doublePathSeparator; // "\\"
    static std::wstring singlePathSeparator; // "\"
    static std::wstring upOneLevelStart; // "..\"
    static std::wstring upOneLevelMiddle; // "\..\"
    static std::wstring currentDirStart; // ".\"
    static std::wstring currentDirMiddle; // "\.\"
    static bool isFirstTime = true;

    if (isFirstTime)
    {
        isFirstTime = false;
        std::wstring extSeparator;
        extSeparator = VSCORE(vscGetOsExtensionSeparator)();

        // Initialize the strings.
        singlePathSeparator = VSCORE(vscGetOsPathSeparator)();

        doublePathSeparator = singlePathSeparator;
        doublePathSeparator += singlePathSeparator;

        currentDirStart = extSeparator;
        currentDirStart += singlePathSeparator;

        currentDirMiddle = singlePathSeparator;
        currentDirMiddle += currentDirStart;

        upOneLevelStart = extSeparator;
        upOneLevelStart += currentDirStart;

        upOneLevelMiddle = singlePathSeparator;
        upOneLevelMiddle += upOneLevelStart;
    }

    // First, remove any double path separators (C:\\Temp\\MyFolder)
    while (projectNameString.find(doublePathSeparator) > -1)
    {
        vspVsUtilsWReplaceAllOccurrences(projectNameString, doublePathSeparator, singlePathSeparator);
    }

    // Now, truncate the relative path markers, if they are present:
    bool goOn = true;

    while (goOn)
    {
        if ((!projectNameString.compare(0, currentDirStart.size(), currentDirStart)) || !(projectNameString.compare(0, upOneLevelStart.size(), upOneLevelStart)))
        {
            // Note that both these strings end with a path separator:
            unsigned int firstSeparator = projectNameString.find(singlePathSeparator);

            if (firstSeparator < projectNameString.length())
            {
                projectNameString = projectNameString.substr(firstSeparator + 1, projectNameString.length() - firstSeparator - 1);
            }
        }
        else // !projectNameString.startsWith(currentDirStart, upOneLevelStart)
        {
            int foundPosition1 = projectNameString.rfind(currentDirMiddle, std::wstring::npos);

            if (foundPosition1 > -1)
            {
                foundPosition1 += currentDirMiddle.length();
            }

            int foundPosition2 = projectNameString.rfind(upOneLevelMiddle, std::wstring::npos);

            if (foundPosition2 > -1)
            {
                foundPosition2 += upOneLevelMiddle.length();
            }

            int foundPosition = std::max(foundPosition1, foundPosition2);

            if (foundPosition > 0 && static_cast<unsigned int>(foundPosition) < projectNameString.length())
            {
                projectNameString = projectNameString.substr(foundPosition, projectNameString.length() - foundPosition);
            }
            else // foundPosition == -1
            {
                goOn = false;
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::clearOpenedFiles
// Description: Clear the list of opened files that are used to check edit and continue
// Author:      Gilad Yarnitzky
// Date:        13/11/2011
// ---------------------------------------------------------------------------
void vspDTEConnector::clearOpenedFiles()
{
    VSCORE(vscDTEConnector_ClearOpenedFiles)(m_pCoreImple);
}


// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::wasAnyOpenedFileModified
// Description: checks if any opened file modified later then opened time
// Author:      Gilad Yarnitzky
// Date:        13/11/2011
// ---------------------------------------------------------------------------
bool vspDTEConnector::wasAnyOpenedFileModified()
{
    bool retVal = false;

    int numOpenedFiles = VSCORE(vscDTEConnector_GetOpenedFilesCount)(m_pCoreImple);

    for (int nFile = 0 ; nFile < numOpenedFiles ; nFile ++)
    {
        // Get the current file checked
        wchar_t* pBuffer = NULL;
        VSCORE(vscDTEConnector_GetOpenedFileAt)(m_pCoreImple, nFile, pBuffer);
        assert(pBuffer);

        if (pBuffer != NULL)
        {
            std::wstring filePath(pBuffer);
            VSCORE(vscDeleteWcharString)(pBuffer);

            // Get current modified time of the file
            time_t lastModifiedFileTime;
            bool isOk = VSCORE(vscGetLastModifiedDate)(filePath.c_str(), lastModifiedFileTime);
            assert(isOk);

            if (isOk)
            {
                time_t fileModificationDate;
                isOk = VSCORE(vscDTEConnector_GetFileModificationDate)(m_pCoreImple, nFile, fileModificationDate);
                assert(isOk);

                if (isOk && (lastModifiedFileTime != fileModificationDate))
                {
                    retVal = true;
                    break;
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::addFileToOpenedList
// Description: Add an opened file if it is not already in the list and of the right type
// Author:      Gilad Yarnitzky
// Date:        13/11/2011
// ---------------------------------------------------------------------------
void vspDTEConnector::addFileToOpenedList(const wchar_t* filePath)
{
    VSCORE(vscDTEConnector_AddFileToOpenedList)(m_pCoreImple, filePath);
}

// ---------------------------------------------------------------------------
// Name:        vspDTEConnector::CloseDisassemblyWindow
// Description: Closes the Visual Studio Disassembly window (if open).
// Author:      Amit Ben-Moshe
// Date:        30/07/2013
// ---------------------------------------------------------------------------
void vspDTEConnector::CloseDisassemblyWindow(void)
{
    VSCORE(vscPrintDebugMsgToDebugLog)(L"vspDTEConnector::CloseDisassemblyWindow -> On CloseDisassemblyWindow: Inside CloseDisassemblyWindow(), trying to acquire a window.");
    // Get the windows collection.
    VxDTE::Windows* piWindows = NULL;
    HRESULT hr = _piDTE->get_Windows(&piWindows);

    if (SUCCEEDED(hr) && (piWindows != NULL))
    {
        VSCORE(vscPrintDebugMsgToDebugLog)(L"vspDTEConnector::CloseDisassemblyWindow -> On CloseDisassemblyWindow: Acquired the windows.");
        // Get the amount of windows.
        long amountOfWindows = 0;
        hr = piWindows->get_Count(&amountOfWindows);

        if (SUCCEEDED(hr) && (amountOfWindows > 0))
        {
            VSCORE(vscPrintDebugMsgToDebugLog)(L"vspDTEConnector::CloseDisassemblyWindow -> On CloseDisassemblyWindow: Amount of windows is positive.");
            // Find the disassembly window.
            VARIANT iAsVAR = {0};
            iAsVAR.vt = VT_INT;
            bool isDone = false;

            for (long i = 0; !isDone && i < amountOfWindows; i++)
            {
                // DTE indices are 1-based.
                iAsVAR.intVal = (INT)i + 1;

                // Get the current window.
                VxDTE::Window* piCurrentWindow = NULL;
                hr = piWindows->Item(iAsVAR, &piCurrentWindow);

                if (SUCCEEDED(hr) && (piCurrentWindow != NULL))
                {
                    BSTR caption = NULL;
                    hr = piCurrentWindow->get_Caption(&caption);

                    if (SUCCEEDED(hr) && (NULL != caption))
                    {
                        static const std::wstring disassemblyWindowCaption = L"Disassembly";

                        if (disassemblyWindowCaption == caption)
                        {
                            hr = piCurrentWindow->Close();
                            isDone = SUCCEEDED(hr);
                            assert(isDone);
                        }

                        SysFreeString(caption);
                    }

                    piCurrentWindow->Release();
                }
            }

            if (isDone)
            {
                VSCORE(vscPrintDebugMsgToDebugLog)(L"vspDTEConnector::CloseDisassemblyWindow -> On CloseDisassemblyWindow: Managed to close Disassembly window.");
            }
            else
            {
                VSCORE(vscPrintDebugMsgToDebugLog)(L"vspDTEConnector::CloseDisassemblyWindow -> On CloseDisassemblyWindow: Failed to close Disassembly window.");
            }
        }

        piWindows->Release();
    }
}

// ---------------------------------------------------------------------------
// Name:        getProjectHierarchy
// Description: Gets the project's IVsHierarchy
// Author:      Ehud Katz
// Date:        28/7/2013
// ---------------------------------------------------------------------------
static IVsHierarchy* getProjectHierarchy(VxDTE::Project& project)
{
    IVsHierarchy* pHierarchy = NULL;

    CodeXLVSPackage* pPackage = vspPackageWrapper::instance().getPackage();
    assert(NULL != pPackage);

    if (NULL != pPackage)
    {
        IVsSolution* spSolution = NULL;
        pPackage->GetVsSiteCache().QueryService(SID_SVsSolution, &spSolution);
        assert(NULL != spSolution);

        if (NULL != spSolution)
        {
            BSTR projectFullName = NULL;
            project.get_FullName(&projectFullName);

            if (NULL != projectFullName)
            {
                spSolution->GetProjectOfUniqueName(projectFullName, &pHierarchy);
                SysFreeString(projectFullName);
            }

            spSolution->Release();
        }
    }

    return pHierarchy;
}

// ---------------------------------------------------------------------------
// Name:        isAppContainer
// Description: Checks if the project is an AppContainer
// Author:      Ehud Katz
// Date:        28/7/2013
// ---------------------------------------------------------------------------
static bool isAppContainer(VxDTE::Project& project)
{
    bool ret = false;
#if defined VSP_VS11BUILD || defined VSP_VS12BUILD || defined VSP_VS14BUILD
    IVsHierarchy* pHierarchy = getProjectHierarchy(project);

    if (NULL != pHierarchy)
    {
        VARIANT propValueAsVar = {0};
        HRESULT hr = pHierarchy->GetProperty(VSITEMID_ROOT, VSHPROPID_AppContainer, &propValueAsVar);

        if (SUCCEEDED(hr) && VT_BOOL == propValueAsVar.vt)
        {
            ret = (VARIANT_FALSE != propValueAsVar.boolVal);
        }

        VariantClear(&propValueAsVar);

        pHierarchy->Release();
    }

#else
    GT_UNREFERENCED_PARAMETER(project);
#endif

    return ret;
}

// ---------------------------------------------------------------------------
// Name:        parseAppxRecipe
// Description: Parse the vs.appxrecipe file after a successful deployment
// Author:      Ehud Katz
// Date:        28/7/2013
// ---------------------------------------------------------------------------
bool parseAppxRecipe(VxDTE::Project& project, BSTR configurationName, std::wstring& appUserModelId)
{
    bool ret = false;
#if defined VSP_VS11BUILD || defined VSP_VS12BUILD || defined VSP_VS14BUILD
    IVsHierarchy* pHierarchy = getProjectHierarchy(project);

    if (NULL != pHierarchy)
    {
        IVsBuildPropertyStorage* pStorage = NULL;
        pHierarchy->QueryInterface(&pStorage);
        pHierarchy->Release();

        if (NULL != pStorage)
        {
            BSTR layoutDirBSTR = NULL;
            pStorage->GetPropertyValue(L"LayoutDir", configurationName, PST_PROJECT_FILE, &layoutDirBSTR);

            if (NULL != layoutDirBSTR)
            {
                char* pFilePath = NULL;
                bool isPathExists = VSCORE(vscDTEConnector_ParseAppxRecipe_IsPathExists)(layoutDirBSTR, pFilePath);
                SysFreeString(layoutDirBSTR);

                if (isPathExists && pFilePath != NULL)
                {
                    // Define an XML document:
                    std::string utf8FilePath(pFilePath);

                    // Delete the allocated memory.
                    VSCORE(vscDeleteCharString)(pFilePath);

                    TiXmlDocument doc(utf8FilePath.c_str());

                    // Load the file:
                    if (doc.LoadFile())
                    {
                        TiXmlHandle docHandle(&doc);

                        TiXmlNode* pProjectNode = docHandle.FirstChild("Project").Node();

                        if (NULL != pProjectNode)
                        {
                            TiXmlNode* pPropertyGroupNode = pProjectNode->FirstChild("PropertyGroup");

                            while (NULL != pPropertyGroupNode)
                            {
                                TiXmlNode* pRegisteredUserModeAppIDNode = pPropertyGroupNode->FirstChild("RegisteredUserModeAppID");

                                if (NULL != pRegisteredUserModeAppIDNode)
                                {
                                    TiXmlNode* pValueNode = pRegisteredUserModeAppIDNode->FirstChild();

                                    if (NULL != pValueNode)
                                    {
                                        TiXmlText* pValueText = pValueNode->ToText();

                                        if (NULL != pValueText)
                                        {
                                            std::wstringstream converter;
                                            converter << pValueText->Value();
                                            appUserModelId = converter.str();
                                            ret = true;
                                            break;
                                        }
                                    }
                                }

                                pPropertyGroupNode = pPropertyGroupNode->NextSibling();
                            }
                        }
                    }
                }
            }

            pStorage->Release();
        }
    }

#else
    GT_UNREFERENCED_PARAMETER(project);
    GT_UNREFERENCED_PARAMETER(configurationName);
    GT_UNREFERENCED_PARAMETER(appUserModelId);
#endif
    return ret;
}

// ---------------------------------------------------------------------------
bool vspDTEConnector::getActiveDocumentFileFullPath(std::wstring& filePath)
{
    bool retVal = false;
    filePath.clear();

    if (_piDTE != NULL)
    {
        // Check if we have an active document:
        VxDTE::Document* piDocument = NULL;
        HRESULT hr = _piDTE->get_ActiveDocument((VxDTE::Document**)(&piDocument));

        if (SUCCEEDED(hr) && (piDocument != NULL))
        {
            BSTR pFilePath = NULL;
            BSTR pFileName = NULL;
            hr = piDocument->get_Path(&pFilePath);

            // Build the output string using this stream:
            std::wstringstream tmpStream;

            if (SUCCEEDED(hr) && (pFilePath != NULL))
            {
                // Copy the selected text to a string:
                tmpStream << pFilePath;
                SysFreeString(pFilePath);
                hr = piDocument->get_Name(&pFileName);

                if (SUCCEEDED(hr) && (pFilePath != NULL))
                {
                    tmpStream << pFileName;
                    SysFreeString(pFileName);
                    retVal = true;
                }
            }

            // Build the output string:
            filePath = std::wstring(tmpStream.str());

            // Release the document:
            piDocument->Release();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
bool vspDTEConnector::GetActiveWindowHandle(HWND& handleWindow)
{
    bool retVal = false;

    if (_piDTE != NULL)
    {
        VxDTE::Window* piWindow = NULL;
        HRESULT hr = _piDTE->get_ActiveWindow((VxDTE::Window**)(&piWindow));

        if (SUCCEEDED(hr) && (piWindow != NULL))
        {
            long windowHandle;
            hr = piWindow->get_HWnd(&windowHandle);

            if (SUCCEEDED(hr))
            {
                handleWindow = (HWND)windowHandle;
                retVal = true;
            }

            // Release the window:
            piWindow->Release();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
bool vspDTEConnector::isFileOpened(const std::wstring& filePath)
{
    bool retVal = false;
    int size = VSCORE(vscDTEConnector_GetOpenedFilesCount)(m_pCoreImple);
    assert(size > 0);

    if (size > 0)
    {
        for (int i = 0; i < size; i++)
        {
            wchar_t* pBuffer = NULL;
            VSCORE(vscDTEConnector_GetOpenedFileAt)(m_pCoreImple, i, pBuffer);
            assert(pBuffer != NULL);

            if (pBuffer != NULL && !filePath.compare(pBuffer))
            {
                retVal = true;
                break;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
bool vspDTEConnector::closeOpenedFile(const wchar_t* filePath)
{
    bool retVal = false;

    assert(_piDTE != NULL);

    if (_piDTE != NULL)
    {
        // Get the windows collection:
        VxDTE::Windows* piWindows = NULL;
        HRESULT hr = _piDTE->get_Windows(&piWindows);

        if (SUCCEEDED(hr) && (piWindows != NULL))
        {
            // Get the amount of windows:
            long amountOfWindows = 0;
            hr = piWindows->get_Count(&amountOfWindows);

            if (SUCCEEDED(hr) && (amountOfWindows > 0))
            {
                // Iterate the windows:
                VARIANT iAsVAR = {0};
                iAsVAR.vt = VT_INT;

                for (long i = 0; i < amountOfWindows; i++)
                {
                    // DTE indices are 1-based:
                    iAsVAR.intVal = (INT)i + 1;

                    // Get the current window:
                    VxDTE::Window* piCurrentWindow = NULL;
                    hr = piWindows->Item(iAsVAR, &piCurrentWindow);

                    if (SUCCEEDED(hr) && (piCurrentWindow != NULL))
                    {
                        // Get the document:
                        VxDTE::Document* piCurrentDocument = NULL;
                        hr = piCurrentWindow->get_Document(&piCurrentDocument);

                        if (SUCCEEDED(hr) && (piCurrentDocument != NULL))
                        {
                            // Get the document's full path:
                            BSTR currentDocumentFullPath = NULL;
                            hr = piCurrentDocument->get_FullName(&currentDocumentFullPath);

                            if (SUCCEEDED(hr) && (currentDocumentFullPath != NULL))
                            {
                                if (VSCORE(vscIsPathStringsEqual)(currentDocumentFullPath, filePath))
                                {
                                    void* windowToClose = (void*)piCurrentWindow;
                                    closeAndReleaseWindow(windowToClose);
                                    retVal = true;
                                }

                                // Free the string:
                                SysFreeString(currentDocumentFullPath);
                            }

                            // Release the document:
                            piCurrentDocument->Release();
                        }
                    }

                    if (retVal)
                    {
                        break;
                    }
                }
            }

            // Release the windows:
            piWindows->Release();
        }
    }

    return retVal;
}

void vspDTEConnector::GetListOfFilesContainedAtDirectory(const std::wstring& folderStr, std::vector<std::wstring>& listOfFilePathContained)
{
    assert(_piDTE != NULL);

    if (_piDTE != NULL)
    {
        // Get the windows collection:
        VxDTE::Windows* piWindows = NULL;
        HRESULT hr = _piDTE->get_Windows(&piWindows);

        if (SUCCEEDED(hr) && (piWindows != NULL))
        {
            // Get the amount of windows:
            long amountOfWindows = 0;
            hr = piWindows->get_Count(&amountOfWindows);

            if (SUCCEEDED(hr) && (amountOfWindows > 0))
            {
                // Iterate the windows:
                VARIANT iAsVAR = { 0 };
                iAsVAR.vt = VT_INT;

                for (long i = 0; i < amountOfWindows; i++)
                {
                    // DTE indices are 1-based:
                    iAsVAR.intVal = (INT)i + 1;

                    // Get the current window:
                    VxDTE::Window* piCurrentWindow = NULL;
                    hr = piWindows->Item(iAsVAR, &piCurrentWindow);

                    if (SUCCEEDED(hr) && (piCurrentWindow != NULL))
                    {
                        // Get the document:
                        VxDTE::Document* piCurrentDocument = NULL;
                        hr = piCurrentWindow->get_Document(&piCurrentDocument);

                        if (SUCCEEDED(hr) && (piCurrentDocument != NULL))
                        {
                            // Get the document's full path:
                            BSTR currentDocumentFullPath = NULL;
                            hr = piCurrentDocument->get_FullName(&currentDocumentFullPath);

                            if (SUCCEEDED(hr) && (currentDocumentFullPath != NULL))
                            {
                                std::wstring pathToCheck(currentDocumentFullPath);

                                // Check if this file contained in the requested folder:
                                std::wstring::size_type pos = pathToCheck.find(folderStr);

                                if (pos != std::wstring::npos)
                                {
                                    listOfFilePathContained.push_back(pathToCheck);
                                }

                                // Free the string:
                                SysFreeString(currentDocumentFullPath);
                            }

                            // Release the document:
                            piCurrentDocument->Release();
                        }
                    }
                }
            }

            // Release the windows:
            piWindows->Release();
        }
    }
}

// ---------------------------------------------------------------------------
// Look for an MDI window with a file whose path matches the argument 'filePath',
// and save the file from that window
bool vspDTEConnector::saveFileWithPath(const std::wstring& filePath)
{
    bool retVal = false;

    assert(_piDTE != NULL);

    if (_piDTE != NULL)
    {
        // Get the windows collection:
        VxDTE::Windows* piWindows = NULL;
        HRESULT hr = _piDTE->get_Windows(&piWindows);

        if (SUCCEEDED(hr) && (piWindows != NULL))
        {
            // Get the amount of windows:
            long amountOfWindows = 0;
            hr = piWindows->get_Count(&amountOfWindows);

            if (SUCCEEDED(hr) && (amountOfWindows > 0))
            {
                // Iterate the windows:
                VARIANT iAsVAR = {0};
                iAsVAR.vt = VT_INT;

                for (long i = 0; i < amountOfWindows; i++)
                {
                    // DTE indices are 1-based:
                    iAsVAR.intVal = (INT)i + 1;

                    // Get the current window:
                    VxDTE::Window* piCurrentWindow = NULL;
                    hr = piWindows->Item(iAsVAR, &piCurrentWindow);

                    if (SUCCEEDED(hr) && (piCurrentWindow != NULL))
                    {
                        // Get the document:
                        VxDTE::Document* piCurrentDocument = NULL;
                        hr = piCurrentWindow->get_Document(&piCurrentDocument);

                        if (SUCCEEDED(hr) && (piCurrentDocument != NULL))
                        {
                            // Get the document's full path:
                            BSTR currentDocumentFullPath = NULL;
                            hr = piCurrentDocument->get_FullName(&currentDocumentFullPath);

                            if (SUCCEEDED(hr) && (currentDocumentFullPath != NULL))
                            {
                                std::wstring pathToCheck(currentDocumentFullPath);

                                if (pathToCheck == filePath)
                                {
                                    // empty file name to save means save with the same name:
                                    BSTR fileToSave = NULL;
                                    VxDTE::vsSaveStatus saveStatus;
                                    piCurrentDocument->Save(fileToSave, &saveStatus);

                                    if (vsSaveSucceeded == saveStatus)
                                    {
                                        retVal = true;
                                    }
                                }

                                // Free the string:
                                SysFreeString(currentDocumentFullPath);
                            }

                            // Release the document:
                            piCurrentDocument->Release();
                        }
                    }

                    if (retVal)
                    {
                        break;
                    }
                }
            }

            // Release the windows:
            piWindows->Release();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
void vspDTEConnector::ShowOutputWindow()
{
    assert(_piDTE != NULL);

    if (_piDTE != NULL)
    {
        BSTR commandStr = SysAllocString(L"View.Output");

        if (commandStr != nullptr)
        {
            _piDTE->ExecuteCommand(commandStr);
            SysFreeString(commandStr);
        }
    }
}

void vspDTEConnector::SaveAllCommand()
{
    assert(_piDTE != NULL);

    if (_piDTE != NULL)
    {
        BSTR commandStr = SysAllocString(L"File.SaveAll");

        if (commandStr != nullptr)
        {
            _piDTE->ExecuteCommand(commandStr);
            SysFreeString(commandStr);
        }
    }
}

void vspDTEConnector::ExecuteCommand(wchar_t* pCommandText)
{
    assert(_piDTE != nullptr);

    if (_piDTE != nullptr && pCommandText != nullptr)
    {
        BSTR commandStr = SysAllocString(pCommandText);

        if (commandStr != nullptr)
        {
            _piDTE->ExecuteCommand(commandStr);
            SysFreeString(commandStr);
        }
    }
}

void vspDTEConnector::vspDtecBuildSaveTree(IDteTreeEventHandler* pDteTreeEventHandler)
{
    buildSaveTree(pDteTreeEventHandler);
}

void vspDTEConnector::vspDtecGetKernelSourceFilePath(wchar_t**& pProgramFilePaths, int& programFilePathsSize, bool isOnlyFromStartupProject)
{
    // Init.
    pProgramFilePaths = NULL;
    programFilePathsSize = 0;

    // Prepare a vector of std::wstrings.
    std::vector<std::wstring> filePathsVec;

    // Invoke the actual logic.
    getKernelSourceFilePath(filePathsVec, isOnlyFromStartupProject);

    size_t sz = filePathsVec.size();

    if (sz > 0)
    {
        programFilePathsSize = (int)sz;

        // Allocate the array.
        pProgramFilePaths = new wchar_t* [sz];

        // Fill the array.
        for (size_t i = 0; i < sz; i++)
        {
            pProgramFilePaths[i] = vspAllocateAndCopy(filePathsVec[i]);
        }
    }
}

void vspDTEConnector::vspDtecDeleteWCharStrBuffers(wchar_t**& pBuffersArr, int arrSize)
{
    vspVsUtilsDeleteWcharBuffersArray(pBuffersArr, arrSize);
}

void vspDTEConnector::vspDtecGetActiveDocumentFileFullPath(wchar_t*& pBuffer)
{
    std::wstring filePath;

    getActiveDocumentFileFullPath(filePath);

    pBuffer = vspAllocateAndCopy(filePath);
}

void vspDTEConnector::vspDtecGetListOfFilesContainedAtDirectory(const wchar_t* pFolderStr, wchar_t**& pListOfFilePathContained, int& listOfPathSize)
{
    pListOfFilePathContained = nullptr;
    listOfPathSize = 0;

    if (pFolderStr != nullptr)
    {
        // Prepare a vector of std::wstrings.
        std::vector<std::wstring> listOfFilePathContained;
        std::wstring folderStr(pFolderStr);

        GetListOfFilesContainedAtDirectory(folderStr, listOfFilePathContained);

        // copy the strings
        size_t sz = listOfFilePathContained.size();

        if (sz > 0)
        {
            listOfPathSize = (int)sz;

            // Allocate the array.
            pListOfFilePathContained = new wchar_t* [sz];

            // Fill the array.
            for (size_t i = 0; i < sz; i++)
            {
                pListOfFilePathContained[i] = vspAllocateAndCopy(listOfFilePathContained[i]);
            }
        }
    }
}

void vspDTEConnector::vspDtecSaveFileWithPath(const wchar_t* folderStr)
{
    std::wstring folderPath(folderStr);
    saveFileWithPath(folderPath);
}

bool vspDTEConnector::vspDtecResumeDebugging()
{
    bool retVal = resumeDebugging();

    VSP_ASSERT(retVal);

    return retVal;
}

int vspDTEConnector::GetOpenedFilesCount()
{
    int retVal = VSCORE(vscDTEConnector_GetOpenedFilesCount)(m_pCoreImple);
    return retVal;
}

bool vspDTEConnector::GetOpenedFileAt(int index, std::wstring& filePath)
{
    bool retVal = false;

    wchar_t* pBuffer = NULL;
    VSCORE(vscDTEConnector_GetOpenedFileAt)(m_pCoreImple, index, pBuffer);
    assert(pBuffer);

    if (pBuffer != NULL)
    {
        filePath = pBuffer;
        VSCORE(vscDeleteWcharString)(pBuffer);
        retVal = true;
    }

    return retVal;
}
