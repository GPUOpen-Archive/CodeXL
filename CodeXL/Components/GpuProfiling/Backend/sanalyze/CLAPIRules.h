//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file provides an interface to get SimpleCLAPIRuleManager and
///         defines the callbacks for the simple rules
//==============================================================================

#ifndef _CL_API_RULES_H_
#define _CL_API_RULES_H_

#include "SimpleCLAPIRuleManager.h"

/// Get Simple CLAPI Rule manager
/// \return pointer to SimpleCLAPIRuleManager
SimpleCLAPIRuleManager* GetSimpleCLAPIRuleManager();

/// Rule callback for the blocking write rule
/// \param info an API to analyze
/// \param[out] strMsg the message related to this api
/// \param[out] type the type of message geenrated (error/warning/betsPractice)
/// \return true if a message is generated, false otherwise
bool CALLBACK CLAPIRule_BlockingWrite(CLAPIInfo* info, std::string& strMsg, APIAnalyzerMessageType& type);

/// Rule callback for the bad global/local work size
/// \param info an API to analyze
/// \param[out] strMsg the message related to this api
/// \param[out] type the type of message geenrated (error/warning/betsPractice)
/// \return true if a message is generated, false otherwise
bool CALLBACK CLAPIRule_BadWorkSize(CLAPIInfo* info, std::string& strMsg, APIAnalyzerMessageType& type);


#endif //_CL_API_RULES_H_
