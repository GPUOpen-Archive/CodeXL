//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file appApplication.cpp
///
//==================================================================================

// =================================================================
// AMD application framework:
//      Initializes the application
//      create the main window
// =================================================================

// Qt:
#include <QApplication>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osEnvironmentVariable.h>
#include <AMDTOSWrappers/Include/osProcess.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acIcons.h>
//#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationFramework/Include/afMessageBox.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afGeneralViewsCreator.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afMDIViewsCreator.h>
#include <AMDTApplicationFramework/Include/afSourceCodeViewsManager.h>
#include <AMDTApplicationFramework/Include/afQtCreatorsManager.h>
#include <AMDTApplicationFramework/Include/commands/afInitializeApplicationCommand.h>

// Local:
#include <inc/appMainAppWindow.h>
#include <inc/appApplication.h>
#include <src/appApplicationCommands.h>
#include <src/appQtApplication.h>


// For insufficient memory event handling.
#include <AMDTOSWrappers/Include/osCallStack.h>
#include <AMDTOSWrappers/Include/osCallsStackReader.h>
#include <QtWidgets/QApplication>
#include <AMDTOSWrappers/Include/osProcess.h>

QSplashScreen* globalSplashScreen = NULL;

#define APP_NO_SPLASH_ARG "nosplash"

typedef int(*errorProcedureAddress)(gtString& string);

static const unsigned int MIN_SCREEN_HEIGHT = 768;
static const unsigned int MIN_SCREEN_WIDTH = 1024;


appApplicationStart::ApplicationStartupData appApplicationStart::m_sApplicationStartupData;

appApplicationStart::ApplicationStartupData::ApplicationStartupData()
{
    m_productName = L"";
    m_applicationIconId = AC_ICON_EMPTY;
    m_initializeExecutionModesManager = false;
}


appApplicationStart::ApplicationStartupData& appApplicationStart::ApplicationStartupData::operator=(const ApplicationStartupData& other)
{
    if (this != &other)
    {
        m_productName = other.m_productName;
        m_dllNamesForLoad = other.m_dllNamesForLoad;
        m_applicationIconId = other.m_applicationIconId;
        m_splashScreenFullPath = other.m_splashScreenFullPath;
        m_initializeExecutionModesManager = other.m_initializeExecutionModesManager;
        m_flagEnable = other.m_flagEnable;
    }
    return *this;
}


// ---------------------------------------------------------------------------
// Name:        dynamicallyCall
// Description: dynamically call the function in all libraries that have a function
//              in that name in the installation directory
// Arguments:   gtString functionName
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        25/1/2012
// ---------------------------------------------------------------------------
void appApplicationStart::DynamicallyCall(const gtASCIIString& functionName, int functionType, gtString& returnString, const gtList<gtString>& modulesNames)
{
    osFilePath modulePath(osFilePath::OS_CODEXL_BINARIES_PATH);
    const char* functionNameAsChar = functionName.asCharArray();

#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS)
    modulePath.setFileExtension(L"so");
#else
    modulePath.setFileExtension(L"dll");
#endif


    // Pass through each module and look for the specific function
    gtList<gtString>::const_iterator iter = modulesNames.begin();
    gtList<gtString>::const_iterator endIter = modulesNames.end();

    while (iter != endIter)
    {
        const gtString& currFilePath = *iter;

        // Prepare the full path:
        modulePath.setFileName(currFilePath);
        //        gtString fileToLoadFullPath = modulePath.asString();

        osModuleHandle moduleHandle;
        bool successfulLoad = osLoadModule(modulePath, moduleHandle);

        if (successfulLoad)
        {
            osProcedureAddress procedureAddress;

            if (osGetProcedureAddress(moduleHandle, functionNameAsChar, procedureAddress, false))
            {
                // Sanity
                GT_IF_WITH_ASSERT(procedureAddress)
                {
                    // It was found so call the function
                    switch (functionType)
                    {
                        case 0:
                            procedureAddress();
                            break;

                        case 1:
                        {
                            gtString errorStr;
                            errorProcedureAddress errProcedureAddress = (errorProcedureAddress)procedureAddress;
                            int retVal = errProcedureAddress(errorStr);

                            if (retVal != 0)
                            {
                                returnString.append(errorStr);
                                returnString.append(L"\n");
                            }
                        }
                        break;

                        default:
                            GT_ASSERT(false);
                            break;
                    }
                }
            }
        }
        else
        {
            // failed to load a basic component:
            OS_OUTPUT_DEBUG_LOG(AF_STR_LogMsg_FailedToLoadAComponent, OS_DEBUG_LOG_DEBUG);
        }

        iter++;
    }
}

