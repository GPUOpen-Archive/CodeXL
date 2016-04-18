//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This analyzer class tracks opencl object create, retain and release and generates warning messages
//==============================================================================

#ifndef _CL_OBJ_REF_TRACKER_H_
#define _CL_OBJ_REF_TRACKER_H_

#include <map>
#include <string>
#include <list>
#include "CLAPIAnalyzer.h"
#include "ObjRefTracker.h"

//------------------------------------------------------------------------------------
/// OpenCL Reference tracker
//------------------------------------------------------------------------------------
class CLObjRefTracker :
    public CLAPIAnalyzer
{
public:
    /// Constructor
    /// \param p CLAPIAnalyzerManager pointer
    CLObjRefTracker(CLAPIAnalyzerManager* p);

    /// Destructor
    ~CLObjRefTracker(void);

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

    /// Helper function that extract event handle from argument list
    /// \param pAPIInfo API Info object
    /// \return Event object handle string
    static std::string GetEventHandle(CLAPIInfo* pAPIInfo);

private:
    /// Helper function that convert APIObjHistoryList to string
    /// \param list APIObjHistoryList object
    /// \return string representation of APIObjHistoryList
    std::string APIObjHistoryListToString(APIObjHistoryList* list);

    /// Helper function that record clCreate API
    /// \param strHandle object handle string
    /// \param pAPIInfo APIInfo object
    void AddCLCreate(std::string strHandle, CLAPIInfo* pAPIInfo);

    /// Helper function that record clRetain/clRelease API
    /// \param strHandle object handle string
    /// \param pAPIInfo APIInfo object
    /// \param action Retain or Release
    /// \param toBeAdd either 1 or -1
    void UpdateCLRefCounter(std::string strHandle, CLAPIInfo* pAPIInfo, APIObjectAction action, int toBeAdd);

    /// Copy constructor
    /// \param obj object
    CLObjRefTracker(const CLObjRefTracker& obj);

    /// Assignment operator
    /// \param obj object
    /// \return ref to itself
    const CLObjRefTracker& operator = (const CLObjRefTracker& obj);

private:
    APITraceMap m_objRefHistoryMap;   ///< CL Object reference history map
};

#endif //_CL_OBJ_REF_TRACKER_H_
