
// Local:
#include <AMDTServerUtilities/Include/suSWMRInstance.h>
#include <src/suSWMRImpl.h>


////////////////////////////////////////////////////////////////////////////////////
/// \brief "Multiple read" shared lock
///
/// \author AMD Developer Tools Team
/// \date 11/05/2016
void suSWMRInstance::SharedLock()
{
    suSWMRImpl::GetInstance().SharedLock();
};

////////////////////////////////////////////////////////////////////////////////////
/// \brief "Multiple read" shared unlock
///
/// \author AMD Developer Tools Team
/// \date 11/05/2016
void suSWMRInstance::SharedUnLock()
{
    suSWMRImpl::GetInstance().SharedUnLock();
};


////////////////////////////////////////////////////////////////////////////////////
/// \brief "Single write" unique lock
///
/// \author AMD Developer Tools Team
/// \date 11/05/2016
void suSWMRInstance::UniqueLock()
{
    suSWMRImpl::GetInstance().UniqueLock();
};

////////////////////////////////////////////////////////////////////////////////////
/// \brief "Single write" unique unlock
///
/// \author AMD Developer Tools Team
/// \date 11/05/2016
void suSWMRInstance::UniqueUnLock()
{
    suSWMRImpl::GetInstance().UniqueUnLock();
}

////////////////////////////////////////////////////////////////////////////////////
/// \brief Set the suSWMRImpl to unlock state. Every UniqLock call will be deferred
///
/// \author AMD Developer Tools Team
/// \date 02/08/2016
void suSWMRInstance::SetUnlockMode()
{
    suSWMRImpl::GetInstance().SetUnlockMode();
}

////////////////////////////////////////////////////////////////////////////////////
/// \brief Reset the suSWMRImpl to unlock state. Every UniqLock call will be deferred
///
/// \author AMD Developer Tools Team
/// \date 02/08/2016
void suSWMRInstance::ResetUnlockMode()
{
    suSWMRImpl::GetInstance().ResetUnlockMode();
}

