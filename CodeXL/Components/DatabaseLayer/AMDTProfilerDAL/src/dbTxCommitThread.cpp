#include <AMDTProfilerDAL/include/dbTxCommitThread.h>
#include <AMDTProfilerDAL/include/AMDTDatabaseAccessor.h>

// Infra.
#include <AMDTBaseTools/Include/gtAssert.h>

namespace AMDTProfilerDAL
{
dbTxCommitThread::dbTxCommitThread(AMDTProfilerDAL::AmdtDatabaseAccessor* pDataAdapter) : osThread(L"DB TRANSACTION COMMIT THREAD"),
    m_pDataAdapter(pDataAdapter), m_isDbCommitTriggerRequired(false), m_isRequestExit(false)
{
}

dbTxCommitThread::~dbTxCommitThread()
{
}

int dbTxCommitThread::entryPoint()
{
    GT_IF_WITH_ASSERT(m_pDataAdapter != nullptr)
    {
        // Check if a DB COMMIT is required every 100 ms.
        const unsigned SLEEP_INTERVAL_MS = 100;

        while (! m_isRequestExit)
        {
            osSleep(SLEEP_INTERVAL_MS);

            if (m_isDbCommitTriggerRequired)
            {
                // Turn off the signal.
                m_isDbCommitTriggerRequired = false;

                if (m_pDataAdapter != nullptr)
                {
                    // Flush the data.
                    m_pDataAdapter->FlushData();
                }
                else
                {
                    // Stop the loop, our data adapter is dead.
                    break;
                }
            }
        }
    }

    return 0;
}

bool dbTxCommitThread::TriggerDbTxCommit()
{
    m_isDbCommitTriggerRequired = true;
    return true;
}
}
