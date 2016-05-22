#include "suSWMRInstance.h"
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <iostream>

boost::shared_mutex _access; ///< Boost shared mutex instance.

////////////////////////////////////////////////////////////////////////////////////
/// \brief Standard constructor. Hidden by private section
///
/// \author AMD Developer Tools Team
/// \date 11/05/2016
suSWMRInstance::suSWMRInstance(): bUniqLocked(false)
{
}

////////////////////////////////////////////////////////////////////////////////////
/// \brief Standard destructor. 
///
/// \author AMD Developer Tools Team
/// \date 11/05/2016
suSWMRInstance::~suSWMRInstance()
{
    UniqueUnLock();
}

////////////////////////////////////////////////////////////////////////////////////
/// \brief "Multiple read" shared lock
///
/// \author AMD Developer Tools Team
/// \date 11/05/2016
void suSWMRInstance::SharedLock()
{
    boost::shared_lock<boost::shared_mutex> lock(_access);
};

////////////////////////////////////////////////////////////////////////////////////
/// \brief "Single write" unique lock
///
/// \author AMD Developer Tools Team
/// \date 11/05/2016
void suSWMRInstance::UniqueLock()
{
    std::unique_lock<std::mutex>   lock(mtxUniqLockedVariable);
    _access.lock();
    bUniqLocked = true;
};

////////////////////////////////////////////////////////////////////////////////////
/// \brief "Single write" unique unlock
///
/// \author AMD Developer Tools Team
/// \date 11/05/2016
void suSWMRInstance::UniqueUnLock()
{
    std::unique_lock<std::mutex>   lock(mtxUniqLockedVariable);
     
    if (bUniqLocked)
    {
        _access.unlock();
        bUniqLocked = false;
    }
}

////////////////////////////////////////////////////////////////////////////////////
/// \brief Get singleton instance
///
/// \return Reference to the suSWMRInstance 
///
/// \author AMD Developer Tools Team
/// \date 11/05/2016
suSWMRInstance& suSWMRInstance::GetInstance()
{
    static suSWMRInstance _instance;

    return _instance;
};
