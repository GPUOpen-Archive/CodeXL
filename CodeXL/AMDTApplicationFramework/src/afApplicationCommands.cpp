//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afApplicationCommands.cpp
///
//==================================================================================

// Qt
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

#include <AMDTApplicationFramework/Include/views/afSourceCodeView.h>

// Infra:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFindWidget.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDesktop.h>
#include <AMDTOSWrappers/Include/osFileLauncher.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osStringConstants.h>


// Local:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afBrowseAction.h>
#include <AMDTApplicationFramework/Include/afCommandIds.h>
#include <AMDTApplicationFramework/Include/afDocUpdateManager.h>
#include <AMDTApplicationFramework/Include/afGeneralViewsCreator.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/dialogs/afGlobalSettingsDialog.h>
#include <AMDTApplicationFramework/Include/afLoadProjectCommand.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afNewProjectDialog.h>
#include <AMDTApplicationFramework/Include/afPluginConnectionManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afSaveProjectCommand.h>
#include <AMDTApplicationFramework/Include/afSoftwareUpdaterWindow.h>
#include <AMDTApplicationFramework/Include/afMessageBox.h>
#include <AMDTApplicationFramework/Include/dialogs/afHelpAboutDialog.h>
#include <AMDTApplicationFramework/Include/views/afInformationView.h>

static QStringList s_sampleSourceNamesListTeapot;
static QStringList s_sampleSourceNamesListD3DMT;
static QStringList s_sampleSourceNamesListMatMul;

// These should match the value of AP_DEFAULT_INTERCEPTION_METHOD:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define AF_DEFAULT_INTERCEPTION_METHOD_STR L"0"
    #define AF_DEFAULT_PLATFROM_ARCHITECHTURE AF_STR_loadProjectArchitecture32Bit
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #if AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT
        #define AF_DEFAULT_INTERCEPTION_METHOD_STR L"2"
        #define AF_DEFAULT_PLATFROM_ARCHITECHTURE AF_STR_loadProjectArchitecture64Bit
    #elif AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT
        #define AF_DEFAULT_INTERCEPTION_METHOD_STR L"1"
    #else
        #error Error: Unknown Linux variant
    #endif
#endif

#define AF_CODEXL_PRINT_TIMESTAMPS_ENV_NAME L"AF_CODEXL_PRINT_TIMESTAMPS"
#define AF_CODEXL_PRINT_TIMESTAMPS_ENV_VALUE L"TRUE"

#define AF_STR_sample_ka_settings   L"<KernelAnalyzer>\r\n"\
    L"<ProjectArchitecture>" AF_DEFAULT_PLATFROM_ARCHITECHTURE L"</ProjectArchitecture>\r\n"\
    L"<FilesSection>REPLACE_ME_WITH_KERNEL_AND_SHADER_FILES\r\n"\
    L"</FilesSection>\r\n"\
    L"<ProgramSection>REPLACE_ME_WITH_PROGRAMS\r\n"\
    L"</ProgramSection>\r\n"\
    L"<BuildOptions>\r\n"\
    L"REPLACE_ME_WITH_BUILD_OPTIONS\r\n"\
    L"</BuildOptions>\r\n"\
    L"<ShaderCompileType>\r\n"\
    L"REPLACE_ME_WITH_SHADER_COMPILE_TYPE\r\n"\
    L"</ShaderCompileType>\r\n"\
    L"</KernelAnalyzer>\r\n"

#define AF_STR_sample_gd_settings   L"<GPUDebug>\r\n"\
    L"<InterceptionMethod>" AF_DEFAULT_INTERCEPTION_METHOD_STR L"</InterceptionMethod>\r\n"\
    L"<DebuggerBreakpoints><Breakpoints>\r\n"\
    L"</Breakpoints></DebuggerBreakpoints>\r\n"\
    L"<FrameTerminators>1</FrameTerminators>\r\n"\
    L"<OpenGLStateVariables>\r\n"\
    L"<StateVariables>\r\n"\
    L"</StateVariables>\r\n"\
    L"</OpenGLStateVariables>\r\n"\
    L"<GLDebugOutput>\r\n"\
    L"<GLDebugOutputLoggingEnabled>false</GLDebugOutputLoggingEnabled>\r\n"\
    L"<GLDebugOutputBreakOnReports>false</GLDebugOutputBreakOnReports>\r\n"\
    L"<GLDebugOutputMessagesMask>0</GLDebugOutputMessagesMask>\r\n"\
    L"<GLDebugOutputSeverity>0</GLDebugOutputSeverity>\r\n"\
    L"</GLDebugOutput>\r\n"\
    L"</GPUDebug>\r\n"

#define AF_STR_sample_gpu_settings1 L"<OpenCLAppTrace>\r\n"\
    L"<Session type=\"Current\">\r\n"\
    L"<GenerateOccupancyInfo>T</GenerateOccupancyInfo>\r\n"\
    L"<AlwaysShowAPIErrorCode>F</AlwaysShowAPIErrorCode>\r\n"\
    L"<CollapseAllclGetEventInfoCalls>T</CollapseAllclGetEventInfoCalls>\r\n"\
    L"<EnableNavigationToSourceCode>F</EnableNavigationToSourceCode>\r\n"\
    L"<GenerateSummaryPage>T</GenerateSummaryPage>\r\n"\
    L"<APIsToTrace>F</APIsToTrace>\r\n"\
    L"<MaximumNumberOfAPIs>1000000</MaximumNumberOfAPIs>\r\n"\
    L"<WriteDataTimeOut>F</WriteDataTimeOut>\r\n"\
    L"<TimeOutInterval>100</TimeOutInterval>\r\n"\
    L"<RuleTree />\r\n"\
    L"<APIsFilterTree />\r\n"\
    L"</Session>\r\n"\
    L"</OpenCLAppTrace>\r\n"

#define AF_STR_sample_gpu_settings2 L"<GPUPerformanceCounters>\r\n"\
    L"<Session type=\"Current\">\r\n"\
    L"<GenerateOccupancyInfo>T</GenerateOccupancyInfo>\r\n"\
    L"<NumberOfCountersSelected>18 Selected Counters</NumberOfCountersSelected>\r\n"\
    L"<LoadSelection>T</LoadSelection>\r\n"\
    L"<SaveSelection>T</SaveSelection>\r\n"\
    L"<CounterTree />\r\n"\
    L"</Session>\r\n"\
    L"</GPUPerformanceCounters>\r\n"

#define AF_STR_TeapotBuildOptions L"-D GRID_NUM_CELLS_X=64 -D GRID_NUM_CELLS_Y=64 -D GRID_NUM_CELLS_Z=64 -D GRID_INV_SPACING=1.000000f -D GRID_SPACING=1.000000f -D GRID_SHIFT_X=6 \r\n"\
    L"-D GRID_SHIFT_Y=6 -D GRID_SHIFT_Z=6 -D GRID_STRIDE_Y=64 -D GRID_STRIDE_SHIFT_Y=6 -D GRID_STRIDE_Z=4096 -D GRID_STRIDE_SHIFT_Z=12 -I REPLACE_INCLUDE\r\n"

#define AF_STR_startup_page_teapot_cxl_ka_default_values L"64,64,64,4,4,4,100"

// Static member initialization:
afApplicationCommands* afApplicationCommands::m_spMySingleInstance = nullptr;
acFindWidget* afApplicationCommands::m_spFindWidget = nullptr;
bool afApplicationCommands::m_sIsInLoadProcess = false;


// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::afApplicationCommands
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        4/4/2012
// ---------------------------------------------------------------------------
afApplicationCommands::afApplicationCommands()
{
    gtString envValue;
    m_shouldPrintPerformanceTimestamps = false;

    if (osGetCurrentProcessEnvVariableValue(AF_CODEXL_PRINT_TIMESTAMPS_ENV_NAME, envValue))
    {
        m_shouldPrintPerformanceTimestamps = (envValue == AF_CODEXL_PRINT_TIMESTAMPS_ENV_VALUE);
    }


    // Initialize the list of sample source names
    s_sampleSourceNamesListMatMul << AF_STR_MatMulSrc1;
    s_sampleSourceNamesListD3DMT << AF_STR_D3DMTSrc1;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc1;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc2;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc3;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc4;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc5;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc6;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc7;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc8;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc9;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc10;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc11;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc12;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc13;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc14;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc15;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc16;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc17;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc18;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc19;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc20;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc21;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc22;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc23;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc24;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc25;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc26;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc27;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc28;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc29;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc30;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc31;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc32;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc33;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc34;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc35;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc36;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc37;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc38;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc39;
    s_sampleSourceNamesListTeapot << AF_STR_TeapotSrc40;
}


// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::~afApplicationCommands
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        4/4/2012
// ---------------------------------------------------------------------------
afApplicationCommands::~afApplicationCommands()
{

}

// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::gdInstance
// Description: Return my single instance
// Return Val:  afApplicationCommands*
// Author:      Sigal Algranaty
// Date:        7/2/2011
// ---------------------------------------------------------------------------
afApplicationCommands* afApplicationCommands::instance()
{
    return m_spMySingleInstance;
}

// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::registerInstance
// Description: Register my single instance
// Arguments:   gdApplicationCommands* pApplicationCommandsInstance
// Author:      Sigal Algranaty
// Date:        7/2/2011
// ---------------------------------------------------------------------------
bool afApplicationCommands::registerInstance(afApplicationCommands* pApplicationCommandsInstance)
{
    bool retVal = false;

    // Do not allow multiple registration for my instance:
    GT_IF_WITH_ASSERT(m_spMySingleInstance == nullptr)
    {
        m_spMySingleInstance = pApplicationCommandsInstance;
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::cleanupInstance
// Description: Cleans up the instance. Should ONLY be called once to destroy the instance
//              in parallel to the registerInstance function.
// Author:      Uri Shomroni
// Date:        12/9/2012
// ---------------------------------------------------------------------------
void afApplicationCommands::cleanupInstance()
{
    if (nullptr != m_spMySingleInstance)
    {
        static bool onlyOnce = true;
        GT_ASSERT(onlyOnce);
        onlyOnce = false;

        delete m_spMySingleInstance;
        m_spMySingleInstance = nullptr;
    }
}

bool afApplicationCommands::ConvertSamplesFilePath(const osFilePath& srcFilePath, osFilePath& localSrcFilePath)
{
    bool retVal = false;

    // By default set the file path to the input source file path
    localSrcFilePath = srcFilePath;

    // If the file exists, we do not touch it
    if (!srcFilePath.exists())
    {
        // Get the source file name
        gtString fileNameAndExtension;
        srcFilePath.getFileNameAndExtension(fileNameAndExtension);
        QString fileNameQt = acGTStringToQString(fileNameAndExtension);

        osFilePath::osApplicationSpecialDirectories folderEnum = osFilePath::OS_CODEXL_EXAMPLES_PATH;
        if (s_sampleSourceNamesListTeapot.contains(fileNameQt.toLower()))
        {
            retVal = true;
            folderEnum = osFilePath::OS_CODEXL_TEAPOT_SAMPLE_PATH;
        }

        else if (s_sampleSourceNamesListMatMul.contains(fileNameQt.toLower()))
        {
            retVal = true;
            folderEnum = osFilePath::OS_CODEXL_MAT_MUL_SAMPLE_PATH;
        }

        if (s_sampleSourceNamesListD3DMT.contains(fileNameQt.toLower()))
        {
            retVal = true;
            folderEnum = osFilePath::OS_CODEXL_D3D_MT_SAMPLE_PATH;
        }

        if (retVal)
        {
            // Convert the file name to local examples path
            localSrcFilePath = osFilePath(folderEnum);

            gtString fileName, extension;
            srcFilePath.getFileName(fileName);
            srcFilePath.getFileExtension(extension);
            localSrcFilePath.setFileName(fileName);
            localSrcFilePath.setFileExtension(extension);
        }
    }
    return retVal;
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        applicationRootString
/// \brief Description: Return the application root string
/// \return gtString
/// -----------------------------------------------------------------------------------------------
gtString afApplicationCommands::applicationRootString()
{
    gtString retVal = afProjectManager::instance().currentProjectSettings().projectName();

    // When there is not debug session:
    if (retVal.isEmpty())
    {
        retVal = AF_STR_TitleNoProjectLoaded;
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::OnFileSaveProject
// Description: Save application framework project
// Author:      Sigal Algranaty
// Date:        5/4/2012
// ---------------------------------------------------------------------------
void afApplicationCommands::OnFileSaveProject()
{
    // Execute the save command only when we are no during the load process:
    if (!m_sIsInLoadProcess)
    {
        // Get the current project name:
        const osFilePath& projectFilePath = afProjectManager::instance().currentProjectFilePath();

        // If there is a loaded project:
        if (!(projectFilePath.asString().isEmpty()))
        {
            // Save the current project into a file:
            afSaveProjectCommand saveProjectCmd(projectFilePath);
            bool rc = saveProjectCmd.execute();
            GT_ASSERT(rc);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::OnFileSaveProjectAs
// Description: Show save as dialog and save application framework project
// Author:      Sigal Algranaty
// Date:        5/4/2012
// ---------------------------------------------------------------------------
void afApplicationCommands::OnFileSaveProjectAs()
{
    // Get the current project name:
    const osFilePath& projectFilePath = afProjectManager::instance().originalProjectFilePath();
    QString defaultFilePath = acGTStringToQString(projectFilePath.asString());

    // Open Save Dialog:
    QString selectedFilePath = ShowFileSelectionDialog(AF_STR_saveProjectDialogHeader, defaultFilePath, AF_STR_projectFileDetails, nullptr, true);

    if (!selectedFilePath.isEmpty())
    {
        // Set the new project file path into the global variables:
        gtString gtPath = acQStringToGTString(selectedFilePath);
        afProjectManager::instance().setCurrentProjectFilePath(gtPath);

        // Run the Save project command
        afSaveProjectCommand saveProjectCmd(gtPath);
        saveProjectCmd.execute();
    }
}

// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::OnFileNewProject
// Description: Create a new application framework project
// Author:      Sigal Algranaty
// Date:        5/4/2012
// ---------------------------------------------------------------------------
void afApplicationCommands::OnFileNewProject(const gtString& executablePath)
{
    // save current project
    osFilePath currentProject = afProjectManager::instance().currentProjectFilePath();

    if (!currentProject.asString().isEmpty())
    {
        afSaveProjectCommand saveProjectCmd(currentProject);
        saveProjectCmd.execute();
    }

    // Show the debug settings dialog:
    afNewProjectDialog::instance().ShowDialog(afNewProjectDialog::AF_DIALOG_NEW_PROJECT, AF_globalSettingsGeneralHeaderUnicode, executablePath);

    // Clear the information view (output view):
    afApplicationCommands::instance()->ClearInformationView();

    // Save the project file:
    OnFileSaveProject();

    UpdateRecentlyUsedProjects();
}

// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::OnFileOpenProject
// Description: Open an application framework project
// Arguments:   const gtString& projectFilePath
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        5/4/2012
// ---------------------------------------------------------------------------
void afApplicationCommands::OnFileOpenProject(const gtString& projectFilePath)
{
    gtString filePath;

    if (projectFilePath.isEmpty())
    {
        // Get the current project name:
        osFilePath projectOsFilePath = afProjectManager::instance().currentProjectFilePath();

        // If there is not current project open:
        if (projectOsFilePath.isEmpty())
        {
            // Get the the User AppData directory:
            afGetUserDataFolderPath(projectOsFilePath);
        }

        // Get the default file path as QString:
        QString defaultFilePath = acGTStringToQString(projectOsFilePath.asString());

        // Open the file selection dialog:
        QString selectedProjectFilePath = ShowFileSelectionDialog(AF_STR_openProjectDialogHeader, defaultFilePath, AF_STR_projectFileDetails, nullptr, false);

        if (!selectedProjectFilePath.isEmpty())
        {
            //Get the file path
            filePath = acQStringToGTString(selectedProjectFilePath);
        }
    }
    else
    {
        filePath = projectFilePath;
    }

    if (!filePath.isEmpty())
    {
        // Save the current project:
        OnFileSaveProject();

        // Set the "in load" flag to true:
        m_sIsInLoadProcess = true;

        // Close the current project:
        OnFileCloseProject(false);

        // Clear the information view (output view):
        afApplicationCommands::instance()->ClearInformationView();

        // Run the Load project command
        afLoadProjectCommand loadProjectCommand(filePath);
        bool rc = loadProjectCommand.execute();

        if (!rc)
        {
            acMessageBox::instance().critical(AF_STR_ErrorA, AF_STR_loadProjectError, QMessageBox::Ok);
        }

        // Update toolbars:
        updateToolbarCommands();
    }

    // Set the "in load" flag to true:
    m_sIsInLoadProcess = false;
}


void afApplicationCommands::OnFileCloseProject(bool shouldOpenWelcomePage)
{
    // Save the project before closing it.
    OnFileSaveProject();

    // Set an empty project as the current project:
    apProjectSettings emptyProjectSettings;
    afProjectManager::instance().setCurrentProject(emptyProjectSettings);

    // Update toolbars:
    updateToolbarCommands();

    // Set the GUI layout as the initial layout:
    afMainAppWindow::instance()->updateLayoutMode(afMainAppWindow::LayoutNoProject);

    if (shouldOpenWelcomePage)
    {
        // Open the startup page:
        OnFileOpenWelcomePage();
    }
}

// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::onUpdateProjectSave
// Description: Enable only when there is a project loaded
// Arguments:   bool& isEnabled
// Author:      Sigal Algranaty
// Date:        27/3/2012
// ---------------------------------------------------------------------------
void afApplicationCommands::onUpdateProjectSave(bool& isEnabled)
{
    // Get the current project name:
    gtString projectName = afProjectManager::instance().currentProjectSettings().projectName();
    isEnabled = !projectName.isEmpty();
}


void afApplicationCommands::onUpdateProjectClose(bool& isEnabled)
{
    // Get the current project name:
    gtString projectName = afProjectManager::instance().currentProjectSettings().projectName();
    isEnabled = !projectName.isEmpty();

    if (isEnabled)
    {
        // Enable only when no process is running:
        enableWhenNoProcess(isEnabled);
    }
}


// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::onUpdateProjectSaveAs
// Description: Enable when there is a project loaded, and we're not debugging
// Arguments:   bool& isEnabled
// Author:      Sigal Algranaty
// Date:        27/3/2012
// ---------------------------------------------------------------------------
void afApplicationCommands::onUpdateProjectSaveAs(bool& isEnabled)
{
    // Enable when there is a project loaded and we're not while debugging:
    afPluginConnectionManager& thePluginConnectionManager = afPluginConnectionManager::instance();
    onUpdateProjectSave(isEnabled);
    isEnabled = isEnabled && !(thePluginConnectionManager.getCurrentRunModeMask() & AF_DEBUGGED_PROCESS_EXISTS);
}
// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::onFileOpenFile
// Description: Is called when the user press the File -> Open file command.
// Author:      Sigal Algranaty
// Date:        10/8/2011
// ---------------------------------------------------------------------------
void afApplicationCommands::onFileOpenFile()
{
    QString selectedFilePath;
    QString fileFiltersStr = QString("%1;;%2;;%3").arg(AF_STR_sourceFileDetails).arg(AF_STR_textFileDetails).arg(AF_STR_allFileDetails);

    selectedFilePath = ShowFileSelectionDialogWithBrowseHistory(AF_STR_openFileDialogCaption, fileFiltersStr, false, AF_Str_OpenFileBrowseFolder);

    // Open the "open project" Dialog:
    if (!selectedFilePath.isEmpty())
    {
        // Open the file:
        OpenFileAtLine(acQStringToGTString(selectedFilePath), -1, false);

        // Update toolbars:
        updateToolbarCommands();
    }
}


// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::onUpdateFileSave
// Description: Check if there is currently an active source code file
// Arguments:   bool& isEnabled - true iff there is a writable file active
// Author:      Sigal Algranaty
// Date:        16/8/2011
// ---------------------------------------------------------------------------
void afApplicationCommands::onUpdateFileSave(bool& isEnabled)
{
    // Get the current application window:
    afMainAppWindow* pAfMainWindow = afMainAppWindow::instance();
    GT_IF_WITH_ASSERT(pAfMainWindow != nullptr)
    {
        // Get the active MDI window:
        afQMdiSubWindow* pActiveWindow = pAfMainWindow->activeMDISubWindow();

        if (pActiveWindow != nullptr)
        {
            if (pActiveWindow->widget() != nullptr)
            {
                gtASCIIString className(pActiveWindow->widget()->metaObject()->className());

                if (className == "afSourceCodeView")
                {
                    // Allow save for source code windows:
                    isEnabled = true;
                }
            }
        }
    }
}

void afApplicationCommands::CreateDefaultProject(const gtString& exeMode, const gtString& sessionType)
{
    // Close the current project:
    OnFileCloseProject(false);

    // Create a default empty project and set it:
    apProjectSettings projectSettings;
    projectSettings.setProjectName(acQStringToGTString(FindDefaultProjectName()));
    projectSettings.setLastActiveMode(exeMode);
    projectSettings.setLastActiveSessionType(sessionType);

    osFilePath projectFilePath;
    getProjectsFilePath(projectSettings.projectName(), projectFilePath);
    afProjectManager::instance().setCurrentProject(projectSettings);
    afProjectManager::instance().setCurrentProjectFilePath(projectFilePath);

    // Save and load the new project:
    if (afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
    {
        OnFileSaveProject();

        // Get the current executable file path:
        osFilePath executableFilePath = afProjectManager::instance().currentProjectSettings().executablePath();

        // Get the current project name:
        gtString projectName = afProjectManager::instance().currentProjectSettings().projectName();

        // Get the project path:
        osFilePath vsProjectFilePath;
        afGetVisualStudioProjectFilePath(executableFilePath, projectName, vsProjectFilePath);
        afProjectManager::instance().setCurrentProjectFilePath(vsProjectFilePath);

        OnFileOpenProject(vsProjectFilePath.asString());
    }
    else
    {
        OnFileSaveProject();
        OnFileOpenProject(projectFilePath.asString());
    }

    UpdateRecentlyUsedProjects();
}


QString afApplicationCommands::FindDefaultProjectName()
{
    QString retVal = AF_STR_newProjectEnterName;

    for (int i = 1; i < 400; i++)
    {
        // Look for a project name that doesn't yet exist:
        QString projectName = QString(AF_STR_newProjectDefaultProjectName).arg(i);

        afApplicationCommands* pCommands = afApplicationCommands::instance();
        GT_IF_WITH_ASSERT(pCommands != nullptr)
        {
            // Build the CXL file path:
            osFilePath projectFilePath;
            pCommands->getProjectsFilePath(acQStringToGTString(projectName), projectFilePath);

            if (!projectFilePath.exists())
            {
                retVal = projectName;
                break;
            }
        }
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::FillRecentlyUsedProjectsNames
// Description: Fill the input vector with the recently used project names
// Arguments:   gtVector<gtString>& projectsNames
//              gtString& currentAppName
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        8/4/2011
// ---------------------------------------------------------------------------
bool afApplicationCommands::FillRecentlyUsedProjectsNames(gtVector<gtString>& projectsNames, gtString& currentProjectPath, bool showAll)
{
    bool retVal = false;

    // Clear the vector of projects names:
    projectsNames.clear();

    // Get the the User work path directory:
    osFilePath projectPath;
    afGetUserDataFolderPath(projectPath);
    osDirectory dir(projectPath);
    bool rc1 = dir.exists();

    if (rc1)
    {
        gtList<osFilePath> filePaths;
        // Obtaining all the .cxl Applications files and putting them in filePaths
        bool rc2 = dir.getContainedFilePaths(gtString(L"*." AF_STR_projectFileExtension), osDirectory::SORT_BY_DATE_DESCENDING, filePaths);

        if (rc2)
        {
            // Get the current project file name:
            currentProjectPath = afProjectManager::instance().currentProjectFilePath().asString();

            // Let the OS know that CodeXL has loaded a new project file so it can add it to the list of recent documents associated with CodeXL.
            osAddToRecentDocs(currentProjectPath);

            // Iterate over the list and add the entires (up to MAX_NUMBER_OF_RECENT_PROJECTS_TO_SHOW)
            gtList<osFilePath>::iterator headIterator = filePaths.begin();
            gtList<osFilePath>::iterator tailIterator = filePaths.end();
            int numberOfRecentProjectsEntries = 0;
            gtString iterProjectName;

            // Iterating over the recent projects paths while not exceeding the maximal number of recent projects allowed
            while ((headIterator != tailIterator) && ((numberOfRecentProjectsEntries < AF_MAX_NUMBER_OF_RECENT_PROJECTS_TO_SHOW) || showAll))
            {
                // Extracting the file name
                (*headIterator).getFileName(iterProjectName);

                // Make sure that it is a cxl file:
                gtString fileExtension;
                (*headIterator).getFileExtension(fileExtension);

                // If this iteration is over the current working directory:
                if (fileExtension == AF_STR_projectFileExtension)
                {
                    // Inserting the full project path to _recentlyUsedProjectsNames
                    projectsNames.push_back((*headIterator).asString());
                    numberOfRecentProjectsEntries++;
                }

                headIterator++;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::onFileSaveFile
// Description: Save the current source code file
// Author:      Sigal Algranaty
// Date:        16/8/2011
// ---------------------------------------------------------------------------
void afApplicationCommands::onFileSaveFile()
{
    // Get the current application window:
    afMainAppWindow* pAfMainWindow = afMainAppWindow::instance();
    GT_IF_WITH_ASSERT(pAfMainWindow != nullptr)
    {
        // Get the active MDI window:
        afQMdiSubWindow* pActiveWindow = pAfMainWindow->activeMDISubWindow();
        GT_IF_WITH_ASSERT(pActiveWindow != nullptr)
        {
            GT_IF_WITH_ASSERT(pActiveWindow->widget() != nullptr)
            {
                gtASCIIString className(pActiveWindow->widget()->metaObject()->className());

                if (className == "afSourceCodeView")
                {
                    // Down cast the widget to a source code view:
                    afSourceCodeView* pSourceCodeView = qobject_cast<afSourceCodeView*>(pActiveWindow->widget());
                    GT_IF_WITH_ASSERT(pSourceCodeView != nullptr)
                    {
                        // Save the file:
                        pSourceCodeView->saveFile();

                        // Set the window caption to a non changed file name:
                        gtString viewCaption;
                        pSourceCodeView->filePath().getFileNameAndExtension(viewCaption);
                        afApplicationCommands::instance()->setActiveWindowCaption(viewCaption);
                    }

                    // update the document to the update mechanism:
                    afDocUpdateManager::instance().UpdateDocument(pSourceCodeView);

                    // Update the recent loaded list save file
                    UpdateRecentlyUsedProjects();
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::onFileSaveFileAs
// Description: Save the current file to a file
// Author:      Sigal Algranaty
// Date:        16/8/2011
// ---------------------------------------------------------------------------
void afApplicationCommands::onFileSaveFileAs()
{
    // Get the current application window:
    afMainAppWindow* pAfMainWindow = afMainAppWindow::instance();
    GT_IF_WITH_ASSERT(pAfMainWindow != nullptr)
    {
        // Get the active MDI window:
        afQMdiSubWindow* pActiveWindow = pAfMainWindow->activeMDISubWindow();
        GT_IF_WITH_ASSERT(pActiveWindow != nullptr)
        {
            GT_IF_WITH_ASSERT(pActiveWindow->widget() != nullptr)
            {
                gtASCIIString className(pActiveWindow->widget()->metaObject()->className());

                if (className == "afSourceCodeView")
                {
                    // Down cast the widget to a source code view:
                    afSourceCodeView* pSourceCodeView = qobject_cast<afSourceCodeView*>(pActiveWindow->widget());
                    GT_IF_WITH_ASSERT(pSourceCodeView != nullptr)
                    {
                        // Save the file:
                        gtString newFilePathStr;

                        // Open the file dialog
                        afMainAppWindow* pMainAppWindow = afMainAppWindow::instance();
                        GT_IF_WITH_ASSERT(pMainAppWindow != nullptr)
                        {
                            // Call the QFileDialog:
                            QString fileName;
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
                            QFileDialog dialog(pMainAppWindow, AF_STR_FileSaveAs, acGTStringToQString(pSourceCodeView->filePath().asString()));
                            dialog.setAcceptMode(QFileDialog::AcceptSave);

                            QFileDialog::Options options = QFileDialog::DontUseNativeDialog;
                            dialog.setOptions(options);

                            prepareDialog(dialog);

                            if (QDialog::Accepted == dialog.exec())
                            {
                                fileName = dialog.selectedFiles().value(0);
                            }

                            fileName.replace('/', '\\');
#elif AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                            osFilePath origFilePath = pSourceCodeView->filePath();
                            fileName = QFileDialog::getSaveFileName(pMainAppWindow, AF_STR_FileSaveAs, acGTStringToQString(origFilePath.asString()), "*.*");
#endif // AMDT_BUILD_TARGET

                            // Convert the QString to gtString
                            gtString newFileName = acQStringToGTString(fileName);

                            bool rc = pSourceCodeView->saveFileAs(newFileName);

                            if (rc)
                            {
                                GT_IF_WITH_ASSERT(!newFileName.isEmpty())
                                {
                                    osFilePath newFilePath(newFilePathStr);
                                    rc = OpenFileAtLine(newFilePath, 0, -1);

                                    // Update the file open last browse folder:
                                    afGlobalVariablesManager::instance().SetHistoryList(AF_Str_OpenFileBrowseFolder, QStringList(fileName));
                                    GT_ASSERT(rc);
                                }

                                // Update the recent loaded list save file
                                UpdateRecentlyUsedProjects();
                            }
                        }
                    }
                }
            }
        }
    }
}

bool afApplicationCommands::ShowQTSaveCSVFileDialog(QString& selectedFilePath, const QString& fileNamePostFix, QWidget* pParent, const QString& title)
{
    QString extension = acGTStringToQString(AF_STR_profileFileExtension4);
    QString csvFileName = acGTStringToQString(applicationRootString());
    csvFileName.append(AF_STR_HyphenA);
    csvFileName.append(afGlobalVariablesManager::ProductNameA());
    csvFileName.append(fileNamePostFix);
    csvFileName.append(".");
    csvFileName.append(extension);

    osFilePath csvFileDefaultPath(osFilePath::OS_USER_DOCUMENTS);

    bool retVal = ShowQTSaveFileDialog(selectedFilePath, fileNamePostFix, acGTStringToQString(csvFileDefaultPath.asString()), pParent, extension, title);
    return retVal;
}

bool afApplicationCommands::ShowQTSaveFileDialog(QString& selectedFilePath, const QString& defaultFileName, const QString& defaultPath, QWidget* pParent, const QString& extension, const QString& title)
{
    bool retVal = false;

    // Get project name as base for file name:

    // Create the default file path:
    osFilePath csvFileDefaultPath(acQStringToGTString(defaultPath));
    csvFileDefaultPath.setFileName(acQStringToGTString(defaultFileName));

    // Get the main window:
    GT_IF_WITH_ASSERT(pParent != nullptr)
    {
        QWidget* pDialogParent = afMainAppWindow::instance();

        if (pDialogParent == nullptr)
        {
            pDialogParent = pParent;
        }

        // Define a QString list for the QFileDialog:
        QString dialogCaption = title.isEmpty() ? AF_STR_saveDataDialogHeaderA : title;

        // Call the QFileDialog:
        QFileDialog::Options options;

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
        QFileDialog dialog(pDialogParent, dialogCaption, acGTStringToQString(csvFileDefaultPath.asString()), extension);
        dialog.setAcceptMode(QFileDialog::AcceptSave);

        options = QFileDialog::DontUseNativeDialog;
        dialog.setOptions(options);

        prepareDialog(dialog);

        if (QDialog::Accepted == dialog.exec())
        {
            selectedFilePath = dialog.selectedFiles().value(0);
        }

#elif AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        QString filter = extension;
        selectedFilePath = QFileDialog::getSaveFileName(pParent, dialogCaption, acGTStringToQString(csvFileDefaultPath.asString()), filter, &filter, options);
#endif // AMDT_BUILD_TARGET

        // If the user pressed the "OK" button:
        if (!selectedFilePath.isEmpty())
        {
            QString nameWithExtension = QString("." + extension);

            if (selectedFilePath.contains(nameWithExtension) == 0)
            {
                if (selectedFilePath.endsWith(".") == false)
                {
                    selectedFilePath.append(".");
                }

                selectedFilePath.append(extension);
            }

            retVal = true;
        }
    }

    return retVal;

}

// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::getProjectsFilePath
// Description: Get a project file path
// Arguments:    const gtString& projectName
//              osFilePath& projectFilePath
// Author:      Sigal Algranaty
// Date:        8/4/2012
// ---------------------------------------------------------------------------
void afApplicationCommands::getProjectsFilePath(const gtString& projectName, osFilePath& projectFilePath)
{
    // Build the file path for the project:
    // Get the the User AppData directory
    afGetUserDataFolderPath(projectFilePath);
    projectFilePath.setFileName(projectName);
    projectFilePath.setFileExtension(AF_STR_projectFileExtension);
}

// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::updateProjectSettingsFromImplementation
// Description: Updates the project settings with implementation-specific data
// Author:      Uri Shomroni
// Date:        21/5/2012
// ---------------------------------------------------------------------------
void afApplicationCommands::updateProjectSettingsFromImplementation()
{

}

// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::OnFileRecentProject
// Description: Load the project in projectIndex
// Arguments:   gtVector<gtString>& projectsFilePaths
//              int projectIndex
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        25/7/2011
// ---------------------------------------------------------------------------
bool afApplicationCommands::OnFileRecentProject(gtVector<gtString>& projectsFilePaths, int projectIndex)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT((projectIndex >= 0) && (projectIndex < (int)projectsFilePaths.size()))
    {
        // Get the chosen entry corresponding full path (we give recentProjectsMenu the entry ID)
        gtString projectsFilePath = projectsFilePaths[projectIndex];

        // Open the project:
        OnFileOpenProject(projectsFilePath);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::UpdateRecentlyUsedProjects
// Description: Update the recently used projects
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        14/8/2011
// ---------------------------------------------------------------------------
bool afApplicationCommands::UpdateRecentlyUsedProjects()
{
    bool retVal = false;

    // Update the recently used projects names:
    retVal = afProjectManager::instance().UpdateRecentlyUsedProjects();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::OnFileOpenStartupDialog
// Description: Open the HTML startup page
// Author:      Sigal Algranaty
// Date:        26/2/2012
// ---------------------------------------------------------------------------
void afApplicationCommands::OnFileOpenWelcomePage()
{
    // Get the main application window:
    afMainAppWindow* pMainApplicationWindow = afMainAppWindow::instance();
    GT_IF_WITH_ASSERT(pMainApplicationWindow != nullptr)
    {
        pMainApplicationWindow->openStartupdialog();
    }
}

// ---------------------------------------------------------------------------
void afApplicationCommands::updateToolbarCommands()
{
    afMainAppWindow* pMainWindow = afMainAppWindow::instance();

    if (nullptr != pMainWindow)
    {
        pMainWindow->updateToolbarsCommands();
    }
}

void afApplicationCommands::OnProjectSettingsEditRemoteHost()
{
    // Show the debug settings dialog:
    afNewProjectDialog::afDialogMode dialogMode = afNewProjectDialog::AF_DIALOG_EDIT_PROJECT;

    // In case of no project, open the dialog in "New Project" mode:
    GT_IF_WITH_ASSERT(!afProjectManager::instance().currentProjectSettings().projectName().isEmpty())
    {
        afNewProjectDialog::instance().ShowDialog(dialogMode, AF_globalSettingsGeneralHeaderUnicode, L"", afNewProjectDialog::AF_FOCUS_REMOTE_HOST);
    }

    // Save the project file:
    OnFileSaveProject();
}


// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::OnProjectSettings
// Description: Edit project settings
// Author:      Sigal Algranaty
// Date:        11/4/2012
// ---------------------------------------------------------------------------
void afApplicationCommands::OnProjectSettings(const gtString& projectSettingsPath)
{
    // Show the debug settings dialog:
    afNewProjectDialog::afDialogMode dialogMode = afNewProjectDialog::AF_DIALOG_EDIT_PROJECT;

    // In case of no project, open the dialog in "New Project" mode:
    if (afProjectManager::instance().currentProjectSettings().projectName().isEmpty())
    {
        dialogMode = afNewProjectDialog::AF_DIALOG_NEW_PROJECT;
    }

    afNewProjectDialog::instance().ShowDialog(dialogMode, projectSettingsPath);

    // Save the project file:
    OnFileSaveProject();
}

// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::onViewResetGUILayout
// Description: Execute reset GUI layout command
// Author:      Sigal Algranaty
// Date:        11/4/2012
// ---------------------------------------------------------------------------
void afApplicationCommands::onViewResetGUILayout()
{
    afMainAppWindow* pMainWindow = afMainAppWindow::instance();

    GT_IF_WITH_ASSERT(pMainWindow != nullptr)
    {
        pMainWindow->resetInitialLayout();
    }
}


// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::bringDockToFront
// Description: bring a dock to front ensure it is visible
// Author:      Gilad Yarnitzky
// Date:        21/8/2013
// ---------------------------------------------------------------------------
void afApplicationCommands::bringDockToFront(const gtString& dockName)
{
    afMainAppWindow* pMainWindow = afMainAppWindow::instance();

    GT_IF_WITH_ASSERT(pMainWindow != nullptr)
    {
        pMainWindow->bringDockToFront(dockName);
    }
}

// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::onUpdateDebugSettings
// Description: Update of the debug settings command
// Arguments:   bool &isEnabled
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
void afApplicationCommands::onUpdateDebugSettings(bool& isEnabled)
{
    // Action is enabled if there is no debugged process running:
    isEnabled = !(afPluginConnectionManager::instance().getCurrentRunModeMask() & AF_DEBUGGED_PROCESS_EXISTS);
}

// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::onHelpAbout
// Description: Is handling the help about command
// Author:      Sigal Algranaty
// Date:        24/7/2011
// ---------------------------------------------------------------------------
void afApplicationCommands::onHelpAbout()
{
    // Load the help-about dialog:
    afMainAppWindow* pMainApplicationWindow = afMainAppWindow::instance();
    afHelpAboutDialog dialog(OS_STANDALONE_APPLICATION_TYPE, pMainApplicationWindow->topLevelWidget());
    dialog.exec();
}

// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::onHelpAbout
// Description: Opens the CodeXL website
// Author:      Uri Shomroni
// Date:        13/3/2012
// ---------------------------------------------------------------------------
void afApplicationCommands::openCodeXLWebsite()
{
    QString CodeXLWebsiteAddress = QString::fromWCharArray(AF_STR_HelpCodeXLWebPage);
    QUrl CodeXLWebsite(CodeXLWebsiteAddress);
    QDesktopServices::openUrl(CodeXLWebsite);
}

// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::onHelpQuickStart
// Description: Is handling the help tutorial command
// Author:      Sigal Algranaty
// Date:        24/7/2011
// ---------------------------------------------------------------------------
void afApplicationCommands::onHelpQuickStart()
{
    // Get the tutorial file:
    osFilePath CodeXLTutorialPath;
    bool rc = CodeXLTutorialPath.SetInstallRelatedPath(osFilePath::OS_CODEXL_TUTORIAL_FILE);
    GT_IF_WITH_ASSERT_EX(rc, L"Could not find tutorial file")
    {
        // Open the tutorial file:
        osFileLauncher fileLauncher(CodeXLTutorialPath.asString());
        rc = fileLauncher.launchFile();
    }

    if (!rc)
    {
        acMessageBox::instance().critical(AF_STR_ErrorA, AF_STR_TutorialFileLoadErrorMessage, QMessageBox::Ok);
    }
}

// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::onHelpUserGuide
// Description: Is handling the help user guide command
// Author:      Sigal Algranaty
// Date:        24/7/2011
// ---------------------------------------------------------------------------
void afApplicationCommands::onHelpUserGuide()
{
    // Get the tutorial file:
    osFilePath CodeXLHelpPath;
    bool rc = CodeXLHelpPath.SetInstallRelatedPath(osFilePath::OS_CODEXL_HELP_FILE);
    GT_IF_WITH_ASSERT_EX(rc, L"Could not find user guide file")
    {
        // Open the tutorial file:
        osFileLauncher fileLauncher(CodeXLHelpPath.asString());
        rc = fileLauncher.launchFile();
    }

    if (!rc)
    {
        acMessageBox::instance().critical(AF_STR_ErrorA, AF_STR_HelpFileLoadErrorMessage, QMessageBox::Ok);
    }
}


// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::onHelpUpdates
// Description: Is handling the help updates command
// Author:      Sigal Algranaty
// Date:        24/7/2011
// ---------------------------------------------------------------------------
void afApplicationCommands::onHelpUpdates()
{
    // Call the check for updates command, which will raise the dialog.
    afSoftwareUpdaterWindow dlg;
    dlg.displayDialog();
}


// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::onHelpOpenURL
// Description: Is handling the help forum command
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        24/7/2011
// ---------------------------------------------------------------------------
void afApplicationCommands::onHelpOpenURL(const gtString& urlStr)
{
    osFileLauncher fileLauncher(urlStr);
    fileLauncher.launchFile();
}

QString afApplicationCommands::ShowFileSelectionDialog(const QString& dialogCaption, QString& defaultFileFullPath, const QString& fileFilters, afBrowseAction* pBrowseAction, bool saveFile)
{
    QString retVal;

    GT_IF_WITH_ASSERT(!fileFilters.isEmpty())
    {
        // Get the main window:
        afMainAppWindow* pMainAppWindow = afMainAppWindow::instance();

        // If there is not selected executable, get the latest browsed exe folder for the user convenience:
        if (defaultFileFullPath.isEmpty() && (pBrowseAction != nullptr))
        {
            defaultFileFullPath = pBrowseAction->LastBrowsedFolder();
        }
        else if (!defaultFileFullPath.isEmpty() && (saveFile == true) && (pBrowseAction != nullptr) && !pBrowseAction->LastBrowsedFolder().isEmpty())
        {
            osFilePath lastBrowsedFilePath;
            lastBrowsedFilePath.setFileDirectory(acQStringToGTString(pBrowseAction->LastBrowsedFolder()));
            QString fileName;
            QString fileExtension;
            fileExtension = defaultFileFullPath.section('.', -1);
            fileName = defaultFileFullPath.section('.', 0, 0);
            lastBrowsedFilePath.setFileName(acQStringToGTString(fileName));
            lastBrowsedFilePath.setFileExtension(acQStringToGTString(fileExtension));
            defaultFileFullPath = acGTStringToQString(lastBrowsedFilePath.asString());
        }

        // Get the first filter as the selected one:
        QStringList filtersList = fileFilters.split(";");
        QString selectedFilter;

        if (!fileFilters.isEmpty())
        {
            selectedFilter = filtersList.first();
        }

        // Call the QFileDialog:
        QFileDialog::Options options;
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
        QFileDialog dialog(pMainAppWindow, dialogCaption, defaultFileFullPath, fileFilters);

        if (saveFile)
        {
            dialog.setFileMode(QFileDialog::AnyFile);
            dialog.setAcceptMode(QFileDialog::AcceptSave);
        }
        else
        {
            dialog.setFileMode(QFileDialog::ExistingFile);
        }

        options = QFileDialog::DontUseNativeDialog;
        dialog.setOptions(options);

        prepareDialog(dialog);

        dialog.selectNameFilter(fileFilters);

        if (QDialog::Accepted == dialog.exec())
        {
            retVal = dialog.selectedFiles().value(0);
        }

        retVal.replace('\\', '/');
#elif AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

        // On Windows, this uses private functions qt_win_get_save_file_name and qt_win_get_open_file_name. Using the above code causes Linux-style dialogs to appear:
        if (saveFile)
        {
            // Call the QFileDialog:
            retVal = QFileDialog::getSaveFileName(pMainAppWindow, dialogCaption, defaultFileFullPath, fileFilters, &selectedFilter, options);
        }
        else
        {
            // Call the QFileDialog:
            retVal = QFileDialog::getOpenFileName(pMainAppWindow, dialogCaption, defaultFileFullPath, fileFilters, &selectedFilter, options);
        }

        retVal.replace('/', '\\');
#endif // AMDT_BUILD_TARGET

        // If the user pressed the "OK" button:
        if (!retVal.isEmpty())
        {
            if (pBrowseAction != nullptr)
            {
                // Set the button last browsed folder:
                QFileInfo fileInfo = retVal;
                QString lastBrowseFolder = fileInfo.absoluteDir().path();
                pBrowseAction->SetLastBrowsedFolder(lastBrowseFolder);
            }
        }
    }

    return retVal;
}

QString afApplicationCommands::ShowFileSelectionDialogWithBrowseHistory(const QString& dialogCaption, const QString& fileFilters, bool saveFile, const QString& fileBrowseLocationFieldID)
{
    QString retVal;

    GT_IF_WITH_ASSERT(!fileFilters.isEmpty())
    {
        // Get the main window:
        afMainAppWindow* pMainAppWindow = afMainAppWindow::instance();

        // Find the default file location:
        QString defaultFileLocation = afGlobalVariablesManager::instance().GetLastBrowseLocation(fileBrowseLocationFieldID);

        // Get the first filter as the selected one:
        QStringList filtersList = fileFilters.split(";");
        QString selectedFilter;

        if (!fileFilters.isEmpty())
        {
            selectedFilter = filtersList.first();
        }

        // Call the QFileDialog:
        QFileDialog::Options options;
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
        QFileDialog dialog(pMainAppWindow, dialogCaption, defaultFileLocation, fileFilters);

        if (saveFile)
        {
            dialog.setFileMode(QFileDialog::AnyFile);
            dialog.setAcceptMode(QFileDialog::AcceptSave);
        }
        else
        {
            dialog.setFileMode(QFileDialog::ExistingFile);
        }

        options = QFileDialog::DontUseNativeDialog;
        dialog.setOptions(options);

        prepareDialog(dialog);

        dialog.selectNameFilter(fileFilters);

        if (QDialog::Accepted == dialog.exec())
        {
            retVal = dialog.selectedFiles().value(0);
        }

        retVal.replace('\\', '/');
#elif AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

        // On Windows, this uses private functions qt_win_get_save_file_name and qt_win_get_open_file_name. Using the above code causes Linux-style dialogs to appear:
        if (saveFile)
        {
            // Call the QFileDialog:
            retVal = QFileDialog::getSaveFileName(pMainAppWindow, dialogCaption, defaultFileLocation, fileFilters, &selectedFilter, options);
        }
        else
        {
            // Call the QFileDialog:
            retVal = QFileDialog::getOpenFileName(pMainAppWindow, dialogCaption, defaultFileLocation, fileFilters, &selectedFilter, options);
        }

#endif // AMDT_BUILD_TARGET

        // If the user pressed the "OK" button:
        if (!retVal.isEmpty())
        {
            if (!defaultFileLocation.isEmpty())
            {
                // Set the button last browsed folder:
                afGlobalVariablesManager::instance().SetLastBrowseLocation(defaultFileLocation, fileBrowseLocationFieldID);
            }
        }
    }

    return retVal;
}

bool afApplicationCommands::ShowMultipleFilesSelectionDialog(gtVector<osFilePath>& selectedFilesVector, const QString& dialogCaption, const QString& defaultFileFullPath, const QString& fileFilters)
{
    bool retVal = false;

    QStringList selectedFilesStrings;

    // Get the main window:
    afMainAppWindow* pMainAppWindow = afMainAppWindow::instance();

    // Call the QFileDialog:
    QFileDialog::Options options;
    QString fileName;
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    QFileDialog dialog(pMainAppWindow, dialogCaption, defaultFileFullPath, fileFilters);

    dialog.setFileMode(QFileDialog::ExistingFile);

    options = QFileDialog::DontUseNativeDialog;
    dialog.setOptions(options);

    prepareDialog(dialog);

    QStringList fileFiltersList = fileFilters.split(";");
    dialog.selectNameFilter(fileFiltersList[0]);
    dialog.setNameFilters(fileFiltersList);

    if (QDialog::Accepted == dialog.exec())
    {
        selectedFilesStrings = dialog.selectedFiles();
    }

#elif AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    // Call the QFileDialog:
    selectedFilesStrings = QFileDialog::getOpenFileNames(pMainAppWindow, dialogCaption, defaultFileFullPath, fileFilters, nullptr, options);

#endif // AMDT_BUILD_TARGET

    // If the user pressed the "OK" button:
    if (!selectedFilesStrings.isEmpty())
    {
        foreach (QString str, selectedFilesStrings)
        {
            osFilePath selectedFilePath;
            gtString fileStr;
            acWideQStringToGTString(str, fileStr);
            selectedFilePath.setFullPathFromString(fileStr);
            selectedFilesVector.push_back(selectedFilePath);
        }

        retVal = true;
    }

    return retVal;
}

QString afApplicationCommands::ShowFolderSelectionDialog(const QString& dialogCaption, QString& defaultFolder, afBrowseAction* pBrowseAction)
{
    QString retVal;
    // Get the main window:
    afMainAppWindow* pMainAppWindow = afMainAppWindow::instance();

    if (defaultFolder.isEmpty() && (pBrowseAction != nullptr))
    {
        defaultFolder = pBrowseAction->LastBrowsedFolder();
    }

    // Call the QFileDialog:
    QFileDialog::Options options;
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    QFileDialog dialog(pMainAppWindow, dialogCaption, defaultFolder);

    options = QFileDialog::DontUseNativeDialog | QFileDialog::ShowDirsOnly;
    dialog.setOptions(options);

    prepareDialog(dialog);

    dialog.setFileMode(QFileDialog::Directory);

    if (QDialog::Accepted == dialog.exec())
    {
        retVal = dialog.selectedFiles().value(0);
    }

#elif AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    retVal = QFileDialog::getExistingDirectory(pMainAppWindow, dialogCaption, defaultFolder);
#endif // AMDT_BUILD_TARGET

    // If the user pressed the "OK" button:
    if (!retVal.isEmpty())
    {
        if (pBrowseAction != nullptr)
        {
            pBrowseAction->SetLastBrowsedFolder(retVal);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::enableWhenNoProcess
// Description: Enable / Disable commands that are enabled when process does not exist
// Arguments:   bool& isEnabled
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
void afApplicationCommands::enableWhenNoProcess(bool& isEnabled) const
{
    // Check if there is already a debugged process running:
    bool debuggedProcessExist = (0 == (afPluginConnectionManager::instance().getCurrentRunModeMask() & AF_DEBUGGED_PROCESS_EXISTS));
    bool analyzeCurrentlyBuilding = (0 == (afPluginConnectionManager::instance().getCurrentRunModeMask() & AF_ANALYZE_CURRENTLY_BUILDING));
    bool analyzeCurrentlyExporting = (0 == (afPluginConnectionManager::instance().getCurrentRunModeMask() & AF_FRAME_ANALYZE_CURRENTLY_EXPORTING));
    bool analyzeCurrentlyImporting = (0 == (afPluginConnectionManager::instance().getCurrentRunModeMask() & AF_FRAME_ANALYZE_CURRENTLY_IMPORTING));
    isEnabled = (debuggedProcessExist && analyzeCurrentlyBuilding && analyzeCurrentlyExporting && analyzeCurrentlyImporting);
}

// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::showModal
// Description: base implementation of virtual function which execs the dialog
// Arguments:   QDialog* pDialog
// Return Val:  int
// Author:      Yoni Rabin
// Date:        31/5/2012
// ---------------------------------------------------------------------------
int afApplicationCommands::showModal(QDialog* pDialog)
{
    int rc = 0;
    GT_IF_WITH_ASSERT(pDialog != nullptr)
    {
        rc = pDialog->exec();
    }
    return rc;
}

// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::setActiveWindowCaption
// Description: Set the active MDI window caption
// Author:      Sigal Algranaty
// Date:        16/8/2011
// ---------------------------------------------------------------------------
void afApplicationCommands::setActiveWindowCaption(const gtString& caption)
{
    // Get the current application window:
    afMainAppWindow* pAfMainWindow = afMainAppWindow::instance();
    GT_IF_WITH_ASSERT(pAfMainWindow != nullptr)
    {
        // Get the active MDI window:
        afQMdiSubWindow* pActiveWindow = pAfMainWindow->activeMDISubWindow();

        if (pActiveWindow != nullptr)
        {
            // Sanity check:
            GT_IF_WITH_ASSERT(pActiveWindow->widget() != nullptr)
            {
                // Set the window title:
                pActiveWindow->widget()->setWindowTitle(acGTStringToQString(caption));
            }
        }
    }
}

void afApplicationCommands::MarkMDIWindowAsChanged(const osFilePath& mdiFilePath, bool isChanged)
{
    // Get the current application window:
    afMainAppWindow* pAfMainWindow = afMainAppWindow::instance();
    GT_IF_WITH_ASSERT(pAfMainWindow != nullptr)
    {
        // Get the active MDI window:
        afQMdiSubWindow* pActiveWindow = pAfMainWindow->findMDISubWindow(mdiFilePath);

        if (pActiveWindow != nullptr)
        {
            // Sanity check:
            GT_IF_WITH_ASSERT(pActiveWindow->widget() != nullptr)
            {
                gtString currentCaption = acQStringToGTString(pActiveWindow->widget()->windowTitle());

                if (currentCaption.isEmpty())
                {
                    currentCaption = acQStringToGTString(pActiveWindow->windowTitle());
                }

                int changedPos = currentCaption.reverseFind(AF_STR_FileChangedMarker);

                if (isChanged)
                {
                    if (-1 == changedPos)
                    {
                        currentCaption += AF_STR_FileChangedMarker;
                    }
                }
                else
                {
                    if (-1 != changedPos)
                    {
                        currentCaption.replace(AF_STR_FileChangedMarker, L"");
                    }
                }

                // Set the window title:
                pActiveWindow->widget()->setWindowTitle(acGTStringToQString(currentCaption));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::setWindowCaption
// Description: Set a Qt window caption
// Arguments:   QWidget* pWindow
//              const gtString& windowCaption
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        23/1/2012
// ---------------------------------------------------------------------------
bool afApplicationCommands::setWindowCaption(QWidget* pWindow, const gtString& windowCaption)
{
    bool retVal = false;

    // Sanity check
    GT_IF_WITH_ASSERT(pWindow != nullptr)
    {
        // Get the window parent:
        QObject* pParent = pWindow->parent();

        if (pParent != nullptr)
        {
            // Get the parent as dock widget:
            QDockWidget* pDockWidget = qobject_cast<QDockWidget*>(pParent);

            while ((nullptr == pDockWidget) && (nullptr != pParent))
            {
                pParent = pParent->parent();
                pDockWidget = qobject_cast<QDockWidget*>(pParent);
            }

            GT_IF_WITH_ASSERT(pDockWidget != nullptr)
            {
                pDockWidget->setWindowTitle(acGTStringToQString(windowCaption));
                pWindow->setWindowTitle(acGTStringToQString(windowCaption));
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::setApplicationCaption
// Description: Set the application caption
// Arguments:   const gtString& caption
// Author:      Sigal Algranaty
// Date:        2/8/2011
// ---------------------------------------------------------------------------
void afApplicationCommands::setApplicationCaption(const gtString& caption)
{
    // Get the main application window:
    afMainAppWindow* pApplicationWindow = afMainAppWindow::instance();

    if (pApplicationWindow != nullptr)
    {
        // Get the state variables view from the main frame:
        pApplicationWindow->setWindowTitle(acGTStringToQString(caption));
    }
}


// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::promptForExit
// Description: Prompts the user for exit
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        14/3/2012
// ---------------------------------------------------------------------------
bool afApplicationCommands::promptForExit()
{
    bool retVal = true;

    // If the user is in the middle of debugging:
    afPluginConnectionManager& thePluginConnectionManager = afPluginConnectionManager::instance();
    afRunModes mode = thePluginConnectionManager.getCurrentRunModeMask();

    if ((AF_DEBUGGED_PROCESS_EXISTS == (mode & AF_DEBUGGED_PROCESS_EXISTS)) || ((mode & AF_FRAME_ANALYZE_CONNECTING) != 0))
    {
        // Get the current action:
        afIExecutionMode* pExecMode = afExecutionModeManager::instance().activeMode();
        GT_IF_WITH_ASSERT(nullptr != pExecMode)
        {
            gtString currentAction = pExecMode->modeActionString();

            // Ask the user if he wants to exit:
            QString msg = QString(AF_STR_beforeExitQuestion).arg(acGTStringToQString(currentAction));
            int userAnswer = acMessageBox::instance().warning(afGlobalVariablesManager::ProductNameA(), msg, QMessageBox::Yes | QMessageBox::No);

            if (userAnswer == QMessageBox::Yes)
            {
                // Terminate the debugged process:
                thePluginConnectionManager.stopCurrentRun(true);

                // Save the project before exit:
                OnFileSaveProject();

                // User selected to exit:
                retVal = true;
            }
            else
            {
                retVal = false;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::onToolsSystemInfo
// Description: Handling the Tools->System Information command
// Author:      Sigal Algranaty
// Date:        24/7/2011
// ---------------------------------------------------------------------------
void afApplicationCommands::onToolsSystemInfo(afSystemInformationDialog::InformationTabs selectedTab)
{
    afSystemInformationDialog dialog(nullptr, selectedTab);
    dialog.exec();
}


// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::onToolsOptions
// Description: Handling the Tools->Options command
// Author:      Sigal Algranaty
// Date:        24/7/2011
// ---------------------------------------------------------------------------
void afApplicationCommands::onToolsOptions(const gtString& openingTab)
{
    afGlobalSettingsDialog::instance().showDialog(openingTab);
}



// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::onFileExit
// Description:
// Author:      Sigal Algranaty
// Date:        20/7/2011
// ---------------------------------------------------------------------------
void afApplicationCommands::onFileExit()
{
    // Check if the user wants to exit:
    bool shouldExit = promptForExit();

    // If the user is in the middle of debugging:
    if (shouldExit)
    {
        // Get the main application window:
        afMainAppWindow* pMainAppWindow = afMainAppWindow::instance();
        GT_IF_WITH_ASSERT(pMainAppWindow != nullptr)
        {
            // Do not prompt before exit:
            pMainAppWindow->setExitingWindow(true);
            pMainAppWindow->close();
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::propertiesView
// Description: Get the application properties view
// Return Val:  afPropertiesView*
// Author:      Sigal Algranaty
// Date:        7/5/2012
// ---------------------------------------------------------------------------
afPropertiesView* afApplicationCommands::propertiesView()
{
    afPropertiesView* pRetVal = afGeneralViewsCreator::propertiesView();
    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::clearInformationView
// Description: Clear the information view
// Author:      Gilad Yarnitzky
// Date:        21/11/2013
// ---------------------------------------------------------------------------
void afApplicationCommands::ClearInformationView()
{
    afInformationView* pInfoView = afGeneralViewsCreator::informationView();

    if (nullptr != pInfoView)
    {
        pInfoView->clearOutputWindow();
    }
}

// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::addStringToInformationView
// Description: message to display in the information view
// Author:      Gilad Yarnitzky
// Date:        21/11/2013
// ---------------------------------------------------------------------------
void afApplicationCommands::AddStringToInformationView(const QString& messageToDisplay)
{
    afInformationView* pInfoView = afGeneralViewsCreator::informationView();

    if (nullptr != pInfoView)
    {
        pInfoView->appendToOutputWindow(messageToDisplay);
    }
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        applicationTree
/// \brief Description: Get the application tree
/// \return afApplicationTree*
/// -----------------------------------------------------------------------------------------------
afApplicationTree* afApplicationCommands::applicationTree()
{
    afApplicationTree* pRetVal = afGeneralViewsCreator::applicationTree();
    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::shouldReportClientApplicationCrash
// Description: Returns true if the call stack indicates we should report the crash
//              to the CodeXL crash report server.
// Author:      Uri Shomroni
// Date:        8/11/2011
// ---------------------------------------------------------------------------
bool afApplicationCommands::shouldReportClientApplicationCrash(const osCallStack& clientAppCrashStack)
{
    GT_UNREFERENCED_PARAMETER(clientAppCrashStack);

    // The standalone application contains only our code, so we should always report crashes:
    return true;
}

// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::writeTeapotSampleCXL
// Description: Write the teapot XML
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        23/7/2012
// ---------------------------------------------------------------------------
bool afApplicationCommands::WriteSampleCXL(afCodeXLSampleID sampleId)
{
    bool retVal = false;

    // Get the properties for the sample according to the sample id
    gtString sampleName, sampleMode, sampleSessionType, sampleDirName, sampleBinaryName, sampleProjectName, buildOptions;
    GetSampleProperties(sampleId, sampleName, sampleMode, sampleSessionType, sampleDirName, sampleBinaryName, sampleProjectName, buildOptions);

    osFilePath samplePath;
    retVal = samplePath.SetInstallRelatedPath(osFilePath::OS_CODEXL_EXAMPLES_PATH, false);
    GT_IF_WITH_ASSERT(retVal)
    {
        // Find the exe path and res path (in Linux it is different then windows and the string const reflects that):
        samplePath.appendSubDirectory(sampleDirName);

        // Get the source file directories for the sample
        gtString sourceFileDirectories;
        GetSampleSourceFileDirectories(sampleId, samplePath, sourceFileDirectories);

        samplePath.appendSubDirectory(AF_STR_CODEXLExampleReleaseDirName);
        apProjectSettings sampleProjectSettings;

        gtString exePathStr, workDirPathStr;

        osDirectory clFilesDirectory;
        osFilePath clFilesPath = samplePath;
        clFilesPath.appendSubDirectory(AF_STR_CodeXLSampleResourcesDirName);
        clFilesPath.getFileDirectory(clFilesDirectory);

        samplePath.setFileName(sampleBinaryName);
        samplePath.setFileExtension(AF_STR_CodeXLSampleBinaryExtension);

        workDirPathStr = samplePath.fileDirectoryAsString();

        // Create the include path:
        gtString clFilePathStr = clFilesPath.asString();
        gtString fullAppFilePath;
        fullAppFilePath.appendFormattedString(L"\"%ls\"", clFilePathStr.asCharArray());
        buildOptions.replace(L"REPLACE_INCLUDE", fullAppFilePath);
        // Add log message for the sample file path:
        OS_OUTPUT_DEBUG_LOG(samplePath.asString().asCharArray(), OS_DEBUG_LOG_INFO);

        // Make sure file exist:
        GT_IF_WITH_ASSERT(samplePath.exists())
        {
            gtString shaderCompileType = AF_Str_optionsDialogD3DCompileType;

            exePathStr = samplePath.asString();

            if (!afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
            {
                sampleProjectSettings.setProjectName(sampleProjectName);
                sampleProjectSettings.setExecutablePath(exePathStr);
                sampleProjectSettings.setWorkDirectoryFromString(workDirPathStr);
            }

            sampleProjectSettings.SetSourceFilesDirectories(sourceFileDirectories);
            sampleProjectSettings.setLastActiveMode(sampleMode);
            sampleProjectSettings.setLastActiveSessionType(sampleSessionType);

            // Build the KA extension strings, with the builds options, compiler type and cl files list:
            gtString kaString = AF_STR_sample_ka_settings;
            kaString.replace(L"REPLACE_ME_WITH_BUILD_OPTIONS", buildOptions);
            kaString.replace(L"REPLACE_ME_WITH_SHADER_COMPILE_TYPE", shaderCompileType);
            BuildSampleCLFilesSection(sampleId, clFilesDirectory, kaString);

            // Build the vector of extension strings:
            gtVector<gtString> extensionsSettings;
            extensionsSettings.push_back(AF_STR_sample_gd_settings);
            extensionsSettings.push_back(AF_STR_sample_gpu_settings1);
            extensionsSettings.push_back(AF_STR_sample_gpu_settings2);
            extensionsSettings.push_back(kaString);

            // Create the teapot project file path:
            osFilePath sampleCXLFile;

            if (afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
            {
                // The sample name in VS uses the vcxproj name
                if (sampleId == AF_TEAPOT_SAMPLE)
                {
                    sampleName = AF_STR_TeapotSampleVSProjectName;
                }
                else if (sampleId == AF_MATMUL_SAMPLE)
                {
                    sampleName = AF_STR_CodeXLMatMulExampleBinaryName;
                }
                else if (sampleId == AF_D3D12MULTITHREADING_SAMPLE)
                {
                    sampleName = AF_STR_D3D12MultithreadingSampleProjectName;
                }

                // Get the sample CXL file path:
                afGetUserDataFolderPath(sampleCXLFile);
                sampleCXLFile.setFileName(sampleName);
                sampleCXLFile.setFileExtension(AF_STR_visualStudioProjectFileExtension);
            }
            else
            {
                // Get the teapot CXL file path:
                getProjectsFilePath(sampleName, sampleCXLFile);
            }

            // Run the save project command with the sample project settings:
            afSaveProjectCommand command(sampleCXLFile);
            command.SetProjectSettings(&sampleProjectSettings, extensionsSettings);
            retVal = command.execute();
        }
    }

    return retVal;
}

void afApplicationCommands::GetSampleProperties(afCodeXLSampleID sampleId, gtString& sampleName, gtString& sampleMode, gtString& sampleSessionType,
                                                gtString& sampleDirName, gtString& sampleBinaryName, gtString& sampleProjectName, gtString& buildOptions)
{
    switch (sampleId)
    {
        case AF_TEAPOT_SAMPLE:
        {
            sampleName = AF_STR_TeapotSampleProjectName;
            sampleDirName = OS_STR_CodeXLTeapotExampleDirName;
            sampleBinaryName = AF_STR_CodeXLTeapotExampleBinaryName;
            sampleProjectName = AF_STR_CodeXLTeapotExampleProjectName;
            buildOptions = AF_STR_TeapotBuildOptions;
            sampleMode = AF_STR_CodeXLTeapotLastMode;
            sampleSessionType = AF_STR_CodeXLTeapotLastMode;
        }
        break;

        case AF_MATMUL_SAMPLE:
        {
            sampleName = AF_STR_MatMulSampleProjectName;
            sampleDirName = OS_STR_CodeXLMatMulExampleDirName;
            sampleBinaryName = AF_STR_CodeXLMatMulExampleBinaryName;
            sampleProjectName = AF_STR_CodeXLMatMulExampleProjectName;
            buildOptions = AF_STR_Empty;
            sampleMode = AF_STR_CodeXLMatMulLastMode;
            sampleSessionType = AF_STR_CodeXLMatMulLastSessionType;
        }
        break;

        case AF_D3D12MULTITHREADING_SAMPLE:
        {
            sampleName = AF_STR_D3D12MultithreadingSampleProjectName;
            sampleDirName = OS_STR_CodeXLD3D12MultithreadingExampleDirName;
            sampleBinaryName = AF_STR_CodeXLD3D12MultithreadingExampleBinaryName;
            sampleProjectName = AF_STR_D3D12MultithreadingSampleProjectName;
            buildOptions = AF_STR_Empty;
            sampleMode = AF_STR_CodeXLD3DMTLastMode;
            sampleSessionType = AF_STR_CodeXLD3DMTLastMode;
        }
        break;

        default:
            break;
    }
}

void afApplicationCommands::GetSampleSourceFileDirectories(afCodeXLSampleID sampleId, const osFilePath& samplePath, gtString& sourceFileDirectories)
{
    switch (sampleId)
    {
        case AF_TEAPOT_SAMPLE:
        {
            // First add the cl files paths
            osDirectory clFilesDirectory;
            osFilePath clFilesPath = samplePath;
            clFilesPath.appendSubDirectory(AF_STR_CodeXLSampleResourcesDirName);
            sourceFileDirectories.append(clFilesPath.fileDirectoryAsString());
            sourceFileDirectories.append(AF_STR_Semicolon);

            // Now, we should add Teapot\AMDTTeapot, Teapot\AMDTTeapot\src, Teapot\AMDTTeapotLib
            osFilePath sourceFilePath = samplePath;
            sourceFilePath.appendSubDirectory(AF_STR_CodeXLTeapotExampleSourceFileFolder1);
            sourceFileDirectories.append(sourceFilePath.fileDirectoryAsString());
            sourceFileDirectories.append(AF_STR_Semicolon);
            sourceFilePath.appendSubDirectory(AF_STR_CodeXLTeapotExampleSrcFolder);
            sourceFileDirectories.append(sourceFilePath.fileDirectoryAsString());
            sourceFileDirectories.append(AF_STR_Semicolon);
            sourceFilePath = samplePath;
            sourceFilePath.appendSubDirectory(AF_STR_CodeXLTeapotExampleSourceFileFolder2);
            sourceFileDirectories.append(sourceFilePath.fileDirectoryAsString());
            sourceFileDirectories.append(AF_STR_Semicolon);
        }
        break;

        case AF_MATMUL_SAMPLE:
        case AF_D3D12MULTITHREADING_SAMPLE:
        {
            // In both these samples, the source is located in the sample root
            osFilePath sourceFilePath = samplePath;
            sourceFileDirectories.append(sourceFilePath.fileDirectoryAsString());
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS

            if (AF_MATMUL_SAMPLE == sampleId)
            {
                sourceFileDirectories.append(osFilePath::osPathSeparator);
                sourceFileDirectories.append(AF_STR_CodeXLMatMulSourceFolderName);
            }

#endif
            sourceFileDirectories.append(AF_STR_Semicolon);
        }
        break;

        case AF_SAMPLE_NONE:
        default:
            break;
    }
}

// ---------------------------------------------------------------------------
void afApplicationCommands::BuildSampleCLFilesSection(const afCodeXLSampleID sampleId, const osDirectory& resDirectory, gtString& cxlContent)
{
    gtString fileInfosAsString;
    gtString programInfoXML;

    if (sampleId == AF_TEAPOT_SAMPLE)
    {
        const wchar_t smokeSimulationProgram[] = L"clSmokeSimulation";
        const wchar_t volumeSlicingProgram[] = L"clVolumeSlicing";
        const wchar_t debugProgram[] = L"clDebug";
        const wchar_t glProgram[] = L"glTeapot";
        enum { TEAPOT_FILE_NAME, TEAPOT_KERNEL_NAME, TEAPOT_LINE_NUM, TEAPOT_PROGRAM_NAME};
        const int kernelsInTeapot = 42;
        // This array is a set of
        // ".cl file name","kernel name","kernel start line number", "program name"
        // Each file loaded is compared to this list and it kernels (there can be more then one)
        // is added to the files kernels with the correct kernel line location
        gtString clFileToKernelMap[kernelsInTeapot][4] =
        {
            { L"tpVolumeSlicing.cl",                   L"computeIntersection",             L"105", volumeSlicingProgram },

            { L"tpApplySources.cl",                    L"applySources",                    L"28",  smokeSimulationProgram },
            { L"tpApplyBuoyancy.cl",                   L"applyBuoyancy",                   L"19",  smokeSimulationProgram },
            { L"tpCalculateCurlU.cl",                  L"calculateCurlU",                  L"19",  smokeSimulationProgram },
            { L"tpApplyVorticity.cl",                  L"applyVorticity",                  L"20",  smokeSimulationProgram },
            { L"tpAdvectFieldVelocity.cl",             L"advectFieldVelocity",             L"25",  smokeSimulationProgram },
            { L"tpApplyVelocityBoundaryCondition.cl",  L"applyVelocityBoundaryCondition",  L"26",  smokeSimulationProgram },
            { L"tpComputeFieldPressurePrep.cl",        L"computeFieldPressurePrep",        L"23",  smokeSimulationProgram },
            { L"tpComputeFieldPressureIter.cl",        L"computeFieldPressureIter",        L"20",  smokeSimulationProgram },
            { L"tpApplyPressureBoundaryCondition.cl",  L"applyPressureBoundaryCondition",  L"24",  smokeSimulationProgram },
            { L"tpProjectFieldVelocity.cl",            L"projectFieldVelocity",            L"20",  smokeSimulationProgram },
            { L"tpAdvectFieldScalar.cl",               L"advectFieldScalar",               L"33",  smokeSimulationProgram },
            { L"tpDissipateDensity.cl",                L"dissipateDensity",                L"17",  smokeSimulationProgram },
            { L"tpDissipateTemperature.cl",            L"dissipateTemperature",            L"17",  smokeSimulationProgram },
            { L"tpCreateDensityTexture.cl",            L"createDensityTexture",            L"18",  smokeSimulationProgram },

            { L"tpDebugDensity.cl",                    L"debugDensity",                    L"16",  debugProgram },
            { L"tpDebugFieldPressure.cl",              L"debugFieldPressure",              L"20",  debugProgram },
            { L"tpDebugTemperature.cl",                L"debugTemperature",                L"15",  debugProgram },
            { L"tpDebugVelocityLength.cl",             L"debugVelocityLength",             L"18",  debugProgram },
            { L"tpDebugVelocityVector.cl",             L"debugVelocityVector",             L"20",  debugProgram },

            { L"tpSmokeSimulation.cl",                 L"applySources",                    L"452", L"" },
            { L"tpSmokeSimulation.cl",                 L"applyBuoyancy",                   L"490", L""},
            { L"tpSmokeSimulation.cl",                 L"calculateCurlU",                  L"507", L"" },
            { L"tpSmokeSimulation.cl",                 L"applyVorticity",                  L"552", L"" },
            { L"tpSmokeSimulation.cl",                 L"advectFieldVelocity",             L"612", L"" },
            { L"tpSmokeSimulation.cl",                 L"applyVelocityBoundaryCondition",  L"646", L"" },
            { L"tpSmokeSimulation.cl",                 L"computeFieldPressurePrep",        L"669", L"" },
            { L"tpSmokeSimulation.cl",                 L"computeFieldPressureIter",        L"717", L"" },
            { L"tpSmokeSimulation.cl",                 L"applyPressureBoundaryCondition",  L"781", L"" },
            { L"tpSmokeSimulation.cl",                 L"projectFieldVelocity",            L"801", L"" },
            { L"tpSmokeSimulation.cl",                 L"advectFieldScalar",               L"862", L"" },
            { L"tpSmokeSimulation.cl",                 L"dissipateDensity",                L"887", L"" },
            { L"tpSmokeSimulation.cl",                 L"dissipateTemperature",            L"902", L"" },
            { L"tpSmokeSimulation.cl",                 L"createDensityTexture",            L"918", L"" },
            { L"tpSmokeSimulation.cl",                 L"debugDensity",                    L"952", L"" },
            { L"tpSmokeSimulation.cl",                 L"debugTemperature",                L"452", L"" },
            { L"tpSmokeSimulation.cl",                 L"debugVelocityVector",             L"987", L"" },
            { L"tpSmokeSimulation.cl",                 L"debugVelocityLength",             L"1006", L"" },
            { L"tpSmokeSimulation.cl",                 L"debugFieldPressure",              L"1024", L"" },

            { L"tpFragmentShader.catchMeIfYouCan.glsl",     L"Fragment",                        L"",    glProgram },
            { L"tpFragmentShader.glsl",                L"Fragment",                        L"",    glProgram },
            { L"tpVertexShader.glsl",                  L"Vertex",                          L"",    glProgram }
        };


        const wchar_t programSectionStart[] = L"                <Program>\r\n"
                                              L"                    <ProgramName>%ls</ProgramName>\r\n"
                                              L"                    <ProgramType>%ls</ProgramType>\r\n"
                                              L"                    <ProgramBitness>0</ProgramBitness >\r\n"
                                              L"                    <SourceFiles>\r\n";
        gtString smokeSimulationFolderXML;
        smokeSimulationFolderXML.appendFormattedString(programSectionStart, smokeSimulationProgram, L"ProgramCL");

        gtString volumeSlicingFolderXML;
        volumeSlicingFolderXML.appendFormattedString(programSectionStart, volumeSlicingProgram, L"ProgramCL");

        gtString debugFolderXML;
        debugFolderXML.appendFormattedString(programSectionStart, debugProgram, L"ProgramCL");

        const wchar_t programSectionFiles[] =   L"                         <SourceFile>\r\n"
                                                L"                             <SourceFileId>VERTEX_FILE_ID</SourceFileId>\r\n"
                                                L"                             <stage>vertex</stage>\r\n"
                                                L"                         </SourceFile>\r\n"
                                                L"                         <SourceFile>\r\n"
                                                L"                             <SourceFileId>-1</SourceFileId>\r\n"
                                                L"                             <stage>tese</stage>\r\n"
                                                L"                         </SourceFile>\r\n"
                                                L"                         <SourceFile>\r\n"
                                                L"                             <SourceFileId>-1</SourceFileId>\r\n"
                                                L"                             <stage>tesc</stage>\r\n"
                                                L"                         </SourceFile>\r\n"
                                                L"                         <SourceFile>\r\n"
                                                L"                             <SourceFileId>-1</SourceFileId>\r\n"
                                                L"                             <stage>geom</stage>\r\n"
                                                L"                         </SourceFile>\r\n"
                                                L"                         <SourceFile>\r\n"
                                                L"                             <SourceFileId>FRAGMENT_FILE_ID</SourceFileId>\r\n"
                                                L"                             <stage>frag</stage>\r\n"
                                                L"                         </SourceFile>\r\n";

        gtString glProgramXML;
        glProgramXML.appendFormattedString(programSectionStart, glProgram, L"ProgramGL_Rendering");
        glProgramXML.append(programSectionFiles);

        gtList<osFilePath> filePaths;
        gtList<osFilePath> glslFilePaths;
        // Get all the cl files in res folder:
        resDirectory.getContainedFilePaths(gtString(L"*." AF_STR_clSourceFileExtension), osDirectory::SORT_BY_DATE_ASCENDING, filePaths);
        // add glsl files
        resDirectory.getContainedFilePaths(gtString(L"*." AF_STR_glslSourceFileExtension), osDirectory::SORT_BY_DATE_ASCENDING, glslFilePaths);
        filePaths.insert(filePaths.end(), glslFilePaths.begin(), glslFilePaths.end());

        osFilePath kernelOutputPath;
        afGetUserDataFolderPath(kernelOutputPath);
        gtString subDirName = AF_STR_TeapotSampleProjectName;

        if (afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
        {
            subDirName = AF_STR_TeapotSampleVSProjectName;
        }

        subDirName.append(AF_STR_KernelOutputExtension);
        kernelOutputPath.appendSubDirectory(subDirName);

        int sourceFileId = 0;

        for (auto iter = filePaths.begin(); iter != filePaths.end(); sourceFileId++, iter++)
        {
            // Get the current file path:

            gtString currentPathStr = iter->asString();

            gtString fileName;
            iter->getFileName(fileName);
            gtString originalFileName;
            iter->getFileNameAndExtension(originalFileName);

            // add to file name the file ext
            gtString fileExt;
            iter->getFileExtension(fileExt);
            fileName.appendFormattedString(L"_%ls", fileExt.asCharArray());

            // Build the kernel output folder:
            osFilePath currentkernelOutputPath = kernelOutputPath;
            currentkernelOutputPath.appendSubDirectory(fileName);

            // Create the overview file associated with the kernel file:
            osDirectory kernelDir(currentkernelOutputPath);
            kernelDir.create();
            osFilePath overViewPath = currentkernelOutputPath;
            overViewPath.setFileName(L"Overview");
            overViewPath.setFileExtension(L"cxlovr");
            osFile overViewFile;
            overViewFile.open(overViewPath, osChannel::OS_UNICODE_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);
            overViewFile.writeString(currentPathStr);
            overViewFile.close();

            gtString filePathXML;
            filePathXML.appendFormattedString(L"                <FileInfo>\r\n"
                                              L"                    <SourceFileId>%d</SourceFileId>\r\n"
                                              L"                    <FilePath>%ls</FilePath>\r\n"
                                              L"                    <ExecutionPath>%ls</ExecutionPath>\r\n"
                                              L"                    <ExecutionDevices />\r\n",
                                              sourceFileId,
                                              currentPathStr.asCharArray(),
                                              currentkernelOutputPath.fileDirectoryAsString().asCharArray());

            // look for the kernel in the kernels list to get kernel name and the kernel line:
            bool wasKernelsFound = false;
            bool isFileIncludedInList = false;

            for (int nFile = 0; nFile < kernelsInTeapot; nFile++)
            {
                if (clFileToKernelMap[nFile][TEAPOT_FILE_NAME] == originalFileName)
                {
                    isFileIncludedInList = true;

                    if (fileExt.compare(AF_STR_clSourceFileExtension) == 0)
                    {
                        // if it is the first kernel found add the data of the opencl information
                        if (!wasKernelsFound)
                        {
                            filePathXML.appendFormattedString(L"                    <ShaderPlatform>OpenCL</ShaderPlatform>\r\n"
                                                              L"                    <ShaderArch>32-bit</ShaderArch>\r\n"
                                                              L"                    <ShaderTarget/>\r\n"
                                                              L"                    <ShaderEntryPoint>%ls</ShaderEntryPoint>\r\n"
                                                              L"                    <AnalyzeSection>\r\n",
                                                              clFileToKernelMap[nFile][TEAPOT_KERNEL_NAME].asCharArray());
                        }

                        wasKernelsFound = true;
                        filePathXML.appendFormattedString(L"                        <KernelAnalysis>%ls,", clFileToKernelMap[nFile][TEAPOT_KERNEL_NAME].asCharArray());
                        filePathXML.appendFormattedString(L"                        %ls,", AF_STR_startup_page_teapot_cxl_ka_default_values);
                        filePathXML.appendFormattedString(L"                        %ls</KernelAnalysis>", clFileToKernelMap[nFile][TEAPOT_LINE_NUM].asCharArray());
                    }
                    else
                    {
                        filePathXML.appendFormattedString(L"                    <ShaderPlatform>OpenGL</ShaderPlatform>\r\n"
                                                          L"                    <ShaderArch>32-bit</ShaderArch>\r\n"
                                                          L"                    <ShaderTarget/>\r\n"
                                                          L"                    <ShaderEntryPoint>main</ShaderEntryPoint>\r\n"
                                                          L"                    <ShaderGLType>%ls</ShaderGLType>\r\n"
                                                          L"                    <AnalyzeSection>\r\n",
                                                          clFileToKernelMap[nFile][TEAPOT_KERNEL_NAME].asCharArray());
                    }

                    // Add this file to a program
                    const wchar_t sourceFileElementXMLFormat[] =    L"                        <SourceFile>\r\n"
                                                                    L"                            <SourceFileId>%d</SourceFileId>\r\n"
                                                                    L"                        </SourceFile>\r\n";

                    if (smokeSimulationProgram == clFileToKernelMap[nFile][TEAPOT_PROGRAM_NAME])
                    {
                        smokeSimulationFolderXML.appendFormattedString(sourceFileElementXMLFormat, sourceFileId);
                    }
                    else if (debugProgram == clFileToKernelMap[nFile][TEAPOT_PROGRAM_NAME])
                    {
                        debugFolderXML.appendFormattedString(sourceFileElementXMLFormat, sourceFileId);
                    }
                    else if (volumeSlicingProgram == clFileToKernelMap[nFile][TEAPOT_PROGRAM_NAME])
                    {
                        volumeSlicingFolderXML.appendFormattedString(sourceFileElementXMLFormat, sourceFileId);
                    }
                    else if (L"tpVertexShader.glsl" == clFileToKernelMap[nFile][TEAPOT_FILE_NAME])
                    {
                        gtString fileId;
                        fileId.appendFormattedString(L"%d", sourceFileId);
                        glProgramXML.replace(L"VERTEX_FILE_ID", fileId);
                    }
                    else if (L"tpFragmentShader.glsl" == clFileToKernelMap[nFile][TEAPOT_FILE_NAME])
                    {
                        gtString fileId;
                        fileId.appendFormattedString(L"%d", sourceFileId);
                        glProgramXML.replace(L"FRAGMENT_FILE_ID", fileId);
                    }
                }
            }

            if (false == isFileIncludedInList)
            {
                filePathXML.appendFormattedString(L"<AnalyzeSection>");
            }

            filePathXML.appendFormattedString(L"</AnalyzeSection></FileInfo>");
            fileInfosAsString.append(L"\n");
            fileInfosAsString.append(filePathXML);

        }

        gtString programSectionEnd(L"                    </SourceFiles>\r\n"
                                   L"                </Program>\r\n");

        smokeSimulationFolderXML.append(programSectionEnd);
        volumeSlicingFolderXML.append(programSectionEnd);
        debugFolderXML.append(programSectionEnd);
        glProgramXML.append(programSectionEnd);

        programInfoXML.append(smokeSimulationFolderXML);
        programInfoXML.append(volumeSlicingFolderXML);
        programInfoXML.append(debugFolderXML);
        programInfoXML.append(glProgramXML);

    }
    else
    {
        // The MatMul sample does not have any CL files
        fileInfosAsString = AF_STR_Empty;
    }

    cxlContent.replace(L"REPLACE_ME_WITH_KERNEL_AND_SHADER_FILES", fileInfosAsString);
    cxlContent.replace(L"REPLACE_ME_WITH_PROGRAMS", programInfoXML);

}

void afApplicationCommands::LoadSample(afCodeXLSampleID sampleId)
{
    // Create the sample project file path:
    osFilePath sampleCXLFile;

    // Get the sample name by its id:
    gtString sampleName;

    switch (sampleId)
    {
        case AF_TEAPOT_SAMPLE:
            sampleName = AF_STR_TeapotSampleProjectName;
            break;

        case AF_MATMUL_SAMPLE:
            sampleName = AF_STR_MatMulSampleProjectName;
            break;

        case AF_D3D12MULTITHREADING_SAMPLE:
            sampleName = AF_STR_D3D12MultithreadingSampleProjectName;
            break;

        default:
            break;
    }

    // Get the teapot CXL file path:
    getProjectsFilePath(sampleName, sampleCXLFile);

    // Always load the teapot sample from scratch:
    // Create the file from hardcoded string:
    bool rc = WriteSampleCXL(sampleId);
    GT_ASSERT(rc);

    // Check that the project exists:
    GT_IF_WITH_ASSERT(sampleCXLFile.exists())
    {
        // Open the teapot sample:
        OnFileOpenProject(sampleCXLFile.asString());
    }
}

// ---------------------------------------------------------------------------
void afApplicationCommands::closeDocumentsOfDeletedFiles()
{
    // Pass through all the MDI files and close those that are do not exist
    QMdiArea* pMdiArea = afMainAppWindow::instance()->mdiArea();

    QList<QMdiSubWindow*> windowsSubList = pMdiArea->subWindowList();

    foreach (QMdiSubWindow* pCurrentSubWindow, windowsSubList)
    {
        // Get the widget from the window:
        afQMdiSubWindow* pAfQTSubWindow = qobject_cast<afQMdiSubWindow*>(pCurrentSubWindow);

        if (pAfQTSubWindow != nullptr)
        {
            osFilePath filePathChecked = pAfQTSubWindow->filePath();

            if (!filePathChecked.exists())
            {
                afMainAppWindow::instance()->closeMDISubWindow(pAfQTSubWindow);
            }
        }
    }
}

// ---------------------------------------------------------------------------
bool afApplicationCommands::saveMDIFile(const osFilePath& filePath)
{
    bool retVal = false;

    afMainAppWindow* pMainWindow = afMainAppWindow::instance();

    if (nullptr != pMainWindow)
    {
        // find the mdi of the path:
        afQMdiSubWindow* pSourceWindowMDI = pMainWindow->findMDISubWindow(filePath);

        if (nullptr != pSourceWindowMDI && (nullptr != pSourceWindowMDI->widget()))
        {
            // make sure it has a source view:
            acSourceCodeView* pSourceWindow = qobject_cast<acSourceCodeView*>(pSourceWindowMDI->widget());

            if (nullptr != pSourceWindow)
            {
                // save it and remove the "*" that marks the modified state:
                bool fileSaved = pSourceWindow->saveFile();

                if (fileSaved)
                {
                    MarkMDIWindowAsChanged(filePath, false);

                    // update the document to the update mechanism:
                    afDocUpdateManager::instance().UpdateDocument(pSourceWindow);
                    retVal = true;
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
bool afApplicationCommands::AttachItemDataToSourceView(const osFilePath& filePath, const afApplicationTreeItemData* pTreeItemData)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(pTreeItemData != nullptr)
    {
        afMainAppWindow* pMainWindow = afMainAppWindow::instance();

        if (nullptr != pMainWindow)
        {
            // find the mdi of the path:
            afQMdiSubWindow* pSourceWindowMDI = pMainWindow->findMDISubWindow(filePath);

            if (nullptr != pSourceWindowMDI && (nullptr != pSourceWindowMDI->widget()))
            {
                // make sure it has a source view:
                afSourceCodeView* pSourceWindow = qobject_cast<afSourceCodeView*>(pSourceWindowMDI->widget());

                if (nullptr != pSourceWindow)
                {
                    pSourceWindow->SetMatchingTreeItemData(pTreeItemData);
                    retVal = true;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
void afApplicationCommands::RestorePropertiesViewToCurrentlySelectedItem()
{
    GT_IF_WITH_ASSERT(applicationTree() != nullptr)
    {
        // Get the currently selected item, and display in the properties view:
        QTreeWidgetItem* pSelectedItem = applicationTree()->getTreeSelection();

        if (pSelectedItem != nullptr)
        {
            const afApplicationTreeItemData* pSelectedItemData = applicationTree()->getTreeItemData(pSelectedItem);

            if (pSelectedItemData != nullptr)
            {
                applicationTree()->DisplayItemProperties(pSelectedItemData);
            }
        }
    }
}

void afApplicationCommands::OpenContainingFolder(const osFilePath& filePath)
{
    osFilePath filePathToOpen = filePath;
    osDirectory directoryToOpen;
    filePathToOpen.getFileDirectory(directoryToOpen);
    GT_IF_WITH_ASSERT(directoryToOpen.exists())
    {
        gtString fileLauncherParameters;
# if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        //highlighting file in containing folder
        fileLauncherParameters << L"/Select,";
        osFileLauncher fileLauncher(filePathToOpen.asString().asCharArray(), fileLauncherParameters);
#else //Linux 
        // just open the folder:
        osFileLauncher fileLauncher(directoryToOpen.asString(true).asCharArray(), fileLauncherParameters);
#endif
        fileLauncher.launchFile();
    }
}

void afApplicationCommands::GetListOfOpenedWindowsForFile(const gtString& containingDirectory, gtVector<osFilePath>& listOfOpenedWindows)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((afMainAppWindow::instance() != nullptr) && (afMainAppWindow::instance()->mdiArea() != nullptr))
    {
        // Get the current list of sub-windows:
        QList<QMdiSubWindow*> subWindowsList = afMainAppWindow::instance()->mdiArea()->subWindowList();

        foreach (QMdiSubWindow* pSubWindow, subWindowsList)
        {
            afQMdiSubWindow* pMDISubWindow = qobject_cast<afQMdiSubWindow*>(pSubWindow);

            if (pMDISubWindow != nullptr)
            {
                osFilePath currentWindowPath = pMDISubWindow->filePath();
                gtString currentDir = currentWindowPath.fileDirectoryAsString();

                if (currentDir.find(containingDirectory) >= 0)
                {
                    listOfOpenedWindows.push_back(currentWindowPath);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
QMessageBox::StandardButton afApplicationCommands::ShowMessageBox(QMessageBox::Icon type, const QString& title, const QString& text, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
    return afMessageBox::instance().ShowMessageBox(type, title, text, buttons, defaultButton);
}

// ---------------------------------------------------------------------------
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::getToolButtonIcon
// Description: service function to load an icon for QToolButton
// Author:      Gilad Yarnitzky
// Date:        9/4/2012
// ---------------------------------------------------------------------------
QIcon afApplicationCommands::getToolButtonIcon(QToolButton* pToolButton)
{
    gtString iconPath;
    bool retVal = afGetApplicationImagesPath(iconPath);
    GT_ASSERT(retVal)
    iconPath.append(osFilePath::osPathSeparator);

    if (pToolButton->objectName() == "backButton")
    {
        iconPath.append(AC_STR_image_back);
    }
    else if (pToolButton->objectName() == "forwardButton")
    {
        iconPath.append(AC_STR_image_forward);
    }
    else if (pToolButton->objectName() == "toParentButton")
    {
        iconPath.append(AC_STR_image_toParent);
    }
    else if (pToolButton->objectName() == "newFolderButton")
    {
        iconPath.append(AC_STR_image_newFolder);
    }
    else if (pToolButton->objectName() == "listModeButton")
    {
        iconPath.append(AC_STR_image_listMode);
    }
    else if (pToolButton->objectName() == "detailModeButton")
    {
        iconPath.append(AC_STR_image_detailMode);
    }

    QString iconPathQStr(QString::fromWCharArray(iconPath.asCharArray()));

    QIcon returnIcon(iconPathQStr);

    return returnIcon;
};

// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::getPushButtonIcon
// Description: service function to load an icon for QPushButton
// Author:      Gilad Yarnitzky
// Date:        9/4/2012
// ---------------------------------------------------------------------------
QIcon afApplicationCommands::getPushButtonIcon(QPushButton* pPushButton)
{
    gtString iconPath;
    bool retVal = afGetApplicationImagesPath(iconPath);
    GT_ASSERT(retVal)
    iconPath.append(osFilePath::osPathSeparator);

    if (pPushButton->text() == "&Open")
    {
        iconPath.append(AC_STR_image_open);
    }
    else if (pPushButton->text() == "&Save")
    {
        iconPath.append(AC_STR_image_save);
    }
    else
    {
        iconPath.append(AC_STR_image_cancel);
    }

    QString iconPathQStr(QString::fromWCharArray(iconPath.asCharArray()));

    QIcon returnIcon(iconPathQStr);

    return returnIcon;
};

// ---------------------------------------------------------------------------
// Name:        afApplicationCommands::prepareDialog
// Description: service function to prepare the dialog in linux
// Author:      Gilad Yarnitzky
// Date:        10/4/2012
// ---------------------------------------------------------------------------
void afApplicationCommands::prepareDialog(QFileDialog& dialog)
{
    // Specific handling of the qtoolbuttons:
    QList<QToolButton*> toolButtonList = dialog.findChildren<QToolButton*>();

    for (int nButton = 0 ; nButton < toolButtonList.count() ; nButton ++)
    {
        QToolButton* pCurrentButton = toolButtonList.at(nButton);

        // Replace the toolbutton icon:
        pCurrentButton->setIcon(getToolButtonIcon(pCurrentButton));

        // hide the mode buttons:
        if ((pCurrentButton->objectName() == "listModeButton") || (pCurrentButton->objectName() == "detailModeButton"))
        {
            pCurrentButton->hide();
        }
    }

    // Specific handling of the qpushbuttons:
    QList<QPushButton*> pushList = dialog.findChildren<QPushButton*>();

    for (int nPush = 0 ; nPush < pushList.count() ; nPush ++)
    {
        // Replace the pushbutton icon:
        pushList.at(nPush)->setIcon(getPushButtonIcon(pushList.at(nPush)));
    }

    // Specific handling of the qtreeview:
    QList<QTreeView*> treeList = dialog.findChildren<QTreeView*>();

    for (int nTree = 0 ; nTree < treeList.count() ; nTree ++)
    {
        treeList.at(nTree)->setHeaderHidden(true);
    }

    dialog.setIconProvider(&globalFileIconProvider);
}

#endif



void afApplicationCommands::StartPerformancePrintout(const QString& tagName)
{
    GT_UNREFERENCED_PARAMETER(tagName);

    if (m_shouldPrintPerformanceTimestamps)
    {
        m_performanceTimer.restart();
        QString startStr = QString("Start time for %1: %2").arg(tagName).arg(m_performanceTimer.toString("HH:mm:ss"));
        AddStringToInformationView("\n");
        AddStringToInformationView(startStr);
    }
}

void afApplicationCommands::EndPerformancePrintout(const QString& tagName)
{
    GT_UNREFERENCED_PARAMETER(tagName);

    if (m_shouldPrintPerformanceTimestamps)
    {

        int millisecs = m_performanceTimer.elapsed() % 1000;
        int secs = m_performanceTimer.elapsed() / 1000;
        int mins = (secs / 60) % 60;
        int hours = (secs / 3600);
        secs = secs % 60;
        QString timeElapsed = QString("%1:%2:%3.%4").arg(hours, 2, 10, QLatin1Char('0')).arg(mins, 2, 10, QLatin1Char('0')).arg(secs, 2, 10, QLatin1Char('0')).arg(millisecs, 3, 10, QLatin1Char('0'));

        m_performanceTimer.restart();

        AddStringToInformationView("\n");
        QString endStr = QString("End time for %1: %2").arg(tagName).arg(m_performanceTimer.toString("HH:mm:ss"));
        AddStringToInformationView(endStr);

        AddStringToInformationView("\n");
        QString elapsedStr = QString("Time elapsed for %1 : %2").arg(tagName).arg(timeElapsed);
        AddStringToInformationView(elapsedStr);
    }
}

afApplicationTree::DragAction afApplicationCommands::DragActionForDropEvent(QDropEvent* pEvent)
{
    afApplicationTree::DragAction retVal = afApplicationTree::DRAG_NO_ACTION;

    if (pEvent != nullptr)
    {
        const QMimeData* pMimeData = pEvent->mimeData();

        if (pMimeData->hasUrls())
        {
            if (!pMimeData->urls().isEmpty())
            {
                foreach (QUrl url, pMimeData->urls())
                {
                    // Get the URL as file path:
                    QString qurlStr = url.toLocalFile();
                    gtString fileUrl;
                    fileUrl.fromASCIIString(qurlStr.toLatin1().data());
                    osFilePath path(fileUrl);
                    gtString extension;
                    path.getFileExtension(extension);



                    if ((extension == AF_STR_profileFileExtension1) || (extension == AF_STR_profileFileExtension2) || (extension == AF_STR_profileFileExtension3)
                        || (extension == AF_STR_profileFileExtension4) || (extension == AF_STR_profileFileExtension5) || (extension == AF_STR_profileFileExtension6)
                        || (extension == AF_STR_profileFileExtension7) || (extension == AF_STR_profileFileExtension8) || (extension == AF_STR_profileFileExtension9))
                    {
                        // Check if the list is mixed:
                        if ((retVal != afApplicationTree::DRAG_NO_ACTION) && (retVal != afApplicationTree::DRAG_ADD_SESSION_TO_TREE))
                        {
                            // Mixed list:
                            retVal = afApplicationTree::DRAG_NO_ACTION;
                            break;
                        }
                        else
                        {
                            retVal = afApplicationTree::DRAG_ADD_SESSION_TO_TREE;
                        }
                    }
                    else
                    {
                        retVal = afApplicationTree::DRAG_ADD_ANALYZED_FILE_TO_TREE;
                    }
                }
            }
        }
    }

    // Check if a process is currently running:
    bool isProcessRunning = (afPluginConnectionManager::instance().getCurrentRunModeMask() & AF_DEBUGGED_PROCESS_EXISTS);

    // Make sure that the action is possible while a process is running:
    if ((retVal != afApplicationTree::DRAG_NO_ACTION) && isProcessRunning)
    {
        if ((retVal == afApplicationTree::DRAG_ADD_SESSION_TO_TREE) || (retVal == afApplicationTree::DRAG_OPEN_PROJECT) || (retVal == afApplicationTree::DRAG_NEW_PROJECT))
        {
            retVal = afApplicationTree::DRAG_NO_ACTION;
        }
    }

    return retVal;
}

void afApplicationCommands::HandleDropEvent(QWidget* receiver, QDropEvent* pEvent)
{
    // Sanity check
    GT_IF_WITH_ASSERT(pEvent != nullptr)
    {
        const QMimeData* pMimeData = pEvent->mimeData();

        if (pMimeData != nullptr)
        {
            if (pMimeData->hasUrls())
            {
                // Check what is the needed drag action for the current event:
                afApplicationTree::DragAction dragActionForFiles = DragActionForDropEvent(pEvent);
                GT_IF_WITH_ASSERT(dragActionForFiles != afApplicationTree::DRAG_NO_ACTION)
                {
                    if ((dragActionForFiles == afApplicationTree::DRAG_NEW_PROJECT) || (dragActionForFiles == afApplicationTree::DRAG_OPEN_PROJECT))
                    {
                        // Single file operation -> open / create a project:
                        gtString fileUrl = acQStringToGTString(pMimeData->urls().at(0).toLocalFile());
                        osFilePath droppedFilePath(fileUrl);
                        gtString extension, projectName;
                        droppedFilePath.getFileExtension(extension);
                        droppedFilePath.getFileName(projectName);

                        if (dragActionForFiles == afApplicationTree::DRAG_NEW_PROJECT)
                        {
                            // Open a new project settings dialog for the dragged file:
                            OpenNewProjectForDraggedFile(pEvent);
                        }
                        else if (dragActionForFiles == afApplicationTree::DRAG_OPEN_PROJECT)
                        {
                            // If this is a cxl file, open the project:
                            afApplicationCommands::instance()->OnFileOpenProject(droppedFilePath.asString());
                        }
                    }

                    else if (dragActionForFiles == afApplicationTree::DRAG_OPEN_FILES)
                    {
                        // Open each of the dragged file in MDI window:
                        foreach (QUrl url, pMimeData->urls())
                        {
                            // Get the current file path:
                            gtString fileUrl = acQStringToGTString(url.toLocalFile());
                            osFilePath droppedFilePath(fileUrl);
                            gtString extension, fileName;
                            droppedFilePath.getFileExtension(extension);
                            droppedFilePath.getFileName(fileName);

                            // Open the requested file in the MDI area:
                            afApplicationCommands::instance()->OpenFileAtLine(droppedFilePath.asString(), -1, false);
                        }

                        // Update toolbars:
                        afApplicationCommands::instance()->updateToolbarCommands();
                    }

                    else if (dragActionForFiles == afApplicationTree::DRAG_ADD_ANALYZED_FILE_TO_TREE)
                    {
                        if (afApplicationCommands::instance()->applicationTree() != nullptr)
                        {
                            QString dragDropFile;

                            if (pEvent != nullptr)
                            {
                                afApplicationCommands::instance()->applicationTree()->ExecuteDropEvent(receiver, pEvent, dragDropFile);
                            }
                        }
                    }

                    else if (dragActionForFiles == afApplicationTree::DRAG_ADD_SESSION_TO_TREE)
                    {
                        if (afApplicationCommands::instance()->applicationTree() != nullptr)
                        {
                            QString dragDropFile;

                            if (pEvent != nullptr)
                            {
                                afApplicationCommands::instance()->applicationTree()->ExecuteDropEvent(receiver, pEvent, dragDropFile);
                            }
                        }
                    }
                }
            }
        }
    }
}

void afApplicationCommands::OpenNewProjectForDraggedFile(QDropEvent* pEvent)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pEvent != nullptr)
    {
        const QMimeData* pMimeData = pEvent->mimeData();

        // We should only get here with one exe file:
        GT_IF_WITH_ASSERT(pMimeData->urls().size() == 1)
        {
            // check for our needed mime type, here a file or a list of files
            {
                QUrl url = pMimeData->urls().at(0);
                QString qurlStr = url.toLocalFile();
                gtString fileUrl;
                fileUrl.fromASCIIString(qurlStr.toLatin1().data());
                osFilePath droppedFilePath(fileUrl);
                gtString extension, projectName;
                droppedFilePath.getFileExtension(extension);
                droppedFilePath.getFileName(projectName);
                GT_IF_WITH_ASSERT(extension == AF_STR_exeFileExtension)
                {
                    // Check if we already have a project with the same name:
                    bool isProjectExist = false;
                    gtString appName;
                    gtVector<gtString> projectFilePaths;
                    afApplicationCommands::instance()->FillRecentlyUsedProjectsNames(projectFilePaths, appName, true);

                    osFilePath existingProjectPath;

                    for (int i = 0; i < (int)projectFilePaths.size(); i++)
                    {
                        osFilePath currentPath(projectFilePaths[i]);
                        gtString currentProjectName;
                        currentPath.getFileName(currentProjectName);

                        if (currentProjectName == projectName)
                        {
                            isProjectExist = true;
                            existingProjectPath = currentPath;
                            break;
                        }
                    }

                    if (isProjectExist)
                    {
                        QString message = QString(AF_STR_newProjectExistingQuestion).arg(acGTStringToQString(projectName));
                        QMessageBox::StandardButton userAnswer = acMessageBox::instance().question(AF_STR_QuestionA, message, QMessageBox::Yes | QMessageBox::No);
                        isProjectExist = (userAnswer == QMessageBox::No);
                    }

                    if (!isProjectExist)
                    {
                        afApplicationCommands::instance()->OnFileNewProject(droppedFilePath.asString());
                    }
                    else
                    {
                        afApplicationCommands::instance()->OnFileOpenProject(existingProjectPath.asString());
                    }
                }
            }
        }
    }
}
