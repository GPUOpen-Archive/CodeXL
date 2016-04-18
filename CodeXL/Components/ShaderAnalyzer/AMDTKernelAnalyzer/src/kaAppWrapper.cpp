//------------------------------ kaAppWrapper.h ------------------------------

#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osApplication.h>

// Application Framework:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afQtCreatorsManager.h>
#include <AMDTApplicationFramework/Include/afPluginConnectionManager.h>
#include <AMDTApplicationFramework/Include/commands/afSystemInformationCommand.h>

// Local:
#include <AMDTKernelAnalyzer/Include/kaAMDTKernelAnalyzerDLLBuild.h>
#include <AMDTKernelAnalyzer/Include/kaAppWrapper.h>
#include <AMDTKernelAnalyzer/Include/kaExecutionMode.h>
#include <AMDTKernelAnalyzer/Include/kaStringConstants.h>
#include <AMDTKernelAnalyzer/src/kaAnalyzeSettingsPage.h>
#include <AMDTKernelAnalyzer/src/kaApplicationTreeHandler.h>
#include <AMDTKernelAnalyzer/src/kaBackendManager.h>
#include <AMDTKernelAnalyzer/src/kaBuildToolbar.h>
#include <AMDTKernelAnalyzer/src/kaEventObserver.h>
#include <AMDTKernelAnalyzer/src/kaGlobalVariableManager.h>
#include <AMDTKernelAnalyzer/src/kaMenuActionsExecutor.h>
#include <AMDTKernelAnalyzer/src/kaMDIViewCreator.h>
#include <AMDTKernelAnalyzer/src/kaProjectSettingsExtension.h>
#include <AMDTKernelAnalyzer/src/kaProjectSettingsExtensionOther.h>
#include <AMDTKernelAnalyzer/src/kaProjectSettingsShaderExtension.h>

// Static members:
kaAppWrapper* kaAppWrapper::m_spMySingleInstance = NULL;
kaAnalyzeSettingsPage* kaAppWrapper::m_spAnalyzeSettingsPage = NULL;
kaEventObserver* kaAppWrapper::m_spApplicationEventObserver = NULL;
kaExecutionMode* kaAppWrapper::m_spKAExecutionMode = NULL;
kaProjectSettingsExtension* kaAppWrapper::m_psProjectSettingsExtension = NULL;
kaProjectSettingsExtensionOther* kaAppWrapper::m_psProjectSettingsExtensionOther = NULL;
kaProjectSettingsShaderExtension* kaAppWrapper::m_psProjectSettingsShaderExtension = NULL;
kaMDIViewCreator* kaAppWrapper::m_psMDIViewCreator = NULL;
kaBuildToolbar* kaAppWrapper::m_psBuildToolbar = NULL;

/// s_loadEnabled is by default true. Only in SA, when we dynamically load the dll,
/// the flag can be false
bool kaAppWrapper::s_loadEnabled = true;

// ---------------------------------------------------------------------------
// Name:        kaAppWrapper::kaAppWrapper
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        20/7/2011
// ---------------------------------------------------------------------------
kaAppWrapper::kaAppWrapper()
{

}
// ---------------------------------------------------------------------------
// Name:        kaAppWrapper::instance
// Description:
// Return Val:  kaAppWrapper&
// Author:      Sigal Algranaty
// Date:        20/7/2011
// ---------------------------------------------------------------------------
kaAppWrapper& kaAppWrapper::instance()
{
    // If this class single instance was not already created:
    if (m_spMySingleInstance == NULL)
    {
        // Create it:
        m_spMySingleInstance = new kaAppWrapper;
        GT_ASSERT(m_spMySingleInstance);
    }

    return *m_spMySingleInstance;
}

