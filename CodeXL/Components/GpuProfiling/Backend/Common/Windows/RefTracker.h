//=====================================================================
//
// Author: Raul Aguaviva
//         Graphics Products Group
//         Developer Tools
//         AMD, Inc.
//
// misc.h
// File contains <Raul file description here>
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/Backend/Common/Windows/RefTracker.h#4 $
//
// Last checkin:  $DateTime: 2014/07/17 21:57:02 $
// Last edited by: $Author: chesik $
//=====================================================================
//   ( C ) AMD, Inc.  All rights reserved.
//=====================================================================

#ifndef _REF_TRACKER_H_
#define _REF_TRACKER_H_

#include <map>
#include "AMDTMutex.h"

/// \addtogroup Common
// @{

//
/// This class maintains a reference count against a thread id
//
class RefTrackerCounter
{
    bool m_IsUsingExternalMutex;             ///< set to true if using an external mutex, false otherwise
    AMDTMutex* m_pmutex;                     ///< a mutex (can be a handle to an external mutex)
    std::map<DWORD, int> m_mapInsideWrapper; ///< holds a mapping of a thread id and reference count

public:
    /// Constructor
    RefTrackerCounter();

    /// Destructor
    ~RefTrackerCounter();

    /// Constructor
    /// \param[in] pM  an input mutex
    RefTrackerCounter(AMDTMutex* pM);

    /// Specify to use an external mutex
    /// \param[in] pM  the input external mutex
    void UseExternalMutex(AMDTMutex* pM);

    /// Increment the reference count
    void operator++(int);

    /// Decrement the reference count
    void operator--(int);

    /// Equality operator
    /// \param[in] v  a reference count
    bool operator==(DWORD v);

    /// Greater operator
    /// \param[in] v  an input reference count
    bool operator>(DWORD v);

    /// Get the reference count
    /// \return the reference count
    DWORD GetRef();

private:
    /// Disable copy constructor
    /// \param[in] rhs  the input object
    RefTrackerCounter(const RefTrackerCounter& rhs);

    /// Disable assignment operator
    /// \param[in] rhs  the input object
    /// \return a reference to the object
    RefTrackerCounter& operator=(const RefTrackerCounter& rhs);
};

/// This class handles reference tracking
class RefTracker
{
    /// pointer to an unsigned long that contains the number of references
    RefTrackerCounter* m_dwVal;

public:
    /// increments dwVal on creation
    /// \param[in,out] dwVal  the reference counter object
    RefTracker(RefTrackerCounter* dwVal);

    /// decrements dwVal on destruction
    ~RefTracker()
    {
        //
        // Protect against the app doing something stupid (like releasing a
        // resource more than once)
        if ((*m_dwVal) > 0)
        {
            (*m_dwVal)--;
        }
        else
        {
            //Log( logWARNING, "RefTracker destructor called with m_dwVal == 0\n" );
        }
    }
};

// @}

#endif //_REF_TRACKER_H_