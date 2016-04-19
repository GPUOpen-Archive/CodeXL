//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This analyzer class tracks api object references and generates warning messages
//==============================================================================

#ifndef _OBJ_REF_TRACKER_H_
#define _OBJ_REF_TRACKER_H_
//------------------------------------------------------------------------------------
/// Object Action
//------------------------------------------------------------------------------------
enum APIObjectAction
{
    API_OBJECT_ACTION_Create,   ///< Object Create... (increase ref count)
    API_OBJECT_ACTION_Retain,   ///< Object Retain... (increase ref count)
    API_OBJECT_ACTION_Release   ///< Object Release...(decrease ref count)
};

//------------------------------------------------------------------------------------
/// Object history
//------------------------------------------------------------------------------------
struct APIObjectHistory
{
    APIObjectAction m_action;      ///< Object Action
    int             m_iCurrentRef; ///< Current reference counter
    APIInfo*        m_pAPIInfoObj; ///< APIInfo object
};

typedef std::list<APIObjectHistory> APIObjHistoryList;
typedef std::map<std::string, APIObjHistoryList*> APITraceMap;

#endif // _OBJ_REF_TRACKER_H_