// ---------------------------------------------------------------------------
// Name:        loadSplashScreen
// Description: Load the splash screen image and create the splash screen
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        25/1/2012
// ---------------------------------------------------------------------------
void appApplicationStart::LoadSplashScreen(const osFilePath& splashScreenFilePath)
{
    GT_UNREFERENCED_PARAMETER(splashScreenFilePath);
    // If we are in debug build, we do not want the splash screen:
#if AMDT_BUILD_CONFIGURATION != AMDT_DEBUG_BUILD
    {
        OS_OUTPUT_DEBUG_LOG(AF_STR_LogMsg_LoadingSplashScreen, OS_DEBUG_LOG_DEBUG);

        // Load the splash screen:
        QString splashNameQt(QString::fromWCharArray(splashScreenFilePath.asString().asCharArray()));
        QPixmap splashPixmap(splashNameQt);

        bool loadSplashScreenOK = !splashPixmap.isNull();

        if (loadSplashScreenOK)
        {
            // Load the splash screen
            globalSplashScreen = new QSplashScreen(splashPixmap);

            globalSplashScreen->show();
        }

        OS_OUTPUT_DEBUG_LOG(AF_STR_LogMsg_FinishedLoadingSplashScreen, OS_DEBUG_LOG_DEBUG);
    }
#endif
}

