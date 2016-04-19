//------------------------------ kaGlobalVariableManager.cpp ------------------------------

// infra
#include <AMDTBaseTools/Include/gtAssert.h>

// local
#include <AMDTKernelAnalyzer/src/kaGlobalVariableManager.h>

// Static member initializations:
kaGlobalVariableManager* kaGlobalVariableManager::m_psMySingleInstance = NULL;


// ---------------------------------------------------------------------------
// Name:        kaGlobalVariableManager::kaGlobalVariableManager
// Description: constructor
// Author:      Gilad Yarnitzky
// Date:        30/7/2013
// ---------------------------------------------------------------------------
kaGlobalVariableManager::kaGlobalVariableManager()
{

}

// ---------------------------------------------------------------------------
// Name:        kaGlobalVariableManager::~kaGlobalVariableManager
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        30/7/2013
// ---------------------------------------------------------------------------
kaGlobalVariableManager::~kaGlobalVariableManager()
{
}

// ---------------------------------------------------------------------------
// Name:        kaGlobalVariableManager::instance
// Description: Returns the single instance of this class.
//              (If it does not exist - create it)
// Author:      Gilad Yarnitzky
// Date:        30/7/2013
// ---------------------------------------------------------------------------
kaGlobalVariableManager& kaGlobalVariableManager::instance()
{
    if (m_psMySingleInstance == NULL)
    {
        m_psMySingleInstance = new kaGlobalVariableManager;
        GT_ASSERT(m_psMySingleInstance);
    }

    return *m_psMySingleInstance;
}


// ---------------------------------------------------------------------------
// Name:        kaGlobalVariableManager::setDefaultTreeList
// Description: sets the default tree list and current tree list
// Arguments:   QStringList& iDefaultTreeList
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        4/8/2013
// ---------------------------------------------------------------------------
void kaGlobalVariableManager::setDefaultTreeList(QStringList& iDefaultTreeList)
{
    m_defaultTreeList = iDefaultTreeList;
    m_currentTreeList = iDefaultTreeList;
}

