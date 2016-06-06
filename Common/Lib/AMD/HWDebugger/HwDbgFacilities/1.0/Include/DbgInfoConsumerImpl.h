//==============================================================================
// Copyright (c) 2012-2015 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools
/// \file
/// \brief  Definition of the DbgInfoConsumerImpl
//==============================================================================
#ifndef DBGINFOCONSUMERIMPL_H_
#define DBGINFOCONSUMERIMPL_H_

// STL:
#include <iterator>

// Local:
#include <DbgInfoIConsumer.h>
#include <DbgInfoLines.h>
#include <DbgInfoData.h>
#include <DbgInfoUtils.h>

namespace HwDbg
{

/// -----------------------------------------------------------------------------------------------
/// \class DbgInfoConsumerImpl
/// \brief Description: A generic (one level) implementation of a consumer
/// Both \a SetLineNumberMap and \a SetCodeScope need to be called if one wants to use the relevant
/// consumer functions relying on these classes
/// \tparam AddrType - The address type
/// \tparam LineType - The line Type
/// \tparam VarLocationType - The variable location type
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
class DbgInfoConsumerImpl : public DbgInfoIConsumer<AddrType, LineType, VarLocationType>
{
public:
    /// Some typedefs for ease of use:
    //@{
    typedef LineNumberMapping<AddrType, LineType> FullLineNumberMapping;
    typedef DbgInfoCodeContext<AddrType, LineType> DbgInfoCallStackFrame;
    typedef VariableInfo<AddrType, VarLocationType> ConsumedVariableInfo;
    typedef CodeScope<AddrType, LineType, VarLocationType> ConsumedCodeScope;
    typedef typename ConsumedVariableInfo::VariableMatchingFunc VarMatchFunc;
    //@}

public:
    /// Constructor
    DbgInfoConsumerImpl();
    /// Destructor
    virtual ~DbgInfoConsumerImpl();
    /// LineNumberMapping Getter
    LineNumberMapping<AddrType, LineType> LineNumberMap() const {return m_pMapping;};
    /// LineNumberMapping Setter
    void SetLineNumberMap(FullLineNumberMapping* pMapping) {m_pMapping = pMapping;};
    /// CodeScope Setter
    void SetCodeScope(ConsumedCodeScope* pTopCodeScope) {m_pTopCodeScope = pTopCodeScope;};
    /// CodeScope Getter
    //ConsumedCodeScope* CodeScope() {return m_pTopCodeScope;};

