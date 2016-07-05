//------------------------------ osCommunicationDebugManager.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Local:
#include <AMDTOSWrappers/Include/osCommunicationDebugManager.h>
#include <AMDTOSWrappers/Include/osCommunicationDebugThread.h>
#include <AMDTOSWrappers/Include/osDoubleBufferQueue.h>
#include <AMDTOSWrappers/Include/osTimeInterval.h>

osCommunicationDebugManager::destroyer osCommunicationDebugManager::m_destroyer;
osCommunicationDebugManager* osCommunicationDebugManager::m_spCommunicationDebugManager = nullptr;
osCriticalSection osCommunicationDebugManager::m_creationCriticalSection;

// ---------------------------------------------------------------------------
// Name:        osCommunicationDebugManager::osCommunicationDebugManager
// Description: Default constructor.
// Author:      Doron Ofek
// Date:        Dec-20, 2015
// ---------------------------------------------------------------------------
osCommunicationDebugManager::osCommunicationDebugManager()
    : m_isCommunicationDebugEnabled(false)
    , m_pDebugThread(nullptr)
    , m_pDebugQ(nullptr)
{
    // Set the static pointer to this because the debug thread uses it to access the debug queue
    m_spCommunicationDebugManager = this;
    m_pDebugQ = new osDoubleBufferQueue<gtString>;
    GT_ASSERT(m_pDebugQ != nullptr);

    m_pDebugThread = new osCommunicationDebugThread;

    GT_IF_WITH_ASSERT(m_pDebugThread != nullptr)
    {
        m_pDebugThread->execute();
    }
}


// ---------------------------------------------------------------------------
// Name:        osCommunicationDebugManager::~osCommunicationDebugManager
// Description: Destructor
// Author:      Doron Ofek
// Date:        Dec-20, 2015
// ---------------------------------------------------------------------------
osCommunicationDebugManager::~osCommunicationDebugManager()
{
    const int debugThreadSelfEndTimeout = 5000;
    m_isCommunicationDebugEnabled = false;
    GT_IF_WITH_ASSERT(m_pDebugThread != nullptr)
    {
        m_pDebugThread->requestExit();
        osTimeInterval timeout;
        timeout.setAsMilliSeconds(debugThreadSelfEndTimeout);
        m_pDebugThread->waitForThreadEnd(timeout);
        m_pDebugThread->terminate();
    }

    if (m_pDebugThread != nullptr)
    {
        delete m_pDebugThread;
        m_pDebugThread = nullptr;
    }

    if (m_pDebugQ != nullptr)
    {
        delete m_pDebugQ;
        m_pDebugQ = nullptr;
    }

}

// ---------------------------------------------------------------------------
// Name:        osCommunicationDebugManager::instance()
// Description: Singleton instance function.
// Author:      Doron Ofek
// Date:        Dec-20, 2015
// ---------------------------------------------------------------------------
osCommunicationDebugManager& osCommunicationDebugManager::instance()
{
    if (nullptr == m_spCommunicationDebugManager)
    {
        osCriticalSectionLocker guard(m_creationCriticalSection);

        if (nullptr == m_spCommunicationDebugManager)
        {
            m_spCommunicationDebugManager = new osCommunicationDebugManager;
            m_destroyer.SetSingletonPointerToPointer(&m_spCommunicationDebugManager);
        }
    }

    return *m_spCommunicationDebugManager;
}

// ---------------------------------------------------------------------------
// Name:        osCommunicationDebugManager::push()
// Description: Push a message containing debug info into the queue
// Author:      Doron Ofek
// Date:        Dec-20, 2015
// ---------------------------------------------------------------------------
void osCommunicationDebugManager::push(const gtString& debugString)
{
    GT_IF_WITH_ASSERT(m_pDebugQ != nullptr)
    {
        m_pDebugQ->push(debugString);
    }
}