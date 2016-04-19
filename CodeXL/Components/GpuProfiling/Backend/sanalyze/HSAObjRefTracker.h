//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This analyzer class tracks HSA object create, release and generates warning messages
//==============================================================================

#ifndef _HSA_OBJ_REF_TRACKER_H_
#define _HSA_OBJ_REF_TRACKER_H_

#include <map>
#include <string>
#include <list>
#include "HSAAPIAnalyzer.h"
#include "ObjRefTracker.h"

//------------------------------------------------------------------------------------
/// HSA Reference tracker
//------------------------------------------------------------------------------------
class HSAObjRefTracker : public HSAAPIAnalyzer
{
public:
    /// Constructor
    HSAObjRefTracker();

    /// Destructor
    ~HSAObjRefTracker(void);

    /// Analyze API
    /// \param pAPIInfo APIInfo object
    void Analyze(APIInfo* pAPIInfo);

    /// Callback function for flattened APIs
    /// \param pAPIInfo APIInfo object
    void FlattenedAPIAnalyze(APIInfo* pAPIInfo);

    /// Generate APIAnalyzerMessage
    void EndAnalyze();

    /// Override clear
    void Clear();

private:
    /// Copy constructor
    /// \param obj object
    HSAObjRefTracker(const HSAObjRefTracker& obj);

    /// Assignment operator
    /// \param obj object
    /// \return ref to itself
    const HSAObjRefTracker& operator = (const HSAObjRefTracker& obj);

    /// Helper function that convert APIObjHistoryList to string
    /// \param list APIObjHistoryList object
    /// \return string representation of APIObjHistoryList
    std::string APIObjHistoryListToString(APIObjHistoryList* list);

    /// Helper function that records resource creation APIs
    /// \param pAPIInfo APIInfo object
    /// \param shouldCheckRetVal if true, resource creation will only be recorded if the API's return code indicates success
    void RecordResourceCreate(HSAAPIInfo* pAPIInfo, bool shouldCheckRetVal = true);

    /// Helper function that records resource free APIs
    /// \param pAPIInfo APIInfo object
    /// \param shouldCheckRetVal if true, resource creation will only be recorded if the API's return code indicates success
    void RecordResourceFree(HSAAPIInfo* pAPIInfo, bool shouldCheckRetVal = true);

    /// Get the resource handle string for the object whose resource is being tracked
    /// \param pAPIInfo the api info instance whose parameter is the a resource begin tracked
    /// \return string representation of the resource beign tracked
    std::string GetResourceHandle(HSAAPIInfo* pAPIInfo);

    static const std::string ms_HSA_RUNTIME_REF;    ///< a special tag to allow tracking of hsa_init/hsa_shut_down
    static const std::string ms_HSA_STATUS_SUCCESS; ///< "HSA_STATUS_SUCCESS" string

    APITraceMap m_objRefHistoryMap;   ///< HSA Object reference history map

    /// map from HSA API to the out parameter index that contains the resource to track
    std::map<HSA_API_Type, unsigned int> m_resourceArgMap;
};

#endif //_HSA_OBJ_REF_TRACKER_H_
