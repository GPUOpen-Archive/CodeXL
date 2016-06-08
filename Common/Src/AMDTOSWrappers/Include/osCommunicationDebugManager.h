//------------------------------ osCommunicationDebugManager.h ------------------------------

#ifndef __OS_COMMUNICATION_DEBUG_MANAGER
#define __OS_COMMUNICATION_DEBUG_MANAGER

#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osCriticalSection.h>
#include <AMDTOSWrappers/Include/osDoubleBufferQueue.h>

class osCommunicationDebugThread;
typedef osDoubleBufferQueue<gtString> osCommunicationDebugQueue;



// ----------------------------------------------------------------------------------
// Class Name:           osCommunicationDebugManager
// General Description:
//   A singleton that contains a queue for storing debug information about read and write
//   operations that osChannel-derived classes perform. A debug thread checks if there is
//   data in the queue and sends it to a debug server.
//
// Author:               Doron Ofek
// Creation Date:        Dec-20, 2015
// ----------------------------------------------------------------------------------
class osCommunicationDebugManager
{
    friend class osCommunicationDebugThread;


public:
    static osCommunicationDebugManager& instance();
    bool isCommunicationDebugEnabled() const { return m_isCommunicationDebugEnabled; }
    void push(const gtString& debugString);

private:
    // Hide the default constructor. This class is a singleton and should not be instantiated
    osCommunicationDebugManager();
    ~osCommunicationDebugManager();

    bool m_isCommunicationDebugEnabled;
    osCommunicationDebugThread* m_pDebugThread;
    osCommunicationDebugQueue* m_pDebugQ;

    /// This class automatically destroys the singleton instance when the process is closed
    class destroyer
    {
    public:
        ~destroyer()
        {
            if (m_pPointerToPointer != nullptr && *m_pPointerToPointer != nullptr)
            {
                osCommunicationDebugManager* pSingleton = *m_pPointerToPointer;
                delete pSingleton;
                *m_pPointerToPointer = nullptr;
            }
        }
        void SetSingletonPointerToPointer(osCommunicationDebugManager** pp)
        {
            m_pPointerToPointer = pp;
        }

        osCommunicationDebugManager** m_pPointerToPointer;
    };
    friend class osCommunicationDebugManager::destroyer;
    static osCommunicationDebugManager::destroyer m_destroyer;
    static osCommunicationDebugManager* m_spCommunicationDebugManager;
    static osCriticalSection m_creationCriticalSection;
};

// A macro for nicer reference to the singleton
#ifndef theCommunicationDebugManager
    #define theCommunicationDebugManager osCommunicationDebugManager::instance()
#endif // theCommunicationDebugManager


#endif  // __OS_COMMUNICATION_DEBUG_MANAGER
