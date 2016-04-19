//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This class manages all simple API rules. We don't want to create a
///         separate CLAPIAnalyzer class for a rule -- simple API rules can be
///         defined by a callback function
//==============================================================================

#ifndef _SIMPLE_CLAPI_RULE_MANAGER_H_
#define _SIMPLE_CLAPI_RULE_MANAGER_H_

#include <string>
#include <vector>
#include "CLAPIAnalyzer.h"
#include "../CLCommon/CLFunctionEnumDefs.h"

#ifndef WIN32
    #define CALLBACK
#endif

/// CL API Rule callback function
/// \param info CL API Info object
/// \param[out] strMsg Message generated from this rule
/// \param[out] type Message type
/// \return true if rule applies
typedef bool (CALLBACK* CLAPIRuleCallbackFnPtr)(CLAPIInfo*,
                                                std::string&,
                                                APIAnalyzerMessageType&);

//------------------------------------------------------------------------------------
/// CL API Rule
//------------------------------------------------------------------------------------
struct CLAPIRule
{
    std::string name;                ///< rule name
    bool enabled;                    ///< flag indicating whether or not a rule is enabled
    CL_FUNC_TYPE type;               ///< API Type
    CLAPIRuleCallbackFnPtr callback; ///< API Callback
};

//------------------------------------------------------------------------------------
/// Create Command Queue Information class
//------------------------------------------------------------------------------------
class CreateCmdQueueInfo
{
public:
    std::string strContex;  ///< command queue contex
    std::string strDevice;  ///< command queue device
    std::string strProp;    ///< command queue property
    std::string strRetVal;  ///< command queue return value
    unsigned int ref_count; ///< command queue reference count
};

typedef std::map<const std::string, CreateCmdQueueInfo* > CmdQueueMap;
typedef std::pair<const std::string, CreateCmdQueueInfo* > CmdQueueMapPair;

//------------------------------------------------------------------------------------
/// Command Queue Information class
//------------------------------------------------------------------------------------
class CmdQueueInfoList
{
public:
    CmdQueueMap m_CmdQueueMap;

    /// Constructor
    CmdQueueInfoList(void);

    /// Destructor
    ~CmdQueueInfoList(void);

    /// Add to command queue info list
    /// \param pAPIInfo object
    void AddToCmdQInfoList(CLAPIInfo* pAPIInfo);

    /// Remove from command queue info list
    /// \param pAPIInfo object
    void RemoveFromCmdQInfoList(CLAPIInfo* pAPIInfo);

    /// Set reference count to command queue
    /// \param pAPIInfo object
    void SetRefCntInCmdQInfoList(CLAPIInfo* pAPIInfo);

    /// Find in command queue info list
    /// \param cl_cmd_queue std::string
    CreateCmdQueueInfo* FindInCmdQInfoList(std::string cl_cmd_queue);
};

//------------------------------------------------------------------------------------
/// Simple CL API Rule Manager class
//------------------------------------------------------------------------------------
class SimpleCLAPIRuleManager :
    public CLAPIAnalyzer
{
public:
    /// Constructor
    /// \param p CLAPIAnalyzerManager pointer
    SimpleCLAPIRuleManager(CLAPIAnalyzerManager* p);

    /// Destructor
    ~SimpleCLAPIRuleManager(void);

    /// Add rule
    /// \param rule CL API Rule
    void AddRule(CLAPIRule& rule)
    {
        m_rules.push_back(rule);
    }

    /// Add rule
    /// \param name Rule name
    /// \param type CL API Type
    /// \param callback rule call back function
    void AddRule(const char* name,
                 CL_FUNC_TYPE type,
                 CLAPIRuleCallbackFnPtr callback)
    {
        CLAPIRule r;
        r.name = name;
        r.enabled = true;
        r.type = type;
        r.callback = callback;
        m_rules.push_back(r);
    }

    /// Analyze API
    /// \param pAPIInfo APIInfo object
    void Analyze(APIInfo* pAPIInfo);

    /// Callback function for flattened APIs
    /// \param pAPIInfo APIInfo object
    void FlattenedAPIAnalyze(APIInfo* pAPIInfo);

    /// Generate APIAnalyzerMessage
    void EndAnalyze() {}

    /// Enable/Disable analyzers according to config file
    /// \param op AnalyzeOps object
    void SetEnable(const AnalyzeOps& op);

    CmdQueueInfoList* GetCmdQueueInfoList(void)
    {
        return m_pCmdQInfoList;
    }

private:
    /// Copy constructor
    /// \param obj object
    SimpleCLAPIRuleManager(const SimpleCLAPIRuleManager& obj);

    /// Assignment operator
    /// \param obj object
    /// \return ref to itself
    SimpleCLAPIRuleManager& operator = (const SimpleCLAPIRuleManager& obj);

private:
    std::vector<CLAPIRule> m_rules;  ///< rules array
    CmdQueueInfoList* m_pCmdQInfoList; ///< pointer to command queue information list
};

#endif //_SIMPLE_CLAPI_RULE_MANAGER_H_
