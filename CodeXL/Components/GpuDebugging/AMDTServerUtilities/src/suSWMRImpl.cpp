
// Local:
#include <src/suSWMRImpl.h>

// STL:
#include <iostream>


////////////////////////////////////////////////////////////////////////////////////
/// \brief Standard constructor. Hidden by private section
///
/// \author AMD Developer Tools Team
/// \date 11/05/2016
suSWMRImpl::suSWMRImpl(): m_bUniqLocked(false)
{
}

////////////////////////////////////////////////////////////////////////////////////
/// \brief Standard destructor. 
///
/// \author AMD Developer Tools Team
/// \date 11/05/2016
suSWMRImpl::~suSWMRImpl()
{
    UniqueUnLock();
}

////////////////////////////////////////////////////////////////////////////////////
/// \brief "Multiple read" shared lock
///
/// \author AMD Developer Tools Team
/// \date 11/05/2016
void suSWMRImpl::SharedLock()
{
    m_mtxShared.lock_shared();
};

////////////////////////////////////////////////////////////////////////////////////
/// \brief "Multiple read" shared unlock
///
/// \author AMD Developer Tools Team
/// \date 11/05/2016
void suSWMRImpl::SharedUnLock()
{
    m_mtxShared.unlock_shared();
};


////////////////////////////////////////////////////////////////////////////////////
/// \brief "Single write" unique lock
///
/// \author AMD Developer Tools Team
/// \date 11/05/2016
void suSWMRImpl::UniqueLock()
{
    std::unique_lock<std::mutex>   lock(m_mtxUniqLockedVariable);

    if (!m_bUniqLocked)
    {
        m_mtxShared.lock();
        m_bUniqLocked = true;
    }
};

////////////////////////////////////////////////////////////////////////////////////
/// \brief "Single write" unique unlock
///
/// \author AMD Developer Tools Team
/// \date 11/05/2016
void suSWMRImpl::UniqueUnLock()
{
    std::unique_lock<std::mutex>   lock(m_mtxUniqLockedVariable);
     
    if (m_bUniqLocked)
    {
        m_mtxShared.unlock();
        m_bUniqLocked = false;
    }
}

////////////////////////////////////////////////////////////////////////////////////
/// \brief Get singleton instance
///
/// \return Reference to the suSWMRInstance 
///
/// \author AMD Developer Tools Team
/// \date 11/05/2016
suSWMRImpl& suSWMRImpl::GetInstance()
{
    static suSWMRImpl _instance;

    return _instance;
};