    /// Get the Line mapped to the Address - or the nearest mapped line if not found
    virtual bool GetLineFromAddress(const AddrType& addr, LineType& o_line) const;
    /// Get the Addresses mapped to the Line - or the nearest mapped addresses if not found
    virtual bool GetAddressesFromLine(const LineType& line, std::vector<AddrType>& o_addresses, bool append = false, bool firstAddr = false) const;
    /// Get the nearest mapped line to the Line
    virtual bool GetNearestMappedLine(const LineType& line, LineType& o_nearestMappedLine) const;
    /// Get the nearest mapped Address to the Address
    virtual bool GetNearestMappedAddress(const AddrType& addr, AddrType& o_nearestMappedAddress) const;
    /// Gets all the mapped addresses
    virtual bool GetMappedAddresses(std::vector<AddrType>& o_addresses) const;
    /// Fills a vector of lines representing the CallStack
    virtual bool GetAddressVirtualCallStack(const AddrType& startAddr, std::vector<DbgInfoCallStackFrame>& io_stack) const;
    /// Fills a vector of addresses representing the cached addresses in the scope
    virtual bool GetCachedAddresses(const AddrType& startAddr, bool includeCurrentScope, std::vector<AddrType>& o_cachedAddresses) const;
    /// Gets the variable info given an address and a variable name
    virtual bool GetMatchingVariableInfoInCurrentScope(const AddrType& startAddr, VarMatchFunc pfnMatch, const void* pMatchData, ConsumedVariableInfo& o_variable) const;
    /// Gets the frame base
    virtual bool GetFrameBase(AddrType startAddr, const std::string& varName, VarLocationType& o_frameBase) const;
    /// Gets all the variables in a given scope at a given stack frame depth
    virtual bool ListVariablesFromAddress(const AddrType& addr, int stackFrameDepth, bool finalMembers, std::vector<std::string>& o_variableNames) const;
    /// Gets the stack depth of an address
    virtual int GetAddressStackDepth(const AddrType& addr) const;

private:
    void addLeafMemberNamesToVector(const ConsumedVariableInfo& rVarInfo, const std::string& namesBase, std::vector<std::string>& io_variableNames) const;

private:
    /// A pointer to a Line-Number mapping
    FullLineNumberMapping* m_pMapping;
    /// A pointer to a Code Scope
    HwDbg::CodeScope<AddrType, LineType, VarLocationType>* m_pTopCodeScope;
}; // class DbgInfoConsumerImpl

/// ---------------------------------------------------------------------------
/// DbgInfoConsumerImpl
/// \brief Description: Default Constructor
/// ---------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
DbgInfoConsumerImpl<AddrType, LineType, VarLocationType>::DbgInfoConsumerImpl()
{
    m_pMapping = nullptr;
    m_pTopCodeScope = nullptr;
}


/// ---------------------------------------------------------------------------
/// ~DbgInfoConsumerImpl
/// \brief Description: Virtual destructor
/// ---------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
DbgInfoConsumerImpl<AddrType, LineType, VarLocationType>::~DbgInfoConsumerImpl()
{

}

/// ---------------------------------------------------------------------------
/// ClearMembers
/// \param[in] pVarInfo - address for which to retrieve line
/// \param[in] namesBase - The base of names to add to the vector (the current parent's full name)
/// \param[inout] io_variableNames - the names vector to which we append.
/// \brief Description: Recursively appends to io_variableNames the names of all leaf members of pVarInfo
/// \return void
/// ---------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
void DbgInfoConsumerImpl<AddrType, LineType, VarLocationType>::addLeafMemberNamesToVector(const ConsumedVariableInfo& rVarInfo, const std::string& namesBase, std::vector<std::string>& io_variableNames) const
{
    std::string namesBaseToUse = namesBase;

    // If the current variable is a pointer:
    if ((HWDBGINFO_VIND_POINTER == rVarInfo.m_varIndirection) || (HWDBGINFO_VIND_ARRAY == rVarInfo.m_varIndirection) /* || (DV_POINTER_ENCODING == rVarInfo.m_varEncoding) */)
    {
        // Pointers should be added as reference and dereferenced:
        io_variableNames.push_back(namesBaseToUse);
        string_prepend(namesBaseToUse, "(*") += ')';
    }

    const std::vector<ConsumedVariableInfo>& variableMembers = rVarInfo.m_varMembers;
    int numberOfMembers = (int)variableMembers.size();

    if (numberOfMembers < 1)
    {
        // This is a leaf, add it:
        io_variableNames.push_back(namesBaseToUse);
    }
    else // numberOfMembers >= 1
    {
        // Iterate the members:
        std::string memberNamesBase;

        for (int i = 0; i < numberOfMembers; i++)
        {
            // Get the member data:
            const ConsumedVariableInfo& currentMember = variableMembers[i];
            memberNamesBase = namesBaseToUse;
            (memberNamesBase += '.') += currentMember.m_varName;

            // Recurse into the member:
            addLeafMemberNamesToVector(currentMember, memberNamesBase, io_variableNames);
        }
    }
}

/// ---------------------------------------------------------------------------
/// GetLineFromAddress
/// \brief Description: Gets the line for a certain address
/// \param[in] addr - address for which to retrieve line
/// \param[out] o_line - the line to retrieve
/// \return Success / failure.
/// ---------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
bool DbgInfoConsumerImpl<AddrType, LineType, VarLocationType>::GetLineFromAddress(const AddrType& addr, LineType& o_line) const
{
    bool retVal = false;

    if (m_pMapping != nullptr)
    {
        AddrType mappedAddr = (HwDbgUInt64)0;
        bool found = m_pMapping->GetNearestMappedAddress(addr, mappedAddr);

        if (!found)
        {
            mappedAddr = addr;
        }

        retVal = m_pMapping->GetLineFromAddress(mappedAddr, o_line);
    }

    return retVal;
}

/// ---------------------------------------------------------------------------
/// GetAddressesFromLine
/// \brief Description: Gets vector of Addresses for a certain Line
/// \param[in] line - which line to retrieve addresses for
/// \param[out] o_addresses - the addresses to retrieve
/// \param[in] append - whether to append to the output vector or to clear it first
/// \return Success / failure.
/// ---------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
bool DbgInfoConsumerImpl<AddrType, LineType, VarLocationType>::GetAddressesFromLine(const LineType& line, std::vector<AddrType>& o_addresses, bool append, bool firstAddr) const
{
    bool retVal = false;

    if (m_pMapping != nullptr)
    {
        LineType mappedLine;
        bool found = m_pMapping->GetNearestMappedLine(line, mappedLine);

        if (!found)
        {
            mappedLine = line;
        }

        if (firstAddr)
        {
            if (!append)
            {
                o_addresses.clear();
            }

            std::vector<AddrType> addrs;
            retVal = m_pMapping->GetAddressesFromLine(mappedLine, addrs);

            if (retVal && (0 < addrs.size()))
            {
                o_addresses.push_back(addrs[0]);
            }
        }
        else
        {
            retVal = m_pMapping->GetAddressesFromLine(mappedLine, o_addresses, append);
        }
    }

    return retVal;
}

/// ---------------------------------------------------------------------------
/// GetNearestMappedLine
/// \brief Description: Get the nearest mapped line to the specified line
/// \param[in] line - the line to look for
/// \param[out] o_nearestMappedLine - the nearest mapped line to return
/// \return Success / failure.
/// ---------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
bool DbgInfoConsumerImpl<AddrType, LineType, VarLocationType>::GetNearestMappedLine(const LineType& line, LineType& o_nearestMappedLine) const
{
    bool retVal = false;

    if (m_pMapping != nullptr)
    {
        retVal = m_pMapping->GetNearestMappedLine(line, o_nearestMappedLine);
    }

    return retVal;
}

/// ---------------------------------------------------------------------------
/// GetNearestMappedAddress
/// \brief Description: Gets the nearest mapped address
/// \param[in] addr - the address to look for
/// \param[out] o_nearestMappedAddress - the nearest mapped address to return
/// \return Success / failure.
/// ---------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
bool DbgInfoConsumerImpl<AddrType, LineType, VarLocationType>::GetNearestMappedAddress(const AddrType& addr, AddrType& o_nearestMappedAddress) const
{
    bool retVal = false;

    if (m_pMapping != nullptr)
    {
        retVal = m_pMapping->GetNearestMappedAddress(addr, o_nearestMappedAddress);
    }

    return retVal;
}


/// ---------------------------------------------------------------------------
/// GetMappedAddresses
/// \brief Description: Gets the mapped addresses from the line number mapping
/// \param[out] o_addresses - out param
/// \return Success / failure.
/// ---------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
bool DbgInfoConsumerImpl<AddrType, LineType, VarLocationType>::GetMappedAddresses(std::vector<AddrType>& o_addresses) const
{
    bool retVal = false;
    o_addresses.clear();

    if (m_pMapping != nullptr)
    {
        retVal = m_pMapping->GetMappedAddresses(o_addresses);
    }

    return retVal;
}

/// ---------------------------------------------------------------------------
/// GetFrameBase
/// \brief Description: Gets the frame base that holds the frame pointer for varName at startAddr
///                THis is used to calculate variable locations along with the offset.
/// \param[in] startAddr - the address with which to seek the scope.
/// \param[in] varName - the variable name with which to seek the scope.
/// \param[out] o_frameBase - the frame base member of the scope.
/// \return Success / Fail
/// ---------------------------------------------------------------------------

template<typename AddrType, typename LineType, typename VarLocationType>
bool DbgInfoConsumerImpl<AddrType, LineType, VarLocationType>::GetFrameBase(AddrType startAddr, const std::string& varName, VarLocationType& o_frameBase) const
{
    bool retVal = false;

    if (m_pTopCodeScope != nullptr)
    {
        // Get the variable's scope:
        ConsumedVariableInfo tmpVar;
        const ConsumedCodeScope* pCurrentScope = m_pTopCodeScope->FindClosestScopeContainingVariable(startAddr, DbgInfoIConsumer<AddrType, LineType, VarLocationType>::MatchVariableByName, (const void*)(&varName), tmpVar);

        while (pCurrentScope != nullptr)
        {
            // If we have a frame pointer:
            if (pCurrentScope->m_pFrameBase != nullptr)
            {
                // return it:
                o_frameBase = *(pCurrentScope->m_pFrameBase);
                retVal = true;
                break;
            }

            // Continue to the next parent:
            pCurrentScope = pCurrentScope->m_pParentScope;
        }
    }

    return retVal;
}

/// ---------------------------------------------------------------------------
/// GetAddressStackDepth
/// \brief Description: Gets the stack depth of a given address
/// \param[in] addr - the address for which to retrieve the depth
/// \return stack depth
/// ---------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
int DbgInfoConsumerImpl<AddrType, LineType, VarLocationType>::GetAddressStackDepth(const AddrType& addr) const
{
    int retVal = 0;

    if (m_pTopCodeScope != nullptr)
    {
        // Go up the frame hierarchy looking for functions:
        const ConsumedCodeScope* pScope = m_pTopCodeScope->FindSmallestScopeContainingAddress(addr);

        while (pScope != nullptr)
        {
            // If this is a function, increment the counter:
            if (pScope->m_scopeType == ConsumedCodeScope::DID_SCT_INLINED_FUNCTION || pScope->m_scopeType == ConsumedCodeScope::DID_SCT_FUNCTION)
            {
                ++retVal;
            }

            // Go up one level:
            pScope = pScope->m_pParentScope;
        }
    }

    return retVal;
}

/// ---------------------------------------------------------------------------
/// GetAddressVirtualCallStack
/// \brief Description: Fills a vector of lines representing the call stack. take the first line from the
/// LineNumberMapping container, and take the rest from the scopes' inline member if it exists. the first address is startAddr, the rest are the lowest addresses in the scopes
/// of the inlined functions from which we take the line type.
/// \param[in] startAddr - the address for which to get the call stack
/// \param[in,out] io_stack - Out param use to retrieve the call stack as a vector of lines and their addresses.
/// \return Success / failure.
/// ---------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
bool DbgInfoConsumerImpl<AddrType, LineType, VarLocationType>::GetAddressVirtualCallStack(const AddrType& startAddr, std::vector<DbgInfoCallStackFrame>& io_stack) const
{
    bool retVal = false;

    if (m_pMapping != nullptr && m_pTopCodeScope != nullptr)
    {
        // Add the first line from the mapping:
        LineType startLine;
        DbgInfoCallStackFrame currentFrame;

        if (m_pMapping->GetLineFromAddress(startAddr, startLine))
        {
            retVal = true;
            currentFrame.m_sourceLocation = startLine;
            currentFrame.m_programCounter = startAddr;
        }

        // Find the bottom scope containing the address:
        const ConsumedCodeScope* pScope = m_pTopCodeScope->FindSmallestScopeContainingAddress(startAddr);

        std::string lastFuncName;
        AddrType lastFunctionBase = (HwDbgUInt64)0;
        m_pTopCodeScope->GetLowestAddressInScope(lastFunctionBase);
        AddrType lastModuleBase = lastFunctionBase;

        while (pScope != nullptr)
        {
            if (lastFuncName.empty())
            {
                lastFuncName = pScope->m_scopeName;
            }

            // For each scope, if it is an inline function, add the line number to the container o_lines:
            if (pScope->m_scopeType == ConsumedCodeScope::DID_SCT_INLINED_FUNCTION)
            {
                AddrType lowAddr = (HwDbgUInt64)0;
                pScope->GetLowestAddressInScope(lowAddr);

                // Add the frame information for the frame we started:
                currentFrame.m_functionName = lastFuncName;
                currentFrame.m_functionBase = lastFunctionBase;
                currentFrame.m_moduleBase = lastModuleBase;
                io_stack.push_back(currentFrame);

                // Add the frame information for the frame we are starting:
                LineType line = pScope->m_inlineInfo.m_inlinedAt;

                currentFrame.m_sourceLocation = line;
                currentFrame.m_programCounter = lowAddr;

                // Clear the containers:
                static const AddrType zeroAddr = (HwDbgUInt64)0;
                lastFuncName.clear();
                lastFunctionBase = zeroAddr;
                lastModuleBase = zeroAddr;
            }

            // Iterate up until the top scope:
            pScope = pScope->m_pParentScope;

            if (nullptr != pScope)
            {
                AddrType newScopeLowAddr = (HwDbgUInt64)0;
                bool rcNL = pScope->GetLowestAddressInScope(newScopeLowAddr);

                if ((rcNL) && (newScopeLowAddr))
                {
                    lastFunctionBase = newScopeLowAddr;
                    lastModuleBase = newScopeLowAddr;
                }
            }
        }

        // Fill in the last frame:
        currentFrame.m_functionName = lastFuncName;
        currentFrame.m_functionBase = lastFunctionBase;
        currentFrame.m_moduleBase = lastModuleBase;
        io_stack.push_back(currentFrame);
    }

    return retVal;
}


/// ---------------------------------------------------------------------------
/// GetCachedAddresses
/// \brief Description: Retrieves the cached address from all inline scopes containing the address
/// \param[in] startAddr - the address
/// \param[in] includeCurrentScope - whether to include the first scope
/// \param[out] o_cachedAddresses - out param containing the addresses
/// \return Success / failure.
/// ---------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
bool DbgInfoConsumerImpl<AddrType, LineType, VarLocationType>::GetCachedAddresses(const AddrType& startAddr, bool includeCurrentScope, std::vector<AddrType>& o_cachedAddresses) const
{
    bool retVal = false;
    bool takeScope = includeCurrentScope;

    if (m_pMapping != nullptr && m_pTopCodeScope != nullptr)
    {
        const ConsumedCodeScope* pScope = m_pTopCodeScope->FindSmallestScopeContainingAddress(startAddr);

        while (pScope != nullptr)
        {
            // For each scope, if it is an inline function, add the cached addresses
            if ((pScope->m_scopeType == ConsumedCodeScope::DID_SCT_INLINED_FUNCTION) || (pScope->m_scopeType == ConsumedCodeScope::DID_SCT_FUNCTION))
            {
                // If we have not included the current scope - skip the first entry and set the flag so that we will take it next time:
                if (!takeScope)
                {
                    takeScope = true;
                }
                // If this is a scope we want to add, take its cached addresses and copy them into the back param.
                else
                {
                    // Copy set to vector:
                    std::copy(pScope->m_addressCache.begin(), pScope->m_addressCache.end(), std::back_inserter(o_cachedAddresses));
                    retVal = true;
                }
            }

            // Go up one level:
            pScope = pScope->m_pParentScope;
        }
    }

    return retVal;
}

/// ---------------------------------------------------------------------------
/// GetVariableInfoInCurrentScope
/// \brief Description: Returns the lowest scope containing the variable, and
///                the variable info itself as an out param
/// \param[in] startAddr - The address to start looking in
/// \param[in] variableName - The variable name to match
/// \param[out] o_variable - out param containing the var info
/// \return Success / failure.
/// ---------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
bool DbgInfoConsumerImpl<AddrType, LineType, VarLocationType>::GetMatchingVariableInfoInCurrentScope(const AddrType& startAddr, VarMatchFunc pfnMatch, const void* pMatchData, ConsumedVariableInfo& o_variable) const
{
    bool retVal = false;

    if (nullptr != m_pTopCodeScope && nullptr != pfnMatch)
    {
        const ConsumedCodeScope* pScope = m_pTopCodeScope->FindClosestScopeContainingVariable(startAddr, pfnMatch, pMatchData, o_variable);

        if (pScope != nullptr)
        {
            retVal = true;
        }
    }

    return retVal;
}

/// ---------------------------------------------------------------------------
/// ListVariablesFromAddress
/// \brief Description: Gets a list of all variables defined in address's scope and all
///                containing scopes - not including leaves
/// \param[in] addr - the address in which to look for the scope
/// \param[in] stackFrameDepth - the stack frame depth in which to look
/// \param[in] finalMembers - true = get leaf descendants, false = get (direct) children
/// \param[out] o_variableNames - output parameter the list of variable names in the address's scope.
/// \return Success / failure.
/// ---------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
bool DbgInfoConsumerImpl<AddrType, LineType, VarLocationType>::ListVariablesFromAddress(const AddrType& addr, int stackFrameDepth, bool finalMembers, std::vector<std::string>& o_variableNames) const
{
    bool retVal = false;

    if (m_pTopCodeScope != nullptr)
    {
        // Get the address's scope:
        const ConsumedCodeScope* pAddrScope = m_pTopCodeScope->FindSmallestScopeContainingAddress(addr);

        if (pAddrScope != nullptr)
        {
            retVal = true;

            int currentStackDepth = 0;
            int totalStackDepth = m_pTopCodeScope->GetStackDepth(addr);

            // Go up in the hierarchy, adding all variables in each scope:
            const ConsumedCodeScope* pCurrentScope = pAddrScope;

            while (pCurrentScope != nullptr)
            {
                // Get the variables vector:
                const std::vector<ConsumedVariableInfo*>& currentScopeVariables = pCurrentScope->m_scopeVars;

                // We add the variables in three cases:
                // 1. The parameter is -1, signifying the caller wants all the locals from all levels.
                // 2. This is the requested scope
                // 3. This is the global scope (outside the kernel)
                if ((stackFrameDepth == -1) || (stackFrameDepth == currentStackDepth) || (currentStackDepth == totalStackDepth))
                {
                    // Get all the variables:
                    int numberOfVariables = (int)currentScopeVariables.size();

                    for (int i = 0; i < numberOfVariables; i++)
                    {
                        // Sanity check:
                        const ConsumedVariableInfo* pCurrentVariable = currentScopeVariables[i];
                        HWDBG_ASSERT(pCurrentVariable != nullptr);

                        if (pCurrentVariable != nullptr)
                        {
                            // Check the variable's scope:
                            if (pCurrentVariable->IsConst() || ((pCurrentVariable->m_highVariablePC >= addr) && (pCurrentVariable->m_lowVariablePC <= addr)))
                            {
                                if (finalMembers)
                                {
                                    // Add any leaves to the vector:
                                    addLeafMemberNamesToVector(*pCurrentVariable, pCurrentVariable->m_varName, o_variableNames);
                                }
                                else // !finalMembers
                                {
                                    // Add the variable to the vector:
                                    o_variableNames.push_back(pCurrentVariable->m_varName);
                                }
                            }
                        }
                    }
                }

                // If this is a function, the next frame will be a higher level:
                if ((pCurrentScope->m_scopeType == ConsumedCodeScope::DID_SCT_INLINED_FUNCTION) || (pCurrentScope->m_scopeType == ConsumedCodeScope::DID_SCT_FUNCTION))
                {
                    currentStackDepth++;
                }

                // Go to this scope's parent scope:
                pCurrentScope = pCurrentScope->m_pParentScope;
            }
        }
    }

    return retVal;
}

} // namespace HwDbg

#endif //DBGINFOCONSUMERIMPL_H_
