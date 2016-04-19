#ifndef dbTxCommitThread_h__
#define dbTxCommitThread_h__

#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTProfilerDAL/include/AMDTDatabaseAccessor.h>

namespace AMDTProfilerDAL
{

class dbTxCommitThread : public osThread
{
public:
    dbTxCommitThread(AMDTProfilerDAL::AmdtDatabaseAccessor* pDataAdapter);
    ~dbTxCommitThread();

    virtual int entryPoint();

    bool TriggerDbTxCommit();
    void requestExit() { m_isRequestExit = true; }

private:
    AmdtDatabaseAccessor* m_pDataAdapter;

    // Indicates whether a DB commit is required.
    bool m_isDbCommitTriggerRequired;
    bool m_isRequestExit;
};
}
#endif // dbTxCommitThread_h__
