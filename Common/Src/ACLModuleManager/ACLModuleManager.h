//==============================================================================
// Copyright (c) 2015-2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file manages multiple instances of the ACL Module
//==============================================================================

#ifndef _ACL_MODULE_MANAGER_H_
#define _ACL_MODULE_MANAGER_H_

#include "TSingleton.h"
#include "ACLModule.h"

/// Singleton class to manage access to the two ACL Modules available in driver 15.20
class ACLModuleManager : public TSingleton<ACLModuleManager>
{
    friend class TSingleton<ACLModuleManager>;

public:
    /// Gets the appropriate acl module and acl compiler instance based on the isOCL2 flag
    /// \param useHSAILPath flag indicating whether or not the ACL module for the HSAIL backend is needed
    /// \param[out] pAclModule the ACL module instance
    /// \param[out] pAclCompiler the ACL compiler instance
    /// \return true if the requested module could be retrieved, false otherwise
    bool GetACLModule(bool useHSAILPath, ACLModule*& pAclModule, aclCompiler*& pAclCompiler);

    /// Unloads and frees any ACL modules loaded
    void UnloadAllACLModules();

private:
    /// constructor
    ACLModuleManager();

    ACLModule*   m_pAclModule12; ///< ACL module for OCL 1.2 and earlier
    aclCompiler* m_pCompiler12;  ///< aclCompiler object for OCL  1.2 and earlier

    ACLModule*   m_pAclModule20; ///< ACL module for OCL 2
    aclCompiler* m_pCompiler20;  ///< aclCompiler object for OCL 2
};

#endif // _ACL_MODULE_MANAGER_H_
