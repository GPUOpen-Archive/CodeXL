//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This class manages all the traced API objects
//==============================================================================

#ifndef _HSA_FDN_API_INFO_MANAGER_H_
#define _HSA_FDN_API_INFO_MANAGER_H_

#include <hsa_ext_profiler.h>

#include <unordered_map>

#include "../Common/APIInfoManagerBase.h"
#include "HSAAPIBase.h"


class HSAAPIInfoManager :
    public APIInfoManagerBase, public TSingleton<HSAAPIInfoManager>
{
    friend class TSingleton<HSAAPIInfoManager>;

public:
    /// Add APIInfo to the list
    /// \param api APIInfo entry
    void AddAPIInfoEntry(APIBase* api);

    /// Destructor
    virtual ~HSAAPIInfoManager();

    /// Check if the specified API should be intercepted
    /// \param type HSA function type
    /// \return true if API should be intercepted
    bool ShouldIntercept(HSA_API_Type type) const;

    /// Adds the specified queue to the queue map
    /// \param pQueue the queue to add
    void AddQueue(const hsa_queue_t* pQueue);

    /// Gets the index of the specified queue
    /// \param pQueue the queue whose index is needed
    /// \param[out] queueIndex the index of the specified queue (if found)
    /// \return true if the queueu is known, false otherwise
    bool GetQueueIndex(const hsa_queue_t* pQueue, size_t& queueIndex) const;

protected:
    /// Flush non-API timestamp data to the output stream
    /// \param foutTS the output stream to write data to
    virtual void FlushNonAPITimestampData(std::ostream& sout);

    /// Add the specified api to the list of APIs to filter
    /// \param strAPIName the name of the API to add to the filter
    void AddAPIToFilter(const std::string& strAPIName);

private:
    /// Constructor
    HSAAPIInfoManager();

    /// Write kernel timestamp data to stream
    /// \param sout the output stream
    /// \param record the kernel timestamp record to write to the stream
    bool WriteKernelTimestampEntry(std::ostream& sout, hsa_profiler_kernel_time_t record);

    /// Check if specified API is in API filter list
    /// \param type HSA function type
    /// \return true if API is in filter list
    bool IsInFilterList(HSA_API_Type type) const;

    /// Return true if max number of APIs are traced.
    /// \return true if max number of APIs are traced.
    bool IsCapReached() const;

    typedef std::unordered_map<const hsa_queue_t*, size_t> QueueIndexMap;     ///< typedef for the queue index map
    typedef std::pair<const hsa_queue_t*, size_t>          QueueIndexMapPair; ///< typedef for the queue index pair

    unsigned int           m_tracedApiCount;     ///< number of APIs that have been traced, used to support max apis to trace option
    std::set<HSA_API_Type> m_filterAPIs;         ///< HSA APIs that are not traced due to API filtering
    std::set<HSA_API_Type> m_mustInterceptAPIs;  ///< HSA APIs that must be intercepted (even when they are filtered out and not traced)
    QueueIndexMap          m_queueIndexMap;      ///< map of a queue to that queue's index (basically creation order)
};

#endif // _HSA_FDN_API_INFO_MANAGER_H_