int appApplicationStart::appMain(int argc, char* argv[], const appApplicationStart::ApplicationStartupData& applicationStartData)
{
    int retVal = 0;

    // Sanity check (QApplication must be initialized before the call to this function):
    GT_IF_WITH_ASSERT(qApp != NULL)
    {
        m_sApplicationStartupData = applicationStartData;

        /// connect the Qt application to the slots that handle the out of memory signals:
        bool rc = QObject::connect(qApp, SIGNAL(AppMemAllocFailureSignal()), qApp, SLOT(OnAppMemAllocFailureSignal()));
        GT_ASSERT(rc);

        rc = QObject::connect(qApp, SIGNAL(ClientMemAllocFailureSignal()), qApp, SLOT(OnClientMemAllocFailureSignal()));
        GT_ASSERT(rc);

        // Call the initialize application command (can be done only after qapp init):
        afInitializeApplicationCommand initAppCmd(m_sApplicationStartupData.m_productName, AF_STR_StandaloneProduct);
        bool rcIni = initAppCmd.execute();

        // instead if doing an exit in the middle of the afInitializeApplicationCommand this is a safer exit for Qt
        // This solves a known bug in Qt + KDE (bug332093 on KDE bug report system) but also happens on CentOS and other platforms
        if (!rcIni)
        {
            return 0;
        }

        // Set the global icon id:
        afGlobalVariablesManager::SetProductIconID(m_sApplicationStartupData.m_applicationIconId);

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
        // In Linux, we must verify that the GTK+ icon cache is valid, and notify the user if it isn't:
        acValidateGTKThemeIcons();
#endif

        bool loadSplash = true;

        for (int i = 1; i < argc; i++)
        {
            gtASCIIString currentArg = argv[i];

            if (currentArg == APP_NO_SPLASH_ARG)
            {
                loadSplash = false;
            }
        }

        if (loadSplash)
        {
            // Load the splashScreen
            LoadSplashScreen(m_sApplicationStartupData.m_splashScreenFullPath);
        }

        // Create and register the application commands instance:
        appApplicationCommands* pAppApplicationCommands = new appApplicationCommands;
        GT_ASSERT(pAppApplicationCommands != NULL);

        // Set the message box icon:
        QPixmap* pPixmap = new QPixmap;
        acSetIconInPixmap(*pPixmap, m_sApplicationStartupData.m_applicationIconId, AC_64x64_ICON);

        acMessageBox::setApplicationIconPixmap(pPixmap);

        // Initialize & register the framework view creator:
        afGeneralViewsCreator::Instance().initialize();

        afQtCreatorsManager::instance().registerViewCreator(&afGeneralViewsCreator::Instance());

        if (m_sApplicationStartupData.m_flagEnable.testFlag(AppCreate::APP_ENABLE_SOURCECODE_MANAGER))
        {
            afSourceCodeViewsManager::instance();

            afMDIViewsCreator* pSourceCodeViewCreator = new afMDIViewsCreator();
            afQtCreatorsManager::instance().registerViewCreator(pSourceCodeViewCreator);
        }

        // Check validity of the plugins
        gtString validityString;
        DynamicallyCall("CheckValidity", 1, validityString, m_sApplicationStartupData.m_dllNamesForLoad);

        // Dynamically call initialize function
        gtString initString;
        DynamicallyCall("initialize", 0, initString, m_sApplicationStartupData.m_dllNamesForLoad);

        // Create the main window:
        appMainAppWindow::initializeStaticInstance();

        // Create the modes toolbars:
        if (appApplicationStart::ApplicationData().m_initializeExecutionModesManager)
        {
            afExecutionModeManager::instance().createModesToolbars();
        }

        // Get the application main window:
        afMainAppWindow* pAppMainWindow = appMainAppWindow::instance();

        pAppMainWindow->setDockOptions(QMainWindow::AllowTabbedDocks);

        // Create an instate of acMessageBox, and set parent:
        acMessageBox::instance().setParentWidget(pAppMainWindow);

        // Dynamically call initialize function
        gtString initWidgetString;
        DynamicallyCall("initializeIndependentWidgets", 0, initWidgetString, m_sApplicationStartupData.m_dllNamesForLoad);

        GT_IF_WITH_ASSERT(pAppMainWindow != NULL)
        {
            pAppMainWindow->setFocus(Qt::NoFocusReason);

            pAppMainWindow->setInitialLayoutMode(afMainAppWindow::LayoutNoProject);
            pAppMainWindow->updateLayoutMode(afMainAppWindow::LayoutNoProject);

            // check display settings
            QRect rec = QApplication::desktop()->screenGeometry();
            int height = rec.height();
            int width = rec.width();

            if (((unsigned int)height < MIN_SCREEN_HEIGHT) || ((unsigned int)width < MIN_SCREEN_WIDTH))
            {
                retVal = false;
                gtString osErrorMsg;
                osErrorMsg.appendFormattedString(AF_STR_ErrorMessageLowResolution, MIN_SCREEN_WIDTH, MIN_SCREEN_HEIGHT);
                OS_OUTPUT_DEBUG_LOG(osErrorMsg.asCharArray(), OS_DEBUG_LOG_ERROR);
                acMessageBox::instance().critical(AF_STR_ErrorA, acGTStringToQString(osErrorMsg));
            }
            else
            {
                retVal = true;
                // Show the main window:
                pAppMainWindow->show();

                // Restore minimal size:
                qobject_cast<appMainAppWindow*>(pAppMainWindow)->restoreMinimalSize();
            }
        }

        if (retVal)
        {
            afApplicationCommands* pApplicationCommnads = afApplicationCommands::instance();
            GT_IF_WITH_ASSERT(pApplicationCommnads != NULL)
            {
                // Skip the nosplash command line argument:
                int currentArgIndex = 1;

                if (argc > currentArgIndex)
                {
                    gtASCIIString currentArg = argv[currentArgIndex];

                    if (currentArg == APP_NO_SPLASH_ARG) { currentArgIndex++; }

                    if (argc > currentArgIndex)
                    {

                        // Check if there is a project file path as argument:

                        gtString projectFileName;
                        projectFileName.fromASCIIString(argv[currentArgIndex]);
                        // Create a path and make sure that it is valid:
                        osFilePath projectPath(projectFileName);
                        gtString fileExtension, fileName;
                        osDirectory fileDirectory;
                        projectPath.getFileName(fileName);
                        projectPath.getFileExtension(fileExtension);
                        projectPath.getFileDirectory(fileDirectory);

                        if (fileExtension.isEmpty())
                        {
                            projectPath.setFileExtension(AF_STR_projectFileExtension);
                        }

                        if (fileDirectory.directoryPath().asString().isEmpty())
                        {
                            pApplicationCommnads->getProjectsFilePath(fileName, projectPath);
                        }

                        // Open the requested project:
                        pApplicationCommnads->OnFileOpenProject(projectPath.asString());

                        // For debug - open a source code file:
                        currentArgIndex++;

                        if (argc > currentArgIndex)
                        {
                            currentArg = argv[currentArgIndex];

                            if (currentArg == APP_NO_SPLASH_ARG) { currentArgIndex++; }

                            if (argc > currentArgIndex)
                            {
                                gtString sourceCodeFileName;
                                sourceCodeFileName.fromASCIIString(argv[currentArgIndex]);
                                osFilePath filePath(sourceCodeFileName);
                                pApplicationCommnads->OpenFileAtLine(filePath, 43, true);
                            }
                        }
                    }
                }

                // Open startup page:
                if (m_sApplicationStartupData.m_flagEnable.testFlag(AppCreate::APP_ENABLE_WELCOME_PAGE))
                {
                    pApplicationCommnads->OnFileOpenWelcomePage();
                }
            }

            // stop the splashScreen
            if (globalSplashScreen != NULL)
            {
                globalSplashScreen->finish(pAppMainWindow);
                delete globalSplashScreen;
                globalSplashScreen = NULL;
            }

            // Calculate the title bar's string:
            gtString titleBarString;
            afCalculateCodeXLTitleBarString(titleBarString);
            pApplicationCommnads->setApplicationCaption(titleBarString);

            // Force refresh:
            pAppMainWindow->activateWindow();

            // Create an action that allows us to save current layout to be used later:
            // After the layout is recorded it can be copied to the .h file that is used as the predefined layout
            // The process is arranging the views as the layout requires, recording it, and then copying it from the .ini file
            // which is C:\Users\username\AppData\Roaming\AMD\CodeXL\Layout
            // to the dialog that will open. in the dialog press the convert button. it will convert the string back to the layout
            // file and that string should be copied into the afPredefinedLayouts.h file.
            // the data that should be copied and converted is from the '(' to the ')' included.
            // when copying change the double @@ to @
#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
            QAction* layoutAction = new QAction("Debug record layout", pAppMainWindow);
            layoutAction->setShortcut(QKeySequence("Shift+F12"));
            pAppMainWindow->connect(layoutAction, SIGNAL(triggered()), pAppMainWindow, SLOT(onLayoutActionTriggered()));
            // add to the Tools menu bar:
            QMenu* pToolsMenu = pAppMainWindow->getActionMenuItemParentMenu(AF_STR_ToolsMenuString, NULL, false);
            pToolsMenu->addAction(layoutAction);
#endif

            // Check that different AMD components are present:
            gtString errorStr = afGlobalVariablesManager::instance().InstalledAMDComponentsErrorMessage();

            if (!errorStr.isEmpty())
            {
                OS_OUTPUT_DEBUG_LOG(errorStr.asCharArray(), OS_DEBUG_LOG_DEBUG);
                afApplicationCommands::instance()->AddStringToInformationView(acGTStringToQString(errorStr));
            }

            // Notify the user if there were problems in loading the plugins
            if (!validityString.isEmpty())
            {
                OS_OUTPUT_DEBUG_LOG(validityString.asCharArray(), OS_DEBUG_LOG_DEBUG);
                afApplicationCommands::instance()->AddStringToInformationView(acGTStringToQString(validityString));
            }

            pAppMainWindow->setInitialLayoutMode(afMainAppWindow::LayoutNoProjectOutput);
            pAppMainWindow->updateLayoutMode(afMainAppWindow::LayoutNoProjectOutput);

            // Start the Qt application message loop:
            retVal = qApp->exec();
        }

    }

    return retVal;

}

