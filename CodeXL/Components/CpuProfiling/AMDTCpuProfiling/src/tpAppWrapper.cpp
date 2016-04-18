//------------------------------ tpAppWrapper.h ------------------------------

#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osApplication.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afQtCreatorsManager.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/ProfileApplicationTreeHandler.h>

// Local:
#include <inc/tpAppController.h>
#include <inc/tpAppWrapper.h>
#include <inc/tpStringConstants.h>
#include <inc/tpProjectSettingsExtension.h>
#include <inc/tpTreeHandler.h>
#include <inc/tpMenuActionsExecutor.h>
#include <inc/tpMDIViewCreator.h>


// Static members initialization:
tpAppWrapper* tpAppWrapper::m_spMySingleInstance = nullptr;
tpProjectSettingsExtension* tpAppWrapper::m_psProjectSettingsExtension = nullptr;

tpAppWrapper::tpAppWrapper()
{

}

tpAppWrapper& tpAppWrapper::Instance()
{
    // If this class single instance was not already created:
    if (m_spMySingleInstance == nullptr)
    {
        // Create it:
        m_spMySingleInstance = new tpAppWrapper;
        GT_ASSERT(m_spMySingleInstance);
    }

    return *m_spMySingleInstance;
}

int tpAppWrapper::CheckValidity(gtString& errString)
{
    GT_UNREFERENCED_PARAMETER(errString);
    int retVal = 0;

    return retVal;
}

void tpAppWrapper::initialize()
{
    // Create and register the project settings object:
    m_psProjectSettingsExtension = new tpProjectSettingsExtension;

    afProjectManager::instance().registerProjectSettingsExtension(m_psProjectSettingsExtension);

    // Create the main menu actions creator:
    tpMenuActionsExecutor* pActionsCreator = new tpMenuActionsExecutor;

    // Register the actions creator:
    afQtCreatorsManager::instance().registerActionExecutor(pActionsCreator);

    // Register the tree handler:
    ProfileApplicationTreeHandler::instance()->registerSessionTypeTreeHandler(TP_STR_ThreadProfileFileExtensionCaption, &tpTreeHandler::Instance());

    tpAppController::Instance();
    tpMDIViewCreator::Instance();

    // Register the views creator:
    afQtCreatorsManager::instance().registerViewCreator(&tpMDIViewCreator::Instance());
}

void tpAppWrapper::initializeIndependentWidgets()
{
}

extern "C"
{

    int CheckValidity(gtString& errString)
    {
        int retVal = tpAppWrapper::Instance().CheckValidity(errString);

        return retVal;
    }

    void initialize()
    {
        tpAppWrapper::Instance().initialize();
    }

    void TP_API initializeIndependentWidgets()
    {
        tpAppWrapper::Instance().initializeIndependentWidgets();
    }

}; // extern "C"

