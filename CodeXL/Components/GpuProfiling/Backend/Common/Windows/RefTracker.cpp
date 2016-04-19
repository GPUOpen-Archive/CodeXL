#include "RefTracker.h"
#include <assert.h>
//
/// This class helps detect reentrance problems
//
RefTrackerCounter::RefTrackerCounter()
{
    m_IsUsingExternalMutex = false;
    m_pmutex = new AMDTMutex();
}

RefTrackerCounter::~RefTrackerCounter()
{
    if (m_pmutex)
    {
        delete m_pmutex;
    }
}

RefTrackerCounter::RefTrackerCounter(AMDTMutex* pM)
{
    m_IsUsingExternalMutex = true;
    assert(pM);
    m_pmutex = pM;
}

void RefTrackerCounter::UseExternalMutex(AMDTMutex* pM)
{
    if (m_IsUsingExternalMutex == false)
    {
        delete m_pmutex;
    }

    m_IsUsingExternalMutex = true;

    m_pmutex = pM;


}

//
// Returns a thread's ID, this function helps debugging by, for example forcing the returned value to be always the same
//
static DWORD GetThreadsID()
{
    return GetCurrentThreadId();
}

void RefTrackerCounter::operator++(int)
{
    AMDTScopeLock lock(m_pmutex);

    DWORD dwThreadId = GetThreadsID();
    std::map<DWORD, int>::iterator it = m_mapInsideWrapper.find(dwThreadId);

    if (it != m_mapInsideWrapper.end())
    {
        it->second++;
    }
    else
    {
        m_mapInsideWrapper[dwThreadId] = 1;
    }
}

void RefTrackerCounter::operator--(int)
{
    AMDTScopeLock lock(m_pmutex);

    DWORD dwThreadId = GetThreadsID();
    std::map<DWORD, int>::iterator it = m_mapInsideWrapper.find(dwThreadId);

    if (it != m_mapInsideWrapper.end())
    {
        it->second--;
    }
    else
    {
        //not found? this should be impossible, assert!!
        assert(false);
    }
}

bool RefTrackerCounter::operator==(DWORD v)
{
    AMDTScopeLock lock(m_pmutex);

    DWORD dwThreadId = GetThreadsID();
    std::map<DWORD, int>::iterator it = m_mapInsideWrapper.find(dwThreadId);

    if (it != m_mapInsideWrapper.end())
    {
        return (DWORD)it->second == v;
    }

    //not found? then its value is zero
    return (v == 0);
}

bool RefTrackerCounter::operator>(DWORD v)
{
    AMDTScopeLock lock(m_pmutex);

    DWORD dwThreadId = GetThreadsID();
    std::map<DWORD, int>::iterator it = m_mapInsideWrapper.find(dwThreadId);

    if (it != m_mapInsideWrapper.end())
    {
        return (DWORD)it->second > v;
    }

    return (v == 0);
}

DWORD RefTrackerCounter::GetRef()
{
    AMDTScopeLock lock(m_pmutex);

    DWORD dwThreadId = GetThreadsID();
    std::map<DWORD, int>::iterator it = m_mapInsideWrapper.find(dwThreadId);

    if (it != m_mapInsideWrapper.end())
    {
        return it->second;
    }

    //zero
    return 0;
}

RefTracker::RefTracker(RefTrackerCounter* dwVal)
{
    m_dwVal = dwVal;

    assert(m_dwVal);

    (*m_dwVal)++;
}