// ---------------------------------------------------------------------------
// Name:        CheckValidity
// Description: check validity of the plug in
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        14/7/2014
// ---------------------------------------------------------------------------
int kaAppWrapper::CheckValidity(gtString& errString)
{
    int retVal = 0;

    unsigned int installedComponentsBitmask = afGlobalVariablesManager::instance().InstalledAMDComponentsBitmask();

    if (!((installedComponentsBitmask & AF_AMD_GPU_COMPONENT) && (installedComponentsBitmask & AF_AMD_CATALYST_COMPONENT)))
    {
        retVal = 1;
        // the error message is displayed in afGlobalVariableManager for all components
        errString = AF_STR_Empty;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        initialize
// Description: Entry point for initialize
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        17/7/2011
// ---------------------------------------------------------------------------
void kaAppWrapper::initialize()
{
    // Create the main menu actions creator:
    kaMenuActionsExecutor* pActionsCreator = new kaMenuActionsExecutor;


    // Register the actions creator:
    afQtCreatorsManager::instance().registerActionExecutor(pActionsCreator);

    // initialize the kaBackendManager, must be initialized before the target device:
    QStringList asicTreeList;
    kaBackendManager::instance().getASICsTreeList(beKA::DeviceTableKind_OpenCL, false, &asicTreeList);
    kaGlobalVariableManager::instance().setDefaultTreeList(asicTreeList);

    // Create and register the global settings page:
    m_spAnalyzeSettingsPage = new kaAnalyzeSettingsPage;

    afGlobalVariablesManager::instance().registerGlobalSettingsPage(m_spAnalyzeSettingsPage);

    // 9/9/2013 Sigal: the execution modes needs to register before the gwEventObserver
    // Register the execution mode manager
    m_spKAExecutionMode = new kaExecutionMode;

    afExecutionModeManager::instance().registerExecutionMode(m_spKAExecutionMode);

    // Create an event observer:
    m_spApplicationEventObserver = new kaEventObserver;


    // Register the run mode manager:
    afPluginConnectionManager::instance().registerRunModeManager(m_spApplicationEventObserver);

    // Create and register the project settings object:
    m_psProjectSettingsExtension = new kaProjectSettingsExtension;

    afProjectManager::instance().registerProjectSettingsExtension(m_psProjectSettingsExtension);

    // Create and register the project settings object:
    m_psProjectSettingsExtensionOther = new kaProjectSettingsExtensionOther;

    afProjectManager::instance().registerProjectSettingsExtension(m_psProjectSettingsExtensionOther);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    // Create and register the project settings object:
    m_psProjectSettingsShaderExtension = new kaProjectSettingsShaderExtension;

    afProjectManager::instance().registerProjectSettingsExtension(m_psProjectSettingsShaderExtension);
#endif

    // Create a debug views creator:
    m_psMDIViewCreator = new kaMDIViewCreator;


    // Initialize (icons):
    m_psMDIViewCreator->initialize();

    // Register the views creator:
    afQtCreatorsManager::instance().registerViewCreator(m_psMDIViewCreator);
}

// ---------------------------------------------------------------------------
// Name:        gwgDEBuggerAppWrapper::initializeIndependentWidgets
// Description: This function initializes all the widget items that are not
//              registered with the creators mechanism. These widgets are
//              responsible for its own callbacks and string
// Author:      Sigal Algranaty
// Date:        15/9/2011
// ---------------------------------------------------------------------------
void kaAppWrapper::initializeIndependentWidgets()
{
    // Get the main application window:
    afMainAppWindow* pAppMainWindow = afMainAppWindow::instance();
    GT_IF_WITH_ASSERT(pAppMainWindow != NULL)
    {
        m_psBuildToolbar = new kaBuildToolbar(pAppMainWindow);


        m_psBuildToolbar->setVisible(false);

        // Get the image and buffers toolbar
        gtString toolbarName = AF_STR_ImagesAndBuffersToolbar;
        QString toolbarNameQstr = QString::fromWCharArray(toolbarName.asCharArray());
        toolbarNameQstr.append(AF_STR_toolbarPostfix);
        QToolBar* pImagesAndBuffersToolbar = pAppMainWindow->findChild<QToolBar*>(toolbarNameQstr);

        pAppMainWindow->addToolbar(m_psBuildToolbar, (acToolBar*)pImagesAndBuffersToolbar);
    }
}

extern "C"
{
    // ---------------------------------------------------------------------------
    // Name:        CheckValidity
    // Description: check validity of the plug in
    // Return Val:  int - error type
    //              0 = no error
    //              != 0 error value and error string contains the error
    // Author:      Gilad Yarnitzky
    // Date:        1/7/2014
    // ---------------------------------------------------------------------------
    int CheckValidity(gtString& errString)
    {
        int retVal = kaAppWrapper::instance().CheckValidity(errString);

        kaAppWrapper::s_loadEnabled = (0 == retVal);

        return retVal;
    }

    // ---------------------------------------------------------------------------
    // Name:        initialize
    // Description: Entry point for initialize
    // Return Val:  void
    // Author:      Gilad Yarnitzky
    // Date:        17/7/2011
    // ---------------------------------------------------------------------------
    void initialize()
    {
        kaAppWrapper::instance().initialize();
    }


    // ---------------------------------------------------------------------------
    // Name:        initializeIndependentWidgets
    // Description: initialize other gui items that can be done only after main window is alive
    // Return Val:  void GW_API
    // Author:      Gilad Yarnitzky
    // Date:        26/1/2012
    // ---------------------------------------------------------------------------
    void KA_API initializeIndependentWidgets()
    {
        kaAppWrapper::instance().initializeIndependentWidgets();
    }

}; // extern "C"

